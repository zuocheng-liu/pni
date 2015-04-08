#include<math.h>
#include "php.h"

zval *PNI_pow(zval **args, int argc) {
    double x,y,z;
    zval *tmp = NULL; 
    zval *res = NULL; 
    tmp = args[0];
    x = Z_DVAL_P(tmp);
    tmp = args[1];
    y = Z_DVAL_P(tmp);
    
    z = pow(x,y);
    
    ALLOC_INIT_ZVAL(res);
    ZVAL_DOUBLE(res, z);
    return res;
}
