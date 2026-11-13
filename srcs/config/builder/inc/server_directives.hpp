#pragma once

#include "Directive.hpp"
#include "ListenAddress.hpp"

#include <string>
#include <vector>

// handlers for directives that are only allowed at server level
namespace config
{
	ListenAddress get_listen_address(const Directive &d);
	std::vector<std::string> get_server_names(const Directive &d);
}
