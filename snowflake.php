<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('snowflake')) {
	dl('snowflake.' . PHP_SHLIB_SUFFIX);
}
$arr = [];
$max = 100;
for ($i = 0; $i < $max; $i++) {
	$arr[$i] = snowflake::getId();
}
var_dump(count(array_unique($arr)));
var_dump(($arr[$max-1] - $arr[0]) == $max - 1);
var_dump(ini_get('snowflake.worker_id'));
var_dump(ini_get('snowflake.region_id'));
var_dump(ini_get('snowflake.region_bits'));
?>
