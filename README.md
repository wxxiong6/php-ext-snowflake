# php-ext-snowflake

[![Build Status](https://travis-ci.org/wxxiong6/php-ext-snowflake.svg?branch=master)](https://travis-ci.org/wxxiong6/php-ext-snowflake)

网上看过很多php版本的snowflake算法包括c写的扩展，基本上都有问题,php是多进程运行模式。在高发时会产生重复的id。本扩展使用高效的```自旋锁``保证唯一性，也兼顾性能。想要不依赖其他中间件实现分布式唯一id，这个扩展是一个非常不错的选择。


- 基于[Twitter SnowFlake](https://github.com/twitter-archive/snowflake "Twitter SnowFlake")分布式ID生成算法,使用c实现的php Extension。
- 默认生成ID是一个64位long型数字。
- 单机每秒内理论上最多可以生成1024*(2^12)，也就是409.6万个ID(1024 X 4096 = 4194304)。
- 本机测试生成100万个ID,耗时0.24624609947205秒。
- 可根据自身情况调整bit位数，从而生成长度合适的ID。
- 支持php8,充分发挥php8的优势。具体看下面两个版执行的例子

|1 标识位|41 时间截(毫秒)|5 数据中心ID |5 机器ID |12 序列 |
|:-:|-|-|-|-|
|0|0000000000 0000000000 0000000000 0000000000 0| 00000| 00000 | 000000000000 |


## Requirement
php-7.2 +

## Install
# Compile snowflake in Linux mac

```shell
phpize
./configure --enable-snowflake
make
make install
```

## Document
```php
SnowFlake::getId();
```

## php.ini 

```ini
[SnowFlake]
;数据中心id,根据实现情况修改
snowflake.region_id = 1
;机器ID,根据实现情况修改
snowflake.worker_id = 1
;起始时间截(毫秒级)
snowflake.epoch = 1576080000000
;时间截(毫秒级) 默认值41
snowflake.time_bits = 41
;数据中心ID 默认值5
snowflake.region_bits = 5
;机器ID 默认值5
snowflake.worker_bits = 5
;序列 默认值12
snowflake.sequence_bits = 12
```
## php版本性能对比  (测试脚本php snowflake.php)
|Testing size|php-8.0|php-7.4|
|:--:| --------------- | ------------- |
| 100W | 0.245499849319458 | 1.2443881034851 |
```