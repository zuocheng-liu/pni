PHP Native Interface
===============

PHP Native Interface (PNI) is a PHP extension that enables PHP code to call and be called by native applications (programs specific to a hardware and operating system platform) and libraries written in other languages such as C, C++ and assembly.

It resembles Java Native Interface (JNI).

[简体中文版 README](README-zh.md)

##  Purpose & Features

### Situations

PNI allows programmers to use native code when an application cannot be written entirely in the PHP language. The following are typical situations where you might decide to use native code:

- You want to implement time-critical code in a lower-level, faster programming language.
- You have legacy code or code libraries that you want to access from PHP programs.
- You need platform-dependent features not supported in PHP.
- You want to call other language interface such as C/C++ assemble etc.

### Compared with PHP Extension

It is a conventional way to call C/C++ that to write PHP extension. However, PNI has multiple virtues:

- Reduce maintenance cost

It's a risk of restarting the PHP service when install or update a new PHP extension, especially while operating the PHP cluster. But with PNI, we just change the local interface library.

- Reduce development cost

Compared with developing PHP extension , developing native interface just like write native C/C++.

- Reduce learning cost

Developers has no need to learn the PHP-API, Zend-API or PHP extension framework any more. 
Data types and PNI framework are more simple.

- Scalable

Increasing native interface has no effect on current PHP service.

## Tutorial 

### Classes and methods

- PNIFunction

```php
$pow = new PNIFunction(PNIDataType::DOUBLE, 'pow', 'libm.so.6');
```
Query and localize the function in library.

Parameter 1 is the function's return data type.

Parameter 2 is the function's name.

Parameter 3 is the library's name.


- PNIException

Be throwed when library or function is not existed.

Data type class :
- PNIDataType
- PNIInteger
- PNILong
- PNIDouble
- PNICHar
- PNIString
- PNIPointer

All pni data type class are based on PNIDataType.It has 3 public methods :
```php
getValue();  // get Zend variable
setValue($value);  // reassign variable
getDataType(); // get data type constants
```
PNIString and PNIPointer have an extra method

```php
systemFree(); 
```
Free the memory which is malloced in C .
Be carefull with such libraries as they may cause memory leak .

### Predefined constants

```php
PNIDataType::VOID
PNIDataType::CHAR
PNIDataType::INTEGER
PNIDataType::LONG
PNIDataType::FLOAT
PNIDataType::DOUBLE
PNIDataType::STRING
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
// file user_math.c
u_int32_t sum(u_int32_t a, u_int32_t b) {
    return a + b;
}
```
- 2.Create the shared library file and move it to the directory which `$LD_LIBRARY_PATH` contains.
- 
```shell
gcc -fPIC -shared -o libusermath.so user_math.c
```
- 3.Create PHP code

```php
// file testPni.php
<?php
try {
    $sum = new PNIFunction(PNIDataType::INTEGER, 'sum', 'libusermath.so');
    $a = new PNIInteger(2);
    $b = new PNIInteger(10);
    $res = $sum($a, $b);
    var_dump($res);
} catch (PNIException $e) {
}
```
- 4.Run the PHP script

```shell
$ php testPni.php 
```
$res is an object of PNIInteger, its value is 12.

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

* PHP 5.3 or higher, includes PHP 7.0 
* GCC compiler
* Architecture x86_64

### Steps

- Download the code source

```shell
git clone https://github.com/zuocheng-liu/pni.git
```

- Switch to the appropriate branch according to the version of php

```shell
git checkout for_php_5  # if you installed php 5.x , exec this line.
git checkout for_php_7  # if you installed php 7.x , exec this line.
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
- [Engineering Note](http://it.zuocheng.net/engineer-note-pni-zh)

### Contacts 

- QQ Group : 297031218
- Zuocheng Liu <zuocheng.liu@gmail.com>

### License

The code for PNI is distributed under the terms of version 3.01 of the PHP license.([see LICENSE](http://php.net/license/3_01.txt))
