#!/bin/bash
# **************************************************************************** #
#                                                                              #
#    webserv-docker.sh                                                         #
#                                                                              #
#    Script de gestión del entorno Docker para webserv                         #
#    Soporta Alpine (ligero) y Ubuntu (compatible con 42 tester)               #
#                                                                              #
#    By: fcela-ga <fcela-ga@student.42barcelona.com>                           #
#                                                                              #
# **************************************************************************** #

set -e

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Directorio del script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Nombre base de la imagen y contenedor
IMAGE_BASE="webserv"
CONTAINER_NAME="webserv"

# Archivo de configuración para recordar la imagen seleccionada
CONFIG_FILE="$SCRIPT_DIR/.docker-config"

# Cargar configuración si existe
if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
fi

# Valores por defecto
DOCKER_BASE="${DOCKER_BASE:-alpine}"

# Función para guardar configuración
save_config() {
    echo "DOCKER_BASE=$DOCKER_BASE" > "$CONFIG_FILE"
}

# Obtener nombre de imagen según la base
get_image_name() {
    echo "${IMAGE_BASE}-${DOCKER_BASE}"
}

# Obtener Dockerfile según la base
get_dockerfile() {
    if [ "$DOCKER_BASE" = "ubuntu" ]; then
        echo "Dockerfile.ubuntu"
    else
        echo "Dockerfile"
    fi
}

# Función para mostrar ayuda
show_help() {
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}           Webserv Docker Development Environment              ${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
    echo -e "${GREEN}Uso:${NC} $0 [opciones] <comando>"
    echo ""
    echo -e "${YELLOW}Opciones:${NC}"
    echo -e "  ${GREEN}--alpine${NC}     Usar imagen Alpine Linux (ligera, ~150MB)"
    echo -e "  ${GREEN}--ubuntu${NC}     Usar imagen Ubuntu Linux (compatible con 42 tester, ~500MB)"
    echo ""
    echo -e "${YELLOW}Comandos disponibles:${NC}"
    echo ""
    echo -e "  ${GREEN}build${NC}        Construir la imagen Docker"
    echo -e "  ${GREEN}start${NC}        Iniciar contenedor en modo interactivo"
    echo -e "  ${GREEN}stop${NC}         Detener contenedor"
    echo -e "  ${GREEN}restart${NC}      Reiniciar contenedor"
    echo -e "  ${GREEN}shell${NC}        Abrir shell en contenedor en ejecución"
    echo -e "  ${GREEN}compile${NC}      Compilar webserv dentro del contenedor"
    echo -e "  ${GREEN}run${NC}          Compilar y ejecutar webserv"
    echo -e "  ${GREEN}test${NC}         Ejecutar tests básicos"
    echo -e "  ${GREEN}test-all${NC}     Ejecutar todos los tests"
    echo -e "  ${GREEN}test-42${NC}      Ejecutar 42 tester (solo Ubuntu)"
    echo -e "  ${GREEN}siege${NC}        Ejecutar test de stress con siege"
    echo -e "  ${GREEN}valgrind${NC}     Ejecutar con valgrind"
    echo -e "  ${GREEN}clean${NC}        Limpiar archivos compilados"
    echo -e "  ${GREEN}fclean${NC}       Limpiar todo (binarios incluidos)"
    echo -e "  ${GREEN}status${NC}       Mostrar estado del contenedor"
    echo -e "  ${GREEN}logs${NC}         Mostrar logs del contenedor"
    echo -e "  ${GREEN}remove${NC}       Eliminar contenedor e imagen"
    echo -e "  ${GREEN}switch${NC}       Cambiar entre Alpine y Ubuntu"
    echo ""
    echo -e "${YELLOW}Ejemplos:${NC}"
    echo "  $0 build              # Construir imagen (usa configuración actual: $DOCKER_BASE)"
    echo "  $0 --ubuntu build     # Construir imagen Ubuntu"
    echo "  $0 --alpine build     # Construir imagen Alpine"
    echo "  $0 switch ubuntu      # Cambiar a Ubuntu y reconstruir"
    echo "  $0 test-42            # Ejecutar 42 tester (requiere Ubuntu)"
    echo ""
    echo -e "${MAGENTA}Configuración actual:${NC}"
    echo -e "  Base: ${GREEN}$DOCKER_BASE${NC}"
    echo -e "  Imagen: ${GREEN}$(get_image_name)${NC}"
    echo ""
    echo -e "${YELLOW}Notas:${NC}"
    echo -e "  - ${CYAN}Alpine${NC}: Más ligero pero el 42 tester oficial NO funciona"
    echo -e "  - ${CYAN}Ubuntu${NC}: Más pesado pero COMPATIBLE con el 42 tester oficial"
    echo ""
}

# Función para verificar Docker
check_docker() {
    if ! command -v docker &> /dev/null; then
        echo -e "${RED}Error: Docker no está instalado${NC}"
        exit 1
    fi
    if ! docker info &> /dev/null; then
        echo -e "${RED}Error: Docker daemon no está corriendo${NC}"
        exit 1
    fi
}

# Función para crear directorios necesarios
create_dirs() {
    echo -e "${BLUE}Creando directorios necesarios...${NC}"
    mkdir -p "$PROJECT_DIR"/{bin,obj,dep,logs,www/uploads,www/errors,www/deletable,www/put_test,testers,YoupiBanane/nop,YoupiBanane/Yeah}
}

# Construir imagen
cmd_build() {
    local dockerfile=$(get_dockerfile)
    local image_name=$(get_image_name)
    
    if [ ! -f "$SCRIPT_DIR/$dockerfile" ]; then
        echo -e "${RED}Error: $dockerfile no encontrado en $SCRIPT_DIR${NC}"
        echo -e "${YELLOW}Asegúrate de tener Dockerfile y Dockerfile.ubuntu${NC}"
        exit 1
    fi
    
    echo -e "${BLUE}Construyendo imagen Docker ($DOCKER_BASE)...${NC}"
    echo -e "${CYAN}Dockerfile: $dockerfile${NC}"
    echo -e "${CYAN}Imagen: $image_name${NC}"
    echo ""
    
    cd "$SCRIPT_DIR"
    docker build -t "$image_name:latest" -f "$dockerfile" .
    
    echo -e "${GREEN}✓ Imagen construida: $image_name:latest${NC}"
    save_config
    
    if [ "$DOCKER_BASE" = "alpine" ]; then
        echo -e "${YELLOW}⚠️  Nota: El 42 tester oficial NO funcionará en Alpine${NC}"
        echo -e "${YELLOW}   Usa --ubuntu o 'switch ubuntu' si necesitas el 42 tester${NC}"
    else
        echo -e "${GREEN}✓ Esta imagen es compatible con el 42 tester oficial${NC}"
    fi
}

# Iniciar contenedor
cmd_start() {
    create_dirs
    
    local image_name=$(get_image_name)
    
    # Verificar si la imagen existe
    if ! docker images --format '{{.Repository}}:{{.Tag}}' | grep -q "^${image_name}:latest$"; then
        echo -e "${YELLOW}Imagen no encontrada. Construyendo...${NC}"
        cmd_build
    fi
    
    # Verificar si ya está corriendo
    if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        echo -e "${YELLOW}Contenedor ya está corriendo${NC}"
        return
    fi
    
    # Eliminar contenedor anterior si existe
    if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        echo -e "${YELLOW}Eliminando contenedor anterior...${NC}"
        docker rm -f "$CONTAINER_NAME" > /dev/null
    fi
    
    echo -e "${BLUE}Iniciando contenedor ($DOCKER_BASE)...${NC}"
    docker run -d \
        --name "$CONTAINER_NAME" \
        -p 8080:8080 \
        -p 8081:8081 \
        -p 8082:8082 \
        -v "$PROJECT_DIR/src:/app/src:rw" \
        -v "$PROJECT_DIR/inc:/app/inc:rw" \
        -v "$PROJECT_DIR/Makefile:/app/Makefile:rw" \
        -v "$PROJECT_DIR/config:/app/config:rw" \
        -v "$PROJECT_DIR/www:/app/www:rw" \
        -v "$PROJECT_DIR/cgi-bin:/app/cgi-bin:rw" \
        -v "$PROJECT_DIR/bin:/app/bin:rw" \
        -v "$PROJECT_DIR/obj:/app/obj:rw" \
        -v "$PROJECT_DIR/dep:/app/dep:rw" \
        -v "$PROJECT_DIR/logs:/app/logs:rw" \
        -v "$PROJECT_DIR/tests:/app/tests:rw" \
        -v "$PROJECT_DIR/testers:/app/testers:rw" \
        -v "$PROJECT_DIR/YoupiBanane:/app/YoupiBanane:rw" \
        -e TERM=xterm-256color \
        "$image_name:latest" \
        tail -f /dev/null
    
    echo -e "${GREEN}✓ Contenedor iniciado: $CONTAINER_NAME (base: $DOCKER_BASE)${NC}"
    echo -e "${CYAN}Usa '$0 shell' para acceder al contenedor${NC}"
}

# Detener contenedor
cmd_stop() {
    echo -e "${BLUE}Deteniendo contenedor...${NC}"
    docker stop "$CONTAINER_NAME" 2>/dev/null || true
    echo -e "${GREEN}✓ Contenedor detenido${NC}"
}

# Reiniciar contenedor
cmd_restart() {
    cmd_stop
    sleep 1
    cmd_start
}

# Abrir shell
cmd_shell() {
    if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        echo -e "${YELLOW}Contenedor no está corriendo. Iniciando...${NC}"
        cmd_start
    fi
    echo -e "${BLUE}Abriendo shell en contenedor...${NC}"
    docker exec -it "$CONTAINER_NAME" /bin/bash
}

# Compilar
cmd_compile() {
    if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        echo -e "${YELLOW}Contenedor no está corriendo. Iniciando...${NC}"
        cmd_start
    fi
    echo -e "${BLUE}Compilando webserv...${NC}"
    docker exec "$CONTAINER_NAME" make re
    echo -e "${GREEN}✓ Compilación completada${NC}"
    echo -e "${CYAN}Binario disponible en: $PROJECT_DIR/bin/webserv${NC}"
}

# Ejecutar servidor
cmd_run() {
    if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        echo -e "${YELLOW}Contenedor no está corriendo. Iniciando...${NC}"
        cmd_start
    fi
    
    # Verificar si existe el binario
    if [ ! -f "$PROJECT_DIR/bin/webserv" ]; then
        echo -e "${YELLOW}Binario no encontrado. Compilando...${NC}"
        cmd_compile
    fi
    
    echo -e "${BLUE}Ejecutando webserv...${NC}"
    echo -e "${CYAN}Servidor disponible en:${NC}"
    echo -e "  - http://localhost:8080"
    echo -e "  - http://localhost:8081"
    echo -e "  - http://localhost:8082"
    echo -e "${YELLOW}Presiona Ctrl+C para detener${NC}"
    echo ""
    docker exec -it "$CONTAINER_NAME" ./bin/webserv config/webserv.conf
}

# Ejecutar tests básicos
cmd_test() {
    if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        cmd_start
    fi
    
    echo -e "${BLUE}Ejecutando tests...${NC}"
    docker exec "$CONTAINER_NAME" make test
}

# Ejecutar todos los tests
cmd_test_all() {
    if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        cmd_start
    fi
    
    echo -e "${BLUE}Ejecutando todos los tests...${NC}"
    
    if [ "$DOCKER_BASE" = "alpine" ]; then
        echo -e "${YELLOW}⚠️  Ejecutando en Alpine - 42 tester será omitido${NC}"
    fi
    
    docker exec -it "$CONTAINER_NAME" make test-all
}

# Ejecutar 42 tester
cmd_test_42() {
    if [ "$DOCKER_BASE" = "alpine" ]; then
        echo -e "${RED}⚠️  El 42 tester NO es compatible con Alpine Linux${NC}"
        echo -e "${YELLOW}El binario ubuntu_tester requiere glibc (Ubuntu/Debian)${NC}"
        echo -e ""
        echo -e "${CYAN}Opciones:${NC}"
        echo -e "  1. Cambiar a Ubuntu: $0 switch ubuntu"
        echo -e "  2. Ejecutar en tu máquina host: make test-42"
        echo -e ""
        exit 1
    fi
    
    if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        cmd_start
    fi
    
    echo -e "${BLUE}Ejecutando 42 tester...${NC}"
    docker exec -it "$CONTAINER_NAME" make test-42
}

# Ejecutar siege
cmd_siege() {
    if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        cmd_start
    fi
    
    if [ ! -f "$PROJECT_DIR/bin/webserv" ]; then
        cmd_compile
    fi
    
    echo -e "${BLUE}Ejecutando test de stress con siege...${NC}"
    docker exec -it "$CONTAINER_NAME" make test-benchmark-quick
}

# Ejecutar valgrind
cmd_valgrind() {
    if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        cmd_start
    fi
    
    if [ ! -f "$PROJECT_DIR/bin/webserv" ]; then
        cmd_compile
    fi
    
    echo -e "${BLUE}Ejecutando con valgrind...${NC}"
    docker exec -it "$CONTAINER_NAME" valgrind --leak-check=full --show-leak-kinds=all \
        --track-origins=yes ./bin/webserv config/webserv.conf
}

# Limpiar compilación
cmd_clean() {
    if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        echo -e "${BLUE}Ejecutando make clean...${NC}"
        docker exec "$CONTAINER_NAME" make clean
    else
        echo -e "${BLUE}Limpiando localmente...${NC}"
        rm -rf "$PROJECT_DIR"/{obj,dep}
    fi
    echo -e "${GREEN}✓ Limpieza completada${NC}"
}

# Limpiar todo
cmd_fclean() {
    if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        echo -e "${BLUE}Ejecutando make fclean...${NC}"
        docker exec "$CONTAINER_NAME" make fclean
    else
        echo -e "${BLUE}Limpiando localmente...${NC}"
        rm -rf "$PROJECT_DIR"/{obj,dep,bin}
    fi
    echo -e "${GREEN}✓ Limpieza completa${NC}"
}

# Mostrar estado
cmd_status() {
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}                    Estado del Entorno                          ${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
    
    echo -e "${YELLOW}Configuración:${NC}"
    echo -e "  Base actual: ${GREEN}$DOCKER_BASE${NC}"
    echo ""
    
    # Imágenes
    echo -e "${YELLOW}Imágenes Docker:${NC}"
    if docker images --format '{{.Repository}}:{{.Tag}}' | grep -q "^${IMAGE_BASE}-alpine:latest$"; then
        local size_alpine=$(docker images --format '{{.Size}}' "${IMAGE_BASE}-alpine:latest" 2>/dev/null)
        echo -e "  ${GREEN}✓${NC} ${IMAGE_BASE}-alpine:latest ($size_alpine)"
    else
        echo -e "  ${RED}✗${NC} ${IMAGE_BASE}-alpine:latest"
    fi
    if docker images --format '{{.Repository}}:{{.Tag}}' | grep -q "^${IMAGE_BASE}-ubuntu:latest$"; then
        local size_ubuntu=$(docker images --format '{{.Size}}' "${IMAGE_BASE}-ubuntu:latest" 2>/dev/null)
        echo -e "  ${GREEN}✓${NC} ${IMAGE_BASE}-ubuntu:latest ($size_ubuntu)"
    else
        echo -e "  ${RED}✗${NC} ${IMAGE_BASE}-ubuntu:latest"
    fi
    echo ""
    
    # Contenedor
    echo -e "${YELLOW}Contenedor:${NC}"
    if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        local running_image=$(docker inspect --format '{{.Config.Image}}' "$CONTAINER_NAME" 2>/dev/null)
        echo -e "  ${GREEN}✓${NC} Corriendo ($running_image)"
    elif docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        echo -e "  ${YELLOW}○${NC} Detenido"
    else
        echo -e "  ${RED}✗${NC} No existe"
    fi
    echo ""
    
    # Binario
    echo -e "${YELLOW}Binario:${NC}"
    if [ -f "$PROJECT_DIR/bin/webserv" ]; then
        echo -e "  ${GREEN}✓${NC} $PROJECT_DIR/bin/webserv"
    else
        echo -e "  ${RED}✗${NC} No compilado"
    fi
    echo ""
    
    # 42 Testers
    echo -e "${YELLOW}42 Testers:${NC}"
    if [ -x "$PROJECT_DIR/testers/ubuntu_tester" ]; then
        echo -e "  ${GREEN}✓${NC} ubuntu_tester"
    else
        echo -e "  ${RED}✗${NC} ubuntu_tester"
    fi
    if [ -x "$PROJECT_DIR/testers/ubuntu_cgi_tester" ]; then
        echo -e "  ${GREEN}✓${NC} ubuntu_cgi_tester"
    else
        echo -e "  ${RED}✗${NC} ubuntu_cgi_tester"
    fi
    echo ""
    
    # Compatibilidad
    echo -e "${YELLOW}Compatibilidad 42 tester:${NC}"
    if [ "$DOCKER_BASE" = "ubuntu" ]; then
        echo -e "  ${GREEN}✓${NC} Ubuntu - Compatible con 42 tester"
    else
        echo -e "  ${YELLOW}⚠${NC} Alpine - 42 tester NO funcionará"
    fi
    echo ""
}

# Mostrar logs
cmd_logs() {
    docker logs -f "$CONTAINER_NAME"
}

# Eliminar todo
cmd_remove() {
    echo -e "${YELLOW}¿Eliminar contenedor e imágenes? (s/N)${NC}"
    read -r response
    if [[ "$response" =~ ^[sS]$ ]]; then
        echo -e "${BLUE}Eliminando contenedor...${NC}"
        docker rm -f "$CONTAINER_NAME" 2>/dev/null || true
        echo -e "${BLUE}Eliminando imágenes...${NC}"
        docker rmi "${IMAGE_BASE}-alpine:latest" 2>/dev/null || true
        docker rmi "${IMAGE_BASE}-ubuntu:latest" 2>/dev/null || true
        echo -e "${GREEN}✓ Eliminado${NC}"
    else
        echo -e "${YELLOW}Cancelado${NC}"
    fi
}

# Cambiar entre Alpine y Ubuntu
cmd_switch() {
    local new_base="$1"
    
    if [ -z "$new_base" ]; then
        echo -e "${YELLOW}Uso: $0 switch <alpine|ubuntu>${NC}"
        echo -e "  Actual: ${GREEN}$DOCKER_BASE${NC}"
        exit 1
    fi
    
    if [ "$new_base" != "alpine" ] && [ "$new_base" != "ubuntu" ]; then
        echo -e "${RED}Error: Base debe ser 'alpine' o 'ubuntu'${NC}"
        exit 1
    fi
    
    if [ "$new_base" = "$DOCKER_BASE" ]; then
        echo -e "${YELLOW}Ya estás usando $new_base${NC}"
        return
    fi
    
    echo -e "${BLUE}Cambiando de $DOCKER_BASE a $new_base...${NC}"
    
    # Detener contenedor actual si existe
    if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        echo -e "${YELLOW}Deteniendo contenedor actual...${NC}"
        docker stop "$CONTAINER_NAME" > /dev/null
        docker rm "$CONTAINER_NAME" > /dev/null
    fi
    
    # Cambiar configuración
    DOCKER_BASE="$new_base"
    save_config
    
    echo -e "${GREEN}✓ Configuración cambiada a: $DOCKER_BASE${NC}"
    
    # Preguntar si construir
    echo -e "${YELLOW}¿Construir imagen $new_base ahora? (S/n)${NC}"
    read -r response
    if [[ ! "$response" =~ ^[nN]$ ]]; then
        cmd_build
        echo -e "${YELLOW}¿Iniciar contenedor ahora? (S/n)${NC}"
        read -r response
        if [[ ! "$response" =~ ^[nN]$ ]]; then
            cmd_start
        fi
    fi
}

# Procesar opciones
while [[ $# -gt 0 ]]; do
    case "$1" in
        --alpine)
            DOCKER_BASE="alpine"
            save_config
            shift
            ;;
        --ubuntu)
            DOCKER_BASE="ubuntu"
            save_config
            shift
            ;;
        *)
            break
            ;;
    esac
done

# Main
check_docker

case "${1:-help}" in
    build)      cmd_build ;;
    start)      cmd_start ;;
    stop)       cmd_stop ;;
    restart)    cmd_restart ;;
    shell)      cmd_shell ;;
    compile)    cmd_compile ;;
    run)        cmd_run ;;
    test)       cmd_test ;;
    test-all)   cmd_test_all ;;
    test-42)    cmd_test_42 ;;
    siege)      cmd_siege ;;
    valgrind)   cmd_valgrind ;;
    clean)      cmd_clean ;;
    fclean)     cmd_fclean ;;
    status)     cmd_status ;;
    logs)       cmd_logs ;;
    remove)     cmd_remove ;;
    switch)     cmd_switch "$2" ;;
    help|--help|-h)
        show_help ;;
    *)
        echo -e "${RED}Comando desconocido: $1${NC}"
        show_help
        exit 1 ;;
esac
