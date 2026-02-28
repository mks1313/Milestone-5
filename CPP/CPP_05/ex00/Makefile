# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/11/13 18:52:38 by mmarinov          #+#    #+#              #
#    Updated: 2026/02/28 16:24:13 by mmarinov         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


MAKEFLAGS += --silent

# ==== Project ====
NAME      := Bureaucrat

# ==== Compiler & Flags ====
CXX       := c++
CXXFLAGS  := -Wall -Wextra -Werror -std=c++98 -pedantic-errors
INC       := -Iinc
DEPFLAGS  := -MMD -MP
RM        := rm -rf

# ==== Dirs ====
SRC_D     := src
OBJ_D     := objs

# ==== Sources  ====
SRCS      := main.cpp Bureaucrat.cpp

# ==== Objects / Deps ====
OBJS      := $(SRCS:%.cpp=$(OBJ_D)/%.o)
DEPS      := $(OBJS:.o=.d)

# ==== Colors ====
COL_RESET := \033[0m
COL_BLUE  := \033[1;34m
COL_GREEN := \033[1;32m
COL_RED   := \033[1;31m
COL_YELLOW:= \033[1;33m

# ==== Default ====
all: $(NAME)

# ==== Link ====
$(NAME): $(OBJS) Makefile
	@$(CXX) $(CXXFLAGS) $(INC) $(OBJS) -o $(NAME)
	@printf "$(COL_GREEN)Built:$(COL_RESET) %s\n" "$(NAME)"

# ==== Compile ====
$(OBJ_D)/%.o: $(SRC_D)/%.cpp | $(OBJ_D)
	@$(CXX) $(CXXFLAGS) $(INC) $(DEPFLAGS) -c $< -o $@
	@printf "$(COL_BLUE)Compiled:$(COL_RESET) %s\n" "$<"

$(OBJ_D):
	@mkdir -p $(OBJ_D)

clean:
	@$(RM) $(OBJ_D)
	@printf "$(COL_RED)Cleaned objects$(COL_RESET)\n"

fclean: clean
	@$(RM) $(NAME)
	@printf "$(COL_YELLOW)Removed binary:$(COL_RESET) %s\n" "$(NAME)"

re: fclean all

.PHONY: all clean fclean re debug

# ==== Auto deps ====
-include $(DEPS)
