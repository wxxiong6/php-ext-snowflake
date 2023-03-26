# php-ext-snowflake

[![Build Status](https://api.travis-ci.org/wxxiong6/php-ext-snowflake.svg?branch=master)](https://travis-ci.org/wxxiong6/php-ext-snowflake) [![Build status](https://ci.appveyor.com/api/projects/status/awii6wf2ocmy202p/branch/master?svg=true)](https://ci.appveyor.com/project/wxxiong6/php-ext-snowflake/branch/master)


扩展使用Snowflake算法实现分布式唯一ID生成器的PHP扩展。它可以生成全局唯一且有序的ID。

PHP多进程环境下确保ID唯一性的确是一个挑战，因为多个进程可能同时尝试生成ID，这可能导致ID重复。在网上看到很多php扩展实现的snowflake都没有解决多进程id重复的问题, 本扩展使用自旋锁确保唯一它可以避免多个进程同时生成ID的情况。

如果您需要在分布式系统中生成唯一ID，而不依赖于其他中间件，本扩展是一种可行的解决方案，因为它可以生成全局唯一的ID，并且不需要依赖其他中间件。

总的来说，使用Snowflake算法实现分布式唯一ID生成器的PHP扩展是一个不错的选择，特别是对于需要在高并发环境下生成ID的应用程序来说。但是请确保正确地配置和使用扩展，并测试其在您的环境中的性能和可靠性。


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
