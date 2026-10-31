#include "utils.hpp"

#include <cctype>
#include <charconv>
#include <stdexcept>

std::string tolower_string(const std::string &str)
{
	std::string lowered_string;

	for (char c : str)
		lowered_string += std::tolower(static_cast<unsigned char>(c));

	return (lowered_string);
}

bool isdigit_string(const std::string &str)
{
	if (str.empty())
		return (false);
	for (char c : str)
		if (!std::isdigit(static_cast<unsigned char>(c)))
			return (false);
	return (true);
}

// benefits of std::from_chars over std::stoi: parses directly into std::size_t, does not throw in case of conversion error or overflow and is stricter than std::stoi (doesn't accept +/- and doesn't just stop at non-digit)
std::size_t str_to_size_t(const std::string &str)
{
	// .data() from std::string returns a pointer to the first character
	const char *start = str.data();
	const char *end = str.data() + str.size();

	std::size_t result;

	// std::from_chars returns a custom feedback struct that contains an error code object and a pointer to the spot one past the last processed character
	std::from_chars_result from_chars_result;
	from_chars_result = std::from_chars(start, end, result);
	// only valid result: error code object is blank ({}) and the pointer points at the end of the digits; anything else = error
	if (from_chars_result.ec != std::errc { } || (from_chars_result.ptr != end))
		throw std::invalid_argument("str_to_size_t: not a valid number");

	return (result);
}
