#include "LocationConfig.hpp"

// initialization list: security fallback for members of types without automatic initialization (bool, size_t); ConfigBuilder overwrites them with own or inherited values
LocationConfig::LocationConfig() :	_exactMatch(false),
									_inheritedRoot(true),
									_autoIndex(false),
									// = 1 MiB (mebibyte) = nginx default
									_maxBodySize(1024 * 1024)
{
}

const std::string &LocationConfig::path() const
{
	return (_path);
}

bool LocationConfig::exactMatch() const
{
	return (_exactMatch);
}

const std::string &LocationConfig::root() const
{
	return (_root);
}

bool LocationConfig::inheritedRoot() const
{
	return (_inheritedRoot);
}

const std::vector<std::string> &LocationConfig::indexFiles() const
{
	return (_indexFiles);
}

bool LocationConfig::autoIndex() const
{
	return (_autoIndex);
}

std::size_t LocationConfig::maxBodySize() const
{
	return (_maxBodySize);
}

const std::map<std::size_t, std::string> &LocationConfig::errorPages() const
{
	return (_errorPages);
}

const std::optional<Return> &LocationConfig::returnDirective() const
{
	return (_returnDirective);
}

const std::set<std::string> &LocationConfig::allowedMethods() const
{
	return (_allowedMethods);
}

const std::string &LocationConfig::uploadDirectory() const
{
	return (_uploadDirectory);
}

const std::map<std::string, std::string> &LocationConfig::cgiHandlers() const
{
	return (_cgiHandlers);
}
