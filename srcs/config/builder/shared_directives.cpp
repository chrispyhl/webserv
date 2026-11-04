#include "shared_directives.hpp"
#include "directive_checks.hpp"
#include "ConfigError.hpp"
#include "utils.hpp"

#include <cctype> // for std::isdigit()
#include <limits>
#include <optional>
#include <stdexcept> // for std::invalid_argument

namespace config
{

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
	auto it = d.args.begin();
	for (; it != d.args.end(); it++)
	{
		if (isdigit_string(*it))
		{
			try
			{
				codes.push_back(str_to_size_t(*it));
			}
			catch (const std::invalid_argument &)
			{
				throw ConfigError("invalid value \"" + *it + "\"", d.line);
			}
		}
		else
			break;
	}
	if (codes.empty())
		throw ConfigError("invalid value \"" + d.args[0] + "\"", d.line);
	if (it + 1 != d.args.end())
		throw ConfigError("invalid value \"" + *it + "\"", d.line);
	std::string uri = *it;
	std::map<std::size_t, std::string> error_pages;
	for (size_t c : codes)
	{
		if (c < 300 || c > 599)
			throw ConfigError("value \"" + std::to_string(c) + "\" must be between 300 and 599", d.line);
		if (c == 499)
			throw ConfigError("invalid value \"499\"", d.line);
		error_pages.insert({ c, uri });
	}
	// TODO: implement nginx's URI guidelines
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
			throw ConfigError("invalid return code \"" + str + "\"", d.line);
	}
	catch (const std::invalid_argument &)
	{
		throw ConfigError("invalid return code \"" + str + "\"", d.line);
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
				throw ConfigError("invalid return code \"" + target + "\"", d.line);
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
		throw ConfigError("invalid return code \"" + d.args[0] + "\"", d.line);

	return (result);
}

}
