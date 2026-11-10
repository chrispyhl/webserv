#pragma once

#include "Config.hpp"
#include "Directive.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include "shared_directives.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

class ConfigBuilder
{
	public:
		// constructor
		ConfigBuilder(std::vector<Directive> directives);
		Config build();

	private:
		ServerConfig buildServer(const Directive &server_block);

		// struct for temp storage of location elements
		struct ServerDefaults
		{
				// potentially written in server block level, but only relevant for LocationConfig
				std::optional<std::string> root;
				std::optional<std::vector<std::string>> index_files;
				std::optional<bool> auto_index;
				std::optional<std::size_t> max_body_size;
		};

		// buildLocations() needs to be a method of ConfigBuilder so it can directly access LocationConfig's private members
		// add ServerConfig for its error_pages
		std::vector<LocationConfig> buildLocations(const std::vector<config::LocationDirective> &location_directives, const ServerDefaults &server_defaults, const ServerConfig &server_config) const;

		// helper that profits from member access
		void warn_about_duplicates(const Config &config) const;
		// members
		std::vector<Directive> _directives;
};
