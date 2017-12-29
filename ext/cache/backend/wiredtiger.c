
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

#include "cache/backend/wiredtiger.h"
#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/exception.h"
#include "storage/wiredtiger.h"

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
 * Phalcon\Cache\Backend\Wiredtiger
 *
 * Allows to cache output fragments, PHP data or raw data to a wiredtiger backend
 *
 * This adapter uses the special wiredtigerd key "_PHCY" to store all the keys internally used by the adapter
 *
 *<code>
 *
 * // Cache data for 2 days
 * $frontCache = new Phalcon\Cache\Frontend\Data(array(
 *    "lifetime" => 172800
 * ));
 *
 * //Create the Cache setting wiredtigerd connection options
 * $cache = new Phalcon\Cache\Backend\Wiredtiger($frontCache, array(
 * 	'home' => __DIR__.'/wiredtiger'
 *	'table' => 'phalcon_test'
 *));
 *
 * //Cache arbitrary data
 * $cache->save('my-data', array(1, 2, 3, 4, 5));
 *
 * //Get data
 * $data = $cache->get('my-data');
 *
 *</code>
 */
zend_class_entry *phalcon_cache_backend_wiredtiger_ce;

PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, __construct);
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, get);
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, save);
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, delete);
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, queryKeys);
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, exists);
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, increment);
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, decrement);
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, flush);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_wiredtiger___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_wiredtiger_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_Wiredtiger, __construct, arginfo_phalcon_cache_backend_wiredtiger___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend_Wiredtiger, get, arginfo_phalcon_cache_backendinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Wiredtiger, save, arginfo_phalcon_cache_backendinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Wiredtiger, delete, arginfo_phalcon_cache_backendinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Wiredtiger, queryKeys, arginfo_phalcon_cache_backendinterface_querykeys, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Wiredtiger, exists, arginfo_phalcon_cache_backendinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Wiredtiger, increment, arginfo_phalcon_cache_backendinterface_increment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Wiredtiger, decrement, arginfo_phalcon_cache_backendinterface_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Wiredtiger, flush, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend\Wiredtiger initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Wiredtiger)
{
	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, Wiredtiger, cache_backend_wiredtiger, phalcon_cache_backend_ce, phalcon_cache_backend_wiredtiger_method_entry, 0);

	zend_declare_property_null(phalcon_cache_backend_wiredtiger_ce, SL("_table"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cache_backend_wiredtiger_ce, SL("_config"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cache_backend_wiredtiger_ce, SL("_wiredtiger"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cache_backend_wiredtiger_ce, SL("_cursor"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_cache_backend_wiredtiger_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\Wiredtiger constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, __construct){

	zval *frontend, *options = NULL, home = {}, table = {}, prefixed_table = {}, config = {}, wiredtiger = {}, cursor = {};

	phalcon_fetch_params(0, 1, 1, &frontend, &options);

	if (!options || Z_TYPE_P(options) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The options must be array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&home, options, SL("home"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The parameter 'home' is required");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&table, options, SL("table"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The parameter 'table' is required");
		return;
	}

	PHALCON_CONCAT_SV(&prefixed_table, "table:", &table);
	ZVAL_STRING(&config, "key_format=S,value_format=Si");

	phalcon_update_property(getThis(), SL("_table"), &prefixed_table);
	phalcon_update_property(getThis(), SL("_config"), &config);

	object_init_ex(&wiredtiger, phalcon_storage_wiredtiger_ce);
	PHALCON_CALL_METHOD(NULL, &wiredtiger, "__construct", &home);
	PHALCON_CALL_METHOD(NULL, &wiredtiger, "create", &prefixed_table, &config);
	PHALCON_CALL_METHOD(&cursor, &wiredtiger, "open", &prefixed_table);

	phalcon_update_property(getThis(), SL("_wiredtiger"), &wiredtiger);
	phalcon_update_property(getThis(), SL("_cursor"), &cursor);
	zval_ptr_dtor(&wiredtiger);
	zval_ptr_dtor(&cursor);

	PHALCON_CALL_PARENT(NULL, phalcon_cache_backend_wiredtiger_ce, getThis(), "__construct", frontend, options);
	zval_ptr_dtor(&prefixed_table);
	zval_ptr_dtor(&config);
}

/**
 * Returns a cached content
 *
 * @param int|string $keyName
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, get){

	zval *key_name, cursor = {}, frontend = {}, cached_content = {}, val = {}, expired = {};
	long int now;

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&cursor, getThis(), SL("_cursor"), PH_READONLY);

	PHALCON_CALL_METHOD(&cached_content, &cursor, "get", key_name);
	if (Z_TYPE(cached_content) != IS_ARRAY) {
		RETURN_NULL();
	}

	if (!phalcon_array_isset_fetch_long(&val, &cached_content, 0, PH_COPY)) {
		ZVAL_NULL(&val);
	}

	now = (long int)time(NULL);

	if (phalcon_array_isset_fetch_long(&expired, &cached_content, 1, PH_READONLY)) {
		if (phalcon_get_intval(&expired) < now) {
			PHALCON_CALL_METHOD(NULL, &cursor, "delete", key_name);
		}
	}
	zval_ptr_dtor(&cached_content);

	if (PHALCON_IS_NOT_EMPTY(&val)) {
		phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);
		PHALCON_RETURN_CALL_METHOD(&frontend, "afterretrieve", &val);
		zval_ptr_dtor(&val);
	} else {
		RETURN_ZVAL(&val, 0, 0);
	}
}

/**
 * Stores cached content into the Wiredtigerd backend and stops the frontend
 *
 * @param int|string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, key = {}, expired = {}, val = {}, prepared_val = {}, cached_content = {}, success = {};
	zval ttl = {}, is_buffering = {}, frontend = {}, cursor = {};

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
	ZVAL_LONG(&expired, (time(NULL) + phalcon_get_intval(&ttl)));

	/**
	 * Check if a connection is created or make a new one
	 */
	phalcon_read_property(&cursor, getThis(), SL("_cursor"), PH_READONLY);

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

	array_init_size(&cached_content, 2);
	phalcon_array_append(&cached_content, &prepared_val, 0);
	phalcon_array_append(&cached_content, &expired, 0);

	PHALCON_CALL_METHOD(&success, &cursor, "set", key_name, &cached_content);

	if (!zend_is_true(&success)) {
		zval_ptr_dtor(&cached_content);
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Failed to store data in wiredtiger");
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
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, delete){

	zval *key_name, cursor = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&cursor, getThis(), SL("_cursor"), PH_READONLY);

	PHALCON_CALL_METHOD(return_value, &cursor, "delete", key_name);
}

/**
 * Query the existing cached keys
 *
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, queryKeys){

	zval *prefix = NULL, cursor = {};

	phalcon_fetch_params(0, 0, 1, &prefix);

	phalcon_read_property(&cursor, getThis(), SL("_cursor"), PH_READONLY);

	array_init(return_value);

	/* Get the key from wiredtigerd */
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
}

/**
 * Checks if cache exists and it hasn't expired
 *
 * @param  string $keyName
 * @param  long $lifetime
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, exists){

	zval *key_name, cursor = {}, cached_content = {}, expired = {};
	long int now;

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&cursor, getThis(), SL("_cursor"), PH_READONLY);

	PHALCON_CALL_METHOD(&cached_content, &cursor, "get", key_name);
	if (Z_TYPE(cached_content) != IS_ARRAY) {
		RETURN_FALSE;
	}

	now = (long int)time(NULL);

	if (phalcon_array_isset_fetch_long(&expired, &cached_content, 1, PH_READONLY)) {
		if (phalcon_get_intval(&expired) < now) {
			PHALCON_CALL_METHOD(NULL, &cursor, "delete", key_name);
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}

/**
 * Atomic increment of a given key, by number $value
 *
 * @param string $keyName
 * @param long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, increment){

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

	PHALCON_RETURN_CALL_METHOD(getThis(), "save", key_name, &tmp);
}

/**
 * Atomic decrement of a given key, by number $value
 *
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, decrement){

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

	PHALCON_RETURN_CALL_METHOD(getThis(), "save", key_name, &tmp);
}

/**
 * Immediately invalidates all existing items.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Wiredtiger, flush){

	zval table = {}, config = {}, wiredtiger = {}, cursor = {};

	phalcon_read_property(&table, getThis(), SL("_table"), PH_READONLY);
	phalcon_read_property(&config, getThis(), SL("_config"), PH_READONLY);

	phalcon_read_property(&wiredtiger, getThis(), SL("_wiredtiger"), PH_READONLY);
	phalcon_read_property(&cursor, getThis(), SL("_cursor"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &cursor, "close");
	PHALCON_CALL_METHOD(return_value, &wiredtiger, "drop", &table);

	PHALCON_CALL_METHOD(NULL, &wiredtiger, "create", &table, &config);
	PHALCON_CALL_METHOD(&cursor, &wiredtiger, "open", &table);

	phalcon_update_property(getThis(), SL("_cursor"), &cursor);

	RETURN_TRUE;
}
