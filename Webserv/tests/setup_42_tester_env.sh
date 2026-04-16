#!/bin/bash
# ============================================================================
# setup_42_tester_env.sh
# 
# Script para crear la estructura de directorios y archivos que requiere
# el tester oficial de 42 para webserv
#
# Requisitos del tester:
#   - Directorio YoupiBanane con estructura específica
#   - Archivos .bla y .bad_extension
#   - Directorio put_test para uploads PUT
#
# Uso:
#   ./setup_42_tester_env.sh
# ============================================================================

set -e

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${BLUE}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║         42 Webserv Tester Environment Setup                   ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""

# ============================================================================
# 1. Crear estructura YoupiBanane (requerida por el tester)
# ============================================================================
echo -e "${CYAN}[1/4] Creating YoupiBanane directory structure...${NC}"

mkdir -p YoupiBanane/nop
mkdir -p YoupiBanane/Yeah

# Archivos en YoupiBanane/
echo "This is youpi.bad_extension - default index file" > YoupiBanane/youpi.bad_extension
echo "This is youpi.bla - CGI test file" > YoupiBanane/youpi.bla

# Archivos en YoupiBanane/nop/
echo "This is youpi.bad_extension in nop directory" > YoupiBanane/nop/youpi.bad_extension
echo "This is other.pouic in nop directory" > YoupiBanane/nop/other.pouic

# Archivos en YoupiBanane/Yeah/
echo "This is not_happy.bad_extension in Yeah directory" > YoupiBanane/Yeah/not_happy.bad_extension

echo -e "  ${GREEN}✓${NC} YoupiBanane/"
echo -e "  ${GREEN}✓${NC} YoupiBanane/youpi.bad_extension"
echo -e "  ${GREEN}✓${NC} YoupiBanane/youpi.bla"
echo -e "  ${GREEN}✓${NC} YoupiBanane/nop/"
echo -e "  ${GREEN}✓${NC} YoupiBanane/nop/youpi.bad_extension"
echo -e "  ${GREEN}✓${NC} YoupiBanane/nop/other.pouic"
echo -e "  ${GREEN}✓${NC} YoupiBanane/Yeah/"
echo -e "  ${GREEN}✓${NC} YoupiBanane/Yeah/not_happy.bad_extension"

# ============================================================================
# 2. Crear directorio para PUT uploads
# ============================================================================
echo ""
echo -e "${CYAN}[2/4] Creating PUT upload directory...${NC}"

mkdir -p www/put_test
chmod 777 www/put_test

echo -e "  ${GREEN}✓${NC} www/put_test/"

# ============================================================================
# 3. Extraer testers de 42 si están comprimidos
# ============================================================================
echo ""
echo -e "${CYAN}[3/4] Setting up 42 testers...${NC}"

mkdir -p testers

if [ -f comp/testers.zip ]; then
    unzip -o comp/testers.zip > /dev/null 2>&1
    echo -e "  ${GREEN}✓${NC} Extracted from comp/testers.zip"
else
    echo -e "  ${YELLOW}⚠${NC} comp/testers.zip not found"
fi

# Copiar testers desde directorio padre si existen
for tester in tester ubuntu_tester cgi_tester ubuntu_cgi_tester; do
    if [ -f "../$tester" ] && [ ! -f "testers/$tester" ]; then
        cp "../$tester" testers/
        echo -e "  ${GREEN}✓${NC} Copied $tester from parent directory"
    fi
done

# Hacer ejecutables
chmod +x testers/* 2>/dev/null || true

# Verificar testers disponibles
echo ""
echo -e "  ${YELLOW}Available testers:${NC}"
[ -x testers/ubuntu_tester ] && echo -e "    ${GREEN}✓${NC} ubuntu_tester" || echo -e "    ${RED}✗${NC} ubuntu_tester"
[ -x testers/tester ] && echo -e "    ${GREEN}✓${NC} tester" || echo -e "    ${RED}✗${NC} tester"
[ -x testers/ubuntu_cgi_tester ] && echo -e "    ${GREEN}✓${NC} ubuntu_cgi_tester" || echo -e "    ${RED}✗${NC} ubuntu_cgi_tester"
[ -x testers/cgi_tester ] && echo -e "    ${GREEN}✓${NC} cgi_tester" || echo -e "    ${RED}✗${NC} cgi_tester"

# ============================================================================
# 4. Crear archivos CGI de prueba
# ============================================================================
echo ""
echo -e "${CYAN}[4/4] Creating test CGI files...${NC}"

mkdir -p cgi-bin

# Test .bla file for CGI tester
cat > cgi-bin/test.bla << 'EOF'
Test file for .bla CGI extension
This should be processed by the 42 cgi_tester
EOF

chmod +x cgi-bin/*.py 2>/dev/null || true

echo -e "  ${GREEN}✓${NC} cgi-bin/test.py"
echo -e "  ${GREEN}✓${NC} cgi-bin/info.py"
echo -e "  ${GREEN}✓${NC} cgi-bin/env.py"
echo -e "  ${GREEN}✓${NC} cgi-bin/test.bla"

# ============================================================================
# Summary
# ============================================================================
echo ""
echo -e "${GREEN}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║                    Setup Complete!                            ║${NC}"
echo -e "${GREEN}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${YELLOW}Directory structure created:${NC}"
echo ""
echo "YoupiBanane/"
echo "├── youpi.bad_extension    (default index)"
echo "├── youpi.bla              (CGI test file)"
echo "├── nop/"
echo "│   ├── youpi.bad_extension"
echo "│   └── other.pouic"
echo "└── Yeah/"
echo "    └── not_happy.bad_extension"
echo ""
echo "www/put_test/              (PUT upload directory)"
echo ""
echo "testers/                   (42 official testers)"
echo ""
echo -e "${YELLOW}Configuration notes:${NC}"
echo "  - / responds to GET only"
echo "  - /put_test responds to PUT (saves to www/put_test/)"
echo "  - /post_body responds to POST with maxBody=100"
echo "  - /directory serves YoupiBanane/ with index=youpi.bad_extension"
echo "  - .bla files are handled by testers/ubuntu_cgi_tester"
echo ""
echo -e "${CYAN}To run the 42 tester:${NC}"
echo "  1. Start server:  make run"
echo "  2. Run tester:    ./testers/ubuntu_tester http://localhost:8080"
echo "  Or use:           make test-42-verbose"
echo ""
