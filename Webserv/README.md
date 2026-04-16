*This project has been created as part of the 42 curriculum by fcela-ga, vberdugo and mmarinov.*

# Webserv

> "This is when you finally understand why URLs start with HTTP"

---

## Description

**Webserv** is a fully functional HTTP/1.1 server written in C++98, developed as part of the 42 school curriculum. The goal of the project is to deeply understand the inner workings of the HyperText Transfer Protocol — the foundation of data communication on the World Wide Web — by building a server from scratch, without relying on any external library.

The server handles real HTTP requests from actual web browsers, supports multiple simultaneous clients through a non-blocking I/O model using `poll()` (or equivalent), and is configurable via an NGINX-inspired configuration file.

### Key features

- Non-blocking I/O driven by a single `poll()` event loop (covering all sockets, including the listening socket)
- Support for **GET**, **POST**, and **DELETE** HTTP methods
- Serving fully static websites
- File upload from clients
- CGI execution based on file extension (Python, PHP, shell scripts, and `.bla` via a custom CGI tester)
- Multiple virtual servers listening on different ports and/or `server_name` values
- Per-route configuration: accepted methods, root/alias directory, redirections, directory listing, default index file, upload storage, and body size limits
- Accurate HTTP response status codes and default error pages
- Resilience: the server never crashes or hangs, regardless of client behaviour

---

## Instructions

### Requirements

- A C++98-compatible compiler (`c++` with `-Wall -Wextra -Werror -std=c++98`)
- A Unix-based operating system (Linux or macOS)
- Python 3 and/or PHP (for CGI support, optional at runtime)

### Compilation

```bash
make
```

This produces the `webserv` executable. Additional Makefile rules available:

| Rule | Description |
|------|-------------|
| `make all` | Compile the project |
| `make clean` | Remove object files |
| `make fclean` | Remove object files and the binary |
| `make re` | Full recompilation from scratch |

### Running the server

```bash
./webserv [configuration_file]
```

If no configuration file is provided, the server falls back to a default path. A ready-to-use configuration file compatible with the 42 campus tester is provided at `config/webserv.conf`.

**Example:**

```bash
./webserv config/webserv.conf
```

The server will then be accessible at `http://localhost:8080` (or whichever port is set in the configuration file).

### Configuration file overview

The configuration format is inspired by NGINX's `server` blocks. A minimal example:

```nginx
server {
    listen 0.0.0.0:8080;
    server_name localhost;
    root ./www;
    index index.html;
    client_max_body_size 1048576;

    location / {
        methods GET;
        autoindex off;
    }

    location /upload {
        methods GET POST DELETE;
        upload_store ./www/uploads;
    }

    location /cgi-bin {
        methods GET POST;
        alias ./cgi-bin;
        cgi .py /usr/bin/python3;
        cgi .php /usr/bin/php-cgi;
    }
}
```

Supported directives include: `listen`, `server_name`, `root`, `alias`, `index`, `client_max_body_size`, `autoindex`, `error_page`, `methods`, `redirect`, `upload_store`, and `cgi`.

### Testing with the 42 campus tester

The provided `config/webserv.conf` is pre-configured to satisfy all mandatory tester requirements:

1. `GET /` — root location responds only to GET (returns 405 for other methods)
2. `PUT /put_test/*` — accepts PUT requests and saves files
3. `POST /directory/*.bla` — routes `.bla` files through `ubuntu_cgi_tester`
4. `POST /post_body` — enforces a maximum body size of 100 bytes
5. `GET /directory/` — serves the `YoupiBanane` directory with `youpi.bad_extension` as default index

The server also listens on ports `8081` and `8082` for secondary and restricted configurations respectively.

### Manual testing with telnet

```bash
telnet localhost 8080
GET / HTTP/1.1
Host: localhost

```

---

## Technical Choices

| Concern | Choice | Rationale |
|---------|--------|-----------|
| I/O multiplexing | `poll()` | Portable across Linux and macOS; monitors read and write events simultaneously |
| Concurrency model | Single event loop, no threads | Avoids race conditions and satisfies the subject's "one poll()" constraint |
| CGI execution | `fork()` + `execve()` | The only context where `fork()` is permitted per the subject rules |
| Config parsing | Custom recursive-descent parser | Full control over NGINX-like block syntax without external libraries |
| C++ standard | C++98 | Mandatory per subject; STL containers and algorithms used throughout |

---

## Resources

### Official specifications and documentation

- [RFC 7230 — HTTP/1.1: Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230) — Primary reference for request/response framing, chunked transfer encoding, and header fields
- [RFC 7231 — HTTP/1.1: Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231) — Method semantics (GET, POST, DELETE), status codes, and content negotiation
- [RFC 3875 — The Common Gateway Interface (CGI/1.1)](https://datatracker.ietf.org/doc/html/rfc3875) — Environment variables and I/O conventions for CGI scripts
- [NGINX Documentation — Server and Location Blocks](https://nginx.org/en/docs/http/ngx_http_core_module.html) — Used as a behavioural reference and configuration inspiration throughout the project
- [MDN Web Docs — HTTP overview](https://developer.mozilla.org/en-US/docs/Web/HTTP/Overview) — Accessible introduction to HTTP concepts consulted during early design phases
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — Essential reference for socket programming in C/C++: `socket()`, `bind()`, `listen()`, `accept()`, `send()`, `recv()`
- [The Linux man-pages project — poll(2)](https://man7.org/linux/man-pages/man2/poll.2.html) — Low-level reference for the `poll()` system call and event flags
- [The Linux man-pages project — execve(2)](https://man7.org/linux/man-pages/man2/execve.2.html) — Reference for CGI process spawning

### Key articles and tutorials consulted

- [How the web works: HTTP and CGI explained](https://www.garshol.priv.no/download/text/http-tut.html) — Useful walkthrough of the HTTP transaction cycle, consulted during request parsing design
- [Writing an HTTP server from scratch (in C)](https://medium.com/@gabriellamedas/the-http-server-explained-c41380307917) — Conceptual reference for the overall server architecture and connection lifecycle
- [Understanding NGINX configuration](https://www.digitalocean.com/community/tutorials/understanding-the-nginx-configuration-file-structure-and-configuration-contexts) — Guided the design of our configuration file parser and block structure

### AI usage

AI tools (primarily conversational assistants) were used as a technical support resource at specific points during the project. In all cases, AI-generated content was critically reviewed, tested, and validated by the team before being integrated.

| Task | How AI was used |
|------|----------------|
| **Understanding RFC sections** | We used AI to rephrase or summarise dense RFC paragraphs (e.g., chunked transfer encoding, status code semantics) after reading the primary sources, to confirm our own interpretation |
| **Debugging poll() event logic** | When our event loop produced unexpected blocking behaviour, we described the symptoms to an AI assistant to get hypotheses; we then reproduced and fixed the issues ourselves |
| **CGI environment variables** | We asked AI to list the standard CGI environment variables (e.g., `REQUEST_METHOD`, `CONTENT_LENGTH`, `PATH_INFO`) as a quick reference, cross-checked against RFC 3875 |
| **Config parser edge cases** | AI was used to generate a list of potential edge cases in our parser (e.g., missing semicolons, nested blocks, empty values) to build a more thorough manual test suite |
| **Error page HTTP compliance** | We asked AI to verify that our default error responses matched the expected format per RFC 7230/7231, then tested against NGINX's own responses |

AI was not used to write, generate, or complete any functional part of the codebase. All implementation decisions — architecture, data structures, algorithms, and system call usage — were made and coded by the team.
