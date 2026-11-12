#pragma once

#include "Directive.hpp"

#include <string>
#include <set>

// handlers for directives that are only allowed at location level
namespace config
{
	std::set<std::string> get_allowed_methods(const Directive &d);
}
