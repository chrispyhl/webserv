#include "shared_directives.hpp"
#include "ConfigError.hpp"
#include "directive_checks.hpp"
#include "utils.hpp"

#include <cctype> // for std::isdigit()
#include <limits>
#include <optional>
#include <stdexcept> // for std::invalid_argument

namespace config
{

static void invalid_return_code_throw(const Directive &d, const std::string &str)
{
	throw ConfigError("invalid return code \"" + str + "\"", d.line);
}

std::string get_root(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() != 1)
		invalid_args_size_throw(d);
	return (d.args[0]);
}

std::vector<std::string> get_index_files(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() == 0)
		invalid_args_size_throw(d);
	for (const std::string &s : d.args)
		if (s.empty())
			throw ConfigError("index \"\" in \"index\" directive is invalid", d.line);
	return (d.args);
}

bool get_auto_index(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() != 1)
		invalid_args_size_throw(d);
	std::string arg = tolower_string(d.args[0]);
	if (arg == "on")
		return (true);
	else if (arg == "off")
		return (false);
	else
		throw ConfigError("invalid value \"" + d.args[0] + "\" in \"autoindex\" directive, it must be \"on\" or \"off\"", d.line);
}

std::size_t get_max_body_size(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() != 1)
		invalid_args_size_throw(d);

	const std::string &str = d.args[0];
	if (str.empty())
		throw ConfigError("\"client_max_body_size\" directive invalid value", d.line);

	const std::string allowed_chars = "kKmMgG";
	std::optional<char> suffix;

	for (auto it = str.begin(); it != str.end(); it++)
	{
		if (!std::isdigit(static_cast<unsigned char>(*it)))
		{
			if ((it + 1) == str.end() && allowed_chars.find(*it) != std::string::npos)
				suffix = *it;
			else
				throw ConfigError("\"client_max_body_size\" directive invalid value", d.line); // unallowed char was found
		}
	}

	std::size_t digits_length;
	if (!suffix.has_value())
		digits_length = str.size();
	else
		digits_length = str.size() - 1;

	std::size_t max_body_size;

	try
	{
		max_body_size = str_to_size_t(str.substr(0, digits_length));
	}
	catch (const std::invalid_argument &)
	{
		throw ConfigError("\"client_max_body_size\" directive invalid value", d.line);
	}

	if (!suffix.has_value())
		return (max_body_size); // no suffix = just return base value

	std::size_t limit = std::numeric_limits<size_t>::max();
	switch (suffix.value())
	{
		case ('k'):
		case ('K'):
		{
			if (max_body_size > (limit / 1024))
				break;
			return (max_body_size * 1024);
		}
		case ('m'):
		case ('M'):
		{
			if (max_body_size > (limit / (1024 * 1024)))
				break;
			return (max_body_size * 1024 * 1024);
		}
		case ('g'):
		case ('G'):
		{
			if (max_body_size > (limit / (1024 * 1024 * 1024)))
				break;
			return (max_body_size * 1024 * 1024 * 1024);
		}
		default:
			break;
	}
	throw ConfigError("\"client_max_body_size\" directive invalid value", d.line);
}

std::map<std::size_t, std::string> get_error_pages(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() < 2)
		invalid_args_size_throw(d);
	std::vector<std::size_t> codes;
	for (std::size_t i = 0; i < (d.args.size() - 2); i++)
	{
		try
		{
			std::size_t code_nbr = str_to_size_t(d.args[i]);
			if (code_nbr < 300 || code_nbr > 599)
				throw ConfigError("value \"" + d.args[i] + "\" must be between 300 and 599", d.line);
			if (code_nbr == 499)
				throw ConfigError("invalid value \"499\"", d.line);
			codes.push_back(code_nbr);
		}
		catch (const std::invalid_argument &)
		{
			throw ConfigError("invalid value \"" + d.args[i] + "\"", d.line);
		}
	}
	std::string uri = d.args[d.args.size() - 1];
	if (uri[0] != '/')
		throw ConfigError("invalid value \"" + uri + "\"", d.line);
	std::map<std::size_t, std::string> error_pages;
	for (size_t code : codes)
		error_pages.insert({ code, uri });
	return (error_pages);
}

static std::size_t get_return_code(const Directive &d)
{
	std::string str = d.args[0];
	std::size_t code;
	try
	{
		code = str_to_size_t(str);
		// code needs to be between (and including) 0 and 999
		// can't be negative due to isdigit_string() check before, so only 999 needs guarding
		if (code > 999)
			invalid_return_code_throw(d, str);
	}
	catch (const std::invalid_argument &)
	{
		invalid_return_code_throw(d, str);
	}
	return (code);
}

Return get_return(const Directive &d)
{
	check_for_illegal_block(d);
	if (d.args.size() < 1 || d.args.size() > 2)
		invalid_args_size_throw(d);

	Return result;

	// one argument is only legal if its a code OR a target that starts with "http://" or "https://" (has default code 302)
	if (d.args.size() == 1)
	{
		if (isdigit_string(d.args[0]))
		{
			std::size_t code = get_return_code(d);
			result.code = code;
			result.target = "";
		}
		else
		{
			std::string target = d.args[0];
			if (target.substr(0, 7) == "http://" || target.substr(0, 8) == "https://")
			{
				result.code = 302;
				result.target = target;
			}
			else
				invalid_return_code_throw(d, target);
		}
	}
	// 2 arg scenario: first must be code, second is target
	else if (isdigit_string(d.args[0]))
	{
		std::size_t code = get_return_code(d);
		std::string target = d.args[1];
		result.code = code;
		result.target = target;
	}
	else
		invalid_return_code_throw(d, d.args[0]);
	return (result);
}

static bool get_location_modifier(const Directive &d)
{
	std::vector<std::string> unsupported_modifiers = { "~", "~*", "^~" };
	std::string modifier = d.args[0];
	if (modifier == "=")
		return (true);
	if (vector_contains(unsupported_modifiers, modifier))
		throw ConfigError("location modifiers are not supported", d.line);

	throw ConfigError("invalid location modifier \"" + modifier + "\"", d.line);
}

// exact_match is used by caller to build LocationDirective
// also serves as a toggle for literal paths here
static std::string get_location_path(const Directive &d, const std::string &str, bool &exact_match)
{
	std::string path;

	if (str[0] == '=')
	{
		if (!exact_match)
		{
			exact_match = true;
			path = str.substr(1);
		}
		else
			throw ConfigError("location path \"" + str + "\" must start with \"/\"", d.line);
	}
	else
		path = str;

	// exact_match == false means that the following is NOT taken literally; not "no /" is the problem, but the user trying to make use of modifiers
	if (!exact_match && (path[0] == '@' || path[0] == '~' || path.substr(0, 2) == "^~"))
	{
		throw ConfigError("location modifiers are not supported", d.line);
	}
	if (path.empty() || path[0] != '/')
		throw ConfigError("location path \"" + path + "\" must start with \"/\"", d.line);

	// trimming trailing forward-slashes
	std::size_t path_end_pos = path.find_last_not_of('/');
	// no non slashes = its all slashes!
	if (path_end_pos == std::string::npos)
		path = "/";
	else
		path = path.substr(0, path_end_pos + 1);

	return (path);
}

LocationDirective get_location_directive(const Directive &d)
{
	if (!d.children.has_value())
		throw ConfigError("directive \"location\" has no opening \"{\"", d.line);
	if (d.args.empty() || d.args.size() > 2)
		invalid_args_size_throw(d);

	std::string path;
	bool exact_match = false;

	if (d.args.size() == 2)
	{
		exact_match = get_location_modifier(d);
		path = get_location_path(d, d.args[1], exact_match);
	}
	else
		path = get_location_path(d, d.args[0], exact_match);

	return (LocationDirective { d, path, exact_match });
}

}
