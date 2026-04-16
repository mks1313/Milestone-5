#include "../../inc/config/Config.hpp"
#include <iostream>
#include <fstream>

/* COLORS */
#define GREEN  "\033[32m"
#define RED    "\033[31m"
#define CYAN   "\033[36m"
#define RESET  "\033[0m"

static int g_passed = 0;
static int g_failed = 0;

static void check(const std::string& label, bool ok)
{
	if (ok) {
		std::cout << GREEN << "✓ " << RESET << label << std::endl;
		++g_passed;
	} else {
		std::cout << RED   << "✗ " << RESET << label << std::endl;
		++g_failed;
	}
}

static std::string writeTmp(const std::string& filename, const std::string& content)
{
	std::string path = "/tmp/" + filename;
	std::ofstream f(path.c_str());
	f << content;
	return path;
}

/* -------------------------------------------------- */
/* TEST MINIMAL (solo lo seguro que compila)           */
/* -------------------------------------------------- */

static void testBasicParsing()
{
	std::cout << CYAN << "\n===== BASIC PARSE =====" << RESET << std::endl;

	const std::string conf =
		"server {\n"
		"    listen 8080;\n"
		"    server_name test.com;\n"
		"    root /var/www;\n"
		"}\n";

	std::string path = writeTmp("cfg_basic.conf", conf);
	Config cfg;

	try {
		cfg.parseConfigFile(path);

		check("Port == 8080", cfg.getPort() == 8080);
		check("Server name parsed", cfg.getServerNames().size() == 1);
		check("Root parsed", cfg.getRoot() == "/var/www");
		check("isValid()", cfg.isValid());

	} catch (...) {
		check("No exception thrown", false);
	}
}

/* -------------------------------------------------- */
/* TEST HOST:PORT                                     */
/* -------------------------------------------------- */

static void testHostPort()
{
	std::cout << CYAN << "\n===== HOST:PORT =====" << RESET << std::endl;

	const std::string conf =
		"server {\n"
		"    listen 127.0.0.1:4242;\n"
		"    root /tmp;\n"
		"}\n";

	std::string path = writeTmp("cfg_host.conf", conf);
	Config cfg;

	try {
		cfg.parseConfigFile(path);

		check("Host parsed", cfg.getHost() == "127.0.0.1");
		check("Port parsed", cfg.getPort() == 4242);

	} catch (...) {
		check("No exception thrown", false);
	}
}

/* -------------------------------------------------- */
/* TEST EXCEPTIONS                                    */
/* -------------------------------------------------- */

static void testException()
{
	std::cout << CYAN << "\n===== EXCEPTION =====" << RESET << std::endl;

	Config cfg;
	bool threw = false;

	try {
		cfg.parseConfigFile("/tmp/no_file.conf");
	} catch (...) {
		threw = true;
	}

	check("Exception on missing file", threw);
}

/* -------------------------------------------------- */
/* MAIN                                               */
/* -------------------------------------------------- */

int main()
{
	std::cout << CYAN << "===== CONFIG QUICK TEST =====" << RESET << std::endl;

	testBasicParsing();
	testHostPort();
	testException();

	std::cout << "\n";
	std::cout << GREEN << g_passed << " passed" << RESET
	          << " / "
	          << (g_failed ? RED : GREEN) << g_failed << " failed" << RESET
	          << std::endl;

	return (g_failed ? 1 : 0);
}

