/* rocksdb extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

extern "C" {
#include "php.h"
#include "ext/standard/info.h"
#include "php_rocksdb.h"
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
PHP_RINIT_FUNCTION(rocksdb)
{
#if defined(ZTS) && defined(COMPILE_DL_ROCKSDB)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 *  */
PHP_MINFO_FUNCTION(rocksdb)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "rocksdb support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 *  */
ZEND_BEGIN_ARG_INFO_EX(arginfo_rocksdb_construct, 0, 0, 1)
 ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rocksdb_get, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rocksdb_put, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rocksdb_open, 0, 0, 1)
    ZEND_ARG_INFO(0, readonly)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rocksdb_void, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ rocksdb_functions[]
 *  */
static const zend_function_entry rocksdb_functions[] = {
    PHP_ME(RocksDB, __construct, arginfo_rocksdb_construct, ZEND_ACC_PUBLIC)
    PHP_ME(RocksDB, get, arginfo_rocksdb_get, ZEND_ACC_PUBLIC)
    PHP_ME(RocksDB, mGet, arginfo_rocksdb_get, ZEND_ACC_PUBLIC)
    PHP_ME(RocksDB, put, arginfo_rocksdb_put, ZEND_ACC_PUBLIC)
    PHP_ME(RocksDB, open, arginfo_rocksdb_open, ZEND_ACC_PUBLIC)
    PHP_ME(RocksDB, lastError, arginfo_rocksdb_void, ZEND_ACC_PUBLIC)
    PHP_ME(RocksDB, __destruct, arginfo_rocksdb_void, ZEND_ACC_PUBLIC)
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
    return put(kDefaultColumnFamilyName, key, value);
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
    return get(kDefaultColumnFamilyName, key, value);
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
    vector<string> _column_families;
    for(auto key : keys) {
        _column_families.push_back(kDefaultColumnFamilyName);
    }
    return mGet(_column_families, keys, _values);
}
zend_bool RocksDB::mGet(vector<string>& _column_families, vector<Slice>& keys, vector<zval *>* _values)
{
    if (!is_open)
    {
        m_last_error = "rocks db not open";
        return false;
    }
    if (_column_families.size() != keys.size()) {
        m_last_error = "the size of column families and keys not match";
        return false;
    }
    vector<string> values;
    vector<ColumnFamilyHandle*> _handles;
    int i;
    for (auto column_family : _column_families) {
        i = handleIndex(column_family);
        if (i < 0) {
            return false;
        }
        _handles.push_back(handles[i]);
    }
    vector<Status> status = m_rdb->MultiGet(ReadOptions(), _handles, keys, &values);
    char *v, *error;
    i = 0;
    for (auto s : status) {
        zval *value;
        value = (zval *)emalloc(sizeof(zval));
        array_init(value);
        add_assoc_string(value, "key", keys[i].data());
        add_assoc_string(value, "column", _column_families[i].data());
        v = const_cast<char *>(values[i].c_str());
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
PHP_METHOD(RocksDB, __construct)
{
    char *path;
    size_t path_len;
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &path, &path_len) == FAILURE) {
        return;
    }
    rocksDb.setPath(path);
    return;
}

PHP_METHOD(RocksDB, open)
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

PHP_METHOD(RocksDB, put)
{
    char *key, *value, *column;
    size_t key_len, value_len, column_len;
    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_PATH(key, key_len)
        Z_PARAM_PATH(value, value_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_PATH(column, column_len)
    ZEND_PARSE_PARAMETERS_END();
    zend_bool is_success;
    if (ZEND_NUM_ARGS() == 3) {
        string column_family = column;
        is_success = rocksDb.put(column_family, key, value);
    } else {
        is_success = rocksDb.put(key, value);
    }
    if(!is_success)
    {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(RocksDB, get)
{
    char *key, *column;
    size_t key_len, column_len;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_PATH(key, key_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_PATH(column, column_len)
    ZEND_PARSE_PARAMETERS_END();
    string value;
    zend_bool is_success;
    if (ZEND_NUM_ARGS() == 2) {
        string column_family = column;
        is_success = rocksDb.get(column_family, key, &value);
    } else {
        is_success = rocksDb.get(key, &value);
    }

    if (!is_success)
    {
        RETURN_FALSE;
    }
    zend_string *retval;
    char *var = const_cast<char *>(value.c_str()) ;
    retval = strpprintf(0, "%s", var);
    RETURN_STR(retval);
}
PHP_METHOD(RocksDB, mGet)
{
    zval *params, *param, *columns, *column;
    vector<Slice> keys;
    vector<zval *> values;
    vector<string> column_families;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ARRAY(params)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(columns)
    ZEND_PARSE_PARAMETERS_END();

    if (!zend_hash_num_elements(Z_ARRVAL_P(params))) {
        RETURN_FALSE;
    }
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(params), param) {
        if (IS_STRING == Z_TYPE_P(param)) {
            keys.push_back(Slice(Z_STRVAL_P(param)));
        }
    } ZEND_HASH_FOREACH_END();

    zend_bool is_success;
    if (ZEND_NUM_ARGS() == 2 && zend_hash_num_elements(Z_ARRVAL_P(columns))) {
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(columns), column) {
            if (IS_STRING == Z_TYPE_P(column)) {
                column_families.push_back(Z_STRVAL_P(column));
            }
        } ZEND_HASH_FOREACH_END();
        is_success = rocksDb.mGet(column_families, keys, &values);
    } else {
        is_success = rocksDb.mGet(keys, &values);
    }
    if(!is_success)
    {
        RETURN_FALSE;
    }
    array_init(return_value);
    for (auto value : values) {
        add_next_index_zval(return_value, value);
    }
}
PHP_METHOD(RocksDB, lastError)
{
    zend_string *error = rocksDb.lastError();
    RETURN_STR(error);
}

PHP_METHOD(RocksDB, __destruct)
{
    rocksDb.close();
    return;
}

zend_class_entry *rocksdb_ce;

void rocksdb_init(void)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "RocksDB", rocksdb_functions);
    rocksdb_ce = zend_register_internal_class(&ce);
}

PHP_MINIT_FUNCTION(rocksdb)
{
    rocksdb_init();
    return SUCCESS;
}


/* {{{ rocksdb_module_entry
 *  */
zend_module_entry rocksdb_module_entry = {
	STANDARD_MODULE_HEADER,
	"rocksdb",					/* Extension name */
	rocksdb_functions,			/* zend_function_entry */
	PHP_MINIT(rocksdb),							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(rocksdb),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(rocksdb),			/* PHP_MINFO - Module info */
    PHP_ROCKSDB_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ROCKSDB
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
BEGIN_EXTERN_C()
ZEND_GET_MODULE(rocksdb)
END_EXTERN_C()
#endif
