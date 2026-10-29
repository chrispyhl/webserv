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

struct Client {
	Socket		socket;
	std::string	send_buffer;

	explicit Client(int fd) : socket(fd), send_buffer("") {}
};

#endif