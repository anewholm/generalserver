# Documents
See	[config/websites/general_server/](config/websites/general_server/):
 * [TODO.xml](config/websites/general_server/TODO.xml)
 * [WHY.xml](config/websites/general_server/WHY.xml)
 * [README.xml](config/websites/general_server/README.xml) just links to the 2 above.

These XML files are better viewed through GS when running.

# Installation
## Build pre-requisites
```sudo apt-get -y install autoconf libtool gdb```

## 3rd party dev libraries
```sudo apt-get -y install uuid-dev```

## 3rd party altered bundled libraries (-RR)
These libraries (RR) have been substantially altered. They are static libraries compiled into `bin/generalserver`.
This sets up for Python 2. Replacements of all python print statements to `print()` will be necessary.
`configure` option with debug: `--with-debug`.

### LibXML2
[XMLSoft](http://xmlsoft.org/) produces XML.
[Full INSTALL instructions](src/installations/libxml2-rr/INSTALL)
```
cd src/installations/libxml2-rr/
autoreconf -f -i
./configure # --with-debug
make clean
make
```
Creates `.libs/libxml2rr.a`.

### LibXSL
[XMLSoft](http://xmlsoft.org/XSLT/) produces XSLT.
[Full INSTALL instructions](src/installations/libxslt-rr/INSTALL)
```
cd src/installations/libxslt-rr/
autoreconf -f -i
./configure # --with-debug
make clean
make
```
Creates `libxslt/libxslrr.a`.

### Issues
Do not concern yourself with the following issues:
```
warning: The macro `AC_*' is obsolete
/usr/bin/rm: cannot remove 'libtoolT': No such file or directory
/usr/bin/ld: cannot find .../installations/libxslt-rr/libxslt/.libs/*.o: No such file or directory
```
It should say `Done configuring` at the end.

## Installation
Requires `installations/*/.libs/lib*.a` above.
`configure` option with debug: `--with-debug`
```
cd src/
./configure # --with-debug
make
make install
```

## Test installation
GS runs all its configured tests by default at startup unless instructed not to.

# Configuration, directories and setup
The `generalserver/` base directory, by default, loads the XML tree from
[./config](config) which contains the overall setup: http services, databases, etc.
that run on address http://localhost:8080.

# Running
Block on the main thread (interactive) for [gdb](https://en.wikipedia.org/wiki/GNU_Debugger) to catch exceptions
```
./bin/generalserver
```
Debugging. Requires `configure --with-debug`:
```
gdb --args bin/generalserver interactive
```

