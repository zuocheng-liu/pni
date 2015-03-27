PNI - PHP Native Interface
===============

PHP Native Interface (PNI) is a PHP extension that lets PHP code use code and code libraries written in other languages, such as C and C++.
It's similar to Java Native Interface (JNI).

## Outline

- Outline
- Purpose & Features
-- Situations
-- Compared with PHP Extension
- Examples
- Installation
- Develpment
- Other

##  Purpose & Features

### Situations

PNI allows you to use native code when an application cannot be written entirely in the PHP language. The following are typical situations where you might decide to use native code:

- You want to implement time-critical code in a lower-level, faster programming language.
- You have legacy code or code libraries that you want to access from PHP programs.
- You need platform-dependent features not supported in PHP.
- You want to call other language interface such as C/C++, Python, R etc.

### Compared with PHP Extension

As all PHPers known, It is a tranditional way to call C/C++ that to write PHP extension. However, PNI has multiple virtues:

- Reduce maintenance cost

It's dangerous to restart the PHP service when we install or update a new PHP extension, especially while operating the PHP cluster. But with PNI, we just change the local interface library.

- Reduce development cost

Compared with developing PHP extension , developing native interface just like write native C/C++.

- Reduce learning cost

Developers has no need to learn the PHP-API, Zend-API or PHP extension skeleton any more. 
Data types and PNI framework are more simple.

- Flexible

PHP-API and Zend API are also available in native interface.

- Scalable

Increasing native interface has no effect on current PHP service.

## Pitfalls

## Examples

### 1.Write the C/C++ code
```C++
// file pni_math.c
#include<math.h>
#include "php.h"
zval *PNI_pow(zval **args) {
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
```
### 2.Create the shared library file

```shell
phpni -lm -o libpnimath.so pni_math.c
```
### 3.Create PHP Code

```php
// file testPni.php
$pniFunction = new PNIFunction('PNI_pow','libpnimath.so');
$a = 2.0;
$b = 2.5;
var_dump($pniFunction->invokeArgs(array($a, $b)));
var_dump($pniFunction->invoke($a, $b));
var_dump($pniFunction->getLibName());
var_dump($pniFunction->getFunctionName());
```
### 4.Run the PHP script

```shell
$ php testPni.php 
```

the output

```shell
float(5.6568542494924)
float(5.6568542494924)
string(13) "libpnimath.so"
string(7) "PNI_pow"
```

## Requirements

* PHP 5.3 or higher,
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
service php-fpm restart
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
