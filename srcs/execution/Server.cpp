/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:26:32 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/23 14:43:25 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "route_request.hpp"
#include "error_response.hpp"

void	Server::mark_for_removal(int fd, std::vector<int>& fds_to_remove){
	if(!vector_contains(fds_to_remove, fd))
		fds_to_remove.push_back(fd);
}

Server::Server(const Config &config) : _config(config) {
	std::vector<ListenAddress> addresses = _config.listenAddresses();

	for (const ListenAddress &la : addresses) {
		Socket	listener_socket;
		listener_socket.listen_on(la.address, la.port);
		
		int	fd = listener_socket.get_fd();
		_listeners.push_back(std::move(listener_socket));
		_listener_addresses[fd] = la;
		
		pollfd	pfd{};
		pfd.fd = fd;
		pfd.events = POLLIN;
		poll_fds.push_back(pfd);

		std::cout << YELLOW << "Listening on: " << address_to_string(la) << ":" << la.port << RESET << std::endl;
	}
}

void	Server::accept_client(int listener_fd){
	int	client_fd = accept(listener_fd, NULL, NULL);
	if (client_fd < 0){
		std::cerr << RED << "accept failed!" << RESET << std::endl;
		return;
	}
	ListenAddress la = _listener_addresses.at(listener_fd);
	
	clients.emplace(client_fd, Client(client_fd, la));

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

		while(true){
			ParseState state = client.request.parse(client.read_buffer);
			
			if (state == STATE_COMPLETE){
				std::cout <<  GREEN << "Request complete! Method: " << client.request.get_method() 
				<< " Path: " << client.request.get_path() << RESET << std::endl;


				// Response
				Response	res = route_request(client.request, client.listen_address, _config);

				std::cout << GREEN << client.request.get_method() << " " << client.request.get_path() << " -> " 
				<< res.statusCode() << " " << get_status_text(res.statusCode()) << RESET << std::endl;

				// As HTTP/1.0 Client should close after response
				res.setCloseConnection(true); // Using response to tell the Client

				client.send_buffer.append(res.toString());
				poll_fds[i].events |= POLLOUT;

				client.should_close = true; // Needed for my send_data loop -> Server has to close the Socket using 'mark_for_removal'

				client.request.clear();
				poll_fds[i].events &= ~POLLIN;
				poll_fds[i].events |= POLLOUT;
				break;
			}
			else if (state == STATE_ERROR){
				std::cerr << RED << "HTTP Parse Error! Closing connection." << RESET << std::endl;
				Response	res_err = get_error_response(client.request.get_error_code());
				res_err.setCloseConnection(true);
				client.send_buffer.append(res_err.toString());
				poll_fds[i].events |= POLLOUT;
				client.should_close = true;
				client.request.clear();
				poll_fds[i].events &= ~POLLIN;
				poll_fds[i].events |= POLLOUT;
				break;
			}
			else if (state == STATE_HEADERS_COMPLETE){
				const ServerConfig &server = _config.selectServer(client.listen_address, client.request.get_header("Host"));
				const LocationConfig &location = server.findLocation(client.request.get_path());
				size_t	max_body_size = location.maxBodySize();

				client.request.set_max_body_size(max_body_size);
				client.request.start_body();
			}
			else
				break;
		}
	}
	// Client disconnected
	else if (bytes_received == 0)
		mark_for_removal(client_fd, fds_to_remove);
	else{
		std::cout << RED << "recv failed! Closing connection. " << RESET << std::endl;
		mark_for_removal(client_fd, fds_to_remove);
	}
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
			mark_for_removal(client_fd, fds_to_remove);
			return;
		}
	}
	if (client.send_buffer.empty()){
		poll_fds[i].events &= ~POLLOUT;
		if (client.should_close){
			mark_for_removal(client_fd, fds_to_remove);
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
			int	current_fd = poll_fds[i].fd;

			// Check if fd is part of listener
			auto	it = std::find_if(_listeners.begin(), _listeners.end(), [current_fd](Socket &s){
				return s.get_fd() == current_fd;
			});
			if (it != _listeners.end()){
				if (poll_fds[i].revents & POLLIN)
					accept_client(current_fd);
			}
			// Normal CLient
			else{
				if (poll_fds[i].revents & POLLIN)
					receive_data(i, fds_to_remove);
				if (poll_fds[i].revents & POLLOUT)
					send_data(i, fds_to_remove);
			}

			if (poll_fds[i].revents & POLLHUP || poll_fds[i].revents & POLLERR){
				mark_for_removal(current_fd, fds_to_remove);
				std::cerr << RED << "Poll error on fd: " << current_fd << RESET << std::endl;
				continue;
			}
		}
		cleanup_fds(fds_to_remove);
	}
}
