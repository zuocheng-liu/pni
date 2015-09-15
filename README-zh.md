PHP Native Interface
===============

## 什么是 PHP Native Interface (PNI) ？

- PHP 的一个C扩展
- 通过它，可以让PHP调用其他语言写的程序，比如C/C++、汇编等等
- 需要PHP来调用，但PHP有限使用的领域里，PNI可以发挥用处，比如统计学习、神经网络、实时性要求高的程序等等

## 使用场景

PHP 不是完美的语言，总有一些情况下，不得不使用其他语言来协助完成。在这些特殊的场景下，使用PNI就可以将PHP与其他语言连接起来：

- 实时性要求特别高，特别底层的程序
- 用其他语言写的，遗留下来的，如果用PHP重新实现成本太高的程序或逻辑
- 基于平台特性的代码，不能用PHP实现的
- 调用系统的动态链接库

## Compared with PHP Extension

As most PHPers known, It is a tranditional way to call C/C++ that to write PHP extension. However, PNI has multiple virtues:

- Reduce maintenance cost

It's a risk of restarting the PHP service when install or update a new PHP extension, especially while operating the PHP cluster. But with PNI, we just change the local interface library.

- Reduce development cost

Compared with developing PHP extension , developing native interface just like write native C/C++.

- Reduce learning cost

Developers has no need to learn the PHP-API, Zend-API or PHP extension framework any more. 
Data types and PNI framework are more simple.

- Scalable

Increasing native interface has no effect on current PHP service.

## 缺点

## 使用手册 

### 类和方法列表

- PNIFunction
- PNIException
- PNIDataType
- PNIInteger
- PNILong
- PNIDouble
- PNICHar
- PNIString

### 定义的常量

```php
PNIDataType::VOID
PNIDataType::CHAR
PNIDataType::INTEGER
PNIDataType::LONG
PNIDataType::FLOAT
PNIDataType::DOUBLE
PNIDataType::POINTER
```




## 示例

### 示例 1 , 调用系统接口 :

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

### 示例 2，调用自己定义的C/C++ 逻辑 :

- 1.构建C程序

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
- 2.创建动态链接库，并把它放到 `$LD_LIBRARY_PATH` 定义的目录里
- 
```shell
php-ni -lm -o libpnimath.so pni_math.c
```
- 3.创建PHP程序

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
- 4.执行PHP程序

```shell
$ php testPni.php 
```

输出如下：

```shell

```

## PNI 数据类型和C语言数据类型对照

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

## 如何安装 

### Requirements

* PHP 5.3 or higher, PHP 7 unsupported
* GCC compiler
* Architecture x86_64

### 安装步骤

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
