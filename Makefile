NAME		=	particle_system
DEBUG_NAME	=	particle_systemDebug

LDFLAGS =	-lGL -lGLU -lglfw -lOpenCL -lGLEW -lX11

CFLAGS	=	-Wall -Wextra -Werror -O2 -g3
DEBUG_CFLAGS	=	-DNDEBUG -Wall -Wextra -Werror -O2 -g3

OBJ_PATH		=	obj/
DEBUG_OBJ_PATH		=	debug_obj/

CC			=	g++
SRC_PATH	=	srcs/
INCLUDES	=	-Iincludes
SRC_NAME	=	main.cpp			\
				camera.cpp			\
				particle_system.cpp	\
				shader.cpp

OBJ_NAME	=	$(SRC_NAME:.cpp=.o)
OBJ		=	$(addprefix $(OBJ_PATH), $(OBJ_NAME))
DEBUG_OBJ	=	$(addprefix $(DEBUG_OBJ_PATH), $(OBJ_NAME))

#----------colors---------#
BLACK		=	\033[1;30m
RED			=	\033[1;31m
GREEN		=	\033[1;32m
BLUE		=	\033[1;34m
PURPLE		=	\033[1;35m
CYAN		=	\033[1;36m
WHITE		=	\033[1;37m
EOC			=	\033[0;0m



all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(RED)=====>Compiling particle_system Test<===== $(WHITE)"
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $(NAME) $(LDFLAGS)
	@echo "$(GREEN)Done ! ✅ $(EOC)"


$(OBJ_PATH)%.o: $(SRC_PATH)%.cpp
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -c $< -o $@

-include $(OBJ:%.o=%.d)

debug: $(DEBUG_NAME)

$(DEBUG_NAME): $(DEBUG_OBJ)
	@echo "$(RED)=====>Compiling particle_system DEBUG<===== $(WHITE)"
	$(CC) $(DEBUG_CFLAGS) $(INCLUDES) $(DEBUG_OBJ) -o $(DEBUG_NAME) $(LDFLAGS)
	@echo "$(GREEN)Done ! ✅ $(EOC)"

$(DEBUG_OBJ_PATH)%.o: $(SRC_PATH)%.cpp
	mkdir -p $(@D)
	$(CC) $(DEBUG_CFLAGS) $(INCLUDES) -MMD -c $< -o $@

-include $(DEBUG_OBJ:%.o=%.d)

clean:
	@echo "$(CYAN)♻  Cleaning obj files ♻ $(WHITE)"
	rm -rf $(OBJ_PATH)
	rm -rf $(DEBUG_OBJ_PATH)
	@echo "$(GREEN)Done !✅ $(EOC)"

fclean: clean
	@echo "$(CYAN)♻  Cleaning executable ♻ $(WHITE)"
	rm -rf $(NAME)
	rm -rf $(DEBUG_NAME)
	@echo "$(GREEN)Done !✅ $(EOC)"

re: fclean all
re_debug: fclean debug

.PHONY: all debug clean fclean re re_debug