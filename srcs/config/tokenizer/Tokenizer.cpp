#include "Tokenizer.hpp"
#include "ConfigError.hpp"

const std::string DELIMITER = "{};";
const std::string WHITESPACE = " \t\n\r";
// does not include } since nginx makes appended } part of WORD
const std::string ENDS_UNQUOTED_WORD = WHITESPACE + "{;";
const std::string ALLOWED_AFTER_CLOSING_QUOTE = WHITESPACE + ";{)";

// HELPERS
static TokenType char_to_tokentype(char c)
{
	switch (c)
	{
		case ('{'):
			return (TokenType::LBRACE);
		case ('}'):
			return (TokenType::RBRACE);
		case (';'):
			return (TokenType::SEMICOLON);
		default:
		{
			std::string error_msg = "DEBUG: fed incompatible input to char_to_tokentype: " + std::string(1, c);
			throw std::logic_error(error_msg);
		}
	}
}

static char char_after_backslash(char c)
{
	switch (c)
	{
		case ('n'):
			return ('\n');
		case ('r'):
			return ('\r');
		case ('t'):
			return ('\t');
		default:
		{
			std::string error_msg = "DEBUG: fed incompatible input to char_after_backslash: " + std::string(1, c);
			throw std::logic_error(error_msg);
		}
	}
}

// CONSTRUCTOR
Tokenizer::Tokenizer(const std::string &str) : _str(str), _pos(0), _line(1)
{
}

// METHODS
std::vector<Token> Tokenizer::tokenize()
{
	// reset, should tokenize() be used more than once (unlikely)
	_pos = 0;
	_line = 1;
	std::vector<Token> tokens;
	std::size_t str_len = _str.length();

	while (_pos < str_len)
	{
		char c = _str[_pos];
		// catches \0 before it's legitimate position (EOF)
		if (!_str[_pos])
			throw ConfigError("unexpected \"\\0\"", _line);
		else if (WHITESPACE.find(c) != std::string::npos)
			advance();
		else if (c == '#')
			while (_str[_pos] && _str[_pos] != '\n')
				advance();
		else if (DELIMITER.find(c) != std::string::npos)
		{
			// emplace creates and pushes Token object
			tokens.emplace_back(char_to_tokentype(c), std::string(1, c), _line);
			// consume delim
			advance();
		}
		else
			tokens.push_back(tokenizeWord());
	}
	tokens.emplace_back(TokenType::END, "end of file", _line);
	return (tokens);
}

Token Tokenizer::tokenizeWord()
{
	// a newline in quotes should only increment line_c after the WORD was stored
	// helpers increment _line for tokenizations afterwards
	std::size_t word_start_line = _line;

	// CASE 1: WORD without quotes
	if (_str[_pos] != '\'' && _str[_pos] != '"')
		return (Token(TokenType::WORD, unquotedWord(), word_start_line));
	// CASE 2: WORD in quotes
	else
		return (Token(TokenType::WORD, quotedWord(), word_start_line));
}

std::string Tokenizer::unquotedWord()
{
	std::string text;

	while (_str[_pos] && ENDS_UNQUOTED_WORD.find(_str[_pos]) == std::string::npos)
	{
		// handle nginx variable ${ (WORD doesn't stop at {)
		if (_str[_pos] == '$' && _str[_pos + 1] == '{')
		{
			text += "${";
			// _pos += 2 would also be safe; 2x advance() for consistency
			advance();
			advance();
			// nginx's tokenizer accepts stuff like: ${{x (gets rejected by nginx's parser)
			// --> same behavior intended here
			while (_str[_pos] == '{')
			{
				text += _str[_pos];
				advance();
			}
		}
		// handle unquoted backslash -> next char is escaped
		else if (_str[_pos] == '\\')
			handleBackslash(text);
		else
		{
			text += _str[_pos];
			advance();
		}
	}
	return (text);
}

std::string Tokenizer::quotedWord()
{
	std::string text;
	char q = _str[_pos];

	// consume quote
	advance();

	while (_str[_pos] != q && _str[_pos])
	{
		if (_str[_pos] == '\\')
			handleBackslash(text);
		else
		{
			text += _str[_pos];
			advance();
		}
	}
	checkClosingQuote();
	// consume closing quote
	advance();
	return (text);
}

void Tokenizer::handleBackslash(std::string &text)
{
	// backslash before these chars make backslash disappear
	std::string consumes_backslash = "'\"\\";
	std::string converts_backslash = "nrt";

	if (_str[_pos + 1])
	{
		if (consumes_backslash.find(_str[_pos + 1]) != std::string::npos)
		{
			// consume backslash
			advance();
			text += _str[_pos];
			advance();
		}
		else if (converts_backslash.find(_str[_pos + 1]) != std::string::npos)
		{
			// consume backslash
			advance();
			text += char_after_backslash(_str[_pos]);
			// consume letter
			advance();
		}
		else
		{
			// add backslash
			text += _str[_pos];
			advance();
			text += _str[_pos];
			advance();
		}
	}
	// backslash is the last char of the file: keep it as a literal char
	// (an unfinished directive at end of file is reported by the parser)
	else
	{
		text += _str[_pos];
		advance();
	}
}

void Tokenizer::checkClosingQuote()
{
	if (!_str[_pos])
	{
		// check for null terminator in middle of string
		if (_pos < _str.length())
			throw ConfigError("unexpected \"\\0\"", _line);
		// check for missing closing quote
		throw ConfigError("unexpected end of file, expecting \";\" or \"}\"", _line);
	}

	// check for illegal character after closing quote
	if (_str[_pos + 1] && ALLOWED_AFTER_CLOSING_QUOTE.find(_str[_pos + 1]) == std::string::npos)
		throw ConfigError("unexpected \"" + std::string(1, _str[_pos + 1]) + "\"", _line);
}

void Tokenizer::advance()
{
	if (_str[_pos] == '\n')
		_line++;
	_pos++;
}
