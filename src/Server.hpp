/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:26:40 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/20 17:34:11 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "incs.hpp"
#include "Socket.hpp"

class Server {
	private:
		int	_fd;
		std::vector<pollfd>	poll_fds;
		std::map<int, Socket>	map;
	public:
		Server();
		void poll_loop(void);
};

#endif
