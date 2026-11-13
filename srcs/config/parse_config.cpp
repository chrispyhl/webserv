#include "Config.hpp"
#include "ConfigBuilder.hpp"
#include "ConfigError.hpp"
#include "Directive.hpp"
#include "Parser.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"

#include <fstream> // for std::ifstream
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef DEFAULT_CONFIG
#define DEFAULT_CONFIG "./configs/webserv.conf"
#endif

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

// static void debug_print_parser(const std::vector<Directive> &directives)
// {
// 	for (Directive d : directives)
// 	{
// 		std::cout << "name: " << d.name << "\nargs: [";
// 		for (std::string s : d.args)
// 			std::cout << s << " ";
// 		std::cout << "]\nline: " << d.line << "\n";
// 		if (!d.children.has_value())
// 			std::cout << "NO CHILDREN\n";
// 		else
// 		{
// 			std::cout << "------\nCHILDREN\n";
// 			debug_print_parser(d.children.value());
// 			std::cout << "------\n";
// 		}
// 	}
// }

Config parse_config(int argc, char **argv)
{
	if (argc > 2)
		throw std::invalid_argument("webserv: too many arguments\nusage: ./webserv [config file] (without a config file, the default one is used)");
	std::string filepath;
	if (argc == 1)
		filepath = DEFAULT_CONFIG;
	else
		filepath = argv[1];

	try
	{
		std::vector<Token> tokens;
		std::vector<Directive> directives;
		std::ifstream filestream = get_filestream(filepath);
		Tokenizer t(condense_to_string(filestream));
		tokens = t.tokenize();
		Parser parser(tokens);
		directives = parser.parse();
		// any non-ConfigError gets send to caller automatically

		if (tokens.back().tokenType() != TokenType::END)
			throw std::logic_error("DEBUG: lexer returned malformed Token vector (does not end with END type Token).");

		return (ConfigBuilder(directives).build());
	}
	catch (const ConfigError &c)
	{
		throw std::runtime_error(c.format(filepath));
	}
}
