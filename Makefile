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
EXEC_DIR = $(SRC_DIR)/execution

# config module: one subdirectory per parsing stage, plus the result the partner reads
TOKENIZER_DIR = $(CONF_DIR)/tokenizer
PARSER_DIR = $(CONF_DIR)/parser
BUILDER_DIR = $(CONF_DIR)/builder
RESULT_DIR = $(CONF_DIR)/result

# every module's include folder; turned into -I flags in CXXFLAGS
INC_DIRS =	$(SRC_DIR)/inc $(UTIL_DIR)/inc $(CONF_DIR)/inc \
			$(TOKENIZER_DIR)/inc $(PARSER_DIR)/inc $(BUILDER_DIR)/inc $(RESULT_DIR)/inc \
			$(EXEC_DIR)/inc

SRC =	$(SRC_DIR)/main.cpp \
		$(UTIL_DIR)/utils.cpp \
		$(CONF_DIR)/parse_config.cpp \
		$(CONF_DIR)/ConfigError.cpp \
		$(TOKENIZER_DIR)/Tokenizer.cpp \
		$(TOKENIZER_DIR)/Token.cpp \
		$(PARSER_DIR)/Parser.cpp \
		$(BUILDER_DIR)/ConfigBuilder.cpp \
		$(BUILDER_DIR)/directive_checks.cpp \
		$(BUILDER_DIR)/server_directives.cpp \
		$(BUILDER_DIR)/location_directives.cpp \
		$(BUILDER_DIR)/shared_directives.cpp \
		$(RESULT_DIR)/Config.cpp \
		$(RESULT_DIR)/ServerConfig.cpp \
		$(RESULT_DIR)/LocationConfig.cpp \
		$(RESULT_DIR)/ListenAddress.cpp \
		$(EXEC_DIR)/Request.cpp \
		$(EXEC_DIR)/Server.cpp \
		$(EXEC_DIR)/Socket.cpp

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
