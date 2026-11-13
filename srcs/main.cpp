#include "Server.hpp"
#include "Socket.hpp"
#include "incs.hpp"
#include "webserv.hpp"

#include <iostream>
#include <stdexcept>

volatile sig_atomic_t g_interrupted = 0;

void handle_sigint(int sig)
{
	(void)sig;
	g_interrupted = 1;
}

int main(int argc, char **argv)
{
	signal(SIGINT, handle_sigint);
	signal(SIGQUIT, handle_sigint);
	signal(SIGPIPE, SIG_IGN);
	try
	{
		Config config = parse_config(argc, argv);
		Server server(8080);

		server.poll_loop();
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;

		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
