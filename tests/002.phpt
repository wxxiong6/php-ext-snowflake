--TEST--
Check for snowflake unique
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--FILE--
<?php
$arr = [];
for ($i = 0; $i < 100; $i++) {
	$arr[$i] = snowflake::getId();
}
var_dump(count(array_unique($arr)));
?>
--EXPECT--
int(100)