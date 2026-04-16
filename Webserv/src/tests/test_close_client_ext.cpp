// test_close_client.cpp
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <poll.h>

class Utils {
public:
    static std::string intToString(int v) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", v);
        return std::string(buf);
    }
    static void logDebug(const std::string& s) {
        std::cerr << "[DEBUG] " << s << std::endl;
    }
};

// Minimal Client stub storing CGI info placeholders
class Client {
public:
    Client(int fd, int cgiFdOut = -1, pid_t cgiPid = 0)
        : _fd(fd), _cgiFdOut(cgiFdOut), _cgiPid(cgiPid) {}

    int getFd() const { return _fd; }
    int getCgiFdOut() const { return _cgiFdOut; }
    pid_t getCgiPid() const { return _cgiPid; }

private:
    int _fd;
    int _cgiFdOut;
    pid_t _cgiPid;
};

// Minimal Server with poll vector and clients map
class Server {
public:
    Server() : _activeCgiCount(0) {}

    // Add a client (simulate accept)
    void addClient(int clientFd, const Client& client) {
        struct pollfd p;
        p.fd = clientFd;
        p.events = POLLIN | POLLOUT;
        p.revents = 0;
        _pollFds.push_back(p);
        _clients[clientFd] = client;
    }

    // Remove fd from poll vector (swap+pop)
    void _removefromPoll(int fd) {
        for (size_t i = 0; i < _pollFds.size(); ++i) {
            if (_pollFds[i].fd == fd) {
                if (i + 1 != _pollFds.size())
                    std::swap(_pollFds[i], _pollFds.back());
                _pollFds.pop_back();
                return;
            }
        }
    }

    // The function under test (adapted to minimal environment)
    void _closeClient(int clientFd) {
        std::map<int, Client>::iterator it = _clients.find(clientFd);
        if (it == _clients.end())
            return;

        Client& client = it->second;

        // 1) Remove from poll set so poll() won't see it
        _removefromPoll(clientFd);

        // 2) Clean up CGI if any (in this minimal test we just close cgi fd if present)
        bool hadActiveCgi = false;

        int cgiFdOut = client.getCgiFdOut();
        if (cgiFdOut >= 0) {
            ::close(cgiFdOut);
            hadActiveCgi = true;
        }

        pid_t pid = client.getCgiPid();
        if (pid > 0) {
            kill(pid, SIGTERM);
            waitpid(pid, NULL, WNOHANG);
            hadActiveCgi = true;
        }

        // 3) Close client socket
        Utils::logDebug("Closing connection fd " + Utils::intToString(clientFd));
        ::close(clientFd);

        // 4) Erase client from map
        _clients.erase(clientFd);

        // 5) Post-CGI adjustments (minimal)
        if (hadActiveCgi && _activeCgiCount > 0) {
            --_activeCgiCount;
            Utils::logDebug("Client with CGI closed, active CGI count: " + Utils::intToString(_activeCgiCount));
            // _processNextCgiFromQueue(); // not implemented in minimal test
        }
    }

    // Helpers for tests
    bool hasClient(int fd) const {
        return _clients.find(fd) != _clients.end();
    }
    bool pollContains(int fd) const {
        for (size_t i = 0; i < _pollFds.size(); ++i)
            if (_pollFds[i].fd == fd) return true;
        return false;
    }
    size_t clientCount() const { return _clients.size(); }
    size_t pollCount() const { return _pollFds.size(); }

private:
    std::vector<struct pollfd> _pollFds;
    std::map<int, Client> _clients;
    size_t _activeCgiCount;
};

// Utility to check if fd is closed (fcntl returns -1 and errno == EBADF)
bool isFdClosed(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1 && errno == EBADF) return true;
    return false;
}

// Create a socketpair and return the server side fd (other end is client)
int createSocketPair(int& peerFd) {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        perror("socketpair");
        return -1;
    }
    // sv[0] and sv[1] are connected; choose sv[0] as server side
    peerFd = sv[1];
    return sv[0];
}

int main() {
    Server server;

    // Test 1: Close single client
    int peer1;
    int fd1 = createSocketPair(peer1);
    assert(fd1 >= 0);
    server.addClient(fd1, Client(fd1));
    std::cout << "Test 1 setup: added client fd " << fd1 << std::endl;

    // Pre-conditions
    assert(server.hasClient(fd1));
    assert(server.pollContains(fd1));
    assert(!isFdClosed(fd1));

    server._closeClient(fd1);

    // Post-conditions
    bool pass1 = !server.hasClient(fd1) && !server.pollContains(fd1) && isFdClosed(fd1);
    std::cout << "Test 1 close single client: " << (pass1 ? "PASS" : "FAIL") << std::endl;

    // Clean peer end
    if (!isFdClosed(peer1)) ::close(peer1);

    // Test 2: Idempotent close (calling again should be safe)
    server._closeClient(fd1); // should do nothing and not crash
    std::cout << "Test 2 idempotent close: PASS (no crash on repeated close)" << std::endl;

    // Test 3: Multiple clients close
    int peer2, peer3;
    int fd2 = createSocketPair(peer2);
    int fd3 = createSocketPair(peer3);
    assert(fd2 >= 0 && fd3 >= 0);
    server.addClient(fd2, Client(fd2));
    server.addClient(fd3, Client(fd3));
    std::cout << "Test 3 setup: added clients fds " << fd2 << " and " << fd3 << std::endl;

    // Pre-conditions
    assert(server.clientCount() == 2);
    assert(server.pollContains(fd2) && server.pollContains(fd3));

    server._closeClient(fd2);
    server._closeClient(fd3);

    bool pass3 = (server.clientCount() == 0) && !server.pollContains(fd2) && !server.pollContains(fd3)
                 && isFdClosed(fd2) && isFdClosed(fd3);
    std::cout << "Test 3 close multiple clients: " << (pass3 ? "PASS" : "FAIL") << std::endl;

    // Clean peers
    if (!isFdClosed(peer2)) ::close(peer2);
    if (!isFdClosed(peer3)) ::close(peer3);

    // Summary
    if (pass1 && pass3) {
        std::cout << "All tests passed." << std::endl;
        return 0;
    } else {
        std::cout << "Some tests failed." << std::endl;
        return 1;
    }
}
