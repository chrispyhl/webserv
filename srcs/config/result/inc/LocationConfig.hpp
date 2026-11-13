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
		bool exactMatch() const;
		const std::string &root() const;
		bool inheritedRoot() const;
		const std::vector<std::string> &indexFiles() const;
		bool autoIndex() const;
		std::size_t maxBodySize() const;
		const std::map<std::size_t, std::string> &errorPages() const;
		const std::optional<Return> &returnDirective() const;
		const std::set<std::string> &allowedMethods() const;
		const std::string &uploadDirectory() const;
		const std::map<std::string, std::string> &cgiHandlers() const;

	private:
		// only friend class ConfigBuilder creates LocationConfigs
		LocationConfig();

		// ===OBLIGATORY MEMBERS===
		// URL prefix (with trailing slash removed)
		std::string _path;
		// info whether there was a '=' prior to the path argument ("location = /a")
		// '=' only allows exact matches with the given location, no subdirectories are handled
		bool _exactMatch;
		// folder on disk
		std::string _root;
		// true = root comes from server, URL is appended (default, closest to nginx behavior)
		// false = location's own root used
		bool _inheritedRoot;
		// file names in order
		std::vector<std::string> _indexFiles;
		// directory listing on or off
		bool _autoIndex;
		// limit for request bodies in bytes
		std::size_t _maxBodySize;
		// accepted methods ("GET" is always set if location block has no allow_methods line)
		std::set<std::string> _allowedMethods;

		// ===OPTIONAL MEMBERS===
		// where uploads go
		// .empty() == no uploads allowed
		std::string _uploadDirectory;
		// pairs of file extensions and their interpreters
		// .empty() == no cgi handlers set
		std::map<std::string, std::string> _cgiHandlers;
		// pairs of status code and page URL
		// .empty() == no error pages defined
		std::map<std::size_t, std::string> _errorPages;
		// code + target
		// !.has_value() == no return directive set
		std::optional<Return> _returnDirective;
};
