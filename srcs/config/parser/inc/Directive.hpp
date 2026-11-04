#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

struct Directive
{
		std::string name;
		std::vector<std::string> args;
		// when initialized like Directive dir{}, line defaults to 0
		std::size_t line;
		std::optional<std::vector<Directive>> children;
};
