#! /bin/sh
set -x
aclocal
autoheader
intltoolize --automake --copy --force
automake --add-missing --copy
autoconf
