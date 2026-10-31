#pragma once

#include <cstddef>
#include <string>

struct Return
{
	// status code
	std::size_t code;
	// URL to redirect to OR an response text
	std::string target;
};
