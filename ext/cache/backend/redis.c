
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

#include "cache/backend/redis.h"
#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/exception.h"

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
 * Phalcon\Cache\Backend\Redis
 *
 * Allows to cache output fragments, PHP data or raw data to a redis backend
 *
 * This adapter uses the special redisd key "_PHCR" to store all the keys internally used by the adapter
 *
 *<code>
 *
 * // Cache data for 2 days
 * $frontCache = new Phalcon\Cache\Frontend\Data(array(
 *    "lifetime" => 172800
 * ));
 *
 * //Create the Cache setting redisd connection options
 * $cache = new Phalcon\Cache\Backend\Redis($frontCache, array(
 *		'host' => 'localhost',
 *		'port' => 6379,
 *		'auth' => 'foobared',
 *  	'persistent' => false
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
zend_class_entry *phalcon_cache_backend_redis_ce;

PHP_METHOD(Phalcon_Cache_Backend_Redis, __construct);
PHP_METHOD(Phalcon_Cache_Backend_Redis, _connect);
PHP_METHOD(Phalcon_Cache_Backend_Redis, get);
PHP_METHOD(Phalcon_Cache_Backend_Redis, save);
PHP_METHOD(Phalcon_Cache_Backend_Redis, delete);
PHP_METHOD(Phalcon_Cache_Backend_Redis, queryKeys);
PHP_METHOD(Phalcon_Cache_Backend_Redis, exists);
PHP_METHOD(Phalcon_Cache_Backend_Redis, increment);
PHP_METHOD(Phalcon_Cache_Backend_Redis, decrement);
PHP_METHOD(Phalcon_Cache_Backend_Redis, flush);
PHP_METHOD(Phalcon_Cache_Backend_Redis, flushDb);
PHP_METHOD(Phalcon_Cache_Backend_Redis, getTrackingKey);
PHP_METHOD(Phalcon_Cache_Backend_Redis, setTrackingKey);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_redis___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_redis_settrackingkey, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_redis_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_Redis, __construct, arginfo_phalcon_cache_backend_redis___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend_Redis, _connect, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Cache_Backend_Redis, get, arginfo_phalcon_cache_backendinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Redis, save, arginfo_phalcon_cache_backendinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Redis, delete, arginfo_phalcon_cache_backendinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Redis, queryKeys, arginfo_phalcon_cache_backendinterface_querykeys, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Redis, exists, arginfo_phalcon_cache_backendinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Redis, increment, arginfo_phalcon_cache_backendinterface_increment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Redis, decrement, arginfo_phalcon_cache_backendinterface_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Redis, flush, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Redis, flushDb, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Redis, getTrackingKey, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Redis, setTrackingKey, arginfo_phalcon_cache_backend_redis_settrackingkey, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend\Redis initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Redis)
{
	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, Redis, cache_backend_redis, phalcon_cache_backend_ce, phalcon_cache_backend_redis_method_entry, 0);

	zend_declare_property_null(phalcon_cache_backend_redis_ce, SL("_redis"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cache_backend_redis_ce, SL("_auth"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_cache_backend_redis_ce, SL("_prefix"), "_PHCR", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_cache_backend_redis_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\Redis constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, __construct){

	zval *frontend, *_options = NULL, options = {}, special_key = {}, redis = {};

	phalcon_fetch_params(1, 1, 1, &frontend, &_options);

	if (!_options || Z_TYPE_P(_options) == IS_NULL) {
		array_init_size(&options, 4);
	} else {
		ZVAL_DUP(&options, _options);
	}
	PHALCON_MM_ADD_ENTRY(&options);

	if (!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) || Z_TYPE(special_key) == IS_TRUE) {
		phalcon_array_update_str_str(&options, SL("statsKey"), SL("_PHCR"), 0);
	}

	if (!phalcon_array_isset_fetch_str(&redis, &options, SL("redis"), PH_READONLY)) {
		if (!phalcon_array_isset_str(&options, SL("host"))) {
			phalcon_array_update_str_str(&options, SL("host"), SL("127.0.0.1"), 0);
		}

		if (!phalcon_array_isset_str(&options, SL("port"))) {
			phalcon_array_update_str_long(&options, SL("port"), 6379, 0);
		}

		if (!phalcon_array_isset_str(&options, SL("persistent"))) {
			phalcon_array_update_str_bool(&options, SL("persistent"), 0, 0);
		}
	} else {
		zend_class_entry *ce0;
		ce0 = phalcon_fetch_str_class(SL("Redis"), ZEND_FETCH_CLASS_AUTO);
		if (Z_TYPE(redis) == IS_STRING) {
			zval service = {};
			PHALCON_MM_CALL_METHOD(&service, getThis(), "getresolveservice", &redis);
			PHALCON_MM_ADD_ENTRY(&service);
			PHALCON_MM_VERIFY_CLASS(&service, ce0);
			phalcon_update_property(getThis(), SL("_redis"), &service);
		} else {
			PHALCON_MM_VERIFY_CLASS(&redis, ce0);
			phalcon_update_property(getThis(), SL("_redis"), &redis);
		}
	}

	PHALCON_MM_CALL_PARENT(NULL, phalcon_cache_backend_redis_ce, getThis(), "__construct", frontend, &options);
	RETURN_MM();
}

/**
 * Create internal connection to redisd
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, _connect)
{
	zval options = {}, redis = {}, host = {}, port = {}, persistent = {}, success = {}, auth = {}, db = {};
	zend_class_entry *ce0;

	PHALCON_MM_INIT();

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (
		   !phalcon_array_isset_fetch_str(&host, &options, SL("host"), PH_READONLY)
		|| !phalcon_array_isset_fetch_str(&port, &options, SL("port"), PH_READONLY)
		|| !phalcon_array_isset_fetch_str(&persistent, &options, SL("persistent"), PH_READONLY)
	) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	ce0 = phalcon_fetch_str_class(SL("Redis"), ZEND_FETCH_CLASS_AUTO);

	object_init_ex(&redis, ce0);
	PHALCON_MM_ADD_ENTRY(&redis);
	if (phalcon_has_constructor(&redis)) {
		PHALCON_MM_CALL_METHOD(NULL, &redis, "__construct");
	}

	if (zend_is_true(&persistent)) {
		PHALCON_MM_CALL_METHOD(&success, &redis, "pconnect", &host, &port);
	} else {
		PHALCON_MM_CALL_METHOD(&success, &redis, "connect", &host, &port);
	}
	PHALCON_MM_ADD_ENTRY(&success);

	if (!zend_is_true(&success)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Cannot connect to Redisd server");
		return;
	}

	if (phalcon_array_isset_fetch_str(&auth, &options, SL("auth"), PH_READONLY) && PHALCON_IS_NOT_EMPTY(&auth)) {
		PHALCON_MM_CALL_METHOD(&success, &redis, "auth", &auth);
		PHALCON_MM_ADD_ENTRY(&success);
		if (!zend_is_true(&success)) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Redisd server is authentication failed");
			return;
		}
	}

	if (phalcon_array_isset_fetch_str(&db, &options, SL("db"), PH_READONLY) && Z_TYPE(db) != IS_NULL) {
		PHALCON_MM_CALL_METHOD(NULL, &redis, "select", &db);
	}

	phalcon_update_property(getThis(), SL("_redis"), &redis);
	RETURN_MM_CTOR(&redis);
}

/**
 * Returns a cached content
 *
 * @param int|string $keyName
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, get){

	zval *key_name, redis = {}, frontend = {}, prefix = {}, prefixed_key = {}, cached_content = {};

	phalcon_fetch_params(1, 1, 0, &key_name);

	phalcon_read_property(&redis, getThis(), SL("_redis"), PH_READONLY);
	if (Z_TYPE(redis) != IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&redis, getThis(), "_connect");
		PHALCON_MM_ADD_ENTRY(&redis);
	}

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);
	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	PHALCON_MM_ADD_ENTRY(&prefixed_key);

	PHALCON_MM_CALL_METHOD(&cached_content, &redis, "get", &prefixed_key);
	PHALCON_MM_ADD_ENTRY(&cached_content);
	if (PHALCON_IS_FALSE(&cached_content)) {
		RETURN_MM_NULL();
	}

	if (phalcon_is_numeric(&cached_content)) {
		RETURN_MM_CTOR(&cached_content);
	}

	PHALCON_RETURN_CALL_METHOD(&frontend, "afterretrieve", &cached_content);
	RETURN_MM();
}

/**
 * Stores cached content into the Redisd backend and stops the frontend
 *
 * @param int|string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, key = {}, prefix = {}, prefixed_key = {}, cached_content = {}, prepared_content = {}, success = {};
	zval ttl = {}, is_buffering = {}, frontend = {}, redis = {}, options = {}, special_key = {};

	phalcon_fetch_params(1, 0, 4, &key_name, &content, &lifetime, &stop_buffer);
	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);

	if (!key_name || Z_TYPE_P(key_name) == IS_NULL) {
		phalcon_read_property(&key, getThis(), SL("_lastKey"), PH_READONLY);
		key_name = &key;
	}

	if (!zend_is_true(key_name)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The cache must be started first");
		return;
	}

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	PHALCON_MM_ADD_ENTRY(&prefixed_key);

	/**
	 * Check if a connection is created or make a new one
	 */
	phalcon_read_property(&redis, getThis(), SL("_redis"), PH_READONLY);
	if (Z_TYPE(redis) != IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&redis, getThis(), "_connect");
		PHALCON_MM_ADD_ENTRY(&redis);
	}

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);
	if (!content || Z_TYPE_P(content) == IS_NULL) {
		PHALCON_MM_CALL_METHOD(&cached_content, &frontend, "getcontent");
	} else {
		ZVAL_COPY(&cached_content, content);
	}
	PHALCON_MM_ADD_ENTRY(&cached_content);

	/**
	 * Take the lifetime from the frontend or read it from the set in start()
	 */
	if (!lifetime || Z_TYPE_P(lifetime) != IS_LONG) {
		PHALCON_MM_CALL_METHOD(&ttl, getThis(), "getlifetime");
		PHALCON_MM_ADD_ENTRY(&ttl);
	} else {
		ZVAL_COPY_VALUE(&ttl, lifetime);
	}

	if (phalcon_is_numeric(&cached_content)) {
		PHALCON_MM_CALL_METHOD(&success, &redis, "set", &prefixed_key, &cached_content, &ttl);
		PHALCON_MM_ADD_ENTRY(&success);
	} else {
		/**
		 * Prepare the content in the frontend
		 */
		PHALCON_MM_CALL_METHOD(&prepared_content, &frontend, "beforestore", &cached_content);
		PHALCON_MM_ADD_ENTRY(&prepared_content);
		PHALCON_MM_CALL_METHOD(&success, &redis, "set", &prefixed_key, &prepared_content);
		PHALCON_MM_ADD_ENTRY(&success);
	}

	if (!zend_is_true(&success)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Failed to store data in redisd");
		return;
	}

	if (!zend_is_true(&success)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Failed to store data in redisd");
		return;
	}

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) && PHALCON_IS_NOT_EMPTY_STRING(&special_key)) {
		PHALCON_MM_CALL_METHOD(NULL, &redis, "sadd", &special_key, &prefixed_key);
	}

	PHALCON_MM_CALL_METHOD(&is_buffering, &frontend, "isbuffering");
	PHALCON_MM_ADD_ENTRY(&is_buffering);

	if (!stop_buffer || PHALCON_IS_TRUE(stop_buffer)) {
		PHALCON_MM_CALL_METHOD(NULL, &frontend, "stop");
	}

	if (PHALCON_IS_TRUE(&is_buffering)) {
		zend_print_zval(&cached_content, 0);
	}

	phalcon_update_property_bool(getThis(), SL("_started"), 0);
	RETURN_MM_TRUE;
}

/**
 * Deletes a value from the cache by its key
 *
 * @param int|string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, delete){

	zval *key_name, redis = {}, prefix = {}, prefixed_key = {}, options = {}, special_key = {}, ret = {};

	phalcon_fetch_params(1, 1, 0, &key_name);

	phalcon_read_property(&redis, getThis(), SL("_redis"), PH_READONLY);
	if (Z_TYPE(redis) != IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&redis, getThis(), "_connect");
		PHALCON_MM_ADD_ENTRY(&redis);
	}

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	PHALCON_MM_ADD_ENTRY(&prefixed_key);

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) && PHALCON_IS_NOT_EMPTY_STRING(&special_key)) {
		PHALCON_MM_CALL_METHOD(NULL, &redis, "srem", &special_key, &prefixed_key);
	}

	/* Delete the key from redisd */
	PHALCON_MM_CALL_METHOD(&ret, &redis, "delete", &prefixed_key);
	PHALCON_MM_ADD_ENTRY(&ret);
	if (zend_is_true(&ret)) {
		RETURN_MM_TRUE;
	}

	RETURN_MM_FALSE;
}

/**
 * Query the existing cached keys
 *
 * @param string $prefix
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, queryKeys){

	zval *prefix = NULL, redis = {}, options = {}, special_key = {}, keys = {}, *value;

	phalcon_fetch_params(1, 0, 1, &prefix);

	array_init(return_value);

	phalcon_read_property(&redis, getThis(), SL("_redis"), PH_READONLY);
	if (Z_TYPE(redis) != IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&redis, getThis(), "_connect");
		PHALCON_MM_ADD_ENTRY(&redis);
	}

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);
	if (!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) || !PHALCON_IS_NOT_EMPTY_STRING(&special_key)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	/* Get the key from redisd */
	PHALCON_MM_CALL_METHOD(&keys, &redis, "smembers", &special_key);
	PHALCON_MM_ADD_ENTRY(&keys);
	if (Z_TYPE(keys) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(keys), value) {
			zval real_key = {};
			if (!prefix || !zend_is_true(prefix) || phalcon_start_with(value, prefix, NULL)) {
				ZVAL_NEW_STR(&real_key, Z_STR_P(value));
				phalcon_array_append(return_value, &real_key, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}
	RETURN_MM();
}

/**
 * Checks if cache exists and it hasn't expired
 *
 * @param  string $keyName
 * @param  long $lifetime
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, exists){

	zval *key_name, value = {}, prefixed_key = {}, prefix = {}, redis = {};

	phalcon_fetch_params(1, 1, 0, &key_name);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);
	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	PHALCON_MM_ADD_ENTRY(&prefixed_key);

	phalcon_read_property(&redis, getThis(), SL("_redis"), PH_READONLY);
	if (Z_TYPE(redis) != IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&redis, getThis(), "_connect");
		PHALCON_MM_ADD_ENTRY(&redis);
	}

	PHALCON_MM_CALL_METHOD(&value, &redis, "get", &prefixed_key);
	PHALCON_MM_ADD_ENTRY(&value);

	RETVAL_BOOL(PHALCON_IS_NOT_FALSE(&value));
	RETURN_MM();
}

/**
 * Atomic increment of a given key, by number $value
 *
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, increment){

	zval *key_name, *value = NULL, redis = {}, prefixed_key = {}, prefix = {};

	phalcon_fetch_params(1, 1, 1, &key_name, &value);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);
	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	PHALCON_MM_ADD_ENTRY(&prefixed_key);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	phalcon_read_property(&redis, getThis(), SL("_redis"), PH_READONLY);
	if (Z_TYPE(redis) != IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&redis, getThis(), "_connect");
		PHALCON_MM_ADD_ENTRY(&redis);
	}

	PHALCON_MM_CALL_METHOD(return_value, &redis, "incrby", &prefixed_key, value);
	RETURN_MM();
}

/**
 * Atomic decrement of a given key, by number $value
 *
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, decrement){

	zval *key_name, *value = NULL, redis = {}, prefixed_key = {}, prefix = {};

	phalcon_fetch_params(1, 1, 1, &key_name, &value);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);
	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	PHALCON_MM_ADD_ENTRY(&prefixed_key);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	phalcon_read_property(&redis, getThis(), SL("_redis"), PH_READONLY);
	if (Z_TYPE(redis) != IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&redis, getThis(), "_connect");
		PHALCON_MM_ADD_ENTRY(&redis);
	}

	PHALCON_MM_CALL_METHOD(return_value, &redis, "decrby", &prefixed_key, value);
	RETURN_MM();
}

/**
 * Immediately invalidates all existing items.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, flush){

	zval redis = {}, options = {}, special_key = {}, keys = {}, *value;

	PHALCON_MM_INIT();

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) || !PHALCON_IS_NOT_EMPTY_STRING(&special_key)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	phalcon_read_property(&redis, getThis(), SL("_redis"), PH_READONLY);
	if (Z_TYPE(redis) != IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&redis, getThis(), "_connect");
		PHALCON_MM_ADD_ENTRY(&redis);
	}

	/* Get the key from redisd */
	PHALCON_MM_CALL_METHOD(&keys, &redis, "smembers", &special_key);
	PHALCON_MM_ADD_ENTRY(&keys);
	if (Z_TYPE(keys) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(keys), value) {
			PHALCON_MM_CALL_METHOD(NULL, &redis, "delete", value);
			PHALCON_MM_CALL_METHOD(NULL, &redis, "srem", &special_key, value);
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_MM_TRUE;
}

/**
 * Remove all keys from the current database.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Redis, flushDb){

	zval redis = {};

	PHALCON_MM_INIT();

	phalcon_read_property(&redis, getThis(), SL("_redis"), PH_READONLY);
	if (Z_TYPE(redis) != IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&redis, getThis(), "_connect");
		PHALCON_MM_ADD_ENTRY(&redis);
	}

	PHALCON_MM_CALL_METHOD(return_value, &redis, "flushdb");
	RETURN_MM();
}

PHP_METHOD(Phalcon_Cache_Backend_Redis, getTrackingKey)
{
	zval options = {};

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (!phalcon_array_isset_fetch_str(return_value, &options, SL("statsKey"), PH_COPY)) {
		RETURN_NULL();
	}
}

PHP_METHOD(Phalcon_Cache_Backend_Redis, setTrackingKey)
{
	zval *key;

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_update_property_array_str(getThis(), SL("_options"), SL("statsKey"), key);

	RETURN_THIS();
}
