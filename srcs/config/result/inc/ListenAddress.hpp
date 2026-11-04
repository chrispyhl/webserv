#pragma once

#include <cstdint>
#include <string>

struct ListenAddress
{
		// webserv only supports IPv4 addresses, which are 32 bits
		std::uint32_t address;
		std::uint16_t port;
};

bool operator==(const ListenAddress &a, const ListenAddress &b);
std::string address_to_string(const ListenAddress &la);
std::string port_to_string(const ListenAddress &la);
std::string listen_address_to_string(const ListenAddress &la);
