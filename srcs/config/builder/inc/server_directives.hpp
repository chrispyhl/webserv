#pragma once

#include "Directive.hpp"
#include "ListenAddress.hpp"

#include <string>
#include <vector>

// functions for parsing directives that are only allowed at server level
namespace config
{
	ListenAddress parse_listen_address(const Directive &d);
	std::vector<std::string> parse_server_names(const Directive &d);
}
