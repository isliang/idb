dnl config.m4 for extension idb

PHP_ARG_ENABLE([idb],
  [whether to enable idb support],
  [AS_HELP_STRING([--enable-idb],
    [Enable idb support])],
  [no])

if test "$PHP_IDB" != "no"; then
  dnl Remove this code block if the library does not support pkg-config.
  PKG_CHECK_MODULES([LIBROCKSDB], [librocksdb])
  PHP_EVAL_INCLINE($LIBROCKSDB_CFLAGS)
  PHP_EVAL_LIBLINE($LIBROCKSDB_LIBS, IDB_SHARED_LIBADD)

  PHP_SUBST(IDB_SHARED_LIBADD)

  dnl In case of no dependencies
  AC_DEFINE(HAVE_IDB, 1, [ Have idb support ])

  PHP_REQUIRE_CXX()
  CXXFLAGS="$CXXFLAGS -std=c++11"
  PHP_ADD_LIBRARY(stdc++, "", EXTRA_LDFLAGS)
  PHP_NEW_EXTENSION(idb, idb.cpp, $ext_shared)
fi
