/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 00:04:04 by fcela-ga          #+#    #+#             */
/*   Updated: 2026/04/10 19:04:31 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/http/Response.hpp"
#include "../../inc/utils/Utils.hpp"
#include "../../inc/http/MimeTypes.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

Response::Response()
	: _statusCode(200)
	, _sent(false)
	, _bytesSent(0)
{
	_setDefaultHeaders();
}

Response::Response(const Response& other)
	: _statusCode(other._statusCode)
	, _headers(other._headers)
	, _body(other._body)
	, _sent(other._sent)
	, _bytesSent(other._bytesSent)
{
}

Response& Response::operator=(const Response& other) {
	if (this != &other) {
		_statusCode = other._statusCode;
		_headers = other._headers;
		_body = other._body;
		_sent = other._sent;
		_bytesSent = other._bytesSent;
	}
	return *this;
}

Response::~Response() {
}

// ============================================================================
// Building response
// ============================================================================

void Response::setStatusCode(int code) {
	_statusCode = code;
}

void Response::setHeader(const std::string& name, const std::string& value) {
	_headers[name] = value;
}

void Response::setBody(const std::string& body) {
	_body = body;
	setHeader("Content-Length", Utils::sizeTToString(_body.length()));
}

void Response::setContentType(const std::string& type) {
	setHeader("Content-Type", type);
}

void Response::setCookie(const std::string& name, const std::string& value,
						 const std::string& path, int maxAge,
						 bool httpOnly, bool secure) {
	std::string cookie = name + "=" + value;
	if (!path.empty())
		cookie += "; Path=" + path;
	if (maxAge >= 0)
		cookie += "; Max-Age=" + Utils::intToString(maxAge);
	if (httpOnly)
		cookie += "; HttpOnly";
	if (secure)
		cookie += "; Secure";

	setHeader("Set-Cookie", cookie);
}

void Response::appendBody(const std::string& data) {
	_body += data;
	setHeader("Content-Length", Utils::sizeTToString(_body.length()));
}

// ============================================================================
// Response generation
// ============================================================================

std::string Response::build(bool excludeBody) const {
    std::string headers = buildHeaders();
    if (excludeBody) {
        return headers;
    }
    return headers + _body;
}

std::string Response::buildHeaders() const {
	std::ostringstream response;

	// Status line
	response << "HTTP/1.1 " << _statusCode << " " << Utils::getStatusMessage(_statusCode) << "\r\n";

	// Headers
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		 it != _headers.end(); ++it) {
		response << it->first << ": " << it->second << "\r\n";
	}

	// End of headers
	response << "\r\n";

	return response.str();
}

const std::string& Response::getBody() const {
	return _body;
}

// ============================================================================
// Getters
// ============================================================================

int Response::getStatusCode() const {
	return _statusCode;
}

std::string Response::getHeader(const std::string& name) const {
	std::map<std::string, std::string>::const_iterator it = _headers.find(name);
	if (it != _headers.end())
		return it->second;
	return "";
}

size_t Response::getContentLength() const {
	return _body.length();
}

bool Response::isSent() const {
	return _sent;
}

size_t Response::getBytesSent() const {
	return _bytesSent;
}

// ============================================================================
// State management
// ============================================================================

void Response::markAsSent() {
	_sent = true;
}

void Response::addBytesSent(size_t bytes) {
	_bytesSent += bytes;
}

void Response::reset() {
	_statusCode = 200;
	_headers.clear();
	_body.clear();
	_sent = false;
	_bytesSent = 0;
	_setDefaultHeaders();
}

// ============================================================================
// Static builders
// ============================================================================

Response Response::makeError(int code, const ServerConfig* config) {
	Response response;
	response.setStatusCode(code);
	response.setContentType("text/html; charset=utf-8");

	std::string errorPage;
	if (config != NULL) {
		std::string customPage = config->getErrorPage(code);
		if (!customPage.empty()) {
			std::string fullPath = Utils::joinPath(config->getRoot(), customPage);
			if (Utils::fileExists(fullPath) && Utils::isReadable(fullPath)) {
				errorPage = Utils::readFile(fullPath);
			}
		}
	}

	if (errorPage.empty()) {
		errorPage = _getDefaultErrorPage(code);
	}

	response.setBody(errorPage);
	return response;
}

Response Response::makeRedirect(int code, const std::string& location) {
	Response response;
	response.setStatusCode(code);
	response.setHeader("Location", location);
	response.setContentType("text/html; charset=utf-8");

	std::ostringstream body;
	body << "<!DOCTYPE html><html><head><title>Redirect</title></head><body>";
	body << "<h1>" << code << " " << Utils::getStatusMessage(code) << "</h1>";
	body << "<p>Redirecting to <a href=\"" << location << "\">" << location << "</a></p>";
	body << "</body></html>";

	response.setBody(body.str());
	return response;
}

Response Response::makeFile(const std::string& path, const std::string& contentType) {
	Response response;

	std::string content = Utils::readFile(path);
	if (content.empty() && Utils::getFileSize(path) > 0) {
		return makeError(500, NULL);
	}

	response.setStatusCode(200);
	response.setContentType(contentType);
	response.setBody(content);

	return response;
}

Response Response::makeDirectoryListing(const std::string& path, const std::string& uri) {
	Response response;
	response.setStatusCode(200);
	response.setContentType("text/html; charset=utf-8");

	std::ostringstream body;
	body << "<!DOCTYPE html>\n<html>\n<head>\n";
	body << "<title>Index of " << uri << "</title>\n";
	body << "<style>\n";
	body << "body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }\n";
	body << "h1 { color: #333; border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n";
	body << "table { border-collapse: collapse; width: 100%; background: white; box-shadow: 0 1px 3px rgba(0,0,0,0.1); }\n";
	body << "th, td { text-align: left; padding: 12px 15px; border-bottom: 1px solid #eee; }\n";
	body << "th { background: #f8f8f8; font-weight: bold; }\n";
	body << "tr:hover { background: #f5f5f5; }\n";
	body << "a { color: #0066cc; text-decoration: none; }\n";
	body << "a:hover { text-decoration: underline; }\n";
	body << ".size { color: #666; }\n";
	body << ".date { color: #888; }\n";
	body << "</style>\n</head>\n<body>\n";
	body << "<h1>Index of " << uri << "</h1>\n";
	body << "<table>\n<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";

	// Parent directory link
	if (uri != "/") {
		std::string parent = uri;
		if (parent[parent.length() - 1] == '/')
			parent = parent.substr(0, parent.length() - 1);
		size_t lastSlash = parent.find_last_of('/');
		if (lastSlash != std::string::npos)
			parent = parent.substr(0, lastSlash + 1);
		body << "<tr><td><a href=\"" << parent << "\">../</a></td><td>-</td><td>-</td></tr>\n";
	}

	// List directory contents
	DIR* dir = opendir(path.c_str());
	if (dir != NULL) {
		struct dirent* entry;
		std::vector<std::string> entries;

		while ((entry = readdir(dir)) != NULL) {
			std::string name = entry->d_name;
			if (name != "." && name != "..")
				entries.push_back(name);
		}
		closedir(dir);

		// Sort entries
		std::sort(entries.begin(), entries.end());

		for (size_t i = 0; i < entries.size(); ++i) {
			std::string fullPath = Utils::joinPath(path, entries[i]);
			struct stat st;

			if (stat(fullPath.c_str(), &st) == 0) {
				std::string displayName = entries[i];
				std::string link = entries[i];

				if (S_ISDIR(st.st_mode)) {
					displayName += "/";
					link += "/";
				}

				std::string size = "-";
				if (S_ISREG(st.st_mode))
					size = Utils::sizeTToString(st.st_size);

				char timeStr[64];
				struct tm* tm_info = localtime(&st.st_mtime);
				strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", tm_info);

				body << "<tr><td><a href=\"" << Utils::urlEncode(link) << "\">" << displayName << "</a></td>";
				body << "<td class=\"size\">" << size << "</td>";
				body << "<td class=\"date\">" << timeStr << "</td></tr>\n";
			}
		}
	}

	body << "</table>\n";
	body << "<hr><p style=\"color:#888;font-size:12px;\">Webserv/1.0</p>\n";
	body << "</body>\n</html>";

	response.setBody(body.str());
	return response;
}

Response Response::makeFromCGI(const std::string& cgiOutput) {
	Response response;
	std::map<std::string, std::string> headers;
	std::string body;
	int statusCode = 200;

	Utils::logDebug("makeFromCGI: total input size = " + Utils::sizeTToString(cgiOutput.size()) + " bytes");

	// Parse CGI output
	size_t headerEnd = cgiOutput.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		headerEnd = cgiOutput.find("\n\n");

	if (headerEnd != std::string::npos) {
		std::string headerPart = cgiOutput.substr(0, headerEnd);
		size_t bodyStart = headerEnd + (cgiOutput[headerEnd] == '\r' ? 4 : 2);
		body = cgiOutput.substr(bodyStart);

		Utils::logDebug("makeFromCGI: headerEnd=" + Utils::sizeTToString(headerEnd) +
		                ", bodyStart=" + Utils::sizeTToString(bodyStart) +
		                ", body size=" + Utils::sizeTToString(body.size()) + " bytes");

		// Parse headers
		std::istringstream headerStream(headerPart);
		std::string line;
		while (std::getline(headerStream, line)) {
			// Remove \r if present
			if (!line.empty() && line[line.length() - 1] == '\r')
				line = line.substr(0, line.length() - 1);

			if (line.empty())
				continue;

			size_t colonPos = line.find(':');
			if (colonPos != std::string::npos) {
				std::string name = Utils::trim(line.substr(0, colonPos));
				std::string value = Utils::trim(line.substr(colonPos + 1));

				if (Utils::toLower(name) == "status") {
					// Parse status line
					statusCode = Utils::stringToInt(value.substr(0, 3));
				} else {
					headers[name] = value;
				}
			}
		}
	} else {
		body = cgiOutput;
	}

	response.setStatusCode(statusCode);

	// First, copy all headers from CGI output EXCEPT Content-Length
	// (Content-Length will be recalculated based on actual body size)
	for (std::map<std::string, std::string>::iterator it = headers.begin();
		 it != headers.end(); ++it) {
		// Skip Content-Length from CGI headers - we'll calculate it ourselves
		if (Utils::toLower(it->first) != "content-length") {
			response.setHeader(it->first, it->second);
		}
	}

	// Set default content type if not set
	if (headers.find("Content-Type") == headers.end())
		response.setContentType("text/html; charset=utf-8");

	// IMPORTANT: setBody() will automatically set Content-Length based on actual body size
	// This ensures correct Content-Length even if CGI doesn't provide it or provides wrong value
	response.setBody(body);
	return response;
}

// ============================================================================
// Private methods
// ============================================================================

void Response::_setDefaultHeaders() {
	setHeader("Server", "Webserv/1.0");
	setHeader("Date", Utils::getHttpDate());
	setHeader("Connection", "keep-alive");
}

std::string Response::_getDefaultErrorPage(int code) {
	std::ostringstream page;
	page << "<!DOCTYPE html>\n<html>\n<head>\n";
	page << "<title>" << code << " " << Utils::getStatusMessage(code) << "</title>\n";
	page << "<style>\n";
	page << "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; margin: 0; min-height: 100vh; }\n";
	page << ".container { max-width: 600px; margin: 0 auto; background: rgba(255,255,255,0.1); padding: 40px; border-radius: 10px; }\n";
	page << "h1 { font-size: 72px; margin: 0; text-shadow: 2px 2px 4px rgba(0,0,0,0.3); }\n";
	page << "h2 { font-size: 24px; margin-top: 10px; opacity: 0.9; }\n";
	page << "p { font-size: 16px; opacity: 0.8; }\n";
	page << "a { color: #fff; text-decoration: underline; }\n";
	page << "</style>\n</head>\n<body>\n";
	page << "<div class=\"container\">\n";
	page << "<h1>" << code << "</h1>\n";
	page << "<h2>" << Utils::getStatusMessage(code) << "</h2>\n";

	switch (code) {
		case 400:
			page << "<p>The request could not be understood by the server.</p>\n";
			break;
		case 403:
			page << "<p>You don't have permission to access this resource.</p>\n";
			break;
		case 404:
			page << "<p>The requested resource could not be found on this server.</p>\n";
			break;
		case 405:
			page << "<p>The request method is not allowed for this resource.</p>\n";
			break;
		case 413:
			page << "<p>The request body is too large.</p>\n";
			break;
		case 500:
			page << "<p>The server encountered an internal error.</p>\n";
			break;
		case 502:
			page << "<p>Bad gateway - invalid response from upstream server.</p>\n";
			break;
		case 503:
			page << "<p>The server is temporarily unavailable.</p>\n";
			break;
		case 504:
			page << "<p>Gateway timeout - upstream server didn't respond in time.</p>\n";
			break;
		default:
			page << "<p>An error occurred while processing your request.</p>\n";
	}

	page << "<p><a href=\"/\">Return to homepage</a></p>\n";
	page << "</div>\n</body>\n</html>";

	return page.str();
}
