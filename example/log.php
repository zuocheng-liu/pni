<?php
try {
    $log = new PNIFunction(PNIDataType::DOUBLE, 'log10', 'libm.so.6');
    $a = new PNIDouble(100000000000000000000000000);
    $b = new PNIDouble(10);
    $res = $log->invoke($a);
    var_dump($res);
    $res = $log->invoke($b);
    var_dump($res);
    var_dump($log->invoke($a)/$log->invoke($b));
} catch (PNIException $e) {
    var_dump($e->getMessage());
    var_dump($e->getTraceAsString());
}
exit;
