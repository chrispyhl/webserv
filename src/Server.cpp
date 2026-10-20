/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:26:32 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/20 17:35:59 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(){
	Socket	socket;
	_fd = socket.get_fd();
	
}


void	Server::poll_loop(void){
	std::vector<pollfd>	poll_fds;

	pollfd	server_pollfd{};
	server_pollfd.fd = _fd;
	server_pollfd.events = POLLIN;

	poll_fds.push_back(server_pollfd);
	while (true){
		int	ret = poll(poll_fds.data(), poll_fds.size(), -1);
		if (ret < 0){
			std::cerr << RED << "poll failed!" << RESET << std::endl;
			throw std::runtime_error("poll failed!");
		}
		for (size_t i = 0; i < poll_fds.size(); ++i){
			if (poll_fds[i].revents & POLLHUP || poll_fds[i].revents & POLLERR){
				std::cerr << RED << "poll mistake!" << RESET << std::endl;
			}
			if (poll_fds[i].revents & POLLIN){
				if (poll_fds[i].fd == _fd){
					int	client_fd = accept(_fd, NULL, NULL);
					if (client_fd < 0){
						std::cerr << RED << "accept failed!" << RESET << std::endl;
						continue;
					}
					pollfd	client_pollfd{};
					client_pollfd.fd = client_fd;
					client_pollfd.events = POLLIN;

					poll_fds.push_back(client_pollfd);
				}
				else{
					char	buffer[1024];
					ssize_t	bytes_received = recv(poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
					
					if (bytes_received > 0){
						buffer[bytes_received] = '\0';
						std::cout << BLUE << "Client says: " << MAGENTA << buffer << RESET << std::endl;
					}
					else if (bytes_received == 0){
						close (poll_fds[i].fd);
						poll_fds.erase(poll_fds.begin() + i);
						--i;
					}
					else{
						std::cerr << RED << "recv failed!" << RESET << std::endl;
						close(poll_fds[i].fd);
						poll_fds.erase(poll_fds.begin() + i);
						--i;
					}
				}
			}
		}
	}
}
