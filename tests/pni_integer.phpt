--TEST--
PNIInteger
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded('pni')) die('skip');
?>
--FILE--
<?php
try {
    $pni = new PNIInteger(123);
    var_dump($pni);     
} catch (PNIException $e) {
    echo $e->getMessage();
}
?>
--EXPECT--
