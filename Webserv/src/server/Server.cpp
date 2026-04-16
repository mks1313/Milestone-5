/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/14 12:17:59 by mmarinov          #+#    #+#             */
/*   Updated: 2026/04/10 19:02:24 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server/Server.hpp"
#include "../../inc/webserv.hpp"
#include "../../inc/utils/Utils.hpp"
#include "../../inc/http/MimeTypes.hpp"
#include "../../inc/session/SessionManager.hpp"
#include "../../inc/http/Response.hpp"
#include "../../inc/cgi/CGIHandler.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <poll.h>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <algorithm>

// Global running flag for signal handling
static volatile sig_atomic_t g_serverRunning = 1;

// ============================================================================
// Signal handler
// ============================================================================

void Server::_signalHandler(int sig)
{
    (void)sig;
    g_serverRunning = 0;
}

// ============================================================================
// Constructors / Destructor
// ============================================================================

Server::Server() : _running(false), _activeCgiCount(0)
{
}

Server::Server(const Config& config) : _config(config), _running(false), _activeCgiCount(0)
{
}

Server::Server(const Server& other)
{
    *this = other;
}

Server& Server::operator=(const Server& other)
{
    if (this != &other)
    {
        _config = other._config;
        _running = false;
        _activeCgiCount = 0;
        // Note: we don't copy active connections/sockets
    }
    return *this;
}

Server::~Server()
{
    _closeAllSockets();
}

// ============================================================================
// Initialization
// ============================================================================

bool Server::init()
{
    // Setup signal handlers
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = _signalHandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);

    return _createListenSockets();
}

bool Server::init(const Config& config)
{
    _config = config;
    return init();
}

bool Server::_createListenSockets()
{
    const std::vector<ServerConfig>& servers = _config.getServers();

    if (servers.empty())
    {
        Utils::logError("No servers configured");
        return false;
    }

    std::set<std::pair<std::string, int> > createdSockets;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        const ServerConfig& serverConf = servers[i];
        std::string host = serverConf.getHost();
        int port = serverConf.getPort();

        std::pair<std::string, int> hostPort(host, port);

        // Skip if already created
        if (createdSockets.find(hostPort) != createdSockets.end())
            continue;

        int fd = _createSocket(host, port);
        if (fd < 0)
        {
            Utils::logError("Failed to create socket on " + host + ":" + Utils::intToString(port));
            _closeAllSockets();
            return false;
        }

        _listenSockets[fd] = port;
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

int Server::_createSocket(const std::string& host, int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        Utils::logError("socket() failed: " + std::string(std::strerror(errno)));
        return -1;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        Utils::logError("setsockopt(SO_REUSEADDR) failed");
        close(sockfd);
        return -1;
    }

    // Set non-blocking
    if (!_setNonBlocking(sockfd))
    {
        close(sockfd);
        return -1;
    }

    // Bind
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (host.empty() || host == "0.0.0.0" || host == "*")
        addr.sin_addr.s_addr = INADDR_ANY;
    else
        addr.sin_addr.s_addr = inet_addr(host.c_str());

    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        Utils::logError("bind() failed on port " + Utils::intToString(port) + ": " + std::strerror(errno));
        close(sockfd);
        return -1;
    }

    // Listen
    if (listen(sockfd, BACKLOG) < 0)
    {
        Utils::logError("listen() failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

bool Server::_setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) >= 0;
}

void Server::_closeAllSockets()
{
    // Close listen sockets
    for (std::map<int, int>::iterator it = _listenSockets.begin();
         it != _listenSockets.end(); ++it)
    {
        close(it->first);
    }
    _listenSockets.clear();

    // Close client sockets
    for (std::map<int, Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        close(it->first);
    }
    _clients.clear();

    // Close CGI pipes
    for (std::map<int, int>::iterator it = _cgiToClient.begin();
         it != _cgiToClient.end(); ++it)
    {
        close(it->first);
    }
    _cgiToClient.clear();

    _pollFds.clear();
}

// ============================================================================
// Poll management
// ============================================================================

void Server::_rebuildPollFds()
{
    _pollFds.clear();

    // Add listen sockets
    for (std::map<int, int>::iterator it = _listenSockets.begin();
         it != _listenSockets.end(); ++it)
    {
        struct pollfd pfd;
        pfd.fd = it->first;
        pfd.events = POLLIN;
        pfd.revents = 0;
        _pollFds.push_back(pfd);
    }

    // Add client sockets
    for (std::map<int, Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        struct pollfd pfd;
        pfd.fd = it->first;
        pfd.events = 0;

        Client& client = it->second;
        ClientState state = client.getState();

        if (state == CLIENT_READING)
        {
            pfd.events |= POLLIN;
        }
        if (state == CLIENT_WRITING || client.getWriteBufferSize() > 0)
        {
            pfd.events |= POLLOUT;
        }

        pfd.revents = 0;
        _pollFds.push_back(pfd);
    }

    // Add CGI pipes
    for (std::map<int, int>::iterator it = _cgiToClient.begin();
         it != _cgiToClient.end(); ++it)
    {
        struct pollfd pfd;
        pfd.fd = it->first;
        pfd.events = POLLIN;
        pfd.revents = 0;
        _pollFds.push_back(pfd);
    }
}

void Server::_addToPoll(int fd, short events)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    _pollFds.push_back(pfd);
}

void Server::_removeFromPoll(int fd)
{
    for (size_t i = 0; i < _pollFds.size(); ++i)
    {
        if (_pollFds[i].fd == fd)
        {
            _pollFds.erase(_pollFds.begin() + i);
            return;
        }
    }
}

void Server::_setEvents(int fd, short events)
{
    for (size_t i = 0; i < _pollFds.size(); ++i)
    {
        if (_pollFds[i].fd == fd)
        {
            _pollFds[i].events = events;
            return;
        }
    }
}

// ============================================================================
// Main loop
// ============================================================================

void Server::run()
{
    _running = true;
    g_serverRunning = 1;

    time_t lastCleanup = std::time(NULL);

    Utils::logInfo("Server started, entering main loop");

    while (_running && g_serverRunning)
    {
        _rebuildPollFds();

        if (_pollFds.empty())
        {
            Utils::logError("No file descriptors to poll");
            break;
        }

        int pollResult = poll(&_pollFds[0], _pollFds.size(), 1000);

        if (pollResult < 0)
        {
            if (errno == EINTR)
                continue;
            Utils::logError("poll() failed: " + std::string(std::strerror(errno)));
            break;
        }

        if (pollResult == 0)
        {
            _checkTimeouts();
            continue;
        }

        // Process events
        for (size_t i = 0; i < _pollFds.size() && pollResult > 0; ++i)
        {
            if (_pollFds[i].revents == 0)
                continue;

            --pollResult;
            int fd = _pollFds[i].fd;
            short revents = _pollFds[i].revents;

            // Check for errors
            if (revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                if (_listenSockets.find(fd) != _listenSockets.end())
                {
                    Utils::logError("Error on listen socket");
                    continue;
                }
                else if (_clients.find(fd) != _clients.end())
                {
                    _closeClient(fd);
                    continue;
                }
                else if (_cgiToClient.find(fd) != _cgiToClient.end())
                {
                    _handleCgiRead(fd);
                    continue;
                }
            }

            // Handle listen socket
            if (_listenSockets.find(fd) != _listenSockets.end())
            {
                if (revents & POLLIN)
                    _acceptNewConnection(fd);
            }
            // Handle CGI pipe
            else if (_cgiToClient.find(fd) != _cgiToClient.end())
            {
                if (revents & POLLIN)
                    _handleCgiRead(fd);
            }
            // Handle client socket
            else if (_clients.find(fd) != _clients.end())
            {
                if (revents & POLLIN)
                    _handleClientRead(fd);
                if (revents & POLLOUT)
                    _handleClientWrite(fd);
            }
        }

        _checkTimeouts();

        // Periodic session cleanup
        time_t now = std::time(NULL);
        if (now - lastCleanup >= 60)
        {
            SessionManager::getInstance().cleanExpiredSessions();
            lastCleanup = now;
        }
    }

    Utils::logInfo("Server shutting down");
    _closeAllSockets();
    _running = false;
}

void Server::stop()
{
    _running = false;
    g_serverRunning = 0;
}

bool Server::isRunning() const
{
    return _running;
}

size_t Server::getClientCount() const
{
    return _clients.size();
}

const Config& Server::getConfig() const
{
    return _config;
}

// ============================================================================
// Connection handling
// ============================================================================

void Server::_acceptNewConnection(int listenFd)
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int clientFd = accept(listenFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen);

    if (clientFd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            Utils::logError("accept() failed: " + std::string(std::strerror(errno)));
        return;
    }

    // Check client limit
    if (_clients.size() >= MAX_CLIENTS)
    {
        Utils::logWarning("Maximum clients reached, rejecting connection");
        close(clientFd);
        return;
    }

    // Set non-blocking
    if (!_setNonBlocking(clientFd))
    {
        close(clientFd);
        return;
    }

    // Create client
    std::string clientIp = inet_ntoa(clientAddr.sin_addr);
    int clientPort = _listenSockets[listenFd];
    Client client(clientFd, clientIp, clientPort);

    _clients[clientFd] = client;

    Utils::logDebug("New connection from " + clientIp +
                    " on fd " + Utils::intToString(clientFd));
}

void Server::_handleClientRead(int clientFd) {
    Client& client = _clients[clientFd];

    // Safety check: don't read if we're not in reading state
    if (client.getState() != CLIENT_READING)
        return;

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = recv(clientFd, buffer, BUFFER_SIZE, 0);

    if (bytesRead < 0) {
        Utils::logError("recv error on fd " + Utils::intToString(clientFd)); //new
        _closeClient(clientFd);
        return;
    }
    if (bytesRead == 0) {
        _closeClient(clientFd);
        return;
    }

    client.updateLastActivity();
    std::string data(buffer, bytesRead);

    try {
        // Bucle de Pipelining
        size_t consumed = client.getRequest().parse(data);

        // If parser is in error state, send error response instead of closing
        if (client.getRequest().hasError()) {
            Utils::logError("Parser error on fd " + Utils::intToString(clientFd) +
                           ", error code: " + Utils::intToString(client.getRequest().getErrorCode()));
            int code = client.getRequest().getErrorCode();
            if (code == 0) code = 400;
            _sendErrorResponse(client, code);
            return;
        }

        while (client.getRequest().isComplete()) {
            // Utils::logDebug("Request complete, processing...");
            _processRequest(client);

            // Tras procesar, vemos si queda algo en lo que Request no consumió
            // O si hay más datos en el buffer que Request aún tiene internamente
            if (client.getState() == CLIENT_READING) {
                 // Intentar parsear el remanente si lo hubiera
                 consumed = client.getRequest().parse("");

                 // Check for parser error after attempting to parse remainder
                 if (client.getRequest().hasError()) {
                     Utils::logError("Parser error on fd " + Utils::intToString(clientFd) +
                                    " while parsing pipelined request");
                     int code = client.getRequest().getErrorCode();
                     if (code == 0) code = 400;
                     _sendErrorResponse(client, code);
                     return;
                 }

                 if (consumed == 0 && !client.getRequest().isComplete()) break;
            } else {
                break; // Salir si el cliente cambió de estado (ej: a WRITING o CGI)
            }
        }
    } catch (const std::exception& e) {
        Utils::logError("Exception in _handleClientRead: " + std::string(e.what()));
        _closeClient(clientFd);
    }
}

void Server::_handleClientWrite(int clientFd)
{
    std::map<int, Client>::iterator it = _clients.find(clientFd);
    if (it == _clients.end())
        return;

    Client& client = it->second;

    if (client.getWriteBufferSize() == 0)
    {
        if (!client.shouldKeepAlive())
            _closeClient(clientFd);
        else
            client.reset();
        return;
    }

    std::string& buffer = client.getWriteBuffer();
    size_t bufferSize = buffer.size();

    size_t totalSent = 0;
    int attempts = (bufferSize > 1048576) ? 10 : 1;

    for (int i = 0; i < attempts && bufferSize > 0; ++i)
    {
        ssize_t sent = send(clientFd, buffer.c_str(), bufferSize, 0);

        if (sent < 0)
        {
            // No errno check: si el buffer del kernel está lleno, poll()
            // volverá a notificar POLLOUT cuando esté listo.
            // Si es un error real, poll() notificará POLLERR/POLLHUP
            // y el bucle principal cerrará la conexión.
            break;  // ← antes era _closeClient() + return — ESE era el bug
        }

        if (sent == 0)
            break;

        totalSent += sent;
        client.eraseFromWriteBuffer(sent);
        bufferSize = client.getWriteBufferSize();

        if (bufferSize == 0)
            break;
    }

    if (totalSent > 0)
        client.updateLastActivity();

    if (client.getWriteBufferSize() == 0)
    {
        if (!_cgiQueue.empty())
            _processNextCgiFromQueue();

        if (!client.shouldKeepAlive())
            _closeClient(clientFd);
        else
            client.reset();
    }
}

void Server::_handleCgiRead(int cgiFd)
{
    std::map<int, int>::iterator it = _cgiToClient.find(cgiFd);
    if (it == _cgiToClient.end())
        return;

    int clientFd = it->second;
    std::map<int, Client>::iterator clientIt = _clients.find(clientFd);
    if (clientIt == _clients.end())
    {
        close(cgiFd);
        _cgiToClient.erase(it);
        return;
    }

    Client& client = clientIt->second;

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = read(cgiFd, buffer, sizeof(buffer));

    // Utils::logDebug("CGI read: " + Utils::intToString(bytesRead) + " bytes");

    if (bytesRead > 0)
    {
        try {
            // PROTECTION: Check CGI output size limit to prevent memory exhaustion
            size_t currentSize = client.getCgiOutput().size();
            size_t newSize = currentSize + bytesRead;

            if (newSize > MAX_CGI_OUTPUT_SIZE)
            {
                Utils::logError("CGI output size limit exceeded: " +
                               Utils::sizeTToString(newSize) + " > " +
                               Utils::intToString(MAX_CGI_OUTPUT_SIZE) + " bytes");

                // Close CGI pipe and kill process
                close(cgiFd);
                _cgiToClient.erase(it);

                pid_t pid = client.getCgiPid();
                if (pid > 0)
                {
                    kill(pid, SIGTERM);
                    waitpid(pid, NULL, 0);
                }

                // Decrement active CGI counter
                if (_activeCgiCount > 0)
                {
                    --_activeCgiCount;

                    // Process next queued CGI request if any
                    _processNextCgiFromQueue();
                }

                _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
                return;
            }

            client.getCgiOutput().append(buffer, bytesRead);
            client.updateLastActivity();
        } catch (const std::exception& e) {
            Utils::logError("Exception in _handleCgiRead: " + std::string(e.what()));

            // Decrement active CGI counter before closing
            if (_activeCgiCount > 0)
            {
                --_activeCgiCount;

                // Process next queued CGI request if any
                _processNextCgiFromQueue();
            }

            _closeClient(clientFd);
        }
    }
    else if (bytesRead == 0 || bytesRead < 0)
    {
        if (bytesRead < 0)
        {
            // No errno check: no sabemos si es EAGAIN o error real.
            // Si el pipe aún tiene datos, poll() dará POLLIN de nuevo.
            // Si es un error real, poll() dará POLLERR y el main loop
            // cerrará el cliente. Aquí simplemente salimos sin hacer nada.
            return;  // ← antes caía al bloque "CGI finished" — incorrecto
        }

        // bytesRead == 0: EOF real, el CGI terminó de escribir
        Utils::logDebug("CGI finished, total output: " +
                        Utils::intToString(client.getCgiOutput().size()) + " bytes");

        close(cgiFd);
        _cgiToClient.erase(it);

        pid_t pid = client.getCgiPid();
        if (pid > 0)
        {
            int status;
            int waitRes = waitpid(pid, &status, WNOHANG);

            if (waitRes == 0) {
                Utils::logWarning("CGI process " + Utils::intToString(pid) +
                                  " did not exit, killing");
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
            }

            if (WIFEXITED(status))
                Utils::logDebug("CGI process exited with status: " +
                                Utils::intToString(WEXITSTATUS(status)));
            else if (WIFSIGNALED(status))
                Utils::logDebug("CGI process killed by signal: " +
                                Utils::intToString(WTERMSIG(status)));
        }

        if (_activeCgiCount > 0)
        {
            --_activeCgiCount;
            Utils::logDebug("CGI finished, active CGI count: " +
                            Utils::sizeTToString(_activeCgiCount));
        }

        try {
            _prepareCgiResponse(client);
        } catch (const std::exception& e) {
            Utils::logError("Exception preparing CGI response: " + std::string(e.what()));
            client.getCgiOutput().clear();
            _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        }

        _processNextCgiFromQueue();
    }
}

void Server::_closeClient(int clientFd)
{
    std::map<int, Client>::iterator it = _clients.find(clientFd);
    if (it == _clients.end())
        return;

    Client& client = it->second;

    // Track if client had active CGI
    bool hadActiveCgi = false;

    // Clean up CGI if any
    int cgiFdOut = client.getCgiFdOut();
    if (cgiFdOut >= 0)
    {
        close(cgiFdOut);
        _cgiToClient.erase(cgiFdOut);
        hadActiveCgi = true;
    }

    pid_t pid = client.getCgiPid();
    if (pid > 0)
    {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, WNOHANG);
        hadActiveCgi = true;
    }

    // Decrement active CGI counter if client had CGI running
    if (hadActiveCgi && _activeCgiCount > 0)
    {
        --_activeCgiCount;
        Utils::logDebug("Client with CGI closed, active CGI count: " + Utils::sizeTToString(_activeCgiCount));

        // Process next queued CGI request if any
        _processNextCgiFromQueue();
    }

    Utils::logDebug("Closing connection fd " + Utils::intToString(clientFd));
    close(clientFd);
    _clients.erase(clientFd);
}

void Server::_checkTimeouts()
{
    std::vector<int> toClose;

    for (std::map<int, Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        Client& client = it->second;
        ClientState state = client.getState();

        // Skip timeout check for clients waiting in CGI queue
        if (state == CLIENT_PROCESSING)
        {
            // Don't timeout clients waiting in queue - they'll be processed eventually
            continue;
        }
        // CGI is running - check CGI-specific timeout
        else if (state == CLIENT_CGI_RUNNING && client.hasCgiTimedOut(CGI_TIMEOUT))
        {
            Utils::logDebug("CGI timeout on fd " + Utils::intToString(it->first));
            toClose.push_back(it->first);
        }
        // Client is writing CGI response (potentially 100MB) - use extended timeout
        // Check if either CGI output or write buffer contains large data (>10MB)
        else if (state == CLIENT_WRITING &&
                (client.getCgiOutput().size() > 10485760 || client.getWriteBufferSize() > 10485760))
        {
            if (client.hasTimedOut(CGI_RESPONSE_TIMEOUT))
            {
                Utils::logDebug("CGI response write timeout on fd " + Utils::intToString(it->first));
                toClose.push_back(it->first);
            }
        }
        // Normal timeout for other states
        else if (client.hasTimedOut(CONNECTION_TIMEOUT))
        {
            Utils::logDebug("Connection timeout on fd " + Utils::intToString(it->first));
            toClose.push_back(it->first);
        }
    }

    for (size_t i = 0; i < toClose.size(); ++i)
    {
        _closeClient(toClose[i]);
    }
}

// ============================================================================
// Request processing
// ============================================================================

void Server::_processRequest(Client& client)
{
    const Request& request = client.getRequest();

    Utils::logInfo(request.getMethod() + " " + request.getUri() + " from " + client.getIp());

    // Log body size for debugging
    if (request.getContentLength() > 0)
    {
        Utils::logDebug("Request Content-Length: " + Utils::intToString(request.getContentLength()) +
                       ", actual body size: " + Utils::intToString(request.getBody().size()) + " bytes");
    }

    // Select server configuration
    const ServerConfig* serverConfig = _selectServer(client);
    if (serverConfig == NULL)
    {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    client.setServerConfig(serverConfig);

    // Find matching location
    const LocationConfig* location = serverConfig->findLocation(request.getPath());

    // Check if redirect
    if (location != NULL && location->hasRedirect())
    {
        _handleRedirect(client, location);
        return;
    }

    // Check method allowed
    const std::string& method = request.getMethod();

	// If method not allowed, return 405 Method Not Allowed
	// According to ubuntu_tester expectations (verified via Slack):
	// POST with size 0 to a GET-only location MUST return 405, not 204
	if (location != NULL && !location->isMethodAllowed(method))
	{
		_sendErrorResponse(client, HTTP_METHOD_NOT_ALLOWED);
		return;
	}

    // Check body size - use location's limit if defined, otherwise server's limit
    size_t maxBodySize = serverConfig->getMaxBodySize();
    if (location != NULL && location->getMaxBodySize() > 0)
        maxBodySize = location->getMaxBodySize();

    if (request.getContentLength() > maxBodySize || request.getBody().size() > maxBodySize)
    {
        _sendErrorResponse(client, HTTP_PAYLOAD_TOO_LARGE);
        return;
    }

    // Route by method
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

const ServerConfig* Server::_selectServer(const Client& client)
{
    const Request& request = client.getRequest();
    std::string host = request.getHost();
    int port = client.getPort();

    const std::vector<ServerConfig>& servers = _config.getServers();
    const ServerConfig* defaultServer = NULL;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        const ServerConfig& server = servers[i];

        if (server.getPort() != port)
            continue;

        if (defaultServer == NULL)
            defaultServer = &server;

        // Check server names
        const std::vector<std::string>& names = server.getServerNames();
        for (size_t j = 0; j < names.size(); ++j)
        {
            if (names[j] == host)
                return &server;
        }
    }

    return defaultServer;
}

// ============================================================================
// Response handlers
// ============================================================================

void Server::_handleGet(Client& client, const LocationConfig* location)
{
    const Request& request = client.getRequest();
    const ServerConfig* serverConfig = client.getServerConfig();

    std::string filePath = _resolvePath(request, location, serverConfig);

    // Check if CGI
    if (location != NULL && _isCgiRequest(filePath, location))
    {
        _handleCgi(client, location, filePath);
        return;
    }

    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) < 0)
    {
        _sendErrorResponse(client, HTTP_NOT_FOUND);
        return;
    }

    // Directory handling
    if (S_ISDIR(fileStat.st_mode))
    {
        // Try index files
        std::vector<std::string> indexFiles;
        if (location != NULL && !location->getIndex().empty())
            indexFiles.push_back(location->getIndex());
        if (indexFiles.empty() && serverConfig != NULL && !serverConfig->getIndex().empty())
            indexFiles.push_back(serverConfig->getIndex());
        if (indexFiles.empty())
            indexFiles.push_back("index.html");

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
            // Check if URI has trailing slash
            // Only show autoindex if URI ends with '/'
            std::string uri = request.getPath();
            bool hasTrailingSlash = (!uri.empty() && uri[uri.length() - 1] == '/');

            // Check autoindex
            bool autoindex = (location != NULL) ? location->getAutoindex() : false;

            // Show autoindex only if URI has trailing slash AND autoindex is enabled
            if (hasTrailingSlash && autoindex)
            {
                _handleDirectoryListing(client, filePath, request.getPath());
                return;
            }
            else
            {
                // If directory exists and autoindex is on, but missing trailing slash -> Redirect 301
                if (!hasTrailingSlash && autoindex)
                {
                    Response resp = Response::makeRedirect(301, uri + "/");
                    std::string response = resp.build();
                    client.appendToWriteBuffer(response);
                    client.setState(CLIENT_WRITING);
                    return;
                }
                // No index file and autoindex disabled -> 404 Not Found (Security through obscurity)
                _sendErrorResponse(client, HTTP_NOT_FOUND);
                return;
            }
        }
    }

    // Check file exists and is readable
    if (stat(filePath.c_str(), &fileStat) < 0 || !S_ISREG(fileStat.st_mode))
    {
        _sendErrorResponse(client, HTTP_NOT_FOUND);
        return;
    }

    if (access(filePath.c_str(), R_OK) < 0)
    {
        _sendErrorResponse(client, HTTP_FORBIDDEN);
        return;
    }

    // Read file
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file)
    {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // Build response
    std::string mimeType = MimeTypes::getInstance().getMimeTypeByFile(filePath);
    Response resp;
    resp.setStatusCode(HTTP_OK);
    resp.setContentType(mimeType);
    resp.setBody(content);

    // build(true) si es HEAD para enviar solo headers
    std::string response = resp.build(request.getMethod() == "HEAD");

    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}

void Server::_handlePost(Client& client, const LocationConfig* location)
{
    const Request& request = client.getRequest();
    const ServerConfig* serverConfig = client.getServerConfig();

    std::string filePath = _resolvePath(request, location, serverConfig);

    // Check if CGI
    if (location != NULL && _isCgiRequest(filePath, location))
    {
        _handleCgi(client, location, filePath);
        return;
    }

    // Handle file upload
    if (location != NULL && location->getUploadEnabled())
    {
        _handleFileUpload(client, location);
        return;
    }

    // Check if resource exists (and is not CGI/Upload)
    struct stat st;
    if (stat(filePath.c_str(), &st) == 0)
    {
        // If directory, check for index CGI
        if (S_ISDIR(st.st_mode))
        {
            std::vector<std::string> indexFiles;
            if (location != NULL && !location->getIndex().empty())
                indexFiles.push_back(location->getIndex());
            if (indexFiles.empty() && serverConfig != NULL && !serverConfig->getIndex().empty())
                indexFiles.push_back(serverConfig->getIndex());
            if (indexFiles.empty())
                indexFiles.push_back("index.html");

            for (size_t i = 0; i < indexFiles.size(); ++i)
            {
                std::string indexPath = filePath;
                if (indexPath[indexPath.length() - 1] != '/')
                    indexPath += '/';
                indexPath += indexFiles[i];

                struct stat indexSt;
                if (stat(indexPath.c_str(), &indexSt) == 0 && S_ISREG(indexSt.st_mode))
                {
                    if (location != NULL && _isCgiRequest(indexPath, location))
                    {
                        _handleCgi(client, location, indexPath);
                        return;
                    }
                    break;
                }
            }
        }

        _sendErrorResponse(client, HTTP_METHOD_NOT_ALLOWED);
        return;
    }

    // If resource does not exist (e.g. /post_body), return 204 No Content
    Response resp;
    resp.setStatusCode(HTTP_NO_CONTENT);
    std::string response = resp.build();
    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}

void Server::_handlePut(Client& client, const LocationConfig* location)
{
    const Request& request = client.getRequest();
    const ServerConfig* serverConfig = client.getServerConfig();

    // Resolve file path
    std::string filePath = _resolvePath(request, location, serverConfig);

    // Check if path is a directory
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode))
    {
        _sendErrorResponse(client, HTTP_FORBIDDEN);
        return;
    }

    // Determine if file exists (for status code 200 vs 201)
    bool fileExists = (access(filePath.c_str(), F_OK) == 0);

    // Create parent directories if they don't exist
    size_t lastSlash = filePath.rfind('/');
    if (lastSlash != std::string::npos && lastSlash > 0)
    {
        std::string directory = filePath.substr(0, lastSlash);
        if (!directory.empty())
        {
            // Create directory recursively using mkdir -p
            std::string cmd = "mkdir -p \"" + directory + "\" 2>/dev/null";
            int ret = system(cmd.c_str());
            (void)ret; // Suppress unused variable warning
        }
    }

    // Write body content to file
    std::ofstream file(filePath.c_str(), std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    const std::string& body = request.getBody();
    if (!body.empty())
    {
        file.write(body.c_str(), body.length());
    }
    file.close();

    if (file.fail())
    {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    // Return 201 if file was created, 200 if file was modified
    Response resp;
    resp.setStatusCode(fileExists ? HTTP_OK : HTTP_CREATED);
    resp.setHeader("Content-Length", "0");
    std::string response = resp.build();
    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}

void Server::_handleDelete(Client& client, const LocationConfig* location)
{
    const Request& request = client.getRequest();
    const ServerConfig* serverConfig = client.getServerConfig();

    std::string filePath = _resolvePath(request, location, serverConfig);

    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) < 0)
    {
        _sendErrorResponse(client, HTTP_NOT_FOUND);
        return;
    }

    if (!S_ISREG(fileStat.st_mode))
    {
        _sendErrorResponse(client, HTTP_FORBIDDEN);
        return;
    }

    if (std::remove(filePath.c_str()) != 0)
    {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    Response resp;
    resp.setStatusCode(HTTP_NO_CONTENT);
    std::string response = resp.build();
    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}

void Server::_handleCgi(Client& client, const LocationConfig* location,
                        const std::string& filePath)
{
    const Request& request = client.getRequest();
    const ServerConfig* serverConfig = client.getServerConfig();

    // Find CGI handler
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

    // CRITICAL: Check if CGI handler (executable) exists and is executable
    if (access(cgiPath.c_str(), X_OK) < 0)
    {
        Utils::logError("CGI handler not found or not executable: " + cgiPath);
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    // PROTECTION: Limit concurrent CGI processes to prevent resource exhaustion
    // If limit reached, queue the request instead of rejecting it
    if (_activeCgiCount >= MAX_CONCURRENT_CGI)
    {
        Utils::logDebug("Maximum concurrent CGI processes reached (" +
                       Utils::sizeTToString(_activeCgiCount) + "/" +
                       Utils::intToString(MAX_CONCURRENT_CGI) + "), queuing request for fd " +
                       Utils::intToString(client.getFd()));

        // Add client to queue and change state to prevent timeout
        _cgiQueue.push_back(client.getFd());
        client.setState(CLIENT_PROCESSING); // Prevent timeout while waiting
        client.updateLastActivity();
        return;
    }

    // Convert CGI path to absolute path BEFORE chdir
    // This is critical because we'll change directory to the script's directory
    std::string absoluteCgiPath;
    if (cgiPath[0] == '/')
    {
        // Already absolute
        absoluteCgiPath = cgiPath;
    }
    else
    {
        // Relative path - convert to absolute
        char currentDir[PATH_MAX];
        if (getcwd(currentDir, sizeof(currentDir)) != NULL)
        {
            absoluteCgiPath = std::string(currentDir) + "/" + cgiPath;
        }
        else
        {
            Utils::logError("Failed to get current directory for CGI");
            _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
            return;
        }
    }

    // Check script exists and is readable
    if (access(filePath.c_str(), R_OK) < 0)
    {
        // For 42 tester compatibility, we must execute CGI even if the script file doesn't exist
        Utils::logWarning("CGI script not found: " + filePath + ", proceeding anyway");
    }

    // Create pipe for CGI output and tmpfile for input
    // Using tmpfile() for stdin avoids pipe deadlock issues with large bodies (100MB+)
    int pipeOut[2];

    if (pipe(pipeOut) < 0)
    {
        Utils::logError("Failed to create pipe for CGI");
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    // Create temporary file for CGI stdin (better than pipe for large data)
    FILE* tmpfp = tmpfile();
    if (tmpfp == NULL)
    {
        close(pipeOut[0]);
        close(pipeOut[1]);
        Utils::logError("Failed to create tmpfile for CGI");
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    int tmpfd = fileno(tmpfp);

    // Write request body to temp file
    const std::string& body = request.getBody();

    Utils::logDebug("CGI request body size: " + Utils::intToString(body.size()) + " bytes");

    if (!body.empty())
    {
        size_t written = fwrite(body.c_str(), 1, body.size(), tmpfp);
        if (written != body.size())
        {
            Utils::logError("Failed to write body to tmpfile: wrote " +
                          Utils::sizeTToString(written) + " of " +
                          Utils::sizeTToString(body.size()));
            fclose(tmpfp);
            close(pipeOut[0]);
            close(pipeOut[1]);
            _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
            return;
        }

        Utils::logDebug("Wrote " + Utils::sizeTToString(written) + " bytes to tmpfile");

        fflush(tmpfp);
        // Rewind to beginning for CGI to read
        lseek(tmpfd, 0, SEEK_SET);

        Utils::logDebug("Tmpfile rewound to start, fd=" + Utils::intToString(tmpfd));
    }

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
        // Child process
        close(pipeOut[0]);

        dup2(tmpfd, STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);

        close(pipeOut[1]);

        // Change to script directory
        std::string scriptDir = filePath.substr(0, filePath.rfind('/'));
        std::string scriptName = filePath.substr(filePath.rfind('/') + 1);

        // Get absolute path of script before chdir
        char absoluteScriptPath[PATH_MAX];
        if (realpath(filePath.c_str(), absoluteScriptPath) == NULL)
        {
            // If realpath fails (e.g. file doesn't exist), construct absolute path manually
            char currentDir[PATH_MAX];
            if (getcwd(currentDir, sizeof(currentDir)) != NULL)
            {
                std::string absPath = std::string(currentDir) + "/" + filePath;
                std::strncpy(absoluteScriptPath, absPath.c_str(), PATH_MAX - 1);
                absoluteScriptPath[PATH_MAX - 1] = '\0';
            }
            else
            {
                Utils::logError("Failed to get absolute path for script: " + filePath);
                _exit(1);
            }
        }

        if (!scriptDir.empty())
            chdir(scriptDir.c_str());

        // PATH_INFO: For 42's ubuntu_cgi_tester, PATH_INFO should be the full request path
        // This is NON-STANDARD but required by 42's tester
        // Standard CGI would use PATH_INFO for the portion AFTER the script
        // But 42 requires: PATH_INFO = SCRIPT_NAME = REQUEST_URI = full request path
        std::string pathInfo = request.getPath();  // Full path like /directory/youpi.bla

        // Build environment
        std::vector<std::string> envStrings;
        envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
        envStrings.push_back("SERVER_PROTOCOL=" + request.getVersion());
        envStrings.push_back("SERVER_SOFTWARE=webserv/1.0");
        envStrings.push_back("REQUEST_METHOD=" + request.getMethod());
        envStrings.push_back("REDIRECT_STATUS=200");

        // SCRIPT_FILENAME must be absolute path
        envStrings.push_back("SCRIPT_FILENAME=" + std::string(absoluteScriptPath));
        envStrings.push_back("SCRIPT_NAME=" + request.getPath());
        envStrings.push_back("PATH_INFO=" + pathInfo);  // Same as SCRIPT_NAME for 42

        // REQUEST_URI is required by ubuntu_cgi_tester (not in RFC 3875)
        envStrings.push_back("REQUEST_URI=" + request.getPath());

        envStrings.push_back("QUERY_STRING=" + request.getQuery());

        std::string contentType = request.getHeader("Content-Type");
        if (!contentType.empty())
            envStrings.push_back("CONTENT_TYPE=" + contentType);

        if (request.getContentLength() > 0)
            envStrings.push_back("CONTENT_LENGTH=" + Utils::intToString(request.getContentLength()));
        else
            envStrings.push_back("CONTENT_LENGTH=0");

        if (serverConfig != NULL)
        {
            envStrings.push_back("SERVER_NAME=" + serverConfig->getServerNames()[0]);
            envStrings.push_back("SERVER_PORT=" + Utils::intToString(serverConfig->getPort()));
        }

        envStrings.push_back("REMOTE_ADDR=" + client.getIp());
        envStrings.push_back("REMOTE_HOST=" + client.getIp());  // Same as REMOTE_ADDR for simplicity

        // Add HTTP headers as HTTP_* environment variables
        const std::map<std::string, std::string>& headers = request.getHeaders();
        for (std::map<std::string, std::string>::const_iterator it = headers.begin();
             it != headers.end(); ++it)
        {
            std::string envName = "HTTP_" + it->first;
            for (size_t i = 0; i < envName.length(); ++i)
            {
                if (envName[i] == '-')
                    envName[i] = '_';
                else
                    envName[i] = std::toupper(envName[i]);
            }
            envStrings.push_back(envName + "=" + it->second);
        }

        // Log all CGI environment variables for debugging
        // Utils::logDebug("CGI Environment Variables:");
        // for (size_t i = 0; i < envStrings.size(); ++i)
        //     Utils::logDebug("  " + envStrings[i]);

        std::vector<char*> env;
        for (size_t i = 0; i < envStrings.size(); ++i)
            env.push_back(const_cast<char*>(envStrings[i].c_str()));
        env.push_back(NULL);

        // Build args - Pass script name (relative to cwd after chdir)
        // After chdir to script directory, script name is just the filename
        char* args[3];
        args[0] = const_cast<char*>(absoluteCgiPath.c_str());
        args[1] = const_cast<char*>(scriptName.c_str());  // Relative: just filename
        args[2] = NULL;

        execve(absoluteCgiPath.c_str(), args, &env[0]);

        // If we get here, execve failed
        Utils::logError("execve failed for CGI: " + absoluteCgiPath);
        _exit(1);
    }

    // Parent process
    close(pipeOut[1]);

    // Close tmpfile - child has its own file descriptor after fork
    fclose(tmpfp);

    // Set up for reading CGI output
    if (!_setNonBlocking(pipeOut[0]))
    {
        close(pipeOut[0]);
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        Utils::logError("Failed to set non-blocking on CGI pipe");
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    _cgiToClient[pipeOut[0]] = client.getFd();
    client.setCgiPid(pid);
    client.setCgiFdOut(pipeOut[0]);
    client.setCgiStartTime(std::time(NULL));
    client.setState(CLIENT_CGI_RUNNING);
    client.getCgiOutput().clear(); // Ensure buffer is clean before starting

    // Increment active CGI counter
    ++_activeCgiCount;

    Utils::logDebug("CGI started: " + absoluteCgiPath + " for script: " + filePath +
                   " (active CGI: " + Utils::sizeTToString(_activeCgiCount) + ")");
}

void Server::_handleRedirect(Client& client, const LocationConfig* location)
{
    int code = location->getRedirectCode();
    std::string url = location->getRedirect();

    Response resp = Response::makeRedirect(code, url);
    std::string response = resp.build();
    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}

void Server::_handleDirectoryListing(Client& client, const std::string& path,
                                     const std::string& uri)
{
    Response resp = Response::makeDirectoryListing(path, uri);
    std::string response = resp.build();

    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}

void Server::_handleFileUpload(Client& client, const LocationConfig* location)
{
    const Request& request = client.getRequest();

    std::string uploadPath = location->getUploadPath();
    if (uploadPath.empty())
        uploadPath = "/tmp/uploads";

    // Create upload directory if needed
    mkdir(uploadPath.c_str(), 0755);

    const std::vector<UploadedFile>& files = request.getUploadedFiles();

    if (files.empty())
    {
        // No multipart, save raw body
        std::string filename = "upload_" + Utils::intToString(std::time(NULL));
        std::string fullPath = uploadPath + "/" + filename;

        std::ofstream file(fullPath.c_str(), std::ios::binary);
        if (!file)
        {
            _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
            return;
        }
        file << request.getBody();
        file.close();
    }
    else
    {
        // Save each uploaded file
        for (size_t i = 0; i < files.size(); ++i)
        {
            std::string filename = files[i].filename;
            if (filename.empty())
                filename = "upload_" + Utils::intToString(std::time(NULL)) + "_" + Utils::intToString(i);

            // Sanitize filename - remove path separators and special characters
            std::string cleanFilename;
            for (size_t j = 0; j < filename.length(); ++j)
            {
                char c = filename[j];
                if (c != '/' && c != '\\' && c != ':' && c != '*' && c != '?' &&
                    c != '"' && c != '<' && c != '>' && c != '|')
                    cleanFilename += c;
            }
            filename = cleanFilename;

            std::string fullPath = uploadPath + "/" + filename;

            std::ofstream file(fullPath.c_str(), std::ios::binary);
            if (!file)
            {
                _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
                return;
            }
            file << files[i].data;
            file.close();
        }
    }

    Response resp;
    resp.setStatusCode(HTTP_CREATED);
    std::string response = resp.build();
    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}

// ============================================================================
// Helper methods
// ============================================================================

std::string Server::_resolvePath(const Request& request, const LocationConfig* location,
                                 const ServerConfig* server)
{
    std::string uri = request.getPath();
    std::string root;

    if (location != NULL && !location->getRoot().empty())
        root = location->getRoot();
    else if (server != NULL)
        root = server->getRoot();
    else
        root = "./www";

    // Handle alias
    if (location != NULL && !location->getAlias().empty())
    {
        std::string locPath = location->getPath();
        if (uri.find(locPath) == 0)
            uri = uri.substr(locPath.length());
        return location->getAlias() + uri;
    }

    // Normal path resolution
    std::string path = root + uri;

    // Normalize (remove .. etc)
    path = Utils::normalizePath(path);

    return path;
}

void Server::_sendResponse(Client& client)
{
    // Response already in write buffer
    client.setState(CLIENT_WRITING);
}

void Server::_sendErrorResponse(Client& client, int code) {
    Response resp = Response::makeError(code, client.getServerConfig());

    if (code == HTTP_METHOD_NOT_ALLOWED) {
        resp.setHeader("Allow", "GET, HEAD");
    }

    // Determinar si cerramos la conexión
    bool keepAlive = client.shouldKeepAlive();
    if (code == HTTP_BAD_REQUEST || code >= 500) {
        keepAlive = false;
    }
    client.setKeepAlive(keepAlive);
    resp.setHeader("Connection", keepAlive ? "keep-alive" : "close");

    // Construir respuesta respetando HEAD (si existe el objeto Request)
    bool isHead = (client.getRequest().getMethod() == "HEAD");
    std::string responseStr = resp.build(isHead);

    // IMPORTANTE: Usar el buffer de escritura del cliente para ser 100% no bloqueante
    client.appendToWriteBuffer(responseStr);
    client.setState(CLIENT_WRITING);

    // Eliminamos el send() y el _closeClient() de aquí,
    // el bucle principal lo manejará a través de POLLOUT.
}

void Server::_prepareCgiResponse(Client& client) {
    std::string& output = client.getCgiOutput();
    if (output.empty()) {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    Utils::logDebug("CGI total output size: " + Utils::intToString(output.size()) + " bytes");

    // Log first 200 chars for debugging
    std::string preview = output.substr(0, output.size() < 200 ? output.size() : 200);
    Utils::logDebug("CGI output preview: " + preview);

    Response resp = Response::makeFromCGI(output);

    // OPTIMIZATION: Clear output buffer immediately to reduce peak memory usage
    // We already have the data in the Response object
    {
        std::string empty;
        client.getCgiOutput().swap(empty); // Force memory release
    }

    // Set Connection header based on keep-alive status
    bool keepAlive = client.shouldKeepAlive();
    client.setKeepAlive(keepAlive);
    resp.setHeader("Connection", keepAlive ? "keep-alive" : "close");

    // Aplicar el fix de HEAD aquí también
    bool isHead = (client.getRequest().getMethod() == "HEAD");
    std::string response = resp.build(isHead);

    Utils::logDebug("Final response size: " + Utils::intToString(response.size()) + " bytes");

    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);

    // FIX: Update activity to prevent immediate timeout and clear CGI buffer to save memory
    client.updateLastActivity();
}

bool Server::_isCgiRequest(const std::string& path, const LocationConfig* location)
{
    if (location == NULL)
        return false;

    size_t dotPos = path.rfind('.');
    if (dotPos == std::string::npos)
        return false;

    std::string extension = path.substr(dotPos);
    return !location->getCgiHandler(extension).empty();
}

void Server::_processNextCgiFromQueue()
{
    // Process queued CGI requests if there's available capacity
    while (!_cgiQueue.empty() && _activeCgiCount < MAX_CONCURRENT_CGI)
    {
        // CRITICAL: Calculate total memory used by write buffers to prevent OOM
        // We need to ensure we have memory available for the new CGI output
        size_t totalWriteBufferSize = 0;
        size_t clientsWithLargeBuffers = 0;

        for (std::map<int, Client>::iterator cit = _clients.begin(); cit != _clients.end(); ++cit)
        {
            size_t writeSize = cit->second.getWriteBufferSize();
            totalWriteBufferSize += writeSize;

            // Count clients with >10MB in write buffer (still sending large responses)
            if (writeSize > 10485760) // 10MB
                ++clientsWithLargeBuffers;
        }

        // PROTECTION: Don't start new CGI if we already have too much data in write buffers
        // Each CGI will produce ~100MB output that goes to write buffer
        // If we already have 3+ clients with large buffers, wait for them to finish sending
        if (clientsWithLargeBuffers >= 3)
        {
            Utils::logDebug("Delaying queue processing: " + Utils::sizeTToString(clientsWithLargeBuffers) +
                           " clients with large write buffers (" + Utils::sizeTToString(totalWriteBufferSize) +
                           " bytes total)");
            break; // Don't process queue yet, wait for write buffers to drain
        }

        int clientFd = _cgiQueue.front();
        _cgiQueue.erase(_cgiQueue.begin());

        // Check if client still exists
        std::map<int, Client>::iterator it = _clients.find(clientFd);
        if (it == _clients.end())
        {
            Utils::logDebug("Queued client fd " + Utils::intToString(clientFd) + " no longer exists, skipping");
            continue;
        }

        Client& client = it->second;
        const Request& request = client.getRequest();
        const ServerConfig* serverConfig = client.getServerConfig();

        Utils::logDebug("Processing queued CGI request for fd " + Utils::intToString(clientFd) +
                       " (total write buffers: " + Utils::sizeTToString(totalWriteBufferSize) + " bytes)");

        // Find matching location
        const LocationConfig* location = serverConfig->findLocation(request.getPath());
        std::string filePath = _resolvePath(request, location, serverConfig);

        // Retry CGI execution (this time it should succeed since we have capacity)
        _handleCgi(client, location, filePath);
    }
}
