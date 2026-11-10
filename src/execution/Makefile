# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cpohl <cpohl@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/10 15:10:37 by cpohl             #+#    #+#              #
#    Updated: 2026/11/10 15:10:37 by cpohl            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv
CXXFLAGS = -Wall -Werror -Wextra -std=c++17
SRCS = main.cpp Server.cpp Socket.cpp Request.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	c++ $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	c++ $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re