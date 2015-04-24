--TEST--
PNIInteger
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded('pni')) die('skip');
?>
--FILE--
<?php
try {
    $a = new PNIInteger(123);
    var_dump($a);     
} catch (PNIException $e) {
    echo $e->getMessage();
}
?>
--EXPECT--
