/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 16:14:40 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/23 14:44:55 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "incs.hpp"

enum ParseState {
	STATE_REQUEST_LINE,
	STATE_HEADERS,
	STATE_HEADERS_COMPLETE,
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

		bool								_is_chunked;
		size_t								_chunk_remaining;
		enum	ChunkState {
			CHUNK_SIZE,
			CHUNK_DATA,
			CHUNK_DATA_CRLF,
			CHUNK_TRAILER
		};
		ChunkState							_chunk_state;
		size_t								_max_body_size;
		int									_error_code;

		bool	parse_request_line(const std::string& line);
		bool	parse_header_line(const std::string& line);
		void	parse_body(std::string &read_buffer);
		void	parse_chunked_body(std::string &read_buffer);
	
	public:
		HttpRequest();
		~HttpRequest();

		ParseState									parse(std::string &read_buffer);

		ParseState									get_state() const;
		size_t										get_content_length() const;
		const std::string&							get_method() const;
		const std::string&							get_path() const;
		const std::string&							get_query() const;
		const std::string&							get_version() const;
		const std::string&							get_body() const;
		const std::string&							get_header(const std::string& key) const;
		const std::map<std::string, std::string>&	get_headers() const;
		void										set_max_body_size(size_t max_size);
		void										start_body();
		int											get_error_code() const;

		void	clear();
};

#endif