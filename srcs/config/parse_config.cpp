#include "parse_config.hpp"
#include "Config.hpp"
#include "Token.hpp"

#include <cctype>
#include <fstream> // for std::ifstream
#include <iostream>
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

// static Token tokenize_delims(char c, std::size_t line_c)
// {
// 	TokenType token_type;
// 	std::string text;
// 	switch (c)
// 	{
// 		case ('{'):
// 		{
// 			token_type = TokenType::LBRACE;
// 			text = "{";
// 			break;
// 		}
// 		case ('}'):
// 		{
// 			token_type = TokenType::RBRACE;
// 			text = "}";
// 			break;
// 		}
// 		case (';'):
// 		{
// 			token_type = TokenType::SEMICOLON;
// 			text = ";";
// 			break;
// 		}
// 	}
// 	Token token(token_type, text, line_c);
// 	return (token);
// }

// static std::string create_err_msg(std::string msg, std::size_t line_c)
// {
// 	std::string error_msg = msg;
// 	error_msg += line_c;
// 	error_msg += ".";
// 	return (error_msg);
// }

// static Token tokenize_word(const std::string &line, std::size_t line_c)
// {
// 	std::string word_end = "{};\n";
// 	std::string text = "";
// 	bool squotes = false;
// 	bool dquotes = false;
// 	std::size_t squote_c = 0;
// 	std::size_t dquote_c = 0;

// 	for (char c : line)
// 	{
// 		if (c == '"' && !squotes)
// 		{
// 			// switch from true to false or vice versa
// 			// ternary is a bit overkill but I always wanted to use one
// 			dquotes = (dquotes) ? false : true;
// 			dquote_c++;
// 			continue;
// 		}
// 		if (c == '\'' && !dquotes)
// 		{
// 			squotes = (squotes) ? false : true;
// 			squote_c++;
// 			continue;
// 		}
// 		else if (word_end.find(c) == std::string::npos)
// 			text += c;
// 		else if (word_end.find(c) != std::string::npos)
// 		{
// 			if (squotes || dquotes)
// 				text += c;
// 			else
// 				break;
// 		}
// 	}

// 	// verification of finished text
// 	if (squote_c % 2 != 0)
// 		throw std::invalid_argument(create_err_msg("ERROR: malformed config: invalid single quotes in line ", line_c));
// 	if (dquote_c % 2 != 0)
// 		throw std::invalid_argument(create_err_msg("ERROR: malformed config: invalid double quotes in line ", line_c));
// 	Token token(TokenType::WORD, text, line_c);
// 	return (token);
// }

// static std::vector<Token> tokenize(std::ifstream &filestream)
// {
// 	std::vector<Token> tokens;
// 	std::size_t line_c = 1;
// 	std::string delimiter = "{};";

// 	while (true)
// 	{
// 		if (filestream.eof())
// 			break;

// 		std::string line;
// 		getline(filestream, line);
// 		std::size_t pos = 0;
// 		for (char c : line)
// 		{
// 			// NEWLINE ADVANCE
// 			if (c == '\n')
// 			{
// 				line_c++;
// 				break;
// 			}
// 			// WHITESPACE SKIP
// 			if (std::isspace(static_cast<unsigned char>(c))) // cast to handle umlauts and the like; exceed 127, which wraps to negative, which causes undefined behavior in  std::isspace()
// 			{
// 				pos++;
// 				continue;
// 			} // COMMENT ADVANCE
// 			if (c == '#')
// 			{
// 				line_c++;
// 				break;
// 			}
// 			if (delimiter.find(c) != std::string::npos)
// 			{
// 				tokens.push_back(tokenize_delims(c, line_c));
// 				pos++;
// 			}
// 			else
// 			{
// 				tokens.push_back(tokenize_word(line, line_c));
// 				pos += tokens.back().text().length();
// 			}
// 		}
// 	}
// 	return (tokens);
// }

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

// static std::vector<Token> tokenize(const std::string &str)
// {
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

	// DEBUG
	std::string str = condense_to_string(filestream);
	std::cout << str;
	// DEBUG END

	// std::vector<Token> tokens = tokenize(condense_to_string(filestream));

	// if (tokens.back().tokenType() != TokenType::END)
	// 	throw std::logic_error("ERROR/DEBUG: lexer returned malformed Token vector (does not end with END type Token).");

	Config config;
	return (config);
}
