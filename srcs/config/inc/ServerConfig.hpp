#pragma once

#include <optional> // allowed under cpp17
#include <string>
#include <utility> // for std::map
#include <vector>

class ServerConfig
{
	public:
	private:
		// SERVER ONLY DIRECTIVES
		// contains pairs of host address and ports
		std::vector<std::pair<size_t, size_t>> _listen;
		std::optional<std::vector<std::string>> _serverName;
		std::optional<std::string> _location;

		// DIRECTIVES THAT MIGHT BE OVERRIDDEN BY LOCATION
		// can be not-set as long as every location under it sets its own root
		std::optional<std::string> _root;
		std::optional<std::vector<std::string>> _index;

		size_t _clientMaxBodySize;
};
