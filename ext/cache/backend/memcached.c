
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

#include "cache/backend/memcached.h"
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
 * Phalcon\Cache\Backend\Memcached
 *
 * Allows to cache output fragments, PHP data or raw data to a memcached backend
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
 * $cache = new Phalcon\Cache\Backend\Memcached($frontCache, array(
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
zend_class_entry *phalcon_cache_backend_memcached_ce;

PHP_METHOD(Phalcon_Cache_Backend_Memcached, __construct);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, _connect);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, get);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, save);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, delete);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, queryKeys);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, exists);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, increment);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, decrement);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, flush);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, getTrackingKey);
PHP_METHOD(Phalcon_Cache_Backend_Memcached, setTrackingKey);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_memcached___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_memcached_settrackingkey, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_memcached_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_Memcached, __construct, arginfo_phalcon_cache_backend_memcached___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend_Memcached, _connect, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Cache_Backend_Memcached, get, arginfo_phalcon_cache_backendinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcached, save, arginfo_phalcon_cache_backendinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcached, delete, arginfo_phalcon_cache_backendinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcached, queryKeys, arginfo_phalcon_cache_backendinterface_querykeys, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcached, exists, arginfo_phalcon_cache_backendinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcached, increment, arginfo_phalcon_cache_backendinterface_increment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcached, decrement, arginfo_phalcon_cache_backendinterface_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcached, flush, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcached, getTrackingKey, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Memcached, setTrackingKey, arginfo_phalcon_cache_backend_memcached_settrackingkey, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend\Memcached initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Memcached){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, Memcached, cache_backend_memcached, phalcon_cache_backend_ce, phalcon_cache_backend_memcached_method_entry, 0);

	zend_declare_property_null(phalcon_cache_backend_memcached_ce, SL("_memcache"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_cache_backend_memcached_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\Memcached constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcached, __construct){

	zval *frontend, *opts = NULL, options = {}, server = {}, servers = {}, special_key = {};

	phalcon_fetch_params(0, 1, 1, &frontend, &opts);

	if (!opts || Z_TYPE_P(opts) == IS_NULL) {
		array_init(&options);
	} else {
		PHALCON_CPY_WRT_CTOR(&options, opts);
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

	if (!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY) || PHALCON_IS_EMPTY_STRING(&special_key)) {
		phalcon_array_update_str_str(&options, SL("statsKey"), SL("_PHCM"), PH_COPY);
	}

	PHALCON_CALL_PARENT(NULL, phalcon_cache_backend_memcached_ce, getThis(), "__construct", frontend, &options);
}

/**
 * Create internal connection to memcached
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcached, _connect){

	zval options = {}, memcache = {}, servers = {}, client = {}, *value;
	zend_string *str_key;
	ulong idx;
	zend_class_entry *ce0;

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);
	ce0 = phalcon_fetch_str_class(SL("Memcached"), ZEND_FETCH_CLASS_AUTO);

	object_init_ex(&memcache, ce0);
	if (phalcon_has_constructor(&memcache)) {
		PHALCON_CALL_METHOD(NULL, &memcache, "__construct");
	}

	if (!phalcon_array_isset_fetch_str(&servers, &options, SL("servers"), PH_READONLY) || Z_TYPE(servers) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Servers must be an array");
		return;
	}

	PHALCON_RETURN_CALL_METHOD(&memcache, "addservers", &servers);

	if (!zend_is_true(return_value)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Cannot connect to Memcached server");
		return;
	}

	if (phalcon_array_isset_fetch_str(&client, &options, SL("client"), PH_READONLY) && Z_TYPE(client) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(client), idx, str_key, value) {
			zval option = {}, *res;

			if (str_key) {
				if ((res = zend_get_constant(str_key)) != NULL) {
					PHALCON_CALL_METHOD(NULL, &memcache, "setoption", res, value);
				}
			} else {
				ZVAL_LONG(&option, idx);
				PHALCON_CALL_METHOD(NULL, &memcache, "setoption", &option, value);
			}
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_update_property(getThis(), SL("_memcache"), &memcache);
}

/**
 * Returns a cached content
 *
 * @param string $keyName
 * @param  long $lifetime
 * @return  mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcached, get)
{
	zval *key_name, memcache = {}, frontend = {}, prefix = {}, prefixed_key = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_connect");
		phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	}

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);
	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	PHALCON_CALL_METHOD(&cached_content, &memcache, "get", &prefixed_key);
	if (PHALCON_IS_FALSE(&cached_content)) {
		RETURN_NULL();
	}

	if (phalcon_is_numeric(&cached_content)) {
		RETURN_CTOR(&cached_content);
	} else {
		PHALCON_RETURN_CALL_METHOD(&frontend, "afterretrieve", &cached_content);
	}
}

/**
 * Stores cached content into the Memcached backend and stops the frontend
 *
 * @param string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcached, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, key = {}, prefix = {}, prefixed_key = {}, frontend = {}, memcache = {}, cached_content = {};
	zval prepared_content = {}, ttl = {}, success = {}, options = {}, special_key = {}, keys = {}, is_buffering = {};

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

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);

	/**
	 * Check if a connection is created or make a new one
	 */
	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_connect");
		phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	}

	if (!content || Z_TYPE_P(content) == IS_NULL) {
		PHALCON_CALL_METHOD(&cached_content, &frontend, "getcontent");
	} else {
		ZVAL_COPY_VALUE(&cached_content, content);
	}

	/**
	 * Prepare the content in the frontend
	 */
	if (!phalcon_is_numeric(&cached_content)) {
		PHALCON_CALL_METHOD(&prepared_content, &frontend, "beforestore", &cached_content);
	}

	if (!lifetime || Z_TYPE_P(lifetime) != IS_LONG) {
		PHALCON_CALL_METHOD(&ttl, getThis(), "getlifetime");
	} else {
		ZVAL_COPY_VALUE(&ttl, lifetime);
	}

	if (Z_TYPE(prepared_content) > IS_NULL) {
		PHALCON_CALL_METHOD(&success, &memcache, "set", &prefixed_key, &prepared_content, &ttl);
	} else {
		PHALCON_CALL_METHOD(&success, &memcache, "set", &prefixed_key, &cached_content, &ttl);
	}

	if (!zend_is_true(&success)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Failed storing data in memcached");
		return;
	}

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	/* Update the stats key */
	PHALCON_CALL_METHOD(&keys, &memcache, "get", &special_key);
	if (Z_TYPE(keys) != IS_ARRAY) {
		array_init(&keys);
	}

	if (!phalcon_array_isset(&keys, &prefixed_key)) {
		phalcon_array_update(&keys, &prefixed_key, &ttl, PH_COPY);
		PHALCON_CALL_METHOD(NULL, &memcache, "set", &special_key, &keys);
	}

	PHALCON_CALL_METHOD(&is_buffering, &frontend, "isbuffering");

	if (!stop_buffer || PHALCON_IS_TRUE(stop_buffer)) {
		PHALCON_CALL_METHOD(NULL, &frontend, "stop");
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
 * @param string $keyName
 * @param long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcached, increment)
{
	zval *key_name, *value = NULL, memcache = {}, prefix = {}, prefixed_key = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	}

	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_connect");
		phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	}

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	PHALCON_CALL_METHOD(&cached_content, &memcache, "increment", &prefixed_key, value);
	if (PHALCON_IS_FALSE(&cached_content)) {
		RETURN_NULL();
	}

	RETURN_CTOR(&cached_content);
}

/**
 * Decrement of a given key, by number $value
 *
 * @param string $keyName
 * @param long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcached, decrement)
{
	zval *key_name, *value = NULL, memcache = {}, prefix = {}, prefixed_key = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	}

	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_connect");
		phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	}

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	PHALCON_CALL_METHOD(&cached_content, &memcache, "decrement", &prefixed_key, value);
	if (PHALCON_IS_FALSE(&cached_content)) {
		RETURN_NULL();
	}

	RETURN_CTOR(&cached_content);
}

/**
 * Deletes a value from the cache by its key
 *
 * @param string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcached, delete){

	zval *key_name, memcache = {}, prefix = {}, prefixed_key = {}, options = {}, special_key = {}, keys = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_connect");
		phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	}

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	PHALCON_CALL_METHOD(&keys, &memcache, "get", &special_key);
	if (Z_TYPE(keys) == IS_ARRAY) {
		phalcon_array_unset(&keys, &prefixed_key, 0);
		PHALCON_CALL_METHOD(NULL, &memcache, "set", &special_key, &keys);
	}

	/* Delete the key from memcached */
	PHALCON_RETURN_CALL_METHOD(&memcache, "delete", &prefixed_key);
}

/**
 * Query the existing cached keys
 *
 * @param string $prefix
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcached, queryKeys){

	zval *prefix = NULL, memcache = {}, options = {}, special_key = {}, keys = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 1, &prefix);

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);
	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY))) {
		zend_throw_exception_ex(phalcon_cache_exception_ce, 0, "Unexpected inconsistency in options");
		return;
	}

	array_init(return_value);

	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_connect");
		phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	}

	/* Get the key from memcached */
	PHALCON_CALL_METHOD(&keys, &memcache, "get", &special_key);
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
 * @param string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcached, exists){

	zval *key_name, prefix = {}, prefixed_key = {}, value = {}, memcache = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);
	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_connect");
		phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	}

	PHALCON_CALL_METHOD(&value, &memcache, "get", &prefixed_key);
	RETVAL_BOOL(PHALCON_IS_NOT_FALSE(&value));
}

/**
 * Immediately invalidates all existing items.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Memcached, flush){

	zval memcache = {}, options = {}, special_key = {}, keys = {};
	zend_string *str_key;
	ulong idx;

	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	if (Z_TYPE(memcache) != IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_connect");
		phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_READONLY);
	}

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (unlikely(!phalcon_array_isset_fetch_str(&special_key, &options, SL("statsKey"), PH_READONLY))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	/* Get the key from memcached */
	if (Z_TYPE(special_key) != IS_NULL) {
		PHALCON_CALL_METHOD(&keys, &memcache, "get", &special_key);
		if (Z_TYPE(keys) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY(Z_ARRVAL(keys), idx, str_key) {
				zval key = {};
				if (str_key) {
					ZVAL_STR(&key, str_key);
				} else {
					ZVAL_LONG(&key, idx);
				}

				PHALCON_CALL_METHOD(NULL, &memcache, "delete", &key);
			} ZEND_HASH_FOREACH_END();

			zend_hash_clean(Z_ARRVAL(keys));
			PHALCON_CALL_METHOD(NULL, &memcache, "set", &special_key, &keys);
		}
	} else {
		PHALCON_CALL_METHOD(&keys, &memcache, "flush");
	}

	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Cache_Backend_Memcached, getTrackingKey)
{
	zval options = {};

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	if (!phalcon_array_isset_fetch_str(return_value, &options, SL("statsKey"), PH_COPY)) {
		RETURN_NULL();
	}
}

PHP_METHOD(Phalcon_Cache_Backend_Memcached, setTrackingKey)
{
	zval *key;

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_update_property_array_str(getThis(), SL("_options"), SL("statsKey"), key);

	RETURN_THIS();
}
