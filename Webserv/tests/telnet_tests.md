# Webserv — Manual Telnet Tests

> **How to use this guide**
> Each test shows the exact text to type into telnet, followed by the response the server should return.
> In telnet, after the last header line you must press **Enter twice** (one blank line) to send the request.
> Lines marked `>` are what you type. Lines marked `<` are what you should receive.
> `[CRLF]` means pressing Enter. In HTTP, each line ends with `\r\n` — telnet handles this automatically.

---

## Connect to the server

```bash
telnet localhost 8080
```

You should see:
```
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
```

To disconnect at any point: `Ctrl + ]` then type `quit`.

---

## 1. GET — Serve the index page (200 OK)

**Request:**
```
GET / HTTP/1.1
Host: localhost

```

**Expected response:**
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: <N>
...

<!DOCTYPE html>
...   ← body of your index.html
```

---

## 2. GET — File that does not exist (404 Not Found)

**Request:**
```
GET /this_file_does_not_exist.html HTTP/1.1
Host: localhost

```

**Expected response:**
```
HTTP/1.1 404 Not Found
Content-Type: text/html
Content-Length: <N>

...   ← body of your custom 404 error page
```

---

## 3. GET — Method not allowed on a restricted route (405 Method Not Allowed)

The root `/` only allows GET. Sending POST must be rejected.

**Request:**
```
POST / HTTP/1.1
Host: localhost
Content-Length: 0

```

**Expected response:**
```
HTTP/1.1 405 Method Not Allowed
Allow: GET
Content-Type: text/html
...
```

---

## 4. GET — Static file (CSS, image, or any asset)

**Request:**
```
GET /static/style.css HTTP/1.1
Host: localhost

```

**Expected response:**
```
HTTP/1.1 200 OK
Content-Type: text/css
Content-Length: <N>
...

body { ... }   ← actual CSS content
```

> If the file doesn't exist: `404 Not Found`.

---

## 5. PUT — Upload a file via PUT (201 Created)

This covers tester requirement 2: `/put_test/*` must accept PUT and save files.

**Request:**
```
PUT /put_test/hello.txt HTTP/1.1
Host: localhost
Content-Length: 13

Hello, world!
```

**Expected response:**
```
HTTP/1.1 201 Created
Content-Length: 0
...
```

Verify the file was saved:
```
GET /put_test/hello.txt HTTP/1.1
Host: localhost

```
Expected: `200 OK` with `Hello, world!` as body.

---

## 6. POST — Body too large (413 Content Too Large / Payload Too Large)

The `/post_body` route has `client_max_body_size 100`. Sending 101 bytes must be rejected.

**Request:**
```
POST /post_body HTTP/1.1
Host: localhost
Content-Type: text/plain
Content-Length: 101

AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEAAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEA
```

(that is exactly 101 `A/B/C/D/E` characters)

**Expected response:**
```
HTTP/1.1 413 Content Too Large
Content-Type: text/html
...
```

> Note: some implementations return `413 Request Entity Too Large` — both are valid.

---

## 7. POST — Body within limit (200 OK or 204 No Content)

**Request:**
```
POST /post_body HTTP/1.1
Host: localhost
Content-Type: text/plain
Content-Length: 10

helloworld
```

**Expected response:**
```
HTTP/1.1 200 OK
...
```

---

## 8. DELETE — Delete an existing file (204 No Content)

First create the file (or ensure one exists at `/deletable/test.txt`), then delete it.

**Request:**
```
DELETE /deletable/test.txt HTTP/1.1
Host: localhost

```

**Expected response:**
```
HTTP/1.1 204 No Content
...
```

Verify it's gone:
```
GET /deletable/test.txt HTTP/1.1
Host: localhost

```
Expected: `404 Not Found`.

---

## 9. DELETE — File does not exist (404 Not Found)

**Request:**
```
DELETE /deletable/nonexistent_file.txt HTTP/1.1
Host: localhost

```

**Expected response:**
```
HTTP/1.1 404 Not Found
...
```

---

## 10. GET — Directory served with default index (tester requirement 5)

The `/directory` route maps to `YoupiBanane/` and serves `youpi.bad_extension` by default.

**Request:**
```
GET /directory/ HTTP/1.1
Host: localhost

```

**Expected response:**
```
HTTP/1.1 200 OK
Content-Type: text/html
...

← content of youpi.bad_extension
```

---

## 11. POST — CGI execution via .bla extension (tester requirement 3)

**Request:**
```
POST /directory/youpi.bla HTTP/1.1
Host: localhost
Content-Type: text/plain
Content-Length: 18

coucou les amis !
```

**Expected response:**
```
HTTP/1.1 200 OK
Content-Type: text/html
...

← CGI output from ubuntu_cgi_tester
```

> The CGI must receive `REQUEST_METHOD=POST`, the body via stdin, and return a valid HTTP body.

---

## 12. GET — CGI execution via .bla extension (tester requirement 3)

**Request:**
```
GET /directory/youpi.bla HTTP/1.1
Host: localhost

```

**Expected response:**
```
HTTP/1.1 200 OK
...
← CGI output
```

---

## 13. HTTP redirect (301 Moved Permanently)

**Request:**
```
GET /old-page HTTP/1.1
Host: localhost

```

**Expected response:**
```
HTTP/1.1 301 Moved Permanently
Location: /
...
```

---

## 14. HTTP redirect (302 Found) to external URL

**Request:**
```
GET /google HTTP/1.1
Host: localhost

```

**Expected response:**
```
HTTP/1.1 302 Found
Location: https://www.google.com
...
```

---

## 15. Malformed request (400 Bad Request)

Send a request line with a missing HTTP version:

**Request:**
```
GET /
Host: localhost

```

**Expected response:**
```
HTTP/1.1 400 Bad Request
...
```

---

## 16. Unsupported HTTP method (405 or 501 Not Implemented)

**Request:**
```
PATCH / HTTP/1.1
Host: localhost
Content-Length: 0

```

**Expected response:**
```
HTTP/1.1 405 Method Not Allowed
```
or
```
HTTP/1.1 501 Not Implemented
```

Both are acceptable depending on your implementation.

---

## 17. Second server on port 8081

```bash
telnet localhost 8081
```

**Request:**
```
GET / HTTP/1.1
Host: localhost

```

**Expected response:**
```
HTTP/1.1 200 OK
...
← content from ./www/secondary/index.html
```

---

## 18. Restricted server on port 8082

```bash
telnet localhost 8082
```

**Request (allowed):**
```
GET / HTTP/1.1
Host: restricted.local

```
Expected: `200 OK`

**Request (forbidden method):**
```
POST / HTTP/1.1
Host: restricted.local
Content-Length: 0

```
Expected: `405 Method Not Allowed`

---

## 19. Keep-Alive / Connection persistence

Send two requests in the same telnet session using `Connection: keep-alive`:

**First request:**
```
GET / HTTP/1.1
Host: localhost
Connection: keep-alive

```

**Second request (same connection):**
```
GET /static/style.css HTTP/1.1
Host: localhost
Connection: close

```

The server should respond to both before closing the connection on the second.

---

## 20. Large body rejected at server level (413)

The default `client_max_body_size` is 1 MB (1048576 bytes). Sending more must be rejected.

You can test this quickly with a 2 MB body. From bash (not telnet) using printf:

```bash
python3 -c "
import socket
s = socket.create_connection(('localhost', 8080))
body = 'A' * 1048577
req = 'POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Length: {}\r\n\r\n{}'.format(len(body), body)
s.sendall(req.encode())
print(s.recv(4096).decode())
"
```

**Expected response:**
```
HTTP/1.1 413 Content Too Large
...
```

---

## Quick reference: expected status codes

| Scenario | Expected code |
|---|---|
| Resource found | `200 OK` |
| File created via PUT | `201 Created` |
| Deleted successfully | `204 No Content` |
| Permanent redirect | `301 Moved Permanently` |
| Temporary redirect | `302 Found` |
| Bad request syntax | `400 Bad Request` |
| Method not allowed for route | `405 Method Not Allowed` |
| Resource not found | `404 Not Found` |
| Body exceeds max size | `413 Content Too Large` |
| Method not implemented at all | `501 Not Implemented` |
| Server-side error | `500 Internal Server Error` |
