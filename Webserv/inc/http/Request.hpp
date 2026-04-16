/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 00:00:00 by mmarinov          #+#    #+#             */
/*   Updated: 2026/04/10 20:15:56 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <map>
# include <vector>
# include <sstream>
# include <cstdlib>

enum ParseState {
	PARSE_REQUEST_LINE,
	PARSE_HEADERS,
	PARSE_BODY,
	PARSE_CHUNKED,
	PARSE_COMPLETE,
	PARSE_ERROR
};

struct UploadedFile {
	std::string			name;
	std::string			filename;
	std::string			contentType;
	std::string			data;
};

class Request {
public:
	Request();
	Request(const Request& other);
	Request& operator=(const Request& other);
	~Request();

	// Parsing
	size_t							parse(const std::string& data);
	void							appendData(const std::string& data);
	bool							isComplete() const;
	bool							hasError() const;
	int								getErrorCode() const;
	void							reset();

	// Getters - Request Line
	const std::string&				getMethod() const;
	const std::string&				getUri() const;
	const std::string&				getPath() const;
	const std::string&				getQuery() const;
	const std::string&				getVersion() const;
	const std::string&				getFragment() const;

	// Getters - Headers
	const std::map<std::string, std::string>&	getHeaders() const;
	std::string						getHeader(const std::string& name) const;
	bool							hasHeader(const std::string& name) const;

	// Getters - Body
	const std::string&				getBody() const;
	size_t							getContentLength() const;
	bool							isChunked() const;

	// Getters - Parsed data
	const std::string&				getHost() const;
	int								getPort() const;
	const std::map<std::string, std::string>&	getQueryParams() const;
	const std::map<std::string, std::string>&	getCookies() const;
	std::string						getCookie(const std::string& name) const;
	const std::vector<UploadedFile>&	getUploadedFiles() const;

	// Getters - State
	ParseState						getState() const;
	const std::string&				getRawRequest() const;
	size_t							getBodyBytesReceived() const;

private:
	// Request data
	std::string						_method;
	std::string						_uri;
	std::string						_path;
	std::string						_query;
	std::string						_fragment;
	std::string						_version;
	std::map<std::string, std::string>	_headers;
	std::string						_body;
	std::string						_rawRequest;

	// Parsed data
	std::string						_host;
	int								_port;
	std::map<std::string, std::string>	_queryParams;
	std::map<std::string, std::string>	_cookies;
	std::vector<UploadedFile>		_uploadedFiles;

	// State
	ParseState						_state;
	int								_errorCode;
	size_t							_contentLength;
	size_t							_bodyBytesReceived;
	bool							_isChunked;
	std::string						_currentChunk;
	size_t							_currentChunkSize;
	bool							_headersParsed;

	// Parsing methods
	bool							_parseRequestLine(const std::string& line);
	bool							_parseHeader(const std::string& line);
	void							_parseUri();
	void							_parseQueryString();
	void							_parseCookies();
	void							_parseHost();
	void							_parseMultipartBody();
	bool							_parseChunkedBody();
	bool							_validateRequest();
	std::string						_normalizeHeaderName(const std::string& name) const;
};

#endif
