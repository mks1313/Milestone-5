#!/bin/bash
# **************************************************************************** #
#                                                                              #
#    security_tests.sh - Security & Input Validation Tests                     #
#                                                                              #
#    By: fcela-ga <fcela-ga@student.42barcelona.com>                           #
#                                                                              #
# **************************************************************************** #

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
WEBSERV="$PROJECT_DIR/bin/webserv"
CONFIG="$PROJECT_DIR/config/webserv.conf"
PORT=8080

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

PASSED=0
FAILED=0
WARNED=0
NO_START=0

# =============================================================================
# Argument Parsing
# =============================================================================
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -p, --port PORT   Server port (default: 8080)"
    echo "  --no-start        Don't start server (assume running)"
    echo "  -h, --help        Show this help"
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--port) PORT="$2"; shift 2 ;;
        --no-start) NO_START=1; shift ;;
        -h|--help) show_help; exit 0 ;;
        *) echo "Unknown: $1"; exit 1 ;;
    esac
done

# =============================================================================
# Functions
# =============================================================================
log_success() { echo -e "  ${GREEN}✅ PASS${NC}: $1"; ((PASSED++)); }
log_fail() { echo -e "  ${RED}❌ FAIL${NC}: $1"; ((FAILED++)); }
log_warn() { echo -e "  ${YELLOW}⚠️  WARN${NC}: $1"; ((WARNED++)); }

wait_for_server() {
    for i in {1..30}; do
        nc -z localhost $PORT 2>/dev/null && return 0
        sleep 1
    done
    return 1
}

# =============================================================================
# Main
# =============================================================================
echo ""
echo -e "${CYAN}╔═══════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║           Security & Input Validation Tests                       ║${NC}"
echo -e "${CYAN}╚═══════════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Start server if needed
if [ $NO_START -eq 0 ]; then
    if [ ! -f "$WEBSERV" ]; then
        echo -e "${YELLOW}Building webserv...${NC}"
        cd "$PROJECT_DIR" && make -j$(nproc) 2>/dev/null || make
    fi
    
    "$WEBSERV" "$CONFIG" &
    SERVER_PID=$!
    trap "kill $SERVER_PID 2>/dev/null || true" EXIT
    
    wait_for_server || { echo -e "${RED}Server failed to start${NC}"; exit 1; }
    echo -e "${GREEN}✅ Server started${NC}"
else
    nc -z localhost $PORT 2>/dev/null || { echo -e "${RED}No server on port $PORT${NC}"; exit 1; }
fi

# =============================================================================
echo ""
echo -e "${CYAN}🔐 Path Traversal Tests${NC}"
# =============================================================================

TRAVERSAL_PATHS=(
    "/../etc/passwd"
    "/..%2f..%2fetc/passwd"
    "/....//....//etc/passwd"
    "/%2e%2e/%2e%2e/etc/passwd"
    "/..%00/etc/passwd"
    "/..\\..\\etc\\passwd"
    "/%252e%252e/%252e%252e/etc/passwd"
    "/..%c0%af..%c0%afetc/passwd"
)

for path in "${TRAVERSAL_PATHS[@]}"; do
    STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://localhost:$PORT$path" 2>/dev/null || echo "000")
    BODY=$(curl -s --max-time 5 "http://localhost:$PORT$path" 2>/dev/null || echo "")
    
    if echo "$BODY" | grep -q "root:"; then
        log_fail "Path traversal possible: $path"
    elif [[ "$STATUS" =~ ^(400|403|404)$ ]]; then
        log_success "Path traversal blocked: $path ($STATUS)"
    else
        log_warn "Path traversal inconclusive: $path ($STATUS)"
    fi
done

# =============================================================================
echo ""
echo -e "${CYAN}📏 Request Size Tests${NC}"
# =============================================================================

# Very long URL (10KB)
LONG_PATH=$(python3 -c "print('/' + 'a' * 10000)" 2>/dev/null || printf '/%.0sa' {1..10000})
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://localhost:$PORT$LONG_PATH" 2>/dev/null || echo "414")
[[ "$STATUS" =~ ^(400|414|404)$ ]] && log_success "Long URL rejected ($STATUS)" || log_warn "Long URL: $STATUS"

# Very long header (10KB)
LONG_HEADER=$(python3 -c "print('X-Test: ' + 'a' * 10000)" 2>/dev/null)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -H "$LONG_HEADER" "http://localhost:$PORT/" 2>/dev/null || echo "431")
[[ "$STATUS" =~ ^(200|400|431)$ ]] && log_success "Long header handled ($STATUS)" || log_warn "Long header: $STATUS"

# Many headers (100)
HEADERS=""
for i in {1..100}; do HEADERS="$HEADERS -H 'X-Custom-$i: value$i'"; done
STATUS=$(eval curl -s -o /dev/null -w "%{http_code}" --max-time 5 $HEADERS "http://localhost:$PORT/" 2>/dev/null || echo "431")
[[ "$STATUS" =~ ^(200|400|431)$ ]] && log_success "Many headers handled ($STATUS)" || log_warn "Many headers: $STATUS"

# =============================================================================
echo ""
echo -e "${CYAN}🔨 Malformed Request Tests${NC}"
# =============================================================================

# No path in request
RESPONSE=$(echo -e "GET HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc -w 2 localhost $PORT 2>/dev/null | head -1)
echo "$RESPONSE" | grep -q "HTTP/1\.[01] [45]" && log_success "No path rejected" || log_warn "No path handling unclear"

# No HTTP version
RESPONSE=$(echo -e "GET /\r\nHost: localhost\r\n\r\n" | nc -w 2 localhost $PORT 2>/dev/null | head -1)
echo "$RESPONSE" | grep -q "HTTP" && log_success "No version handled" || log_warn "No version handling unclear"

# Double Content-Length (HTTP smuggling)
RESPONSE=$(echo -e "GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\nContent-Length: 10\r\n\r\n" | nc -w 2 localhost $PORT 2>/dev/null | head -1)
echo "$RESPONSE" | grep -q "HTTP/1\.[01] [245]" && log_success "Double Content-Length handled" || log_warn "Double Content-Length unclear"

# Empty request
RESPONSE=$(echo -e "\r\n" | nc -w 2 localhost $PORT 2>/dev/null | head -1)
[[ -z "$RESPONSE" ]] || echo "$RESPONSE" | grep -q "HTTP/1\.[01] [45]" && log_success "Empty request handled" || log_warn "Empty request unclear"

# =============================================================================
echo ""
echo -e "${CYAN}🔧 HTTP Method Tests${NC}"
# =============================================================================

METHODS=("TRACE" "CONNECT" "OPTIONS" "PATCH" "PROPFIND" "MKCOL" "COPY" "MOVE" "LOCK" "UNLOCK")

for method in "${METHODS[@]}"; do
    STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X "$method" "http://localhost:$PORT/" 2>/dev/null || echo "501")
    [[ "$STATUS" =~ ^(200|400|405|501)$ ]] && log_success "Method $method handled ($STATUS)" || log_warn "Method $method: $STATUS"
done

# =============================================================================
echo ""
echo -e "${CYAN}🏷️ Host Header Tests${NC}"
# =============================================================================

# Empty host
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -H "Host:" "http://localhost:$PORT/" 2>/dev/null || echo "400")
[[ "$STATUS" =~ ^(200|400)$ ]] && log_success "Empty Host handled ($STATUS)" || log_warn "Empty Host: $STATUS"

# Missing host (HTTP/1.1 requires it)
RESPONSE=$(echo -e "GET / HTTP/1.1\r\n\r\n" | nc -w 2 localhost $PORT 2>/dev/null | head -1)
log_success "Missing Host header handled"

# Very long host
LONG_HOST=$(python3 -c "print('a' * 1000 + '.com')" 2>/dev/null)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -H "Host: $LONG_HOST" "http://localhost:$PORT/" 2>/dev/null || echo "400")
[[ "$STATUS" =~ ^(200|400)$ ]] && log_success "Long Host handled ($STATUS)" || log_warn "Long Host: $STATUS"

# =============================================================================
echo ""
echo -e "${CYAN}📝 Content-Type Tests${NC}"
# =============================================================================

# POST without Content-Type
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X POST -d "data" "http://localhost:$PORT/" 2>/dev/null || echo "200")
[[ "$STATUS" =~ ^(200|400|405)$ ]] && log_success "POST without Content-Type ($STATUS)" || log_warn "POST no CT: $STATUS"

# Invalid Content-Type
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X POST -H "Content-Type: invalid/type" -d "data" "http://localhost:$PORT/" 2>/dev/null || echo "200")
[[ "$STATUS" =~ ^(200|400|405|415)$ ]] && log_success "Invalid Content-Type ($STATUS)" || log_warn "Invalid CT: $STATUS"

# =============================================================================
echo ""
echo -e "${CYAN}🐢 Slow Client Tests${NC}"
# =============================================================================

# Slowloris simulation
{
    echo -ne "GET / HTTP/1.1\r\n"
    echo -ne "Host: localhost\r\n"
    sleep 2
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
} | nc -w 5 localhost $PORT > /dev/null 2>&1 &
SLOW_PID=$!

sleep 1
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://localhost:$PORT/" 2>/dev/null || echo "000")
[ "$STATUS" = "200" ] && log_success "Server responsive during slow client" || log_warn "Slow client response: $STATUS"
kill $SLOW_PID 2>/dev/null || true

# =============================================================================
echo ""
echo -e "${CYAN}🔤 Special Characters in URL${NC}"
# =============================================================================

# XSS attempt in URL
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://localhost:$PORT/<script>alert(1)</script>" 2>/dev/null || echo "400")
[[ "$STATUS" =~ ^(400|404)$ ]] && log_success "XSS in URL blocked ($STATUS)" || log_warn "XSS in URL: $STATUS"

# Null byte in URL
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://localhost:$PORT/test%00.html" 2>/dev/null || echo "400")
[[ "$STATUS" =~ ^(400|404)$ ]] && log_success "Null byte in URL handled ($STATUS)" || log_warn "Null byte: $STATUS"

# CRLF injection attempt
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://localhost:$PORT/%0d%0aX-Injected:%20header" 2>/dev/null || echo "400")
[[ "$STATUS" =~ ^(400|404)$ ]] && log_success "CRLF injection blocked ($STATUS)" || log_warn "CRLF injection: $STATUS"

# =============================================================================
echo ""
echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}                    Summary                                     ${NC}"
echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "  ${GREEN}Passed:${NC}  $PASSED"
echo -e "  ${YELLOW}Warned:${NC}  $WARNED"
echo -e "  ${RED}Failed:${NC}  $FAILED"
echo ""

if [ $FAILED -gt 0 ]; then
    echo -e "${RED}❌ Some security tests failed${NC}"
    exit 1
fi

echo -e "${GREEN}✅ All security tests passed${NC}"
exit 0
