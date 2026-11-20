/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 14:34:40 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/20 16:14:15 by cpohl            ###   ########.fr       */
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
		void	listen_on(std::uint32_t address, int port);
		void	set_nonblocking(void);
};

#endif
