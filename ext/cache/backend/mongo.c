
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

#include "cache/backend/mongo.h"
#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/exception.h"

#include <ext/standard/php_rand.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/concat.h"
#include "kernel/operators.h"

/**
 * Phalcon\Cache\Backend\Mongo
 *
 * Allows to cache output fragments, PHP data or raw data to a MongoDb backend
 *
 *<code>
 *
 * // Cache data for 2 days
 * $frontCache = new Phalcon\Cache\Frontend\Base64(array(
 *		"lifetime" => 172800
 * ));
 *
 * //Create a MongoDB cache
 * $cache = new Phalcon\Cache\Backend\Mongo($frontCache, array(
 *		'server' => "mongodb://localhost",
 *      'db' => 'caches',
 *		'collection' => 'images'
 * ));
 *
 * //Cache arbitrary data
 * $cache->save('my-data', file_get_contents('some-image.jpg'));
 *
 * //Get data
 * $data = $cache->get('my-data');
 *
 *</code>
 */
zend_class_entry *phalcon_cache_backend_mongo_ce;

PHP_METHOD(Phalcon_Cache_Backend_Mongo, __construct);
PHP_METHOD(Phalcon_Cache_Backend_Mongo, _getCollection);
PHP_METHOD(Phalcon_Cache_Backend_Mongo, get);
PHP_METHOD(Phalcon_Cache_Backend_Mongo, save);
PHP_METHOD(Phalcon_Cache_Backend_Mongo, delete);
PHP_METHOD(Phalcon_Cache_Backend_Mongo, queryKeys);
PHP_METHOD(Phalcon_Cache_Backend_Mongo, exists);
PHP_METHOD(Phalcon_Cache_Backend_Mongo, gc);
PHP_METHOD(Phalcon_Cache_Backend_Mongo, increment);
PHP_METHOD(Phalcon_Cache_Backend_Mongo, decrement);
PHP_METHOD(Phalcon_Cache_Backend_Mongo, flush);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_mongo___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_mongo_empty, 0, 0, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_mongo_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_Mongo, __construct, arginfo_phalcon_cache_backend_mongo___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend_Mongo, _getCollection, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Cache_Backend_Mongo, get, arginfo_phalcon_cache_backendinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Mongo, save, arginfo_phalcon_cache_backendinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Mongo, delete, arginfo_phalcon_cache_backendinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Mongo, queryKeys, arginfo_phalcon_cache_backendinterface_querykeys, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Mongo, exists, arginfo_phalcon_cache_backendinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Mongo, gc, arginfo_phalcon_cache_backend_mongo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Mongo, increment, arginfo_phalcon_cache_backendinterface_increment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Mongo, decrement, arginfo_phalcon_cache_backendinterface_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_Mongo, flush, arginfo_phalcon_cache_backend_mongo_empty, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend\Mongo initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Mongo){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, Mongo, cache_backend_mongo, phalcon_cache_backend_ce, phalcon_cache_backend_mongo_method_entry, 0);

	zend_declare_property_null(phalcon_cache_backend_mongo_ce, SL("_collection"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_cache_backend_mongo_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\Mongo constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, __construct){

	zval *frontend, *options = NULL;

	phalcon_fetch_params(0, 1, 1, &frontend, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	if (!phalcon_array_isset_str(options, SL("collection"))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The parameter 'collection' is required");
		return;
	}

	PHALCON_CALL_PARENTW(NULL, phalcon_cache_backend_mongo_ce, getThis(), "__construct", frontend, options);
}

/**
 * Returns a MongoDb collection based on the backend parameters
 *
 * @return MongoCollection
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, _getCollection){

	zval mongo_collection = {}, options = {}, mongo = {}, class_name = {}, server = {}, database = {}, collection = {}, service_name = {};
	zend_class_entry *ce0;

	phalcon_return_property(&mongo_collection, getThis(), SL("_collection"));
	if (Z_TYPE(mongo_collection) != IS_OBJECT) {
		phalcon_return_property(&options, getThis(), SL("_options"));

		if (phalcon_array_isset_fetch_str(&mongo, &options, SL("mongo"))) {
			if (Z_TYPE(mongo) != IS_OBJECT) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The 'mongo' parameter must be a valid Mongo instance");
				return;
			}

			if (!phalcon_array_isset_fetch_str(&database, &options, SL("db")) || Z_TYPE(database) != IS_STRING) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The backend requires a valid MongoDB db");
				return;
			}

			if (!phalcon_array_isset_fetch_str(&collection, &options, SL("collection")) || Z_TYPE(collection) != IS_STRING) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The backend requires a valid MongoDB collection");
				return;
			}

			PHALCON_RETURN_CALL_METHODW(&mongo, "selectcollection", &database, &collection);
		} else if (phalcon_array_isset_fetch_str(&server, &options, SL("server"))) {
			if (Z_TYPE(server) != IS_STRING) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The backend requires a valid MongoDB connection string");
				return;
			}

			ZVAL_STRING(&class_name, "MongoClient");
			if (phalcon_class_exists(&class_name, 0) != NULL) {
				ce0 = phalcon_fetch_str_class(SL("MongoClient"), ZEND_FETCH_CLASS_AUTO);
			} else {
				ce0 = phalcon_fetch_str_class(SL("Mongo"), ZEND_FETCH_CLASS_AUTO);
			}

			object_init_ex(&mongo, ce0);
			assert(phalcon_has_constructor(&mongo));
			PHALCON_CALL_METHODW(NULL, &mongo, "__construct", &server);

			if (!phalcon_array_isset_fetch_str(&database, &options, SL("db")) || Z_TYPE(database) != IS_STRING) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The backend requires a valid MongoDB db");
				return;
			}

			if (!phalcon_array_isset_fetch_str(&collection, &options, SL("collection")) || Z_TYPE(collection) != IS_STRING) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The backend requires a valid MongoDB collection");
				return;
			}

			PHALCON_RETURN_CALL_METHODW(&mongo, "selectcollection", &database, &collection);
		} else {
			ZVAL_STRING(&service_name, "mongo");
			PHALCON_CALL_METHODW(&mongo, getThis(), "getresolveservice", &service_name);

			if (!phalcon_array_isset_fetch_str(&collection, &options, SL("collection")) || Z_TYPE(collection) != IS_STRING) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The backend requires a valid MongoDB collection");
				return;
			}

			PHALCON_RETURN_CALL_METHODW(&mongo, "selectcollection", &collection);
		}
	} else {
		RETURN_CTORW(&mongo_collection);
	}
}

/**
 * Returns a cached content
 *
 * @param int|string $keyName
 * @param   long $lifetime
 * @return  mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, get){

	zval *key_name, *lifetime = NULL, frontend = {}, prefix = {}, prefixed_key = {}, collection = {}, conditions = {}, document = {}, time_condition = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &lifetime);

	phalcon_return_property(&frontend, getThis(), SL("_frontend"));
	phalcon_return_property(&prefix, getThis(), SL("_prefix"));

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	phalcon_update_property_this(getThis(), SL("_lastKey"), &prefixed_key);

	PHALCON_CALL_METHODW(&collection, getThis(), "_getcollection");

	array_init_size(&conditions, 2);
	phalcon_array_update_str(&conditions, SL("key"), &prefixed_key, PH_COPY);

	array_init_size(&time_condition, 1);
	add_assoc_long_ex(&time_condition, SL("$gt"), (long int)time(NULL));
	add_assoc_zval_ex(&conditions, SL("time"), &time_condition);

	PHALCON_CALL_METHODW(&document, &collection, "findone", &conditions);

	if (Z_TYPE(document) == IS_ARRAY) { 
		if (likely(phalcon_array_isset_fetch_str(&cached_content, &document, SL("data")))) {
			if (phalcon_is_numeric(&cached_content)) {
				RETURN_CTORW(&cached_content);
			} else {
				PHALCON_RETURN_CALL_METHODW(&frontend, "afterretrieve", &cached_content);
				return;
			}
		} else {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The cache is corrupt");
			return;
		}
	}

	RETURN_NULL();
}

/**
 * Stores cached content into the Mongo backend and stops the frontend
 *
 * @param int|string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, last_key = {}, prefix = {}, frontend = {}, cached_content = {};
	zval prepared_content = {}, ttl = {}, collection = {}, timestamp = {}, conditions = {}, document = {}, data = {}, is_buffering = {};


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
	if (!content || Z_TYPE_P(content) == IS_NULL) {
		PHALCON_CALL_METHODW(&cached_content, &frontend, "getcontent");
	} else {
		PHALCON_CPY_WRT(&cached_content, content);
	}

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

	PHALCON_CALL_METHODW(&collection, getThis(), "_getcollection");

	ZVAL_LONG(&timestamp, (long) time(NULL) + phalcon_get_intval(&ttl));

	array_init_size(&conditions, 1);
	phalcon_array_update_str(&conditions, SL("key"), &last_key, PH_COPY);

	PHALCON_CALL_METHODW(&document, &collection, "findone", &conditions);

	if (Z_TYPE(document) == IS_ARRAY) { 
		phalcon_array_update_str(&document, SL("time"), &timestamp, PH_COPY);
		if (Z_TYPE(prepared_content) > IS_NULL) {
			phalcon_array_update_str(&document, SL("data"), &prepared_content, PH_COPY);
		} else {
			phalcon_array_update_str(&document, SL("data"), &cached_content, PH_COPY);
		}
		PHALCON_CALL_METHODW(NULL, &collection, "save", &document);
	} else {
		array_init_size(&data, 3);
		phalcon_array_update_str(&data, SL("key"), &last_key, PH_COPY);
		phalcon_array_update_str(&data, SL("time"), &timestamp, PH_COPY);

		if (Z_TYPE(prepared_content) > IS_NULL) {
			phalcon_array_update_str(&data, SL("data"), &prepared_content, PH_COPY);
		} else {
			phalcon_array_update_str(&data, SL("data"), &cached_content, PH_COPY);
		}

		PHALCON_CALL_METHODW(NULL, &collection, "save", &data);
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
PHP_METHOD(Phalcon_Cache_Backend_Mongo, delete){

	zval *key_name, prefix = {}, prefixed_key = {}, collection = {}, conditions = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_return_property(&prefix, getThis(), SL("_prefix"));

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	PHALCON_CALL_METHODW(&collection, getThis(), "_getcollection");

	array_init_size(&conditions, 1);
	phalcon_array_update_str(&conditions, SL("key"), &prefixed_key, PH_COPY);
	PHALCON_CALL_METHODW(NULL, &collection, "remove", &conditions);

	if ((php_rand() % 100) == 0) {
		PHALCON_CALL_METHODW(NULL, getThis(), "gc");
	}

	RETURN_TRUE;
}

/**
 * Query the existing cached keys
 *
 * @param string $prefix
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, queryKeys){

	zval *prefix = NULL, collection = {}, fields = {}, pattern = {}, regex = {}, conditions = {}, documents = {}, *document, documents_array = {}, time_condition = {};
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 0, 1, &prefix);

	PHALCON_CALL_METHODW(&collection, getThis(), "_getcollection");

	array_init_size(&fields, 1);
	add_next_index_stringl(&fields, SL("key"));

	array_init_size(&conditions, 2);

	if (prefix && zend_is_true(prefix)) {
		PHALCON_CONCAT_SVS(&pattern, "/^", prefix, "/");
		ce0 = phalcon_fetch_str_class(SL("MongoRegex"), ZEND_FETCH_CLASS_AUTO);

		object_init_ex(&regex, ce0);
		assert(phalcon_has_constructor(&regex));
		PHALCON_CALL_METHODW(NULL, &regex, "__construct", &pattern);

		phalcon_array_update_str(&conditions, SL("key"), &regex, PH_COPY);
	}

	array_init_size(&time_condition, 1);
	add_assoc_long_ex(&time_condition, SL("$gt"), (long int)time(NULL));
	phalcon_array_update_str(&conditions, SL("time"), &time_condition, 0);

	PHALCON_CALL_METHODW(&documents, &collection, "find", &conditions, &fields);

	array_init(return_value);

	PHALCON_CALL_FUNCTIONW(&documents_array, "iterator_to_array", &documents);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(documents_array), document) {
		zval key = {};
		if (likely(phalcon_array_isset_fetch_str(&key, document, SL("key")))) {
			phalcon_array_append(return_value, &key, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();
}

/**
 * Checks if cache exists and it hasn't expired
 *
 * @param  string $keyName
 * @param  long $lifetime
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, exists){

	zval *key_name = NULL, *lifetime = NULL, collection = {}, last_key = {}, prefix = {}, conditions = {}, number = {}, time_condition = {};
	long int n;

	phalcon_fetch_params(0, 0, 2, &key_name, &lifetime);

	if (!key_name || Z_TYPE_P(key_name) == IS_NULL) {
		phalcon_return_property(&last_key, getThis(), SL("_lastKey"));
	} else {
		phalcon_return_property(&prefix, getThis(), SL("_prefix"));
		PHALCON_CONCAT_VV(&last_key, &prefix, key_name);
	}

	if (zend_is_true(&last_key)) {
		PHALCON_CALL_METHODW(&collection, getThis(), "_getcollection");

		array_init_size(&conditions, 2);
		phalcon_array_update_str(&conditions, SL("key"), &last_key, PH_COPY);

		array_init_size(&time_condition, 1);
		add_assoc_long_ex(&time_condition, SL("$gt"), (long int)time(NULL));
		phalcon_array_update_str(&conditions, SL("time"), &time_condition, 0);

		PHALCON_CALL_METHODW(&number, &collection, "count", &conditions);

		n = phalcon_get_intval(&number);

		RETVAL_BOOL(n > 0);
	} else {
		RETURN_FALSE;
	}
}

PHP_METHOD(Phalcon_Cache_Backend_Mongo, gc) {

	zval conditions = {}, time_condition = {}, collection = {};

	array_init_size(&time_condition, 1);
	add_assoc_long_ex(&time_condition, SL("$gt"), (long int)time(NULL));

	array_init_size(&conditions, 1);
	add_assoc_zval_ex(&conditions, SL("time"), &time_condition);

	PHALCON_CALL_METHODW(&collection, getThis(), "_getcollection");
	PHALCON_CALL_METHODW(NULL, &collection, "remove", &conditions);
}

/**
 * Increment of a given key by $value
 *
 * @param int|string $keyName
 * @param   long $value
 * @return  mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, increment){

	zval *key_name, *value = NULL, lifetime = {}, frontend = {}, prefix = {}, prefixed_key = {}, collection = {}, conditions = {}, document = {}, timestamp = {};
	zval modified_time = {}, difference = {}, not_expired = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {	
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	phalcon_return_property(&frontend, getThis(), SL("_frontend"));
	phalcon_return_property(&prefix, getThis(), SL("_prefix"));

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	phalcon_update_property_this(getThis(), SL("_lastKey"), &prefixed_key);

	PHALCON_CALL_METHODW(&collection, getThis(), "_getcollection");

	array_init_size(&conditions, 1);
	phalcon_array_update_str(&conditions, SL("key"), &prefixed_key, PH_COPY);

	PHALCON_CALL_METHODW(&document, &collection, "findone", &conditions);

	ZVAL_LONG(&timestamp, (long) time(NULL));

	phalcon_return_property(&lifetime, getThis(), SL("_lastLifetime"));
	if (Z_TYPE(lifetime) == IS_NULL) {
		PHALCON_CALL_METHODW(&lifetime, &frontend, "getlifetime");
	}

	if (!phalcon_array_isset_str(&document, SL("time"))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The cache is currupted");
		return;
	}

	phalcon_array_fetch_str(&modified_time, &document, SL("time"), PH_NOISY);
	phalcon_sub_function(&difference, &timestamp, &lifetime);
	is_smaller_function(&not_expired, &difference, &modified_time);

	/** 
	 * The expiration is based on the column 'time'
	 */
	if (PHALCON_IS_TRUE(&not_expired)) {
		if (!phalcon_array_isset_str(&document, SL("data"))) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The cache is currupted");
			return;
		}

		phalcon_array_fetch_str(&cached_content, &document, SL("data"), PH_NOISY);

		if(Z_TYPE(cached_content) != IS_LONG) {
			convert_to_long_ex(&cached_content);
		}

		phalcon_add_function(return_value, &cached_content, value);
		PHALCON_CALL_METHODW(NULL, getThis(), "save", &prefixed_key, return_value);
		return;
	}

	RETURN_NULL();
}

/**
 * Decrement of a given key by $value
 *
 * @param int|string $keyName
 * @param   long $value
 * @return  mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, decrement){

	zval *key_name, *value = NULL, lifetime = {}, frontend, prefix = {}, prefixed_key = {}, collection = {}, conditions = {}, document = {}, timestamp = {};
	zval modified_time = {}, difference = {}, not_expired = {}, cached_content = {};

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value || Z_TYPE_P(value) == IS_NULL) {
		value = &PHALCON_GLOBAL(z_one);
	} else if (Z_TYPE_P(value) != IS_LONG) {	
		PHALCON_SEPARATE_PARAM(value);
		convert_to_long_ex(value);
	}

	phalcon_return_property(&frontend, getThis(), SL("_frontend"));
	phalcon_return_property(&prefix, getThis(), SL("_prefix"));

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	phalcon_update_property_this(getThis(), SL("_lastKey"), &prefixed_key);

	PHALCON_CALL_METHODW(&collection, getThis(), "_getcollection");

	array_init_size(&conditions, 1);
	phalcon_array_update_str(&conditions, SL("key"), &prefixed_key, PH_COPY);

	PHALCON_CALL_METHODW(&document, &collection, "findone", &conditions);

	ZVAL_LONG(&timestamp, (long) time(NULL));

	phalcon_return_property(&lifetime, getThis(), SL("_lastLifetime"));
	if (Z_TYPE(lifetime) == IS_NULL) {
		PHALCON_CALL_METHODW(&lifetime, &frontend, "getlifetime");
	}

	if (!phalcon_array_isset_str(&document, SL("time"))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The cache is currupted");
		return;
	}

	phalcon_array_fetch_str(&modified_time, &document, SL("time"), PH_NOISY);
	phalcon_sub_function(&difference, &timestamp, &lifetime);
	is_smaller_function(&not_expired, &difference, &modified_time);

	/** 
	 * The expiration is based on the column 'time'
	 */
	if (PHALCON_IS_TRUE(&not_expired)) {
		if (!phalcon_array_isset_str(&document, SL("data"))) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_cache_exception_ce, "The cache is currupted");
			return;
		}

		phalcon_array_fetch_str(&cached_content, &document, SL("data"), PH_NOISY);

		if(Z_TYPE(cached_content) != IS_LONG) {
			convert_to_long_ex(&cached_content);
		}

		phalcon_sub_function(return_value, &cached_content, value);
		PHALCON_CALL_METHODW(NULL, getThis(), "save", &prefixed_key, return_value);
		return;
	}

	RETURN_NULL();
}

/**
 * Immediately invalidates all existing items.
 * 
 * @return bool
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, flush){

	zval collection = {};

	PHALCON_CALL_METHODW(&collection, getThis(), "_getcollection");
	PHALCON_CALL_METHODW(NULL, &collection, "remove");

	if ((php_rand() % 100) == 0) {
		PHALCON_CALL_METHODW(NULL, getThis(), "gc");
	}

	RETURN_TRUE;
}
