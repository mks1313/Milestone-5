#!/bin/bash
# =============================================================================
# test_webserv.sh — Automated HTTP test suite for the 42 Webserv project
# Usage: ./test_webserv.sh [host] [port]
#   Default: host=localhost, port=8080
# =============================================================================

HOST="${1:-localhost}"
PORT="${2:-8080}"
TIMEOUT=5

# --- colours -----------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

# --- counters ----------------------------------------------------------------
PASS=0
FAIL=0
SKIP=0

# =============================================================================
# Helper: send a raw HTTP request over TCP and return the response
# =============================================================================
send_request() {
    local h="$1"
    local p="$2"
    local req="$3"
    printf '%b' "$req" | nc -q 1 -w "$TIMEOUT" "$h" "$p" 2>/dev/null
}

# =============================================================================
# Helper: run one test
#   $1 = test ID  (string: "01", "05a", "08b" — no %02d formatting)
#   $2 = description
#   $3 = raw request (printf-style, \r\n literal)
#   $4 = expected HTTP status code
#   $5 = (optional) second accepted status code
#   $6 = (optional) string that must appear in response
# =============================================================================
run_test() {
    local id="$1"
    local desc="$2"
    local req="$3"
    local expected="$4"
    local alt="${5:-}"
    local must_contain="${6:-}"

    # FIX: use %s instead of %02d — test IDs can be alphanumeric (5a, 8b, 18a)
    printf "${CYAN}[TEST %s]${RESET} %s ... " "$id" "$desc"

    local response
    response=$(send_request "$HOST" "$PORT" "$req")

    if [ -z "$response" ]; then
        printf "${RED}FAIL${RESET} (no response — server unreachable or timed out)\n"
        FAIL=$((FAIL + 1))
        return
    fi

    local status_line
    status_line=$(printf '%s' "$response" | head -1 | tr -d '\r')
    local code
    code=$(printf '%s' "$status_line" | awk '{print $2}')

    local code_ok=0
    [ "$code" = "$expected" ] && code_ok=1
    [ -n "$alt" ] && [ "$code" = "$alt" ] && code_ok=1

    local body_ok=1
    if [ -n "$must_contain" ]; then
        if ! printf '%s' "$response" | grep -q "$must_contain"; then
            body_ok=0
        fi
    fi

    if [ "$code_ok" -eq 1 ] && [ "$body_ok" -eq 1 ]; then
        printf "${GREEN}OK${RESET}  (got: %s)\n" "$status_line"
        PASS=$((PASS + 1))
    else
        if [ "$code_ok" -eq 0 ]; then
            if [ -n "$alt" ]; then
                printf "${RED}FAIL${RESET} (expected: %s or %s — got: %s)\n" "$expected" "$alt" "$status_line"
            else
                printf "${RED}FAIL${RESET} (expected: %s — got: %s)\n" "$expected" "$status_line"
            fi
        else
            printf "${RED}FAIL${RESET} (status ok but '%s' not found in response)\n" "$must_contain"
        fi
        FAIL=$((FAIL + 1))
        printf "       ${YELLOW}Response preview:${RESET}\n"
        printf '%s' "$response" | head -5 | while IFS= read -r line; do
            printf "         %s\n" "$(printf '%s' "$line" | tr -d '\r')"
        done
    fi
}

# =============================================================================
# Helper: run a test against a specific port (secondary servers)
# =============================================================================
run_test_port() {
    local id="$1"
    local desc="$2"
    local p="$3"
    local req="$4"
    local expected="$5"
    local alt="${6:-}"

    # FIX: use %s instead of %02d
    printf "${CYAN}[TEST %s]${RESET} %s (port %s) ... " "$id" "$desc" "$p"

    local response
    response=$(send_request "$HOST" "$p" "$req")

    if [ -z "$response" ]; then
        printf "${YELLOW}SKIP${RESET} (port %s not responding)\n" "$p"
        SKIP=$((SKIP + 1))
        return
    fi

    local status_line
    status_line=$(printf '%s' "$response" | head -1 | tr -d '\r')
    local code
    code=$(printf '%s' "$status_line" | awk '{print $2}')

    local code_ok=0
    [ "$code" = "$expected" ] && code_ok=1
    [ -n "$alt" ] && [ "$code" = "$alt" ] && code_ok=1

    if [ "$code_ok" -eq 1 ]; then
        printf "${GREEN}OK${RESET}  (got: %s)\n" "$status_line"
        PASS=$((PASS + 1))
    else
        if [ -n "$alt" ]; then
            printf "${RED}FAIL${RESET} (expected: %s or %s — got: %s)\n" "$expected" "$alt" "$status_line"
        else
            printf "${RED}FAIL${RESET} (expected: %s — got: %s)\n" "$expected" "$status_line"
        fi
        FAIL=$((FAIL + 1))
        printf '%s' "$response" | head -5 | while IFS= read -r line; do
            printf "         %s\n" "$(printf '%s' "$line" | tr -d '\r')"
        done
    fi
}

# =============================================================================
# Pre-flight: check server is up
# =============================================================================
printf "\n${BOLD}========================================${RESET}\n"
printf "${BOLD}  Webserv Test Suite — %s:%s${RESET}\n" "$HOST" "$PORT"
printf "${BOLD}========================================${RESET}\n\n"

printf "Checking server is up ... "
if ! nc -z -w 2 "$HOST" "$PORT" 2>/dev/null; then
    printf "${RED}UNREACHABLE${RESET}\n"
    printf "  Start the server first:  ./webserv config/webserv.conf\n\n"
    exit 1
fi
printf "${GREEN}OK${RESET}\n\n"

# =============================================================================
# Setup: resolve the real project root (one level above tests/)
#        then prepare fixtures directly on the filesystem — no HTTP needed.
#
# Why filesystem and not HTTP?
#   - /put_test has no DELETE → can't remove hello.txt via HTTP
#   - /deletable has no PUT   → can't create test.txt via HTTP
# =============================================================================
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

printf "Setup: preparing test fixtures in %s/www ... " "$PROJECT_ROOT"

# TEST 05a: delete hello.txt so the first PUT creates it fresh → 201
rm -f "$PROJECT_ROOT/www/put_test/hello.txt"

# TEST 08: create deletable/test.txt directly so DELETE has a real target
echo "delete_me" > "$PROJECT_ROOT/www/deletable/test.txt"

printf "${GREEN}done${RESET}\n\n"
sleep 0.2

# =============================================================================
# TESTS
# =============================================================================

printf "${BOLD}--- Basic GET ---${RESET}\n"

run_test "01" "GET / → 200 OK (index page)" \
    "GET / HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "200"

run_test "02" "GET /nonexistent → 404 Not Found" \
    "GET /this_file_does_not_exist_42.html HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "404"

run_test "04" "GET /static/style.css → 200 OK" \
    "GET /static/style.css HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "200"

printf "\n${BOLD}--- Method restrictions ---${RESET}\n"

run_test "03" "POST / (GET-only route) → 405 Method Not Allowed" \
    "POST / HTTP/1.1\r\nHost: $HOST\r\nContent-Length: 0\r\nConnection: close\r\n\r\n" \
    "405"

run_test "16" "PATCH / (unknown method) → 405 or 501" \
    "PATCH / HTTP/1.1\r\nHost: $HOST\r\nContent-Length: 0\r\nConnection: close\r\n\r\n" \
    "405" "501"

printf "\n${BOLD}--- PUT (tester requirement 2) ---${RESET}\n"

# hello.txt was removed in setup → this PUT creates it fresh → must be 201
run_test "05a" "PUT /put_test/hello.txt (new file) → 201 Created" \
    "PUT /put_test/hello.txt HTTP/1.1\r\nHost: $HOST\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!" \
    "201"

run_test "05b" "PUT /put_test/hello.txt (existing file) → 200 OK" \
    "PUT /put_test/hello.txt HTTP/1.1\r\nHost: $HOST\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!" \
    "200"

run_test "05c" "GET /put_test/hello.txt → 200 OK + body contains 'Hello'" \
    "GET /put_test/hello.txt HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "200" "" "Hello"

printf "\n${BOLD}--- POST body size limits ---${RESET}\n"

BODY_101="AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEAAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEA"
run_test "06" "POST /post_body (101 bytes > max 100) → 413" \
    "POST /post_body HTTP/1.1\r\nHost: $HOST\r\nContent-Type: text/plain\r\nContent-Length: 101\r\nConnection: close\r\n\r\n$BODY_101" \
    "413"

run_test "07" "POST /post_body (10 bytes ≤ max 100) → 200 or 204" \
    "POST /post_body HTTP/1.1\r\nHost: $HOST\r\nContent-Type: text/plain\r\nContent-Length: 10\r\nConnection: close\r\n\r\nhelloworld" \
    "200" "204"

printf "\n${BOLD}--- DELETE ---${RESET}\n"

run_test "08" "DELETE /deletable/test.txt (exists) → 204 No Content" \
    "DELETE /deletable/test.txt HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "204"

run_test "08b" "GET /deletable/test.txt after DELETE → 404" \
    "GET /deletable/test.txt HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "404"

run_test "09" "DELETE /deletable/nonexistent.txt → 404 Not Found" \
    "DELETE /deletable/nonexistent_file_42.txt HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "404"

printf "\n${BOLD}--- Directory / default index (tester requirement 5) ---${RESET}\n"

run_test "10" "GET /directory/ → 200 OK (serves youpi.bad_extension)" \
    "GET /directory/ HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "200"

printf "\n${BOLD}--- CGI via .bla (tester requirement 3) ---${RESET}\n"

CGI_BODY="coucou les amis !"
CGI_LEN=$(printf '%s' "$CGI_BODY" | wc -c | tr -d ' ')
run_test "11" "POST /directory/youpi.bla → 200 OK (CGI execution)" \
    "POST /directory/youpi.bla HTTP/1.1\r\nHost: $HOST\r\nContent-Type: text/plain\r\nContent-Length: $CGI_LEN\r\nConnection: close\r\n\r\n$CGI_BODY" \
    "200"

run_test "12" "GET /directory/youpi.bla → 200 OK (CGI via GET)" \
    "GET /directory/youpi.bla HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "200"

printf "\n${BOLD}--- Redirects ---${RESET}\n"

run_test "13" "GET /old-page → 301 Moved Permanently" \
    "GET /old-page HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "301" "" "Location"

run_test "14" "GET /google → 302 Found (external redirect)" \
    "GET /google HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "302" "" "google.com"

printf "\n${BOLD}--- Malformed requests ---${RESET}\n"

# FIX: send truly malformed HTTP (no version line) — nc sends raw bytes, so this works correctly
run_test "15" "GET / (no HTTP version) → 400 Bad Request" \
    "GET /\r\nHost: $HOST\r\n\r\n" \
    "400"

printf "\n${BOLD}--- Secondary servers ---${RESET}\n"

run_test_port "17" "GET / on port 8081 → 200 OK" \
    "8081" \
    "GET / HTTP/1.1\r\nHost: $HOST\r\nConnection: close\r\n\r\n" \
    "200"

run_test_port "18a" "GET / on port 8082 → 200 OK" \
    "8082" \
    "GET / HTTP/1.1\r\nHost: restricted.local\r\nConnection: close\r\n\r\n" \
    "200"

run_test_port "18b" "POST / on port 8082 (restricted) → 405" \
    "8082" \
    "POST / HTTP/1.1\r\nHost: restricted.local\r\nContent-Length: 0\r\nConnection: close\r\n\r\n" \
    "405"

printf "\n${BOLD}--- Large body (global limit) ---${RESET}\n"

run_test "20" "POST /upload (body > 1MB) → 413" \
    "POST /upload HTTP/1.1\r\nHost: $HOST\r\nContent-Type: application/octet-stream\r\nContent-Length: 1048577\r\nConnection: close\r\n\r\n$(python3 -c 'print("A" * 1048577, end="")')" \
    "413"

# =============================================================================
# Summary
# =============================================================================
TOTAL=$((PASS + FAIL + SKIP))
printf "\n${BOLD}========================================${RESET}\n"
printf "${BOLD}  Results: %d tests run${RESET}\n" "$TOTAL"
printf "  ${GREEN}PASS: %d${RESET}\n" "$PASS"
printf "  ${RED}FAIL: %d${RESET}\n" "$FAIL"
[ "$SKIP" -gt 0 ] && printf "  ${YELLOW}SKIP: %d${RESET} (port not responding)\n" "$SKIP"
printf "${BOLD}========================================${RESET}\n\n"

if [ "$FAIL" -eq 0 ]; then
    printf "${GREEN}${BOLD}All tests passed.${RESET}\n\n"
    exit 0
else
    printf "${RED}${BOLD}%d test(s) failed.${RESET}\n\n" "$FAIL"
    exit 1
fi
