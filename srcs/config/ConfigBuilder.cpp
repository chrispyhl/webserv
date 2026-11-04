#include "ConfigBuilder.hpp"
#include "ConfigError.hpp"
#include "utils.hpp"

#include <algorithm> // for std::count()
#include <cctype> // for std::isdigit()
#include <iostream>
#include <limits>

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

static void bad_directive_throw(const Directive &d)
{
	if (vector_contains(KNOWN_DIRECTIVES, d.name))
		throw ConfigError("\"" + d.name + "\" directive is not allowed here", d.line);
	else
		throw ConfigError("unknown directive \"" + d.name + "\"", d.line);
}

static void duplicate_throw(const Directive &d)
{
	throw ConfigError("\"" + d.name + "\" directive is duplicate", d.line);
}

static void invalid_args_size_throw(const Directive &d)
{
	throw ConfigError("invalid number of arguments in \"" + d.name + "\" directive", d.line);
}

static void host_not_found_throw(const Directive &d)
{
	throw ConfigError("host not found in \"" + d.args[0] + "\" of the \"listen\" directive", d.line);
}

static void invalid_port_throw(const Directive &d)
{
	throw ConfigError("invalid port in \"" + d.args[0] + "\" of the \"listen\" directive", d.line);
}

static void check_for_illegal_block(const Directive &d)
{
	if (d.children.has_value())
		throw ConfigError("directive \"" + d.name + "\" is not terminated by \";\"", d.line);
}

static void verify_top_level(const std::vector<Directive> &directives)
{
	for (const Directive &d : directives)
	{
		if (d.name != "server")
			bad_directive_throw(d);
		if (d.args.size() != 0)
			invalid_args_size_throw(d);
		if (!d.children.has_value())
			throw ConfigError("directive \"server\" has no opening \"{\"", d.line);
	}
}

static void verify_address_str(const std::string &address, const Directive &d)
{
	if (std::count(address.begin(), address.end(), '.') != 3)
		host_not_found_throw(d);
	for (char c : address)
		if (c != '.' && !std::isdigit(static_cast<unsigned char>(c)))
			host_not_found_throw(d);
}

static void verify_address_nums(const std::vector<std::size_t> &address_nums, const Directive &d)
{
	for (std::size_t n : address_nums)
		if (n > 255)
			host_not_found_throw(d);
}

static std::uint16_t port_str_to_uint16(const std::string &port, const Directive &d)
{
	// 65535 is the max value of an unsigned 16bit integer, so we can check if its <= 5 to avoid overflow for std::stoi()
	if (!isdigit_string(port) || port.length() > 5)
		invalid_port_throw(d);
	int port_int = std::stoi(port);
	if (port_int > 65535 || port_int == 0)
		invalid_port_throw(d);
	return (port_int);
}

// this helper turns strings into uint32_t and uint16_t, builds a ListenAddress struct and returns it
static ListenAddress build_listenaddress(const std::string &address, const std::string &port, const Directive &d)
{
	ListenAddress listen_address;

	if (address.empty())
		listen_address.address = 0;
	else
	{
		verify_address_str(address, d);
		std::size_t substr_start = 0;
		std::size_t period_pos = address.find(".");
		std::vector<std::string> octets_str;
		while (period_pos != std::string::npos)
		{
			std::size_t substr_n = period_pos - substr_start;
			std::string substr = address.substr(substr_start, substr_n);
			if (substr.empty() || substr.length() > 3)
				host_not_found_throw(d);
			octets_str.push_back(substr);
			substr_start = period_pos + 1;
			period_pos = address.find(".", substr_start);
		}
		std::string substr = address.substr(substr_start);
		if (substr.empty() || substr.length() > 3)
			host_not_found_throw(d);
		octets_str.push_back(substr);
		std::vector<std::size_t> octets;
		for (const std::string &s : octets_str)
			octets.push_back(std::stoi(s));
		verify_address_nums(octets, d);
		listen_address.address = (octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3];
	}

	listen_address.port = port_str_to_uint16(port, d);

	return (listen_address);
}

static ListenAddress get_listen_address(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() == 0 || d.args.size() > 1)
		invalid_args_size_throw(d);

	ListenAddress listen_address;

	std::string address;
	std::string port;

	if (d.args[0].empty())
		throw ConfigError("no host in \"\" of the \"listen\" directive", d.line);
	std::size_t colon_pos = d.args[0].find(":");

	// no colon = no address specified = all addresses valid, as long as there is a port
	// the argument is read as the port
	if (colon_pos == std::string::npos)
	{
		// all digits: read as port
		if (isdigit_string(d.args[0]))
			listen_address = build_listenaddress("", d.args[0], d);
		// otherwise: no port = not a valid address as per webserv's own definition (custom message)
		else
			throw ConfigError("no port in \"" + d.args[0] + "\" of the \"listen\" directive", d.line);
	}
	// if there is a colon ...
	else
	{
		// ... there has to be something in front of it
		address = d.args[0].substr(0, colon_pos);
		if (address.empty())
			throw ConfigError("no host in \"" + d.args[0] + "\" of the \"listen\" directive", d.line);
		if (tolower_string(address) == "localhost")
			address = "127.0.0.1";
		port = d.args[0].substr(colon_pos + 1);
		// port is still obligatory
		if (port.empty())
			invalid_port_throw(d);
		listen_address = build_listenaddress(address, port, d);
	}
	return (listen_address);
}

static std::vector<std::string> get_server_names(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() == 0)
		invalid_args_size_throw(d);
	std::vector<std::string> server_names;
	for (const std::string &arg : d.args)
	{
		std::string lower_server_name = tolower_string(arg);
		if (!vector_contains(server_names, lower_server_name))
			server_names.push_back(lower_server_name);
	}
	return (server_names);
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

static std::string get_root(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() != 1)
		invalid_args_size_throw(d);
	return (d.args[0]);
}

static std::vector<std::string> get_index_files(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() == 0)
		invalid_args_size_throw(d);
	for (const std::string &s : d.args)
		if (s.empty())
			throw ConfigError("index \"\" in \"index\" directive is invalid", d.line);
	return (d.args);
}

static bool get_auto_index(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() != 1)
		invalid_args_size_throw(d);
	std::string arg = tolower_string(d.args[0]);
	if (arg == "on")
		return (true);
	else if (arg == "off")
		return (false);
	else
		throw ConfigError("invalid value \"" + d.args[0] + "\" in \"autoindex\" directive, it must be \"on\" or \"off\"", d.line);
}

static std::size_t get_max_body_size(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() != 1)
		invalid_args_size_throw(d);

	const std::string &str = d.args[0];
	if (str.empty())
		throw ConfigError("\"client_max_body_size\" directive invalid value", d.line);

	const std::string allowed_chars = "kKmMgG";
	std::optional<char> suffix;

	for (auto it = str.begin(); it != str.end(); it++)
	{
		if (!std::isdigit(static_cast<unsigned char>(*it)))
		{
			if ((it + 1) == str.end() && allowed_chars.find(*it) != std::string::npos)
				suffix = *it;
			else
				throw ConfigError("\"client_max_body_size\" directive invalid value", d.line); // unallowed char was found
		}
	}

	std::size_t digits_length;
	if (!suffix.has_value())
		digits_length = str.size();
	else
		digits_length = str.size() - 1;

	std::size_t max_body_size;

	try
	{
		max_body_size = str_to_size_t(str.substr(0, digits_length));
	}
	catch (const std::invalid_argument &)
	{
		throw ConfigError("\"client_max_body_size\" directive invalid value", d.line);
	}

	if (!suffix.has_value())
		return (max_body_size); // no suffix = just return base value

	std::size_t limit = std::numeric_limits<size_t>::max();
	switch (suffix.value())
	{
		case ('k'):
		case ('K'):
		{
			if (max_body_size > (limit / 1024))
				break;
			return (max_body_size * 1024);
		}
		case ('m'):
		case ('M'):
		{
			if (max_body_size > (limit / (1024 * 1024)))
				break;
			return (max_body_size * 1024 * 1024);
		}
		case ('g'):
		case ('G'):
		{
			if (max_body_size > (limit / (1024 * 1024 * 1024)))
				break;
			return (max_body_size * 1024 * 1024 * 1024);
		}
		default:
			break;
	}
	throw ConfigError("\"client_max_body_size\" directive invalid value", d.line);
}

static std::map<std::size_t, std::string> get_error_pages(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() < 2)
		invalid_args_size_throw(d);
	std::vector<std::size_t> codes;
	auto it = d.args.begin();
	for (; it != d.args.end(); it++)
	{
		if (isdigit_string(*it))
		{
			try
			{
				codes.push_back(str_to_size_t(*it));
			}
			catch (const std::invalid_argument &)
			{
				throw ConfigError("invalid value \"" + *it + "\"", d.line);
			}
		}
		else
			break;
	}
	if (codes.empty())
		throw ConfigError("invalid value \"" + d.args[0] + "\"", d.line);
	if (it + 1 != d.args.end())
		throw ConfigError("invalid value \"" + *it + "\"", d.line);
	std::string uri = *it;
	std::map<std::size_t, std::string> error_pages;
	for (size_t c : codes)
	{
		if (c < 300 || c > 599)
			throw ConfigError("value \"" + std::to_string(c) + "\" must be between 300 and 599", d.line);
		if (c == 499)
			throw ConfigError("invalid value \"499\"", d.line);
		error_pages.insert({ c, uri });
	}
	// TODO: implement nginx's URI guidelines
	return (error_pages);
}

static std::size_t get_return_code(const Directive &d)
{
	std::string str = d.args[0];
	std::size_t code;
	try
	{
		code = str_to_size_t(str);
		// code needs to be between (and including) 0 and 999
		// can't be negative due to isdigit_string() check before, so only 999 needs guarding
		if (code > 999)
			throw ConfigError("invalid return code \"" + str + "\"", d.line);
	}
	catch (const std::invalid_argument &)
	{
		throw ConfigError("invalid return code \"" + str + "\"", d.line);
	}
	return (code);
}

static Return get_return(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() < 1 || d.args.size() > 2)
		invalid_args_size_throw(d);

	Return result;

	// one argument is only legal if its a code OR a target that starts with "http://" or "https://" (has default code 302)
	if (d.args.size() == 1)
	{
		if (isdigit_string(d.args[0]))
		{
			std::size_t code = get_return_code(d);
			result.code = code;
			result.target = "";
		}
		else
		{
			std::string target = d.args[0];
			if (target.substr(0, 7) == "http://" || target.substr(0, 8) == "https://")
			{
				result.code = 302;
				result.target = target;
			}
			else
				throw ConfigError("invalid return code \"" + target + "\"", d.line);
		}
	}
	// 2 arg scenario: first must be code, second is target
	else if (isdigit_string(d.args[0]))
	{
		std::size_t code = get_return_code(d);
		std::string target = d.args[1];
		result.code = code;
		result.target = target;
	}
	else
		throw ConfigError("invalid return code \"" + d.args[0] + "\"", d.line);

	return (result);
}

ServerConfig ConfigBuilder::buildServer(const Directive &server_block)
{
	ServerConfig server_config;

	// locations need to be built last since directives that might function as default server values for locations also apply retroactively
	std::vector<Directive> location_directives;

	ServerDefaults defaults;

	for (const Directive &d : server_block.children.value())
	{
		// LOCATION HANDLING
		if (d.name == "location")
		{
			if (!d.children.has_value())
				throw ConfigError("directive \"location\" has no opening \"{\"", d.line);
			location_directives.push_back(d);
		}
		// LISTEN ADDRESS HANDLING
		else if (d.name == "listen")
			server_config._listenAddresses.push_back(get_listen_address(d));
		// SERVER NAME HANDLING
		else if (d.name == "server_name")
		{
			std::vector<std::string> server_names = get_server_names(d);
			server_config._serverNames.insert(server_config._serverNames.end(), server_names.begin(), server_names.end());
		}
		else if (d.name == "root")
		{
			std::string root = get_root(d);
			if (defaults.root.has_value())
				duplicate_throw(d);
			defaults.root = root;
		}
		else if (d.name == "index")
		{
			std::vector<std::string> index_files = get_index_files(d);
			// multiple occurrences of "index" in a server block are allowed
			// can't .insert() on an optional without value
			if (!defaults.index_files.has_value())
				defaults.index_files = index_files;
			else
				defaults.index_files->insert(defaults.index_files->end(), index_files.begin(), index_files.end());
		}
		else if (d.name == "autoindex")
		{
			bool auto_index = get_auto_index(d);
			if (defaults.auto_index.has_value())
				duplicate_throw(d);
			defaults.auto_index = auto_index;
		}
		else if (d.name == "client_max_body_size")
		{
			std::size_t max_body_size = get_max_body_size(d);
			if (defaults.max_body_size.has_value())
				duplicate_throw(d);
			defaults.max_body_size = max_body_size;
		}
		else if (d.name == "error_page")
		{
			std::map<std::size_t, std::string> error_pages = get_error_pages(d);
			for (const auto &p : error_pages)
			{
				// nginx only uses the first entry in case of duplicate error codes
				if (server_config._errorPages.find(p.first) == server_config._errorPages.end())
					server_config._errorPages.insert(p);
			}
		}
		else if (d.name == "return")
		{
			Return return_directive = get_return(d);
			// nginx only uses the first mention of "return", ignores all others
			if (!server_config._returnDirective.has_value())
				server_config._returnDirective = return_directive;
		}
		// FALLTHROUGH HANDLING (no valid directive for this scope!)
		else
			bad_directive_throw(d);
	}

	// own message since nginx would not complain about missing listen directives (defaults to 80/8000, which webserv doesn't emulate)
	if (server_config._listenAddresses.size() == 0)
		throw ConfigError("no \"listen\" directive in \"server\" block", server_block.line);

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
