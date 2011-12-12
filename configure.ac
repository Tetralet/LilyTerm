#					       -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(LilyTerm, 0.9.9, tetralet@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.
AM_PATH_GTK_2_0(2.8.0,,AC_MSG_ERROR([You need Gtk+ >= 2.8.0 to build $PACKAGE]))
AM_PATH_GLIB_2_0(2.12.0,,AC_MSG_ERROR([You need Glib >= 2.12.0 to build $PACKAGE]))
PKG_CHECK_MODULES(vte, [vte >= 0.12.0],, AC_MSG_ERROR([You need libvte >= 0.12.0 to build $PACKAGE]))

AC_DEFINE(DEFENSIVE,, [Enable defensive programing])
AC_DEFINE(FATAL,, [Enable fatal error messages])

AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [turn on debug]),
	      CFLAGS="$CFLAGS -g"
	      AC_DEFINE(DEBUG,, [turn on debug]))

PKG_CHECK_EXISTS(gtk+-2.0 < 2.12,
		 AC_MSG_WARN([Using Gtk+ >= 2.12.0 to build $PACKAGE is recommend]))

AC_MSG_CHECKING([whether to install GNOME control-center default application definition])
if $PKG_CONFIG --variable=keysdir gnome-default-applications >/dev/null ; then
     AC_MSG_RESULT([yes])
     DEFAULTAPPS_DIR="`$PKG_CONFIG --variable=defappsdir gnome-default-applications`"
     AC_SUBST(DEFAULTAPPS_DIR)
else
     AC_MSG_RESULT([no])
     DEFAULTAPPS_DIR=""
fi
AM_CONDITIONAL([DEFAULT_APPS_DEFINITION],[test -n "$DEFAULTAPPS_DIR"])

AC_PROG_INTLTOOL
ALL_LINGUAS="de es fr it nl pl ru sk tr uk zh_CN zh_TW"
GETTEXT_PACKAGE="$PACKAGE"
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(LOCALEDIR)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([locale.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T

# Checks for library functions.
AC_CHECK_FUNCS([setlocale])

AC_CONFIG_FILES([
Makefile
src/Makefile
po/Makefile.in
data/Makefile
])
AC_OUTPUT
