/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_snowflake.h"
#include <sys/time.h>


/* True global resources - no need for thread safety here */
static int le_snowflake;

static snowflake sf;
zend_class_entry *snowflake_ce;
/* {{{ PHP_INI
 

PHP_INI_BEGIN()
    // STD_PHP_INI_ENTRY("snowflake.worker_id",     SNOWFLAKE_WORKER_ID,     PHP_INI_SYSTEM, OnUpdateLongGEZero, worker_id,     zend_snowflake_globals, snowflake_globals)
    // STD_PHP_INI_ENTRY("snowflake.region_id",     SNOWFLAKE_REGION_ID,     PHP_INI_SYSTEM, OnUpdateLongGEZero, region_id,     zend_snowflake_globals, snowflake_globals)
	// STD_PHP_INI_ENTRY("snowflake.epoch",         SNOWFLAKE_EPOCH,         PHP_INI_SYSTEM, OnUpdateLongGEZero, epoch,         zend_snowflake_globals, snowflake_globals)
	// STD_PHP_INI_ENTRY("snowflake.region_bits",   SNOWFLAKE_REGIONID_BITS, PHP_INI_SYSTEM, OnUpdateLongGEZero, region_bits,   zend_snowflake_globals, snowflake_globals)
	
    // STD_PHP_INI_ENTRY("snowflake.worker_bits",   SNOWFLAKE_WORKERID_BITS, PHP_INI_SYSTEM, OnUpdateLongGEZero, worker_bits,   zend_snowflake_globals, snowflake_globals)
	// STD_PHP_INI_ENTRY("snowflake.sequence_bits", SNOWFLAKE_SEQUENCE_BITS, PHP_INI_SYSTEM, OnUpdateLongGEZero, sequence_bits, zend_snowflake_globals, snowflake_globals)
	// STD_PHP_INI_ENTRY("snowflake.time_bits",     SNOWFLAKE_TIME_BITS,     PHP_INI_SYSTEM, OnUpdateLongGEZero, time_bits,     zend_snowflake_globals, snowflake_globals)
PHP_INI_END()
 */
/* }}} */

static inline zend_long timestamp_gen()
{
    struct  timeval    tv;
    gettimeofday(&tv, NULL);
    return (zend_long)tv.tv_sec * 1000 + tv.tv_usec / 1000; 
} 

static zend_long snowflake_id(snowflake *sf) 
{
    zend_long millisecs = timestamp_gen();
    zend_long id = 0LL;

    if (millisecs == sf->time)
    {
        sf->seq = (sf->seq + 1) & sf->seq_max;
        if (sf->seq == 0) 
        {
            while (millisecs <= timestamp_gen())
            {
                millisecs = timestamp_gen();
            }
        }
    } else if (millisecs > sf->time) {
        sf->seq = 1;
    } else {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "epoch in the range of 0, %lld", millisecs);
    }
 
    id = ((millisecs-sf->epoch) << sf->time_bits) 
            | (sf->region_id << sf->region_bits) 
            | (sf->worker_id << sf->worker_bits) 
            | (sf->seq); 


    sf->time = millisecs;

    return id;
}

static int snowflake_init(snowflake *sf) 
{
    int region_id = SNOWFLAKE_REGION_ID;
    int max_region_id = (1 << SNOWFLAKE_REGIONID_BITS) - 1;
    if(region_id < 0 || region_id > max_region_id){
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Region ID must be in the range : 0-%d", max_region_id);
        return -1;
    }
    int worker_id = SNOWFLAKE_WORKER_ID;
    int max_worker_id = (1 << SNOWFLAKE_WORKERID_BITS) - 1;
    if(worker_id < 0 || worker_id > max_worker_id){
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Worker ID must be in the range: 0-%d", max_worker_id);
        return -1;
    }

    
    /* 
    sf->time_bits   = SF_G(region_bits) + SF_G(worker_bits) + SF_G(sequence_bits);
    sf->region_bits = SF_G(worker_bits) + SF_G(sequence_bits);
    sf->worker_bits = SF_G(sequence_bits);
    */
    sf->time_bits   = SNOWFLAKE_REGIONID_BITS + SNOWFLAKE_WORKERID_BITS + SNOWFLAKE_SEQUENCE_BITS;
    sf->region_bits = SNOWFLAKE_WORKERID_BITS + SNOWFLAKE_SEQUENCE_BITS;
    sf->worker_bits = SNOWFLAKE_SEQUENCE_BITS;

    sf->epoch       = SNOWFLAKE_EPOCH; 
    sf->worker_id    = worker_id;
    sf->region_id    = region_id;
    sf->seq_max      = (1 << SNOWFLAKE_SEQUENCE_BITS) - 1;
    sf->seq          = 0;
    sf->time         = 0LL;
    return 1;
}

/* {{{ php_snowflake_init_globals
 */

static void php_snowflake_init_globals(zend_snowflake_globals *snowflake_globals)
{

   snowflake_globals->region_id =  SNOWFLAKE_REGION_ID;
   snowflake_globals->epoch     =  SNOWFLAKE_EPOCH;
   snowflake_globals->region_bits =  SNOWFLAKE_REGIONID_BITS;
   snowflake_globals->worker_bits   =  SNOWFLAKE_WORKERID_BITS;
   snowflake_globals->sequence_bits = SNOWFLAKE_SEQUENCE_BITS;
   snowflake_globals->time_bits     =  SNOWFLAKE_TIME_BITS;
}
/* }}} */


PHP_METHOD(snowflake, getId)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
    zend_long id = snowflake_id(&sf);
    if (id) {
        RETURN_LONG(id);
    } else {
        RETURN_BOOL(0);
    }
}


/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(snowflake)
{
	/*UNREGISTER_INI_ENTRIES();	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(snowflake)
{
#if PHP_MAJOR_VERSION == 7
#if defined(COMPILE_DL_SNOWFLAKE) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(snowflake)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(snowflake)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "snowflake support", "enabled");
	php_info_print_table_end();
    
	/* Remove comments if you have entries in php.ini */
	DISPLAY_INI_ENTRIES();
	
}
/* }}} */

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO(arginfo_snowflake_void, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ snowflake_functions[]
 *
 * Every user visible function must have an entry in snowflake_functions[].
 */
const zend_function_entry snowflake_functions[] = {
	PHP_FE_END	/* Must be the last line in snowflake_functions[] */
};
/* }}} */

/* {{{ snowflake_method[]
 *
 * Every user visible function must have an entry in snowflake_functions[].
 */
static const zend_function_entry snowflake_methods[] = { 
	PHP_ME(snowflake, getId,        arginfo_snowflake_void,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};
/* }}} */


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(snowflake)
{
	zend_class_entry ce;
	/* REGISTER_INI_ENTRIES(); */
	INIT_CLASS_ENTRY(ce, "snowflake", snowflake_methods); //注册类及类方法


 #if PHP_MAJOR_VERSION < 7
    snowflake_ce = zend_register_internal_class(&ce TSRMLS_CC);
 #else
    snowflake_ce = zend_register_internal_class(&ce);
#endif

    snowflake_init(&sf);

	return SUCCESS;
}
/* }}} */

/* {{{ snowflake_module_entry
 */
zend_module_entry snowflake_module_entry = {
	STANDARD_MODULE_HEADER,
	"snowflake",
	snowflake_functions,
	PHP_MINIT(snowflake),
	PHP_MSHUTDOWN(snowflake),
	PHP_RINIT(snowflake),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(snowflake),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(snowflake),
	PHP_SNOWFLAKE_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SNOWFLAKE
#if PHP_MAJOR_VERSION == 7
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
#endif
ZEND_GET_MODULE(snowflake)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
