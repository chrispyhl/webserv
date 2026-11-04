#pragma once

#include "Directive.hpp"
#include "Return.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

// handlers for directives that are allowed at server AND location level
namespace config
{
	std::string get_root(const Directive &d);
	std::vector<std::string> get_index_files(const Directive &d);
	bool get_auto_index(const Directive &d);
	std::size_t get_max_body_size(const Directive &d);
	std::map<std::size_t, std::string> get_error_pages(const Directive &d);
	Return get_return(const Directive &d);
}
