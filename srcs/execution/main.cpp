/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 15:12:05 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/10 15:14:54 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "incs.hpp"
#include "Socket.hpp"
#include "Server.hpp"

volatile sig_atomic_t g_interrupted = 0;

void	handle_sigint(int sig){
	(void)sig;
	g_interrupted = 1;
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
