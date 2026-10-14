#include "parse_config.hpp"
#include "Config.hpp"
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
	// choosing ifstream over fstream means: "in" = "read" only
	// as such, writability is never checked, as it isn't needed
	// readable, but non-writable config is not declined
	std::ifstream filestream(filepath);
	// std::ifstreams destructor closes the FD automatically; no filestream.close() needed!
	if (!filestream.is_open())
		throw std::invalid_argument("ERROR: unable to open config file.");
	return (filestream);
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

static Token tokenize_word(const std::string &str, std::size_t &pos, std::size_t &line_c, char quote = 'x')
{
	std::string text;
	// word ends upon delimiter or a whitespace also used by nginx
	std::string word_end = "{};\n \t\n\r";
	// a newline in quotes should only increment line_c after the WORD was stored
	// rule: each token gets the line where it starts
	std::size_t quoted_newline_c = 0;

	if (quote == 'x')
	{
		while (word_end.find(str[pos]) == std::string::npos)
		{
			text += str[pos];
			pos++;
		}
		if (str[pos] == '\n')
		{
			line_c++;
			pos++;
		}
	}
	else
	{
		if (quote == '\'')
			while (str[pos] != '\'' && str[pos])
			{
				text += str[pos];
				if (str[pos] == '\n')
					quoted_newline_c++;
				pos++;
			}
		else if (quote == '"')
			while (str[pos] != '"' && str[pos])
			{
				text += str[pos];
				if (str[pos] == '\n')
					quoted_newline_c++;
				pos++;
			}
		if (!str[pos])
		{
			std::string error_msg = "ERROR: unclosed quotes in line: " + std::to_string(line_c);
			throw std::invalid_argument(error_msg);
		}
		// consume quote
		pos++;
	}
	Token token(TokenType::WORD, text, line_c);
	line_c += quoted_newline_c;
	return (token);
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
		throw std::runtime_error("ERROR: failed to read config file.");
	return (str);
}

static std::vector<Token> tokenize(const std::string &str)
{
	// result setup
	std::vector<Token> tokens;
	std::size_t line_c = 1;

	// string traversal setup
	std::string delimiter = "{};";
	std::string whitespace = " \t\n\r";
	std::size_t pos = 0;
	std::size_t str_len = str.length();

	// quotes management setup
	bool squotes = false;
	bool dquotes = false;

	while (pos < str_len)
	{
		char c = str[pos];

		if (str[pos] == '#')
			while (str[pos] != '\n' && str[pos])
				pos++;

		else if (c == '"' && !squotes)
		{
			// switch from true to false or vice versa
			// ternary is a bit overkill but I always wanted to use one
			dquotes = (dquotes) ? false : true;
			pos++;
		}
		else if (c == '\'' && !dquotes)
		{
			squotes = (squotes) ? false : true;
			pos++;
		}
		else if (!squotes && !dquotes)
		{
			// DELIMITER
			if (delimiter.find(c) != std::string::npos)
			{
				tokens.push_back(tokenize_delims(c, line_c));
				pos++;
			}
			// WORD
			else
			{
				if (c == '\n')
				{
					pos++;
					line_c++;
				}
				else if (whitespace.find(c) != std::string::npos)
					pos++;
				else
					tokens.push_back(tokenize_word(str, pos, line_c));
			}
		}
		// QUOTED WORD
		else
		{
			if (squotes)
			{
				tokens.push_back(tokenize_word(str, pos, line_c, '\''));
				squotes = false;
			}
			else
			{
				tokens.push_back(tokenize_word(str, pos, line_c, '"'));
				dquotes = false;
			}
		}
	}
	// emplace_back contstructs a T (here: Token) from the given arguments in the container's memory storage and push_backs it
	tokens.emplace_back(TokenType::END, "end of file", line_c);
	return (tokens);
}

// TODO: mixed up braces and quotes here, needs total rework

// void verify_tokens(const std::vector<Token> &tokens)
// {
// 	std::string required_after_quote = " \t\n\r";
// 	for (auto it = tokens.begin(); it != tokens.end(); it++)
// 	{
// 		if (it->tokenType() == TokenType::RBRACE)
// 		{
// 			it++;
// 			if (required_after_rbrace.find(it->text()) == std::string::npos)
// 			{
// 				std::string error_msg = "ERROR: unexpected \"" + it->text() + "\" in line " + std::to_string(it->line()) + ").";
// 				throw std::invalid_argument(error_msg);
// 			}
// 		}
// 	}
// }

Config parseConfig(int argc, char **argv)
{
	if (argc > 2)
		throw std::invalid_argument("ERROR: invalid argument provided.\nCall via:\n\t\"./webserv\" (uses default path)\nor\n\t \"./webserv [PATH TO CONFIG FILE]\"");
	std::string filepath;
	if (argc == 1)
		filepath = DEFAULT_CONFIG;
	else
		filepath = argv[1];
	std::ifstream filestream = get_filestream(filepath);

	std::vector<Token> tokens = tokenize(condense_to_string(filestream));

	if (tokens.back().tokenType() != TokenType::END)
		throw std::logic_error("DEBUG: lexer returned malformed Token vector (does not end with END type Token).");

	// DEBUG
	for (Token t : tokens)
	{
		std::cout << "line " << t.line() << " = " << t.tokenTypeString() << ": " << t.text() << "\n";
	}
	// DEBUG END

	// verify_tokens(tokens);

	Config config;
	return (config);
}
