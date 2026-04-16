/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 22:40:31 by victor            #+#    #+#             */
/*   Updated: 2026/04/10 19:05:54 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <vector>
# include <sstream>
# include <ctime>
# include <sys/stat.h>
# include <cstdlib>
# include <iostream>
# include <fstream>
# include <algorithm>
# include <cctype>

namespace Utils {
	// String utilities
	std::string					trim(const std::string& str);
	std::string					toLower(const std::string& str);
	std::string					toUpper(const std::string& str);
	std::vector<std::string>	split(const std::string& str, char delimiter);
	std::vector<std::string>	split(const std::string& str, const std::string& delimiter);
	bool						startsWith(const std::string& str, const std::string& prefix);
	bool						endsWith(const std::string& str, const std::string& suffix);
	std::string					replaceAll(const std::string& str, const std::string& from, const std::string& to);

	// Number conversions
	int							stringToInt(const std::string& str);
	size_t						stringToSizeT(const std::string& str);
	std::string					intToString(int n);
	std::string					sizeTToString(size_t n);
	size_t						hexToSizeT(const std::string& hex);

	// File utilities
	bool						fileExists(const std::string& path);
	bool						isDirectory(const std::string& path);
	bool						isReadable(const std::string& path);
	bool						isWritable(const std::string& path);
	bool						isExecutable(const std::string& path);
	size_t						getFileSize(const std::string& path);
	std::string					getFileExtension(const std::string& path);
	std::string					getFileName(const std::string& path);
	std::string					getDirectory(const std::string& path);
	std::string					readFile(const std::string& path);
	bool						writeFile(const std::string& path, const std::string& content);
	bool						deleteFile(const std::string& path);
	bool						createDirectory(const std::string& path);
	std::string					normalizePath(const std::string& path);
	std::string					joinPath(const std::string& base, const std::string& path);

	// HTTP utilities
	std::string					urlDecode(const std::string& str);
	std::string					urlEncode(const std::string& str);
	std::string					getHttpDate();
	std::string					getHttpDate(time_t timestamp);
	std::string					getStatusMessage(int code);
	bool						isValidMethod(const std::string& method);

	// Logging
	void						logInfo(const std::string& msg);
	void						logWarning(const std::string& msg);
	void						logError(const std::string& msg);
	void						logDebug(const std::string& msg);
	void						logRequest(const std::string& method, const std::string& uri, int code);

	// Random utilities
	std::string					generateSessionId();
	std::string					generateBoundary();
}

#endif
