<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);
try {
    $libPath =  dirname(__FILE__) . '/libusermath.so';
    $sum = new PNIFunction(PNIDataType::INTEGER, 'sum', $libPath);
    $a = new PNIInteger(100);
    $b = new PNIInteger(10);
    $res = $sum($a, $b);
    var_dump($res);

    $sum = new PNIFunction(PNIDataType::LONG, 'mul', $libPath);
    $a = new PNILONG(100);
    $b = new PNILONG(10);
    $res = $sum($a, $b);
    var_dump($res);

} catch (PNIException $e) {
    var_dump($e->getMessage());
    var_dump($e->getTraceAsString());
}
exit;
