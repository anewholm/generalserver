noinst_PROGRAMS	= gjobread

AM_CPPFLAGS = -I$(top_builddir)/include -I$(top_srcdir)/include -I@srcdir@/include @THREAD_CFLAGS@ @Z_CFLAGS@
DEPS = $(top_builddir)/libxml2rr.la
LDADDS = @STATIC_BINARIES@ $(top_builddir)/libxml2rr.la @THREAD_LIBS@ @Z_LIBS@ $(ICONV_LIBS) -lm @WIN32_EXTRA_LIBADD@

gjobread_SOURCES=gjobread.c
gjobread_LDFLAGS=
gjobread_DEPENDENCIES= $(DEPS)
gjobread_LDADD= @RDL_LIBS@ $(LDADDS)
