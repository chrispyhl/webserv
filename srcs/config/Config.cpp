#include "Config.hpp"

#include <algorithm> // for std::find()

Config::Config()
{
}

std::vector<ListenAddress> Config::listenAddresses() const
{
	std::vector<ListenAddress> listen_addresses;

	for (const ServerConfig &s : _servers)
	{
		for (const ListenAddress &a : s.listenAddresses())
		{
			// std::find() works with the == operator to compare container elements with the given argument (here: a)
			// the == operator is overloaded for ListenAddressess, comparing both address and port and only returning true if both match!
			if (std::find(listen_addresses.begin(), listen_addresses.end(), a) == listen_addresses.end())
				listen_addresses.push_back(a);
		}
	}
	return (listen_addresses);
}
