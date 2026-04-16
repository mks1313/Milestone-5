/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 00:00:00 by mmarinov          #+#    #+#             */
/*   Updated: 2026/04/10 19:58:07 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <vector>
# include <fstream>
# include <sstream>
# include <stdexcept>
# include "../../inc/config/ServerConfig.hpp"

class ConfigException : public std::exception {
public:
	ConfigException(const std::string& msg) : _msg(msg) {}
	virtual ~ConfigException() throw() {}
	virtual const char* what() const throw() { return _msg.c_str(); }
private:
	std::string _msg;
};

class Config {
public:
	Config();
	Config(const std::string& filename);
	Config(const Config& other);
	Config& operator=(const Config& other);
	~Config();

	// Parsing
	void										parse(const std::string& filename);
	void										parseFromString(const std::string& content);

	// Getters
	const std::vector<ServerConfig>&			getServers() const;
	const ServerConfig*							findServer(const std::string& host, int port) const;

	// Validation
	bool										isValid() const;
	void										validate() const;

	// Info
	void										print() const;

private:
	std::vector<ServerConfig>					_servers;

	// Parsing helpers
	void										_parseServer(std::istream& stream);
	void										_parseLocation(std::istream& stream, ServerConfig& server);
	std::string									_getNextToken(std::istream& stream);
	void										_skipWhitespaceAndComments(std::istream& stream);
	bool										_expectToken(std::istream& stream, const std::string& expected);
	void										_parseDirective(std::istream& stream, ServerConfig& server, const std::string& directive);
	void										_parseLocationDirective(std::istream& stream, LocationConfig& location, const std::string& directive);
	size_t										_parseSize(const std::string& str);
	void										_removeComments(std::string& content);
};

#endif
