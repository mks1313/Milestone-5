# WEBSERV - Fase 2: Análisis Exhaustivo del Código Fuente

## Documentación Técnica Línea por Línea

**Proyecto:** 42 Barcelona - webserv  
**Autor:** fcela-ga (Felipe Cela García)  
**Fase:** 2 de 4  
**Enfoque:** Server.cpp, Request.cpp, Client.cpp

---

# ÍNDICE FASE 2

1. [Server.cpp - Análisis Completo](#1-servercpp---análisis-completo)
2. [Request.cpp - Parser HTTP Detallado](#2-requestcpp---parser-http-detallado)
3. [Client.cpp - Gestión de Estado de Conexión](#3-clientcpp---gestión-de-estado-de-conexión)
4. [Interacción entre Módulos](#4-interacción-entre-módulos)
5. [Patrones de Diseño Utilizados](#5-patrones-de-diseño-utilizados)
6. [Análisis de Complejidad](#6-análisis-de-complejidad)

---

# 1. SERVER.CPP - ANÁLISIS COMPLETO

## 1.1 Visión General

`Server.cpp` es el **núcleo del servidor HTTP**. Con 1822 líneas de código, implementa:

- Bucle de eventos basado en `poll()`
- Gestión de sockets de escucha
- Manejo de conexiones de clientes
- Enrutamiento de peticiones HTTP
- Ejecución de CGI
- Control de timeouts y recursos

## 1.2 Includes y Dependencias

```cpp
// Líneas 13-37: Headers del sistema y del proyecto
#include "server/Server.hpp"      // Definición de la clase Server
#include "webserv.hpp"            // Constantes globales y includes comunes
#include "utils/Utils.hpp"        // Utilidades (logging, strings, files)
#include "http/MimeTypes.hpp"     // Mapeo extensión → Content-Type
#include "session/SessionManager.hpp"  // Gestión de sesiones
#include "http/Response.hpp"      // Constructor de respuestas HTTP
#include "cgi/CGIHandler.hpp"     // Manejador de CGI

// Headers de sistema POSIX
#include <sys/socket.h>   // socket(), bind(), listen(), accept(), send(), recv()
#include <sys/types.h>    // Tipos de datos POSIX
#include <sys/stat.h>     // stat() para información de archivos
#include <sys/wait.h>     // waitpid() para procesos CGI
#include <netinet/in.h>   // struct sockaddr_in
#include <arpa/inet.h>    // inet_ntoa(), inet_addr()
#include <unistd.h>       // close(), fork(), dup2(), chdir()
#include <limits.h>       // PATH_MAX
#include <fcntl.h>        // fcntl() para non-blocking
#include <poll.h>         // poll() - multiplexación I/O
#include <csignal>        // signal(), sigaction()
#include <cerrno>         // errno
#include <cstring>        // strerror()
#include <cstdio>         // tmpfile(), fwrite()
#include <dirent.h>       // opendir(), readdir() para directory listing
#include <fstream>        // ifstream, ofstream
#include <algorithm>      // std::sort() para directory listing
```

**Justificación de dependencias:**

| Header | Uso Principal | RFC/Estándar |
|--------|---------------|--------------|
| `poll.h` | Multiplexación I/O | POSIX.1-2001 |
| `sys/socket.h` | API de sockets | BSD sockets |
| `netinet/in.h` | Estructuras de red | RFC 791 (IP) |
| `sys/wait.h` | Control de procesos CGI | POSIX.1 |
| `fcntl.h` | I/O no bloqueante | POSIX.1 |

## 1.3 Variable Global de Señales

```cpp
// Línea 40
static volatile sig_atomic_t g_serverRunning = 1;
```

**Análisis técnico:**

- `static`: Visibilidad limitada a este archivo (linkage interno)
- `volatile`: Previene optimizaciones del compilador que podrían cachear el valor
- `sig_atomic_t`: Tipo garantizado atómico en contexto de señales (POSIX)
- Valor inicial `1`: Servidor activo

**¿Por qué es necesario?**

Cuando llega una señal (SIGINT/SIGTERM), el handler de señales modifica esta variable. Sin `volatile`, el compilador podría optimizar el bucle `while(_running && g_serverRunning)` asumiendo que `g_serverRunning` nunca cambia, causando un bucle infinito.

## 1.4 Handler de Señales

```cpp
// Líneas 46-50
void Server::_signalHandler(int sig)
{
    (void)sig;           // Suprimir warning de parámetro no usado
    g_serverRunning = 0; // Señalar al bucle principal que debe terminar
}
```

**Restricciones de handlers de señales (POSIX):**

Un signal handler solo puede:
1. Modificar variables `volatile sig_atomic_t`
2. Llamar funciones async-signal-safe (lista limitada)
3. NO puede usar `malloc`, `printf`, ni la mayoría de funciones de biblioteca

Por eso el handler es tan minimalista.

## 1.5 Constructores y Destructor

```cpp
// Líneas 56-84
Server::Server() : _running(false), _activeCgiCount(0)
{
    // Constructor por defecto
    // _config queda vacío
    // _pollFds, _clients, _listenSockets quedan vacíos
}

Server::Server(const Config& config) : _config(config), _running(false), _activeCgiCount(0)
{
    // Constructor con configuración
    // Se copia la configuración completa
}

Server::Server(const Server& other)
{
    *this = other;  // Delega al operador de asignación
}

Server& Server::operator=(const Server& other)
{
    if (this != &other)
    {
        _config = other._config;
        _running = false;        // NO copiar estado de ejecución
        _activeCgiCount = 0;     // NO copiar contador de CGIs
        // NOTA: No se copian conexiones activas (_clients, _pollFds, _listenSockets)
        // Esto es intencional: un servidor copiado empieza "limpio"
    }
    return *this;
}

Server::~Server()
{
    _closeAllSockets();  // Liberar todos los recursos de red
}
```

**Principio de diseño:** El destructor garantiza que no quedan sockets abiertos (RAII - Resource Acquisition Is Initialization).

## 1.6 Inicialización del Servidor

### 1.6.1 Método init()

```cpp
// Líneas 90-107
bool Server::init()
{
    // 1. Configurar handlers de señales
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));  // Inicializar estructura a ceros
    sa.sa_handler = _signalHandler;    // Asignar nuestro handler
    sigaction(SIGINT, &sa, NULL);      // Ctrl+C
    sigaction(SIGTERM, &sa, NULL);     // kill
    
    // 2. Ignorar SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    
    // 3. Crear sockets de escucha
    return _createListenSockets();
}
```

**¿Por qué ignorar SIGPIPE?**

Cuando intentamos escribir a un socket cuyo peer ha cerrado la conexión, el sistema operativo envía SIGPIPE al proceso. Por defecto, SIGPIPE termina el proceso.

```
Cliente          Servidor
   |                |
   |<--- close() ---|  (Cliente cierra)
   |                |
   |    send() ---->|  ← Servidor intenta enviar
   |                |
   |   [SIGPIPE]    |  ← Sin SIG_IGN, el servidor muere
```

Al ignorar SIGPIPE, `send()` retorna -1 con `errno = EPIPE`, permitiendo manejar el error gracefully.

### 1.6.2 Creación de Sockets de Escucha

```cpp
// Líneas 109-154
bool Server::_createListenSockets()
{
    const std::vector<ServerConfig>& servers = _config.getServers();
    
    if (servers.empty())
    {
        Utils::logError("No servers configured");
        return false;
    }

    // Set para evitar duplicados: (host, port) ya creados
    std::set<std::pair<std::string, int> > createdSockets;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        const ServerConfig& serverConf = servers[i];
        std::string host = serverConf.getHost();
        int port = serverConf.getPort();

        std::pair<std::string, int> hostPort(host, port);
        
        // IMPORTANTE: Múltiples server{} pueden compartir el mismo puerto
        // (virtual hosts). Solo creamos UN socket por combinación host:port
        if (createdSockets.find(hostPort) != createdSockets.end())
            continue;

        int fd = _createSocket(host, port);
        if (fd < 0)
        {
            Utils::logError("Failed to create socket on " + host + ":" + 
                           Utils::intToString(port));
            _closeAllSockets();  // Cleanup en caso de error
            return false;
        }

        _listenSockets[fd] = port;  // Mapeo fd → puerto para virtual hosts
        createdSockets.insert(hostPort);

        Utils::logInfo("Listening on " + host + ":" + Utils::intToString(port));
    }

    if (_listenSockets.empty())
    {
        Utils::logError("No listening sockets created");
        return false;
    }

    return true;
}
```

**Diagrama de Virtual Hosts:**

```
Configuración:
┌─────────────────────────────┐
│ server { listen 8080;       │ ← server_name: www.site1.com
│          server_name ...    │
│ }                           │
├─────────────────────────────┤
│ server { listen 8080;       │ ← server_name: www.site2.com
│          server_name ...    │
│ }                           │
├─────────────────────────────┤
│ server { listen 8081;       │ ← server_name: api.site.com
│          server_name ...    │
│ }                           │
└─────────────────────────────┘

Sockets creados:
┌─────────────────┐
│ Socket fd=3     │ ← Puerto 8080 (compartido por site1 y site2)
│ 0.0.0.0:8080    │
├─────────────────┤
│ Socket fd=4     │ ← Puerto 8081
│ 0.0.0.0:8081    │
└─────────────────┘
```

### 1.6.3 Creación de Socket Individual

```cpp
// Líneas 156-208
int Server::_createSocket(const std::string& host, int port)
{
    // 1. Crear socket TCP/IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //                  │        │            └─ Protocolo (0 = auto-select TCP)
    //                  │        └─ Tipo de socket (stream = TCP)
    //                  └─ Familia de direcciones (IPv4)
    
    if (sockfd < 0)
    {
        Utils::logError("socket() failed: " + std::string(std::strerror(errno)));
        return -1;
    }

    // 2. Configurar SO_REUSEADDR
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        Utils::logError("setsockopt(SO_REUSEADDR) failed");
        close(sockfd);
        return -1;
    }
    
    /*
     * ¿Por qué SO_REUSEADDR?
     * 
     * Cuando un socket se cierra, entra en estado TIME_WAIT durante ~2 minutos
     * (para manejar paquetes retrasados). Sin SO_REUSEADDR, bind() falla si
     * intentamos reiniciar el servidor en ese período.
     * 
     * Timeline sin SO_REUSEADDR:
     * [Servidor inicia en :8080] → [Ctrl+C] → [TIME_WAIT 2min] → [bind() falla]
     * 
     * Timeline con SO_REUSEADDR:
     * [Servidor inicia en :8080] → [Ctrl+C] → [bind() OK inmediatamente]
     */

    // 3. Configurar non-blocking
    if (!_setNonBlocking(sockfd))
    {
        close(sockfd);
        return -1;
    }

    // 4. Preparar dirección de bind
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);  // Host to Network Short (endianness)
    
    if (host.empty() || host == "0.0.0.0" || host == "*")
        addr.sin_addr.s_addr = INADDR_ANY;  // Todas las interfaces
    else
        addr.sin_addr.s_addr = inet_addr(host.c_str());  // IP específica

    // 5. Bind: Asociar socket a dirección
    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        Utils::logError("bind() failed on port " + Utils::intToString(port) + 
                       ": " + std::strerror(errno));
        close(sockfd);
        return -1;
    }

    // 6. Listen: Marcar socket como pasivo (acepta conexiones)
    if (listen(sockfd, BACKLOG) < 0)  // BACKLOG = 128
    {
        Utils::logError("listen() failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}
```

**Secuencia de creación de socket:**

```
┌────────────┐     ┌────────────┐     ┌────────────┐     ┌────────────┐
│  socket()  │────►│ setsockopt │────►│   bind()   │────►│  listen()  │
│            │     │            │     │            │     │            │
│ Crear fd   │     │ SO_REUSE   │     │ Asignar    │     │ Activar    │
│            │     │ ADDR       │     │ IP:Puerto  │     │ backlog    │
└────────────┘     └────────────┘     └────────────┘     └────────────┘
```

### 1.6.4 Configuración Non-Blocking

```cpp
// Líneas 210-216
bool Server::_setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);  // Obtener flags actuales
    if (flags < 0)
        return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) >= 0;  // Añadir O_NONBLOCK
}
```

**¿Por qué non-blocking?**

| Modo | `recv()` sin datos | `send()` buffer lleno | `accept()` sin conexiones |
|------|--------------------|-----------------------|---------------------------|
| Blocking | Bloquea indefinidamente | Bloquea indefinidamente | Bloquea indefinidamente |
| Non-blocking | Retorna -1, errno=EAGAIN | Retorna -1, errno=EAGAIN | Retorna -1, errno=EAGAIN |

En modo non-blocking, `poll()` coordina cuándo hay datos disponibles, evitando bloqueos.

## 1.7 Gestión de poll()

### 1.7.1 Reconstrucción del Array de pollfd

```cpp
// Líneas 251-300
void Server::_rebuildPollFds()
{
    _pollFds.clear();

    // 1. Añadir sockets de escucha (siempre esperando conexiones)
    for (std::map<int, int>::iterator it = _listenSockets.begin();
         it != _listenSockets.end(); ++it)
    {
        struct pollfd pfd;
        pfd.fd = it->first;
        pfd.events = POLLIN;   // Solo lectura (accept)
        pfd.revents = 0;
        _pollFds.push_back(pfd);
    }

    // 2. Añadir sockets de clientes
    for (std::map<int, Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        struct pollfd pfd;
        pfd.fd = it->first;
        pfd.events = 0;  // Empezar sin eventos

        Client& client = it->second;
        ClientState state = client.getState();
        
        // Si está leyendo, queremos saber cuándo hay datos
        if (state == CLIENT_READING)
        {
            pfd.events |= POLLIN;
        }
        
        // Si tiene datos que enviar, queremos saber cuándo podemos escribir
        if (state == CLIENT_WRITING || client.getWriteBufferSize() > 0)
        {
            pfd.events |= POLLOUT;
        }

        pfd.revents = 0;
        _pollFds.push_back(pfd);
    }

    // 3. Añadir pipes de CGI (lectura de output)
    for (std::map<int, int>::iterator it = _cgiToClient.begin();
         it != _cgiToClient.end(); ++it)
    {
        struct pollfd pfd;
        pfd.fd = it->first;      // fd del pipe de lectura
        pfd.events = POLLIN;     // Esperar output del CGI
        pfd.revents = 0;
        _pollFds.push_back(pfd);
    }
}
```

**Estructura del array de poll:**

```
_pollFds[]:
┌───────────────────────────────────────────────────────────────────┐
│ [0] Listen :8080  │ [1] Listen :8081  │ [2] Client fd=5  │ ...    │
│ events: POLLIN    │ events: POLLIN    │ events: POLLIN   │        │
│                   │                   │ |POLLOUT         │        │
├───────────────────┴───────────────────┴──────────────────┴────────┤
│ [n] CGI pipe fd=8                                                 │
│ events: POLLIN                                                    │
└───────────────────────────────────────────────────────────────────┘
```

**Eventos de poll:**

| Evento | Significado | Uso |
|--------|-------------|-----|
| `POLLIN` | Datos disponibles para leer | Nuevas conexiones, datos HTTP, output CGI |
| `POLLOUT` | Socket listo para escribir | Enviar respuestas HTTP |
| `POLLERR` | Error en el descriptor | Cerrar conexión |
| `POLLHUP` | Peer cerró la conexión | Cerrar conexión |
| `POLLNVAL` | Descriptor inválido | Cerrar conexión |

## 1.8 Bucle Principal del Servidor

```cpp
// Líneas 339-440
void Server::run()
{
    _running = true;
    g_serverRunning = 1;
    
    time_t lastCleanup = std::time(NULL);  // Para limpieza periódica de sesiones

    Utils::logInfo("Server started, entering main loop");

    // ═══════════════════════════════════════════════════════════════════
    // BUCLE PRINCIPAL - El corazón del servidor
    // ═══════════════════════════════════════════════════════════════════
    while (_running && g_serverRunning)
    {
        // PASO 1: Reconstruir array de descriptores a monitorear
        _rebuildPollFds();

        if (_pollFds.empty())
        {
            Utils::logError("No file descriptors to poll");
            break;
        }

        // PASO 2: Esperar eventos (timeout 1 segundo)
        int pollResult = poll(&_pollFds[0], _pollFds.size(), 1000);
        /*
         * poll() retorna:
         *   > 0 : Número de descriptores con eventos
         *   = 0 : Timeout (sin eventos)
         *   < 0 : Error (verificar errno)
         */

        if (pollResult < 0)
        {
            if (errno == EINTR)
                continue;  // Interrumpido por señal, reintentar
            Utils::logError("poll() failed: " + std::string(std::strerror(errno)));
            break;
        }

        if (pollResult == 0)
        {
            // Timeout - buen momento para verificar timeouts de clientes
            _checkTimeouts();
            continue;
        }

        // PASO 3: Procesar eventos
        for (size_t i = 0; i < _pollFds.size() && pollResult > 0; ++i)
        {
            if (_pollFds[i].revents == 0)
                continue;  // Este descriptor no tiene eventos
            
            --pollResult;  // Optimización: dejar de buscar si ya procesamos todos
            int fd = _pollFds[i].fd;
            short revents = _pollFds[i].revents;

            // 3a. Verificar errores
            if (revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                if (_listenSockets.find(fd) != _listenSockets.end())
                {
                    Utils::logError("Error on listen socket");
                    continue;  // Error grave, pero seguimos con otros sockets
                }
                else if (_clients.find(fd) != _clients.end())
                {
                    _closeClient(fd);  // Cliente desconectado o error
                    continue;
                }
                else if (_cgiToClient.find(fd) != _cgiToClient.end())
                {
                    _handleCgiRead(fd);  // CGI terminó o error
                    continue;
                }
            }

            // 3b. Socket de escucha - nueva conexión
            if (_listenSockets.find(fd) != _listenSockets.end())
            {
                if (revents & POLLIN)
                    _acceptNewConnection(fd);
            }
            // 3c. Pipe CGI - output disponible
            else if (_cgiToClient.find(fd) != _cgiToClient.end())
            {
                if (revents & POLLIN)
                    _handleCgiRead(fd);
            }
            // 3d. Socket de cliente
            else if (_clients.find(fd) != _clients.end())
            {
                if (revents & POLLIN)
                    _handleClientRead(fd);
                if (revents & POLLOUT)
                    _handleClientWrite(fd);
            }
        }

        // PASO 4: Verificar timeouts de conexiones
        _checkTimeouts();

        // PASO 5: Limpieza periódica de sesiones (cada 60 segundos)
        time_t now = std::time(NULL);
        if (now - lastCleanup >= 60)
        {
            SessionManager::getInstance().cleanExpiredSessions();
            lastCleanup = now;
        }
    }

    // Cleanup al salir del bucle
    Utils::logInfo("Server shutting down");
    _closeAllSockets();
    _running = false;
}
```

**Diagrama de flujo del bucle:**

```
                    ┌─────────────────┐
                    │  Inicializar    │
                    │  _running = 1   │
                    └────────┬────────┘
                             │
              ┌──────────────▼──────────────┐
              │   while(_running &&         │◄─────────────────────┐
              │         g_serverRunning)    │                      │
              └──────────────┬──────────────┘                      │
                             │                                     │
              ┌──────────────▼──────────────┐                      │
              │   _rebuildPollFds()         │                      │
              │   (Actualizar descriptores) │                      │
              └──────────────┬──────────────┘                      │
                             │                                     │
              ┌──────────────▼──────────────┐                      │
              │   poll(fds, n, 1000ms)      │                      │
              │   (Esperar eventos)         │                      │
              └──────────────┬──────────────┘                      │
                             │                                     │
           ┌─────────────────┼─────────────────┐                   │
           │                 │                 │                   │
       ┌───▼───┐         ┌───▼───┐         ┌───▼───┐               │
       │ < 0   │         │ = 0   │         │ > 0   │               │
       │ Error │         │Timeout│         │Eventos│               │
       └───┬───┘         └───┬───┘         └───┬───┘               │
           │                 │                 │                   │
           │                 │        ┌────────▼────────┐          │
    ┌──────▼──────┐          │        │ for each fd     │          │
    │ EINTR?      │          │        │ with events     │          │
    │ → continue  │          │        └────────┬────────┘          │
    │ else break  │          │                 │                   │
    └─────────────┘          │     ┌───────────┼───────────┐       │
                             │     │           │           │       │
                             │ ┌───▼───┐   ┌───▼───┐   ┌───▼───┐   │
                             │ │Listen │   │Client │   │ CGI   │   │
                             │ │Socket │   │Socket │   │ Pipe  │   │
                             │ └───┬───┘   └───┬───┘   └───┬───┘   │
                             │     │           │           │       │
                             │ ┌───▼───┐   ┌───▼───┐   ┌───▼───┐   │
                             │ │accept │   │read/  │   │ read  │   │
                             │ │       │   │write  │   │output │   │
                             │ └───────┘   └───────┘   └───────┘   │
                             │                 │                   │
              ┌──────────────▼─────────────────▼───────────────┐   │
              │   _checkTimeouts()                             │   │
              │   SessionManager::cleanExpiredSessions()       │   │
              └────────────────────────┬───────────────────────┘   │
                                       │                           │
                                       └───────────────────────────┘
```

## 1.9 Aceptación de Nuevas Conexiones

```cpp
// Líneas 467-505
void Server::_acceptNewConnection(int listenFd)
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    // accept() extrae la primera conexión de la cola de pendientes
    int clientFd = accept(listenFd, 
                          reinterpret_cast<struct sockaddr*>(&clientAddr), 
                          &clientLen);
    
    if (clientFd < 0)
    {
        // EAGAIN/EWOULDBLOCK: No hay conexiones pendientes (normal en non-blocking)
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            Utils::logError("accept() failed: " + std::string(std::strerror(errno)));
        return;
    }

    // Verificar límite de clientes
    if (_clients.size() >= MAX_CLIENTS)  // MAX_CLIENTS = 1024
    {
        Utils::logWarning("Maximum clients reached, rejecting connection");
        close(clientFd);
        return;
    }

    // Configurar socket del cliente como non-blocking
    if (!_setNonBlocking(clientFd))
    {
        close(clientFd);
        return;
    }

    // Crear objeto Client
    std::string clientIp = inet_ntoa(clientAddr.sin_addr);  // IP en formato string
    int clientPort = _listenSockets[listenFd];  // Puerto del servidor (no del cliente)
    Client client(clientFd, clientIp, clientPort);
    
    // Registrar en el mapa de clientes
    _clients[clientFd] = client;

    Utils::logDebug("New connection from " + clientIp + 
                    " on fd " + Utils::intToString(clientFd));
}
```

**Flujo de conexión TCP:**

```
Cliente                                  Servidor
   │                                        │
   │─────── SYN ──────────────────────────►│ (entra en cola de backlog)
   │                                        │
   │◄────── SYN-ACK ───────────────────────│
   │                                        │
   │─────── ACK ──────────────────────────►│ (conexión establecida en cola)
   │                                        │
   │                          accept() ─────┤ (extrae de la cola)
   │                                        │
   │◄─────────────────────────── clientFd ──│ (nuevo fd para esta conexión)
```

## 1.10 Lectura de Peticiones HTTP

```cpp
// Líneas 507-575
void Server::_handleClientRead(int clientFd) {
    Client& client = _clients[clientFd];

    // Verificación de estado: solo leer si estamos esperando datos
    if (client.getState() != CLIENT_READING)
        return;

    // Buffer temporal para recv()
    char buffer[BUFFER_SIZE];  // BUFFER_SIZE = 65536
    ssize_t bytesRead = recv(clientFd, buffer, BUFFER_SIZE, 0);

    // Manejo de errores y EOF
    if (bytesRead < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;  // No hay datos disponibles ahora, reintentar después
        Utils::logError("recv error on fd " + Utils::intToString(clientFd) + 
                       ": " + std::strerror(errno));
        _closeClient(clientFd);
        return;
    }
    if (bytesRead == 0) {
        // EOF: Cliente cerró la conexión
        _closeClient(clientFd);
        return;
    }

    client.updateLastActivity();  // Resetear timeout
    std::string data(buffer, bytesRead);

    try {
        // ═══════════════════════════════════════════════════════════════
        // PIPELINING HTTP (RFC 7230 §6.3.2)
        // ═══════════════════════════════════════════════════════════════
        // El cliente puede enviar múltiples peticiones sin esperar respuestas.
        // Debemos procesar todas las peticiones completas en el buffer.
        
        size_t consumed = client.getRequest().parse(data);

        // Verificar errores de parsing
        if (client.getRequest().hasError()) {
            Utils::logError("Parser error on fd " + Utils::intToString(clientFd) +
                           ", error code: " + Utils::intToString(
                               client.getRequest().getErrorCode()));
            int code = client.getRequest().getErrorCode();
            if (code == 0) code = 400;  // Default: Bad Request
            _sendErrorResponse(client, code);
            return;
        }

        // Procesar todas las peticiones completas (pipelining)
        while (client.getRequest().isComplete()) {
            _processRequest(client);

            // Después de procesar, verificar si hay más datos
            if (client.getState() == CLIENT_READING) {
                // Intentar parsear más datos del buffer interno
                consumed = client.getRequest().parse("");

                // Verificar errores en petición pipelined
                if (client.getRequest().hasError()) {
                    Utils::logError("Parser error on fd " + Utils::intToString(clientFd) +
                                   " while parsing pipelined request");
                    int code = client.getRequest().getErrorCode();
                    if (code == 0) code = 400;
                    _sendErrorResponse(client, code);
                    return;
                }

                // Si no hay más datos completos, salir del bucle
                if (consumed == 0 && !client.getRequest().isComplete()) 
                    break;
            } else {
                // Cliente cambió de estado (WRITING, CGI_RUNNING)
                break;
            }
        }
    } catch (const std::exception& e) {
        Utils::logError("Exception in _handleClientRead: " + std::string(e.what()));
        _closeClient(clientFd);
    }
}
```

**HTTP Pipelining (RFC 7230 §6.3.2):**

```
Sin Pipelining:                      Con Pipelining:
─────────────────                    ─────────────────
Cliente → Servidor                   Cliente → Servidor
  GET /a                               GET /a
        ← Response /a                  GET /b
  GET /b                               GET /c
        ← Response /b                        ← Response /a
  GET /c                                     ← Response /b
        ← Response /c                        ← Response /c

Tiempo: ─────────────────────────    Tiempo: ─────────────
        Largo (round-trips)                  Corto (paralelo)
```

## 1.11 Escritura de Respuestas

```cpp
// Líneas 577-647
void Server::_handleClientWrite(int clientFd)
{
    std::map<int, Client>::iterator it = _clients.find(clientFd);
    if (it == _clients.end())
        return;

    Client& client = it->second;

    // Si el buffer está vacío, verificar qué hacer
    if (client.getWriteBufferSize() == 0)
    {
        if (!client.shouldKeepAlive())
        {
            _closeClient(clientFd);  // Cerrar si no hay keep-alive
        }
        else
        {
            client.reset();  // Preparar para siguiente petición
        }
        return;
    }

    std::string& buffer = client.getWriteBuffer();
    size_t bufferSize = buffer.size();

    // ═══════════════════════════════════════════════════════════════════
    // OPTIMIZACIÓN: Para respuestas grandes (>1MB), intentar múltiples
    // llamadas a send() en una sola iteración del bucle principal.
    // Esto es crucial para respuestas CGI de ~100MB.
    // ═══════════════════════════════════════════════════════════════════
    size_t totalSent = 0;
    int attempts = (bufferSize > 1048576) ? 10 : 1;  // 10 intentos para grandes

    for (int i = 0; i < attempts && bufferSize > 0; ++i)
    {
        ssize_t sent = send(clientFd, buffer.c_str(), bufferSize, 0);

        if (sent < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;  // Buffer del socket lleno, reintentar después

            // Error real
            _closeClient(clientFd);
            return;
        }

        if (sent == 0)
            break;  // No se pudo enviar nada

        totalSent += sent;
        client.eraseFromWriteBuffer(sent);  // Eliminar bytes enviados
        bufferSize = client.getWriteBufferSize();

        if (bufferSize == 0)
            break;  // Todo enviado
    }

    if (totalSent > 0)
        client.updateLastActivity();  // Resetear timeout

    // Verificar si terminamos de enviar
    if (client.getWriteBufferSize() == 0)
    {
        // Buffer completamente enviado
        
        // Procesar CGI pendiente si hay capacidad disponible
        if (!_cgiQueue.empty())
            _processNextCgiFromQueue();

        if (!client.shouldKeepAlive())
            _closeClient(clientFd);
        else
            client.reset();  // Listo para siguiente petición
    }
}
```

**Flujo de send() con buffer grande:**

```
Buffer de escritura: [████████████████████████████████] 100MB
                     ▲
                     └── Posición actual

Iteración 1:
send(64KB) → éxito
Buffer: [▓▓▓▓████████████████████████████] restante

Iteración 2:
send(64KB) → éxito
Buffer: [▓▓▓▓▓▓▓▓████████████████████████] restante

... (continúa hasta EAGAIN o 10 intentos)

Siguiente ciclo de poll():
POLLOUT indica que el socket está listo para más datos
→ Continuar enviando
```

## 1.12 Procesamiento de Peticiones

```cpp
// Líneas 875-942
void Server::_processRequest(Client& client)
{
    const Request& request = client.getRequest();

    // Log de la petición
    Utils::logInfo(request.getMethod() + " " + request.getUri() + 
                   " from " + client.getIp());

    // Debug: mostrar tamaño del body
    if (request.getContentLength() > 0)
    {
        Utils::logDebug("Request Content-Length: " + 
                       Utils::intToString(request.getContentLength()) +
                       ", actual body size: " + 
                       Utils::intToString(request.getBody().size()) + " bytes");
    }

    // ═══════════════════════════════════════════════════════════════════
    // PASO 1: Seleccionar servidor virtual
    // ═══════════════════════════════════════════════════════════════════
    const ServerConfig* serverConfig = _selectServer(client);
    if (serverConfig == NULL)
    {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    client.setServerConfig(serverConfig);

    // ═══════════════════════════════════════════════════════════════════
    // PASO 2: Buscar location más específica
    // ═══════════════════════════════════════════════════════════════════
    const LocationConfig* location = serverConfig->findLocation(request.getPath());

    // ═══════════════════════════════════════════════════════════════════
    // PASO 3: Verificar redirección
    // ═══════════════════════════════════════════════════════════════════
    if (location != NULL && location->hasRedirect())
    {
        _handleRedirect(client, location);
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // PASO 4: Verificar método permitido
    // ═══════════════════════════════════════════════════════════════════
    const std::string& method = request.getMethod();
    
    if (location != NULL && !location->isMethodAllowed(method))
    {
        // RFC 7231 §6.5.5: 405 Method Not Allowed
        // DEBE incluir header Allow con los métodos permitidos
        _sendErrorResponse(client, HTTP_METHOD_NOT_ALLOWED);
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // PASO 5: Verificar tamaño del body
    // ═══════════════════════════════════════════════════════════════════
    size_t maxBodySize = serverConfig->getMaxBodySize();
    if (location != NULL && location->getMaxBodySize() > 0)
        maxBodySize = location->getMaxBodySize();  // Override de location
    
    if (request.getContentLength() > maxBodySize || 
        request.getBody().size() > maxBodySize)
    {
        // RFC 7231 §6.5.11: 413 Payload Too Large
        _sendErrorResponse(client, HTTP_PAYLOAD_TOO_LARGE);
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // PASO 6: Enrutar por método HTTP
    // ═══════════════════════════════════════════════════════════════════
    if (method == "GET" || method == "HEAD")
        _handleGet(client, location);
    else if (method == "POST")
        _handlePost(client, location);
    else if (method == "PUT")
        _handlePut(client, location);
    else if (method == "DELETE")
        _handleDelete(client, location);
    else
        _sendErrorResponse(client, HTTP_NOT_IMPLEMENTED);
}
```

## 1.13 Selección de Virtual Host

```cpp
// Líneas 944-973
const ServerConfig* Server::_selectServer(const Client& client)
{
    const Request& request = client.getRequest();
    std::string host = request.getHost();   // Del header Host
    int port = client.getPort();            // Puerto donde se recibió

    const std::vector<ServerConfig>& servers = _config.getServers();
    const ServerConfig* defaultServer = NULL;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        const ServerConfig& server = servers[i];
        
        // Filtrar por puerto
        if (server.getPort() != port)
            continue;

        // Guardar el primer servidor de este puerto como default
        if (defaultServer == NULL)
            defaultServer = &server;

        // Buscar coincidencia de server_name
        const std::vector<std::string>& names = server.getServerNames();
        for (size_t j = 0; j < names.size(); ++j)
        {
            if (names[j] == host)
                return &server;  // Coincidencia exacta
        }
    }

    // Si no hay coincidencia, usar el servidor por defecto del puerto
    return defaultServer;
}
```

**Algoritmo de selección:**

```
Petición: GET / HTTP/1.1
          Host: api.example.com:8080

Servidores configurados:
┌─────────────────────────────────────────────────────────┐
│ [0] listen 8080; server_name www.example.com            │ ← default para :8080
│ [1] listen 8080; server_name api.example.com            │ ← MATCH!
│ [2] listen 8081; server_name admin.example.com          │ ← puerto diferente
└─────────────────────────────────────────────────────────┘

Resultado: Servidor [1]
```

## 1.14 Handler GET/HEAD

```cpp
// Líneas 979-1099
void Server::_handleGet(Client& client, const LocationConfig* location)
{
    const Request& request = client.getRequest();
    const ServerConfig* serverConfig = client.getServerConfig();
    
    // Resolver ruta del archivo
    std::string filePath = _resolvePath(request, location, serverConfig);

    // ═══════════════════════════════════════════════════════════════════
    // Verificar si es CGI
    // ═══════════════════════════════════════════════════════════════════
    if (location != NULL && _isCgiRequest(filePath, location))
    {
        _handleCgi(client, location, filePath);
        return;
    }

    // Obtener información del archivo
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) < 0)
    {
        _sendErrorResponse(client, HTTP_NOT_FOUND);
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Manejo de directorios
    // ═══════════════════════════════════════════════════════════════════
    if (S_ISDIR(fileStat.st_mode))
    {
        // Buscar archivo índice
        std::vector<std::string> indexFiles;
        if (location != NULL && !location->getIndex().empty())
            indexFiles.push_back(location->getIndex());
        if (indexFiles.empty() && serverConfig != NULL && 
            !serverConfig->getIndex().empty())
            indexFiles.push_back(serverConfig->getIndex());
        if (indexFiles.empty())
            indexFiles.push_back("index.html");  // Default

        bool indexFound = false;
        for (size_t i = 0; i < indexFiles.size(); ++i)
        {
            std::string indexPath = filePath;
            if (indexPath[indexPath.length() - 1] != '/')
                indexPath += '/';
            indexPath += indexFiles[i];

            if (stat(indexPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode))
            {
                filePath = indexPath;
                indexFound = true;
                break;
            }
        }

        if (!indexFound)
        {
            std::string uri = request.getPath();
            bool hasTrailingSlash = (!uri.empty() && uri[uri.length() - 1] == '/');
            bool autoindex = (location != NULL) ? location->getAutoindex() : false;

            if (hasTrailingSlash && autoindex)
            {
                // Mostrar listado de directorio
                _handleDirectoryListing(client, filePath, request.getPath());
                return;
            }
            else if (!hasTrailingSlash && autoindex)
            {
                // Redireccionar a URI con / final
                // Esto es importante para que los links relativos funcionen
                Response resp = Response::makeRedirect(301, uri + "/");
                std::string response = resp.build();
                client.appendToWriteBuffer(response);
                client.setState(CLIENT_WRITING);
                return;
            }
            else
            {
                // Sin índice y sin autoindex: 404
                _sendErrorResponse(client, HTTP_NOT_FOUND);
                return;
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    // Servir archivo
    // ═══════════════════════════════════════════════════════════════════
    
    // Verificar que es archivo regular
    if (stat(filePath.c_str(), &fileStat) < 0 || !S_ISREG(fileStat.st_mode))
    {
        _sendErrorResponse(client, HTTP_NOT_FOUND);
        return;
    }

    // Verificar permisos de lectura
    if (access(filePath.c_str(), R_OK) < 0)
    {
        _sendErrorResponse(client, HTTP_FORBIDDEN);
        return;
    }

    // Leer contenido del archivo
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file)
    {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // Construir respuesta
    std::string mimeType = MimeTypes::getInstance().getMimeTypeByFile(filePath);
    Response resp;
    resp.setStatusCode(HTTP_OK);
    resp.setContentType(mimeType);
    resp.setBody(content);
    
    // HEAD: mismos headers pero sin body (RFC 7231 §4.3.2)
    std::string response = resp.build(request.getMethod() == "HEAD");

    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}
```

## 1.15 Ejecución de CGI

```cpp
// Líneas 1265-1553
void Server::_handleCgi(Client& client, const LocationConfig* location, 
                        const std::string& filePath)
{
    const Request& request = client.getRequest();
    const ServerConfig* serverConfig = client.getServerConfig();

    // Obtener handler CGI por extensión
    std::string extension;
    size_t dotPos = filePath.rfind('.');
    if (dotPos != std::string::npos)
        extension = filePath.substr(dotPos);

    std::string cgiPath;
    if (location != NULL)
        cgiPath = location->getCgiHandler(extension);
    
    if (cgiPath.empty())
    {
        Utils::logError("CGI handler not configured for extension: " + extension);
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    // Verificar que el handler existe y es ejecutable
    if (access(cgiPath.c_str(), X_OK) < 0)
    {
        Utils::logError("CGI handler not found or not executable: " + cgiPath);
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Control de concurrencia CGI
    // ═══════════════════════════════════════════════════════════════════
    if (_activeCgiCount >= MAX_CONCURRENT_CGI)  // MAX_CONCURRENT_CGI = 5
    {
        Utils::logDebug("Maximum concurrent CGI processes reached, queuing request");
        _cgiQueue.push_back(client.getFd());
        client.setState(CLIENT_PROCESSING);  // Evitar timeout mientras espera
        client.updateLastActivity();
        return;
    }

    // Convertir path del handler a absoluto
    std::string absoluteCgiPath;
    if (cgiPath[0] == '/')
    {
        absoluteCgiPath = cgiPath;
    }
    else
    {
        char currentDir[PATH_MAX];
        if (getcwd(currentDir, sizeof(currentDir)) != NULL)
            absoluteCgiPath = std::string(currentDir) + "/" + cgiPath;
        else
        {
            _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
            return;
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    // Crear pipe para stdout del CGI y tmpfile para stdin
    // ═══════════════════════════════════════════════════════════════════
    /*
     * ¿Por qué tmpfile en vez de pipe para stdin?
     * 
     * Con cuerpos grandes (100MB), un pipe causaría deadlock:
     * - Padre intenta escribir 100MB al pipe
     * - Pipe se llena (64KB por defecto en Linux)
     * - Padre se bloquea esperando que el hijo lea
     * - Hijo no puede leer porque aún no ha hecho exec()
     * 
     * Solución: tmpfile() permite escribir todo el body antes de fork()
     */
    
    int pipeOut[2];  // Para leer stdout del CGI
    if (pipe(pipeOut) < 0)
    {
        Utils::logError("Failed to create pipe for CGI");
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    FILE* tmpfp = tmpfile();  // Archivo temporal para stdin
    if (tmpfp == NULL)
    {
        close(pipeOut[0]);
        close(pipeOut[1]);
        Utils::logError("Failed to create tmpfile for CGI");
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    int tmpfd = fileno(tmpfp);

    // Escribir body de la petición al tmpfile
    const std::string& body = request.getBody();
    if (!body.empty())
    {
        size_t written = fwrite(body.c_str(), 1, body.size(), tmpfp);
        if (written != body.size())
        {
            Utils::logError("Failed to write body to tmpfile");
            fclose(tmpfp);
            close(pipeOut[0]);
            close(pipeOut[1]);
            _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
            return;
        }
        fflush(tmpfp);
        lseek(tmpfd, 0, SEEK_SET);  // Rebobinar para que CGI lea desde el inicio
    }

    // ═══════════════════════════════════════════════════════════════════
    // fork() - Crear proceso hijo
    // ═══════════════════════════════════════════════════════════════════
    pid_t pid = fork();
    if (pid < 0)
    {
        fclose(tmpfp);
        close(pipeOut[0]);
        close(pipeOut[1]);
        Utils::logError("Failed to fork for CGI");
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    if (pid == 0)
    {
        // ═══════════════════════════════════════════════════════════════
        // PROCESO HIJO - Ejecutar CGI
        // ═══════════════════════════════════════════════════════════════
        close(pipeOut[0]);  // Cerrar extremo de lectura

        // Redirigir stdin y stdout
        dup2(tmpfd, STDIN_FILENO);      // tmpfile → stdin
        dup2(pipeOut[1], STDOUT_FILENO); // pipe → stdout

        close(pipeOut[1]);

        // Cambiar al directorio del script
        std::string scriptDir = filePath.substr(0, filePath.rfind('/'));
        std::string scriptName = filePath.substr(filePath.rfind('/') + 1);
        
        // Obtener ruta absoluta del script
        char absoluteScriptPath[PATH_MAX];
        if (realpath(filePath.c_str(), absoluteScriptPath) == NULL)
        {
            // Si realpath falla, construir manualmente
            char currentDir[PATH_MAX];
            if (getcwd(currentDir, sizeof(currentDir)) != NULL)
            {
                std::string absPath = std::string(currentDir) + "/" + filePath;
                std::strncpy(absoluteScriptPath, absPath.c_str(), PATH_MAX - 1);
            }
        }
        
        if (!scriptDir.empty())
            chdir(scriptDir.c_str());

        // ═══════════════════════════════════════════════════════════════
        // Construir variables de entorno (RFC 3875)
        // ═══════════════════════════════════════════════════════════════
        std::vector<std::string> envStrings;
        
        // Variables CGI obligatorias
        envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
        envStrings.push_back("SERVER_PROTOCOL=" + request.getVersion());
        envStrings.push_back("SERVER_SOFTWARE=webserv/1.0");
        envStrings.push_back("REQUEST_METHOD=" + request.getMethod());
        envStrings.push_back("REDIRECT_STATUS=200");  // Requerido por php-cgi
        
        // PATH_INFO: Para 42 tester, es la ruta completa (no estándar)
        envStrings.push_back("SCRIPT_FILENAME=" + std::string(absoluteScriptPath));
        envStrings.push_back("SCRIPT_NAME=" + request.getPath());
        envStrings.push_back("PATH_INFO=" + request.getPath());
        envStrings.push_back("REQUEST_URI=" + request.getPath());
        envStrings.push_back("QUERY_STRING=" + request.getQuery());
        
        // Content-Type y Content-Length
        std::string contentType = request.getHeader("Content-Type");
        if (!contentType.empty())
            envStrings.push_back("CONTENT_TYPE=" + contentType);
        
        if (request.getContentLength() > 0)
            envStrings.push_back("CONTENT_LENGTH=" + 
                                Utils::intToString(request.getContentLength()));
        else
            envStrings.push_back("CONTENT_LENGTH=0");
        
        // Información del servidor
        if (serverConfig != NULL)
        {
            envStrings.push_back("SERVER_NAME=" + 
                                serverConfig->getServerNames()[0]);
            envStrings.push_back("SERVER_PORT=" + 
                                Utils::intToString(serverConfig->getPort()));
        }

        // Información del cliente
        envStrings.push_back("REMOTE_ADDR=" + client.getIp());
        envStrings.push_back("REMOTE_HOST=" + client.getIp());

        // Convertir headers HTTP a variables HTTP_*
        const std::map<std::string, std::string>& headers = request.getHeaders();
        for (std::map<std::string, std::string>::const_iterator it = headers.begin();
             it != headers.end(); ++it)
        {
            std::string envName = "HTTP_" + it->first;
            // Convertir a mayúsculas y - a _
            for (size_t i = 0; i < envName.length(); ++i)
            {
                if (envName[i] == '-')
                    envName[i] = '_';
                else
                    envName[i] = std::toupper(envName[i]);
            }
            envStrings.push_back(envName + "=" + it->second);
        }

        // Convertir a char**
        std::vector<char*> env;
        for (size_t i = 0; i < envStrings.size(); ++i)
            env.push_back(const_cast<char*>(envStrings[i].c_str()));
        env.push_back(NULL);

        // Argumentos: [handler, script, NULL]
        char* args[3];
        args[0] = const_cast<char*>(absoluteCgiPath.c_str());
        args[1] = const_cast<char*>(scriptName.c_str());
        args[2] = NULL;

        // ═══════════════════════════════════════════════════════════════
        // execve() - Reemplazar proceso con el CGI
        // ═══════════════════════════════════════════════════════════════
        execve(absoluteCgiPath.c_str(), args, &env[0]);
        
        // Si llegamos aquí, execve falló
        Utils::logError("execve failed for CGI: " + absoluteCgiPath);
        _exit(1);
    }

    // ═══════════════════════════════════════════════════════════════════
    // PROCESO PADRE - Configurar monitoreo
    // ═══════════════════════════════════════════════════════════════════
    close(pipeOut[1]);  // Cerrar extremo de escritura
    fclose(tmpfp);      // El hijo tiene su propia copia del fd

    // Configurar pipe como non-blocking
    if (!_setNonBlocking(pipeOut[0]))
    {
        close(pipeOut[0]);
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        Utils::logError("Failed to set non-blocking on CGI pipe");
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    // Registrar CGI activo
    _cgiToClient[pipeOut[0]] = client.getFd();
    client.setCgiPid(pid);
    client.setCgiFdOut(pipeOut[0]);
    client.setCgiStartTime(std::time(NULL));
    client.setState(CLIENT_CGI_RUNNING);
    client.getCgiOutput().clear();

    ++_activeCgiCount;

    Utils::logDebug("CGI started: " + absoluteCgiPath + " for script: " + filePath);
}
```

**Diagrama de CGI:**

```
┌────────────────────────────────────────────────────────────────────────────┐
│                            ANTES DE fork()                                  │
│                                                                            │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │                        PROCESO SERVIDOR                              │  │
│  │                                                                      │  │
│  │  tmpfile ─────────────────────────────┐                              │  │
│  │  [body de petición HTTP]              │                              │  │
│  │                                       │                              │  │
│  │  pipeOut[0] ◄────── pipe ─────────── pipeOut[1]                      │  │
│  │  (lectura)                           (escritura)                     │  │
│  │                                                                      │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘

                                   │
                                   │ fork()
                                   ▼

┌────────────────────────────────────────────────────────────────────────────┐
│                           DESPUÉS DE fork()                                 │
│                                                                            │
│  ┌─────────────────────────────────┐    ┌─────────────────────────────────┐│
│  │      PROCESO PADRE              │    │       PROCESO HIJO              ││
│  │      (Servidor)                 │    │       (CGI)                     ││
│  │                                 │    │                                 ││
│  │  tmpfile: cerrado (fclose)      │    │  tmpfile → STDIN (dup2)         ││
│  │                                 │    │                                 ││
│  │  pipeOut[0] ◄── [lectura] ──────┼────┼── pipeOut[1] → STDOUT (dup2)    ││
│  │  (monitorear con poll())        │    │                                 ││
│  │                                 │    │  execve(handler, script, env)   ││
│  │  pipeOut[1]: cerrado            │    │  → /usr/bin/python3 script.py   ││
│  │                                 │    │                                 ││
│  └─────────────────────────────────┘    └─────────────────────────────────┘│
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
```

## 1.16 Verificación de Timeouts

```cpp
// Líneas 824-869
void Server::_checkTimeouts()
{
    std::vector<int> toClose;

    for (std::map<int, Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        Client& client = it->second;
        ClientState state = client.getState();

        // Clientes en cola CGI: no aplicar timeout (esperando slot)
        if (state == CLIENT_PROCESSING)
        {
            continue;
        }
        // CGI ejecutándose: timeout específico de CGI
        else if (state == CLIENT_CGI_RUNNING && client.hasCgiTimedOut(CGI_TIMEOUT))
        {
            Utils::logDebug("CGI timeout on fd " + Utils::intToString(it->first));
            toClose.push_back(it->first);
        }
        // Enviando respuesta CGI grande: timeout extendido
        else if (state == CLIENT_WRITING &&
                (client.getCgiOutput().size() > 10485760 || 
                 client.getWriteBufferSize() > 10485760))  // >10MB
        {
            if (client.hasTimedOut(CGI_RESPONSE_TIMEOUT))  // 180 segundos
            {
                Utils::logDebug("CGI response write timeout on fd " + 
                               Utils::intToString(it->first));
                toClose.push_back(it->first);
            }
        }
        // Timeout normal de conexión
        else if (client.hasTimedOut(CONNECTION_TIMEOUT))  // 60 segundos
        {
            Utils::logDebug("Connection timeout on fd " + 
                           Utils::intToString(it->first));
            toClose.push_back(it->first);
        }
    }

    // Cerrar conexiones con timeout
    for (size_t i = 0; i < toClose.size(); ++i)
    {
        _closeClient(toClose[i]);
    }
}
```

**Tabla de Timeouts:**

| Estado | Timeout | Valor | Justificación |
|--------|---------|-------|---------------|
| `CLIENT_READING` | `CONNECTION_TIMEOUT` | 60s | Petición incompleta |
| `CLIENT_WRITING` | `CONNECTION_TIMEOUT` | 60s | Respuesta pequeña |
| `CLIENT_WRITING` + >10MB | `CGI_RESPONSE_TIMEOUT` | 180s | Respuesta CGI grande |
| `CLIENT_CGI_RUNNING` | `CGI_TIMEOUT` | 120s | CGI en ejecución |
| `CLIENT_PROCESSING` | N/A | ∞ | Esperando en cola CGI |

---

# 2. REQUEST.CPP - PARSER HTTP DETALLADO

## 2.1 Máquina de Estados del Parser

```
                    ┌─────────────────────────┐
                    │  PARSE_REQUEST_LINE     │
                    │  Esperando: METHOD SP   │
                    │  URI SP VERSION CRLF    │
                    └───────────┬─────────────┘
                                │
                          ┌─────┴─────┐
                          │ ¿Válida?  │
                          └─────┬─────┘
                       NO ┌─────┴─────┐ SÍ
                          │           │
              ┌───────────▼───┐   ┌───▼───────────────────┐
              │ PARSE_ERROR   │   │    PARSE_HEADERS      │
              │ code: 400/501 │   │ Esperando: name:value │
              │      /505/414 │   │ hasta CRLF CRLF       │
              └───────────────┘   └───────────┬───────────┘
                                              │
                                   ┌──────────┴──────────┐
                                   │ ¿Línea vacía (CRLF)?│
                                   └──────────┬──────────┘
                                  NO ┌────────┴────────┐ SÍ
                                     │                │
                            ┌────────▼────────┐   ┌───▼───────────────────┐
                            │ Parsear header  │   │ ¿Transfer-Encoding:   │
                            │ y continuar     │   │  chunked?             │
                            └─────────────────┘   └───────────┬───────────┘
                                                       SÍ ┌───┴───┐ NO
                                                          │       │
                                           ┌──────────────▼──┐ ┌──▼───────────────┐
                                           │ PARSE_CHUNKED   │ │ ¿Content-Length  │
                                           │                 │ │  > 0?            │
                                           └────────┬────────┘ └────────┬─────────┘
                                                    │              SÍ ┌─┴─┐ NO
                                                    │                 │   │
                                           ┌────────▼────────┐ ┌──────▼──┐│
                                           │ Parsear chunks  │ │PARSE_   ││
                                           │ hasta 0\r\n     │ │BODY     │▼
                                           └────────┬────────┘ └────┬────┘┌─────────────┐
                                                    │               │     │PARSE_COMPLETE│
                                                    │               │     │(sin body)   │
                                                    └───────┬───────┘     └─────────────┘
                                                            │
                                                   ┌────────▼────────┐
                                                   │ PARSE_COMPLETE  │
                                                   │ (con body)      │
                                                   └─────────────────┘
```

## 2.2 Método Principal de Parsing

```cpp
// Líneas 91-174
size_t Request::parse(const std::string& data) {
    // 1. Guardar longitud inicial del buffer interno
    size_t initial_internal_len = _rawRequest.length();
    
    // 2. Añadir nuevos datos al buffer
    _rawRequest += data;
    
    // 3. Máquina de estados principal
    while (_state != PARSE_COMPLETE && _state != PARSE_ERROR) {
        size_t old_len = _rawRequest.length();
        
        // ═══════════════════════════════════════════════════════════════
        // ESTADO: PARSE_REQUEST_LINE
        // Formato: METHOD SP request-target SP HTTP-version CRLF
        // Ejemplo: "GET /index.html HTTP/1.1\r\n"
        // ═══════════════════════════════════════════════════════════════
        if (_state == PARSE_REQUEST_LINE) {
            size_t pos = _rawRequest.find("\r\n");
            if (pos == std::string::npos) 
                break;  // Necesitamos más datos
            
            std::string line = _rawRequest.substr(0, pos);
            _rawRequest.erase(0, pos + 2);  // Eliminar línea + CRLF
            
            // RFC 7230 §3.5: Ignorar líneas vacías antes de request-line
            if (line.empty()) 
                continue;
            
            if (!_parseRequestLine(line)) { 
                _state = PARSE_ERROR; 
                break; 
            }
            _state = PARSE_HEADERS;
        }
        
        // ═══════════════════════════════════════════════════════════════
        // ESTADO: PARSE_HEADERS
        // Formato: field-name ":" OWS field-value OWS CRLF
        // Termina: CRLF (línea vacía)
        // ═══════════════════════════════════════════════════════════════
        else if (_state == PARSE_HEADERS) {
            size_t pos = _rawRequest.find("\r\n");
            if (pos == std::string::npos) 
                break;  // Necesitamos más datos
            
            if (pos == 0) {
                // Línea vacía: fin de headers
                _rawRequest.erase(0, 2);
                _headersParsed = true;
                
                // Parsear headers especiales
                _parseHost();      // Extraer Host
                _parseCookies();   // Extraer Cookies

                // Determinar cómo leer el body
                std::string transferEncoding = getHeader("Transfer-Encoding");
                if (Utils::toLower(transferEncoding) == "chunked") {
                    _isChunked = true;
                    _state = PARSE_CHUNKED;
                } else if (hasHeader("Content-Length")) {
                    _contentLength = Utils::stringToSizeT(
                        getHeader("Content-Length"));
                    _state = (_contentLength > 0) ? PARSE_BODY : PARSE_COMPLETE;
                } else {
                    // Sin body
                    _state = PARSE_COMPLETE;
                }
            } else {
                // Parsear header
                std::string line = _rawRequest.substr(0, pos);
                _rawRequest.erase(0, pos + 2);
                if (!_parseHeader(line)) {
                    _state = PARSE_ERROR;
                    break;
                }
            }
        }
        
        // ═══════════════════════════════════════════════════════════════
        // ESTADO: PARSE_BODY (Content-Length)
        // Leer exactamente _contentLength bytes
        // ═══════════════════════════════════════════════════════════════
        else if (_state == PARSE_BODY) {
            size_t bytesToRead = _contentLength - _bodyBytesReceived;
            size_t available = _rawRequest.length();
            size_t readNow = std::min(bytesToRead, available);
            
            _body += _rawRequest.substr(0, readNow);
            _rawRequest.erase(0, readNow);
            _bodyBytesReceived += readNow;
            
            if (_bodyBytesReceived >= _contentLength) {
                // Body completo - verificar si es multipart
                std::string contentType = getHeader("Content-Type");
                if (contentType.find("multipart/form-data") != std::string::npos) {
                    _parseMultipartBody();
                }
                _state = PARSE_COMPLETE;
            }
            break;  // No hay más que hacer hasta recibir más datos
        }
        
        // ═══════════════════════════════════════════════════════════════
        // ESTADO: PARSE_CHUNKED (Transfer-Encoding: chunked)
        // Formato: chunk-size CRLF chunk-data CRLF ... 0 CRLF CRLF
        // ═══════════════════════════════════════════════════════════════
        else if (_state == PARSE_CHUNKED) {
            if (!_parseChunkedBody())
                break;  // Necesitamos más datos
        }
        
        // Detectar bucle infinito (sin progreso)
        if (old_len == _rawRequest.length()) 
            break;
    }

    // 4. Calcular bytes consumidos
    // (inicial + nuevos) - restantes = consumidos
    size_t total_at_start = initial_internal_len + data.length();
    size_t remaining_at_end = _rawRequest.length();
    
    return total_at_start - remaining_at_end;
}
```

## 2.3 Parsing de Request Line

```cpp
// Líneas 263-300
bool Request::_parseRequestLine(const std::string& line) {
    // RFC 7230 §3.1.1: request-line = method SP request-target SP HTTP-version
    
    std::vector<std::string> parts = Utils::split(line, ' ');
    if (parts.size() != 3) {
        _errorCode = 400;  // Bad Request
        return false;
    }
    
    _method = parts[0];   // GET, POST, PUT, DELETE, HEAD
    _uri = parts[1];      // /path/to/resource?query=value#fragment
    _version = parts[2];  // HTTP/1.0 o HTTP/1.1
    
    // Validar método
    // RFC 7231 §4: Métodos HTTP definidos
    if (!Utils::isValidMethod(_method)) {
        _errorCode = 501;  // Not Implemented
        return false;
    }
    
    // Validar versión HTTP
    // Solo soportamos HTTP/1.0 y HTTP/1.1
    if (_version != "HTTP/1.0" && _version != "HTTP/1.1") {
        _errorCode = 505;  // HTTP Version Not Supported
        return false;
    }
    
    // Validar longitud de URI
    // RFC 7230 §3.1.1: No especifica límite, pero recomienda ≥8000
    if (_uri.length() > 8192) {
        _errorCode = 414;  // URI Too Long
        return false;
    }
    
    // Parsear componentes del URI
    _parseUri();

    if (_errorCode != 0) {
        return false;
    }

    return true;
}
```

**Ejemplos de Request Lines:**

```
✓ GET / HTTP/1.1
✓ POST /api/users HTTP/1.1
✓ DELETE /files/old.txt HTTP/1.0
✓ PUT /upload/new.bin HTTP/1.1

✗ GET HTTP/1.1              → 400 (falta URI)
✗ PATCH /api HTTP/1.1       → 501 (método no implementado)
✗ GET /api HTTP/2.0         → 505 (versión no soportada)
✗ GET /<8193 chars> HTTP/1.1 → 414 (URI muy larga)
```

## 2.4 Parsing de URI

```cpp
// Líneas 346-391
void Request::_parseUri() {
    std::string uri = _uri;

    // ═══════════════════════════════════════════════════════════════════
    // SEGURIDAD: Rechazar null bytes
    // Los null bytes pueden usarse para bypass de seguridad:
    // "/valid.txt%00.php" podría interpretarse como "/valid.txt"
    // ═══════════════════════════════════════════════════════════════════
    if (uri.find('\0') != std::string::npos || 
        uri.find("%00") != std::string::npos)
    {
        _errorCode = 400;
        return;
    }

    // Extraer fragment (#)
    // RFC 3986 §3.5: fragment = *( pchar / "/" / "?" )
    // El fragment no se envía al servidor, pero por si acaso...
    size_t fragPos = uri.find('#');
    if (fragPos != std::string::npos) {
        _fragment = uri.substr(fragPos + 1);
        uri = uri.substr(0, fragPos);
    }
    
    // Extraer query string (?)
    // RFC 3986 §3.4: query = *( pchar / "/" / "?" )
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos) {
        _query = uri.substr(queryPos + 1);
        uri = uri.substr(0, queryPos);
        _parseQueryString();  // Parsear parámetros
    }
    
    // URL decode el path
    // %20 → " ", %2F → "/", etc.
    _path = Utils::urlDecode(uri);
    
    // SEGURIDAD: Verificar null bytes después de decode
    if (_path.find('\0') != std::string::npos)
    {
        _errorCode = 400;
        return;
    }

    // Preservar trailing slash (importante para directorios)
    bool trailingSlash = (_path.length() > 0 && 
                         _path[_path.length() - 1] == '/');

    // Normalizar path (eliminar .., .)
    _path = Utils::normalizePath(_path);

    // Restaurar trailing slash si fue eliminado
    if (trailingSlash && _path.length() > 1 && 
        _path[_path.length() - 1] != '/')
        _path += "/";
}
```

**Ejemplo de parsing de URI:**

```
URI de entrada: /foo/../bar/./baz?name=john&age=30#section

Paso 1: Extraer fragment
  uri = "/foo/../bar/./baz?name=john&age=30"
  _fragment = "section"

Paso 2: Extraer query
  uri = "/foo/../bar/./baz"
  _query = "name=john&age=30"
  _queryParams = {"name": "john", "age": "30"}

Paso 3: URL decode
  _path = "/foo/../bar/./baz"

Paso 4: Normalizar
  _path = "/bar/baz"
```

## 2.5 Parsing de Transfer-Encoding: chunked

```cpp
// Líneas 524-562
bool Request::_parseChunkedBody() {
    // RFC 7230 §4.1: Chunked Transfer Coding
    // 
    // chunked-body = *chunk last-chunk trailer-part CRLF
    // chunk        = chunk-size CRLF chunk-data CRLF
    // chunk-size   = 1*HEXDIG
    // last-chunk   = 1*("0") CRLF
    
    while (!_rawRequest.empty()) {
        if (_currentChunkSize == 0) {
            // Buscando línea de tamaño de chunk
            size_t pos = _rawRequest.find("\r\n");
            if (pos == std::string::npos)
                return false;  // Necesitamos más datos
            
            std::string sizeLine = _rawRequest.substr(0, pos);
            _rawRequest.erase(0, pos + 2);
            
            // Ignorar extensiones de chunk (;extension=value)
            size_t extPos = sizeLine.find(';');
            if (extPos != std::string::npos)
                sizeLine = sizeLine.substr(0, extPos);
            
            // Parsear tamaño hexadecimal
            _currentChunkSize = Utils::hexToSizeT(sizeLine);
            
            if (_currentChunkSize == 0) {
                // Último chunk - saltar trailer y CRLF final
                size_t trailerEnd = _rawRequest.find("\r\n");
                if (trailerEnd != std::string::npos)
                    _rawRequest.erase(0, trailerEnd + 2);
                _state = PARSE_COMPLETE;
                return true;
            }
        } else {
            // Leyendo datos del chunk
            size_t available = _rawRequest.length();
            if (available < _currentChunkSize + 2)  // +2 para CRLF final
                return false;  // Necesitamos más datos
            
            // Añadir datos al body
            _body += _rawRequest.substr(0, _currentChunkSize);
            _rawRequest.erase(0, _currentChunkSize + 2);  // Eliminar datos + CRLF
            _currentChunkSize = 0;
        }
    }
    return false;  // Necesitamos más datos
}
```

**Ejemplo de body chunked:**

```
7\r\n                    ← Tamaño: 7 bytes (hex)
Mozilla\r\n              ← Datos: "Mozilla"
9\r\n                    ← Tamaño: 9 bytes
Developer\r\n            ← Datos: "Developer"
7\r\n                    ← Tamaño: 7 bytes
Network\r\n              ← Datos: "Network"
0\r\n                    ← Último chunk
\r\n                     ← Fin

Body resultante: "MozillaDeveloperNetwork"
```

## 2.6 Parsing de Multipart

```cpp
// Líneas 442-522
void Request::_parseMultipartBody() {
    // RFC 7578: multipart/form-data
    // RFC 2046 §5.1: Multipart Media Type
    
    // Extraer boundary del Content-Type
    std::string contentType = getHeader("Content-Type");
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos)
        return;
    
    std::string boundary = contentType.substr(boundaryPos + 9);
    // Eliminar comillas si las hay
    if (!boundary.empty() && boundary[0] == '"')
        boundary = boundary.substr(1, boundary.length() - 2);
    
    std::string delimiter = "--" + boundary;
    std::string endDelimiter = delimiter + "--";
    
    size_t pos = 0;
    while (pos < _body.length()) {
        // Buscar inicio de parte
        size_t partStart = _body.find(delimiter, pos);
        if (partStart == std::string::npos)
            break;
        
        partStart += delimiter.length();
        if (_body.substr(partStart, 2) == "--")
            break;  // Fin del multipart
        
        partStart += 2;  // Saltar \r\n
        
        // Buscar fin de parte
        size_t partEnd = _body.find(delimiter, partStart);
        if (partEnd == std::string::npos)
            break;
        
        std::string part = _body.substr(partStart, partEnd - partStart - 2);
        
        // Separar headers de datos
        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos)
            continue;
        
        std::string partHeaders = part.substr(0, headerEnd);
        std::string partData = part.substr(headerEnd + 4);
        
        // Parsear Content-Disposition
        size_t cdPos = partHeaders.find("Content-Disposition:");
        if (cdPos != std::string::npos) {
            size_t cdEnd = partHeaders.find("\r\n", cdPos);
            std::string cd = partHeaders.substr(cdPos + 20, cdEnd - cdPos - 20);
            
            UploadedFile file;
            
            // Extraer name="..."
            size_t namePos = cd.find("name=\"");
            if (namePos != std::string::npos) {
                namePos += 6;
                size_t nameEnd = cd.find("\"", namePos);
                file.name = cd.substr(namePos, nameEnd - namePos);
            }
            
            // Extraer filename="..."
            size_t fnPos = cd.find("filename=\"");
            if (fnPos != std::string::npos) {
                fnPos += 10;
                size_t fnEnd = cd.find("\"", fnPos);
                file.filename = cd.substr(fnPos, fnEnd - fnPos);
            }
            
            // Extraer Content-Type de la parte
            size_t ctPos = partHeaders.find("Content-Type:");
            if (ctPos != std::string::npos) {
                ctPos += 13;
                size_t ctEnd = partHeaders.find("\r\n", ctPos);
                file.contentType = Utils::trim(
                    partHeaders.substr(ctPos, ctEnd - ctPos));
            }
            
            file.data = partData;
            _uploadedFiles.push_back(file);
        }
        
        pos = partEnd;
    }
}
```

**Ejemplo de multipart:**

```
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA

------WebKitFormBoundary7MA
Content-Disposition: form-data; name="field1"

valor del campo 1
------WebKitFormBoundary7MA
Content-Disposition: form-data; name="file"; filename="test.txt"
Content-Type: text/plain

contenido del archivo
------WebKitFormBoundary7MA--

Resultado:
_uploadedFiles[0]:
  name: "field1"
  filename: ""
  contentType: ""
  data: "valor del campo 1"

_uploadedFiles[1]:
  name: "file"
  filename: "test.txt"
  contentType: "text/plain"
  data: "contenido del archivo"
```

---

# 3. CLIENT.CPP - GESTIÓN DE ESTADO DE CONEXIÓN

## 3.1 Estructura de Datos del Cliente

```cpp
class Client {
private:
    // ═══════════════════════════════════════════════════════════════════
    // Información de conexión
    // ═══════════════════════════════════════════════════════════════════
    int _fd;                          // File descriptor del socket
    std::string _ip;                  // IP del cliente (ej: "192.168.1.100")
    int _port;                        // Puerto del servidor donde conectó
    
    // ═══════════════════════════════════════════════════════════════════
    // Objetos HTTP
    // ═══════════════════════════════════════════════════════════════════
    Request _request;                 // Parser de petición HTTP
    Response _response;               // Constructor de respuesta HTTP
    const ServerConfig* _serverConfig;// Configuración del virtual host
    
    // ═══════════════════════════════════════════════════════════════════
    // Estado de la conexión
    // ═══════════════════════════════════════════════════════════════════
    ClientState _state;               // Estado actual en la máquina de estados
    time_t _lastActivity;             // Timestamp del último I/O (para timeout)
    
    // ═══════════════════════════════════════════════════════════════════
    // Buffers de I/O
    // ═══════════════════════════════════════════════════════════════════
    std::string _readBuffer;          // Datos leídos pendientes de parsing
    std::string _writeBuffer;         // Datos de respuesta pendientes de envío
    
    // ═══════════════════════════════════════════════════════════════════
    // Estado CGI
    // ═══════════════════════════════════════════════════════════════════
    pid_t _cgiPid;                    // PID del proceso CGI (-1 si no hay)
    int _cgiFdIn;                     // fd para enviar datos al CGI
    int _cgiFdOut;                    // fd para leer output del CGI
    std::string _cgiOutput;           // Buffer para acumular output CGI
    time_t _cgiStartTime;             // Timestamp de inicio del CGI
    
    // ═══════════════════════════════════════════════════════════════════
    // Keep-Alive
    // ═══════════════════════════════════════════════════════════════════
    bool _keepAlive;                  // ¿Mantener conexión después de respuesta?
    int _requestCount;                // Número de peticiones en esta conexión
};
```

## 3.2 Lógica de Keep-Alive

```cpp
// Líneas 284-294
bool Client::shouldKeepAlive() const {
    // Si fue explícitamente deshabilitado
    if (!_keepAlive)
        return false;
    
    std::string connection = _request.getHeader("Connection");
    
    // HTTP/1.0: No keep-alive por defecto
    // Solo si el cliente envía "Connection: keep-alive"
    if (_request.getVersion() == "HTTP/1.0") {
        return connection == "keep-alive";
    }
    
    // HTTP/1.1: Keep-alive por defecto (RFC 7230 §6.3)
    // Solo se cierra si el cliente envía "Connection: close"
    return connection != "close";
}
```

**Tabla de comportamiento Keep-Alive:**

| Versión HTTP | Header Connection | Resultado |
|--------------|-------------------|-----------|
| HTTP/1.0 | (ninguno) | Cerrar |
| HTTP/1.0 | keep-alive | Mantener |
| HTTP/1.0 | close | Cerrar |
| HTTP/1.1 | (ninguno) | Mantener |
| HTTP/1.1 | keep-alive | Mantener |
| HTTP/1.1 | close | Cerrar |

## 3.3 Reset para Pipelining

```cpp
// Líneas 308-325
void Client::reset() {
    // Limpiar estado de petición/respuesta
    _request.reset();
    _response.reset();
    
    // Volver al estado de lectura
    _state = CLIENT_READING;
    
    // Limpiar estado CGI
    _cgiPid = -1;
    _cgiFdIn = -1;
    _cgiFdOut = -1;
    _cgiOutput.clear();
    _cgiStartTime = 0;
    
    // ═══════════════════════════════════════════════════════════════════
    // CRÍTICO: NO limpiar _readBuffer
    // 
    // En HTTP pipelining, el cliente puede enviar múltiples peticiones
    // sin esperar respuestas. Después de procesar una petición, pueden
    // quedar datos de la siguiente petición en el buffer.
    // 
    // Ejemplo:
    // _readBuffer: "GET /page1 HTTP/1.1\r\n...\r\n\r\nGET /page2 HTTP/1.1\r\n..."
    //              ^^^^^^^^^^^^^^^^^^^^^^^^^^^^   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //              Primera petición (procesada)   Segunda petición (pendiente)
    // ═══════════════════════════════════════════════════════════════════
    // _readBuffer.clear();  // ¡¡¡NO HACER ESTO!!!
    
    // Sí limpiar buffer de escritura (respuesta ya enviada)
    _writeBuffer.clear();
    
    // Actualizar timestamp
    updateLastActivity();
}
```

---

# 4. INTERACCIÓN ENTRE MÓDULOS

## 4.1 Diagrama de Secuencia: Petición GET Completa

```
┌─────────┐          ┌────────┐          ┌────────┐          ┌──────────┐
│ Cliente │          │ Server │          │ Client │          │ Request  │
│ (TCP)   │          │        │          │        │          │          │
└────┬────┘          └────┬───┘          └────┬───┘          └────┬─────┘
     │                    │                   │                    │
     │ TCP Connect        │                   │                    │
     │───────────────────>│                   │                    │
     │                    │                   │                    │
     │                    │ accept()          │                    │
     │                    │──────────────────>│ new Client(fd)     │
     │                    │                   │                    │
     │                    │                   │ setState(READING)  │
     │                    │                   │<───────────────────│
     │                    │                   │                    │
     │                    │ poll() returns    │                    │
     │                    │ POLLIN on fd      │                    │
     │                    │                   │                    │
     │ GET /index.html    │                   │                    │
     │───────────────────>│                   │                    │
     │                    │                   │                    │
     │                    │ recv(fd)          │                    │
     │                    │──────────────────>│                    │
     │                    │                   │                    │
     │                    │                   │ getRequest()       │
     │                    │                   │───────────────────>│
     │                    │                   │                    │
     │                    │                   │ parse(data)        │
     │                    │                   │───────────────────>│
     │                    │                   │                    │
     │                    │                   │ isComplete() = true│
     │                    │                   │<───────────────────│
     │                    │                   │                    │
     │                    │ _processRequest() │                    │
     │                    │<──────────────────│                    │
     │                    │                   │                    │
     │                    │ _selectServer()   │                    │
     │                    │ _handleGet()      │                    │
     │                    │                   │                    │
     │                    │                   │ appendToWriteBuffer│
     │                    │──────────────────>│                    │
     │                    │                   │                    │
     │                    │                   │ setState(WRITING)  │
     │                    │──────────────────>│                    │
     │                    │                   │                    │
     │                    │ poll() returns    │                    │
     │                    │ POLLOUT on fd     │                    │
     │                    │                   │                    │
     │                    │ send()            │                    │
     │<───────────────────│──────────────────>│                    │
     │ HTTP/1.1 200 OK    │                   │                    │
     │ Content-Type: ...  │                   │                    │
     │                    │                   │                    │
     │                    │                   │ shouldKeepAlive()  │
     │                    │                   │ = true             │
     │                    │                   │                    │
     │                    │                   │ reset()            │
     │                    │──────────────────>│                    │
     │                    │                   │                    │
     │                    │                   │ setState(READING)  │
     │                    │                   │ (listo para más)   │
     │                    │                   │                    │
```

## 4.2 Diagrama de Secuencia: Petición CGI

```
┌─────────┐     ┌────────┐     ┌────────┐     ┌───────────┐     ┌─────────┐
│ Cliente │     │ Server │     │ Client │     │ CGI Proc  │     │ Python  │
│ (TCP)   │     │        │     │        │     │ (fork)    │     │ Script  │
└────┬────┘     └────┬───┘     └────┬───┘     └─────┬─────┘     └────┬────┘
     │               │              │               │                 │
     │ POST /cgi/api │              │               │                 │
     │ body: {...}   │              │               │                 │
     │──────────────>│              │               │                 │
     │               │              │               │                 │
     │               │ parse()      │               │                 │
     │               │─────────────>│               │                 │
     │               │              │               │                 │
     │               │ _handleCgi() │               │                 │
     │               │              │               │                 │
     │               │ tmpfile()    │               │                 │
     │               │ write(body)  │               │                 │
     │               │              │               │                 │
     │               │ pipe()       │               │                 │
     │               │              │               │                 │
     │               │ fork() ──────┼───────────────┼─────────────────>│
     │               │              │               │                 │
     │               │              │               │ dup2(tmpfile,   │
     │               │              │               │      STDIN)     │
     │               │              │               │                 │
     │               │              │               │ dup2(pipe,      │
     │               │              │               │      STDOUT)    │
     │               │              │               │                 │
     │               │              │               │ execve(python)  │
     │               │              │               │────────────────>│
     │               │              │               │                 │
     │               │ setState(    │               │                 │
     │               │ CGI_RUNNING) │               │                 │
     │               │─────────────>│               │                 │
     │               │              │               │                 │
     │               │ poll() on    │               │ read STDIN      │
     │               │ cgi pipe     │               │<────────────────│
     │               │              │               │                 │
     │               │              │               │ process...      │
     │               │              │               │                 │
     │               │              │               │ write STDOUT    │
     │               │              │               │────────────────>│
     │               │              │               │                 │
     │               │ POLLIN       │               │                 │
     │               │<─────────────┼───────────────│                 │
     │               │              │               │                 │
     │               │ read(pipe)   │               │                 │
     │               │─────────────>│ appendCgiOut  │                 │
     │               │              │               │                 │
     │               │ (repeat...)  │               │                 │
     │               │              │               │                 │
     │               │ EOF on pipe  │               │                 │
     │               │<─────────────┼───────────────│ exit(0)         │
     │               │              │               │                 │
     │               │ waitpid()    │               │                 │
     │               │              │               │                 │
     │               │ _prepareCgi  │               │                 │
     │               │ Response()   │               │                 │
     │               │              │               │                 │
     │               │ Response::   │               │                 │
     │               │ makeFromCGI()│               │                 │
     │               │              │               │                 │
     │               │ appendTo     │               │                 │
     │               │ WriteBuffer  │               │                 │
     │               │─────────────>│               │                 │
     │               │              │               │                 │
     │               │ setState(    │               │                 │
     │               │ WRITING)     │               │                 │
     │               │─────────────>│               │                 │
     │               │              │               │                 │
     │ HTTP Response │              │               │                 │
     │<──────────────│              │               │                 │
     │               │              │               │                 │
```

---

# 5. PATRONES DE DISEÑO UTILIZADOS

## 5.1 Singleton (SessionManager)

```cpp
class SessionManager {
public:
    static SessionManager& getInstance() {
        static SessionManager instance;  // Meyer's Singleton
        return instance;
    }
    
private:
    SessionManager();  // Constructor privado
    SessionManager(const SessionManager&);  // No copiable
    SessionManager& operator=(const SessionManager&);  // No asignable
};

// Uso:
SessionManager::getInstance().createSession();
```

**Justificación:** Solo debe existir una instancia del gestor de sesiones para mantener consistencia global.

## 5.2 State Pattern (ClientState)

```cpp
enum ClientState {
    CLIENT_READING,     // Estado: Leyendo petición
    CLIENT_PROCESSING,  // Estado: Procesando (o en cola CGI)
    CLIENT_WRITING,     // Estado: Enviando respuesta
    CLIENT_CGI_RUNNING, // Estado: Esperando CGI
    CLIENT_DONE,        // Estado: Completado
    CLIENT_ERROR        // Estado: Error
};
```

**Transiciones de estado:**

```
┌─────────────────────────────────────────────────────────────────────────┐
│                                                                         │
│  READING ──────────────────────────────────────────────────► ERROR     │
│    │                                                           ▲        │
│    │ petición completa                                         │        │
│    │                                                           │        │
│    ▼                                                           │        │
│  PROCESSING ───────────────────────────────────────────────────┤        │
│    │                                                           │        │
│    ├─► CGI_RUNNING ──────────────────────────────────────────►─┤        │
│    │         │                                                 │        │
│    │         │ CGI completado                                  │        │
│    │         │                                                 │        │
│    │         ▼                                                 │        │
│    └────► WRITING ─────────────────────────────────────────────┤        │
│              │                                                 │        │
│              │ buffer enviado                                  │        │
│              │                                                 │        │
│              ├─► DONE (close)                                  │        │
│              │                                                 │        │
│              └─► READING (keep-alive)                          │        │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

## 5.3 Builder Pattern (Response)

```cpp
Response resp;
resp.setStatusCode(HTTP_OK);
resp.setContentType("text/html");
resp.setHeader("X-Custom", "value");
resp.setBody("<html>...</html>");
std::string http = resp.build();
```

## 5.4 Factory Method (Response::make*)

```cpp
// Métodos factory estáticos
Response Response::makeError(int code, const ServerConfig* config);
Response Response::makeRedirect(int code, const std::string& location);
Response Response::makeFile(const std::string& path, const std::string& contentType);
Response Response::makeDirectoryListing(const std::string& path, const std::string& uri);
Response Response::makeFromCGI(const std::string& cgiOutput);
```

---

# 6. ANÁLISIS DE COMPLEJIDAD

## 6.1 Complejidad de Operaciones Principales

| Operación | Complejidad | Notas |
|-----------|-------------|-------|
| `_rebuildPollFds()` | O(n) | n = número de clientes + CGIs |
| `poll()` | O(n) | Donde n = número de fds |
| `_selectServer()` | O(s × m) | s = servidores, m = server_names |
| `findLocation()` | O(l) | l = número de locations |
| `Request::parse()` | O(n) | n = bytes de la petición |
| `_parseMultipartBody()` | O(n × p) | n = body size, p = partes |
| `_cgiQueue` operaciones | O(1) | push_back, front, erase(begin) |

## 6.2 Uso de Memoria

| Componente | Memoria Típica | Máximo |
|------------|----------------|--------|
| Cliente (sin buffer) | ~500 bytes | - |
| Read buffer | Variable | Sin límite explícito |
| Write buffer | Variable | ~100MB (CGI response) |
| CGI output | Variable | MAX_CGI_OUTPUT_SIZE (~200MB) |
| Sesión | ~200 bytes + datos | - |

## 6.3 Cuellos de Botella Potenciales

1. **CGI concurrente limitado**: MAX_CONCURRENT_CGI = 5
   - Mitigación: Cola de CGIs pendientes

2. **Buffers grandes en memoria**: Respuestas CGI de ~100MB
   - Mitigación: Envío incremental con múltiples send()

3. **Rebuild de pollFds cada iteración**: O(n)
   - Mitigación: Aceptable para ≤1024 clientes

4. **Lectura de archivos completa en memoria**:
   - No hay streaming de archivos
   - Problema para archivos muy grandes

---

**Fin de la Documentación - Fase 2**

*La Fase 3 cubrirá: Response.cpp, CGIHandler.cpp, y comparativa detallada con RFCs.*
