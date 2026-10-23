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
		std::vector<Directive> _directives;
};
