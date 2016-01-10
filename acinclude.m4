dnl AC_CXX_IN_STD(IDENTIFIER, HEADER, MACRO)
dnl   tests the C++ for the presence of IDENTIFIER in namespace std.
dnl   If IDENTIFIER is _not_ in namespace std, MACRO is #defined to
dnl   'namespace std {using ::IDENTIFIER;}', otherwise empty.
dnl
dnl   This should be used in configure.in
dnl     e.g. AC_CXX_IN_STD([string],[string],GTKMM_USING_STD_STRING)
dnl
AC_DEFUN([AC_CXX_IN_STD],
[
  AC_CACHE_CHECK([if C++ environment has $1 in std],[gtkmm_cv_cxx_$1_in_std],
  [
    AC_TRY_COMPILE(
      [#include <]ifelse($2,,$1,$2)[>],
      [using std::$1;],
      [gtkmm_cv_cxx_$1_in_std="yes"],
      [gtkmm_cv_cxx_$1_in_std="no"]
    )
  ])
  if test "x${gtkmm_cv_cxx_$1_in_std}" = "xyes"; then
    AC_DEFINE([$3],1, [Define if std::$1 exists])
  fi
])

AC_DEFUN([GABBER_GET_PLATFORM],
[
    GABBER_PLATFORM=linux
    AC_ARG_WITH(gabber-platform, 
        AC_HELP_STRING([--with-gabber-platform],
            [The platform to build for, defaults to linux.]),
        [GABBER_PLATFORM=$withval],[GABBER_PLATFORM=linux])

    case $GABBER_PLATFORM in
        linux)
            ;;
        osx)
            PLATFORM_LIBS="-framework Carbon -framework ApplicationServices ${PLATFORM_LIBS}"
            ;;
        win32)
            ;;
    esac

    AC_SUBST(GABBER_PLATFORM)
    AC_SUBST(PLATFORM_CFLAGS)
    AC_SUBST(PLATFORM_LIBS)

    AC_CONFIG_FILES([src/linux/Makefile \
                     src/osx/Makefile \
                     src/win32/Makefile])
])
