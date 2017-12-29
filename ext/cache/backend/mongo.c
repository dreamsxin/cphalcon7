
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

#include "cache/backend/mongo.h"
#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/exception.h"

#include <ext/standard/php_rand.h>
#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

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
 *		'uri' => "mongodb://localhost:27017",
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
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_mongo_empty, 0, 0, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_mongo_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_Mongo, __construct, arginfo_phalcon_cache_backend_mongo___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
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

zend_object_handlers phalcon_cache_backend_mongo_object_handlers;
zend_object* phalcon_cache_backend_mongo_object_create_handler(zend_class_entry *ce)
{
	phalcon_cache_backend_mongo_object *intern = ecalloc(1, sizeof(phalcon_cache_backend_mongo_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_cache_backend_mongo_object_handlers;

	intern->client = NULL;
	intern->collection = NULL;
	return &intern->std;
}

void phalcon_cache_backend_mongo_object_free_handler(zend_object *object)
{
	phalcon_cache_backend_mongo_object *intern;
	intern = phalcon_cache_backend_mongo_object_from_obj(object);
	if (intern->collection) {
		mongoc_collection_destroy(intern->collection);
		intern->collection = NULL;
	}
	if (intern->client) {
		mongoc_client_destroy(intern->client);
		intern->client = NULL;
	}
	zend_object_std_dtor(object);
}

/**
 * Phalcon\Cache\Backend\Mongo initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Mongo){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT_EX(Phalcon\\Cache\\Backend, Mongo, cache_backend_mongo, phalcon_cache_backend_ce, phalcon_cache_backend_mongo_method_entry, 0);

	zend_declare_property_string(phalcon_cache_backend_mongo_ce, SL("_uri"), "mongodb://localhost:27017", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cache_backend_mongo_ce, SL("_db"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cache_backend_mongo_ce, SL("_collection"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\Mongo constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, __construct){

	zval *frontend, *options = NULL, uri = {}, db_name = {}, collection_name = {};
	phalcon_cache_backend_mongo_object *mongo_object;

	phalcon_fetch_params(0, 1, 1, &frontend, &options);

	if (!options || Z_TYPE_P(options) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The options must be array");
		return;
	}
	PHALCON_SEPARATE_PARAM(options);
	if (phalcon_array_isset_fetch_str(&uri, options, SL("uri"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_uri"), &uri);
		phalcon_array_unset_str(options, SL("uri"), 0);
	} else {
		phalcon_read_property(&uri, getThis(), SL("_uri"), PH_READONLY);
	}

	if (phalcon_array_isset_fetch_str(&db_name, options, SL("db"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_db"), &db_name);
		phalcon_array_unset_str(options, SL("db"), 0);
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The parameter 'db' is required");
		return;
	}

	if (phalcon_array_isset_fetch_str(&collection_name, options, SL("collection"), PH_READONLY) && PHALCON_IS_NOT_EMPTY(&collection_name)) {
		phalcon_update_property(getThis(), SL("_collection"), &collection_name);
		phalcon_array_unset_str(options, SL("collection"), 0);
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The parameter 'collection' is required");
		return;
	}

	PHALCON_CALL_PARENT(NULL, phalcon_cache_backend_mongo_ce, getThis(), "__construct", frontend, options);

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));
	mongo_object->client = mongoc_client_new(Z_STRVAL(uri));
	if (!mongo_object->client) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The URI parsed unsuccessfully");
		return;
	}

	mongo_object->collection = mongoc_client_get_collection(mongo_object->client, Z_STRVAL(db_name), Z_STRVAL(collection_name));
	if (!mongo_object->collection) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Can't get collection");
		return;
	}
}

static int phalcon_cache_mongo_get_value(zval* return_value, const bson_t* doc, const char* key) {
	bson_iter_t iter;
	bson_iter_init(&iter, doc);
	if (bson_iter_find(&iter, key)){
		if (BSON_ITER_HOLDS_UTF8(&iter)) {
			const char *str;
			uint32_t len;
			str = bson_iter_utf8(&iter, &len);
			ZVAL_NEW_STR(return_value, zend_string_init(str, len, 0));
		} else if (BSON_ITER_HOLDS_INT32 (&iter)) {
			ZVAL_LONG(return_value, bson_iter_int32(&iter));
		} else if (BSON_ITER_HOLDS_INT64 (&iter)) {
			ZVAL_LONG(return_value, bson_iter_int64(&iter));
		} else if (BSON_ITER_HOLDS_DOUBLE (&iter)) {
			ZVAL_DOUBLE(return_value, bson_iter_double(&iter));
		} else if (BSON_ITER_HOLDS_BOOL (&iter)) {
			if (bson_iter_bool(&iter)) {
				ZVAL_TRUE(return_value);
			} else {
				ZVAL_FALSE(return_value);
			}
		} else if (BSON_ITER_HOLDS_NULL (&iter)) {
			ZVAL_NULL(return_value);
		} else {
			ZVAL_NULL(return_value);
		}
		return 1;
	} else {
		ZVAL_NULL(return_value);
		return 0;
	}
}

/**
 * Returns a cached content
 *
 * @param string $keyName
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, get){

	zval *key_name, frontend = {}, prefix = {}, prefixed_key = {}, ttl = {};
	phalcon_cache_backend_mongo_object *mongo_object;
	mongoc_cursor_t *cursor;
	bson_t *query;
    const bson_t *doc;

	phalcon_fetch_params(0, 1, 0, &key_name);

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	PHALCON_CALL_METHOD(&ttl, getThis(), "getlifetime");

	if (likely(zend_is_true(&ttl))) {
		query = BCON_NEW (
			"key", BCON_UTF8(Z_STRVAL(prefixed_key)),
			"time", "{",
				"$gt", BCON_DATE_TIME(time(NULL) * 1000),
			"}"
		);
	} else {
		query = BCON_NEW (
			"key", BCON_UTF8(Z_STRVAL(prefixed_key)),
			"time", "{",
				"$gt", BCON_DATE_TIME(time(NULL) * 1000),
			"}"
		);
	}
	zval_ptr_dtor(&prefixed_key);

	ZVAL_NULL(return_value);

#ifdef PHALCON_MONGOC_HAS_FIND_OPTS
    cursor = mongoc_collection_find_with_opts(mongo_object->collection, query, NULL, NULL);
#else
    cursor = mongoc_collection_find(mongo_object->collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
#endif
    while (mongoc_cursor_next(cursor, &doc)) {
		zval cached_content = {};
		if (phalcon_cache_mongo_get_value(&cached_content, doc, "data")) {
			if (Z_TYPE(cached_content) == IS_STRING) {
				PHALCON_CALL_METHOD(return_value, &frontend, "afterretrieve", &cached_content);
			} else {
				ZVAL_COPY(return_value, &cached_content);
			}
			zval_ptr_dtor(&cached_content);
			break;
		}
    }

    bson_destroy(query);
	mongoc_cursor_destroy(cursor);
}

/**
 * Stores cached content into the Mongo backend and stops the frontend
 *
 * @param string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, key = {}, prefix = {}, prefixed_key = {}, frontend = {}, cached_content = {};
	zval prepared_content = {}, ttl = {}, is_buffering = {};
	phalcon_cache_backend_mongo_object *mongo_object;
	bson_t *query, *update, reply;
	bson_error_t error;

	phalcon_fetch_params(0, 0, 4, &key_name, &content, &lifetime, &stop_buffer);

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));

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

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_NOISY|PH_READONLY);

	if (!content || Z_TYPE_P(content) == IS_NULL) {
		PHALCON_CALL_METHOD(&cached_content, &frontend, "getcontent");
	} else {
		ZVAL_COPY(&cached_content, content);
	}

	if (!lifetime || Z_TYPE_P(lifetime) != IS_LONG) {
		PHALCON_CALL_METHOD(&ttl, getThis(), "getlifetime");
	} else {
		ZVAL_COPY(&ttl, lifetime);
	}

	query = BCON_NEW (
		"key", BCON_UTF8(Z_STRVAL(prefixed_key))
	);

	update = BCON_NEW (
		"key", BCON_UTF8(Z_STRVAL(prefixed_key)),
		"time", BCON_DATE_TIME((time(NULL) + phalcon_get_intval(&ttl)) * 1000)
	);
	zval_ptr_dtor(&prefixed_key);
	zval_ptr_dtor(&ttl);

	switch (Z_TYPE(cached_content)) {
		case IS_LONG:
			BSON_APPEND_INT64(update, "data", Z_LVAL(cached_content));
			break;
		case IS_DOUBLE:
			BSON_APPEND_DOUBLE(update, "data", Z_DVAL(cached_content));
			break;
		case IS_TRUE:
			BSON_APPEND_BOOL(update, "data", true);
			break;
		case IS_FALSE:
			BSON_APPEND_BOOL(update, "data", false);
			break;
		case IS_NULL:
			BSON_APPEND_NULL(update, "data");
			break;
		default:
			PHALCON_CALL_METHOD(&prepared_content, &frontend, "beforestore", &cached_content);
			if (Z_TYPE(prepared_content) != IS_STRING) {
				PHALCON_THROW_EXCEPTION_FORMAT(phalcon_cache_exception_ce, "The value can't save, type is %s", zend_get_type_by_const(Z_TYPE(prepared_content)));
				return;
			}
			BSON_APPEND_UTF8(update, "data", Z_STRVAL(prepared_content));
			zval_ptr_dtor(&prepared_content);
			break;
	}

    if (!mongoc_collection_find_and_modify(mongo_object->collection, query, NULL,
		update,
		NULL,
		false,
		true,
		true,
		&reply,
		&error
	)) {
		ZVAL_FALSE(return_value);
	} else {
		ZVAL_TRUE(return_value);
	}

	bson_destroy(update);
	bson_destroy(query);

	PHALCON_CALL_METHOD(&is_buffering, &frontend, "isbuffering");

	if (!stop_buffer || PHALCON_IS_TRUE(stop_buffer)) {
		PHALCON_CALL_METHOD(NULL, &frontend, "stop");
	}

	if (PHALCON_IS_TRUE(&is_buffering)) {
		zend_print_zval(&cached_content, 0);
	}
	zval_ptr_dtor(&cached_content);

	phalcon_update_property_bool(getThis(), SL("_started"), 0);
}

/**
 * Deletes a value from the cache by its key
 *
 * @param string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, delete){

	zval *key_name, prefix = {}, prefixed_key = {};
	phalcon_cache_backend_mongo_object *mongo_object;
	bson_t *query;
	bson_error_t error;

	phalcon_fetch_params(0, 1, 0, &key_name);

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	query = BCON_NEW (
		"key", BCON_UTF8(Z_STRVAL(prefixed_key))
	);
	zval_ptr_dtor(&prefixed_key);

    if (!mongoc_collection_remove(mongo_object->collection, MONGOC_REMOVE_SINGLE_REMOVE, query, NULL, &error)) {
		ZVAL_FALSE(return_value);
	} else {
		ZVAL_TRUE(return_value);
	}

    bson_destroy(query);

	if ((php_rand() % 100) == 0) {
		PHALCON_CALL_METHOD(NULL, getThis(), "gc");
	}
}

/**
 * Query the existing cached keys
 *
 * @param string $prefix
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, queryKeys){

	zval *_prefix = NULL, prefix = {}, prefixed_key = {};
	phalcon_cache_backend_mongo_object *mongo_object;
	mongoc_cursor_t *cursor;
	bson_t *query; // BSON_INITIALIZER
    const bson_t *doc;

	phalcon_fetch_params(0, 0, 1, &_prefix);

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));

	if (!_prefix || Z_TYPE_P(_prefix) != IS_NULL) {
		phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&prefix, _prefix);
	}

	query = bson_new();

	if (PHALCON_IS_NOT_EMPTY(&prefix)) {
		PHALCON_CONCAT_SVS(&prefixed_key, "#^", &prefix, "#");
		BSON_APPEND_REGEX(query, "key", Z_STRVAL(prefixed_key), "i");
		zval_ptr_dtor(&prefixed_key);
	}

 #if PHALCON_MONGOC_HAS_FIND_OPTS
     cursor = mongoc_collection_find_with_opts(mongo_object->collection, query, NULL, NULL);
 #else
     cursor = mongoc_collection_find(mongo_object->collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
 #endif

	array_init(return_value);
    while (mongoc_cursor_next(cursor, &doc)) {
		bson_iter_t iter;
		bson_iter_init(&iter, doc);
		if (bson_iter_find(&iter, "key") && BSON_ITER_HOLDS_UTF8(&iter)) {
			zval key_value = {};
			const char *str;
			uint32_t len;
			str = bson_iter_utf8(&iter, &len);
			ZVAL_NEW_STR(&key_value, zend_string_init(str, len, 0));
			phalcon_array_append(return_value, &key_value, 0);
		}
    }

	bson_destroy(query);
	mongoc_cursor_destroy(cursor);
}

/**
 * Checks if cache exists and it hasn't expired
 *
 * @param string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, exists){
	zval *key_name = NULL, prefix = {}, prefixed_key = {};
	phalcon_cache_backend_mongo_object *mongo_object;
	bson_t *query;
	bson_error_t error;

	phalcon_fetch_params(0, 1, 0, &key_name);

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);
	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	query = BCON_NEW (
		"time", "{",
			"$gt", BCON_DATE_TIME(time(NULL) * 1000),
		"}"
	);

	BSON_APPEND_UTF8(query, "key", Z_STRVAL(prefixed_key));
	zval_ptr_dtor(&prefixed_key);

    if (mongoc_collection_count(mongo_object->collection, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error)) {
		ZVAL_TRUE(return_value);
    } else {
		ZVAL_FALSE(return_value);
	}

	bson_destroy(query);
}

PHP_METHOD(Phalcon_Cache_Backend_Mongo, gc) {

	phalcon_cache_backend_mongo_object *mongo_object;
	bson_t query = BSON_INITIALIZER;
	bson_t child;
	bson_error_t error;

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));

	BSON_APPEND_DOCUMENT_BEGIN(&query, "time", &child);
	BSON_APPEND_DATE_TIME(&child, "$lt", time(NULL) * 1000);
	bson_append_document_end(&query, &child);
    if (!mongoc_collection_remove(mongo_object->collection, MONGOC_REMOVE_NONE, &query, NULL, &error)) {
		ZVAL_FALSE(return_value);
	} else {
		ZVAL_TRUE(return_value);
	}
}

/**
 * Increment of a given key by $value
 *
 * @param int|string $keyName
 * @param long $value
 * @return  mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, increment){

	zval *key_name, *value = NULL, prefix = {}, prefixed_key = {};
	phalcon_cache_backend_mongo_object *mongo_object;
	bson_t *query, update = BSON_INITIALIZER, reply;
	bson_error_t error;
	bson_t child;

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));

	if (!value || Z_TYPE_P(value) != IS_LONG) {
		value = &PHALCON_GLOBAL(z_one);
	}

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	query = BCON_NEW (
		"key", BCON_UTF8(Z_STRVAL(prefixed_key)),
		"time", "{",
			"$gt", BCON_DATE_TIME(time(NULL) * 1000),
		"}"
	);
	zval_ptr_dtor(&prefixed_key);

	BSON_APPEND_DOCUMENT_BEGIN(&update, "$inc", &child);
	BSON_APPEND_INT64(&child, "data", Z_LVAL_P(value));
	bson_append_document_end(&update, &child);

	if (!mongoc_collection_find_and_modify(mongo_object->collection, query, NULL, &update,
		NULL,
		false,
		false,
		true,
		&reply,
		&error
	)) {
		ZVAL_FALSE(return_value);
	} else {
        bson_iter_t iter;
        bson_iter_init(&iter, &reply);
        if (bson_iter_find(&iter, "value"))
        {
            const uint8_t *buf;
            uint32_t len;
            bson_iter_document(&iter, &len, &buf);
            bson_t value;
            bson_init_static(&value, buf, len);

            if (!phalcon_cache_mongo_get_value(return_value, &value, "data")) {
				ZVAL_FALSE(return_value);
			}
        } else {
			ZVAL_FALSE(return_value);
		}
	}
	bson_destroy(query);
}

/**
 * Decrement of a given key by $value
 *
 * @param int|string $keyName
 * @param long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, decrement){

	zval *key_name, *value = NULL, prefix = {}, prefixed_key = {};
	phalcon_cache_backend_mongo_object *mongo_object;
	bson_t *query, update = BSON_INITIALIZER, reply;
	bson_error_t error;
	bson_t child;
	long v = -1;

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));

	if (!value || Z_TYPE_P(value) != IS_LONG) {
		value = &PHALCON_GLOBAL(z_one);
	} else {
		v = -labs(Z_LVAL_P(value));
	}

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	query = BCON_NEW (
		"key", BCON_UTF8(Z_STRVAL(prefixed_key)),
		"time", "{",
			"$gt", BCON_DATE_TIME(time(NULL) * 1000),
		"}"
	);
	zval_ptr_dtor(&prefixed_key);

	BSON_APPEND_DOCUMENT_BEGIN(&update, "$inc", &child);
	BSON_APPEND_INT64(&child, "data", v);
	bson_append_document_end(&update, &child);

	if (!mongoc_collection_find_and_modify(mongo_object->collection, query, NULL, &update,
		NULL,
		false,
		false,
		true,
		&reply,
		&error
	)) {
		ZVAL_FALSE(return_value);
	} else {
        bson_iter_t iter;
        bson_iter_init(&iter, &reply);
        if (bson_iter_find(&iter, "value"))
        {
            const uint8_t *buf;
            uint32_t len;
            bson_iter_document(&iter, &len, &buf);
            bson_t value;
            bson_init_static(&value, buf, len);

            if (!phalcon_cache_mongo_get_value(return_value, &value, "data")) {
				ZVAL_FALSE(return_value);
			}
        } else {
			ZVAL_FALSE(return_value);
		}
	}
	bson_destroy(query);
}

/**
 * Immediately invalidates all existing items.
 *
 * @return bool
 */
PHP_METHOD(Phalcon_Cache_Backend_Mongo, flush){

	zval prefix = {}, prefixed_key = {};
	phalcon_cache_backend_mongo_object *mongo_object;
	bson_t query = BSON_INITIALIZER;
	bson_error_t error;

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));

	if (PHALCON_IS_NOT_EMPTY(&prefix)) {
		PHALCON_CONCAT_SVS(&prefixed_key, "#^", &prefix, "#");
		BSON_APPEND_REGEX(&query, "key", Z_STRVAL(prefixed_key), "i");
		zval_ptr_dtor(&prefixed_key);
	}

    if (!mongoc_collection_remove(mongo_object->collection, MONGOC_REMOVE_NONE, &query, NULL, &error)) {
		ZVAL_FALSE(return_value);
	} else {
		ZVAL_TRUE(return_value);
	}

	if ((php_rand() % 100) == 0) {
		PHALCON_CALL_METHOD(NULL, getThis(), "gc");
	}
}
