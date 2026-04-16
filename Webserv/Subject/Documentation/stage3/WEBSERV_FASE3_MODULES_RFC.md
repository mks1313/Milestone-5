# WEBSERV - Fase 3: Módulos Auxiliares y Comparativa RFC

## CGIHandler, Utils, MimeTypes, Config y Conformidad con Estándares

**Proyecto:** 42 Barcelona - webserv  
**Autor:** fcela-ga (Felipe Cela García)  
**Fase:** 3 de 4

---

# ÍNDICE FASE 3

1. [CGIHandler.cpp - Manejador CGI Dedicado](#1-cgihandlercpp---manejador-cgi-dedicado)
2. [Utils.cpp - Biblioteca de Utilidades](#2-utilscpp---biblioteca-de-utilidades)
3. [MimeTypes.cpp - Gestión de Content-Type](#3-mimetypescpp---gestión-de-content-type)
4. [Config.cpp - Parser de Configuración](#4-configcpp---parser-de-configuración)
5. [ServerConfig y LocationConfig](#5-serverconfig-y-locationconfig)
6. [Comparativa Exhaustiva con RFCs](#6-comparativa-exhaustiva-con-rfcs)
7. [Matriz de Cumplimiento 42](#7-matriz-de-cumplimiento-42)

---

# 1. CGIHANDLER.CPP - MANEJADOR CGI DEDICADO

## 1.1 Visión General

`CGIHandler.cpp` proporciona una clase encapsulada para la ejecución de scripts CGI según RFC 3875 (Common Gateway Interface). Aunque la implementación principal de CGI está en `Server.cpp` (para integración con el event loop asíncrono), `CGIHandler` ofrece una interfaz más limpia para ejecución síncrona.

## 1.2 Estructura de la Clase

```cpp
class CGIHandler {
private:
    // Referencias a objetos externos
    const Request* _request;             // Petición HTTP a procesar
    const ServerConfig* _serverConfig;   // Configuración del servidor
    const LocationConfig* _locationConfig; // Configuración de la location
    
    // Configuración del CGI
    std::string _scriptPath;             // Ruta absoluta al script
    std::string _cgiExecutable;          // Ruta al intérprete (python, php-cgi)
    
    // Información del cliente
    std::string _clientIp;               // IP del cliente
    int _clientPort;                     // Puerto del cliente
    
    // Resultados
    std::string _output;                 // Output completo del CGI
    int _exitStatus;                     // Código de salida del proceso
    bool _hasError;                      // Flag de error
    std::string _errorMessage;           // Mensaje de error detallado
};
```

## 1.3 Ejecución Síncrona de CGI

```cpp
// Líneas 96-130
bool CGIHandler::execute() {
    int fdIn, fdOut;
    pid_t pid;
    
    // 1. Iniciar ejecución (fork + setup pipes)
    if (!startExecution(fdIn, fdOut, pid))
        return false;
    
    // 2. Escribir body de la petición al stdin del CGI
    if (!_request->getBody().empty()) {
        write(fdIn, _request->getBody().c_str(), _request->getBody().length());
    }
    close(fdIn);  // Señalar EOF al CGI
    
    // 3. Leer output del CGI
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(fdOut, buffer, sizeof(buffer))) > 0) {
        _output.append(buffer, bytesRead);
    }
    close(fdOut);
    
    // 4. Esperar terminación del proceso hijo
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        _exitStatus = WEXITSTATUS(status);
    } else {
        _hasError = true;
        _errorMessage = "CGI process terminated abnormally";
        return false;
    }
    
    return true;
}
```

**Nota importante:** Esta versión síncrona **bloquea** el servidor durante la ejecución del CGI. La implementación en `Server.cpp` usa pipes non-blocking con `poll()` para evitar este problema.

## 1.4 Setup de Pipes y Fork

```cpp
// Líneas 132-214
bool CGIHandler::startExecution(int& fdIn, int& fdOut, pid_t& pid) {
    int pipeIn[2];   // Padre → Hijo (stdin del CGI)
    int pipeOut[2];  // Hijo → Padre (stdout del CGI)
    
    // Crear pipes
    if (pipe(pipeIn) < 0) {
        _hasError = true;
        _errorMessage = "Failed to create input pipe";
        return false;
    }
    
    if (pipe(pipeOut) < 0) {
        close(pipeIn[0]);
        close(pipeIn[1]);
        _hasError = true;
        _errorMessage = "Failed to create output pipe";
        return false;
    }
    
    pid = fork();
    if (pid < 0) {
        // Error en fork - cleanup
        close(pipeIn[0]);
        close(pipeIn[1]);
        close(pipeOut[0]);
        close(pipeOut[1]);
        _hasError = true;
        _errorMessage = "Failed to fork";
        return false;
    }
    
    if (pid == 0) {
        // ═══════════════════════════════════════════════════════════════
        // PROCESO HIJO
        // ═══════════════════════════════════════════════════════════════
        close(pipeIn[1]);   // Cerrar extremo de escritura del pipe de entrada
        close(pipeOut[0]);  // Cerrar extremo de lectura del pipe de salida
        
        // Redirigir stdin y stdout
        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);
        
        close(pipeIn[0]);
        close(pipeOut[1]);
        
        // Cambiar al directorio del script
        std::string scriptDir = Utils::getDirectory(_scriptPath);
        if (!scriptDir.empty() && scriptDir != ".") {
            if (chdir(scriptDir.c_str()) != 0) {
                std::cerr << "CGI Error: Failed to change directory to " 
                         << scriptDir << std::endl;
                exit(1);
            }
        }
        
        // Construir entorno CGI (RFC 3875)
        std::vector<std::string> envVec = _buildEnvironment();
        char** env = _envToCharArray(envVec);
        
        // Construir argumentos
        std::string scriptName = Utils::getFileName(_scriptPath);
        char* argv[3];
        argv[0] = const_cast<char*>(_cgiExecutable.c_str());
        argv[1] = const_cast<char*>(scriptName.c_str());
        argv[2] = NULL;
        
        // Ejecutar
        execve(_cgiExecutable.c_str(), argv, env);
        
        // Si llegamos aquí, execve falló
        std::cerr << "CGI Error: Failed to execute " << _cgiExecutable << std::endl;
        _freeCharArray(env);
        exit(1);
    }
    
    // ═══════════════════════════════════════════════════════════════════
    // PROCESO PADRE
    // ═══════════════════════════════════════════════════════════════════
    close(pipeIn[0]);   // Cerrar extremo de lectura del pipe de entrada
    close(pipeOut[1]);  // Cerrar extremo de escritura del pipe de salida
    
    fdIn = pipeIn[1];   // Retornar fd para escribir al CGI
    fdOut = pipeOut[0]; // Retornar fd para leer del CGI
    
    // Configurar como non-blocking
    fcntl(fdIn, F_SETFL, O_NONBLOCK);
    fcntl(fdOut, F_SETFL, O_NONBLOCK);
    
    return true;
}
```

**Diagrama de pipes:**

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         ANTES DE fork()                                  │
│                                                                         │
│   pipeIn:   [0]────────read────────[1]                                  │
│   pipeOut:  [0]────────read────────[1]                                  │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────┐
│                        DESPUÉS DE fork()                                 │
│                                                                         │
│   ┌──────────────────────┐           ┌──────────────────────┐           │
│   │   PROCESO PADRE      │           │   PROCESO HIJO       │           │
│   │                      │           │                      │           │
│   │  write() → pipeIn[1] │───────────│→ pipeIn[0] → STDIN   │           │
│   │                      │           │                      │           │
│   │  read() ← pipeOut[0] │←──────────│─ pipeOut[1] ← STDOUT │           │
│   │                      │           │                      │           │
│   │  Cerrados:           │           │  Cerrados:           │           │
│   │  - pipeIn[0]         │           │  - pipeIn[1]         │           │
│   │  - pipeOut[1]        │           │  - pipeOut[0]        │           │
│   └──────────────────────┘           └──────────────────────┘           │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

## 1.5 Construcción del Entorno CGI (RFC 3875)

```cpp
// Líneas 293-347
std::vector<std::string> CGIHandler::_buildEnvironment() const {
    std::vector<std::string> env;
    
    // ═══════════════════════════════════════════════════════════════════
    // Variables CGI requeridas por RFC 3875
    // ═══════════════════════════════════════════════════════════════════
    
    // §4.1.4: GATEWAY_INTERFACE
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    
    // §4.1.16: SERVER_PROTOCOL
    env.push_back("SERVER_PROTOCOL=" + _request->getVersion());
    
    // §4.1.17: SERVER_SOFTWARE
    env.push_back("SERVER_SOFTWARE=Webserv/1.0");
    
    // §4.1.12: REQUEST_METHOD
    env.push_back("REQUEST_METHOD=" + _request->getMethod());
    
    // §4.1.13: SCRIPT_NAME
    env.push_back("SCRIPT_NAME=" + _getScriptName());
    
    // §4.1.14: SCRIPT_FILENAME (extensión común)
    env.push_back("SCRIPT_FILENAME=" + _scriptPath);
    
    // §4.1.7: QUERY_STRING
    env.push_back("QUERY_STRING=" + _request->getQuery());
    
    // REQUEST_URI (extensión Apache/NGINX)
    env.push_back("REQUEST_URI=" + _request->getUri());
    
    // §4.1.5: PATH_INFO y §4.1.6: PATH_TRANSLATED
    std::string pathInfo = _getPathInfo();
    if (!pathInfo.empty()) {
        env.push_back("PATH_INFO=" + pathInfo);
        env.push_back("PATH_TRANSLATED=" + _getPathTranslated());
    }
    
    // ═══════════════════════════════════════════════════════════════════
    // Información del servidor
    // ═══════════════════════════════════════════════════════════════════
    if (_serverConfig) {
        // §4.1.14: SERVER_NAME
        env.push_back("SERVER_NAME=" + _serverConfig->getHost());
        
        // §4.1.15: SERVER_PORT
        env.push_back("SERVER_PORT=" + Utils::intToString(_serverConfig->getPort()));
        
        // DOCUMENT_ROOT (extensión común)
        env.push_back("DOCUMENT_ROOT=" + _serverConfig->getRoot());
    }
    
    // ═══════════════════════════════════════════════════════════════════
    // Información del cliente
    // ═══════════════════════════════════════════════════════════════════
    
    // §4.1.8: REMOTE_ADDR
    env.push_back("REMOTE_ADDR=" + _clientIp);
    
    // REMOTE_PORT (extensión)
    env.push_back("REMOTE_PORT=" + Utils::intToString(_clientPort));
    
    // ═══════════════════════════════════════════════════════════════════
    // Información del contenido
    // ═══════════════════════════════════════════════════════════════════
    
    // §4.1.3: CONTENT_TYPE
    if (_request->hasHeader("Content-Type"))
        env.push_back("CONTENT_TYPE=" + _request->getHeader("Content-Type"));
    
    // §4.1.2: CONTENT_LENGTH
    if (_request->hasHeader("Content-Length"))
        env.push_back("CONTENT_LENGTH=" + _request->getHeader("Content-Length"));
    
    // ═══════════════════════════════════════════════════════════════════
    // Headers HTTP como HTTP_* (RFC 3875 §4.1.18)
    // ═══════════════════════════════════════════════════════════════════
    const std::map<std::string, std::string>& headers = _request->getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        std::string name = "HTTP_" + Utils::toUpper(it->first);
        // Reemplazar - por _
        for (size_t i = 0; i < name.length(); ++i) {
            if (name[i] == '-')
                name[i] = '_';
        }
        env.push_back(name + "=" + it->second);
    }
    
    // REDIRECT_STATUS (requerido por php-cgi)
    env.push_back("REDIRECT_STATUS=200");
    
    return env;
}
```

**Tabla de Variables CGI (RFC 3875):**

| Variable | RFC 3875 | Descripción | Ejemplo |
|----------|----------|-------------|---------|
| `GATEWAY_INTERFACE` | §4.1.4 | Versión CGI | `CGI/1.1` |
| `SERVER_PROTOCOL` | §4.1.16 | Protocolo HTTP | `HTTP/1.1` |
| `SERVER_SOFTWARE` | §4.1.17 | ID del servidor | `Webserv/1.0` |
| `REQUEST_METHOD` | §4.1.12 | Método HTTP | `POST` |
| `SCRIPT_NAME` | §4.1.13 | Path URI al script | `/cgi-bin/test.py` |
| `QUERY_STRING` | §4.1.7 | Query string | `name=john&age=30` |
| `PATH_INFO` | §4.1.5 | Path extra después del script | `/extra/path` |
| `PATH_TRANSLATED` | §4.1.6 | PATH_INFO traducido a filesystem | `/var/www/extra/path` |
| `SERVER_NAME` | §4.1.14 | Nombre del servidor | `localhost` |
| `SERVER_PORT` | §4.1.15 | Puerto del servidor | `8080` |
| `REMOTE_ADDR` | §4.1.8 | IP del cliente | `192.168.1.100` |
| `CONTENT_TYPE` | §4.1.3 | MIME type del body | `application/json` |
| `CONTENT_LENGTH` | §4.1.2 | Tamaño del body | `1234` |
| `HTTP_*` | §4.1.18 | Headers HTTP | `HTTP_USER_AGENT` |

## 1.6 Parsing del Output CGI

```cpp
// Líneas 240-287
bool CGIHandler::parseCgiOutput(const std::string& output,
                                std::map<std::string, std::string>& headers,
                                std::string& body, int& statusCode) {
    statusCode = 200;  // Default
    
    // Buscar fin de headers (puede ser \r\n\r\n o \n\n)
    size_t headerEnd = output.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        headerEnd = output.find("\n\n");
    
    if (headerEnd == std::string::npos) {
        // Sin headers detectables - todo es body
        body = output;
        return true;
    }
    
    std::string headerPart = output.substr(0, headerEnd);
    size_t bodyStart = headerEnd + (output[headerEnd] == '\r' ? 4 : 2);
    body = output.substr(bodyStart);
    
    // Parsear headers línea por línea
    std::istringstream stream(headerPart);
    std::string line;
    while (std::getline(stream, line)) {
        // Eliminar \r si está presente
        if (!line.empty() && line[line.length() - 1] == '\r')
            line = line.substr(0, line.length() - 1);
        
        if (line.empty())
            continue;
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string name = Utils::trim(line.substr(0, colonPos));
            std::string value = Utils::trim(line.substr(colonPos + 1));
            
            // Header especial: Status (RFC 3875 §6.3.3)
            if (Utils::toLower(name) == "status") {
                std::istringstream statusStream(value);
                statusStream >> statusCode;
            } else {
                headers[name] = value;
            }
        }
    }
    
    return true;
}
```

**Ejemplo de output CGI:**

```
Content-Type: text/html; charset=utf-8
Status: 200 OK
X-Custom-Header: custom-value

<!DOCTYPE html>
<html>
<body>Hello from CGI!</body>
</html>
```

---

# 2. UTILS.CPP - BIBLIOTECA DE UTILIDADES

## 2.1 Organización del Namespace

```cpp
namespace Utils {
    // ═══════════════════════════════════════════════════════════════════
    // Categorías de funciones:
    // ═══════════════════════════════════════════════════════════════════
    
    // 1. Manipulación de strings
    // 2. Conversiones numéricas
    // 3. Operaciones de archivos
    // 4. Utilidades HTTP
    // 5. Logging
    // 6. Generación aleatoria
}
```

## 2.2 Funciones de String

```cpp
// Eliminar espacios al inicio y final
std::string trim(const std::string& str);
// "  hello world  " → "hello world"

// Conversión de caso
std::string toLower(const std::string& str);
std::string toUpper(const std::string& str);

// Dividir string por delimitador
std::vector<std::string> split(const std::string& str, char delimiter);
std::vector<std::string> split(const std::string& str, const std::string& delimiter);
// "a,b,c" con ',' → ["a", "b", "c"]

// Verificaciones de prefijo/sufijo
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::string& str, const std::string& suffix);

// Reemplazo de substrings
std::string replaceAll(const std::string& str, const std::string& from, 
                       const std::string& to);
```

## 2.3 Conversiones Numéricas

```cpp
// String → Número
int stringToInt(const std::string& str);
size_t stringToSizeT(const std::string& str);
size_t hexToSizeT(const std::string& hex);  // Para chunked encoding

// Número → String
std::string intToString(int n);
std::string sizeTToString(size_t n);
```

**Uso de hexToSizeT para chunked:**

```cpp
// Input: "1A"  (hexadecimal)
// Output: 26   (decimal)
size_t hexToSizeT(const std::string& hex) {
    size_t result = 0;
    std::istringstream stream(hex);
    stream >> std::hex >> result;
    return result;
}
```

## 2.4 Operaciones de Archivos

```cpp
// Verificaciones de existencia y permisos
bool fileExists(const std::string& path);      // stat() == 0
bool isDirectory(const std::string& path);     // S_ISDIR(mode)
bool isReadable(const std::string& path);      // access(R_OK)
bool isWritable(const std::string& path);      // access(W_OK)
bool isExecutable(const std::string& path);    // access(X_OK)

// Información de archivos
size_t getFileSize(const std::string& path);
std::string getFileExtension(const std::string& path);  // "file.txt" → "txt"
std::string getFileName(const std::string& path);       // "/path/file.txt" → "file.txt"
std::string getDirectory(const std::string& path);      // "/path/file.txt" → "/path"

// Lectura/escritura
std::string readFile(const std::string& path);
bool writeFile(const std::string& path, const std::string& content);
bool deleteFile(const std::string& path);
bool createDirectory(const std::string& path);
```

## 2.5 Normalización de Paths (Seguridad)

```cpp
// Líneas 209-248
std::string normalizePath(const std::string& path) {
    if (path.empty())
        return path;
    
    bool isRelative = (path[0] != '/');
    bool startsWithDot = (path.length() >= 2 && path[0] == '.' && path[1] == '/');
    
    std::vector<std::string> parts;
    std::vector<std::string> segments = split(path, '/');
    
    for (size_t i = 0; i < segments.size(); ++i) {
        if (segments[i] == "." || segments[i].empty())
            continue;  // Ignorar . y componentes vacíos
        
        if (segments[i] == "..") {
            // Subir un nivel si es posible
            if (!parts.empty() && parts.back() != "..")
                parts.pop_back();
            else if (isRelative)
                parts.push_back("..");  // Preservar .. en paths relativos
        } else {
            parts.push_back(segments[i]);
        }
    }
    
    // Reconstruir path
    std::string result;
    if (!isRelative)
        result = "/";
    else if (startsWithDot)
        result = "./";
    
    for (size_t i = 0; i < parts.size(); ++i) {
        result += parts[i];
        if (i < parts.size() - 1)
            result += "/";
    }
    
    if (result.empty())
        result = isRelative ? "." : "/";
    
    return result;
}
```

**Ejemplos de normalización:**

| Input | Output | Explicación |
|-------|--------|-------------|
| `/a/b/../c` | `/a/c` | `..` elimina `b` |
| `/a/./b/./c` | `/a/b/c` | `.` es ignorado |
| `/../../../etc/passwd` | `/etc/passwd` | `..` no puede subir más allá de raíz |
| `./a/../b` | `./b` | Path relativo preservado |
| `/a//b///c` | `/a/b/c` | Múltiples `/` colapsados |

## 2.6 URL Encoding/Decoding

```cpp
// Líneas 272-305
std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            // Secuencia %XX
            std::string hex = str.substr(i + 1, 2);
            char c = static_cast<char>(hexToSizeT(hex));
            result += c;
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';  // + representa espacio en query strings
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string urlEncode(const std::string& str) {
    static const char* hexChars = "0123456789ABCDEF";
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        // Caracteres seguros: alfanuméricos y -_.~
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        } else if (c == ' ') {
            result += '+';
        } else {
            // Codificar como %XX
            result += '%';
            result += hexChars[(c >> 4) & 0x0F];  // Nibble alto
            result += hexChars[c & 0x0F];          // Nibble bajo
        }
    }
    return result;
}
```

**Tabla de codificación URL común:**

| Carácter | Codificado | Uso |
|----------|------------|-----|
| ` ` (espacio) | `%20` o `+` | Separador en query |
| `!` | `%21` | - |
| `#` | `%23` | Fragment |
| `$` | `%24` | - |
| `%` | `%25` | Escape |
| `&` | `%26` | Separador query |
| `=` | `%3D` | Asignación query |
| `?` | `%3F` | Inicio query |
| `/` | `%2F` | Separador path |

## 2.7 Utilidades HTTP

```cpp
// Fecha HTTP (RFC 7231 §7.1.1.1)
std::string getHttpDate();
std::string getHttpDate(time_t timestamp);
// "Fri, 27 Dec 2024 10:30:00 GMT"

// Mensajes de estado HTTP
std::string getStatusMessage(int code);
// 200 → "OK", 404 → "Not Found", etc.

// Validación de métodos
bool isValidMethod(const std::string& method);
// true para: GET, POST, DELETE, PUT, HEAD, OPTIONS, PATCH, CONNECT, TRACE
```

## 2.8 Sistema de Logging

```cpp
// Líneas 367-393
void logInfo(const std::string& msg) {
    std::cout << "\033[0;32m[INFO]\033[0m " << msg << std::endl;
}                 // Verde

void logWarning(const std::string& msg) {
    std::cout << "\033[0;33m[WARN]\033[0m " << msg << std::endl;
}                 // Amarillo

void logError(const std::string& msg) {
    std::cerr << "\033[0;31m[ERROR]\033[0m " << msg << std::endl;
}                 // Rojo

void logDebug(const std::string& msg) {
    std::cout << "\033[0;36m[DEBUG]\033[0m " << msg << std::endl;
}                 // Cyan

void logRequest(const std::string& method, const std::string& uri, int code) {
    std::string color;
    if (code >= 200 && code < 300)
        color = "\033[0;32m";      // Verde para 2xx
    else if (code >= 300 && code < 400)
        color = "\033[0;33m";      // Amarillo para 3xx
    else
        color = "\033[0;31m";      // Rojo para 4xx/5xx
    
    std::cout << color << "[" << code << "]\033[0m " 
              << method << " " << uri << std::endl;
}
```

**Códigos de color ANSI:**

| Código | Color |
|--------|-------|
| `\033[0;31m` | Rojo |
| `\033[0;32m` | Verde |
| `\033[0;33m` | Amarillo |
| `\033[0;36m` | Cyan |
| `\033[0m` | Reset |

---

# 3. MIMETYPES.CPP - GESTIÓN DE CONTENT-TYPE

## 3.1 Patrón Singleton

```cpp
// Meyer's Singleton - Thread-safe en C++11+
MimeTypes& MimeTypes::getInstance() {
    static MimeTypes instance;  // Inicialización lazy y thread-safe
    return instance;
}

// Constructor privado - solo accesible internamente
MimeTypes::MimeTypes() {
    _initMimeTypes();  // Cargar tabla de MIME types
}
```

## 3.2 Tabla de MIME Types

La clase mantiene un `std::map<std::string, std::string>` con ~100 extensiones:

```cpp
void MimeTypes::_initMimeTypes() {
    // ═══════════════════════════════════════════════════════════════════
    // TEXTO
    // ═══════════════════════════════════════════════════════════════════
    _mimeTypes["html"] = "text/html";
    _mimeTypes["htm"] = "text/html";
    _mimeTypes["css"] = "text/css";
    _mimeTypes["js"] = "application/javascript";
    _mimeTypes["json"] = "application/json";
    _mimeTypes["xml"] = "text/xml";
    _mimeTypes["txt"] = "text/plain";
    _mimeTypes["csv"] = "text/csv";
    _mimeTypes["md"] = "text/markdown";
    
    // ═══════════════════════════════════════════════════════════════════
    // IMÁGENES
    // ═══════════════════════════════════════════════════════════════════
    _mimeTypes["jpg"] = "image/jpeg";
    _mimeTypes["jpeg"] = "image/jpeg";
    _mimeTypes["png"] = "image/png";
    _mimeTypes["gif"] = "image/gif";
    _mimeTypes["svg"] = "image/svg+xml";
    _mimeTypes["webp"] = "image/webp";
    _mimeTypes["ico"] = "image/x-icon";
    
    // ═══════════════════════════════════════════════════════════════════
    // AUDIO/VIDEO
    // ═══════════════════════════════════════════════════════════════════
    _mimeTypes["mp3"] = "audio/mpeg";
    _mimeTypes["mp4"] = "video/mp4";
    _mimeTypes["webm"] = "video/webm";
    _mimeTypes["ogg"] = "audio/ogg";
    
    // ═══════════════════════════════════════════════════════════════════
    // DOCUMENTOS
    // ═══════════════════════════════════════════════════════════════════
    _mimeTypes["pdf"] = "application/pdf";
    _mimeTypes["doc"] = "application/msword";
    _mimeTypes["docx"] = "application/vnd.openxmlformats-officedocument...";
    
    // ═══════════════════════════════════════════════════════════════════
    // ARCHIVOS
    // ═══════════════════════════════════════════════════════════════════
    _mimeTypes["zip"] = "application/zip";
    _mimeTypes["gz"] = "application/gzip";
    _mimeTypes["tar"] = "application/x-tar";
    
    // ═══════════════════════════════════════════════════════════════════
    // CÓDIGO FUENTE
    // ═══════════════════════════════════════════════════════════════════
    _mimeTypes["py"] = "text/x-python";
    _mimeTypes["php"] = "text/x-php";
    _mimeTypes["c"] = "text/x-c";
    _mimeTypes["cpp"] = "text/x-c++";
    
    // ... ~100 extensiones más ...
}
```

## 3.3 Métodos de Consulta

```cpp
// Por extensión
std::string getMimeType(const std::string& extension) const;
// getMimeType("html") → "text/html"
// getMimeType(".html") → "text/html" (con punto también funciona)
// getMimeType("xyz") → "application/octet-stream" (default)

// Por nombre de archivo
std::string getMimeTypeByFile(const std::string& filename) const;
// getMimeTypeByFile("/path/to/file.html") → "text/html"

// Clasificación
bool isTextType(const std::string& mimeType) const;
// true para: text/*, application/json, application/javascript, *+xml, *+json

bool isBinaryType(const std::string& mimeType) const;
// Inverso de isTextType
```

---

# 4. CONFIG.CPP - PARSER DE CONFIGURACIÓN

## 4.1 Sintaxis de Configuración (estilo NGINX)

```nginx
# Comentario de línea

server {
    listen 0.0.0.0:8080;
    server_name localhost www.example.com;
    root ./www;
    index index.html index.htm;
    client_max_body_size 10M;
    autoindex off;
    
    error_page 404 /errors/404.html;
    error_page 500 502 503 504 /errors/50x.html;
    
    location / {
        methods GET HEAD;
    }
    
    location /upload {
        methods GET POST DELETE;
        upload_store ./www/uploads;
        client_max_body_size 100M;
    }
    
    location /cgi-bin {
        methods GET POST;
        alias ./cgi-bin;
        cgi .py /usr/bin/python3;
        cgi .php /usr/bin/php-cgi;
        cgi .pl /usr/bin/perl;
    }
    
    location /old-page {
        redirect 301 /new-page;
    }
}
```

## 4.2 Algoritmo de Parsing

```cpp
// Líneas 51-83
void Config::parse(const std::string& filename) {
    // 1. Abrir y leer archivo
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw ConfigException("Cannot open configuration file: " + filename);
    
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    parseFromString(content);
}

void Config::parseFromString(const std::string& content) {
    // 2. Eliminar comentarios
    std::string cleanContent = content;
    _removeComments(cleanContent);
    
    // 3. Tokenizar y parsear
    std::istringstream stream(cleanContent);
    std::string token;
    
    while (!(token = _getNextToken(stream)).empty()) {
        if (token == "server") {
            _parseServer(stream);  // Parsear bloque server
        } else {
            throw ConfigException("Unexpected token: " + token);
        }
    }
    
    // 4. Validar configuración
    if (_servers.empty())
        throw ConfigException("No server block found in configuration");
    
    validate();  // Verificar consistencia
}
```

## 4.3 Tokenizador

```cpp
// Líneas 284-310
std::string Config::_getNextToken(std::istream& stream) {
    _skipWhitespaceAndComments(stream);
    
    std::string token;
    char c;
    
    while (stream.get(c)) {
        if (std::isspace(c)) {
            if (!token.empty())
                break;
            continue;
        }
        
        // Caracteres especiales son tokens individuales
        if (c == '{' || c == '}' || c == ';') {
            if (!token.empty()) {
                stream.putback(c);
                break;
            }
            token += c;
            break;
        }
        
        token += c;
    }
    
    return token;
}
```

**Ejemplo de tokenización:**

```
Input: "server { listen 8080; }"

Tokens:
1. "server"
2. "{"
3. "listen"
4. "8080"
5. ";"
6. "}"
```

## 4.4 Parsing de Tamaños

```cpp
// Líneas 333-360
size_t Config::_parseSize(const std::string& str) {
    size_t len = str.length();
    if (len == 0)
        return 0;
    
    char suffix = str[len - 1];
    std::string numPart = str;
    size_t multiplier = 1;
    
    if (!std::isdigit(suffix)) {
        numPart = str.substr(0, len - 1);
        switch (std::tolower(suffix)) {
            case 'k':
                multiplier = 1024;                    // Kilobytes
                break;
            case 'm':
                multiplier = 1024 * 1024;             // Megabytes
                break;
            case 'g':
                multiplier = 1024 * 1024 * 1024;      // Gigabytes
                break;
            default:
                throw ConfigException("Invalid size suffix: " + str);
        }
    }
    
    return Utils::stringToSizeT(numPart) * multiplier;
}
```

**Ejemplos de parsing de tamaños:**

| Input | Output (bytes) |
|-------|----------------|
| `1024` | 1024 |
| `1K` | 1024 |
| `10M` | 10,485,760 |
| `1G` | 1,073,741,824 |
| `100k` | 102,400 |

## 4.5 Validación de Configuración

```cpp
// Líneas 391-424
void Config::validate() const {
    if (_servers.empty())
        throw ConfigException("No server blocks defined");
    
    std::set<std::pair<std::string, int> > listeners;
    
    for (size_t i = 0; i < _servers.size(); ++i) {
        const ServerConfig& server = _servers[i];
        
        if (!server.isValid())
            throw ConfigException("Invalid server configuration");
        
        std::pair<std::string, int> listener(server.getHost(), server.getPort());
        
        if (listeners.find(listener) != listeners.end()) {
            // Mismo host:port - verificar que server_names sean diferentes
            for (size_t j = 0; j < i; ++j) {
                if (_servers[j].getHost() == server.getHost() &&
                    _servers[j].getPort() == server.getPort()) {
                    // Verificar conflictos de server_name
                    const std::vector<std::string>& names1 = server.getServerNames();
                    const std::vector<std::string>& names2 = _servers[j].getServerNames();
                    for (size_t k = 0; k < names1.size(); ++k) {
                        for (size_t l = 0; l < names2.size(); ++l) {
                            if (names1[k] == names2[l])
                                throw ConfigException(
                                    "Duplicate server_name for same listen address");
                        }
                    }
                }
            }
        }
        listeners.insert(listener);
    }
}
```

---

# 5. SERVERCONFIG Y LOCATIONCONFIG

## 5.1 ServerConfig

```cpp
class ServerConfig {
private:
    std::string _host;                           // IP de escucha
    int _port;                                   // Puerto
    std::vector<std::string> _serverNames;       // Nombres de servidor (virtual hosts)
    std::string _root;                           // Document root
    std::string _index;                          // Archivo índice por defecto
    size_t _maxBodySize;                         // Límite de body
    bool _autoindex;                             // Listado de directorios
    std::map<int, std::string> _errorPages;      // Páginas de error personalizadas
    std::vector<LocationConfig> _locations;      // Bloques location
    
public:
    // Búsqueda de location por URI (longest prefix match)
    const LocationConfig* findLocation(const std::string& uri) const;
    
    // Verificación de server_name
    bool matchServerName(const std::string& host) const;
};
```

## 5.2 Algoritmo Longest Prefix Match

```cpp
const LocationConfig* ServerConfig::findLocation(const std::string& uri) const {
    const LocationConfig* bestMatch = NULL;
    size_t bestLength = 0;
    
    for (size_t i = 0; i < _locations.size(); ++i) {
        const std::string& locPath = _locations[i].getPath();
        
        // Verificar si la URI empieza con el path de la location
        if (uri.compare(0, locPath.length(), locPath) == 0) {
            // Seleccionar el match más largo
            if (locPath.length() > bestLength) {
                bestMatch = &_locations[i];
                bestLength = locPath.length();
            }
        }
    }
    
    return bestMatch;
}
```

**Ejemplo de longest prefix match:**

```
Locations configuradas:
- /
- /api
- /api/v1
- /static

URI: /api/v1/users/123

Matches:
- / ✓ (longitud 1)
- /api ✓ (longitud 4)
- /api/v1 ✓ (longitud 7) ← GANADOR (más largo)
- /static ✗ (no coincide)

Resultado: location /api/v1
```

## 5.3 LocationConfig

```cpp
class LocationConfig {
private:
    std::string _path;                           // Path de la location
    std::string _root;                           // Root override
    std::string _alias;                          // Alias (reemplazo de path)
    std::string _index;                          // Índice override
    bool _autoindex;                             // Autoindex override
    size_t _maxBodySize;                         // Max body override
    std::set<std::string> _allowedMethods;       // Métodos permitidos
    std::map<std::string, std::string> _cgiHandlers;  // Extensión → handler
    bool _uploadEnabled;                         // Upload habilitado
    std::string _uploadPath;                     // Directorio de uploads
    int _redirectCode;                           // Código de redirección
    std::string _redirect;                       // URL de redirección
    
public:
    bool isMethodAllowed(const std::string& method) const;
    bool hasRedirect() const;
    std::string getCgiHandler(const std::string& extension) const;
};
```

---

# 6. COMPARATIVA EXHAUSTIVA CON RFCS

## 6.1 RFC 7230 - HTTP/1.1 Message Syntax and Routing

| Sección | Requisito | Implementado | Notas |
|---------|-----------|--------------|-------|
| §2.6 | Versiones HTTP/1.0 y HTTP/1.1 | ✅ | Detectado en request-line |
| §3.1.1 | Request-line: method SP request-target SP HTTP-version CRLF | ✅ | Request::_parseRequestLine() |
| §3.1.2 | Status-line: HTTP-version SP status-code SP reason-phrase CRLF | ✅ | Response::buildHeaders() |
| §3.2 | Header fields: field-name ":" OWS field-value OWS | ✅ | Request::_parseHeader() |
| §3.2.4 | Nombres de header case-insensitive | ✅ | Utils::toLower() |
| §3.3 | Message body | ✅ | Request::_body |
| §3.3.1 | Transfer-Encoding | ✅ | Chunked soportado |
| §3.3.2 | Content-Length | ✅ | Parsing y validación |
| §3.3.3 | Message body length determination | ✅ | Prioridad chunked > C-L |
| §3.5 | Tolerar líneas vacías antes de request-line | ✅ | Request::parse() |
| §4.1 | Chunked transfer encoding | ✅ | Request::_parseChunkedBody() |
| §4.1.1 | Chunk format: size CRLF data CRLF | ✅ | Implementado |
| §4.1.3 | Last-chunk: 0 CRLF | ✅ | Detectado y manejado |
| §5.3 | Request target forms | ⚠️ | Solo origin-form |
| §5.4 | Host header requerido en HTTP/1.1 | ✅ | Validado |
| §6.1 | Connection header | ✅ | Parseado |
| §6.3 | Persistent connections | ✅ | Keep-alive por defecto en 1.1 |
| §6.3.2 | Pipelining | ✅ | Soporte completo |
| §6.6 | Tear-down | ✅ | close() apropiado |

## 6.2 RFC 7231 - HTTP/1.1 Semantics and Content

| Sección | Requisito | Implementado | Notas |
|---------|-----------|--------------|-------|
| §4.1 | Request methods overview | ✅ | GET, POST, PUT, DELETE, HEAD |
| §4.2.1 | Safe methods | ✅ | GET, HEAD son safe |
| §4.2.2 | Idempotent methods | ✅ | GET, HEAD, PUT, DELETE |
| §4.3.1 | GET | ✅ | Retorna recurso |
| §4.3.2 | HEAD | ✅ | Como GET pero sin body |
| §4.3.3 | POST | ✅ | Procesar con CGI o upload |
| §4.3.4 | PUT | ✅ | Crear/actualizar recurso |
| §4.3.5 | DELETE | ✅ | Eliminar recurso |
| §5.1.1 | Expect header | ❌ | No implementado |
| §6.1 | Status codes overview | ✅ | 2xx, 3xx, 4xx, 5xx |
| §6.2.1 | 200 OK | ✅ | - |
| §6.3.1 | 201 Created | ✅ | PUT exitoso |
| §6.3.5 | 204 No Content | ✅ | DELETE exitoso |
| §6.4.2 | 301 Moved Permanently | ✅ | Redirección |
| §6.4.3 | 302 Found | ✅ | Redirección |
| §6.5.1 | 400 Bad Request | ✅ | Error de parsing |
| §6.5.3 | 403 Forbidden | ✅ | Sin permisos |
| §6.5.4 | 404 Not Found | ✅ | Recurso no existe |
| §6.5.5 | 405 Method Not Allowed | ✅ | Con header Allow |
| §6.5.11 | 413 Payload Too Large | ✅ | Body excede límite |
| §6.5.12 | 414 URI Too Long | ✅ | URI > 8192 |
| §6.6.1 | 500 Internal Server Error | ✅ | Error de servidor |
| §6.6.2 | 501 Not Implemented | ✅ | Método desconocido |
| §6.6.4 | 503 Service Unavailable | ✅ | Disponible |
| §6.6.5 | 505 HTTP Version Not Supported | ✅ | No 1.0/1.1 |
| §7.1.1.1 | Date header | ✅ | En cada respuesta |
| §7.1.2 | Location header | ✅ | En redirecciones |
| §7.4.1 | Allow header | ✅ | En 405 |
| §7.4.2 | Server header | ✅ | "Webserv/1.0" |

## 6.3 RFC 7578 - multipart/form-data

| Sección | Requisito | Implementado | Notas |
|---------|-----------|--------------|-------|
| §4.1 | Boundary parameter | ✅ | Extraído de Content-Type |
| §4.2 | Content-Disposition header | ✅ | Parseado para name y filename |
| §4.4 | Multiple files | ✅ | Vector de UploadedFile |
| §5.1 | Non-ASCII filenames | ⚠️ | No RFC 5987 encoding |

## 6.4 RFC 3875 - CGI/1.1

| Sección | Requisito | Implementado | Notas |
|---------|-----------|--------------|-------|
| §4.1.1 | AUTH_TYPE | ❌ | No autenticación |
| §4.1.2 | CONTENT_LENGTH | ✅ | Del header |
| §4.1.3 | CONTENT_TYPE | ✅ | Del header |
| §4.1.4 | GATEWAY_INTERFACE | ✅ | "CGI/1.1" |
| §4.1.5 | PATH_INFO | ✅ | Path extra |
| §4.1.6 | PATH_TRANSLATED | ✅ | Traducido |
| §4.1.7 | QUERY_STRING | ✅ | Query string |
| §4.1.8 | REMOTE_ADDR | ✅ | IP cliente |
| §4.1.9 | REMOTE_HOST | ⚠️ | Igual que REMOTE_ADDR |
| §4.1.10 | REMOTE_IDENT | ❌ | No implementado |
| §4.1.11 | REMOTE_USER | ❌ | No autenticación |
| §4.1.12 | REQUEST_METHOD | ✅ | Método HTTP |
| §4.1.13 | SCRIPT_NAME | ✅ | Path al script |
| §4.1.14 | SERVER_NAME | ✅ | Hostname |
| §4.1.15 | SERVER_PORT | ✅ | Puerto |
| §4.1.16 | SERVER_PROTOCOL | ✅ | "HTTP/1.1" |
| §4.1.17 | SERVER_SOFTWARE | ✅ | "Webserv/1.0" |
| §4.1.18 | HTTP_* variables | ✅ | Todos los headers |
| §6.2.1 | Document response | ✅ | Output parseado |
| §6.3.1 | Content-Type header | ✅ | Requerido o default |
| §6.3.3 | Status header | ✅ | Parseado |

## 6.5 RFC 6265 - HTTP Cookies

| Sección | Requisito | Implementado | Notas |
|---------|-----------|--------------|-------|
| §4.1 | Set-Cookie header | ✅ | Response::setCookie() |
| §4.1.1 | Cookie attribute syntax | ✅ | Path, Max-Age, HttpOnly |
| §4.2 | Cookie header | ✅ | Request::_parseCookies() |
| §5.2 | Cookie parsing | ✅ | Nombre=valor |
| §5.4 | HttpOnly attribute | ✅ | Soportado |

---

# 7. MATRIZ DE CUMPLIMIENTO 42

## 7.1 Requisitos Obligatorios

| Requisito | Estado | Implementación |
|-----------|--------|----------------|
| Servidor HTTP en C++98 | ✅ | Compilado con -std=c++98 |
| poll() o equivalente | ✅ | poll() en Server.cpp |
| Non-blocking I/O | ✅ | fcntl(O_NONBLOCK) |
| Un solo poll() para todo | ✅ | _rebuildPollFds() |
| Configuración en archivo | ✅ | Config.cpp parser NGINX-like |
| Puerto y host configurables | ✅ | listen directive |
| server_name | ✅ | Virtual hosts |
| error_page | ✅ | Páginas personalizadas |
| client_max_body_size | ✅ | Límite configurable |
| Métodos GET, POST, DELETE | ✅ | Handlers dedicados |
| Servir archivos estáticos | ✅ | _handleGet() |
| File upload | ✅ | Multipart parsing |
| CGI (al menos un tipo) | ✅ | Python, PHP, Perl, Shell |
| Autoindex | ✅ | Directory listing |
| Redirecciones | ✅ | 301, 302 |
| No crash | ✅ | Manejo de errores robusto |
| Sin memory leaks | ⚠️ | Revisar con valgrind |

## 7.2 Requisitos Bonus

| Bonus | Estado | Implementación |
|-------|--------|----------------|
| Cookies | ✅ | Parsing y Set-Cookie |
| Sesiones | ✅ | SessionManager Singleton |
| Múltiples CGI | ✅ | .py, .php, .pl, .sh |

## 7.3 Requisitos del Tester 42

| Test | Estado | Notas |
|------|--------|-------|
| GET básico | ✅ | - |
| POST con body | ✅ | - |
| DELETE | ✅ | - |
| Chunked transfer | ✅ | - |
| CGI con POST | ✅ | - |
| CGI con body grande | ✅ | ~100MB soportado |
| Múltiples peticiones | ✅ | Keep-alive |
| Pipelining | ✅ | Soporte completo |
| Errores 4xx | ✅ | - |
| Errores 5xx | ✅ | - |
| Timeout de conexión | ✅ | 60s |
| Timeout de CGI | ✅ | 120s |
| PATH_INFO compatible | ⚠️ | Adaptado para tester |

---

# RESUMEN FASE 3

## Archivos Documentados

| Archivo | Líneas | Propósito |
|---------|--------|-----------|
| CGIHandler.cpp | 407 | Ejecución de scripts CGI |
| Utils.cpp | 419 | Funciones de utilidad |
| MimeTypes.cpp | 276 | Mapeo extensión → Content-Type |
| Config.cpp | 452 | Parser de configuración |
| ServerConfig.cpp | ~200 | Configuración de servidor |
| LocationConfig.cpp | ~150 | Configuración de location |

## Funciones Documentadas

- **CGIHandler**: 15 métodos
- **Utils**: 30+ funciones
- **MimeTypes**: 6 métodos, ~100 extensiones
- **Config**: 12 métodos de parsing

## Conformidad RFC

| RFC | Conformidad | Notas |
|-----|-------------|-------|
| RFC 7230 | ~95% | Falta support forms |
| RFC 7231 | ~90% | Falta Expect header |
| RFC 7578 | ~85% | Falta RFC 5987 |
| RFC 3875 | ~90% | Variables básicas |
| RFC 6265 | ~80% | Cookies básicas |

---

**Fin de Fase 3**

*La Fase 4 cubrirá: Guía de debugging, casos de prueba, optimizaciones posibles, y manual de uso.*
