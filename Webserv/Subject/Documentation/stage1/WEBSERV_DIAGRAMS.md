# WEBSERV - Diagramas de Flujo (Mermaid)

Este archivo contiene los diagramas de flujo del servidor webserv en formato Mermaid.
Para visualizarlos, usa un visor de Markdown compatible (VS Code, GitHub, etc.) o
copia el código a https://mermaid.live

---

## 1. Diagrama de Arquitectura General

```mermaid
graph TB
    subgraph CONFIG["📁 Configuración"]
        A[Config.cpp] --> B[ServerConfig]
        A --> C[LocationConfig]
    end
    
    subgraph SERVER["🖥️ Servidor"]
        D[Server.cpp] --> E[poll loop]
        E --> F[Client Handler]
    end
    
    subgraph HTTP["📨 HTTP"]
        G[Request Parser] --> H[Response Builder]
    end
    
    subgraph CGI["⚡ CGI"]
        I[CGIHandler] --> J[fork/exec]
    end
    
    subgraph SESSION["🔐 Sesiones"]
        K[SessionManager] --> L[Cookies]
    end
    
    CONFIG --> SERVER
    SERVER --> HTTP
    SERVER --> CGI
    HTTP --> SESSION
```

---

## 2. Bucle Principal del Servidor

```mermaid
flowchart TD
    START([Inicio]) --> INIT[Inicializar Server]
    INIT --> BUILD[Reconstruir pollfd array]
    BUILD --> POLL["poll(fds, 1000ms)"]
    POLL --> CHECK{¿Eventos?}
    
    CHECK -->|No timeout| TIMEOUT[Verificar Timeouts]
    TIMEOUT --> CLEAN[Limpiar sesiones]
    CLEAN --> BUILD
    
    CHECK -->|Sí| TYPE{Tipo de FD}
    
    TYPE -->|Listen Socket| ACCEPT["accept()"]
    ACCEPT --> NEWCLIENT[Crear Cliente]
    NEWCLIENT --> BUILD
    
    TYPE -->|Client POLLIN| READ["recv()"]
    READ --> PARSE[Parsear HTTP]
    PARSE --> COMPLETE{¿Completa?}
    COMPLETE -->|No| BUILD
    COMPLETE -->|Sí| PROCESS[Procesar Petición]
    PROCESS --> RESPONSE[Preparar Response]
    RESPONSE --> BUILD
    
    TYPE -->|Client POLLOUT| WRITE["send()"]
    WRITE --> EMPTY{¿Buffer vacío?}
    EMPTY -->|No| BUILD
    EMPTY -->|Sí| KEEPALIVE{¿Keep-Alive?}
    KEEPALIVE -->|Sí| RESET[Reset cliente]
    RESET --> BUILD
    KEEPALIVE -->|No| CLOSE[Cerrar conexión]
    CLOSE --> BUILD
    
    TYPE -->|CGI Pipe| CGIREAD[Leer CGI output]
    CGIREAD --> EOF{¿EOF?}
    EOF -->|No| BUILD
    EOF -->|Sí| CGIRESP[Parsear CGI response]
    CGIRESP --> BUILD
    
    TYPE -->|Error| CLOSEERR[Cerrar conexión]
    CLOSEERR --> BUILD
```

---

## 3. Procesamiento de Petición HTTP

```mermaid
flowchart TD
    REQ([Petición Recibida]) --> SELECT[Seleccionar Virtual Host]
    SELECT --> FINDLOC[Buscar Location]
    FINDLOC --> REDIRECT{¿Redirect?}
    
    REDIRECT -->|Sí| REDIR[Responder 301/302]
    REDIRECT -->|No| METHOD{¿Método Permitido?}
    
    METHOD -->|No| M405[405 Method Not Allowed]
    METHOD -->|Sí| BODY{¿Body Size OK?}
    
    BODY -->|No| B413[413 Payload Too Large]
    BODY -->|Sí| ROUTE{Router por Método}
    
    ROUTE -->|GET/HEAD| GET[Handle GET]
    ROUTE -->|POST| POST[Handle POST]
    ROUTE -->|PUT| PUT[Handle PUT]
    ROUTE -->|DELETE| DELETE[Handle DELETE]
    
    GET --> CGI1{¿Es CGI?}
    CGI1 -->|Sí| CGIG[Ejecutar CGI]
    CGI1 -->|No| FILE{¿Es Directorio?}
    
    FILE -->|Sí| INDEX{¿Hay index?}
    INDEX -->|Sí| SERVE[Servir archivo]
    INDEX -->|No| AUTO{¿Autoindex ON?}
    AUTO -->|Sí| LISTING[Directory Listing]
    AUTO -->|No| N404[404 Not Found]
    
    FILE -->|No| SERVE
    
    POST --> CGI2{¿Es CGI?}
    CGI2 -->|Sí| CGIP[Ejecutar CGI]
    CGI2 -->|No| UPLOAD{¿Upload habilitado?}
    UPLOAD -->|Sí| SAVEUP[Guardar archivos]
    UPLOAD -->|No| P204[204 No Content]
    
    PUT --> PUTFILE[Crear/Sobrescribir archivo]
    PUTFILE --> PUT201[200/201 Response]
    
    DELETE --> EXISTS{¿Archivo existe?}
    EXISTS -->|No| D404[404 Not Found]
    EXISTS -->|Sí| ISFILE{¿Es archivo?}
    ISFILE -->|No| D403[403 Forbidden]
    ISFILE -->|Sí| REMOVE[Eliminar archivo]
    REMOVE --> D204[204 No Content]
    
    REDIR --> RESP([Response])
    M405 --> RESP
    B413 --> RESP
    CGIG --> RESP
    CGIP --> RESP
    SERVE --> RESP
    LISTING --> RESP
    N404 --> RESP
    SAVEUP --> RESP
    P204 --> RESP
    PUT201 --> RESP
    D404 --> RESP
    D403 --> RESP
    D204 --> RESP
```

---

## 4. Parser de Request HTTP

```mermaid
stateDiagram-v2
    [*] --> PARSE_REQUEST_LINE
    
    PARSE_REQUEST_LINE: Parseando Request Line
    PARSE_HEADERS: Parseando Headers
    PARSE_BODY: Parseando Body (Content-Length)
    PARSE_CHUNKED: Parseando Body (Chunked)
    PARSE_COMPLETE: Parsing Completado
    PARSE_ERROR: Error de Parsing
    
    PARSE_REQUEST_LINE --> PARSE_HEADERS: Request line OK
    PARSE_REQUEST_LINE --> PARSE_ERROR: Formato inválido
    
    PARSE_HEADERS --> PARSE_BODY: Content-Length presente
    PARSE_HEADERS --> PARSE_CHUNKED: Transfer-Encoding: chunked
    PARSE_HEADERS --> PARSE_COMPLETE: Sin body
    PARSE_HEADERS --> PARSE_ERROR: Header inválido
    
    PARSE_BODY --> PARSE_COMPLETE: Body completo
    PARSE_BODY --> PARSE_ERROR: Error
    
    PARSE_CHUNKED --> PARSE_COMPLETE: Chunk final (0)
    PARSE_CHUNKED --> PARSE_ERROR: Error
    
    PARSE_COMPLETE --> [*]
    PARSE_ERROR --> [*]
```

---

## 5. Ejecución de CGI

```mermaid
sequenceDiagram
    participant C as Cliente
    participant S as Servidor
    participant P as Proceso CGI
    
    C->>S: POST /cgi-bin/script.py
    
    Note over S: Verificar límite CGI
    
    alt _activeCgiCount < MAX_CONCURRENT_CGI
        S->>S: Crear tmpfile para body
        S->>S: Crear pipe para stdout
        S->>P: fork()
        
        activate P
        P->>P: dup2(tmpfile, STDIN)
        P->>P: dup2(pipe, STDOUT)
        P->>P: Configurar env vars
        P->>P: chdir(script_dir)
        P->>P: execve(handler, script, env)
        
        loop Mientras CGI ejecuta
            S->>S: poll() en pipe
            P-->>S: Output (chunks)
            S->>S: Acumular en buffer
        end
        
        P-->>S: EOF (termina)
        deactivate P
        
        S->>S: waitpid()
        S->>S: Parsear headers CGI
        S->>S: Construir Response
        S-->>C: HTTP Response
        
    else Cola llena
        S->>S: Encolar petición
        Note over S: Esperar slot disponible
    end
```

---

## 6. Gestión de Conexiones Keep-Alive

```mermaid
sequenceDiagram
    participant C as Cliente
    participant S as Servidor
    
    C->>S: GET /page1.html HTTP/1.1
    Note right of C: Connection: keep-alive
    S-->>C: HTTP/1.1 200 OK
    Note left of S: Connection: keep-alive
    
    Note over C,S: Conexión se mantiene abierta
    
    C->>S: GET /style.css HTTP/1.1
    S-->>C: HTTP/1.1 200 OK
    
    C->>S: GET /script.js HTTP/1.1
    S-->>C: HTTP/1.1 200 OK
    
    C->>S: GET /image.png HTTP/1.1
    Note right of C: Connection: close
    S-->>C: HTTP/1.1 200 OK
    Note left of S: Connection: close
    
    Note over C,S: Conexión cerrada
```

---

## 7. Selección de Virtual Host

```mermaid
flowchart TD
    REQ([Petición con Host header]) --> PORT[Filtrar por Puerto]
    PORT --> SERVERS[Lista de servidores en ese puerto]
    SERVERS --> FIRST{¿Primer servidor?}
    FIRST -->|Sí| DEFAULT[Guardar como default]
    DEFAULT --> MATCH
    FIRST -->|No| MATCH{¿server_name coincide?}
    
    MATCH -->|Sí| FOUND([Servidor encontrado])
    MATCH -->|No| NEXT{¿Más servidores?}
    NEXT -->|Sí| SERVERS
    NEXT -->|No| USEDEF([Usar servidor default])
```

---

## 8. Estados del Cliente

```mermaid
stateDiagram-v2
    [*] --> CLIENT_READING: Nueva conexión
    
    CLIENT_READING: Leyendo petición
    CLIENT_PROCESSING: Procesando / En cola CGI
    CLIENT_WRITING: Escribiendo respuesta
    CLIENT_CGI_RUNNING: CGI en ejecución
    CLIENT_DONE: Completado
    CLIENT_ERROR: Error
    
    CLIENT_READING --> CLIENT_PROCESSING: Petición completa
    CLIENT_READING --> CLIENT_ERROR: Error de parsing
    CLIENT_READING --> CLIENT_ERROR: Timeout
    
    CLIENT_PROCESSING --> CLIENT_WRITING: Response lista
    CLIENT_PROCESSING --> CLIENT_CGI_RUNNING: Iniciar CGI
    CLIENT_PROCESSING --> CLIENT_ERROR: Error
    
    CLIENT_CGI_RUNNING --> CLIENT_WRITING: CGI terminado
    CLIENT_CGI_RUNNING --> CLIENT_ERROR: CGI timeout
    CLIENT_CGI_RUNNING --> CLIENT_ERROR: CGI error
    
    CLIENT_WRITING --> CLIENT_DONE: Buffer enviado
    CLIENT_WRITING --> CLIENT_READING: Keep-Alive + Reset
    CLIENT_WRITING --> CLIENT_ERROR: Error de envío
    
    CLIENT_DONE --> [*]
    CLIENT_ERROR --> [*]
```

---

## 9. Parsing de Multipart/form-data

```mermaid
flowchart TD
    START([Body recibido]) --> BOUNDARY[Extraer boundary de Content-Type]
    BOUNDARY --> FIND[Buscar delimitador]
    FIND --> FOUND{¿Encontrado?}
    
    FOUND -->|No| END([Fin del parsing])
    FOUND -->|Sí| CHECK{¿Es final --?}
    CHECK -->|Sí| END
    CHECK -->|No| HEADERS[Parsear headers de parte]
    
    HEADERS --> CD[Extraer Content-Disposition]
    CD --> NAME[Extraer name]
    CD --> FILENAME[Extraer filename]
    HEADERS --> CT[Extraer Content-Type]
    
    NAME --> DATA[Extraer datos de la parte]
    FILENAME --> DATA
    CT --> DATA
    
    DATA --> FILE[Crear UploadedFile struct]
    FILE --> ADD[Añadir a _uploadedFiles]
    ADD --> FIND
```

---

## 10. Timeouts y Limpieza

```mermaid
flowchart TD
    LOOP([Cada iteración del bucle]) --> CHECK[Verificar timeouts]
    CHECK --> ITER[Para cada cliente]
    
    ITER --> STATE{Estado del cliente}
    
    STATE -->|CGI_RUNNING| CGITO{¿CGI timeout?}
    CGITO -->|Sí| CLOSE1[Cerrar cliente]
    CGITO -->|No| NEXT
    
    STATE -->|WRITING + Big Buffer| BIGTO{¿CGI response timeout?}
    BIGTO -->|Sí| CLOSE2[Cerrar cliente]
    BIGTO -->|No| NEXT
    
    STATE -->|Otro| NORMTO{¿Connection timeout?}
    NORMTO -->|Sí| CLOSE3[Cerrar cliente]
    NORMTO -->|No| NEXT
    
    STATE -->|PROCESSING| SKIP[Skip - en cola CGI]
    SKIP --> NEXT
    
    NEXT{¿Más clientes?} -->|Sí| ITER
    NEXT -->|No| SESSION[Limpiar sesiones]
    
    SESSION --> EACH[Para cada sesión]
    EACH --> EXPIRED{¿Expirada?}
    EXPIRED -->|Sí| DESTROY[Destruir sesión]
    EXPIRED -->|No| NEXTSES
    DESTROY --> NEXTSES
    NEXTSES{¿Más sesiones?} -->|Sí| EACH
    NEXTSES -->|No| DONE([Continuar bucle])
    
    CLOSE1 --> NEXT
    CLOSE2 --> NEXT
    CLOSE3 --> NEXT
```

---

## 11. Secuencia de Petición HTTP Completa

```mermaid
sequenceDiagram
    participant Browser as 🌐 Navegador
    participant Server as 🖥️ Webserv
    participant FS as 📁 Sistema de Archivos
    participant CGI as ⚡ Proceso CGI
    
    Browser->>Server: TCP Connect
    Note over Server: accept() → nuevo fd
    Note over Server: Estado: READING
    
    Browser->>Server: GET /page.html HTTP/1.1
    Browser->>Server: Host: localhost:8080
    Browser->>Server: (headers...)
    Browser->>Server: (línea vacía)
    
    Note over Server: Parser: REQUEST_LINE
    Note over Server: Parser: HEADERS
    Note over Server: Parser: COMPLETE
    
    Server->>Server: Seleccionar Virtual Host
    Server->>Server: Buscar Location
    Server->>Server: Verificar método GET
    
    Server->>FS: stat("/var/www/page.html")
    FS-->>Server: OK, es archivo
    Server->>FS: read("/var/www/page.html")
    FS-->>Server: Contenido HTML
    
    Note over Server: Estado: WRITING
    Server-->>Browser: HTTP/1.1 200 OK
    Server-->>Browser: Content-Type: text/html
    Server-->>Browser: Content-Length: 1234
    Server-->>Browser: (línea vacía)
    Server-->>Browser: (contenido HTML)
    
    Note over Server: ¿Keep-Alive?
    alt Keep-Alive
        Note over Server: Reset cliente, Estado: READING
    else Close
        Server->>Browser: TCP Close
    end
```

---

## Notas de Visualización

Para visualizar estos diagramas:

1. **VS Code**: Instala la extensión "Markdown Preview Mermaid Support"
2. **GitHub**: Los diagramas se renderizan automáticamente en archivos .md
3. **Online**: Copia el código a https://mermaid.live
4. **Obsidian**: Soporte nativo de Mermaid
5. **GitLab**: Soporte nativo en archivos Markdown

---

*Diagramas generados para la documentación de webserv - 42 Barcelona*
