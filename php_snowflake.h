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

#ifndef PHP_SNOWFLAKE_H
#define PHP_SNOWFLAKE_H

extern zend_module_entry snowflake_module_entry;
#define phpext_snowflake_ptr &snowflake_module_entry

#define PHP_SNOWFLAKE_VERSION "0.1.0" /* Replace with version number for your extension */

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

/* Always refer to the globals in your function as SNOWFLAKE_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
// #define SNOWFLAKE_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(snowflake, v)

#if defined(ZTS) && defined(COMPILE_DL_SNOWFLAKE)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#ifdef ZTS
#define SF_G(v) TSRMG(sf_globals_id, zend_sf_globals *, v)
#else
#define SF_G(v) (snowflake_globals.v)
#endif
/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:
*/
ZEND_BEGIN_MODULE_GLOBALS(snowflake)
	zend_long  worker_id;
  zend_long region_id;
  zend_long epoch;
  zend_long region_bits;
  zend_long worker_bits;
  zend_long sequence_bits;
  zend_long time_bits;
ZEND_END_MODULE_GLOBALS(snowflake)

#define SNOWFLAKE_WORKER_ID "1"
#define SNOWFLAKE_REGION_ID "1"
#define SNOWFLAKE_EPOCH "1576080000000" //2019-12-12
#define SNOWFLAKE_TIME_BITS "41"
#define SNOWFLAKE_REGIONID_BITS "4"
#define SNOWFLAKE_WORKERID_BITS "10"
#define SNOWFLAKE_SEQUENCE_BITS "8"
typedef struct _snowflake_state  snowflake;
struct _snowflake_state {
    zend_long time;
    zend_long seq_max;
    zend_long worker_id;
    zend_long region_id;
    zend_long seq;
    zend_long time_bits;
    zend_long region_bits;
    zend_long worker_bits;
};

static zend_long snowflake_id(snowflake *);
static int snowflake_init(int region_id, int worker_id, snowflake *);


#endif	/* PHP_SNOWFLAKE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
