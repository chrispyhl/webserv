/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/18 15:42:46 by chris             #+#    #+#             */
/*   Updated: 2026/11/18 17:02:22 by chris            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() : _status_code(200), _reason_phrase("OK"){
	_headers["Connection"] = "close";
}

Response::~Response(){}

void	Response::set_status(std::size_t code, const std::string &reason){
	_status_code = code;
	_reason_phrase = reason;
}

void	Response::set_header(const std::string &key, const std::string &value){
	_headers[key] = value;
}

void	Response::set_body(const std::string &body){
	_body = body;
}

void	Response::set_content_type(const std::string &path){
	size_t		dot = path.find_last_of('.');
	std::string	ext = (dot == std::string::npos) ? "" : path.substr(dot);

	std::string	mime = "application/octet-stream"; // Default
	if (ext == ".html" || ext == ".htm") mime = "text/html";
	else if (ext == ".css") mime = "text/css";
	else if (ext == ".js") mime = "application/javascript";
	else if (ext == ".png") mime = "image/png";
	else if (ext == ".jpg" || ext == ".jpeg") mime = "image/jpeg";
	else if (ext == ".txt") mime = "text/plain";

	set_header("Content-Type", mime);
}

std::string	Response::build() const{
	std::ostringstream	oss;
	// 1. Status Line
	oss << "HTTP/1.0 " << _status_code << " " << _reason_phrase << "\r\n";

	// 2. Set Content-Length automatic (if it wasn't set manually)
	std::map<std::string, std::string> headers_copy = _headers;
	if (headers_copy.find("Content-Length") == headers_copy.end()){
		headers_copy["Content-Length"] = std::to_string(_body.size());
	}

	// 3. Headers
	for (const auto &header : headers_copy){
		oss << header.first << ": " << header.second << "\r\n";
	}

	// 4. Blank Line for seperation of Header and Body
	oss << "\r\n";
	
	// 5. Body
	oss << _body;

	return (oss.str());
}