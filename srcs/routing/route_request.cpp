#include "route_request.hpp"

#include "Config.hpp"
#include "ListenAddress.hpp"
#include "LocationConfig.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConfig.hpp"
#include "error_response.hpp"

#include <set>
#include <string>

static std::string build_method_header_value(const std::set<std::string> &allowed_methods)
{
	std::string header;
	for (const std::string &s : allowed_methods)
	{
		if (!header.empty())
			header += ", ";
		header += s;
	}
	return (header);
}

// to be used to find the actual server disk locations of files and directories targeted by GET and DELETE methods
static std::string resolve_to_disk_path(const std::string &req_path, const LocationConfig &location_config)
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

// for debug print; TODO: delete when done
#include <iomanip>
#include <iostream>
#include <sstream>

static Response get_get_response(const HttpRequest &req, const ServerConfig &server_config, const LocationConfig &location_config)
{
	std::string disk_path = resolve_to_disk_path(req.get_path(), location_config);
	// if (!verify_disk_path_get(disk_path))
	// 	return (get_error_response(404));
	// TODO: replace DEBUG answer
	Response res(200);
	res.addHeader("Content-Type", "text/plain");

	std::ostringstream formatted;
	std::string server_name = "(none)";
	if (!server_config.serverNames().empty())
		server_name = server_config.serverNames()[0];

	formatted << "\nDEBUG MSG\n===\n"
			  << std::left
			  << std::setw(16) << "method:" << "\"" << req.get_method() << "\"\n"
			  << std::setw(16) << "server name:" << "\"" << server_name << "\"\n"
			  << std::setw(16) << "location path:" << "\"" << location_config.path() << "\"\n"
			  << std::setw(16) << "disk path:" << "\"" << disk_path << "\"\n"
			  << "===\n\n";

	res.setBody(formatted.str());
	return (res);
}

Response route_request(const HttpRequest &req, const ListenAddress &addr, const Config &config)
{
	// find relevant server
	const ServerConfig &server_config = config.selectServer(addr, req.get_header("Host"));

	// TODO: server's "return", path cleanup

	// find relevant location
	const LocationConfig &location_config = server_config.findLocation(req.get_path());

	// TODO: location's "return"

	// check method
	// allowed!
	if (location_config.allowedMethods().count(req.get_method()) != 0)
	{
		if (req.get_method() == "GET")
			return (get_get_response(req, server_config, location_config));
		else
			// TODO: handle other methods!
			return (get_error_response(404));
	}
	// not allowed!
	else
	{
		Response res = get_error_response(405);
		// NOTE: usually, the error code and msg are enough for error responses; 405 is a special case: HTTP requires an Allow header to go with it
		res.addHeader("Allow", build_method_header_value(location_config.allowedMethods()));
		return (res);
	}
}
