/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/01 00:00:00 by fcela-ga          #+#    #+#             */
/*   Updated: 2026/04/10 20:06:02 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <string>
# include <vector>
# include <map>
# include "../../inc/config/LocationConfig.hpp"

class ServerConfig {
public:
	ServerConfig();
	ServerConfig(const ServerConfig& other);
	ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();

	// Getters
	const std::string&						getHost() const;
	int										getPort() const;
	const std::vector<std::string>&			getServerNames() const;
	const std::string&						getRoot() const;
	const std::string&						getIndex() const;
	size_t									getMaxBodySize() const;
	const std::map<int, std::string>&		getErrorPages() const;
	const std::vector<LocationConfig>&		getLocations() const;
	bool									getAutoindex() const;

	// Setters
	void									setHost(const std::string& host);
	void									setPort(int port);
	void									addServerName(const std::string& name);
	void									setRoot(const std::string& root);
	void									setIndex(const std::string& index);
	void									setMaxBodySize(size_t size);
	void									addErrorPage(int code, const std::string& path);
	void									addLocation(const LocationConfig& location);
	void									setAutoindex(bool autoindex);

	// Methods
	std::string								getErrorPage(int code) const;
	const LocationConfig*					findLocation(const std::string& uri) const;
	bool									matchServerName(const std::string& host) const;
	void									clear();
	bool									isValid() const;

private:
	std::string								_host;
	int										_port;
	std::vector<std::string>				_serverNames;
	std::string								_root;
	std::string								_index;
	size_t									_maxBodySize;
	std::map<int, std::string>				_errorPages;
	std::vector<LocationConfig>				_locations;
	bool									_autoindex;
};

#endif
