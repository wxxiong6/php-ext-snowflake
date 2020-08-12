export USE_ZEND_ALLOC=0

export ZEND_DONT_UNLOAD_MODULES=1

#内存
#valgrind --tool=memcheck --leak-check=full  --show-leak-kinds=all --num-callers=30 --log-file=php.log  php-debug snowflake.php

#耗时采样 此时当前目录下产生文件：callgrind.out.134
valgrind --tool=callgrind  --num-callers=30 --log-file=php.log  php-debug snowflake.php
#利用valgrind callgrind_annotate获取报表，采用--inclusive=no参数
#--inclusive=yes表示报表统计的函数的时间消耗包含其内部子函数的时间消耗
valgrind callgrind_annotate --inclusive=yes callgrind.out.134