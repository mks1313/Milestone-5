/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 22:52:55 by fcela-ga          #+#    #+#             */
/*   Updated: 2026/04/10 20:14:09 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/cgi/CGIHandler.hpp"
#include "../../inc/utils/Utils.hpp"
#include <fcntl.h>
#include <signal.h>

CGIHandler::CGIHandler()
	: _request(NULL)
	, _serverConfig(NULL)
	, _locationConfig(NULL)
	, _clientPort(0)
	, _exitStatus(0)
	, _hasError(false)
{
}

CGIHandler::CGIHandler(const CGIHandler& other)
	: _request(other._request)
	, _serverConfig(other._serverConfig)
	, _locationConfig(other._locationConfig)
	, _scriptPath(other._scriptPath)
	, _cgiExecutable(other._cgiExecutable)
	, _clientIp(other._clientIp)
	, _clientPort(other._clientPort)
	, _output(other._output)
	, _exitStatus(other._exitStatus)
	, _hasError(other._hasError)
	, _errorMessage(other._errorMessage)
{
}

CGIHandler& CGIHandler::operator=(const CGIHandler& other) {
	if (this != &other) {
		_request = other._request;
		_serverConfig = other._serverConfig;
		_locationConfig = other._locationConfig;
		_scriptPath = other._scriptPath;
		_cgiExecutable = other._cgiExecutable;
		_clientIp = other._clientIp;
		_clientPort = other._clientPort;
		_output = other._output;
		_exitStatus = other._exitStatus;
		_hasError = other._hasError;
		_errorMessage = other._errorMessage;
	}
	return *this;
}

CGIHandler::~CGIHandler() {
}

// ============================================================================
// Setup
// ============================================================================

void CGIHandler::setRequest(const Request& request) {
	_request = &request;
}

void CGIHandler::setServerConfig(const ServerConfig* config) {
	_serverConfig = config;
}

void CGIHandler::setLocationConfig(const LocationConfig* location) {
	_locationConfig = location;
}

void CGIHandler::setScriptPath(const std::string& path) {
	_scriptPath = path;
}

void CGIHandler::setCgiExecutable(const std::string& executable) {
	_cgiExecutable = executable;
}

void CGIHandler::setClientInfo(const std::string& ip, int port) {
	_clientIp = ip;
	_clientPort = port;
}

// ============================================================================
// Execution
// ============================================================================

bool CGIHandler::execute() {
	int fdIn, fdOut;
	pid_t pid;

	if (!startExecution(fdIn, fdOut, pid))
		return false;

	// Write request body to CGI
	if (!_request->getBody().empty()) {
		write(fdIn, _request->getBody().c_str(), _request->getBody().length());
	}
	close(fdIn);

	// Read CGI output
	char buffer[4096];
	ssize_t bytesRead;
	while ((bytesRead = read(fdOut, buffer, sizeof(buffer))) > 0) {
		_output.append(buffer, bytesRead);
	}
	close(fdOut);

	// Wait for child process
	int status;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status)) {
		_exitStatus = WEXITSTATUS(status);
	} else {
		_hasError = true;
		_errorMessage = "CGI process terminated abnormally";
		return false;
	}

	return true;
}

bool CGIHandler::startExecution(int& fdIn, int& fdOut, pid_t& pid) {
	int pipeIn[2];
	int pipeOut[2];

	if (pipe(pipeIn) < 0) {
		_hasError = true;
		_errorMessage = "Failed to create input pipe";
		return false;
	}

	if (pipe(pipeOut) < 0) {
		close(pipeIn[0]);
		close(pipeIn[1]);
		_hasError = true;
		_errorMessage = "Failed to create output pipe";
		return false;
	}

	pid = fork();
	if (pid < 0) {
		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);
		_hasError = true;
		_errorMessage = "Failed to fork";
		return false;
	}

	if (pid == 0) {
		// Child process
		close(pipeIn[1]);
		close(pipeOut[0]);

		// Redirect stdin and stdout
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);

		close(pipeIn[0]);
		close(pipeOut[1]);

		// Change to script directory for relative path access
		std::string scriptDir = Utils::getDirectory(_scriptPath);
		if (!scriptDir.empty() && scriptDir != ".") {
			if (chdir(scriptDir.c_str()) != 0) {
				std::cerr << "CGI Error: Failed to change directory to " << scriptDir << std::endl;
				exit(1);
			}
		}

		// Build environment
		std::vector<std::string> envVec = _buildEnvironment();
		char** env = _envToCharArray(envVec);

		// Build arguments
		std::string scriptName = Utils::getFileName(_scriptPath);
		char* argv[3];
		argv[0] = const_cast<char*>(_cgiExecutable.c_str());
		argv[1] = const_cast<char*>(scriptName.c_str());
		argv[2] = NULL;

		// Execute
		execve(_cgiExecutable.c_str(), argv, env);

		// If execve returns, there was an error
		std::cerr << "CGI Error: Failed to execute " << _cgiExecutable << std::endl;
		_freeCharArray(env);
		exit(1);
	}

	// Parent process
	close(pipeIn[0]);
	close(pipeOut[1]);

	fdIn = pipeIn[1];
	fdOut = pipeOut[0];

	// Set non-blocking
	fcntl(fdIn, F_SETFL, O_NONBLOCK);
	fcntl(fdOut, F_SETFL, O_NONBLOCK);

	return true;
}

// ============================================================================
// Results
// ============================================================================

const std::string& CGIHandler::getOutput() const {
	return _output;
}

int CGIHandler::getExitStatus() const {
	return _exitStatus;
}

bool CGIHandler::hasError() const {
	return _hasError;
}

const std::string& CGIHandler::getErrorMessage() const {
	return _errorMessage;
}

// ============================================================================
// Static methods
// ============================================================================

bool CGIHandler::parseCgiOutput(const std::string& output,
								std::map<std::string, std::string>& headers,
								std::string& body, int& statusCode) {
	statusCode = 200;

	// Find end of headers
	size_t headerEnd = output.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		headerEnd = output.find("\n\n");

	if (headerEnd == std::string::npos) {
		// No headers, treat all as body
		body = output;
		return true;
	}

	std::string headerPart = output.substr(0, headerEnd);
	size_t bodyStart = headerEnd + (output[headerEnd] == '\r' ? 4 : 2);
	body = output.substr(bodyStart);

	// Parse headers
	std::istringstream stream(headerPart);
	std::string line;
	while (std::getline(stream, line)) {
		// Remove trailing \r
		if (!line.empty() && line[line.length() - 1] == '\r')
			line = line.substr(0, line.length() - 1);

		if (line.empty())
			continue;

		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos) {
			std::string name = Utils::trim(line.substr(0, colonPos));
			std::string value = Utils::trim(line.substr(colonPos + 1));

			if (Utils::toLower(name) == "status") {
				// Parse status: "200 OK" or just "200"
				std::istringstream statusStream(value);
				statusStream >> statusCode;
			} else {
				headers[name] = value;
			}
		}
	}

	return true;
}

// ============================================================================
// Private methods
// ============================================================================

std::vector<std::string> CGIHandler::_buildEnvironment() const {
	std::vector<std::string> env;

	// Required CGI environment variables
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_PROTOCOL=" + _request->getVersion());
	env.push_back("SERVER_SOFTWARE=Webserv/1.0");
	env.push_back("REQUEST_METHOD=" + _request->getMethod());
	env.push_back("SCRIPT_NAME=" + _getScriptName());
	env.push_back("SCRIPT_FILENAME=" + _scriptPath);
	env.push_back("QUERY_STRING=" + _request->getQuery());
	env.push_back("REQUEST_URI=" + _request->getUri());

	// PATH_INFO and PATH_TRANSLATED
	std::string pathInfo = _getPathInfo();
	if (!pathInfo.empty()) {
		env.push_back("PATH_INFO=" + pathInfo);
		env.push_back("PATH_TRANSLATED=" + _getPathTranslated());
	}

	// Server information
	if (_serverConfig) {
		env.push_back("SERVER_NAME=" + _serverConfig->getHost());
		env.push_back("SERVER_PORT=" + Utils::intToString(_serverConfig->getPort()));
		env.push_back("DOCUMENT_ROOT=" + _serverConfig->getRoot());
	}

	// Client information
	env.push_back("REMOTE_ADDR=" + _clientIp);
	env.push_back("REMOTE_PORT=" + Utils::intToString(_clientPort));

	// Content information
	if (_request->hasHeader("Content-Type"))
		env.push_back("CONTENT_TYPE=" + _request->getHeader("Content-Type"));
	if (_request->hasHeader("Content-Length"))
		env.push_back("CONTENT_LENGTH=" + _request->getHeader("Content-Length"));

	// HTTP headers as environment variables
	const std::map<std::string, std::string>& headers = _request->getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		 it != headers.end(); ++it) {
		std::string name = "HTTP_" + Utils::toUpper(it->first);
		// Replace - with _
		for (size_t i = 0; i < name.length(); ++i) {
			if (name[i] == '-')
				name[i] = '_';
		}
		env.push_back(name + "=" + it->second);
	}

	// Add some additional useful variables
	env.push_back("REDIRECT_STATUS=200");

	return env;
}

char** CGIHandler::_envToCharArray(const std::vector<std::string>& env) const {
	char** arr = new char*[env.size() + 1];
	for (size_t i = 0; i < env.size(); ++i) {
		arr[i] = new char[env[i].length() + 1];
		std::strcpy(arr[i], env[i].c_str());
	}
	arr[env.size()] = NULL;
	return arr;
}

void CGIHandler::_freeCharArray(char** arr) const {
	if (arr == NULL)
		return;
	for (size_t i = 0; arr[i] != NULL; ++i) {
		delete[] arr[i];
	}
	delete[] arr;
}

std::string CGIHandler::_getPathInfo() const {
	// PATH_INFO is the extra path information after the script name
	// For example: /cgi-bin/script.py/extra/path -> PATH_INFO = /extra/path

	std::string uri = _request->getPath();
	std::string scriptName = _getScriptName();

	if (uri.length() > scriptName.length() &&
		uri.compare(0, scriptName.length(), scriptName) == 0) {
		return uri.substr(scriptName.length());
	}
	return "";
}

std::string CGIHandler::_getPathTranslated() const {
	std::string pathInfo = _getPathInfo();
	if (pathInfo.empty())
		return "";

	if (_serverConfig)
		return Utils::joinPath(_serverConfig->getRoot(), pathInfo);
	return pathInfo;
}

std::string CGIHandler::_getScriptName() const {
	// SCRIPT_NAME should be the URI path to the script
	std::string uri = _request->getPath();

	// Find where the script extension ends
	std::string ext = Utils::getFileExtension(_scriptPath);
	if (!ext.empty()) {
		size_t extPos = uri.find("." + ext);
		if (extPos != std::string::npos) {
			return uri.substr(0, extPos + ext.length() + 1);
		}
	}

	return uri;
}
