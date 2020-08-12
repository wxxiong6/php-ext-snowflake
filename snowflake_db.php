<?php
$br = (php_sapi_name() == "cli")? "":"<br>";
set_time_limit(0);
if(!extension_loaded('snowflake')) {
	dl('snowflake.' . PHP_SHLIB_SUFFIX);
}
$start = microtime(true);

$arr = [];
$max = 50;
for ($i = 0; $i < $max; $i++) {
	$arr[$i] = getmypid() . '-' . snowflake::getId(). '-'. microtime(true) . "\n";	
}
var_dump(microtime(true)-$start);

$dbh = new PDO("mysql:dbname=test;host=10.77.204.19", "root", "root");
$sql = 'INSERT INTO `snowflake` (`sid`, `pid`, `ts`) VALUES';
foreach ($arr as $v) {
	$fields = explode('-', $v, 3);
	$sql .= "({$fields[1]}, {$fields[0]}, {$fields[2]}),";
}
var_dump($dbh->exec(rtrim($sql, ',')));


/**
 * sql
 */

/*
CREATE TABLE `snowflake` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `sid` bigint(20) DEFAULT NULL,
  `pid` smallint(6) DEFAULT NULL,
  `ts` bigint(30) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8


TRUNCATE snowflake;

SELECT COUNT(*),pid FROM snowflake GROUP BY pid;

SELECT 
  COUNT(sid) AS cid,sid
FROM
  snowflake 
GROUP BY sid 
HAVING COUNT(*) > 1 

SELECT * FROM snowflake ORDER BY sid DESC ;

*/