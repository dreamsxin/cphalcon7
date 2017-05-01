
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

#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"

/**
 * Phalcon\Cache\Multiple
 *
 * Allows to read to chained backends writing to multiple backends
 *
 *<code>
 *   use Phalcon\Cache\Frontend\Data as DataFrontend,
 *       Phalcon\Cache\Multiple,
 *       Phalcon\Cache\Backend\Apc as ApcCache,
 *       Phalcon\Cache\Backend\Memcache as MemcacheCache,
 *       Phalcon\Cache\Backend\File as FileCache;
 *
 *   $ultraFastFrontend = new DataFrontend(array(
 *       "lifetime" => 3600
 *   ));
 *
 *   $fastFrontend = new DataFrontend(array(
 *       "lifetime" => 86400
 *   ));
 *
 *   $slowFrontend = new DataFrontend(array(
 *       "lifetime" => 604800
 *   ));
 *
 *   //Backends are registered from the fastest to the slower
 *   $cache = new Multiple(array(
 *       new ApcCache($ultraFastFrontend, array(
 *           "prefix" => 'cache',
 *       )),
 *       new MemcacheCache($fastFrontend, array(
 *           "prefix" => 'cache',
 *           "host" => "localhost",
 *           "port" => "11211"
 *       )),
 *       new FileCache($slowFrontend, array(
 *           "prefix" => 'cache',
 *           "cacheDir" => "../app/cache/"
 *       ))
 *   ));
 *
 *   //Save, saves in every backend
 *   $cache->save('my-key', $data);
 *</code>
 */
zend_class_entry *phalcon_cache_multiple_ce;

PHP_METHOD(Phalcon_Cache_Multiple, __construct);
PHP_METHOD(Phalcon_Cache_Multiple, push);
PHP_METHOD(Phalcon_Cache_Multiple, get);
PHP_METHOD(Phalcon_Cache_Multiple, start);
PHP_METHOD(Phalcon_Cache_Multiple, save);
PHP_METHOD(Phalcon_Cache_Multiple, delete);
PHP_METHOD(Phalcon_Cache_Multiple, exists);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_multiple___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, backends, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_multiple_push, 0, 0, 1)
	ZEND_ARG_INFO(0, backend)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_multiple_get, 0, 0, 1)
	ZEND_ARG_INFO(0, keyName)
	ZEND_ARG_INFO(0, lifetime)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_multiple_start, 0, 0, 1)
	ZEND_ARG_INFO(0, keyName)
	ZEND_ARG_INFO(0, lifetime)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_multiple_save, 0, 0, 0)
	ZEND_ARG_INFO(0, keyName)
	ZEND_ARG_INFO(0, content)
	ZEND_ARG_INFO(0, lifetime)
	ZEND_ARG_INFO(0, stopBuffer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_multiple_delete, 0, 0, 1)
	ZEND_ARG_INFO(0, keyName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_multiple_exists, 0, 0, 0)
	ZEND_ARG_INFO(0, keyName)
	ZEND_ARG_INFO(0, lifetime)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_multiple_method_entry[] = {
	PHP_ME(Phalcon_Cache_Multiple, __construct, arginfo_phalcon_cache_multiple___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Multiple, push, arginfo_phalcon_cache_multiple_push, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Multiple, get, arginfo_phalcon_cache_multiple_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Multiple, start, arginfo_phalcon_cache_multiple_start, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Multiple, save, arginfo_phalcon_cache_multiple_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Multiple, delete, arginfo_phalcon_cache_multiple_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Multiple, exists, arginfo_phalcon_cache_multiple_exists, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Multiple initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Multiple){

	PHALCON_REGISTER_CLASS(Phalcon\\Cache, Multiple, cache_multiple, phalcon_cache_multiple_method_entry, 0);

	zend_declare_property_null(phalcon_cache_multiple_ce, SL("_backends"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Multiple constructor
 *
 * @param Phalcon\Cache\BackendInterface[] $backends
 */
PHP_METHOD(Phalcon_Cache_Multiple, __construct){

	zval *backends = NULL;

	phalcon_fetch_params(0, 0, 1, &backends);

	if (backends) {
		phalcon_update_property(getThis(), SL("_backends"), backends);
	}
}

/**
 * Adds a backend
 *
 * @param Phalcon\Cache\BackendInterface $backend
 * @return Phalcon\Cache\Multiple
 */
PHP_METHOD(Phalcon_Cache_Multiple, push){

	zval *backend;

	phalcon_fetch_params(0, 1, 0, &backend);

	PHALCON_VERIFY_INTERFACE_EX(backend, phalcon_cache_backendinterface_ce, phalcon_cache_exception_ce);
	phalcon_update_property_array_append(getThis(), SL("_backends"), backend);
	RETURN_THIS();
}

/**
 * Returns a cached content reading the internal backends
 *
 * @param 	string $keyName
 * @param   long $lifetime
 * @return  mixed
 */
PHP_METHOD(Phalcon_Cache_Multiple, get){

	zval *key_name, *lifetime = NULL, backends = {}, *backend;

	phalcon_fetch_params(0, 1, 1, &key_name, &lifetime);

	if (!lifetime) {
		lifetime = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&backends, getThis(), SL("_backends"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(backends) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(backends), backend) {
			zval content = {};
			PHALCON_CALL_METHOD(&content, backend, "get", key_name, lifetime);
			if (Z_TYPE(content) > IS_NULL) {
				RETVAL_ZVAL(&content, 0, 0);
				return;
			}
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_NULL();
}

/**
 * Starts every backend
 *
 * @param int|string $keyName
 * @param   long $lifetime
 * @return  mixed
 */
PHP_METHOD(Phalcon_Cache_Multiple, start){

	zval *key_name, *lifetime = NULL, backends = {}, *backend;

	phalcon_fetch_params(0, 1, 1, &key_name, &lifetime);

	if (!lifetime) {
		lifetime = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&backends, getThis(), SL("_backends"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(backends) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(backends), backend) {
			PHALCON_CALL_METHOD(NULL, backend, "start", key_name, lifetime);
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * Stores cached content into all backends and stops the frontend
 *
 * @param string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Multiple, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, backends = {}, *backend;

	phalcon_fetch_params(0, 0, 4, &key_name, &content, &lifetime, &stop_buffer);

	if (!key_name) {
		key_name = &PHALCON_GLOBAL(z_null);
	}

	if (!content) {
		content = &PHALCON_GLOBAL(z_null);
	}

	if (!lifetime) {
		lifetime = &PHALCON_GLOBAL(z_null);
	}

	if (!stop_buffer) {
		stop_buffer = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&backends, getThis(), SL("_backends"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(backends) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(backends), backend) {
			PHALCON_CALL_METHOD(NULL, backend, "save", key_name, content, lifetime, stop_buffer);
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * Deletes a value from each backend
 *
 * @param int|string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Multiple, delete){

	zval *key_name, backends = {}, *backend;

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&backends, getThis(), SL("_backends"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(backends) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(backends), backend) {
			PHALCON_CALL_METHOD(NULL, backend, "delete", key_name);
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * Checks if cache exists in at least one backend
 *
 * @param  string $keyName
 * @param  long $lifetime
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Multiple, exists){

	zval *key_name = NULL, *lifetime = NULL, backends = {}, *backend;

	phalcon_fetch_params(0, 0, 2, &key_name, &lifetime);

	if (!key_name) {
		key_name = &PHALCON_GLOBAL(z_null);
	}

	if (!lifetime) {
		lifetime = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&backends, getThis(), SL("_backends"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(backends) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(backends), backend) {
			zval exists = {};
			PHALCON_CALL_METHOD(&exists, backend, "exists", key_name, lifetime);
			if (zend_is_true(&exists)) {
				RETURN_TRUE;
			}
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_FALSE;
}
