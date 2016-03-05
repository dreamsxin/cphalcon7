
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

#include "cache/backend/memcache.h"
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
 * Phalcon\Cache\Backend\Memcache
 *
 * Allows to cache output fragments, PHP data or raw data to a memcache backend
 *
 * This adapter uses the special memcached key "_PHCM" to store all the keys internally used by the adapter
 *
 *<code>
 *
 * // Cache data for 2 days
 * $frontCache = new Phalcon\Cache\Frontend\Data(array(
 *    "lifetime" => 172800
 * ));
 *
 * //Create the Cache setting memcached connection options
 * $cache = new Phalcon\Cache\Backend\Memcache($frontCache, array(
 *		'host' => 'localhost',
 *		'port' => 11211,
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
zend_class_entry *phalcon_cache_backend_memcache_ce;

PHP_METHOD(Phalcon_Cache_Backend_Memcache, __construct);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, _connect);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, get);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, save);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, delete);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, queryKeys);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, exists);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, increment);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, decrement);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, flush);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, getTrackingKey);
PHP_METHOD(Phalcon_Cache_Backend_Memcache, setTrackingKey);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_memcache___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_memcache_settrackingkey, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_memcache_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_Memcache, __construct, arginfo_phalcon_cache_backend_memcache___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend_Memcache, _connect, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Cache_Backend_Memcache, get, arginfo_phalcon_cache_backendinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcache, save, arginfo_phalcon_cache_backendinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcache, delete, arginfo_phalcon_cache_backendinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcache, queryKeys, arginfo_phalcon_cache_backendinterface_querykeys, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcache, exists, arginfo_phalcon_cache_backendinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcache, increment, arginfo_phalcon_cache_backendinterface_increment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcache, decrement, arginfo_phalcon_cache_backendinterface_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcache, flush, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcache, getTrackingKey, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcache, setTrackingKey, arginfo_phalcon_cache_backend_memcache_settrackingkey, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend\Memcache initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Memcache)
{
	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, Memcache, cache_backend_memcache, phalcon_cache_backend_ce, phalcon_cache_backend_memcache_method_entry, 0);

	zend_declare_property_null(phalcon_cache_backend_memcache_ce, SL("_memcache"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_cache_backend_memcache_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\Memcache constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcache, __construct){

	zval *frontend, *opts = NULL, options;

	phalcon_fetch_params(0, 1, 1, &frontend, &opts);

	if (opts) {
		ZVAL_COPY(&options, opts);
	}

	if (Z_TYPE(options) != IS_ARRAY) { 
		array_init_size(&options, 4);
	}

	if (!phalcon_array_isset_str(&options, SL("host"))) {
		phalcon_array_update_str_str(&options, SL("host"), SL("127.0.0.1"), PH_COPY);
	}

	if (!phalcon_array_isset_str(&options, SL("port"))) {
		phalcon_array_update_str_long(&options, SL("port"), 11211, 0);
	}

	if (!phalcon_array_isset_str(&options, SL("persistent"))) {
		phalcon_array_update_str_bool(&options, SL("persistent"), 0, 0);
	}

	if (!phalcon_array_isset_str(&options, SL("statsKey"))) {
		phalcon_array_update_str_str(&options, SL("statsKey"), SL("_PHCM"), PH_COPY);
	}

	PHALCON_CALL_PARENTW(NULL, phalcon_cache_backend_memcache_ce, getThis(), "__construct", frontend, &options);
}

/**
 * Create internal connection to memcached
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcache, _connect)
{
	zval options, memcache, host, port, persistent, success;
	zend_class_entry *ce0;

	phalcon_return_property(&options, getThis(), SL("_options"));
	ce0 = zend_fetch_class(SSL("Memcache"), ZEND_FETCH_CLASS_AUTO);

	object_init_ex(&memcache, ce0);
	if (phalcon_has_constructor(&memcache)) {
		PHALCON_CALL_METHODW(NULL, &memcache, "__construct");
	}

	if (
		   !phalcon_array_isset_fetch_str(&host, &options, SL("host"))
		|| !phalcon_array_isset_fetch_str(&port, &options, SL("port"))
		|| !phalcon_array_isset_fetch_str(&persistent, &options, SL("persistent"))
	) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	if (zend_is_true(&persistent)) {
		PHALCON_CALL_METHODW(&success, &memcache, "pconnect", &host, &port);
	} else {
		PHALCON_CALL_METHODW(&success, &memcache, "connect", &host, &port);
	}

	if (!zend_is_true(&success)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Cannot connect to Memcached server");
		return;
	}

	phalcon_update_property_this(getThis(), SL("_memcache"), &memcache);
	RETURN_CTORW(&memcache);
}

/**
 * Returns a cached content
 *
 * @param int|string $keyName
 * @param   long $lifetime
 * @return  mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcache, get){

	zval *key_name, *lifetime = NULL, memcache, frontend, prefix, prefixed_key, cached_content;

	phalcon_fetch_params(0, 1, 1, &key_name, &lifetime);

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(&memcache, getThis(), "_connect");
	}

	phalcon_return_property(&frontend, getThis(), SL("_frontend"));
	phalcon_return_property(&prefix, getThis(), SL("_prefix"));

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	phalcon_update_property_this(getThis(), SL("_lastKey"), &prefixed_key);

	PHALCON_CALL_METHODW(&cached_content, &memcache, "get", &prefixed_key);
	if (PHALCON_IS_FALSE(&cached_content)) {
		RETURN_NULL();
	}

	if (phalcon_is_numeric(&cached_content)) {
		RETURN_CTORW(&cached_content);
	}

	PHALCON_RETURN_CALL_METHOD(&frontend, "afterretrieve", &cached_content);
}

/**
 * Stores cached content into the Memcached backend and stops the frontend
 *
 * @param int|string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcache, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, last_key, prefix, cached_content, prepared_content, ttl, flags, success;
	zval keys, is_buffering, frontend, memcache, options, special_key;

	phalcon_fetch_params(0, 0, 4, &key_name, &content, &lifetime, &stop_buffer);

	if (!key_name || Z_TYPE_P(key_name) == IS_NULL) {
		phalcon_return_property(&last_key, getThis(), SL("_lastKey"));
	} else {
		phalcon_return_property(&prefix, getThis(), SL("_prefix"));
		PHALCON_CONCAT_VV(&last_key, &prefix, key_name);
	}

	if (!zend_is_true(&last_key)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The cache must be started first");
		return;
	}

	phalcon_return_property(&frontend, getThis(), SL("_frontend"));

	/** 
	 * Check if a connection is created or make a new one
	 */
	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(&memcache, getThis(), "_connect");
	}

	if (!content || Z_TYPE_P(content) == IS_NULL) {
		PHALCON_CALL_METHODW(&cached_content, &frontend, "getcontent");
	} else {
		ZVAL_COPY(&cached_content, content);
	}

	/** 
	 * Prepare the content in the frontend
	 */
	PHALCON_CALL_METHODW(&prepared_content, &frontend, "beforestore", &cached_content);

	if (!lifetime || Z_TYPE_P(lifetime) == IS_NULL) {
		phalcon_return_property(&ttl, getThis(), SL("_lastLifetime"));

		if (Z_TYPE(ttl) == IS_NULL) {
			PHALCON_CALL_METHODW(&ttl, &frontend, "getlifetime");
		}
	} else {
		ZVAL_COPY(&ttl, lifetime);
	}

	ZVAL_LONG(&flags, 0);

	/** 
	 * We store without flags
	 */
	if (phalcon_is_numeric(&cached_content)) {
		PHALCON_CALL_METHODW(&success, &memcache, "set", &last_key, &cached_content, &flags, &ttl);
	} else {
		PHALCON_CALL_METHODW(&success, &memcache, "set", &last_key, &prepared_content, &flags, &ttl);
	}

	if (!zend_is_true(&success)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Failed to store data in memcached");
		return;
	}

	phalcon_return_property(&options, getThis(), SL("_options"));

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	if (Z_TYPE(special_key) != IS_NULL) {
		/* Update the stats key */
		PHALCON_CALL_METHODW(&keys, &memcache, "get", &special_key);
		if (Z_TYPE(keys) != IS_ARRAY) {
			array_init(&keys);
		}

		if (!phalcon_array_isset(&keys, &last_key)) {
			phalcon_array_update_zval(&keys, &last_key, &ttl, PH_COPY);
			PHALCON_CALL_METHODW(NULL, &memcache, "set", &special_key, &keys);
		}
	}

	PHALCON_CALL_METHODW(&is_buffering, &frontend, "isbuffering");

	if (!stop_buffer || PHALCON_IS_TRUE(stop_buffer)) {
		PHALCON_CALL_METHODW(NULL, &frontend, "stop");
	}

	if (PHALCON_IS_TRUE(&is_buffering)) {
		zend_print_zval(&cached_content, 0);
	}

	phalcon_update_property_bool(getThis(), SL("_started"), 0);
}

/**
 * Deletes a value from the cache by its key
 *
 * @param int|string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcache, delete){

	zval *key_name, memcache, prefix, prefixed_key, options, special_key, keys;

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(&memcache, getThis(), "_connect");
	}

	phalcon_return_property(&prefix, getThis(), SL("_prefix"));

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	phalcon_return_property(&options, getThis(), SL("_options"));

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	if (Z_TYPE(special_key) != IS_NULL) {
		PHALCON_CALL_METHODW(&keys, &memcache, "get", &special_key);
		if (Z_TYPE(keys) == IS_ARRAY) {
			phalcon_array_unset(&keys, &prefixed_key, 0);
			PHALCON_CALL_METHODW(NULL, &memcache, "set", &special_key, &keys);
		}
	}

	/* Delete the key from memcached */
	PHALCON_RETURN_CALL_METHODW(&memcache, "delete", &prefixed_key);
}

/**
 * Query the existing cached keys
 *
 * @param string $prefix
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcache, queryKeys){

	zval *prefix = NULL, memcache, options, special_key, keys;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 1, &prefix);

	phalcon_return_property(&options, getThis(), SL("_options"));
	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
		zend_throw_exception_ex(phalcon_cache_exception_ce, 0, "Unexpected inconsistency in options");
		return;
	}

	array_init(return_value);
	if (Z_TYPE(special_key) == IS_NULL) {
		return;
	}

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(&memcache, getThis(), "_connect");
	}

	/* Get the key from memcached */
	PHALCON_CALL_METHODW(&keys, &memcache, "get", &special_key);
	if (Z_TYPE(keys) == IS_ARRAY) { 
		ZEND_HASH_FOREACH_KEY(Z_ARRVAL(keys), idx, str_key) {
			zval key;
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}
			phalcon_array_append(return_value, &key, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * Checks if cache exists and it hasn't expired
 *
 * @param  string $keyName
 * @param  long $lifetime
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcache, exists){

	zval *key_name = NULL, *lifetime = NULL, value, last_key, prefix, memcache;

	phalcon_fetch_params(0, 0, 2, &key_name, &lifetime);

	if (!key_name || Z_TYPE_P(key_name) == IS_NULL) {
		phalcon_return_property(&last_key, getThis(), SL("_lastKey"));
	} else {
		phalcon_return_property(&prefix, getThis(), SL("_prefix"));
		PHALCON_CONCAT_VV(&last_key, &prefix, key_name);
	}

	if (zend_is_true(&last_key)) {
		phalcon_return_property(&memcache, getThis(), SL("_memcache"));
		if (Z_TYPE(memcache) != IS_OBJECT) {
			PHALCON_CALL_METHODW(&memcache, getThis(), "_connect");
		}

		PHALCON_CALL_METHODW(&value, &memcache, "get", &last_key);
		RETVAL_BOOL(PHALCON_IS_NOT_FALSE(&value));
	} else {
		RETURN_FALSE;
	}
}

/**
 * Atomic increment of a given key, by number $value
 * 
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcache, increment){

	zval *key_name = NULL, *value = NULL, memcache;

	phalcon_fetch_params(0, 0, 2, &key_name, &value);

	if (!key_name) {
		key_name = &PHALCON_GLOBAL(z_null);
	}

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(&memcache, getThis(), "_connect");
	}

	PHALCON_RETURN_CALL_METHODW(&memcache, "increment", key_name, value);
}

/**
 * Atomic decrement of a given key, by number $value
 * 
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcache, decrement){

	zval *key_name = NULL, *value = NULL, memcache;

	phalcon_fetch_params(0, 0, 2, &key_name, &value);

	if (!key_name) {
		key_name = &PHALCON_GLOBAL(z_null);
	}

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(&memcache, getThis(), "_connect");
	}

	PHALCON_RETURN_CALL_METHODW(&memcache, "decrement", key_name, value);
}

/**
 * Immediately invalidates all existing items.
 * 
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcache, flush){

	zval memcache, options, special_key, keys, param, all_slabs, *slabs, cachedump;
	zend_string *str_key;
	ulong idx;

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(&memcache, getThis(), "_connect");
	}

	phalcon_return_property(&options, getThis(), SL("_options"));

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	/* Get the key from memcached */
	if (Z_TYPE(special_key) != IS_NULL) {
		PHALCON_CALL_METHODW(&keys, &memcache, "get", &special_key);
		if (Z_TYPE(keys) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY(Z_ARRVAL(keys), idx, str_key) {
				zval key;
				if (str_key) {
					ZVAL_STR(&key, str_key);
				} else {
					ZVAL_LONG(&key, idx);
				}
				PHALCON_CALL_METHODW(NULL, &memcache, "delete", &key);
			} ZEND_HASH_FOREACH_END();
			
			zend_hash_clean(Z_ARRVAL(keys));
			PHALCON_CALL_METHODW(NULL, &memcache, "set", &special_key, &keys);
		}
	} else {
		ZVAL_STRING(&param, "slabs");

		PHALCON_CALL_METHODW(&all_slabs, &memcache, "getextendedstats", &param);

		if (Z_TYPE(all_slabs) != IS_ARRAY) {
			RETURN_FALSE;
		}

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(all_slabs), slabs) {
			ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(slabs), idx, str_key) {
				zval slabid, *tmp_keys;
				if (str_key) {
					ZVAL_STR(&slabid, str_key);
				} else {
					ZVAL_LONG(&slabid, idx);
				}

				PHALCON_CALL_METHODW(&cachedump, &memcache, "cachedump", &slabid);

				if (Z_TYPE(cachedump) == IS_ARRAY) {
					ZEND_HASH_FOREACH_VAL(Z_ARRVAL(cachedump), tmp_keys) {
						zend_string *str_key2;
						ulong idx2;
						if (Z_TYPE_P(tmp_keys) == IS_ARRAY) {
							ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(tmp_keys), idx2, str_key2) {
								zval key;
								if (str_key) {
									ZVAL_STR(&key, str_key2);
								} else {
									ZVAL_LONG(&key, idx2);
								}
								PHALCON_CALL_METHODW(NULL, &memcache, "delete", &key);
							} ZEND_HASH_FOREACH_END();
						}
					} ZEND_HASH_FOREACH_END();
				}
			} ZEND_HASH_FOREACH_END();
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Cache_Backend_Memcache, getTrackingKey)
{
	zval stats_key, options;

	phalcon_return_property(&options, getThis(), SL("_options"));

	if (!phalcon_array_isset_fetch_str(&stats_key, &options, SL("statsKey"))) {
		RETURN_NULL();
	}

	RETURN_CTORW(&stats_key);
}

PHP_METHOD(Phalcon_Cache_Backend_Memcache, setTrackingKey)
{
	zval *key;

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_update_property_array_str(getThis(), SL("_options"), SL("statsKey"), key);

	RETURN_THISW();
}
