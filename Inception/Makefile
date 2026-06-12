# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/26 17:26:05 by mmarinov          #+#    #+#              #
#    Updated: 2026/06/12 15:00:00 by mmarinov         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

MAKEFLAGS += --silent

# ==== Project ====
COMPOSE   := srcs/docker-compose.yml
ENV       := srcs/.env
DATA_DIR  := /home/mmarinov/data

# ==== Volumes dirs ====
MARIADB_DIR  := $(DATA_DIR)/mariadb
WORDPRESS_DIR := $(DATA_DIR)/wordpress

# ==== Colors ====
COL_RESET  := \033[0m
COL_BLUE   := \033[1;34m
COL_GREEN  := \033[1;32m
COL_RED    := \033[1;31m
COL_YELLOW := \033[1;33m

# ==== Default ====
all: up

# ==== Up ====
up: $(MARIADB_DIR) $(WORDPRESS_DIR)
	@printf "$(COL_GREEN)Starting Inception...$(COL_RESET)\n"
	@docker compose -f $(COMPOSE) up -d --build

# ==== Down ====
down:
	@printf "$(COL_YELLOW)Stopping Inception...$(COL_RESET)\n"
	@docker compose -f $(COMPOSE) down

# ==== Create volume dirs ====
$(MARIADB_DIR):
	@mkdir -p $(MARIADB_DIR)
	@printf "$(COL_BLUE)Created:$(COL_RESET) %s\n" "$(MARIADB_DIR)"

$(WORDPRESS_DIR):
	@mkdir -p $(WORDPRESS_DIR)
	@printf "$(COL_BLUE)Created:$(COL_RESET) %s\n" "$(WORDPRESS_DIR)"

# ==== Clean (stop + remove volumes) ====
clean: down
	@printf "$(COL_RED)Removing volumes...$(COL_RESET)\n"
	@docker compose -f $(COMPOSE) down -v

# ==== Fclean (clean + data dirs + images) ====
fclean: clean
	@printf "$(COL_RED)Removing images and data...$(COL_RESET)\n"
	@docker compose -f $(COMPOSE) down --rmi all
	@rm -rf $(DATA_DIR)
	@printf "$(COL_RED)Removed:$(COL_RESET) %s\n" "$(DATA_DIR)"

# ==== Re ====
re: fclean all

# ==== Logs ====
logs:
	@docker compose -f $(COMPOSE) logs -f

# ==== Status ====
status:
	@docker compose -f $(COMPOSE) ps

.PHONY: all up down clean fclean re logs status
