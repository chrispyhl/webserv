#include "route_request.hpp"

#include "Config.hpp"
#include "ListenAddress.hpp"
#include "LocationConfig.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConfig.hpp"
#include "error_response.hpp"
#include "handle_get.hpp"

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
			return (handle_get(req, server_config, location_config));
		else
			// TODO: handle other methods!
			return (build_error_response(404));
	}
	// not allowed!
	else
	{
		Response res = build_error_response(405);
		// NOTE: usually, the error code and msg are enough for error responses; 405 is a special case: HTTP requires an Allow header to go with it
		res.addHeader("Allow", build_method_header_value(location_config.allowedMethods()));
		return (res);
	}
}
