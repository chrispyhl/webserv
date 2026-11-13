/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 14:34:40 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/20 17:32:39 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

#include "incs.hpp"

class Socket {
	private:
		int					_fd;
	public:
		Socket();
		explicit Socket(int fd);
		Socket(const Socket&) = delete;
		Socket &operator=(const Socket&) = delete;
		Socket(Socket &&other) noexcept;
		~Socket();

		int		get_fd(void);
		void	listen_on(int port);
		void	set_nonblocking(void);
};

#endif
