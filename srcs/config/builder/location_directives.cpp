#include "location_directives.hpp"
#include "ConfigError.hpp"
#include "directive_checks.hpp"
#include "utils.hpp"

#include <string>
#include <utility> // for std::pair
#include <vector>

namespace config
{

static const std::vector<std::string> SUPPORTED_METHODS = { "GET", "POST", "DELETE" };

std::set<std::string> parse_allowed_methods(const Directive &d)
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

std::string parse_upload_directory(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() != 1)
		invalid_args_size_throw(d);

	return (trim_trailing_forward_slashes(d.args[0]));
}

std::pair<std::string, std::string> parse_cgi_handler_pair(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() == 0 || d.args.size() > 2)
		invalid_args_size_throw(d);

	// extension check
	std::string extension = d.args[0];
	// fail cases: just one char, doesn't start with dot, contains '/'
	if (extension.size() < 2 || extension[0] != '.' || extension.find('/') != std::string::npos)
		throw ConfigError("invalid cgi extension \"" + extension + "\"", d.line);

	std::string interpreter = "";
	if (d.args.size() == 2)
		// validity of interpreter path is not checked at parse time
		interpreter = d.args[1];
	std::pair<std::string, std::string> pair = { extension, interpreter };
	return (pair);
}

}
