# 🚀 Plan de Desarrollo Incremental - Webserv

## Visión General

Este documento estructura el desarrollo del servidor HTTP webserv en **7 Épicas (Milestones)** progresivas. Cada épica contiene historias de usuario con issues específicas que pueden asignarse a diferentes miembros del equipo.

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        ARQUITECTURA DE DESARROLLO                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  EPIC 1: Fundamentos     ──►  EPIC 2: Sockets    ──►  EPIC 3: HTTP Parser   │
│  [Utils, MimeTypes]          [Server básico]         [Request, Response]    │
│                                                                             │
│                    ┌──────────────────────────────────┐                     │
│                    ▼                                  ▼                     │
│  EPIC 4: Configuración   ──►  EPIC 5: Handlers   ──►  EPIC 6: CGI           │
│  [Config parser]             [GET, POST, DELETE]     [Fork, exec, pipes]    │
│                                                                             │
│                              ┌────────────────┐                             │
│                              ▼                                              │
│                         EPIC 7: Features Avanzadas                          │
│                         [Sessions, Keep-alive, Upload]                      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Distribución Sugerida por Equipos

| Equipo | Épicas Principales | Habilidades Clave |
|--------|-------------------|-------------------|
| **A** | 1, 3 | C++ básico, strings, parsing |
| **B** | 2, 5 | Sockets, I/O, sistemas |
| **C** | 4, 6, 7 | Configuración, procesos, features |

---

# 📋 EPIC 1: Fundamentos y Utilidades
**Milestone:** `v0.1-foundation`  
**Duración estimada:** 1 semana  
**Dependencias:** Ninguna  
**Resultado:** Biblioteca de utilidades funcional

## Historia de Usuario 1.1: Utilidades de Strings
> *Como desarrollador, necesito funciones para manipular strings de forma eficiente para poder procesar datos HTTP.*

### Issue #1: Implementar funciones básicas de string
**Archivo:** `src/Utils.cpp`, `inc/Utils.hpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 3

**Tareas:**
- [ ] Crear namespace `Utils`
- [ ] Implementar `trim()` - eliminar espacios al inicio/final
- [ ] Implementar `toLower()` y `toUpper()`
- [ ] Implementar `split()` con char y string delimitadores
- [ ] Implementar `startsWith()` y `endsWith()`
- [ ] Implementar `replaceAll()`

**Criterios de Aceptación:**
```cpp
// Tests que deben pasar:
assert(Utils::trim("  hello  ") == "hello");
assert(Utils::toLower("HeLLo") == "hello");
assert(Utils::split("a,b,c", ',').size() == 3);
assert(Utils::startsWith("hello", "hel") == true);
```

**Código de referencia:**
```cpp
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
```

---

### Issue #2: Implementar conversiones numéricas
**Archivo:** `src/Utils.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 2

**Tareas:**
- [ ] `stringToInt()` - string a entero
- [ ] `stringToSizeT()` - string a size_t
- [ ] `intToString()` - entero a string
- [ ] `sizeTToString()` - size_t a string
- [ ] `hexToSizeT()` - hexadecimal a size_t (para chunked encoding)

**Criterios de Aceptación:**
```cpp
assert(Utils::stringToInt("42") == 42);
assert(Utils::intToString(42) == "42");
assert(Utils::hexToSizeT("1a") == 26);
```

---

### Issue #3: Implementar utilidades de archivos
**Archivo:** `src/Utils.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 5

**Tareas:**
- [ ] `fileExists()` - verificar existencia
- [ ] `isDirectory()` - verificar si es directorio
- [ ] `isReadable()`, `isWritable()`, `isExecutable()`
- [ ] `getFileSize()` - obtener tamaño
- [ ] `getFileExtension()` - obtener extensión
- [ ] `getFileName()` y `getDirectory()`
- [ ] `readFile()` - leer contenido completo
- [ ] `writeFile()` - escribir contenido
- [ ] `deleteFile()` - eliminar archivo
- [ ] `normalizePath()` - normalizar rutas (eliminar `..`, `.`)
- [ ] `joinPath()` - unir rutas

**Criterios de Aceptación:**
```cpp
assert(Utils::getFileExtension("test.html") == "html");
assert(Utils::getFileName("/path/to/file.txt") == "file.txt");
assert(Utils::normalizePath("/a/b/../c") == "/a/c");
```

---

### Issue #4: Implementar utilidades HTTP
**Archivo:** `src/Utils.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 4

**Tareas:**
- [ ] `urlDecode()` - decodificar URL (%20 → espacio)
- [ ] `urlEncode()` - codificar URL
- [ ] `getHttpDate()` - formato RFC 7231
- [ ] `getStatusMessage()` - código a mensaje (200 → "OK")
- [ ] `isValidMethod()` - validar métodos HTTP

**Criterios de Aceptación:**
```cpp
assert(Utils::urlDecode("hello%20world") == "hello world");
assert(Utils::getStatusMessage(404) == "Not Found");
assert(Utils::isValidMethod("GET") == true);
assert(Utils::isValidMethod("INVENTED") == false);
```

---

### Issue #5: Implementar sistema de logging
**Archivo:** `src/Utils.cpp`  
**Asignado a:** ___________  
**Prioridad:** Media  
**Puntos:** 2

**Tareas:**
- [ ] `logInfo()` - mensajes informativos (verde)
- [ ] `logWarning()` - advertencias (amarillo)
- [ ] `logError()` - errores (rojo)
- [ ] `logDebug()` - debug (cyan)
- [ ] Usar códigos ANSI para colores

**Código de referencia:**
```cpp
void logInfo(const std::string& msg) {
    std::cout << "\033[0;32m[INFO]\033[0m " << msg << std::endl;
}
```

---

## Historia de Usuario 1.2: Tipos MIME
> *Como servidor web, necesito identificar el tipo de contenido de los archivos para enviar el header Content-Type correcto.*

### Issue #6: Implementar singleton MimeTypes
**Archivo:** `src/MimeTypes.cpp`, `inc/MimeTypes.hpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 3

**Tareas:**
- [ ] Implementar patrón Singleton
- [ ] Crear mapa de extensiones → tipos MIME
- [ ] `getMimeType(extension)` - obtener por extensión
- [ ] `getMimeTypeByFile(filename)` - obtener por nombre de archivo
- [ ] `isTextType()` y `isBinaryType()`
- [ ] `addMimeType()` - añadir tipos personalizados

**Tipos MIME mínimos requeridos:**
```
html → text/html
css  → text/css
js   → application/javascript
json → application/json
png  → image/png
jpg  → image/jpeg
gif  → image/gif
txt  → text/plain
pdf  → application/pdf
```

**Criterios de Aceptación:**
```cpp
MimeTypes& mime = MimeTypes::getInstance();
assert(mime.getMimeType("html") == "text/html");
assert(mime.getMimeTypeByFile("style.css") == "text/css");
```

---

## Historia de Usuario 1.3: Header Principal
> *Como desarrollador, necesito un header central que incluya todas las dependencias y constantes del proyecto.*

### Issue #7: Crear webserv.hpp
**Archivo:** `inc/webserv.hpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 2

**Tareas:**
- [ ] Incluir headers C++ estándar necesarios
- [ ] Incluir headers de sistema (socket, poll, etc.)
- [ ] Definir constantes del servidor:
  - `WEBSERV_VERSION "1.0.0"`
  - `BUFFER_SIZE 65536`
  - `MAX_CLIENTS 1024`
  - `CONNECTION_TIMEOUT 60`
  - `CGI_TIMEOUT 30`
- [ ] Definir códigos HTTP como macros
- [ ] Definir colores para logging

---

# 📋 EPIC 2: Infraestructura de Red
**Milestone:** `v0.2-networking`  
**Duración estimada:** 2 semanas  
**Dependencias:** Epic 1  
**Resultado:** Servidor que acepta conexiones TCP

## Historia de Usuario 2.1: Socket de Escucha
> *Como servidor, necesito crear sockets TCP para escuchar conexiones entrantes.*

### Issue #8: Crear estructura básica de Server
**Archivo:** `src/Server.cpp`, `inc/Server.hpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 5

**Tareas:**
- [ ] Definir clase `Server` con constructor/destructor
- [ ] Atributos privados:
  - `_running` (bool)
  - `_listenSockets` (map<int, int>) fd → puerto
  - `_pollFds` (vector<pollfd>)
- [ ] Implementar `_createSocket(host, port)`:
  - Crear socket con `socket(AF_INET, SOCK_STREAM, 0)`
  - Configurar `SO_REUSEADDR`
  - Hacer `bind()` y `listen()`
- [ ] Implementar `_setNonBlocking(fd)`:
  - Usar `fcntl(fd, F_SETFL, O_NONBLOCK)`

**Código de referencia:**
```cpp
int Server::_createSocket(const std::string& host, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;
    
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return -1;
    }
    
    if (listen(sockfd, 128) < 0) {
        close(sockfd);
        return -1;
    }
    
    _setNonBlocking(sockfd);
    return sockfd;
}
```

---

### Issue #9: Implementar event loop con poll()
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 8

**Tareas:**
- [ ] Implementar `_rebuildPollFds()`:
  - Añadir sockets de escucha con POLLIN
  - Añadir sockets de clientes con eventos apropiados
- [ ] Implementar `run()`:
  - Loop principal mientras `_running`
  - Llamar `poll()` con timeout de 1000ms
  - Procesar eventos (POLLIN, POLLOUT, POLLERR)
- [ ] Implementar `stop()`:
  - Poner `_running = false`

**Diagrama del event loop:**
```
┌─────────────────────────────────────────┐
│              while(_running)            │
├─────────────────────────────────────────┤
│  1. _rebuildPollFds()                   │
│  2. poll(&_pollFds[0], size, 1000)      │
│  3. for each fd with events:            │
│     ├─ if listen socket → accept()      │
│     ├─ if client POLLIN → read()        │
│     └─ if client POLLOUT → write()      │
│  4. _checkTimeouts()                    │
└─────────────────────────────────────────┘
```

---

### Issue #10: Manejar señales de sistema
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 2

**Tareas:**
- [ ] Crear variable global `g_serverRunning`
- [ ] Implementar `_signalHandler(int sig)`:
  - Poner `g_serverRunning = 0`
- [ ] En `init()`:
  - Configurar handler para SIGINT y SIGTERM
  - Ignorar SIGPIPE con `signal(SIGPIPE, SIG_IGN)`

---

## Historia de Usuario 2.2: Gestión de Clientes
> *Como servidor, necesito gestionar múltiples conexiones de clientes simultáneamente.*

### Issue #11: Implementar clase Client
**Archivo:** `src/Client.cpp`, `inc/Client.hpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 5

**Tareas:**
- [ ] Definir enum `ClientState`:
  ```cpp
  enum ClientState {
      CLIENT_READING,
      CLIENT_PROCESSING,
      CLIENT_WRITING,
      CLIENT_CGI_RUNNING,
      CLIENT_DONE,
      CLIENT_ERROR
  };
  ```
- [ ] Atributos del cliente:
  - `_fd`, `_ip`, `_port`
  - `_state`
  - `_readBuffer`, `_writeBuffer`
  - `_lastActivity` (time_t)
- [ ] Métodos de buffer:
  - `appendToReadBuffer(data, len)`
  - `appendToWriteBuffer(data)`
  - `eraseFromWriteBuffer(len)`
- [ ] Métodos de estado:
  - `isReadyToRead()`, `isReadyToWrite()`
  - `hasTimedOut(timeout)`
  - `updateLastActivity()`

---

### Issue #12: Aceptar nuevas conexiones
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 3

**Tareas:**
- [ ] Implementar `_acceptNewConnection(listenFd)`:
  - `accept()` para obtener nuevo fd
  - Verificar límite de clientes
  - Hacer socket no bloqueante
  - Crear objeto `Client` y añadir a `_clients`
  - Log de nueva conexión

**Código de referencia:**
```cpp
void Server::_acceptNewConnection(int listenFd) {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    
    int clientFd = accept(listenFd, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientFd < 0) return;
    
    if (_clients.size() >= MAX_CLIENTS) {
        close(clientFd);
        return;
    }
    
    _setNonBlocking(clientFd);
    
    std::string ip = inet_ntoa(clientAddr.sin_addr);
    Client client(clientFd, ip, _listenSockets[listenFd]);
    _clients[clientFd] = client;
    
    Utils::logDebug("New connection from " + ip);
}
```

---

### Issue #13: Leer y escribir datos de clientes
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 4

**Tareas:**
- [ ] Implementar `_handleClientRead(clientFd)`:
  - `recv()` datos al buffer
  - Si 0 bytes → cerrar conexión
  - Si error EAGAIN → ignorar
  - Actualizar `lastActivity`
- [ ] Implementar `_handleClientWrite(clientFd)`:
  - `send()` datos del write buffer
  - Si error EAGAIN → ignorar
  - Borrar datos enviados del buffer
  - Si buffer vacío → según keep-alive

---

### Issue #14: Cerrar conexiones y timeouts
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 3

**Tareas:**
- [ ] Implementar `_closeClient(clientFd)`:
  - Limpiar recursos CGI si hay
  - Cerrar socket
  - Eliminar de `_clients`
  - Log de cierre
- [ ] Implementar `_checkTimeouts()`:
  - Iterar clientes
  - Si `hasTimedOut(CONNECTION_TIMEOUT)` → cerrar
  - Si CGI timeout → cerrar

---

# 📋 EPIC 3: Parser HTTP
**Milestone:** `v0.3-http-parser`  
**Duración estimada:** 2 semanas  
**Dependencias:** Epic 1  
**Resultado:** Parser de peticiones y generador de respuestas HTTP

## Historia de Usuario 3.1: Parser de Request
> *Como servidor, necesito parsear peticiones HTTP/1.1 para extraer método, URI, headers y body.*

### Issue #15: Estructura base de Request
**Archivo:** `src/Request.cpp`, `inc/Request.hpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 3

**Tareas:**
- [ ] Definir enum `ParseState`:
  ```cpp
  enum ParseState {
      PARSE_REQUEST_LINE,
      PARSE_HEADERS,
      PARSE_BODY,
      PARSE_CHUNKED,
      PARSE_COMPLETE,
      PARSE_ERROR
  };
  ```
- [ ] Atributos privados:
  - `_method`, `_uri`, `_path`, `_query`, `_version`
  - `_headers` (map<string, string>)
  - `_body`
  - `_state`, `_errorCode`
  - `_contentLength`, `_bodyBytesReceived`
  - `_isChunked`
- [ ] Constructor que inicializa estado a `PARSE_REQUEST_LINE`
- [ ] `reset()` para reutilizar objeto

---

### Issue #16: Parsear línea de petición
**Archivo:** `src/Request.cpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 4

**Tareas:**
- [ ] Implementar `_parseRequestLine(line)`:
  - Split por espacios → [METHOD, URI, VERSION]
  - Validar que hay 3 partes
  - Validar método con `Utils::isValidMethod()`
  - Validar versión (HTTP/1.0 o HTTP/1.1)
  - Si método inválido → error 501
  - Si versión inválida → error 505
- [ ] Implementar `_parseUri()`:
  - Extraer fragment (#...)
  - Extraer query string (?...)
  - URL decode del path
  - Normalizar path

**Formato de línea de petición:**
```
GET /path/to/resource?query=value HTTP/1.1
│   │                 │           │
│   │                 │           └─ VERSION
│   │                 └─ QUERY STRING
│   └─ PATH (URI)
└─ METHOD
```

---

### Issue #17: Parsear headers HTTP
**Archivo:** `src/Request.cpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 4

**Tareas:**
- [ ] Implementar `_parseHeader(line)`:
  - Encontrar primer ':'
  - Extraer name y value
  - Normalizar nombre del header (Content-Type)
  - Almacenar en `_headers`
- [ ] Implementar `_normalizeHeaderName(name)`:
  - Primera letra mayúscula después de '-'
- [ ] En parse loop:
  - Línea vacía `\r\n` indica fin de headers
  - Detectar Content-Length
  - Detectar Transfer-Encoding: chunked

**Headers importantes a reconocer:**
```
Host: localhost:8080
Content-Type: application/json
Content-Length: 42
Transfer-Encoding: chunked
Connection: keep-alive
Cookie: session=abc123
```

---

### Issue #18: Parsear body normal y chunked
**Archivo:** `src/Request.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 5

**Tareas:**
- [ ] Parsear body con Content-Length:
  - Leer `_contentLength` bytes
  - Ir acumulando en `_body`
  - Cuando `_bodyBytesReceived >= _contentLength` → COMPLETE
- [ ] Implementar `_parseChunkedBody()`:
  - Formato: `SIZE\r\nDATA\r\n`
  - Tamaño en hexadecimal
  - Chunk de tamaño 0 indica fin
  - Puede haber trailer headers

**Formato chunked:**
```
POST /upload HTTP/1.1
Transfer-Encoding: chunked

7\r\n
Mozilla\r\n
9\r\n
Developer\r\n
0\r\n
\r\n
```

---

### Issue #19: Parsear datos especiales
**Archivo:** `src/Request.cpp`  
**Asignado a:** ___________  
**Prioridad:** Media  
**Puntos:** 4

**Tareas:**
- [ ] Implementar `_parseHost()`:
  - Extraer hostname y puerto del header Host
- [ ] Implementar `_parseCookies()`:
  - Parsear header Cookie
  - Format: `name1=value1; name2=value2`
- [ ] Implementar `_parseQueryString()`:
  - Format: `key1=value1&key2=value2`
  - URL decode keys y values
- [ ] Implementar `_parseMultipartBody()`:
  - Detectar boundary en Content-Type
  - Parsear cada parte
  - Extraer filename, content-type, data

---

### Issue #20: Método principal parse()
**Archivo:** `src/Request.cpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 3

**Tareas:**
- [ ] Implementar `parse(data)`:
  - Llamar `appendData(data)`
  - Retornar `isComplete()`
- [ ] Implementar `appendData(data)`:
  - Acumular en `_rawRequest`
  - Estado PARSE_REQUEST_LINE:
    - Buscar `\r\n`
    - Parsear línea
    - Cambiar a PARSE_HEADERS
  - Estado PARSE_HEADERS:
    - Buscar `\r\n`
    - Si línea vacía → procesar body o COMPLETE
    - Si no → parsear header
  - Estado PARSE_BODY/CHUNKED:
    - Acumular body
    - Verificar completitud

---

## Historia de Usuario 3.2: Generador de Response
> *Como servidor, necesito construir respuestas HTTP válidas con headers y body.*

### Issue #21: Estructura base de Response
**Archivo:** `src/Response.cpp`, `inc/Response.hpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 3

**Tareas:**
- [ ] Atributos:
  - `_statusCode` (int)
  - `_headers` (map<string, string>)
  - `_body` (string)
  - `_sent`, `_bytesSent`
- [ ] Constructor con defaults:
  - Status 200
  - Headers por defecto (Server, Date, Connection)
- [ ] Setters:
  - `setStatusCode(code)`
  - `setHeader(name, value)`
  - `setBody(body)` - también actualiza Content-Length
  - `setContentType(type)`
  - `appendBody(data)`

---

### Issue #22: Construir respuesta HTTP
**Archivo:** `src/Response.cpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 3

**Tareas:**
- [ ] Implementar `build()`:
  - Construir string completo de respuesta
  - `buildHeaders()` + body
- [ ] Implementar `buildHeaders()`:
  - Status line: `HTTP/1.1 CODE MESSAGE\r\n`
  - Headers: `Name: Value\r\n`
  - Línea vacía final: `\r\n`

**Formato de respuesta:**
```
HTTP/1.1 200 OK\r\n
Server: Webserv/1.0\r\n
Date: Mon, 01 Jan 2024 00:00:00 GMT\r\n
Content-Type: text/html\r\n
Content-Length: 13\r\n
Connection: keep-alive\r\n
\r\n
Hello, World!
```

---

### Issue #23: Builders estáticos de Response
**Archivo:** `src/Response.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 5

**Tareas:**
- [ ] `makeError(code, config)`:
  - Buscar página de error personalizada
  - Si no hay, generar HTML por defecto
  - Estilo visual atractivo
- [ ] `makeRedirect(code, location)`:
  - Header Location
  - Body con link
- [ ] `makeFile(path, contentType)`:
  - Leer archivo
  - Configurar Content-Type
- [ ] `makeDirectoryListing(path, uri)`:
  - Generar HTML con listado
  - Links a archivos/directorios
  - Estilos CSS inline
- [ ] `makeFromCGI(cgiOutput)`:
  - Parsear headers del CGI
  - Detectar Status header
  - Extraer body

---

# 📋 EPIC 4: Sistema de Configuración
**Milestone:** `v0.4-config`  
**Duración estimada:** 1.5 semanas  
**Dependencias:** Epic 1  
**Resultado:** Parser de archivos de configuración estilo NGINX

## Historia de Usuario 4.1: Estructura de Configuración
> *Como administrador, necesito configurar el servidor mediante archivos de texto con sintaxis clara.*

### Issue #24: Implementar LocationConfig
**Archivo:** `src/LocationConfig.cpp`, `inc/LocationConfig.hpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 4

**Tareas:**
- [ ] Atributos:
  - `_path` - ruta del location
  - `_root` - directorio raíz
  - `_index` - archivo índice
  - `_autoindex` - listado de directorios
  - `_allowedMethods` (set<string>)
  - `_redirect`, `_redirectCode`
  - `_cgiHandlers` (map<ext, handler>)
  - `_uploadPath`, `_uploadEnabled`
  - `_alias`
- [ ] Getters y setters
- [ ] `isMethodAllowed(method)`
- [ ] `getCgiHandler(extension)`
- [ ] `hasRedirect()`

---

### Issue #25: Implementar ServerConfig
**Archivo:** `src/ServerConfig.cpp`, `inc/ServerConfig.hpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 4

**Tareas:**
- [ ] Atributos:
  - `_host`, `_port`
  - `_serverNames` (vector<string>)
  - `_root`, `_index`
  - `_maxBodySize`
  - `_errorPages` (map<int, string>)
  - `_locations` (vector<LocationConfig>)
  - `_autoindex`
- [ ] Getters y setters
- [ ] `findLocation(uri)`:
  - Buscar location más específica
  - Match por prefijo más largo
- [ ] `matchServerName(host)`:
  - Comparar con server_names
- [ ] `isValid()` - validar configuración

---

### Issue #26: Parser de configuración
**Archivo:** `src/Config.cpp`, `inc/Config.hpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 8

**Tareas:**
- [ ] `parse(filename)`:
  - Abrir archivo
  - Leer contenido
  - Llamar `parseFromString()`
- [ ] `_removeComments(content)`:
  - Eliminar líneas que empiezan con #
- [ ] `_getNextToken(stream)`:
  - Saltear whitespace
  - Retornar siguiente token
  - Reconocer `{`, `}`, `;`
- [ ] `_parseServer(stream)`:
  - Crear ServerConfig
  - Parsear directivas hasta `}`
- [ ] `_parseLocation(stream, server)`:
  - Crear LocationConfig
  - Parsear directivas hasta `}`
- [ ] `_parseDirective(stream, server, directive)`:
  - listen, server_name, root, index, etc.
- [ ] `_parseSize(str)`:
  - Soportar sufijos K, M, G

**Directivas a soportar:**
```nginx
server {
    listen 0.0.0.0:8080;
    server_name localhost;
    root ./www;
    index index.html;
    client_max_body_size 1M;
    autoindex on;
    error_page 404 /errors/404.html;
    
    location / {
        methods GET POST;
        autoindex on;
    }
    
    location /cgi-bin {
        cgi .py /usr/bin/python3;
    }
    
    location /old {
        redirect 301 /new;
    }
}
```

---

### Issue #27: Validación de configuración
**Archivo:** `src/Config.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 3

**Tareas:**
- [ ] `validate()`:
  - Verificar que hay al menos un server
  - Verificar puertos válidos (1-65535)
  - Verificar no hay server_names duplicados en mismo puerto
  - Verificar rutas existen o son creables
- [ ] Clase `ConfigException`:
  - Heredar de `std::exception`
  - Mensaje de error descriptivo
- [ ] `findServer(host, port)`:
  - Buscar server por Host header
  - Retornar default si no hay match

---

# 📋 EPIC 5: Handlers HTTP
**Milestone:** `v0.5-handlers`  
**Duración estimada:** 2 semanas  
**Dependencias:** Epics 2, 3, 4  
**Resultado:** Servidor que sirve archivos estáticos y maneja métodos básicos

## Historia de Usuario 5.1: Procesar Peticiones
> *Como servidor, necesito routing de peticiones a handlers apropiados.*

### Issue #28: Implementar _processRequest()
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 5

**Tareas:**
- [ ] Seleccionar ServerConfig según Host header
- [ ] Encontrar LocationConfig que matchea
- [ ] Verificar redirect → `_handleRedirect()`
- [ ] Verificar método permitido:
  - HEAD debe ser permitido donde GET es permitido
- [ ] Verificar tamaño de body
- [ ] Routing por método:
  - GET/HEAD → `_handleGet()`
  - POST → `_handlePost()`
  - DELETE → `_handleDelete()`

**Flujo:**
```
Request → Select Server → Find Location → Check Redirect
                                              ↓
                                        Check Method
                                              ↓
                                        Check Body Size
                                              ↓
                                        Route to Handler
```

---

### Issue #29: Resolver rutas de archivos
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 3

**Tareas:**
- [ ] Implementar `_resolvePath(request, location, server)`:
  - Obtener root de location o server
  - Si hay alias → reemplazar path del location
  - Si no → concatenar root + URI
  - Normalizar path resultante
  - Verificar que no sale del root (path traversal)

---

### Issue #30: Handler GET para archivos
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 6

**Tareas:**
- [ ] Implementar `_handleGet()`:
  - Resolver path del archivo
  - Si es CGI → `_handleCgi()`
  - `stat()` para verificar existencia
  - Si directorio:
    - Buscar archivo índice
    - Si no hay y autoindex → listado
    - Si no → 403 Forbidden
  - Si archivo:
    - Verificar permisos de lectura
    - Leer contenido
    - Determinar MIME type
    - Construir Response
  - Si HEAD → enviar solo headers

---

### Issue #31: Handler DELETE
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 2

**Tareas:**
- [ ] Implementar `_handleDelete()`:
  - Resolver path
  - Verificar que existe
  - Verificar que es archivo (no directorio)
  - `std::remove()` para eliminar
  - Si éxito → 204 No Content
  - Si falla → error apropiado

---

### Issue #32: Enviar respuestas de error
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 2

**Tareas:**
- [ ] Implementar `_sendErrorResponse(client, code)`:
  - Crear Response con `Response::makeError()`
  - Añadir al write buffer del cliente
  - Marcar keep-alive = false
  - Cambiar estado a WRITING

---

## Historia de Usuario 5.2: Listado de Directorios
> *Como usuario, quiero ver el contenido de directorios cuando autoindex está habilitado.*

### Issue #33: Generar listado de directorios
**Archivo:** `src/Response.cpp` o `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Media  
**Puntos:** 4

**Tareas:**
- [ ] Implementar `makeDirectoryListing(path, uri)`:
  - Abrir directorio con `opendir()`
  - Leer entradas con `readdir()`
  - Ordenar alfabéticamente
  - Generar HTML con tabla
  - Mostrar: nombre, tamaño, fecha
  - Link ".." para directorio padre
  - Estilos CSS inline para presentación

**HTML de ejemplo:**
```html
<!DOCTYPE html>
<html>
<head><title>Index of /uploads/</title></head>
<body>
<h1>Index of /uploads/</h1>
<table>
  <tr><th>Name</th><th>Size</th><th>Modified</th></tr>
  <tr><td><a href="../">../</a></td><td>-</td><td>-</td></tr>
  <tr><td><a href="file.txt">file.txt</a></td><td>1234</td><td>2024-01-01</td></tr>
</table>
</body>
</html>
```

---

# 📋 EPIC 6: CGI (Common Gateway Interface)
**Milestone:** `v0.6-cgi`  
**Duración estimada:** 2 semanas  
**Dependencias:** Epic 5  
**Resultado:** Soporte para scripts CGI (Python, PHP, etc.)

## Historia de Usuario 6.1: Ejecutar Scripts CGI
> *Como servidor, necesito ejecutar scripts externos y retornar su output como respuesta HTTP.*

### Issue #34: Estructura CGIHandler
**Archivo:** `src/CGIHandler.cpp`, `inc/CGIHandler.hpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 3

**Tareas:**
- [ ] Atributos:
  - `_request` - referencia a Request
  - `_serverConfig`, `_locationConfig`
  - `_scriptPath` - ruta al script
  - `_cgiExecutable` - intérprete (python3, php-cgi)
  - `_clientIp`, `_clientPort`
  - `_output` - salida del CGI
  - `_exitStatus`
  - `_hasError`, `_errorMessage`
- [ ] Setters para configurar
- [ ] Getters para resultados

---

### Issue #35: Construir entorno CGI
**Archivo:** `src/CGIHandler.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 5

**Tareas:**
- [ ] Implementar `_buildEnvironment()`:
  - Variables obligatorias RFC 3875:
    ```
    GATEWAY_INTERFACE=CGI/1.1
    SERVER_PROTOCOL=HTTP/1.1
    REQUEST_METHOD=GET/POST/etc
    SCRIPT_NAME=/cgi-bin/script.py
    SCRIPT_FILENAME=/full/path/to/script.py
    QUERY_STRING=param=value
    PATH_INFO=/extra/path
    CONTENT_TYPE=application/x-www-form-urlencoded
    CONTENT_LENGTH=42
    SERVER_NAME=localhost
    SERVER_PORT=8080
    REMOTE_ADDR=127.0.0.1
    ```
  - Convertir HTTP headers a `HTTP_*`:
    ```
    Host: localhost → HTTP_HOST=localhost
    User-Agent: curl → HTTP_USER_AGENT=curl
    ```
- [ ] `_envToCharArray()` - convertir vector a char**
- [ ] `_freeCharArray()` - liberar memoria

---

### Issue #36: Fork y exec del CGI
**Archivo:** `src/CGIHandler.cpp`  
**Asignado a:** ___________  
**Prioridad:** Crítica  
**Puntos:** 6

**Tareas:**
- [ ] Implementar `startExecution(fdIn, fdOut, pid)`:
  - Crear pipes para stdin y stdout
  - `fork()` para crear proceso hijo
  - En hijo:
    - Redirigir stdin/stdout con `dup2()`
    - Cambiar directorio al del script
    - `execve()` con intérprete y script
    - `_exit(1)` si execve falla
  - En padre:
    - Cerrar extremos no usados de pipes
    - Guardar fds y pid
    - Hacer pipes non-blocking

**Diagrama de pipes:**
```
┌──────────┐    pipeIn[1]     ┌──────────┐
│  Parent  │ ──────────────►  │  Child   │
│ (Server) │                  │  (CGI)   │
│          │ ◄──────────────  │          │
└──────────┘    pipeOut[0]    └──────────┘
```

---

### Issue #37: Integrar CGI en Server
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 5

**Tareas:**
- [ ] Implementar `_isCgiRequest(path, location)`:
  - Verificar extensión del archivo
  - Buscar handler en location config
- [ ] Implementar `_handleCgi()`:
  - Verificar que script existe
  - Crear pipes
  - Fork proceso
  - Escribir body al stdin del CGI
  - Registrar fd de salida en poll
  - Cambiar estado cliente a CGI_RUNNING
- [ ] Implementar `_handleCgiRead(cgiFd)`:
  - Leer output del CGI
  - Acumular en `cgiOutput` del cliente
  - Si EOF o error:
    - Cerrar pipe
    - `waitpid()` para recoger hijo
    - Llamar `_prepareCgiResponse()`
- [ ] Implementar `_prepareCgiResponse()`:
  - Parsear output del CGI
  - Crear Response
  - Añadir a write buffer

---

### Issue #38: Parsear output CGI
**Archivo:** `src/CGIHandler.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 3

**Tareas:**
- [ ] Implementar `parseCgiOutput(output, headers, body, statusCode)`:
  - Encontrar fin de headers (`\r\n\r\n` o `\n\n`)
  - Parsear headers
  - Si hay header "Status:" → extraer código
  - Retornar headers, body y status

**Formato output CGI:**
```
Content-Type: text/html
Status: 200 OK

<html>
<body>Hello from CGI!</body>
</html>
```

---

# 📋 EPIC 7: Features Avanzadas
**Milestone:** `v0.7-advanced`  
**Duración estimada:** 1.5 semanas  
**Dependencias:** Epics 5, 6  
**Resultado:** Servidor completo con sesiones, uploads y keep-alive

## Historia de Usuario 7.1: Upload de Archivos
> *Como usuario, quiero poder subir archivos al servidor mediante POST multipart.*

### Issue #39: Handler POST para uploads
**Archivo:** `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 5

**Tareas:**
- [ ] Implementar `_handlePost()`:
  - Si es CGI → `_handleCgi()`
  - Si upload habilitado → `_handleFileUpload()`
  - Si no → 405
- [ ] Implementar `_handleFileUpload()`:
  - Obtener `uploadPath` de location
  - Crear directorio si no existe
  - Si multipart:
    - Usar `getUploadedFiles()` del Request
    - Guardar cada archivo
    - Sanitizar nombres de archivo
  - Si no multipart:
    - Guardar body como archivo
  - Responder 201 Created

---

## Historia de Usuario 7.2: Gestión de Sesiones
> *Como servidor, necesito mantener estado entre peticiones mediante cookies de sesión.*

### Issue #40: Implementar SessionManager
**Archivo:** `src/SessionManager.cpp`, `inc/SessionManager.hpp`  
**Asignado a:** ___________  
**Prioridad:** Media  
**Puntos:** 5

**Tareas:**
- [ ] Patrón Singleton
- [ ] Estructura `Session`:
  - `id`, `data` (map), `createdAt`, `lastAccessedAt`
- [ ] `createSession()`:
  - Generar ID aleatorio de 32 caracteres
  - Asegurar unicidad
- [ ] `getSession(id)` - obtener sesión
- [ ] `destroySession(id)` - eliminar sesión
- [ ] Operaciones de datos:
  - `set(sessionId, key, value)`
  - `get(sessionId, key)`
  - `has(sessionId, key)`
  - `remove(sessionId, key)`
- [ ] `cleanExpiredSessions()`:
  - Eliminar sesiones con timeout
  - Llamar periódicamente en run()

---

## Historia de Usuario 7.3: Keep-Alive
> *Como servidor, quiero reutilizar conexiones TCP para múltiples peticiones.*

### Issue #41: Soporte Keep-Alive
**Archivo:** `src/Client.cpp`, `src/Server.cpp`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 3

**Tareas:**
- [ ] En Client:
  - `shouldKeepAlive()`:
    - HTTP/1.1 default es keep-alive
    - HTTP/1.0 requiere header explícito
    - Si Connection: close → false
  - `reset()` - preparar para siguiente request
- [ ] En Server `_handleClientWrite()`:
  - Si buffer vacío y keep-alive:
    - Llamar `client.reset()`
    - No cerrar conexión
  - Si buffer vacío y no keep-alive:
    - Cerrar conexión

---

## Historia de Usuario 7.4: Main y Banner
> *Como usuario, quiero un punto de entrada limpio con opciones de línea de comandos.*

### Issue #42: Implementar main.cpp
**Archivo:** `src/main.cpp`  
**Asignado a:** ___________  
**Prioridad:** Media  
**Puntos:** 3

**Tareas:**
- [ ] Parsear argumentos:
  - `-h, --help` - mostrar ayuda
  - `-v, --version` - mostrar versión
  - Archivo de configuración opcional
- [ ] `printBanner()` - ASCII art
- [ ] `printUsage()` - instrucciones de uso
- [ ] Setup de señales
- [ ] Cargar configuración
- [ ] Inicializar y ejecutar servidor
- [ ] Manejo de errores con mensajes claros

---

# 📋 Checklist de Integración Final

## Tests de Integración

### Issue #43: Suite de tests automatizada
**Archivo:** `tests/test_suite.sh`  
**Asignado a:** ___________  
**Prioridad:** Alta  
**Puntos:** 5

**Tests requeridos:**
- [ ] GET / returns 200
- [ ] GET /nonexistent returns 404
- [ ] HEAD / returns 200 (sin body)
- [ ] POST to CGI works
- [ ] DELETE file returns 204
- [ ] Unknown method returns 501
- [ ] Redirect returns 301/302
- [ ] Upload returns 201
- [ ] Body size limit returns 413
- [ ] Directory listing works
- [ ] MIME types correctos
- [ ] Keep-alive funciona
- [ ] CGI timeout manejado
- [ ] Conexiones concurrentes

---

# 📊 Resumen de Asignaciones

## Tabla de Issues

| Issue | Título | Epic | Puntos | Asignado |
|-------|--------|------|--------|----------|
| #1 | Funciones básicas de string | 1 | 3 | |
| #2 | Conversiones numéricas | 1 | 2 | |
| #3 | Utilidades de archivos | 1 | 5 | |
| #4 | Utilidades HTTP | 1 | 4 | |
| #5 | Sistema de logging | 1 | 2 | |
| #6 | Singleton MimeTypes | 1 | 3 | |
| #7 | Header webserv.hpp | 1 | 2 | |
| #8 | Estructura básica Server | 2 | 5 | |
| #9 | Event loop con poll() | 2 | 8 | |
| #10 | Señales de sistema | 2 | 2 | |
| #11 | Clase Client | 2 | 5 | |
| #12 | Aceptar conexiones | 2 | 3 | |
| #13 | Leer/escribir datos | 2 | 4 | |
| #14 | Cerrar y timeouts | 2 | 3 | |
| #15 | Estructura Request | 3 | 3 | |
| #16 | Parsear request line | 3 | 4 | |
| #17 | Parsear headers | 3 | 4 | |
| #18 | Parsear body | 3 | 5 | |
| #19 | Parsear datos especiales | 3 | 4 | |
| #20 | Método parse() | 3 | 3 | |
| #21 | Estructura Response | 3 | 3 | |
| #22 | Construir respuesta | 3 | 3 | |
| #23 | Builders estáticos | 3 | 5 | |
| #24 | LocationConfig | 4 | 4 | |
| #25 | ServerConfig | 4 | 4 | |
| #26 | Parser de configuración | 4 | 8 | |
| #27 | Validación de config | 4 | 3 | |
| #28 | _processRequest() | 5 | 5 | |
| #29 | Resolver rutas | 5 | 3 | |
| #30 | Handler GET | 5 | 6 | |
| #31 | Handler DELETE | 5 | 2 | |
| #32 | Respuestas de error | 5 | 2 | |
| #33 | Listado de directorios | 5 | 4 | |
| #34 | Estructura CGIHandler | 6 | 3 | |
| #35 | Entorno CGI | 6 | 5 | |
| #36 | Fork y exec | 6 | 6 | |
| #37 | Integrar CGI en Server | 6 | 5 | |
| #38 | Parsear output CGI | 6 | 3 | |
| #39 | Handler POST/Upload | 7 | 5 | |
| #40 | SessionManager | 7 | 5 | |
| #41 | Keep-Alive | 7 | 3 | |
| #42 | main.cpp | 7 | 3 | |
| #43 | Suite de tests | - | 5 | |

**Total de puntos:** 165

---

# 🗓️ Cronograma Sugerido

```
Semana 1-2:   Epic 1 (Fundamentos)     - Todos en paralelo
Semana 2-3:   Epic 2 (Sockets)         - Equipo B
              Epic 3 (HTTP Parser)     - Equipo A
Semana 3-4:   Epic 4 (Config)          - Equipo C
              Integración parcial
Semana 4-5:   Epic 5 (Handlers)        - Todos
Semana 5-6:   Epic 6 (CGI)             - Equipo C
              Epic 7 (Advanced)        - Equipo A, B
Semana 6-7:   Testing e integración final
```

---

# 📝 Notas para el Equipo

## Convenciones de Código

1. **Norminette:** Respetar la norma de 42 (aunque en C++)
2. **Indentación:** Tabs o espacios consistentes
3. **Nombres:** 
   - Clases: PascalCase (`ServerConfig`)
   - Métodos: camelCase (`getServerConfig`)
   - Atributos privados: `_prefixUnderscore`
   - Constantes: UPPER_SNAKE_CASE
4. **Comentarios:** En inglés, explicar el "por qué" no el "qué"

## Git Workflow

```bash
# Crear branch para cada issue
git checkout -b feature/issue-XX-descripcion

# Commits atómicos
git commit -m "feat(utils): implement trim() function"

# Pull request para merge
# Code review obligatorio
```

## Comunicación

- Daily standup de 10 min
- Documentar decisiones de diseño
- Preguntar dudas antes de asumir

---

¡Buena suerte con el desarrollo! 🚀
