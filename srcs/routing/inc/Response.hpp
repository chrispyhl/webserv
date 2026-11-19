#pragma once

#include <cstddef>
#include <map>
#include <string>

class Response
{
	public:
		// NOTE: without the keyword "explicit", a constructor with 1 argument doubles as an automatic type conversion (built-in CPP behavior)
		// where a Response is expected and a number (std::size_t) is given, the compiler silently builds a Response from it
		// with "explicit", the compiler rejects such code; a Response has to be built on purpose
		explicit Response(std::size_t status_code);

		/* most important result for the execution part; serializes everything into one string in this format:

		HTTP/1.0 CODE STATUS_TEXT\r\n
		Name: value\r\n				(one line per stored header)
		Content-Length: BODY_SIZE\r\n
		Connection: close\r\n		(only if closeConnection() is true)
		\r\n						(empty line: end of the headers)
		BODY
		
		*/
		std::string toString() const;

		// setters
		void setStatusCode(std::size_t status_code);
		void addHeader(const std::string &name, const std::string &value);
		void setHeaders(const std::map<std::string, std::string> &headers);
		void setBody(const std::string &body);
		void setCloseConnection(bool close_connection);

		// getters
		std::size_t statusCode() const;
		const std::map<std::string, std::string> &headers() const;
		const std::string &body() const;
		bool closeConnection() const;

	private:
		std::size_t _statusCode; // set in the constructor's member initializer list
		std::map<std::string, std::string> _headers; // header name -> header value
		std::string _body;
		bool _closeConnection; // false by default (set in the constructor)
};

// free function: status code -> status text (404 -> "Not Found")
// used by toString() and error_response()
std::string get_status_text(std::size_t status_code);
