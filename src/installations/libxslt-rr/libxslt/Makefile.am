AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libxslt $(LIBXML_CFLAGS)

lib_LTLIBRARIES = libxsltrr.la

xsltincdir = $(includedir)/libxslt

xsltinc_HEADERS = 			\
	xslt.h				\
	xsltutils.h			\
	pattern.h			\
	templates.h			\
	variables.h			\
	keys.h				\
	numbersInternals.h		\
	extensions.h			\
	extra.h				\
	functions.h			\
	namespaces.h			\
	imports.h			\
	attributes.h			\
	documents.h			\
	preproc.h			\
	transform.h			\
	security.h			\
	xsltInternals.h			\
	xsltconfig.h			\
	xsltexports.h			\
	xsltlocale.h

AUTOMAKE_OPTIONS = subdir-objects
libxsltrr_la_SOURCES = 			\
$(abs_top_srcdir)/libxslt/attrvt.c			\
$(abs_top_srcdir)/libxslt/xslt.c				\
$(abs_top_srcdir)/libxslt/xsltlocale.c			\
$(abs_top_srcdir)/libxslt/xsltutils.c			\
$(abs_top_srcdir)/libxslt/pattern.c			\
$(abs_top_srcdir)/libxslt/templates.c			\
$(abs_top_srcdir)/libxslt/variables.c			\
$(abs_top_srcdir)/libxslt/keys.c				\
$(abs_top_srcdir)/libxslt/numbers.c			\
$(abs_top_srcdir)/libxslt/extensions.c			\
$(abs_top_srcdir)/libxslt/extra.c				\
$(abs_top_srcdir)/libxslt/functions.c			\
$(abs_top_srcdir)/libxslt/namespaces.c			\
$(abs_top_srcdir)/libxslt/imports.c			\
$(abs_top_srcdir)/libxslt/attributes.c			\
$(abs_top_srcdir)/libxslt/documents.c			\
$(abs_top_srcdir)/libxslt/preproc.c			\
$(abs_top_srcdir)/libxslt/transform.c			\
$(abs_top_srcdir)/libxslt/security.c			\
$(abs_top_srcdir)/libxslt/win32config.h			\
$(abs_top_srcdir)/libxslt/xsltwin32config.h		\
$(abs_top_srcdir)/libxslt/xsltwin32config.h.in		\
$(abs_top_srcdir)/libxslt/libxslt.h

if USE_VERSION_SCRIPT
LIBXSLT_VERSION_SCRIPT = $(VERSION_SCRIPT_FLAGS)$(srcdir)/libxslt.syms
else
LIBXSLT_VERSION_SCRIPT =
endif

libxsltrr_la_LIBADD = $(EXTRA_LIBS)
libxsltrr_la_LDFLAGS =					\
		$(WIN32_EXTRA_LDFLAGS)			\
		$(LIBXSLT_VERSION_SCRIPT)		\
		-version-info @LIBXSLT_VERSION_INFO@

man_MANS = libxslt.3

EXTRA_DIST = $(man_MANS) trio.h triodef.h libxslt.syms

xsltproc: all
	@(cd ../xsltproc ; $(MAKE))

install-exec-hook:
	$(mkinstalldirs) "$(DESTDIR)$(libdir)/libxslt-plugins"
