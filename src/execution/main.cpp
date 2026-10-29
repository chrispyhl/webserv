/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 15:12:05 by cpohl             #+#    #+#             */
/*   Updated: 2026/10/29 15:48:51 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "incs.hpp"
#include "Socket.hpp"
#include "Server.hpp"

void	handle_sigint(int sig){
	(void)sig;
}

int main(){
	signal(SIGINT, handle_sigint);
	signal(SIGQUIT, handle_sigint);
	signal(SIGPIPE, SIG_IGN);
	try{
		Server	server(8080);

		server.poll_loop();
	}
	catch (const std::exception &e){
		std::cerr << RED << "Failed: " << e.what() << RESET << std::endl;

		return	EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
