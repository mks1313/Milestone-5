# WEBSERV - Fase 3 (Parte 2): Guía Práctica y Testing

## Ejemplos, Casos de Prueba y Debugging

**Proyecto:** 42 Barcelona - webserv  
**Autor:** fcela-ga (Felipe Cela García)  
**Fase:** 3 de 4 (Continuación)

---

# ÍNDICE FASE 3 - PARTE 2

8. [Ejemplos de Configuración](#8-ejemplos-de-configuración)
9. [Casos de Prueba Completos](#9-casos-de-prueba-completos)
10. [Guía de Debugging](#10-guía-de-debugging)
11. [Problemas Comunes y Soluciones](#11-problemas-comunes-y-soluciones)
12. [Scripts de Testing](#12-scripts-de-testing)

---

# 8. EJEMPLOS DE CONFIGURACIÓN

## 8.1 Configuración Mínima

```nginx
# webserv_minimal.conf
server {
    listen 8080;
    root ./www;
}
```

Esta configuración:
- Escucha en el puerto 8080 (todas las interfaces)
- Sirve archivos desde `./www`
- Usa valores por defecto para todo lo demás

## 8.2 Configuración Completa de Producción

```nginx
# webserv_production.conf

# Servidor principal
server {
    listen 0.0.0.0:8080;
    server_name localhost www.example.com;
    
    root ./www;
    index index.html index.htm;
    
    # Límite de body: 10MB
    client_max_body_size 10M;
    
    # Autoindex deshabilitado por seguridad
    autoindex off;
    
    # Páginas de error personalizadas
    error_page 400 /errors/400.html;
    error_page 403 /errors/403.html;
    error_page 404 /errors/404.html;
    error_page 405 /errors/405.html;
    error_page 413 /errors/413.html;
    error_page 500 502 503 504 /errors/50x.html;
    
    # Raíz del sitio
    location / {
        methods GET HEAD;
    }
    
    # Directorio de imágenes con listado
    location /images {
        methods GET HEAD;
        autoindex on;
    }
    
    # API con todos los métodos
    location /api {
        methods GET POST PUT DELETE;
        client_max_body_size 1M;
    }
    
    # Uploads
    location /upload {
        methods GET POST DELETE;
        upload_store ./www/uploads;
        client_max_body_size 100M;
    }
    
    # CGI
    location /cgi-bin {
        methods GET POST;
        alias ./cgi-bin;
        cgi .py /usr/bin/python3;
        cgi .php /usr/bin/php-cgi;
        cgi .pl /usr/bin/perl;
        cgi .sh /bin/bash;
    }
    
    # Redirección permanente
    location /old-api {
        redirect 301 /api;
    }
    
    # Redirección temporal
    location /maintenance {
        redirect 302 /maintenance.html;
    }
}

# Segundo servidor (virtual host)
server {
    listen 0.0.0.0:8080;
    server_name api.example.com;
    
    root ./api;
    index index.json;
    
    client_max_body_size 5M;
    
    location / {
        methods GET POST PUT DELETE;
    }
}

# Servidor en puerto diferente
server {
    listen 0.0.0.0:8081;
    server_name admin.example.com;
    
    root ./admin;
    index index.html;
    
    location / {
        methods GET HEAD;
    }
}
```

## 8.3 Configuración para Tests

```nginx
# webserv_test.conf
server {
    listen 8080;
    server_name localhost;
    
    root ./test_www;
    index index.html;
    client_max_body_size 100M;
    autoindex on;
    
    # Test de métodos
    location / {
        methods GET HEAD POST PUT DELETE;
    }
    
    # Test de uploads
    location /upload {
        methods GET POST DELETE;
        upload_store ./test_www/uploads;
    }
    
    # Test de CGI
    location /cgi-bin {
        methods GET POST;
        alias ./test_cgi;
        cgi .py /usr/bin/python3;
    }
    
    # Test de redirecciones
    location /redir301 {
        redirect 301 /;
    }
    
    location /redir302 {
        redirect 302 /;
    }
    
    # Test de body grande
    location /bigbody {
        methods POST;
        client_max_body_size 200M;
    }
    
    # Test de límite de body
    location /smallbody {
        methods POST;
        client_max_body_size 100;
    }
}
```

---

# 9. CASOS DE PRUEBA COMPLETOS

## 9.1 Test GET Básico

```bash
# Request
curl -v http://localhost:8080/index.html

# Expected Response
HTTP/1.1 200 OK
Server: Webserv/1.0
Date: Fri, 27 Dec 2024 10:00:00 GMT
Content-Type: text/html
Content-Length: 1234
Connection: keep-alive

<!DOCTYPE html>...
```

## 9.2 Test HEAD

```bash
# Request
curl -v -I http://localhost:8080/index.html

# Expected Response (sin body)
HTTP/1.1 200 OK
Server: Webserv/1.0
Date: Fri, 27 Dec 2024 10:00:00 GMT
Content-Type: text/html
Content-Length: 1234
Connection: keep-alive
```

## 9.3 Test POST con JSON

```bash
# Request
curl -v -X POST http://localhost:8080/api/users \
     -H "Content-Type: application/json" \
     -d '{"name":"John","email":"john@example.com"}'

# Si es CGI, ejecuta el script
# Si no hay CGI, 204 No Content o 405
```

## 9.4 Test PUT (Crear archivo)

```bash
# Request
curl -v -X PUT http://localhost:8080/files/newfile.txt \
     -d "Este es el contenido del archivo"

# Expected Response
HTTP/1.1 201 Created
Content-Length: 0
```

## 9.5 Test DELETE

```bash
# Request
curl -v -X DELETE http://localhost:8080/files/oldfile.txt

# Expected Response
HTTP/1.1 204 No Content
```

## 9.6 Test Upload Multipart

```bash
# Request
curl -v -X POST http://localhost:8080/upload \
     -F "file=@/path/to/image.jpg" \
     -F "description=Mi imagen"

# Expected Response
HTTP/1.1 201 Created
```

## 9.7 Test Chunked Transfer

```bash
# Request con body chunked
curl -v -X POST http://localhost:8080/api/data \
     -H "Transfer-Encoding: chunked" \
     -d @- <<EOF
7
Mozilla
9
Developer
7
Network
0

EOF

# El servidor debe ensamblar: "MozillaDeveloperNetwork"
```

## 9.8 Test CGI Python

```python
#!/usr/bin/env python3
# test_cgi/test.py

import os
import sys

print("Content-Type: text/html")
print("Status: 200 OK")
print()
print("<html><body>")
print("<h1>CGI Test</h1>")
print("<h2>Environment Variables:</h2>")
print("<ul>")
for key, value in sorted(os.environ.items()):
    if key.startswith(('REQUEST', 'CONTENT', 'SERVER', 'REMOTE', 'HTTP', 'PATH', 'SCRIPT', 'QUERY', 'GATEWAY')):
        print(f"<li><b>{key}</b>: {value}</li>")
print("</ul>")

# Leer stdin si hay body
content_length = os.environ.get('CONTENT_LENGTH', '0')
if content_length and int(content_length) > 0:
    body = sys.stdin.read(int(content_length))
    print(f"<h2>Request Body ({content_length} bytes):</h2>")
    print(f"<pre>{body}</pre>")

print("</body></html>")
```

```bash
# Test del CGI
curl -v http://localhost:8080/cgi-bin/test.py

# Con POST
curl -v -X POST http://localhost:8080/cgi-bin/test.py \
     -H "Content-Type: application/json" \
     -d '{"test": "data"}'
```

## 9.9 Test de Errores

```bash
# 400 Bad Request
printf "INVALID REQUEST\r\n\r\n" | nc localhost 8080

# 404 Not Found
curl -v http://localhost:8080/nonexistent.html

# 405 Method Not Allowed (si location solo permite GET)
curl -v -X DELETE http://localhost:8080/readonly/file.txt

# 413 Payload Too Large (si client_max_body_size es pequeño)
curl -v -X POST http://localhost:8080/smallbody \
     -d "$(head -c 1000 /dev/zero | tr '\0' 'A')"

# 414 URI Too Long
curl -v "http://localhost:8080/$(head -c 10000 /dev/zero | tr '\0' 'A')"
```

## 9.10 Test de Pipelining

```bash
# Enviar múltiples requests en una conexión
(
echo -e "GET /file1.txt HTTP/1.1\r\nHost: localhost\r\n\r\n"
echo -e "GET /file2.txt HTTP/1.1\r\nHost: localhost\r\n\r\n"
echo -e "GET /file3.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
) | nc localhost 8080

# Debe recibir 3 respuestas en orden
```

## 9.11 Test de Keep-Alive

```bash
# HTTP/1.1 - Keep-alive por defecto
curl -v http://localhost:8080/file1.txt http://localhost:8080/file2.txt

# HTTP/1.0 - Cerrar por defecto
curl -v --http1.0 http://localhost:8080/file.txt

# HTTP/1.0 con keep-alive explícito
curl -v --http1.0 -H "Connection: keep-alive" http://localhost:8080/file.txt
```

---

# 10. GUÍA DE DEBUGGING

## 10.1 Compilación con Debug

```makefile
# En Makefile
debug:
    $(CXX) $(CXXFLAGS) -g -DDEBUG -fsanitize=address,undefined $(SRCS) -o webserv_debug
```

## 10.2 Logs de Debug

```cpp
// Habilitar logs detallados modificando Utils::logDebug
void logDebug(const std::string& msg) {
#ifdef DEBUG
    std::cout << "\033[0;36m[DEBUG]\033[0m " << msg << std::endl;
#else
    (void)msg;
#endif
}
```

## 10.3 Puntos de Debug Importantes

### En Server.cpp:

```cpp
// En _handleClientRead() - Ver datos recibidos
Utils::logDebug("Received " + Utils::intToString(bytesRead) + " bytes from fd " + 
                Utils::intToString(clientFd));
Utils::logDebug("Data: " + std::string(buffer, bytesRead < 100 ? bytesRead : 100));

// En _processRequest() - Ver request parseada
Utils::logDebug("Method: " + request.getMethod());
Utils::logDebug("URI: " + request.getUri());
Utils::logDebug("Path: " + request.getPath());
Utils::logDebug("Query: " + request.getQuery());
Utils::logDebug("Version: " + request.getVersion());

// En _handleCgi() - Ver variables de entorno
for (size_t i = 0; i < envStrings.size(); ++i)
    Utils::logDebug("ENV: " + envStrings[i]);
```

### En Request.cpp:

```cpp
// En parse() - Ver estado del parser
Utils::logDebug("Parser state: " + Utils::intToString(_state));
Utils::logDebug("Buffer size: " + Utils::sizeTToString(_rawRequest.size()));

// En _parseRequestLine() - Ver línea de request
Utils::logDebug("Request line: " + line);
```

## 10.4 Uso de strace

```bash
# Ver syscalls del servidor
strace -f -e trace=network,read,write,poll ./webserv config.conf

# Solo syscalls de red
strace -f -e trace=socket,bind,listen,accept,connect,send,recv ./webserv

# Con timestamps
strace -f -t -e trace=network ./webserv

# Guardar a archivo
strace -f -o webserv_trace.log ./webserv
```

## 10.5 Uso de valgrind

```bash
# Detectar memory leaks
valgrind --leak-check=full --show-leak-kinds=all ./webserv config.conf

# Con callstack completo
valgrind --leak-check=full --track-origins=yes ./webserv config.conf

# Detectar file descriptor leaks
valgrind --track-fds=yes ./webserv config.conf
```

## 10.6 Debugging de CGI

```bash
# Ejecutar CGI manualmente para verificar
cd cgi-bin
GATEWAY_INTERFACE="CGI/1.1" \
REQUEST_METHOD="GET" \
SCRIPT_NAME="/cgi-bin/test.py" \
QUERY_STRING="name=test" \
SERVER_PROTOCOL="HTTP/1.1" \
/usr/bin/python3 test.py

# Verificar permisos
ls -la cgi-bin/test.py
chmod +x cgi-bin/test.py

# Verificar shebang
head -1 cgi-bin/test.py
# Debe ser: #!/usr/bin/env python3 o similar
```

## 10.7 Debugging de Conexiones

```bash
# Ver conexiones activas
ss -tlnp | grep webserv
netstat -tlnp | grep 8080

# Ver estado de sockets
lsof -i -P -n | grep webserv

# Probar conexión manual
nc -v localhost 8080
# Escribir request manualmente:
# GET / HTTP/1.1
# Host: localhost
# [Enter][Enter]
```

---

# 11. PROBLEMAS COMUNES Y SOLUCIONES

## 11.1 El servidor no arranca

**Síntoma:** "bind() failed: Address already in use"

**Causa:** Otro proceso está usando el puerto, o el servidor anterior no cerró correctamente.

**Solución:**
```bash
# Encontrar proceso usando el puerto
lsof -i :8080
# o
fuser 8080/tcp

# Matar proceso
kill -9 <PID>

# Esperar TIME_WAIT (o usar SO_REUSEADDR)
sleep 60
```

## 11.2 Conexiones que se cuelgan

**Síntoma:** curl se queda esperando indefinidamente.

**Posibles causas:**
1. Content-Length incorrecto
2. Connection: keep-alive sin más datos
3. Bucle infinito en el parser

**Diagnóstico:**
```bash
# Ver qué está haciendo el servidor
strace -p <PID>

# Timeout forzado
curl --max-time 5 http://localhost:8080/
```

## 11.3 CGI no funciona

**Síntoma:** 500 Internal Server Error o sin respuesta.

**Checklist:**
```bash
# 1. Verificar permisos
ls -la cgi-bin/script.py
chmod +x cgi-bin/script.py

# 2. Verificar intérprete
which python3
head -1 cgi-bin/script.py  # Shebang correcto?

# 3. Ejecutar manualmente
cd cgi-bin && python3 script.py

# 4. Verificar configuración
grep -A5 "cgi" config.conf
```

## 11.4 Respuestas truncadas

**Síntoma:** Solo se recibe parte de la respuesta.

**Posibles causas:**
1. Content-Length incorrecto
2. Buffer de escritura no se envía completo
3. Timeout durante envío

**Solución en código:**
```cpp
// Verificar que todo el buffer se envía
while (client.getWriteBufferSize() > 0) {
    ssize_t sent = send(fd, buffer.c_str(), buffer.size(), 0);
    if (sent > 0) {
        client.eraseFromWriteBuffer(sent);
    } else if (sent < 0 && errno != EAGAIN) {
        // Error real
        break;
    }
}
```

## 11.5 Memory leaks

**Síntoma:** Uso de memoria crece con el tiempo.

**Diagnóstico:**
```bash
# Monitorear memoria
watch -n 1 'ps aux | grep webserv'

# Valgrind
valgrind --leak-check=full ./webserv
```

**Lugares comunes de leak:**
1. Strings no liberadas en CGI
2. Clientes no eliminados del mapa
3. File descriptors no cerrados

## 11.6 Pipelining no funciona

**Síntoma:** Solo se procesa la primera petición.

**Causa común:** `_readBuffer.clear()` en `Client::reset()`.

**Solución:**
```cpp
// NO hacer esto en reset():
// _readBuffer.clear();

// El buffer debe conservarse para peticiones pipelined
```

## 11.7 Archivos grandes fallan

**Síntoma:** Timeout o crash con archivos de 100MB+.

**Soluciones:**
1. Aumentar timeouts para respuestas grandes
2. Enviar en múltiples iteraciones de send()
3. Liberar memoria del CGI output después de copiar a write buffer

```cpp
// Optimización de memoria
{
    std::string empty;
    client.getCgiOutput().swap(empty);  // Liberar inmediatamente
}
```

---

# 12. SCRIPTS DE TESTING

## 12.1 Script de Test Automatizado

```bash
#!/bin/bash
# test_webserv.sh

SERVER_URL="http://localhost:8080"
PASSED=0
FAILED=0

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

test_case() {
    local name="$1"
    local expected_code="$2"
    shift 2
    local actual_code=$(curl -s -o /dev/null -w "%{http_code}" "$@")
    
    if [ "$actual_code" == "$expected_code" ]; then
        echo -e "${GREEN}[PASS]${NC} $name (got $actual_code)"
        ((PASSED++))
    else
        echo -e "${RED}[FAIL]${NC} $name (expected $expected_code, got $actual_code)"
        ((FAILED++))
    fi
}

echo "=== WEBSERV TESTS ==="
echo ""

# Básicos
echo "--- Basic Tests ---"
test_case "GET /" "200" "$SERVER_URL/"
test_case "GET /index.html" "200" "$SERVER_URL/index.html"
test_case "HEAD /" "200" -I "$SERVER_URL/"
test_case "GET /nonexistent" "404" "$SERVER_URL/nonexistent"

# Métodos
echo ""
echo "--- Method Tests ---"
test_case "POST /api" "204" -X POST "$SERVER_URL/api" -d "test"
test_case "PUT /files/new.txt" "201" -X PUT "$SERVER_URL/files/new.txt" -d "content"
test_case "DELETE /files/new.txt" "204" -X DELETE "$SERVER_URL/files/new.txt"

# Redirecciones
echo ""
echo "--- Redirect Tests ---"
test_case "GET /redir301" "301" "$SERVER_URL/redir301"
test_case "GET /redir302" "302" "$SERVER_URL/redir302"

# CGI
echo ""
echo "--- CGI Tests ---"
test_case "GET CGI" "200" "$SERVER_URL/cgi-bin/test.py"
test_case "POST CGI" "200" -X POST "$SERVER_URL/cgi-bin/test.py" -d "data"

# Errores
echo ""
echo "--- Error Tests ---"
test_case "Method Not Allowed" "405" -X PATCH "$SERVER_URL/"
test_case "Payload Too Large" "413" -X POST "$SERVER_URL/smallbody" \
    -H "Content-Length: 1000000" -d @/dev/zero

echo ""
echo "=== RESULTS ==="
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"

exit $FAILED
```

## 12.2 Script de Stress Test

```bash
#!/bin/bash
# stress_test.sh

SERVER_URL="http://localhost:8080"
CONCURRENT=50
REQUESTS=1000

echo "=== STRESS TEST ==="
echo "Concurrent: $CONCURRENT"
echo "Total requests: $REQUESTS"
echo ""

# Usando ab (Apache Benchmark)
if command -v ab &> /dev/null; then
    ab -n $REQUESTS -c $CONCURRENT "$SERVER_URL/"
else
    # Fallback con curl
    echo "ab not found, using curl..."
    for i in $(seq 1 $CONCURRENT); do
        (
            for j in $(seq 1 $((REQUESTS / CONCURRENT))); do
                curl -s -o /dev/null "$SERVER_URL/"
            done
        ) &
    done
    wait
    echo "Done!"
fi
```

## 12.3 Script de Test de Pipelining

```python
#!/usr/bin/env python3
# test_pipelining.py

import socket

HOST = 'localhost'
PORT = 8080

# Crear conexión
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

# Enviar múltiples requests pipelined
requests = [
    b"GET /file1.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
    b"GET /file2.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
    b"GET /file3.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n",
]

for req in requests:
    sock.send(req)

# Recibir todas las respuestas
response = b""
while True:
    data = sock.recv(4096)
    if not data:
        break
    response += data

sock.close()

# Contar respuestas HTTP
count = response.count(b"HTTP/1.1")
print(f"Sent {len(requests)} requests, received {count} responses")

if count == len(requests):
    print("[PASS] Pipelining works correctly")
else:
    print("[FAIL] Pipelining issue detected")
```

## 12.4 Script de Test de CGI Grande

```python
#!/usr/bin/env python3
# test_large_cgi.py

import requests
import time

URL = "http://localhost:8080/cgi-bin/large_output.py"

print("Testing large CGI output...")
start = time.time()

try:
    response = requests.get(URL, timeout=300)
    elapsed = time.time() - start
    
    print(f"Status: {response.status_code}")
    print(f"Content-Length header: {response.headers.get('Content-Length', 'N/A')}")
    print(f"Actual size: {len(response.content)} bytes")
    print(f"Time: {elapsed:.2f}s")
    
    if response.status_code == 200:
        print("[PASS]")
    else:
        print("[FAIL]")
except Exception as e:
    print(f"[FAIL] {e}")
```

---

# RESUMEN FASE 3 (COMPLETA)

## Documentación Generada

| Archivo | Contenido |
|---------|-----------|
| WEBSERV_FASE3_MODULES_RFC.md | CGIHandler, Utils, MimeTypes, Config, RFC compliance |
| WEBSERV_FASE3_PART2_TESTING.md | Ejemplos, tests, debugging |

## Cobertura de Testing

- ✅ Tests básicos (GET, HEAD, POST, PUT, DELETE)
- ✅ Tests de errores (4xx, 5xx)
- ✅ Tests de CGI
- ✅ Tests de pipelining
- ✅ Tests de keep-alive
- ✅ Tests de stress

## Guías Incluidas

- Configuración mínima a producción
- Debugging con strace y valgrind
- Problemas comunes y soluciones
- Scripts de testing automatizados

---

**Fin de Fase 3**

*La Fase 4 (final) cubrirá: Manual de usuario, optimizaciones avanzadas, y checklist de evaluación.*
