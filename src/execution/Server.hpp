/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:26:40 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/29 16:13:28 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "incs.hpp"
#include "Socket.hpp"
#include "Client.hpp"

class Server {
	private:
		int						_fd;
		std::vector<pollfd>		poll_fds;
		std::map<int, Client>	clients;
		Socket					_listener;
	public:
		explicit	Server(int port);
		void 		poll_loop(void);
};

#endif
