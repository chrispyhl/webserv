#include "ListenAddress.hpp"

bool operator==(const ListenAddress &a, const ListenAddress &b)
{
	return (a.address == b.address && a.port == b.port);
}
