#include "parse_config.hpp"
#include "Config.hpp"
#include "ConfigError.hpp"
#include "Token.hpp"

#include <cctype>
#include <fstream> // for std::ifstream
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

static std::ifstream get_filestream(const std::string &filepath)
{
	// choosing ifstream over fstream means: "i" = "in" = "read" only
	// as such, writability is never checked, as it isn't needed
	// readable, but non-writable config is not declined
	std::ifstream filestream(filepath);
	// std::ifstreams destructor closes the FD automatically; no filestream.close() needed!
	if (!filestream.is_open())
		throw ConfigError("unable to open config file", std::nullopt);
	return (filestream);
}

static std::string condense_to_string(std::ifstream &filestream)
{
	std::string str;
	std::string line;
	// reminder: getline() reads char by char until it hits a newline (swallows it) or runs out of data (sets .eof())
	while (getline(filestream, line))
	{
		str += line;
		if (!filestream.eof())
			str += '\n';
	}
	// throws if getline returns error or if a path to a directory (not file!) was provided (both set .bad() = 1)
	if (filestream.bad())
		throw ConfigError("failed to read config file", std::nullopt);
	return (str);
}

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

static void check_closing_quote(const std::string &str, std::size_t pos, std::size_t line_c)
{
	// check for missing closing quote
	if (!str[pos])
	{
		throw ConfigError("unexpected end of file, expecting \";\" or \"}\"", line_c);
	}

	// check for illegal character after closing quote
	std::string allowed_chars = " \t\n\r;{";
	if (str[pos + 1] && allowed_chars.find(str[pos + 1]) == std::string::npos)
	{
		throw ConfigError("unexpected \"" + std::string(1, str[pos + 1]) + "\"", line_c);
	}
}

// passing pos and line_c NOT as reference: scout ahead in string without actually incrementing

static void handle_backslash(const std::string &str, std::size_t &pos, std::string &text, std::size_t &quoted_newline_c)
{
	// backslash before these chars make backslash disappear
	std::string consumes_backslash = "'\"\\";
	std::string converts_backslash = "nrt";

	if (str[pos + 1])
	{
		if (consumes_backslash.find(str[pos + 1]) != std::string::npos)
		{
			// consume backslash
			pos++;
			text += str[pos];
			pos++;
		}
		else if (converts_backslash.find(str[pos + 1]) != std::string::npos)
		{
			// consume backslash
			pos++;
			switch (str[pos])
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
			pos++;
		}
		else
		{
			// add backslash
			text += str[pos];
			pos++;
			text += str[pos];
			if (str[pos] == '\n')
				quoted_newline_c++;
			pos++;
		}
	}
	// backslash is the last char of the file: keep it as a literal char
	// (an unfinished directive at end of file is reported by the parser)
	else
	{
		text += str[pos];
		pos++;
	}
}

static Token tokenize_word(const std::string &str, std::size_t &pos, std::size_t &line_c)
{
	std::string text;
	// word ends upon delimiter or a whitespace also used by nginx
	std::string ends_word = "{};\n \t\n\r";
	// a newline in quotes should only increment line_c after the WORD was stored
	// rule: each token gets the line where it starts
	std::size_t quoted_newline_c = 0;

	// CASE 1: no quote starting the WORD
	if (str[pos] != '\'' && str[pos] != '"')
	{
		while (str[pos] && ends_word.find(str[pos]) == std::string::npos)
		{
			// handle unquoted backslash -> next char is escaped
			if (str[pos] == '\\')
				handle_backslash(str, pos, text, quoted_newline_c);
			else
			{
				text += str[pos];
				pos++;
			}
		}
		if (str[pos] == '\n')
		{
			quoted_newline_c++;
			pos++;
		}
	}
	// CASE 2: quote starts the WORD
	else
	{
		char q = str[pos];
		if (str[pos] == q)
		{
			pos++;
			while (str[pos] != q && str[pos])
			{
				if (str[pos] == '\\')
					handle_backslash(str, pos, text, quoted_newline_c);
				else
				{
					text += str[pos];
					if (str[pos] == '\n')
						quoted_newline_c++;
					pos++;
				}
			}
			check_closing_quote(str, pos, line_c + quoted_newline_c);
		}
		// consume closing quote
		pos++;
	}
	Token token(TokenType::WORD, text, line_c);

	// // DEBUG
	// if (token.text().substr(0, 4) != "case")
	// 	std::cout << "line " << token.line() << " = " << token.tokenTypeString() << ": " << token.text() << "\n";
	// // DEBUG END

	line_c += quoted_newline_c;
	return (token);
}

// basically just decides what handler to use for a given char
static std::vector<Token> tokenize(const std::string &str)
{
	std::vector<Token> tokens;
	std::size_t line_c = 1;
	std::size_t pos = 0;
	std::size_t str_len = str.length();

	std::string delimiter = "{};";
	std::string whitespace = " \t\n\r";

	while (pos < str_len)
	{
		char c = str[pos];
		if (c == '\n')
		{
			line_c++;
			pos++;
		}
		else if (whitespace.find(c) != std::string::npos)
			pos++;
		else if (c == '#')
			while (str[pos] && str[pos] != '\n')
				pos++;
		else if (delimiter.find(c) != std::string::npos)
		{
			tokens.push_back(tokenize_delims(c, line_c));
			// consume delim
			pos++;
		}
		else
			tokens.push_back(tokenize_word(str, pos, line_c));
	}
	// emplace_back contstructs a T (here: Token) from the given arguments in the container's memory storage and push_backs it
	tokens.emplace_back(TokenType::END, "end of file", line_c);
	return (tokens);
}

Config parse_config(int argc, char **argv)
{
	if (argc > 2)
		throw std::invalid_argument("webserv: too many arguments\nusage: ./webserv [config file] (without a config file, the default one is used)");
	std::string filepath;
	if (argc == 1)
		filepath = DEFAULT_CONFIG;
	else
		filepath = argv[1];

	std::vector<Token> tokens;
	try
	{
		std::ifstream filestream = get_filestream(filepath);
		tokens = tokenize(condense_to_string(filestream));
	}
	catch (const ConfigError &c)
	{
		throw std::runtime_error(c.format(filepath));
	}
	// ConfigError inherits from std::exception
	// this is why this catch needs to come second, otherwise it would always come first (ConfigError is a std::runtime_error is a std::stdexception)
	catch (const std::exception &e)
	{
		throw;
	}

	if (tokens.back().tokenType() != TokenType::END)
		throw std::logic_error("DEBUG: lexer returned malformed Token vector (does not end with END type Token).");

	// DEBUG
	for (Token t : tokens)
	{
		std::cout << "line " << t.line() << " = " << t.tokenTypeString() << ": " << t.text() << "\n";
	}
	// DEBUG END

	Config config;
	return (config);
}
