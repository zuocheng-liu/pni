<?php
try {
    $log = new PNIFunction(PNIDataType::INTEGER, 'PNI_log', '/root/local/lib/libpnimath.so');
    $integer = new PNIInteger(100);
    $res = $log->invoke($integer);
    var_dump($res);
} catch (PNIException $e) {
    var_dump($e->getMessage());
    var_dump($e->getTraceAsString());
}
exit;
