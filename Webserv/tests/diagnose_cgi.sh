#!/bin/bash

# Script de diagnóstico para problemas CGI

echo "═══════════════════════════════════════════════════════════════"
echo "  Diagnóstico CGI - Webserv 42"
echo "═══════════════════════════════════════════════════════════════"
echo ""

# 1. Verificar que YoupiBanane existe
echo "1️⃣  Verificando estructura de directorios..."
if [ -d "YoupiBanane" ]; then
    echo "  ✅ YoupiBanane/ existe"
else
    echo "  ❌ YoupiBanane/ NO existe"
    echo "     → Crear: mkdir YoupiBanane"
fi

# 2. Verificar youpi.bla
if [ -f "YoupiBanane/youpi.bla" ]; then
    echo "  ✅ YoupiBanane/youpi.bla existe"
    ls -lh YoupiBanane/youpi.bla
else
    echo "  ❌ YoupiBanane/youpi.bla NO existe"
    echo "     → Crear archivo de prueba: echo 'test' > YoupiBanane/youpi.bla"
fi

# 3. Verificar CGI tester
echo ""
echo "2️⃣  Verificando CGI tester..."
if [ -f "testers/ubuntu_cgi_tester" ]; then
    echo "  ✅ testers/ubuntu_cgi_tester existe"
    ls -lh testers/ubuntu_cgi_tester
    
    # Verificar permisos de ejecución
    if [ -x "testers/ubuntu_cgi_tester" ]; then
        echo "  ✅ testers/ubuntu_cgi_tester es ejecutable"
    else
        echo "  ❌ testers/ubuntu_cgi_tester NO es ejecutable"
        echo "     → Arreglar: chmod +x testers/ubuntu_cgi_tester"
    fi
else
    echo "  ❌ testers/ubuntu_cgi_tester NO existe"
    echo "     → Este archivo debe ser proporcionado por 42"
    echo "     → Descarga desde el subject o desde otro proyecto"
fi

# 4. Test manual del CGI
echo ""
echo "3️⃣  Probando CGI manualmente..."
if [ -f "testers/ubuntu_cgi_tester" ] && [ -x "testers/ubuntu_cgi_tester" ] && [ -f "YoupiBanane/youpi.bla" ]; then
    echo "  Ejecutando: ./testers/ubuntu_cgi_tester YoupiBanane/youpi.bla"
    echo ""
    
    export REQUEST_METHOD=POST
    export CONTENT_LENGTH=0
    export CONTENT_TYPE=""
    export QUERY_STRING=""
    
    ./testers/ubuntu_cgi_tester YoupiBanane/youpi.bla
    EXIT_CODE=$?
    
    echo ""
    if [ $EXIT_CODE -eq 0 ]; then
        echo "  ✅ CGI ejecutado correctamente (exit code: 0)"
    else
        echo "  ❌ CGI falló (exit code: $EXIT_CODE)"
    fi
else
    echo "  ⚠️  No se puede probar - falta archivo(s)"
fi

# 5. Verificar configuración
echo ""
echo "4️⃣  Verificando configuración en webserv.conf..."
if grep -q "cgi .bla ./testers/ubuntu_cgi_tester" config/webserv.conf 2>/dev/null; then
    echo "  ✅ CGI .bla configurado correctamente"
    grep "cgi .bla" config/webserv.conf
else
    echo "  ⚠️  No se encontró configuración CGI para .bla"
fi

# 6. Test con servidor
echo ""
echo "5️⃣  Test con servidor (si está corriendo)..."
if nc -z localhost 8080 2>/dev/null; then
    echo "  Servidor detectado en puerto 8080"
    echo "  Probando: curl -X POST http://localhost:8080/directory/youpi.bla"
    
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST http://localhost:8080/directory/youpi.bla)
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | head -n-1)
    
    echo ""
    echo "  HTTP Code: $HTTP_CODE"
    
    if [ "$HTTP_CODE" = "200" ]; then
        echo "  ✅ CGI funcionando correctamente"
        echo "  Respuesta:"
        echo "$BODY" | head -20
    elif [ "$HTTP_CODE" = "500" ]; then
        echo "  ❌ Error 500 - Internal Server Error"
        echo "  Causas posibles:"
        echo "    1. CGI tester no existe o no es ejecutable"
        echo "    2. Script youpi.bla no existe"
        echo "    3. Error en variables de entorno"
        echo "    4. Error en ejecución del CGI"
    else
        echo "  ⚠️  Código inesperado: $HTTP_CODE"
    fi
else
    echo "  ⚠️  Servidor no está corriendo en puerto 8080"
    echo "     → Iniciar con: ./bin/webserv config/webserv.conf"
fi

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo "  Resumen"
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "Si todos los checks pasan ✅ pero el CGI devuelve 500:"
echo "  → Compilar con debug: make re CXXFLAGS=\"-Wall -Wextra -Werror -std=c++98 -DDEBUG_CGI\""
echo "  → Ver logs del servidor para detalles"
echo ""
echo "Si falta el CGI tester:"
echo "  → Descargarlo del subject de 42"
echo "  → O copiarlo de otro proyecto webserv"
echo "  → chmod +x testers/ubuntu_cgi_tester"
echo ""
