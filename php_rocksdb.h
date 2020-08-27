/* rocksdb extension for PHP */

#ifndef PHP_ROCKSDB_H
# define PHP_ROCKSDB_H

extern zend_module_entry rocksdb_module_entry;
# define phpext_rocksdb_ptr &rocksdb_module_entry
#define ROCKSDB_NS "RocksDB"
#define ROCKSDB_NS_NAME(name) ROCKSDB_NS"\\"#name
PHP_METHOD(RocksDB, open);
PHP_METHOD(RocksDB, put);
PHP_METHOD(RocksDB, get);
PHP_METHOD(RocksDB, mGet);
PHP_METHOD(RocksDB, lastError);
# define PHP_ROCKSDB_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_ROCKSDB)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif  /* PHP_ROCKSDB_H */
