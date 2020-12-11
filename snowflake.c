/* snowflake extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_snowflake.h"
#include "snowflake_arginfo.h"

#include <sys/time.h>
#include <sched.h>
#include <sys/ipc.h>
#include <sys/shm.h>

static uint32_t ncpu = 1;
static uint32_t pid = 0;

static snowflake snowf;
static sf_shmtx_t *mtx;
static int shm_id;

zend_class_entry snowflake_ce;

ZEND_DECLARE_MODULE_GLOBALS(snowflake)
static int le_snowflake;

static void php_snowflake_init_globals(zend_snowflake_globals *snowflake_globals)
{
	snowflake_globals->region_id     = 2;
}


/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void test1() */
PHP_FUNCTION(test1)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_printf("The extension %s is loaded and working!\r\n", "snowflake");
}
/* }}} */

/* {{{ string test2( [ string $var ] ) */
PHP_FUNCTION(test2)
{
	char *var = "World";
	size_t var_len = sizeof("World") - 1;
	zend_string *retval;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(var, var_len)
	ZEND_PARSE_PARAMETERS_END();

	retval = strpprintf(0, "Hello %s", var);

	RETURN_STR(retval);
}
/* }}}*/




static int trylock(sf_shmtx_t *mtx)
{
	return (mtx->lock == 0 && __sync_bool_compare_and_swap(&(mtx->lock), 0, &pid)); 
}

static void spinlock(sf_shmtx_t *mtx)
{
	uint32_t  i, n;
	for ( ;; ) {
		if (mtx->lock == 0 && __sync_bool_compare_and_swap(&(mtx->lock), 0, &pid)) {
			return;
		}
		if (ncpu > 1) { // ncpu CPU个数
			for (n = 1; n < mtx->spin; n <<= 1) {
				for (i = 0; i < n; i++) {
					__asm__ __volatile__ ("pause");
				}
				if (mtx->lock == 0 && __sync_bool_compare_and_swap(&(mtx->lock), 0, &pid)) {
					return;
				}
			}
		}
		sched_yield(); // 只旋一个周期，没有获取到锁，退出CPU控制权
	}   
}

static void spinlock_unlock(sf_shmtx_t *mtx)
{
	__sync_bool_compare_and_swap(&(mtx->lock), &pid, 0);
}

static int sf_shmtx_shutdown(sf_shmtx_t **mtx)
{
	if ((*mtx)->lock != NULL && (*mtx)->lock != 0) 
		{
			spinlock_unlock(*mtx);
		}
	shmdt(*mtx);
	shmctl(shm_id, IPC_RMID, 0) ;
	
	return SUCCESS;
}

static int sf_shmtx_create(sf_shmtx_t **mtx) 
{
	shm_id = shmget(IPCKEY , sizeof(sf_shmtx_t), 0640);
	if (shm_id != -1)
		{
			*mtx = (sf_shmtx_t *)shmat(shm_id, NULL, 0);
			sf_shmtx_shutdown(mtx);
		} 
	
	shm_id = shmget(IPCKEY, sizeof(sf_shmtx_t), 0640 | IPC_CREAT | IPC_EXCL);
	if (shm_id == -1) 
		{
			php_error_docref(NULL, E_WARNING, "Init the shared memory[%lu] failed [%d:%s]!", sizeof(sf_shmtx_t), errno, strerror(errno));
			return FAILURE;
		} 
	else
		{
			*mtx = (sf_shmtx_t *) shmat(shm_id, NULL, 0);
		}
	
	
	(*mtx)->lock = 0;
	(*mtx)->spin = 2048;
	(*mtx)->last_time = 0;
	(*mtx)->seq = 0;
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
	
	sf->time_shift   = SF_G(region_bits) + SF_G(worker_bits) + SF_G(sequence_bits);
	sf->region_shift = SF_G(worker_bits) + SF_G(sequence_bits);
	sf->worker_shift = SF_G(sequence_bits);
	sf->seq_mask     = (-1L << SF_G(sequence_bits)) ^ -1L;	return region_id;
}

static inline uint64_t timestamp_gen()
{
	struct  timeval    tv;
	gettimeofday(&tv, NULL);
	return ((uint64_t)tv.tv_sec) * 1000 + ((uint64_t)tv.tv_usec) / 1000; 
} 

static uint64_t snowflake_id(snowflake *sf) 
{
	uint64_t now = timestamp_gen();
	
	if (EXPECTED(now == mtx->last_time))
		{
			mtx->seq = (mtx->seq+1) & sf->seq_mask;
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
			php_error_docref(NULL, E_WARNING, "last_time in the range of 0, %lld, last_time:%lld", now, mtx->last_time);
		}
	mtx->last_time = now;
	return ((now-SF_G(epoch)) << sf->time_shift) 
	| (SF_G(region_id) << sf->region_shift) 
	| (SF_G(worker_id) << sf->worker_shift) 
	| (mtx->seq);
}

PHP_METHOD(snowflake, getId)
{
	if (zend_parse_parameters_none() == FAILURE) 
	{
			return;
	}
	//	spinlock(mtx);
	uint64_t id = snowflake_id(&snowf);
	//	spinlock_unlock(mtx);
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
STD_PHP_INI_ENTRY("snowflake.worker_id",     "1",     PHP_INI_SYSTEM, OnUpdateLongGEZero, worker_id,     zend_snowflake_globals, snowflake_globals)
STD_PHP_INI_ENTRY("snowflake.region_id",     "1",     PHP_INI_SYSTEM, OnUpdateLongGEZero, region_id,     zend_snowflake_globals, snowflake_globals)
//1576080000000 2019-12-12
STD_PHP_INI_ENTRY("snowflake.epoch",         "1576080000000",         PHP_INI_SYSTEM, OnUpdateLongGEZero, epoch,         zend_snowflake_globals, snowflake_globals)
STD_PHP_INI_ENTRY("snowflake.time_bits",     "41",     PHP_INI_SYSTEM, OnUpdateLongGEZero, time_bits,     zend_snowflake_globals, snowflake_globals)
STD_PHP_INI_ENTRY("snowflake.region_bits",   "5",      PHP_INI_SYSTEM, OnUpdateLongGEZero, region_bits,   zend_snowflake_globals, snowflake_globals)
STD_PHP_INI_ENTRY("snowflake.worker_bits",   "5",      PHP_INI_SYSTEM, OnUpdateLongGEZero, worker_bits,   zend_snowflake_globals, snowflake_globals)
STD_PHP_INI_ENTRY("snowflake.sequence_bits", "12",     PHP_INI_SYSTEM, OnUpdateLongGEZero, sequence_bits, zend_snowflake_globals, snowflake_globals)
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


/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(snowflake)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "snowflake support", "enabled");
	php_info_print_table_end();
}
/* }}} */
PHP_MINIT_FUNCTION(snowflake)
{
	ZEND_INIT_MODULE_GLOBALS(snowflake, php_snowflake_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	INIT_CLASS_ENTRY(snowflake_ce, "snowflake", snowflake_methods); //注册类及类方法
	
	zend_register_internal_class(&snowflake_ce);
	
	if (sf_shmtx_create(&mtx) == FAILURE) 
		{
			return FAILURE;
		}
	ncpu = sysconf(_SC_NPROCESSORS_ONLN);
	
	snowflake_init(&snowf);
	return SUCCESS;
	
}

PHP_MSHUTDOWN_FUNCTION(snowflake)
{
	ZEND_INIT_MODULE_GLOBALS(snowflake, php_snowflake_init_globals, NULL);
	UNREGISTER_INI_ENTRIES();
	
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
	"snowflake",					/* Extension name */
	ext_functions,					/* zend_function_entry */
	PHP_MINIT(snowflake),							/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(snowflake),							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(snowflake),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(snowflake),			/* PHP_MINFO - Module info */
	PHP_TEST_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */


#ifdef COMPILE_DL_SNOWFLAKE
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(snowflake)
#endif
