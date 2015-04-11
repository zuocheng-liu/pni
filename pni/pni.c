/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Zuocheng Liu <zuocheng.liu@gmail.com>                        |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_pni.h"
#include <dlfcn.h>

/* function, variable and class property lables definination */
#define PNI_PROPERTY_LIBNAME_LABEL "_libName"

ZEND_BEGIN_ARG_INFO_EX(arginfo_pni___call, 0, 0, 2)
     ZEND_ARG_INFO(0, function_name)
     ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pni__void, 0)
ZEND_END_ARG_INFO()

/* If you declare any globals in php_pni.h uncomment this:*/
ZEND_DECLARE_MODULE_GLOBALS(pni)
/* True global resources - no need for thread safety here */
static int le_dl_handle_persist;


/* Local functions */
static void php_dl_handle_persist_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);


static zend_class_entry *pni_ptr;
static zend_class_entry *pni_exception_ptr;

/* PNI functions */
PHP_FUNCTION(get_pni_version);
PHP_METHOD(PNI, __construct);                                      
PHP_METHOD(PNI, __call);
PHP_METHOD(PNI, getLibName);

/* {{{ pni_functions[]
 *
 * Every user visible function must have an entry in pni_functions[].
 */
const zend_function_entry pni_functions[] = {
    PHP_FE(get_pni_version,    NULL) 
    
    PHP_ME(PNI, __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_ME(PNI, __call,         arginfo_pni___call, 0) 
    PHP_ME(PNI, getLibName,     arginfo_pni__void, ZEND_ACC_PUBLIC) 
    
    PHP_FE_END  /* Must be the last line in pni_functions[] */
};
/* }}} */

/* {{{ pni_exception_functions[]
 */
const zend_function_entry pni_exception_functions[] = {
    PHP_FE_END  /* Must be the last line in pni_functions[] */
};
/* }}} */


/* {{{ pni_module_entry
 */
zend_module_entry pni_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "pni",
    pni_functions,
    PHP_MINIT(pni),
    PHP_MSHUTDOWN(pni),
    PHP_RINIT(pni),     /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(pni), /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(pni),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_PNI_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PNI
ZEND_GET_MODULE(pni)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("pni.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_pni_globals, pni_globals)
    STD_PHP_INI_ENTRY("pni.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_pni_globals, pni_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_pni_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_pni_init_globals(zend_pni_globals *pni_globals)
{
    pni_globals->global_value = 0;
    pni_globals->global_string = NULL;
}
*/
/* }}} */


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pni) {
    zend_class_entry pni; 
    /* If you have INI entries, uncomment these lines 
    REGISTER_INI_ENTRIES();
    */
    le_dl_handle_persist = zend_register_list_destructors_ex(NULL, php_dl_handle_persist_dtor, PHP_DL_HANDLE_RES_NAME, module_number);
    
    /* class PNI */
    INIT_CLASS_ENTRY(pni, "PNI", pni_functions);
    pni_ptr = zend_register_internal_class_ex(&pni, NULL, NULL TSRMLS_CC);
    zend_declare_property_string(pni_ptr, PNI_PROPERTY_LIBNAME_LABEL, sizeof(PNI_PROPERTY_LIBNAME_LABEL) - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);

    /* class PNIException*/
    INIT_CLASS_ENTRY(pni, "PNIException", pni_exception_functions);
    pni_exception_ptr = zend_register_internal_class_ex(&pni, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
    
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pni) {
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(pni)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(pni)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pni)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "pni support", "enabled");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */


/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_pni_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(get_pni_version) {
    RETURN_STRING(PHP_PNI_VERSION, 1);
}

/* {{{ proto public void PNI::__construct($libName)
 *    Constructor. Throws an PNIException in case the given shared library does not exist */
PHP_METHOD(PNI, __construct) {
    char *lib_name = NULL;
    int lib_name_len = 0;
    zval *self = NULL;
   
    char *key = NULL;
    char * error_msg = NULL;
    int key_len = 0;
    void * dlHandle = NULL;
    zend_rsrc_list_entry *le, new_le;
    
    /* get the parameter $libName */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &lib_name, &lib_name_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

 
    /* get the dl handle, if not exists, create it and persist it */
    key_len = spprintf(&key, 0, "pni_dl_handle_%s\n", lib_name);
    if (zend_hash_find(&EG(persistent_list), key, key_len + 1, (void **)&le) == SUCCESS) {
        ZEND_REGISTER_RESOURCE(return_value, le->ptr, le_dl_handle_persist);
        efree(key);
        RETURN_TRUE;
    }
    
    /* init the dl handle resource */
    dlHandle = dlopen(lib_name, RTLD_LAZY);
    if (!dlHandle) {
        //php_error_docref(NULL TSRMLS_CC, E_WARNING, "dlopen error (%s) , dl handle resource not created.", dlerror());
        //RETURN_FALSE;
        spprintf(&error_msg, 0, "%s,  dl handle resource is not created.", dlerror());
        zend_throw_exception(pni_exception_ptr, error_msg, 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    /* registe dl handle resource */
    ZEND_REGISTER_RESOURCE(return_value, &dlHandle, le_dl_handle_persist);
    /* persist dl handle */
    new_le.ptr = dlHandle;
    new_le.type = le_dl_handle_persist;
    zend_hash_add(&EG(persistent_list), key, key_len + 1, &new_le, sizeof(zend_rsrc_list_entry), NULL);
    efree(key);
    /* save the libname to private variable */
    self = getThis();
    zend_update_property_stringl(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_LIBNAME_LABEL), lib_name, lib_name_len TSRMLS_CC);
    RETURN_TRUE;
}
/* }}} */


/* {{{ proto public void PNI::__call($functionName, $args)
Returns a zval pointer */
PHP_METHOD(PNI, __call) {
    zval *self,  *lib_name, *args, *res;
    HashTable *arr_hash_tb;
    HashPosition pointer;
    zval **data = NULL;
    int index = 0;
    char * function_name;
    char * error_msg;
    int argc = 0;
    int function_name_len = 0;
    zval *argList[MAX_PNI_FUNCTION_PARAMS];
    NATIVE_INTERFACE nativeInterface = NULL;
    char *key = NULL;
    int key_len = 0;
    void * dlHandle = NULL;
    zend_rsrc_list_entry *le, new_le;

    self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &function_name, &function_name_len, &args) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    lib_name = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_LIBNAME_LABEL), 0 TSRMLS_CC);
    /* trans zend args to c array  */
    arr_hash_tb = Z_ARRVAL_P(args);
    argc = zend_hash_num_elements(arr_hash_tb);
    for (zend_hash_internal_pointer_reset_ex(arr_hash_tb, &pointer);
            zend_hash_get_current_data_ex(arr_hash_tb, (void**) &data, &pointer) == SUCCESS;
            zend_hash_move_forward_ex(arr_hash_tb, &pointer)) {
        argList[index] = *data;
        index ++;
    }

    /* seek the persisted dl handle */
    key_len = spprintf(&key, 0, "pni_dl_handle_%s\n", Z_STRVAL_P(lib_name));
    if (zend_hash_find(&EG(persistent_list), key, key_len + 1, (void **)&le) == SUCCESS) {
        ZEND_REGISTER_RESOURCE(return_value, le->ptr, le_dl_handle_persist);
        efree(key);
    }
    dlHandle = le->ptr;
    if (!dlHandle) {
        spprintf(&error_msg, 0, "Fail to dl Native Interface. The PNI dl handle (%s) is invalid.", Z_STRVAL_P(lib_name));
        zend_throw_exception(pni_exception_ptr, error_msg, 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    
    /* seek dynamic library symbol */
    nativeInterface = (NATIVE_INTERFACE)dlsym(dlHandle, function_name);
    if (!nativeInterface) {
        spprintf(&error_msg, 0, "Dlsym %s error (%s). ", function_name, dlerror());
        zend_throw_exception(pni_exception_ptr, error_msg, 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    /* call native interface */
    res = nativeInterface(argList, argc);
    RETURN_ZVAL(res, 1, 0);
}
/* }}} */

/* {{{ proto public void PNI::getLibName()
Returns a string */
PHP_METHOD(PNI, getLibName) {
    zval *self, *value;
    self = getThis();
    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }
    value = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_LIBNAME_LABEL), 0 TSRMLS_CC);
    RETURN_STRING(Z_STRVAL_P(value), 1);
}
/* }}} */


/* release the dl resource*/
static void php_dl_handle_persist_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
    void *dlHandle = (void *) rsrc->ptr;
    dlclose(dlHandle);
}

/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
