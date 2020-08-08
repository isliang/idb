/* idb extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

extern "C" {
#include "php.h"
#include "ext/standard/info.h"
#include "php_idb.h"
}

#include <rocksdb.h>

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif


/* {{{ PHP_RINIT_FUNCTION
 *  */
PHP_RINIT_FUNCTION(idb)
{
#if defined(ZTS) && defined(COMPILE_DL_IDB)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 *  */
PHP_MINFO_FUNCTION(idb)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "idb support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 *  */
ZEND_BEGIN_ARG_INFO_EX(arginfo_idb_construct, 0, 0, 1)
 ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_idb_get, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_idb_put, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_idb_open, 0, 0, 1)
    ZEND_ARG_INFO(0, readonly)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_idb_void, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ idb_functions[]
 *  */
static const zend_function_entry idb_functions[] = {
    PHP_ME(IDB, __construct, arginfo_idb_construct, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, get, arginfo_idb_get, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, put, arginfo_idb_put, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, open, arginfo_idb_open, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, lastError, arginfo_idb_void, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, __destruct, arginfo_idb_void, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* }}} */
DB* m_rdb;
void RocksDB::setPath(char *_path)
{
    path = _path;
}
zend_string* RocksDB::lastError(void)
{
    char *last_error = const_cast<char *>(m_last_error.c_str()) ;
    zend_string *error = strpprintf(0, "%s", last_error);
    return error;
}
zend_bool RocksDB::open(zend_bool readonly, Options options)
{
    if (readonly)
    {
        status = DB::OpenForReadOnly(options, path, &m_rdb);
    } else {
        status = DB::Open(options, path, &m_rdb);
    }
    if(!status.ok())
    {
        m_last_error = status.ToString();
        return false;
    }
    is_open = true;
    return true;
}
zend_bool RocksDB::put(char *key, char *value)
{
    if (!is_open)
    {
        m_last_error = "rocks db not open";
        return false;
    }
    status = m_rdb->Put(WriteOptions(), key, value);
    if(!status.ok())
    {
        m_last_error = status.ToString();
        return false;
    }
    return true;
}
ValueData RocksDB::get(char *key)
{
    if (!is_open)
    {
        m_last_error = "rocks db not open";
        ValueData result = {false, ""};
        return result;
    }
    string value;
    status = m_rdb->Get(ReadOptions(), key, &value);
    if(!status.ok())
    {
        m_last_error = status.ToString();
        ValueData result = {false, ""};
        return result;
    }
    ValueData result = {true, value};
    return result;
}
RocksDB rocksDb;
PHP_METHOD(IDB, __construct)
{
    char *path;
    size_t path_len;
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &path, &path_len) == FAILURE) {
        return;
    }
    rocksDb.setPath(path);
    return;
}

PHP_METHOD(IDB, open)
{
    zend_bool readonly = false;
    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(readonly)
    ZEND_PARSE_PARAMETERS_END();

    Options options;
    options.create_if_missing = true;
    zend_bool is_success = rocksDb.open(readonly, options);
    if(!is_success)
    {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(IDB, put)
{
    char *key, *value;
    size_t key_len, value_len;
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "ss", &key, &key_len, &value, &value_len) == FAILURE) {
        return;
    }
    zend_bool is_success = rocksDb.put(key, value);
    if(!is_success)
    {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(IDB, get)
{
    char *key;
    size_t key_len;
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &key, &key_len) == FAILURE) {
        return;
    }
    ValueData valueData = rocksDb.get(key);
    if (!valueData.is_success)
    {
        RETURN_FALSE;
    }
    zend_string *retval;
    char *var = const_cast<char *>(valueData.value.c_str()) ;
    retval = strpprintf(0, "%s", var);
    RETURN_STR(retval);
}

PHP_METHOD(IDB, lastError)
{
    zend_string *error = rocksDb.lastError();
    RETURN_STR(error);
}

PHP_METHOD(IDB, __destruct)
{
    delete m_rdb;
    return;
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
 *  */
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
