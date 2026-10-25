#pragma once

#include <cstdint>

struct ListenAddress
{
		// webserv only supports IPv4 addresses, which are 32 bits
		std::uint32_t address;
		std::uint16_t port;
};

bool operator==(const ListenAddress &a, const ListenAddress &b);
