#!/bin/bash
# **************************************************************************** #
#                                                                              #
#    benchmark_tests.sh - Performance & Stress Tests for Webserv               #
#                                                                              #
#    By: fcela-ga <fcela-ga@student.42barcelona.com>                           #
#                                                                              #
#    This is the MAIN benchmark script. For backward compatibility:            #
#    - siege_test.sh calls this script                                         #
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
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Default values
CONCURRENT_USERS=50
DURATION=30
WARMUP_REQUESTS=100
NO_START=0
QUICK_MODE=0

# Counters
PASS_COUNT=0
WARN_COUNT=0
FAIL_COUNT=0

# =============================================================================
# Argument Parsing
# =============================================================================
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -c, --concurrent N  Concurrent users (default: 50)"
    echo "  -t, --time N        Duration in seconds (default: 30)"
    echo "  -q, --quick         Quick mode: 10 users, 10 seconds"
    echo "  -p, --port PORT     Server port (default: 8080)"
    echo "  --no-start          Don't start server (assume running)"
    echo "  -h, --help          Show this help"
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--concurrent) CONCURRENT_USERS="$2"; shift 2 ;;
        -t|--time) DURATION="$2"; shift 2 ;;
        -q|--quick) QUICK_MODE=1; CONCURRENT_USERS=10; DURATION=10; WARMUP_REQUESTS=20; shift ;;
        -p|--port) PORT="$2"; shift 2 ;;
        --no-start) NO_START=1; shift ;;
        -h|--help) show_help; exit 0 ;;
        *) echo "Unknown option: $1"; show_help; exit 1 ;;
    esac
done

# =============================================================================
# Functions
# =============================================================================
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
echo -e "${CYAN}║           Performance & Benchmark Tests                           ║${NC}"
echo -e "${CYAN}╚═══════════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${BLUE}Configuration:${NC}"
echo "  Concurrent users: $CONCURRENT_USERS"
echo "  Duration: ${DURATION}s"
echo "  Port: $PORT"
[ $QUICK_MODE -eq 1 ] && echo -e "  ${YELLOW}Quick mode enabled${NC}"
echo ""

# Check for siege
if ! command -v siege &> /dev/null; then
    echo -e "${RED}Error: siege is not installed${NC}"
    echo "Install with: apt install siege / brew install siege"
    exit 1
fi

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
    echo -e "${GREEN}✅ Server already running${NC}"
fi

# =============================================================================
echo ""
echo -e "${CYAN}🔥 Warmup Phase${NC}"
# =============================================================================
echo "Running $WARMUP_REQUESTS warmup requests..."
for i in $(seq 1 $WARMUP_REQUESTS); do
    curl -s --max-time 5 "http://localhost:$PORT/" > /dev/null 2>&1 &
done
# Esperar máximo 30 segundos para el warmup
timeout 30 bash -c 'wait' 2>/dev/null || echo "Warmup timeout (continuing anyway)"
echo -e "${GREEN}Warmup complete${NC}"

# =============================================================================
echo ""
echo -e "${CYAN}🚀 Siege Stress Test${NC}"
# =============================================================================
echo "Running siege: $CONCURRENT_USERS concurrent users, ${DURATION}s..."
echo ""

# Configure siege
mkdir -p ~/.siege 2>/dev/null
cat > ~/.siege/siege.conf << EOF
verbose = false
quiet = true
json_output = false
show-logfile = false
logging = false
protocol = HTTP/1.1
chunked = true
connection = keep-alive
concurrent = $CONCURRENT_USERS
time = ${DURATION}S
EOF

# Run siege con timeout
timeout 120 siege -b -c $CONCURRENT_USERS -t ${DURATION}S "http://localhost:$PORT/" 2>&1 | tee /tmp/siege_results.txt || true

# =============================================================================
echo ""
echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}                    Benchmark Results                           ${NC}"
echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
# =============================================================================

# Parse results
AVAILABILITY=$(grep "Availability:" /tmp/siege_results.txt 2>/dev/null | awk '{print $2}' | tr -d '%')
TRANSACTIONS=$(grep "Transactions:" /tmp/siege_results.txt 2>/dev/null | awk '{print $2}')
TRANS_RATE=$(grep "Transaction rate:" /tmp/siege_results.txt 2>/dev/null | awk '{print $3}')
RESPONSE_TIME=$(grep "Response time:" /tmp/siege_results.txt 2>/dev/null | awk '{print $3}')
THROUGHPUT=$(grep "Throughput:" /tmp/siege_results.txt 2>/dev/null | awk '{print $2}')
CONCURRENCY=$(grep "Concurrency:" /tmp/siege_results.txt 2>/dev/null | awk '{print $2}')
FAILED=$(grep "Failed transactions:" /tmp/siege_results.txt 2>/dev/null | awk '{print $3}')
LONGEST=$(grep "Longest transaction:" /tmp/siege_results.txt 2>/dev/null | awk '{print $3}')
SHORTEST=$(grep "Shortest transaction:" /tmp/siege_results.txt 2>/dev/null | awk '{print $3}')

echo ""
echo -e "${BLUE}Performance Metrics:${NC}"
echo "  Availability:       ${AVAILABILITY:-N/A}%"
echo "  Total Transactions: ${TRANSACTIONS:-N/A}"
echo "  Transaction Rate:   ${TRANS_RATE:-N/A} trans/sec"
echo "  Response Time:      ${RESPONSE_TIME:-N/A} secs"
echo "  Throughput:         ${THROUGHPUT:-N/A} MB/sec"
echo "  Concurrency:        ${CONCURRENCY:-N/A}"
echo "  Failed:             ${FAILED:-N/A}"
echo "  Longest Trans:      ${LONGEST:-N/A} secs"
echo "  Shortest Trans:     ${SHORTEST:-N/A} secs"

# =============================================================================
echo ""
echo -e "${CYAN}📊 Performance Evaluation${NC}"
# =============================================================================

# Availability check (target: > 99.5%)
if [ -n "$AVAILABILITY" ]; then
    if (( $(echo "$AVAILABILITY >= 99.5" | bc -l 2>/dev/null || echo "0") )); then
        echo -e "  ${GREEN}✅ Availability >= 99.5%${NC} ($AVAILABILITY%)"
        ((PASS_COUNT++))
    elif (( $(echo "$AVAILABILITY >= 95" | bc -l 2>/dev/null || echo "0") )); then
        echo -e "  ${YELLOW}⚠️  Availability 95-99.5%${NC} ($AVAILABILITY%)"
        ((WARN_COUNT++))
    else
        echo -e "  ${RED}❌ Availability < 95%${NC} ($AVAILABILITY%)"
        ((FAIL_COUNT++))
    fi
fi

# Response time check (target: < 1s)
if [ -n "$RESPONSE_TIME" ]; then
    if (( $(echo "$RESPONSE_TIME < 1.0" | bc -l 2>/dev/null || echo "0") )); then
        echo -e "  ${GREEN}✅ Response time < 1s${NC} (${RESPONSE_TIME}s)"
        ((PASS_COUNT++))
    elif (( $(echo "$RESPONSE_TIME < 2.0" | bc -l 2>/dev/null || echo "0") )); then
        echo -e "  ${YELLOW}⚠️  Response time 1-2s${NC} (${RESPONSE_TIME}s)"
        ((WARN_COUNT++))
    else
        echo -e "  ${RED}❌ Response time > 2s${NC} (${RESPONSE_TIME}s)"
        ((FAIL_COUNT++))
    fi
fi

# Transaction rate check
if [ -n "$TRANS_RATE" ]; then
    if (( $(echo "$TRANS_RATE >= 100" | bc -l 2>/dev/null || echo "0") )); then
        echo -e "  ${GREEN}✅ Transaction rate >= 100/sec${NC} (${TRANS_RATE}/sec)"
        ((PASS_COUNT++))
    elif (( $(echo "$TRANS_RATE >= 50" | bc -l 2>/dev/null || echo "0") )); then
        echo -e "  ${YELLOW}⚠️  Transaction rate 50-100/sec${NC} (${TRANS_RATE}/sec)"
        ((WARN_COUNT++))
    else
        echo -e "  ${RED}❌ Transaction rate < 50/sec${NC} (${TRANS_RATE}/sec)"
        ((FAIL_COUNT++))
    fi
fi

# Failed transactions check
if [ -n "$FAILED" ]; then
    if [ "$FAILED" = "0" ]; then
        echo -e "  ${GREEN}✅ No failed transactions${NC}"
        ((PASS_COUNT++))
    elif [ "$FAILED" -lt "10" ]; then
        echo -e "  ${YELLOW}⚠️  Few failed transactions${NC} ($FAILED)"
        ((WARN_COUNT++))
    else
        echo -e "  ${RED}❌ Many failed transactions${NC} ($FAILED)"
        ((FAIL_COUNT++))
    fi
fi

# =============================================================================
echo ""
echo -e "${CYAN}🔍 Post-Stress Validation${NC}"
# =============================================================================

# Server still responsive?
echo -n "  Server responsive after stress: "
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://localhost:$PORT/" 2>/dev/null)
if [ "$STATUS" = "200" ]; then
    echo -e "${GREEN}✅ Yes${NC}"
    ((PASS_COUNT++))
else
    echo -e "${RED}❌ No ($STATUS)${NC}"
    ((FAIL_COUNT++))
fi

# Rapid requests test
echo -n "  Handles rapid requests: "
SUCCESS=0
for i in {1..10}; do
    S=$(curl -s -o /dev/null -w "%{http_code}" --max-time 2 "http://localhost:$PORT/" 2>/dev/null)
    [ "$S" = "200" ] && ((SUCCESS++))
done
if [ $SUCCESS -eq 10 ]; then
    echo -e "${GREEN}✅ 10/10${NC}"
    ((PASS_COUNT++))
else
    echo -e "${YELLOW}⚠️  $SUCCESS/10${NC}"
    ((WARN_COUNT++))
fi

# =============================================================================
echo ""
echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}                    Summary                                     ${NC}"
echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "  ${GREEN}Passed:${NC}   $PASS_COUNT"
echo -e "  ${YELLOW}Warnings:${NC} $WARN_COUNT"
echo -e "  ${RED}Failed:${NC}   $FAIL_COUNT"
echo ""

# Cleanup
rm -f /tmp/siege_results.txt

if [ $FAIL_COUNT -gt 0 ]; then
    echo -e "${RED}❌ Performance issues detected${NC}"
    exit 1
elif [ $WARN_COUNT -gt 0 ]; then
    echo -e "${YELLOW}⚠️  Performance acceptable with warnings${NC}"
    exit 0
else
    echo -e "${GREEN}✅ Performance test passed!${NC}"
    exit 0
fi
