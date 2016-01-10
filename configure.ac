## $Id: configure.ac,v 1.10 2004/06/30 18:01:05 julian Exp $
##
## Copyright (c) 2004 Julian Missig <julian@jabber.org>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License VERSION 2 as
## published by the Free Software Foundation.  You are not allowed to
## use any other version of the license; unless you got the explicit
## permission from the author to do so.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

AC_INIT([Gabber], [1.9.4.3], [http://www.jabberstudio.org/projects/gabber/bugs/])

AC_PREREQ([2.53])

AC_REVISION([$Revision: 1.10 $])
AC_CONFIG_SRCDIR([src/main.cc])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER([config.h])
## ^ AC_CONFIG_HEADERS instead? seems to be an automake-1.7 thing

## AC_SUBST([ACLOCAL_AMFLAGS], ['-I macros ${ACLOCAL_FLAGS}'])

AC_PROG_CC()
AC_PROG_CXX()
AC_PROG_INSTALL()
AC_PROG_LIBTOOL()
AM_PROG_LIBTOOL()

## i18n
AC_PROG_INTLTOOL([0.23])
ALL_LINGUAS=`cat "$srcdir/po/LINGUAS"`
GETTEXT_PACKAGE=Gabber
AC_SUBST([CONFIG_STATUS_DEPENDENCIES], ['$(top_srcdir)/po/LINGUAS'])
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], "$GETTEXT_PACKAGE", "Gettext package name")
AM_GLIB_GNU_GETTEXT()

## X and its path
AC_PATH_XTRA

## Check modules
PKG_CHECK_MODULES([DEPS], gtkmm-2.4 gconfmm-2.6 libglademm-2.4, [], 
                  [PKG_CHECK_MODULES([DEPS], 
                    gtkmm-2.0 gconfmm-2.0 libglademm-2.0, [
                    AC_DEFINE([OLD_GTKMM],,"Old gtkmm")
                    AC_DEFINE([OLD_SIGC],,"Old sigc++")
                    ])
                  ])
AC_SUBST([DEPS_CFLAGS])
AC_SUBST([DEPS_LIBS])


AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
if test x"$GCONFTOOL" = xno; then
  AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
fi

AM_GCONF_SOURCE_2()



AC_ARG_WITH(static-jabberoo,[  --with-static-jabberoo    compile jabberoo into gabber statically], [
    JABBEROO_CFLAGS=`pkg-config  --cflags jabberoo`
    JABBEROO_LIBS="`pkg-config --libs-only-L jabberoo | tr -d '\-L '`/libjabberoo.a"
    ], [PKG_CHECK_MODULES([JABBEROO], [jabberoo >= 1.9.4])])
AC_SUBST([JABBEROO_CFLAGS])
AC_SUBST([JABBEROO_LIBS])


use_ssl="yes"
AC_MSG_CHECKING([SSL Support])
AC_ARG_ENABLE(ssl, AC_HELP_STRING(--disable-ssl,
              [Disable OpenSSL support]),[use_ssl="no"])
AC_MSG_RESULT($use_ssl)
AM_CONDITIONAL(WITH_SSL, test "x$use_ssl" = "xyes")
if test "x$use_ssl" = "xyes"; then
    AC_DEFINE(WITH_SSL,,"Use SSL Support")
fi
SSL_CFLAGS="-DOPENSSL_NO_KRB5"
SSL_LIBS="-lssl -lcrypto"
AC_ARG_WITH(ssl-includes, AC_HELP_STRING([--with-ssl-includes],
            [Path to the ssl headers.]),[SSL_CFLAGS="-I$withval ${SSL_CFLAGS}"])
AC_ARG_WITH(ssl-libs, AC_HELP_STRING([--with-ssl-libs],
            [Path to the ssl libraries.]),[SSL_LIBS="-L$withval ${SSL_LIBS}"])
AC_SUBST(SSL_CFLAGS)
AC_SUBST(SSL_LIBS)


use_gtkspell="yes"
AC_MSG_CHECKING([GtkSpell])
AC_ARG_ENABLE(gtkspell, AC_HELP_STRING(--disable-gtkspell,
              [Disable GtkSpell spell checking]),[use_gtkspell="no"])
AC_MSG_RESULT($use_gtkspell)
AM_CONDITIONAL(GTKSPELL, test "x$use_gtkspell" = "xyes")
if test "x$use_gtkspell" = "xyes"; then
    AC_DEFINE(GTKSPELL,,"Enable GtkSpell compilation")
AC_CHECK_HEADER(aspell.h, [AC_DEFINE(HAVE_ASPELL_H,1,
   [Define to 1 if you have the <aspell.h> header file.] )] )
AC_CHECK_HEADER(pspell/pspell.h, [AC_DEFINE(HAVE_PSPELL_H,1,
   [Define to 1 if you have the <pspell/pspell.h> header file.] )] )
AC_CHECK_LIB(aspell, new_aspell_speller, SPELLER_LIB="-laspell",
   [AC_CHECK_LIB(pspell, new_pspell_manager, SPELLER_LIB="-lpspell",
      [AC_MSG_ERROR([You must have the aspell or pspell dev libraries to build gtkspell.]) ] ) ] )

AC_SUBST(SPELLER_LIB)
fi

dnl CFLAGS for release and devel versions
CFLAGS="$CFLAGS -Wall"
CXXFLAGS="$CXXFLAGS -Wall"

debug="no"
AC_ARG_ENABLE(debug,
        [  --enable-debug          compile with debugging (for development)],
        debug=$enable_debug
)
AC_MSG_CHECKING(for debugging)
if test "x$debug" = xyes; then
        AC_MSG_RESULT(yes)
        BUILD_TYPE="$BUILD_TYPE Debug"
        CFLAGS="$CFLAGS -g"
	CXXFLAGS="$CXXFLAGS -g -DSESSION_DEBUG"
else
	CFLAGS="$CFLAGS -O2"
	CXXFLAGS="$CXXFLAGS -O2"
        AC_MSG_RESULT(no)
fi

dnl **************************************************
dnl * C++ STL checks
dnl **************************************************
AC_LANG_CPLUSPLUS

AC_CXX_IN_STD([ostringstream],[sstream],HAVE_STD_SSTREAM)

dnl *************************************************
dnl * Check if we have gethostbyname_r
dnl *************************************************

dnl we need this, otherwise AC_CHECK_FUNC doesn't work properly
AC_LANG_C
AC_CHECK_FUNC(gethostbyname_r,
  [

  dnl  First check for the glibc variant of gethostbyname_r

  AC_MSG_CHECKING(for glibc gethostbyname_r)
  AC_TRY_LINK([	#include <netdb.h>],[
	  struct hostent result_buf;
	  char buf[1024];
	  struct hostent* result;
	  int h_errnop;

	  gethostbyname_r("localhost", &result_buf, buf, sizeof(buf),
	                  &result, &h_errnop);
	], [

	  dnl Have glibc gethostbyname_r

	  AC_MSG_RESULT(yes)
	  AC_DEFINE(HAVE_GETHOSTBYNAME_R_GLIBC, 1, [Define if gethostbyname_r is in glibc])
	  HAVE_GETHOSTBYNAME_R=yes

        ], [

  dnl  If we don't have glibc gethostbyname_r, check
  dnl  for Solaris/Irix gethostbyname_r

  AC_MSG_RESULT(no)
  AC_MSG_CHECKING(for Solaris/Irix gethostbyname_r)
  AC_TRY_LINK([ #include <netdb.h>],[
	  struct hostent result;
	  char buf[1024];
	  int h_errnop;

	  gethostbyname_r("localhost", &result, buf, sizeof(buf), &h_errnop);

	], [

	  dnl Have Solaris/Irix gethostbyname_r

  	  AC_MSG_RESULT(yes)
	  AC_DEFINE(HAVE_GETHOSTBYNAME_R_SOLARIS, 1, [Define for the Solaris version of gethostbyname_r])
	  HAVE_GETHOSTBYNAME_R=yes

	], [
  dnl  If don't have Solaris/Irix gethostbyname_r, check
  dnl  for HP-UX gethostbyname_r

  AC_MSG_RESULT(no)
  AC_MSG_CHECKING(for HP-UX gethostbyname_r)
  AC_TRY_LINK([ #include <netdb.h>],[
	  struct hostent result;
	  char buf[1024];
          gethostbyname_r("localhost", &result, buf);
        ], [
        
          dnl Have HP-UX gethostbyname_r

          AC_MSG_RESULT(yes)
	  AC_DEFINE(HAVE_GETHOSTBYNAME_R_HPUX, 1, [Define for the HP-UX version of gethostbyname_r])
	  HAVE_GETHOSTBYNAME_R=yes

	]
     )]
  )]
)])


dnl If we don't have gethostbyname_r, try to use Glib mutexes
if test -z "$HAVE_GETHOSTBYNAME_R"; then

  AM_PATH_GLIB_2_0(2.0.0, AC_DEFINE(HAVE_GETHOSTBYNAME_R_GLIB_MUTEX, 1, [Define for glib threads]), [
      AC_MSG_WARN(You have neither Glib threads nor the function gethostbyname_r.
This means that calls to gethostbyname (used for address resolution) will
not be thread safe so could malfunction in programs that use threads.)
    ])
fi


dnl ******************************
dnl Check for socklen_t
dnl ******************************
AC_LANG_C
AC_MSG_CHECKING(for socklen_t)
  AC_TRY_COMPILE([
 	#include <sys/types.h>
 	#include <sys/socket.h>
	socklen_t somesocket;
	],[
	],[
  	  AC_MSG_RESULT(yes)
	  AC_DEFINE(HAVE_SOCKLEN_T, 1, [Define if type socklen_t exists])
	], [
	  AC_TRY_COMPILE([
		#include <sys/types.h>
		#include <sys/socket.h>
		int accept(int, struct sockaddr*, size_t*);
	  ],[
	  ],[
	    AC_MSG_RESULT(size_t)
	    AC_DEFINE(USE_SOCKLEN_T_SIZE_T, 1, [Define if size_t should be used for socklen_t])
	  ],[
	    AC_MSG_RESULT(int)
	    AC_DEFINE(USE_SOCKLEN_T_INT, 1, [Define if int should be used for socklen_t])
	  ])
	]
  )

dnl ******************************
dnl Check for X ScreenSaver Extension
dnl ******************************

AC_ARG_ENABLE(xss,   [  --disable-xss   compile without X screensaver extension],enable_xss=no,enable_xss=yes)
dnl Check for XScreenSaver                                                                                                         
if test "x$enable_xss" = "xyes" ; then
        XSS_LIBS="no"
        XSS_HEADERS="no"
        AC_CHECK_LIB(Xext, XScreenSaverRegister,[XSS_LIBS="$X_LIBS"],[],[$X_LIBS -lX11 -lXext -lm])
        AC_CHECK_LIB(Xss, XScreenSaverRegister,[XSS_LIBS="$X_LIBS -lXss -lX11 -lXext -lm"],[],[$X_LIBS -lX11 -lXext -lm])
        if test \! "$XSS_LIBS" = "no"; then
                AC_TRY_COMPILE([
#include <X11/extensions/scrnsaver.h>                                                                                              
                                ],[],[enable_xss=no],[
                                AC_DEFINE(HAVE_XSS, 1, [Define if libXss for screensaver extensions is usable])
                                ])
        else
                XSS_LIBS=""
                enable_xss=no
        fi
else
        XSS_LIBS=""
        enable_xss=no
fi
AC_SUBST(XSS_LIBS)


GABBER_GET_PLATFORM

dnl Create makefiles
AC_CONFIG_FILES([
	Makefile
	doxygen.cfg
     gabber.schemas
     gabber.desktop
	src/Makefile
     src/modules/Makefile
     pixmaps/Makefile
     po/Makefile.in
	ui/Makefile
])

AC_OUTPUT

echo "
Configuration:
  Install path:                   ${prefix}
  Debugging symbols:              $debug
  Enable OpenSSL support:         $use_ssl
  GtkSpell spell checking:        $use_gtkspell
  XScreenSaver Extension:         $enable_xss
  Gabber Platform:                $GABBER_PLATFORM"
