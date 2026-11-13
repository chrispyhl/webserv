#include "directive_checks.hpp"
#include "ConfigError.hpp"
#include "utils.hpp"

#include <string>
#include <vector>

namespace config
{

static const std::vector<std::string> KNOWN_DIRECTIVES = {
	"server",
	"location",
	"listen",
	"server_name",
	"error_page",
	"client_max_body_size",
	"root",
	"index",
	"autoindex",
	"return",
	"allow_methods",
	"upload_dir",
	"cgi"
};

void bad_directive_throw(const Directive &d)
{
	if (vector_contains(KNOWN_DIRECTIVES, d.name))
		throw ConfigError("\"" + d.name + "\" directive is not allowed here", d.line);
	else
		throw ConfigError("unknown directive \"" + d.name + "\"", d.line);
}

void duplicate_throw(const Directive &d)
{
	throw ConfigError("\"" + d.name + "\" directive is duplicate", d.line);
}

void invalid_args_size_throw(const Directive &d)
{
	throw ConfigError("invalid number of arguments in \"" + d.name + "\" directive", d.line);
}

void check_for_illegal_block(const Directive &d)
{
	if (d.children.has_value())
		throw ConfigError("directive \"" + d.name + "\" is not terminated by \";\"", d.line);
}

}
