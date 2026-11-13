#pragma once

#include "Config.hpp"
#include "Directive.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include "shared_directives.hpp"

#include <cstddef>
#include <map>
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

		// values a block (server or location) sets itself via the shared directives
		// not in here: block-specific directives (listen, server_name, allow_methods, upload_dir, cgi) and return
		struct BlockValues
		{
				// potentially written on server/location block level, but only relevant for LocationConfig
				std::optional<std::string> root;
				std::optional<std::vector<std::string>> index_files;
				std::optional<bool> auto_index;
				std::optional<std::size_t> max_body_size;
				// copy of a server, set after all server directives are read (for location only if it has no own error_pages)
				std::optional<std::map<std::size_t, std::string>> error_pages;
		};

		// builds all locations of a server block, then checks for duplicates
		std::vector<LocationConfig> buildLocations(const std::vector<config::LocationDirective> &location_directives, const BlockValues &inherited_values) const;

		// buildLocation() needs to be a method of ConfigBuilder so it can directly access LocationConfig's private members
		LocationConfig buildLocation(const config::LocationDirective &location_directive, const BlockValues &inherited_values) const;

		// helpers that profit from member access
		void warnAboutDuplicates(const Config &config) const;
		void mergeLocationInheritance(LocationConfig &location_config, const BlockValues &block_values, const BlockValues &inherited_values) const;

		// members
		std::vector<Directive> _directives;
};
