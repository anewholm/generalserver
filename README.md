# Documents
See:
	generalserver/config/websites/general_server/
		* TODO.xml
		* WHY.xml
		* README.xml

# Installation
## 3rd party libraries
```
# These are static libraries compiled into generalserver
cd src/installations/libx*rr/
# This sets up for Python 2
# replacements of all python print statements to print() will be necessary
autoreconf -f -i
./configure [--with-debug]
# Create .libs/libx*rr.a
make
```

## Installation
Requires installations/*/.libs/lib*.a above
```
cd src/
./configure # Option with debug: --with-debug
make
make install
```

## Test installation
GS runs all its configured tests by default at startup unless instructed not to.

# Configuration, directories and setup
In
	generalserver/ this base directory from github.com
This, by default, loads the XML tree from
	./config
Which contains the overall setup: http services, databases, etc.
that run on port http://localhost:8080

# Running
Block on the main thread (interactive) for gdb to catch exceptions
```
src/generalserver
```
Debugging:
```
gdb --args src/generalserver interactive
```

