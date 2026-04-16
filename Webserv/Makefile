# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/26 05:15:54 by fcela-ga          #+#    #+#              #
#    Updated: 2026/04/11 00:14:48 by fcela-ga         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

BIN_DIR			:=	bin
INC_DIR			:=	inc
SRC_DIR			:=	src
OBJ_DIR			:=	obj
DEP_DIR			:=	dep

CXX			:=	c++
RM			:=	rm -f

CXXFLAGS		:=	-Wall -Wextra -Werror -std=c++98 -pedantic-errors
CPPFLAGS		:=	-I$(INC_DIR)
DEPFLAGS		=	-MMD -MP -MF $(DEP_DIR)/$*.d

# SUBDIRS: Carpetas dentro de src, pero ignorando la carpeta 'tests'
SUBDIRS         :=  $(shell find $(SRC_DIR) -mindepth 1 -type d -printf '%P\n' | grep -v '^tests')

# 1. Obtener todos los archivos .cpp excepto los que están en tests/
# Usamos grep -E para mayor seguridad en el filtrado
SRCS_FILES_ALL := $(shell find $(SRC_DIR) -type f -name '*.cpp' -printf '%P\n')
SRCS_FILES     := $(filter-out tests/%, $(SRCS_FILES_ALL))

# 2. Definir los archivos para la compilación normal (sin tests)
SRCS_FILES_WITHOUT_MAIN := $(filter-out main.cpp, $(SRCS_FILES))

# 3. Lógica de selección según el target
ifeq ($(MAKECMDGOALS),Utils_test1)
    NAME       := $(BIN_DIR)/webserv_test1
    # IMPORTANTE: Aquí SRCS_FILES ahora contendrá solo las clases + el test
    NAME := $(BIN_DIR)/webserv_test1
    SRCS_FILES := config/Config.cpp tests/Utils_test1.cpp
else ifeq ($(MAKECMDGOALS),Utils_test_str)
    NAME       := $(BIN_DIR)/webserv_test_str
    SRCS_FILES := $(SRCS_FILES_WITHOUT_MAIN) tests/Test_utils_strings.cpp
else ifeq ($(MAKECMDGOALS),Utils_test_issue4)
    NAME       := $(BIN_DIR)/unit_test_utils_4
    SRCS_FILES := $(SRCS_FILES_WITHOUT_MAIN) tests/test_utils_issue_4.cpp
else ifeq ($(MAKECMDGOALS),test_log)
    NAME       := $(BIN_DIR)/test_logging
    SRCS_FILES := $(SRCS_FILES_WITHOUT_MAIN) tests/test_logging.cpp
else ifeq ($(MAKECMDGOALS),Server_test_issue15)
    NAME       := $(BIN_DIR)/test_closeclient
    SRCS_FILES := $(SRCS_FILES_WITHOUT_MAIN) tests/test_close_client.cpp
# else ifeq ($(MAKECMDGOALS),Utils_test2)
#     NAME       := $(BIN_DIR)/webserv_test2
#     SRCS_FILES := $(SRCS_FILES_WITHOUT_MAIN) tests/Utils_test2.cpp
else
    NAME       := $(BIN_DIR)/webserv
    # Si no es test, aseguramos que main.cpp esté en la lista
    SRCS_FILES := $(SRCS_FILES_WITHOUT_MAIN) main.cpp
endif

# 4. Generación de rutas (Añadida la barra '/' necesaria)
SRCS			:= $(addprefix $(SRC_DIR)/, $(SRCS_FILES))
OBJS			:= $(addprefix $(OBJ_DIR)/, $(SRCS_FILES:.cpp=.o))
DEPS			:= $(addprefix $(DEP_DIR)/, $(SRCS_FILES:.cpp=.d))

# Generar listas de directorios
ALL_DIRS		:=	$(OBJ_DIR) $(DEP_DIR) \
				$(addprefix $(OBJ_DIR)/, $(SUBDIRS)) \
				$(addprefix $(DEP_DIR)/, $(SUBDIRS)) \
				$(OBJ_DIR)/tests \
				$(DEP_DIR)/tests

# -----------------------------------------------------------------------------
# Colors (use with printf for Alpine/Docker compatibility)
# -----------------------------------------------------------------------------
RED			:=	\033[0;31m
GREEN			:=	\033[0;32m
YELLOW			:=	\033[1;33m
BLUE			:=	\033[0;34m
CYAN			:=	\033[0;36m
MAGENTA			:=	\033[0;35m
RESET			:=	\033[0m

# -----------------------------------------------------------------------------
# Test configuration
# -----------------------------------------------------------------------------
TEST_DIR		:=	tests
CGI_DIR			:=	cgi-bin
TEST_PORT		:=	8080
CONFIG			:=	config/webserv.conf

# 42 Official Testers
TESTER_DIR		:=	testers
TESTER_42		:=	$(shell [ -x $(TESTER_DIR)/ubuntu_tester ] && echo "$(TESTER_DIR)/ubuntu_tester" || \
				([ -x $(TESTER_DIR)/tester ] && echo "$(TESTER_DIR)/tester" || echo ""))
CGI_TESTER		:=	$(shell [ -x $(TESTER_DIR)/ubuntu_cgi_tester ] && echo "$(TESTER_DIR)/ubuntu_cgi_tester" || \
				([ -x $(TESTER_DIR)/cgi_tester ] && echo "$(TESTER_DIR)/cgi_tester" || echo ""))

# -----------------------------------------------------------------------------
# Docker Configuration (Updated from Fragment)
# -----------------------------------------------------------------------------
# Alpine: Ligero (~150MB) pero INCOMPATIBLE con 42 tester
# Ubuntu: Más pesado (~500MB) pero COMPATIBLE con 42 tester

DOCKER_IMAGE_ALPINE	:=	webserv-alpine
DOCKER_IMAGE_UBUNTU	:=	webserv-ubuntu
DOCKER_CONTAINER	:=	webserv
DOCKER_DIR		:=	docker
DOCKER_PORTS		:=	-p 8080:8080 -p 8081:8081 -p 8082:8082

# Archivo de configuración Docker (para recordar la base seleccionada)
DOCKER_CONFIG		:=	$(DOCKER_DIR)/.docker-config

# Cargar configuración si existe (alpine por defecto)
DOCKER_BASE		:=	$(shell [ -f $(DOCKER_CONFIG) ] && cat $(DOCKER_CONFIG) | grep DOCKER_BASE | cut -d= -f2 || echo "alpine")
DOCKER_IMAGE		:=	$(shell [ "$(DOCKER_BASE)" = "ubuntu" ] && echo "$(DOCKER_IMAGE_UBUNTU)" || echo "$(DOCKER_IMAGE_ALPINE)")
DOCKER_FILE		:=	$(shell [ "$(DOCKER_BASE)" = "ubuntu" ] && echo "$(DOCKER_DIR)/Dockerfile.ubuntu" || echo "$(DOCKER_DIR)/Dockerfile")

DOCKER_VOLUMES		:=	-v $(PWD)/src:/app/src:rw \
				-v $(PWD)/inc:/app/inc:rw \
				-v $(PWD)/Makefile:/app/Makefile:rw \
				-v $(PWD)/config:/app/config:rw \
				-v $(PWD)/www:/app/www:rw \
				-v $(PWD)/cgi-bin:/app/cgi-bin:rw \
				-v $(PWD)/bin:/app/bin:rw \
				-v $(PWD)/logs:/app/logs:rw \
				-v $(PWD)/tests:/app/tests:rw \
				-v $(PWD)/testers:/app/testers:rw \
				-v $(PWD)/comp:/app/comp:rw \
				-v $(PWD)/YoupiBanane:/app/YoupiBanane:rw

# =============================================================================
# Build targets
# =============================================================================

all:				$(NAME)

$(NAME):			$(OBJS) | $(BIN_DIR)
					$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp Makefile | $(OBJ_DIR) $(DEP_DIR) $(INC_DIR) $(ALL_DIRS)
					$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(ALL_DIRS):
					@mkdir -p $@

$(BIN_DIR) $(INC_DIR) $(TESTER_DIR):
					@mkdir -p $@

Utils_test1:		all
Utils_test_str:		all
Utils_test_issue4:	all
Server_test_issue15:	all
test_log:			all
# Utils_test2:		all

clean:
					@printf "$(YELLOW)Vaciando contenido de $(OBJ_DIR) y $(DEP_DIR) (manteniendo raíz)...$(RESET)\n"
					@rm -rf $(OBJ_DIR)/* $(DEP_DIR)/*

fclean:				clean
					@$(RM) $(NAME)

re:				fclean $(NAME)

debug:				CXXFLAGS += -g -O0 -DDEBUG_MODE
debug:				fclean all

-include $(DEPS)

# =============================================================================
# Run targets
# =============================================================================

run: $(NAME)
	@./$(NAME) $(CONFIG)

run-default: $(NAME)
	@./$(NAME)

valgrind: $(NAME)
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--suppressions=.valgrind.supp ./$(NAME) $(CONFIG)

valgrind-quiet: $(NAME)
	@valgrind --leak-check=summary ./$(NAME) $(CONFIG)

# =============================================================================
# Test targets
# =============================================================================

# Run integration tests only (without 42 tester)
test: $(NAME) setup-42-env
	@printf "$(CYAN)Running integration tests...$(RESET)\n"
	@chmod +x $(TEST_DIR)/*.sh $(CGI_DIR)/*.py $(TESTER_DIR)/* 2>/dev/null || true
	@$(TEST_DIR)/test_suite.sh --skip-42

# Run quick tests for CI
test-quick: $(NAME) setup-42-env
	@chmod +x $(TEST_DIR)/*.sh $(TESTER_DIR)/* 2>/dev/null || true
	@$(TEST_DIR)/test_suite.sh --quick --skip-42

# Run ALL tests including 42 tester (interactive at the end)
test-all: $(NAME) setup-42-env
	@printf "$(CYAN)Running complete test suite...$(RESET)\n"
	@chmod +x $(TEST_DIR)/*.sh $(CGI_DIR)/*.py $(TESTER_DIR)/* 2>/dev/null || true
	@# Run integration tests (starts/stops its own server)
	@$(TEST_DIR)/test_suite.sh --skip-42
	@# Start server for security and benchmark tests
	@printf "\n$(CYAN)Starting server for security and benchmark tests...$(RESET)\n"
	@./$(NAME) $(CONFIG) & sleep 2
	@# Run security tests if available
	@if [ -f $(TEST_DIR)/security_tests.sh ]; then \
		$(TEST_DIR)/security_tests.sh --no-start 2>/dev/null || true; \
	fi
	@# Run benchmark tests if available
	@if [ -f $(TEST_DIR)/benchmark_tests.sh ]; then \
		$(TEST_DIR)/benchmark_tests.sh --quick --no-start 2>/dev/null || true; \
	fi
	@# Stop the server after security/benchmark tests
	@pkill -f "./$(NAME)" 2>/dev/null || true
	@sleep 1
	@# Now run 42 tester interactively
	@printf "\n"
	@printf "$(MAGENTA)╔═══════════════════════════════════════════════════════════════╗$(RESET)\n"
	@printf "$(MAGENTA)║     42 Official Tester (Interactive - Press Enter to continue)║$(RESET)\n"
	@printf "$(MAGENTA)╚═══════════════════════════════════════════════════════════════╝$(RESET)\n"
	@printf "\n"
	@if [ -n "$(TESTER_42)" ]; then \
		if [ -f /etc/alpine-release ]; then \
			printf "$(RED)⚠️  42 Tester binary is INCOMPATIBLE with Alpine Linux$(RESET)\n"; \
			printf "$(YELLOW)The ubuntu_tester requires glibc, but Alpine uses musl libc.$(RESET)\n"; \
			printf "$(CYAN)Run the 42 tester on your Ubuntu host machine instead:$(RESET)\n"; \
			printf "  make test-42\n"; \
		else \
			printf "$(YELLOW)Starting server for 42 tester...$(RESET)\n"; \
			./$(NAME) $(CONFIG) & sleep 2; \
			printf "$(GREEN)Server started. Running 42 tester...$(RESET)\n"; \
			printf "$(YELLOW)⚠️  Press ENTER when prompted to continue through tests$(RESET)\n"; \
			printf "\n"; \
			$(TESTER_42) http://localhost:$(TEST_PORT) || true; \
			pkill -f "./$(NAME)" 2>/dev/null || true; \
		fi; \
	else \
		printf "$(RED)42 Tester not found. Place in $(TESTER_DIR)/$(RESET)\n"; \
	fi

# =============================================================================
# 42 Official Tester Targets
# =============================================================================

# Setup environment for 42 tester
setup-42-env:
	@mkdir -p $(TESTER_DIR) YoupiBanane/nop YoupiBanane/Yeah www/put_test $(CGI_DIR)
	@# Extract testers if zip exists
	@[ -f comp/testers.zip ] && unzip -o comp/testers.zip > /dev/null 2>&1 || true
	@chmod +x $(TESTER_DIR)/* 2>/dev/null || true
	@# Create YoupiBanane structure (required by 42 tester)
	@echo "Default index file for YoupiBanane" > YoupiBanane/youpi.bad_extension 2>/dev/null || true
	@echo "CGI test file for .bla extension" > YoupiBanane/youpi.bla 2>/dev/null || true
	@echo "File in nop directory" > YoupiBanane/nop/youpi.bad_extension 2>/dev/null || true
	@echo "Other file in nop" > YoupiBanane/nop/other.pouic 2>/dev/null || true
	@echo "Not happy file in Yeah" > YoupiBanane/Yeah/not_happy.bad_extension 2>/dev/null || true
	@chmod 777 www/put_test 2>/dev/null || true

# Run 42 official tester - INTERACTIVE MODE
test-42: $(NAME) setup-42-env
	@printf "$(MAGENTA)╔═══════════════════════════════════════════════════════════════╗$(RESET)\n"
	@printf "$(MAGENTA)║           42 Official Tester (Interactive Mode)               ║$(RESET)\n"
	@printf "$(MAGENTA)╚═══════════════════════════════════════════════════════════════╝$(RESET)\n"
	@printf "\n"
	@printf "$(YELLOW)⚠️  This tester requires you to press ENTER multiple times$(RESET)\n"
	@printf "\n"
	@if [ -n "$(TESTER_42)" ]; then \
		if [ -f /etc/alpine-release ]; then \
			printf "$(RED)⚠️  42 Tester binary is INCOMPATIBLE with Alpine Linux$(RESET)\n"; \
			printf "$(YELLOW)The ubuntu_tester requires glibc, but Alpine uses musl libc.$(RESET)\n"; \
			printf "$(CYAN)Run the 42 tester on your Ubuntu host machine instead.$(RESET)\n"; \
		else \
			printf "$(CYAN)Starting server in background...$(RESET)\n"; \
			./$(NAME) $(CONFIG) & sleep 2; \
			printf "$(GREEN)Server started on port $(TEST_PORT)$(RESET)\n"; \
			printf "\n"; \
			printf "$(CYAN)Running: $(TESTER_42) http://localhost:$(TEST_PORT)$(RESET)\n"; \
			printf "\n"; \
			$(TESTER_42) http://localhost:$(TEST_PORT); \
			pkill -f "./$(NAME)" 2>/dev/null || true; \
		fi; \
	else \
		printf "$(RED)42 Tester not found$(RESET)\n"; \
		printf "Place testers in: $(TESTER_DIR)/\n"; \
	fi

# Run 42 tester - AUTOMATIC MODE (for CI/scripts)
test-42-auto: $(NAME) setup-42-env
	@printf "$(MAGENTA)╔═══════════════════════════════════════════════════════════════╗$(RESET)\n"
	@printf "$(MAGENTA)║           42 Official Tester (Automatic Mode)                 ║$(RESET)\n"
	@printf "$(MAGENTA)╚═══════════════════════════════════════════════════════════════╝$(RESET)\n"
	@printf "\n"
	@if [ -n "$(TESTER_42)" ]; then \
		if [ -f /etc/alpine-release ]; then \
			printf "$(RED)⚠️  42 Tester binary is INCOMPATIBLE with Alpine Linux$(RESET)\n"; \
			printf "$(YELLOW)Run on Ubuntu/Debian host instead.$(RESET)\n"; \
		else \
			printf "$(CYAN)Starting server in background...$(RESET)\n"; \
			./$(NAME) $(CONFIG) & sleep 2; \
			printf "$(GREEN)Server started on port $(TEST_PORT)$(RESET)\n"; \
			printf "\n"; \
			printf "$(CYAN)Running tester with auto-Enter...$(RESET)\n"; \
			yes "" 2>/dev/null | head -n 100 | $(TESTER_42) http://localhost:$(TEST_PORT) 2>&1 | tee 42_tester_output.txt || true; \
			pkill -f "./$(NAME)" 2>/dev/null || true; \
			printf "\n"; \
			printf "$(GREEN)Output saved to: 42_tester_output.txt$(RESET)\n"; \
		fi; \
	else \
		printf "$(RED)42 Tester not found$(RESET)\n"; \
	fi

# Diagnose server for 42 tester requirements
test-42-diagnose: $(NAME) setup-42-env
	@printf "$(CYAN)Running 42 Tester Diagnostics...$(RESET)\n"
	@printf "\n"
	@./$(NAME) $(CONFIG) & sleep 2
	@printf "$(GREEN)Server started. Running diagnostics...$(RESET)\n"
	@printf "\n"
	@printf "=== Test 1: / must answer to GET only ===\n"
	@printf "GET /:\n"
	@curl -s -o /dev/null -w "  Status: %{http_code}\n" http://localhost:$(TEST_PORT)/
	@printf "POST /:\n"
	@curl -s -o /dev/null -w "  Status: %{http_code} (should be 405)\n" -X POST http://localhost:$(TEST_PORT)/
	@printf "\n"
	@printf "=== Test 2: /put_test must accept PUT ===\n"
	@printf "PUT /put_test/test.txt:\n"
	@curl -s -o /dev/null -w "  Status: %{http_code} (should be 200 or 201)\n" -X PUT -d "test" http://localhost:$(TEST_PORT)/put_test/test.txt
	@printf "\n"
	@printf "=== Test 3: /post_body maxBody=100 ===\n"
	@printf "POST /post_body (small):\n"
	@curl -s -o /dev/null -w "  Status: %{http_code}\n" -X POST -d "small" http://localhost:$(TEST_PORT)/post_body
	@printf "POST /post_body (>100 bytes):\n"
	@curl -s -o /dev/null -w "  Status: %{http_code} (should be 413)\n" -X POST -d "$$(printf 'A%.0s' {1..150})" http://localhost:$(TEST_PORT)/post_body
	@printf "\n"
	@printf "=== Test 4: /directory serves YoupiBanane ===\n"
	@printf "GET /directory/:\n"
	@curl -s -o /dev/null -w "  Status: %{http_code}\n" http://localhost:$(TEST_PORT)/directory/
	@printf "\n"
	@pkill -f "./$(NAME)" 2>/dev/null || true
	@printf "$(GREEN)Diagnostics complete.$(RESET)\n"

# Test CGI functionality
test-cgi: $(NAME) setup-42-env
	@printf "$(CYAN)Testing CGI functionality...$(RESET)\n"
	@chmod +x $(CGI_DIR)/*.py 2>/dev/null || true
	@./$(NAME) $(CONFIG) & sleep 2
	@printf "\n"
	@printf "$(YELLOW)Python CGI scripts:$(RESET)\n"
	@for script in test.py info.py env.py; do \
		if [ -f "$(CGI_DIR)/$$script" ]; then \
			STATUS=$$(curl -s -o /dev/null -w "%{http_code}" "http://localhost:$(TEST_PORT)/cgi-bin/$$script"); \
			if [ "$$STATUS" = "200" ]; then \
				printf "  $(GREEN)✅ $$script OK$(RESET)\n"; \
			else \
				printf "  $(YELLOW)⚠️  $$script ($$STATUS)$(RESET)\n"; \
			fi; \
		fi; \
	done
	@printf "\n"
	@printf "$(YELLOW)42 CGI Tester (.bla extension):$(RESET)\n"
	@if [ -n "$(CGI_TESTER)" ] && [ -f "YoupiBanane/youpi.bla" ]; then \
		STATUS=$$(curl -s -o /dev/null -w "%{http_code}" -X POST "http://localhost:$(TEST_PORT)/directory/youpi.bla"); \
		if [ "$$STATUS" = "200" ]; then \
			printf "  $(GREEN)✅ youpi.bla CGI OK$(RESET)\n"; \
		else \
			printf "  $(YELLOW)⚠️  youpi.bla ($$STATUS)$(RESET)\n"; \
		fi; \
	else \
		printf "  $(YELLOW)⚠️  CGI tester or .bla file not available$(RESET)\n"; \
	fi
	@pkill -f "./$(NAME)" 2>/dev/null || true

# Check 42 tester status
check-42-testers:
	@printf "$(CYAN)═══════════════════════════════════════════════════════════════$(RESET)\n"
	@printf "$(CYAN)                    42 Tester Status                            $(RESET)\n"
	@printf "$(CYAN)═══════════════════════════════════════════════════════════════$(RESET)\n"
	@printf "\n"
	@printf "$(YELLOW)Testers in $(TESTER_DIR)/:$(RESET)\n"
	@[ -x $(TESTER_DIR)/ubuntu_tester ] && printf "  $(GREEN)✓$(RESET) ubuntu_tester\n" || printf "  $(RED)✗$(RESET) ubuntu_tester\n"
	@[ -x $(TESTER_DIR)/tester ] && printf "  $(GREEN)✓$(RESET) tester\n" || printf "  $(RED)✗$(RESET) tester\n"
	@[ -x $(TESTER_DIR)/ubuntu_cgi_tester ] && printf "  $(GREEN)✓$(RESET) ubuntu_cgi_tester\n" || printf "  $(RED)✗$(RESET) ubuntu_cgi_tester\n"
	@[ -x $(TESTER_DIR)/cgi_tester ] && printf "  $(GREEN)✓$(RESET) cgi_tester\n" || printf "  $(RED)✗$(RESET) cgi_tester\n"
	@printf "\n"
	@printf "$(YELLOW)YoupiBanane directory:$(RESET)\n"
	@[ -d YoupiBanane ] && printf "  $(GREEN)✓$(RESET) YoupiBanane exists\n" || printf "  $(RED)✗$(RESET) YoupiBanane missing\n"
	@[ -f YoupiBanane/youpi.bla ] && printf "  $(GREEN)✓$(RESET) youpi.bla\n" || printf "  $(RED)✗$(RESET) youpi.bla\n"
	@[ -f YoupiBanane/youpi.bad_extension ] && printf "  $(GREEN)✓$(RESET) youpi.bad_extension\n" || printf "  $(RED)✗$(RESET) youpi.bad_extension\n"
	@printf "\n"
	@printf "$(YELLOW)System compatibility:$(RESET)\n"
	@if [ -f /etc/alpine-release ]; then \
		printf "  $(YELLOW)⚠$(RESET) Alpine Linux detected - ubuntu_tester will NOT work here\n"; \
		printf "    Run 42 tester on Ubuntu/Debian host instead\n"; \
	else \
		printf "  $(GREEN)✓$(RESET) System should be compatible with testers\n"; \
	fi
	@printf "\n"
	@printf "$(YELLOW)Commands:$(RESET)\n"
	@printf "  make test-42          - Interactive 42 tester\n"
	@printf "  make test-42-auto     - Automatic 42 tester\n"
	@printf "  make test-42-diagnose - Diagnose requirements\n"
	@printf "  make test-all         - All tests + 42 tester at end\n"

# Run 42 tester with verbose output (interactive)
test-42-verbose: $(NAME) setup-42-env
	@echo "$(MAGENTA)╔═══════════════════════════════════════════════════════════════╗$(RESET)"
	@echo "$(MAGENTA)║           42 Official Tester (Verbose/Interactive)            ║$(RESET)"
	@echo "$(MAGENTA)╚═══════════════════════════════════════════════════════════════╝$(RESET)"
	@echo ""
	@echo "$(YELLOW)Starting server in background...$(RESET)"
	@./$(NAME) $(CONFIG) & sleep 2
	@echo "$(GREEN)Server started on port $(TEST_PORT)$(RESET)"
	@echo ""
	@if [ -n "$(TESTER_42)" ]; then \
		echo "$(CYAN)Running: $(TESTER_42) http://localhost:$(TEST_PORT)$(RESET)"; \
		echo ""; \
		$(TESTER_42) http://localhost:$(TEST_PORT) 2>&1 | tee 42_tester_output.txt; \
	else \
		echo "$(RED)42 Tester not found$(RESET)"; \
	fi
	@pkill -f "./$(NAME)" 2>/dev/null || true
	@echo ""
	@echo "$(GREEN)Output saved to: 42_tester_output.txt$(RESET)"

# =============================================================================
# Other Test Targets
# =============================================================================

test-security: $(NAME)
	@chmod +x $(TEST_DIR)/*.sh 2>/dev/null || true
	@if [ -f $(TEST_DIR)/security_tests.sh ]; then \
		./$(NAME) $(CONFIG) & sleep 2; \
		$(TEST_DIR)/security_tests.sh --no-start; \
		pkill -f "./$(NAME)" 2>/dev/null || true; \
	else \
		printf "$(YELLOW)security_tests.sh not found$(RESET)\n"; \
	fi

test-benchmark: $(NAME)
	@chmod +x $(TEST_DIR)/*.sh 2>/dev/null || true
	@./$(NAME) $(CONFIG) & sleep 2
	@if [ -f $(TEST_DIR)/benchmark_tests.sh ]; then \
		$(TEST_DIR)/benchmark_tests.sh; \
	else \
		siege -b -c 50 -t 30S http://localhost:$(TEST_PORT)/ 2>/dev/null || printf "$(YELLOW)siege not installed$(RESET)\n"; \
	fi
	@pkill -f "./$(NAME)" 2>/dev/null || true

test-benchmark-quick: $(NAME)
	@chmod +x $(TEST_DIR)/*.sh 2>/dev/null || true
	@./$(NAME) $(CONFIG) & sleep 2
	@if [ -f $(TEST_DIR)/benchmark_tests.sh ]; then \
		$(TEST_DIR)/benchmark_tests.sh --quick --no-start; \
	else \
		siege -b -c 10 -t 10S http://localhost:$(TEST_PORT)/ 2>/dev/null || printf "$(YELLOW)siege not installed$(RESET)\n"; \
	fi
	@pkill -f "./$(NAME)" 2>/dev/null || true

# Test file upload functionality
test-upload: $(NAME)
	@printf "$(BLUE)Testing file upload...$(RESET)\n"
	@./$(NAME) $(CONFIG) & sleep 1
	@printf "Test file content $$(date)" > /tmp/test_upload.txt
	@curl -s -X POST -F "file=@/tmp/test_upload.txt" http://localhost:$(TEST_PORT)/upload
	@rm -f /tmp/test_upload.txt
	@pkill -f "./$(NAME)" 2>/dev/null || true
	@printf "\n$(GREEN)✅ Upload test completed$(RESET)\n"

# Telnet testing instructions
test-telnet:
	@printf "$(BLUE)Use these commands to test with telnet:$(RESET)\n"
	@printf "  telnet localhost $(TEST_PORT)\n"
	@printf "  GET / HTTP/1.1\n"
	@printf "  Host: localhost\n"
	@printf "  (press Enter twice)\n"

test-subject: $(NAME) setup-42-env
	@printf "$(CYAN)42 Subject Validation$(RESET)\n"
	@[ -f ./$(NAME) ] && printf "$(GREEN)✅ Binary compiled$(RESET)\n" || printf "$(RED)❌ No binary$(RESET)\n"
	@[ -f $(CONFIG) ] && printf "$(GREEN)✅ Config file$(RESET)\n" || printf "$(RED)❌ No config$(RESET)\n"
	@[ -n "$(TESTER_42)" ] && printf "$(GREEN)✅ 42 Tester available$(RESET)\n" || printf "$(YELLOW)⚠️  No 42 Tester$(RESET)\n"
	@[ -d YoupiBanane ] && printf "$(GREEN)✅ YoupiBanane directory$(RESET)\n" || printf "$(RED)❌ No YoupiBanane$(RESET)\n"

# =============================================================================
# Utility targets
# =============================================================================

setup:
	@mkdir -p $(BIN_DIR) $(OBJ_DIR) $(DEP_DIR) $(INC_DIR) www/uploads www/errors \
		www/deletable www/put_test logs $(TESTER_DIR) $(CGI_DIR) comp YoupiBanane
	@printf "$(GREEN)✓ All directories created$(RESET)\n"

install-siege:
	@printf "$(BLUE)Installing siege...$(RESET)\n"
	@which siege > /dev/null 2>&1 || \
		(which brew > /dev/null 2>&1 && brew install siege) || \
		(which apt-get > /dev/null 2>&1 && sudo apt-get install -y siege) || \
		(which apk > /dev/null 2>&1 && apk add --no-cache siege) || \
		printf "$(RED)Please install siege manually$(RESET)\n"

docs:
	@command -v doxygen > /dev/null && doxygen Doxyfile 2>/dev/null || echo "doxygen not installed"

count:
	@find src srcs -name '*.cpp' 2>/dev/null | xargs wc -l || echo "0"
	@find inc include includes -name '*.hpp' -o -name '*.h' 2>/dev/null | xargs wc -l || echo "0"

style:
	@cppcheck --enable=all --std=c++03 --suppress=missingIncludeSystem -I inc/ src/ 2>&1 || echo "cppcheck not installed"

# =============================================================================
# Docker Build Targets (Enhanced for Compose)
# =============================================================================

# Construir imagen usando configuración actual (basado en .docker-config)
docker-build:
	@printf "$(CYAN)Construyendo entorno Docker con Compose ($(DOCKER_BASE))...$(RESET)\n"
	@if [ "$(DOCKER_BASE)" = "ubuntu" ]; then \
		docker compose -f $(DOCKER_COMPOSE_FILE) --profile ubuntu build webserv-ubuntu; \
	else \
		docker compose -f $(DOCKER_COMPOSE_FILE) build webserv; \
	fi
	@# También construimos el proxy por si acaso se ha modificado el nginx.conf o similar
	@docker compose -f $(DOCKER_COMPOSE_FILE) build proxy
	@printf "$(GREEN)✓ Construcción completada ($(DOCKER_BASE))$(RESET)\n"
	@if [ "$(DOCKER_BASE)" = "alpine" ]; then \
		printf "$(YELLOW)⚠️  Nota: El 42 tester NO funcionará en Alpine$(RESET)\n"; \
		printf "$(YELLOW)   Usa 'make docker-build-ubuntu' para compatibilidad oficial$(RESET)\n"; \
	else \
		printf "$(GREEN)✓ Esta imagen es compatible con el 42 tester oficial$(RESET)\n"; \
	fi

# Construir imagen Alpine específicamente
docker-build-alpine:
	@printf "$(CYAN)Construyendo imagen Docker Alpine vía Compose...$(RESET)\n"
	@docker compose -f $(DOCKER_COMPOSE_FILE) build webserv
	@echo "DOCKER_BASE=alpine" > $(DOCKER_CONFIG)
	@printf "$(GREEN)✓ Imagen construida: $(DOCKER_IMAGE_ALPINE):latest$(RESET)\n"
	@printf "$(YELLOW)⚠️  Nota: El 42 tester NO funcionará en Alpine$(RESET)\n"

# Construir imagen Ubuntu específicamente
docker-build-ubuntu:
	@printf "$(CYAN)Construyendo imagen Docker Ubuntu vía Compose...$(RESET)\n"
	@docker compose -f $(DOCKER_COMPOSE_FILE) --profile ubuntu build webserv-ubuntu
	@echo "DOCKER_BASE=ubuntu" > $(DOCKER_CONFIG)
	@printf "$(GREEN)✓ Imagen construida: $(DOCKER_IMAGE_UBUNTU):latest$(RESET)\n"
	@printf "$(GREEN)✓ Esta imagen es compatible con el 42 tester oficial$(RESET)\n"

# Cambiar configuración a Alpine (sin construir)
docker-use-alpine:
	@echo "DOCKER_BASE=alpine" > $(DOCKER_CONFIG)
	@printf "$(GREEN)✓ Configurado para usar Alpine$(RESET)\n"
	@printf "$(YELLOW)Ejecuta 'make docker-build' para actualizar la imagen$(RESET)\n"

# Cambiar configuración a Ubuntu (sin construir)
docker-use-ubuntu:
	@echo "DOCKER_BASE=ubuntu" > $(DOCKER_CONFIG)
	@printf "$(GREEN)✓ Configurado para usar Ubuntu$(RESET)\n"
	@printf "$(YELLOW)Ejecuta 'make docker-build' para actualizar la imagen$(RESET)\n"

# =============================================================================
# Docker Configuration (Enhanced for Compose + Proxy)
# =============================================================================
DOCKER_DIR          := docker
DOCKER_COMPOSE_FILE := $(DOCKER_DIR)/docker-compose.yml

# Nombres de contenedores según el nuevo docker-compose.yml
DOCKER_CON_ALPINE   := webserv-alpine
DOCKER_CON_UBUNTU   := webserv-ubuntu
DOCKER_CON_PROXY    := webserv-proxy

# Determinamos cuál es el contenedor activo según la base
DOCKER_ACTUAL_CON   := $(shell [ "$(DOCKER_BASE)" = "ubuntu" ] && echo "$(DOCKER_CON_UBUNTU)" || echo "$(DOCKER_CON_ALPINE)")

# =============================================================================
# Docker Container Targets (Preserving Advanced Logic)
# =============================================================================

docker-start:
	@printf "$(CYAN)Iniciando entorno Compose ($(DOCKER_BASE))...$(RESET)\n"
	@if [ "$(DOCKER_BASE)" = "ubuntu" ]; then \
		docker compose -f $(DOCKER_COMPOSE_FILE) --profile ubuntu up -d $(DOCKER_CON_UBUNTU); \
	else \
		docker compose -f $(DOCKER_COMPOSE_FILE) up -d webserv; \
	fi
	@printf "$(GREEN)✓ Entorno iniciado correctamente$(RESET)\n"

docker-start-alpine:
	@$(MAKE) docker-stop DOCKER_BASE=alpine
	@printf "$(CYAN)Iniciando entorno Alpine...$(RESET)\n"
	@docker compose -f $(DOCKER_COMPOSE_FILE) up -d webserv
	@printf "$(GREEN)✓ Contenedor iniciado (Alpine + Proxy)$(RESET)\n"

docker-start-ubuntu:
	@$(MAKE) docker-stop DOCKER_BASE=ubuntu
	@printf "$(CYAN)Iniciando entorno Ubuntu...$(RESET)\n"
	@docker compose -f $(DOCKER_COMPOSE_FILE) --profile ubuntu up -d webserv-ubuntu
	@printf "$(GREEN)✓ Contenedor iniciado (Ubuntu + Proxy)$(RESET)\n"

docker-stop:
	@printf "$(BLUE)Deteniendo contenedores del proyecto...$(RESET)\n"
	@# Primero intentamos el camino elegante con Compose
	@docker compose -f $(DOCKER_COMPOSE_FILE) --profile ubuntu stop 2>/dev/null || true
	@# Lógica avanzada para asegurar que el contenedor de WebServer muere
	@if docker ps --format '{{.Names}}' | grep -q "^$(DOCKER_ACTUAL_CON)$$"; then \
		printf "$(YELLOW)⚠️  El contenedor $(DOCKER_ACTUAL_CON) no se detiene, forzando...$(RESET)\n"; \
		CONTAINER_PID=$$(docker inspect -f '{{.State.Pid}}' $(DOCKER_ACTUAL_CON) 2>/dev/null); \
		if [ -n "$$CONTAINER_PID" ] && [ "$$CONTAINER_PID" != "0" ]; then \
			sudo kill -9 $$CONTAINER_PID 2>/dev/null || true; \
			sleep 1; \
		fi; \
		docker rm -f $(DOCKER_ACTUAL_CON) 2>/dev/null || true; \
	fi
	@# Aseguramos que el proxy también caiga
	@docker rm -f $(DOCKER_CON_PROXY) 2>/dev/null || true
	@printf "$(GREEN)✓ Entorno detenido$(RESET)\n"

docker-restart: docker-stop
	@sleep 1
	@$(MAKE) docker-start

docker-shell:
	@if ! docker ps --format '{{.Names}}' | grep -q "^$(DOCKER_ACTUAL_CON)$$"; then \
		$(MAKE) docker-start; \
	fi
	@printf "$(BLUE)Opening shell in $(DOCKER_ACTUAL_CON)...$(RESET)\n"
	@docker exec -it $(DOCKER_ACTUAL_CON) /bin/bash

docker-compile:
	@if ! docker ps --format '{{.Names}}' | grep -q "^$(DOCKER_ACTUAL_CON)$$"; then \
		$(MAKE) docker-start; \
	fi
	@printf "$(BLUE)Compiling webserv inside $(DOCKER_ACTUAL_CON)...$(RESET)\n"
	@docker exec -it $(DOCKER_ACTUAL_CON) make re
	@printf "$(GREEN)✅ Compilation completed$(RESET)\n"

docker-run:
	@if docker ps --format '{{.Names}}' | grep -q "^$(DOCKER_ACTUAL_CON)$$"; then \
		docker exec $(DOCKER_ACTUAL_CON) pkill -9 webserv 2>/dev/null || true; \
	fi
	@if ! docker ps --format '{{.Names}}' | grep -q "^$(DOCKER_ACTUAL_CON)$$"; then \
		$(MAKE) docker-start; \
	fi
	@echo "$(BLUE)Running webserv inside container...$(RESET)"
	@echo "$(CYAN)  Acceso vía Proxy (Campus compatible):$(RESET)"
	@echo "    - http://localhost:8083  (example.local)"
	@echo "    - http://localhost:8086  (webserv.local)"
	@docker exec -it $(DOCKER_ACTUAL_CON) ./$(NAME) $(CONFIG)

# =============================================================================
# Docker Test Targets (Rescatados e integrados)
# =============================================================================

docker-test-42:
	@if [ "$(DOCKER_BASE)" = "alpine" ]; then \
		printf "$(RED)⚠️  El 42 tester NO es compatible con Alpine$(RESET)\n"; \
		exit 1; \
	fi
	@docker exec -it $(DOCKER_ACTUAL_CON) make test-42

docker-siege:
	@if ! docker ps --format '{{.Names}}' | grep -q "^$(DOCKER_ACTUAL_CON)$$"; then \
		$(MAKE) docker-start; \
	fi
	@echo "$(BLUE)Running siege stress test...$(RESET)"
	@docker exec -d $(DOCKER_ACTUAL_CON) ./bin/webserv $(CONFIG)
	@sleep 2
	@docker exec -it $(DOCKER_ACTUAL_CON) siege -b -c 50 -t 30S http://localhost:$(TEST_PORT)/
	@docker exec $(DOCKER_ACTUAL_CON) pkill -f webserv 2>/dev/null || true

# =============================================================================
# Docker Clean Targets (Avanzado)
# =============================================================================

docker-clean-all: docker-stop
	@echo "$(YELLOW)Cleaning all Docker resources...$(RESET)"
	@docker compose -f $(DOCKER_COMPOSE_FILE) --profile ubuntu down --rmi all --volumes --remove-orphans 2>/dev/null || true
	@docker system prune -f 2>/dev/null || true
	@echo "$(GREEN)✓ Cleaned$(RESET)"

# =============================================================================
# Docker Status & Utility
# =============================================================================

docker-status:
	@printf "$(CYAN)Docker Status$(RESET)\n"
	@printf "$(YELLOW)Configuración actual:$(RESET) $(DOCKER_BASE)\n"
	@printf "\n$(YELLOW)Imágenes:$(RESET)\n"
	@docker images --format '{{.Repository}}:{{.Tag}}' | grep -q "^$(DOCKER_IMAGE_ALPINE):latest$$" && \
		printf "  $(GREEN)✓$(RESET) $(DOCKER_IMAGE_ALPINE):latest\n" || \
		printf "  $(RED)✗$(RESET) $(DOCKER_IMAGE_ALPINE):latest\n"
	@docker images --format '{{.Repository}}:{{.Tag}}' | grep -q "^$(DOCKER_IMAGE_UBUNTU):latest$$" && \
		printf "  $(GREEN)✓$(RESET) $(DOCKER_IMAGE_UBUNTU):latest\n" || \
		printf "  $(RED)✗$(RESET) $(DOCKER_IMAGE_UBUNTU):latest\n"
	@printf "\n$(YELLOW)Contenedor:$(RESET)\n"
	@docker ps --format '{{.Names}}' | grep -q "^$(DOCKER_CONTAINER)$$" && \
		printf "  $(GREEN)✓$(RESET) Corriendo\n" || \
		printf "  $(RED)✗$(RESET) No corriendo\n"

docker-logs:
	@docker logs -f $(DOCKER_CONTAINER)

# -----------------------------------------------------------------------------
# Docker: Diagnose issues
# -----------------------------------------------------------------------------
docker-diagnose:
	@echo "$(CYAN)═══════════════════════════════════════════════════════════════$(RESET)"
	@echo "$(CYAN)                    Docker Diagnostics                          $(RESET)"
	@echo "$(CYAN)═══════════════════════════════════════════════════════════════$(RESET)"
	@echo ""
	@echo "$(YELLOW)Container status:$(RESET)"
	@docker ps -a --filter name=$(DOCKER_CONTAINER) --format "table {{.ID}}\t{{.Status}}\t{{.Names}}"
	@echo ""
	@if docker ps --format '{{.Names}}' | grep -q "^$(DOCKER_CONTAINER)$$"; then \
		echo "$(YELLOW)Container PID:$(RESET)"; \
		docker inspect -f '{{.State.Pid}}' $(DOCKER_CONTAINER); \
		echo ""; \
		echo "$(YELLOW)Running processes in container:$(RESET)"; \
		docker exec $(DOCKER_CONTAINER) ps aux 2>/dev/null || echo "Cannot access container"; \
	else \
		echo "$(RED)Container not running$(RESET)"; \
	fi
	@echo ""
	@echo "$(YELLOW)Docker context:$(RESET)"
	@docker context show
	@echo ""
	@echo "$(YELLOW)User groups:$(RESET)"
	@groups | grep docker && echo "✓ User in docker group" || echo "✗ User NOT in docker group"

docker-check-42-testers:
	@docker exec $(DOCKER_CONTAINER) make check-42-testers

# -----------------------------------------------------------------------------
# Docker: Force stop (nuclear option)
# -----------------------------------------------------------------------------
docker-force-stop:
	@echo "$(RED)Force stopping container (nuclear option)...$(RESET)"
	@CONTAINER_PID=$$(docker inspect -f '{{.State.Pid}}' $(DOCKER_CONTAINER) 2>/dev/null); \
	if [ -n "$$CONTAINER_PID" ] && [ "$$CONTAINER_PID" != "0" ]; then \
		echo "Killing PID: $$CONTAINER_PID"; \
		sudo kill -9 $$CONTAINER_PID; \
	fi
	@sleep 1
	@docker rm -f $(DOCKER_CONTAINER) 2>/dev/null || \
		sudo docker rm -f $(DOCKER_CONTAINER) 2>/dev/null || true
	@echo "$(GREEN)✓ Force stopped$(RESET)"

docker-compose-up:
	@docker-compose -f $(DOCKER_DIR)/docker-compose.yml up -d webserv

docker-compose-up-ubuntu:
	@docker-compose -f $(DOCKER_DIR)/docker-compose.yml --profile ubuntu up -d webserv-ubuntu

docker-compose-up-alpine:
	@docker-compose -f $(DOCKER_DIR)/docker-compose.yml --profile alpine up -d webserv-alpine

docker-compose-down:
	@docker-compose -f $(DOCKER_DIR)/docker-compose.yml down

# =============================================================================
# Help
# =============================================================================

help:
	@printf "$(CYAN)═══════════════════════════════════════════════════════════════$(RESET)\n"
	@printf "$(CYAN)                    Webserv Makefile Help                       $(RESET)\n"
	@printf "$(CYAN)═══════════════════════════════════════════════════════════════$(RESET)\n"
	@printf "\n"
	@printf "$(YELLOW)Build:$(RESET)\n"
	@printf "  all, clean, fclean, re, debug\n"
	@printf "\n"
	@printf "$(YELLOW)Run:$(RESET)\n"
	@printf "  run              - Start server with config\n"
	@printf "  valgrind         - Run with memory check\n"
	@printf "\n"
	@printf "$(YELLOW)Tests:$(RESET)\n"
	@printf "  test             - Integration tests only\n"
	@printf "  test-all         - ALL tests + 42 tester interactive at end\n"
	@printf "  test-quick       - Quick tests for CI\n"
	@printf "  test-cgi         - Test CGI functionality\n"
	@printf "  test-security    - Security tests\n"
	@printf "  test-benchmark   - Benchmark tests (requires siege)\n"
	@printf "  test-subject     - 42 subject validation\n"
	@printf "\n"
	@printf "$(MAGENTA)42 Official Testers:$(RESET)\n"
	@printf "  test-42          - Interactive 42 tester\n"
	@printf "  test-42-auto     - Automatic 42 tester (for scripts)\n"
	@printf "  test-42-diagnose - Diagnose 42 tester requirements\n"
	@printf "  check-42-testers - Check tester availability\n"
	@printf "  setup-42-env     - Setup YoupiBanane structure\n"
	@printf "\n"
	@printf "$(YELLOW)Docker (Enhanced):$(RESET)\n"
	@printf "  docker-build     - Build image (based on current config)\n"
	@printf "  docker-use-alpine - Switch to Alpine (Small but no 42 tester)\n"
	@printf "  docker-use-ubuntu - Switch to Ubuntu (Full support)\n"
	@printf "  docker-start     - Start container\n"
	@printf "  docker-test-42   - Run 42 tester in Docker (Ubuntu only)\n"
	@printf "\n"

.PHONY: all clean fclean re debug run run-default valgrind valgrind-quiet \
		test test-quick test-all test-cgi test-security test-benchmark test-benchmark-quick test-subject \
		test-42 test-42-auto test-42-diagnose setup-42-env check-42-testers \
		setup install-siege help \
		docker-build docker-build-alpine docker-build-ubuntu \
		docker-use-alpine docker-use-ubuntu \
		docker-start docker-start-alpine docker-start-ubuntu \
		docker-stop docker-shell docker-test docker-test-42 \
		docker-clean docker-fclean docker-remove docker-restart docker-compile docker-run \
		docker-test-42-auto docker-test-security docker-test-subject docker-test-cgi \
		docker-test-all docker-test-quick docker-valgrind docker-logs docker-status \
		docker-force-stop docker-clean-all docker-compose-up docker-compose-down \
		docker-compose-up-ubuntu docker-siege docs count style docker-diagnose docker-check-42-testers \
		test-42-verbose test-upload test-telnet docker-compose-up-alpine \
		Utils_test1
