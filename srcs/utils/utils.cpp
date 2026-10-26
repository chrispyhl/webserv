#include "utils.hpp"

#include <cctype>

std::string tolower_string(const std::string &str)
{
	std::string lowered_string;

	for (char c : str)
		lowered_string += std::tolower(static_cast<unsigned char>(c));

	return (lowered_string);
}
