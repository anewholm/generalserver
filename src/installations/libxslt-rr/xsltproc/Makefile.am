AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libxslt -I$(top_srcdir)/libexslt \
           -I$(top_builddir) -I$(top_builddir)/libxslt \
	   -I$(top_builddir)/libexslt $(LIBXML_CFLAGS) $(CFLAGS)

EXTRA_PROGRAMS=
bin_PROGRAMS = xsltproc $(XSLTPROCDV)

noinst_PROGRAMS=testThreads

AM_CFLAGS = $(LIBGCRYPT_CFLAGS)

AUTOMAKE_OPTIONS = subdir-objects
xsltproc_SOURCES = $(top_srcdir)/xsltproc/xsltproc.c
xsltproc_LDFLAGS = 
xsltproc_DEPENDENCIES = $(DEPS)

testThreads_SOURCES=testThreads.c
testThreads_LDFLAGS =
testThreads_DEPENDENCIES = $(DEPS)
testThreads_LDADD=  $(THREAD_LIBS) $(LDADDS)

DEPS = $(top_builddir)/libxslt/libxsltrr.la \
	$(top_builddir)/libexslt/libexsltrr.la 

LDADDS = @STATIC_BINARIES@ \
         $(top_builddir)/libxslt/libxsltrr.la \
         $(top_builddir)/libexslt/libexsltrr.la \
	@LIBXML_LIBS@ $(EXTRA_LIBS) @WIN32_EXTRA_LIBADD@

xsltproc_LDADD = $(LIBGCRYPT_LIBS) $(LDADDS)

xsltproc.dv: xsltproc.o
	$(CC) $(CFLAGS) -o xsltproc xsltproc.o ../libexslt/.libs/libexsltrr.a ../libxslt/.libs/libxsltrr.a @LIBXML_LIBS@ $(EXTRA_LIBS) $(LIBGCRYPT_LIBS)

tests: testThreads
	@echo '## Running testThreads'
	@($(CHECKER) ./testThreads ; grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0" || true)
