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
#include "zend_exceptions.h"

/* function variable and class property lables definination */
#define PNI_DL_HANDLE_RES_NAME "DL Handle"
#define MAX_PNI_FUNCTION_PARAMS 255
#define MAX_PNI_FUNCTION_LONG_PARAMS 8
#define MAX_PNI_FUNCTION_DOUBLE_PARAMS 6
#define PNI_PROPERTY_DL_HANDLE_LABEL "_dlHandle"
#define PNI_PROPERTY_LIBNAME_LABEL "_libName"
#define PNI_PROPERTY_FUNCTIONNAME_LABEL "_functionName"
#define PNI_PROPERTY_RETURN_DATA_TYPE_LABEL "_returnDataType"
#define PNI_PROPERTY_DATA_TYPE_LABEL "_dataType"
#define PNI_PROPERTY_VALUE_LABEL "_value"

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
typedef void *PNI_DATA_TYPE_ANY;


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

/* functions declaration */
static void pni_dl_handle_persist_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void trans_args_to_param_list(zval ***, const int, long *, int *, double *, int *);
static PNI_DATA_TYPE_ANY call_native_interface(NATIVE_INTERFACE_SYMBOL,const long const *, const int ,const double const *, const int);
static void assign_value_to_pni_return_data(int pni_data_type, PNI_DATA_TYPE_ANY, zval * TSRMLS_CC);
static int get_persisted_dl_handle (char *, void ** , zval * );
static inline int pni_data_factory(int, zval * TSRMLS_DC);
/* asm debug mark */

/* class entry declare */
static zend_class_entry *pni_ptr;
static zend_class_entry *pni_function_ptr;
static zend_class_entry *pni_exception_ptr;
static zend_class_entry *pni_data_type_ptr;
static zend_class_entry *pni_char_ptr;
static zend_class_entry *pni_integer_ptr;
static zend_class_entry *pni_long_ptr;
static zend_class_entry *pni_float_ptr;
static zend_class_entry *pni_double_ptr;
static zend_class_entry *pni_string_ptr;
static zend_class_entry *pni_pointer_ptr;

/* If you declare any globals in php_pni.h uncomment this:*/
ZEND_DECLARE_MODULE_GLOBALS(pni)
/* True global resources - no need for thread safety here */
static int le_dl_handle_persist;


/* PNI functions */
PHP_FUNCTION(get_pni_version);
PHP_METHOD(PNI, __construct);                                      
PHP_METHOD(PNI, __call);
PHP_METHOD(PNI, getLibName);
PHP_METHOD(PNIFunction, __construct);                                      
PHP_METHOD(PNIFunction, invoke);                                      

PHP_METHOD(PNIDataType, getValue);                                      
PHP_METHOD(PNIDataType, setValue);                                      
PHP_METHOD(PNIDataType, getDataType);                                      
PHP_METHOD(PNIDataType, systemFree);                                      

PHP_METHOD(PNIChar, __construct);                                      
PHP_METHOD(PNIInteger, __construct);                                      
PHP_METHOD(PNILong, __construct);                                      
PHP_METHOD(PNIFloat, __construct);                                      
PHP_METHOD(PNIDouble, __construct);                                      
PHP_METHOD(PNIString, __construct);                                      
PHP_METHOD(PNIPointer, __construct);                                      

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

/* {{{ pni_function_functions[]
 */
const zend_function_entry pni_function_functions[] = {
    PHP_ME(PNIFunction, __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_ME(PNIFunction, invoke,         arginfo_pni_function_invoke, 0) 
    PHP_MALIAS(PNIFunction, __invoke, invoke, arginfo_pni_function_invoke, ZEND_ACC_PUBLIC)
    PHP_FE_END 
};
/* }}} */

/* {{{ pni_exception_functions[]
 */
const zend_function_entry pni_exception_functions[] = {
    PHP_FE_END
};
/* }}} */

/* {{{ pni_data_type_functions[]
 */
const zend_function_entry pni_data_type_functions[] = {
    PHP_ME(PNIDataType, getValue,       NULL, ZEND_ACC_PUBLIC) 
    PHP_ME(PNIDataType, setValue,       NULL, ZEND_ACC_PUBLIC) 
    PHP_ME(PNIDataType, getDataType,    NULL, ZEND_ACC_PUBLIC) 
    PHP_ME(PNIDataType, systemFree,     NULL, ZEND_ACC_PUBLIC) 
    PHP_FE_END 
};
/* }}} */

const zend_function_entry pni_char_functions[] = {
    PHP_ME(PNIChar,     __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_FE_END 
};
const zend_function_entry pni_integer_functions[] = {
    PHP_ME(PNIInteger,     __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_FE_END 
};
const zend_function_entry pni_long_functions[] = {
    PHP_ME(PNILong,     __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_FE_END 
};
const zend_function_entry pni_float_functions[] = {
    PHP_ME(PNIFloat,     __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_FE_END 
};
const zend_function_entry pni_double_functions[] = {
    PHP_ME(PNIDouble,     __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_FE_END 
};
const zend_function_entry pni_string_functions[] = {
    PHP_ME(PNIString,     __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_FE_END 
};
const zend_function_entry pni_pointer_functions[] = {
    PHP_ME(PNIPointer,     __construct,    NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR) 
    PHP_FE_END 
};
 
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
    le_dl_handle_persist = zend_register_list_destructors_ex(NULL, pni_dl_handle_persist_dtor, PNI_DL_HANDLE_RES_NAME, module_number);
    
    /* class PNI */
    INIT_CLASS_ENTRY(pni, "PNI", pni_functions);
    pni_ptr = zend_register_internal_class_ex(&pni, NULL, NULL TSRMLS_CC);
    //zend_declare_property(pni_ptr, PNI_PROPERTY_DL_HANDLE_LABEL, sizeof(PNI_PROPERTY_DL_HANDLE_LABEL) - 1, NULL, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(pni_ptr, PNI_PROPERTY_LIBNAME_LABEL, sizeof(PNI_PROPERTY_LIBNAME_LABEL) - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);

    /* class PNIFunction */
    INIT_CLASS_ENTRY(pni, "PNIFunction", pni_function_functions);
    pni_function_ptr = zend_register_internal_class_ex(&pni, NULL, NULL TSRMLS_CC);
    //zend_declare_property(pni_function_ptr, PNI_PROPERTY_DL_HANDLE_LABEL, sizeof(PNI_PROPERTY_DL_HANDLE_LABEL) - 1, NULL, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(pni_function_ptr, PNI_PROPERTY_FUNCTIONNAME_LABEL, sizeof(PNI_PROPERTY_FUNCTIONNAME_LABEL) - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_long(pni_function_ptr, PNI_PROPERTY_RETURN_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_RETURN_DATA_TYPE_LABEL) - 1, 0, ZEND_ACC_PROTECTED TSRMLS_CC);

    /* class PNIException*/
    INIT_CLASS_ENTRY(pni, "PNIException", pni_exception_functions);
    pni_exception_ptr = zend_register_internal_class_ex(&pni, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
    
    /* class PNIDataType */
    INIT_CLASS_ENTRY(pni, "PNIDataType", pni_data_type_functions);
    pni_data_type_ptr = zend_register_internal_class_ex(&pni, NULL, NULL TSRMLS_CC);
    zend_declare_class_constant_long(pni_data_type_ptr, "VOID",     sizeof("VOID") -1 ,     PNI_DATA_TYPE_VOID TSRMLS_CC);
    zend_declare_class_constant_long(pni_data_type_ptr, "CHAR",     sizeof("CHAR") -1 ,     PNI_DATA_TYPE_CHAR TSRMLS_CC);
    zend_declare_class_constant_long(pni_data_type_ptr, "INTEGER",  sizeof("INTEGER") -1 ,  PNI_DATA_TYPE_INT TSRMLS_CC);
    zend_declare_class_constant_long(pni_data_type_ptr, "LONG",     sizeof("LONG") -1 ,     PNI_DATA_TYPE_LONG TSRMLS_CC);
    zend_declare_class_constant_long(pni_data_type_ptr, "FLOAT",    sizeof("FLOAT") -1 ,    PNI_DATA_TYPE_FLOAT TSRMLS_CC);
    zend_declare_class_constant_long(pni_data_type_ptr, "DOUBLE",   sizeof("DOUBLE") -1 ,   PNI_DATA_TYPE_DOUBLE TSRMLS_CC);
    zend_declare_class_constant_long(pni_data_type_ptr, "STRING",   sizeof("STRING") -1 ,   PNI_DATA_TYPE_STRING TSRMLS_CC);
    zend_declare_class_constant_long(pni_data_type_ptr, "POINTER",  sizeof("POINTER") -1 ,  PNI_DATA_TYPE_POINTER TSRMLS_CC);
    zend_declare_property_null(pni_data_type_ptr, PNI_PROPERTY_VALUE_LABEL, sizeof(PNI_PROPERTY_VALUE_LABEL) - 1, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pni_data_type_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIInteger */
    INIT_CLASS_ENTRY(pni, "PNIInteger", pni_integer_functions);
    pni_integer_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, "PNIDataType" TSRMLS_CC);
    zend_declare_property_long(pni_integer_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_INT, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNILong */
    INIT_CLASS_ENTRY(pni, "PNILong", pni_long_functions);
    pni_long_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, "PNIDataType" TSRMLS_CC);
    zend_declare_property_long(pni_long_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_LONG, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIChar */
    INIT_CLASS_ENTRY(pni, "PNIChar", pni_char_functions);
    pni_char_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, "PNIDataType" TSRMLS_CC);
    zend_declare_property_long(pni_char_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_CHAR, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIFloat */
    INIT_CLASS_ENTRY(pni, "PNIFloat", pni_float_functions);
    pni_float_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, "PNIDataType" TSRMLS_CC);
    zend_declare_property_long(pni_float_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_FLOAT, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIDouble */
    INIT_CLASS_ENTRY(pni, "PNIDouble", pni_double_functions);
    pni_double_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, "PNIDataType" TSRMLS_CC);
    zend_declare_property_long(pni_double_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_DOUBLE, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIString */
    INIT_CLASS_ENTRY(pni, "PNIString", pni_string_functions);
    pni_string_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, "PNIDataType" TSRMLS_CC);
    zend_declare_property_long(pni_string_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_STRING, ZEND_ACC_PROTECTED TSRMLS_CC);
    
    /* class PNIPointer */
    INIT_CLASS_ENTRY(pni, "PNIPointer", pni_pointer_functions);
    pni_pointer_ptr = zend_register_internal_class_ex(&pni, pni_data_type_ptr, "PNIDataType" TSRMLS_CC);
    zend_declare_property_long(pni_pointer_ptr, PNI_PROPERTY_DATA_TYPE_LABEL, sizeof(PNI_PROPERTY_DATA_TYPE_LABEL) - 1, PNI_DATA_TYPE_POINTER, ZEND_ACC_PROTECTED TSRMLS_CC);
    
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
    char *lib_name;
    int lib_name_len;
    zval *self, *property_dl_handle;
    void * dl_handle;
    
    /* get the parameter $libName */
    if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &lib_name, &lib_name_len) == FAILURE ) {
        WRONG_PARAM_COUNT;
    }
    ALLOC_INIT_ZVAL(property_dl_handle);
    if ( FAILURE == get_persisted_dl_handle (lib_name, &dl_handle, property_dl_handle) ) {
        return;
    } 
    /* save the libname to private variable */
    self = getThis();
    zend_update_property_stringl(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_LIBNAME_LABEL), lib_name, lib_name_len TSRMLS_CC);
    zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_DL_HANDLE_LABEL), property_dl_handle TSRMLS_CC);
}
/* }}} */


/* {{{ proto public void PNI::__call($functionName, $args)
Returns a zval pointer */
PHP_METHOD(PNI, __call) {
    zval *self,  *lib_name, *args, *res, *property_dl_handle;
    HashTable *arr_hash_tb;
    HashPosition pointer;
    zval **data = NULL;
    int index = 0;
    char * function_name;
    char * error_msg;
    int argc;
    int function_name_len = 0;
    zval *argList[MAX_PNI_FUNCTION_PARAMS];
    NATIVE_INTERFACE nativeInterface = NULL;
    char *key = NULL;
    int key_len = 0;
    void * dl_handle;
    zend_rsrc_list_entry *le, new_le;

    self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &function_name, &function_name_len, &args) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    lib_name = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_LIBNAME_LABEL), 0 TSRMLS_CC);
    property_dl_handle = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_DL_HANDLE_LABEL), 0 TSRMLS_CC);
    
    /* trans zend args to c array  */
    arr_hash_tb = Z_ARRVAL_P(args);
    argc = zend_hash_num_elements(arr_hash_tb);
    for (zend_hash_internal_pointer_reset_ex(arr_hash_tb, &pointer);
            zend_hash_get_current_data_ex(arr_hash_tb, (void**) &data, &pointer) == SUCCESS;
            zend_hash_move_forward_ex(arr_hash_tb, &pointer)) {
        argList[index] = *data;
        index ++;
    }

    ZEND_FETCH_RESOURCE(dl_handle, void *, &property_dl_handle, -1, PNI_DL_HANDLE_RES_NAME, le_dl_handle_persist);  

    /* seek dynamic library symbol */
    nativeInterface = (NATIVE_INTERFACE)dlsym(dl_handle, function_name);
    if (!nativeInterface) {
        spprintf(&error_msg, 0, "Dlsym %s error (%s). ", function_name, dlerror());
        zend_throw_exception(pni_exception_ptr, error_msg, 0 TSRMLS_CC);
        efree(error_msg);
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
    int data_type;
    char *lib_name;
    int lib_name_len;
    char *function_name;
    int function_name_len;
    
    zval *self, *property_dl_handle;
   
    void * dl_handle;
    
    /* get the parameter $libName */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lss", &data_type, &function_name, &function_name_len, &lib_name, &lib_name_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    /* get the persisted dl handle */
    ALLOC_INIT_ZVAL(property_dl_handle);
    if ( FAILURE == get_persisted_dl_handle (lib_name, &dl_handle, property_dl_handle) ) {
        return;
    } 

    self = getThis();
    /* save the libname to private variable */
    zend_update_property_stringl(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_LIBNAME_LABEL), lib_name, lib_name_len TSRMLS_CC);
    /* save the function name to private variable */
    zend_update_property_stringl(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_FUNCTIONNAME_LABEL), function_name, function_name_len TSRMLS_CC);
    /* save the data_type to private variable */
    zend_update_property_long(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_RETURN_DATA_TYPE_LABEL), data_type TSRMLS_CC);
    /* save the dl handle resource to private variable */   
    zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_DL_HANDLE_LABEL), property_dl_handle TSRMLS_CC);
}
/* }}} */

/* {{{ proto public void PNIFunction::invoke(PNIDataType arg1, ...)
 *   Call the native function.Return PNI value. Throws an PNIException. */
PHP_METHOD(PNIFunction, invoke) {
    PNI_DATA_TYPE_ANY pni_return_value;
    int pni_return_data_type;
    zval *self,  *lib_name, *function_name, *return_data_type, *res, *property_dl_handle;
    zval ***args;
    int argc;

    void * dl_handle;
    zend_rsrc_list_entry *le, new_le;
    char * error_msg;

    long long_param_list[MAX_PNI_FUNCTION_LONG_PARAMS];
    int long_param_count;
    double double_param_list[MAX_PNI_FUNCTION_DOUBLE_PARAMS];
    int double_param_count;
    NATIVE_INTERFACE_SYMBOL nativeInterface;
    PNI_DATA_TYPE_ANY *value_p;                                    
    double * double_value_p;                            
    
    self = getThis();
    lib_name = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_LIBNAME_LABEL), 0 TSRMLS_CC);
    function_name = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_FUNCTIONNAME_LABEL), 0 TSRMLS_CC);
    return_data_type = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_RETURN_DATA_TYPE_LABEL), 0 TSRMLS_CC);
    property_dl_handle = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_DL_HANDLE_LABEL), 0 TSRMLS_CC);
    
    ZEND_FETCH_RESOURCE(dl_handle, void *, &property_dl_handle, -1, PNI_DL_HANDLE_RES_NAME, le_dl_handle_persist);  

    /* seek dynamic library function symbol */
    nativeInterface = (NATIVE_INTERFACE_SYMBOL)dlsym(dl_handle, Z_STRVAL_P(function_name));
    if (!nativeInterface) {
        spprintf(&error_msg, 0, "Dlsym %s error (%s). ", Z_STRVAL_P(function_name), dlerror());
        zend_throw_exception(pni_exception_ptr, error_msg, 0 TSRMLS_CC);
        efree(error_msg);
        efree(args);
        RETURN_FALSE;
    }
    
    argc = ZEND_NUM_ARGS();
    if ( argc > 0 ) {
        args = (zval ***)safe_emalloc(argc, sizeof(zval **), 0);
        if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
            efree(args);
            WRONG_PARAM_COUNT;
        }
        trans_args_to_param_list(args, argc, long_param_list, &long_param_count, double_param_list, &double_param_count);
        efree(args);
    } else {
        long_param_count = 0;
        double_param_count = 0;
    }
    /* the key command */
    pni_return_value = call_native_interface(nativeInterface, long_param_list, long_param_count, double_param_list, double_param_count);
    pni_return_data_type = Z_LVAL_P(return_data_type);
    assign_value_to_pni_return_data(pni_return_data_type, pni_return_value, return_value);   
}
/* }}} */


/* {{{ proto public void PNIInteger::__construct($value)
 *    Constructor. Throws an PNIException in case value is illegal */
PHP_METHOD(PNIChar, __construct) {
    pni_data_factory(ht, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ proto public void PNIInteger::__construct($value)
 *    Constructor. Throws an PNIException in case value is illegal */
PHP_METHOD(PNIInteger, __construct) {
    pni_data_factory(ht, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ proto public void PNIInteger::__construct($value)
 *    Constructor. Throws an PNIException in case value is illegal */
PHP_METHOD(PNILong, __construct) {
    pni_data_factory(ht, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ proto public void PNIFloat::__construct($value)
 *    Constructor. Throws an PNIException in case value is illegal */
PHP_METHOD(PNIFloat, __construct) {
    pni_data_factory(ht, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ proto public void PNIDouble::__construct($value)
 *    Constructor. Throws an PNIException in case value is illegal */
PHP_METHOD(PNIDouble, __construct) {
    pni_data_factory(ht, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ proto public void PNIString::__construct($value)
 *    Constructor. Throws an PNIException in case value is illegal */
PHP_METHOD(PNIString, __construct) {
    pni_data_factory(ht, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ proto public void PNIPointer::__construct($value)
 *    Constructor. Throws an PNIException in case value is illegal */
PHP_METHOD(PNIPointer, __construct) {
    pni_data_factory(ht, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ proto public void PNIDataType::getValue()
 */
PHP_METHOD(PNIDataType, getValue) {
    zval * self;
    zval * value;
    self = getThis();
    value = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), 0 TSRMLS_CC);
    RETURN_ZVAL(value, 1, 0);
}

/* {{{ proto public void PNIDataType::setValue()
 */
PHP_METHOD(PNIDataType, setValue) {
    pni_data_factory(ht, getThis() TSRMLS_CC);
    RETURN_NULL();
}

/* {{{ proto public void PNI::__construct($libName)
 *    Constructor. Throws an PNIException in case the given shared library does not exist */
PHP_METHOD(PNIDataType, getDataType) {
    zval * self;
    zval * value;
    self = getThis();
    value = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_DATA_TYPE_LABEL), 0 TSRMLS_CC);
    RETURN_LONG(Z_LVAL_P(value));
}
/* }}} */

/* {{{ proto public void PNIDataType::systemFree()
 */
PHP_METHOD(PNIDataType, systemFree) {
    zval * self;
    zval * value;
    zval * zval_data_type;
    long pni_data_type;
    char * error_msg;
    self = getThis();
    value = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), 0 TSRMLS_CC);
    zval_data_type = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(PNI_PROPERTY_DATA_TYPE_LABEL), 0 TSRMLS_CC);
    pni_data_type = Z_LVAL_P(zval_data_type);
    switch ( pni_data_type ) {
        case PNI_DATA_TYPE_STRING :
        case PNI_DATA_TYPE_POINTER:
            convert_to_string(value);
            free(Z_STRVAL_P(value));
            break;
        default : 
            spprintf(&error_msg, 0, "Parameter error");
            zend_throw_exception(pni_exception_ptr, error_msg, 0 TSRMLS_CC);
            efree(error_msg);
    }
    RETURN_NULL();
}


/* }}} */

/* get the dl handle, if not exists, create and persist it */
static int get_persisted_dl_handle (char *lib_name, void ** dl_handle_ptr, zval * z) {
    char * key;
    int key_len;
    char * error_msg;
    zend_rsrc_list_entry *le, new_le;

    key_len = spprintf(&key, 0, "pni_dl_handle_%s", lib_name);
    if (zend_hash_find(&EG(persistent_list), key, key_len + 1, (void **)&le) == SUCCESS) {
        *dl_handle_ptr = le->ptr;
        ZEND_REGISTER_RESOURCE(z, *dl_handle_ptr, le_dl_handle_persist);
        efree(key);
        return SUCCESS;
    }
    
    /* init the dl handle resource */
    *dl_handle_ptr = dlopen(lib_name, RTLD_LAZY);
    if (!*dl_handle_ptr) {
        spprintf(&error_msg, 0, "%s,  dl handle resource is not created.", dlerror());
        zend_throw_exception(pni_exception_ptr, error_msg, 0 TSRMLS_CC);
        efree(error_msg);
        return FAILURE;
    }
    /* registe dl handle resource */
    ZEND_REGISTER_RESOURCE(z, *dl_handle_ptr, le_dl_handle_persist);
    /* persist dl handle */
    new_le.ptr = *dl_handle_ptr;
    new_le.type = le_dl_handle_persist;
    zend_hash_add(&EG(persistent_list), key, key_len + 1, &new_le, sizeof(zend_rsrc_list_entry), NULL);
    efree(key);
    return SUCCESS;
}


/* trans the parameters value to c array list */
static void trans_args_to_param_list(zval ***args,const int argc, 
        long * long_param_list, int *long_param_count, 
        double * double_param_list, int *double_param_count) {
    
    int long_offset = 0;
    int double_offset = 0;
    int pni_data_type;
    zval *zval_value, *zval_data_type;
    zval *argObj;
    int i;
    for (i = 0; i < argc ; i++) {
		/* Gcc only supports 8 double parameters */
		if (double_offset > MAX_PNI_FUNCTION_DOUBLE_PARAMS) {
			break;
		}
		/* PNI only supports 32 double parameters */
		if (long_offset > MAX_PNI_FUNCTION_LONG_PARAMS) {
			break;
		}
        argObj = *(args[i]);
        zval_value = zend_read_property(Z_OBJCE_P(argObj), argObj, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), 0 TSRMLS_CC);       
        zval_data_type = zend_read_property(Z_OBJCE_P(argObj), argObj, ZEND_STRL(PNI_PROPERTY_DATA_TYPE_LABEL), 0 TSRMLS_CC);
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
static void pni_dl_handle_persist_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
    void *dlHandle = (void *) rsrc->ptr;
    dlclose(dlHandle);
}

/* {{{ call the interface in the library */
static inline PNI_DATA_TYPE_ANY call_native_interface(NATIVE_INTERFACE_SYMBOL interface, 
        const long const *long_param_list,                              
        const int long_param_count,
        const double const *double_param_list,
        const int double_param_count
        ) {
    void *res;
    __asm__ __volatile__ (
            "movq %2, %%rbx\n\t"
            "cmpq $6, %3\n\t"
            "je .L_PNI_PARAM_6\n\t"
            "cmpq $5, %3\n\t"
            "je .L_PNI_PARAM_5\n\t"
            "cmpq $4, %3\n\t"
            "je .L_PNI_PARAM_4\n\t"
            "cmpq $3, %3\n\t"
            "je .L_PNI_PARAM_3\n\t"
            "cmpq $2, %3\n\t"
            "je .L_PNI_PARAM_2\n\t"
            "cmpq $1, %3\n\t"
            "je .L_PNI_PARAM_1\n\t"
            "cmpq $0, %3\n\t"
            "je .L_PNI_PARAM_0\n\t"
        ".L_PNI_PARAM_6:\n\t"
            "movq 40(%%rbx), %%r9\n\t"
        ".L_PNI_PARAM_5:\n\t"
            "movq 32(%%rbx), %%r8\n\t"
        ".L_PNI_PARAM_4:\n\t"
            "movq 24(%%rbx), %%rcx\n\t"
        ".L_PNI_PARAM_3:\n\t"
            "movq 16(%%rbx), %%rdx\n\t"
        ".L_PNI_PARAM_2:\n\t"
            "movq 8(%%rbx), %%rsi\n\t"
        ".L_PNI_PARAM_1:\n\t"
            "movq (%%rbx), %%rdi\n\t"
        ".L_PNI_PARAM_0:\n\t"
        
            "movq %4, %%rbx\n\t"
            "cmpq $8, %5\n\t"
            "je .L_PNI_DOUBLE_PARAM_8\n\t"
            "cmpq $7, %5\n\t"
            "je .L_PNI_DOUBLE_PARAM_7\n\t"
            "cmpq $6, %5\n\t"
            "je .L_PNI_DOUBLE_PARAM_6\n\t"
            "cmpq $5, %5\n\t"
            "je .L_PNI_DOUBLE_PARAM_5\n\t"
            "cmpq $4, %5\n\t"
            "je .L_PNI_DOUBLE_PARAM_4\n\t"
            "cmpq $3, %5\n\t"
            "je .L_PNI_DOUBLE_PARAM_3\n\t"
            "cmpq $2, %5\n\t"
            "je .L_PNI_DOUBLE_PARAM_2\n\t"
            "cmpq $1, %5\n\t"
            "je .L_PNI_DOUBLE_PARAM_1\n\t"
            "cmpq $0, %5\n\t"
            "je .L_PNI_DOUBLE_PARAM_0\n\t"
        ".L_PNI_DOUBLE_PARAM_8:\n\t"
            "movq 56(%%rbx), %%xmm7\n\t"
        ".L_PNI_DOUBLE_PARAM_7:\n\t"
            "movq 48(%%rbx), %%xmm6\n\t"
        ".L_PNI_DOUBLE_PARAM_6:\n\t"
            "movq 40(%%rbx), %%xmm5\n\t"
        ".L_PNI_DOUBLE_PARAM_5:\n\t"
            "movq 32(%%rbx), %%xmm4\n\t"
        ".L_PNI_DOUBLE_PARAM_4:\n\t"
            "movq 24(%%rbx), %%xmm3\n\t"
        ".L_PNI_DOUBLE_PARAM_3:\n\t"
            "movq 16(%%rbx), %%xmm2\n\t"
        ".L_PNI_DOUBLE_PARAM_2:\n\t"
            "movq 8(%%rbx), %%xmm1\n\t"
        ".L_PNI_DOUBLE_PARAM_1:\n\t"
            "movq (%%rbx), %%xmm0\n\t"
        ".L_PNI_DOUBLE_PARAM_0:\n\t"

        ".L_CALL_INTERFACE:\n\t"
            "call *%1\n\t"
            "movq %%rax, %0"
            :"=m" (res)
            :"m" (interface), "m" (long_param_list), "m" (long_param_count), "m" (double_param_list), "m" (double_param_count));

        return res;
}
/* }}} */

/* {{{ return the value according to the return data type */
static inline void assign_value_to_pni_return_data(int pni_data_type, PNI_DATA_TYPE_ANY value, zval *object TSRMLS_CC) {
    PNI_DATA_TYPE_ANY *value_p;
    double *double_value_p;
    if (!object) {
        ALLOC_ZVAL(object);
    }
    if (PNI_DATA_TYPE_VOID == pni_data_type) {
        ZVAL_NULL(object);
        return;
    }

    Z_TYPE_P(object) = IS_OBJECT;
    switch (pni_data_type) {                                
        case PNI_DATA_TYPE_CHAR :
            object_init_ex(object, pni_char_ptr);
            zend_update_property_long(Z_OBJCE_P(object), object, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), (long)value TSRMLS_CC);
            break;
        case PNI_DATA_TYPE_INT :
            object_init_ex(object, pni_integer_ptr);
            zend_update_property_long(Z_OBJCE_P(object), object, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), (long)value TSRMLS_CC);
            break;
        case PNI_DATA_TYPE_LONG :
            object_init_ex(object, pni_long_ptr);
            zend_update_property_long(Z_OBJCE_P(object), object, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), (long)value TSRMLS_CC);
            break;                                      
        case PNI_DATA_TYPE_FLOAT :                      
            object_init_ex(object, pni_float_ptr);
            value_p = &value;                           
            double_value_p = (double *)value_p;         
            zend_update_property_double(Z_OBJCE_P(object), object, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), *double_value_p TSRMLS_CC);
            break;                                      
        case PNI_DATA_TYPE_DOUBLE :                     
            object_init_ex(object, pni_double_ptr);
            value_p = &value;                           
            double_value_p = (double *)value_p;         
            zend_update_property_double(Z_OBJCE_P(object), object, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), *double_value_p TSRMLS_CC);
            break;                                      
        case PNI_DATA_TYPE_STRING :                     
            object_init_ex(object, pni_string_ptr);
            zend_update_property_string(Z_OBJCE_P(object), object, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), (char *)value TSRMLS_CC);
            break;                                      
        case PNI_DATA_TYPE_POINTER :
            object_init_ex(object, pni_pointer_ptr);
            zend_update_property_string(Z_OBJCE_P(object), object, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), (char *)value TSRMLS_CC);
            break;                                      
        default :
            ZVAL_NULL(object);
            return;
    }    
    Z_SET_REFCOUNT_P(object, 1);
    Z_SET_ISREF_P(object);
}
/* }}}*/

/* {{{ pni_data_factory */
static inline int pni_data_factory(int ht, zval *object TSRMLS_DC) {
    char * error_msg;
    zval *value;
    zval *property_data_type = zend_read_property(Z_OBJCE_P(object), object, ZEND_STRL(PNI_PROPERTY_DATA_TYPE_LABEL), 0 TSRMLS_CC);
    int pni_data_type = Z_LVAL_P(property_data_type);
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
        spprintf(&error_msg, 0, "Parameter error");
        zend_throw_exception(pni_exception_ptr, error_msg, 0 TSRMLS_CC);
        efree(error_msg);
        return FAILURE;
    }
    switch (pni_data_type) {
        case PNI_DATA_TYPE_CHAR :
        case PNI_DATA_TYPE_INT :
        case PNI_DATA_TYPE_LONG :
            convert_to_long(value);
            break;
        case PNI_DATA_TYPE_FLOAT :                      
        case PNI_DATA_TYPE_DOUBLE :                     
            convert_to_double(value);
            break;                                      
        case PNI_DATA_TYPE_STRING :                     
        case PNI_DATA_TYPE_POINTER :                    
            convert_to_string(value);
            break;                                      
        default : 
            spprintf(&error_msg, 0, "Unknow PNI data type");
            zend_throw_exception(pni_exception_ptr, error_msg, 0 TSRMLS_CC);
            efree(error_msg);
            return FAILURE;
    }
    zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL(PNI_PROPERTY_VALUE_LABEL), value TSRMLS_CC);
    return SUCCESS;
}
/* }}}*/

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
