/* idb extension for PHP */

#ifndef PHP_IDB_H
# define PHP_IDB_H

extern zend_module_entry idb_module_entry;
# define phpext_idb_ptr &idb_module_entry
PHP_METHOD(IDB, __construct);
PHP_METHOD(IDB, set);
PHP_METHOD(IDB, get);
# define PHP_IDB_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_IDB)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_IDB_H */
