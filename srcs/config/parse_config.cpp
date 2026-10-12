#include "parse_config.hpp"
#include <fstream> // for std::fstream
#include <iostream>
#include <stdexcept>
#include <string>

static std::fstream get_filestream(const std::string &filepath)
{
	// :std::ios::in restricts the file access to reading-mode
	// avoids potentially declining a valid config file (only readability needed, not writability)
	std::fstream filestream(filepath, std::ios::in);
	// std::fstreams destructor closes the FD automatically; no filestream.close() needed!
	if (!filestream.is_open())
		throw std::invalid_argument("ERROR: unable to open file.");
	// catch paths to folders and empty files
	std::string line;
	getline(filestream, line);
	if (filestream.fail())
		throw std::invalid_argument("ERROR: unable to read file.");
	// reset filestream (otherwise, first line would be consumed) before returning
	filestream.seekg(0);
	return (filestream);
}

Config parseConfig(int argc, char **argv)
{
	if (argc > 2)
		throw std::invalid_argument("ERROR: invalid argument provided.\nCall via:\n\t\"./webserv\" (uses default path)\nor\n\t \"./webserv [PATH TO CONFIG FILE]\"");
	std::string filepath;
	if (argc == 1)
		filepath = "./configs/webserv.conf";
	else
		filepath = argv[1];
	std::fstream filestream = get_filestream(filepath);
	
	// DEBUG
	for (std::string line; std::getline(filestream, line);)
		std::cout << line << std::endl;
	// DEBUG OVER

	Config config;
	return (config);
}
