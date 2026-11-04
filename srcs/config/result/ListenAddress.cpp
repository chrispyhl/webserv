#include "ListenAddress.hpp"

bool operator==(const ListenAddress &a, const ListenAddress &b)
{
	return (a.address == b.address && a.port == b.port);
}

std::string address_to_string(const ListenAddress &la)
{
	std::string octet1 = std::to_string((la.address >> 24) & 255);
	std::string octet2 = std::to_string((la.address >> 16) & 255);
	std::string octet3 = std::to_string((la.address >> 8) & 255);
	std::string octet4 = std::to_string((la.address) & 255);

	return (octet1 + "." + octet2 + "." + octet3 + "." + octet4);

}

std::string port_to_string(const ListenAddress &la)
{
	return (std::to_string(la.port));
}

std::string listen_address_to_string(const ListenAddress &la)
{
	return (address_to_string(la) + ":" + port_to_string(la));
}
