/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/10 13:44:33 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/10 13:44:33 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

HttpRequest::HttpRequest() : _state(STATE_REQUEST_LINE), _content_length(0) {}

HttpRequest::~HttpRequest() {}

ParseState HttpRequest::get_state() const { return _state; }
const std::string& HttpRequest::get_method() const { return _method; }
const std::string& HttpRequest::get_path() const { return _path; }
const std::string& HttpRequest::get_query() const { return _query; }
const std::string& HttpRequest::get_version() const { return _version; }
const std::string& HttpRequest::get_body() const { return _body; }
const std::map<std::string, std::string>& HttpRequest::get_headers() const { return _headers; }
const std::string& HttpRequest::get_header(const std::string& key) const {
	static const std::string empty = "";
	std::string lower_key = key;
	std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);

	auto it = _headers.find(lower_key);
	if (it != _headers.end())
		return it->second;
	return empty;
}

void HttpRequest::clear(){
	_method.clear();
	_path.clear();
	_query.clear();
	_version.clear();
	_headers.clear();
	_body.clear();
	_state = STATE_REQUEST_LINE;
	_content_length = 0;
}

// PARSE HELPERS

bool HttpRequest::parse_request_line(const std::string& line){
	std::istringstream stream(line);
	std::string	raw_uri;
	std::string	extra;

	if (!(stream >> _method >> raw_uri >> _version) || (stream >> extra))
		return false;

	size_t pos = raw_uri.find('?');
	if (pos != std::string::npos){
		_path = raw_uri.substr(0, pos);
		_query = raw_uri.substr(pos + 1);
	}
	else{
		_path = raw_uri;
		_query.clear();
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

	std::transform(key.begin(), key.end(), key.begin(), ::tolower);

	size_t start = value.find_first_not_of(" \t");
	if (start != std::string::npos)
		value = value.substr(start);
	else
		value.clear();

	_headers[key] = value;

	if (key == "content-length"){

		if (value.empty() || value.find('-') != std::string::npos || value.find_first_not_of("0123456789") != std::string::npos)
			return false;
		try{
			_content_length = std::stoul(value);
		} 
		catch (const std::exception&){
			return false;
		}
	}
	return true;
}

void	HttpRequest::parse_body(std::string &read_buffer){
	size_t	needed = _content_length - _body.size();
	size_t	available = read_buffer.size();
	size_t	to_copy = std::min(needed, available);

	_body.append(read_buffer, 0, to_copy);
	read_buffer.erase(0, to_copy);

	if (_body.size() >= _content_length)
		_state = STATE_COMPLETE;
}

// MAIN PARSE

ParseState HttpRequest::parse(std::string &read_buffer) {
	while (_state != STATE_COMPLETE && _state != STATE_ERROR){
		if (_state == STATE_REQUEST_LINE || _state == STATE_HEADERS){
			size_t pos = read_buffer.find("\r\n");
			if (pos == std::string::npos)
				break; // Wait for more data
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
					if (_content_length > 0)
						_state = STATE_BODY;
					else
						_state = STATE_COMPLETE;
				}
				else {
					if (!parse_header_line(line)) {
						_state = STATE_ERROR;
						break;
					}
				}
			}
		}
		if (_state == STATE_BODY) {
			parse_body(read_buffer);
			if (_state != STATE_COMPLETE)
				break; // Wait for more data
		}
	}
	return _state;
}
