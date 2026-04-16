/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 13:48:57 by victor            #+#    #+#             */
/*   Updated: 2026/04/10 20:01:44 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/session/SessionManager.hpp"
#include <cstdlib>
#include <sstream>

// Characters for session ID generation
static const char SESSION_ID_CHARS[] =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// ============================================================================
// Singleton access
// ============================================================================

SessionManager& SessionManager::getInstance()
{
    static SessionManager instance;
    return instance;
}

// ============================================================================
// Constructors / Destructor
// ============================================================================

SessionManager::SessionManager() : _sessionTimeout(SESSION_TIMEOUT)
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));
}

SessionManager::~SessionManager()
{
}

SessionManager::SessionManager(const SessionManager& other)
{
    (void)other;
}

SessionManager& SessionManager::operator=(const SessionManager& other)
{
    (void)other;
    return *this;
}

// ============================================================================
// Private helpers
// ============================================================================

std::string SessionManager::_generateSessionId() const
{
    std::string sessionId;
    sessionId.reserve(SESSION_ID_LENGTH);

    const size_t charsLen = sizeof(SESSION_ID_CHARS) - 1;

    for (int i = 0; i < SESSION_ID_LENGTH; ++i)
    {
        sessionId += SESSION_ID_CHARS[std::rand() % charsLen];
    }

    return sessionId;
}

void SessionManager::_updateLastAccessed(const std::string& sessionId)
{
    std::map<std::string, Session>::iterator it = _sessions.find(sessionId);
    if (it != _sessions.end())
    {
        it->second.lastAccessedAt = std::time(NULL);
    }
}

// ============================================================================
// Session management
// ============================================================================

std::string SessionManager::createSession()
{
    // Clean up expired sessions first
    cleanExpiredSessions();

    // Generate unique session ID
    std::string sessionId;
    do {
        sessionId = _generateSessionId();
    } while (_sessions.find(sessionId) != _sessions.end());

    // Create new session
    Session newSession;
    newSession.id = sessionId;
    newSession.createdAt = std::time(NULL);
    newSession.lastAccessedAt = newSession.createdAt;

    _sessions[sessionId] = newSession;

    return sessionId;
}

bool SessionManager::sessionExists(const std::string& sessionId) const
{
    std::map<std::string, Session>::const_iterator it = _sessions.find(sessionId);
    if (it == _sessions.end())
        return false;

    // Check if session has expired
    time_t now = std::time(NULL);
    if (now - it->second.lastAccessedAt > _sessionTimeout)
        return false;

    return true;
}

Session* SessionManager::getSession(const std::string& sessionId)
{
    if (!sessionExists(sessionId))
        return NULL;

    _updateLastAccessed(sessionId);
    return &_sessions[sessionId];
}

const Session* SessionManager::getSession(const std::string& sessionId) const
{
    std::map<std::string, Session>::const_iterator it = _sessions.find(sessionId);
    if (it == _sessions.end())
        return NULL;

    // Check if session has expired
    time_t now = std::time(NULL);
    if (now - it->second.lastAccessedAt > _sessionTimeout)
        return NULL;

    return &it->second;
}

void SessionManager::destroySession(const std::string& sessionId)
{
    _sessions.erase(sessionId);
}

// ============================================================================
// Session data operations
// ============================================================================

void SessionManager::set(const std::string& sessionId,
                         const std::string& key, const std::string& value)
{
    Session* session = getSession(sessionId);
    if (session != NULL)
    {
        session->data[key] = value;
    }
}

std::string SessionManager::get(const std::string& sessionId,
                                const std::string& key) const
{
    const Session* session = getSession(sessionId);
    if (session == NULL)
        return "";

    std::map<std::string, std::string>::const_iterator it = session->data.find(key);
    if (it == session->data.end())
        return "";

    return it->second;
}

bool SessionManager::has(const std::string& sessionId, const std::string& key) const
{
    const Session* session = getSession(sessionId);
    if (session == NULL)
        return false;

    return session->data.find(key) != session->data.end();
}

void SessionManager::remove(const std::string& sessionId, const std::string& key)
{
    Session* session = getSession(sessionId);
    if (session != NULL)
    {
        session->data.erase(key);
    }
}

// ============================================================================
// Maintenance
// ============================================================================

void SessionManager::cleanExpiredSessions()
{
    time_t now = std::time(NULL);

    std::map<std::string, Session>::iterator it = _sessions.begin();
    while (it != _sessions.end())
    {
        if (now - it->second.lastAccessedAt > _sessionTimeout)
        {
            std::map<std::string, Session>::iterator toErase = it;
            ++it;
            _sessions.erase(toErase);
        }
        else
        {
            ++it;
        }
    }
}

size_t SessionManager::getSessionCount() const
{
    return _sessions.size();
}

// ============================================================================
// Settings
// ============================================================================

void SessionManager::setSessionTimeout(int timeout)
{
    if (timeout > 0)
        _sessionTimeout = timeout;
}

int SessionManager::getSessionTimeout() const
{
    return _sessionTimeout;
}
