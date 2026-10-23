#include "ConfigBuilder.hpp"
#include "ConfigError.hpp"

#include <algorithm>

const std::vector<std::string> KNOWN_DIRECTIVES = {
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

ConfigBuilder::ConfigBuilder(std::vector<Directive> directives) : _directives(directives)
{
}

static void throw_error_not_server(const Directive &directive)
{
	if (std::find(KNOWN_DIRECTIVES.begin(), KNOWN_DIRECTIVES.end(), directive.name) != KNOWN_DIRECTIVES.end())
		throw ConfigError("\"" + directive.name + "\" directive is not allowed here", directive.line);
	else
		throw ConfigError("unknown directive \"" + directive.name + "\"", directive.line);
}

// INFO: nginx has a http {[...]} wrapper level around all server directives
// webserv does not handle this so far; it assumes the config file is equivalent to what is supposed to be inside the http {[...]} block of a standard nginx config file
// TODO: decide whether to add http wrapper handling
Config ConfigBuilder::build()
{
	if (_directives.size() == 0)
		throw ConfigError("no \"server\" directive found", std::nullopt);

	// check top level for config errors
	for (const Directive &d : _directives)
	{
		if (d.name != "server")
			throw_error_not_server(d);
		if (d.args.size() != 0)
			throw ConfigError("invalid number of arguments in \"server\" directive", d.line);
		if (!d.children.has_value())
			throw ConfigError("directive \"server\" has no opening \"{\"", d.line);
	}
	return (Config());
}
