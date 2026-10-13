/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client_test.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/13 15:08:04 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/13 16:04:34 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "incs.hpp"
#include <arpa/inet.h>

int main(){
	// 1. Socket creation
	int	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd < 0){
		std::cerr << RED << "Socket-creation failed!" << RESET << std::endl;
		return 1;
	}
	
	// 2. Define Server-address
	sockaddr_in	server_address{};
	server_address.sin_family	= AF_INET;
	server_address.sin_port = htons(8080);
	
	// Change IP from text to binary
	if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0){
		std::cerr << RED << "Invalid address / address not supported!" << RESET << std::endl;
		return 1;
	}

	// 3. Connect to server
	if (connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
		std::cerr << RED << "Connection to server failed!" << RESET << std::endl;
		return 1;
	}
	std::cout << GREEN << "Successfully connected to Server!" << RESET << std::endl;

	// 4. Send message to Server
	const char *message = "Hi Server! I'm a Client!";
	send(client_fd, message, strlen(message), 0);

	// 5. Receive message from Server
	char	buffer[1024] = {0};
	ssize_t	bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_received > 0)
		std::cout << BLUE << "Server responded:\n" << YELLOW << buffer << RESET << std::endl;

	// 6. Close socket
	close(client_fd);
	return 0;
}