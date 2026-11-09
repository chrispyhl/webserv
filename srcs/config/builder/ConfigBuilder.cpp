#include "ConfigBuilder.hpp"
#include "ConfigError.hpp"
#include "directive_checks.hpp"
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

ServerConfig ConfigBuilder::buildServer(const Directive &server_block)
{
	ServerConfig server_config;

	// locations need to be built last since directives that might function as default server values for locations also apply retroactively
	std::vector<config::LocationDirective> location_directives;
	ServerDefaults defaults;

	for (const Directive &d : server_block.children.value())
	{
		// LOCATION HANDLING
		if (d.name == "location")
		{
			// locations may be constructed later, but if there is a faulty config in there, it has to throw immediately
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
			if (defaults.root.has_value())
				config::duplicate_throw(d);
			defaults.root = root;
		}
		else if (d.name == "index")
		{
			std::vector<std::string> index_files = config::get_index_files(d);
			// multiple occurrences of "index" in a server block are allowed
			// can't .insert() on an optional without value
			if (!defaults.index_files.has_value())
				defaults.index_files = index_files;
			else
				defaults.index_files->insert(defaults.index_files->end(), index_files.begin(), index_files.end());
		}
		else if (d.name == "autoindex")
		{
			bool auto_index = config::get_auto_index(d);
			if (defaults.auto_index.has_value())
				config::duplicate_throw(d);
			defaults.auto_index = auto_index;
		}
		else if (d.name == "client_max_body_size")
		{
			std::size_t max_body_size = config::get_max_body_size(d);
			if (defaults.max_body_size.has_value())
				config::duplicate_throw(d);
			defaults.max_body_size = max_body_size;
		}
		else if (d.name == "error_page")
		{
			std::map<std::size_t, std::string> error_pages = config::get_error_pages(d);
			for (const auto &p : error_pages)
			{
				// nginx only uses the first entry in case of duplicate error codes
				// .insert() is a no-op if the key is already in the container
				server_config._errorPages.insert(p);
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

	// DEBUG
	for (const config::LocationDirective &ld : location_directives)
	{
		std::cout << "=== LOCATIONDIRECTIVE DEBUG ===\nlocation path: \"" << ld.path << "\"\nexact match: " << (ld.exact_match ? "yes" : "no") << "\n";
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
