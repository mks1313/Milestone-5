#!/bin/bash
# ============================================================================
# import_issues_to_github.sh - VERSIÓN CORREGIDA
# 
# Script para crear issues en GitHub desde el plan de desarrollo de webserv
# Requiere: GitHub CLI (gh) instalado y autenticado
#
# CORRECCIONES:
#   - Usa --body-file para preservar formato markdown
#   - Desactiva el pager para evitar bloqueos con 'Q'
#   - No usa eval para evitar corrupción de caracteres
#
# Uso:
#   ./import_issues_to_github.sh owner/repo
#   ./import_issues_to_github.sh felipecela/42-C06-Prj14-Webserv
# ============================================================================

set -e

# CRÍTICO: Desactivar el pager de gh para evitar bloqueos
export GH_PAGER=""
export PAGER=""

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Verificar argumentos
if [ -z "$1" ]; then
    echo -e "${RED}Error: Debes especificar el repositorio${NC}"
    echo "Uso: $0 owner/repo"
    echo "Ejemplo: $0 felipecela/42-C06-Prj14-Webserv"
    exit 1
fi

REPO="$1"

# Verificar gh está instalado
if ! command -v gh &> /dev/null; then
    echo -e "${RED}Error: GitHub CLI (gh) no está instalado${NC}"
    echo "Instalar con:"
    echo "  Ubuntu/Debian: sudo apt install gh"
    echo "  Mac: brew install gh"
    exit 1
fi

# Verificar autenticación
if ! gh auth status &> /dev/null; then
    echo -e "${RED}Error: No estás autenticado en GitHub CLI${NC}"
    echo "Ejecuta: gh auth login"
    exit 1
fi

# Crear directorio temporal para los body files
TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

echo -e "${BLUE}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║     Importador de Issues - Webserv Development Plan          ║${NC}"
echo -e "${BLUE}║                    (Versión Corregida)                        ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "Repositorio: ${GREEN}$REPO${NC}"
echo -e "Temp dir: ${CYAN}$TEMP_DIR${NC}"
echo ""

# ============================================================================
# Paso 1: Crear Labels
# ============================================================================
echo -e "${YELLOW}[1/3] Creando labels...${NC}"

declare -A LABELS=(
    ["epic-1-foundation"]="E1F5FE:Epic 1 - Fundamentos y Utilidades"
    ["epic-2-networking"]="FFF3E0:Epic 2 - Infraestructura de Red"
    ["epic-3-http-parser"]="F3E5F5:Epic 3 - Parser HTTP"
    ["epic-4-config"]="E8F5E9:Epic 4 - Sistema de Configuración"
    ["epic-5-handlers"]="FCE4EC:Epic 5 - Handlers HTTP"
    ["epic-6-cgi"]="FFF8E1:Epic 6 - CGI"
    ["epic-7-advanced"]="E0F2F1:Epic 7 - Features Avanzadas"
    ["critical"]="D32F2F:Tarea crítica para el funcionamiento"
    ["enhancement"]="A2EEEF:Nueva funcionalidad"
    ["testing"]="FBCA04:Tests y QA"
    ["documentation"]="0075CA:Documentación"
)

for label in "${!LABELS[@]}"; do
    IFS=':' read -r color description <<< "${LABELS[$label]}"
    echo -n "  Creando label '$label'... "
    if gh label create "$label" --repo "$REPO" --color "$color" --description "$description" 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
    else
        echo -e "${YELLOW}(ya existe)${NC}"
    fi
done

# ============================================================================
# Paso 2: Crear Milestones
# ============================================================================
echo ""
echo -e "${YELLOW}[2/3] Creando milestones...${NC}"

declare -A MILESTONES=(
    ["v0.1-foundation"]="Fundamentos: Utils, MimeTypes, webserv.hpp"
    ["v0.2-networking"]="Infraestructura de red: Server, Client, poll()"
    ["v0.3-http-parser"]="Parser HTTP: Request y Response"
    ["v0.4-config"]="Sistema de configuración estilo NGINX"
    ["v0.5-handlers"]="Handlers HTTP: GET, POST, DELETE"
    ["v0.6-cgi"]="Soporte CGI: fork, exec, pipes"
    ["v0.7-advanced"]="Features avanzadas: Sessions, Upload, Keep-alive"
)

for milestone in "${!MILESTONES[@]}"; do
    echo -n "  Creando milestone '$milestone'... "
    if gh api repos/$REPO/milestones -f title="$milestone" -f description="${MILESTONES[$milestone]}" -f state="open" 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
    else
        echo -e "${YELLOW}(ya existe)${NC}"
    fi
done

# ============================================================================
# Paso 3: Crear Issues
# ============================================================================
echo ""
echo -e "${YELLOW}[3/3] Creando issues...${NC}"
echo ""

ISSUE_COUNT=0

# Función para crear issue usando archivo temporal para el body
create_issue() {
    local title="$1"
    local body="$2"
    local labels="$3"
    local milestone="$4"
    
    ISSUE_COUNT=$((ISSUE_COUNT + 1))
    
    echo -n "  [$ISSUE_COUNT] $title... "
    
    # Escribir el body a un archivo temporal (preserva formato markdown)
    local body_file="$TEMP_DIR/issue_${ISSUE_COUNT}.md"
    printf '%s' "$body" > "$body_file"
    
    # Construir argumentos
    local args=(
        --repo "$REPO"
        --title "$title"
        --body-file "$body_file"
    )
    
    # Añadir labels si existen
    if [ -n "$labels" ]; then
        # Separar labels por coma y añadir cada uno
        IFS=',' read -ra LABEL_ARRAY <<< "$labels"
        for lbl in "${LABEL_ARRAY[@]}"; do
            args+=(--label "$lbl")
        done
    fi
    
    # Añadir milestone si existe
    if [ -n "$milestone" ]; then
        args+=(--milestone "$milestone")
    fi
    
    # Crear el issue
    if gh issue create "${args[@]}" > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC}"
    else
        echo -e "${RED}✗${NC}"
    fi
    
    # Pequeña pausa para no sobrecargar la API
    sleep 0.3
}

# ============================================================================
# EPIC 1: Fundamentos
# ============================================================================
echo -e "${BLUE}── Epic 1: Fundamentos ──${NC}"

create_issue "[Epic 1] #1: Implementar funciones básicas de string" \
"## Descripción
Implementar funciones de utilidad para manipulación de strings en el namespace Utils.

## Archivos
- \`src/Utils.cpp\`
- \`inc/Utils.hpp\`

## Tareas
- [ ] Crear namespace \`Utils\`
- [ ] Implementar \`trim()\` - eliminar espacios al inicio/final
- [ ] Implementar \`toLower()\` y \`toUpper()\`
- [ ] Implementar \`split()\` con char y string delimitadores
- [ ] Implementar \`startsWith()\` y \`endsWith()\`
- [ ] Implementar \`replaceAll()\`

## Criterios de Aceptación
\`\`\`cpp
assert(Utils::trim(\"  hello  \") == \"hello\");
assert(Utils::toLower(\"HeLLo\") == \"hello\");
assert(Utils::split(\"a,b,c\", ',').size() == 3);
\`\`\`

## Puntos: 3" \
"enhancement,epic-1-foundation" \
"v0.1-foundation"

create_issue "[Epic 1] #2: Implementar conversiones numéricas" \
"## Descripción
Implementar funciones para convertir entre strings y números.

## Archivos
- \`src/Utils.cpp\`

## Tareas
- [ ] \`stringToInt()\` - string a entero
- [ ] \`stringToSizeT()\` - string a size_t
- [ ] \`intToString()\` - entero a string
- [ ] \`sizeTToString()\` - size_t a string
- [ ] \`hexToSizeT()\` - hexadecimal a size_t

## Puntos: 2" \
"enhancement,epic-1-foundation" \
"v0.1-foundation"

create_issue "[Epic 1] #3: Implementar utilidades de archivos" \
"## Descripción
Implementar funciones para manejo de archivos y rutas.

## Archivos
- \`src/Utils.cpp\`

## Tareas
- [ ] \`fileExists()\` - verificar existencia
- [ ] \`isDirectory()\` - verificar si es directorio
- [ ] \`isReadable()\`, \`isWritable()\`, \`isExecutable()\`
- [ ] \`getFileSize()\` - obtener tamaño
- [ ] \`getFileExtension()\` - obtener extensión
- [ ] \`getFileName()\` y \`getDirectory()\`
- [ ] \`readFile()\` - leer contenido completo
- [ ] \`writeFile()\` - escribir contenido
- [ ] \`normalizePath()\` - normalizar rutas
- [ ] \`joinPath()\` - unir rutas

## Puntos: 5" \
"enhancement,epic-1-foundation" \
"v0.1-foundation"

create_issue "[Epic 1] #4: Implementar utilidades HTTP" \
"## Descripción
Implementar funciones específicas para el protocolo HTTP.

## Archivos
- \`src/Utils.cpp\`

## Tareas
- [ ] \`urlDecode()\` - decodificar URL (%20 → espacio)
- [ ] \`urlEncode()\` - codificar URL
- [ ] \`getHttpDate()\` - formato RFC 7231
- [ ] \`getStatusMessage()\` - código a mensaje
- [ ] \`isValidMethod()\` - validar métodos HTTP

## Puntos: 4" \
"enhancement,epic-1-foundation" \
"v0.1-foundation"

create_issue "[Epic 1] #5: Implementar sistema de logging" \
"## Descripción
Implementar funciones de logging con colores ANSI.

## Archivos
- \`src/Utils.cpp\`

## Tareas
- [ ] \`logInfo()\` - mensajes informativos (verde)
- [ ] \`logWarning()\` - advertencias (amarillo)
- [ ] \`logError()\` - errores (rojo)
- [ ] \`logDebug()\` - debug (cyan)

## Puntos: 2" \
"enhancement,epic-1-foundation" \
"v0.1-foundation"

create_issue "[Epic 1] #6: Implementar singleton MimeTypes" \
"## Descripción
Implementar clase singleton para mapeo de extensiones a tipos MIME.

## Archivos
- \`src/MimeTypes.cpp\`
- \`inc/MimeTypes.hpp\`

## Tareas
- [ ] Implementar patrón Singleton
- [ ] Crear mapa de extensiones → tipos MIME
- [ ] \`getMimeType(extension)\`
- [ ] \`getMimeTypeByFile(filename)\`
- [ ] \`isTextType()\` y \`isBinaryType()\`

## Puntos: 3" \
"enhancement,epic-1-foundation" \
"v0.1-foundation"

create_issue "[Epic 1] #7: Crear webserv.hpp" \
"## Descripción
Crear header principal con includes y constantes.

## Archivos
- \`inc/webserv.hpp\`

## Tareas
- [ ] Incluir headers C++ estándar necesarios
- [ ] Incluir headers de sistema (socket, poll, etc.)
- [ ] Definir constantes del servidor
- [ ] Definir códigos HTTP como macros
- [ ] Definir colores para logging

## Puntos: 2" \
"enhancement,epic-1-foundation" \
"v0.1-foundation"

# ============================================================================
# EPIC 2: Sockets
# ============================================================================
echo -e "${BLUE}── Epic 2: Networking ──${NC}"

create_issue "[Epic 2] #8: Estructura básica Server" \
"## Descripción
Crear la estructura básica de la clase Server.

## Archivos
- \`src/Server.cpp\`
- \`inc/Server.hpp\`

## Tareas
- [ ] Declarar atributos: _configs, _serverFds, _clients, _pollFds
- [ ] Constructor y destructor
- [ ] Métodos stub vacíos

## Puntos: 3" \
"enhancement,epic-2-networking" \
"v0.2-networking"

create_issue "[Epic 2] #9: Crear sockets de servidor" \
"## Descripción
Implementar creación de sockets TCP.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_createServerSocket(port)\`
- [ ] socket(), setsockopt(), bind(), listen()
- [ ] Modo no-bloqueante con fcntl()

## Puntos: 5" \
"enhancement,epic-2-networking,critical" \
"v0.2-networking"

create_issue "[Epic 2] #10: Event loop con poll()" \
"## Descripción
Implementar el bucle principal de eventos.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`run()\`
- [ ] Configurar pollfd para server sockets
- [ ] Iterar sobre eventos POLLIN/POLLOUT

## Puntos: 5" \
"enhancement,epic-2-networking,critical" \
"v0.2-networking"

create_issue "[Epic 2] #11: Aceptar conexiones" \
"## Descripción
Implementar aceptación de conexiones de clientes.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_acceptConnection(serverFd)\`
- [ ] accept() no bloqueante
- [ ] Añadir cliente a _pollFds
- [ ] Crear objeto Client

## Puntos: 4" \
"enhancement,epic-2-networking,critical" \
"v0.2-networking"

create_issue "[Epic 2] #12: Estructura Client" \
"## Descripción
Crear la estructura básica de la clase Client.

## Archivos
- \`src/Client.cpp\`
- \`inc/Client.hpp\`

## Tareas
- [ ] Atributos: fd, state, request, response, buffers
- [ ] Constructor y destructor
- [ ] Getters y setters

## Puntos: 3" \
"enhancement,epic-2-networking" \
"v0.2-networking"

create_issue "[Epic 2] #13: Leer datos del cliente" \
"## Descripción
Implementar lectura de datos de los clientes.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_handleClientRead(clientFd)\`
- [ ] recv() no bloqueante
- [ ] Acumular en buffer del cliente
- [ ] Detectar cierre de conexión

## Puntos: 4" \
"enhancement,epic-2-networking,critical" \
"v0.2-networking"

create_issue "[Epic 2] #14: Enviar datos al cliente" \
"## Descripción
Implementar envío de datos a los clientes.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_handleClientWrite(clientFd)\`
- [ ] send() no bloqueante
- [ ] Manejar envío parcial
- [ ] Actualizar offset

## Puntos: 4" \
"enhancement,epic-2-networking,critical" \
"v0.2-networking"

create_issue "[Epic 2] #15: Cerrar conexiones" \
"## Descripción
Implementar cierre limpio de conexiones.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_closeClient(clientFd)\`
- [ ] close() del socket
- [ ] Limpiar de _pollFds y _clients

## Puntos: 2" \
"enhancement,epic-2-networking" \
"v0.2-networking"

# ============================================================================
# EPIC 3: HTTP Parser
# ============================================================================
echo -e "${BLUE}── Epic 3: HTTP Parser ──${NC}"

create_issue "[Epic 3] #16: Estructura Request" \
"## Descripción
Crear la estructura básica de la clase Request.

## Archivos
- \`src/Request.cpp\`
- \`inc/Request.hpp\`

## Tareas
- [ ] Atributos: method, uri, version, headers, body, state
- [ ] Constructor y destructor
- [ ] Getters y setters
- [ ] Estados: INCOMPLETE, HEADERS_COMPLETE, COMPLETE, ERROR

## Puntos: 3" \
"enhancement,epic-3-http-parser" \
"v0.3-http-parser"

create_issue "[Epic 3] #17: Parsear Request Line" \
"## Descripción
Implementar parsing de la línea de petición HTTP.

## Archivos
- \`src/Request.cpp\`

## Tareas
- [ ] Implementar \`parseRequestLine()\`
- [ ] Extraer método, URI, versión
- [ ] Validar formato
- [ ] Implementar \`parseUri()\`

## Puntos: 4" \
"enhancement,epic-3-http-parser,critical" \
"v0.3-http-parser"

create_issue "[Epic 3] #18: Parsear Headers" \
"## Descripción
Implementar parsing de headers HTTP.

## Archivos
- \`src/Request.cpp\`

## Tareas
- [ ] Implementar \`parseHeaders()\`
- [ ] Separar nombre: valor
- [ ] Detectar fin de headers
- [ ] Normalizar nombres

## Puntos: 4" \
"enhancement,epic-3-http-parser,critical" \
"v0.3-http-parser"

create_issue "[Epic 3] #19: Parsear Body" \
"## Descripción
Implementar parsing del body HTTP.

## Archivos
- \`src/Request.cpp\`

## Tareas
- [ ] Implementar \`parseBody()\`
- [ ] Content-Length
- [ ] Transfer-Encoding: chunked
- [ ] Implementar \`parseChunkedBody()\`

## Puntos: 5" \
"enhancement,epic-3-http-parser,critical" \
"v0.3-http-parser"

create_issue "[Epic 3] #20: Método feed() incremental" \
"## Descripción
Implementar método feed() para parsing incremental.

## Archivos
- \`src/Request.cpp\`

## Tareas
- [ ] Implementar \`feed(data)\`
- [ ] Acumular en buffer
- [ ] Llamar a parsers según estado
- [ ] Retornar estado actual

## Puntos: 4" \
"enhancement,epic-3-http-parser,critical" \
"v0.3-http-parser"

create_issue "[Epic 3] #21: Estructura Response" \
"## Descripción
Crear la estructura básica de la clase Response.

## Archivos
- \`src/Response.cpp\`
- \`inc/Response.hpp\`

## Tareas
- [ ] Atributos: statusCode, headers, body
- [ ] Constructor y destructor
- [ ] Métodos set (status, headers, body)
- [ ] Método \`build()\` para generar string

## Puntos: 3" \
"enhancement,epic-3-http-parser" \
"v0.3-http-parser"

create_issue "[Epic 3] #22: Construir respuesta HTTP" \
"## Descripción
Implementar generación de respuesta HTTP válida.

## Archivos
- \`src/Response.cpp\`

## Tareas
- [ ] Implementar \`build()\`
- [ ] Formato correcto con CRLF

## Puntos: 3" \
"enhancement,epic-3-http-parser,critical" \
"v0.3-http-parser"

create_issue "[Epic 3] #23: Métodos helper de Response" \
"## Descripción
Implementar métodos estáticos helper.

## Archivos
- \`src/Response.cpp\`

## Tareas
- [ ] \`makeOk()\`
- [ ] \`makeError(code, message)\`
- [ ] \`makeRedirect(code, location)\`
- [ ] \`makeFile(path)\`

## Puntos: 3" \
"enhancement,epic-3-http-parser" \
"v0.3-http-parser"

# ============================================================================
# EPIC 4: Configuración
# ============================================================================
echo -e "${BLUE}── Epic 4: Configuración ──${NC}"

create_issue "[Epic 4] #24: Estructura LocationConfig" \
"## Descripción
Crear estructura para configuración de locations.

## Archivos
- \`inc/Config.hpp\`

## Tareas
- [ ] Atributos: path, methods, root/alias, index
- [ ] Atributos: autoindex, redirect, cgi
- [ ] Constructor con valores por defecto

## Puntos: 2" \
"enhancement,epic-4-config" \
"v0.4-config"

create_issue "[Epic 4] #25: Estructura ServerConfig" \
"## Descripción
Crear estructura para configuración de servers.

## Archivos
- \`inc/Config.hpp\`

## Tareas
- [ ] Atributos: host, port, server_names
- [ ] Atributos: root, error_pages, client_max_body_size
- [ ] Vector de LocationConfig
- [ ] Método \`findLocation(uri)\`

## Puntos: 3" \
"enhancement,epic-4-config" \
"v0.4-config"

create_issue "[Epic 4] #26: Parser de configuración" \
"## Descripción
Implementar parser del archivo de configuración.

## Archivos
- \`src/Config.cpp\`
- \`inc/Config.hpp\`

## Tareas
- [ ] Implementar \`parseConfigFile(path)\`
- [ ] Tokenizar y parsear bloques
- [ ] Parsear directivas server y location
- [ ] Validar configuración

## Puntos: 8" \
"enhancement,epic-4-config,critical" \
"v0.4-config"

create_issue "[Epic 4] #27: Integrar configuración en Server" \
"## Descripción
Integrar sistema de configuración en Server.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Cargar configuración en constructor
- [ ] Crear sockets según configs
- [ ] Implementar \`_findServerConfig()\`

## Puntos: 4" \
"enhancement,epic-4-config,critical" \
"v0.4-config"

# ============================================================================
# EPIC 5: Handlers
# ============================================================================
echo -e "${BLUE}── Epic 5: Handlers ──${NC}"

create_issue "[Epic 5] #28: Implementar _processRequest()" \
"## Descripción
Implementar el router principal de peticiones.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Seleccionar ServerConfig
- [ ] Encontrar LocationConfig
- [ ] Verificar redirect, método y body size
- [ ] Routing a handlers

## Puntos: 5" \
"enhancement,epic-5-handlers,critical" \
"v0.5-handlers"

create_issue "[Epic 5] #29: Resolver rutas de archivos" \
"## Descripción
Implementar resolución de paths de archivos.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_resolvePath(request, location, server)\`
- [ ] Manejar root y alias
- [ ] Prevenir path traversal

## Puntos: 3" \
"enhancement,epic-5-handlers" \
"v0.5-handlers"

create_issue "[Epic 5] #30: Handler GET para archivos" \
"## Descripción
Implementar handler para peticiones GET.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_handleGet()\`
- [ ] Manejar archivos y directorios
- [ ] Buscar archivo índice
- [ ] Soporte para HEAD

## Puntos: 6" \
"enhancement,epic-5-handlers,critical" \
"v0.5-handlers"

create_issue "[Epic 5] #31: Handler DELETE" \
"## Descripción
Implementar handler para peticiones DELETE.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_handleDelete()\`
- [ ] Verificar existencia y permisos
- [ ] Eliminar archivo

## Puntos: 2" \
"enhancement,epic-5-handlers" \
"v0.5-handlers"

create_issue "[Epic 5] #32: Enviar respuestas de error" \
"## Descripción
Implementar método para enviar errores HTTP.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_sendErrorResponse(client, code)\`
- [ ] Usar páginas de error personalizadas

## Puntos: 2" \
"enhancement,epic-5-handlers" \
"v0.5-handlers"

create_issue "[Epic 5] #33: Generar listado de directorios" \
"## Descripción
Implementar generación de HTML para listado de directorios.

## Archivos
- \`src/Response.cpp\`

## Tareas
- [ ] Implementar \`makeDirectoryListing()\`
- [ ] Leer directorio con opendir/readdir
- [ ] Generar HTML con estilos

## Puntos: 4" \
"enhancement,epic-5-handlers" \
"v0.5-handlers"

# ============================================================================
# EPIC 6: CGI
# ============================================================================
echo -e "${BLUE}── Epic 6: CGI ──${NC}"

create_issue "[Epic 6] #34: Estructura CGIHandler" \
"## Descripción
Crear la estructura básica de la clase CGIHandler.

## Archivos
- \`src/CGIHandler.cpp\`
- \`inc/CGIHandler.hpp\`

## Tareas
- [ ] Atributos: request, config, scriptPath, output
- [ ] Setters para configurar
- [ ] Getters para resultados

## Puntos: 3" \
"enhancement,epic-6-cgi" \
"v0.6-cgi"

create_issue "[Epic 6] #35: Construir entorno CGI" \
"## Descripción
Implementar construcción de variables de entorno CGI.

## Archivos
- \`src/CGIHandler.cpp\`

## Tareas
- [ ] Implementar \`_buildEnvironment()\`
- [ ] Variables obligatorias RFC 3875
- [ ] Convertir HTTP headers a HTTP_*
- [ ] Conversión a char**

## Puntos: 5" \
"enhancement,epic-6-cgi" \
"v0.6-cgi"

create_issue "[Epic 6] #36: Fork y exec del CGI" \
"## Descripción
Implementar ejecución del proceso CGI con fork/exec.

## Archivos
- \`src/CGIHandler.cpp\`

## Tareas
- [ ] Implementar \`startExecution()\`
- [ ] Crear pipes
- [ ] Fork proceso hijo
- [ ] Redirigir stdio con dup2
- [ ] Ejecutar con execve

## Puntos: 6" \
"enhancement,epic-6-cgi,critical" \
"v0.6-cgi"

create_issue "[Epic 6] #37: Integrar CGI en Server" \
"## Descripción
Integrar el handler CGI en el servidor.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_isCgiRequest()\`
- [ ] Implementar \`_handleCgi()\`
- [ ] Implementar \`_handleCgiRead()\`
- [ ] Implementar \`_prepareCgiResponse()\`

## Puntos: 5" \
"enhancement,epic-6-cgi,critical" \
"v0.6-cgi"

create_issue "[Epic 6] #38: Parsear output CGI" \
"## Descripción
Implementar parsing de la salida del CGI.

## Archivos
- \`src/CGIHandler.cpp\`

## Tareas
- [ ] Implementar \`parseCgiOutput()\`
- [ ] Extraer headers y body
- [ ] Detectar Status header

## Puntos: 3" \
"enhancement,epic-6-cgi" \
"v0.6-cgi"

# ============================================================================
# EPIC 7: Advanced
# ============================================================================
echo -e "${BLUE}── Epic 7: Advanced ──${NC}"

create_issue "[Epic 7] #39: Handler POST para uploads" \
"## Descripción
Implementar handler POST con soporte para uploads.

## Archivos
- \`src/Server.cpp\`

## Tareas
- [ ] Implementar \`_handlePost()\`
- [ ] Implementar \`_handleFileUpload()\`
- [ ] Guardar archivos multipart
- [ ] Sanitizar nombres de archivo

## Puntos: 5" \
"enhancement,epic-7-advanced" \
"v0.7-advanced"

create_issue "[Epic 7] #40: Implementar SessionManager" \
"## Descripción
Implementar gestión de sesiones con cookies.

## Archivos
- \`src/SessionManager.cpp\`
- \`inc/SessionManager.hpp\`

## Tareas
- [ ] Patrón Singleton
- [ ] Estructura Session
- [ ] CRUD de sesiones
- [ ] Limpieza de expiradas

## Puntos: 5" \
"enhancement,epic-7-advanced" \
"v0.7-advanced"

create_issue "[Epic 7] #41: Soporte Keep-Alive" \
"## Descripción
Implementar reutilización de conexiones TCP.

## Archivos
- \`src/Client.cpp\`
- \`src/Server.cpp\`

## Tareas
- [ ] \`shouldKeepAlive()\`
- [ ] \`reset()\` en Client
- [ ] Lógica en handleClientWrite

## Puntos: 3" \
"enhancement,epic-7-advanced" \
"v0.7-advanced"

create_issue "[Epic 7] #42: Implementar main.cpp" \
"## Descripción
Implementar punto de entrada con opciones CLI.

## Archivos
- \`src/main.cpp\`

## Tareas
- [ ] Parsear argumentos
- [ ] Banner ASCII
- [ ] Setup de señales
- [ ] Manejo de errores

## Puntos: 3" \
"enhancement,epic-7-advanced" \
"v0.7-advanced"

# ============================================================================
# Testing
# ============================================================================
echo -e "${BLUE}── Testing ──${NC}"

create_issue "#43: Suite de tests automatizada" \
"## Descripción
Crear suite de tests de integración.

## Archivos
- \`tests/test_suite.sh\`

## Tareas
- [ ] Tests de métodos HTTP (GET, POST, DELETE, HEAD)
- [ ] Tests de errores (404, 405, 413, 500)
- [ ] Tests de CGI
- [ ] Tests de concurrencia
- [ ] Tests de redirect
- [ ] Tests de upload

## Puntos: 5" \
"testing,documentation" \
""

# ============================================================================
# Resumen
# ============================================================================
echo ""
echo -e "${GREEN}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║                    ¡Importación completada!                   ║${NC}"
echo -e "${GREEN}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "Total de issues creados: ${CYAN}$ISSUE_COUNT${NC}"
echo ""
echo -e "Ver issues en: ${BLUE}https://github.com/$REPO/issues${NC}"
echo -e "Ver milestones en: ${BLUE}https://github.com/$REPO/milestones${NC}"
echo ""
