<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('snowflake')) {
	dl('snowflake.' . PHP_SHLIB_SUFFIX);
}
if(!extension_loaded('phreads')) {
    echo 'phreads.', PHP_SHLIB_SUFFIX;
	exit();
}


class My extends Thread {
    function run(){
        for($i=1;$i<10;$i++){
            $id = snowflake::getId();
            echo "\n ThreadId: ", Thread::getCurrentThreadId() , " ", $id, " $";
        }
   
    }
}

for($i=0;$i<100;$i++){
    $pool[] = new My();
}

foreach($pool as $worker){
    $worker->start();
}
foreach($pool as $worker){
    $worker->join();
}

?>
