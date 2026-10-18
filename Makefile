NAME = webserv

CXX = c++
CXXFLAGS = -Werror -Wall -Wextra -std=c++17 $(addprefix -I, $(INC_DIRS))
RM = rm -rf

SRC_DIR = srcs
OBJ_DIR = obj

# one directory per module
CONF_DIR = $(SRC_DIR)/config

# every module's include folder; turned into -I flags in CXXFLAGS
INC_DIRS =	$(CONF_DIR)/inc

SRC =	$(SRC_DIR)/main.cpp \
		$(CONF_DIR)/parse_config.cpp \
		$(CONF_DIR)/Tokenizer.cpp \
		$(CONF_DIR)/Token.cpp \
		$(CONF_DIR)/Config.cpp \
		$(CONF_DIR)/ConfigError.cpp

OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "get webserv'd"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	@./webserv

clean:
	@$(RM) $(OBJ_DIR)
	@echo "Cleaned up objects."

fclean: clean
	@$(RM) $(NAME)
	@echo "fool"

re: fclean all

.PHONY: all clean fclean re
