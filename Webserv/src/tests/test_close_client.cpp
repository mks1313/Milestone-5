// tests/test_close_client.cpp
// Compilar ejemplo:
// g++ -std=c++98 -Iinc tests/test_close_client.cpp obj/server/Server.o obj/server/Client.o obj/utils/Utils.o -o test_close_client
// Ajusta rutas a .o según tu build.

#include <iostream>
#include <cassert>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <cerrno>

#include "../../inc/server/Server.hpp"
#include "../../inc/server/Client.hpp"
#include "../../inc/utils/Utils.hpp"

// Helper: crear socketpair
static int createSocketPair(int &peer) {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        std::perror("socketpair");
        return -1;
    }
    peer = sv[1];
    return sv[0];
}

// Helper: comprobar si fd está cerrado
static bool isFdClosed(int fd) {
    return (fcntl(fd, F_GETFD) == -1 && errno == EBADF);
}

int main() {
    std::cout << "=== test_close_client (API real) ===" << std::endl;

    // Construye el Server real (ajusta si tu constructor necesita argumentos)
    Server server;

    // Crear socketpair para simular cliente
    int peer;
    int clientFd = createSocketPair(peer);
    if (clientFd < 0) return 2;

    std::cout << "Setup: created client fd " << clientFd << std::endl;

    // Registrar cliente en el Server usando la API de test que debe existir
    // Requisitos en Server:
    //   void test_registerClient(int clientFd);
    //   void test_closeClient(int clientFd);
    //   bool test_hasClient(int clientFd);
    //   bool test_pollContains(int clientFd);
    //
    // Estas funciones deben estar implementadas en Server.hpp/Server.cpp (por ejemplo bajo UNIT_TEST).
    server.test_registerClient(clientFd);

    // Pre-checks (si los helpers existen)
    assert(server.test_hasClient(clientFd));
    assert(server.test_pollContains(clientFd));
    assert(!isFdClosed(clientFd));

    // Invocar el cierre real
    server.test_closeClient(clientFd);

    // Verificaciones
    bool closed = isFdClosed(clientFd);
    bool removed = !server.test_hasClient(clientFd);
    bool pollRemoved = !server.test_pollContains(clientFd);

    std::cout << "Post-close: fd closed=" << (closed ? "yes" : "no")
              << ", client removed=" << (removed ? "yes" : "no")
              << ", poll removed=" << (pollRemoved ? "yes" : "no") << std::endl;

    assert(closed && removed && pollRemoved);

    // Idempotencia: llamar de nuevo no debe crashar
    server.test_closeClient(clientFd);
    std::cout << "Idempotency check: PASS" << std::endl;

    // Prueba con múltiples clientes
    int peer2, peer3;
    int fd2 = createSocketPair(peer2);
    int fd3 = createSocketPair(peer3);
    assert(fd2 >= 0 && fd3 >= 0);

    server.test_registerClient(fd2);
    server.test_registerClient(fd3);

    assert(server.test_hasClient(fd2) && server.test_hasClient(fd3));
    assert(server.test_pollContains(fd2) && server.test_pollContains(fd3));

    server.test_closeClient(fd2);
    server.test_closeClient(fd3);

    assert(!server.test_hasClient(fd2) && !server.test_hasClient(fd3));
    assert(isFdClosed(fd2) && isFdClosed(fd3));
    assert(!server.test_pollContains(fd2) && !server.test_pollContains(fd3));

    if (!isFdClosed(peer)) ::close(peer);
    if (!isFdClosed(peer2)) ::close(peer2);
    if (!isFdClosed(peer3)) ::close(peer3);

    std::cout << "All tests passed." << std::endl;
    return 0;
}
