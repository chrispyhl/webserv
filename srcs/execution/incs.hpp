/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   incs.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/13 14:28:56 by cpohl             #+#    #+#             */
/*   Updated: 2026/11/13 12:29:51 by cpohl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INCS_HPP
# define INCS_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
// Socket
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>

#include <utility>
#include <fcntl.h>

#include <poll.h>
#include <vector>
#include <map>
#include <stdexcept>

#include <csignal>
#include <cerrno>

#include <algorithm>

#define RESET "\033[0m"
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define WHITE "\033[37m"
#define CONSTRU "\033[3m"
#define ORANGE "\033[38;5;208m"

extern volatile sig_atomic_t g_interrupted;

#endif