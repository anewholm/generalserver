# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

### 1. Build modified bundled libraries first (only needed once, or after library changes)

```bash
cd src/installations/libxml2-rr/
autoreconf -f -i && ./configure && make clean && make
# Produces: .libs/libxml2rr.a

cd src/installations/libxslt-rr/
autoreconf -f -i && ./configure && make clean && make
# Produces: libxslt/.libs/libxsltrr.a  libexslt/.libs/libexsltrr.a
```

### 2. Build the main server

```bash
cd src/
./configure        # add --with-debug for -O0 -ggdb
make clean
make
make install       # copies binary to ../bin/generalserver
```

Ignore these expected warnings during configure/autoreconf:
- `warning: The macro 'AC_*' is obsolete`
- `/usr/bin/rm: cannot remove 'libtoolT': No such file or directory`

Configure should end with `Done configuring`.

### Build options (./configure flags)
| Flag | Effect |
|------|--------|
| `--with-debug` | -O0 -ggdb, enables Debug module |
| `--with-database-readonly` | Prevent permanent DB changes |
| `--with-mm` | Custom memory management |
| `--with-database-idmode` | Enforce @xml:id on all elements |
| `--with-database-namespaceroot` | Centralise namespace declarations |

## Running

```bash
./bin/generalserver                        # interactive (blocks main thread for gdb)
gdb --args bin/generalserver interactive   # debug mode (requires --with-debug build)
```

Serves on **http://localhost:8080** by default. Loads config from `./config/`.

GS runs its full test suite automatically at startup unless told otherwise.

## Architecture

### Request pipeline
HTTP request → `Service` → `Server` → `Request` parsing → XSLT/Regex transform → `Response`

### XML as the database
The `./config/` directory tree *is* the database. `Repository` maps directory/file structure into an in-memory XML DOM. All runtime state, routing, users, sessions, classes, and transforms live as XML nodes.

### Layered XML/XSLT abstraction
```
config/ (XML files)
    └── Repository       — maps filesystem tree → XML DOM
    └── Database.cpp     — core in-memory XML engine (~204 KB)
        └── DatabaseNode / DatabaseClass
    └── IXml/            — pure interfaces
    └── server/Xml/      — GS-level wrappers (XmlBaseDoc, XmlBaseNode, XslDoc …)
    └── LibXml/          — libxml2/libxslt bindings (LibXmlBaseNode ~123 KB, LibXslTransformContext)
    └── installations/libxml2-rr + libxslt-rr  — patched upstream libraries
```

### Modified libraries (libxml2-rr / libxslt-rr)
These are **substantially patched** forks of libxml2 and libxslt compiled as static archives (`libxml2rr.a`, `libxsltrr.a`, `libexsltrr.a`). They are linked directly into the binary. When debugging XML/XSLT behaviour, diff against upstream to understand what was changed before assuming a bug is in GS code.

### Key source files
| File | Role |
|------|------|
| `src/generalserver.cpp` | Entry point → `Main()` |
| `src/server/Main.cpp` | Bootstrap: load config, start services, run tests |
| `src/server/Database.cpp` | Core XML database engine |
| `src/server/Repository.cpp` | Filesystem-to-XML mapping |
| `src/server/TXml.cpp` | Template/extended XML processing |
| `src/server/XSLFunctions.cpp` | Custom XSLT extension functions |
| `src/LibXml/LibXmlBaseNode.cpp` | libxml2 node binding layer |
| `src/LibXml/LibXslTransformContext.cpp` | XSLT transform context |
| `src/server/Utilities/regexpr2.cpp` | Custom regex engine (232 KB) |

### Configuration tree layout
```
config/
├── services/HTTP/requests/   — request routing rules
├── services/HTTP/sessions/   — session config
├── websites/                 — per-site XSLT + pages
├── classes/                  — class definitions (XML)
├── databases/                — DB connection config
├── users/ groups/            — auth
├── system_transforms/        — system-level XSLT
└── tests/                    — test definitions (run at startup)
```

### Threading & sessions
`Thread.cpp` / `ProfilerThread.cpp` provide POSIX threading. Sessions are stateful and XPath-addressable (`--with-session-xpath`). Database triggers fire on read/write (`--with-database-trigger`).

## Dependencies
- `libpq` — PostgreSQL
- `uuid`, `pthread`, `readline`, `history`, `gcrypt`, `zlib`, `libdl`
- All XML/XSLT via the bundled `-rr` static libraries (not system libxml2/libxslt)
