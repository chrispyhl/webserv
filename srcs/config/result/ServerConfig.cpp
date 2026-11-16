#include "ServerConfig.hpp"
#include "utils.hpp"

#include <algorithm> // for std::count()
#include <stdexcept>

// no list initialization: class has no members that don't initialize by themselves
ServerConfig::ServerConfig()
{
}

const std::vector<ListenAddress> &ServerConfig::listenAddresses() const
{
	return (_listenAddresses);
}

// find the server's location that is responsible for the path of the request
// path "/files/a.txt" wants "location "/files"
const LocationConfig &ServerConfig::findLocation(const std::string &path) const
{
	// the absolute best match is a full match of the path argument and exactMatch == true
	// if this isn't found, the ranking is as follows:
	// 3) path() == "/" (matches everything, due to implicit location "/" always the bottomline)
	// 2) path() == path
	// 1) path() == path + "/" + [...] (begins with path and continues)
	// basically, longest path wins
	const LocationConfig *best_match = nullptr;

	std::string clean_path = trim_trailing_forward_slashes(path);

	for (const LocationConfig &lc : _locations)
	{
		if (lc.exactMatch())
		{
			if (lc.path() == clean_path)
				return (lc);
			// all other exact locations can't be exactly == clean_path
			continue;
		}
		bool matches = (lc.path() == "/" // 3)
			|| lc.path() == clean_path // 2)
			|| clean_path.substr(0, lc.path().size() + 1) == lc.path() + "/"); // 1)
		// if there is a match AND EITHER no match was saved so far OR this match is better (= longer) than the saved one => save this one
		if (matches && (!best_match || lc.path().size() > (*best_match).path().size()))
			best_match = &lc;
	}
	if (!best_match)
		throw std::logic_error("LOGIC ERROR: ServerConfig::findLocation() didn't even find a implicit location with path \"/\"!");
	return (*best_match);
}

const std::vector<std::string> &ServerConfig::serverNames() const
{
	return (_serverNames);
}

const std::vector<LocationConfig> &ServerConfig::locations() const
{
	return (_locations);
}

const std::optional<Return> &ServerConfig::returnDirective() const
{
	return (_returnDirective);
}

const std::map<std::size_t, std::string> &ServerConfig::errorPages() const
{
	return (_errorPages);
}
