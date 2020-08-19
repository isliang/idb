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
    PHP_ME(IDB, mGet, arginfo_idb_get, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, put, arginfo_idb_put, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, open, arginfo_idb_open, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, lastError, arginfo_idb_void, ZEND_ACC_PUBLIC)
    PHP_ME(IDB, __destruct, arginfo_idb_void, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* }}} */
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
    status = DB::ListColumnFamilies(options, path, &column_families);
    if(!status.ok())
    {
        m_last_error = status.ToString();
        return false;
    }
    vector<ColumnFamilyDescriptor> _column_families;
    for (auto column_family : column_families) {
        _column_families.push_back(ColumnFamilyDescriptor(
                column_family, ColumnFamilyOptions()));
    }
    if (readonly)
    {
        status = DB::OpenForReadOnly(options, path, _column_families, &handles, &m_rdb);
    } else {
        status = DB::Open(options, path, _column_families, &handles, &m_rdb);
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
zend_bool RocksDB::put(string column_family, char *key, char *value)
{
    if (!is_open)
    {
        m_last_error = "rocks db not open";
        return false;
    }
    //is column already exist
    int i = handleIndex(column_family);
    if (i < 0) {
        return false;
    }
    status = m_rdb->Put(WriteOptions(), handles[i], key, value);
    if(!status.ok())
    {
        m_last_error = status.ToString();
        return false;
    }
    return true;
}
zend_bool RocksDB::get(char *key, string* value)
{
    if (!is_open)
    {
        m_last_error = "rocks db not open";
        return false;
    }
    status = m_rdb->Get(ReadOptions(), key, value);
    if(!status.ok())
    {
        m_last_error = status.ToString();
        return false;
    }
    return true;
}
zend_bool RocksDB::get(string column_family, char *key, string* value)
{
    if (!is_open)
    {
        m_last_error = "rocks db not open";
        return false;
    }
    //is column already exist
    int i = handleIndex(column_family);
    if (i < 0) {
        return false;
    }
    status = m_rdb->Get(ReadOptions(), handles[i], key, value);
    if(!status.ok())
    {
        m_last_error = status.ToString();
        return false;
    }
    return true;
}
int RocksDB::handleIndex(string column_family)
{
    int i;
    for(i = 0; i < column_families.size(); i++) {
        if (column_families[i] == column_family) {
            break;
        }
    }
    //not exist, create
    if (i >= column_families.size()) {
        column_families.push_back(column_family);
        ColumnFamilyHandle *handle;
        status = m_rdb->CreateColumnFamily(ColumnFamilyOptions(), column_family, &handle);
        if(!status.ok())
        {
            m_last_error = status.ToString();
            return -1;
        }
        handles.push_back(handle);
    }
    return i;
}
zend_bool RocksDB::mGet(vector<Slice>& keys, vector<zval *>* _values)
{
    if (!is_open)
    {
        m_last_error = "rocks db not open";
        return false;
    }
    vector<string> values;
    vector<Status> status = m_rdb->MultiGet(ReadOptions(), keys, &values);
    int i = 0;
    char *v, *error;
    for (auto s : status) {
        zval *value;
        value = (zval *)emalloc(sizeof(zval));
        array_init(value);
        v = const_cast<char *>(values[i].c_str());
        add_assoc_string(value, "key", keys[i].data());
        add_assoc_string(value, "value", v);
        if(!s.ok()) {
            add_assoc_long(value, "code", 0);
            error = const_cast<char *>(s.ToString().c_str()) ;
            add_assoc_string(value, "error", error);
        } else {
            add_assoc_long(value, "code", 1);
            add_assoc_null(value, "error");
        }
        _values->push_back(value);
        i++;
    }
    return true;
}
void RocksDB::close(void)
{
    delete m_rdb;
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
    string value;
    zend_bool is_success = rocksDb.get(key, &value);
    if (!is_success)
    {
        RETURN_FALSE;
    }
    zend_string *retval;
    char *var = const_cast<char *>(value.c_str()) ;
    retval = strpprintf(0, "%s", var);
    RETURN_STR(retval);
}
PHP_METHOD(IDB, mGet)
{
    zval *params, *param;
    vector<Slice> keys;
    vector<zval *> values;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a", &params) == FAILURE) {
        return;
    }
    if (!zend_hash_num_elements(Z_ARRVAL_P(params))) {
        RETURN_FALSE;
    }
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(params), param) {
        if (IS_STRING == Z_TYPE_P(param)) {
            keys.push_back(Slice(Z_STRVAL_P(param)));
        }
    } ZEND_HASH_FOREACH_END();

    zend_bool is_success = rocksDb.mGet(keys, &values);
    if(!is_success)
    {
        RETURN_FALSE;
    }
    array_init(return_value);
    for (auto value : values) {
        add_next_index_zval(return_value, value);
    }
}
PHP_METHOD(IDB, lastError)
{
    zend_string *error = rocksDb.lastError();
    RETURN_STR(error);
}

PHP_METHOD(IDB, __destruct)
{
    rocksDb.close();
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
