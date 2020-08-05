/* idb extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

extern "C" {
#include "php.h"
#include "ext/standard/info.h"
#include "php_idb.h"
}

#include <rocksdb/db.h>

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif


/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(idb)
{
#if defined(ZTS) && defined(COMPILE_DL_IDB)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(idb)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "idb support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_idb_construct, 0, 0, 1)
 ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_idb_get, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_idb_set, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ idb_functions[]
 */
static const zend_function_entry idb_functions[] = {
    PHP_ME(IDB, __construct, arginfo_idb_construct, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, get, arginfo_idb_get, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, set, arginfo_idb_set, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* }}} */
rocksdb::DB* m_rdb;

PHP_METHOD(IDB, __construct)
{
    char *path;
    size_t path_len;
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &path, &path_len) == FAILURE) {
        return;
    }
    rocksdb::Options options;
    std::string m_last_error;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, path, &m_rdb);
    if(!status.ok())
    {
        m_last_error = status.ToString();
        char *error = const_cast<char *>(m_last_error.c_str()) ;
        zend_throw_error(NULL, error);
    }
    return;
}

PHP_METHOD(IDB, set)
{
    char *key, *value;
    size_t key_len, value_len;
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "ss", &key, &key_len, &value, &value_len) == FAILURE) {
        return;
    }
    if (m_rdb == NULL)
    {
        zend_throw_error(NULL, "un initial error");
    }
    rocksdb::Status status = m_rdb->Put(rocksdb::WriteOptions(), key, value);
    if(!status.ok())
    {
        std::string m_last_error = status.ToString();
        char *error = const_cast<char *>(m_last_error.c_str()) ;
        zend_throw_error(NULL, error);
    }
    return;
}

PHP_METHOD(IDB, get)
{
    char *key;
    size_t key_len;
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &key, &key_len) == FAILURE) {
        return;
    }
    if (m_rdb == NULL)
    {
        zend_throw_error(NULL, "un initial error");
    }
    std::string value;
    rocksdb::Status status = m_rdb->Get(rocksdb::ReadOptions(), key, &value);
    zend_string *retval;
    char *var = const_cast<char *>(value.c_str()) ;
    retval = strpprintf(0, "%s", var);
    RETURN_STR(retval);
}

zend_class_entry *idb_ce;

void idb_init(void)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "IDB", idb_functions);
    idb_ce = zend_register_internal_class(&ce);
}

PHP_MINIT_FUNCTION(idb)
{
    idb_init();
    return SUCCESS;
}


/* {{{ idb_module_entry
 */
zend_module_entry idb_module_entry = {
	STANDARD_MODULE_HEADER,
	"idb",					/* Extension name */
	idb_functions,			/* zend_function_entry */
	PHP_MINIT(idb),							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(idb),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(idb),			/* PHP_MINFO - Module info */
	PHP_IDB_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_IDB
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
BEGIN_EXTERN_C()
ZEND_GET_MODULE(idb)
END_EXTERN_C()
#endif
