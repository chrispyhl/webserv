#include "ServerConfig.hpp"

// no list initialization: class has no members that don't initialize by themselves
ServerConfig::ServerConfig()
{
}

const std::vector<ListenAddress> &ServerConfig::listenAddresses() const
{
	return (_listenAddresses);
}

const std::vector<std::string> &ServerConfig::serverNames() const
{
	return (_serverNames);
}

const std::vector<LocationConfig> &ServerConfig::locations() const
{
	return (_locations);
}

const std::optional<Redirect> &ServerConfig::redirect() const
{
	return (_redirect);
}

const std::map<std::size_t, std::string> &ServerConfig::errorPages() const
{
	return (_errorPages);
}
