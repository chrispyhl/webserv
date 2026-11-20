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

Response route_request(const HttpRequest &req, const ListenAddress &addr, const Config &config)
{
	// find relevant server
	const ServerConfig &server = config.selectServer(addr, req.get_header("Host"));

	// TODO: server's "return", path cleanup

	// find relevant location
	const LocationConfig &location = server.findLocation(req.get_path());

	// TODO: location's "return"

	// check method
	// allowed!
	if (location.allowedMethods().count(req.get_method()) != 0)
	{
		// TODO: replace DEBUG answer
		Response res(200);
		res.addHeader("Content-Type", "text/plain");
		res.setBody("\nDEBUG MSG\n===\nmethod: " + 
			req.get_method() + "; " + 
			(!server.serverNames().empty() ? "server name: " + server.serverNames()[0] : "no server name") + 
			"; location path: " + location.path() + "\n===\n\n");
		return (res);
	}
	// not allowed!
	else
	{
		Response res = get_error_response(405);
		res.addHeader("Allow", build_method_header_value(location.allowedMethods()));
		return (res);
	}
}
