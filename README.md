# Documents
See	`generalserver/config/websites/general_server/`:
 * [TODO.xml](config/websites/general_server/TODO.xml)
 * [WHY.xml](config/websites/general_server/WHY.xml)
 * [README.xml](config/websites/general_server/README.xml)
These XML files are better viewed through GS when running.

# Installation
## 3rd party libraries (altered)
These are static libraries compiled into `bin/generalserver`.
This sets up for Python 2. Replacements of all python print statements to `print()` will be necessary.
`configure` option with debug: `--with-debug`.
Creates `.libs/libx*rr.a`.
```
cd src/installations/libx*rr/
autoreconf -f -i
./configure # --with-debug
make
```

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

