#include "webserv.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char **argv)
{
	try
	{
		Config config = parse_config(argc, argv);
		// DEBUG
		const std::vector<std::string> hostnames = { "localhost", "api.localhost:8080", "API.LOCALHOST", "unknown", "" };
		for (const ListenAddress &la : config.listenAddresses())
		{
			std::cout << "LOOKING FOR SERVERS FOR: " << ((la.address >> 24) & 255) << "." << ((la.address >> 16) & 255) << "."
				<< ((la.address >> 8) & 255) << "." << (la.address & 255) << ":" << la.port << "\n=============\n";
			for (const std::string &hn : hostnames)
			{
				const ServerConfig &sc = config.selectServer(la, hn);
				if (sc.serverNames().size() != 0)

					std::cout << "selected server for hostname \"" << hn << "\": " << sc.serverNames()[0] << "\n";
				else
					std::cout << "server without name found for \"" << hn << "\"\n";
			}
			std::cout << "\n";
		}
		std::cout << "DELIBERATE FAIL CASE\n=============\n";
		config.selectServer({123,123}, "abc abc");
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
