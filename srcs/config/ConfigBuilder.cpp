#include "ConfigBuilder.hpp"
#include "ConfigError.hpp"

#include <algorithm> // for std::count()
#include <cctype> // for std::isdigit()

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

static void verify_address_str(const std::string &address, std::size_t line)
{
	if (std::count(address.begin(), address.end(), '.') != 3)
		throw ConfigError("faulty listen address", line);
	for (char c : address)
		if (c != '.' && !isdigit(static_cast<unsigned char>(c)))
			throw ConfigError("faulty listen address", line);
}

static void verify_address_nums(const std::vector<std::size_t> &address_nums, std::size_t line)
{
	for (std::size_t n : address_nums)
		if (n > 255)
			throw ConfigError("faulty listen address", line);
}

static int port_str_to_int(const std::string &port, std::size_t line)
{
	for (char c : port)
		if (!isdigit(static_cast<unsigned char>(c)))
			throw ConfigError("faulty listen address port", line);
	// 65535 is the max value of an unsigned 16bit integer
	if (port.length() > 5)
		throw ConfigError("faulty listen address port", line);
	int port_int = stoi(port);
	if (port_int > 65535 || port_int == 0)
		throw ConfigError("faulty listen address port", line);
	return (port_int);
}

// this helper turns strings into uint32_t and uint16_t, builds a ListenAddress struct and returns it
static ListenAddress build_listenaddress(const std::string &address, const std::string &port, size_t line)
{
	ListenAddress listen_address;

	if (address == "" || address == "*")
		listen_address.address = 0;
	else
	{
		verify_address_str(address, line);
		std::size_t substr_start = 0;
		std::size_t period_pos = address.find(".");
		std::vector<std::string> octets_str;
		while (period_pos != std::string::npos)
		{
			std::size_t substr_n = period_pos - substr_start;
			std::string substr = address.substr(substr_start, substr_n);
			if (substr == "" || substr.length() > 3)
				throw ConfigError("faulty listen address", line);
			octets_str.push_back(substr);
			substr_start = period_pos + 1;
			period_pos = address.find(".", substr_start);
		}
		std::string substr = address.substr(substr_start);
		if (substr == "" || substr.length() > 3)
			throw ConfigError("faulty listen address", line);
		octets_str.push_back(substr);
		std::vector<std::size_t> octets;
		for (const std::string &s : octets_str)
			octets.push_back(std::stoi(s));
		verify_address_nums(octets, line);
		listen_address.address = (octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3];
	}

	listen_address.port = port_str_to_int(port, line);

	return (listen_address);
}

static ListenAddress add_address(const Directive &d)
{
	if (d.args.size() == 0)
		// TODO: match nginx error here
		throw ConfigError("no arguments for \"listen\" directive", d.line);
	else if (d.args.size() > 1)
		// TODO: match nginx error here
		throw ConfigError("more than one argument for \"listen\" directive", d.line);
	else
	{
		ListenAddress listen_address;

		std::string address;
		std::string port;

		if (d.args[0].empty())
			// TODO: match nginx error here
			throw ConfigError("no host", d.line);

		std::size_t colon_pos = d.args[0].find(":");

		// no colon = no address specified = all addresses valid, as long as there is a port
		// the argument is read as the port
		if (colon_pos == std::string::npos)
			listen_address = build_listenaddress("", d.args[0], d.line);
		// if there is a colon ...
		else
		{
			// ... there has to be something in front of it
			address = d.args[0].substr(0, colon_pos);
			if (address == "")
				// TODO: match nginx error here
				throw ConfigError("no host", d.line);
			port = d.args[0].substr(colon_pos + 1);
			// port is still obligatory
			if (port == "")
				// TODO: match nginx error here
				throw ConfigError("listen address has no port", d.line);
			listen_address = build_listenaddress(address, port, d.line);
		}
		return (listen_address);
	}
}

ServerConfig ConfigBuilder::buildServer(const std::vector<Directive> &server_directives, std::size_t line)
{
	ServerConfig server_config;

	for (const Directive &d : server_directives)
		if (d.name == "listen")
			server_config._listenAddresses.push_back(add_address(d));

	if (server_config._listenAddresses.size() == 0)
		// TODO: match nginx error here
		throw ConfigError("no server address given", line);

	return (server_config);
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
	// TODO: remnant line; children should be checked by verify_top_level, making this redundant; verification needed	
	// if (d.children.has_value())
			config._servers.push_back(buildServer(d.children.value(), d.line));

	return (config);
}
