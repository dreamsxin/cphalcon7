
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

#include "cache/backend/libmemcached.h"
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
 * Phalcon\Cache\Backend\Libmemcached
 *
 * Allows to cache output fragments, PHP data or raw data to a libmemcached backend
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
 * $cache = new Phalcon\Cache\Backend\Libmemcached($frontCache, array(
 *     'servers' => array(
 *         array('host' => 'localhost',
 *               'port' => 11211,
 *               'weight' => 1),
 *     ),
 *     'client' => array(
 *         Memcached::OPT_HASH => Memcached::HASH_MD5,
 *         Memcached::OPT_PREFIX_KEY => 'prefix.',
 *     )
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
zend_class_entry *phalcon_cache_backend_libmemcached_ce;

PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, __construct);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, _connect);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, get);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, save);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, delete);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, queryKeys);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, exists);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, increment);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, decrement);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, flush);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, getTrackingKey);
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, setTrackingKey);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_libmemcached___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_libmemcached_settrackingkey, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_libmemcached_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, __construct, arginfo_phalcon_cache_backend_libmemcached___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, _connect, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, get, arginfo_phalcon_cache_backendinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, save, arginfo_phalcon_cache_backendinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, delete, arginfo_phalcon_cache_backendinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, queryKeys, arginfo_phalcon_cache_backendinterface_querykeys, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, exists, arginfo_phalcon_cache_backendinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, increment, arginfo_phalcon_cache_backendinterface_increment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, decrement, arginfo_phalcon_cache_backendinterface_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, flush, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, getTrackingKey, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Libmemcached, setTrackingKey, arginfo_phalcon_cache_backend_libmemcached_settrackingkey, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend\Libmemcached initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Libmemcached){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, Libmemcached, cache_backend_libmemcached, phalcon_cache_backend_ce, phalcon_cache_backend_libmemcached_method_entry, 0);

	zend_declare_property_null(phalcon_cache_backend_libmemcached_ce, SL("_memcache"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_cache_backend_libmemcached_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\Libmemcached constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, __construct){

	zval *frontend, *opts = NULL, options = {}, server = {}, servers = {};

	phalcon_fetch_params(0, 1, 1, &frontend, &opts);

	if (opts) {
		PHALCON_CPY_WRT_CTOR(&options, opts);
	}

	if (Z_TYPE(options) != IS_ARRAY) {
		array_init(&options);
	}

	if (!phalcon_array_isset_str(&options, SL("servers"))) {
		array_init_size(&servers, 1);
		array_init_size(&server, 3);

		phalcon_array_update_str_str(&server, SL("host"), SL("127.0.0.1"), PH_COPY);
		phalcon_array_update_str_long(&server, SL("port"), 11211, PH_COPY);
		phalcon_array_update_str_long(&server, SL("weight"), 1, PH_COPY);

		phalcon_array_append(&servers, &server, PH_COPY);

		phalcon_array_update_str(&options, SL("servers"), &servers, PH_COPY);
	}

	if (!phalcon_array_isset_str(&options, SL("statsKey"))) {
		phalcon_array_update_str_str(&options, SL("statsKey"), SL("_PHCM"), PH_COPY);
	}

	PHALCON_CALL_PARENTW(NULL, phalcon_cache_backend_libmemcached_ce, getThis(), "__construct", frontend, &options);
}

/**
 * Create internal connection to memcached
 */
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, _connect){

	zval options = {}, memcache = {}, servers = {}, client = {}, *value;
	zend_string *str_key;
	ulong idx;
	zend_class_entry *ce0;

	phalcon_return_property(&options, getThis(), SL("_options"));
	ce0 = phalcon_fetch_str_class(SL("Memcached"), ZEND_FETCH_CLASS_AUTO);

	object_init_ex(&memcache, ce0);
	if (phalcon_has_constructor(&memcache)) {
		PHALCON_CALL_METHODW(NULL, &memcache, "__construct");
	}

	if (!phalcon_array_isset_fetch_str(&servers, &options, SL("servers")) || Z_TYPE(servers) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Servers must be an array");
		return;
	}

	PHALCON_RETURN_CALL_METHODW(&memcache, "addservers", &servers);

	if (!zend_is_true(return_value)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Cannot connect to Memcached server");
		return;
	}

	if (phalcon_array_isset_fetch_str(&client, &options, SL("client")) && Z_TYPE(client) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(client), idx, str_key, value) {
			zval option = {}, *res;

			if (str_key) {
				if ((res = zend_get_constant(str_key)) != NULL) {
					PHALCON_CALL_METHODW(NULL, &memcache, "setoption", res, value);
				}
			} else {
				ZVAL_LONG(&option, idx);
				PHALCON_CALL_METHODW(NULL, &memcache, "setoption", &option, value);
			}
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_update_property_zval(getThis(), SL("_memcache"), &memcache);
}

/**
 * Returns a cached content
 *
 * @param int|string $keyName
 * @param   long $lifetime
 * @return  mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, get)
{
	zval *key_name, *lifetime = NULL, memcache = {}, frontend = {}, prefix = {}, prefixed_key = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &lifetime);

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, getThis(), "_connect");
		phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	}

	phalcon_return_property(&frontend, getThis(), SL("_frontend"));
	phalcon_return_property(&prefix, getThis(), SL("_prefix"));

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	phalcon_update_property_zval(getThis(), SL("_lastKey"), &prefixed_key);

	PHALCON_CALL_METHODW(&cached_content, &memcache, "get", &prefixed_key);
	if (PHALCON_IS_FALSE(&cached_content)) {
		RETURN_NULL();
	}

	if (phalcon_is_numeric(&cached_content)) {
		RETURN_CTORW(&cached_content);
	} else {
		PHALCON_RETURN_CALL_METHODW(&frontend, "afterretrieve", &cached_content);
	}
}

/**
 * Stores cached content into the Memcached backend and stops the frontend
 *
 * @param int|string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, prefix = {}, last_key = {}, frontend = {}, memcache = {}, cached_content = {};
	zval prepared_content = {}, ttl = {}, success = {}, options = {}, special_key = {}, keys = {}, is_buffering = {};

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
		PHALCON_CALL_METHODW(NULL, getThis(), "_connect");
		phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	}

	if (!content || Z_TYPE_P(content) == IS_NULL) {
		PHALCON_CALL_METHODW(&cached_content, &frontend, "getcontent");
	} else {
		PHALCON_CPY_WRT(&cached_content, content);
	}

	/**
	 * Prepare the content in the frontend
	 */
	if (!phalcon_is_numeric(&cached_content)) {
		PHALCON_CALL_METHODW(&prepared_content, &frontend, "beforestore", &cached_content);
	}

	if (!lifetime || Z_TYPE_P(lifetime) == IS_NULL) {
		phalcon_return_property(&ttl, getThis(), SL("_lastLifetime"));

		if (Z_TYPE(ttl) == IS_NULL) {
			PHALCON_CALL_METHODW(&ttl, &frontend, "getlifetime");
		}
	} else {
		PHALCON_CPY_WRT(&ttl, lifetime);
	}

	if (Z_TYPE(prepared_content) > IS_NULL) {
		PHALCON_CALL_METHODW(&success, &memcache, "set", &last_key, &prepared_content, &ttl);
	} else {
		PHALCON_CALL_METHODW(&success, &memcache, "set", &last_key, &cached_content, &ttl);
	}

	if (!zend_is_true(&success)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Failed storing data in memcached");
		return;
	}

	phalcon_return_property(&options, getThis(), SL("_options"));

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	} else if (Z_TYPE(special_key) != IS_NULL) {
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
	RETURN_TRUE;
}

/**
 * Increment of a given key, by number $value
 *
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, increment)
{
	zval *key_name, *value = NULL, memcache = {}, prefix = {}, prefixed_key = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, getThis(), "_connect");
		phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	}

	phalcon_return_property(&prefix, getThis(), SL("_prefix"));

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	phalcon_update_property_zval(getThis(), SL("_lastKey"), &prefixed_key);

	PHALCON_CALL_METHODW(&cached_content, &memcache, "increment", &prefixed_key, value);
	if (PHALCON_IS_FALSE(&cached_content)) {
		RETURN_NULL();
	}

	RETURN_CTORW(&cached_content);
}

/**
 * Decrement of a given key, by number $value
 *
 * @param  string $keyName
 * @param  long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, decrement)
{
	zval *key_name, *value = NULL, memcache = {}, prefix = {}, prefixed_key = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, getThis(), "_connect");
		phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	}

	phalcon_return_property(&prefix, getThis(), SL("_prefix"));

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	phalcon_update_property_zval(getThis(), SL("_lastKey"), &prefixed_key);

	PHALCON_CALL_METHODW(&cached_content, &memcache, "decrement", &prefixed_key, value);
	if (PHALCON_IS_FALSE(&cached_content)) {
		RETURN_NULL();
	}

	RETURN_CTORW(&cached_content);
}

/**
 * Deletes a value from the cache by its key
 *
 * @param int|string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, delete){

	zval *key_name, memcache = {}, prefix = {}, prefixed_key = {}, options = {}, special_key = {}, keys = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, getThis(), "_connect");
		phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	}

	phalcon_return_property(&prefix, getThis(), SL("_prefix"));

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	phalcon_return_property(&options, getThis(), SL("_options"));

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	} else if (Z_TYPE(special_key) != IS_NULL) {
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
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, queryKeys){

	zval *prefix = NULL, memcache = {}, options = {}, special_key = {}, keys = {};
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
		PHALCON_CALL_METHODW(NULL, getThis(), "_connect");
		phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	}

	/* Get the key from memcached */
	PHALCON_CALL_METHODW(&keys, &memcache, "get", &special_key);
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
}

/**
 * Checks if cache exists and it hasn't expired
 *
 * @param  string $keyName
 * @param  long $lifetime
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, exists){

	zval *key_name = NULL, lifetime = {}, value = {}, last_key = {}, prefix = {}, memcache = {};

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
			PHALCON_CALL_METHODW(NULL, getThis(), "_connect");
			phalcon_return_property(&memcache, getThis(), SL("_memcache"));
		}

		PHALCON_CALL_METHODW(&value, &memcache, "get", &last_key);
		RETVAL_BOOL(PHALCON_IS_NOT_FALSE(&value));
	} else {
		RETURN_FALSE;
	}
}

/**
 * Immediately invalidates all existing items.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, flush){

	zval memcache = {}, options = {}, special_key = {}, keys = {};
	zend_string *str_key;
	ulong idx;

	phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, getThis(), "_connect");
		phalcon_return_property(&memcache, getThis(), SL("_memcache"));
	}

	phalcon_return_property(&options, getThis(), SL("_options"));

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey")))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	/* Get the key from memcached */
	if (Z_TYPE_P(&special_key) != IS_NULL) {
		PHALCON_CALL_METHODW(&keys, &memcache, "get", &special_key);
		if (Z_TYPE(keys) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY(Z_ARRVAL(keys), idx, str_key) {
				zval key = {};
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
		PHALCON_CALL_METHODW(&keys, &memcache, "flush");
	}

	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, getTrackingKey)
{
	zval options = {}, stats_key = {};

	phalcon_return_property(&options, getThis(), SL("_options"));

	if (!phalcon_array_isset_fetch_str(&stats_key, &options, SL("statsKey"))) {
		RETURN_NULL();
	}

	RETURN_CTORW(&stats_key);
}

PHP_METHOD(Phalcon_Cache_Backend_Libmemcached, setTrackingKey)
{
	zval *key;

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_update_property_array_str(getThis(), SL("_options"), SL("statsKey"), key);

	RETURN_THISW();
}
