#!/bin/sh

CONF_FILE='.default'
AC_FILE='configure.ac'

Replace_Parameter()
{
  # Find the parameter from CONF_FILE
  Parameter=`grep "^$* = " $CONF_FILE | sed -e "s|$* = ||g" | tr -d '\\\\'`
  if [ $? != 0 ]; then
    echo "Can Not find the string for "$*" from $CONF_FILE. exit..." >&2
    exit 1
  fi
  if [ -n "`echo $Parameter | grep '^\$(shell .*)$'`" ]; then
    Parameter=`$(echo $Parameter | sed -e 's/^\$(shell \(.*\))$/\1/g')`
  fi

  echo "$AC_FILE: Replacing variable '$*' with '$Parameter'..." >&2

  # Replace the parameter in AC_FILE
  sed -i "s|\$_$*|$Parameter|g" $AC_FILE
  if [ $? != 0 ]; then
    echo "Something goes wrong when replace '$*' with '$Parameter' for $AC_FILE. exit..." >&2
    exit 1
  fi
}

echo ""
Replace_Parameter BINARY
Replace_Parameter PACKAGE
Replace_Parameter VERSION
Replace_Parameter BUGREPORT
Replace_Parameter ISSUES
Replace_Parameter YEAR
Replace_Parameter AUTHOR
Replace_Parameter MAINSITE
Replace_Parameter GITHUBURL
Replace_Parameter BLOG
Replace_Parameter WIKI
Replace_Parameter IRC
Replace_Parameter LANG_LIST

AC_FILE='src/Makefile.am'
Replace_Parameter BINARY
echo ""

# for GTK3+
pkg-config --cflags gtk+-2.0 > /dev/null 2>&1
if [ $? != 0 ]; then
  sed -i 's/^AM_PATH_GTK_2_0.*/PKG_CHECK_MODULES([GTK], [gtk+-3.0])/g' configure.ac
  pkg-config --cflags vte-2.91 > /dev/null 2>&1
  if [ $? = 0 ]; then
    sed -i 's/^PKG_CHECK_MODULES(vte, \[vte >= .*/PKG_CHECK_MODULES(vte, [vte-2.91 >= 0.38.0],, AC_MSG_ERROR([You need libvte-2.91 >= 0.38.0 to build $_PACKAGE]))/g' configure.ac
    sed -i 's/^lilyterm_LDADD\(.*\)$/lilyterm_LDADD\1 -lX11/g' src/Makefile.am
  else
    sed -i 's/^PKG_CHECK_MODULES(vte, \[vte >= .*/PKG_CHECK_MODULES(vte, [vte-2.90 >= 0.30.0],, AC_MSG_ERROR([You need libvte-2.90 >= 0.30.0 to build $_PACKAGE]))/g' configure.ac
  fi
fi

set -x

aclocal
autoheader
intltoolize --automake --copy --force
automake --add-missing --copy
autoconf

