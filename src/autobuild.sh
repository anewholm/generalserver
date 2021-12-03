#!/bin/sh

# --------------------------------------- why?
# we want to enable autoscan to determine everything in the long run.
# it should be configurable but we can't figure it out

# --------------------------------------- functions
rmif() {
  if [ -f $1 ]; then
    rm $1
    echo removed $1
  fi
}

replace() {
  perl -0pi -w -e "s/$1/$2/gm;" configure.scan
}
replace_literal() {
  replace "\[$1\]" "[$2]"
}

remove_clause() {
  replace "^($1\([^)]*\))" "# $1"
  replace "^$1" "# $1"
}

AC_CHECK_LIB() {
  replace "#\s*(Checks for libraries.*)" "# \1\nAC_CHECK_LIB([$1], [main])"
}

AC_SUBST() {
  if [ -z "$2" ]; then
    replace AC_OUTPUT "AC_SUBST($1)\nAC_OUTPUT"
  else
    replace AC_OUTPUT "AC_SUBST($1,\"$2\")\nAC_OUTPUT"
  fi
}

AC_CONFIG_COMMANDS() {
  replace AC_OUTPUT "AC_CONFIG_COMMANDS_POST([$1])\nAC_OUTPUT"
}

AC_ARG_WITH() {
  SPACES_PAD="                        "
  WITH=with
  if [ $3 -eq "1" ]; then
    WITH=without
  fi
  MACRONAME_UPPER=$(echo "${WITH}_$1" | tr '[:lower:]-' '[:upper:]_')
  MACRONAME_LEN=${#MACRONAME_UPPER}
  #MACRO_PAD=${SPACES_PAD:0:$MACRONAME_LEN}
  if [ -z "$2" ]; then
    replace AM_INIT_AUTOMAKE "AM_INIT_AUTOMAKE\nAC_ARG_WITH($1, [  --$WITH-$1            $1])"
  elif [ -z "$3" ]; then
    replace AM_INIT_AUTOMAKE "AM_INIT_AUTOMAKE\nAC_ARG_WITH($1, [  --$WITH-$1            $2])"
  elif [ -z "$4" ]; then
    replace AM_INIT_AUTOMAKE "AM_INIT_AUTOMAKE\nAC_ARG_WITH($1, [  --$WITH-$1            $2], [AC_DEFINE($MACRONAME_UPPER,1,0)\nAC_SUBST($MACRONAME_UPPER,1)])"
  else
    replace AM_INIT_AUTOMAKE "AM_INIT_AUTOMAKE\nAC_ARG_WITH($1, [  --$WITH-$1            $2], [AC_DEFINE($MACRONAME_UPPER,1,0)\nAC_SUBST($MACRONAME_UPPER,1)\n$4])"
  fi
}
AC_DEFINE_STR() {
  AC_DEFINE $1 \"$2\" $3
}

AC_DEFINE() {
  if [ -z "$2" ]; then
    replace "^AC_CONFIG_HEADERS(.*)" "AC_CONFIG_HEADERS\1\nAH_TEMPLATE($1)\nAC_DEFINE($1)"
  elif [ -z "$3" ]; then
    replace "^AC_CONFIG_HEADERS(.*)" "AC_CONFIG_HEADERS\1\nAH_TEMPLATE($1)\nAC_DEFINE($1,$2)"
  else
    replace "^AC_CONFIG_HEADERS(.*)" "AC_CONFIG_HEADERS\1\nAH_TEMPLATE($1)\nAC_DEFINE($1,$2,\"$3\")"
  fi
}

# --------------------------------------- clean up
rmif configure.scan
rmif configure.ac
rmif config.status
rmif config.h
# do not want to ZAP installations/libx* Makefile.in or object files also...
# if find | grep -q Makefile.in; then
#   find | grep Makefile.in | xargs rm
# fi
# if find | grep -q "\.o"; then
#   find | grep "\.o" | xargs rm
# fi

# --------------------------------------- create and setup configure.scan
autoscan
replace_literal FULL-PACKAGE-NAME generalserver
replace_literal VERSION 1.0
replace_literal BUG-REPORT-ADDRESS "annesley_newholm\@yahoo.it"

# --------------------------------------- setup configure
# remove the discovered installations configure.ac list
remove_clause AC_CONFIG_SUBDIRS
if ! grep -q AC_CONFIG_HEADERS configure.scan
then
  perl -pi -w -e "s/^AC_OUTPUT/AC_CONFIG_HEADERS([config.h])\nAC_OUTPUT/g;" configure.scan
fi
# add AM_INIT_AUTOMAKE
replace "^AC_CONFIG_SRCDIR\(([^)]*)\)" "AC_CONFIG_SRCDIR(\1) # random file just to check this is the correct src\nAM_INIT_AUTOMAKE"
# after AM_INIT_AUTOMAKE
# perl -pi -w -e "s/^AM_INIT_AUTOMAKE/AM_INIT_AUTOMAKE\nAC_PROG_RANLIB/g;" configure.scan
remove_clause AC_PROG_RANLIB "rendered obsolete by libtool LT_INIT"
# https://ftp.gnu.org/old-gnu/Manuals/libtool-1.4.2/html_node/libtool_27.html
perl -pi -w -e "s/^AM_INIT_AUTOMAKE/AM_INIT_AUTOMAKE\nAC_PROG_LIBTOOL/g;" configure.scan
perl -pi -w -e "s/^AM_INIT_AUTOMAKE/AM_INIT_AUTOMAKE\nAC_CONFIG_MACRO_DIRS([m4])/g;" configure.scan
# -Wunused 
# -Wreturn-type troubles with LibXml2 types
AC_SUBST CPPFLAGS "-pedantic -Wno-ignored-qualifiers -W -Wformat -Wswitch -Wcomment -Wtrigraphs -Wformat -Wchar-subscripts -Wuninitialized -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Winline -Wredundant-decls"
AC_SUBST LIBTOOL_DEPS
AC_SUBST AR_FLAGS "crD" # ar: `u' modifier ignored since `D' is the default (see `U')
# manual dependencies
# https://www.gnu.org/software/autoconf-archive/ax_pthread.html#ax_pthread
# HAVE_PTHREAD
# Note that AX_PTHREAD does not take parameters
# pthread check is carried out by libtool now
perl -pi -w -e "s/#\s*(Checks for libraries.*)/# \1\n# AX_PTHREAD() rendered obsolete by libtool/g;" configure.scan
AC_CHECK_LIB dl
AC_CHECK_LIB gcrypt
AC_CHECK_LIB m
AC_CHECK_LIB pq
AC_CHECK_LIB pthread
AC_CHECK_LIB readline
AC_CHECK_LIB history
AC_CHECK_LIB rt
AC_CHECK_LIB uuid
AC_CHECK_LIB z

# --------------------------------------- configure arguments
AC_ARG_WITH debug "add the debugging module" 0 "CXXFLAGS=\"-O0 -ggdb\""
AC_ARG_WITH debug-rx "debug rxsl processing" 0
AC_ARG_WITH debug-exceptions "debug exceptions flow" 0
AC_ARG_WITH document-root "dir for the writeable base DAG configuration data (\/var\/www\/generalserver\/)" 0
AC_ARG_WITH logdir "dir where logs reside (\/var\/log\/generalserver\/)" 0
AC_ARG_WITH mm "general server memory management" 0
AC_ARG_WITH database-readonly "prevent permanent changes" 0
AC_ARG_WITH database-idmode "ensure \@xml:id on every element" 0
AC_ARG_WITH database-namespaceroot "move all xmlns:* to the root element always" 0
AC_ARG_WITH locking "disable native library node level locks" 1
AC_ARG_WITH threading "disable multi-thread" 1
AC_ARG_WITH xsl-extensions "XSL language datavase extensions" 1
AC_ARG_WITH session-xpath "stateful storage of node-sets against session" 1
AC_ARG_WITH database-trigger "programmable database changes during read\/write operations" 1
#AC_ARG_WITH last "writes xml/xsl to file during server side XSLT" "\/var\/www\/general-resources-server\/last" 

AC_DEFINE_STR _EXECUTABLE generalserver   # has _ for compatability with windows
AC_DEFINE_STR VERSION_MAJOR 1
AC_DEFINE_STR VERSION_MINOR 0
AC_DEFINE_STR VERSION_BUILD 1
AC_DEFINE_STR VERSION_TYPE "Alpha" "first-go!"
AC_DEFINE SERVER_MAIN_XML_LIBRARY_PREFIX Lib "prefix for all *Xml* includes and classes"
AC_DEFINE DATABASE_TRIGGER_CONTEXT TriggerContextDatabaseAttribute "database:on-before-read"
AC_DEFINE DATABASE_MASK_CONTEXT    MaskContext

# ./configure will create $(abs_top_srcdir) directories for the .deps
AC_CONFIG_COMMANDS "find -name *ABS_DIR* -exec rm -r {} +"

# --------------------------------------- create configure and Makefile.in s
# http://amjith.blogspot.hu/2009/04/autoconf-and-automake-tutorial.html
mv configure.scan configure.ac
touch config.h.in
autoheader
autoconf
libtoolize
aclocal
automake --add-missing
autoconf
