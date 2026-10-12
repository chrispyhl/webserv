#include <stdexcept>

#include "parse_config.hpp"
#include <iostream>

int main(int argc, char **argv)
{
	try
	{
		Config config = parseConfig(argc, argv);
		(void)config;
		std::cout << "DEBUG: file accessed successfully!" << "\n";
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << "\n";
		return (1);
	}
	return (0);
}
