--TEST--
Check for snowflake serial
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--FILE--
<?php
$arr = [];
$max = 1024;
for ($i = 0; $i < $max; $i++) {
	$arr[$i] = snowflake::getId();
}

var_dump(($arr[$max-1] - $arr[0]) == ($max-1));
?>
--EXPECT--
bool(true)