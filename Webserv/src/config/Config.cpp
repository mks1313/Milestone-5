/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 14:05:58 by victor            #+#    #+#             */
/*   Updated: 2026/04/10 19:59:46 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/config/Config.hpp"
#include "../../inc/utils/Utils.hpp"
#include <iostream>

Config::Config() {
}

Config::Config(const std::string& filename) {
	parse(filename);
}

Config::Config(const Config& other)
	: _servers(other._servers)
{
}

Config& Config::operator=(const Config& other) {
	if (this != &other) {
		_servers = other._servers;
	}
	return *this;
}

Config::~Config() {
}

void Config::_removeComments(std::string& content) {
	size_t pos;
	// Remove single line comments (#)
	while ((pos = content.find('#')) != std::string::npos) {
		size_t endLine = content.find('\n', pos);
		if (endLine == std::string::npos)
			content.erase(pos);
		else
			content.erase(pos, endLine - pos);
	}
}

void Config::parse(const std::string& filename) {
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw ConfigException("Cannot open configuration file: " + filename);

	std::ostringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	file.close();

	parseFromString(content);
}

void Config::parseFromString(const std::string& content) {
	std::string cleanContent = content;
	_removeComments(cleanContent);

	std::istringstream stream(cleanContent);
	std::string token;

	while (!(token = _getNextToken(stream)).empty()) {
		if (token == "server") {
			_parseServer(stream);
		} else {
			throw ConfigException("Unexpected token: " + token);
		}
	}

	if (_servers.empty())
		throw ConfigException("No server block found in configuration");

	validate();
}

void Config::_parseServer(std::istream& stream) {
	ServerConfig server;
	std::string token = _getNextToken(stream);

	if (token != "{")
		throw ConfigException("Expected '{' after 'server', got: " + token);

	while (!(token = _getNextToken(stream)).empty() && token != "}") {
		if (token == "location") {
			_parseLocation(stream, server);
		} else {
			_parseDirective(stream, server, token);
		}
	}

	if (token != "}")
		throw ConfigException("Missing '}' at end of server block");

	_servers.push_back(server);
}

void Config::_parseLocation(std::istream& stream, ServerConfig& server) {
	LocationConfig location;

	// Get location path
	std::string path = _getNextToken(stream);
	if (path.empty() || path == "{")
		throw ConfigException("Missing path in location block");

	location.setPath(path);

	// Clear default methods - will be set by config
	location = LocationConfig();
	location.setPath(path);

	std::string token = _getNextToken(stream);
	if (token != "{")
		throw ConfigException("Expected '{' after location path, got: " + token);

	while (!(token = _getNextToken(stream)).empty() && token != "}") {
		_parseLocationDirective(stream, location, token);
	}

	if (token != "}")
		throw ConfigException("Missing '}' at end of location block");

	server.addLocation(location);
}

void Config::_parseDirective(std::istream& stream, ServerConfig& server, const std::string& directive) {
	std::string token;

	if (directive == "listen") {
		token = _getNextToken(stream);
		// Parse host:port or just port
		size_t colonPos = token.find(':');
		if (colonPos != std::string::npos) {
			server.setHost(token.substr(0, colonPos));
			server.setPort(Utils::stringToInt(token.substr(colonPos + 1)));
		} else {
			server.setPort(Utils::stringToInt(token));
		}
		_expectToken(stream, ";");
	}
	else if (directive == "server_name") {
		while (!(token = _getNextToken(stream)).empty() && token != ";") {
			server.addServerName(token);
		}
	}
	else if (directive == "root") {
		token = _getNextToken(stream);
		server.setRoot(token);
		_expectToken(stream, ";");
	}
	else if (directive == "index") {
		// Read all index files until ;
		std::string indices;
		while (!(token = _getNextToken(stream)).empty() && token != ";") {
			if (!indices.empty())
				indices += " ";
			indices += token;
		}
		// Use first index file as the primary
		size_t spacePos = indices.find(' ');
		if (spacePos != std::string::npos)
			server.setIndex(indices.substr(0, spacePos));
		else
			server.setIndex(indices);
	}
	else if (directive == "client_max_body_size") {
		token = _getNextToken(stream);
		server.setMaxBodySize(_parseSize(token));
		_expectToken(stream, ";");
	}
	else if (directive == "error_page") {
		// error_page can have multiple codes: error_page 500 502 503 504 /path;
		std::vector<int> codes;
		std::string path;
		while (!(token = _getNextToken(stream)).empty() && token != ";") {
			// If it starts with / or ., it's the path
			if (token[0] == '/' || token[0] == '.') {
				path = token;
			} else {
				codes.push_back(Utils::stringToInt(token));
			}
		}
		for (size_t i = 0; i < codes.size(); ++i) {
			server.addErrorPage(codes[i], path);
		}
	}
	else if (directive == "autoindex") {
		token = _getNextToken(stream);
		server.setAutoindex(token == "on");
		_expectToken(stream, ";");
	}
	else {
		throw ConfigException("Unknown directive: " + directive);
	}
}

void Config::_parseLocationDirective(std::istream& stream, LocationConfig& location, const std::string& directive) {
	std::string token;

	if (directive == "root") {
		token = _getNextToken(stream);
		location.setRoot(token);
		_expectToken(stream, ";");
	}
	else if (directive == "alias") {
		token = _getNextToken(stream);
		location.setAlias(token);
		_expectToken(stream, ";");
	}
	else if (directive == "index") {
		// Read all index files until ;
		std::string indices;
		while (!(token = _getNextToken(stream)).empty() && token != ";") {
			if (!indices.empty())
				indices += " ";
			indices += token;
		}
		// Use first index file as the primary
		size_t spacePos = indices.find(' ');
		if (spacePos != std::string::npos)
			location.setIndex(indices.substr(0, spacePos));
		else
			location.setIndex(indices);
	}
	else if (directive == "autoindex") {
		token = _getNextToken(stream);
		location.setAutoindex(token == "on");
		_expectToken(stream, ";");
	}
	else if (directive == "allow_methods" || directive == "methods" || directive == "limit_except") {
		// CRITICAL: Clear default methods first before adding configured ones
		location.clearAllowedMethods();
		std::set<std::string> methods;
		while (!(token = _getNextToken(stream)).empty() && token != ";") {
			methods.insert(token);
		}
		// Add only configured methods
		for (std::set<std::string>::iterator it = methods.begin(); it != methods.end(); ++it) {
			location.addAllowedMethod(*it);
		}
	}
	else if (directive == "return" || directive == "redirect") {
		int code = Utils::stringToInt(_getNextToken(stream));
		std::string url = _getNextToken(stream);
		location.setRedirectCode(code);
		location.setRedirect(url);
		_expectToken(stream, ";");
	}
	else if (directive == "cgi" || directive == "cgi_pass") {
		std::string ext = _getNextToken(stream);
		std::string handler = _getNextToken(stream);
		location.addCgiHandler(ext, handler);
		_expectToken(stream, ";");
	}
	else if (directive == "upload_enable" || directive == "upload") {
		token = _getNextToken(stream);
		location.setUploadEnabled(token == "on");
		_expectToken(stream, ";");
	}
	else if (directive == "upload_store" || directive == "upload_path") {
		token = _getNextToken(stream);
		location.setUploadPath(token);
		location.setUploadEnabled(true);
		_expectToken(stream, ";");
	}
	else if (directive == "client_max_body_size") {
		token = _getNextToken(stream);
		location.setMaxBodySize(_parseSize(token));
		_expectToken(stream, ";");
	}
	else {
		throw ConfigException("Unknown location directive: " + directive);
	}
}

std::string Config::_getNextToken(std::istream& stream) {
	_skipWhitespaceAndComments(stream);

	std::string token;
	char c;

	while (stream.get(c)) {
		if (std::isspace(c)) {
			if (!token.empty())
				break;
			continue;
		}

		if (c == '{' || c == '}' || c == ';') {
			if (!token.empty()) {
				stream.putback(c);
				break;
			}
			token += c;
			break;
		}

		token += c;
	}

	return token;
}

void Config::_skipWhitespaceAndComments(std::istream& stream) {
	char c;
	while (stream.get(c)) {
		if (c == '#') {
			// Skip until end of line
			while (stream.get(c) && c != '\n')
				;
		} else if (!std::isspace(c)) {
			stream.putback(c);
			return;
		}
	}
}

bool Config::_expectToken(std::istream& stream, const std::string& expected) {
	std::string token = _getNextToken(stream);
	if (token != expected)
		throw ConfigException("Expected '" + expected + "', got: " + token);
	return true;
}

size_t Config::_parseSize(const std::string& str) {
	size_t len = str.length();
	if (len == 0)
		return 0;

	char suffix = str[len - 1];
	std::string numPart = str;
	size_t multiplier = 1;

	if (!std::isdigit(suffix)) {
		numPart = str.substr(0, len - 1);
		switch (std::tolower(suffix)) {
			case 'k':
				multiplier = 1024;
				break;
			case 'm':
				multiplier = 1024 * 1024;
				break;
			case 'g':
				multiplier = 1024 * 1024 * 1024;
				break;
			default:
				throw ConfigException("Invalid size suffix: " + str);
		}
	}

	return Utils::stringToSizeT(numPart) * multiplier;
}

const std::vector<ServerConfig>& Config::getServers() const {
	return _servers;
}

const ServerConfig* Config::findServer(const std::string& host, int port) const {
	const ServerConfig* defaultServer = NULL;

	for (size_t i = 0; i < _servers.size(); ++i) {
		if (_servers[i].getPort() == port) {
			if (defaultServer == NULL)
				defaultServer = &_servers[i];
			if (_servers[i].matchServerName(host))
				return &_servers[i];
		}
	}

	return defaultServer;
}

bool Config::isValid() const {
	if (_servers.empty())
		return false;
	for (size_t i = 0; i < _servers.size(); ++i) {
		if (!_servers[i].isValid())
			return false;
	}
	return true;
}

void Config::validate() const {
	if (_servers.empty())
		throw ConfigException("No server blocks defined");

	std::set<std::pair<std::string, int> > listeners;

	for (size_t i = 0; i < _servers.size(); ++i) {
		const ServerConfig& server = _servers[i];

		if (!server.isValid())
			throw ConfigException("Invalid server configuration");

		std::pair<std::string, int> listener(server.getHost(), server.getPort());
		if (listeners.find(listener) != listeners.end()) {
			// It's OK to have same host:port with different server names
			// Just check that server names are different
			for (size_t j = 0; j < i; ++j) {
				if (_servers[j].getHost() == server.getHost() &&
					_servers[j].getPort() == server.getPort()) {
					// Check for conflicting server names
					const std::vector<std::string>& names1 = server.getServerNames();
					const std::vector<std::string>& names2 = _servers[j].getServerNames();
					for (size_t k = 0; k < names1.size(); ++k) {
						for (size_t l = 0; l < names2.size(); ++l) {
							if (names1[k] == names2[l])
								throw ConfigException("Duplicate server_name for same listen address");
						}
					}
				}
			}
		}
		listeners.insert(listener);
	}
}

void Config::print() const {
	std::cout << "Configuration:" << std::endl;
	for (size_t i = 0; i < _servers.size(); ++i) {
		std::cout << "  Server " << i + 1 << ":" << std::endl;
		std::cout << "    Listen: " << _servers[i].getHost() << ":" << _servers[i].getPort() << std::endl;
		std::cout << "    Root: " << _servers[i].getRoot() << std::endl;
		std::cout << "    Index: " << _servers[i].getIndex() << std::endl;
		std::cout << "    Max body size: " << _servers[i].getMaxBodySize() << std::endl;

		const std::vector<std::string>& names = _servers[i].getServerNames();
		if (!names.empty()) {
			std::cout << "    Server names: ";
			for (size_t j = 0; j < names.size(); ++j)
				std::cout << names[j] << " ";
			std::cout << std::endl;
		}

		const std::vector<LocationConfig>& locations = _servers[i].getLocations();
		for (size_t j = 0; j < locations.size(); ++j) {
			std::cout << "    Location " << locations[j].getPath() << ":" << std::endl;
			std::cout << "      Root: " << locations[j].getRoot() << std::endl;
			std::cout << "      Index: " << locations[j].getIndex() << std::endl;
			std::cout << "      Autoindex: " << (locations[j].getAutoindex() ? "on" : "off") << std::endl;
		}
	}
}
