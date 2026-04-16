# Docker Development Environment - Webserv

## 📦 Imágenes Disponibles

| Imagen | Base | Tamaño | 42 Tester | Uso |
|--------|------|--------|-----------|-----|
| `webserv-alpine` | Alpine 3.19 | ~150MB | ❌ NO compatible | Desarrollo general |
| `webserv-ubuntu` | Ubuntu 22.04 | ~500MB | ✅ Compatible | Tests oficiales 42 |

## 🚀 Uso Rápido

### Con Makefile

```bash
# Construir Alpine (por defecto, ligero)
make docker-build

# Construir Ubuntu (compatible con 42 tester)
make docker-build-ubuntu

# Cambiar entre bases
make docker-use-alpine    # Cambiar a Alpine
make docker-use-ubuntu    # Cambiar a Ubuntu
make docker-build         # Reconstruir con la base seleccionada

# Iniciar contenedor
make docker-start         # Usa la imagen configurada actualmente
make docker-start-alpine  # Fuerza Alpine
make docker-start-ubuntu  # Fuerza Ubuntu

# Ejecutar tests
make docker-test          # Tests de integración
make docker-test-42       # 42 tester (requiere Ubuntu)
make docker-test-all      # Todos los tests
```

### Con Script

```bash
cd docker

# Construir con Alpine (default)
./webserv-docker.sh build

# Construir con Ubuntu
./webserv-docker.sh --ubuntu build

# Cambiar entre bases
./webserv-docker.sh switch ubuntu
./webserv-docker.sh switch alpine

# Ver estado
./webserv-docker.sh status

# Ejecutar tests
./webserv-docker.sh test
./webserv-docker.sh test-42    # Requiere Ubuntu
```

### Con Docker Compose

```bash
cd docker

# Desarrollo con Alpine
docker-compose up webserv

# Desarrollo con Ubuntu (42 tester)
docker-compose --profile ubuntu up webserv-ubuntu
```

## 📁 Estructura de Archivos

```
docker/
├── Dockerfile           # Alpine (default)
├── Dockerfile.alpine    # Alpine (backup explícito)
├── Dockerfile.ubuntu    # Ubuntu (42 tester compatible)
├── docker-compose.yml   # Servicios para ambas bases
├── webserv-docker.sh    # Script de gestión
├── .docker-config       # Configuración actual (auto-generado)
└── README.md            # Este archivo
```

## ⚠️ Notas Importantes

### 42 Tester Oficial

El binario `ubuntu_tester` está compilado para **Ubuntu/Debian (glibc)** y **NO funcionará** en Alpine (musl libc).

**Si necesitas ejecutar el 42 tester oficial:**

1. **Opción 1**: Usar Ubuntu en Docker
   ```bash
   make docker-build-ubuntu
   make docker-start-ubuntu
   make docker-test-42
   ```

2. **Opción 2**: Ejecutar en tu máquina host (Ubuntu)
   ```bash
   make test-42
   ```

### Diferencias entre Alpine y Ubuntu

| Característica | Alpine | Ubuntu |
|----------------|--------|--------|
| Tamaño imagen | ~150MB | ~500MB |
| Tiempo build | ~1 min | ~3 min |
| libc | musl | glibc |
| 42 tester | ❌ | ✅ |
| siege | Compilado | Preinstalado |

## 🔧 Comandos Makefile Completos

### Build
```bash
make docker-build          # Construir imagen actual
make docker-build-alpine   # Construir Alpine
make docker-build-ubuntu   # Construir Ubuntu
```

### Container
```bash
make docker-start          # Iniciar contenedor
make docker-stop           # Detener contenedor
make docker-restart        # Reiniciar contenedor
make docker-shell          # Abrir shell
```

### Tests
```bash
make docker-test           # Tests integración
make docker-test-all       # Todos los tests
make docker-test-42        # 42 tester (solo Ubuntu)
make docker-test-42-auto   # 42 tester automático
make docker-siege          # Benchmark
```

### Utility
```bash
make docker-status         # Ver estado
make docker-logs           # Ver logs
make docker-remove         # Eliminar todo
make docker-clean-all      # Limpiar todo
```

## 📝 Configuración Persistente

El script guarda tu preferencia en `.docker-config`:

```bash
# Ver configuración actual
cat docker/.docker-config
# Output: DOCKER_BASE=alpine

# Cambiar manualmente
echo "DOCKER_BASE=ubuntu" > docker/.docker-config
```

## 🐛 Troubleshooting

### Error: "ubuntu_tester: not found"
- **Causa**: Estás usando Alpine
- **Solución**: `make docker-start-ubuntu`

### Error: "apk: not found" al construir
- **Causa**: Dockerfile mezcla comandos Alpine/Ubuntu
- **Solución**: Usar los Dockerfiles correctos proporcionados

### Contenedor no inicia
```bash
make docker-remove
make docker-build
make docker-start
```

### Tests fallan con "No server on port 8080"
- El servidor se detuvo antes de los tests adicionales
- Usar el Makefile actualizado que maneja esto correctamente
