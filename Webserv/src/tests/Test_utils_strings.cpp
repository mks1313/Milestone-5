#include "../../inc/utils/Utils.hpp"
#include <iostream>
#include <cassert>
#include <vector>

// Colores para el output (basados en webserv.hpp [cite: 147])
#define GREEN "\033[0;32m"
#define RED   "\033[0;31m"
#define RESET "\033[0m"

void test_trim() {
    std::cout << "Testing trim()... ";
    // Verifica limpieza de espacios, tabs y saltos de línea [cite: 880, 881]
    assert(Utils::trim("   hello   ") == "hello");
    assert(Utils::trim("\t\n world \r") == "world");
    assert(Utils::trim("   ") == "");
    assert(Utils::trim("") == "");
    std::cout << GREEN "OK" RESET << std::endl;
}

void test_case_conversion() {
    std::cout << "Testing toLower/toUpper()... ";
    // Verifica conversión y persistencia de caracteres no alfabéticos [cite: 882-884]
    assert(Utils::toLower("HeLLo") == "hello");
    assert(Utils::toUpper("hello") == "HELLO");

    // Verificación de casting seguro (unsigned char) para evitar Undefined Behavior
    assert(Utils::toLower("123!@#") == "123!@#");
    std::cout << GREEN "OK" RESET << std::endl;
}

void test_start_end_with() {
    std::cout << "Testing startsWith/endsWith()... ";
    // Validación de prefijos y sufijos con gestión de tamaño [cite: 887-889]
    assert(Utils::startsWith("GET /index.html", "GET ") == true);
    assert(Utils::startsWith("hello", "hellow") == false);
    assert(Utils::endsWith("image.png", ".png") == true);
    assert(Utils::endsWith("a", "abc") == false);
    std::cout << GREEN "OK" RESET << std::endl;
}

void test_split() {
    std::cout << "Testing split()... ";

    // Split por CHAR: Ahora CONSISTENTE (no ignora vacíos )
    std::vector<std::string> v1 = Utils::split("a,,b", ',');
    assert(v1.size() == 3);
    assert(v1[0] == "a" && v1[1] == "" && v1[2] == "b");

    // Split por STRING: Lógica robusta del desarrollo original [cite: 886, 887]
    std::vector<std::string> v2 = Utils::split("NAME--VALUE--", "--");
    assert(v2.size() == 3);
    assert(v2[0] == "NAME" && v2[1] == "VALUE" && v2[2] == "");

    std::cout << GREEN "OK" RESET << std::endl;
}

void test_replace_all() {
    std::cout << "Testing replaceAll()... ";

    // Caso estándar de reemplazo múltiple [cite: 889, 890]
    assert(Utils::replaceAll("aaa", "a", "bb") == "bbbbbb");

    // PRUEBA DE SEGURIDAD CRÍTICA: Evitar bucle infinito si 'from' es vacío
    // Si la corrección 'if (from.empty())' no estuviera, el test se congelaría aquí.
    assert(Utils::replaceAll("test", "", "error") == "test");

    std::cout << GREEN "OK" RESET << std::endl;
}

int main() {
    std::cout << GREEN "=== STARTING UTILS TESTS (C++98 COMPLIANT) ===" RESET << std::endl;

    try {
        test_trim();
        test_case_conversion();
        test_start_end_with();
        test_split();
        test_replace_all();

        std::cout << GREEN "\nCONGRATULATIONS: ALL TESTS PASSED!" RESET << std::endl;
    } catch (...) {
        std::cerr << RED "\nCRITICAL FAILURE: A test did not pass." RESET << std::endl;
        return 1;
    }

    return 0;
}
