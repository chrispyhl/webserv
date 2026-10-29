/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:26:32 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/29 15:18:02 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port){
	_listener.listen_on(port);
	_fd = _listener.get_fd();

	pollfd	server_pollfd{};
	server_pollfd.fd = _fd;
	server_pollfd.events = POLLIN;
	poll_fds.push_back(server_pollfd);
}


void	Server::poll_loop(void){

	while (true){
		int	ret = poll(poll_fds.data(), poll_fds.size(), -1);
		if (ret < 0){
			if (errno == EINTR){
				std::cout << YELLOW << "Server interupted by signal. Shutting down..." << RESET << std::endl;
				break;
			}
			std::cerr << RED << "poll failed!" << RESET << std::endl;
			throw std::runtime_error("poll failed!");
		}
		std::vector<int>	fds_to_remove;
		for (size_t i = 0; i < poll_fds.size(); ++i){

			if (poll_fds[i].revents & POLLHUP || poll_fds[i].revents & POLLERR){
				fds_to_remove.push_back(poll_fds[i].fd);
				std::cerr << RED << "poll mistake!" << RESET << std::endl;
				continue;
			}

			if (poll_fds[i].fd == _fd){
				if (poll_fds[i].revents & POLLIN){
					int	client_fd = accept(_fd, NULL, NULL);
				if (client_fd < 0){
					std::cerr << RED << "accept failed!" << RESET << std::endl;
					continue;
				}
				clients.emplace(client_fd, Client(client_fd));

				pollfd	client_pollfd{};
				client_pollfd.fd = client_fd;
				client_pollfd.events = POLLIN;
				poll_fds.push_back(client_pollfd);
				}
			}

			else{
					int		client_fd = poll_fds[i].fd;

					if (poll_fds[i].revents & POLLIN){
						char	buffer[1024];
						ssize_t	bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
						
						if (bytes_received > 0){
							buffer[bytes_received] = '\0';
							std::cout << BLUE << "Client says: " << MAGENTA << buffer << RESET << std::endl;
							clients.at(client_fd).send_buffer.append(buffer, bytes_received);
							poll_fds[i].events |= POLLOUT;
						}
					else {
						fds_to_remove.push_back(client_fd);
						continue;
					}
				}

					if (poll_fds[i].revents & POLLOUT){
						Client &client = clients.at(client_fd);
						if (!client.send_buffer.empty()){
							ssize_t bytes_sent = send(client_fd, client.send_buffer.data(), client.send_buffer.size(), 0);
							if (bytes_sent > 0){
								client.send_buffer.erase(0, bytes_sent);
							}
							else if (bytes_sent < 0){
								std::cerr << RED << "send failed!" << RESET << std::endl;
								fds_to_remove.push_back(client_fd);
								continue;
							}
						}
						if (client.send_buffer.empty()){
							poll_fds[i].events &= ~POLLOUT;
						}
					}
				}
		}
		for (int fd : fds_to_remove){
			clients.erase(fd);
			for (auto it = poll_fds.begin(); it != poll_fds.end(); ++it){
				if (it->fd == fd){
					poll_fds.erase(it);
					break;
				}
			}
		}
	}
}
