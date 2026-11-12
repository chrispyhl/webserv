#include "ConfigBuilder.hpp"
#include "ConfigError.hpp"
#include "directive_checks.hpp"
#include "location_directives.hpp"
#include "server_directives.hpp"
#include "shared_directives.hpp"
#include "utils.hpp"

#include <iostream>

ConfigBuilder::ConfigBuilder(std::vector<Directive> directives) : _directives(directives)
{
}

static void verify_top_level(const std::vector<Directive> &directives)
{
	for (const Directive &d : directives)
	{
		if (d.name != "server")
			config::bad_directive_throw(d);
		if (d.args.size() != 0)
			config::invalid_args_size_throw(d);
		if (!d.children.has_value())
			throw ConfigError("directive \"server\" has no opening \"{\"", d.line);
	}
}

void ConfigBuilder::warn_about_duplicates(const Config &config) const
{
	// looking through servers
	for (auto server1_it = config._servers.begin(); server1_it != config._servers.end(); server1_it++)
	{
		for (auto server2_it = server1_it + 1; server2_it != config._servers.end(); server2_it++)
		{
			// looking through addresses
			for (const ListenAddress &la : server2_it->_listenAddresses)
			{
				// server1 doesn't listen on this address = no competition here
				if (!vector_contains(server1_it->_listenAddresses, la))
					continue;
				// looking through server names
				for (const std::string &sn : server2_it->_serverNames)
				{
					// server1 and server 2 listen to the same address, but have completely different names = no competition here
					if (!vector_contains(server1_it->_serverNames, sn))
						continue;
					std::cerr << "warn: conflicting server name \"" << sn << "\" on " << listen_address_to_string(la) << ", ignored\n";
				}
			}
		}
	}
}

LocationConfig ConfigBuilder::buildLocation(const config::LocationDirective &location_directive, const BlockValues &inherited_values) const
{
	// final product
	LocationConfig location_config;

	location_config._path = location_directive.path;
	location_config._exactMatch = location_directive.exact_match;

	// temp storage struct for block values for duplicate checking
	BlockValues block_values;

	// .has_value() will always be true since a location without a block will throw before
	// even an empty block is valid, according to nginx
	const std::vector<Directive> &location_block = location_directive.directive.children.value();
	// if location_block is empty, this is a harmless no-op
	for (const Directive &d : location_block)
	{
		// ===SHARED DIRECTIVES===
		if (d.name == "root")
		{
			std::string root = config::get_root(d);
			if (block_values.root.has_value())
				config::duplicate_throw(d);
			block_values.root = root;
		}
		else if (d.name == "index")
		{
			std::vector<std::string> index_files = config::get_index_files(d);
			// multiple occurrences of "index" in a location block are allowed
			// can't .insert() on an optional without value
			if (!block_values.index_files.has_value())
				block_values.index_files = index_files;
			else
				block_values.index_files->insert(block_values.index_files->end(), index_files.begin(), index_files.end());
		}
		else if (d.name == "autoindex")
		{
			bool auto_index = config::get_auto_index(d);
			if (block_values.auto_index.has_value())
				config::duplicate_throw(d);
			block_values.auto_index = auto_index;
		}
		else if (d.name == "client_max_body_size")
		{
			std::size_t max_body_size = config::get_max_body_size(d);
			if (block_values.max_body_size.has_value())
				config::duplicate_throw(d);
			block_values.max_body_size = max_body_size;
		}
		// "error_page" and "return" don't need duplicate protection, so they go directly into the result object location_config
		else if (d.name == "error_page")
		{
			std::map<std::size_t, std::string> error_pages = config::get_error_pages(d);
			for (const auto &ep : error_pages)
			{
				// nginx only uses the first entry in case of duplicate error codes
				// .insert() is a no-op if the key is already in the container
				location_config._errorPages.insert(ep);
			}
		}
		else if (d.name == "return")
		{
			Return return_directive = config::get_return(d);
			// nginx only uses the first mention of "return", ignores all others
			if (!location_config._returnDirective.has_value())
				location_config._returnDirective = return_directive;
		}
		// ===LOCATION-ONLY DIRECTIVES===
		else if (d.name == "allow_methods")
		{
			if (!location_config._allowedMethods.empty())
				config::duplicate_throw(d);
			location_config._allowedMethods = config::get_allowed_methods(d);
		}
		else if (d.name == "upload_dir")
		{
			std::cerr << "DEBUG WARNING: \"upload_dir\" directive ignored; not supported yet\n";
		}
		else if (d.name == "cgi")
		{
			std::cerr << "DEBUG WARNING: \"cgi\" directive ignored; not supported yet\n";
		}
		// ===NESTED LOCATION DIRECTIVE===
		else if (d.name == "location")
		{
			std::cerr << "DEBUG WARNING: nested location directive ignored; not supported yet\n";
		}
		else
			config::bad_directive_throw(d);
	}

	// TODO: da big MERGE: fill blanks with server defaults from inherited_values
	// if inherited_values does not contain values for every empty member, set them to default values: _root = "html"; _indexFiles = "index.html"
	// if not set after loop here: _allowedMethods = {"GET"}
	(void)inherited_values;

	return (location_config);
}

std::vector<LocationConfig> ConfigBuilder::buildLocations(const std::vector<config::LocationDirective> &location_directives, const BlockValues &inherited_values) const
{
	std::vector<LocationConfig> locations;

	for (const config::LocationDirective &ld : location_directives)
		locations.push_back(buildLocation(ld, inherited_values));

	// duplicate check: path and exact_match flag identical --> throw!
	// NOTE: needs to be below the location building section as a final check to emulate nginx
	for (auto it1 = location_directives.begin(); it1 != location_directives.end(); it1++)
	{
		for (auto it2 = it1 + 1; it2 != location_directives.end(); it2++)
		{
			if (it1->path == it2->path && it1->exact_match == it2->exact_match)
				throw ConfigError("duplicate location \"" + it2->path + "\"", it2->directive.line);
		}
	}

	return (locations);
}

ServerConfig ConfigBuilder::buildServer(const Directive &server_block)
{
	ServerConfig server_config;

	// locations need to be built last since server directives that locations inherit also apply retroactively
	std::vector<config::LocationDirective> location_directives;
	// values of the server's shared directives; the locations inherit them
	BlockValues inherited_values;

	for (const Directive &d : server_block.children.value())
	{
		// LOCATION HANDLING
		if (d.name == "location")
		{
			// locations are built after the loop, but a faulty location line has to throw immediately
			location_directives.push_back(config::get_location_directive(d));
		}
		// LISTEN ADDRESS HANDLING
		else if (d.name == "listen")
			server_config._listenAddresses.push_back(config::get_listen_address(d));
		// SERVER NAME HANDLING
		else if (d.name == "server_name")
		{
			std::vector<std::string> server_names = config::get_server_names(d);
			server_config._serverNames.insert(server_config._serverNames.end(), server_names.begin(), server_names.end());
		}
		else if (d.name == "root")
		{
			std::string root = config::get_root(d);
			if (inherited_values.root.has_value())
				config::duplicate_throw(d);
			inherited_values.root = root;
		}
		else if (d.name == "index")
		{
			std::vector<std::string> index_files = config::get_index_files(d);
			// multiple occurrences of "index" in a server block are allowed
			// can't .insert() on an optional without value
			if (!inherited_values.index_files.has_value())
				inherited_values.index_files = index_files;
			else
				inherited_values.index_files->insert(inherited_values.index_files->end(), index_files.begin(), index_files.end());
		}
		else if (d.name == "autoindex")
		{
			bool auto_index = config::get_auto_index(d);
			if (inherited_values.auto_index.has_value())
				config::duplicate_throw(d);
			inherited_values.auto_index = auto_index;
		}
		else if (d.name == "client_max_body_size")
		{
			std::size_t max_body_size = config::get_max_body_size(d);
			if (inherited_values.max_body_size.has_value())
				config::duplicate_throw(d);
			inherited_values.max_body_size = max_body_size;
		}
		else if (d.name == "error_page")
		{
			std::map<std::size_t, std::string> error_pages = config::get_error_pages(d);
			for (const auto &ep : error_pages)
			{
				// nginx only uses the first entry in case of duplicate error codes
				// .insert() is a no-op if the key is already in the container
				server_config._errorPages.insert(ep);
			}
		}
		else if (d.name == "return")
		{
			Return return_directive = config::get_return(d);
			// nginx only uses the first mention of "return", ignores all others
			if (!server_config._returnDirective.has_value())
				server_config._returnDirective = return_directive;
		}
		// FALLTHROUGH HANDLING (no valid directive for this scope!)
		else
			config::bad_directive_throw(d);
	}

	// own message since nginx would not complain about missing listen directives (defaults to 80/8000, which webserv doesn't emulate)
	if (server_config._listenAddresses.size() == 0)
		throw ConfigError("no \"listen\" directive in \"server\" block", server_block.line);

	if (!server_config._errorPages.empty())
		inherited_values.error_pages = server_config._errorPages;
	server_config._locations = buildLocations(location_directives, inherited_values);

	// DEBUG
	for (const config::LocationDirective &ld : location_directives)
	{
		std::cout << "=== LOCATION DIRECTIVE DEBUG ===\nlocation path: \"" << ld.path << "\"\nexact match: " << (ld.exact_match ? "yes" : "no") << "\n";
	}
	// DEBUG END

	return (server_config);
}

// INFO: nginx has a http {[...]} wrapper level around all server directives
// webserv does not handle this so far; it assumes the config file is equivalent to what is supposed to be inside the http {[...]} block of a standard nginx config file
// NOTE: decide whether to add http wrapper handling
Config ConfigBuilder::build()
{
	if (_directives.size() == 0)
		throw ConfigError("no \"server\" directive found", std::nullopt);
	verify_top_level(_directives);

	Config config;

	for (const Directive &d : _directives)
		config._servers.push_back(buildServer(d));

	warn_about_duplicates(config);

	return (config);
}
