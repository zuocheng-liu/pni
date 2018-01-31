<?php
try {
    $libPath =  'libm.so.6';
    $pow = new PNIFunction(PNIDataType::DOUBLE, 'pow', $libPath);
    $a = new PNIDouble(2);
    $b = new PNIDouble(10);
    var_dump($a);
    var_dump($b);
    var_dump($pow);
    $res = $pow($a, $b);
    var_dump($res);
} catch (PNIException $e) {
  var_dump($e->getMessage());
  var_dump($e->getTraceAsString());
}
