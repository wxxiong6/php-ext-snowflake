<?php
$br = (php_sapi_name() == "cli")? "":"<br>";
set_time_limit(0);
if(!extension_loaded('snowflake')) {
	dl('snowflake.' . PHP_SHLIB_SUFFIX);
}
$start = microtime(true);

$max = 1000000;
for ($i = 0; $i < $max; $i++) {
	Snowflake::getId();
}

var_dump(microtime(true)-$start);





