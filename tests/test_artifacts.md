# Test Artifacts

This document contains the test artifacts used to validate the behavior of the HTTP/HTTPS Proxy Server. The tests demonstrate correct request forwarding, policy enforcement, concurrent client handling, and robustness against malformed input.

All tests are executed against the running proxy using standard command-line tools. Representative log entries and a final metrics snapshot are included to verify observable server behavior during testing.

## Test 1: Successful HTTP Request Forwarding

**Purpose**  
To verify that the proxy correctly forwards a valid HTTP/HTTPS request to the destination server and returns the response to the client.

#### Test Command - Basic HTTP GET

```bash
curl -x http://localhost:2205 http://example.com
```

**Observed Behavior**

The client receives a valid HTTP response from the destination server.The connection is closed after the response is fully received.

**Log Entry**

```bash
[YYYY-MM-DD HH:MM:SS] 127.0.0.1:59488 | "GET / HTTP/1.0" | example.com:80 | ALLOWED | 200 | bytes=782
```

#### Test Command - HTTP GET with Explicit Path

```bash
curl -x http://localhost:2205 http://example.com/index.html
```

**Observed Behavior**

The client receives the correct response content for /index.html.

**Log Entry**

```bash
[YYYY-MM-DD HH:MM:SS] 127.0.0.1:36852 | "GET /index.html HTTP/1.0" | example.com:80 | ALLOWED | 200 | bytes=702
```

#### Test Command - HTTPS Request via CONNECT

```bash
curl -x http://localhost:2205 https://www.google.com
```

**Observed Behavior**

The proxy establishes an HTTPS tunnel using the CONNECT method. Encrypted traffic is forwarded transparently.The client successfully receives the HTTPS response.

**Log Entry**

```bash
[YYYY-MM-DD HH:MM:SS] 127.0.0.1:53986 | "CONNECT  HTTP/1.0" | www.google.com:443 | ALLOWED | 200 | bytes=27124
```

---

## Test 2: Blocking of Blacklisted Domains and Sub-domains

**Purpose**  
To verify that the proxy enforces domain-based blocking rules and prevents requests to configured blacklisted domains.

### Test Setup

Add the target domain to the blocklist file specified in the configuration:
Example: example.net(say)

**Test Command**

```bash
curl -x http://localhost:2205 http://example.net
```

**Observed Behavior**

The client receives an HTTP 403 Forbidden response and the connection is closed.
Response: Access to the requested domain is blocked.

**Log Entry**

```bash
[YYYY-MM-DD HH:MM:SS] 127.0.0.1:42636 | "GET / HTTP/1.0" | example.net:80 | BLOCKED | 403 | bytes=0
```

**Test Command - For Sub-domain**

```bash
curl -x http://localhost:2205 http://sub.example.net
```

**Observed Behavior**

The client receives the same HTTP 403 Forbidden response as above and the connection is closed.
Response: Access to the requested domain is blocked.

**Log Entry**

```bash
[YYYY-MM-DD HH:MM:SS] 127.0.0.1:35830 | "GET / HTTP/1.0" | sub.example.net:80 | BLOCKED | 403 | bytes=0
```

---

## Test 3: Behavior Under Concurrent Clients

**Purpose**  
To verify that the proxy server handles multiple concurrent client connections correctly using a fixed-size thread pool, without crashing, deadlocking, or misrouting requests.

### Test Command

The following command generates multiple concurrent HTTP requests through the proxy:

```bash
for i in {1..20}; do
  curl -x http://localhost:2205 http://example.com &
done
wait
```

**Observed Behavior**

All client requests complete successfully. Each request receives a valid HTTP response. No crashes, deadlocks, or resource exhaustion are observed.

**Log Entry**

Following but just repeated 20 times :

```bash
[YYYY-MM-DD HH:MM:SS] 127.0.0.1:35764 | "GET / HTTP/1.0" | example.com:80 | ALLOWED | 200 | bytes=782
```

---

## Test 4: Handling of Malformed Requests

**Purpose**  
To verify that the proxy server correctly detects malformed HTTP requests, returns an appropriate error response, and remains stable without forwarding any traffic.

### Test Command

A malformed request is sent manually using `telnet` to simulate raw client input:

```bash
telnet localhost 2205
```

After connecting, type the following exactly, pressing ENTER after each line:
Example : Request missing HTTP version

```bash
GET /
Host : example.com
```

**Observed Behavior**

The proxy responds with an HTTP 400 Bad Request error with proper response on the terminal.

**Log Entry**

```bash
[YYYY-MM-DD HH:MM:SS] 127.0.0.1:33736 | "INVALID REQUEST" | - | FAILED | 400 | bytes=0
```

---

## Test 5: Connection Timeout Handling (Idle Client)

**Purpose**  
To verify that the proxy server correctly enforces socket-level timeouts and does not allow worker threads to remain blocked indefinitely when a client connects but sends no data.

### Test Command

Establish an idle TCP connection without sending any request data:

```bash
nc -v localhost 2205 < /dev/null
```

**Observed Behavior**

The proxy closes the idle connection automatically after the timeout expires and displays a well formed BAD REQUEST Response.

**Log Entry**

```bash
[YYYY-MM-DD HH:MM:SS] 127.0.0.1:38642 | "INVALID REQUEST" | - | FAILED | 400 | bytes=0
```

---

## Test 6: Graceful Shutdown

**Purpose**  
To verify that the proxy server shuts down gracefully when a termination signal is received, allowing in-flight requests to complete while preventing acceptance of new connections.

### Test Command

Start a request through the proxy and initiate shutdown while the request is in progress.

```bash
curl -x http://localhost:2205 http://example.com
```

In a separate terminal, identify the proxy process ID and send a termination signal:

```bash
pgrep proxy
```

Then Put the PID in the desired position of next Command

```bash
kill -SIGINT <PID>
```

Alternatively press **Ctrl+C** in the terminal where the server is running .

**Observed Behavior**

The Server initiates Graceful Shutdown and cleanly closes the server and the open file if any.
Output in the terminal:

```
[INFO] Graceful shutdown initiated...
[INFO] Server stopped accepting connections
[INFO] Log File Closed
[INFO] Proxy Server stopped cleanly
```

---
