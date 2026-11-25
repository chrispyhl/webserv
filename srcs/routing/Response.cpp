#include "Response.hpp"
#include "utils.hpp"

#include <cstddef>
#include <map>
#include <string>

static const std::map<std::size_t, std::string> CODE_CATALOGUE = {
	{ 200, "OK" },
	{ 201, "Created" },
	{ 204, "No Content" },
	{ 301, "Moved Permanently" },
	{ 302, "Found" },
	{ 303, "See Other" },
	{ 307, "Temporary Redirect" },
	{ 308, "Permanent Redirect" },
	{ 400, "Bad Request" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 405, "Method Not Allowed" },
	{ 408, "Request Timeout" },
	{ 413, "Content Too Large" },
	{ 414, "URI Too Long" },
	{ 431, "Request Header Fields Too Large" },
	{ 500, "Internal Server Error" },
	{ 502, "Bad Gateway" },
	{ 504, "Gateway Timeout" },
	{ 505, "HTTP Version Not Supported" }
};

// _headers and _body start empty (std::map and std::string construct themselves empty)
Response::Response(std::size_t status_code) : _statusCode(status_code), _closeConnection(false)
{
}

std::string Response::toString() const
{
	// for format, see Response.hpp

	std::string str = "HTTP/1.0 ";
	// add code and code text
	str += std::to_string(_statusCode) + " " + get_status_text(_statusCode) + "\r\n";
	// headers
	for (const std::pair<const std::string, std::string> &h : _headers)
	{
		// some headers may submit their own "Content-Length" line; those must be filtered out here
		// same goes for "Connection:" lines
		std::string header_name = tolower_string(h.first);
		if (header_name != "content-length" && header_name != "connection")
			str += h.first + ": " + h.second + "\r\n";
	}
	// body size
	str += "Content-Length: " + std::to_string(_body.size()) + "\r\n";
	// close connection?
	if (_closeConnection)
		str += "Connection: close\r\n";
	// empty line
	str += "\r\n";
	// body
	str += _body;

	return (str);
}

// setters
void Response::setStatusCode(std::size_t status_code)
{
	_statusCode = status_code;
}

// a header with an already stored name overwrites the old value
// new headers are simply added
void Response::addHeader(const std::string &name, const std::string &value)
{
	_headers[tolower_string(name)] = value;
}

// replaces all stored headers
void Response::setHeaders(const std::map<std::string, std::string> &headers)
{
	_headers.clear();
	for (const auto &p : headers)
		addHeader(tolower_string(p.first), p.second);
}

void Response::setBody(const std::string &body)
{
	_body = body;
}

void Response::setCloseConnection(bool close_connection)
{
	_closeConnection = close_connection;
}

// getters
std::size_t Response::statusCode() const
{
	return (_statusCode);
}

const std::map<std::string, std::string> &Response::headers() const
{
	return (_headers);
}

const std::string &Response::body() const
{
	return (_body);
}

bool Response::closeConnection() const
{
	return (_closeConnection);
}

// free function
std::string get_status_text(std::size_t status_code)
{
	auto found = CODE_CATALOGUE.find(status_code);

	if (found != CODE_CATALOGUE.end())
		return (found->second);

	// TODO: agree on proper message
	switch (status_code / 100)
	{
		case (3):
			return ("redirect");
		case (4):
			return ("client error");
		case (5):
			return ("server error");
		default:
			return ("unknown code");
	}
}
