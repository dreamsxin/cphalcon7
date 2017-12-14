
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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "cache/backend/lmdb.h"
#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/exception.h"
#include "storage/lmdb.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/hash.h"
#include "kernel/string.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Cache\Backend\Lmdb
 *
 * Allows to cache output fragments, PHP data or raw data to a lmdb backend
 *
 * This adapter uses the special lmdbd key "_PHCY" to store all the keys internally used by the adapter
 *
 *<code>
 *
 * // Cache data for 2 days
 * $frontCache = new Phalcon\Cache\Frontend\Data(array(
 *    "lifetime" => 172800
 * ));
 *
 * // Create the Cache setting lmdb connection options
 * $cache = new Phalcon\Cache\Backend\Lmdb($frontCache, array(
 * 	'path' => __DIR__.'/lmdb',
 *	'name' => 'phalcon_test'
 *));
 *
 * // Or
 * $cache = new Phalcon\Cache\Backend\Lmdb($frontCache, array(
 *		'lmdb' => new Phalcon\Storage\Lmdb(__DIR__.'/lmdb')
 * ));
 *
 * // Cache arbitrary data
 * $cache->save('my-data', array(1, 2, 3, 4, 5));
 *
 * // Get data
 * $data = $cache->get('my-data');
 *
 *</code>
 */
zend_class_entry *phalcon_cache_backend_lmdb_ce;

PHP_METHOD(Phalcon_Cache_Backend_Lmdb, __construct);
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, get);
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, save);
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, delete);
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, queryKeys);
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, exists);
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, increment);
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, decrement);
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, flush);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_lmdb___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_lmdb_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_Lmdb, __construct, arginfo_phalcon_cache_backend_lmdb___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend_Lmdb, get, arginfo_phalcon_cache_backendinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Lmdb, save, arginfo_phalcon_cache_backendinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Lmdb, delete, arginfo_phalcon_cache_backendinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Lmdb, queryKeys, arginfo_phalcon_cache_backendinterface_querykeys, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Lmdb, exists, arginfo_phalcon_cache_backendinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Lmdb, increment, arginfo_phalcon_cache_backendinterface_increment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Lmdb, decrement, arginfo_phalcon_cache_backendinterface_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Lmdb, flush, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend\Lmdb initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Lmdb)
{
	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, Lmdb, cache_backend_lmdb, phalcon_cache_backend_ce, phalcon_cache_backend_lmdb_method_entry, 0);

	zend_declare_property_null(phalcon_cache_backend_lmdb_ce, SL("_config"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cache_backend_lmdb_ce, SL("_lmdb"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cache_backend_lmdb_ce, SL("_lmdb"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_cache_backend_lmdb_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\Lmdb constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, __construct){

	zval *frontend, *options = NULL, path = {}, name = {}, lmdb = {};

	phalcon_fetch_params(0, 1, 1, &frontend, &options);

	if (!options || Z_TYPE_P(options) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The options must be array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&lmdb, options, SL("lmdb"), PH_COPY)) {
		if (!phalcon_array_isset_fetch_str(&path, options, SL("path"), PH_READONLY)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The parameter 'path' is required");
			return;
		}

		if (!phalcon_array_isset_fetch_str(&name, options, SL("name"), PH_READONLY)) {
			ZVAL_NULL(&name);
		}

		object_init_ex(&lmdb, phalcon_storage_lmdb_ce);
		PHALCON_CALL_METHOD(NULL, &lmdb, "__construct", &path, &name);
	} else {
		PHALCON_VERIFY_CLASS(&lmdb, phalcon_storage_lmdb_ce);
	}

	phalcon_update_property(getThis(), SL("_lmdb"), &lmdb);
	zval_ptr_dtor(&lmdb);

	PHALCON_CALL_PARENT(NULL, phalcon_cache_backend_lmdb_ce, getThis(), "__construct", frontend, options);
}

/**
 * Returns a cached content
 *
 * @param int|string $keyName
 * @param int $lifetime
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, get){

	zval *key_name, *lifetime = NULL, lmdb = {}, save_time = {}, frontend = {}, cached_content = {}, val = {}, expired = {};
	long now;

	phalcon_fetch_params(0, 1, 1, &key_name, &lifetime);

	phalcon_read_property(&lmdb, getThis(), SL("_lmdb"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &lmdb, "begin");
	PHALCON_CALL_METHOD(&cached_content, &lmdb, "get", key_name);
	if (Z_TYPE(cached_content) != IS_ARRAY) {
		PHALCON_CALL_METHOD(NULL, &lmdb, "commit");
		zval_ptr_dtor(&cached_content);
		RETURN_NULL();
	}

	if (!phalcon_array_isset_fetch_long(&val, &cached_content, 0, PH_READONLY)) {
		zval_ptr_dtor(&cached_content);
		RETURN_NULL();
	}

	now = (long)time(NULL);
	
	if (lifetime && phalcon_array_isset_fetch_long(&save_time, &cached_content, 2, PH_READONLY)) {
		if ((now - phalcon_get_intval(&save_time)) > phalcon_get_intval(lifetime)) {
			PHALCON_CALL_METHOD(NULL, &lmdb, "commit");
			zval_ptr_dtor(&cached_content);
			RETURN_NULL();
		}
	}

	if (phalcon_array_isset_fetch_long(&expired, &cached_content, 1, PH_READONLY)) {
		if (phalcon_get_intval(&expired) < now) {
			PHALCON_CALL_METHOD(NULL, &lmdb, "delete", key_name);
		}
	}
	PHALCON_CALL_METHOD(NULL, &lmdb, "commit");
	zval_ptr_dtor(&cached_content);

	if (PHALCON_IS_NOT_EMPTY(&val)) {
		phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);
		PHALCON_RETURN_CALL_METHOD(&frontend, "afterretrieve", &val);
	} else {
		RETURN_CTOR(&val);
	}
}

/**
 * Stores cached content into the Lmdbd backend and stops the frontend
 *
 * @param int|string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, key = {}, expired = {}, val = {}, prepared_val = {}, cached_content = {}, success = {};
	zval ttl = {}, is_buffering = {}, frontend = {}, lmdb = {};
	long now_time;

	phalcon_fetch_params(0, 0, 4, &key_name, &content, &lifetime, &stop_buffer);

	if (!key_name || Z_TYPE_P(key_name) == IS_NULL) {
		phalcon_read_property(&key, getThis(), SL("_lastKey"), PH_READONLY);
		key_name = &key;
	}

	if (!zend_is_true(key_name)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The cache must be started first");
		return;
	}

	/**
	 * Take the lifetime from the frontend or read it from the set in start()
	 */
	if (!lifetime || Z_TYPE_P(lifetime) != IS_LONG) {
		PHALCON_CALL_METHOD(&ttl, getThis(), "getlifetime");
	} else {
		ZVAL_COPY_VALUE(&ttl, lifetime);
	}
	now_time = (long)time(NULL);
	ZVAL_LONG(&expired, (now_time + phalcon_get_intval(&ttl)));
	
	/**
	 * Check if a connection is created or make a new one
	 */
	phalcon_read_property(&lmdb, getThis(), SL("_lmdb"), PH_READONLY);

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);
	if (!content || Z_TYPE_P(content) == IS_NULL) {
		PHALCON_CALL_METHOD(&val, &frontend, "getcontent");
	} else {
		ZVAL_COPY(&val, content);
	}

	/**
	 * Prepare the content in the frontend
	 */
	PHALCON_CALL_METHOD(&prepared_val, &frontend, "beforestore", &val);
	zval_ptr_dtor(&val);

	array_init_size(&cached_content, 3);
	phalcon_array_append(&cached_content, &prepared_val, PH_READONLY);
	phalcon_array_append(&cached_content, &expired, PH_READONLY);
	phalcon_array_append_long(&cached_content, now_time, PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &lmdb, "begin");
	PHALCON_CALL_METHOD(&success, &lmdb, "put", key_name, &cached_content);
	PHALCON_CALL_METHOD(NULL, &lmdb, "commit");

	if (!zend_is_true(&success)) {
		zval_ptr_dtor(&cached_content);
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Failed to store data in lmdb");
		return;
	}

	PHALCON_CALL_METHOD(&is_buffering, &frontend, "isbuffering");

	if (!stop_buffer || PHALCON_IS_TRUE(stop_buffer)) {
		PHALCON_CALL_METHOD(NULL, &frontend, "stop");
	}

	if (PHALCON_IS_TRUE(&is_buffering)) {
		zend_print_zval(&cached_content, 0);
	}
	zval_ptr_dtor(&cached_content);

	phalcon_update_property_bool(getThis(), SL("_started"), 0);

	RETURN_TRUE;
}

/**
 * Deletes a value from the cache by its key
 *
 * @param int|string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, delete){

	zval *key_name, lmdb = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&lmdb, getThis(), SL("_lmdb"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &lmdb, "begin");
	PHALCON_CALL_METHOD(return_value, &lmdb, "delete", key_name);
	PHALCON_CALL_METHOD(NULL, &lmdb, "commit");
}

/**
 * Query the existing cached keys
 *
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, queryKeys){

	zval *prefix = NULL, lmdb = {}, cursor = {};

	phalcon_fetch_params(0, 0, 1, &prefix);

	phalcon_read_property(&lmdb, getThis(), SL("_lmdb"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &lmdb, "begin");
	PHALCON_CALL_METHOD(&cursor, &lmdb, "cursor");

	array_init(return_value);

	PHALCON_CALL_METHOD(NULL, &cursor, "rewind");
	while (1) {
		zval r0 = {}, key = {};
		PHALCON_CALL_METHOD(&r0, &cursor, "valid");
		if (!zend_is_true(&r0)) {
			break;
		}
		PHALCON_CALL_METHOD(&key, &cursor, "key");

		if (!prefix || !zend_is_true(prefix) || phalcon_start_with(&key, prefix, NULL)) {
			phalcon_array_append(return_value, &key, 0);
		}
		PHALCON_CALL_METHOD(NULL, &cursor, "next");
	}
	zval_ptr_dtor(&cursor);
	PHALCON_CALL_METHOD(NULL, &lmdb, "commit");
}

/**
 * Checks if cache exists and it hasn't expired
 *
 * @param  string $keyName
 * @param  long $lifetime
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, exists){

	zval *key_name, lmdb = {}, cached_content = {}, expired = {};
	long int now;

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&lmdb, getThis(), SL("_lmdb"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &lmdb, "begin");
	PHALCON_CALL_METHOD(&cached_content, &lmdb, "get", key_name);
	if (Z_TYPE(cached_content) != IS_ARRAY) {
		PHALCON_CALL_METHOD(NULL, &lmdb, "commit");
		RETURN_FALSE;
	}

	RETVAL_TRUE;

	now = (long int)time(NULL);

	if (phalcon_array_isset_fetch_long(&expired, &cached_content, 1, PH_READONLY)) {
		if (phalcon_get_intval(&expired) < now) {
			PHALCON_CALL_METHOD(NULL, &lmdb, "delete", key_name);
			RETVAL_FALSE;
		}
	}

	PHALCON_CALL_METHOD(NULL, &lmdb, "commit");
}

/**
 * Atomic increment of a given key, by number $value
 *
 * @param string $keyName
 * @param long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, increment){

	zval *key_name, *value = NULL, val = {}, tmp = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	PHALCON_CALL_METHOD(&val, getThis(), "get", key_name);
	phalcon_add_function(&tmp, &val, value);
	zval_ptr_dtor(&val);

	PHALCON_RETURN_CALL_METHOD(getThis(), "put", key_name, &tmp);
	zval_ptr_dtor(&tmp);
}

/**
 * Atomic decrement of a given key, by number $value
 *
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, decrement){

	zval *key_name, *value = NULL, val = {}, tmp = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	PHALCON_CALL_METHOD(&val, getThis(), "get", key_name);
	phalcon_sub_function(&tmp, &val, value);
	zval_ptr_dtor(&val);

	PHALCON_RETURN_CALL_METHOD(getThis(), "put", key_name, &tmp);
	zval_ptr_dtor(&tmp);
}

/**
 * Immediately invalidates all existing items.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Lmdb, flush){

	zval lmdb = {};

	phalcon_read_property(&lmdb, getThis(), SL("_lmdb"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &lmdb, "begin");
	PHALCON_CALL_METHOD(return_value, &lmdb, "drop");
	PHALCON_CALL_METHOD(NULL, &lmdb, "commit");

	RETURN_TRUE;
}
