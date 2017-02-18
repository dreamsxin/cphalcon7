
/*
 +------------------------------------------------------------------------+
 | Phalcon Framework                                                      |
 +------------------------------------------------------------------------+
 | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
 +------------------------------------------------------------------------+
 | This source file is subject to the New BSD License that is bundled     |
 | with this package in the file docs/LICENSE.txt.                        |
 |                                                                        |
 | If you did not receive a copy of the license and are unable to         |
 | obtain it through the world-wide-web, please send an email             |
 | to license@phalconphp.com so we can send you a copy immediately.       |
 +------------------------------------------------------------------------+
 | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
 |          Eduar Carvajal <eduar@phalconphp.com>                         |
 +------------------------------------------------------------------------+
*/

#ifndef PHP_PHALCON_H
#define PHP_PHALCON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <main/php.h>
#ifdef ZTS
#include <TSRM/TSRM.h>
#endif

#define PHP_PHALCON_VERSION "Phalcon7(Dao7)-1.2.2"
#define PHP_PHALCON_VERSION_MAJOR           1
#define PHP_PHALCON_VERSION_MED             2
#define PHP_PHALCON_VERSION_MIN             3
#define PHP_PHALCON_VERSION_RELEASE         PHALCON_VERSION_STABLE
#define PHP_PHALCON_VERSION_RELEASE_VERSION 0
#define PHP_PHALCON_EXTNAME "phalcon7"

/** DEBUG options */
typedef struct _phalcon_debug_options {
	int debug_level;
	zend_bool enable_debug;
} phalcon_debug_options;

/** ORM options */
typedef struct _phalcon_orm_options {
	HashTable *ast_cache;
	int cache_level;
	zend_bool events;
	zend_bool virtual_foreign_keys;
	zend_bool not_null_validations;
	zend_bool length_validations;
	zend_bool exception_on_failed_save;
	zend_bool enable_literals;
	zend_bool enable_ast_cache;
	zend_bool enable_property_method;
	zend_bool enable_auto_convert;
	zend_bool allow_update_primary;
	zend_bool enable_strict;
} phalcon_orm_options;

/** Security options */
typedef struct _phalcon_security_options {
	zend_bool crypt_std_des_supported;
	zend_bool crypt_ext_des_supported;
	zend_bool crypt_md5_supported;
	zend_bool crypt_blowfish_supported;
	zend_bool crypt_blowfish_y_supported;
	zend_bool crypt_sha256_supported;
	zend_bool crypt_sha512_supported;
} phalcon_security_options;

/** DB options */
typedef struct _phalcon_db_options {
	zend_bool escape_identifiers;
} phalcon_db_options;

/** Cache options */
typedef struct _phalcon_cache_options {
	zend_bool enable_shmemory;
	zend_bool enable_shmemory_cli;
	size_t shmemory_keys_size;
	size_t shmemory_values_size;
} phalcon_cache_options;

ZEND_BEGIN_MODULE_GLOBALS(phalcon)

	/* Controls double initialization of memory frames */
	int initialized;

	/** Frequently used zvals */
	zval z_null;
	zval z_true;
	zval z_false;
	zval z_zero;
	zval z_one;
	zval z_two;

	/** DEBUG */
	phalcon_debug_options debug;

	/** ORM */
	phalcon_orm_options orm;

	/** Max recursion control */
	unsigned int recursive_lock;

	/** Security */
	phalcon_security_options security;

	/** DB */
	phalcon_db_options db;

	/** Cache */
	phalcon_cache_options cache;

ZEND_END_MODULE_GLOBALS(phalcon)


ZEND_EXTERN_MODULE_GLOBALS(phalcon)

#ifdef ZTS
	#define PHALCON_GLOBAL(v) TSRMG(phalcon_globals_id, zend_phalcon_globals *, v)
	#define PHALCON_VGLOBAL   ((zend_phalcon_globals *) (*((void ***) tsrm_get_ls_cache()))[TSRM_UNSHUFFLE_RSRC_ID(phalcon_globals_id)])
#else
	#define PHALCON_GLOBAL(v) (phalcon_globals.v)
	#define PHALCON_VGLOBAL &(phalcon_globals)
#endif

extern zend_module_entry phalcon_module_entry;
#define phpext_phalcon_ptr &phalcon_module_entry

#define INIT_ZVAL(z) z = EG(uninitialized_zval);

#define PHALCON_ALLOC_ZVAL(z) \
	(z) = (zval *) emalloc(sizeof(zval)); \

#define PHALCON_ALLOC_INIT_ZVAL(z) \
	PHALCON_ALLOC_ZVAL(z); \

#ifndef INIT_PZVAL
#	define INIT_PZVAL(z) \
		Z_SET_REFCOUNT_P(z, 1); \
		ZVAL_UNREF(z);
#endif

#ifndef INIT_PZVAL_COPY
#	define INIT_PZVAL_COPY(z, v) \
		ZVAL_COPY_VALUE(z, v);   \
		Z_SET_REFCOUNT_P(z, 1);  \
		ZVAL_UNREF(z);
#endif

#ifndef ZVAL_COPY_VALUE
#	define ZVAL_COPY_VALUE(z, v) \
		(z)->value  = (v)->value; \
		Z_TYPE_P(z) = Z_TYPE_P(v);
#endif

#define PHALCON_INIT_CLASS(name) \
	int phalcon_ ##name## _init(int module_number)

#define PHALCON_INIT(name) \
	if (phalcon_ ##name## _init(module_number) == FAILURE) { \
		return FAILURE; \
	}

/** Macros for branch prediction */
#define likely(x)       EXPECTED(x)
#define unlikely(x)     UNEXPECTED(x)

#if defined(__GNUC__) && (defined(__clang__) || ((__GNUC__ * 100 + __GNUC_MINOR__) >= 405))
#	define UNREACHABLE() __builtin_unreachable()
#	define ASSUME(x)     if (x) {} else __builtin_unreachable()
#else
#	define UNREACHABLE() assert(0)
#	define ASSUME(x)     assert(!!(x));
#endif

#if defined(__GNUC__) || defined(__clang__)
#	define PHALCON_ATTR_NONNULL            __attribute__((nonnull))
#	define PHALCON_ATTR_NONNULL1(x)        __attribute__((nonnull (x)))
#	define PHALCON_ATTR_NONNULL2(x, y)     __attribute__((nonnull (x, y)))
#	define PHALCON_ATTR_NONNULL3(x, y, z)  __attribute__((nonnull (x, y, z)))
#	define PHALCON_ATTR_PURE               __attribute__((pure))
#	define PHALCON_ATTR_CONST              __attribute__((const))
#	define PHALCON_ATTR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#	define PHALCON_ATTR_NONNULL
#	define PHALCON_ATTR_NONNULL1(x)
#	define PHALCON_ATTR_NONNULL2(x, y)
#	define PHALCON_ATTR_NONNULL3(x, y, z)
#	define PHALCON_ATTR_PURE
#	define PHALCON_ATTR_CONST
#	define PHALCON_ATTR_WARN_UNUSED_RESULT
#endif

#if !defined(__GNUC__) && !(defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590))
#	define __builtin_constant_p(s)    (0)
#endif

#ifndef ZEND_MOD_END
#	define ZEND_MOD_END { NULL, NULL, NULL, 0 }
#endif

#ifndef __func__
#	define __func__ __FUNCTION__
#endif

#define PHALCON_STATIC

/* This is a temporary fix until config.w32 is updated */
#if !defined(__CYGWIN__) && defined(WIN32) && defined(HAVE_CONFIG_H)

#	if defined(HAVE_JSON) && !defined(PHALCON_USE_PHP_JSON)
#		define PHALCON_USE_PHP_JSON 1
#	endif

#	if defined(HAVE_BUNDLED_PCRE) && !defined(PHALCON_USE_PHP_PCRE)
#		define PHALCON_USE_PHP_PCRE 1
#	endif

#	if defined(HAVE_PHP_SESSION) && !defined(PHALCON_USE_PHP_SESSION)
#		define PHALCON_USE_PHP_SESSION 1
#	endif

#	if defined(HAVE_HASH_EXT) && !defined(PHALCON_USE_PHP_HASH)
#		define PHALCON_USE_PHP_HASH 1
#	endif

#endif /* !defined(__CYGWIN__) && !defined(WIN32) && defined(HAVE_CONFIG_H) */

#if !defined(__CYGWIN__) && defined(WIN32)
double round(double num) {
	return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
}
#endif

#endif /* PHP_PHALCON_H */
