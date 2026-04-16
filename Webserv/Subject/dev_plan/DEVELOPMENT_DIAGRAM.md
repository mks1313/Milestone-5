```mermaid
flowchart TB
    subgraph EPIC1["📦 EPIC 1: Fundamentos<br/>(Semana 1)"]
        U[Utils.cpp<br/>21 puntos]
        M[MimeTypes.cpp<br/>3 puntos]
        W[webserv.hpp<br/>2 puntos]
    end

    subgraph EPIC2["🔌 EPIC 2: Sockets<br/>(Semana 2-3)"]
        S[Server básico<br/>30 puntos]
        C[Client.cpp<br/>5 puntos]
    end

    subgraph EPIC3["📝 EPIC 3: HTTP Parser<br/>(Semana 2-3)"]
        RQ[Request.cpp<br/>23 puntos]
        RS[Response.cpp<br/>11 puntos]
    end

    subgraph EPIC4["⚙️ EPIC 4: Config<br/>(Semana 3-4)"]
        LC[LocationConfig<br/>4 puntos]
        SC[ServerConfig<br/>4 puntos]
        CF[Config parser<br/>11 puntos]
    end

    subgraph EPIC5["🎯 EPIC 5: Handlers<br/>(Semana 4-5)"]
        H[GET/DELETE handlers<br/>22 puntos]
    end

    subgraph EPIC6["⚡ EPIC 6: CGI<br/>(Semana 5-6)"]
        CGI[CGIHandler.cpp<br/>22 puntos]
    end

    subgraph EPIC7["🚀 EPIC 7: Advanced<br/>(Semana 6)"]
        UP[Upload/POST<br/>5 puntos]
        SS[SessionManager<br/>5 puntos]
        KA[Keep-Alive<br/>3 puntos]
        MA[main.cpp<br/>3 puntos]
    end

    %% Dependencies
    EPIC1 --> EPIC2
    EPIC1 --> EPIC3
    EPIC1 --> EPIC4
    
    EPIC2 --> EPIC5
    EPIC3 --> EPIC5
    EPIC4 --> EPIC5
    
    EPIC5 --> EPIC6
    EPIC5 --> EPIC7
    EPIC6 --> EPIC7

    %% Parallel work indicators
    EPIC2 -.- |"Paralelo"| EPIC3
    EPIC3 -.- |"Paralelo"| EPIC4

    style EPIC1 fill:#e1f5fe
    style EPIC2 fill:#fff3e0
    style EPIC3 fill:#f3e5f5
    style EPIC4 fill:#e8f5e9
    style EPIC5 fill:#fce4ec
    style EPIC6 fill:#fff8e1
    style EPIC7 fill:#e0f2f1
```

## Distribución Recomendada por Desarrollador

### Si son 3 desarrolladores:

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                        TIMELINE DE DESARROLLO                                │
├──────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  Semana    │ 1      │ 2      │ 3      │ 4      │ 5         │ 6      │ 7      │
│  ──────────┼────────┼────────┼────────┼────────┼───────────┼────────┼────────│
│  Dev A     │ Epic1  │ Epic3 ─────────►│ Epic5 ────────────►│ Epic7  │ Tests  │
│  (Parser)  │ Utils  │ Request/Response│ Handlers           │ K-Alive│        │
│            │        │                 │                    │        │        │
│  Dev B     │ Epic1  │ Epic2 ──────────────────►│ Epic5     │ Epic7  │ Tests  │
│  (Network) │ Utils  │ Server/Client/Poll       │ Handlers  │ Upload │        │
│            │        │                          │           │        │        │
│  Dev C     │ Epic1  │ Epic4 ─────────►│ Epic5  │ Epic6 ────────────►│ Tests  │
│  (Config)  │ MIME   │ Config parser   │ Errors │ CGI Handler        │        │
│            │        │                 │        │                    │        │
└──────────────────────────────────────────────────────────────────────────────┘
```

### Asignación detallada:

| Desarrollador | Issues Asignados                                           | Puntos Totales |
|---------------|------------------------------------------------------------|----------------|
| **Dev A**     | #15-20 (Request), #21-23 (Response), #28, #30, #41         | ~55 puntos     |
| **Dev B**     | #8-14 (Server/Client), #29, #31-33, #39                    | ~55 puntos     |
| **Dev C**     | #1-7 (Utils), #24-27 (Config), #32, #34-38 (CGI), #40, #42 | ~55 puntos     |

## Hitos de Integración

```
                    ┌──────────────────┐
                    │  Semana 2        │
                    │  INTEGRACIÓN 1   │
                    │  Server acepta   │
                    │  conexiones TCP  │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │  Semana 3        │
                    │  INTEGRACIÓN 2   │
                    │  Parser HTTP     │
                    │  funcional       │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │  Semana 4        │
                    │  INTEGRACIÓN 3   │
                    │  Config + GET    │
                    │  archivos        │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │  Semana 5        │
                    │  INTEGRACIÓN 4   │
                    │  CGI funcionando │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │  Semana 6-7      │
                    │  RELEASE         │
                    │  Servidor        │
                    │  completo        │
                    └──────────────────┘
```

## Criterios de Done por Epic

### Epic 1 - Fundamentos ✅
- [ ] Todos los tests de Utils pasan
- [ ] MimeTypes reconoce extensiones básicas
- [ ] No hay memory leaks

### Epic 2 - Sockets ✅
- [ ] Server escucha en puerto configurado
- [ ] Acepta múltiples conexiones
- [ ] Maneja SIGINT gracefully

### Epic 3 - HTTP Parser ✅
- [ ] Parsea peticiones GET, POST, DELETE
- [ ] Maneja chunked encoding
- [ ] Genera respuestas HTTP válidas

### Epic 4 - Config ✅
- [ ] Lee archivo de configuración NGINX-like
- [ ] Múltiples servers y locations
- [ ] Valida configuración

### Epic 5 - Handlers ✅
- [ ] Sirve archivos estáticos
- [ ] Directory listing funciona
- [ ] DELETE elimina archivos

### Epic 6 - CGI ✅
- [ ] Ejecuta scripts Python/PHP
- [ ] Pasa variables de entorno correctas
- [ ] Timeout de CGI funciona

### Epic 7 - Advanced ✅
- [ ] Upload de archivos funciona
- [ ] Keep-alive mantiene conexiones
- [ ] Sesiones funcionan
