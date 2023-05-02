# Documents
See:
	generalserver/config/websites/general_server/
		* TODO.xml
		* WHY.xml
		* README.xml

# Server Control
In
	generalserver/ this base directory from github.com
Run
	bin/generalserver
This, by default, loads the XML tree from
	./config
Which contains the overall setup: http services, databases, etc.

# 3rd party libraries
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

# Installation
```
cd src/
./configure
# Requires installations/*/.libs/lib*.a
make
make install
```

# Debugging
```
cd src
./configure --with-debug
make
make install
# Block on the main thread (interactive) for gdb to catch exceptions
gdb --args src/generalserver interactive
```
