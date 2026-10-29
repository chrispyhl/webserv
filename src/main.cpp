/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 15:12:05 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/28 14:29:39 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "incs.hpp"
#include "Socket.hpp"
#include "Server.hpp"

int main(){
	try{
		Server	server(8080);

		std::cout << BLUE << 8080 << RESET << std::endl;
		
		server.poll_loop();
	}
	catch (const std::exception &e){
		std::cerr << RED << "Failed: " << e.what() << RESET << std::endl;

		return	EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}