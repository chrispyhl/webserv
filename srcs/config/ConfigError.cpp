#include "ConfigError.hpp"

ConfigError::ConfigError(const std::string &msg, std::optional<std::size_t> line)
	: std::runtime_error(msg), _line(line)
{
}

std::string ConfigError::format(const std::string &filepath) const
{
	std::string msg = "webserv: " + filepath;

	if (_line.has_value())
		msg += ":" + std::to_string(_line.value());
	msg += ": ";
	// what() msg (a char*) needs to be added separately for whatever reason
	msg += what();
	return (msg);
}
