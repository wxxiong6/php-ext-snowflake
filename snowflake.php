<?php
$br = (php_sapi_name() == "cli")? "":"<br>";
set_time_limit(0);
if(!extension_loaded('snowflake')) {
	dl('snowflake.' . PHP_SHLIB_SUFFIX);
}
$start = microtime(true);

$arr = [];
$max = 10;
for ($i = 0; $i < $max; $i++) {
	$arr[$i] = getmypid() . '-' . snowflake::getId(). "\n";
	//snowflake::getId();	
}
// foreach ($arr as $k => $v) {
// 	echo $k, " " , $v, PHP_EOL;
// }
 var_dump($start);
// var_dump(microtime(true));

var_dump(microtime(true)-$start);
var_dump($arr);


$dbh = new PDO("mysql:dbname=test;host=10.77.204.19", "root", "root");
$sql = 'INSERT INTO `snowflake` (`sid`, `pid`) VALUES';
foreach ($arr as $v) {
	$feilds = explode('-', $v, 2);
	$sql .= "({$feilds[1]}, {$feilds[0]}),";
}
var_dump($dbh->exec(rtrim($sql, ',')));

//file_put_contents('1.txt', $arr, FILE_APPEND);