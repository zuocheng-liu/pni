<?php
try {
    $mul = new PNIFunction(PNIDataType::DOUBLE, 'PNI_mul', '/root/local/lib/libpnimath.so');
    $a = new PNIDouble(100);
    $b = new PNIDouble(50);
    $res = $mul->invoke($a, $b);
    var_dump($res);
} catch (PNIException $e) {
    var_dump($e->getMessage());
    var_dump($e->getTraceAsString());
}
exit;
