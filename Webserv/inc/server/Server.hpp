/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/14 12:17:32 by mmarinov          #+#    #+#             */
/*   Updated: 2026/04/10 19:55:44 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include <map>
# include <set>
# include <poll.h>
# include "../../inc/config/Config.hpp"
# include "../../inc/server/Client.hpp"
# include "../../inc/cgi/CGIHandler.hpp"

class Server {
public:
	Server();
	Server(const Config& config);
	Server(const Server& other);
	Server& operator=(const Server& other);
	~Server();

	// Initialization
	bool							init();
	bool							init(const Config& config);

	// Main loop
	void							run();
	void							stop();

	// Status
	bool							isRunning() const;
	size_t							getClientCount() const;

	// Configuration
	const Config&					getConfig() const;

private:
	Config							_config;
	bool							_running;
	std::vector<struct pollfd>		_pollFds;
	std::map<int, Client>			_clients;
	std::map<int, int>				_listenSockets;  // fd -> port mapping
	std::map<int, int>				_cgiToClient;    // cgi fd -> client fd mapping
	size_t							_activeCgiCount; // Track concurrent CGI processes
	std::vector<int>				_cgiQueue;       // Queue of client FDs waiting for CGI slot

	// Socket management
	bool							_createListenSockets();
	int								_createSocket(const std::string& host, int port);
	void							_closeAllSockets();
	bool							_setNonBlocking(int fd);

	// Poll management
	void							_rebuildPollFds();
	void							_addToPoll(int fd, short events);
	void							_removeFromPoll(int fd);
	void							_setEvents(int fd, short events);

	// Connection handling
	void							_acceptNewConnection(int listenFd);
	void							_handleClientRead(int clientFd);
	void							_handleClientWrite(int clientFd);
	void							_handleCgiRead(int cgiFd);
	void							_closeClient(int clientFd);
	void							_checkTimeouts();

	// Request processing
	void							_processRequest(Client& client);
	const ServerConfig*				_selectServer(const Client& client);

	// Response handlers
	void							_handleGet(Client& client, const LocationConfig* location);
	void							_handlePost(Client& client, const LocationConfig* location);
	void							_handlePut(Client& client, const LocationConfig* location);
	void							_handleDelete(Client& client, const LocationConfig* location);
	void							_handleCgi(Client& client, const LocationConfig* location,
										const std::string& filePath);
	void							_handleRedirect(Client& client, const LocationConfig* location);
	void							_handleDirectoryListing(Client& client, const std::string& path,
										const std::string& uri);
	void							_handleFileUpload(Client& client, const LocationConfig* location);

	// Helper methods
	std::string						_resolvePath(const Request& request, const LocationConfig* location,
										const ServerConfig* server);
	void							_sendResponse(Client& client);
	void							_sendErrorResponse(Client& client, int code);
	void							_prepareCgiResponse(Client& client);
	bool							_isCgiRequest(const std::string& path, const LocationConfig* location);
	void							_processNextCgiFromQueue(); // Process next queued CGI request

	// Static helpers
	static void						_signalHandler(int sig);
};

#endif
