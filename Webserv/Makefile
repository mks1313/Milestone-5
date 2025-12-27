# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/26 17:26:05 by mmarinov          #+#    #+#              #
#    Updated: 2025/12/27 16:44:37 by mmarinov         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# MAKEFLAGS += --silent

# ==== Project ====
NAME        := CPP-05

# ==== Compiler & Flags ====
CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -std=c++98 -pedantic-errors
INC         := -Iinc
DEPFLAGS    := -MMD -MP
RM          := rm -rf

# ==== Extras ====
# debug: CXXFLAGS += -g3 -O0
# debug: re
#
# asan: CXXFLAGS += -g3 -O0 -fsanitize=address
# asan: re

# ==== Directories ====
SRC_D       := src
OBJ_D       := objs

# ==== Sources ====
SRCS        := main.cpp & *.cpp

# ==== Objects / Deps ====
OBJS        := $(SRCS:%.cpp=$(OBJ_D)/%.o)
DEPS        := $(OBJS:.o=.d)

# ==== Colors ====
COL_RESET   := \033[0m
COL_BLUE    := \033[1;34m
COL_GREEN   := \033[1;32m
COL_RED     := \033[1;31m
COL_YELLOW  := \033[1;33m

# ==== Default rule ====
all: $(NAME)

# ==== Link ====
$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@printf "$(COL_GREEN)Built:$(COL_RESET) %s\n" "$(NAME)"

# ==== Compile ====
$(OBJ_D)/%.o: $(SRC_D)/%.cpp | $(OBJ_D)
	@$(CXX) $(CXXFLAGS) $(INC) $(DEPFLAGS) -c $< -o $@
	@printf "$(COL_BLUE)Compiled:$(COL_RESET) %s\n" "$<"

# ==== Create object directory ====
$(OBJ_D):
	@mkdir -p $(OBJ_D)

# ==== Clean ====
clean:
	@$(RM) $(OBJ_D)
	@printf "$(COL_RED)Cleaned objects$(COL_RESET)\n"

fclean: clean
	@$(RM) $(NAME)
	@printf "$(COL_YELLOW)Removed binary:$(COL_RESET) %s\n" "$(NAME)"

re: fclean all

# ==== Phony ====
.PHONY: all clean fclean re debug asan

# ==== Auto dependencies ====
-include $(DEPS)
