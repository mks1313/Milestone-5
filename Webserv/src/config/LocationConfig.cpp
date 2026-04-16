/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/01 00:00:00 by fcela-ga          #+#    #+#             */
/*   Updated: 2026/04/10 20:04:39 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/config/LocationConfig.hpp"

LocationConfig::LocationConfig()
	: _path("/")
	, _root("")
	, _index("index.html")
	, _uploadPath("")
	, _redirect("")
	, _redirectCode(0)
	, _autoindex(false)
	, _uploadEnabled(false)
	, _maxBodySize(0)
	, _alias("")
{
	// Default allowed methods
	_allowedMethods.insert("GET");
	_allowedMethods.insert("POST");
	_allowedMethods.insert("DELETE");
}

LocationConfig::LocationConfig(const LocationConfig& other)
	: _path(other._path)
	, _root(other._root)
	, _index(other._index)
	, _uploadPath(other._uploadPath)
	, _redirect(other._redirect)
	, _redirectCode(other._redirectCode)
	, _autoindex(other._autoindex)
	, _uploadEnabled(other._uploadEnabled)
	, _allowedMethods(other._allowedMethods)
	, _cgiHandlers(other._cgiHandlers)
	, _maxBodySize(other._maxBodySize)
	, _alias(other._alias)
{
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
	if (this != &other) {
		_path = other._path;
		_root = other._root;
		_index = other._index;
		_uploadPath = other._uploadPath;
		_redirect = other._redirect;
		_redirectCode = other._redirectCode;
		_autoindex = other._autoindex;
		_uploadEnabled = other._uploadEnabled;
		_allowedMethods = other._allowedMethods;
		_cgiHandlers = other._cgiHandlers;
		_maxBodySize = other._maxBodySize;
		_alias = other._alias;
	}
	return *this;
}

LocationConfig::~LocationConfig() {
}

// ============================================================================
// Getters
// ============================================================================

const std::string& LocationConfig::getPath() const {
	return _path;
}

const std::string& LocationConfig::getRoot() const {
	return _root;
}

const std::string& LocationConfig::getIndex() const {
	return _index;
}

const std::string& LocationConfig::getUploadPath() const {
	return _uploadPath;
}

const std::string& LocationConfig::getRedirect() const {
	return _redirect;
}

int LocationConfig::getRedirectCode() const {
	return _redirectCode;
}

bool LocationConfig::getAutoindex() const {
	return _autoindex;
}

bool LocationConfig::getUploadEnabled() const {
	return _uploadEnabled;
}

const std::set<std::string>& LocationConfig::getAllowedMethods() const {
	return _allowedMethods;
}

const std::map<std::string, std::string>& LocationConfig::getCgiHandlers() const {
	return _cgiHandlers;
}

size_t LocationConfig::getMaxBodySize() const {
	return _maxBodySize;
}

const std::string& LocationConfig::getAlias() const {
	return _alias;
}

// ============================================================================
// Setters
// ============================================================================

void LocationConfig::setPath(const std::string& path) {
	_path = path;
}

void LocationConfig::setRoot(const std::string& root) {
	_root = root;
}

void LocationConfig::setIndex(const std::string& index) {
	_index = index;
}

void LocationConfig::setUploadPath(const std::string& path) {
	_uploadPath = path;
}

void LocationConfig::setRedirect(const std::string& redirect) {
	_redirect = redirect;
}

void LocationConfig::setRedirectCode(int code) {
	_redirectCode = code;
}

void LocationConfig::setAutoindex(bool autoindex) {
	_autoindex = autoindex;
}

void LocationConfig::setUploadEnabled(bool enabled) {
	_uploadEnabled = enabled;
}

void LocationConfig::addAllowedMethod(const std::string& method) {
	_allowedMethods.insert(method);
}

void LocationConfig::addCgiHandler(const std::string& extension, const std::string& handler) {
	_cgiHandlers[extension] = handler;
}

void LocationConfig::setMaxBodySize(size_t size) {
	_maxBodySize = size;
}

void LocationConfig::setAlias(const std::string& alias) {
	_alias = alias;
}

// ============================================================================
// Methods
// ============================================================================

bool LocationConfig::isMethodAllowed(const std::string& method) const {
	return _allowedMethods.find(method) != _allowedMethods.end();
}

bool LocationConfig::isCgiExtension(const std::string& extension) const {
	std::string ext = extension;
	if (!ext.empty() && ext[0] != '.')
		ext = "." + ext;
	return _cgiHandlers.find(ext) != _cgiHandlers.end();
}

std::string LocationConfig::getCgiHandler(const std::string& extension) const {
	std::string ext = extension;
	if (!ext.empty() && ext[0] != '.')
		ext = "." + ext;
	std::map<std::string, std::string>::const_iterator it = _cgiHandlers.find(ext);
	if (it != _cgiHandlers.end())
		return it->second;
	return "";
}

bool LocationConfig::hasRedirect() const {
	return !_redirect.empty();
}

void LocationConfig::clear() {
	_path = "/";
	_root = "";
	_index = "index.html";
	_uploadPath = "";
	_redirect = "";
	_redirectCode = 0;
	_autoindex = false;
	_uploadEnabled = false;
	_allowedMethods.clear();
	_allowedMethods.insert("GET");
	_allowedMethods.insert("POST");
	_allowedMethods.insert("DELETE");
	_cgiHandlers.clear();
	_maxBodySize = 0;
	_alias = "";
}

void LocationConfig::clearAllowedMethods() {
	_allowedMethods.clear();
}
