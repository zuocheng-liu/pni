<?php
try {
    $pni = new PNI('/root/C/libtest.so');
    var_dump($pni->PNI_pow(2.0,6.0));
    //$noPni = new PNI('/unexisted/library.so');
    //var_dump($pni->unDefinedFunction(2.0,6.0));
} catch (PNIException $e) {
    var_dump($e->getMessage());
    var_dump($e->getTraceAsString());
}
