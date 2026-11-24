#include "handle_helpers.hpp"

#include "LocationConfig.hpp"

#include <filesystem>
#include <string>
#include <system_error>

// to be used to find the actual server disk locations of files and directories targeted by GET and DELETE methods
std::string resolve_to_disk_path(const std::string &req_path, const LocationConfig &location_config)
{
	std::string disk_path;

	// the path from the request (req_path) is guaranteed to start with the location path
	// otherwise, this location wouldn't have been chosen

	// .inheritedRoot() = location inherits root from server = the location path sits on the server root
	if (location_config.inheritedRoot())
		disk_path = location_config.root() + req_path;
	// !.inheritedRoot() = location has own root directive = replaces location path
	else
	{
		// edge case: root wasn't inherited, req_path is "/location" (looking for a folder), location_path is also "/location"
		if (req_path == location_config.path())
			return (location_config.root());

		if (location_config.path() == "/")
			disk_path = location_config.root() + req_path;
		else
		{
			std::string stripped_req_path = req_path.substr(location_config.path().size());
			disk_path = location_config.root() + stripped_req_path;
		}
	}

	return (disk_path);

	// TODO: see later if double slash sanitation ever gets necessary

	// // sanitize any double forward slashes (request path does not come trimmed)
	// std::string res;
	// bool found_slash = false;
	// for (const char c : disk_path)
	// {
	// 	if (c == '/')
	// 	{
	// 		if (found_slash)
	// 			continue;
	// 		found_slash = true;
	// 	}
	// 	else
	// 		found_slash = false;
	// 	res += c;
	// }
	// return (res);
}

DiskPathTarget identify_disk_path_target(const std::string &disk_path)
{
	// object that stores parameter-output of status check (would throw without it)
	std::error_code ec;
	// filesystem version of the disk path
	std::filesystem::path path(disk_path);
	// holds the type of what's at the path; status.type() == none + ec set if it couldn't even look
	std::filesystem::file_status status = std::filesystem::status(path, ec);
	std::filesystem::file_type status_type = status.type();

	switch (status_type)
	{
		case (std::filesystem::file_type::not_found):
			return (NOT_FOUND);
		case (std::filesystem::file_type::none):
		{
			if (ec == std::errc::permission_denied)
				return (NO_PERM);
			return (OTHER_ERR); // couldn't even look for some other reason than "no permission"
		}
		case (std::filesystem::file_type::regular):
			return (IS_FILE);
		case (std::filesystem::file_type::directory):
			return (IS_DIR);
		default:
			return (IS_OTHER); // something other than a file or folder, such as a device or socket
	}
}
