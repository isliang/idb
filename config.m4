dnl config.m4 for extension idb

PHP_ARG_ENABLE([idb],
  [whether to enable idb support],
  [AS_HELP_STRING([--enable-idb],
    [Enable idb support])],
  [no])

if test "$PHP_IDB" != "no"; then
  PHP_ADD_LIBRARY_WITH_PATH(rocksdb, /usr/local/lib, IDB_SHARED_LIBADD)
  PHP_SUBST(IDB_SHARED_LIBADD)
  PHP_REQUIRE_CXX()
  CXXFLAGS="$CXXFLAGS -std=c++11"
  PHP_ADD_LIBRARY(stdc++, "", EXTRA_LDFLAGS)
  PHP_NEW_EXTENSION(idb, idb.cpp, $ext_shared)
fi
