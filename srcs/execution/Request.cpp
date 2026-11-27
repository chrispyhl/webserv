/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/10 13:44:33 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/27 10:59:13 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

HttpRequest::HttpRequest() : _state(STATE_REQUEST_LINE), _content_length(0), _is_chunked(false), _chunk_remaining(0), _chunk_state(CHUNK_SIZE), _max_body_size(0), _error_code(400), _header_bytes(0) {}

HttpRequest::~HttpRequest() {}

ParseState HttpRequest::get_state() const { return _state; }
size_t	HttpRequest::get_content_length() const { return _content_length; }
const std::string& HttpRequest::get_method() const { return _method; }
const std::string& HttpRequest::get_path() const { return _path; }
const std::string& HttpRequest::get_raw_path() const { return _raw_path; }
const std::string& HttpRequest::get_query() const { return _query; }
const std::string& HttpRequest::get_version() const { return _version; }
const std::string& HttpRequest::get_body() const { return _body; }
const std::map<std::string, std::string>& HttpRequest::get_headers() const { return _headers; }
const std::string& HttpRequest::get_header(const std::string& key) const {
	static const std::string empty = "";
	std::string lower_key;
	lower_key = tolower_string(key);

	auto it = _headers.find(lower_key);
	if (it != _headers.end())
		return it->second;
	return empty;
}

void	HttpRequest::set_max_body_size(size_t max_size){
	_max_body_size = max_size;
}

void	HttpRequest::start_body(){
	_state = STATE_BODY;
}

int	HttpRequest::get_error_code() const{ return _error_code; }

void HttpRequest::clear(){
	_method.clear();
	_path.clear();
	_raw_path.clear();
	_query.clear();
	_version.clear();
	_headers.clear();
	_body.clear();
	_state = STATE_REQUEST_LINE;
	_content_length = 0;
	_is_chunked = false;
	_chunk_state = CHUNK_SIZE;
	_chunk_remaining = 0;
	_max_body_size = 0;
	_error_code = 400;
	_header_bytes = 0;
}

static bool	normalize_path(const std::string &raw, std::string &out){
	size_t		i = 0;
	std::string	decoded;

	while (i < raw.size()){
		if (raw[i] == '%'){
			if (i + 2 >= raw.size() || !std::isxdigit(static_cast<unsigned char>(raw[i + 1])) || !std::isxdigit(static_cast<unsigned char>(raw[i + 2]))) // if we find %, check if there even are 2 more chars
				return false;
			std::string	hex = raw.substr(i + 1, 2);
			unsigned long	byte_value = std::stoul(hex, nullptr, 16); // converts to base-16-hex
			if (byte_value == 0) // catches %00 (0-byte)
				return false;
			decoded.push_back(static_cast<char>(byte_value));
			i += 3; // skips over %XX
		}
		else{
			decoded.push_back(raw[i]);
			i += 1;
		}
	}
	std::vector<std::string>	segments;
	std::istringstream			iss(decoded);
	std::string					seg;

	while (std::getline(iss, seg, '/')){ // cuts at / into segments
		if (seg.empty() || seg == ".") // empty segments or '.'(current) gets skipped
			continue;
		if (seg == ".."){
			if (segments.empty())
				return false;
			segments.pop_back(); // removes last entered segment
			continue;
		}
		segments.push_back(seg);
	}
	if (segments.empty()){ // if e.g. /a/b and pop_back removed, we never go further then /
		out = "/";
		return true;
	}
	out.clear();
	for (const std::string &part : segments) // every segment gets added into out with '/' in between
		out += "/" + part;
	return true;
}

// PARSE HELPERS

bool HttpRequest::parse_request_line(const std::string& line){
	if (line.size() > 8192){ // 8192 ist just a example value (8KiB)
		_error_code = 414;
		return false;
	}

	std::istringstream stream(line);
	std::string	raw_uri;
	std::string	extra;
	
	if (!(stream >> _method >> raw_uri >> _version) || (stream >> extra)){
		_error_code = 400;
		return false;
	}
	if (raw_uri.size() > 8192){
		_error_code = 414;
		return false;
	}
	if (_version != "HTTP/1.0" && _version != "HTTP/1.1" && _version.rfind("HTTP/", 0) == 0){ // starting from pos 0 check for HTTP/
		_error_code = 505;
		return false;
	}
	size_t pos = raw_uri.find('?');
	if (pos != std::string::npos){
		_raw_path = raw_uri.substr(0, pos);
		_query = raw_uri.substr(pos + 1);
	}
	else{
		_raw_path = raw_uri;
		_query.clear();
	}
	if (_raw_path.empty() || _raw_path[0] != '/'){
		_error_code = 400;
		return false;
	}
	if (!normalize_path(_raw_path, _path)){
		_error_code = 400;
		return false;
	}
	return true;
}

bool HttpRequest::parse_header_line(const std::string& line){
	size_t colon = line.find(':');
	if (colon == 0 || colon == std::string::npos)
		return false;

	std::string key = line.substr(0, colon);
	std::string value = line.substr(colon + 1);

	size_t key_start = key.find_first_not_of(" \t");
	size_t key_end = key.find_last_not_of(" \t");
	if (key_start != std::string::npos && key_end != std::string::npos)
		key = key.substr(key_start, key_end - key_start + 1);
	else
		key.clear();
	size_t	value_start = value.find_first_not_of(" \t");
	size_t	value_end = value.find_last_not_of(" \t");
	if (value_start != std::string::npos && value_end != std::string::npos)
		value = value.substr(value_start, value_end - value_start + 1);
	else
		value.clear();

	key = tolower_string(key);

	if (key == "transfer-encoding"){
		if (_headers.find("content-length") != _headers.end() || value.empty()){
			_error_code = 400;
			_state = STATE_ERROR;
			return false;
		}
		std::istringstream	stream(tolower_string(value));
		std::string			token;
		while(std::getline(stream, token, ',')){
			size_t	start = token.find_first_not_of(" \t");
			size_t	end = token.find_last_not_of(" \t");
			if (start != std::string::npos && end != std::string::npos)
				token = token.substr(start, end - start + 1);
			else
				token.clear();

			if (token.empty()){
				_error_code = 400;
				_state = STATE_ERROR;
				return false;
			}
			else if (token == "chunked")
				_is_chunked = true;
			else{
				_error_code = 501;
				_state = STATE_ERROR;
				return false;
			}
		}
	}

	if (key == "content-length"){
		if (_headers.find("transfer-encoding") != _headers.end() || value.empty() || value.find('-') != std::string::npos || value.find_first_not_of("0123456789") != std::string::npos)
			return false;
		size_t	parsed_length = 0;
		try{
			parsed_length = std::stoul(value);
		}
		catch (const std::exception&){
			return false;
		}
		if (_headers.find("content-length") != _headers.end() && parsed_length != _content_length)
			return false;
		_content_length = parsed_length;
	}
	_headers[key] = value;
	return true;
}

void	HttpRequest::parse_body(std::string &read_buffer){
	if (!_is_chunked && _content_length > _max_body_size){
		_error_code = 413;
		_state = STATE_ERROR;
		return;
	}
	if (_is_chunked == true){
		parse_chunked_body(read_buffer);
		return;
	}
	size_t	needed = _content_length - _body.size();
	size_t	available = read_buffer.size();
	size_t	to_copy = std::min(needed, available);

	_body.append(read_buffer, 0, to_copy);
	read_buffer.erase(0, to_copy);

	if (_body.size() >= _content_length)
		_state = STATE_COMPLETE;
}

void	HttpRequest::parse_chunked_body(std::string &read_buffer){
	while (_state == STATE_BODY){
		if (_chunk_state == CHUNK_SIZE){
			size_t	pos = read_buffer.find("\r\n");
			if (pos == std::string::npos){
				if (read_buffer.size() > 8192){
					_state = STATE_ERROR;
					_error_code = 400;
					return;
				}
				else
					return;
			}
			else{
				std::string chunk_sized_string;
				chunk_sized_string.append(read_buffer, 0, pos);
				read_buffer.erase(0, pos + 2);
				size_t	semi = chunk_sized_string.find(";");
				if (semi != std::string::npos)
					chunk_sized_string = chunk_sized_string.substr(0, semi);
				if (chunk_sized_string.empty() || chunk_sized_string.find('-') != std::string::npos || chunk_sized_string.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos){
					_state = STATE_ERROR;
					_error_code = 400;
					return;
				}
				size_t	i_chunk_size;
				try{
					i_chunk_size = std::stoul(chunk_sized_string, nullptr, 16);
				}
				catch (const std::exception&){
					_state = STATE_ERROR;
					_error_code = 400;
					return;
				}
				if (_body.size() > _max_body_size){
					_error_code = 413;
					_state = STATE_ERROR;
					return;
				}
				else if (i_chunk_size > _max_body_size - _body.size()){
					_error_code = 413;
					_state = STATE_ERROR;
					return;
				}
				if (i_chunk_size == 0)
					_chunk_state = CHUNK_TRAILER;
				else{
					_chunk_remaining = i_chunk_size;
					_chunk_state = CHUNK_DATA;
				}
			}
		}
		else if (_chunk_state == CHUNK_DATA){
			if (read_buffer.empty())
				return;
			else{
				size_t	mini;
				mini = std::min(_chunk_remaining, read_buffer.size());
				if (_body.size() >= _max_body_size || mini > _max_body_size - _body.size()){
					_error_code = 413;
					_state = STATE_ERROR;
					return;
				}
				_body.append(read_buffer, 0, mini);
				read_buffer.erase(0, mini);
				_chunk_remaining -= mini;
				if (_chunk_remaining == 0)
					_chunk_state = CHUNK_DATA_CRLF;
			}
		}
		else if (_chunk_state == CHUNK_DATA_CRLF){
			if (read_buffer.size() < 2)
				return;
			else if (read_buffer.substr(0, 2) == "\r\n"){
				read_buffer.erase(0, 2);
				_chunk_state = CHUNK_SIZE;
			}
			else{
				_state = STATE_ERROR;
				_error_code = 400;
				return;
			}
		}
		else if (_chunk_state == CHUNK_TRAILER){
			size_t	pos = read_buffer.find("\r\n");
			if (pos == std::string::npos){
				if (read_buffer.size() > 8192){
					_state = STATE_ERROR;
					_error_code = 400;
					return;
				}
				else
					return;
			}
			else{
				if (pos == 0){
					read_buffer.erase(0, pos + 2);
					_state = STATE_COMPLETE;
					return;
				}
				else if (pos > 0)
					read_buffer.erase(0, pos + 2);
			}
		}
	}
}

// MAIN PARSE

ParseState HttpRequest::parse(std::string &read_buffer) {
	while (_state != STATE_COMPLETE && _state != STATE_ERROR){
		if (_state == STATE_REQUEST_LINE || _state == STATE_HEADERS){
			size_t pos = read_buffer.find("\r\n");
			if (pos == std::string::npos){
				if (_state == STATE_REQUEST_LINE && read_buffer.size() > 8192){
					_error_code = 414;
					_state = STATE_ERROR;
					break;
				}
				else if (_state == STATE_HEADERS && _header_bytes + read_buffer.size() > 8192){
					_error_code = 431;
					_state = STATE_ERROR;
					break;
				}
				else
					break; // Wait for more data
			}
			std::string line = read_buffer.substr(0, pos);
			read_buffer.erase(0, pos + 2); // (+ 2 for \r\n)

			if (_state == STATE_REQUEST_LINE) {
				if (line.empty()) continue; // Ignore empty lines before the request line
				if (!parse_request_line(line)) {
					_state = STATE_ERROR;
					break;
				}
				_state = STATE_HEADERS;
			}
			else if (_state == STATE_HEADERS) {
				if (line.empty()) { // End of headers
					if (_content_length > 0 || _is_chunked == true){
						_state = STATE_HEADERS_COMPLETE;
						break;
					}
					else
						_state = STATE_COMPLETE;
				}
				else {
					_header_bytes += line.size() + 2;
					if (_header_bytes > 8192){
						_error_code = 431;
						_state = STATE_ERROR;
						break;
					}
					if (!parse_header_line(line)) {
						_state = STATE_ERROR;
						break;
					}
				}
			}
		}
		else if (_state == STATE_HEADERS_COMPLETE){
			break;
		}
		else if (_state == STATE_BODY) {
			parse_body(read_buffer);
			if (_state != STATE_COMPLETE)
				break; // Wait for more data
		}
	}
	return _state;
}
