#pragma once

#include "Config.hpp"
#include "Directive.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

#include <optional>
#include <string>
#include <vector>

class ConfigBuilder
{
	public:
		ConfigBuilder(std::vector<Directive> directives);
		Config build();

	private:
		// constructor
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
		// helper that profits from member access
		void warn_about_duplicates(const Config &config) const;
		// members
		std::vector<Directive> _directives;

};
