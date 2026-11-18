#pragma once

#include "Directive.hpp"

#include <set>
#include <string>
#include <utility> // for std::pair

// handlers for directives that are only allowed at location level
namespace config
{
std::set<std::string> get_allowed_methods(const Directive &d);
std::string get_upload_directory(const Directive &d);
std::pair<std::string, std::string> get_cgi_handler_pair(const Directive &d);
}
