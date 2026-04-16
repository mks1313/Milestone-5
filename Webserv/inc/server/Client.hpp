/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 00:00:00 by mmarinov          #+#    #+#             */
/*   Updated: 2026/04/10 18:29:33 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <ctime>
# include "../../inc/http/Request.hpp"
# include "../../inc/http/Response.hpp"
# include "../../inc/config/ServerConfig.hpp"

enum ClientState {
	CLIENT_READING,
	CLIENT_PROCESSING,
	CLIENT_WRITING,
	CLIENT_CGI_RUNNING,
	CLIENT_DONE,
	CLIENT_ERROR
};

class Client {
public:
	Client();
	Client(int fd, const std::string& ip, int port);
	Client(const Client& other);
	Client& operator=(const Client& other);
	~Client();

	// Socket operations
	int								getFd() const;
	const std::string&				getIp() const;
	int								getPort() const;
	void							setFd(int fd);

	// Request/Response
	Request&						getRequest();
	const Request&					getRequest() const;
	Response&						getResponse();
	const Response&					getResponse() const;
	void							setResponse(const Response& response);

	// Server config
	void							setServerConfig(const ServerConfig* config);
	const ServerConfig*				getServerConfig() const;

	// State management
	ClientState						getState() const;
	void							setState(ClientState state);
	bool							isReadyToRead() const;
	bool							isReadyToWrite() const;
	bool							isDone() const;
	bool							hasError() const;

	// Timeout management
	void							updateLastActivity();
	time_t							getLastActivity() const;
	bool							hasTimedOut(int timeout) const;

	// Buffer management
	std::string&					getReadBuffer();
	std::string&					getWriteBuffer();
	void							appendToReadBuffer(const char* data, size_t len);
	void							appendToWriteBuffer(const std::string& data);
	void							clearReadBuffer();
	void							clearWriteBuffer();
	size_t							getWriteBufferSize() const;
	void							eraseFromWriteBuffer(size_t len);

	// CGI support
	void							setCgiPid(pid_t pid);
	pid_t							getCgiPid() const;
	void							setCgiFdIn(int fd);
	void							setCgiFdOut(int fd);
	int								getCgiFdIn() const;
	int								getCgiFdOut() const;
	std::string&					getCgiOutput();
	void							appendCgiOutput(const std::string& data);
	time_t							getCgiStartTime() const;
	void							setCgiStartTime(time_t time);
	bool							hasCgiTimedOut(int timeout) const;

	// Keep-alive support
	bool							shouldKeepAlive() const;
	void							setKeepAlive(bool keepAlive);
	int								getRequestCount() const;
	void							incrementRequestCount();
	void							reset();

private:
	int								_fd;
	std::string						_ip;
	int								_port;
	Request							_request;
	Response						_response;
	const ServerConfig*				_serverConfig;
	ClientState						_state;
	time_t							_lastActivity;
	std::string						_readBuffer;
	std::string						_writeBuffer;

	// CGI related
	pid_t							_cgiPid;
	int								_cgiFdIn;
	int								_cgiFdOut;
	std::string						_cgiOutput;
	time_t							_cgiStartTime;

	// Keep-alive
	bool							_keepAlive;
	int								_requestCount;
};

#endif
