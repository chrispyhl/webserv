#pragma once

#include "Directive.hpp"

#include <set>
#include <string>
#include <utility> // for std::pair

// functions for parsing directives that are only allowed at location level
namespace config
{
std::set<std::string> parse_allowed_methods(const Directive &d);
std::string parse_upload_directory(const Directive &d);
std::pair<std::string, std::string> parse_cgi_handler_pair(const Directive &d);
}
