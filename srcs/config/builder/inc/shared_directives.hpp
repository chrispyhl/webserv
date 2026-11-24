#pragma once

#include "Directive.hpp"
#include "Return.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

// functions for parsing directives that are allowed at server AND location level
namespace config
{

struct LocationDirective
{
		// reference to the server's directive tree
		// only valid during ConfigBuilder::buildServer()
		const Directive &directive;
		std::string path;
		bool exact_match;
};

std::string parse_root(const Directive &d);
std::vector<std::string> parse_index_files(const Directive &d);
bool parse_auto_index(const Directive &d);
std::size_t parse_max_body_size(const Directive &d);
std::map<std::size_t, std::string> parse_error_pages(const Directive &d);
Return parse_return(const Directive &d);
LocationDirective parse_location_directive(const Directive &d);
}
