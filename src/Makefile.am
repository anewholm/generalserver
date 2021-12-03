## Process this file with automake to produce Makefile.in
ABS_DIR_HERE = $(abs_top_srcdir)
SUBDIRS = server LibXml
AUTOMAKE_OPTIONS = subdir-objects
INCLUDE_DIRS_STANDARD = \
-I$(abs_top_srcdir) \
-I$(abs_top_srcdir)/server \
-I$(abs_top_srcdir)/LibXml \
-I$(abs_top_srcdir)/installations/libxml2-rr/include \
-I$(abs_top_srcdir)/installations/libxslt-rr
export INCLUDE_DIRS_STANDARD

ACLOCAL_AMFLAGS = -I m4

LINKER_DIRS_STANDARD = \
-L$(abs_top_srcdir) \
-L$(abs_top_srcdir)/server \
-L$(abs_top_srcdir)/LibXml \
-L$(abs_top_srcdir)/installations/libxml2-rr/.libs \
-L$(abs_top_srcdir)/installations/libxslt-rr/libxslt/.libs \
-L$(abs_top_srcdir)/installations/libxslt-rr/libexslt/.libs

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
        
bin_PROGRAMS = generalserver
# generalserver_CC = $(PTHREAD_CC)
AM_CPPFLAGS = $(INCLUDE_DIRS_STANDARD) $(PTHREAD_CFLAGS)
# AM_LDFLAGS = $(LINKER_DIRS_STANDARD)
# shared library options: -lxml2rr -lexsltrr -lxsltrr
# libxml2rr requires m,z,pthread
# we are using the archives which contain no dependency info
# shared libs contain dependency info
AM_LDFLAGS = $(LINKER_DIRS_STANDARD)
# compiling *-rr libraries directly in to the source
generalserver_LDADD = \
server/libserver.a \
LibXml/libxml2wrapper.a \
server/Xml/libgsxml.a \
server/QueryEnvironment/libqueryenvironment.a \
server/Utilities/libutilities.a \
installations/libxslt-rr/libexslt/.libs/libexsltrr.a \
installations/libxslt-rr/libxslt/.libs/libxsltrr.a \
installations/libxml2-rr/.libs/libxml2rr.a
generalserver_SOURCES = $(abs_top_srcdir)/generalserver.cpp

