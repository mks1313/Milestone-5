/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 19:43:55 by fcela-ga          #+#    #+#             */
/*   Updated: 2026/04/11 11:38:17 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/webserv.hpp"
#include "../inc/config/Config.hpp"
#include "../inc/server/Server.hpp"
#include "../inc/utils/Utils.hpp"
#include <csignal>
#include <cstdlib>
#include <iostream>

// Default configuration file path
static const char* DEFAULT_CONFIG_PATH = "./config/webserv.conf";

// Global server pointer for signal handling
static Server* g_server = NULL;

/**
 * Signal handler for graceful shutdown
 */
void signalHandler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM)
    {
        std::cout << std::endl;
        Utils::logInfo("Received signal " + Utils::intToString(signum) + ", shutting down...");
        if (g_server != NULL)
        {
            g_server->stop();
        }
    }
}

/**
 * Setup signal handlers
 */
void setupSignals(void)
{
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) < 0)
    {
        Utils::logError("Failed to set SIGINT handler");
    }

    if (sigaction(SIGTERM, &sa, NULL) < 0)
    {
        Utils::logError("Failed to set SIGTERM handler");
    }

    // Ignore SIGPIPE to prevent crashes when writing to closed sockets
    signal(SIGPIPE, SIG_IGN);
}

/**
 * Print usage information
 */
void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " [configuration file]" << std::endl;
    std::cout << std::endl;
    std::cout << "If no configuration file is specified, the default path is used:" << std::endl;
    std::cout << "  " << DEFAULT_CONFIG_PATH << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help     Display this help message" << std::endl;
    std::cout << "  -v, --version  Display version information" << std::endl;
}

/**
 * Print version information
 */
void printVersion(void)
{
    std::cout << "webserv " << WEBSERV_VERSION << std::endl;
    std::cout << "A simple HTTP/1.1 server written in C++98" << std::endl;
    std::cout << "Authors: fcela-ga, vberdugo and mmarinov" << std::endl;
}

/**
 * Print server banner
 */
void printBanner(void)
{
    std::cout << "\033[1;36m" << std::endl;
    std::cout << "╦ ╦╔═╗╔╗ ╔═╗╔═╗╦═╗╦  ╦" << std::endl;
    std::cout << "║║║║╣ ╠╩╗╚═╗║╣ ╠╦╝╚╗╔╝" << std::endl;
    std::cout << "╚╩╝╚═╝╚═╝╚═╝╚═╝╩╚═ ╚╝ " << std::endl;
    std::cout << "\033[0m" << std::endl;
    std::cout << "Version " << WEBSERV_VERSION << " - HTTP/1.1 Server" << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << std::endl;
}

/**
 * Main entry point
 */
int main(int argc, char* argv[])
{
    // Determine configuration file path
    std::string configPath = DEFAULT_CONFIG_PATH;

    if (argc > 1)
    {
        std::string arg = argv[1];

        // Check for help flag
        if (arg == "-h" || arg == "--help")
        {
            printUsage(argv[0]);
            return EXIT_SUCCESS;
        }

        // Check for version flag
        if (arg == "-v" || arg == "--version")
        {
            printVersion();
            return EXIT_SUCCESS;
        }

        // Assume it's a configuration file path
        configPath = arg;
    }

    // Print banner
    printBanner();

    // Check if configuration file exists
    if (!Utils::fileExists(configPath))
    {
        Utils::logError("Configuration file not found: " + configPath);
        return EXIT_FAILURE;
    }

    Utils::logInfo("Loading configuration from: " + configPath);

    // Parse configuration
    Config config;
    try
    {
        config.parse(configPath);
    }
    catch (const std::exception& e)
    {
        Utils::logError(std::string("Configuration error: ") + e.what());
        return EXIT_FAILURE;
    }

    Utils::logInfo("Configuration loaded successfully");
    Utils::logInfo("Number of servers configured: " +
                   Utils::intToString(config.getServers().size()));

    // Setup signal handlers
    setupSignals();

    // Create and initialize server
    Server server(config);
    g_server = &server;

    if (!server.init())
    {
        Utils::logError("Failed to initialize server");
        return EXIT_FAILURE;
    }

    Utils::logInfo("Server initialized successfully");
    Utils::logInfo("Press Ctrl+C to stop the server");
    std::cout << std::endl;

    // Run server
    server.run();

    // Cleanup
    g_server = NULL;
    Utils::logInfo("Server stopped");

    return EXIT_SUCCESS;
}
