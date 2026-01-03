# Custom Network Proxy Server

This project implements a **Custom Proxy Server** in C++.  
The proxy supports HTTP request forwarding, HTTPS tunneling via the CONNECT method, domain-based request blocking, logging, metrics collection, and graceful shutdown.

All architectural design decisions, execution flow, concurrency model, and flowcharts are documented in **`docs/design.md`**.

---

## Features

- HTTP request forwarding with non-persistent (HTTP/1.0-style) semantics
- HTTPS tunneling using the CONNECT method
- Fixed-size thread pool for controlled concurrency
- Blocking I/O with socket-level read and write timeouts
- Domain-based request blocking using a configurable blocklist
- Graceful handling of idle or slow clients via enforced timeouts
- Structured logging of requests, errors, and connection events
- Runtime metrics collection for traffic and request statistics
- Graceful shutdown on termination signals, allowing in-flight requests to complete
- External configuration through a file for runtime behavior tuning
- Safe handling of partial reads and writes on TCP sockets
- Clear separation of concerns through a modular code structure

---

## HTTP Behavior

The proxy intentionally follows non-persistent HTTP semantics, consistent with HTTP/1.0-style behavior, for request forwarding:

- Each client connection processes exactly one HTTP request

- The connection is closed after the response is forwarded

- Persistent connections (keep-alive) and request pipelining are not supported

The server uses a fixed-size thread pool for request handling.
Each accepted connection is assigned to an available worker thread from the pool and handled using blocking I/O.

To prevent worker threads from being indefinitely occupied by idle or slow clients, socket-level timeouts are enforced. This ensures bounded resource usage and predictable behavior even under adverse client conditions.

HTTPS traffic is handled separately using the CONNECT method and is tunneled transparently. Once the tunnel is established, encrypted data is relayed without HTTP-level interpretation, and the HTTP version used for forwarding does not affect HTTPS traffic.

---

## Configuration

The proxy server is configured using:

```
config/proxy.conf
```

This file defines runtime parameters such as:

- Listening address and port
- Thread pool size
- Socket timeouts
- Log file location and size limits
- Metrics output file
- Blocklist file path and enable/disable flag

---

## Build Instructions

### 1.1 Prerequisites

To build and run the proxy server, the following requirements must be met:

- **Operating System:**  
  Linux (POSIX-compliant environment)

- **Compiler:**  
  `g++` with **C++17 support**  
  Recommended version: g++ 7.0 or newer

### 1.2 Build the Server

The project is built using `g++` and a Makefile.
Copy and run the following command:

```bash
make clean
make
```

### 1.3 Run the Server

```bash
./proxy
```
