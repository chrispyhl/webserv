#pragma once

#include "Config.hpp"
#include "Directive.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

#include <vector>

class ConfigBuilder
{
	public:
		ConfigBuilder(std::vector<Directive> directives);
		Config build();

	private:
		ServerConfig buildServer(const std::vector<Directive> &server_directives, std::size_t line);
		void warn_about_duplicates(const Config &config) const;

		std::vector<Directive> _directives;
};
