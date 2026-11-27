#include "handle_get.hpp"
#include "LocationConfig.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConfig.hpp"
#include "error_response.hpp"
#include "handle_helpers.hpp"

#include <filesystem> // for std::filesystem::path
#include <fstream> // for std::ifstream
#include <sstream> // for std::ostringstream
#include <string>

// for debug print in handle_get(); TODO: delete when done
#include <iomanip>
#include <iostream>

static Response handle_get_file(const std::string &disk_path)
{
	// open filestream in binary mode
	std::ifstream filestream(disk_path, std::ios_base::binary);
	// check if readable (catches "chmod 000")
	if (!filestream.is_open())
		return (build_error_response(403));

	// every stream has internal buffer object; filestream can write its whole content into std::ostringstream's buffer with ">>"
	std::ostringstream buffer_dump;
	filestream >> buffer_dump.rdbuf();
	// if something went wrong (like disk error) while reading, filestream stops before the end of the file and its eof flag stays unset
	// extremely rare, but checked here anyway
	if (!filestream.eof())
		return (build_error_response(500));

	Response res(200);
	res.addHeader("content-type", get_content_type(disk_path));
	res.setBody(buffer_dump.str());

	return (res);
}

/*
	- unlike GET for files, a folder can't (or shouldn't) be sent per Response
	- answer can be one of three kinds:
		1) index file: most website-folders have index.html; config file lists which file names to try, first one that exists gets sent
		2) folder listing: if there is no index file, but "autoindex on"; HTML page gets built on the fly, listing folder's contents
		3) error: 404 Not Found
*/
static Response handle_get_directory(const std::string &disk_path, const LocationConfig &location_config)
{
	// CASE 1: index file
	std::filesystem::path path(disk_path);
	for (const std::string &file : location_config.indexFiles())
	{
		// std::filesystem has a / operator to join a path + a path/a string which handles missing '/' between components
		std::filesystem::path joined_path = path / file;
		// this turns path back into a string and sees if it points at a file
		std::string joined_path_string = joined_path.string();
		DiskPathTarget path_target = identify_disk_path_target(joined_path_string);
		if (path_target == NO_PERM)
			return (build_error_response(403));
		if (path_target == IS_FILE)
			return (handle_get_file(joined_path_string));
	}

	// CASE 2: folder listing
	if (location_config.autoIndex())
	{
		// TODO: replace placeholder with proper folder listing logic
		return (build_error_response(501)); // "Not Implemented"
	}
	// CASE 3: error
	return (build_error_response(403)); // code that nginx gives in this scenario
}

Response handle_get(const HttpRequest &req, const ServerConfig &server_config, const LocationConfig &location_config)
{
	std::string disk_path = resolve_to_disk_path(req.get_path(), location_config);
	DiskPathTarget target_type = identify_disk_path_target(disk_path);
	if (target_type == NOT_FOUND)
		return (build_error_response(404));
	if (target_type == NO_PERM)
		return (build_error_response(403));
	if (target_type == OTHER_ERR)
		return (build_error_response(500));
	if (target_type == IS_FILE)
		return (handle_get_file(disk_path));
	if (target_type == IS_DIR)
		return (handle_get_directory(disk_path, location_config));
	// else
	// {
	// TODO: implement handling of IS_OTHER (only possible scenario left)
	// }

	// TODO: delete DEBUG answer when IS_OTHER is handled
	Response res(200);
	res.addHeader("Content-Type", "text/plain");

	std::ostringstream formatted;
	std::string server_name = "(none)";
	if (!server_config.serverNames().empty())
		server_name = server_config.serverNames()[0];

	formatted << "\nDEBUG MSG\nhandle_get()\n===\n"
			  << std::left
			  << std::setw(16) << "method:" << "\"" << req.get_method() << "\"\n"
			  << std::setw(16) << "server name:" << "\"" << server_name << "\"\n"
			  << std::setw(16) << "location path:" << "\"" << location_config.path() << "\"\n"
			  << std::setw(16) << "disk path:" << "\"" << disk_path << "\"\n"
			  << "===\n\n";

	res.setBody(formatted.str());
	return (res);
}
