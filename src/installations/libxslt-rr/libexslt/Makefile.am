AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libxslt -I$(top_srcdir)/libexslt \
	   -I$(top_builddir) -I$(top_builddir)/libxslt \
	   -I$(top_builddir)/libexslt $(LIBXML_CFLAGS) $(CFLAGS)

AM_CFLAGS = $(LIBGCRYPT_CFLAGS)

lib_LTLIBRARIES = libexsltrr.la

exsltincdir = $(includedir)/libexslt

exsltinc_HEADERS =                      \
	exslt.h				\
	exsltconfig.h			\
	exsltexports.h

AUTOMAKE_OPTIONS = subdir-objects
libexsltrr_la_SOURCES =                   \
$(abs_top_srcdir)/libexslt/exslt.c				\
$(abs_top_srcdir)/libexslt/common.c			\
$(abs_top_srcdir)/libexslt/crypto.c			\
$(abs_top_srcdir)/libexslt/math.c				\
$(abs_top_srcdir)/libexslt/sets.c				\
$(abs_top_srcdir)/libexslt/functions.c			\
$(abs_top_srcdir)/libexslt/strings.c			\
$(abs_top_srcdir)/libexslt/date.c				\
$(abs_top_srcdir)/libexslt/saxon.c				\
$(abs_top_srcdir)/libexslt/libexslt.h			\
$(abs_top_srcdir)/libexslt/dynamic.c

libexsltrr_la_LIBADD = $(top_builddir)/libxslt/libxsltrr.la $(EXTRA_LIBS)
libexsltrr_la_LDFLAGS = $(WIN32_EXTRA_LDFLAGS) -version-info @LIBEXSLT_VERSION_INFO@ $(LIBGCRYPT_LIBS)

man_MANS = libexslt.3

EXTRA_DIST = $(man_MANS)
