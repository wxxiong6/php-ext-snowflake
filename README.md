# php-ext-snowflake

[![Build Status](https://travis-ci.org/wxxiong6/php-ext-snowflake.svg?branch=master)](https://travis-ci.org/wxxiong6/php-ext-snowflake)

[Twitter SnowFlake](https://github.com/twitter-archive/snowflake, "Twitter SnowFlake") PHP Extension

## Requirement
php-5.0 +

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
snowflake::getId();
//3373092110074113
```
