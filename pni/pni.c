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
  | Author: Zuocheng Liu<i@zuocheng.net>                                 |
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

/* If you declare any globals in php_pni.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(pni)
*/
#define MAX_PNI_FUNCTION_PARAMS 255
/* True global resources - no need for thread safety here */
static int le_pni;

typedef zval *(*NATIVE_INTERFACE)(zval **args);

PHP_FUNCTION(get_pni_version);
PHP_METHOD(PNIFunction, __construct);                                      
PHP_METHOD(PNIFunction, __destruct);
PHP_METHOD(PNIFunction, invokeArgs);
PHP_METHOD(PNIFunction, invoke);
PHP_METHOD(PNIFunction, getFunctionName);
PHP_METHOD(PNIFunction, getLibName);


/* {{{ pni_functions[]
 *
 * Every user visible function must have an entry in pni_functions[].
 */
const zend_function_entry pni_functions[] = {
    PHP_FE(get_pni_version,    NULL) 
    PHP_ME(PNIFunction, __construct,     NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_ME(PNIFunction, __destruct,      NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR) 
    PHP_ME(PNIFunction, getFunctionName, NULL, ZEND_ACC_PUBLIC) 
    PHP_ME(PNIFunction, getLibName,      NULL, ZEND_ACC_PUBLIC) 
    PHP_ME(PNIFunction, invokeArgs,      NULL, ZEND_ACC_PUBLIC) 
    PHP_ME(PNIFunction, invoke,          NULL, ZEND_ACC_PUBLIC) 
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

zend_class_entry *pniFunction_ce;
PHP_MINIT_FUNCTION(pni)
{
    /* If you have INI entries, uncomment these lines 
    REGISTER_INI_ENTRIES();
    */
    zend_class_entry pnifunction; 
    INIT_CLASS_ENTRY(pnifunction, "PNIFunction", pni_functions);
    pniFunction_ce = zend_register_internal_class_ex(&pnifunction, NULL, NULL TSRMLS_CC);
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pni)
{
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

/* {{{ proto public void ReflectionFunction::__construct(string name)
 *    Constructor. Throws an Exception in case the given function does not exist */
PHP_METHOD(PNIFunction, __construct) {
    char *functionName = NULL;
    int functionName_len;
    char *libName = NULL;
    int libName_len;
    zval *value, *self;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &functionName, &functionName_len,&libName, &libName_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    self = getThis();
    MAKE_STD_ZVAL(value);
    ZVAL_STRINGL(value, functionName, functionName_len, 0);
    SEPARATE_ZVAL_TO_MAKE_IS_REF(&value);
    zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL("_functionName"), value TSRMLS_CC);
    
    ZVAL_STRINGL(value, libName, libName_len, 0);
    SEPARATE_ZVAL_TO_MAKE_IS_REF(&value);
    zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL("_libName"), value TSRMLS_CC);
    
    RETURN_TRUE;
}

/* {{{ proto public string PNIFunction::getFunctionName()
Returns a string representation */
PHP_METHOD(PNIFunction, getFunctionName) {
    zval *self, *name;
    self = getThis();
    name = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("_functionName"), 0 TSRMLS_CC);
    RETURN_STRING(Z_STRVAL_P(name), 0);
}
/* }}} */

/* {{{ proto public string PNIFunction::getLibName()
Returns a string representation */
PHP_METHOD(PNIFunction, getLibName) {
    zval *self, *name;
    self = getThis();
    name = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("_libName"), 0 TSRMLS_CC);
    RETURN_STRING(Z_STRVAL_P(name), 0);
}
/* }}} */

/** {{{ 
 * */
PHP_METHOD(PNIFunction, __destruct) {
}
/* }}} */

/* {{{ proto public string PNIFunction::invokeArgs(array args)
Returns a zval pointer */
PHP_METHOD(PNIFunction, invokeArgs) {
    zval *self, *functionName, *libName, *args, *res;
    zval **data;
    HashTable *arrHash;
    HashPosition pointer;
    int i = 0;
    zval *argList[MAX_PNI_FUNCTION_PARAMS];
    
    NATIVE_INTERFACE nativeInterface = NULL;
    
    self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &args) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    libName = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("_libName"), 0 TSRMLS_CC);
    functionName = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("_functionName"), 0 TSRMLS_CC);

    arrHash = Z_ARRVAL_P(args);
    //arrayCount = zend_hash_num_elements(arrHash);
    for(zend_hash_internal_pointer_reset_ex(arrHash, &pointer);
            zend_hash_get_current_data_ex(arrHash, (void**) &data, &pointer) == SUCCESS;
            zend_hash_move_forward_ex(arrHash, &pointer)) {
        argList[i] = *data;
        i++;
    }
    void *dlHandle = dlopen(Z_STRVAL_P(libName),RTLD_LAZY);
    if(!dlHandle) {
        php_printf("%s \n",dlerror());
    }
    nativeInterface = (NATIVE_INTERFACE)dlsym(dlHandle, Z_STRVAL_P(functionName));
    res = nativeInterface(argList);
    
    dlclose(dlHandle);
    RETURN_ZVAL(res, 1, 0);
}
/* }}} */

/* {{{ proto public string PNIFunction::invoke(mix a,mix b,...)
Returns a zval pointer */
PHP_METHOD(PNIFunction, invoke) {
    zval *self, *functionName, *libName, *res;
    zval ***params = NULL;
    int argc = 0; 
    
    NATIVE_INTERFACE nativeInterface = NULL;
    
    self = getThis();
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "*", &params, &argc) == FAILURE) {
        return;
    } 
    
    libName = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("_libName"), 0 TSRMLS_CC);
    functionName = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("_functionName"), 0 TSRMLS_CC);

    void *dlHandle = dlopen(Z_STRVAL_P(libName),RTLD_LAZY);
    if(!dlHandle) {
        php_printf("%s \n",dlerror());
    }
    
    nativeInterface = (NATIVE_INTERFACE)dlsym(dlHandle, Z_STRVAL_P(functionName));
    res = nativeInterface(*params);
    
    dlclose(dlHandle);
    RETURN_ZVAL(res, 1, 0);
}
/* }}} */



/* }}} */
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
