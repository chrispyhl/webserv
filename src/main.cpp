/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 15:12:05 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/20 17:33:13 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "incs.hpp"
#include "Socket.hpp"
#include "Server.hpp"

int main(){
	try{
		Server	server;
		Socket	socket;
		int		port;

		port = 8080;
		socket.listen_on(port);
		std::cout << BLUE << port << RESET << std::endl;
		
		server.poll_loop();
	}
	catch (const std::exception &e){
		std::cerr << RED << "Failed: " << e.what() << RESET << std::endl;

		return	EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}