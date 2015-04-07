PHP Native Interface
===============

PHP Native Interface (PNI) is a PHP extension that enables PHP code to call and be called by native applications (programs specific to a hardware and operating system platform) and libraries written in other languages such as C, C++ and assembly..

It resembles Java Native Interface (JNI).

## Outline

- Outline
- Purpose & Features
- Pitfalls
- Tutorial & Examples
- Installation
- Develpment
- Other

##  Purpose & Features

### Situations

PNI allows programmers to use native code when an application cannot be written entirely in the PHP language. The following are typical situations where you might decide to use native code:

- You want to implement time-critical code in a lower-level, faster programming language.
- You have legacy code or code libraries that you want to access from PHP programs.
- You need platform-dependent features not supported in PHP.
- You want to call other language interface such as C/C++ assemble etc.

### Compared with PHP Extension

As all PHPers known, It is a tranditional way to call C/C++ that to write PHP extension. However, PNI has multiple virtues:

- Reduce maintenance cost

It's a risk of restarting the PHP service when install or update a new PHP extension, especially while operating the PHP cluster. But with PNI, we just change the local interface library.

- Reduce development cost

Compared with developing PHP extension , developing native interface just like write native C/C++.

- Reduce learning cost

Developers has no need to learn the PHP-API, Zend-API or PHP extension framework any more. 
Data types and PNI framework are more simple.

- Flexible

PHP-API and Zend API are also available in native interface.

- Scalable

Increasing native interface has no effect on current PHP service.

## Pitfalls

## Tutorial & Examples

### 1.Write the C/C++ code
```C++
// file pni_math.c
#include<math.h>
#include "php.h"

/* 
 * double pow(double x, double y); 
 */
zval *PNI_pow(zval **args) {  // every PNI function returns zval(php variable) , the paramters are in the args
    double x,y,z;
    zval *tmp = NULL;
    zval *res = NULL;
    tmp = args[0];     
    x = Z_DVAL_P(tmp);  // get the double value via Z_DVAL_P
    tmp = args[1];
    y = Z_DVAL_P(tmp); // Why we write it like this instead of `y = Z_DVAL_P(args[1]);`? It's a C Trap.
    
    z = pow(x,y);    // 
    ALLOC_INIT_ZVAL(res);  //  It's essential to init return value unless the return value is NULL.
    ZVAL_DOUBLE(res, z);   // Use ZVAL_DOUBLE to assign the result to the return variable， the data type is double.
    return res;
}
```
### 2.Create the shared library file and move it to the directory which `$LD_LIBRARY_PATH` contains.
```shell
php-ni -lm -o libpnimath.so pni_math.c
```
### 3.Create PHP code

```php
// file testPni.php
<?php
try {
    $pni = new PNI('libpnimath.so');
    var_dump($pni->PNI_pow(2.0,6.0));
    $noPni = new PNI('/unexisted/library.so');
    var_dump($pni->unDefinedFunction(2.0,6.0));
} catch (PNIException $e) {
    var_dump($e->getMessage());
    var_dump($e->getTraceAsString());
}

```
### 4.Run the PHP script

```shell
$ php testPni.php 
```

the output as below

```shell
float(64)
string(154) "Dlopen /unexisted/library.so error (/unexisted/library.so: cannot open shared object file: No such file or directory),  dl handle resource is not created."
string(69) "#0 /root/pni.php(5): PNI->__construct('/unexisted/libr...')
#1 {main}"
```

### How to get data from zval?

All the operator macros are defined in the Zend APIs. 

```c
Z_LVAL_P(zval_p)   // get Long(no int)
Z_BVAL_P(zval_p)   // get Boolean
Z_DVAL_P(zval_p)   // get Double
Z_STRVAL_P(zval_p) // get char *
Z_STRLEN_P(zval_p) // get the length of a string / Long
```
### How to assign a value to the return variable

Thus the PNI function return variable is zval,first of all, you need to initialise it by using  `ALLOC_INIT_ZVAL(res)`. And then, assign the value to it.

```
ZVAL_NULL(z)      // assign NULL
ZVAL_LONG(z, l)    // assign LONG
ZVAL_STRING(z, s, duplicate)     //assign a string/char * . Duplicate ? allways be 1.
ZVAL_STRINGL(z, s, l, duplicate) //assign a string with fixed length. Duplicate ? the same as above.
ZVAL_FALSE(z)
ZVAL_TRUE(z)
ZVAL_BOOL(z, boolean)    // ZVAL_BOOL(z, 1) and ZVAL_TRUE(z) are the same.Likely, ZVAL_BOOL(z, 0) and ZVAL_FALSE(z) are the same.
```

It's unnecessary to know more about Zend APIs or PHP APIs. All referred above is ample to help us achieve the simple communication between PHP code and C code. 

## Requirements

* PHP 5.3 or higher, PHP 5.2 untested
*  *NIX Platform 
* windows untested.

## Installation 

- Download the code source

```shell
git clone https://github.com/zuocheng-liu/pni.git
```
- Complie the pni extension code

```shell
cd <src-pni>
phpize
./configure
make && make install
```
- Make PNI work

add the line below to php.ini

```shell
extension=pni.so;
```
- Restart PHP service

```bash
service php-fpm restart  // cgi mode
apachectl restart   // sapi mode 
// do nothing in cli mode
```
## Development

### Reporting bugs and contributing code

Contributions to PNI are highly appreciated either in the form of pull requests for new features, bug fixes, or just bug reports.

## Other

### Related links

- [Source code](https://github.com/zuocheng-liu/pni)

### Author 

- Zuocheng Liu <zuocheng.liu@gmail.com>

### License

The code for PNI is distributed under the terms of version 3.01 of the PHP license.([see LICENSE](http://php.net/license/3_01.txt))
