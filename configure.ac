#					       -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT($_PACKAGE, $_VERSION, $_BUGREPORT)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.
AM_PATH_GTK_2_0(2.8.0,,AC_MSG_ERROR([You need Gtk+ >= 2.8.0 to build $_PACKAGE]))
AM_PATH_GLIB_2_0(2.12.0,,AC_MSG_ERROR([You need Glib >= 2.12.0 to build $_PACKAGE]))
PKG_CHECK_MODULES(vte, [vte >= 0.12.0],, AC_MSG_ERROR([You need libvte >= 0.12.0 to build $_PACKAGE]))

AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [enable debug mode]),
	      CFLAGS="$CFLAGS -g"
	      AC_DEFINE(DEBUG,, [enable debug mode]))

AC_ARG_ENABLE(safe-mode, AC_HELP_STRING([--disable-safe-mode], [disable run in safe mode]),
	      CFLAGS="$CFLAGS -g"
	      AC_DEFINE(REALMODE,, [disable run in safe mode]))

PKG_CHECK_EXISTS(gtk+-2.0 < 2.12,
		 AC_MSG_WARN([Using Gtk+ >= 2.12.0 to build $_PACKAGE is recommended]))

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

AC_DEFINE([BINARY], ["$_BINARY"], "The binary file name of LilyTerm.")
AC_DEFINE([ISSUES], ["$_ISSUES"], "The web site for reporting issues.")
AC_DEFINE([YEAR], ["$_YEAR"], "The current year.")
AC_DEFINE([AUTHOR], ["$_AUTHOR"], "The author's name.")
AC_DEFINE([MAINSITE], ["$_MAINSITE"], "The main site.")
AC_DEFINE([GITHUBURL], ["$_GITHUBURL"], "The github site.")
AC_DEFINE([BLOG], ["$_BLOG"], "The blog site.")
AC_DEFINE([WIKI], ["$_WIKI"], "The wiki site.")
AC_DEFINE([IRC], ["$_IRC"], "The irc channel.")
AC_DEFINE([BUGREPORT], ["$_BUGREPORT"], "The author's email address.")

AC_PROG_INTLTOOL
ALL_LINGUAS="$_LANG_LIST"
GETTEXT_PACKAGE="$_BINARY"
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
