#include "ConfigBuilder.hpp"
#include "ConfigError.hpp"

#include <algorithm>

const std::vector<std::string> KNOWN_DIRECTIVES = {
	"server",
	"location",
	"listen",
	"server_name",
	"error_page",
	"client_max_body_size",
	"root",
	"index",
	"autoindex",
	"return",
	"allow_methods",
	"upload_dir",
	"cgi"
};

ConfigBuilder::ConfigBuilder(std::vector<Directive> directives) : _directives(directives)
{
}

static void verify_top_level(const std::vector<Directive> &directives)
{
	for (const Directive &d : directives)
	{
		if (d.name != "server")
		{
			if (std::find(KNOWN_DIRECTIVES.begin(), KNOWN_DIRECTIVES.end(), d.name) != KNOWN_DIRECTIVES.end())
				throw ConfigError("\"" + d.name + "\" directive is not allowed here", d.line);
			else
				throw ConfigError("unknown directive \"" + d.name + "\"", d.line);
		}
		if (d.args.size() != 0)
			throw ConfigError("invalid number of arguments in \"server\" directive", d.line);
		if (!d.children.has_value())
			throw ConfigError("directive \"server\" has no opening \"{\"", d.line);
	}
}

// this helper turns strings into uint32_t and uint16_t, builds a ListenAddress struct and returns it
static ListenAddress build_address(const std::string &address, const std::string &port, size_t line)
{
	ListenAddress listen_address;

	if (address == "")
		listen_address.address = 0;
	else
	{
		if (address.length() > 16)
			// TODO: match nginx error here
			throw ConfigError("faulty listen address", line);

		try
		{
			std::string str1 = address.substr(0, 4);
			

			// std::size_t octat1 = stoi;
			// std::size_t octat2;
			// std::size_t octat3;
			// std::size_t octat4;
		}
		catch (...)
		{
			throw ConfigError("faulty listen address", line);
		}
	}
	std::uint16_t port_bits;

	return (listen_address);
}

static std::vector<ListenAddress> addAddresses(const Directive &d)
{
	if (d.args.size() == 0)
	{
		// TODO: match nginx error here
		throw ConfigError("no arguments for \"listen\" directive", d.line);
	}
	else
	{
		std::vector<ListenAddress> addresses;

		for (const std::string &a : d.args)
		{
			std::string address;
			std::string port;

			auto colon = a.find(":");
			if (colon == std::string::npos)
				port = a;
			else
			{
				address = a.substr(0, colon);
				port = a.substr(colon + 1, a.length() - colon);
			}
			if (port == "")
				throw ConfigError("listen address has no port", d.line);
			addresses.push_back(build_address(address, port, d.line));
		}
	}
}

ServerConfig ConfigBuilder::buildServer(std::vector<Directive> server_directives)
{
	ServerConfig server_config;

	for (const Directive &d : server_directives)
	{
		if (d.name == "listen")
			server_config._listenAddresses = addAddresses(d);
	}
}

// INFO: nginx has a http {[...]} wrapper level around all server directives
// webserv does not handle this so far; it assumes the config file is equivalent to what is supposed to be inside the http {[...]} block of a standard nginx config file
// TODO: decide whether to add http wrapper handling
Config ConfigBuilder::build()
{
	if (_directives.size() == 0)
		throw ConfigError("no \"server\" directive found", std::nullopt);
	verify_top_level(_directives);

	Config config;

	for (const Directive &d : _directives)
		if (d.children.has_value())
			config._servers.push_back(buildServer(d.children.value()));

	return (config);
}
