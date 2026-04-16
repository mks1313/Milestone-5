@echo off
REM **************************************************************************** #
REM                                                                              #
REM    webserv-docker.bat                                                        #
REM                                                                              #
REM    Script de gestión del entorno Docker para webserv (Windows)               #
REM                                                                              #
REM    By: fcela-ga <fcela-ga@student.42barcelona.com>                           #
REM                                                                              #
REM **************************************************************************** #

setlocal enabledelayedexpansion

REM Colores (Windows 10+)
set "GREEN=[92m"
set "RED=[91m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "CYAN=[96m"
set "NC=[0m"

REM Configuración
set "IMAGE_NAME=webserv"
set "CONTAINER_NAME=webserv"
set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."

REM Verificar argumento
if "%1"=="" goto :help
if "%1"=="help" goto :help
if "%1"=="--help" goto :help
if "%1"=="-h" goto :help

REM Verificar Docker
docker info >nul 2>&1
if errorlevel 1 (
    echo %RED%Error: Docker no está corriendo%NC%
    exit /b 1
)

REM Ejecutar comando
goto :%1 2>nul || (
    echo %RED%Comando desconocido: %1%NC%
    goto :help
)

:build
echo %BLUE%Construyendo imagen Docker...%NC%
cd /d "%SCRIPT_DIR%"
docker build -t %IMAGE_NAME%:latest .
echo %GREEN%✓ Imagen construida: %IMAGE_NAME%:latest%NC%
goto :eof

:start
REM Crear directorios necesarios
if not exist "%PROJECT_DIR%\bin" mkdir "%PROJECT_DIR%\bin"
if not exist "%PROJECT_DIR%\obj" mkdir "%PROJECT_DIR%\obj"
if not exist "%PROJECT_DIR%\dep" mkdir "%PROJECT_DIR%\dep"
if not exist "%PROJECT_DIR%\logs" mkdir "%PROJECT_DIR%\logs"
if not exist "%PROJECT_DIR%\www\uploads" mkdir "%PROJECT_DIR%\www\uploads"

REM Verificar si ya está corriendo
docker ps --format "{{.Names}}" | findstr /x "%CONTAINER_NAME%" >nul 2>&1
if not errorlevel 1 (
    echo %YELLOW%Contenedor ya está corriendo%NC%
    goto :eof
)

REM Eliminar contenedor anterior si existe
docker ps -a --format "{{.Names}}" | findstr /x "%CONTAINER_NAME%" >nul 2>&1
if not errorlevel 1 (
    echo %YELLOW%Eliminando contenedor anterior...%NC%
    docker rm -f %CONTAINER_NAME% >nul
)

echo %BLUE%Iniciando contenedor...%NC%
docker run -d ^
    --name %CONTAINER_NAME% ^
    -p 8080:8080 ^
    -p 8081:8081 ^
    -p 8082:8082 ^
    -v "%PROJECT_DIR%\src:/app/src:rw" ^
    -v "%PROJECT_DIR%\inc:/app/inc:rw" ^
    -v "%PROJECT_DIR%\Makefile:/app/Makefile:rw" ^
    -v "%PROJECT_DIR%\config:/app/config:rw" ^
    -v "%PROJECT_DIR%\www:/app/www:rw" ^
    -v "%PROJECT_DIR%\cgi-bin:/app/cgi-bin:rw" ^
    -v "%PROJECT_DIR%\bin:/app/bin:rw" ^
    -v "%PROJECT_DIR%\obj:/app/obj:rw" ^
    -v "%PROJECT_DIR%\dep:/app/dep:rw" ^
    -v "%PROJECT_DIR%\logs:/app/logs:rw" ^
    -e TERM=xterm-256color ^
    -it ^
    %IMAGE_NAME%:latest ^
    /bin/bash -c "tail -f /dev/null"

echo %GREEN%✓ Contenedor iniciado: %CONTAINER_NAME%%NC%
echo %CYAN%Usa '%~nx0 shell' para acceder al contenedor%NC%
goto :eof

:stop
echo %BLUE%Deteniendo contenedor...%NC%
docker stop %CONTAINER_NAME% 2>nul
echo %GREEN%✓ Contenedor detenido%NC%
goto :eof

:restart
call :stop
call :start
goto :eof

:shell
docker ps --format "{{.Names}}" | findstr /x "%CONTAINER_NAME%" >nul 2>&1
if errorlevel 1 (
    echo %YELLOW%Contenedor no está corriendo. Iniciando...%NC%
    call :start
)
echo %BLUE%Abriendo shell en contenedor...%NC%
docker exec -it %CONTAINER_NAME% /bin/bash
goto :eof

:compile
docker ps --format "{{.Names}}" | findstr /x "%CONTAINER_NAME%" >nul 2>&1
if errorlevel 1 (
    echo %YELLOW%Contenedor no está corriendo. Iniciando...%NC%
    call :start
)
echo %BLUE%Compilando webserv...%NC%
docker exec -it %CONTAINER_NAME% make re
echo %GREEN%✓ Compilación completada%NC%
echo %CYAN%Binario disponible en: %PROJECT_DIR%\bin\webserv%NC%
goto :eof

:run
docker ps --format "{{.Names}}" | findstr /x "%CONTAINER_NAME%" >nul 2>&1
if errorlevel 1 (
    echo %YELLOW%Contenedor no está corriendo. Iniciando...%NC%
    call :start
)

if not exist "%PROJECT_DIR%\bin\webserv" (
    echo %YELLOW%Binario no encontrado. Compilando...%NC%
    call :compile
)

echo %BLUE%Ejecutando webserv...%NC%
echo %CYAN%Servidor disponible en:%NC%
echo   - http://localhost:8080
echo   - http://localhost:8081
echo   - http://localhost:8082
echo %YELLOW%Presiona Ctrl+C para detener%NC%
echo.
docker exec -it %CONTAINER_NAME% ./bin/webserv config/webserv.conf
goto :eof

:test
docker ps --format "{{.Names}}" | findstr /x "%CONTAINER_NAME%" >nul 2>&1
if errorlevel 1 call :start

if not exist "%PROJECT_DIR%\bin\webserv" call :compile

echo %BLUE%Ejecutando tests básicos...%NC%

REM Ejecutar servidor en background
docker exec -d %CONTAINER_NAME% ./bin/webserv config/webserv.conf
timeout /t 2 /nobreak >nul

echo.
echo %CYAN%Testing GET /...%NC%
curl -s http://localhost:8080/

echo.
echo %CYAN%Testing 404...%NC%
curl -s -o nul -w "Status: %%{http_code}" http://localhost:8080/nonexistent
echo.

echo.
echo %CYAN%Testing CGI...%NC%
curl -s http://localhost:8080/cgi-bin/test.py

REM Detener servidor
docker exec %CONTAINER_NAME% pkill -f webserv 2>nul

echo.
echo %GREEN%✓ Tests completados%NC%
goto :eof

:clean
docker ps --format "{{.Names}}" | findstr /x "%CONTAINER_NAME%" >nul 2>&1
if errorlevel 1 (
    echo %BLUE%Limpiando localmente...%NC%
    if exist "%PROJECT_DIR%\obj" rmdir /s /q "%PROJECT_DIR%\obj"
    if exist "%PROJECT_DIR%\dep" rmdir /s /q "%PROJECT_DIR%\dep"
) else (
    echo %BLUE%Ejecutando make clean...%NC%
    docker exec %CONTAINER_NAME% make clean
)
echo %GREEN%✓ Limpieza completada%NC%
goto :eof

:fclean
docker ps --format "{{.Names}}" | findstr /x "%CONTAINER_NAME%" >nul 2>&1
if errorlevel 1 (
    echo %BLUE%Limpiando localmente...%NC%
    if exist "%PROJECT_DIR%\obj" rmdir /s /q "%PROJECT_DIR%\obj"
    if exist "%PROJECT_DIR%\dep" rmdir /s /q "%PROJECT_DIR%\dep"
    if exist "%PROJECT_DIR%\bin" rmdir /s /q "%PROJECT_DIR%\bin"
) else (
    echo %BLUE%Ejecutando make fclean...%NC%
    docker exec %CONTAINER_NAME% make fclean
)
echo %GREEN%✓ Limpieza completa%NC%
goto :eof

:status
echo %CYAN%═══════════════════════════════════════════════════════════════%NC%
echo %CYAN%                    Estado del Entorno                          %NC%
echo %CYAN%═══════════════════════════════════════════════════════════════%NC%
echo.

docker images --format "{{.Repository}}:{{.Tag}}" | findstr /x "%IMAGE_NAME%:latest" >nul 2>&1
if errorlevel 1 (
    echo %RED%✗%NC% Imagen: No encontrada
) else (
    echo %GREEN%✓%NC% Imagen: %IMAGE_NAME%:latest
)

docker ps --format "{{.Names}}" | findstr /x "%CONTAINER_NAME%" >nul 2>&1
if errorlevel 1 (
    docker ps -a --format "{{.Names}}" | findstr /x "%CONTAINER_NAME%" >nul 2>&1
    if errorlevel 1 (
        echo %RED%✗%NC% Contenedor: No existe
    ) else (
        echo %YELLOW%○%NC% Contenedor: Detenido
    )
) else (
    echo %GREEN%✓%NC% Contenedor: Corriendo
)

if exist "%PROJECT_DIR%\bin\webserv" (
    echo %GREEN%✓%NC% Binario: %PROJECT_DIR%\bin\webserv
) else (
    echo %RED%✗%NC% Binario: No compilado
)
echo.
goto :eof

:logs
docker logs -f %CONTAINER_NAME%
goto :eof

:remove
echo %YELLOW%¿Eliminar contenedor e imagen? (S/N)%NC%
set /p response=
if /i "%response%"=="s" (
    echo %BLUE%Eliminando contenedor...%NC%
    docker rm -f %CONTAINER_NAME% 2>nul
    echo %BLUE%Eliminando imagen...%NC%
    docker rmi %IMAGE_NAME%:latest 2>nul
    echo %GREEN%✓ Eliminado%NC%
) else (
    echo %YELLOW%Cancelado%NC%
)
goto :eof

:help
echo %CYAN%═══════════════════════════════════════════════════════════════%NC%
echo %CYAN%           Webserv Docker Development Environment              %NC%
echo %CYAN%═══════════════════════════════════════════════════════════════%NC%
echo.
echo %GREEN%Uso:%NC% %~nx0 ^<comando^>
echo.
echo %YELLOW%Comandos disponibles:%NC%
echo.
echo   %GREEN%build%NC%        Construir la imagen Docker (solo entorno)
echo   %GREEN%start%NC%        Iniciar contenedor en modo interactivo
echo   %GREEN%stop%NC%         Detener contenedor
echo   %GREEN%restart%NC%      Reiniciar contenedor
echo   %GREEN%shell%NC%        Abrir shell en contenedor en ejecución
echo   %GREEN%compile%NC%      Compilar webserv dentro del contenedor
echo   %GREEN%run%NC%          Compilar y ejecutar webserv
echo   %GREEN%test%NC%         Ejecutar tests básicos
echo   %GREEN%clean%NC%        Limpiar archivos compilados
echo   %GREEN%fclean%NC%       Limpiar todo (binarios incluidos)
echo   %GREEN%status%NC%       Mostrar estado del contenedor
echo   %GREEN%logs%NC%         Mostrar logs del contenedor
echo   %GREEN%remove%NC%       Eliminar contenedor e imagen
echo.
echo %YELLOW%Ejemplos:%NC%
echo   %~nx0 build      # Construir imagen (primera vez)
echo   %~nx0 start      # Iniciar entorno de desarrollo
echo   %~nx0 compile    # Compilar el proyecto
echo   %~nx0 run        # Ejecutar el servidor
echo   %~nx0 shell      # Acceder al contenedor
echo.
goto :eof
