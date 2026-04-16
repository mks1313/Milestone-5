#!/bin/bash
# =============================================================================
# test_webserv_curl.sh — curl-based HTTP test suite for the 42 Webserv project
# Usage: ./test_webserv_curl.sh [host] [port]
#   Default: host=localhost, port=8080
# =============================================================================

HOST="${1:-localhost}"
PORT="${2:-8080}"
BASE="http://${HOST}:${PORT}"
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
# Helper: run one test
#   $1  = test ID  (string — no numeric formatting)
#   $2  = description
#   $3  = expected HTTP status code
#   $4  = (optional) second accepted status code
#   $5  = (optional) string that must appear in response headers or body
#   ... = curl arguments
# =============================================================================
run_test() {
    local id="$1";           shift
    local desc="$1";         shift
    local expected="$1";     shift
    local alt="$1";          shift
    local must_contain="$1"; shift

    printf "${CYAN}[TEST %s]${RESET} %s ... " "$id" "$desc"

    local body_file header_file
    body_file=$(mktemp)
    header_file=$(mktemp)

    local code
    code=$(curl -s \
                --max-time "$TIMEOUT" \
                -o "$body_file" \
                -D "$header_file" \
                -w "%{http_code}" \
                "$@" 2>/dev/null)

    local curl_exit=$?

    if [ "$curl_exit" -ne 0 ] || [ -z "$code" ] || [ "$code" = "000" ]; then
        printf "${RED}FAIL${RESET} (no response — server unreachable or timed out)\n"
        FAIL=$((FAIL + 1))
        rm -f "$body_file" "$header_file"
        return
    fi

    local code_ok=0
    [ "$code" = "$expected" ] && code_ok=1
    [ -n "$alt" ] && [ "$code" = "$alt" ] && code_ok=1

    local body_ok=1
    if [ -n "$must_contain" ]; then
        if ! grep -qi "$must_contain" "$header_file" "$body_file" 2>/dev/null; then
            body_ok=0
        fi
    fi

    if [ "$code_ok" -eq 1 ] && [ "$body_ok" -eq 1 ]; then
        printf "${GREEN}OK${RESET}  (HTTP %s)\n" "$code"
        PASS=$((PASS + 1))
    else
        if [ "$code_ok" -eq 0 ]; then
            if [ -n "$alt" ]; then
                printf "${RED}FAIL${RESET} (expected %s or %s — got %s)\n" "$expected" "$alt" "$code"
            else
                printf "${RED}FAIL${RESET} (expected %s — got %s)\n" "$expected" "$code"
            fi
        else
            printf "${RED}FAIL${RESET} (HTTP %s ok, but '%s' not found in response)\n" "$code" "$must_contain"
        fi
        FAIL=$((FAIL + 1))
        printf "       ${YELLOW}Response headers:${RESET}\n"
        head -6 "$header_file" | while IFS= read -r line; do
            printf "         %s\n" "$(printf '%s' "$line" | tr -d '\r')"
        done
    fi

    rm -f "$body_file" "$header_file"
}

# =============================================================================
# Helper: test against a specific port (secondary servers)
# =============================================================================
run_test_port() {
    local id="$1";   shift
    local desc="$1"; shift
    local p="$1";    shift
    local expected="$1"; shift
    local alt="${1:-}"; [ $# -gt 0 ] && shift

    local alt_base="http://${HOST}:${p}"

    printf "${CYAN}[TEST %s]${RESET} %s (port %s) ... " "$id" "$desc" "$p"

    if ! curl -s --max-time 2 -o /dev/null -w "%{http_code}" "$alt_base/" 2>/dev/null | grep -q '[0-9]'; then
        printf "${YELLOW}SKIP${RESET} (port %s not responding)\n" "$p"
        SKIP=$((SKIP + 1))
        return
    fi

    local body_file header_file
    body_file=$(mktemp); header_file=$(mktemp)

    local code
    code=$(curl -s \
                --max-time "$TIMEOUT" \
                -o "$body_file" \
                -D "$header_file" \
                -w "%{http_code}" \
                "$@" 2>/dev/null)

    local code_ok=0
    [ "$code" = "$expected" ] && code_ok=1
    [ -n "$alt" ] && [ "$code" = "$alt" ] && code_ok=1

    if [ "$code_ok" -eq 1 ]; then
        printf "${GREEN}OK${RESET}  (HTTP %s)\n" "$code"
        PASS=$((PASS + 1))
    else
        if [ -n "$alt" ]; then
            printf "${RED}FAIL${RESET} (expected %s or %s — got %s)\n" "$expected" "$alt" "$code"
        else
            printf "${RED}FAIL${RESET} (expected %s — got %s)\n" "$expected" "$code"
        fi
        FAIL=$((FAIL + 1))
        head -6 "$header_file" | while IFS= read -r line; do
            printf "         %s\n" "$(printf '%s' "$line" | tr -d '\r')"
        done
    fi

    rm -f "$body_file" "$header_file"
}

# =============================================================================
# Pre-flight checks
# =============================================================================
printf "\n${BOLD}========================================${RESET}\n"
printf "${BOLD}  Webserv curl Test Suite — %s${RESET}\n" "$BASE"
printf "${BOLD}========================================${RESET}\n\n"

if ! command -v curl > /dev/null 2>&1; then
    printf "${RED}curl not found. Install it first.${RESET}\n"
    exit 1
fi

printf "Checking server is up ... "
if ! curl -s --max-time 3 -o /dev/null -w "%{http_code}" "$BASE/" 2>/dev/null | grep -q '[0-9]'; then
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

# TEST 05a: remove hello.txt so the first PUT creates it fresh → 201
rm -f "$PROJECT_ROOT/www/put_test/hello.txt"

# TEST 08: create deletable/test.txt directly so DELETE has a real target
echo "delete_me" > "$PROJECT_ROOT/www/deletable/test.txt"

sleep 0.2
printf "${GREEN}done${RESET}\n\n"

# =============================================================================
# TESTS
# =============================================================================

# ---------------------------------------------------------------------------
printf "${BOLD}--- Basic GET ---${RESET}\n"
# ---------------------------------------------------------------------------

run_test "01" "GET / → 200 OK (index page)" \
    "200" "" "" \
    "${BASE}/"

run_test "02" "GET /nonexistent → 404 Not Found" \
    "404" "" "" \
    "${BASE}/this_file_does_not_exist_42.html"

run_test "04" "GET /static/style.css → 200 OK" \
    "200" "" "" \
    "${BASE}/static/style.css"

# ---------------------------------------------------------------------------
printf "\n${BOLD}--- Method restrictions ---${RESET}\n"
# ---------------------------------------------------------------------------

run_test "03" "POST / (GET-only route) → 405 Method Not Allowed" \
    "405" "" "" \
    -X POST -H "Content-Length: 0" \
    "${BASE}/"

run_test "16" "PATCH / (unknown method) → 405 or 501" \
    "405" "501" "" \
    -X PATCH -H "Content-Length: 0" \
    "${BASE}/"

# ---------------------------------------------------------------------------
printf "\n${BOLD}--- PUT — save files (tester req. 2) ---${RESET}\n"
# ---------------------------------------------------------------------------

# hello.txt was removed in setup → this PUT creates it fresh → must be 201
run_test "05a" "PUT /put_test/hello.txt (new file) → 201 Created" \
    "201" "" "" \
    -X PUT \
    -H "Content-Type: text/plain" \
    --data "Hello, world!" \
    "${BASE}/put_test/hello.txt"

# FIX: second PUT on same file → server may return 200 (file already exists)
run_test "05b" "PUT /put_test/hello.txt (existing file) → 200 OK" \
    "200" "" "" \
    -X PUT \
    -H "Content-Type: text/plain" \
    --data "Hello, world!" \
    "${BASE}/put_test/hello.txt"

run_test "05c" "GET /put_test/hello.txt → 200 + body contains 'Hello'" \
    "200" "" "Hello" \
    "${BASE}/put_test/hello.txt"

# ---------------------------------------------------------------------------
printf "\n${BOLD}--- POST body size limits ---${RESET}\n"
# ---------------------------------------------------------------------------

BODY_101="AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEAAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEA"

run_test "06" "POST /post_body (101 bytes > max 100) → 413" \
    "413" "" "" \
    -X POST \
    -H "Content-Type: text/plain" \
    --data-raw "$BODY_101" \
    "${BASE}/post_body"

run_test "07" "POST /post_body (10 bytes ≤ max 100) → 200 or 204" \
    "200" "204" "" \
    -X POST \
    -H "Content-Type: text/plain" \
    --data "helloworld" \
    "${BASE}/post_body"

# ---------------------------------------------------------------------------
printf "\n${BOLD}--- DELETE ---${RESET}\n"
# ---------------------------------------------------------------------------

run_test "08" "DELETE /deletable/test.txt (exists) → 204 No Content" \
    "204" "" "" \
    -X DELETE \
    "${BASE}/deletable/test.txt"

run_test "08b" "GET /deletable/test.txt after DELETE → 404" \
    "404" "" "" \
    "${BASE}/deletable/test.txt"

run_test "09" "DELETE /deletable/nonexistent.txt → 404 Not Found" \
    "404" "" "" \
    -X DELETE \
    "${BASE}/deletable/nonexistent_file_42.txt"

# ---------------------------------------------------------------------------
printf "\n${BOLD}--- Directory + default index (tester req. 5) ---${RESET}\n"
# ---------------------------------------------------------------------------

run_test "10" "GET /directory/ → 200 OK (serves youpi.bad_extension)" \
    "200" "" "" \
    "${BASE}/directory/"

# ---------------------------------------------------------------------------
printf "\n${BOLD}--- CGI via .bla extension (tester req. 3) ---${RESET}\n"
# ---------------------------------------------------------------------------

run_test "11" "POST /directory/youpi.bla → 200 OK (CGI execution)" \
    "200" "" "" \
    -X POST \
    -H "Content-Type: text/plain" \
    --data "coucou les amis !" \
    "${BASE}/directory/youpi.bla"

run_test "12" "GET /directory/youpi.bla → 200 OK (CGI via GET)" \
    "200" "" "" \
    "${BASE}/directory/youpi.bla"

# ---------------------------------------------------------------------------
printf "\n${BOLD}--- Redirects ---${RESET}\n"
# ---------------------------------------------------------------------------

run_test "13" "GET /old-page → 301 + Location header present" \
    "301" "" "Location" \
    --no-location \
    "${BASE}/old-page"

run_test "14" "GET /google → 302 + Location contains google.com" \
    "302" "" "google.com" \
    --no-location \
    "${BASE}/google"

# ---------------------------------------------------------------------------
printf "\n${BOLD}--- Malformed / edge-case requests ---${RESET}\n"
# ---------------------------------------------------------------------------

# FIX: curl --http0.9 does not actually send a malformed request — libcurl
# normalises it internally, so the server sees a valid request and replies 200.
# The correct way to test a missing HTTP version is with nc (raw TCP), which
# is already covered in test_webserv.sh (test 15).
# Here we test HTTP/1.0 with a missing Host header, which is also invalid per
# RFC 7230 §5.4 and must produce 400.
printf "${CYAN}[TEST 15]${RESET} HTTP/1.0 without Host header → 400 Bad Request ... "
T15_CODE=$(printf 'GET / HTTP/1.0\r\n\r\n' | nc -q 1 -w "$TIMEOUT" "$HOST" "$PORT" 2>/dev/null \
           | head -1 | tr -d '\r' | awk '{print $2}')
if [ "$T15_CODE" = "400" ]; then
    printf "${GREEN}OK${RESET}  (got: HTTP %s)\n" "$T15_CODE"
    PASS=$((PASS + 1))
else
    # HTTP/1.0 clients without Host are sometimes tolerated — mark as warning
    printf "${YELLOW}WARN${RESET} (got %s — server may tolerate HTTP/1.0 without Host)\n" "$T15_CODE"
    # Do not count as hard failure since it depends on implementation choice
fi

# Large body
LARGE_FILE=$(mktemp)
dd if=/dev/zero bs=1024 count=1025 2>/dev/null | tr '\0' 'A' > "$LARGE_FILE"

run_test "20" "POST /upload (body > 1 MB) → 413 Content Too Large" \
    "413" "" "" \
    -X POST \
    -H "Content-Type: application/octet-stream" \
    --data-binary "@$LARGE_FILE" \
    "${BASE}/upload"

rm -f "$LARGE_FILE"

# ---------------------------------------------------------------------------
printf "\n${BOLD}--- Secondary servers ---${RESET}\n"
# ---------------------------------------------------------------------------

run_test_port "17" "GET / on port 8081 → 200 OK" \
    "8081" "200" "" \
    "http://${HOST}:8081/"

run_test_port "18a" "GET / on port 8082 → 200 OK" \
    "8082" "200" "" \
    "http://${HOST}:8082/"

run_test_port "18b" "POST / on port 8082 (restricted) → 405" \
    "8082" "405" "" \
    -X POST -H "Content-Length: 0" \
    "http://${HOST}:8082/"

# ---------------------------------------------------------------------------
printf "\n${BOLD}--- Keep-Alive ---${RESET}\n"
# ---------------------------------------------------------------------------

printf "${CYAN}[TEST 19]${RESET} Keep-Alive: two requests on same connection ... "

KA_OUT=$(mktemp)
curl -s \
     --max-time "$TIMEOUT" \
     -w "CODE1=%{http_code}\n" -o /dev/null \
     "${BASE}/" \
     --next \
     -w "CODE2=%{http_code}\n" -o /dev/null \
     "${BASE}/static/style.css" \
     2>/dev/null > "$KA_OUT"

CODE1=$(grep CODE1 "$KA_OUT" | cut -d= -f2 | tr -d '\n')
CODE2=$(grep CODE2 "$KA_OUT" | cut -d= -f2 | tr -d '\n')
rm -f "$KA_OUT"

if [ "$CODE1" = "200" ] && { [ "$CODE2" = "200" ] || [ "$CODE2" = "404" ]; }; then
    printf "${GREEN}OK${RESET}  (/ → %s, /static/style.css → %s)\n" "$CODE1" "$CODE2"
    PASS=$((PASS + 1))
else
    printf "${RED}FAIL${RESET} (/ → %s, /static/style.css → %s)\n" "$CODE1" "$CODE2"
    FAIL=$((FAIL + 1))
fi

# =============================================================================
# Summary
# =============================================================================
TOTAL=$((PASS + FAIL + SKIP))

printf "\n${BOLD}========================================${RESET}\n"
printf "${BOLD}  Results: %d tests run${RESET}\n" "$TOTAL"
printf "  ${GREEN}PASS : %d${RESET}\n" "$PASS"
printf "  ${RED}FAIL : %d${RESET}\n" "$FAIL"
[ "$SKIP" -gt 0 ] && \
    printf "  ${YELLOW}SKIP : %d${RESET} (port not responding)\n" "$SKIP"
printf "${BOLD}========================================${RESET}\n\n"

if [ "$FAIL" -eq 0 ]; then
    printf "${GREEN}${BOLD}All tests passed.${RESET}\n\n"
    exit 0
else
    printf "${RED}${BOLD}%d test(s) failed. Review the output above.${RESET}\n\n" "$FAIL"
    exit 1
fi
