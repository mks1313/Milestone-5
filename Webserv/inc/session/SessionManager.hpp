/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 13:48:35 by victor            #+#    #+#             */
/*   Updated: 2026/04/10 20:02:42 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSIONMANAGER_HPP
# define SESSIONMANAGER_HPP

# include <string>
# include <map>
# include <ctime>

# define SESSION_COOKIE_NAME "WEBSERV_SESSION"
# define SESSION_TIMEOUT 3600
# define SESSION_ID_LENGTH 32

struct Session {
	std::string						id;
	std::map<std::string, std::string>	data;
	time_t							createdAt;
	time_t							lastAccessedAt;

	Session() : createdAt(0), lastAccessedAt(0) {}
};

class SessionManager {
public:
	// Singleton access
	static SessionManager&			getInstance();

	// Session management
	std::string						createSession();
	bool							sessionExists(const std::string& sessionId) const;
	Session*						getSession(const std::string& sessionId);
	const Session*					getSession(const std::string& sessionId) const;
	void							destroySession(const std::string& sessionId);

	// Session data operations
	void							set(const std::string& sessionId,
										const std::string& key, const std::string& value);
	std::string						get(const std::string& sessionId,
										const std::string& key) const;
	bool							has(const std::string& sessionId,
										const std::string& key) const;
	void							remove(const std::string& sessionId,
										const std::string& key);

	// Maintenance
	void							cleanExpiredSessions();
	size_t							getSessionCount() const;

	// Settings
	void							setSessionTimeout(int timeout);
	int								getSessionTimeout() const;

private:
	SessionManager();
	~SessionManager();
	SessionManager(const SessionManager&);
	SessionManager& operator=(const SessionManager&);

	std::map<std::string, Session>	_sessions;
	int								_sessionTimeout;

	std::string						_generateSessionId() const;
	void							_updateLastAccessed(const std::string& sessionId);
};

#endif
