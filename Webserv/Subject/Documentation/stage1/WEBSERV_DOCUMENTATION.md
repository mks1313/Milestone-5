# WEBSERV - Documentación Técnica Completa

## Servidor HTTP/1.1 Compatible con RFC 7230-7235

**Versión:** 1.0.0  
**Autor:** fcela-ga (Felipe Cela García)  
**Proyecto:** 42 Barcelona - webserv  
**Fecha de documentación:** Enero 2026

---

# ÍNDICE

1. [Resumen Ejecutivo](#1-resumen-ejecutivo)
2. [Arquitectura General](#2-arquitectura-general)
3. [Módulos del Sistema](#3-módulos-del-sistema)
4. [Flujo de Procesamiento de Peticiones](#4-flujo-de-procesamiento-de-peticiones)
5. [Protocolo HTTP/1.1 Implementado](#5-protocolo-http11-implementado)
6. [Referencias RFC](#6-referencias-rfc)
7. [Sistema de Configuración](#7-sistema-de-configuración)
8. [Manejo de CGI](#8-manejo-de-cgi)
9. [Gestión de Sesiones](#9-gestión-de-sesiones)
10. [Análisis Técnico Detallado](#10-análisis-técnico-detallado)

---

# 1. RESUMEN EJECUTIVO

## 1.1 Descripción General

**webserv** es un servidor HTTP/1.1 desarrollado en C++98 que implementa un subconjunto del protocolo HTTP definido en las RFC 7230-7235. El servidor está diseñado para cumplir con los requisitos del proyecto de 42 Barcelona, proporcionando:

- **Multiplexación I/O no bloqueante** mediante `poll()`
- **Soporte de Virtual Hosts** (múltiples servidores en el mismo puerto)
- **Métodos HTTP**: GET, POST, PUT, DELETE, HEAD
- **Ejecución CGI** para scripts dinámicos (Python, PHP, Perl, Shell)
- **Uploads de archivos** via multipart/form-data
- **Transferencia Chunked** (RFC 7230 §4.1)
- **Conexiones persistentes** (Keep-Alive)
- **Gestión de sesiones** mediante cookies

## 1.2 Constantes del Sistema

| Constante | Valor | Descripción |
|-----------|-------|-------------|
| `BUFFER_SIZE` | 65536 bytes | Tamaño del buffer de lectura/escritura |
| `MAX_CLIENTS` | 1024 | Número máximo de conexiones simultáneas |
| `MAX_HEADER_SIZE` | 8192 bytes | Tamaño máximo de headers (RFC 7230 recomienda ≥8000) |
| `MAX_URI_LENGTH` | 8192 bytes | Longitud máxima de URI |
| `DEFAULT_MAX_BODY_SIZE` | 1 MB | Tamaño máximo por defecto del body |
| `MAX_CGI_OUTPUT_SIZE` | ~200 MB | Tamaño máximo de salida CGI |
| `MAX_CONCURRENT_CGI` | 5 | CGIs concurrentes máximos |
| `CONNECTION_TIMEOUT` | 60 seg | Timeout de conexión |
| `CGI_TIMEOUT` | 120 seg | Timeout de ejecución CGI |
| `BACKLOG` | 128 | Cola de conexiones pendientes |

## 1.3 Códigos de Estado HTTP Soportados

```
2xx Éxito:     200 OK, 201 Created, 204 No Content
3xx Redirección: 301 Moved Permanently, 302 Found, 304 Not Modified, 307 Temporary Redirect
4xx Error Cliente: 400 Bad Request, 401 Unauthorized, 403 Forbidden, 404 Not Found,
                   405 Method Not Allowed, 408 Request Timeout, 409 Conflict,
                   411 Length Required, 413 Payload Too Large, 414 URI Too Long,
                   415 Unsupported Media Type
5xx Error Servidor: 500 Internal Server Error, 501 Not Implemented, 502 Bad Gateway,
                    503 Service Unavailable, 504 Gateway Timeout, 505 HTTP Version Not Supported
```

---

# 2. ARQUITECTURA GENERAL

## 2.1 Diagrama de Componentes

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              WEBSERV v1.0.0                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐                  │
│  │   Config     │    │   Server     │    │   Client     │                  │
│  │   Module     │───►│    Core      │◄───│   Handler    │                  │
│  │              │    │              │    │              │                  │
│  │ ┌──────────┐ │    │ ┌──────────┐ │    │ ┌──────────┐ │                  │
│  │ │ServerCfg │ │    │ │ poll()   │ │    │ │ Request  │ │                  │
│  │ │Location  │ │    │ │ loop     │ │    │ │ Parser   │ │                  │
│  │ └──────────┘ │    │ └──────────┘ │    │ └──────────┘ │                  │
│  └──────────────┘    └──────────────┘    └──────────────┘                  │
│                              │                                              │
│                              ▼                                              │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐                  │
│  │     HTTP     │    │     CGI      │    │   Session    │                  │
│  │   Handler    │    │   Handler    │    │   Manager    │                  │
│  │              │    │              │    │              │                  │
│  │ ┌──────────┐ │    │ ┌──────────┐ │    │ ┌──────────┐ │                  │
│  │ │ Response │ │    │ │ fork()   │ │    │ │ Cookies  │ │                  │
│  │ │ Builder  │ │    │ │ execve() │ │    │ │ Storage  │ │                  │
│  │ └──────────┘ │    │ └──────────┘ │    │ └──────────┘ │                  │
│  └──────────────┘    └──────────────┘    └──────────────┘                  │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                        Utilidades Comunes                            │   │
│  │  Utils.cpp  │  MimeTypes.cpp  │  Logging  │  String/File helpers    │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 2.2 Estructura de Archivos del Proyecto

```
webserv/
├── config/
│   └── webserv.conf              # Archivo de configuración principal
├── inc/                          # Headers (.hpp)
│   ├── webserv.hpp               # Header principal con includes y constantes
│   ├── config/
│   │   ├── Config.hpp            # Parser de configuración
│   │   ├── ServerConfig.hpp      # Configuración de servidor virtual
│   │   └── LocationConfig.hpp    # Configuración de ubicaciones
│   ├── http/
│   │   ├── Request.hpp           # Parser de peticiones HTTP
│   │   ├── Response.hpp          # Constructor de respuestas HTTP
│   │   └── MimeTypes.hpp         # Mapeo de tipos MIME
│   ├── server/
│   │   ├── Server.hpp            # Núcleo del servidor
│   │   └── Client.hpp            # Estado de conexión de cliente
│   ├── cgi/
│   │   └── CGIHandler.hpp        # Manejador de CGI
│   ├── session/
│   │   └── SessionManager.hpp    # Gestión de sesiones
│   └── utils/
│       └── Utils.hpp             # Utilidades generales
├── src/                          # Implementaciones (.cpp)
│   ├── main.cpp                  # Punto de entrada
│   ├── config/
│   ├── http/
│   ├── server/
│   ├── cgi/
│   ├── session/
│   └── utils/
└── www/                          # Contenido web estático
    ├── index.html
    ├── errors/
    ├── uploads/
    └── ...
```

## 2.3 Diagrama de Dependencias entre Módulos

```
                    ┌─────────────┐
                    │   main.cpp   │
                    └──────┬──────┘
                           │
              ┌────────────┴────────────┐
              ▼                         ▼
       ┌──────────┐              ┌──────────┐
       │  Config  │              │  Server  │
       └────┬─────┘              └────┬─────┘
            │                         │
    ┌───────┼───────┐         ┌───────┼───────┬───────────┐
    ▼       ▼       ▼         ▼       ▼       ▼           ▼
┌───────┐┌───────┐┌───────┐┌───────┐┌───────┐┌────────┐┌─────────┐
│Server ││Location││ Utils ││Client ││Request││Response││CGIHandler│
│Config ││Config  ││       ││       ││       ││        ││          │
└───────┘└───────┘└───────┘└───────┘└───────┘└────────┘└─────────┘
                                                              │
                                              ┌───────────────┘
                                              ▼
                                       ┌─────────────┐
                                       │SessionManager│
                                       └─────────────┘
```

## 2.4 Modelo de Ejecución

El servidor utiliza un **modelo de I/O no bloqueante basado en eventos** con `poll()`:

```
┌─────────────────────────────────────────────────────────────────┐
│                    BUCLE PRINCIPAL (Server::run)                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  while (running) {                                              │
│      1. Reconstruir array de pollfd (_rebuildPollFds)           │
│         - Agregar sockets de escucha (POLLIN)                   │
│         - Agregar sockets de clientes (POLLIN/POLLOUT)          │
│         - Agregar pipes CGI (POLLIN)                            │
│                                                                 │
│      2. poll(fds, nfds, 1000ms timeout)                         │
│                                                                 │
│      3. Procesar eventos:                                       │
│         - POLLIN en socket escucha → _acceptNewConnection()     │
│         - POLLIN en socket cliente → _handleClientRead()        │
│         - POLLOUT en socket cliente → _handleClientWrite()      │
│         - POLLIN en pipe CGI → _handleCgiRead()                 │
│         - POLLERR/POLLHUP/POLLNVAL → _closeClient()             │
│                                                                 │
│      4. Verificar timeouts (_checkTimeouts)                     │
│                                                                 │
│      5. Limpieza periódica de sesiones (cada 60s)               │
│  }                                                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

# 3. MÓDULOS DEL SISTEMA

## 3.1 Módulo de Configuración (Config)

### 3.1.1 Responsabilidades
- Parsear archivos de configuración estilo NGINX
- Validar configuración
- Proporcionar acceso a configuración de servidores virtuales

### 3.1.2 Clases

#### Config
```cpp
class Config {
    std::vector<ServerConfig> _servers;
    
    void parse(const std::string& filename);
    void parseFromString(const std::string& content);
    const std::vector<ServerConfig>& getServers() const;
    const ServerConfig* findServer(const std::string& host, int port) const;
    void validate() const;
};
```

#### ServerConfig
```cpp
class ServerConfig {
    std::string _host;                    // IP de escucha (ej: "0.0.0.0")
    int _port;                            // Puerto (ej: 8080)
    std::vector<std::string> _serverNames;// Nombres de servidor (virtual hosts)
    std::string _root;                    // Directorio raíz
    std::string _index;                   // Archivo índice por defecto
    size_t _maxBodySize;                  // Tamaño máximo de body
    std::map<int, std::string> _errorPages;// Páginas de error personalizadas
    std::vector<LocationConfig> _locations;// Configuraciones de ubicación
    bool _autoindex;                      // Listado automático de directorios
    
    const LocationConfig* findLocation(const std::string& uri) const;
    bool matchServerName(const std::string& host) const;
};
```

#### LocationConfig
```cpp
class LocationConfig {
    std::string _path;                    // Ruta (ej: "/api", "/upload")
    std::string _root;                    // Root override para esta ubicación
    std::string _alias;                   // Alias de ruta
    std::string _index;                   // Índice override
    std::string _uploadPath;              // Ruta para uploads
    std::string _redirect;                // URL de redirección
    int _redirectCode;                    // Código de redirección (301, 302, etc.)
    bool _autoindex;                      // Autoindex para esta ubicación
    bool _uploadEnabled;                  // Habilitar uploads
    std::set<std::string> _allowedMethods;// Métodos permitidos
    std::map<std::string, std::string> _cgiHandlers; // ext → handler
    size_t _maxBodySize;                  // Tamaño máximo de body (override)
    
    bool isMethodAllowed(const std::string& method) const;
    bool isCgiExtension(const std::string& extension) const;
    std::string getCgiHandler(const std::string& extension) const;
};
```

### 3.1.3 Sintaxis de Configuración

```nginx
# Comentarios con #

server {
    listen 0.0.0.0:8080;              # IP:Puerto de escucha
    server_name localhost example.com; # Nombres de servidor (virtual hosts)
    root ./www;                        # Directorio raíz
    index index.html index.htm;        # Archivos índice
    client_max_body_size 1048576;      # 1MB máximo (soporta K, M, G)
    autoindex off;                     # Desactivar listado de directorios
    
    error_page 404 /errors/404.html;   # Página de error personalizada
    error_page 500 502 503 504 /errors/50x.html;
    
    location / {
        methods GET;                   # Solo GET permitido
        autoindex off;
    }
    
    location /upload {
        methods GET POST DELETE;
        upload_store ./www/uploads;    # Directorio de uploads
    }
    
    location /cgi-bin {
        methods GET POST;
        alias ./cgi-bin;               # Alias de directorio
        cgi .py /usr/bin/python3;      # Handler CGI por extensión
        cgi .php /usr/bin/php-cgi;
        cgi .pl /usr/bin/perl;
    }
    
    location /old-page {
        redirect 301 /;                # Redirección permanente
    }
}
```

## 3.2 Módulo del Servidor (Server)

### 3.2.1 Responsabilidades
- Gestión del bucle principal de eventos
- Aceptación de conexiones
- Multiplexación I/O con poll()
- Enrutamiento de peticiones
- Gestión de CGI concurrente

### 3.2.2 Estructuras de Datos Principales

```cpp
class Server {
    Config _config;                       // Configuración
    bool _running;                        // Flag de ejecución
    std::vector<struct pollfd> _pollFds;  // Array para poll()
    std::map<int, Client> _clients;       // fd → Client
    std::map<int, int> _listenSockets;    // fd → puerto
    std::map<int, int> _cgiToClient;      // fd CGI → fd cliente
    size_t _activeCgiCount;               // CGIs activos
    std::vector<int> _cgiQueue;           // Cola de CGIs pendientes
};
```

### 3.2.3 Estados del Cliente

```cpp
enum ClientState {
    CLIENT_READING,     // Leyendo petición del socket
    CLIENT_PROCESSING,  // Procesando petición (o esperando en cola CGI)
    CLIENT_WRITING,     // Escribiendo respuesta
    CLIENT_CGI_RUNNING, // CGI en ejecución
    CLIENT_DONE,        // Petición completada
    CLIENT_ERROR        // Error
};
```

## 3.3 Módulo HTTP (Request/Response)

### 3.3.1 Estados de Parsing

```cpp
enum ParseState {
    PARSE_REQUEST_LINE,  // Parseando línea de petición
    PARSE_HEADERS,       // Parseando headers
    PARSE_BODY,          // Parseando body (Content-Length)
    PARSE_CHUNKED,       // Parseando body chunked
    PARSE_COMPLETE,      // Parsing completado
    PARSE_ERROR          // Error de parsing
};
```

### 3.3.2 Estructura de Petición Parseada

```cpp
class Request {
    // Request Line
    std::string _method;          // GET, POST, PUT, DELETE, HEAD
    std::string _uri;             // URI completa
    std::string _path;            // Path decodificado
    std::string _query;           // Query string (sin ?)
    std::string _fragment;        // Fragment (sin #)
    std::string _version;         // HTTP/1.0 o HTTP/1.1
    
    // Headers
    std::map<std::string, std::string> _headers;
    
    // Body
    std::string _body;
    size_t _contentLength;
    bool _isChunked;
    
    // Datos parseados
    std::string _host;            // Del header Host
    int _port;                    // Del header Host
    std::map<std::string, std::string> _queryParams;  // Parámetros GET
    std::map<std::string, std::string> _cookies;      // Cookies parseadas
    std::vector<UploadedFile> _uploadedFiles;         // Archivos multipart
};

struct UploadedFile {
    std::string name;        // Nombre del campo
    std::string filename;    // Nombre del archivo
    std::string contentType; // Content-Type del archivo
    std::string data;        // Contenido binario
};
```

## 3.4 Módulo CGI (CGIHandler)

### 3.4.1 Variables de Entorno CGI (RFC 3875)

El servidor implementa las siguientes variables de entorno para CGI:

| Variable | Descripción | Ejemplo |
|----------|-------------|---------|
| `GATEWAY_INTERFACE` | Versión del protocolo CGI | `CGI/1.1` |
| `SERVER_PROTOCOL` | Protocolo HTTP | `HTTP/1.1` |
| `SERVER_SOFTWARE` | Identificación del servidor | `webserv/1.0` |
| `REQUEST_METHOD` | Método HTTP | `GET`, `POST` |
| `SCRIPT_FILENAME` | Ruta absoluta del script | `/var/www/cgi/script.py` |
| `SCRIPT_NAME` | Ruta URI del script | `/cgi-bin/script.py` |
| `PATH_INFO` | Información adicional de ruta | `/extra/path` |
| `PATH_TRANSLATED` | PATH_INFO traducido a sistema | `/var/www/extra/path` |
| `REQUEST_URI` | URI completa de la petición | `/cgi-bin/test.py?foo=bar` |
| `QUERY_STRING` | Query string | `foo=bar&baz=qux` |
| `CONTENT_TYPE` | Tipo de contenido del body | `application/json` |
| `CONTENT_LENGTH` | Longitud del body | `1234` |
| `SERVER_NAME` | Nombre del servidor | `localhost` |
| `SERVER_PORT` | Puerto del servidor | `8080` |
| `REMOTE_ADDR` | IP del cliente | `192.168.1.100` |
| `REMOTE_HOST` | Hostname del cliente | `192.168.1.100` |
| `HTTP_*` | Headers HTTP convertidos | `HTTP_USER_AGENT`, `HTTP_ACCEPT` |
| `REDIRECT_STATUS` | Estado de redirección | `200` |

### 3.4.2 Proceso de Ejecución CGI

```
┌─────────────────────────────────────────────────────────────────┐
│                    EJECUCIÓN CGI                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Verificar límite de CGIs concurrentes                       │
│     - Si alcanzado: encolar en _cgiQueue                        │
│                                                                 │
│  2. Preparación:                                                │
│     - Crear pipe para stdout del CGI                            │
│     - Crear tmpfile para stdin del CGI                          │
│     - Escribir body de la petición al tmpfile                   │
│                                                                 │
│  3. fork()                                                      │
│     │                                                           │
│     ├─► Proceso hijo:                                           │
│     │   - dup2(tmpfile, STDIN)                                  │
│     │   - dup2(pipe_write, STDOUT)                              │
│     │   - chdir al directorio del script                        │
│     │   - Configurar variables de entorno                       │
│     │   - execve(cgi_handler, [script], env)                    │
│     │                                                           │
│     └─► Proceso padre:                                          │
│         - Cerrar extremos no usados de pipes                    │
│         - Registrar pipe en poll() para lectura                 │
│         - Actualizar estado cliente a CGI_RUNNING               │
│         - Incrementar _activeCgiCount                           │
│                                                                 │
│  4. Lectura asíncrona de salida CGI (en bucle poll):            │
│     - Acumular en client.getCgiOutput()                         │
│     - Verificar límite MAX_CGI_OUTPUT_SIZE                      │
│                                                                 │
│  5. Al terminar CGI (EOF en pipe o error):                      │
│     - waitpid() para recoger proceso hijo                       │
│     - Parsear salida CGI (headers + body)                       │
│     - Construir Response                                        │
│     - Decrementar _activeCgiCount                               │
│     - Procesar siguiente CGI de la cola                         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## 3.5 Módulo de Sesiones (SessionManager)

### 3.5.1 Características

- **Patrón Singleton** para acceso global
- **Nombre de cookie**: `WEBSERV_SESSION`
- **Timeout de sesión**: 3600 segundos (1 hora)
- **Longitud de ID**: 32 caracteres alfanuméricos

### 3.5.2 Estructura de Sesión

```cpp
struct Session {
    std::string id;                           // ID único de 32 caracteres
    std::map<std::string, std::string> data;  // Datos clave-valor
    time_t createdAt;                         // Timestamp de creación
    time_t lastAccessedAt;                    // Último acceso
};
```

---

# 4. FLUJO DE PROCESAMIENTO DE PETICIONES

## 4.1 Diagrama de Flujo Principal

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        FLUJO DE PROCESAMIENTO HTTP                          │
└─────────────────────────────────────────────────────────────────────────────┘

     ┌───────────────┐
     │ NUEVA CONEXIÓN│
     │  (accept)     │
     └───────┬───────┘
             │
             ▼
     ┌───────────────┐
     │ Crear Cliente │
     │ Estado: READING│
     └───────┬───────┘
             │
             ▼
     ┌───────────────┐       ┌─────────────────┐
     │ Leer datos    │──────►│ Parser Request  │
     │ recv()        │       │                 │
     └───────────────┘       └────────┬────────┘
                                      │
                           ┌──────────┴──────────┐
                           │ ¿Petición completa? │
                           └──────────┬──────────┘
                                      │
                    ┌─────────────────┼─────────────────┐
                    │ NO              │ SÍ              │ ERROR
                    ▼                 ▼                 ▼
            ┌───────────────┐ ┌───────────────┐ ┌───────────────┐
            │ Esperar más   │ │ Procesar      │ │ Responder     │
            │ datos (poll)  │ │ petición      │ │ Error 4xx     │
            └───────────────┘ └───────┬───────┘ └───────────────┘
                                      │
                                      ▼
                            ┌─────────────────┐
                            │ Seleccionar     │
                            │ servidor virtual│
                            │ (Host header)   │
                            └────────┬────────┘
                                     │
                                     ▼
                            ┌─────────────────┐
                            │ Buscar location │
                            │ más específica  │
                            └────────┬────────┘
                                     │
                    ┌────────────────┼────────────────┐
                    │                │                │
                    ▼                ▼                ▼
            ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
            │ ¿Redirect?  │  │ ¿Método     │  │ ¿Body size  │
            │             │  │  permitido? │  │  excedido?  │
            └──────┬──────┘  └──────┬──────┘  └──────┬──────┘
                   │                │                │
             SÍ    │          NO    │         SÍ     │
             ▼     │                ▼                ▼
        ┌─────────┐│         ┌───────────┐    ┌───────────┐
        │301/302  ││         │405 Method │    │413 Payload│
        │Redirect ││         │Not Allowed│    │Too Large  │
        └─────────┘│         └───────────┘    └───────────┘
                   │
                   ▼
        ┌─────────────────────────────────────┐
        │         ROUTING POR MÉTODO          │
        └─────────────────┬───────────────────┘
                          │
     ┌────────┬───────────┼───────────┬────────┐
     │        │           │           │        │
     ▼        ▼           ▼           ▼        ▼
┌────────┐┌────────┐┌────────┐┌────────┐┌────────┐
│  GET   ││ HEAD   ││  POST  ││  PUT   ││ DELETE │
│        ││        ││        ││        ││        │
└───┬────┘└───┬────┘└───┬────┘└───┬────┘└───┬────┘
    │         │         │         │         │
    └────┬────┴─────────┴────┬────┴─────────┘
         │                   │
         ▼                   ▼
  ┌─────────────┐    ┌─────────────┐
  │ ¿CGI req?   │    │ Escribir    │
  │             │    │ respuesta   │
  └──────┬──────┘    └─────────────┘
         │
   SÍ    │    NO
   ▼     └─────────────────┐
┌─────────────┐            ▼
│ Ejecutar    │    ┌─────────────┐
│ CGI         │    │ Servir      │
│             │    │ archivo/    │
└──────┬──────┘    │ directorio  │
       │           └──────┬──────┘
       │                  │
       └──────────┬───────┘
                  │
                  ▼
         ┌─────────────────┐
         │ Construir       │
         │ Response        │
         └────────┬────────┘
                  │
                  ▼
         ┌─────────────────┐
         │ Estado: WRITING │
         │ send() response │
         └────────┬────────┘
                  │
         ┌────────┴────────┐
         │ ¿Keep-Alive?    │
         └────────┬────────┘
                  │
         ┌────────┴────────┐
         │ SÍ             NO│
         ▼                 ▼
  ┌─────────────┐  ┌─────────────┐
  │ Reset client│  │ Cerrar      │
  │ esperar más │  │ conexión    │
  └─────────────┘  └─────────────┘
```

## 4.2 Procesamiento de GET/HEAD

```
┌─────────────────────────────────────────────────────────────────┐
│                    HANDLER GET/HEAD                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Resolver ruta del archivo:                                  │
│     - Combinar root + URI                                       │
│     - Aplicar alias si existe                                   │
│     - Normalizar path (eliminar .., etc.)                       │
│                                                                 │
│  2. ¿Es petición CGI?                                           │
│     - Verificar extensión del archivo                           │
│     - Si hay handler configurado → _handleCgi()                 │
│                                                                 │
│  3. Verificar archivo:                                          │
│     - stat() para obtener información                           │
│     - Si no existe → 404 Not Found                              │
│                                                                 │
│  4. Si es directorio:                                           │
│     a) Buscar archivo índice (index.html, etc.)                 │
│     b) Si no hay índice:                                        │
│        - Si autoindex ON y URI tiene / final → Directory Listing│
│        - Si autoindex ON pero sin / final → 301 Redirect        │
│        - Si autoindex OFF → 404 Not Found                       │
│                                                                 │
│  5. Si es archivo:                                              │
│     - Verificar permisos de lectura                             │
│     - Determinar MIME type                                      │
│     - Leer contenido                                            │
│     - Construir respuesta con headers apropiados                │
│                                                                 │
│  6. Para HEAD: build(excludeBody=true) → solo headers           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## 4.3 Procesamiento de POST

```
┌─────────────────────────────────────────────────────────────────┐
│                       HANDLER POST                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Resolver ruta del archivo                                   │
│                                                                 │
│  2. ¿Es petición CGI?                                           │
│     - Si hay handler para la extensión → _handleCgi()           │
│                                                                 │
│  3. ¿Está habilitado upload en esta location?                   │
│     - Si uploadEnabled && upload_store definido                 │
│       → _handleFileUpload()                                     │
│                                                                 │
│  4. ¿Existe el recurso?                                         │
│     a) Si es directorio: buscar índice CGI                      │
│     b) Si recurso existe pero no es CGI ni upload               │
│        → 405 Method Not Allowed                                 │
│     c) Si recurso no existe                                     │
│        → 204 No Content (según requisitos 42)                   │
│                                                                 │
│  _handleFileUpload():                                           │
│  ├─ Verificar/crear directorio de uploads                       │
│  ├─ Si Content-Type es multipart/form-data:                     │
│  │  └─ Parsear cada parte:                                      │
│  │     ├─ Extraer nombre de campo                               │
│  │     ├─ Extraer filename                                      │
│  │     ├─ Sanitizar filename (eliminar /, \, :, etc.)           │
│  │     └─ Guardar archivo                                       │
│  └─ Si no es multipart:                                         │
│     └─ Guardar body completo como archivo                       │
│                                                                 │
│  Respuesta: 201 Created                                         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## 4.4 Procesamiento de PUT

```
┌─────────────────────────────────────────────────────────────────┐
│                        HANDLER PUT                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Resolver ruta del archivo                                   │
│                                                                 │
│  2. Verificar que no sea un directorio                          │
│     - Si es directorio → 403 Forbidden                          │
│                                                                 │
│  3. Determinar si el archivo existe (para código de respuesta)  │
│     - access(path, F_OK)                                        │
│                                                                 │
│  4. Crear directorios padres si no existen                      │
│     - mkdir -p para ruta completa                               │
│                                                                 │
│  5. Escribir contenido del body al archivo                      │
│     - Modo: binary | trunc (sobrescribir)                       │
│     - Si falla apertura/escritura → 500 Internal Server Error   │
│                                                                 │
│  6. Responder:                                                  │
│     - Si archivo existía: 200 OK                                │
│     - Si archivo fue creado: 201 Created                        │
│                                                                 │
│  Header: Content-Length: 0                                      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## 4.5 Procesamiento de DELETE

```
┌─────────────────────────────────────────────────────────────────┐
│                       HANDLER DELETE                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Resolver ruta del archivo                                   │
│                                                                 │
│  2. Verificar existencia con stat()                             │
│     - Si no existe → 404 Not Found                              │
│                                                                 │
│  3. Verificar que es archivo regular (no directorio)            │
│     - Si es directorio → 403 Forbidden                          │
│                                                                 │
│  4. Eliminar archivo con std::remove()                          │
│     - Si falla → 500 Internal Server Error                      │
│                                                                 │
│  5. Responder: 204 No Content                                   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

# 5. PROTOCOLO HTTP/1.1 IMPLEMENTADO

## 5.1 Formato de Petición HTTP

Según **RFC 7230 §3**:

```
HTTP-message = start-line CRLF
               *( header-field CRLF )
               CRLF
               [ message-body ]

start-line (request) = method SP request-target SP HTTP-version CRLF
```

### Ejemplo de Petición:
```http
GET /index.html?foo=bar HTTP/1.1\r\n
Host: localhost:8080\r\n
User-Agent: curl/7.68.0\r\n
Accept: */*\r\n
Connection: keep-alive\r\n
\r\n
```

## 5.2 Formato de Respuesta HTTP

Según **RFC 7230 §3**:

```
HTTP-message = start-line CRLF
               *( header-field CRLF )
               CRLF
               [ message-body ]

start-line (response) = HTTP-version SP status-code SP reason-phrase CRLF
```

### Ejemplo de Respuesta:
```http
HTTP/1.1 200 OK\r\n
Server: Webserv/1.0\r\n
Date: Fri, 10 Jan 2026 12:00:00 GMT\r\n
Content-Type: text/html; charset=utf-8\r\n
Content-Length: 1234\r\n
Connection: keep-alive\r\n
\r\n
<!DOCTYPE html>...
```

## 5.3 Métodos HTTP Implementados

| Método | RFC | Descripción | Body Request | Body Response |
|--------|-----|-------------|--------------|---------------|
| GET | RFC 7231 §4.3.1 | Obtener recurso | No | Sí |
| HEAD | RFC 7231 §4.3.2 | Headers sin body | No | No |
| POST | RFC 7231 §4.3.3 | Enviar datos | Sí | Opcional |
| PUT | RFC 7231 §4.3.4 | Crear/Reemplazar | Sí | No |
| DELETE | RFC 7231 §4.3.5 | Eliminar recurso | No | No |

## 5.4 Transfer-Encoding: chunked

Según **RFC 7230 §4.1**, el servidor soporta recepción de peticiones con transferencia chunked:

```
chunked-body   = *chunk
                 last-chunk
                 trailer-part
                 CRLF

chunk          = chunk-size CRLF
                 chunk-data CRLF
chunk-size     = 1*HEXDIG
last-chunk     = 1*("0") CRLF
```

### Ejemplo:
```http
POST /upload HTTP/1.1\r\n
Host: localhost:8080\r\n
Transfer-Encoding: chunked\r\n
\r\n
7\r\n
Mozilla\r\n
9\r\n
Developer\r\n
7\r\n
Network\r\n
0\r\n
\r\n
```

## 5.5 Multipart/form-data (Uploads)

Según **RFC 7578**, el servidor parsea peticiones multipart para uploads de archivos:

```
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW

------WebKitFormBoundary7MA4YWxkTrZu0gW
Content-Disposition: form-data; name="file"; filename="test.txt"
Content-Type: text/plain

contenido del archivo
------WebKitFormBoundary7MA4YWxkTrZu0gW--
```

## 5.6 Conexiones Persistentes (Keep-Alive)

Según **RFC 7230 §6.3**:

- HTTP/1.1 mantiene conexiones persistentes por defecto
- Header `Connection: close` indica cierre después de la respuesta
- Header `Connection: keep-alive` es opcional (implícito en HTTP/1.1)
- El servidor respeta el header del cliente
- Tras errores graves (4xx/5xx), se cierra la conexión

## 5.7 Cookies y Sesiones

Según **RFC 6265**:

### Set-Cookie (Respuesta):
```http
Set-Cookie: WEBSERV_SESSION=abc123def456; Path=/; Max-Age=3600; HttpOnly
```

### Cookie (Petición):
```http
Cookie: WEBSERV_SESSION=abc123def456
```

---

# 6. REFERENCIAS RFC

## 6.1 RFCs Implementadas

| RFC | Título | Secciones Relevantes |
|-----|--------|---------------------|
| **RFC 7230** | HTTP/1.1 Message Syntax and Routing | §3 (Message Format), §4.1 (Chunked), §6.3 (Persistence) |
| **RFC 7231** | HTTP/1.1 Semantics and Content | §4 (Request Methods), §6 (Response Status Codes) |
| **RFC 7232** | HTTP/1.1 Conditional Requests | §3.3 (If-Modified-Since) - *No implementado* |
| **RFC 7233** | HTTP/1.1 Range Requests | *No implementado* |
| **RFC 7234** | HTTP/1.1 Caching | *No implementado* |
| **RFC 7235** | HTTP/1.1 Authentication | *No implementado* |
| **RFC 3875** | CGI/1.1 | Variables de entorno, protocolo CGI |
| **RFC 6265** | HTTP State Management (Cookies) | §4 (Set-Cookie), §5 (Cookie) |
| **RFC 7578** | multipart/form-data | Formato de uploads |

## 6.2 Cumplimiento y Desviaciones

### ✅ Implementado según RFC

1. **Formato de mensaje HTTP** (RFC 7230 §3)
   - Request-line: `method SP request-target SP HTTP-version CRLF`
   - Status-line: `HTTP-version SP status-code SP reason-phrase CRLF`
   - Headers con formato `field-name: field-value CRLF`
   - Separación headers/body con `CRLF CRLF`

2. **Métodos HTTP** (RFC 7231 §4)
   - GET, HEAD, POST, PUT, DELETE implementados correctamente
   - HEAD devuelve mismos headers que GET sin body

3. **Códigos de estado** (RFC 7231 §6)
   - Todos los códigos implementados con mensajes correctos

4. **Transfer-Encoding: chunked** (RFC 7230 §4.1)
   - Parsing correcto de chunks hexadecimales
   - Soporte para extensiones de chunk (ignoradas)
   - Detección correcta del último chunk (0\r\n)

5. **Host header obligatorio** (RFC 7230 §5.4)
   - HTTP/1.1 requiere header Host
   - Error 400 si falta

6. **Content-Length** (RFC 7230 §3.3.2)
   - Validación de valores duplicados conflictivos → Error 400
   - Lectura exacta de bytes especificados

7. **Conexiones persistentes** (RFC 7230 §6.3)
   - Por defecto en HTTP/1.1
   - Respetado header Connection: close

### ⚠️ Desviaciones de RFC (Justificadas)

1. **PATH_INFO en CGI** (RFC 3875 §4.1.5)
   - **RFC**: PATH_INFO es la porción de URI después del nombre del script
   - **Implementación**: Para compatibilidad con 42 tester, PATH_INFO = SCRIPT_NAME = ruta completa
   - **Razón**: El tester de 42 espera este comportamiento no estándar

2. **REQUEST_URI en CGI**
   - **RFC 3875**: No define REQUEST_URI
   - **Implementación**: Se incluye REQUEST_URI como variable extra
   - **Razón**: Compatibilidad con testers y scripts que lo esperan

3. **Límite de header size** (RFC 7230 §3.2.5)
   - **RFC**: "SHOULD" al menos 8000 bytes
   - **Implementación**: Exactamente 8192 bytes
   - **Razón**: Coincide con recomendación y límite práctico

### ❌ No Implementado (Fuera de Alcance)

1. **Range Requests** (RFC 7233)
   - No soportado: `Accept-Ranges: none` implícito
   - Peticiones con Range header se procesan sin el range

2. **Conditional Requests** (RFC 7232)
   - No soportado: If-Modified-Since, If-None-Match, etc.
   - Siempre se devuelve el recurso completo

3. **Caching** (RFC 7234)
   - No implementado: No headers Cache-Control, ETag, etc.

4. **Authentication** (RFC 7235)
   - No implementado: No Basic/Digest auth

5. **HTTPS/TLS**
   - Solo HTTP plano
   - No encriptación de transporte

---

# 7. SISTEMA DE CONFIGURACIÓN

## 7.1 Directivas de Servidor

| Directiva | Sintaxis | Descripción | Valor por defecto |
|-----------|----------|-------------|-------------------|
| `listen` | `host:port` o `port` | IP y puerto de escucha | `0.0.0.0:80` |
| `server_name` | `name1 [name2 ...]` | Nombres de servidor (virtual hosts) | - |
| `root` | `path` | Directorio raíz | `./www` |
| `index` | `file1 [file2 ...]` | Archivos índice | `index.html` |
| `client_max_body_size` | `size[K|M|G]` | Tamaño máximo de body | `1M` |
| `error_page` | `code [code2 ...] path` | Páginas de error personalizadas | Built-in |
| `autoindex` | `on|off` | Listado de directorios | `off` |

## 7.2 Directivas de Location

| Directiva | Sintaxis | Descripción |
|-----------|----------|-------------|
| `root` | `path` | Override del root para esta ubicación |
| `alias` | `path` | Sustituye la ruta de la ubicación |
| `index` | `file` | Override del archivo índice |
| `methods` | `METHOD1 [METHOD2 ...]` | Métodos HTTP permitidos |
| `autoindex` | `on|off` | Listado para esta ubicación |
| `cgi` | `.ext handler` | Handler CGI por extensión |
| `upload_store` | `path` | Directorio para uploads |
| `redirect` | `code url` | Redirección HTTP |
| `client_max_body_size` | `size` | Override del tamaño máximo |

## 7.3 Algoritmo de Matching de Location

```
findLocation(uri):
    best_match = NULL
    best_length = 0
    
    for each location in server.locations:
        if uri.startsWith(location.path):
            if location.path.length > best_length:
                best_match = location
                best_length = location.path.length
    
    return best_match
```

**Ejemplo**:
```
Locations configuradas:
  /           (length=1)
  /api        (length=4)
  /api/v1     (length=7)

URI: /api/v1/users
→ Matches: /, /api, /api/v1
→ Selected: /api/v1 (más específica)
```

## 7.4 Virtual Hosts

El servidor soporta múltiples servidores virtuales en el mismo puerto mediante el header `Host`:

```nginx
# Servidor 1 en puerto 8080
server {
    listen 0.0.0.0:8080;
    server_name www.example.com example.com;
    root ./www/example;
}

# Servidor 2 en mismo puerto 8080
server {
    listen 0.0.0.0:8080;
    server_name api.example.com;
    root ./www/api;
}
```

**Algoritmo de selección**:
1. Buscar servidor que coincida con puerto
2. Entre los servidores del mismo puerto, buscar coincidencia de `server_name`
3. Si no hay coincidencia, usar el primer servidor de ese puerto (default)

---

# 8. MANEJO DE CGI

## 8.1 Arquitectura de Ejecución CGI

```
┌──────────────────────────────────────────────────────────────────────────┐
│                        PROCESO DE CGI                                    │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  SERVIDOR (proceso padre)                    SCRIPT CGI (proceso hijo)   │
│  ─────────────────────                       ────────────────────────    │
│                                                                          │
│  ┌─────────────┐                             ┌─────────────┐             │
│  │ Body de     │──────── tmpfile ──────────►│ STDIN       │             │
│  │ petición    │                             │ (fd=0)      │             │
│  └─────────────┘                             └─────────────┘             │
│                                                                          │
│  ┌─────────────┐                             ┌─────────────┐             │
│  │ Output      │◄──────── pipe ─────────────│ STDOUT      │             │
│  │ buffer      │                             │ (fd=1)      │             │
│  └─────────────┘                             └─────────────┘             │
│                                                                          │
│  Variables de entorno:                                                   │
│  ┌────────────────────────────────────────────────────────────────┐     │
│  │ GATEWAY_INTERFACE=CGI/1.1                                      │     │
│  │ SERVER_PROTOCOL=HTTP/1.1                                       │     │
│  │ REQUEST_METHOD=POST                                            │     │
│  │ SCRIPT_FILENAME=/var/www/cgi-bin/script.py                     │     │
│  │ CONTENT_TYPE=application/json                                  │     │
│  │ CONTENT_LENGTH=1234                                            │     │
│  │ HTTP_HOST=localhost:8080                                       │     │
│  │ HTTP_USER_AGENT=curl/7.68.0                                    │     │
│  │ ...                                                            │     │
│  └────────────────────────────────────────────────────────────────┘     │
│                                                                          │
└──────────────────────────────────────────────────────────────────────────┘
```

## 8.2 Formato de Salida CGI

El script CGI debe producir output con headers seguidos del body:

```
Content-Type: text/html\r\n
Status: 200 OK\r\n
Set-Cookie: session=abc123\r\n
\r\n
<!DOCTYPE html>
<html>...
```

**Headers CGI reconocidos**:
- `Content-Type`: Obligatorio para respuestas con body
- `Status`: Código y mensaje de estado (ej: "200 OK", "404 Not Found")
- `Location`: Para redirecciones
- Cualquier otro header se pasa al cliente

## 8.3 Control de Concurrencia CGI

```
┌────────────────────────────────────────────────────────────────┐
│                    COLA DE CGI                                 │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  MAX_CONCURRENT_CGI = 5                                        │
│                                                                │
│  Petición CGI llega:                                           │
│  ├─ Si _activeCgiCount < MAX_CONCURRENT_CGI:                   │
│  │   └─ Ejecutar inmediatamente                                │
│  └─ Si _activeCgiCount >= MAX_CONCURRENT_CGI:                  │
│      └─ Encolar en _cgiQueue                                   │
│         └─ Cliente pasa a estado CLIENT_PROCESSING             │
│                                                                │
│  CGI termina:                                                  │
│  ├─ Decrementar _activeCgiCount                                │
│  └─ _processNextCgiFromQueue():                                │
│      └─ Si hay clientes en cola y capacidad disponible:        │
│          └─ Procesar siguiente CGI                             │
│                                                                │
│  PROTECCIÓN DE MEMORIA:                                        │
│  - No iniciar nuevo CGI si hay ≥3 clientes con buffers >10MB   │
│  - Límite de salida CGI: MAX_CGI_OUTPUT_SIZE (~200MB)          │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

## 8.4 Timeouts CGI

| Timeout | Valor | Descripción |
|---------|-------|-------------|
| `CGI_TIMEOUT` | 120 seg | Tiempo máximo de ejecución del proceso CGI |
| `CGI_RESPONSE_TIMEOUT` | 180 seg | Tiempo máximo para enviar respuesta CGI grande |

---

# 9. GESTIÓN DE SESIONES

## 9.1 Arquitectura del SessionManager

```cpp
// Patrón Singleton
class SessionManager {
    static SessionManager& getInstance();
    
    std::map<std::string, Session> _sessions;
    int _sessionTimeout;  // Default: 3600 segundos
    
    std::string createSession();
    Session* getSession(const std::string& id);
    void destroySession(const std::string& id);
    void cleanExpiredSessions();
};
```

## 9.2 Ciclo de Vida de Sesión

```
┌─────────────────────────────────────────────────────────────────┐
│                    CICLO DE VIDA DE SESIÓN                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. CREACIÓN                                                    │
│     ├─ Cliente hace petición sin cookie de sesión               │
│     ├─ Servidor genera ID único (32 caracteres alfanuméricos)   │
│     ├─ Crea estructura Session con timestamps                   │
│     └─ Envía Set-Cookie: WEBSERV_SESSION=<id>                   │
│                                                                 │
│  2. USO                                                         │
│     ├─ Cliente envía Cookie: WEBSERV_SESSION=<id>               │
│     ├─ Servidor busca sesión en _sessions map                   │
│     ├─ Actualiza lastAccessedAt                                 │
│     └─ Accede/modifica datos de sesión                          │
│                                                                 │
│  3. EXPIRACIÓN                                                  │
│     ├─ Cada 60 segundos: cleanExpiredSessions()                 │
│     ├─ Para cada sesión:                                        │
│     │   └─ Si (now - lastAccessedAt) > SESSION_TIMEOUT:         │
│     │       └─ Eliminar sesión                                  │
│     └─ SESSION_TIMEOUT = 3600 segundos (1 hora)                 │
│                                                                 │
│  4. DESTRUCCIÓN EXPLÍCITA                                       │
│     └─ Aplicación puede llamar destroySession(id)               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## 9.3 Generación de ID de Sesión

```cpp
std::string generateSessionId() {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    std::string result;
    result.reserve(32);
    
    std::srand(static_cast<unsigned int>(std::time(NULL)) ^ getpid());
    for (int i = 0; i < 32; ++i) {
        result += alphanum[std::rand() % (sizeof(alphanum) - 1)];
    }
    return result;
}
```

**Nota de Seguridad**: Esta implementación usa `rand()` que no es criptográficamente seguro. Para producción, debería usarse `/dev/urandom` o similar.

---

# 10. ANÁLISIS TÉCNICO DETALLADO

## 10.1 Parsing de Request Line

```cpp
// RFC 7230 §3.1.1: request-line = method SP request-target SP HTTP-version CRLF
bool Request::_parseRequestLine(const std::string& line) {
    std::vector<std::string> parts = Utils::split(line, ' ');
    if (parts.size() != 3) {
        _errorCode = 400;  // Bad Request
        return false;
    }
    
    _method = parts[0];    // GET, POST, etc.
    _uri = parts[1];       // /path/to/resource?query=value
    _version = parts[2];   // HTTP/1.0 o HTTP/1.1
    
    // Validar método
    if (!Utils::isValidMethod(_method)) {
        _errorCode = 501;  // Not Implemented
        return false;
    }
    
    // Validar versión
    if (_version != "HTTP/1.0" && _version != "HTTP/1.1") {
        _errorCode = 505;  // HTTP Version Not Supported
        return false;
    }
    
    // Validar longitud de URI (RFC 7230 §3.1.1)
    if (_uri.length() > 8192) {
        _errorCode = 414;  // URI Too Long
        return false;
    }
    
    _parseUri();  // Decodificar y normalizar
    return true;
}
```

## 10.2 Parsing de Headers

```cpp
// RFC 7230 §3.2: header-field = field-name ":" OWS field-value OWS
bool Request::_parseHeader(const std::string& line) {
    size_t pos = line.find(':');
    if (pos == std::string::npos) {
        _errorCode = 400;
        return false;
    }

    std::string name = Utils::trim(line.substr(0, pos));
    std::string value = Utils::trim(line.substr(pos + 1));

    if (name.empty()) {
        _errorCode = 400;
        return false;
    }

    std::string normalizedName = _normalizeHeaderName(name);
    
    // RFC 7230 §3.3.2: Múltiples Content-Length con valores diferentes = error
    if (normalizedName == "Content-Length") {
        if (_headers.find("Content-Length") != _headers.end()) {
            if (_headers["Content-Length"] != value) {
                _errorCode = 400;
                return false;
            }
            return true;  // Mismo valor, ignorar duplicado
        }
    }
    
    _headers[normalizedName] = value;
    return true;
}
```

## 10.3 Normalización de URI

```cpp
void Request::_parseUri() {
    std::string uri = _uri;

    // Seguridad: Rechazar null bytes (ataques de inyección)
    if (uri.find('\0') != std::string::npos || uri.find("%00") != std::string::npos) {
        _errorCode = 400;
        return;
    }

    // Extraer fragment (#)
    size_t fragPos = uri.find('#');
    if (fragPos != std::string::npos) {
        _fragment = uri.substr(fragPos + 1);
        uri = uri.substr(0, fragPos);
    }
    
    // Extraer query string (?)
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos) {
        _query = uri.substr(queryPos + 1);
        uri = uri.substr(0, queryPos);
        _parseQueryString();
    }
    
    // URL decode y normalizar path
    _path = Utils::urlDecode(uri);
    
    // Preservar trailing slash para semántica de directorio
    bool trailingSlash = (_path.length() > 0 && _path[_path.length() - 1] == '/');
    
    _path = Utils::normalizePath(_path);
    
    // Restaurar trailing slash
    if (trailingSlash && _path.length() > 1 && _path[_path.length() - 1] != '/')
        _path += "/";
}
```

## 10.4 Normalización de Path (Seguridad)

```cpp
// Elimina componentes ".." y "." para prevenir path traversal
std::string Utils::normalizePath(const std::string& path) {
    if (path.empty())
        return path;
    
    bool isRelative = (path[0] != '/');
    std::vector<std::string> parts;
    std::vector<std::string> segments = split(path, '/');
    
    for (size_t i = 0; i < segments.size(); ++i) {
        if (segments[i] == "." || segments[i].empty())
            continue;  // Ignorar . y segmentos vacíos
        if (segments[i] == "..") {
            if (!parts.empty() && parts.back() != "..")
                parts.pop_back();  // Subir un nivel
            else if (isRelative)
                parts.push_back("..");  // Mantener .. en rutas relativas
        } else {
            parts.push_back(segments[i]);
        }
    }
    
    std::string result;
    if (!isRelative)
        result = "/";
    
    for (size_t i = 0; i < parts.size(); ++i) {
        result += parts[i];
        if (i < parts.size() - 1)
            result += "/";
    }
    
    if (result.empty())
        result = isRelative ? "." : "/";
    
    return result;
}
```

**Ejemplo de normalización**:
```
/foo/bar/../baz     → /foo/baz
/foo/./bar          → /foo/bar
/foo/bar/../../baz  → /baz
/../../../etc/passwd → /etc/passwd  (no puede escapar de /)
```

## 10.5 Construcción de Respuesta HTTP

```cpp
std::string Response::build(bool excludeBody) const {
    std::ostringstream response;
    
    // Status line (RFC 7230 §3.1.2)
    response << "HTTP/1.1 " << _statusCode << " " 
             << Utils::getStatusMessage(_statusCode) << "\r\n";
    
    // Headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    
    // End of headers
    response << "\r\n";
    
    // Body (opcional para HEAD)
    if (!excludeBody) {
        response << _body;
    }
    
    return response.str();
}

void Response::setBody(const std::string& body) {
    _body = body;
    // Automáticamente actualizar Content-Length
    setHeader("Content-Length", Utils::sizeTToString(_body.length()));
}
```

## 10.6 Manejo No Bloqueante de I/O

```cpp
void Server::_handleClientWrite(int clientFd) {
    Client& client = _clients[clientFd];
    
    if (client.getWriteBufferSize() == 0) {
        // Buffer vacío - verificar keep-alive
        if (!client.shouldKeepAlive()) {
            _closeClient(clientFd);
        } else {
            client.reset();  // Preparar para siguiente petición
        }
        return;
    }

    std::string& buffer = client.getWriteBuffer();
    size_t bufferSize = buffer.size();

    // OPTIMIZACIÓN: Para respuestas grandes, intentar múltiples envíos
    int attempts = (bufferSize > 1048576) ? 10 : 1;

    for (int i = 0; i < attempts && bufferSize > 0; ++i) {
        ssize_t sent = send(clientFd, buffer.c_str(), bufferSize, 0);

        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;  // Buffer de socket lleno, reintentar después
            _closeClient(clientFd);
            return;
        }

        if (sent == 0)
            break;

        client.eraseFromWriteBuffer(sent);
        bufferSize = client.getWriteBufferSize();
    }

    client.updateLastActivity();

    if (client.getWriteBufferSize() == 0) {
        // Procesar siguiente CGI de la cola si hay capacidad
        if (!_cgiQueue.empty())
            _processNextCgiFromQueue();

        if (!client.shouldKeepAlive())
            _closeClient(clientFd);
        else
            client.reset();
    }
}
```

---

# APÉNDICE A: TIPOS MIME SOPORTADOS

| Extensión | Tipo MIME |
|-----------|-----------|
| .html, .htm | text/html |
| .css | text/css |
| .js | application/javascript |
| .json | application/json |
| .xml | application/xml |
| .txt | text/plain |
| .png | image/png |
| .jpg, .jpeg | image/jpeg |
| .gif | image/gif |
| .svg | image/svg+xml |
| .ico | image/x-icon |
| .pdf | application/pdf |
| .zip | application/zip |
| .tar | application/x-tar |
| .gz | application/gzip |
| .mp3 | audio/mpeg |
| .mp4 | video/mp4 |
| .webm | video/webm |
| .woff | font/woff |
| .woff2 | font/woff2 |
| (default) | application/octet-stream |

---

# APÉNDICE B: CÓDIGOS DE ERROR Y MENSAJES

| Código | Mensaje | Descripción |
|--------|---------|-------------|
| 100 | Continue | Continuar con el cuerpo de la petición |
| 200 | OK | Petición exitosa |
| 201 | Created | Recurso creado exitosamente |
| 204 | No Content | Éxito sin contenido de respuesta |
| 301 | Moved Permanently | Redirección permanente |
| 302 | Found | Redirección temporal |
| 304 | Not Modified | Recurso no modificado (caché) |
| 307 | Temporary Redirect | Redirección temporal (preserva método) |
| 400 | Bad Request | Error de sintaxis en la petición |
| 403 | Forbidden | Acceso denegado |
| 404 | Not Found | Recurso no encontrado |
| 405 | Method Not Allowed | Método HTTP no permitido |
| 408 | Request Timeout | Timeout de petición |
| 411 | Length Required | Se requiere Content-Length |
| 413 | Payload Too Large | Body excede el límite |
| 414 | URI Too Long | URI excede 8192 caracteres |
| 415 | Unsupported Media Type | Content-Type no soportado |
| 500 | Internal Server Error | Error interno del servidor |
| 501 | Not Implemented | Método no implementado |
| 502 | Bad Gateway | Error de gateway/CGI |
| 503 | Service Unavailable | Servicio no disponible |
| 504 | Gateway Timeout | Timeout de CGI |
| 505 | HTTP Version Not Supported | Solo HTTP/1.0 y HTTP/1.1 |

---

**Fin de la Documentación Técnica - Fase 1**

*Esta documentación continuará con análisis más detallados de flujos específicos, diagramas de secuencia y casos de uso en las siguientes fases.*
