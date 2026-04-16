/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 13:39:03 by vberdugo          #+#    #+#             */
/*   Updated: 2026/04/10 19:07:07 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/utils/Utils.hpp"
#include <unistd.h>
#include <dirent.h>

namespace Utils {

// ============================================================================
// String utilities
// ============================================================================

std::string trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(" \t\r\n");
	return str.substr(start, end - start + 1);
}

std::string toLower(const std::string& str) {
	std::string result = str;
	for (size_t i = 0; i < result.length(); ++i)
		result[i] = std::tolower(static_cast<unsigned char>(result[i]));
	return result;
}

std::string toUpper(const std::string& str) {
	std::string result = str;
	for (size_t i = 0; i < result.length(); ++i)
		result[i] = std::toupper(static_cast<unsigned char>(result[i]));
	return result;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::istringstream stream(str);
	std::string token;
	while (std::getline(stream, token, delimiter)) {
		if (!token.empty())
			tokens.push_back(token);
	}
	return tokens;
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = str.find(delimiter);
	while (end != std::string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + delimiter.length();
		end = str.find(delimiter, start);
	}
	tokens.push_back(str.substr(start));
	return tokens;
}

bool startsWith(const std::string& str, const std::string& prefix) {
	if (prefix.length() > str.length())
		return false;
	return str.compare(0, prefix.length(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix) {
	if (suffix.length() > str.length())
		return false;
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string replaceAll(const std::string& str, const std::string& from, const std::string& to) {
	std::string result = str;
	size_t pos = 0;
	while ((pos = result.find(from, pos)) != std::string::npos) {
		result.replace(pos, from.length(), to);
		pos += to.length();
	}
	return result;
}

// ============================================================================
// Number conversions
// ============================================================================

int stringToInt(const std::string& str) {
	std::istringstream stream(str);
	int result;
	stream >> result;
	return result;
}

size_t stringToSizeT(const std::string& str) {
	std::istringstream stream(str);
	size_t result;
	stream >> result;
	return result;
}

std::string intToString(int n) {
	std::ostringstream stream;
	stream << n;
	return stream.str();
}

std::string sizeTToString(size_t n) {
	std::ostringstream stream;
	stream << n;
	return stream.str();
}

size_t hexToSizeT(const std::string& hex) {
	size_t result = 0;
	std::istringstream stream(hex);
	stream >> std::hex >> result;
	return result;
}

// ============================================================================
// File utilities
// ============================================================================

bool fileExists(const std::string& path) {
	struct stat st;
	return stat(path.c_str(), &st) == 0;
}

bool isDirectory(const std::string& path) {
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;
	return S_ISDIR(st.st_mode);
}

bool isReadable(const std::string& path) {
	return access(path.c_str(), R_OK) == 0;
}

bool isWritable(const std::string& path) {
	return access(path.c_str(), W_OK) == 0;
}

bool isExecutable(const std::string& path) {
	return access(path.c_str(), X_OK) == 0;
}

size_t getFileSize(const std::string& path) {
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return 0;
	return static_cast<size_t>(st.st_size);
}

std::string getFileExtension(const std::string& path) {
	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos || pos == path.length() - 1)
		return "";
	return path.substr(pos + 1);
}

std::string getFileName(const std::string& path) {
	size_t pos = path.find_last_of('/');
	if (pos == std::string::npos)
		return path;
	return path.substr(pos + 1);
}

std::string getDirectory(const std::string& path) {
	size_t pos = path.find_last_of('/');
	if (pos == std::string::npos)
		return ".";
	return path.substr(0, pos);
}

std::string readFile(const std::string& path) {
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
		return "";
	std::ostringstream content;
	content << file.rdbuf();
	return content.str();
}

bool writeFile(const std::string& path, const std::string& content) {
	std::ofstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
		return false;
	file.write(content.c_str(), content.length());
	return file.good();
}

bool deleteFile(const std::string& path) {
	return unlink(path.c_str()) == 0;
}

bool createDirectory(const std::string& path) {
	return mkdir(path.c_str(), 0755) == 0;
}

std::string normalizePath(const std::string& path) {
	if (path.empty())
		return path;

	bool isRelative = (path[0] != '/');
	bool startsWithDot = (path.length() >= 2 && path[0] == '.' && path[1] == '/');

	std::vector<std::string> parts;
	std::vector<std::string> segments = split(path, '/');

	for (size_t i = 0; i < segments.size(); ++i) {
		if (segments[i] == "." || segments[i].empty())
			continue;
		if (segments[i] == "..") {
			if (!parts.empty() && parts.back() != "..")
				parts.pop_back();
			else if (isRelative)
				parts.push_back("..");
		} else {
			parts.push_back(segments[i]);
		}
	}

	std::string result;
	if (!isRelative)
		result = "/";
	else if (startsWithDot)
		result = "./";

	for (size_t i = 0; i < parts.size(); ++i) {
		result += parts[i];
		if (i < parts.size() - 1)
			result += "/";
	}

	if (result.empty())
		result = isRelative ? "." : "/";

	return result;
}

std::string joinPath(const std::string& base, const std::string& path) {
	if (base.empty())
		return path;
	if (path.empty())
		return base;

	std::string result = base;
	if (result[result.length() - 1] != '/')
		result += '/';

	if (path[0] == '/')
		result += path.substr(1);
	else
		result += path;

	return result;
}

// ============================================================================
// HTTP utilities
// ============================================================================

std::string urlDecode(const std::string& str) {
	std::string result;
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '%' && i + 2 < str.length()) {
			std::string hex = str.substr(i + 1, 2);
			char c = static_cast<char>(hexToSizeT(hex));
			result += c;
			i += 2;
		} else if (str[i] == '+') {
			result += ' ';
		} else {
			result += str[i];
		}
	}
	return result;
}

std::string urlEncode(const std::string& str) {
	static const char* hexChars = "0123456789ABCDEF";
	std::string result;
	for (size_t i = 0; i < str.length(); ++i) {
		unsigned char c = static_cast<unsigned char>(str[i]);
		if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			result += c;
		} else if (c == ' ') {
			result += '+';
		} else {
			result += '%';
			result += hexChars[(c >> 4) & 0x0F];
			result += hexChars[c & 0x0F];
		}
	}
	return result;
}

std::string getHttpDate() {
	return getHttpDate(std::time(NULL));
}

std::string getHttpDate(time_t timestamp) {
	char buffer[100];
	struct tm* tm_info = std::gmtime(&timestamp);
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
	return std::string(buffer);
}

std::string getStatusMessage(int code) {
	switch (code) {
		case 100: return "Continue";
		case 101: return "Switching Protocols";
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 204: return "No Content";
		case 206: return "Partial Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 416: return "Range Not Satisfiable";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		default: return "Unknown";
	}
}

bool isValidMethod(const std::string& method) {
	return method == "GET" || method == "POST" || method == "DELETE" ||
		   method == "PUT" || method == "HEAD" || method == "OPTIONS" ||
		   method == "PATCH" || method == "CONNECT" || method == "TRACE";
}

// ============================================================================
// Logging
// ============================================================================

void logInfo(const std::string& msg) {
	std::cout << "\033[0;32m[INFO]\033[0m " << msg << std::endl;
}

void logWarning(const std::string& msg) {
	std::cout << "\033[0;33m[WARN]\033[0m " << msg << std::endl;
}

void logError(const std::string& msg) {
	std::cerr << "\033[0;31m[ERROR]\033[0m " << msg << std::endl;
}

void logDebug(const std::string& msg) {
	std::cout << "\033[0;36m[DEBUG]\033[0m " << msg << std::endl;
}

void logRequest(const std::string& method, const std::string& uri, int code) {
	std::string color;
	if (code >= 200 && code < 300)
		color = "\033[0;32m";
	else if (code >= 300 && code < 400)
		color = "\033[0;33m";
	else
		color = "\033[0;31m";

	std::cout << color << "[" << code << "]\033[0m " << method << " " << uri << std::endl;
}

// ============================================================================
// Random utilities
// ============================================================================

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

std::string generateBoundary() {
	return "----WebServBoundary" + generateSessionId();
}

} // namespace Utils
