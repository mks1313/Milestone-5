/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/23 13:13:53 by vberdugo          #+#    #+#             */
/*   Updated: 2026/04/10 20:35:31 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

// C++ Standard Library Headers
# include <iostream>
# include <string>
# include <sstream>
# include <fstream>
# include <vector>
# include <map>
# include <set>
# include <algorithm>
# include <cstdlib>
# include <cstring>
# include <ctime>
# include <cerrno>
# include <climits>

// C System Headers
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <unistd.h>
# include <fcntl.h>
# include <poll.h>
# include <signal.h>
# include <dirent.h>

// Project Headers (organized by module)
# include "../inc/utils/Utils.hpp"
# include "../inc/config/Config.hpp"
# include "../inc/config/ServerConfig.hpp"
# include "../inc/config/LocationConfig.hpp"
# include "../inc/http/Request.hpp"
# include "../inc/http/Response.hpp"
# include "../inc/server/Client.hpp"
# include "../inc/server/Server.hpp"
# include "../inc/cgi/CGIHandler.hpp"
# include "../inc/session/SessionManager.hpp"
# include "../inc/http/MimeTypes.hpp"

// Constants
# define WEBSERV_VERSION "1.0.0"
# define BUFFER_SIZE 65536
# define MAX_CLIENTS 1024
# define MAX_HEADER_SIZE 8192
# define MAX_URI_LENGTH 8192
# define DEFAULT_MAX_BODY_SIZE 1048576
# define MAX_CGI_OUTPUT_SIZE 200000000
# define MAX_CONCURRENT_CGI 5
# define CONNECTION_TIMEOUT 60
# define CGI_TIMEOUT 120
# define CGI_RESPONSE_TIMEOUT 180
# define BACKLOG 128

// HTTP Status Codes
# define HTTP_OK 200
# define HTTP_CREATED 201
# define HTTP_NO_CONTENT 204
# define HTTP_MOVED_PERMANENTLY 301
# define HTTP_FOUND 302
# define HTTP_NOT_MODIFIED 304
# define HTTP_TEMPORARY_REDIRECT 307
# define HTTP_BAD_REQUEST 400
# define HTTP_UNAUTHORIZED 401
# define HTTP_FORBIDDEN 403
# define HTTP_NOT_FOUND 404
# define HTTP_METHOD_NOT_ALLOWED 405
# define HTTP_REQUEST_TIMEOUT 408
# define HTTP_CONFLICT 409
# define HTTP_LENGTH_REQUIRED 411
# define HTTP_PAYLOAD_TOO_LARGE 413
# define HTTP_URI_TOO_LONG 414
# define HTTP_UNSUPPORTED_MEDIA_TYPE 415
# define HTTP_INTERNAL_SERVER_ERROR 500
# define HTTP_NOT_IMPLEMENTED 501
# define HTTP_BAD_GATEWAY 502
# define HTTP_SERVICE_UNAVAILABLE 503
# define HTTP_GATEWAY_TIMEOUT 504
# define HTTP_VERSION_NOT_SUPPORTED 505

// Colors for logging
# define RED "\033[0;31m"
# define GREEN "\033[0;32m"
# define YELLOW "\033[0;33m"
# define BLUE "\033[0;34m"
# define MAGENTA "\033[0;35m"
# define CYAN "\033[0;36m"
# define RESET "\033[0m"

// Global signal flag
extern volatile sig_atomic_t g_running;

#endif
