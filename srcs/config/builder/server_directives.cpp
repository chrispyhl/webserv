#include "server_directives.hpp"
#include "directive_checks.hpp"
#include "ConfigError.hpp"
#include "utils.hpp"

#include <algorithm> // for std::count()
#include <cctype> // for std::isdigit()
#include <cstddef>
#include <cstdint>

namespace config
{

static void host_not_found_throw(const Directive &d)
{
	throw ConfigError("host not found in \"" + d.args[0] + "\" of the \"listen\" directive", d.line);
}

static void invalid_port_throw(const Directive &d)
{
	throw ConfigError("invalid port in \"" + d.args[0] + "\" of the \"listen\" directive", d.line);
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

ListenAddress get_listen_address(const Directive &d)
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
		else if (address == "*")
			address = "0.0.0.0";
		port = d.args[0].substr(colon_pos + 1);
		// port is still obligatory
		if (port.empty())
			invalid_port_throw(d);
		listen_address = build_listenaddress(address, port, d);
	}
	return (listen_address);
}

std::vector<std::string> get_server_names(const Directive &d)
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

}
