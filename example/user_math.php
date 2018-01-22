<?php
#error_reporting(E_ALL);
ini_set("error_reprorting", "E_ALL");
ini_set("display_errors", "On");
#ini_set("log_errors", "On");
#ini_set("error_log", "/opt/lampp/htdocs/error_log.log");   //´Ë·¾¶×ÐÅÖ
$i = 0;
while ($i++ < 1) {
    try {
        $libPath =  dirname(__FILE__) . '/libusermath.so';
        $sum = new PNIFunction(PNIDataType::INTEGER, 'sum', $libPath);
        //var_dump($sum);
        $a = new PNIInteger(100);
        $b = new PNIInteger(10);
        $res = $sum($a, $b);
        var_dump($res);
        //unset($sum);
    } catch (PNIException $e) {
        var_dump($e->getMessage());
        var_dump($e->getTraceAsString());
    }
}

echo "done!";
exit;
