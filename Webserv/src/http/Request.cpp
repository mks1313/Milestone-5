/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 19:52:19 by fcela-ga          #+#    #+#             */
/*   Updated: 2026/04/10 20:16:57 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/http/Request.hpp"
#include "../../inc/utils/Utils.hpp"
#include <algorithm>
#include <cctype>

Request::Request()
	: _port(80)
	, _state(PARSE_REQUEST_LINE)
	, _errorCode(0)
	, _contentLength(0)
	, _bodyBytesReceived(0)
	, _isChunked(false)
	, _currentChunkSize(0)
	, _headersParsed(false)
{
}

Request::Request(const Request& other)
	: _method(other._method)
	, _uri(other._uri)
	, _path(other._path)
	, _query(other._query)
	, _fragment(other._fragment)
	, _version(other._version)
	, _headers(other._headers)
	, _body(other._body)
	, _rawRequest(other._rawRequest)
	, _host(other._host)
	, _port(other._port)
	, _queryParams(other._queryParams)
	, _cookies(other._cookies)
	, _uploadedFiles(other._uploadedFiles)
	, _state(other._state)
	, _errorCode(other._errorCode)
	, _contentLength(other._contentLength)
	, _bodyBytesReceived(other._bodyBytesReceived)
	, _isChunked(other._isChunked)
	, _currentChunk(other._currentChunk)
	, _currentChunkSize(other._currentChunkSize)
	, _headersParsed(other._headersParsed)
{
}

Request& Request::operator=(const Request& other) {
	if (this != &other) {
		_method = other._method;
		_uri = other._uri;
		_path = other._path;
		_query = other._query;
		_fragment = other._fragment;
		_version = other._version;
		_headers = other._headers;
		_body = other._body;
		_rawRequest = other._rawRequest;
		_host = other._host;
		_port = other._port;
		_queryParams = other._queryParams;
		_cookies = other._cookies;
		_uploadedFiles = other._uploadedFiles;
		_state = other._state;
		_errorCode = other._errorCode;
		_contentLength = other._contentLength;
		_bodyBytesReceived = other._bodyBytesReceived;
		_isChunked = other._isChunked;
		_currentChunk = other._currentChunk;
		_currentChunkSize = other._currentChunkSize;
		_headersParsed = other._headersParsed;
	}
	return *this;
}

Request::~Request() {
}

// ============================================================================
// Parsing
// ============================================================================

size_t Request::parse(const std::string& data) {
    // 1. Guardamos cuánto había en el buffer interno antes de añadir lo nuevo
    size_t initial_internal_len = _rawRequest.length();

    // 2. Añadimos los nuevos datos recibidos del socket
    _rawRequest += data;

    // 3. Procesamos los datos (mientras el estado lo permita)
    while (_state != PARSE_COMPLETE && _state != PARSE_ERROR) {
        size_t old_len = _rawRequest.length();

        if (_state == PARSE_REQUEST_LINE) {
            size_t pos = _rawRequest.find("\r\n");
            if (pos == std::string::npos) break;
            std::string line = _rawRequest.substr(0, pos);
            _rawRequest.erase(0, pos + 2);

            // RFC 7230 3.5: Ignore empty lines (CRLF) before request-line
            if (line.empty()) continue;

            if (!_parseRequestLine(line)) { _state = PARSE_ERROR; break; }
            _state = PARSE_HEADERS;
        }
        else if (_state == PARSE_HEADERS) {
            size_t pos = _rawRequest.find("\r\n");
            if (pos == std::string::npos) break;
            if (pos == 0) {
                _rawRequest.erase(0, 2);
                _headersParsed = true;

                _parseHost();
                _parseCookies();

                std::string transferEncoding = getHeader("Transfer-Encoding");
                if (Utils::toLower(transferEncoding) == "chunked") {
                    _isChunked = true;
                    _state = PARSE_CHUNKED;
                } else if (hasHeader("Content-Length")) {
                    _contentLength = Utils::stringToSizeT(getHeader("Content-Length"));
                    _state = (_contentLength > 0) ? PARSE_BODY : PARSE_COMPLETE;
                } else {
                    _state = PARSE_COMPLETE;
                }
            } else {
                std::string line = _rawRequest.substr(0, pos);
                _rawRequest.erase(0, pos + 2);
                if (!_parseHeader(line)) {
                    _state = PARSE_ERROR;
                    break;
                }
            }
        }
        else if (_state == PARSE_BODY) {
            size_t bytesToRead = _contentLength - _bodyBytesReceived;
            size_t available = _rawRequest.length();
            size_t readNow = std::min(bytesToRead, available);
            _body += _rawRequest.substr(0, readNow);
            _rawRequest.erase(0, readNow);
            _bodyBytesReceived += readNow;
            if (_bodyBytesReceived >= _contentLength) {
                std::string contentType = getHeader("Content-Type");
                if (contentType.find("multipart/form-data") != std::string::npos) {
                    _parseMultipartBody();
                }
                _state = PARSE_COMPLETE;
            }
            break;
        }
        else if (_state == PARSE_CHUNKED) {
            if (!_parseChunkedBody())
                break;
        }

        if (old_len == _rawRequest.length()) break;
    }

    // 4. SOLUCIÓN AL ERROR DE COMPILACIÓN:
    // Calculamos cuántos bytes se han "extraído" realmente del flujo.
    // (Lo que había al principio + lo que entró nuevo) - lo que queda sin procesar.
    size_t total_at_start = initial_internal_len + data.length();
    size_t remaining_at_end = _rawRequest.length();

    return total_at_start - remaining_at_end;
}

void Request::appendData(const std::string& data) {
	_rawRequest += data;

	while (_state != PARSE_COMPLETE && _state != PARSE_ERROR) {
		if (_state == PARSE_REQUEST_LINE) {
			size_t pos = _rawRequest.find("\r\n");
			if (pos == std::string::npos)
				return;  // Need more data

			std::string line = _rawRequest.substr(0, pos);
			_rawRequest.erase(0, pos + 2);

			if (!_parseRequestLine(line)) {
				_state = PARSE_ERROR;
				return;
			}
			_state = PARSE_HEADERS;
		}
		else if (_state == PARSE_HEADERS) {
			size_t pos = _rawRequest.find("\r\n");
			if (pos == std::string::npos)
				return;  // Need more data

			if (pos == 0) {
				// Empty line - end of headers
				_rawRequest.erase(0, 2);
				_headersParsed = true;

				// Parse special headers
				_parseHost();
				_parseCookies();

				// Check for body
				std::string transferEncoding = getHeader("Transfer-Encoding");
				if (Utils::toLower(transferEncoding) == "chunked") {
					_isChunked = true;
					_state = PARSE_CHUNKED;
				} else if (hasHeader("Content-Length")) {
					_contentLength = Utils::stringToSizeT(getHeader("Content-Length"));
					if (_contentLength > 0)
						_state = PARSE_BODY;
					else
						_state = PARSE_COMPLETE;
				} else {
					// No body expected
					_state = PARSE_COMPLETE;
				}
			} else {
				std::string line = _rawRequest.substr(0, pos);
				_rawRequest.erase(0, pos + 2);

				if (!_parseHeader(line)) {
					_state = PARSE_ERROR;
					return;
				}
			}
		}
		else if (_state == PARSE_BODY) {
			size_t bytesToRead = _contentLength - _bodyBytesReceived;
			size_t availableBytes = _rawRequest.length();
			size_t readBytes = std::min(bytesToRead, availableBytes);

			_body += _rawRequest.substr(0, readBytes);
			_rawRequest.erase(0, readBytes);
			_bodyBytesReceived += readBytes;

			if (_bodyBytesReceived >= _contentLength) {
				// Check for multipart
				std::string contentType = getHeader("Content-Type");
				if (contentType.find("multipart/form-data") != std::string::npos) {
					_parseMultipartBody();
				}
				_state = PARSE_COMPLETE;
			}
			return;
		}
		else if (_state == PARSE_CHUNKED) {
			if (!_parseChunkedBody())
				return;  // Need more data
		}
	}

	if (_state == PARSE_COMPLETE) {
		_validateRequest();
	}
}

bool Request::_parseRequestLine(const std::string& line) {
	std::vector<std::string> parts = Utils::split(line, ' ');
	if (parts.size() != 3) {
		_errorCode = 400;
		return false;
	}

	_method = parts[0];
	_uri = parts[1];
	_version = parts[2];

	// Validate method
	if (!Utils::isValidMethod(_method)) {
		_errorCode = 501;
		return false;
	}

	// Validate version
	if (_version != "HTTP/1.0" && _version != "HTTP/1.1") {
		_errorCode = 505;
		return false;
	}

	// Validate URI length
	if (_uri.length() > 8192) {
		_errorCode = 414;
		return false;
	}

	// Parse URI
	_parseUri();

	if (_errorCode != 0) {
		return false;
	}

	return true;
}

bool Request::_parseHeader(const std::string& line) {
	size_t pos = line.find(':');
	if (pos == std::string::npos) {
		Utils::logError("Header parse error: no colon in: '" + line.substr(0, 50) + "'");
		_errorCode = 400;
		return false;
	}

	std::string name = Utils::trim(line.substr(0, pos));
	std::string value = Utils::trim(line.substr(pos + 1));

	if (name.empty()) {
		Utils::logError("Header parse error: empty name");
		_errorCode = 400;
		return false;
	}

	std::string normalizedName = _normalizeHeaderName(name);

	// Debug log Content-Length
	if (normalizedName == "Content-Length") {
		Utils::logDebug("Parsed Content-Length: " + value);
	}

	// RFC 7230: Multiple Content-Length headers with different values is an error
	if (normalizedName == "Content-Length")
	{
		if (_headers.find("Content-Length") != _headers.end())
		{
			// Header already exists - check if values are the same
			if (_headers["Content-Length"] != value)
			{
				_errorCode = 400;
				return false;
			}
			// Same value - just ignore the duplicate
			return true;
		}
	}

	_headers[normalizedName] = value;
	return true;
}

void Request::_parseUri() {
	std::string uri = _uri;

	// Security: Reject raw null bytes or encoded null bytes (%00) early
	if (uri.find('\0') != std::string::npos || uri.find("%00") != std::string::npos)
	{
		_errorCode = 400;
		return;
	}

	// Extract fragment
	size_t fragPos = uri.find('#');
	if (fragPos != std::string::npos) {
		_fragment = uri.substr(fragPos + 1);
		uri = uri.substr(0, fragPos);
	}

	// Extract query string
	size_t queryPos = uri.find('?');
	if (queryPos != std::string::npos) {
		_query = uri.substr(queryPos + 1);
		uri = uri.substr(0, queryPos);
		_parseQueryString();
	}

	// URL decode and normalize path
	_path = Utils::urlDecode(uri);

	// Security: Reject paths with null bytes or other dangerous characters
	// Null bytes can be used to bypass security checks
	if (_path.find('\0') != std::string::npos)
	{
		_errorCode = 400;
		return;
	}

	// Check if path ends with slash before normalization (to preserve directory semantics)
	bool trailingSlash = (_path.length() > 0 && _path[_path.length() - 1] == '/');

	_path = Utils::normalizePath(_path);

	// Restore trailing slash if it was removed by normalization (and path is not just "/")
	// This is crucial for directory listing logic which relies on the trailing slash
	if (trailingSlash && _path.length() > 1 && _path[_path.length() - 1] != '/')
		_path += "/";
}

void Request::_parseQueryString() {
	if (_query.empty())
		return;

	std::vector<std::string> pairs = Utils::split(_query, '&');
	for (size_t i = 0; i < pairs.size(); ++i) {
		size_t pos = pairs[i].find('=');
		if (pos != std::string::npos) {
			std::string key = Utils::urlDecode(pairs[i].substr(0, pos));
			std::string value = Utils::urlDecode(pairs[i].substr(pos + 1));
			_queryParams[key] = value;
		} else {
			_queryParams[Utils::urlDecode(pairs[i])] = "";
		}
	}
}

void Request::_parseCookies() {
	std::string cookieHeader = getHeader("Cookie");
	if (cookieHeader.empty())
		return;

	std::vector<std::string> pairs = Utils::split(cookieHeader, ';');
	for (size_t i = 0; i < pairs.size(); ++i) {
		std::string pair = Utils::trim(pairs[i]);
		size_t pos = pair.find('=');
		if (pos != std::string::npos) {
			std::string key = Utils::trim(pair.substr(0, pos));
			std::string value = Utils::trim(pair.substr(pos + 1));
			_cookies[key] = value;
		}
	}
}

void Request::_parseHost() {
	std::string hostHeader = getHeader("Host");
	if (hostHeader.empty())
		return;

	size_t colonPos = hostHeader.find(':');
	if (colonPos != std::string::npos) {
		_host = hostHeader.substr(0, colonPos);
		_port = Utils::stringToInt(hostHeader.substr(colonPos + 1));
	} else {
		_host = hostHeader;
		_port = 80;
	}
}

void Request::_parseMultipartBody() {
	std::string contentType = getHeader("Content-Type");
	size_t boundaryPos = contentType.find("boundary=");
	if (boundaryPos == std::string::npos)
		return;

	std::string boundary = contentType.substr(boundaryPos + 9);
	// Remove quotes if present
	if (!boundary.empty() && boundary[0] == '"')
		boundary = boundary.substr(1, boundary.length() - 2);

	std::string delimiter = "--" + boundary;
	std::string endDelimiter = delimiter + "--";

	size_t pos = 0;
	while (pos < _body.length()) {
		// Find start of part
		size_t partStart = _body.find(delimiter, pos);
		if (partStart == std::string::npos)
			break;

		partStart += delimiter.length();
		if (_body.substr(partStart, 2) == "--")
			break;  // End of multipart

		partStart += 2;  // Skip \r\n

		// Find end of part
		size_t partEnd = _body.find(delimiter, partStart);
		if (partEnd == std::string::npos)
			break;

		std::string part = _body.substr(partStart, partEnd - partStart - 2);

		// Parse part headers
		size_t headerEnd = part.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
			continue;

		std::string partHeaders = part.substr(0, headerEnd);
		std::string partData = part.substr(headerEnd + 4);

		// Parse Content-Disposition
		size_t cdPos = partHeaders.find("Content-Disposition:");
		if (cdPos != std::string::npos) {
			size_t cdEnd = partHeaders.find("\r\n", cdPos);
			std::string cd = partHeaders.substr(cdPos + 20, cdEnd - cdPos - 20);

			UploadedFile file;

			// Get name
			size_t namePos = cd.find("name=\"");
			if (namePos != std::string::npos) {
				namePos += 6;
				size_t nameEnd = cd.find("\"", namePos);
				file.name = cd.substr(namePos, nameEnd - namePos);
			}

			// Get filename
			size_t fnPos = cd.find("filename=\"");
			if (fnPos != std::string::npos) {
				fnPos += 10;
				size_t fnEnd = cd.find("\"", fnPos);
				file.filename = cd.substr(fnPos, fnEnd - fnPos);
			}

			// Get content type
			size_t ctPos = partHeaders.find("Content-Type:");
			if (ctPos != std::string::npos) {
				ctPos += 13;
				size_t ctEnd = partHeaders.find("\r\n", ctPos);
				file.contentType = Utils::trim(partHeaders.substr(ctPos, ctEnd - ctPos));
			}

			file.data = partData;
			_uploadedFiles.push_back(file);
		}

		pos = partEnd;
	}
}

bool Request::_parseChunkedBody() {
	while (!_rawRequest.empty()) {
		if (_currentChunkSize == 0) {
			// Looking for chunk size line
			size_t pos = _rawRequest.find("\r\n");
			if (pos == std::string::npos)
				return false;  // Need more data

			std::string sizeLine = _rawRequest.substr(0, pos);
			_rawRequest.erase(0, pos + 2);

			// Parse chunk size (ignore extensions)
			size_t extPos = sizeLine.find(';');
			if (extPos != std::string::npos)
				sizeLine = sizeLine.substr(0, extPos);

			_currentChunkSize = Utils::hexToSizeT(sizeLine);

			if (_currentChunkSize == 0) {
				// Last chunk - skip trailer
				size_t trailerEnd = _rawRequest.find("\r\n");
				if (trailerEnd != std::string::npos)
					_rawRequest.erase(0, trailerEnd + 2);
				_state = PARSE_COMPLETE;
				return true;
			}
		} else {
			// Reading chunk data
			size_t available = _rawRequest.length();
			if (available < _currentChunkSize + 2)
				return false;  // Need more data

			_body += _rawRequest.substr(0, _currentChunkSize);
			_rawRequest.erase(0, _currentChunkSize + 2);  // +2 for \r\n
			_currentChunkSize = 0;
		}
	}
	return false;  // Need more data
}

bool Request::_validateRequest() {
	// HTTP/1.1 requires Host header
	if (_version == "HTTP/1.1" && !hasHeader("Host")) {
		_errorCode = 400;
		_state = PARSE_ERROR;
		return false;
	}

	return true;
}

std::string Request::_normalizeHeaderName(const std::string& name) const {
	std::string result = name;
	bool capitalize = true;
	for (size_t i = 0; i < result.length(); ++i) {
		if (capitalize) {
			result[i] = std::toupper(static_cast<unsigned char>(result[i]));
			capitalize = false;
		} else {
			result[i] = std::tolower(static_cast<unsigned char>(result[i]));
		}
		if (result[i] == '-')
			capitalize = true;
	}
	return result;
}

// ============================================================================
// Getters
// ============================================================================

bool Request::isComplete() const {
	return _state == PARSE_COMPLETE;
}

bool Request::hasError() const {
	return _state == PARSE_ERROR || _errorCode != 0;
}

int Request::getErrorCode() const {
	return _errorCode;
}

void Request::reset() {
	_method.clear();
	_uri.clear();
	_path.clear();
	_query.clear();
	_fragment.clear();
	_version.clear();
	_headers.clear();
	_body.clear();
	// _rawRequest.clear();
	_host.clear();
	_port = 80;
	_queryParams.clear();
	_cookies.clear();
	_uploadedFiles.clear();
	_state = PARSE_REQUEST_LINE;
	_errorCode = 0;
	_contentLength = 0;
	_bodyBytesReceived = 0;
	_isChunked = false;
	_currentChunk.clear();
	_currentChunkSize = 0;
	_headersParsed = false;
}

const std::string& Request::getMethod() const { return _method; }
const std::string& Request::getUri() const { return _uri; }
const std::string& Request::getPath() const { return _path; }
const std::string& Request::getQuery() const { return _query; }
const std::string& Request::getVersion() const { return _version; }
const std::string& Request::getFragment() const { return _fragment; }
const std::map<std::string, std::string>& Request::getHeaders() const { return _headers; }
const std::string& Request::getBody() const { return _body; }
const std::string& Request::getHost() const { return _host; }
int Request::getPort() const { return _port; }
const std::map<std::string, std::string>& Request::getQueryParams() const { return _queryParams; }
const std::map<std::string, std::string>& Request::getCookies() const { return _cookies; }
const std::vector<UploadedFile>& Request::getUploadedFiles() const { return _uploadedFiles; }
ParseState Request::getState() const { return _state; }
const std::string& Request::getRawRequest() const { return _rawRequest; }
size_t Request::getBodyBytesReceived() const { return _bodyBytesReceived; }
size_t Request::getContentLength() const { return _contentLength; }
bool Request::isChunked() const { return _isChunked; }

std::string Request::getHeader(const std::string& name) const {
	std::string normalized = _normalizeHeaderName(name);
	std::map<std::string, std::string>::const_iterator it = _headers.find(normalized);
	if (it != _headers.end())
		return it->second;
	return "";
}

bool Request::hasHeader(const std::string& name) const {
	std::string normalized = _normalizeHeaderName(name);
	return _headers.find(normalized) != _headers.end();
}

std::string Request::getCookie(const std::string& name) const {
	std::map<std::string, std::string>::const_iterator it = _cookies.find(name);
	if (it != _cookies.end())
		return it->second;
	return "";
}
