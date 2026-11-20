#include "Config.hpp"
#include "utils.hpp"

#include <cstddef>
#include <stdexcept>

Config::Config()
{
}

const std::vector<ListenAddress> &Config::listenAddresses() const
{
	return (_listenAddresses);
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
		if (!vector_contains(addresses, listen_address))
			continue;
		// address match? save this server as default fallback (virtual host rule) if not set yet
		if (!default_server)
			default_server = &s;
		const std::vector<std::string> &server_names = s.serverNames();
		// name match! return this ServerConfig right now (not the default one!)
		if (vector_contains(server_names, name))
			return (s);
	}
	if (!default_server)
		throw std::logic_error("Config::selectServer was passed an address no server listens on");
	// if function reaches this, no address AND name match was found --> return default server (first address match)
	return (*default_server);
}
