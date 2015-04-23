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

/* function variable and class property lables definination */
#define PHP_DL_HANDLE_RES_NAME "DL Handle"
#define MAX_PNI_FUNCTION_PARAMS 255
#define PNI_PROPERTY_LIBNAME_LABEL "_libName"
#define PNI_PROPERTY_FUNCTIONNAME_LABEL "_functionName"
#define PNI_PROPERTY_RETURN_DATA_TYPE_LABEL "_returnDataType"
#define PNI_PROPERTY_DATA_TYPE_LABEL "dataType"
#define PNI_PROPERTY_VALUE_LABEL "value"

#define PNI_DATA_TYPE_VOID      0
#define PNI_DATA_TYPE_CHAR      1
#define PNI_DATA_TYPE_INT       2
#define PNI_DATA_TYPE_LONG      3
#define PNI_DATA_TYPE_FLOAT     4
#define PNI_DATA_TYPE_DOUBLE    5
#define PNI_DATA_TYPE_STRING    6
#define PNI_DATA_TYPE_POINTER   7

typedef zval *(*NATIVE_INTERFACE)(zval **args, int argc);
typedef void *(*NATIVE_INTERFACE_SYMBOL)();

typedef void * BASE_DATA_TYPE;


/* arg_info definition */
ZEND_BEGIN_ARG_INFO_EX(arginfo_pni___call, 0, 0, 2)
     ZEND_ARG_INFO(0, function_name)
     ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pni__void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_pni_function_invoke, 0, 0, 0) 
     ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

/* macro */
#define PNI_RETURN(res,data_type,value) do {        \
    switch (data_type) {                            \
        case PNI_DATA_TYPE_VOID :                   \
            RETURN_NULL();                          \
        case PNI_DATA_TYPE_CHAR :                   \
        case PNI_DATA_TYPE_INT :                    \
        case PNI_DATA_TYPE_LONG :                   \
            ZVAL_LONG((res), (long)(value));        \
            break;                                  \
        case PNI_DATA_TYPE_FLOAT :                  \
        case PNI_DATA_TYPE_DOUBLE :                 \
            ZVAL_DOUBLE((res), (double)(value));    \
            break;                                  \
        case PNI_DATA_TYPE_STRING :                 \
        case PNI_DATA_TYPE_POINTER :                \
            ZVAL_STRING((res), (char *)(value), 1); \
            break;                                  \
        default :                                   \
            RETURN_NULL();                          \
    }                                               \
    RETURN_ZVAL((res), 1,0);                        \
}while (0)

/* If you declare any globals in php_pni.h uncomment this:*/
ZEND_DECLARE_MODULE_GLOBALS(pni)
/* True global resources - no need for thread safety here */
static int le_dl_handle_persist;

/* Local functions */
static void php_dl_handle_persist_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void trans_args_to_param_list(zval ***, const int, long *, int *, double *, int *);
/* asm debug mark */
static void asm_debug_mark(){};

static zend_class_entry *pni_ptr;
static zend_class_entry *pni_function_ptr;
static zend_class_entry *pni_exception_ptr;
static zend_class_entry *pni_integer_ptr;
static zend_class_entry *pni_data_type_ptr;
/*
static zend_class_entry *pni_float_ptr;
static zend_class_entry *pni_double_ptr;
static zend_class_entry *pni_string_ptr;
static zend_class_entry *pni_void_ptr;
static zend_class_entry *pni_char_ptr;
static zend_class_entry *pni_data_type_ptr;
*/
/* PNI functions */
PHP_FUNCTION(get_pni_version);
PHP_METHOD(PNI, __construct);                                      
PHP_METHOD(PNI, __call);
PHP_METHOD(PNI, getLibName);
PHP_METHOD(PNIFunction, __construct);                                      
PHP_METHOD(PNIFunction, invoke);                                      
PHP_METHOD(PNIDataType, __construct);                                      

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
const zend_function_entry pni_function_functions[] = {
    PHP_ME(PNIFunction, __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_ME(PNIFunction, invoke,         arginfo_pni_function_invoke, 0) 
    PHP_FE_END 
};
/* }}} */

/* {{{ pni_function_functions[]
 */
const zend_function_entry pni_exception_functions[] = {
    PHP_FE_END  /* Must be the last line in pni_functions[] */
};
/* }}} */

/* {{{ pni_function_functions[]
 */
const zend_function_entry pni_data_type_functions[] = {
    PHP_ME(PNIDataType, __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
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

    /* class PNIFunction */
    INIT_CLASS_ENTRY(pni, "PNIFunction", pni_function_functions);
    pni_function_ptr = zend_register_internal_class_ex(&pni, NULL, NULL TSRMLS_CC);
    zend_declare_property_string(pni_function_ptr, PNI_PROPERTY_LIBNAME_LABEL, sizeof(PNI_PROPERTY_LIBNAME_LABEL) - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(pni_function_ptr, PNI_PROPERTY_FUNCTIONNAME_LABEL, sizeof(PNI_PROPERTY_FUNCTIONNAME_LABEL) - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_long(pni_function_ptr, PNI_PROPERTY_RETURN_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_RETURN_DATA_TYPE_LABEL) - 1, 0, ZEND_ACC_PROTECTED TSRMLS_CC);

    /* class PNIException*/
    INIT_CLASS_ENTRY(pni, "PNIException", pni_exception_functions);
    pni_exception_ptr = zend_register_internal_class_ex(&pni, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
    
    /* class PNIDataType */
    INIT_CLASS_ENTRY(pni, "PNIDataType", pni_data_type_functions);
    pni_data_type_ptr = zend_register_internal_class_ex(&pni, NULL, NULL TSRMLS_CC);
    zend_declare_class_constant_long(pni_data_type_ptr, "INTEGER", sizeof("INTEGER") -1 , PNI_DATA_TYPE_INT TSRMLS_CC);
    zend_declare_property_null(pni_data_type_ptr, PNI_PROPERTY_VALUE_LABEL, sizeof(PNI_PROPERTY_VALUE_LABEL) - 1, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pni_data_type_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIInteger */
    INIT_CLASS_ENTRY(pni, "PNIInteger", pni_data_type_functions);
    pni_integer_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, NULL TSRMLS_CC);
    zend_declare_property_long(pni_data_type_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_INT, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNILong */
    INIT_CLASS_ENTRY(pni, "PNILong", pni_data_type_functions);
    pni_integer_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, NULL TSRMLS_CC);
    zend_declare_property_long(pni_data_type_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_LONG, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIChar */
    INIT_CLASS_ENTRY(pni, "PNIChar", pni_data_type_functions);
    pni_integer_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, NULL TSRMLS_CC);
    zend_declare_property_long(pni_data_type_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_CHAR, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIFloat */
    INIT_CLASS_ENTRY(pni, "PNIFloat", pni_data_type_functions);
    pni_integer_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, NULL TSRMLS_CC);
    zend_declare_property_long(pni_data_type_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_FLOAT, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIDouble */
    INIT_CLASS_ENTRY(pni, "PNIDouble", pni_data_type_functions);
    pni_integer_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, NULL TSRMLS_CC);
    zend_declare_property_long(pni_data_type_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_DOUBLE, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIDouble */
    INIT_CLASS_ENTRY(pni, "PNIString", pni_data_type_functions);
    pni_integer_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, NULL TSRMLS_CC);
    zend_declare_property_long(pni_data_type_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_STRING, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIPointer */
    INIT_CLASS_ENTRY(pni, "PNIPointer", pni_data_type_functions);
    pni_integer_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, NULL TSRMLS_CC);
    zend_declare_property_long(pni_data_type_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_POINTER, ZEND_ACC_PROTECTED TSRMLS_CC);
    
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

/* {{{ proto public void PNIFunction::__construct($returnDataType, $functionName, $libName)
 *    Constructor. Throws an PNIException in case the given shared library does not exist */
PHP_METHOD(PNIFunction, __construct) {
    int data_type = 0;
    char *lib_name = NULL;
    int lib_name_len = 0;
    char *function_name = NULL;
    int function_name_len = 0;
    
    zval *self = NULL;
   
    char *key = NULL;
    char * error_msg = NULL;
    int key_len = 0;
    void * dlHandle = NULL;
    zend_rsrc_list_entry *le, new_le;
    
    /* get the parameter $libName */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Iss", &data_type, &function_name, &function_name_len, &lib_name, &lib_name_len) == FAILURE) {
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

    self = getThis();
    /* save the libname to private variable */
    zend_update_property_stringl(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_LIBNAME_LABEL), lib_name, lib_name_len TSRMLS_CC);
    /* save the function name to private variable */
    zend_update_property_stringl(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_FUNCTIONNAME_LABEL), function_name, function_name_len TSRMLS_CC);
    /* save the data_type to private variable */
    zend_update_property_long(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_RETURN_DATA_TYPE_LABEL), data_type TSRMLS_CC);
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto public void PNIFunction::invoke(PNIDataType arg1, ...)
 *   Call the native function . Throws an PNIException. */
PHP_METHOD(PNIFunction, invoke) {
    NATIVE_INTERFACE_SYMBOL nativeInterface;
    char * pni_return_value;
    int pni_return_data_type;
    zval *self,  *lib_name, *function_name, *return_data_type, *res;
    zval ***args;
    int argc;
    
    HashTable *arr_hash_tb;
    HashPosition pointer;
    
    char *key;
    int key_len;
    
    void * dlHandle;
    zend_rsrc_list_entry *le, new_le;
    char * error_msg;
    
    void * long_param_list[MAX_PNI_FUNCTION_PARAMS];
    int long_param_count;

    self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &argc) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    
    lib_name = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_LIBNAME_LABEL), 0 TSRMLS_CC);
    function_name = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_FUNCTIONNAME_LABEL), 0 TSRMLS_CC);
    return_data_type = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_RETURN_DATA_TYPE_LABEL), 0 TSRMLS_CC);

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
        efree(args);
        RETURN_FALSE;
    }
    
    /* seek dynamic library function symbol */
    nativeInterface = (NATIVE_INTERFACE_SYMBOL)dlsym(dlHandle, Z_STRVAL_P(function_name));
    if (!nativeInterface) {
        spprintf(&error_msg, 0, "Dlsym %s error (%s). ", Z_STRVAL_P(function_name), dlerror());
        zend_throw_exception(pni_exception_ptr, error_msg, 0 TSRMLS_CC);
        efree(args);
        RETURN_FALSE;
    }
    
    efree(args);
    RETURN_NULL();
    //RETURN_ZVAL(args ,1,0);
    ALLOC_INIT_ZVAL(res);
    
    /* call native interface */
    //res = nativeInterface(argList, argc);
    __asm__ __volatile__ ("");
    pni_return_data_type = Z_DVAL_P(return_data_type);
    PNI_RETURN(res, pni_return_data_type, *pni_return_value);
}
/* }}} */

/* {{{ proto public void PNI::__construct($libName)
 *    Constructor. Throws an PNIException in case the given shared library does not exist */
PHP_METHOD(PNIDataType, __construct) {
    zval * value;
    zval * self;
    /* get the parameter $value */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    self = getThis();
    zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), value TSRMLS_CC);
}
/* }}} */

/* trans the parameters value to c array list */
static void trans_args_to_param_list(zval ***args,const int argc, 
        long * long_param_list, int *long_param_count, 
        double * double_param_list, int *double_param_count) {
    
    int long_offset = 0;
    int double_offset = 0;
    int pni_data_type;
    zval **array;
    zval *zval_value, *zval_data_type;
    zval *obj;
    if (0 == argc) {
        return ;
    }
    array = args[0];
    SEPARATE_ZVAL(array);
    convert_to_array_ex(array);
    for (zend_hash_internal_pointer_reset(Z_ARRVAL_PP(array));
            zend_hash_get_current_data(Z_ARRVAL_PP(array), (void **)&obj) == SUCCESS;
            zend_hash_move_forward(Z_ARRVAL_PP(array))) {

        zval_value = zend_read_property(Z_OBJCE_P(obj), obj, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), 0 TSRMLS_CC);       
        zval_data_type = zend_read_property(Z_OBJCE_P(obj), obj, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), 0 TSRMLS_CC);       
        pni_data_type = Z_LVAL_P(zval_data_type);
        switch ( pni_data_type ) {
            case PNI_DATA_TYPE_CHAR :
            case PNI_DATA_TYPE_INT :
            case PNI_DATA_TYPE_LONG :
                convert_to_long(zval_value);
                long_param_list[long_offset] = (long)(Z_LVAL_P(zval_value)); 
                long_offset ++;
                break;
            case PNI_DATA_TYPE_STRING :
            case PNI_DATA_TYPE_POINTER:
                convert_to_string(zval_value);
                long_param_list[long_offset] = (long)(Z_STRVAL_P(zval_value));;
                long_offset ++;
                break;

            case PNI_DATA_TYPE_FLOAT :
            case PNI_DATA_TYPE_DOUBLE :
                convert_to_double(zval_value);
                double_param_list[double_offset] = (double)(Z_DVAL_P(zval_value));
                double_offset ++;
                break;
            default :
                break;
        }
    }
    *long_param_count = long_offset;
    *double_param_count = double_offset;
}

/* release the dl resource */
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
