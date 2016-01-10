#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="gabber"

(test -f $srcdir/configure.ac \
  && test -f $srcdir/src/main.cc) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}


which gnome-autogen.sh || {
    echo "You need to install gnome-common from GNOME CVS"
    exit 1
}
XGETTEXT_KEYWORDS='--keyword=_ --keyword=N_ --keyword=translate --qt' \
USE_GNOME2_MACROS=1 \
REQUIRED_AUTOMAKE_VERSION=1.6 \
. gnome-autogen.sh
