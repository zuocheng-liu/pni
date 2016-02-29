PHP Native Interface
===============

## 什么是 PHP Native Interface (PNI) ？

- PHP 的一个C扩展
- 通过它，可以让PHP调用其他语言写的程序，比如C/C++、汇编等等
- 需要PHP来调用，但PHP有限使用的领域里，PNI可以发挥用处，比如图像处理、统计学习、神经网络、实时性要求高的程序等等

## 使用场景

PHP 不是完美的语言，总有一些情况下，不得不使用其他语言来协助完成。在这些特殊的场景下，使用PNI就可以将PHP与其他语言连接起来：

- 实时性要求特别高的程序，特别底层的程序
- 用其他语言写的程序，历史遗留下来的程序，如果用PHP重新实现成本太高的程序或逻辑
- 基于平台特性的代码，不能用PHP实现的程序
- 调用系统的动态链接库

## 与直接编写PHP扩展相比

直接编写PHP扩展去调用其他语言的接口是常用方法，不过PNI有更多的好处：

- 降低开发和运维成本

不需要每次有新的需求，就去编写或改动PHP的扩展。对PHP扩展的开发、调试会占用很多的时间。

PHP扩展更改后上线，需要重启PHP服务，这是有一定风险的。

如果使用PNI，就会便捷很多，对新功能的开发和上线，只需操作PHP的代码即可。

- 降低学习成本

开发PHP扩展，需要开发人员去学习 PHP-API、 Zend-API 、 PHP扩展框架，甚至需要深入去理解PHP内核。
有了PNI，问题就简单多了。

- 灵活性

使用PNI，可以更灵活地使用本地类库。

## 使用手册 

### 类和方法列表

- PNIFunction

方法类，此类定位动态链接库中的函数名
```php
$pow = new PNIFunction(PNIDataType::DOUBLE, 'pow', 'libm.so.6');
```
上面的例子，在构造函数中，第一个参数是需要找寻函数的返回值类型，第二参数是函数的名字，第三个参数是到那个动态链接库中找寻函数。

- PNIException

异常类，在无法找到动态链接库或函数名的时候，会抛出异常。

数据类型类

- PNIDataType
- PNIInteger
- PNILong
- PNIDouble
- PNIFLOAT
- PNIChar
- PNIString
- PNIPointer

所有数据类型类都继承PNIDataType抽象类，此抽象类包含两个共有方法
```php
getValue(); // 获取值
getDataType(); // 获取数据类型
```
PNIString 和 PNIPointer 中还额外包含一个接口

```php
systemFree(); 
```
用于释放C函数中malloc申请的内存资源。
强烈不推荐使用PNI直接调用这样的库函数。

### 定义的常量

表示数据类型常量
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

上面例子，使用PNI调用系统math库中的pow函数

### 示例 2，调用自己定义的C/C++ 逻辑 :

- 1.构建C程序

```C++
// file user_math.c
u_int32_t sum(u_int32_t a, u_int32_t b) {
    return a + b;
}
```
- 2.创建动态链接库，并把它放到 `$LD_LIBRARY_PATH` 包含的目录里
```shell
gcc -fPIC -shared -o libusermath.so user_math.c
```
- 3.创建PHP程序

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
- 4.执行PHP程序

```shell
$ php testPni.php 
```
$res 是 PNIInteger类型，其中包含数值结果为12的成员变量
## PNI 数据类型类和C语言数据类型对照

PNI 数据类型类  | C 数据类型 | 说明
------------| ----------	| ----------
PNILong   	| long int/ int	| PHP has no unsigned int 
PNIInteger  | long int/ int | PHP has no 32bit Int
PNIDouble  	| double / float| 
PNIFloat  	| double / float| PHP has no 32bit float
PNIChar  	| char 			| 
PNIString  	| char* 		|
PNIPointer  | char* 		|

由于PHP只有64整形，所以PNILong 和 PNIInteger 实际上是等效的。

如果通过PNI调用的函数参数类型是32位、16位数据怎么办？需要开发人员保证PNILong和PNIInteger存放的值不能超出大小。

PNIDouble 和 PNIFloat 也是等效的,因为PHP只有64位浮点。如果调用的C函数参数列表里有32位浮点呢? 不用担心，即使是32位的浮点，在x86_64架构的CPU里，也是赋给了64位的浮点运算器。

## 缺点或注意事项

- 目前还不支持PHP7 ，但作者会争取尽快开发适用PHP7版本的PNI
- 如果PHP是多线程运行，需要注意PNI调用的动态链接库是否是线程安全的
- 对于在动态链接库中申请的资源，要及时释放
- 目前PNI还不支持对复杂数据类型的操作，比如struct，C++的类等

## 如何安装 

### 环境要求

* PHP 5.3 以上版本, 但不包含PHP 7 
* 必须是GCC编译器
* CPU 必须是x86_64架构或被兼容的架构

### 安装步骤

- 下载

```shell
git clone https://github.com/zuocheng-liu/pni.git
```
- 编译和安装

```shell
cd <src-pni>
phpize
./configure
make && make install
```
- 配置PHP，使其生效

把下面一行添加到 php.ini

```shell
extension=pni.so;
```
- 重启PHP服务

```bash
service php-fpm restart  // cgi mode
apachectl restart   // sapi mode 
// do nothing in cli mode
```
## 开发

### 提出建议和提交Bug

热盼您的联系！

## 其他

### 网址

- [源代码](https://github.com/zuocheng-liu/pni)
- [开发笔记](http://it.zuocheng.net/engineer-note-pni-zh)

### 联系方式 

- QQ群 : 297031218  
- Zuocheng Liu <zuocheng.liu@gmail.com>
- [新浪微博](http://weibo.com/zuocheng1990)

### 协议

version 3.01 of the PHP license.([see LICENSE](http://php.net/license/3_01.txt))
