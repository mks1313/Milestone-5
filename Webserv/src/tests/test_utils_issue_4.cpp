#include "../../inc/utils/Utils.hpp"
#include <iostream>
#include <string>

/**
 * @brief Unit tests for HTTP utility functions implemented for Issue #4.
 * Ensures compliance with C++98 and mandatory project requirements.
 */
int main() {
    int failed_tests = 0;

    std::cout << "--- Starting Tests for HTTP Utils (#4) ---" << std::endl;

    // 1. Test urlDecode
    std::cout << "[TEST] urlDecode: ";
    if (Utils::urlDecode("hello%20world%21") == "hello world!" &&
        Utils::urlDecode("path/to/file%2Btest") == "path/to/file+test") {
        std::cout << "OK" << std::endl;
    } else {
        std::cout << "FAIL" << std::endl;
        failed_tests++;
    }

    // 2. Test urlEncode
    std::cout << "[TEST] urlEncode: ";
    if (Utils::urlEncode("hello world!") == "hello%20world%21") {
        std::cout << "OK" << std::endl;
    } else {
        std::cout << "FAIL" << std::endl;
        failed_tests++;
    }

    // 3. Test getHttpDate (RFC 7231)
    std::cout << "[TEST] getHttpDate: ";
    std::string date = Utils::getHttpDate();
    if (date.length() > 20 && date.find("GMT") != std::string::npos) {
        std::cout << "OK (" << date << ")" << std::endl;
    } else {
        std::cout << "FAIL" << std::endl;
        failed_tests++;
    }

    // 4. Test getStatusMessage
    std::cout << "[TEST] getStatusMessage: ";
    if (Utils::getStatusMessage(200) == "OK" &&
        Utils::getStatusMessage(404) == "Not Found" &&
        Utils::getStatusMessage(413) == "Payload Too Large") {
        std::cout << "OK" << std::endl;
    } else {
        std::cout << "FAIL" << std::endl;
        failed_tests++;
    }

    // 5. Test isValidMethod (Mandatory: GET, POST, DELETE)
    std::cout << "[TEST] isValidMethod: ";
    if (Utils::isValidMethod("GET") &&
        Utils::isValidMethod("POST") &&
        Utils::isValidMethod("DELETE") &&
        !Utils::isValidMethod("PATCH") &&
        !Utils::isValidMethod("PUT")) {
        std::cout << "OK" << std::endl;
    } else {
        std::cout << "FAIL (Check mandatory methods requirement)" << std::endl;
        failed_tests++;
    }

    std::cout << "--- Finished Tests ---" << std::endl;
    if (failed_tests == 0) {
        std::cout << "Result: ALL TESTS PASSED" << std::endl;
        return 0;
    } else {
        std::cout << "Result: " << failed_tests << " TESTS FAILED" << std::endl;
        return 1;
    }
}
