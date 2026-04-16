#!/bin/bash
# ============================================================================
# delete_github_issues.sh
# 
# Script para eliminar issues de GitHub de forma masiva con selección
# Requiere: GitHub CLI (gh) instalado y autenticado
#
# Uso:
#   ./delete_github_issues.sh owner/repo              # Modo interactivo
#   ./delete_github_issues.sh owner/repo --list       # Solo listar
#   ./delete_github_issues.sh owner/repo --all        # Eliminar todos
#   ./delete_github_issues.sh owner/repo --keep 1,5,10-15  # Conservar algunos
#   ./delete_github_issues.sh owner/repo --delete 1-10,25  # Eliminar específicos
# ============================================================================

# CRÍTICO: Desactivar el pager de gh
export GH_PAGER=""
export PAGER=""

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

# Verificar argumentos
if [ -z "$1" ]; then
    echo -e "${RED}Error: Debes especificar el repositorio${NC}"
    echo ""
    echo "Uso: $0 owner/repo [opciones]"
    echo ""
    echo "Opciones:"
    echo "  --list              Solo listar issues (no eliminar)"
    echo "  --all               Eliminar TODOS los issues"
    echo "  --keep INDICES      Conservar los issues especificados"
    echo "  --delete INDICES    Eliminar solo los issues especificados"
    echo ""
    echo "Formato de INDICES:"
    echo "  Individuales: 1,3,5,8"
    echo "  Rangos: 1-10,20-30"
    echo "  Combinados: 1,3,5-10,15,20-25"
    echo ""
    echo "Ejemplos:"
    echo "  $0 felipecela/42-C06-Prj14-Webserv --list"
    echo "  $0 felipecela/42-C06-Prj14-Webserv --all"
    echo "  $0 felipecela/42-C06-Prj14-Webserv --keep 1-5,10"
    echo "  $0 felipecela/42-C06-Prj14-Webserv --delete 6-9,11-43"
    exit 1
fi

REPO="$1"
shift

# Verificar gh está instalado
if ! command -v gh &> /dev/null; then
    echo -e "${RED}Error: GitHub CLI (gh) no está instalado${NC}"
    exit 1
fi

# Verificar autenticación
if ! gh auth status &> /dev/null; then
    echo -e "${RED}Error: No estás autenticado en GitHub CLI${NC}"
    echo "Ejecuta: gh auth login"
    exit 1
fi

# Función para expandir rangos (ej: "1-5,8,10-12" -> "1 2 3 4 5 8 10 11 12")
expand_ranges() {
    local input="$1"
    local result=()
    
    IFS=',' read -ra parts <<< "$input"
    for part in "${parts[@]}"; do
        if [[ "$part" == *-* ]]; then
            # Es un rango
            local start=$(echo "$part" | cut -d'-' -f1)
            local end=$(echo "$part" | cut -d'-' -f2)
            for ((i=start; i<=end; i++)); do
                result+=("$i")
            done
        else
            # Es un número individual
            result+=("$part")
        fi
    done
    
    echo "${result[@]}"
}

# Función para verificar si un índice está en la lista
is_in_list() {
    local needle="$1"
    shift
    local haystack=("$@")
    
    for item in "${haystack[@]}"; do
        if [ "$item" -eq "$needle" ] 2>/dev/null; then
            return 0
        fi
    done
    return 1
}

echo -e "${BLUE}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║              GitHub Issues Manager                            ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "Repositorio: ${GREEN}$REPO${NC}"
echo ""

# Obtener lista de issues
echo -e "${CYAN}Obteniendo lista de issues...${NC}"
ISSUES_JSON=$(gh issue list --repo "$REPO" --state all --limit 500 --json number,title,state 2>/dev/null)

if [ -z "$ISSUES_JSON" ] || [ "$ISSUES_JSON" == "[]" ]; then
    echo -e "${YELLOW}No hay issues en el repositorio${NC}"
    exit 0
fi

# Parsear issues
ISSUE_COUNT=$(echo "$ISSUES_JSON" | jq length)
echo -e "Total de issues encontrados: ${MAGENTA}$ISSUE_COUNT${NC}"
echo ""

# Mostrar lista de issues
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}  IDX   #NUM   ESTADO     TÍTULO${NC}"
echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"

declare -a ISSUE_NUMBERS
declare -a ISSUE_TITLES

for i in $(seq 0 $((ISSUE_COUNT - 1))); do
    NUMBER=$(echo "$ISSUES_JSON" | jq -r ".[$i].number")
    TITLE=$(echo "$ISSUES_JSON" | jq -r ".[$i].title")
    STATE=$(echo "$ISSUES_JSON" | jq -r ".[$i].state")
    
    ISSUE_NUMBERS+=("$NUMBER")
    ISSUE_TITLES+=("$TITLE")
    
    IDX=$((i + 1))
    
    # Color según estado
    if [ "$STATE" == "OPEN" ]; then
        STATE_COLOR="${GREEN}OPEN  ${NC}"
    else
        STATE_COLOR="${RED}CLOSED${NC}"
    fi
    
    # Truncar título si es muy largo
    if [ ${#TITLE} -gt 50 ]; then
        TITLE="${TITLE:0:47}..."
    fi
    
    printf "  ${CYAN}%-4s${NC}  #%-4s  %b  %s\n" "$IDX" "$NUMBER" "$STATE_COLOR" "$TITLE"
done

echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo ""

# Procesar modo
MODE="interactive"
KEEP_LIST=()
DELETE_LIST=()

while [ $# -gt 0 ]; do
    case "$1" in
        --list)
            echo -e "${GREEN}Modo: Solo listar (no se eliminará nada)${NC}"
            exit 0
            ;;
        --all)
            MODE="all"
            shift
            ;;
        --keep)
            MODE="keep"
            KEEP_LIST=($(expand_ranges "$2"))
            shift 2
            ;;
        --delete)
            MODE="delete"
            DELETE_LIST=($(expand_ranges "$2"))
            shift 2
            ;;
        *)
            shift
            ;;
    esac
done

# Modo interactivo
if [ "$MODE" == "interactive" ]; then
    echo -e "${YELLOW}MODO INTERACTIVO${NC}"
    echo ""
    echo "Opciones:"
    echo "  1) Eliminar TODOS los issues"
    echo "  2) Especificar cuáles CONSERVAR"
    echo "  3) Especificar cuáles ELIMINAR"
    echo "  4) Cancelar"
    echo ""
    read -p "Selecciona opción (1-4): " OPTION
    
    case "$OPTION" in
        1)
            MODE="all"
            ;;
        2)
            echo ""
            echo "Formato: Individuales (1,3,5) o rangos (1-10,20-30) o combinados (1,3,5-10)"
            read -p "Índices a CONSERVAR: " KEEP_INPUT
            KEEP_LIST=($(expand_ranges "$KEEP_INPUT"))
            MODE="keep"
            ;;
        3)
            echo ""
            echo "Formato: Individuales (1,3,5) o rangos (1-10,20-30) o combinados (1,3,5-10)"
            read -p "Índices a ELIMINAR: " DELETE_INPUT
            DELETE_LIST=($(expand_ranges "$DELETE_INPUT"))
            MODE="delete"
            ;;
        *)
            echo -e "${YELLOW}Operación cancelada${NC}"
            exit 0
            ;;
    esac
fi

# Construir lista de issues a eliminar
ISSUES_TO_DELETE=()

case "$MODE" in
    all)
        echo -e "${RED}Se eliminarán TODOS los $ISSUE_COUNT issues${NC}"
        for i in $(seq 0 $((ISSUE_COUNT - 1))); do
            ISSUES_TO_DELETE+=("${ISSUE_NUMBERS[$i]}")
        done
        ;;
    keep)
        echo -e "${GREEN}Se conservarán los índices: ${KEEP_LIST[*]}${NC}"
        for i in $(seq 0 $((ISSUE_COUNT - 1))); do
            IDX=$((i + 1))
            if ! is_in_list "$IDX" "${KEEP_LIST[@]}"; then
                ISSUES_TO_DELETE+=("${ISSUE_NUMBERS[$i]}")
            fi
        done
        ;;
    delete)
        echo -e "${RED}Se eliminarán los índices: ${DELETE_LIST[*]}${NC}"
        for idx in "${DELETE_LIST[@]}"; do
            if [ "$idx" -ge 1 ] && [ "$idx" -le "$ISSUE_COUNT" ]; then
                ISSUES_TO_DELETE+=("${ISSUE_NUMBERS[$((idx - 1))]}")
            fi
        done
        ;;
esac

DELETE_COUNT=${#ISSUES_TO_DELETE[@]}

if [ "$DELETE_COUNT" -eq 0 ]; then
    echo -e "${YELLOW}No hay issues para eliminar${NC}"
    exit 0
fi

echo ""
echo -e "${RED}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${RED}║  ATENCIÓN: Se eliminarán $DELETE_COUNT issues                          ║${NC}"
echo -e "${RED}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Mostrar issues que se eliminarán
echo "Issues a eliminar:"
for num in "${ISSUES_TO_DELETE[@]}"; do
    for i in $(seq 0 $((ISSUE_COUNT - 1))); do
        if [ "${ISSUE_NUMBERS[$i]}" == "$num" ]; then
            echo "  - #$num: ${ISSUE_TITLES[$i]}"
            break
        fi
    done
done
echo ""

# Confirmación
read -p "¿Estás seguro? Escribe 'ELIMINAR' para confirmar: " CONFIRM

if [ "$CONFIRM" != "ELIMINAR" ]; then
    echo -e "${YELLOW}Operación cancelada${NC}"
    exit 0
fi

echo ""
echo -e "${CYAN}Eliminando issues...${NC}"
echo ""

DELETED=0
FAILED=0

for num in "${ISSUES_TO_DELETE[@]}"; do
    echo -n "  Eliminando #$num... "
    
    if gh issue delete "$num" --repo "$REPO" --yes 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        DELETED=$((DELETED + 1))
    else
        echo -e "${RED}✗${NC}"
        FAILED=$((FAILED + 1))
    fi
    
    # Pequeña pausa para no sobrecargar la API
    sleep 0.2
done

echo ""
echo -e "${GREEN}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║                    ¡Operación completada!                     ║${NC}"
echo -e "${GREEN}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "Eliminados: ${GREEN}$DELETED${NC}"
echo -e "Fallidos: ${RED}$FAILED${NC}"
echo ""
