/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/29 13:27:03 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/29 13:27:03 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Socket.hpp"
#include "Request.hpp"

struct Client {
	Socket		socket;
	std::string	send_buffer;
	std::string	read_buffer;
	HttpRequest	request;
	bool		should_close;

	explicit Client(int fd) : socket(fd), send_buffer(""), read_buffer(""), request(), should_close(false) {}
};

#endif