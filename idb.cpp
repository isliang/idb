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
ZEND_BEGIN_ARG_INFO_EX(arginfo_idb_echo, 0, 0, 1)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ idb_functions[]
 */
static const zend_function_entry idb_functions[] = {
        PHP_ME(IDB, echo, arginfo_idb_echo, ZEND_ACC_PUBLIC)
	PHP_FE_END
};
/* }}} */

PHP_METHOD(IDB, echo)
{
    char *var = "World";
    size_t var_len = sizeof("World") - 1;
    zend_string *retval;
    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_STRING(var, var_len)
    ZEND_PARSE_PARAMETERS_END();
    retval = strpprintf(0, "Hello %s", var);
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
