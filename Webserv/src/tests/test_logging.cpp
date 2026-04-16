#include "../../inc/utils/Utils.hpp"
#include <iostream>

/**
 * TestLogging: Clase de prueba para verificar la salida por consola.
 * Cumple con el estándar C++98 y las reglas del proyecto[cite: 39, 40].
 */
class TestLogging {
public:
    static void runAllTests() {
        std::cout << "--- INICIO DE PRUEBAS DE LOGGING ---" << std::endl << std::endl;

        // 1. Probar niveles básicos de log
        Utils::logInfo("Este es un mensaje de informacion (verde).");
        Utils::logWarning("Este es un mensaje de advertencia (amarillo).");
        Utils::logDebug("Este es un mensaje de depuracion (cian).");
        Utils::logError("Este es un mensaje de error critico (rojo).");

        std::cout << std::endl << "--- PROBANDO LOGREQUEST (HTTP CODES) ---" << std::endl;

        // 2. Probar logRequest con diferentes códigos de estado HTTP
        // Basado en la lógica de colores dinámica
        Utils::logRequest("GET", "/index.html", 200);   // Éxito: Verde
        Utils::logRequest("POST", "/upload", 201);     // Creado: Verde
        Utils::logRequest("GET", "/old-page", 301);    // Redirección: Amarillo
        Utils::logRequest("GET", "/private", 403);     // Prohibido: Rojo
        Utils::logRequest("DELETE", "/database", 500); // Error de Servidor: Rojo

        std::cout << std::endl << "--- PRUEBAS COMPLETADAS ---" << std::endl;
    }
};

int main() {
    try {
        TestLogging::runAllTests();
    } catch (const std::exception& e) {
        // Aunque logError usa cerr, aquí imprimimos fallos del test mismo
        std::cerr << "Error inesperado durante los tests: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
