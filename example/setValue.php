<?php
$int = new PNILong(66);
var_dump($int);
$int->setValue(88.88);
var_dump($int);

$double = new PNIDouble(66.66);
var_dump($double);
$double->setValue(88.88);
var_dump($double);

$string = new PNIString(66.66);
var_dump($string);
$string->setValue(88.88);
var_dump($string);
