#include "Tokenizer.hpp"
#include "ConfigError.hpp"

static Token tokenize_delims(char c, std::size_t line_c)
{
	TokenType token_type;
	std::string text;
	switch (c)
	{
		case ('{'):
		{
			token_type = TokenType::LBRACE;
			text = "{";
			break;
		}
		case ('}'):
		{
			token_type = TokenType::RBRACE;
			text = "}";
			break;
		}
		case (';'):
		{
			token_type = TokenType::SEMICOLON;
			text = ";";
			break;
		}
	}
	Token token(token_type, text, line_c);
	return (token);
}

void Tokenizer::checkClosingQuote(std::size_t line_temp)
{
	if (!_str[_pos])
	{
		// check for null terminator in middle of string
		if (_pos < _str.length())
			throw ConfigError("unexpected \"\\0\"", line_temp);
		// check for missing closing quote
		throw ConfigError("unexpected end of file, expecting \";\" or \"}\"", line_temp);
	}

	// check for illegal character after closing quote
	std::string allowed_chars = " \t\n\r;{)";
	if (_str[_pos + 1] && allowed_chars.find(_str[_pos + 1]) == std::string::npos)
		throw ConfigError("unexpected \"" + std::string(1, _str[_pos + 1]) + "\"", line_temp);
}

void Tokenizer::handleBackslash(std::string &text, std::size_t &quoted_newline_c)
{
	// backslash before these chars make backslash disappear
	std::string consumes_backslash = "'\"\\";
	std::string converts_backslash = "nrt";

	if (_str[_pos + 1])
	{
		if (consumes_backslash.find(_str[_pos + 1]) != std::string::npos)
		{
			// consume backslash
			_pos++;
			text += _str[_pos];
			_pos++;
		}
		else if (converts_backslash.find(_str[_pos + 1]) != std::string::npos)
		{
			// consume backslash
			_pos++;
			switch (_str[_pos])
			{
				case ('n'):
				{
					text += '\n';
					break;
				}
				case ('r'):
				{
					text += '\r';
					break;
				}
				case ('t'):
				{
					text += '\t';
					break;
				}
			}
			// consume letter
			_pos++;
		}
		else
		{
			// add backslash
			text += _str[_pos];
			_pos++;
			text += _str[_pos];
			if (_str[_pos] == '\n')
				quoted_newline_c++;
			_pos++;
		}
	}
	// backslash is the last char of the file: keep it as a literal char
	// (an unfinished directive at end of file is reported by the parser)
	else
	{
		text += _str[_pos];
		_pos++;
	}
}

Token Tokenizer::tokenizeWord()
{
	std::string text;
	// word ends upon delimiter or a whitespace also used by nginx
	std::string ends_word = "{;\n \t\n\r";
	// a newline in quotes should only increment line_c after the WORD was stored
	// rule: each token gets the line where it starts
	std::size_t quoted_newline_c = 0;

	// CASE 1: no quote starting the WORD
	if (_str[_pos] != '\'' && _str[_pos] != '"')
	{
		while (_str[_pos] && ends_word.find(_str[_pos]) == std::string::npos)
		{
			// handle envar indicator ${ (WORD doesn't stop at {)
			if (_str[_pos] == '$' && _str[_pos + 1] == '{')
			{
				text += "${";
				_pos += 2;
				// nginx's tokenizer accepts stuff like: ${{x (gets rejected by nginx's parser) --> same behavior intended here
				while (_str[_pos] == '{')
				{
					text += _str[_pos];
					_pos++;
				}
			}
			// handle unquoted backslash -> next char is escaped
			else if (_str[_pos] == '\\')
				handleBackslash(text, quoted_newline_c);
			else
			{
				text += _str[_pos];
				_pos++;
			}
		}
		if (_str[_pos] == '\n')
		{
			quoted_newline_c++;
			_pos++;
		}
	}
	// CASE 2: quote starts the WORD
	else
	{
		char q = _str[_pos];

		// consume quote
		_pos++;

		while (_str[_pos] != q && _str[_pos])
		{
			if (_str[_pos] == '\\')
				handleBackslash(text, quoted_newline_c);
			else
			{
				text += _str[_pos];
				if (_str[_pos] == '\n')
					quoted_newline_c++;
				_pos++;
			}
		}
		checkClosingQuote(_line + quoted_newline_c);

		// consume closing quote
		_pos++;
	}
	Token token(TokenType::WORD, text, _line);

	_line += quoted_newline_c;
	return (token);
}

std::vector<Token> Tokenizer::tokenize()
{
	// reset should tokenize() be used more than once (unlikely)
	_pos = 0;
	_line = 1;
	std::vector<Token> tokens;
	std::size_t str_len = _str.length();

	std::string delimiter = "{};";
	std::string whitespace = " \t\n\r";

	while (_pos < str_len)
	{
		char c = _str[_pos];
		// unexpected null terminator
		// also catches null terms that make helper stop and return to here
		if (!_str[_pos])
			throw ConfigError("unexpected \"\\0\"", _line);
		else if (c == '\n')
		{
			_line++;
			_pos++;
		}
		else if (whitespace.find(c) != std::string::npos)
			_pos++;
		else if (c == '#')
			while (_str[_pos] && _str[_pos] != '\n')
				_pos++;
		else if (delimiter.find(c) != std::string::npos)
		{
			tokens.push_back(tokenize_delims(c, _line));
			// consume delim
			_pos++;
		}
		else
			tokens.push_back(tokenizeWord());
	}
	// emplace_back contstructs a T (here: Token) from the given arguments in the container's memory storage and push_backs it
	tokens.emplace_back(TokenType::END, "end of file", _line);
	return (tokens);
}

Tokenizer::Tokenizer(const std::string &str) : _str(str), _pos(0), _line(1)
{
}
