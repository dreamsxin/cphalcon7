
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
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  +------------------------------------------------------------------------+
*/

#include "cache/backend/xcache.h"
#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/concat.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/hash.h"
#include "kernel/string.h"

/**
 * Phalcon\Cache\Backend\Xcache
 *
 * Allows to cache output fragments, PHP data and raw data using an XCache backend
 *
 *<code>
 *	//Cache data for 2 days
 *	$frontCache = new Phalcon\Cache\Frontend\Data(array(
 *		'lifetime' => 172800
 *	));
 *
 *  $cache = new Phalcon\Cache\Backend\Xcache($frontCache, array(
 *      'prefix' => 'app-data'
 *  ));
 *
 *	//Cache arbitrary data
 *	$cache->save('my-data', array(1, 2, 3, 4, 5));
 *
 *	//Get data
 *	$data = $cache->get('my-data');
 *
 *</code>
 */
zend_class_entry *phalcon_cache_backend_xcache_ce;

PHP_METHOD(Phalcon_Cache_Backend_Xcache, __construct);
PHP_METHOD(Phalcon_Cache_Backend_Xcache, get);
PHP_METHOD(Phalcon_Cache_Backend_Xcache, save);
PHP_METHOD(Phalcon_Cache_Backend_Xcache, delete);
PHP_METHOD(Phalcon_Cache_Backend_Xcache, queryKeys);
PHP_METHOD(Phalcon_Cache_Backend_Xcache, exists);
PHP_METHOD(Phalcon_Cache_Backend_Xcache, increment);
PHP_METHOD(Phalcon_Cache_Backend_Xcache, decrement);
PHP_METHOD(Phalcon_Cache_Backend_Xcache, flush);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_xcache___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_xcache_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_Xcache, __construct, arginfo_phalcon_cache_backend_xcache___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend_Xcache, get, arginfo_phalcon_cache_backendinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Xcache, save, arginfo_phalcon_cache_backendinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Xcache, delete, arginfo_phalcon_cache_backendinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Xcache, queryKeys, arginfo_phalcon_cache_backendinterface_querykeys, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Xcache, exists, arginfo_phalcon_cache_backendinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Xcache, increment, arginfo_phalcon_cache_backendinterface_increment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Xcache, decrement, arginfo_phalcon_cache_backendinterface_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Xcache, flush, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend\Xcache initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Xcache){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, Xcache, cache_backend_xcache, phalcon_cache_backend_ce, phalcon_cache_backend_xcache_method_entry, 0);

	zend_class_implements(phalcon_cache_backend_xcache_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\Xcache constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_Xcache, __construct){

	zval *frontend, *_options = NULL, options = {}, special_key = {};

	phalcon_fetch_params(0, 1, 1, &frontend, &_options);

	if (!_options || Z_TYPE_P(_options) == IS_NULL) {
		array_init(&options);
	} else {
		PHALCON_CPY_WRT_CTOR(&options, _options);
	}

	if (!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")) || PHALCON_IS_EMPTY_STRING(&special_key)) {
		phalcon_array_update_str_str(&options, SL("statsKey"), SL("_PHCX"), PH_COPY);
	}

	PHALCON_CALL_PARENT(NULL, phalcon_cache_backend_xcache_ce, getThis(), "__construct", frontend, &options);
}

/**
 * Returns cached content
 *
 * @param string $keyName
 * @param long $lifetime
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Xcache, get){

	zval *key_name, frontend = {}, prefix = {}, prefixed_key = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);
	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);

	PHALCON_CONCAT_SVV(&prefixed_key, "_PHCX", &prefix, key_name);

	PHALCON_CALL_FUNCTION(&cached_content, "xcache_get", &prefixed_key);
	if (Z_TYPE(cached_content) == IS_NULL) {
		RETURN_NULL();
	}

	if (phalcon_is_numeric(&cached_content)) {
		RETURN_CTOR(&cached_content);
	} else {
		PHALCON_RETURN_CALL_METHOD(&frontend, "afterretrieve", &cached_content);
	}
}

/**
 * Stores cached content into the XCache backend and stops the frontend
 *
 * @param string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Xcache, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, key = {}, cached_content = {}, keys = {}, last_key = {}, frontend = {};
	zval prepared_content = {}, ttl = {}, success = {}, is_buffering = {}, prefix = {}, prefixed_key = {}, options = {}, special_key = {};

	phalcon_fetch_params(0, 0, 4, &key_name, &content, &lifetime, &stop_buffer);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);

	if (!key_name || Z_TYPE_P(key_name) == IS_NULL) {
		phalcon_read_property(&key, getThis(), SL("_lastKey"), PH_READONLY);
		key_name = &key;
	}

	if (!zend_is_true(key_name)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The cache must be started first");
		return;
	}

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	PHALCON_CONCAT_SV(&last_key, "_PHCX", &prefixed_key);

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);
	if (!content || Z_TYPE_P(content) == IS_NULL) {
		PHALCON_CALL_METHOD(&cached_content, &frontend, "getcontent");
	} else {
		ZVAL_COPY_VALUE(&cached_content, content);
	}

	if (!phalcon_is_numeric(&cached_content)) {
		PHALCON_CALL_METHOD(&prepared_content, &frontend, "beforestore", &cached_content);
	}

	/**
	 * Take the lifetime from the frontend or read it from the set in start()
	 */
	if (!lifetime || Z_TYPE_P(lifetime) != IS_LONG) {
		PHALCON_CALL_METHOD(&ttl, getThis(), "getlifetime");
	} else {
		ZVAL_COPY_VALUE(&ttl, lifetime);
	}

	if (Z_TYPE(prepared_content) > IS_NULL) {
		PHALCON_CALL_FUNCTION(&success, "xcache_set", &last_key, &prepared_content, &ttl);
	} else {
		PHALCON_CALL_FUNCTION(&success, "xcache_set", &last_key, &cached_content, &ttl);
	}

	PHALCON_CALL_METHOD(&is_buffering, &frontend, "isbuffering");
	if (!stop_buffer || PHALCON_IS_TRUE(stop_buffer)) {
		PHALCON_CALL_METHOD(NULL, &frontend, "stop");
	}

	if (PHALCON_IS_TRUE(&is_buffering)) {
		zend_print_zval(&cached_content, 0);
	}

	phalcon_update_property_bool(getThis(), SL("_started"), 0);

	if (zend_is_true(&success)) {
		phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

		if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
			return;
		}

		/**
		 * xcache_list() is available only to the administrator (unless XCache was
		 * patched). We have to update the list of the stored keys.
		 */
		PHALCON_CALL_FUNCTION(&keys, "xcache_get", &special_key);
		if (Z_TYPE(keys) != IS_ARRAY) {
			array_init(&keys);
		}

		phalcon_array_update(&keys, &prefixed_key, &ttl, PH_COPY);
		PHALCON_CALL_FUNCTION(NULL, "xcache_set", &special_key, &keys, &PHALCON_GLOBAL(z_zero));
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

/**
 * Deletes a value from the cache by its key
 *
 * @param string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Xcache, delete){

	zval *key_name, prefix = {}, prefixed_key = {}, options = {}, special_key = {}, keys = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);
	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	PHALCON_CONCAT_SVV(&prefixed_key, "_PHCX", &prefix, key_name);

	PHALCON_CALL_FUNCTION(NULL, "xcache_unset", &prefixed_key);

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	PHALCON_CALL_FUNCTION(&keys, "xcache_get", &special_key);
	if (Z_TYPE(keys) == IS_ARRAY) {
		phalcon_array_unset(&keys, &prefixed_key, 0);
		PHALCON_CALL_FUNCTION(NULL, "xcache_set", &special_key, &keys, &PHALCON_GLOBAL(z_zero));
	}
}

/**
 * Query the existing cached keys
 *
 * @param string $prefix
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_Xcache, queryKeys){

	zval *prefix = NULL, prefixed = {}, options = {}, special_key = {}, keys = {};
	zend_string *str_key;

	phalcon_fetch_params(0, 0, 1, &prefix);

	if (!prefix) {
		ZVAL_STRING(&prefixed, "_PHCX");
	} else {
		PHALCON_CONCAT_SV(&prefixed, "_PHCX", prefix);
	}

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	array_init(return_value);

	/**
	 * Get the key from XCache (we cannot use xcache_list() as it is available only to
	 * the administrator)
	 */
	PHALCON_CALL_FUNCTION(&keys, "xcache_get", &special_key);
	if (Z_TYPE(keys) == IS_ARRAY) {
		ZEND_HASH_FOREACH_STR_KEY(Z_ARRVAL(keys), str_key) {
			zval real_key = {};
			if (str_key && phalcon_memnstr_string_string(str_key, Z_STR(prefixed))) {
				phalcon_substr_string(&real_key, str_key, 5, PH_COPY);
				phalcon_array_append(return_value, &real_key, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * Checks if the cache entry exists and has not expired
 *
 * @param string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Xcache, exists){

	zval *key_name = NULL, prefix = {}, last_key = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);
	PHALCON_CONCAT_SVV(&last_key, "_PHCX", &prefix, key_name);

	PHALCON_RETURN_CALL_FUNCTION("xcache_isset", &last_key);
}

/**
 * Atomic increment of a given key, by number $value
 *
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Xcache, increment){

	zval *key_name = NULL, *value = NULL, last_key = {}, prefix = {}, orig_val = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	}

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);
	PHALCON_CONCAT_SVV(&last_key, "_PHCX", &prefix, key_name);

	if (phalcon_function_exists_ex(SL("xcache_inc")) == SUCCESS) {
		PHALCON_RETURN_CALL_FUNCTION("xcache_inc", &last_key, value);
	} else {
		PHALCON_CALL_FUNCTION(&orig_val, "xcache_get", &last_key);
		add_function(return_value, &orig_val, value);
		PHALCON_CALL_METHOD(NULL, getThis(), "save", key_name, return_value);
	}
}

/**
 * Atomic decrement of a given key, by number $value
 *
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Xcache, decrement){

	zval *key_name = NULL, *value = NULL, last_key = {}, prefix = {}, orig_val = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	}

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);
	PHALCON_CONCAT_SVV(&last_key, "_PHCX", &prefix, key_name);

	if (phalcon_function_exists_ex(SL("xcache_inc")) == SUCCESS) {
		PHALCON_RETURN_CALL_FUNCTION("xcache_inc", &last_key, value);
	} else {
		PHALCON_CALL_FUNCTION(&orig_val, "xcache_get", &last_key);
		phalcon_sub_function(return_value, &orig_val, value);
		PHALCON_CALL_METHOD(NULL, getThis(), "save", key_name, return_value);
	}
}

/**
 * Immediately invalidates all existing items.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Xcache, flush){

	zval prefixed = {}, options = {}, special_key = {}, keys = {};
	zend_string *str_key;
	ulong idx;

	ZVAL_STRING(&prefixed, "_PHCX");

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	PHALCON_CALL_FUNCTION(&keys, "xcache_get", &special_key);
	if (Z_TYPE(keys) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY(Z_ARRVAL(keys), idx, str_key) {
			zval key = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}

			PHALCON_CALL_FUNCTION(NULL, "xcache_unset", &key);
		} ZEND_HASH_FOREACH_END();

		zend_hash_clean(Z_ARRVAL(keys));
		PHALCON_CALL_FUNCTION(NULL, "xcache_set", &special_key, &keys, &PHALCON_GLOBAL(z_zero));
	}

	RETURN_TRUE;
}
