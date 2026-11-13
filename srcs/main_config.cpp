#include "webserv.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char **argv)
{
	try
	{
		Config config = parse_config(argc, argv);
	}
	// catches all used exceptions, including ConfigError (inherits from std::exception via std::runtime_error)
	catch (const std::exception &e)
	{
		std::cerr << e.what() << "\n";
		return (1);
	}
	return (0);
}
