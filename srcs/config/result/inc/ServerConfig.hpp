#pragma once

#include "ListenAddress.hpp"
#include "LocationConfig.hpp"
#include "Return.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <map>
#include <vector>

class ServerConfig
{
		friend class ConfigBuilder;

	public:
		const std::vector<ListenAddress> &listenAddresses() const;
		const std::vector<std::string> &serverNames() const;
		const std::vector<LocationConfig> &locations() const;
		const std::optional<Return> &returnDirective() const;
		const std::map<std::size_t, std::string> &errorPages() const;

	private:
		// only friend class ConfigBuilder creates ServerConfigs, hence private
		ServerConfig();

		std::vector<ListenAddress> _listenAddresses;
		std::vector<std::string> _serverNames;
		std::vector<LocationConfig> _locations;
		std::optional<Return> _returnDirective;
		std::map<std::size_t, std::string> _errorPages;
};
