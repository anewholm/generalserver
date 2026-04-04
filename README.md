# General Server — XML/XSLT Web and Database Server

[![Build](https://github.com/anewholm/generalserver/actions/workflows/build.yml/badge.svg)](https://github.com/anewholm/generalserver/actions/workflows/build.yml)

General Server is a C++ web and application server that uses XML as its data model and XSLT as its query and transformation language. The entire server state — configuration, data, and logic — is represented as a live XML tree navigated via XPath and transformed via XSLT.

## Why XML as a database?

- The data model, schema, and transformation logic are all in the same query language (XPath/XSLT).
- XSLT stylesheets act as both views and controllers, producing HTML, XML, JSON, or any other output format.
- The XML tree is persistent, addressable by URL, and directly servable as a website.
- Security policies, caching rules, and service configuration are themselves XML nodes — live, introspectable, and modifiable at runtime.

For a full architectural rationale see [config/websites/general_server/WHY.xml](config/websites/general_server/WHY.xml) (best viewed through a running General Server instance).

## Compatibility

| OS              | Status  |
|-----------------|---------|
| Ubuntu 22.04    | Tested  |
| Ubuntu 24.04    | Tested  |
| Linux Mint 21   | Tested  |
| Linux Mint 22   | Tested  |
| Fedora 40       | Tested  |
| Fedora 41       | Tested  |

CI runs a matrix build across all six distributions on every push.

## Prerequisites

```bash
sudo apt-get -y install autoconf libtool gdb uuid-dev libpq-dev
```

## Building

General Server bundles modified versions of libxml2 and libxslt (suffixed `-rr`) that are compiled as static libraries. These must be built first.

### 1. Build libxml2-rr

```bash
cd src/installations/libxml2-rr/
autoreconf -f -i
./configure
make clean
make
```

Creates `src/installations/libxml2-rr/.libs/libxml2rr.a`.

### 2. Build libxslt-rr

```bash
cd src/installations/libxslt-rr/
autoreconf -f -i
./configure
make clean
make
```

Creates `src/installations/libxslt-rr/libxslt/libxsltrr.la`.

### 3. Build General Server

```bash
cd src/
./configure
make clean
make
make install
```

To build with debug symbols, pass `--with-debug` to each `./configure` call.

You may safely ignore these warnings during `autoreconf`:
```
warning: The macro `AC_*' is obsolete
/usr/bin/rm: cannot remove 'libtoolT': No such file or directory
```
The build is successful if `configure` ends with `Done configuring`.

## Running

```bash
./bin/generalserver
```

General Server listens on `http://localhost:8080` by default and runs its built-in test suite on startup.

For interactive debugging with gdb:

```bash
gdb --args bin/generalserver interactive
```

## Configuration

The server loads its configuration from [`./config`](config), which contains HTTP service definitions, database connections, and website trees as XML. See [`config/websites/general_server/`](config/websites/general_server/) for the bundled documentation website.

## Sister project

[general_resources_server](https://github.com/anewholm/general_resources_server) — a lightweight Apache/PHP server that serves static assets (images, CSS, downloads) as a companion to General Server.

## License

MIT
