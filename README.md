# General Server — XML/XSLT Web and Database Server

[![Build](https://github.com/anewholm/generalserver/actions/workflows/build.yml/badge.svg)](https://github.com/anewholm/generalserver/actions/workflows/build.yml)
[![CodeQL](https://github.com/anewholm/generalserver/actions/workflows/codeql.yml/badge.svg)](https://github.com/anewholm/generalserver/actions/workflows/codeql.yml)
![Human made content](human-made-content.png "Human made content")

GS (General Server) has innovative solutions for many of the issues in the industry today. Physically, it is a C++ web and application server that uses XML as its data model and XSLT as its query and transformation language. The entire server state — configuration, data, and logic — is represented as a live XML tree navigated via XPath and transformed via XSLT. Semantically, it is a different, more flexible and secure way of accessing information and working together.

| Date          | Project Event         |
|---------------|-----------------------|
| Janruary/2002 | Project start         |
| February/2005 | v1.0 Development completed |
| March/2020    | Pushed to GitHub      |
| April/2026    | CI/CD M4-autoconf multi-platform compatibility (Claude assisted) |

## Industry solutions / Key features

Everything in GS is a node. For example, a JavaScript `js:if` statement, a CSS `overflow:hidden` rule, an `object:user`, a document `doc:paragraph`, a `service:website @port="80"`. Every configuration value, every piece of logic — all stored as XML nodes and all **addressable via XPath**. There is no distinction between code and data at the storage level. This means that all the features below apply to data, markup, transforms and source code alike.

### XPath-based hooking / patching into JavaScript and XSLT
> For decades hooks have been used to allow 3rd party plugins to be extended. For decades, developers have been limited to the hooks that other developers _predict_ might be needed. For decades, awkward line number based patches have been used.

JavaScript and XSLT stylesheets are stored as **XML nodes** with associated attributes, not flat files. This enables a hooking/patching model that no other server provides: instead of hooking in where the developer allows, or patching by line number, you alter/add code by **XPath expression**. See the section on [Versioning (Deviations)](#versioning-deviations) below to understand how these changes are saved.

This means any programmer can hook into any JavaScript function or XSLT template at a structural named, described position — without the original author needing to provide a hook. It is the XPath equivalent of monkey-patching, but safe and declarative.

### XML inheritance
> OO makes everything more extensible and future proof

All technologies in General Server — JSL transforms, JavaScript, and CSS — support DTD defiend inheritance. A stylesheet, script, or style rule can inherit from another node. This means that an XSLT `<xsl:template match="css:overflow">` will match `css:my-overflow` if the DTD defines it as a derived element. The same goes for `object:person` and `object:employee`. Combined with XPath addressability, this allows fine-grained overriding at the node level.

### JSL — JSON-style XSL with IDE debugger
> Nobody liked XSL syntax

JSL (JSON-style XSL) is one of General Server's transformation languages. It has all the power of XSLT with a more readable syntax and a built-in **stepping IDE debugger**:

```jsl
if (/config/object:Database/@name == $database-name) {
  <p>Using main DB</p>
} else {
  <p>Using {$database-name} DB</p>
}
```

### Database-level security / visibility control — zero application code
> Complexity and multiple layers introduces security risks and overlaps with permissions and visibility.

GS Security is enforced entirely at the database level. This means that even a CSS node, or JavaScript `if` statement can be subject to security / visibility. The server sets the security context on login; nodes that the logged-in user cannot see are simply not returned. Programmers never write login, session, or authorisation code. It cannot be bypassed.

### Internet data triggers - foreign nodes
> Systems integration, micro-services, distributed architectures across different data-sources are difficult to setup and maintain. XML has always been the secure common ground

Individual database nodes can transparently represent remote data via read/write triggers. XPath queries seamlessly access external APIs / URLs — no HTTP client code needed; just add a trigger and query as normal. Remote text streams can be automatically converted in to XML documents seamlessly using RegularX and traversed, HTML documents are automatically parsed in to valid XML node structures.

### Hard and Soft-linking
> Where should the Soya milk go? In the hippy-food aisle or in the breakfast aisle?

Soft-linking is implemented, the same way as Linux filesystem does. It is an extra soft-link node that contains an xpath attribute pointing to its desired alternate location. In these cases, XPath must be written to traverse such links.

The server-side XML specification has been extended to include hard links. These work identically to Linux filesystem hardlinks. That is, any node can have _multiple_ parents. For example, a single `policy document` node can be attached to many parent `meeting` nodes. This is implemented deep down in the augmented `LibXML-rr` and is seamless to XPath. However, XPath will _re-ascend_ up along the `ancestor` and `parent` axises the same way it took to come down. And I deserve a medal for achieving that :)

### Versioning (Deviations)
> Source Code Versioning, document change suggestions, alternate configurations, all implemented differently but all sitting on essentially the same concept: stated and visible divergent changes on top of the same base information.

A hardlinked node may attach _deviations_ to its sub-tree content and structure specific to a stated parent. In other words, content can be different depending to how you arrived at it. In other words, you can _deviate_ a nodes' content or attributes linked to the hardlinks in its ancestor path.

For example, `philip/my-documents/yearly-report` is also hardlinked to `sarah/my-documents/yearly-report`. The `yearly-report` is 1 node with 2 parents and 1 sub-tree. However, Sarah has re-written paragraph 2 of the Summary `./section[title=Summary]/p[2]` chosing the **deviate** option when saving. Thus, the paragraph has not been changed, just a deviation registered for that `p[2]` at the `yearly-report` hardlink for `sarah`s view. `philip` will not see that in his `my-documents/yearly-report`.

This enables community versioning of any node, which is everything, inculding JavaScript, CSS, XSL, documents, configurations, etc. Deviations can be listed, accepted, talked about, as explained in [Connected developer ecosystem](#connected-developer-ecosystem).

### Connected developer ecosystem
> Software developers need 3rd party online discussion forums to harvest use-cases, issues and ideas for their software. The issues are often difficult to understand.

All General Server installations are networked. Each node is a discussion forum, visible to anyone who accesses it. An XPath expression is sufficient to see who is extending a class, anywhere in the diaspora. Patches, inheritance, discussion, and code sharing happen within the IDE itself — there is no separate module store or discussion forum. For example, you can click on a 3rd party `<css:overflow>scroll</css:overflow>` node and discuss it with anyone else who is looking at it, or watching it.

### Performance
> HTML is big, cannot be cached on the client-side and repeats itself endlessly. HTTP Servers are sending the entire _presentation_ to the client with raw information hidden within it

All modern browsers can carry out XSLT for XML => (X)HTML natively. Thus, GS returns XML to the client browser, with an embedded XSL transform directive. XSL stylesheets are requested once and cached on the client side thus providing a huge reduction in bandwidth usage and performance boost over other frameworks. Similar architectural concepts are now implemented manually in Node.js with React.js+Next.js / Vue.js+Nuxt.js transporting only JSON variables and cached / re-used client-side HTML templates.

> **Note:** that there are calls [by Google](https://developer.chrome.com/docs/web-platform/deprecating-xslt) to discontinue XSLT support in major browsers. This would force GS to conduct XSLTs server-side thus negating its bandwidth performance advantage.

## Why XML as a database?
There has always been a general and un-necessary development task with RDBs (Relation Databases) to twist them in to hierarchical output, fitting a square peg in to a round hole.

- Websites, that is HTML, navigation, OO languages, Prototype-chain languages, SCSS etc. are inherently hierarchical. XML as a datasource simplifies this, limited by DTDs, the equivalent of RDBMS DDL. XML Hierarchy => XHTML/API XML/JSON hierarchy.
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
| Kali Linux (rolling) | Tested in CI |
| Kali Linux (last-release) | Tested in CI |

CI runs a matrix build across all eight distributions on every push.

## Prerequisites

```bash
sudo apt-get -y install autoconf libtool gdb uuid-dev libpq-dev
```

## Building

General Server bundles modified versions of libxml2 and libxslt (suffixed `-rr`) that are compiled as static libraries. These must be built first. Note that [General Resources Server](https://github.com/anewholm/general_resources_server) must also be up and running for the **Web-based IDE admin suite** to work.

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

> For the **Web-based IDE admin suite**: The standard General Server IDE will not be available without [General Resources Server](https://github.com/anewholm/general_resources_server) installed and running. See below for instructions.

```bash
./bin/generalserver
```

General Server listens on `http://localhost:8776` by default and runs its built-in test suite on startup.

For interactive debugging with gdb:

```bash
gdb --args bin/generalserver interactive
```

## Installing as a systemd service (Ubuntu)

General Server reads its configuration from `./config` using a path relative to its working directory, so the service must be started from the repository root.

1. Create the service file:

   ```bash
   sudo tee /etc/systemd/system/generalserver.service << 'EOF'
   [Unit]
   Description=General Server XML/XSLT web and application server
   After=network.target postgresql.service
   Wants=postgresql.service

   [Service]
   Type=simple
   User=YOUR_USERNAME
   Group=YOUR_USERNAME
   WorkingDirectory=/path/to/generalserver
   ExecStart=/path/to/generalserver/bin/generalserver single
   Restart=on-failure
   RestartSec=5s
   AmbientCapabilities=CAP_NET_BIND_SERVICE

   [Install]
   WantedBy=multi-user.target
   EOF
   ```

   Replace `/path/to/generalserver` with the actual path to your clone (e.g. `/home/user/Software/generalserver`), and `YOUR_USERNAME` with the user who owns that directory. Do not use `www-data` if the repository lives under a home directory, as `www-data` cannot change into it.

2. Reload systemd and verify the service is registered:

   ```bash
   sudo systemctl daemon-reload
   sudo systemctl status generalserver
   ```

3. Start and enable on boot when ready:

   ```bash
   sudo systemctl start generalserver
   sudo systemctl enable generalserver
   ```

4. View logs:

   ```bash
   sudo journalctl -u generalserver -f
   ```

To install as **disabled** (registered but not started automatically), skip the `enable` step. The service can then be started manually with `sudo systemctl start generalserver`.

## Production deployment

General Server currently must run behind a load balancer or reverse proxy (e.g. [Apache](https://httpd.apache.org/docs/2.4/howto/reverse_proxy.html), Nginx, HAProxy, or a cloud gateway) that terminates HTTPS and forwards plain HTTP to General Server. This is a standard production pattern the same as [Next.js](https://nextjs.org/) — General Server handles application logic while the gateway handles TLS, rate limiting, and upstream routing.

> In future GS may handle HTTPS directly.

## Configuration

The server loads its configuration from [`./config`](config), which contains HTTP service definitions, database connections, and website trees as XML. See [`config/websites/general_server/`](config/websites/general_server/) for the bundled documentation website.

## General Resources Server (companion)

[GRS (General Resources Server)](https://github.com/anewholm/general_resources_server) is a lightweight companion website for serving binary and static assets (images, fonts, third-party JS/CSS libraries). General Server is XML-only. It can serve Base64 encoded binary files directly but GRS does it better. GRS is also the place for PHP custom connectors to external systems.

The admin interface and documentation pages will not render correctly in a browser without GRS running, because they reference jQuery, CodeMirror, and image files served from `http://general-resources-server.laptop/`.

### Installing GRS

1. Clone into the web root and set ownership:

   ```bash
   sudo git clone https://github.com/anewholm/general_resources_server /var/www/general_resources_server
   sudo chown -R www-data:www-data /var/www/general_resources_server
   ```

2. Create an Apache vhost at `/etc/apache2/sites-available/general-resources-server.conf`:

   ```apache
   <VirtualHost *:80>
       ServerName general-resources-server.laptop
       DocumentRoot /var/www/general_resources_server
       DirectoryIndex index.php index.html

       <Directory /var/www/general_resources_server>
           Options Indexes FollowSymLinks
           AllowOverride All
           Require all granted
       </Directory>

       ErrorLog ${APACHE_LOG_DIR}/general-resources-server-error.log
       CustomLog ${APACHE_LOG_DIR}/general-resources-server-access.log combined
   </VirtualHost>
   ```

3. Enable the vhost and add the hostname:

   ```bash
   sudo a2ensite general-resources-server
   sudo systemctl reload apache2
   echo "127.0.0.1  general-resources-server.laptop" | sudo tee -a /etc/hosts
   ```

4. Verify by visiting `http://general-resources-server.laptop/` — you should see the resources directory listing.

## Architecture

### MVC (Model-View-Controller)
GS uses industry standard MVC ideas:

- Model: a standard XML + DTD combination.
- View: XSL
- Front-end Controllers: Javascript
- Back-end Controllers: DOM level 3 HTTP/CRUD operations, XPath + XSL

### Request - response process
GS does not natively understand the HTTP or other protocols. These protocols are created and configured in the `/services` collection. This requires the ability to convert incoming text streams in to XML documents, which is done by the `RegularX.cpp` system using configurable regular expressions.

1. A `/service/<name>` has a defined port to listen on, like HTTP on 80. Incoming text stream client requests will initiate this process
2. A defined RegularX document in the service transforms the incoming text streams in to an XML document into the service `./requests` collection
3. A defined server-side XSL document transforms the request, with access to the whole database limited by security
4. The resultant XML document will be returned to the client, potentially with a client side XSLT command if required.

### RegularX
RegularX transforms text-streams in to XML documents using Regular Expressions. Below we see the transform for HTTP/1.0. It is only triggered if the incoming text-stream matches the stated regular expression in `accepted_format.xml`:
`^(GET) /([^? ]*)\??([^ ]*) HTTP/(\d+)\.(\d+)\s+
[Hh]ost:\s*([^\s]+)\s+
(.*)\s+`

```
<object:Request url="$2">
  <HTTP type="$1" major="$4" minor="$5" X-Requested-With="^X-Requested-With:\s*([^\r\n]+)">=http</HTTP>
  <rx:scope rx:regex="^Host:\s*([^\n\r]+)\s*$">
    <host port=":([^\n\r]+)">^([^:]+)</host>
  </rx:scope>
  <user-agent>
    <rx:scope rx:regex="^User-Agent:\s*([^\n\r]+)\s*$">
      <rx:multi rx:name="agent" rx:regex="([^ ]+/[^ ]+(?: [(][^)]+[)])?) "/>
    </rx:scope>
  </user-agent>
  ...
<object:Request url="$2">
```

## License

MIT
