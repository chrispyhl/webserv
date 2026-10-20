#include "LocationConfig.hpp"

// default values for all members unique to LocationConfig
// the rest gets values from ConfigBuilder: either default values or inherited data from Server
LocationConfig::LocationConfig() :
	_root("html"),
	_inheritedRoot(true),
	_indexFiles({"index.html"}),
	_autoIndex(false),
	// = 1 MiB (mebibyte) = nginx default
	_maxBodySize(1024 * 1024),
	_allowedMethods({"GET"}),
	_uploadDirectory(std::nullopt),
	_cgiHandlers({})
{
}

const std::string &LocationConfig::path() const
{
	return (_path);
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

const std::optional<Redirect> &LocationConfig::redirect() const
{
	return (_redirect);
}

const std::set<std::string> &LocationConfig::allowedMethods() const
{
	return (_allowedMethods);
}

const std::optional<std::string> &LocationConfig::uploadDirectory() const
{
	return (_uploadDirectory);
}

const std::map<std::string, std::string> &LocationConfig::cgiHandlers() const
{
	return (_cgiHandlers);
}
