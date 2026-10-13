/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_test.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/12 14:18:21 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/13 16:10:00 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "incs.hpp"
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>

int main(){
	// 1. Create Socket
	int	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0){
		std::cerr << RED << "Socket-creation failed!" << RESET << std::endl;
		return 1;
	}
	std::cout << GREEN << "Socket creation success. Descriptor: " << server_fd << RESET << std::endl;

	// Option: Make port reusable to avoid "Address already in use"- fail
	int	opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		std::cerr << RED << "setsocket failed!" << RESET << std::endl;
		return 1;
	}

	// 2. Define Address
	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(8080);

	// 3. Call bind
	if (bind(server_fd, reinterpret_cast<const struct sockaddr*>(&address), sizeof(address)) < 0){
		std::cerr << RED << "Bind failed!" << RESET << std::endl;
		close(server_fd);
		return 1;
	}

	// 4. Switch to listen-mode (Queue for max. 3 Connections)
	if (listen(server_fd, 3) < 0){
		std::cerr << RED << "Listen failed!" << RESET << std::endl;
		close(server_fd);
		return 1;
	}
	
	std::cout << YELLOW << "Server waiting on port 8080..." << RESET << std::endl;
	
	// 5. Client address to accept connections
	sockaddr_in	client_address{};
	socklen_t	addr_len = sizeof(client_address);
	// Stop until client connection
	int	client_socket = accept(server_fd, (struct sockaddr*)&client_address, &addr_len);

	if (client_socket < 0){
		std::cerr << RED << "Accept failed!" << RESET <<std::endl;
		close(server_fd);
		return 1;
	}

	std::cout << GREEN << "Client connection success!" << RESET << std::endl;

	// 6. Send data to client
	const char	*message = "Hi there! --from the Server!\n";
	send(client_socket, message, strlen(message), 0);


	// 7. Receive data from client
	char	buffer[1024] = {0};
	ssize_t	bytes_received;

	while ((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0){
		buffer[bytes_received] = '\0';
		std::cout << BLUE << "Client says:\n" << MAGENTA << buffer << std::endl;
		
		// Option: Send response to client
		const char	*response = "Message received!\n";
		ssize_t output = send(client_socket, response, strlen(response), 0);
		if (sizeof(output) < sizeof(response))
			std::cerr << RED << "Response wasn't fully sent!" << RESET << std::endl;
		memset(buffer, 0, sizeof(buffer));
	}
	
	if (bytes_received == 0)
		std::cout << BLACK << "Client disconnected." << RESET << std::endl;
	else if (bytes_received < 0)
		std::cerr << RED << "Error while receiving data!" << RESET << std::endl;


	// 8. Close sockets
	close(client_socket);
	close(server_fd);
	return 0;
}