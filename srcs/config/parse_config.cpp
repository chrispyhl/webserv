#include "parse_config.hpp"
#include "Config.hpp"
#include "ConfigError.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"

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
		Tokenizer t(condense_to_string(filestream));
		tokens = t.tokenize();
	}
	catch (const ConfigError &c)
	{
		throw std::runtime_error(c.format(filepath));
	}
	// any non-ConfigError gets send to caller automatically
	
	if (tokens.back().tokenType() != TokenType::END)
		throw std::logic_error("DEBUG: lexer returned malformed Token vector (does not end with END type Token).");

	// DEBUG
	for (const Token &t : tokens)
	{
		std::cout << "line " << t.line() << " = " << t.tokenTypeString() << ": " << t.text() << "\n";
	}
	// DEBUG END

	Config config;
	return (config);
}
