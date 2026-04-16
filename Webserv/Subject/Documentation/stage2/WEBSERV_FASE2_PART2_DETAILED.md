# WEBSERV - Fase 2 (Parte 2): Análisis Detallado Adicional

## Response.cpp, Métodos HTTP, Seguridad y Gestión de Memoria

**Proyecto:** 42 Barcelona - webserv  
**Autor:** fcela-ga (Felipe Cela García)  
**Fase:** 2 de 4 (Continuación)

---

# ÍNDICE FASE 2 - PARTE 2

7. [Response.cpp - Constructor de Respuestas HTTP](#7-responsecpp---constructor-de-respuestas-http)
8. [Handlers de Métodos HTTP](#8-handlers-de-métodos-http)
9. [Sistema de Cola CGI y Gestión de Memoria](#9-sistema-de-cola-cgi-y-gestión-de-memoria)
10. [Análisis de Seguridad](#10-análisis-de-seguridad)
11. [Resolución de Rutas y Alias](#11-resolución-de-rutas-y-alias)
12. [Manejo de Errores y Timeouts](#12-manejo-de-errores-y-timeouts)

---

# 7. RESPONSE.CPP - CONSTRUCTOR DE RESPUESTAS HTTP

## 7.1 Estructura de la Clase Response

```cpp
class Response {
private:
    int _statusCode;                              // Código HTTP (200, 404, etc.)
    std::map<std::string, std::string> _headers;  // Headers HTTP
    std::string _body;                            // Cuerpo de la respuesta
    bool _sent;                                   // ¿Ya se envió?
    size_t _bytesSent;                            // Bytes enviados (para tracking)
};
```

## 7.2 Constructor y Headers por Defecto

```cpp
// Líneas 20-26
Response::Response()
    : _statusCode(200)      // HTTP 200 OK por defecto
    , _sent(false)          // No enviado aún
    , _bytesSent(0)         // 0 bytes enviados
{
    _setDefaultHeaders();   // Establecer headers obligatorios
}

// Líneas 396-400
void Response::_setDefaultHeaders() {
    setHeader("Server", "Webserv/1.0");          // Identificación del servidor
    setHeader("Date", Utils::getHttpDate());     // RFC 7231 §7.1.1.2: Fecha actual
    setHeader("Connection", "keep-alive");        // Mantener conexión por defecto
}
```

**Headers por defecto según RFC 7231:**

| Header | Valor | Requisito RFC |
|--------|-------|---------------|
| `Server` | `Webserv/1.0` | OPCIONAL (§7.4.2) |
| `Date` | Fecha HTTP | DEBE incluirse en respuestas (§7.1.1.2) |
| `Connection` | `keep-alive` | Comportamiento por defecto HTTP/1.1 |

## 7.3 Construcción de la Respuesta HTTP

```cpp
// Líneas 97-103
std::string Response::build(bool excludeBody) const {
    std::string headers = buildHeaders();
    if (excludeBody) {
        return headers;  // Para método HEAD
    }
    return headers + _body;
}

// Líneas 105-121
std::string Response::buildHeaders() const {
    std::ostringstream response;
    
    // Línea de estado (Status Line)
    // RFC 7230 §3.1.2: status-line = HTTP-version SP status-code SP reason-phrase CRLF
    response << "HTTP/1.1 " << _statusCode << " " 
             << Utils::getStatusMessage(_statusCode) << "\r\n";
    
    // Headers
    // RFC 7230 §3.2: header-field = field-name ":" OWS field-value OWS
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    
    // Línea vacía que separa headers del body
    // RFC 7230 §3: HTTP-message = start-line *( header-field CRLF ) CRLF [ message-body ]
    response << "\r\n";
    
    return response.str();
}
```

**Anatomía de una respuesta HTTP:**

```
HTTP/1.1 200 OK\r\n              ← Status Line
Server: Webserv/1.0\r\n          ← Header
Date: Fri, 27 Dec 2024 ...\r\n   ← Header
Content-Type: text/html\r\n      ← Header
Content-Length: 1234\r\n         ← Header
Connection: keep-alive\r\n       ← Header
\r\n                             ← Línea vacía (fin de headers)
<!DOCTYPE html>...               ← Body (opcional)
```

## 7.4 Gestión de Cookies (RFC 6265)

```cpp
// Líneas 72-86
void Response::setCookie(const std::string& name, const std::string& value,
                         const std::string& path, int maxAge,
                         bool httpOnly, bool secure) {
    std::string cookie = name + "=" + value;
    
    // Path: Ámbito de la cookie
    if (!path.empty())
        cookie += "; Path=" + path;
    
    // Max-Age: Duración en segundos (0 = eliminar cookie)
    if (maxAge >= 0)
        cookie += "; Max-Age=" + Utils::intToString(maxAge);
    
    // HttpOnly: No accesible desde JavaScript (protección XSS)
    if (httpOnly)
        cookie += "; HttpOnly";
    
    // Secure: Solo enviar sobre HTTPS
    if (secure)
        cookie += "; Secure";
    
    setHeader("Set-Cookie", cookie);
}
```

**Formato de Set-Cookie:**

```
Set-Cookie: WEBSERV_SESSION=abc123; Path=/; Max-Age=3600; HttpOnly
            ▲                  ▲          ▲             ▲
            │                  │          │             │
            Nombre=Valor       Ámbito     Expiración    Seguridad
```

## 7.5 Factory Methods para Tipos de Respuesta

### 7.5.1 Response::makeError()

```cpp
// Líneas 179-201
Response Response::makeError(int code, const ServerConfig* config) {
    Response response;
    response.setStatusCode(code);
    response.setContentType("text/html; charset=utf-8");
    
    std::string errorPage;
    
    // 1. Intentar cargar página de error personalizada
    if (config != NULL) {
        std::string customPage = config->getErrorPage(code);
        if (!customPage.empty()) {
            std::string fullPath = Utils::joinPath(config->getRoot(), customPage);
            if (Utils::fileExists(fullPath) && Utils::isReadable(fullPath)) {
                errorPage = Utils::readFile(fullPath);
            }
        }
    }
    
    // 2. Si no hay personalizada, usar página por defecto
    if (errorPage.empty()) {
        errorPage = _getDefaultErrorPage(code);
    }
    
    response.setBody(errorPage);
    return response;
}
```

**Flujo de selección de página de error:**

```
┌─────────────────────────────────────────────────────────────────┐
│                    makeError(404, config)                       │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
              ┌──────────────────────────────┐
              │ ¿config tiene error_page 404?│
              └──────────────┬───────────────┘
                    SÍ ┌─────┴─────┐ NO
                       │           │
              ┌────────▼────────┐  │
              │ Cargar archivo  │  │
              │ /errors/404.html│  │
              └────────┬────────┘  │
                       │           │
              ┌────────▼────────┐  │
              │ ¿Archivo existe │  │
              │ y es legible?   │  │
              └────────┬────────┘  │
                  SÍ ┌─┴─┐ NO     │
                     │   │        │
              ┌──────▼───┼────────▼────────┐
              │ Usar     │ Generar página  │
              │ archivo  │ por defecto     │
              └──────────┴─────────────────┘
```

### 7.5.2 Response::makeRedirect()

```cpp
// Líneas 203-217
Response Response::makeRedirect(int code, const std::string& location) {
    Response response;
    response.setStatusCode(code);
    response.setHeader("Location", location);  // RFC 7231 §7.1.2: DEBE incluirse
    response.setContentType("text/html; charset=utf-8");
    
    // Body informativo para navegadores que no redirigen automáticamente
    std::ostringstream body;
    body << "<!DOCTYPE html><html><head><title>Redirect</title></head><body>";
    body << "<h1>" << code << " " << Utils::getStatusMessage(code) << "</h1>";
    body << "<p>Redirecting to <a href=\"" << location << "\">" 
         << location << "</a></p>";
    body << "</body></html>";
    
    response.setBody(body.str());
    return response;
}
```

**Códigos de redirección soportados:**

| Código | Significado | Uso |
|--------|-------------|-----|
| 301 | Moved Permanently | URL cambió permanentemente |
| 302 | Found | Redirección temporal |
| 307 | Temporary Redirect | Mantiene método HTTP |
| 308 | Permanent Redirect | 301 + mantiene método |

### 7.5.3 Response::makeDirectoryListing()

```cpp
// Líneas 234-318
Response Response::makeDirectoryListing(const std::string& path, const std::string& uri) {
    Response response;
    response.setStatusCode(200);
    response.setContentType("text/html; charset=utf-8");
    
    std::ostringstream body;
    
    // Generar HTML con estilo CSS integrado
    body << "<!DOCTYPE html>\n<html>\n<head>\n";
    body << "<title>Index of " << uri << "</title>\n";
    body << "<style>\n";
    // ... estilos CSS ...
    body << "</style>\n</head>\n<body>\n";
    body << "<h1>Index of " << uri << "</h1>\n";
    body << "<table>\n<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";
    
    // Link al directorio padre
    if (uri != "/") {
        // Calcular ruta del padre
        std::string parent = uri;
        if (parent[parent.length() - 1] == '/')
            parent = parent.substr(0, parent.length() - 1);
        size_t lastSlash = parent.find_last_of('/');
        if (lastSlash != std::string::npos)
            parent = parent.substr(0, lastSlash + 1);
        body << "<tr><td><a href=\"" << parent << "\">../</a></td>"
             << "<td>-</td><td>-</td></tr>\n";
    }
    
    // Listar contenido del directorio
    DIR* dir = opendir(path.c_str());
    if (dir != NULL) {
        struct dirent* entry;
        std::vector<std::string> entries;
        
        // Recolectar entradas (excepto . y ..)
        while ((entry = readdir(dir)) != NULL) {
            std::string name = entry->d_name;
            if (name != "." && name != "..")
                entries.push_back(name);
        }
        closedir(dir);
        
        // Ordenar alfabéticamente
        std::sort(entries.begin(), entries.end());
        
        // Generar filas de la tabla
        for (size_t i = 0; i < entries.size(); ++i) {
            std::string fullPath = Utils::joinPath(path, entries[i]);
            struct stat st;
            
            if (stat(fullPath.c_str(), &st) == 0) {
                std::string displayName = entries[i];
                std::string link = entries[i];
                
                // Añadir / a directorios
                if (S_ISDIR(st.st_mode)) {
                    displayName += "/";
                    link += "/";
                }
                
                // Tamaño (solo para archivos regulares)
                std::string size = "-";
                if (S_ISREG(st.st_mode))
                    size = Utils::sizeTToString(st.st_size);
                
                // Fecha de última modificación
                char timeStr[64];
                struct tm* tm_info = localtime(&st.st_mtime);
                strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", tm_info);
                
                // URL encode del link para caracteres especiales
                body << "<tr><td><a href=\"" << Utils::urlEncode(link) << "\">" 
                     << displayName << "</a></td>";
                body << "<td class=\"size\">" << size << "</td>";
                body << "<td class=\"date\">" << timeStr << "</td></tr>\n";
            }
        }
    }
    
    body << "</table>\n";
    body << "<hr><p style=\"color:#888;font-size:12px;\">Webserv/1.0</p>\n";
    body << "</body>\n</html>";
    
    response.setBody(body.str());
    return response;
}
```

### 7.5.4 Response::makeFromCGI()

```cpp
// Líneas 320-390
Response Response::makeFromCGI(const std::string& cgiOutput) {
    Response response;
    std::map<std::string, std::string> headers;
    std::string body;
    int statusCode = 200;

    // ═══════════════════════════════════════════════════════════════════
    // PARSING DE OUTPUT CGI (RFC 3875 §6)
    // 
    // El CGI produce:
    // Content-Type: text/html\n
    // Status: 200 OK\n
    // X-Custom: value\n
    // \n
    // <html>body...</html>
    // ═══════════════════════════════════════════════════════════════════
    
    // Buscar separador entre headers y body
    // Puede ser \r\n\r\n (Windows/HTTP) o \n\n (Unix)
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        headerEnd = cgiOutput.find("\n\n");

    if (headerEnd != std::string::npos) {
        std::string headerPart = cgiOutput.substr(0, headerEnd);
        size_t bodyStart = headerEnd + (cgiOutput[headerEnd] == '\r' ? 4 : 2);
        body = cgiOutput.substr(bodyStart);
        
        // Parsear headers línea por línea
        std::istringstream headerStream(headerPart);
        std::string line;
        while (std::getline(headerStream, line)) {
            // Eliminar \r si está presente
            if (!line.empty() && line[line.length() - 1] == '\r')
                line = line.substr(0, line.length() - 1);
            
            if (line.empty())
                continue;
            
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string name = Utils::trim(line.substr(0, colonPos));
                std::string value = Utils::trim(line.substr(colonPos + 1));
                
                // Header especial: Status
                // RFC 3875 §6.3.3: Status = "Status:" status-code SP reason-phrase
                if (Utils::toLower(name) == "status") {
                    statusCode = Utils::stringToInt(value.substr(0, 3));
                } else {
                    headers[name] = value;
                }
            }
        }
    } else {
        // Sin headers, todo es body
        body = cgiOutput;
    }
    
    response.setStatusCode(statusCode);

    // Copiar headers EXCEPTO Content-Length (se recalculará)
    for (std::map<std::string, std::string>::iterator it = headers.begin();
         it != headers.end(); ++it) {
        if (Utils::toLower(it->first) != "content-length") {
            response.setHeader(it->first, it->second);
        }
    }

    // Content-Type por defecto si no lo proporciona el CGI
    if (headers.find("Content-Type") == headers.end())
        response.setContentType("text/html; charset=utf-8");

    // IMPORTANTE: setBody() calcula Content-Length automáticamente
    // Esto corrige CGIs que proporcionan Content-Length incorrecto
    response.setBody(body);
    return response;
}
```

**Ejemplo de parsing CGI:**

```
Input del CGI:
┌─────────────────────────────────────────────┐
│ Content-Type: application/json              │ ← Header CGI
│ Status: 201 Created                         │ ← Header especial
│ X-Request-Id: abc123                        │ ← Header personalizado
│                                             │ ← Línea vacía
│ {"success": true, "id": 42}                 │ ← Body
└─────────────────────────────────────────────┘

Output HTTP:
┌─────────────────────────────────────────────┐
│ HTTP/1.1 201 Created                        │ ← De "Status: 201"
│ Content-Type: application/json              │ ← Del CGI
│ X-Request-Id: abc123                        │ ← Del CGI
│ Content-Length: 27                          │ ← Calculado
│ Server: Webserv/1.0                         │ ← Por defecto
│ Date: ...                                   │ ← Por defecto
│                                             │
│ {"success": true, "id": 42}                 │
└─────────────────────────────────────────────┘
```

---

# 8. HANDLERS DE MÉTODOS HTTP

## 8.1 Handler POST

```cpp
// Líneas 1101-1167
void Server::_handlePost(Client& client, const LocationConfig* location)
{
    const Request& request = client.getRequest();
    const ServerConfig* serverConfig = client.getServerConfig();
    
    std::string filePath = _resolvePath(request, location, serverConfig);

    // ═══════════════════════════════════════════════════════════════════
    // CASO 1: Es una petición CGI
    // ═══════════════════════════════════════════════════════════════════
    if (location != NULL && _isCgiRequest(filePath, location))
    {
        _handleCgi(client, location, filePath);
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // CASO 2: Upload de archivos habilitado
    // ═══════════════════════════════════════════════════════════════════
    if (location != NULL && location->getUploadEnabled())
    {
        _handleFileUpload(client, location);
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // CASO 3: Recurso estático existente
    // ═══════════════════════════════════════════════════════════════════
    struct stat st;
    if (stat(filePath.c_str(), &st) == 0)
    {
        // Si es directorio, buscar CGI index
        if (S_ISDIR(st.st_mode))
        {
            // ... búsqueda de index.php, index.py, etc. ...
        }
        
        // POST a recurso estático no tiene sentido
        _sendErrorResponse(client, HTTP_METHOD_NOT_ALLOWED);
        return;
    }

    // ═══════════════════════════════════════════════════════════════════
    // CASO 4: Recurso no existe
    // Para /post_body del tester, retornar 204 No Content
    // ═══════════════════════════════════════════════════════════════════
    Response resp;
    resp.setStatusCode(HTTP_NO_CONTENT);  // 204
    std::string response = resp.build();
    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}
```

**Diagrama de decisión POST:**

```
                    POST /path
                        │
                        ▼
              ┌─────────────────────┐
              │ ¿Es extensión CGI?  │
              │ (.py, .php, .pl)    │
              └──────────┬──────────┘
                   SÍ ┌──┴──┐ NO
                      │     │
               ┌──────▼──┐  │
               │ _handle │  │
               │ Cgi()   │  │
               └─────────┘  │
                            ▼
              ┌─────────────────────┐
              │ ¿Upload habilitado? │
              └──────────┬──────────┘
                   SÍ ┌──┴──┐ NO
                      │     │
               ┌──────▼─────┐│
               │ _handleFile││
               │ Upload()   ││
               └────────────┘│
                             ▼
              ┌─────────────────────┐
              │ ¿Recurso existe?    │
              └──────────┬──────────┘
                   SÍ ┌──┴──┐ NO
                      │     │
               ┌──────▼─────┐│
               │ 405 Method ││
               │ Not Allowed││
               └────────────┘│
                             ▼
                      ┌──────────┐
                      │ 204 No   │
                      │ Content  │
                      └──────────┘
```

## 8.2 Handler PUT

```cpp
// Líneas 1169-1230
void Server::_handlePut(Client& client, const LocationConfig* location)
{
    const Request& request = client.getRequest();
    const ServerConfig* serverConfig = client.getServerConfig();
    
    std::string filePath = _resolvePath(request, location, serverConfig);
    
    // Verificar que no es un directorio
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode))
    {
        _sendErrorResponse(client, HTTP_FORBIDDEN);
        return;
    }
    
    // Determinar si el archivo ya existe (para código de estado)
    bool fileExists = (access(filePath.c_str(), F_OK) == 0);
    
    // ═══════════════════════════════════════════════════════════════════
    // Crear directorios padre si no existen
    // Esto permite PUT /new/path/file.txt cuando /new/path/ no existe
    // ═══════════════════════════════════════════════════════════════════
    size_t lastSlash = filePath.rfind('/');
    if (lastSlash != std::string::npos && lastSlash > 0)
    {
        std::string directory = filePath.substr(0, lastSlash);
        if (!directory.empty())
        {
            // mkdir -p crea todos los directorios intermedios
            std::string cmd = "mkdir -p \"" + directory + "\" 2>/dev/null";
            system(cmd.c_str());
        }
    }
    
    // Escribir contenido al archivo
    std::ofstream file(filePath.c_str(), std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    
    const std::string& body = request.getBody();
    if (!body.empty())
    {
        file.write(body.c_str(), body.length());
    }
    file.close();
    
    if (file.fail())
    {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    
    // ═══════════════════════════════════════════════════════════════════
    // RFC 7231 §4.3.4:
    // - 201 Created: Si el recurso fue creado
    // - 200 OK: Si el recurso existente fue modificado
    // ═══════════════════════════════════════════════════════════════════
    Response resp;
    resp.setStatusCode(fileExists ? HTTP_OK : HTTP_CREATED);
    resp.setHeader("Content-Length", "0");
    std::string response = resp.build();
    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}
```

## 8.3 Handler DELETE

```cpp
// Líneas 1232-1263
void Server::_handleDelete(Client& client, const LocationConfig* location)
{
    const Request& request = client.getRequest();
    const ServerConfig* serverConfig = client.getServerConfig();
    
    std::string filePath = _resolvePath(request, location, serverConfig);

    // Verificar que el archivo existe
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) < 0)
    {
        _sendErrorResponse(client, HTTP_NOT_FOUND);
        return;
    }

    // Solo permitir borrar archivos regulares (no directorios)
    if (!S_ISREG(fileStat.st_mode))
    {
        _sendErrorResponse(client, HTTP_FORBIDDEN);
        return;
    }

    // Intentar eliminar el archivo
    if (std::remove(filePath.c_str()) != 0)
    {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    // RFC 7231 §4.3.5: 204 No Content para eliminación exitosa
    Response resp;
    resp.setStatusCode(HTTP_NO_CONTENT);
    std::string response = resp.build();
    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}
```

## 8.4 Handler de File Upload

```cpp
// Líneas 1576-1642
void Server::_handleFileUpload(Client& client, const LocationConfig* location)
{
    const Request& request = client.getRequest();
    
    // Obtener directorio de uploads
    std::string uploadPath = location->getUploadPath();
    if (uploadPath.empty())
        uploadPath = "/tmp/uploads";

    // Crear directorio si no existe
    mkdir(uploadPath.c_str(), 0755);

    const std::vector<UploadedFile>& files = request.getUploadedFiles();
    
    if (files.empty())
    {
        // ═══════════════════════════════════════════════════════════════
        // No es multipart: guardar body completo como archivo
        // ═══════════════════════════════════════════════════════════════
        std::string filename = "upload_" + Utils::intToString(std::time(NULL));
        std::string fullPath = uploadPath + "/" + filename;

        std::ofstream file(fullPath.c_str(), std::ios::binary);
        if (!file)
        {
            _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
            return;
        }
        file << request.getBody();
        file.close();
    }
    else
    {
        // ═══════════════════════════════════════════════════════════════
        // Multipart: guardar cada archivo por separado
        // ═══════════════════════════════════════════════════════════════
        for (size_t i = 0; i < files.size(); ++i)
        {
            std::string filename = files[i].filename;
            if (filename.empty())
                filename = "upload_" + Utils::intToString(std::time(NULL)) + 
                          "_" + Utils::intToString(i);

            // ═══════════════════════════════════════════════════════════
            // SEGURIDAD: Sanitizar nombre de archivo
            // Eliminar caracteres que podrían causar path traversal
            // ═══════════════════════════════════════════════════════════
            std::string cleanFilename;
            for (size_t j = 0; j < filename.length(); ++j)
            {
                char c = filename[j];
                // Caracteres prohibidos: / \ : * ? " < > |
                if (c != '/' && c != '\\' && c != ':' && c != '*' && 
                    c != '?' && c != '"' && c != '<' && c != '>' && c != '|')
                    cleanFilename += c;
            }
            filename = cleanFilename;

            std::string fullPath = uploadPath + "/" + filename;

            std::ofstream file(fullPath.c_str(), std::ios::binary);
            if (!file)
            {
                _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
                return;
            }
            file << files[i].data;
            file.close();
        }
    }

    // 201 Created
    Response resp;
    resp.setStatusCode(HTTP_CREATED);
    std::string response = resp.build();
    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
}
```

**Sanitización de nombres de archivo:**

```
Entrada maliciosa:              Resultado sanitizado:
─────────────────               ────────────────────
"../../etc/passwd"          →   "....etcpasswd"
"C:\Windows\System32\x"     →   "CWindowsSystem32x"
"file<script>.txt"          →   "filescript.txt"
"normal_file.jpg"           →   "normal_file.jpg" (sin cambios)
```

---

# 9. SISTEMA DE COLA CGI Y GESTIÓN DE MEMORIA

## 9.1 Control de Concurrencia CGI

```cpp
// Constante en webserv.hpp
#define MAX_CONCURRENT_CGI 5

// Variables de instancia en Server
size_t _activeCgiCount;                // Contador de CGIs activos
std::vector<int> _cgiQueue;            // Cola de clientes esperando CGI
std::map<int, int> _cgiToClient;       // Mapeo pipe fd → client fd
```

**¿Por qué limitar CGIs concurrentes?**

1. **Memoria**: Cada CGI puede producir ~100MB de output
2. **PIDs**: Sistema operativo tiene límite de procesos
3. **CPU**: Evitar saturación del servidor
4. **Estabilidad**: Prevenir condiciones de OOM (Out Of Memory)

## 9.2 Procesamiento de la Cola CGI

```cpp
// Líneas 1765-1821
void Server::_processNextCgiFromQueue()
{
    // Procesar mientras haya capacidad y elementos en cola
    while (!_cgiQueue.empty() && _activeCgiCount < MAX_CONCURRENT_CGI)
    {
        // ═══════════════════════════════════════════════════════════════
        // PROTECCIÓN DE MEMORIA: Calcular uso actual de buffers
        // ═══════════════════════════════════════════════════════════════
        size_t totalWriteBufferSize = 0;
        size_t clientsWithLargeBuffers = 0;

        for (std::map<int, Client>::iterator cit = _clients.begin(); 
             cit != _clients.end(); ++cit)
        {
            size_t writeSize = cit->second.getWriteBufferSize();
            totalWriteBufferSize += writeSize;

            // Contar clientes con >10MB pendiente de envío
            if (writeSize > 10485760) // 10MB
                ++clientsWithLargeBuffers;
        }

        // ═══════════════════════════════════════════════════════════════
        // PROTECCIÓN: Si hay muchas respuestas grandes pendientes,
        // esperar a que se envíen antes de generar más
        // ═══════════════════════════════════════════════════════════════
        if (clientsWithLargeBuffers >= 3)
        {
            Utils::logDebug("Delaying queue: " + 
                Utils::sizeTToString(clientsWithLargeBuffers) +
                " clients with large buffers (" + 
                Utils::sizeTToString(totalWriteBufferSize) + " bytes)");
            break;
        }

        // Obtener siguiente cliente de la cola
        int clientFd = _cgiQueue.front();
        _cgiQueue.erase(_cgiQueue.begin());

        // Verificar que el cliente aún existe
        std::map<int, Client>::iterator it = _clients.find(clientFd);
        if (it == _clients.end())
        {
            // Cliente desconectó mientras esperaba, saltar
            continue;
        }

        Client& client = it->second;
        const Request& request = client.getRequest();
        const ServerConfig* serverConfig = client.getServerConfig();

        // Reintentar ejecución de CGI
        const LocationConfig* location = serverConfig->findLocation(request.getPath());
        std::string filePath = _resolvePath(request, location, serverConfig);
        _handleCgi(client, location, filePath);
    }
}
```

**Diagrama de flujo de la cola:**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         SISTEMA DE COLA CGI                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   Nueva petición CGI                                                        │
│          │                                                                  │
│          ▼                                                                  │
│   ┌─────────────────────────┐                                               │
│   │ _activeCgiCount < 5 ?   │                                               │
│   └────────────┬────────────┘                                               │
│          SÍ ┌──┴──┐ NO                                                      │
│             │     │                                                         │
│   ┌─────────▼─┐ ┌─▼─────────────────┐                                       │
│   │ fork() +  │ │ Añadir a cola     │                                       │
│   │ execve()  │ │ _cgiQueue.push()  │                                       │
│   │           │ │ state=PROCESSING  │                                       │
│   │ _active   │ └─────────┬─────────┘                                       │
│   │ CgiCount++│           │                                                 │
│   └───────────┘           │                                                 │
│                           │                                                 │
│   ════════════════════════│═══════════════════════════════════════════════  │
│                           │                                                 │
│   CGI termina             │                                                 │
│   _activeCgiCount--       │                                                 │
│          │                │                                                 │
│          ▼                │                                                 │
│   ┌─────────────────────┐ │                                                 │
│   │ _processNextCgi     │◄┘                                                 │
│   │ FromQueue()         │                                                   │
│   └──────────┬──────────┘                                                   │
│              │                                                              │
│              ▼                                                              │
│   ┌─────────────────────────────────┐                                       │
│   │ ¿clientsWithLargeBuffers >= 3 ? │                                       │
│   └──────────────┬──────────────────┘                                       │
│            SÍ ┌──┴──┐ NO                                                    │
│               │     │                                                       │
│   ┌───────────▼─┐ ┌─▼───────────────┐                                       │
│   │ ESPERAR     │ │ Procesar        │                                       │
│   │ (no saturar │ │ siguiente de    │                                       │
│   │  memoria)   │ │ la cola         │                                       │
│   └─────────────┘ └─────────────────┘                                       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 9.3 Optimización de Memoria en CGI

```cpp
// Líneas 1712-1750
void Server::_prepareCgiResponse(Client& client) {
    std::string& output = client.getCgiOutput();
    if (output.empty()) {
        _sendErrorResponse(client, HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    Response resp = Response::makeFromCGI(output);
    
    // ═══════════════════════════════════════════════════════════════════
    // OPTIMIZACIÓN: Liberar memoria del output CGI inmediatamente
    // El contenido ya está copiado al Response
    // ═══════════════════════════════════════════════════════════════════
    {
        std::string empty;
        client.getCgiOutput().swap(empty);  // swap-to-deallocate idiom
    }
    // En este punto, la memoria del CGI output está liberada

    // Construir respuesta HTTP
    bool keepAlive = client.shouldKeepAlive();
    client.setKeepAlive(keepAlive);
    resp.setHeader("Connection", keepAlive ? "keep-alive" : "close");

    bool isHead = (client.getRequest().getMethod() == "HEAD");
    std::string response = resp.build(isHead);

    client.appendToWriteBuffer(response);
    client.setState(CLIENT_WRITING);
    client.updateLastActivity();
}
```

**Swap-to-deallocate idiom:**

```cpp
// Problema: clear() no libera memoria reservada
std::string large_string;
large_string.reserve(100000000);  // Reserva 100MB
large_string = "data...";
large_string.clear();             // Borra contenido
// Pero la capacidad reservada (100MB) sigue asignada!

// Solución: swap con string vacío
std::string empty;
large_string.swap(empty);
// large_string ahora tiene capacidad 0
// empty se destruye inmediatamente, liberando los 100MB
```

---

# 10. ANÁLISIS DE SEGURIDAD

## 10.1 Protecciones Implementadas

### 10.1.1 Null Byte Injection

```cpp
// En Request::_parseUri()
if (uri.find('\0') != std::string::npos || 
    uri.find("%00") != std::string::npos)
{
    _errorCode = 400;
    return;
}
```

**Ataque prevenido:**
```
GET /admin/config.txt%00.jpg HTTP/1.1
                     ▲
                     │
                     Null byte

Sin protección: El servidor podría interpretar como "/admin/config.txt"
Con protección: 400 Bad Request
```

### 10.1.2 Path Traversal

```cpp
// En Utils::normalizePath()
// Elimina secuencias ../ y ./
std::string Utils::normalizePath(const std::string& path) {
    std::vector<std::string> parts;
    // ... tokenizar por / ...
    
    std::vector<std::string> normalized;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (parts[i] == "..") {
            if (!normalized.empty())
                normalized.pop_back();  // Subir un nivel
        } else if (parts[i] != ".") {
            normalized.push_back(parts[i]);
        }
    }
    // ... reconstruir path ...
}
```

**Ataque prevenido:**
```
GET /static/../../../etc/passwd HTTP/1.1

Normalización:
  /static/../../../etc/passwd
  ↓
  /etc/passwd  (pero normalizado a la raíz del servidor)
  ↓
  ./www/etc/passwd (dentro de document root)
```

### 10.1.3 Sanitización de Filenames en Upload

```cpp
// En _handleFileUpload()
std::string cleanFilename;
for (size_t j = 0; j < filename.length(); ++j)
{
    char c = filename[j];
    if (c != '/' && c != '\\' && c != ':' && c != '*' && 
        c != '?' && c != '"' && c != '<' && c != '>' && c != '|')
        cleanFilename += c;
}
```

**Caracteres peligrosos eliminados:**

| Carácter | Peligro |
|----------|---------|
| `/` `\` | Path traversal |
| `:` | Alternate Data Streams (Windows) |
| `*` `?` | Wildcards |
| `"` | Inyección de comandos |
| `<` `>` | Redirección de shell |
| `|` | Pipe de shell |

### 10.1.4 Límites de Tamaño

```cpp
// Constantes en webserv.hpp
#define MAX_HEADER_SIZE 8192        // Previene headers gigantes
#define MAX_CGI_OUTPUT_SIZE ...     // ~200MB máximo de CGI
#define MAX_CLIENTS 1024            // Límite de conexiones

// En configuración
client_max_body_size 1M;            // Límite de body configurable
```

### 10.1.5 Timeouts

```cpp
#define CONNECTION_TIMEOUT 60       // Conexiones inactivas
#define CGI_TIMEOUT 120             // CGIs colgados
#define CGI_RESPONSE_TIMEOUT 180    // Envío de respuestas grandes
```

## 10.2 Tabla de Vectores de Ataque y Mitigaciones

| Vector de Ataque | Mitigación | Ubicación |
|------------------|------------|-----------|
| Path Traversal | `normalizePath()` | Request.cpp, Utils.cpp |
| Null Byte Injection | Verificación en URI | Request.cpp |
| DoS (conexiones) | MAX_CLIENTS=1024 | Server.cpp |
| DoS (body grande) | client_max_body_size | Config |
| DoS (headers) | MAX_HEADER_SIZE | Request.cpp |
| DoS (CGI) | CGI_TIMEOUT, MAX_CONCURRENT_CGI | Server.cpp |
| Slowloris | CONNECTION_TIMEOUT | Server.cpp |
| Directory Traversal (upload) | Sanitización filename | Server.cpp |
| SIGPIPE crash | `signal(SIGPIPE, SIG_IGN)` | Server.cpp |

---

# 11. RESOLUCIÓN DE RUTAS Y ALIAS

## 11.1 Algoritmo de Resolución

```cpp
// Líneas 1648-1677
std::string Server::_resolvePath(const Request& request, 
                                 const LocationConfig* location,
                                 const ServerConfig* server)
{
    std::string uri = request.getPath();
    std::string root;

    // 1. Determinar root (prioridad: location > server > default)
    if (location != NULL && !location->getRoot().empty())
        root = location->getRoot();
    else if (server != NULL)
        root = server->getRoot();
    else
        root = "./www";

    // 2. Manejar alias (reemplazo de prefijo)
    if (location != NULL && !location->getAlias().empty())
    {
        std::string locPath = location->getPath();
        // Eliminar prefijo de location de la URI
        if (uri.find(locPath) == 0)
            uri = uri.substr(locPath.length());
        return location->getAlias() + uri;
    }

    // 3. Resolución normal: root + uri
    std::string path = root + uri;

    // 4. Normalizar (eliminar .., ., dobles //)
    path = Utils::normalizePath(path);

    return path;
}
```

## 11.2 Diferencia entre root y alias

**root:** Concatena la URI completa al directorio

```nginx
location /images {
    root /var/www;
}
# GET /images/logo.png → /var/www/images/logo.png
```

**alias:** Reemplaza el path de la location

```nginx
location /images {
    alias /var/www/static;
}
# GET /images/logo.png → /var/www/static/logo.png
```

**Diagrama de resolución:**

```
Petición: GET /api/v1/users HTTP/1.1

                    ┌─────────────────────────────────────────┐
                    │           _resolvePath()                │
                    └───────────────────┬─────────────────────┘
                                        │
                    ┌───────────────────▼───────────────────┐
                    │ location = findLocation("/api/v1/users")│
                    │ Result: location /api con alias ./backend│
                    └───────────────────┬───────────────────┘
                                        │
                    ┌───────────────────▼───────────────────┐
                    │ ¿location tiene alias?                │
                    │ SÍ: alias = ./backend                 │
                    └───────────────────┬───────────────────┘
                                        │
                    ┌───────────────────▼───────────────────┐
                    │ locPath = /api                        │
                    │ uri = /api/v1/users                   │
                    │ uri.substr(locPath.length())          │
                    │ → /v1/users                           │
                    └───────────────────┬───────────────────┘
                                        │
                    ┌───────────────────▼───────────────────┐
                    │ return alias + uri                    │
                    │ → ./backend/v1/users                  │
                    └───────────────────────────────────────┘
```

---

# 12. MANEJO DE ERRORES Y TIMEOUTS

## 12.1 Flujo de Envío de Errores

```cpp
// Líneas 1685-1710
void Server::_sendErrorResponse(Client& client, int code) {
    // 1. Crear respuesta de error
    Response resp = Response::makeError(code, client.getServerConfig());
    
    // 2. Header Allow para 405 (RFC 7231 §6.5.5)
    if (code == HTTP_METHOD_NOT_ALLOWED) {
        resp.setHeader("Allow", "GET, HEAD"); 
    }

    // 3. Determinar si cerrar conexión
    bool keepAlive = client.shouldKeepAlive();
    if (code == HTTP_BAD_REQUEST || code >= 500) {
        keepAlive = false;  // Errores graves: cerrar conexión
    }
    client.setKeepAlive(keepAlive);
    resp.setHeader("Connection", keepAlive ? "keep-alive" : "close");

    // 4. Respetar método HEAD
    bool isHead = (client.getRequest().getMethod() == "HEAD");
    std::string responseStr = resp.build(isHead);
    
    // 5. Añadir a buffer (NO enviar directamente - es non-blocking)
    client.appendToWriteBuffer(responseStr);
    client.setState(CLIENT_WRITING);
}
```

## 12.2 Códigos de Error Soportados

| Código | Constante | Mensaje | Cuándo se usa |
|--------|-----------|---------|---------------|
| 400 | HTTP_BAD_REQUEST | Bad Request | Parsing fallido, null bytes |
| 403 | HTTP_FORBIDDEN | Forbidden | Sin permisos de lectura |
| 404 | HTTP_NOT_FOUND | Not Found | Archivo no existe |
| 405 | HTTP_METHOD_NOT_ALLOWED | Method Not Allowed | Método no permitido en location |
| 413 | HTTP_PAYLOAD_TOO_LARGE | Payload Too Large | Body excede límite |
| 414 | HTTP_URI_TOO_LONG | URI Too Long | URI > 8192 bytes |
| 500 | HTTP_INTERNAL_SERVER_ERROR | Internal Server Error | Error de servidor |
| 501 | HTTP_NOT_IMPLEMENTED | Not Implemented | Método desconocido |
| 502 | HTTP_BAD_GATEWAY | Bad Gateway | CGI devolvió error |
| 504 | HTTP_GATEWAY_TIMEOUT | Gateway Timeout | CGI timeout |
| 505 | HTTP_VERSION_NOT_SUPPORTED | HTTP Version Not Supported | No HTTP/1.0 o 1.1 |

## 12.3 Cleanup al Cerrar Cliente

```cpp
// Líneas 781-822
void Server::_closeClient(int clientFd)
{
    std::map<int, Client>::iterator it = _clients.find(clientFd);
    if (it == _clients.end())
        return;

    Client& client = it->second;
    bool hadActiveCgi = false;

    // 1. Limpiar recursos CGI si los hay
    int cgiFdOut = client.getCgiFdOut();
    if (cgiFdOut >= 0)
    {
        close(cgiFdOut);
        _cgiToClient.erase(cgiFdOut);
        hadActiveCgi = true;
    }

    // 2. Matar proceso CGI si está corriendo
    pid_t pid = client.getCgiPid();
    if (pid > 0)
    {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, WNOHANG);
        hadActiveCgi = true;
    }

    // 3. Actualizar contador de CGIs activos
    if (hadActiveCgi && _activeCgiCount > 0)
    {
        --_activeCgiCount;
        // Procesar cola si hay CGIs esperando
        _processNextCgiFromQueue();
    }

    // 4. Cerrar socket
    close(clientFd);
    _clients.erase(clientFd);
}
```

**Diagrama de cleanup:**

```
┌────────────────────────────────────────────────────────────────┐
│                    _closeClient(fd)                            │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│   ┌─────────────────────────────────────────────────────────┐  │
│   │ 1. ¿Cliente tiene CGI pipe abierto?                     │  │
│   │    SÍ → close(cgiFdOut)                                 │  │
│   │       → _cgiToClient.erase(cgiFdOut)                    │  │
│   └─────────────────────────────────────────────────────────┘  │
│                              │                                 │
│                              ▼                                 │
│   ┌─────────────────────────────────────────────────────────┐  │
│   │ 2. ¿Cliente tiene proceso CGI?                          │  │
│   │    SÍ → kill(pid, SIGTERM)                              │  │
│   │       → waitpid(pid, NULL, WNOHANG)                     │  │
│   └─────────────────────────────────────────────────────────┘  │
│                              │                                 │
│                              ▼                                 │
│   ┌─────────────────────────────────────────────────────────┐  │
│   │ 3. ¿Tenía CGI activo?                                   │  │
│   │    SÍ → --_activeCgiCount                               │  │
│   │       → _processNextCgiFromQueue()                      │  │
│   └─────────────────────────────────────────────────────────┘  │
│                              │                                 │
│                              ▼                                 │
│   ┌─────────────────────────────────────────────────────────┐  │
│   │ 4. Cerrar socket y eliminar de mapa                     │  │
│   │    close(clientFd)                                      │  │
│   │    _clients.erase(clientFd)                             │  │
│   └─────────────────────────────────────────────────────────┘  │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

# RESUMEN FASE 2

## Archivos Documentados

| Archivo | Líneas | Funciones Principales |
|---------|--------|----------------------|
| Server.cpp | 1822 | run(), _handleGet/Post/Put/Delete(), _handleCgi(), _processRequest() |
| Request.cpp | 670 | parse(), _parseRequestLine(), _parseHeaders(), _parseChunkedBody() |
| Response.cpp | 455 | build(), makeError(), makeFromCGI(), makeDirectoryListing() |
| Client.cpp | 326 | shouldKeepAlive(), reset(), getters/setters |

## Patrones y Técnicas Clave

1. **Event Loop con poll()**: Multiplexación I/O no bloqueante
2. **Máquina de Estados**: Parser HTTP y estados de cliente
3. **Cola de CGI**: Control de concurrencia y memoria
4. **Factory Methods**: Construcción de respuestas HTTP
5. **Swap-to-deallocate**: Optimización de memoria

## Métricas de Código

- **Total líneas analizadas**: ~3270
- **Funciones documentadas**: ~45
- **Diagramas creados**: 15+
- **Vectores de seguridad cubiertos**: 8

---

**Fin de Fase 2 - Parte 2**

*La Fase 3 cubrirá: CGIHandler.cpp detallado, Utils.cpp, MimeTypes.cpp, comparativa exhaustiva con RFCs, y casos de prueba.*
