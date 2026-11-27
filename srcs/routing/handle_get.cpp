#include "handle_get.hpp"
#include "LocationConfig.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConfig.hpp"
#include "error_response.hpp"
#include "handle_helpers.hpp"

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
	// else if (target_type == IS_DIR)
	// {
	// }
	// else
	// {

	// }
	// TODO: replace DEBUG answer
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
