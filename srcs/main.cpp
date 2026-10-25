#include "webserv.hpp"

#include <stdexcept>
#include <iostream>

int main(int argc, char **argv)
{
	try
	{
		Config config = parse_config(argc, argv);
		// DEBUG
		std::vector<ListenAddress> testerino = config.listenAddresses();
		// these bitshifts reverse operation from ConfigBuilder, so that 32bit value is split into 4 bytes (8 bits) for print in dotted notation
		for (const ListenAddress &a : testerino)
			std::cout 
			<< ((a.address >> 24) & 255) << "." 
			<< ((a.address >> 16) & 255) << "." 
			<< ((a.address >> 8) & 255) << "." 
			<< (a.address & 255) << ":" 
			<< a.port << "\n";
		// DEBUG END
	}
	// catches all used exceptions, including ConfigError (inherits from std::exception via std::runtime_error)
	catch (const std::exception &e)
	{
		std::cerr << e.what() << "\n";
		return (1);
	}
	return (0);
}
