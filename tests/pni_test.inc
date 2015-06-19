<?php
# PNI test framework utilities
$integer = new PNIINTEGER(1234);
$value = $integer->getValue();
var_dump($value);
$value = 3223;
var_dump($value);
var_dump($integer);
var_dump($integer->getDataType());
?>
