# General Server — XML/XSLT Web and Database Server

[![Build](https://github.com/anewholm/generalserver/actions/workflows/build.yml/badge.svg)](https://github.com/anewholm/generalserver/actions/workflows/build.yml)
[![CodeQL](https://github.com/anewholm/generalserver/actions/workflows/codeql.yml/badge.svg)](https://github.com/anewholm/generalserver/actions/workflows/codeql.yml)

General Server is a C++ web and application server that uses XML as its data model and XSLT as its query and transformation language. The entire server state — configuration, data, and logic — is represented as a live XML tree navigated via XPath and transformed via XSLT.

## Key features

### XML inheritance
All technologies in General Server — JSL transforms, JavaScript, and CSS — support **multiple inheritance**. A stylesheet, script, or style rule can inherit from several parents simultaneously. Combined with XPath addressability, this allows fine-grained overriding at the node level rather than the file level.

### XPath-based patching of JavaScript and XSLT
JavaScript and XSLT stylesheets are stored as **XML nodes**, not flat files. This enables a patching model that no other server provides: instead of patching by line number, you patch by **XPath expression**.

```xml
<!-- Patch after a specific conditional block, by structure — not by line -->
<patch:after select="js:if[@when='authenticated']">
  <js:statement>logAccess($user)</js:statement>
</patch:after>
```

This means any programmer can hook into any JavaScript function or XSLT template at a structural position — without the original author needing to provide a hook. It is the XPath equivalent of monkey-patching, but safe and declarative.

### JSL — JSON-style XSL with IDE debugger
JSL (JSON-style XSL) is General Server's transformation language. It has all the power of XSLT with a more readable syntax and a built-in **stepping IDE debugger**:

```jsl
if (/config/object:Database/@name == $database-name) {
  <p>Using main DB</p>
} else {
  <p>Using {$database-name} DB</p>
}
```

### Everything is a node
Every JavaScript function, every CSS rule, every configuration value, every piece of logic — all stored as XML nodes and all **addressable via XPath**. There is no distinction between code and data at the storage level.

### Database-level security — zero application code
Security is enforced entirely at the database level. The server sets the security context on login; nodes that the logged-in user cannot see are simply not returned. Programmers never write login, session, or authorisation code. It cannot be bypassed.

### Internet data triggers
Individual database nodes can transparently represent remote data via read/write triggers. XPath queries seamlessly access external APIs — no HTTP client code needed; just add a trigger and query as normal.

### Connected developer ecosystem
All General Server installations are networked. Code is always publicly accessible across servers. An XPath expression is sufficient to see who is extending a class, anywhere in the network. Patches, inheritance, discussion, and code sharing happen within the IDE itself — there is no separate module store.

## Why XML as a database?

- The data model, schema, and transformation logic are all in the same query language (XPath/XSLT).
- XSLT stylesheets act as both views and controllers, producing HTML, XML, JSON, or any other output format.
- The XML tree is persistent, addressable by URL, and directly servable as a website.
- Security policies, caching rules, and service configuration are themselves XML nodes — live, introspectable, and modifiable at runtime.

For a full architectural rationale see [config/websites/general_server/WHY.xml](config/websites/general_server/WHY.xml) (best viewed through a running General Server instance).

## Compatibility

| OS                | Status     |
|-------------------|------------|
| Ubuntu 22.04      | Tested in CI |
| Ubuntu 24.04      | Tested in CI |
| Linux Mint 21     | Tested in CI |
| Linux Mint 22     | Tested in CI |
| Fedora 40         | Tested in CI |
| Fedora 41         | Tested in CI |
| Kali Linux 2024.4 | Tested in CI |
| Kali Linux 2025.1 | Tested in CI |

CI runs a matrix build across all eight distributions on every push.

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
