#include "Config.hpp"
#include "utils.hpp"

#include <algorithm> // for std::find()
#include <cstddef>
#include <stdexcept>

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

const ServerConfig &Config::selectServer(const ListenAddress &listen_address, const std::string &host_header) const
{
	// HEADER PROCESSING
	std::string name;
	// colon exists = name is what comes before
	std::size_t colon_pos = host_header.find(':');
	if (colon_pos != std::string::npos)
		name = tolower_string(host_header.substr(0, colon_pos));
	//otherwise: while string is heaer
	else
		name = tolower_string(host_header);

	// server lookup
	// this function is returns a reference; a ref can'ts tart empty, so this is a pointer
	const ServerConfig *default_server = nullptr;
	for (const ServerConfig &s : _servers)
	{
		const std::vector<ListenAddress> &addresses = s.listenAddresses();
		// no address match? look at next server
		if (std::find(addresses.begin(), addresses.end(), listen_address) == addresses.end())
			continue;
		// address match? save this server as default fallback (virtual host rule) if not set yet
		if (!default_server)
			default_server = &s;
		const std::vector<std::string> &server_names = s.serverNames();
		// name match! return this ServerConfig right now (not the default one!)
		if (std::find(server_names.begin(), server_names.end(), name) != server_names.end())
			return (s);
	}
	if (!default_server)
		throw std::logic_error("Config::selectServer was passed an address no server listens on");
	// if function reaches this, no address AND name match was found --> return default server (first address match)
	return (*default_server);
}
