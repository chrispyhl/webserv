#pragma once

#include "LocationConfig.hpp"

#include <string>

enum DiskPathTarget
{
	IS_FILE,
	IS_DIR,
	IS_OTHER,
	NOT_FOUND,
	NO_PERM,
	OTHER_ERR
};

std::string resolve_to_disk_path(const std::string &req_path, const LocationConfig &location_config);
DiskPathTarget identify_disk_path_target(const std::string &disk_path);
std::string get_content_type(const std::string &disk_path);
