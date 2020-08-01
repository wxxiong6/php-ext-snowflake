--TEST--
Check for snowflake unique
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--FILE--
<?php
$arr = [];
$max = 100000;
for ($i = 0; $i < $max; $i++) {
	$id = snowflake::getId();
	$arr[$id] = '';
}
var_dump(count($arr));
?>
--EXPECT--
int(100000)