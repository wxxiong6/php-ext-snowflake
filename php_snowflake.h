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


#define IPCKEY 0x366378

typedef struct _snowflake_state  snowflake;
typedef volatile uint32_t sf_atomic_t;
typedef struct _shmtx  sf_shmtx_t;

ZEND_BEGIN_MODULE_GLOBALS(snowflake)
  uint8_t region_id;
  uint8_t worker_id;
  uint8_t time_bits;
  uint8_t region_bits;
  uint8_t worker_bits;
  uint8_t sequence_bits;
  uint64_t epoch;
ZEND_END_MODULE_GLOBALS(snowflake)

ZEND_DECLARE_MODULE_GLOBALS(snowflake)

#define SF_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(snowflake, v)


#if defined(ZTS) && defined(COMPILE_DL_SNOWFLAKE)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

struct _shmtx {  
  sf_atomic_t  *lock; 
  sf_atomic_t  spin;
  sf_atomic_t  seq;
  uint64_t last_time;
};

struct _snowflake_state {
    uint8_t time_shift;
    uint8_t region_shift;
    uint8_t worker_shift;
    uint32_t seq_mask;
};


static int snowflake_init(snowflake *);
static uint64_t snowflake_id(snowflake *);
static void spinlock_unlock(sf_shmtx_t *);
static int trylock(sf_shmtx_t *);
static void spinlock(sf_shmtx_t *);

#endif	/* PHP_SNOWFLAKE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
