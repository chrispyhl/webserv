/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 15:12:05 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/19 16:25:12 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "incs.hpp"
#include "Socket.hpp"

int main(){
	Socket a;
	a.listen_on(8080);
	std::cout << YELLOW << "8080..." << RESET << std::endl;

	sockaddr_in	client_address{};
	socklen_t	addr_len = sizeof(client_address);
	// Stop until client connection
	int	fd = a.get_fd();
	int	client_fd = accept(fd, reinterpret_cast<struct sockaddr*>(&client_address), &addr_len);
	if (client_fd < 0){
		std::cerr << RED << "Accept failed!" << RESET <<std::endl;
		return 1;
	}
	Socket client_socket(client_fd);

	std::cout << GREEN << "Client connection success!" << RESET << std::endl;

	// 6. Send data to client
	const char	*message = "Hi there! --from the Server!\n";
	send(client_fd, message, strlen(message), 0);


	// 7. Receive data from client
	char	buffer[1024] = {0};
	ssize_t	bytes_received;

	while ((bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0){
		buffer[bytes_received] = '\0';
		std::cout << BLUE << "Client says:\n" << MAGENTA << buffer << std::endl;
		
		// Option: Send response to client
		const char	*response = "Message received!\n";
		ssize_t output = send(client_fd, response, strlen(response), 0);
		if (output < 0){
			std::cerr << RED << "Response crashed!" << RESET << std::endl;
			return 1;
		}
		if (output < (unsigned int)strlen(response))
			std::cerr << RED << "Response wasn't fully sent!" << RESET << std::endl;
		memset(buffer, 0, sizeof(buffer));
	}
	
	if (bytes_received == 0)
		std::cout << BLACK << "Client disconnected." << RESET << std::endl;
	else if (bytes_received < 0)
		std::cerr << RED << "Error while receiving data!" << RESET << std::endl;

	return 0;
}