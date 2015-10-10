PHP Native Interface
===============

PHP Native Interface (PNI) is a PHP extension that enables PHP code to call and be called by native applications (programs specific to a hardware and operating system platform) and libraries written in other languages such as C, C++ and assembly.

It resembles Java Native Interface (JNI).

[简体中文版README](README-zh.md)

##  Purpose & Features

### Situations

PNI allows programmers to use native code when an application cannot be written entirely in the PHP language. The following are typical situations where you might decide to use native code:

- You want to implement time-critical code in a lower-level, faster programming language.
- You have legacy code or code libraries that you want to access from PHP programs.
- You need platform-dependent features not supported in PHP.
- You want to call other language interface such as C/C++ assemble etc.

### Compared with PHP Extension

As most PHPers known, It is a conventional way to call C/C++ that to write PHP extension. However, PNI has multiple virtues:

- Reduce maintenance cost

It's a risk of restarting the PHP service when install or update a new PHP extension, especially while operating the PHP cluster. But with PNI, we just change the local interface library.

- Reduce development cost

Compared with developing PHP extension , developing native interface just like write native C/C++.

- Reduce learning cost

Developers has no need to learn the PHP-API, Zend-API or PHP extension framework any more. 
Data types and PNI framework are more simple.

- Scalable

Increasing native interface has no effect on current PHP service.

## Pitfalls

## Tutorial 

### Classes and methods

- PNIFunction
- PNIException
- PNIDataType
- PNIInteger
- PNILong
- PNIDouble
- PNICHar
- PNIString

### Predefined constants

```php
PNIDataType::VOID
PNIDataType::CHAR
PNIDataType::INTEGER
PNIDataType::LONG
PNIDataType::FLOAT
PNIDataType::DOUBLE
PNIDataType::POINTER
```




## Examples

### Example 1 , call system function :

```php
try {
    $pow = new PNIFunction(PNIDataType::DOUBLE, 'pow', 'libm.so.6');
    $a = new PNIDouble(2);
    $b = new PNIDouble(10);
    $res = $pow($a, $b);
    var_dump($res);
} catch (PNIException $e) {
}
```

### Example 2 :

- 1.Write the C/C++ code

```C++
// file pni_math.c
#include<math.h>
#include "php.h"

/* 
 * double pow(double x, double y); 
 * every PNI function returns zval(php variable) , the paramters are in the args
 */
zval *PNI_pow(zval **args, int argc) {
    zval *tmp, *res;
    double x,y,z;
    tmp = args[0];     
    x = Z_DVAL_P(tmp);  // get the double value via Z_DVAL_P
    tmp = args[1];
    y = Z_DVAL_P(tmp); // Why we write it like this instead of `y = Z_DVAL_P(args[1]);`? It's a C Trap.
    
    z = pow(x,y);    // Function pow is the target.
    ALLOC_INIT_ZVAL(res);  //  It's essential to init return value unless the return value is NULL.
    ZVAL_DOUBLE(res, z);   // Use ZVAL_DOUBLE to assign the result to the return variable，the data type is double.
    return res;
}
```
- 2.Create the shared library file and move it to the directory which `$LD_LIBRARY_PATH` contains.
- 
```shell
php-ni -lm -o libpnimath.so pni_math.c
```
- 3.Create PHP code

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
- 4.Run the PHP script

```shell
$ php testPni.php 
```

the output as below

```shell

```

## PNI Data Type Map

PNI data type class  | C data type | remark
------------| ----------	| ----------
PNILong   	| long int/ int	| PHP has no unsigned int 
PNIInteger  | long int/ int | PHP has no 32bit Int
PNIDouble  	| double / float| 
PNIFloat  	| double / float| PHP has no 32bit float
PNIChar  	| char 			| 
PNIString  	| char* 		|
PNIPointer  | char* 		|

Does PNI really make sense? Yes. Believe me.  PNI has less data types than C,but int and long int are stored in the same type, 64bit CPU register when a function is called. So as float and double.

## Installation 

### Requirements

* PHP 5.3 or higher, PHP 7 unsupported
* GCC compiler
* Architecture x86_64

### Steps

- Download the code source

```shell
git clone https://github.com/zuocheng-liu/pni.git
```
- Complie the pni extension code and install

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
