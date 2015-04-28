<?php
try {
    $div = new PNIFunction(PNIDataType::DOUBLE, 'PNI_div', '/root/local/lib/libpnimath.so');
    $a = new PNIDouble(100);
    $b = new PNIInteger(50);
    $res = $div->invoke($a, $b);
    var_dump($res);
} catch (PNIException $e) {
    var_dump($e->getMessage());
    var_dump($e->getTraceAsString());
}
exit;
