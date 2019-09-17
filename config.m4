dnl config.m4 for extension xz

dnl Check PHP version:
AC_MSG_CHECKING(PHP version)
if test ! -z "$phpincludedir"; then
    PHP_VERSION=`grep 'PHP_VERSION ' $phpincludedir/main/php_version.h | sed -e 's/.*"\([[0-9\.]]*\)".*/\1/g' 2>/dev/null`
elif test ! -z "$PHP_CONFIG"; then
    PHP_VERSION=`$PHP_CONFIG --version 2>/dev/null`
fi

if test x"$PHP_VERSION" = "x"; then
    AC_MSG_WARN([none])
else
    PHP_MAJOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/g' 2>/dev/null`
    PHP_MINOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/g' 2>/dev/null`
    PHP_RELEASE_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/g' 2>/dev/null`
    AC_MSG_RESULT([$PHP_VERSION])
fi

if test $PHP_MAJOR_VERSION -lt 5; then
    AC_MSG_ERROR([need at least PHP 5 or newer])
fi

PHP_ARG_ENABLE(xz, whether to enable xz support,
[  --enable-xz           Enable xz support])

if test "$PHP_XZ" != "no"; then

  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

  AC_MSG_CHECKING(for liblzma)
  if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists liblzma; then
    if $PKG_CONFIG liblzma --atleast-version 5; then
      LIBLZMA_CFLAGS=`$PKG_CONFIG liblzma --cflags`
      LIBLZMA_LIBDIR=`$PKG_CONFIG liblzma --libs`
      LIBLZMA_VERSON=`$PKG_CONFIG liblzma --modversion`
      AC_MSG_RESULT(from pkgconfig: version $LIBLZMA_VERSON)
    else
      AC_MSG_ERROR(system liblzma is too old)
    fi
  else
    AC_MSG_ERROR(pkg-config not found)
  fi
  PHP_EVAL_LIBLINE($LIBLZMA_LIBDIR, XZ_SHARED_LIBADD)
  PHP_EVAL_INCLINE($LIBLZMA_CFLAGS)

  PHP_NEW_EXTENSION(xz, xz.c, $ext_shared)
  PHP_SUBST(XZ_SHARED_LIBADD)

  dnl ifdef([PHP_INSTALL_HEADERS],
  dnl [
  dnl   PHP_INSTALL_HEADERS([ext/xz/], [php_xz.h])
  dnl ], [
  dnl   PHP_ADD_MAKEFILE_FRAGMENT
  dnl ])
fi

dnl coverage
PHP_ARG_ENABLE(coverage, whether to enable coverage support,
[  --enable-coverage       Enable coverage support], no, no)

if test "$PHP_COVERAGE" != "no"; then
    EXTRA_CFLAGS="--coverage"
    PHP_SUBST(EXTRA_CFLAGS)
fi
