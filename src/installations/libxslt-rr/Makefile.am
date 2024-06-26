SUBDIRS = \
	libxslt \
	libexslt \
	xsltproc \
	doc \
	@PYTHON_SUBDIR@ \
	tests

DIST_SUBDIRS = libxslt libexslt xsltproc python doc tests
ACLOCAL_AMFLAGS = -I m4

confexecdir=$(libdir)
confexec_DATA = xsltConf.sh

bin_SCRIPTS = xslt-config

dist-hook: cleanup libxslt.spec
	(cd $(srcdir) ; tar -cf - --exclude CVS --exclude .svn win32 vms examples) | (cd $(distdir); tar xf -)

CVS_EXTRA_DIST =

EXTRA_DIST = xsltConf.sh.in xslt-config.in libxslt.spec libxslt.spec.in \
             FEATURES TODO Copyright libxslt.m4 \
	     win32/libxslt/libxslt.def win32/libxslt/libxslt.dsw \
	     win32/libxslt/libxslt_so.dsp win32/libxslt/xsltproc.dsp \
	     $(CVS_EXTRA_DIST)

## We create xsltConf.sh here and not from configure because we want
## to get the paths expanded correctly.  Macros like srcdir are given
## the value NONE in configure if the user doesn't specify them (this
## is an autoconf feature, not a bug).

xsltConf.sh: xsltConf.sh.in Makefile
## Use sed and then mv to avoid problems if the user interrupts.
	sed -e 's?\@XSLT_LIBDIR\@?$(XSLT_LIBDIR)?g' \
	    -e 's?\@XSLT_INCLUDEDIR\@?$(XSLT_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@XSLT_LIBS\@?$(XSLT_LIBS)?g' \
	       < $(srcdir)/xsltConf.sh.in > xsltConf.tmp \
	&& mv xsltConf.tmp xsltConf.sh

CLEANFILES = xsltConf.sh

check-local: tests

dummy:

tests: dummy
	@echo '## Running the regression test suite'
	@(cd tests ; $(MAKE) MAKEFLAGS+=--silent tests)
	@(cd xsltproc ; $(MAKE) MAKEFLAGS+=--silent tests)
	@(if [ "@PYTHON_SUBDIR@" != "" ] ; then cd python ; $(MAKE) MAKEFLAGS+=--silent tests ; fi)

valgrind:
	@echo '## Running the regression tests under Valgrind'
	@echo '## Go get a cup of coffee it is gonna take a while ...'
	@(cd tests ; $(MAKE) CHECKER='valgrind -q' tests)
	@(cd xsltproc ; $(MAKE) CHECKER='valgrind -q' tests)

cleanup:
	-@(find . -name .\#\* -exec rm {} \;)

cleantar:
	@(rm -f libxslt*.tar.gz)

rpm: cleantar
	@(unset CDPATH ; $(MAKE) dist && rpmbuild -ta $(distdir).tar.gz)


pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA = libxslt.pc libexslt.pc

m4datadir = $(datadir)/aclocal
m4data_DATA = libxslt.m4

