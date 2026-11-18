/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/18 15:43:00 by chris             #+#    #+#             */
/*   Updated: 2026/11/18 16:28:18 by chris            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "incs.hpp"

class Response{
	private:
			std::size_t							_status_code;
			std::string							_reason_phrase;
			std::map<std::string, std::string>	_headers;
			std::string							_body;
	public:
			Response();
			~Response();

			void	set_status(std::size_t code, const std::string &reason);
			void	set_header(const std::string &key, const std::string &value);
			void	set_body(const std::string &body);
			void	set_content_type(const std::string &path);

			std::string	build() const;
};

#endif
