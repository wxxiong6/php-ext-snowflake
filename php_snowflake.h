/* snowflake extension for PHP */

#ifndef PHP_SNOWFLAKE_H
# define PHP_SNOWFLAKE_H

extern zend_module_entry snowflake_module_entry;
# define phpext_snowflake_ptr &snowflake_module_entry

# define PHP_SNOWFLAKE_VERSION "0.1.0"
#define SF_SUPPORT_URL	   "https://github.com/wxxiong6/php-ext-snowflake"

#define IPCKEY 0x366378

typedef struct _snowflake_state  snowflake;
typedef volatile uint32_t sf_atomic_t;
typedef struct _shmtx  sf_shmtx_t;

extern zend_module_entry test_module_entry;

ZEND_BEGIN_MODULE_GLOBALS(snowflake)
zend_ulong region_id;
zend_ulong worker_id;
zend_ulong time_bits;
zend_ulong region_bits;
zend_ulong worker_bits;
zend_ulong sequence_bits;
zend_ulong epoch;
ZEND_END_MODULE_GLOBALS(snowflake)

#define SF_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(snowflake, v)

struct _snowflake_state {
	uint32_t time_shift;
	uint32_t region_shift;
	uint32_t worker_shift;
	uint32_t seq_mask;
};

struct _shmtx {
	sf_atomic_t  lock; 
	sf_atomic_t  seq;
	uint64_t last_time;
};







# if defined(ZTS) && defined(COMPILE_DL_SNOWFLAKE)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_SNOWFLAKE_H */
