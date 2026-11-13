#pragma once

#include "ServerConfig.hpp"

#include <vector>



class Config
{
		friend class ConfigBuilder;

	public:
		std::vector<ListenAddress> listenAddresses() const;
		// empty host_header string ("") == no header (HTTP/1.0) --> default server answers
		const ServerConfig &selectServer(
				const ListenAddress &listen_address,
				const std::string &host_header) const;

	private:
		Config();

		std::vector<ServerConfig> _servers;
};
