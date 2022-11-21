#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([generalserver], [1.0], [annesley_newholm@yahoo.it])
AC_CONFIG_SRCDIR([config.h.in]) # random file just to check this is the correct src
AM_INIT_AUTOMAKE
AC_ARG_WITH(database-trigger, [  --without-database-trigger            programmable database changes during read/write operations], [AC_DEFINE(WITHOUT_DATABASE_TRIGGER,1,0)
AC_SUBST(WITHOUT_DATABASE_TRIGGER,1)])
AC_ARG_WITH(session-xpath, [  --without-session-xpath            stateful storage of node-sets against session], [AC_DEFINE(WITHOUT_SESSION_XPATH,1,0)
AC_SUBST(WITHOUT_SESSION_XPATH,1)])
AC_ARG_WITH(xsl-extensions, [  --without-xsl-extensions            XSL language datavase extensions], [AC_DEFINE(WITHOUT_XSL_EXTENSIONS,1,0)
AC_SUBST(WITHOUT_XSL_EXTENSIONS,1)])
AC_ARG_WITH(threading, [  --without-threading            disable multi-thread], [AC_DEFINE(WITHOUT_THREADING,1,0)
AC_SUBST(WITHOUT_THREADING,1)])
AC_ARG_WITH(locking, [  --without-locking            disable native library node level locks], [AC_DEFINE(WITHOUT_LOCKING,1,0)
AC_SUBST(WITHOUT_LOCKING,1)])
AC_ARG_WITH(database-namespaceroot, [  --with-database-namespaceroot            move all xmlns:* to the root element always], [AC_DEFINE(WITH_DATABASE_NAMESPACEROOT,1,0)
AC_SUBST(WITH_DATABASE_NAMESPACEROOT,1)])
AC_ARG_WITH(database-idmode, [  --with-database-idmode            ensure @xml:id on every element], [AC_DEFINE(WITH_DATABASE_IDMODE,1,0)
AC_SUBST(WITH_DATABASE_IDMODE,1)])
AC_ARG_WITH(database-readonly, [  --with-database-readonly            prevent permanent changes], [AC_DEFINE(WITH_DATABASE_READONLY,1,0)
AC_SUBST(WITH_DATABASE_READONLY,1)])
AC_ARG_WITH(mm, [  --with-mm            general server memory management], [AC_DEFINE(WITH_MM,1,0)
AC_SUBST(WITH_MM,1)])
AC_ARG_WITH(logdir, [  --with-logdir            dir where logs reside (/var/log/generalserver/)], [AC_DEFINE(WITH_LOGDIR,1,0)
AC_SUBST(WITH_LOGDIR,1)])
AC_ARG_WITH(document-root, [  --with-document-root            dir for the writeable base DAG configuration data (/var/www/generalserver/)], [AC_DEFINE(WITH_DOCUMENT_ROOT,1,0)
AC_SUBST(WITH_DOCUMENT_ROOT,1)])
AC_ARG_WITH(debug-exceptions, [  --with-debug-exceptions            debug exceptions flow], [AC_DEFINE(WITH_DEBUG_EXCEPTIONS,1,0)
AC_SUBST(WITH_DEBUG_EXCEPTIONS,1)])
AC_ARG_WITH(debug-rx, [  --with-debug-rx            debug rxsl processing], [AC_DEFINE(WITH_DEBUG_RX,1,0)
AC_SUBST(WITH_DEBUG_RX,1)])
AC_ARG_WITH(debug, [  --with-debug            add the debugging module], [AC_DEFINE(WITH_DEBUG,1,0)
AC_SUBST(WITH_DEBUG,1)
CXXFLAGS="-O0 -ggdb"])
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_LIBTOOL
AC_CONFIG_HEADERS([config.h])
AH_TEMPLATE(DATABASE_MASK_CONTEXT)
AC_DEFINE(DATABASE_MASK_CONTEXT,MaskContext)
AH_TEMPLATE(DATABASE_TRIGGER_CONTEXT)
AC_DEFINE(DATABASE_TRIGGER_CONTEXT,TriggerContextDatabaseAttribute,"database:on-before-read")
AH_TEMPLATE(SERVER_MAIN_XML_LIBRARY_PREFIX)
AC_DEFINE(SERVER_MAIN_XML_LIBRARY_PREFIX,Lib,"prefix for all *Xml* includes and classes")
AH_TEMPLATE(VERSION_TYPE)
AC_DEFINE(VERSION_TYPE,"Alpha","first-go!")
AH_TEMPLATE(VERSION_BUILD)
AC_DEFINE(VERSION_BUILD,"1")
AH_TEMPLATE(VERSION_MINOR)
AC_DEFINE(VERSION_MINOR,"0")
AH_TEMPLATE(VERSION_MAJOR)
AC_DEFINE(VERSION_MAJOR,"1")
AH_TEMPLATE(_EXECUTABLE)
AC_DEFINE(_EXECUTABLE,"generalserver")

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
# AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([z], [main])
AC_CHECK_LIB([uuid], [main])
AC_CHECK_LIB([rt], [main])
AC_CHECK_LIB([history], [main])
AC_CHECK_LIB([readline], [main])
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([pq], [main])
AC_CHECK_LIB([m], [main])
AC_CHECK_LIB([gcrypt], [main])
AC_CHECK_LIB([dl], [main])
# AX_PTHREAD() rendered obsolete by libtool

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netdb.h netinet/in.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([btowc clock_gettime memmove memset mkdir realpath rmdir select setlocale socket strcasecmp strchr strerror strndup strrchr strstr strtol])

AC_CONFIG_FILES([LibXml/Makefile
                 Makefile
                 server/Makefile
                 server/QueryEnvironment/Makefile
                 server/Utilities/Makefile
                 server/Xml/Makefile])
# AC_CONFIG_SUBDIRS
AC_SUBST(CPPFLAGS,"-pedantic -Wno-ignored-qualifiers -W -Wformat -Wswitch -Wcomment -Wtrigraphs -Wformat -Wchar-subscripts -Wuninitialized -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Winline -Wredundant-decls")
AC_SUBST(LIBTOOL_DEPS)
AC_SUBST(AR_FLAGS,"crD")
AC_CONFIG_COMMANDS_POST([find -name *ABS_DIR* -exec rm -r {} +])
AC_OUTPUT
