
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

#include "cache/backend/yac.h"
#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/exception.h"
#include "cache/yac.h"

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
 * Phalcon\Cache\Backend\Yac
 *
 * Allows to cache output fragments, PHP data or raw data to a yac backend
 *
 * This adapter uses the special yacd key "_PHCY" to store all the keys internally used by the adapter
 *
 *<code>
 *
 * // Cache data for 2 days
 * $frontCache = new Phalcon\Cache\Frontend\Data(array(
 *    "lifetime" => 172800
 * ));
 *
 * //Create the Cache setting yacd connection options
 * $cache = new Phalcon\Cache\Backend\Yac($frontCache, array(
 *		'prefix' => "myproduct_"
 * ));
 *
 * //Cache arbitrary data
 * $cache->save('my-data', array(1, 2, 3, 4, 5));
 *
 * //Get data
 * $data = $cache->get('my-data');
 *
 *</code>
 */
zend_class_entry *phalcon_cache_backend_yac_ce;

PHP_METHOD(Phalcon_Cache_Backend_Yac, __construct);
PHP_METHOD(Phalcon_Cache_Backend_Yac, _connect);
PHP_METHOD(Phalcon_Cache_Backend_Yac, get);
PHP_METHOD(Phalcon_Cache_Backend_Yac, save);
PHP_METHOD(Phalcon_Cache_Backend_Yac, delete);
PHP_METHOD(Phalcon_Cache_Backend_Yac, queryKeys);
PHP_METHOD(Phalcon_Cache_Backend_Yac, exists);
PHP_METHOD(Phalcon_Cache_Backend_Yac, increment);
PHP_METHOD(Phalcon_Cache_Backend_Yac, decrement);
PHP_METHOD(Phalcon_Cache_Backend_Yac, flush);
PHP_METHOD(Phalcon_Cache_Backend_Yac, getTrackingKey);
PHP_METHOD(Phalcon_Cache_Backend_Yac, setTrackingKey);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_yac___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_yac_settrackingkey, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_yac_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_Yac, __construct, arginfo_phalcon_cache_backend_yac___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend_Yac, _connect, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Cache_Backend_Yac, get, arginfo_phalcon_cache_backendinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Yac, save, arginfo_phalcon_cache_backendinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Yac, delete, arginfo_phalcon_cache_backendinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Yac, queryKeys, arginfo_phalcon_cache_backendinterface_querykeys, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Yac, exists, arginfo_phalcon_cache_backendinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Yac, increment, arginfo_phalcon_cache_backendinterface_increment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Yac, decrement, arginfo_phalcon_cache_backendinterface_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Yac, flush, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Yac, getTrackingKey, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Yac, setTrackingKey, arginfo_phalcon_cache_backend_yac_settrackingkey, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend\Yac initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Yac)
{
	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, Yac, cache_backend_yac, phalcon_cache_backend_ce, phalcon_cache_backend_yac_method_entry, 0);

	zend_declare_property_null(phalcon_cache_backend_yac_ce, SL("_yac"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_cache_backend_yac_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\Yac constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_Yac, __construct){

	zval *frontend, *_options = NULL, options = {}, special_key = {};

	phalcon_fetch_params(0, 1, 1, &frontend, &_options);

	if (!_options || Z_TYPE_P(_options) == IS_NULL) {
		array_init_size(&options, 4);
	} else {
		ZVAL_DUP(&options, _options);
	}

	if (!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) || Z_TYPE(special_key) == IS_TRUE) {
		phalcon_array_update_str_str(&options, SL("statsKey"), SL("_PHCY"), 0);
	}

	PHALCON_CALL_PARENT(NULL, phalcon_cache_backend_yac_ce, getThis(), "__construct", frontend, &options);
	zval_ptr_dtor(&options);
}

/**
 * Create internal connection to yacd
 */
PHP_METHOD(Phalcon_Cache_Backend_Yac, _connect)
{
	zval options = {}, prefix = {}, yac = {};
	zend_class_entry *ce0;

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);
#ifdef PHALCON_CACHE_YAC
	ce0 = phalcon_fetch_str_class(SL("Yac"), ZEND_FETCH_CLASS_AUTO | ZEND_FETCH_CLASS_SILENT);
	if (!ce0) {
		ce0 = phalcon_cache_yac_ce;
	}
#else
	ce0 = phalcon_fetch_str_class(SL("Yac"), ZEND_FETCH_CLASS_AUTO);
#endif

	if (!phalcon_array_isset_fetch_str(&prefix, &options, SL("prefix"), PH_READONLY)) {
		ZVAL_NULL(&prefix);
	}

	object_init_ex(&yac, ce0);
	if (phalcon_has_constructor(&yac)) {
		PHALCON_CALL_METHOD(NULL, &yac, "__construct", &prefix);
	}

	phalcon_update_property(getThis(), SL("_yac"), &yac);
	RETURN_NCTOR(&yac);
}

/**
 * Returns a cached content
 *
 * @param int|string $keyName
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Yac, get){

	zval *key_name, yac = {}, frontend = {}, last_key = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&yac, getThis(), SL("_yac"), PH_COPY);
	if (Z_TYPE(yac) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&yac, getThis(), "_connect");
	}

	PHALCON_CONCAT_SV(&last_key, "_PHCY", key_name);

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);

	PHALCON_CALL_METHOD(&cached_content, &yac, "get", &last_key);
	zval_ptr_dtor(&last_key);
	zval_ptr_dtor(&yac);
	if (PHALCON_IS_FALSE(&cached_content)) {
		RETURN_NULL();
	}

	if (phalcon_is_numeric(&cached_content)) {
		RETURN_NCTOR(&cached_content);
	}

	PHALCON_RETURN_CALL_METHOD(&frontend, "afterretrieve", &cached_content);
	zval_ptr_dtor(&cached_content);
}

/**
 * Stores cached content into the Yacd backend and stops the frontend
 *
 * @param int|string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Yac, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, key = {}, last_key = {}, cached_content = {}, prepared_content = {}, success = {};
	zval ttl = {}, is_buffering = {}, frontend = {}, yac = {}, options = {}, special_key = {}, keys = {};

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
		ZVAL_COPY(&ttl, lifetime);
	}

	PHALCON_CONCAT_SV(&last_key, "_PHCY", key_name);

	/**
	 * Check if a connection is created or make a new one
	 */
	phalcon_read_property(&yac, getThis(), SL("_yac"), PH_COPY);
	if (Z_TYPE(yac) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&yac, getThis(), "_connect");
	}

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);
	if (!content || Z_TYPE_P(content) == IS_NULL) {
		PHALCON_CALL_METHOD(&cached_content, &frontend, "getcontent");
	} else {
		ZVAL_COPY(&cached_content, content);
	}

	/**
	 * Prepare the content in the frontend
	 */

	if (phalcon_is_numeric(&cached_content)) {
		PHALCON_CALL_METHOD(&success, &yac, "set", &last_key, &cached_content, &ttl);
	} else {
		PHALCON_CALL_METHOD(&prepared_content, &frontend, "beforestore", &cached_content);
		PHALCON_CALL_METHOD(&success, &yac, "set", &last_key, &prepared_content, &ttl);
		zval_ptr_dtor(&prepared_content);
	}
	zval_ptr_dtor(&last_key);

	if (!zend_is_true(&success)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Failed to store data in yac");
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

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) && PHALCON_IS_NOT_EMPTY_STRING(&special_key)) {
		PHALCON_CALL_METHOD(&keys, &yac, "get", &special_key);
		if (Z_TYPE(keys) != IS_ARRAY) {
			array_init(&keys);
		}

		phalcon_array_update(&keys, key_name, &ttl, PH_COPY);
		PHALCON_CALL_METHOD(&success, &yac, "set", &special_key, &keys);
		zval_ptr_dtor(&keys);
	}
	zval_ptr_dtor(&yac);
	RETURN_TRUE;
}

/**
 * Deletes a value from the cache by its key
 *
 * @param int|string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Yac, delete){

	zval *key_name, yac = {}, last_key = {}, options = {}, special_key = {}, ret = {}, keys = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&yac, getThis(), SL("_yac"), PH_COPY);
	if (Z_TYPE(yac) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&yac, getThis(), "_connect");
	}

	PHALCON_CONCAT_SV(&last_key, "_PHCY", key_name);

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	/* Delete the key from yacd */
	PHALCON_CALL_METHOD(&ret, &yac, "delete", &last_key);
	zval_ptr_dtor(&last_key);
	if (zend_is_true(&ret)) {
		if (phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) && PHALCON_IS_NOT_EMPTY_STRING(&special_key)) {
			PHALCON_CALL_METHOD(&keys, &yac, "get", &special_key);
			if (Z_TYPE(keys) == IS_ARRAY) {
				phalcon_array_unset(&keys, key_name, 0);
				PHALCON_CALL_METHOD(return_value, &yac, "set", &special_key, &keys);
			}
			zval_ptr_dtor(&keys);
		}
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
	zval_ptr_dtor(&yac);
}

/**
 * Query the existing cached keys
 *
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_Yac, queryKeys){

	zval *prefix = NULL, yac = {}, options = {}, special_key = {}, keys = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 1, &prefix);

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);
	if (!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) || !PHALCON_IS_NOT_EMPTY_STRING(&special_key)) {
		zend_throw_exception_ex(phalcon_cache_exception_ce, 0, "Unexpected inconsistency in options");
		return;
	}

	phalcon_read_property(&yac, getThis(), SL("_yac"), PH_COPY);
	if (Z_TYPE(yac) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&yac, getThis(), "_connect");
	}

	array_init(return_value);

	/* Get the key from yacd */
	PHALCON_CALL_METHOD(&keys, &yac, "get", &special_key);
	if (Z_TYPE(keys) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY(Z_ARRVAL(keys), idx, str_key) {
			zval key = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}

			if (!prefix || !zend_is_true(prefix) || phalcon_start_with(&key, prefix, NULL)) {
				phalcon_array_append(return_value, &key, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}
	zval_ptr_dtor(&keys);
	zval_ptr_dtor(&yac);
}

/**
 * Checks if cache exists and it hasn't expired
 *
 * @param  string $keyName
 * @param  long $lifetime
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Yac, exists){

	zval *key_name, value = {}, last_key = {}, yac = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	PHALCON_CONCAT_SV(&last_key, "_PHCY", key_name);

	phalcon_read_property(&yac, getThis(), SL("_yac"), PH_COPY);
	if (Z_TYPE(yac) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&yac, getThis(), "_connect");
	}

	PHALCON_CALL_METHOD(&value, &yac, "get", &last_key);
	zval_ptr_dtor(&last_key);
	zval_ptr_dtor(&yac);
	RETVAL_BOOL(PHALCON_IS_NOT_FALSE(&value));
	zval_ptr_dtor(&value);
}

/**
 * Atomic increment of a given key, by number $value
 *
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Yac, increment){

	zval *key_name, *value = NULL, yac = {}, last_key = {}, cached_content = {}, tmp = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	PHALCON_CONCAT_SV(&last_key, "_PHCY", key_name);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	phalcon_read_property(&yac, getThis(), SL("_yac"), PH_COPY);
	if (Z_TYPE(yac) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&yac, getThis(), "_connect");
	}

	PHALCON_CALL_METHOD(&cached_content, &yac, "get", &last_key);
	phalcon_add_function(&tmp, &cached_content, value);
	zval_ptr_dtor(&cached_content);

	PHALCON_RETURN_CALL_METHOD(&yac, "set", &last_key, &tmp);
	zval_ptr_dtor(&last_key);
	zval_ptr_dtor(&yac);
}

/**
 * Atomic decrement of a given key, by number $value
 *
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Yac, decrement){

	zval *key_name, *value = NULL, yac = {}, last_key = {}, cached_content = {}, tmp = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	PHALCON_CONCAT_SV(&last_key, "_PHCY", key_name);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	phalcon_read_property(&yac, getThis(), SL("_yac"), PH_COPY);
	if (Z_TYPE(yac) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&yac, getThis(), "_connect");
	}

	PHALCON_CALL_METHOD(&cached_content, &yac, "get", &last_key);
	phalcon_sub_function(&tmp, &cached_content, value);
	zval_ptr_dtor(&cached_content);

	PHALCON_RETURN_CALL_METHOD(&yac, "set", &last_key, &tmp);
	zval_ptr_dtor(&yac);
}

/**
 * Immediately invalidates all existing items.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Yac, flush){

	zval yac = {}, options = {}, special_key = {};

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) || !PHALCON_IS_NOT_EMPTY_STRING(&special_key)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	phalcon_read_property(&yac, getThis(), SL("_yac"), PH_COPY);
	if (Z_TYPE(yac) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&yac, getThis(), "_connect");
	}

	/* Get the key from yacd */
	PHALCON_CALL_METHOD(return_value, &yac, "flush");
	zval_ptr_dtor(&yac);
}

PHP_METHOD(Phalcon_Cache_Backend_Yac, getTrackingKey)
{
	zval options = {};

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (!phalcon_array_isset_fetch_str(return_value, &options, SL("statsKey"), PH_COPY)) {
		RETURN_NULL();
	}
}

PHP_METHOD(Phalcon_Cache_Backend_Yac, setTrackingKey)
{
	zval *key;

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_update_property_array_str(getThis(), SL("_options"), SL("statsKey"), key);

	RETURN_THIS();
}
