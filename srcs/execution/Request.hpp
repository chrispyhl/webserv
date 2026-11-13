#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "incs.hpp"

enum ParseState {
	STATE_REQUEST_LINE,
	STATE_HEADERS,
	STATE_BODY,
	STATE_COMPLETE,
	STATE_ERROR
};

class HttpRequest {
	private:
		std::string							_method;
		std::string							_path;
		std::string							_query;
		std::string							_version;
		std::map<std::string, std::string>	_headers;
		std::string							_body;

		ParseState							_state;
		size_t								_content_length;

		bool	parse_request_line(const std::string& line);
		bool	parse_header_line(const std::string& line);
		void	parse_body(std::string &read_buffer);
	
	public:
		HttpRequest();
		~HttpRequest();

		ParseState									parse(std::string &read_buffer);

		ParseState									get_state() const;
		const std::string&							get_method() const;
		const std::string&							get_path() const;
		const std::string&							get_query() const;
		const std::string&							get_version() const;
		const std::string&							get_body() const;
		const std::string&							get_header(const std::string& key) const;
		const std::map<std::string, std::string>&	get_headers() const;

		void	set_method(const std::string& method);
		void	set_path(const std::string& path);
		void	set_query(const std::string& query);
		void	set_version(const std::string& version);
		void	add_header(const std::string& key, const std::string& value);
		void	append_body(const char* data, size_t len);

		void	clear();
};

#endif