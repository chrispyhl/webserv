/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craabe <craabe@42>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:26:32 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/13 15:54:25 by craabe           ###   ########.fr       */
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

void	Server::accept_client(){
	int	client_fd = accept(_fd, NULL, NULL);
	if (client_fd < 0){
		std::cerr << RED << "accept failed!" << RESET << std::endl;
		return;
	}
	clients.emplace(client_fd, Client(client_fd));

	pollfd	client_pollfd{};
	client_pollfd.fd = client_fd;
	client_pollfd.events = POLLIN;
	poll_fds.push_back(client_pollfd);
}

void	Server::receive_data(size_t i, std::vector<int>& fds_to_remove){
	int		client_fd = poll_fds[i].fd;
	char	buffer[1024];
	ssize_t	bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	
	if (bytes_received > 0){
		buffer[bytes_received] = '\0';
		Client &client = clients.at(client_fd);

		client.read_buffer.append(buffer, bytes_received);

		ParseState state = client.request.parse(client.read_buffer);

		if (state == STATE_COMPLETE){
			std::cout <<  GREEN << "Request complete! Method: " << client.request.get_method() 
			<< " Path: " << client.request.get_path() << RESET << std::endl;

			std::string response = "HTTP/1.0 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!"; // TEMP RESPONSE
			client.send_buffer.append(response);
			poll_fds[i].events |= POLLOUT;

			client.request.clear();
		}
		else if (state == STATE_ERROR){
			std::cerr << RED << "HTTP Parse Error! Closing connection." << RESET << std::endl;
			std::string	bad_request = "HTTP/1.0 400 Bad Request\r\nContent-Length: 11\r\n\r\nBad Request";
			client.send_buffer.append(bad_request);
			poll_fds[i].events |= POLLOUT;
			client.should_close = true;
		}
	}
	else
		fds_to_remove.push_back(client_fd);
}

void	Server::send_data(size_t i, std::vector<int>& fds_to_remove){
	int		client_fd = poll_fds[i].fd;
	Client &client = clients.at(client_fd);

	if (!client.send_buffer.empty()){
		ssize_t bytes_sent = send(client_fd, client.send_buffer.data(), client.send_buffer.size(), 0);
		if (bytes_sent > 0){
			client.send_buffer.erase(0, bytes_sent);
		}
		else if (bytes_sent < 0){
			std::cerr << RED << "send failed!" << RESET << std::endl;
			fds_to_remove.push_back(client_fd);
			return;
		}
	}
	if (client.send_buffer.empty()){
		poll_fds[i].events &= ~POLLOUT;
		if (client.should_close){
			fds_to_remove.push_back(client_fd);
		}
	}
}

void	Server::cleanup_fds(std::vector<int>& fds_to_remove){
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

void	Server::poll_loop(void){
	while (!g_interrupted){
		int	ret = poll(poll_fds.data(), poll_fds.size(), -1);
		if (ret < 0){
			if (errno == EINTR || g_interrupted){
				std::cout << YELLOW << "\nServer interupted by signal. Shutting down..." << RESET << std::endl;
				break;
			}
			std::cerr << RED << "poll failed!" << RESET << std::endl;
			throw std::runtime_error("poll failed!");
		}
		std::vector<int>	fds_to_remove;
		for (size_t i = 0; i < poll_fds.size(); ++i){

			if (poll_fds[i].revents & POLLHUP || poll_fds[i].revents & POLLERR){
				if (poll_fds[i].fd == _fd){
					std::cerr << RED << "Error on listener socket!" << RESET << std::endl;
					continue;
				}
				fds_to_remove.push_back(poll_fds[i].fd);
				std::cerr << RED << "poll mistake on client FD: " << poll_fds[i].fd << "!" << RESET << std::endl;
				continue;
			}
			if (poll_fds[i].fd == _fd){
				if (poll_fds[i].revents & POLLIN){
					accept_client();
				}
			}
			else{
				if (poll_fds[i].revents & POLLIN){
					receive_data(i, fds_to_remove);
				}
				if (poll_fds[i].revents & POLLOUT){
					send_data(i, fds_to_remove);
				}
			}
		}
		cleanup_fds(fds_to_remove);
	}
}
