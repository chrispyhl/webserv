#include "location_directives.hpp"
#include "ConfigError.hpp"
#include "directive_checks.hpp"
#include "utils.hpp"

#include <string>
#include <vector>

namespace config
{

static const std::vector<std::string> SUPPORTED_METHODS = { "GET", "POST", "DELETE" };

std::set<std::string> get_allowed_methods(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() == 0)
		invalid_args_size_throw(d);

	std::set<std::string> allowed_methods;

	for (const std::string &arg : d.args)
	{
		if (!vector_contains(SUPPORTED_METHODS, arg))
			throw ConfigError("invalid method \"" + arg + "\"", d.line);
		// values already in set are ignored
		allowed_methods.insert(arg);
	}

	return (allowed_methods);
}

}
