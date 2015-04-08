--TEST--
PNI Class
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded('pni')) die('skip');
?>
--FILE--
<?php
try {
 $pni = new PNI('adfsfsfafxx1234.so');      
} catch (PNIException $e) {
    echo $e->getMessage();
}
?>
--EXPECT--
adfsfsfafxx1234.so: cannot open shared object file: No such file or directory,  dl handle resource is not created.
