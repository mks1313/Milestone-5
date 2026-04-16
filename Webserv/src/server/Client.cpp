/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/01 00:00:00 by fcela-ga          #+#    #+#             */
/*   Updated: 2026/04/10 18:29:44 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server/Client.hpp"

Client::Client()
	: _fd(-1)
	, _ip("")
	, _port(0)
	, _serverConfig(NULL)
	, _state(CLIENT_READING)
	, _lastActivity(std::time(NULL))
	, _cgiPid(-1)
	, _cgiFdIn(-1)
	, _cgiFdOut(-1)
	, _cgiStartTime(0)
	, _keepAlive(true)
	, _requestCount(0)
{
}

Client::Client(int fd, const std::string& ip, int port)
	: _fd(fd)
	, _ip(ip)
	, _port(port)
	, _serverConfig(NULL)
	, _state(CLIENT_READING)
	, _lastActivity(std::time(NULL))
	, _cgiPid(-1)
	, _cgiFdIn(-1)
	, _cgiFdOut(-1)
	, _cgiStartTime(0)
	, _keepAlive(true)
	, _requestCount(0)
{
}

Client::Client(const Client& other)
	: _fd(other._fd)
	, _ip(other._ip)
	, _port(other._port)
	, _request(other._request)
	, _response(other._response)
	, _serverConfig(other._serverConfig)
	, _state(other._state)
	, _lastActivity(other._lastActivity)
	, _readBuffer(other._readBuffer)
	, _writeBuffer(other._writeBuffer)
	, _cgiPid(other._cgiPid)
	, _cgiFdIn(other._cgiFdIn)
	, _cgiFdOut(other._cgiFdOut)
	, _cgiOutput(other._cgiOutput)
	, _cgiStartTime(other._cgiStartTime)
	, _keepAlive(other._keepAlive)
	, _requestCount(other._requestCount)
{
}

Client& Client::operator=(const Client& other) {
	if (this != &other) {
		_fd = other._fd;
		_ip = other._ip;
		_port = other._port;
		_request = other._request;
		_response = other._response;
		_serverConfig = other._serverConfig;
		_state = other._state;
		_lastActivity = other._lastActivity;
		_readBuffer = other._readBuffer;
		_writeBuffer = other._writeBuffer;
		_cgiPid = other._cgiPid;
		_cgiFdIn = other._cgiFdIn;
		_cgiFdOut = other._cgiFdOut;
		_cgiOutput = other._cgiOutput;
		_cgiStartTime = other._cgiStartTime;
		_keepAlive = other._keepAlive;
		_requestCount = other._requestCount;
	}
	return *this;
}

Client::~Client() {
}

// ============================================================================
// Socket operations
// ============================================================================

int Client::getFd() const {
	return _fd;
}

const std::string& Client::getIp() const {
	return _ip;
}

int Client::getPort() const {
	return _port;
}

void Client::setFd(int fd) {
	_fd = fd;
}

// ============================================================================
// Request/Response
// ============================================================================

Request& Client::getRequest() {
	return _request;
}

const Request& Client::getRequest() const {
	return _request;
}

Response& Client::getResponse() {
	return _response;
}

const Response& Client::getResponse() const {
	return _response;
}

void Client::setResponse(const Response& response) {
	_response = response;
}

// ============================================================================
// Server config
// ============================================================================

void Client::setServerConfig(const ServerConfig* config) {
	_serverConfig = config;
}

const ServerConfig* Client::getServerConfig() const {
	return _serverConfig;
}

// ============================================================================
// State management
// ============================================================================

ClientState Client::getState() const {
	return _state;
}

void Client::setState(ClientState state) {
	_state = state;
}

bool Client::isReadyToRead() const {
	return _state == CLIENT_READING;
}

bool Client::isReadyToWrite() const {
	return _state == CLIENT_WRITING;
}

bool Client::isDone() const {
	return _state == CLIENT_DONE;
}

bool Client::hasError() const {
	return _state == CLIENT_ERROR;
}

// ============================================================================
// Timeout management
// ============================================================================

void Client::updateLastActivity() {
	_lastActivity = std::time(NULL);
}

time_t Client::getLastActivity() const {
	return _lastActivity;
}

bool Client::hasTimedOut(int timeout) const {
	return (std::time(NULL) - _lastActivity) > timeout;
}

// ============================================================================
// Buffer management
// ============================================================================

std::string& Client::getReadBuffer() {
	return _readBuffer;
}

std::string& Client::getWriteBuffer() {
	return _writeBuffer;
}

void Client::appendToReadBuffer(const char* data, size_t len) {
	_readBuffer.append(data, len);
}

void Client::appendToWriteBuffer(const std::string& data) {
	_writeBuffer += data;
}

void Client::clearReadBuffer() {
	_readBuffer.clear();
}

void Client::clearWriteBuffer() {
	_writeBuffer.clear();
}

size_t Client::getWriteBufferSize() const {
	return _writeBuffer.length();
}

void Client::eraseFromWriteBuffer(size_t len) {
	_writeBuffer.erase(0, len);
}

// ============================================================================
// CGI support
// ============================================================================

void Client::setCgiPid(pid_t pid) {
	_cgiPid = pid;
}

pid_t Client::getCgiPid() const {
	return _cgiPid;
}

void Client::setCgiFdIn(int fd) {
	_cgiFdIn = fd;
}

void Client::setCgiFdOut(int fd) {
	_cgiFdOut = fd;
}

int Client::getCgiFdIn() const {
	return _cgiFdIn;
}

int Client::getCgiFdOut() const {
	return _cgiFdOut;
}

std::string& Client::getCgiOutput() {
	return _cgiOutput;
}

void Client::appendCgiOutput(const std::string& data) {
	_cgiOutput += data;
}

time_t Client::getCgiStartTime() const {
	return _cgiStartTime;
}

void Client::setCgiStartTime(time_t time) {
	_cgiStartTime = time;
}

bool Client::hasCgiTimedOut(int timeout) const {
	if (_cgiStartTime == 0)
		return false;
	return (std::time(NULL) - _cgiStartTime) > timeout;
}

// ============================================================================
// Keep-alive support
// ============================================================================

bool Client::shouldKeepAlive() const {
	if (!_keepAlive)
		return false;

	std::string connection = _request.getHeader("Connection");
	if (_request.getVersion() == "HTTP/1.0") {
		return connection == "keep-alive";
	}
	// HTTP/1.1 default is keep-alive
	return connection != "close";
}

void Client::setKeepAlive(bool keepAlive) {
	_keepAlive = keepAlive;
}

int Client::getRequestCount() const {
	return _requestCount;
}

void Client::incrementRequestCount() {
	_requestCount++;
}

void Client::reset() {
    _request.reset();
    _response.reset();
    _state = CLIENT_READING;
    _cgiPid = -1;
    _cgiFdIn = -1;
    _cgiFdOut = -1;
    _cgiOutput.clear();
    _cgiStartTime = 0;

    // ERROR DETECTADO:
    // _readBuffer.clear();  <-- ELIMINA ESTA LÍNEA DEFINITIVAMENTE.
    // El buffer debe conservarse para que el bucle de Pipelining en Server.cpp
    // pueda procesar los datos restantes.

    _writeBuffer.clear(); // Esta sí se puede limpiar si la respuesta ya se envió.
    updateLastActivity();
}
