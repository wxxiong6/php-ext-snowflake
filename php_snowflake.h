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
  | Author: wxxiong6@gmail.com                                           |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_SNOWFLAKE_H
#define PHP_SNOWFLAKE_H

extern zend_module_entry snowflake_module_entry;
#define phpext_snowflake_ptr &snowflake_module_entry

#define PHP_SNOWFLAKE_VERSION "1.0.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_SNOWFLAKE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_SNOWFLAKE_API __attribute__ ((visibility("default")))
#else
#	define PHP_SNOWFLAKE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

typedef struct _snowflake_state  snowflake;

ZEND_BEGIN_MODULE_GLOBALS(snowflake)
  int worker_id;
  int region_id;
  int region_bits;
  int worker_bits;
  int sequence_bits;
  int time_bits;
  zend_long epoch;
ZEND_END_MODULE_GLOBALS(snowflake)

ZEND_DECLARE_MODULE_GLOBALS(snowflake)

#define SF_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(snowflake, v)


#if defined(ZTS) && defined(COMPILE_DL_SNOWFLAKE)
ZEND_TSRMLS_CACHE_EXTERN()
#endif


#define SNOWFLAKE_WORKER_ID 1
#define SNOWFLAKE_REGION_ID 1
#define SNOWFLAKE_EPOCH 1576080000000 //2019-12-12
#define SNOWFLAKE_TIME_BITS 41
#define SNOWFLAKE_REGIONID_BITS 5
#define SNOWFLAKE_WORKERID_BITS 5
#define SNOWFLAKE_SEQUENCE_BITS 12



struct _snowflake_state {
    zend_long last_time;
    zend_long epoch;
    int seq_mask;
    int worker_id;
    int region_id;
    int seq;
    int time_bits;
    int region_bits;
    int worker_bits;
};

static zend_long snowflake_id(snowflake *);
static int snowflake_init(snowflake **);


#endif	/* PHP_SNOWFLAKE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
