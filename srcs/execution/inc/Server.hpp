/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:26:40 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/25 12:26:04 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "incs.hpp"
#include "Socket.hpp"
#include "Client.hpp"
#include "Config.hpp"

class Server {
	private:
		std::vector<pollfd>				poll_fds;
		std::map<int, Client>			clients;
		std::vector<Socket>				_listeners;
		Config							_config;
		std::map<int, ListenAddress>	_listener_addresses;		

		void	mark_for_removal(int fd, std::vector<int>& fds_to_remove);
		void	accept_client(int listener_fd);
		void	receive_data(size_t i, std::vector<int>& fds_to_remove);
		void	send_data(size_t i, std::vector<int>& fds_to_remove);
		void	cleanup_fds(std::vector<int>& fds_to_remove);
		void	check_timeouts(std::vector<int>& fds_to_remove);

	public:
		explicit	Server(const Config &config);
		void 		poll_loop(void);
};

#endif

