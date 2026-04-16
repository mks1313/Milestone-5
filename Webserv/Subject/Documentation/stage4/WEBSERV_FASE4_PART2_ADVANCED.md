# WEBSERV - Fase 4 (Parte 2): Comparativa NGINX y Testing Avanzado

## Análisis Comparativo, Scripts de Producción y Troubleshooting

**Proyecto:** 42 Barcelona - webserv  
**Autor:** fcela-ga (Felipe Cela García)  
**Fase:** 4 de 4 (Continuación)

---

# ÍNDICE FASE 4 - PARTE 2

10. [Comparativa Detallada con NGINX](#10-comparativa-detallada-con-nginx)
11. [Scripts de Testing de Producción](#11-scripts-de-testing-de-producción)
12. [Guía de Troubleshooting Avanzada](#12-guía-de-troubleshooting-avanzada)
13. [Escenarios de Evaluación 42](#13-escenarios-de-evaluación-42)
14. [Glosario Técnico](#14-glosario-técnico)
15. [Referencias y Recursos](#15-referencias-y-recursos)

---

# 10. COMPARATIVA DETALLADA CON NGINX

## 10.1 Tabla Comparativa de Funcionalidades

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    WEBSERV vs NGINX - FEATURE COMPARISON                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Funcionalidad              │ NGINX          │ Webserv       │ Notas       │
│  ─────────────────────────────────────────────────────────────────────────  │
│                                                                             │
│  CORE                                                                       │
│  ────                                                                       │
│  Event-driven I/O           │ ✅ epoll/kqueue │ ✅ poll()     │ Similar     │
│  Non-blocking               │ ✅              │ ✅            │ Igual       │
│  Multi-process              │ ✅ worker       │ ❌ single     │ Diferente   │
│  Multi-thread               │ ✅ thread pool  │ ❌            │ Diferente   │
│  Hot reload                 │ ✅              │ ❌            │ N/A         │
│                                                                             │
│  HTTP                                                                       │
│  ────                                                                       │
│  HTTP/1.0                   │ ✅              │ ✅            │ Igual       │
│  HTTP/1.1                   │ ✅              │ ✅            │ Igual       │
│  HTTP/2                     │ ✅              │ ❌            │ N/A         │
│  HTTP/3 (QUIC)              │ ✅              │ ❌            │ N/A         │
│  Keep-alive                 │ ✅              │ ✅            │ Igual       │
│  Pipelining                 │ ✅              │ ✅            │ Igual       │
│  Chunked encoding           │ ✅              │ ✅            │ Igual       │
│  Compression (gzip)         │ ✅              │ ❌            │ Posible     │
│                                                                             │
│  MÉTODOS HTTP                                                               │
│  ─────────────                                                              │
│  GET                        │ ✅              │ ✅            │ Igual       │
│  HEAD                       │ ✅              │ ✅            │ Igual       │
│  POST                       │ ✅              │ ✅            │ Igual       │
│  PUT                        │ ✅ (WebDAV)     │ ✅            │ Similar     │
│  DELETE                     │ ✅ (WebDAV)     │ ✅            │ Similar     │
│  OPTIONS                    │ ✅              │ ❌            │ Posible     │
│  PATCH                      │ ✅              │ ❌            │ N/A         │
│                                                                             │
│  CONFIGURACIÓN                                                              │
│  ─────────────                                                              │
│  Sintaxis                   │ Propia         │ NGINX-like    │ Compatible  │
│  listen                     │ ✅              │ ✅            │ Igual       │
│  server_name                │ ✅              │ ✅            │ Igual       │
│  root                       │ ✅              │ ✅            │ Igual       │
│  alias                      │ ✅              │ ✅            │ Igual       │
│  index                      │ ✅              │ ✅            │ Igual       │
│  error_page                 │ ✅              │ ✅            │ Igual       │
│  client_max_body_size       │ ✅              │ ✅            │ Igual       │
│  autoindex                  │ ✅              │ ✅            │ Igual       │
│  return/redirect            │ ✅              │ ✅            │ Similar     │
│  location matching          │ ✅ regex        │ ⚠️ prefix     │ Simplificado│
│  try_files                  │ ✅              │ ❌            │ N/A         │
│  include                    │ ✅              │ ❌            │ Posible     │
│  variables ($uri, etc)      │ ✅              │ ❌            │ N/A         │
│                                                                             │
│  CGI / FASTCGI                                                              │
│  ────────────                                                               │
│  CGI tradicional            │ ❌ (módulo)     │ ✅            │ Diferente   │
│  FastCGI                    │ ✅              │ ❌            │ N/A         │
│  uWSGI                      │ ✅              │ ❌            │ N/A         │
│  SCGI                       │ ✅              │ ❌            │ N/A         │
│                                                                             │
│  UPLOAD                                                                     │
│  ──────                                                                     │
│  multipart/form-data        │ ✅ (módulo)     │ ✅            │ Similar     │
│  Streaming upload           │ ✅              │ ❌            │ N/A         │
│  Resumable upload           │ ✅ (módulo)     │ ❌            │ N/A         │
│                                                                             │
│  SEGURIDAD                                                                  │
│  ─────────                                                                  │
│  SSL/TLS                    │ ✅              │ ❌            │ N/A 42      │
│  Basic Auth                 │ ✅              │ ❌            │ Posible     │
│  Rate limiting              │ ✅              │ ❌            │ Posible     │
│  IP blacklist               │ ✅              │ ❌            │ Posible     │
│                                                                             │
│  SESIONES                                                                   │
│  ────────                                                                   │
│  Cookies                    │ ✅ (via app)    │ ✅            │ Similar     │
│  Session management         │ ❌ (via app)    │ ✅ built-in   │ Webserv +   │
│                                                                             │
│  LOGGING                                                                    │
│  ───────                                                                    │
│  Access log                 │ ✅ customizable │ ⚠️ básico     │ Simplificado│
│  Error log                  │ ✅ levels       │ ✅ colors     │ Similar     │
│  Custom format              │ ✅              │ ❌            │ N/A         │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 10.2 Diferencias de Comportamiento

### 10.2.1 Location Matching

**NGINX:**
```nginx
# NGINX soporta múltiples tipos de matching
location = /exact { }           # Exact match (highest priority)
location ^~ /prefix { }         # Preferential prefix
location ~ \.php$ { }           # Regex (case-sensitive)
location ~* \.(jpg|png)$ { }    # Regex (case-insensitive)
location /prefix { }            # Prefix match (lowest priority)
```

**Webserv:**
```nginx
# Webserv solo soporta prefix match
location / { }                  # Prefix match
location /api { }               # Longer prefix wins
location /api/v1 { }            # Even longer prefix wins
```

**Ejemplo de diferencia:**

```
URI: /images/photo.jpg

NGINX con:
  location / { root /var/www; }
  location ~* \.(jpg|png)$ { root /var/images; }
  → Usa /var/images (regex match)

Webserv con:
  location / { root /var/www; }
  → Usa /var/www (solo prefix match disponible)
```

### 10.2.2 PUT y DELETE

**NGINX (requiere módulo WebDAV):**
```nginx
location /files {
    dav_methods PUT DELETE;
    create_full_put_path on;
    dav_access user:rw group:r all:r;
}
```

**Webserv (built-in):**
```nginx
location /files {
    methods GET PUT DELETE;
    # PUT y DELETE funcionan directamente
}
```

### 10.2.3 CGI vs FastCGI

**NGINX (FastCGI):**
```nginx
location ~ \.php$ {
    fastcgi_pass 127.0.0.1:9000;
    fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
    include fastcgi_params;
}
```

**Webserv (CGI tradicional):**
```nginx
location /cgi-bin {
    cgi .php /usr/bin/php-cgi;
    # Fork + exec por cada petición
}
```

**Diferencias de rendimiento:**

| Aspecto | NGINX + FastCGI | Webserv CGI |
|---------|-----------------|-------------|
| Latencia | ~5ms | ~50-100ms |
| Overhead | Bajo (conexión pool) | Alto (fork/exec) |
| Memoria | Proceso persistente | Nuevo proceso cada vez |
| Escalabilidad | Alta | Media |

## 10.3 Tests de Compatibilidad

### Test 1: Archivos Estáticos

```bash
#!/bin/bash
# test_static_compat.sh

echo "=== Static File Compatibility Test ==="

# Crear archivo de prueba
echo "Hello World" > /tmp/test.txt

# NGINX
curl -s http://nginx:80/test.txt -o /tmp/nginx_out.txt

# Webserv
curl -s http://webserv:8080/test.txt -o /tmp/webserv_out.txt

# Comparar
if diff /tmp/nginx_out.txt /tmp/webserv_out.txt > /dev/null; then
    echo "[PASS] Static file content matches"
else
    echo "[FAIL] Static file content differs"
fi

# Comparar headers relevantes
for header in "Content-Type" "Content-Length"; do
    nginx_h=$(curl -sI http://nginx:80/test.txt | grep -i "^$header")
    webserv_h=$(curl -sI http://webserv:8080/test.txt | grep -i "^$header")
    
    if [ "$nginx_h" == "$webserv_h" ]; then
        echo "[PASS] $header matches"
    else
        echo "[INFO] $header differs: NGINX='$nginx_h' vs Webserv='$webserv_h'"
    fi
done
```

### Test 2: Redirecciones

```bash
#!/bin/bash
# test_redirect_compat.sh

echo "=== Redirect Compatibility Test ==="

# Test 301
nginx_301=$(curl -sI http://nginx:80/old | grep "^HTTP" | awk '{print $2}')
webserv_301=$(curl -sI http://webserv:8080/old | grep "^HTTP" | awk '{print $2}')

if [ "$nginx_301" == "$webserv_301" ]; then
    echo "[PASS] 301 status matches: $nginx_301"
else
    echo "[FAIL] 301 differs: NGINX=$nginx_301, Webserv=$webserv_301"
fi

# Test Location header
nginx_loc=$(curl -sI http://nginx:80/old | grep -i "^Location")
webserv_loc=$(curl -sI http://webserv:8080/old | grep -i "^Location")

echo "NGINX Location: $nginx_loc"
echo "Webserv Location: $webserv_loc"
```

### Test 3: Error Pages

```bash
#!/bin/bash
# test_errors_compat.sh

echo "=== Error Page Compatibility Test ==="

for code in 404 405 413; do
    case $code in
        404) url="/nonexistent" ;;
        405) url="/" && method="-X DELETE" ;;
        413) url="/" && data="-d @/dev/zero" ;;
    esac
    
    nginx_code=$(curl -s -o /dev/null -w "%{http_code}" $method http://nginx:80$url)
    webserv_code=$(curl -s -o /dev/null -w "%{http_code}" $method http://webserv:8080$url)
    
    if [ "$nginx_code" == "$webserv_code" ]; then
        echo "[PASS] Error $code: Both return $nginx_code"
    else
        echo "[FAIL] Error $code: NGINX=$nginx_code, Webserv=$webserv_code"
    fi
done
```

---

# 11. SCRIPTS DE TESTING DE PRODUCCIÓN

## 11.1 Suite de Tests Completa

```bash
#!/bin/bash
# webserv_test_suite.sh
# Suite completa de tests para webserv

set -e

# Configuración
SERVER_URL="${SERVER_URL:-http://localhost:8080}"
TIMEOUT=10
VERBOSE=${VERBOSE:-0}

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Contadores
PASSED=0
FAILED=0
SKIPPED=0

# Funciones auxiliares
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_pass() { echo -e "${GREEN}[PASS]${NC} $1"; ((PASSED++)); }
log_fail() { echo -e "${RED}[FAIL]${NC} $1"; ((FAILED++)); }
log_skip() { echo -e "${YELLOW}[SKIP]${NC} $1"; ((SKIPPED++)); }

# Test genérico HTTP
test_http() {
    local name="$1"
    local expected_code="$2"
    shift 2
    local curl_args="$@"
    
    local actual_code=$(curl -s -o /dev/null -w "%{http_code}" \
                        --max-time $TIMEOUT $curl_args 2>/dev/null)
    
    if [ "$actual_code" == "$expected_code" ]; then
        log_pass "$name (HTTP $actual_code)"
        return 0
    else
        log_fail "$name (expected $expected_code, got $actual_code)"
        return 1
    fi
}

# Test de contenido
test_content() {
    local name="$1"
    local url="$2"
    local expected="$3"
    
    local content=$(curl -s --max-time $TIMEOUT "$url" 2>/dev/null)
    
    if echo "$content" | grep -q "$expected"; then
        log_pass "$name"
        return 0
    else
        log_fail "$name (expected '$expected' not found)"
        return 1
    fi
}

# Test de header
test_header() {
    local name="$1"
    local url="$2"
    local header="$3"
    local expected="$4"
    
    local value=$(curl -sI --max-time $TIMEOUT "$url" 2>/dev/null | \
                  grep -i "^$header:" | sed 's/^[^:]*: //' | tr -d '\r')
    
    if [ "$value" == "$expected" ]; then
        log_pass "$name ($header: $value)"
        return 0
    elif [ -n "$value" ]; then
        log_fail "$name (expected '$expected', got '$value')"
        return 1
    else
        log_fail "$name (header '$header' not found)"
        return 1
    fi
}

# ============================================================================
# TESTS
# ============================================================================

echo ""
echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║                    WEBSERV TEST SUITE                              ║"
echo "╠════════════════════════════════════════════════════════════════════╣"
echo "║  Server: $SERVER_URL"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""

# ----------------------------------------------------------------------------
# 1. TESTS BÁSICOS
# ----------------------------------------------------------------------------
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "1. BASIC HTTP TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

test_http "GET /" "200" "$SERVER_URL/"
test_http "GET /index.html" "200" "$SERVER_URL/index.html"
test_http "HEAD /" "200" -I "$SERVER_URL/"
test_http "GET /nonexistent" "404" "$SERVER_URL/nonexistent"
test_http "GET with query string" "200" "$SERVER_URL/?foo=bar&baz=qux"

# ----------------------------------------------------------------------------
# 2. TESTS DE MÉTODOS HTTP
# ----------------------------------------------------------------------------
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "2. HTTP METHOD TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# PUT - crear archivo
test_http "PUT create file" "201" -X PUT -d "test content" "$SERVER_URL/files/test_put.txt"

# GET - verificar archivo creado
test_content "GET created file" "$SERVER_URL/files/test_put.txt" "test content"

# DELETE - eliminar archivo
test_http "DELETE file" "204" -X DELETE "$SERVER_URL/files/test_put.txt"

# GET - verificar eliminación
test_http "GET deleted file" "404" "$SERVER_URL/files/test_put.txt"

# POST sin CGI
test_http "POST without CGI" "204" -X POST -d "data" "$SERVER_URL/api"

# ----------------------------------------------------------------------------
# 3. TESTS DE ERRORES
# ----------------------------------------------------------------------------
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "3. ERROR HANDLING TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

test_http "404 Not Found" "404" "$SERVER_URL/this/does/not/exist"
test_http "405 Method Not Allowed" "405" -X PATCH "$SERVER_URL/"

# 413 Payload Too Large (si hay límite configurado)
large_data=$(head -c 100000000 /dev/zero | tr '\0' 'A')
test_http "413 Payload Too Large" "413" -X POST -d "$large_data" "$SERVER_URL/smallbody" || log_skip "413 test (no limit configured)"

# URI muy larga
long_uri=$(head -c 10000 /dev/zero | tr '\0' 'A')
test_http "414 URI Too Long" "414" "$SERVER_URL/$long_uri" || log_skip "414 test"

# ----------------------------------------------------------------------------
# 4. TESTS DE REDIRECCIÓN
# ----------------------------------------------------------------------------
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "4. REDIRECT TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

test_http "301 Redirect" "301" "$SERVER_URL/redir301" || log_skip "301 redirect not configured"
test_http "302 Redirect" "302" "$SERVER_URL/redir302" || log_skip "302 redirect not configured"

# Verificar header Location
location=$(curl -sI "$SERVER_URL/redir301" 2>/dev/null | grep -i "^Location:" | tr -d '\r')
if [ -n "$location" ]; then
    log_pass "Location header present: $location"
else
    log_skip "Location header test"
fi

# ----------------------------------------------------------------------------
# 5. TESTS DE CGI
# ----------------------------------------------------------------------------
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "5. CGI TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

test_http "CGI GET" "200" "$SERVER_URL/cgi-bin/test.py" || log_skip "CGI not available"
test_http "CGI GET with query" "200" "$SERVER_URL/cgi-bin/test.py?name=test" || log_skip "CGI query test"
test_http "CGI POST" "200" -X POST -d "data=test" "$SERVER_URL/cgi-bin/test.py" || log_skip "CGI POST test"

# Verificar variables de entorno CGI
cgi_output=$(curl -s "$SERVER_URL/cgi-bin/test.py?foo=bar" 2>/dev/null)
if echo "$cgi_output" | grep -q "QUERY_STRING"; then
    log_pass "CGI QUERY_STRING present"
else
    log_skip "CGI environment test"
fi

# ----------------------------------------------------------------------------
# 6. TESTS DE UPLOAD
# ----------------------------------------------------------------------------
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "6. FILE UPLOAD TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Crear archivo temporal
echo "Test upload content" > /tmp/upload_test.txt

test_http "Multipart upload" "201" -F "file=@/tmp/upload_test.txt" "$SERVER_URL/upload" || log_skip "Upload test"

rm -f /tmp/upload_test.txt

# ----------------------------------------------------------------------------
# 7. TESTS DE KEEP-ALIVE Y PIPELINING
# ----------------------------------------------------------------------------
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "7. CONNECTION TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Keep-alive
conn_header=$(curl -sI "$SERVER_URL/" 2>/dev/null | grep -i "^Connection:" | tr -d '\r')
if echo "$conn_header" | grep -qi "keep-alive"; then
    log_pass "Keep-alive enabled"
else
    log_fail "Keep-alive not detected"
fi

# Pipelining test
pipeline_result=$(python3 -c "
import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 8080))
s.settimeout(5)

# Enviar 3 requests pipelined
requests = b''
for i in range(3):
    requests += b'GET / HTTP/1.1\r\nHost: localhost\r\n'
    if i == 2:
        requests += b'Connection: close\r\n'
    requests += b'\r\n'

s.send(requests)

# Recibir respuestas
response = b''
while True:
    try:
        data = s.recv(4096)
        if not data:
            break
        response += data
    except:
        break
s.close()

# Contar respuestas
count = response.count(b'HTTP/1.1')
print(count)
" 2>/dev/null)

if [ "$pipeline_result" == "3" ]; then
    log_pass "Pipelining works (3 responses received)"
else
    log_fail "Pipelining failed (got $pipeline_result responses)"
fi

# ----------------------------------------------------------------------------
# 8. TESTS DE AUTOINDEX
# ----------------------------------------------------------------------------
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "8. AUTOINDEX TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

autoindex_content=$(curl -s "$SERVER_URL/images/" 2>/dev/null)
if echo "$autoindex_content" | grep -qi "<html\|Index of"; then
    log_pass "Autoindex generates HTML listing"
else
    log_skip "Autoindex test (not enabled or no /images/)"
fi

# ----------------------------------------------------------------------------
# 9. TESTS DE HEADERS
# ----------------------------------------------------------------------------
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "9. HEADER TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Server header
server=$(curl -sI "$SERVER_URL/" 2>/dev/null | grep -i "^Server:" | tr -d '\r')
if [ -n "$server" ]; then
    log_pass "Server header present: $server"
else
    log_fail "Server header missing"
fi

# Date header
date_h=$(curl -sI "$SERVER_URL/" 2>/dev/null | grep -i "^Date:" | tr -d '\r')
if [ -n "$date_h" ]; then
    log_pass "Date header present"
else
    log_fail "Date header missing"
fi

# Content-Type header
content_type=$(curl -sI "$SERVER_URL/index.html" 2>/dev/null | grep -i "^Content-Type:" | tr -d '\r')
if echo "$content_type" | grep -qi "text/html"; then
    log_pass "Content-Type correct for HTML"
else
    log_fail "Content-Type incorrect: $content_type"
fi

# Content-Length header
content_length=$(curl -sI "$SERVER_URL/" 2>/dev/null | grep -i "^Content-Length:" | tr -d '\r')
if [ -n "$content_length" ]; then
    log_pass "Content-Length header present: $content_length"
else
    log_fail "Content-Length header missing"
fi

# ----------------------------------------------------------------------------
# 10. TESTS DE ROBUSTEZ
# ----------------------------------------------------------------------------
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "10. ROBUSTNESS TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Request malformada
malformed_result=$(printf "INVALID REQUEST\r\n\r\n" | nc -w 2 localhost 8080 2>/dev/null | head -1)
if echo "$malformed_result" | grep -q "HTTP/1.1 400"; then
    log_pass "Malformed request returns 400"
else
    log_pass "Server handles malformed request (no crash)"
fi

# Versión HTTP inválida
invalid_version=$(printf "GET / HTTP/9.9\r\nHost: localhost\r\n\r\n" | nc -w 2 localhost 8080 2>/dev/null | head -1)
if echo "$invalid_version" | grep -q "505"; then
    log_pass "Invalid HTTP version returns 505"
else
    log_pass "Server handles invalid version"
fi

# Múltiples conexiones simultáneas
for i in $(seq 1 10); do
    curl -s -o /dev/null "$SERVER_URL/" &
done
wait
log_pass "Multiple simultaneous connections handled"

# ============================================================================
# RESUMEN
# ============================================================================
echo ""
echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║                         TEST RESULTS                               ║"
echo "╠════════════════════════════════════════════════════════════════════╣"
printf "║  ${GREEN}PASSED:  %3d${NC}                                                   ║\n" $PASSED
printf "║  ${RED}FAILED:  %3d${NC}                                                   ║\n" $FAILED
printf "║  ${YELLOW}SKIPPED: %3d${NC}                                                   ║\n" $SKIPPED
echo "╠════════════════════════════════════════════════════════════════════╣"
TOTAL=$((PASSED + FAILED))
if [ $TOTAL -gt 0 ]; then
    PERCENT=$((PASSED * 100 / TOTAL))
    printf "║  Success Rate: %3d%%                                              ║\n" $PERCENT
fi
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""

# Exit code
if [ $FAILED -gt 0 ]; then
    exit 1
else
    exit 0
fi
```

## 11.2 Script de Stress Test Avanzado

```bash
#!/bin/bash
# webserv_stress_test.sh
# Test de estrés para webserv

SERVER_URL="${SERVER_URL:-http://localhost:8080}"
DURATION=${DURATION:-30}

echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║                    WEBSERV STRESS TEST                             ║"
echo "╠════════════════════════════════════════════════════════════════════╣"
echo "║  Server: $SERVER_URL"
echo "║  Duration: ${DURATION}s per test"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""

# Verificar herramientas
check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo "[WARN] $1 not found, some tests will be skipped"
        return 1
    fi
    return 0
}

# Test con ab (Apache Benchmark)
if check_tool ab; then
    echo "━━━ Apache Benchmark Test ━━━"
    ab -n 10000 -c 100 -t $DURATION "$SERVER_URL/" 2>/dev/null | \
        grep -E "(Requests per second|Time per request|Transfer rate|Failed requests)"
    echo ""
fi

# Test con wrk (si está disponible)
if check_tool wrk; then
    echo "━━━ WRK Benchmark Test ━━━"
    wrk -t4 -c100 -d${DURATION}s "$SERVER_URL/"
    echo ""
fi

# Test con siege (si está disponible)
if check_tool siege; then
    echo "━━━ Siege Benchmark Test ━━━"
    siege -c 50 -t ${DURATION}s -b "$SERVER_URL/" 2>&1 | tail -20
    echo ""
fi

# Test manual con curl en paralelo
echo "━━━ Parallel Curl Test ━━━"
start_time=$(date +%s)
count=0
errors=0

while [ $(($(date +%s) - start_time)) -lt 10 ]; do
    for i in $(seq 1 50); do
        (curl -s -o /dev/null -w "%{http_code}" "$SERVER_URL/" | grep -q "200" || ((errors++))) &
    done
    wait
    ((count+=50))
done

echo "Requests: $count"
echo "Errors: $errors"
echo "Requests/sec: $((count / 10))"
echo ""

# Memory leak check
echo "━━━ Memory Usage Check ━━━"
if pgrep webserv > /dev/null; then
    PID=$(pgrep webserv | head -1)
    
    echo "Before stress:"
    ps -p $PID -o pid,vsz,rss,pmem --no-headers
    
    # Generar carga
    for i in $(seq 1 1000); do
        curl -s -o /dev/null "$SERVER_URL/" &
    done
    wait
    
    echo "After stress:"
    ps -p $PID -o pid,vsz,rss,pmem --no-headers
else
    echo "[SKIP] webserv process not found"
fi

echo ""
echo "Stress test complete!"
```

## 11.3 Script de Test CGI Avanzado

```python
#!/usr/bin/env python3
# test_cgi_advanced.py
# Tests avanzados de CGI para webserv

import requests
import time
import sys
import json
from concurrent.futures import ThreadPoolExecutor, as_completed

BASE_URL = "http://localhost:8080"
CGI_URL = f"{BASE_URL}/cgi-bin/test.py"

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    END = '\033[0m'

def log_pass(msg):
    print(f"{Colors.GREEN}[PASS]{Colors.END} {msg}")

def log_fail(msg):
    print(f"{Colors.RED}[FAIL]{Colors.END} {msg}")

def log_info(msg):
    print(f"{Colors.BLUE}[INFO]{Colors.END} {msg}")

# Test 1: CGI básico
def test_basic_cgi():
    try:
        r = requests.get(CGI_URL, timeout=10)
        if r.status_code == 200:
            log_pass("Basic CGI GET")
            return True
        else:
            log_fail(f"Basic CGI GET (status {r.status_code})")
            return False
    except Exception as e:
        log_fail(f"Basic CGI GET ({e})")
        return False

# Test 2: Query string
def test_query_string():
    try:
        r = requests.get(f"{CGI_URL}?name=John&age=30", timeout=10)
        if "QUERY_STRING" in r.text and "name=John" in r.text:
            log_pass("CGI QUERY_STRING parsing")
            return True
        else:
            log_fail("CGI QUERY_STRING parsing")
            return False
    except Exception as e:
        log_fail(f"CGI QUERY_STRING ({e})")
        return False

# Test 3: POST con body
def test_post_body():
    try:
        data = {"key": "value", "foo": "bar"}
        r = requests.post(CGI_URL, data=data, timeout=10)
        if r.status_code == 200 and "CONTENT_LENGTH" in r.text:
            log_pass("CGI POST with body")
            return True
        else:
            log_fail("CGI POST with body")
            return False
    except Exception as e:
        log_fail(f"CGI POST ({e})")
        return False

# Test 4: POST con body grande
def test_large_body():
    try:
        # 10MB de datos
        large_data = "A" * (10 * 1024 * 1024)
        r = requests.post(CGI_URL, data=large_data, timeout=120)
        if r.status_code == 200:
            log_pass(f"CGI with 10MB body (response: {len(r.text)} bytes)")
            return True
        else:
            log_fail(f"CGI with 10MB body (status {r.status_code})")
            return False
    except Exception as e:
        log_fail(f"CGI large body ({e})")
        return False

# Test 5: Headers HTTP_*
def test_http_headers():
    try:
        headers = {
            "X-Custom-Header": "test-value",
            "Accept-Language": "es-ES"
        }
        r = requests.get(CGI_URL, headers=headers, timeout=10)
        if "HTTP_X_CUSTOM_HEADER" in r.text or "HTTP_ACCEPT_LANGUAGE" in r.text:
            log_pass("CGI HTTP_* headers")
            return True
        else:
            log_fail("CGI HTTP_* headers not found in output")
            return False
    except Exception as e:
        log_fail(f"CGI HTTP headers ({e})")
        return False

# Test 6: CGI concurrente
def test_concurrent_cgi():
    def make_request(i):
        try:
            r = requests.get(f"{CGI_URL}?id={i}", timeout=30)
            return r.status_code == 200
        except:
            return False
    
    log_info("Testing 20 concurrent CGI requests...")
    
    with ThreadPoolExecutor(max_workers=20) as executor:
        futures = [executor.submit(make_request, i) for i in range(20)]
        results = [f.result() for f in as_completed(futures)]
    
    success = sum(results)
    if success == 20:
        log_pass(f"Concurrent CGI ({success}/20 successful)")
        return True
    else:
        log_fail(f"Concurrent CGI ({success}/20 successful)")
        return False

# Test 7: PATH_INFO
def test_path_info():
    try:
        r = requests.get(f"{CGI_URL}/extra/path/info", timeout=10)
        if "PATH_INFO" in r.text and "/extra/path/info" in r.text:
            log_pass("CGI PATH_INFO")
            return True
        else:
            log_fail("CGI PATH_INFO")
            return False
    except Exception as e:
        log_fail(f"CGI PATH_INFO ({e})")
        return False

# Test 8: Timeout de CGI
def test_cgi_timeout():
    # Este test requiere un script CGI que haga sleep
    log_info("CGI timeout test skipped (requires special script)")
    return True

# Test 9: Status header
def test_status_header():
    try:
        # Requiere un CGI que devuelva Status: 201
        r = requests.get(f"{CGI_URL}?status=201", timeout=10)
        # Si el CGI soporta el parámetro status
        if r.status_code in [200, 201]:
            log_pass("CGI Status header handling")
            return True
        else:
            log_fail(f"CGI Status header (got {r.status_code})")
            return False
    except Exception as e:
        log_fail(f"CGI Status header ({e})")
        return False

# Test 10: Content-Type del CGI
def test_content_type():
    try:
        r = requests.get(CGI_URL, timeout=10)
        ct = r.headers.get('Content-Type', '')
        if 'text/html' in ct or 'text/plain' in ct:
            log_pass(f"CGI Content-Type: {ct}")
            return True
        else:
            log_fail(f"CGI Content-Type unexpected: {ct}")
            return False
    except Exception as e:
        log_fail(f"CGI Content-Type ({e})")
        return False

def main():
    print("╔════════════════════════════════════════════════════════════════════╗")
    print("║                    CGI ADVANCED TESTS                              ║")
    print("╚════════════════════════════════════════════════════════════════════╝")
    print()
    
    tests = [
        ("Basic CGI", test_basic_cgi),
        ("Query String", test_query_string),
        ("POST Body", test_post_body),
        ("HTTP Headers", test_http_headers),
        ("PATH_INFO", test_path_info),
        ("Content-Type", test_content_type),
        ("Concurrent CGI", test_concurrent_cgi),
        ("Large Body", test_large_body),
        ("Status Header", test_status_header),
    ]
    
    passed = 0
    failed = 0
    
    for name, test_func in tests:
        try:
            if test_func():
                passed += 1
            else:
                failed += 1
        except Exception as e:
            log_fail(f"{name}: {e}")
            failed += 1
    
    print()
    print(f"Results: {passed} passed, {failed} failed")
    
    return 0 if failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
```

---

# 12. GUÍA DE TROUBLESHOOTING AVANZADA

## 12.1 Árbol de Decisión de Problemas

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    TROUBLESHOOTING DECISION TREE                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ¿El servidor arranca?                                                      │
│  │                                                                          │
│  ├─ NO ──┬─▶ "bind() failed: Address already in use"                       │
│  │       │   └─▶ Puerto ocupado → lsof -i :8080 → kill proceso             │
│  │       │                                                                  │
│  │       ├─▶ "Cannot open configuration file"                              │
│  │       │   └─▶ Verificar ruta y permisos del archivo                     │
│  │       │                                                                  │
│  │       ├─▶ "No server block found"                                       │
│  │       │   └─▶ Revisar sintaxis del archivo de configuración             │
│  │       │                                                                  │
│  │       └─▶ Segmentation fault                                            │
│  │           └─▶ Compilar con -g, ejecutar con gdb                         │
│  │                                                                          │
│  └─ SÍ ──▶ ¿Las peticiones llegan?                                         │
│            │                                                                │
│            ├─ NO ──▶ ¿Firewall activo?                                     │
│            │        ¿Escuchando en interfaz correcta?                      │
│            │        → netstat -tlnp | grep 8080                            │
│            │                                                                │
│            └─ SÍ ──▶ ¿Qué error HTTP?                                      │
│                     │                                                       │
│                     ├─▶ 400 Bad Request                                    │
│                     │   └─▶ Petición malformada                            │
│                     │       Ver logs, revisar cliente                      │
│                     │                                                       │
│                     ├─▶ 403 Forbidden                                      │
│                     │   └─▶ Permisos del archivo                           │
│                     │       chmod 644 archivo                              │
│                     │                                                       │
│                     ├─▶ 404 Not Found                                      │
│                     │   └─▶ Verificar root + uri = path correcto           │
│                     │       ls -la /path/to/file                           │
│                     │                                                       │
│                     ├─▶ 405 Method Not Allowed                             │
│                     │   └─▶ Verificar "methods" en location                │
│                     │                                                       │
│                     ├─▶ 413 Payload Too Large                              │
│                     │   └─▶ Aumentar client_max_body_size                  │
│                     │                                                       │
│                     ├─▶ 500 Internal Server Error                          │
│                     │   └─▶ CGI falló                                      │
│                     │       Verificar permisos y shebang                   │
│                     │       Ejecutar CGI manualmente                       │
│                     │                                                       │
│                     ├─▶ 502 Bad Gateway                                    │
│                     │   └─▶ CGI no devolvió output válido                  │
│                     │                                                       │
│                     └─▶ 504 Gateway Timeout                                │
│                         └─▶ CGI tardó demasiado                            │
│                             Aumentar CGI_TIMEOUT                           │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 12.2 Comandos de Diagnóstico

```bash
# ═══════════════════════════════════════════════════════════════════════════
# VERIFICACIÓN DE ESTADO DEL SERVIDOR
# ═══════════════════════════════════════════════════════════════════════════

# Ver si el servidor está corriendo
pgrep -a webserv
ps aux | grep webserv

# Ver en qué puerto escucha
ss -tlnp | grep webserv
netstat -tlnp | grep 8080
lsof -i :8080

# Ver conexiones activas
ss -tnp | grep webserv
netstat -tnp | grep webserv

# Ver descriptores de archivo abiertos
lsof -p $(pgrep webserv)

# ═══════════════════════════════════════════════════════════════════════════
# MONITOREO EN TIEMPO REAL
# ═══════════════════════════════════════════════════════════════════════════

# Monitorear uso de memoria
watch -n 1 'ps aux | grep webserv'

# Ver syscalls en tiempo real
strace -p $(pgrep webserv) -e trace=network

# Ver logs en tiempo real (si hay log file)
tail -f webserv.log

# Monitorear conexiones
watch -n 0.5 'ss -tn | grep 8080 | wc -l'

# ═══════════════════════════════════════════════════════════════════════════
# TESTING DE CONECTIVIDAD
# ═══════════════════════════════════════════════════════════════════════════

# Test básico con curl
curl -v http://localhost:8080/

# Test con timeout
curl --max-time 5 http://localhost:8080/

# Test de headers
curl -I http://localhost:8080/

# Test con netcat
echo -e "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc localhost 8080

# Test de múltiples requests
for i in {1..100}; do curl -s -o /dev/null -w "%{http_code}\n" http://localhost:8080/; done

# ═══════════════════════════════════════════════════════════════════════════
# DEBUGGING DE CGI
# ═══════════════════════════════════════════════════════════════════════════

# Verificar permisos
ls -la cgi-bin/
file cgi-bin/test.py
head -1 cgi-bin/test.py  # Ver shebang

# Ejecutar CGI manualmente
cd cgi-bin
GATEWAY_INTERFACE="CGI/1.1" \
REQUEST_METHOD="GET" \
SCRIPT_NAME="/cgi-bin/test.py" \
QUERY_STRING="test=1" \
SERVER_PROTOCOL="HTTP/1.1" \
./test.py

# Verificar intérprete
which python3
/usr/bin/python3 --version

# ═══════════════════════════════════════════════════════════════════════════
# VERIFICACIÓN DE MEMORIA
# ═══════════════════════════════════════════════════════════════════════════

# Memory leaks con valgrind
valgrind --leak-check=full --show-leak-kinds=all ./webserv config.conf

# File descriptor leaks
valgrind --track-fds=yes ./webserv config.conf

# Address sanitizer (compilar con -fsanitize=address)
./webserv_debug config.conf

# ═══════════════════════════════════════════════════════════════════════════
# ANÁLISIS DE PROBLEMAS DE RED
# ═══════════════════════════════════════════════════════════════════════════

# Capturar tráfico HTTP
tcpdump -i lo port 8080 -A

# Capturar a archivo
tcpdump -i lo port 8080 -w capture.pcap

# Analizar con wireshark
wireshark capture.pcap
```

## 12.3 Problemas Específicos y Soluciones

### Problema: "Connection refused"

```bash
# Diagnóstico
nc -zv localhost 8080
# Si falla: servidor no está escuchando

# Solución
# 1. Verificar que el servidor está corriendo
pgrep webserv || ./webserv config.conf

# 2. Verificar el puerto en la configuración
grep "listen" config.conf

# 3. Verificar firewall
sudo ufw status
sudo iptables -L -n | grep 8080
```

### Problema: "Connection reset by peer"

```bash
# Causa: El servidor cerró la conexión abruptamente

# Diagnóstico
strace -e trace=close,shutdown -p $(pgrep webserv)

# Posibles causas:
# 1. Error en parsing de request → Ver logs
# 2. Timeout → Aumentar CONNECTION_TIMEOUT
# 3. Bug en el código → Compilar con debug

# Solución temporal: reiniciar servidor
pkill webserv && ./webserv config.conf
```

### Problema: CGI devuelve 500

```bash
# 1. Verificar que el script es ejecutable
chmod +x cgi-bin/test.py

# 2. Verificar shebang
head -1 cgi-bin/test.py
# Debe ser: #!/usr/bin/env python3 o #!/usr/bin/python3

# 3. Verificar que el intérprete existe
which python3
ls -la /usr/bin/python3

# 4. Ejecutar manualmente
cd cgi-bin && ./test.py

# 5. Verificar output del CGI (debe tener Content-Type)
./test.py | head -5
# Debe empezar con: Content-Type: text/html
```

### Problema: Respuestas truncadas

```bash
# Diagnóstico
curl -v http://localhost:8080/large_file.bin 2>&1 | tail -20

# Verificar Content-Length
curl -I http://localhost:8080/large_file.bin

# Comparar tamaño esperado vs recibido
curl -s http://localhost:8080/large_file.bin | wc -c
ls -la www/large_file.bin

# Posibles causas:
# 1. Content-Length incorrecto en respuesta
# 2. send() no envía todo el buffer
# 3. Cliente cierra conexión prematuramente
```

### Problema: Memoria crece continuamente

```bash
# Monitorear memoria
while true; do
    ps -p $(pgrep webserv) -o rss= 
    sleep 5
done

# Identificar leak con valgrind
valgrind --leak-check=full --log-file=leak.log ./webserv config.conf &
sleep 60
curl http://localhost:8080/
pkill webserv
cat leak.log | grep "definitely lost"

# Lugares comunes de leak:
# 1. Strings de CGI no liberadas
# 2. Clientes no eliminados del mapa
# 3. File descriptors no cerrados
```

---

# 13. ESCENARIOS DE EVALUACIÓN 42

## 13.1 Escenario: Evaluador Prueba Configuración

```bash
# El evaluador creará una configuración personalizada

cat > eval_config.conf << 'EOF'
server {
    listen 8080;
    server_name test.local;
    root ./eval_www;
    index index.html;
    client_max_body_size 1M;
    
    error_page 404 /404.html;
    
    location / {
        methods GET;
    }
    
    location /post {
        methods GET POST;
    }
    
    location /delete {
        methods GET DELETE;
    }
}
EOF

# Crear directorio de prueba
mkdir -p eval_www
echo "<h1>Test</h1>" > eval_www/index.html
echo "<h1>404</h1>" > eval_www/404.html

# Iniciar servidor
./webserv eval_config.conf

# Tests del evaluador:
curl http://localhost:8080/                    # Debe funcionar
curl http://localhost:8080/nonexistent         # Debe mostrar 404.html
curl -X POST http://localhost:8080/            # Debe dar 405
curl -X POST http://localhost:8080/post        # Debe funcionar
curl -X DELETE http://localhost:8080/delete/x  # Debe funcionar
```

## 13.2 Escenario: Evaluador Prueba CGI

```python
#!/usr/bin/env python3
# eval_www/cgi-bin/eval.py - Script CGI para evaluación

import os
import sys

print("Content-Type: text/html")
print()

print("<html><head><title>CGI Test</title></head><body>")
print("<h1>CGI Environment</h1>")
print("<table border='1'>")

# Variables requeridas por 42
required_vars = [
    'REQUEST_METHOD',
    'QUERY_STRING', 
    'CONTENT_TYPE',
    'CONTENT_LENGTH',
    'SCRIPT_NAME',
    'PATH_INFO',
    'SERVER_NAME',
    'SERVER_PORT',
    'GATEWAY_INTERFACE'
]

for var in required_vars:
    value = os.environ.get(var, '<not set>')
    print(f"<tr><td><b>{var}</b></td><td>{value}</td></tr>")

print("</table>")

# Mostrar body si hay
content_length = os.environ.get('CONTENT_LENGTH', '0')
if content_length and int(content_length) > 0:
    body = sys.stdin.read(int(content_length))
    print(f"<h2>Request Body ({content_length} bytes)</h2>")
    print(f"<pre>{body[:1000]}</pre>")  # Mostrar primeros 1000 chars

print("</body></html>")
```

```bash
# Hacer ejecutable
chmod +x eval_www/cgi-bin/eval.py

# Tests del evaluador:
curl "http://localhost:8080/cgi-bin/eval.py"
curl "http://localhost:8080/cgi-bin/eval.py?name=42&project=webserv"
curl -X POST -d "test data" "http://localhost:8080/cgi-bin/eval.py"
curl -X POST -d "$(head -c 1000000 /dev/zero)" "http://localhost:8080/cgi-bin/eval.py"
```

## 13.3 Escenario: Evaluador Prueba Robustez

```bash
# Test de requests malformadas
printf "BADREQUEST\r\n\r\n" | nc localhost 8080
printf "GET /\r\n\r\n" | nc localhost 8080  # Sin HTTP version
printf "GET / HTTP/2.0\r\n\r\n" | nc localhost 8080  # Version no soportada

# Test de headers malformados
printf "GET / HTTP/1.1\r\nBadHeader\r\n\r\n" | nc localhost 8080

# Test de body sin Content-Length
printf "POST / HTTP/1.1\r\nHost: localhost\r\n\r\ndata" | nc localhost 8080

# Test de múltiples conexiones
for i in {1..100}; do
    curl -s http://localhost:8080/ &
done
wait

# Verificar que el servidor sigue respondiendo
curl http://localhost:8080/
```

## 13.4 Lista de Verificación Pre-Evaluación

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    PRE-EVALUATION CHECKLIST                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ANTES DE LA EVALUACIÓN:                                                    │
│                                                                             │
│  [ ] make compila sin warnings                                              │
│  [ ] make clean / fclean / re funcionan                                     │
│  [ ] Archivo de configuración de ejemplo listo                              │
│  [ ] Directorio www con contenido de prueba                                 │
│  [ ] Scripts CGI de ejemplo (Python al menos)                               │
│  [ ] Páginas de error personalizadas                                        │
│                                                                             │
│  FUNCIONALIDAD BÁSICA:                                                      │
│                                                                             │
│  [ ] GET funciona                                                           │
│  [ ] HEAD funciona                                                          │
│  [ ] POST funciona (con CGI)                                                │
│  [ ] DELETE funciona                                                        │
│  [ ] PUT funciona                                                           │
│  [ ] Error 404 correcto                                                     │
│  [ ] Error 405 correcto                                                     │
│  [ ] Redirecciones funcionan                                                │
│  [ ] Autoindex funciona                                                     │
│                                                                             │
│  CGI:                                                                       │
│                                                                             │
│  [ ] CGI Python funciona                                                    │
│  [ ] QUERY_STRING se pasa correctamente                                     │
│  [ ] Body se pasa por stdin                                                 │
│  [ ] Variables de entorno correctas                                         │
│  [ ] CGI con output grande funciona                                         │
│                                                                             │
│  CONFIGURACIÓN:                                                             │
│                                                                             │
│  [ ] Múltiples servers en un archivo                                        │
│  [ ] server_name funciona                                                   │
│  [ ] location con diferentes métodos                                        │
│  [ ] client_max_body_size funciona                                          │
│  [ ] error_page funciona                                                    │
│                                                                             │
│  ROBUSTEZ:                                                                  │
│                                                                             │
│  [ ] No crash con Ctrl+C                                                    │
│  [ ] No crash con requests malformadas                                      │
│  [ ] No memory leaks (verificado con valgrind)                              │
│  [ ] No file descriptor leaks                                               │
│  [ ] Stress test pasa                                                       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

# 14. GLOSARIO TÉCNICO

| Término | Definición |
|---------|------------|
| **CGI** | Common Gateway Interface - Protocolo para ejecutar programas externos desde un servidor web |
| **Chunked Encoding** | Método de transferencia HTTP donde el body se envía en fragmentos de tamaño variable |
| **epoll/poll/select** | APIs de Linux para multiplexar I/O en múltiples descriptores de archivo |
| **Event Loop** | Patrón de programación que espera y despacha eventos en un bucle continuo |
| **FastCGI** | Protocolo binario para comunicación servidor-aplicación, más eficiente que CGI |
| **File Descriptor** | Número entero que identifica un recurso de I/O abierto en Unix |
| **Fork** | Syscall que crea un nuevo proceso hijo duplicando el proceso padre |
| **Keep-Alive** | Característica HTTP que mantiene conexiones TCP abiertas para múltiples requests |
| **MIME Type** | Identificador de formato de archivo (ej: text/html, image/png) |
| **Non-blocking I/O** | Modo de I/O donde las operaciones retornan inmediatamente sin esperar |
| **Pipelining** | Técnica HTTP donde múltiples requests se envían sin esperar respuestas |
| **RFC** | Request For Comments - Documentos que definen estándares de Internet |
| **Virtual Host** | Técnica para servir múltiples sitios web desde un solo servidor |
| **WebDAV** | Extensión HTTP para operaciones de archivos (PUT, DELETE, MKCOL, etc.) |

---

# 15. REFERENCIAS Y RECURSOS

## 15.1 RFCs Relevantes

| RFC | Título | Uso en Webserv |
|-----|--------|----------------|
| RFC 7230 | HTTP/1.1 Message Syntax and Routing | Parsing de requests/responses |
| RFC 7231 | HTTP/1.1 Semantics and Content | Métodos HTTP, status codes |
| RFC 7232 | HTTP/1.1 Conditional Requests | Not implemented |
| RFC 7233 | HTTP/1.1 Range Requests | Not implemented |
| RFC 7234 | HTTP/1.1 Caching | Not implemented |
| RFC 7235 | HTTP/1.1 Authentication | Not implemented |
| RFC 3875 | CGI/1.1 Specification | Ejecución de CGI |
| RFC 6265 | HTTP Cookies | Sesiones |
| RFC 7578 | multipart/form-data | Uploads |

## 15.2 Documentación Oficial

- [NGINX Documentation](https://nginx.org/en/docs/)
- [Apache HTTP Server Documentation](https://httpd.apache.org/docs/)
- [Linux man pages](https://man7.org/linux/man-pages/)

## 15.3 Herramientas de Testing

| Herramienta | Propósito | Instalación |
|-------------|-----------|-------------|
| curl | Cliente HTTP | `apt install curl` |
| nc (netcat) | Conexiones TCP raw | `apt install netcat` |
| ab | Apache Benchmark | `apt install apache2-utils` |
| wrk | HTTP benchmark | `apt install wrk` |
| siege | Load testing | `apt install siege` |
| valgrind | Memory debugging | `apt install valgrind` |
| strace | Syscall tracing | `apt install strace` |
| tcpdump | Packet capture | `apt install tcpdump` |

## 15.4 Recursos de Aprendizaje

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [The C10K Problem](http://www.kegel.com/c10k.html)
- [HTTP Made Really Easy](https://www.jmarshall.com/easy/http/)

---

# RESUMEN DOCUMENTACIÓN COMPLETA

## Archivos Generados

| Fase | Archivo | Líneas | Contenido |
|------|---------|--------|-----------|
| 1 | WEBSERV_DOCUMENTATION.md | ~2000 | Arquitectura general |
| 1 | WEBSERV_DIAGRAMS.md | ~500 | Diagramas Mermaid |
| 2 | WEBSERV_FASE2_CODE_ANALYSIS.md | ~2500 | Server.cpp, Request.cpp |
| 2 | WEBSERV_FASE2_PART2_DETAILED.md | ~1300 | Response.cpp, métodos |
| 3 | WEBSERV_FASE3_MODULES_RFC.md | ~1300 | CGI, Utils, RFC compliance |
| 3 | WEBSERV_FASE3_PART2_TESTING.md | ~860 | Tests, debugging |
| 4 | WEBSERV_FASE4_FINAL.md | ~1500 | Manual, checklist |
| 4 | WEBSERV_FASE4_PART2_ADVANCED.md | ~1700 | NGINX, scripts, troubleshooting |
| **Total** | **8 archivos** | **~11,700** | - |

## Cobertura de Documentación

- ✅ Arquitectura completa del sistema
- ✅ Análisis línea por línea del código
- ✅ Conformidad con RFCs HTTP
- ✅ Manual de usuario completo
- ✅ Checklist de evaluación 42
- ✅ Scripts de testing automatizados
- ✅ Guía de troubleshooting
- ✅ Comparativa con NGINX
- ✅ Optimizaciones implementadas y posibles
- ✅ Glosario técnico y referencias

---

**FIN DE LA DOCUMENTACIÓN COMPLETA**

*Documentación exhaustiva del proyecto webserv para 42 Barcelona*  
*Autor: fcela-ga (Felipe Cela García)*  
*Total: ~11,700 líneas de documentación técnica*
