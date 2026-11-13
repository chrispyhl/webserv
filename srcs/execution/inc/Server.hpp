/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:26:40 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/10 14:56:59 by cpohl            ###   ########.fr       */
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

		void	accept_client();
		void	receive_data(size_t i, std::vector<int>& fds_to_remove);
		void	send_data(size_t i, std::vector<int>& fds_to_remove);
		void	cleanup_fds(std::vector<int>& fds_to_remove);

	public:
		explicit	Server(int port);
		void 		poll_loop(void);
};

#endif
