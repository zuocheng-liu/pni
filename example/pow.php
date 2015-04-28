<?php
try {
    $pow = new PNIFunction(PNIDataType::DOUBLE, 'pow', 'libm.so.6');
    $a = new PNIDouble(2);
    $b = new PNIDouble(10);
    $res = $pow($a, $b);
    var_dump($res);
    var_dump($res);
    var_dump($res);
} catch (PNIException $e) {
}
