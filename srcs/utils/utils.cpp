#include "utils.hpp"

#include <cctype>

std::string tolower_string(const std::string &str)
{
	std::string lowered_string;

	for (char c : str)
		lowered_string += std::tolower(static_cast<unsigned char>(c));

	return (lowered_string);
}

bool isdigit_string(const std::string &str)
{
	for (char c : str)
		if (!std::isdigit(static_cast<unsigned char>(c)))
			return (false);
	return (true);
}
