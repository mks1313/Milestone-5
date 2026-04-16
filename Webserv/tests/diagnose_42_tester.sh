#!/bin/bash
# ============================================================================
# diagnose_42_tester.sh
#
# Diagnóstico para verificar que el servidor cumple los requisitos del 42 tester
#
# Requisitos del 42 tester:
#   - / must answer to GET request ONLY
#   - /put_test/* must answer to PUT request and save files
#   - any file with .bla extension must answer to POST by calling cgi_test
#   - /post_body must answer to POST with maxBody of 100
#   - /directory/ must serve YoupiBanane with index youpi.bad_extension
#
# Uso: ./diagnose_42_tester.sh [port]
# ============================================================================

PORT=${1:-8080}
BASE_URL="http://localhost:$PORT"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║         42 Tester Requirements Diagnostic                     ║${NC}"
echo -e "${CYAN}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Check if server is running
if ! nc -z localhost $PORT 2>/dev/null; then
    echo -e "${RED}❌ Server not running on port $PORT${NC}"
    echo "   Start the server first: ./bin/webserv config/webserv.conf"
    exit 1
fi

echo -e "${GREEN}✓ Server is running on port $PORT${NC}"
echo ""

PASSED=0
FAILED=0

test_endpoint() {
    local method="$1"
    local url="$2"
    local expected="$3"
    local description="$4"
    
    local status
    case "$method" in
        GET)
            status=$(curl -s -o /dev/null -w "%{http_code}" "$url" 2>/dev/null)
            ;;
        POST)
            status=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "" "$url" 2>/dev/null)
            ;;
        PUT)
            status=$(curl -s -o /dev/null -w "%{http_code}" -X PUT -d "test" "$url" 2>/dev/null)
            ;;
        DELETE)
            status=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$url" 2>/dev/null)
            ;;
    esac
    
    if [ "$status" = "$expected" ]; then
        echo -e "  ${GREEN}✅ PASS:${NC} $description"
        echo -e "         $method $url → $status (expected $expected)"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}❌ FAIL:${NC} $description"
        echo -e "         $method $url → $status (expected $expected)"
        FAILED=$((FAILED + 1))
    fi
}

# ============================================================================
# Test 1: / must answer to GET request ONLY
# ============================================================================
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}  Test 1: / must answer to GET request ONLY${NC}"
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"

test_endpoint "GET" "$BASE_URL/" "200" "GET / should return 200"
test_endpoint "POST" "$BASE_URL/" "405" "POST / should return 405 (Method Not Allowed)"
test_endpoint "PUT" "$BASE_URL/" "405" "PUT / should return 405"
test_endpoint "DELETE" "$BASE_URL/" "405" "DELETE / should return 405"

# ============================================================================
# Test 2: /put_test/* must answer to PUT request
# ============================================================================
echo ""
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}  Test 2: /put_test/* must answer to PUT request${NC}"
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"

# PUT should work (200 or 201)
status=$(curl -s -o /dev/null -w "%{http_code}" -X PUT -d "test content" "$BASE_URL/put_test/testfile.txt" 2>/dev/null)
if [ "$status" = "200" ] || [ "$status" = "201" ]; then
    echo -e "  ${GREEN}✅ PASS:${NC} PUT /put_test/testfile.txt → $status"
    PASSED=$((PASSED + 1))
else
    echo -e "  ${RED}❌ FAIL:${NC} PUT /put_test/testfile.txt → $status (expected 200 or 201)"
    FAILED=$((FAILED + 1))
fi

# GET should NOT work on /put_test (it's PUT only)
# Note: Some implementations might return 405 or 403

# ============================================================================
# Test 3: /post_body must answer to POST with maxBody of 100
# ============================================================================
echo ""
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}  Test 3: /post_body must answer to POST with maxBody of 100${NC}"
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"

# Small body should work
status=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "small" "$BASE_URL/post_body" 2>/dev/null)
if [ "$status" = "200" ] || [ "$status" = "204" ]; then
    echo -e "  ${GREEN}✅ PASS:${NC} POST /post_body (small body) → $status"
    PASSED=$((PASSED + 1))
else
    echo -e "  ${RED}❌ FAIL:${NC} POST /post_body (small body) → $status (expected 200 or 204)"
    FAILED=$((FAILED + 1))
fi

# Large body (>100 bytes) should fail with 413
large_body=$(printf 'A%.0s' {1..150})
status=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$large_body" "$BASE_URL/post_body" 2>/dev/null)
if [ "$status" = "413" ]; then
    echo -e "  ${GREEN}✅ PASS:${NC} POST /post_body (large body >100) → $status (413 Payload Too Large)"
    PASSED=$((PASSED + 1))
else
    echo -e "  ${RED}❌ FAIL:${NC} POST /post_body (large body >100) → $status (expected 413)"
    FAILED=$((FAILED + 1))
fi

# ============================================================================
# Test 4: /directory/ must serve YoupiBanane with index youpi.bad_extension
# ============================================================================
echo ""
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}  Test 4: /directory/ must serve YoupiBanane${NC}"
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"

test_endpoint "GET" "$BASE_URL/directory/" "200" "GET /directory/ should return 200"

# Check if it returns the content of youpi.bad_extension
content=$(curl -s "$BASE_URL/directory/" 2>/dev/null)
if echo "$content" | grep -qi "bad_extension\|youpi\|default"; then
    echo -e "  ${GREEN}✅ PASS:${NC} /directory/ serves youpi.bad_extension content"
    PASSED=$((PASSED + 1))
else
    echo -e "  ${YELLOW}⚠️  CHECK:${NC} /directory/ content may not be from youpi.bad_extension"
    echo -e "         Content preview: $(echo "$content" | head -c 100)..."
fi

# ============================================================================
# Test 5: .bla files should trigger CGI
# ============================================================================
echo ""
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}  Test 5: .bla files should trigger CGI (POST)${NC}"
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"

# Check if YoupiBanane/youpi.bla exists and is accessible via CGI
if [ -f "YoupiBanane/youpi.bla" ]; then
    echo -e "  ${GREEN}✓${NC} YoupiBanane/youpi.bla exists"
else
    echo -e "  ${RED}✗${NC} YoupiBanane/youpi.bla not found - run: make setup-42-env"
fi

if [ -x "testers/ubuntu_cgi_tester" ] || [ -x "testers/cgi_tester" ]; then
    echo -e "  ${GREEN}✓${NC} CGI tester binary available"
else
    echo -e "  ${RED}✗${NC} CGI tester binary not found in testers/"
fi

# Test POST to .bla file
status=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/directory/youpi.bla" 2>/dev/null)
echo -e "  POST /directory/youpi.bla → $status"
if [ "$status" = "200" ]; then
    echo -e "  ${GREEN}✅ PASS:${NC} .bla CGI execution works"
    PASSED=$((PASSED + 1))
else
    echo -e "  ${YELLOW}⚠️  CHECK:${NC} .bla CGI may not be configured correctly"
    FAILED=$((FAILED + 1))
fi

# ============================================================================
# Summary
# ============================================================================
echo ""
echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}  Summary${NC}"
echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "  Passed: ${GREEN}$PASSED${NC}"
echo -e "  Failed: ${RED}$FAILED${NC}"
echo ""

if [ $FAILED -gt 0 ]; then
    echo -e "${RED}❌ Some tests failed. Fix these issues before running the 42 tester.${NC}"
    echo ""
    echo -e "${YELLOW}Common fixes:${NC}"
    echo "  1. Check webserv.conf - ensure / only allows GET method"
    echo "  2. Ensure /put_test allows PUT method"
    echo "  3. Ensure /post_body has client_max_body_size 100"
    echo "  4. Ensure /directory serves YoupiBanane with index youpi.bad_extension"
    echo "  5. Ensure .bla files are configured for CGI with cgi_tester"
    exit 1
else
    echo -e "${GREEN}✅ All diagnostic tests passed!${NC}"
    echo ""
    echo "You can now run: make test-42"
    exit 0
fi
