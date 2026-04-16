/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 00:00:00 by mmarinov          #+#    #+#             */
/*   Updated: 2026/04/10 20:03:49 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include <string>
# include <vector>
# include <map>
# include <set>

class LocationConfig {
public:
	LocationConfig();
	LocationConfig(const LocationConfig& other);
	LocationConfig& operator=(const LocationConfig& other);
	~LocationConfig();

	// Getters
	const std::string&					getPath() const;
	const std::string&					getRoot() const;
	const std::string&					getIndex() const;
	const std::string&					getUploadPath() const;
	const std::string&					getRedirect() const;
	int									getRedirectCode() const;
	bool								getAutoindex() const;
	bool								getUploadEnabled() const;
	const std::set<std::string>&		getAllowedMethods() const;
	const std::map<std::string, std::string>&	getCgiHandlers() const;
	size_t								getMaxBodySize() const;
	const std::string&					getAlias() const;

	// Setters
	void								setPath(const std::string& path);
	void								setRoot(const std::string& root);
	void								setIndex(const std::string& index);
	void								setUploadPath(const std::string& path);
	void								setRedirect(const std::string& redirect);
	void								setRedirectCode(int code);
	void								setAutoindex(bool autoindex);
	void								setUploadEnabled(bool enabled);
	void								addAllowedMethod(const std::string& method);
	void								addCgiHandler(const std::string& extension, const std::string& handler);
	void								setMaxBodySize(size_t size);
	void								setAlias(const std::string& alias);

	// Methods
	bool								isMethodAllowed(const std::string& method) const;
	bool								isCgiExtension(const std::string& extension) const;
	std::string							getCgiHandler(const std::string& extension) const;
	bool								hasRedirect() const;
	void								clear();
	void								clearAllowedMethods();

private:
	std::string							_path;
	std::string							_root;
	std::string							_index;
	std::string							_uploadPath;
	std::string							_redirect;
	int									_redirectCode;
	bool								_autoindex;
	bool								_uploadEnabled;
	std::set<std::string>				_allowedMethods;
	std::map<std::string, std::string>	_cgiHandlers;
	size_t								_maxBodySize;
	std::string							_alias;
};

#endif
