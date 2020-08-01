<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('snowflake')) {
	dl('snowflake.' . PHP_SHLIB_SUFFIX);
}
$start = microtime(true);

$arr = [];
$max = 1000000;
for ($i = 0; $i < $max; $i++) {
	// $arr[$i] = snowflake::getId();
	snowflake::getId();	
}
// foreach ($arr as $k => $v) {
// 	echo $k, " " , $v, PHP_EOL;
// }
// var_dump($start);
// var_dump(microtime(true));
var_dump(microtime(true)-$start);