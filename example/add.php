<?php
try {
    $add = new PNIFunction(PNIDataType::LONG, 'PNI_add', '/root/local/lib/libpnimath.so');
    $a = new PNILong(100);
    $b = new PNILong(10);
    $res = $add->invoke($a, $b);
    var_dump($res);
} catch (PNIException $e) {
    var_dump($e->getMessage());
    var_dump($e->getTraceAsString());
}
exit;
