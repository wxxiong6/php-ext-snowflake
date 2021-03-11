/* snowflake extension for PHP */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_snowflake.h"
#include "snowflake_arginfo.h"

#include <sys/time.h>
#include <sched.h>
#include <sys/ipc.h>
#include <sys/shm.h>

static uint8_t ncpu = 1U;
static uint32_t pid = 0U;
static uint32_t lock = 0U;

static int le_snowflake;
static snowflake snowf;
static sf_shmtx_t *mtx;
static int shm_id;

zend_class_entry snowflake_ce;

ZEND_DECLARE_MODULE_GLOBALS(snowflake)
static int le_snowflake;

static void php_snowflake_init_globals(zend_snowflake_globals *snowflake_globals)
{
	snowflake_globals->region_id = 2;
}

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE()  \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif


static int trylock(sf_shmtx_t *mtx)
{
	return (mtx->lock == 0 && __sync_bool_compare_and_swap(&(mtx->lock), 0, pid));
}

static void spinlock(sf_shmtx_t *mtx)
{
	uint32_t i, n;
	for (;;)
	{
		if (trylock(mtx))
		{
			return;
		}
		if (ncpu > 1)
		{ // ncpu CPU个数
			for (n = 1; n < 2048; n <<= 1)
			{
				for (i = 0; i < n; i++)
				{
					__asm__ __volatile__("pause");
				}
				
				if (trylock(mtx))
				{
					return;
				}
			}
		}
		sched_yield(); // 只旋一个周期，没有获取到锁，退出CPU控制权
	}
}

static void spinlock_unlock(sf_shmtx_t *mtx)
{
	__sync_bool_compare_and_swap(&(mtx->lock), pid, 0);
}

static int sf_shmtx_shutdown()
{
	if (mtx->lock != 0)
	{
		spinlock_unlock(mtx);
	}
	shmdt(mtx);
	shmctl(shm_id, IPC_RMID, 0);
	return SUCCESS;
}

static int sf_shmtx_create()
{
	shm_id = shmget(IPCKEY, sizeof(sf_shmtx_t), 0600);
	if (shm_id != -1)
	{
		mtx = (sf_shmtx_t *)shmat(shm_id, NULL, 0);
		sf_shmtx_shutdown();
		return SUCCESS;
	}

	shm_id = shmget(IPCKEY, sizeof(sf_shmtx_t),  IPC_CREAT | IPC_EXCL | 0600);
	if (shm_id == -1)
	{
		php_error_docref(NULL, E_WARNING, "Init the shared memory[%lu] failed [%d:%s]!", sizeof(sf_shmtx_t), errno, strerror(errno));
		return FAILURE;
	}
	mtx = (sf_shmtx_t *)shmat(shm_id, NULL, 0);
	mtx->lock = 0U;
	mtx->last_time = 0ULL;
	mtx->seq = 0U;
	return SUCCESS;
}

static zend_ulong snowflake_init(snowflake *sf)
{
	zend_ulong region_id = SF_G(region_id);
	zend_ulong max_region_id = (-1L << SF_G(region_bits)) ^ -1L;
	if (region_id < 0 || region_id > max_region_id)
	{
		php_error_docref(NULL, E_WARNING, "Region ID must be in the range : 0-%llu", max_region_id);
		return FAILURE;
	}

	zend_ulong worker_id = SF_G(worker_id);
	zend_ulong max_worker_id = (-1L << SF_G(worker_bits)) ^ -1L;
	if (worker_id < 0 || worker_id > max_worker_id)
	{
		php_error_docref(NULL, E_WARNING, "Worker ID must be in the range: 0-%llu", max_worker_id);
		return FAILURE;
	}

	sf->time_shift = SF_G(region_bits) + SF_G(worker_bits) + SF_G(sequence_bits);
	sf->region_shift = SF_G(worker_bits) + SF_G(sequence_bits);
	sf->worker_shift = SF_G(sequence_bits);
	sf->seq_mask = (-1L << SF_G(sequence_bits)) ^ -1L;
	return region_id;
}

static inline uint64_t timestamp_gen()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((uint64_t)tv.tv_sec) * 1000 + ((uint64_t)tv.tv_usec) / 1000;
}

static uint64_t snowflake_id(snowflake *sf)
{
	uint64_t now = timestamp_gen();

	if (EXPECTED(now == mtx->last_time))
	{
		mtx->seq = (mtx->seq + 1) & sf->seq_mask;
		if (mtx->seq == 0)
		{
			while (now <= mtx->last_time)
			{
				now = timestamp_gen();
			}
		}
	}
	else if (now > mtx->last_time)
	{
		mtx->seq = 0;
	}
	else
	{
		php_error_docref(NULL, E_WARNING, "last_time in the range of 0, %llu, last_time:%llu", now, mtx->last_time);
	}
	mtx->last_time = now;
	return ((now - SF_G(epoch)) << sf->time_shift) | (SF_G(region_id) << sf->region_shift) | (SF_G(worker_id) << sf->worker_shift) | (mtx->seq);
	return now;
}

PHP_METHOD(snowflake, getId)
{
	if (zend_parse_parameters_none() == FAILURE)
	{
		return;
	}
	spinlock(mtx);
	uint64_t id = snowflake_id(&snowf);
	spinlock_unlock(mtx);
	if (id)
	{
		RETURN_LONG(id);
	}
	else
	{
		RETURN_BOOL(0);
	}
}

PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("snowflake.worker_id", "1", PHP_INI_SYSTEM, OnUpdateLongGEZero, worker_id, zend_snowflake_globals, snowflake_globals)
STD_PHP_INI_ENTRY("snowflake.region_id", "1", PHP_INI_SYSTEM, OnUpdateLongGEZero, region_id, zend_snowflake_globals, snowflake_globals)
//1576080000000 2019-12-12
STD_PHP_INI_ENTRY("snowflake.epoch", "1576080000000", PHP_INI_SYSTEM, OnUpdateLongGEZero, epoch, zend_snowflake_globals, snowflake_globals)
STD_PHP_INI_ENTRY("snowflake.time_bits", "41", PHP_INI_SYSTEM, OnUpdateLongGEZero, time_bits, zend_snowflake_globals, snowflake_globals)
STD_PHP_INI_ENTRY("snowflake.region_bits", "5", PHP_INI_SYSTEM, OnUpdateLongGEZero, region_bits, zend_snowflake_globals, snowflake_globals)
STD_PHP_INI_ENTRY("snowflake.worker_bits", "5", PHP_INI_SYSTEM, OnUpdateLongGEZero, worker_bits, zend_snowflake_globals, snowflake_globals)
STD_PHP_INI_ENTRY("snowflake.sequence_bits", "12", PHP_INI_SYSTEM, OnUpdateLongGEZero, sequence_bits, zend_snowflake_globals, snowflake_globals)
PHP_INI_END()

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(snowflake)
{
#if defined(ZTS) && defined(COMPILE_DL_TEST)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	if (pid == 0)
	{
		pid = (uint32_t)getpid();
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RSHUTDOWN_FUNCTION(snowflake)
{
#if defined(ZTS) && defined(COMPILE_DL_TEST)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(snowflake)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Snowflake support", "enabled");
	php_info_print_table_row(2, "Snowflake Version", PHP_SNOWFLAKE_VERSION);
	php_info_print_table_row(2, "Supports", SF_SUPPORT_URL);
	php_info_print_table_row(2, "worker_id", "1");
	php_info_print_table_row(2, "region_id", "1");
	php_info_print_table_row(2, "epoch", "1576080000000");
	php_info_print_table_row(2, "time_bits", "41");
	php_info_print_table_row(2, "region_bits", "5");
	php_info_print_table_row(2, "worker_bits", "5");
	php_info_print_table_row(2, "sequence_bits", "12");

	php_info_print_table_end();
}
/* }}} */
PHP_MINIT_FUNCTION(snowflake)
{
	ZEND_INIT_MODULE_GLOBALS(snowflake, php_snowflake_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	INIT_CLASS_ENTRY(snowflake_ce, "snowflake", snowflake_methods); //注册类及类方法
	zend_register_internal_class(&snowflake_ce);
	
	if (sf_shmtx_create() == FAILURE) 
    {
        return FAILURE;
    }
    if (snowflake_init(&snowf) == FAILURE) 
    {
        return FAILURE;
    }
    ncpu = sysconf(_SC_NPROCESSORS_ONLN);
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(snowflake)
{
	ZEND_INIT_MODULE_GLOBALS(snowflake, php_snowflake_init_globals, NULL);
	UNREGISTER_INI_ENTRIES();
	sf_shmtx_shutdown();
	return SUCCESS;
}
/* {{{ snowflake_method[]
*
* Every user visible function must have an entry in snowflake_functions[].
*/

/* }}} */
/* {{{ snowflake_module_entry */
zend_module_entry snowflake_module_entry = {
	STANDARD_MODULE_HEADER,
	"snowflake",			  /* Extension name */
	ext_functions,			  /* zend_function_entry */
	PHP_MINIT(snowflake),	  /* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(snowflake), /* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(snowflake),	  /* PHP_RINIT - Request initialization */
	PHP_RSHUTDOWN(snowflake), /* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(snowflake),	  /* PHP_MINFO - Module info */
	PHP_SNOWFLAKE_VERSION,		  /* Version */
	STANDARD_MODULE_PROPERTIES};
/* }}} */

#ifdef COMPILE_DL_SNOWFLAKE
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(snowflake)
#endif
