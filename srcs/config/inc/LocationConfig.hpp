#pragma once

#include "Return.hpp"

#include <cstddef>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

class LocationConfig
{
		// friend classes have access to private members
		// ConfigBuilder writes private fields directly
		friend class ConfigBuilder;

	public:
		const std::string &path() const;
		const std::string &root() const;
		bool inheritedRoot() const;
		const std::vector<std::string> &indexFiles() const;
		bool autoIndex() const;
		std::size_t maxBodySize() const;
		const std::map<std::size_t, std::string> &errorPages() const;
		const std::optional<Return> &returnDirective() const;
		const std::set<std::string> &allowedMethods() const;
		const std::optional<std::string> &uploadDirectory() const;
		const std::map<std::string, std::string> &cgiHandlers() const;

	private:
		// only friend class ConfigBuilder creates LocationConfigs
		LocationConfig();

		// URL prefix (with trailing slash removed)
		std::string _path;
		// folder on disk
		std::string _root;
		// true = root comes from server, URL is appended
		// false = location's own root used
		bool _inheritedRoot;
		// file names in order
		std::vector<std::string> _indexFiles;
		// directory listing on or off
		bool _autoIndex;
		// limit for request bodies in bytes
		std::size_t _maxBodySize;
		// accepted methods; at least GET by default (no duplicates in std::set)
		std::set<std::string> _allowedMethods;
		// where uploads go OR uploads disabled
		std::optional<std::string> _uploadDirectory;
		// pairs of file extensions and their interpreters
		std::map<std::string, std::string> _cgiHandlers;

		// these two are shared by ServerConfig and might be imported from there if no insert by Location
		// pairs of status code and page URL
		std::map<std::size_t, std::string> _errorPages;
		// code + target OR no return directive
		std::optional<Return> _returnDirective;
};
