# 🎯 Asignación Definitiva de Issues - Webserv

## Visión General de Roles

| Desarrollador | Rol Principal | Épicas | Puntos | Foco |
|--------------|---------------|---------|--------|------|
| **Dev A** | HTTP & Parsing | 1, 3, 5 | 57 pts | Request/Response, Handlers GET |
| **Dev B** | Network & I/O | 1, 2, 5, 7 | 56 pts | Sockets, Client, Upload |
| **Dev C** | Config & CGI | 1, 4, 6, 7 | 52 pts | Parser config, CGI, Sessions |

---

## 👤 DEV A - HTTP Parser & Handlers
**Especialidad:** Parsing de protocolos, manipulación de strings, lógica HTTP

### Semana 1: Fundamentos
- [x] **Issue #4** - Utilidades HTTP (urlEncode/decode, status) - **4 pts**
- [x] **Issue #5** - Sistema de logging - **2 pts**

### Semana 2-3: HTTP Parser (EPIC 3)
- [x] **Issue #15** - Estructura Request - **3 pts**
- [x] **Issue #16** - Parsear request line - **4 pts**
- [x] **Issue #17** - Parsear headers HTTP - **4 pts**
- [x] **Issue #18** - Parsear body y chunked - **5 pts**
- [x] **Issue #19** - Parsear cookies, query, multipart - **4 pts**
- [x] **Issue #20** - Método parse() principal - **3 pts**
- [x] **Issue #21** - Estructura Response - **3 pts**
- [x] **Issue #22** - Construir respuesta HTTP - **3 pts**
- [x] **Issue #23** - Builders estáticos (error pages, etc) - **5 pts**

### Semana 4-5: Handlers (EPIC 5)
- [x] **Issue #28** - _processRequest() (routing) - **5 pts**
- [x] **Issue #30** - Handler GET para archivos - **6 pts**
- [x] **Issue #33** - Generador directory listing - **4 pts**

### Semana 6: Advanced Features
- [x] **Issue #41** - Implementar Keep-Alive - **3 pts**

### Semana 7: Testing
- [x] **Issue #43** - Suite de tests (compartida) - **2 pts** (de 5)

**Total Dev A: 57 puntos**

---

## 👤 DEV B - Networking & I/O
**Especialidad:** Sockets, multiplexing, I/O no bloqueante, manejo de conexiones

### Semana 1: Fundamentos
- [x] **Issue #1** - Funciones básicas de strings - **3 pts**
- [x] **Issue #2** - Conversiones numéricas - **2 pts**

### Semana 2-3: Sockets (EPIC 2)
- [x] **Issue #8** - Estructura básica Server - **5 pts**
- [x] **Issue #9** - Event loop con poll() - **8 pts**
- [x] **Issue #10** - Manejo de señales - **2 pts**
- [x] **Issue #11** - Clase Client - **5 pts**
- [x] **Issue #12** - Aceptar nuevas conexiones - **3 pts**
- [x] **Issue #13** - Leer/escribir datos de clientes - **4 pts**
- [x] **Issue #14** - Cerrar conexiones y timeouts - **3 pts**

### Semana 4-5: Handlers (EPIC 5)
- [x] **Issue #29** - Resolver rutas de archivos - **3 pts**
- [x] **Issue #31** - Handler DELETE - **2 pts**
- [x] **Issue #32** - Enviar respuestas de error - **2 pts**

### Semana 5-6: Upload (EPIC 7)
- [x] **Issue #39** - Handler POST/Upload - **5 pts**

### Semana 6-7: Integration
- [x] **Issue #42** - main.cpp y banner - **3 pts**

### Semana 7: Testing
- [x] **Issue #43** - Suite de tests (compartida) - **2 pts** (de 5)

**Total Dev B: 56 puntos**

---

## 👤 DEV C - Configuration & CGI
**Especialidad:** Parsers de configuración, procesos (fork/exec), gestión de estado

### Semana 1: Fundamentos
- [x] **Issue #3** - Utilidades de archivos - **5 pts**
- [x] **Issue #6** - Singleton MimeTypes - **3 pts**
- [x] **Issue #7** - Header webserv.hpp - **2 pts**

### Semana 2-3: Configuración (EPIC 4)
- [x] **Issue #24** - LocationConfig - **4 pts**
- [x] **Issue #25** - ServerConfig - **4 pts**
- [x] **Issue #26** - Parser de configuración - **8 pts**
- [x] **Issue #27** - Validación de config - **3 pts**

### Semana 4-5: CGI (EPIC 6)
- [x] **Issue #34** - Estructura CGIHandler - **3 pts**
- [x] **Issue #35** - Construir entorno CGI - **5 pts**
- [x] **Issue #36** - Fork y exec del CGI - **6 pts**
- [x] **Issue #37** - Integrar CGI en Server - **5 pts**
- [x] **Issue #38** - Parsear output CGI - **3 pts**

### Semana 6: Sessions (EPIC 7)
- [x] **Issue #40** - SessionManager - **5 pts**

### Semana 7: Testing
- [x] **Issue #43** - Suite de tests (compartida) - **1 pt** (de 5)

**Total Dev C: 52 puntos**

---

## 📅 Timeline Visual Corregido

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                        TIMELINE DE DESARROLLO                                │
├──────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  Semana    │ 1      │ 2-3              │ 4-5              │ 6      │ 7      │
│  ──────────┼────────┼──────────────────┼──────────────────┼────────┼────────│
│            │        │                  │                  │        │        │
│  Dev A     │ #4,#5  │ EPIC 3: Parser   │ EPIC 5: Handlers │ #41    │ Tests  │
│  (Parser)  │ HTTP   │ #15-23           │ #28,#30,#33      │K-Alive │ #43    │
│            │ Utils  │ Request/Response │ GET/Routing/List │        │        │
│            │        │                  │                  │        │        │
│  ──────────┼────────┼──────────────────┼──────────────────┼────────┼────────│
│            │        │                  │                  │        │        │
│  Dev B     │ #1,#2  │ EPIC 2: Network  │ EPIC 5: Support  │ #39,#42│ Tests  │
│  (Network) │ String │ #8-14            │ #29,#31,#32      │ Upload │ #43    │
│            │ Utils  │ Server/Client    │ Routes/DELETE    │ Main   │        │
│            │        │ Poll/Sockets     │                  │        │        │
│            │        │                  │                  │        │        │
│  ──────────┼────────┼──────────────────┼──────────────────┼────────┼────────│
│            │        │                  │                  │        │        │
│  Dev C     │ #3,#6  │ EPIC 4: Config   │ EPIC 6: CGI      │ #40    │ Tests  │
│  (Config)  │ #7     │ #24-27           │ #34-38           │Session │ #43    │
│            │ Files  │ Parser NGINX-like│ Fork/Exec/Pipes  │Manager │        │
│            │ MIME   │ Location/Server  │ Environment      │        │        │
│            │        │                  │                  │        │        │
└──────────────────────────────────────────────────────────────────────────────┘
```

---

## 🔗 Dependencias y Sincronización

### Puntos de Integración Críticos

#### 🔄 Integración 1 (Fin Semana 1)
**Participantes:** Todos  
**Objetivo:** Biblioteca Utils funcional
- Dev A entrega: #4, #5
- Dev B entrega: #1, #2
- Dev C entrega: #3, #6, #7

**Criterio:** Todos los tests de Utils pasan

---

#### 🔄 Integración 2 (Fin Semana 3)
**Participantes:** Dev A + Dev B  
**Objetivo:** Server acepta conexiones y parsea HTTP
- Dev A entrega: Request + Response completos (#15-23)
- Dev B entrega: Server básico funcionando (#8-14)

**Criterio:** `curl http://localhost:8080/` retorna respuesta (aunque sea error)

---

#### 🔄 Integración 3 (Fin Semana 4)
**Participantes:** Todos  
**Objetivo:** Servidor sirve archivos estáticos
- Dev A entrega: Handlers GET (#28, #30, #33)
- Dev B entrega: Routes y DELETE (#29, #31, #32)
- Dev C entrega: Config parser completo (#24-27)

**Criterio:** GET de archivos funciona con configuración

---

#### 🔄 Integración 4 (Fin Semana 5)
**Participantes:** Dev B + Dev C  
**Objetivo:** CGI funcionando
- Dev C entrega: CGI completo (#34-38)
- Dev B coordina: Integración en event loop

**Criterio:** Script Python ejecuta y retorna respuesta

---

#### 🔄 Integración Final (Semana 6-7)
**Participantes:** Todos  
**Objetivo:** Servidor completo y testeado
- Dev A: Keep-alive (#41)
- Dev B: Upload (#39), Main (#42)
- Dev C: Sessions (#40)
- Todos: Tests (#43)

---

## 📋 Checklist de Coordinación

### Para Dev A (Parser)
- [ ] Diseñar interfaz de Request/Response antes de implementar
- [ ] Compartir headers de Request/Response con Dev B (para Server)
- [ ] Proveer ejemplos de uso de Response::makeError() a todos
- [ ] Documentar formato de output esperado para directory listing

### Para Dev B (Network)
- [ ] Definir interfaz de Client antes de implementar
- [ ] Coordinar con Dev A: cuándo marcar Request como completo
- [ ] Coordinar con Dev C: hooks para CGI en event loop
- [ ] Proveer mock de Server para que otros testeen offline

### Para Dev C (Config)
- [ ] Publicar estructura de ServerConfig/LocationConfig temprano
- [ ] Proveer archivo de configuración de ejemplo
- [ ] Coordinar con Dev B: cuándo registrar pipes de CGI en poll
- [ ] Documentar variables de entorno CGI

---

## 🚨 Reglas de Desarrollo

### 1. No Bloquear a Otros
Si tu issue depende de código de otro dev:
- Crea un **mock/stub** temporal
- Documenta la interfaz esperada
- Comunica en el chat del equipo

### 2. Merge Frequency
- **Dev A:** Merge después de cada grupo de issues (#15-17, #18-20, #21-23)
- **Dev B:** Merge después de Server básico (#8-10), luego Client (#11-14)
- **Dev C:** Merge después de LocationConfig (#24), ServerConfig (#25), Parser (#26-27)

### 3. Code Review
- Mínimo 1 aprobación antes de merge
- Reviewer verifica:
  - [ ] Compila sin warnings
  - [ ] No memory leaks (valgrind)
  - [ ] Tests unitarios pasan
  - [ ] Respeta norminette C++

### 4. Comunicación Diaria
**Daily Standup (10 min):**
1. ¿Qué hice ayer?
2. ¿Qué haré hoy?
3. ¿Tengo algún blocker?

---

## 📊 Seguimiento de Progreso

### Tablero Kanban Sugerido

```
┌──────────┬──────────┬──────────┬──────────┐
│  TODO    │  DOING   │ REVIEW   │   DONE   │
├──────────┼──────────┼──────────┼──────────┤
│          │          │          │          │
│  #1-43   │  #8 (B)  │  #4 (A)  │  #7 (C)  │
│          │  #15 (A) │          │          │
│          │  #24 (C) │          │          │
│          │          │          │          │
└──────────┴──────────┴──────────┴──────────┘
```

**Herramientas sugeridas:**
- GitHub Projects
- Trello
- Notion

---

## 🎯 Métricas de Éxito por Semana

### Semana 1
- [ ] 10 issues cerrados (#1-7, #4, #5)
- [ ] Utils compila sin warnings
- [ ] Tests básicos pasan

### Semana 3
- [ ] Server acepta conexiones TCP
- [ ] Parser HTTP funciona con peticiones simples
- [ ] 20 issues adicionales cerrados

### Semana 5
- [ ] GET/DELETE funcionan con config
- [ ] CGI ejecuta scripts
- [ ] 30 issues totales cerrados

### Semana 7
- [ ] Todos los issues cerrados (43/43)
- [ ] Suite de tests al 100%
- [ ] Documentación completa
- [ ] Demo funcional preparada

---

## 💡 Tips Finales

### Para Dev A
- Usa herramientas de testing HTTP: Postman, curl, httpie
- Testea con payloads grandes (chunked encoding)
- Verifica edge cases: headers duplicados, encodings raros

### Para Dev B
- Usa `netcat` para debugging de sockets
- Implementa logging extensivo en el event loop
- Testea con muchos clientes concurrentes (Apache Bench)

### Para Dev C
- Testea CGI con scripts que fallan deliberadamente
- Verifica timeouts de CGI funcionan
- Usa archivos de config complejos para testing

---

## 📞 Contacto y Ayuda

Si tienes dudas sobre:
- **Issues de Parser:** Consulta a Dev A
- **Issues de Network:** Consulta a Dev B  
- **Issues de Config/CGI:** Consulta a Dev C

**Sesiones de pair programming:**
- Miércoles: Dev A + Dev B (integración Request/Server)
- Jueves: Dev B + Dev C (integración CGI/Server)
- Viernes: Todos (code review semanal)

---

¡Buena suerte con el proyecto! 🚀
