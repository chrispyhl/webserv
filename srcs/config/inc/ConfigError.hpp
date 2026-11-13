#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>

class ConfigError : public std::runtime_error
{
	public:
		ConfigError(const std::string &msg, std::optional<std::size_t> line);
		std::string format(const std::string &filepath) const;

	private:
		std::optional<std::size_t> _line;
};
