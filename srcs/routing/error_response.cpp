#include "Response.hpp"
#include "error_response.hpp"

#include <cstddef>
#include <string>

Response build_error_response(std::size_t error_code)
{
	Response response(error_code);

	response.addHeader("Content-Type", "text/html");
	std::string title_text = std::to_string(error_code) + " " + get_status_text(error_code);

	// title = text in browser tab
	// h1 = big heading
	// html, head, body = frames required for every page
	// browsers don't care about line breaks; \n just for tidy curl output
	std::string body = "<html><head><title>\n";
	body += title_text + "\n";
	body += "</title></head><body><h1>\n";
	body += title_text + "\n";
	body += "</h1></body></html>\n";

	response.setBody(body);

	return (response);
}
