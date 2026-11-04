NAME = webserv

CXX = c++
CXXFLAGS = -Werror -Wall -Wextra -std=c++17 $(addprefix -I, $(INC_DIRS))
# -MMD: while compiling, also write a .d file next to each .o that lists the headers it includes
# -MP: add an empty rule per header, so a deleted or renamed header doesn't break the build
# --> this guarantees that header changes are considered in recompiles
DEPFLAGS = -MMD -MP
RM = rm -rf

SRC_DIR = srcs
UTIL_DIR = $(SRC_DIR)/utils
OBJ_DIR = obj

# one directory per module
CONF_DIR = $(SRC_DIR)/config

# every module's include folder; turned into -I flags in CXXFLAGS
INC_DIRS =	$(SRC_DIR)/inc $(UTIL_DIR)/inc $(CONF_DIR)/inc 

SRC =	$(SRC_DIR)/main.cpp \
		$(UTIL_DIR)/utils.cpp \
		$(CONF_DIR)/parse_config.cpp \
		$(CONF_DIR)/Tokenizer.cpp \
		$(CONF_DIR)/Token.cpp \
		$(CONF_DIR)/Config.cpp \
		$(CONF_DIR)/ConfigError.cpp \
		$(CONF_DIR)/Parser.cpp \
		$(CONF_DIR)/LocationConfig.cpp \
		$(CONF_DIR)/ServerConfig.cpp \
		$(CONF_DIR)/ConfigBuilder.cpp \
		$(CONF_DIR)/ListenAddress.cpp

OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))
DEP = $(OBJ:.o=.d)

all: $(NAME)

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "get webserv'd"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# read the header lists from the .d files, so changing a header recompiles every .o that uses it
# leading "-": no error while the .d files don't exist yet (first build, after clean)
-include $(DEP)

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
