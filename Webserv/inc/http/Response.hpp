/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 00:00:00 by mmarinov          #+#    #+#             */
/*   Updated: 2026/04/10 18:59:35 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include <map>
# include <sstream>
# include "../../inc/http/Request.hpp"
# include "../../inc/config/ServerConfig.hpp"

class Response {
public:
	Response();
	Response(const Response& other);
	Response& operator=(const Response& other);
	~Response();

	// Building response
	void							setStatusCode(int code);
	void							setHeader(const std::string& name, const std::string& value);
	void							setBody(const std::string& body);
	void							setContentType(const std::string& type);
	void							setCookie(const std::string& name, const std::string& value,
										const std::string& path = "/", int maxAge = -1,
										bool httpOnly = false, bool secure = false);
	void							appendBody(const std::string& data);

	// Response generation
	std::string						build(bool excludeBody = false) const;
	std::string						buildHeaders() const;
	const std::string&				getBody() const;

	// Getters
	int								getStatusCode() const;
	std::string						getHeader(const std::string& name) const;
	size_t							getContentLength() const;
	bool							isSent() const;
	size_t							getBytesSent() const;

	// State management
	void							markAsSent();
	void							addBytesSent(size_t bytes);
	void							reset();

	// Static builders
	static Response					makeError(int code, const ServerConfig* config = NULL);
	static Response					makeRedirect(int code, const std::string& location);
	static Response					makeFile(const std::string& path, const std::string& contentType);
	static Response					makeDirectoryListing(const std::string& path, const std::string& uri);
	static Response					makeFromCGI(const std::string& cgiOutput);

private:
	int								_statusCode;
	std::map<std::string, std::string>	_headers;
	std::string						_body;
	bool							_sent;
	size_t							_bytesSent;

	void							_setDefaultHeaders();
	static std::string				_getDefaultErrorPage(int code);
};

#endif
