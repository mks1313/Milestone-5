# Milestone 5 — 42 Barcelona

This repository contains my work for Milestone 5 of the 42 Common Core at Campus 42 Barcelona.

This milestone focuses on advanced C++, system programming, networking, and web infrastructure, combining low-level programming with real-world server and deployment concepts.

## Projects

### 1. Inception

Inception is a DevOps-oriented project where I build a complete web infrastructure using Docker and Docker Compose.

The project consists of setting up and orchestrating multiple services in isolated containers, typically including:

* NGINX (with TLS)
* WordPress
* MariaDB
* Docker networks and volumes

Main concepts covered:

* Containerization
* Service isolation
* Persistent volumes
* Container networking
* Environment-based configuration
* Reproducible deployments

This project provides a solid understanding of how modern web services are deployed and managed.

### 2. Webserv

Webserv consists of implementing a HTTP/1.1 web server in C++, inspired by servers like NGINX.

Key topics addressed:

* TCP/IP sockets
* Non-blocking I/O
* Multiplexing (`poll`, `select`, or equivalent)
* HTTP protocol and status codes
* Configuration file parsing
* Robust error handling

The focus is on correctness, performance, and compliance with the HTTP specification, while maintaining clean and maintainable C++ code.

### 3. CPP Modules (CPP05 → CPP09)

This milestone includes CPP modules 05 to 09, which cover advanced C++ object-oriented programming concepts.

Topics include:

* Exceptions and error handling
* Operator overloading
* Polymorphism and abstraction
* STL containers and algorithms
* Templates
* Type casting and runtime behavior

All modules follow the C++98 standard, emphasizing disciplined design, correct memory management, and predictable behavior.

## Repository Structure
```txt
milestone-5/
├── cpp/
│   ├── cpp05/
│   ├── cpp06/
│   ├── cpp07/
│   ├── cpp08/
│   └── cpp09/
├── inception/
├── webserv/
└── README.md
```

## What I Learned in This Milestone

* Designing and deploying containerized web infrastructures
* Understanding how HTTP servers work internally
* Managing complexity in larger C++ projects
* Writing robust, evaluable, and maintainable code
* Thinking in terms of systems, not only individual programs

## Technical Skills

* **C++ (C++98)** — Advanced OOP, STL, templates, memory management
* **Networking & HTTP** — Sockets, TCP/IP, HTTP/1.1, multiplexing
* **DevOps Fundamentals** — Docker, Docker Compose, volumes, container networking
* **System Programming** — Low-level reasoning, resource control, robustness

## Author

**Maksim Georgiev Marinov**  
42 Barcelona  
GitHub: [https://github.com/mks1313](https://github.com/mks1313)

---

Thank you for visiting this repository and following my progress at 42.
