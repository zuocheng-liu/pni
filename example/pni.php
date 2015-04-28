<?php

try {
    $pni = new PNI('/root/local/lib/libpnimath.so');
    //var_dump($pni->getLibName());
    var_dump($pni->PNI_pow(2.0,8.0));
} catch (PNIException $e) {
    var_dump($e->getMessage());
    var_dump($e->getTraceAsString());
}
exit;
