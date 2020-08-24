dnl config.m4 for extension rocksdb

PHP_ARG_ENABLE([rocksdb],
  [whether to enable rocksdb support],
  [AS_HELP_STRING([--enable-rocksdb],
    [Enable rocksdb support])],
  [no])

if test "$PHP_ROCKSDB" != "no"; then
  PHP_ADD_LIBRARY_WITH_PATH(rocksdb, /usr/local/lib, ROCKSDB_SHARED_LIBADD)
  PHP_SUBST(ROCKSDB_SHARED_LIBADD)
  PHP_REQUIRE_CXX()
  CXXFLAGS="$CXXFLAGS -std=c++11"
  PHP_ADD_LIBRARY(stdc++, "", EXTRA_LDFLAGS)
  PHP_NEW_EXTENSION(rocksdb, rocksdb.cpp, $ext_shared)
fi
