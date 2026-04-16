/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 13:36:10 by victor            #+#    #+#             */
/*   Updated: 2026/04/10 20:12:46 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <string>
# include <map>
# include <vector>
# include <unistd.h>
# include <sys/wait.h>
# include <cstdlib>
# include <cstring>
# include "../../inc/http/Request.hpp"
# include "../../inc/config/ServerConfig.hpp"

class CGIHandler {
public:
	CGIHandler();
	CGIHandler(const CGIHandler& other);
	CGIHandler& operator=(const CGIHandler& other);
	~CGIHandler();

	// Setup
	void							setRequest(const Request& request);
	void							setServerConfig(const ServerConfig* config);
	void							setLocationConfig(const LocationConfig* location);
	void							setScriptPath(const std::string& path);
	void							setCgiExecutable(const std::string& executable);
	void							setClientInfo(const std::string& ip, int port);

	// Execution
	bool							execute();
	bool							startExecution(int& fdIn, int& fdOut, pid_t& pid);

	// Results
	const std::string&				getOutput() const;
	int								getExitStatus() const;
	bool							hasError() const;
	const std::string&				getErrorMessage() const;

	// Parse CGI output
	static bool						parseCgiOutput(const std::string& output,
										std::map<std::string, std::string>& headers,
										std::string& body, int& statusCode);

private:
	const Request*					_request;
	const ServerConfig*				_serverConfig;
	const LocationConfig*			_locationConfig;
	std::string						_scriptPath;
	std::string						_cgiExecutable;
	std::string						_clientIp;
	int								_clientPort;
	std::string						_output;
	int								_exitStatus;
	bool							_hasError;
	std::string						_errorMessage;

	// Environment setup
	std::vector<std::string>		_buildEnvironment() const;
	char**							_envToCharArray(const std::vector<std::string>& env) const;
	void							_freeCharArray(char** arr) const;

	// Path handling
	std::string						_getPathInfo() const;
	std::string						_getPathTranslated() const;
	std::string						_getScriptName() const;
};

#endif
