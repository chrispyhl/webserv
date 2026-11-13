#pragma once

#include <cstddef>
#include <string>

struct Redirect
{
	// status code
	std::size_t code;
	// URL to redirect to
	std::string target;
};
