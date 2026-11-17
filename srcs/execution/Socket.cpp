/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chris <chris@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 14:39:13 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/16 14:28:58 by chris            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(){
	// 1. Socket creation
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0){
		std::cerr << RED << "Socket-creation failed!" << RESET << std::endl;
		throw std::runtime_error("Socket-creation failed!");
	}
	std::cout << GREEN << "Socket creation success. Descriptor: " << _fd << RESET << std::endl;
	set_nonblocking();
}

Socket::Socket(int fd){
	_fd = fd;
	set_nonblocking();
}

Socket::Socket(Socket &&other) noexcept {
	_fd = other._fd;
	other._fd = -1;
}

Socket::~Socket(){
	if (_fd >= 0)
		close(_fd);
}

int	Socket::get_fd(){
	return (_fd);
}

void	Socket::listen_on(std::uint32_t address, int port){
	// Option: Make port reusable to avoid "Address already in use"- fail
	int	opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		std::cerr << RED << "setsocket failed!" << RESET << std::endl;
		throw std::runtime_error("setsocket failed!");
	}
	// 2. Define Address
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(address);
	addr.sin_port = htons(port);

	// 3. Call bind
	if (bind(_fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)) < 0){
		std::cerr << RED << "Bind failed!" << RESET << std::endl;
		throw std::runtime_error("Bind failed!");
	}

	// 4. Switch to listen-mode (Queue for max. X Connections)
	if (listen(_fd, 50) < 0){
		std::cerr << RED << "Listen failed!" << RESET << std::endl;
		throw std::runtime_error("Listen failed!");
	}
	std::cout << YELLOW << "Server a waiting on port: " << ORANGE << port << RESET << std::endl;
}

void	Socket::set_nonblocking(void){
	int	ret;
	// int	flags;
	// flags = fcntl(_fd, F_GETFL, 0);
	// if (flags < 0){
	// 	std::cerr << RED << "Flag retrieval failed!" << RESET << std::endl;
	// 	throw std::runtime_error("Flag retrieval failed!");
	// }
	ret = fcntl(_fd, F_SETFL, O_NONBLOCK);
	if (ret < 0){
		std::cerr << RED << "Non-blocking failed!" << RESET << std::endl;
		throw std::runtime_error("Non-blocking failed!");
	}
	
}
