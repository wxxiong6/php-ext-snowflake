dnl $Id$
dnl config.m4 for extension snowflake

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(snowflake, for snowflake support,
dnl Make sure that the comment is aligned:
dnl [  --with-snowflake             Include snowflake support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(snowflake, whether to enable snowflake support,
dnl Make sure that the comment is aligned:
[  --enable-snowflake           Enable snowflake support])

if test "$PHP_SNOWFLAKE" != "no"; then
  dnl Write more examples of tests here...

  dnl # get library FOO build options from pkg-config output
  dnl AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  dnl AC_MSG_CHECKING(for libfoo)
  dnl if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists foo; then
  dnl   if $PKG_CONFIG foo --atleast-version 1.2.3; then
  dnl     LIBFOO_CFLAGS=`$PKG_CONFIG foo --cflags`
  dnl     LIBFOO_LIBDIR=`$PKG_CONFIG foo --libs`
  dnl     LIBFOO_VERSON=`$PKG_CONFIG foo --modversion`
  dnl     AC_MSG_RESULT(from pkgconfig: version $LIBFOO_VERSON)
  dnl   else
  dnl     AC_MSG_ERROR(system libfoo is too old: version 1.2.3 required)
  dnl   fi
  dnl else
  dnl   AC_MSG_ERROR(pkg-config not found)
  dnl fi
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBDIR, SNOWFLAKE_SHARED_LIBADD)
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)

  dnl # --with-snowflake -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/snowflake.h"  # you most likely want to change this
  dnl if test -r $PHP_SNOWFLAKE/$SEARCH_FOR; then # path given as parameter
  dnl   SNOWFLAKE_DIR=$PHP_SNOWFLAKE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for snowflake files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SNOWFLAKE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SNOWFLAKE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the snowflake distribution])
  dnl fi

  dnl # --with-snowflake -> add include path
  dnl PHP_ADD_INCLUDE($SNOWFLAKE_DIR/include)

  dnl # --with-snowflake -> check for lib and symbol presence
  dnl LIBNAME=snowflake # you may want to change this
  dnl LIBSYMBOL=snowflake # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SNOWFLAKE_DIR/$PHP_LIBDIR, SNOWFLAKE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SNOWFLAKELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong snowflake lib version or lib not found])
  dnl ],[
  dnl   -L$SNOWFLAKE_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(SNOWFLAKE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(snowflake, snowflake.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
