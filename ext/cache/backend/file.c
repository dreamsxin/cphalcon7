
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

#include "cache/backend/file.h"
#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/exception.h"

#include <ext/standard/file.h>
#include <ext/spl/spl_directory.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/concat.h"
#include "kernel/file.h"
#include "kernel/operators.h"
#include "kernel/string.h"

/**
 * Phalcon\Cache\Backend\File
 *
 * Allows to cache output fragments using a file backend
 *
 *<code>
 *	//Cache the file for 2 days
 *	$frontendOptions = array(
 *		'lifetime' => 172800
 *	);
 *
 *  //Create a output cache
 *  $frontCache = \Phalcon\Cache\Frontend\Output($frontOptions);
 *
 *	//Set the cache directory
 *	$backendOptions = array(
 *		'cacheDir' => '../app/cache/'
 *	);
 *
 *  //Create the File backend
 *  $cache = new \Phalcon\Cache\Backend\File($frontCache, $backendOptions);
 *
 *	$content = $cache->start('my-cache');
 *	if ($content === null) {
 *  	echo '<h1>', time(), '</h1>';
 *  	$cache->save();
 *	} else {
 *		echo $content;
 *	}
 *</code>
 */
zend_class_entry *phalcon_cache_backend_file_ce;

PHP_METHOD(Phalcon_Cache_Backend_File, __construct);
PHP_METHOD(Phalcon_Cache_Backend_File, get);
PHP_METHOD(Phalcon_Cache_Backend_File, save);
PHP_METHOD(Phalcon_Cache_Backend_File, delete);
PHP_METHOD(Phalcon_Cache_Backend_File, queryKeys);
PHP_METHOD(Phalcon_Cache_Backend_File, exists);
PHP_METHOD(Phalcon_Cache_Backend_File, increment);
PHP_METHOD(Phalcon_Cache_Backend_File, decrement);
PHP_METHOD(Phalcon_Cache_Backend_File, flush);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_file___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_file_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend_File, __construct, arginfo_phalcon_cache_backend_file___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend_File, get, arginfo_phalcon_cache_backendinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_File, save, arginfo_phalcon_cache_backendinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_File, delete, arginfo_phalcon_cache_backendinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_File, queryKeys, arginfo_phalcon_cache_backendinterface_querykeys, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_File, exists, arginfo_phalcon_cache_backendinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_File, increment, arginfo_phalcon_cache_backendinterface_increment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_File, decrement, arginfo_phalcon_cache_backendinterface_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend_File, flush, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend\File initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_File){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, File, cache_backend_file, phalcon_cache_backend_ce, phalcon_cache_backend_file_method_entry, 0);

	zend_declare_property_null(phalcon_cache_backend_file_ce, SL("_cacheDir"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_cache_backend_file_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend\File constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend_File, __construct){

	zval *frontend, *options, cache_dir = {};
	zend_long mode = 0777;

	phalcon_fetch_params(0, 2, 0, &frontend, &options);

	if (unlikely(!phalcon_array_isset_fetch_str(&cache_dir, options, SL("cacheDir"), PH_READONLY))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Cache directory must be specified with the option cacheDir");
		return;
	}
	phalcon_update_property(getThis(), SL("_cacheDir"), &cache_dir);

	PHALCON_CALL_PARENT(NULL, phalcon_cache_backend_file_ce, getThis(), "__construct", frontend, options);

	if (phalcon_file_exists(&cache_dir) == FAILURE) {
		php_stream_mkdir(Z_STRVAL(cache_dir), (int)mode, PHP_STREAM_MKDIR_RECURSIVE | REPORT_ERRORS, php_stream_context_from_zval(NULL, 0));
	}
}

/**
 * Returns a cached content
 *
 * @param string $keyName
 * @param int $lifetime
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_File, get){

	zval *key_name, *lifetime = NULL, prefix = {}, prefixed_key = {}, cache_dir = {}, cache_file = {}, frontend = {};
	zval modified_time = {}, cached_content = {}, exception_message = {};
	long int now, ttl, mtime, diff;
	int expired;

	phalcon_fetch_params(0, 1, 1, &key_name, &lifetime);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cache_dir, getThis(), SL("_cacheDir"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	PHALCON_CONCAT_VV(&cache_file, &cache_dir, &prefixed_key);
	zval_ptr_dtor(&prefixed_key);

	if (phalcon_file_exists(&cache_file) == SUCCESS) {
		phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);

		/**
		 * Check if the file has expired
		 */
		now = (long int)time(NULL);

		phalcon_filemtime(&modified_time, &cache_file);
		if (unlikely(Z_TYPE(modified_time) != IS_LONG)) {
			convert_to_long(&modified_time);
		}

		if (!lifetime || Z_TYPE_P(lifetime) != IS_LONG) {
			zval tmp = {};
			PHALCON_CALL_METHOD(&tmp, getThis(), "getlifetime");
			ttl = phalcon_get_intval(&tmp);
			zval_ptr_dtor(&tmp);
		} else {
			ttl = phalcon_get_intval(lifetime);
		}

		mtime   = Z_LVAL(modified_time);
		diff    = now - ttl;
		expired = diff > mtime;

		/**
		 * The content is only retrieved if the content has not expired
		 */
		if (!expired) {
			/**
			 * Use file-get-contents to control that the openbase_dir can't be skipped
			 */
			phalcon_file_get_contents(&cached_content, &cache_file);
			if (PHALCON_IS_FALSE(&cached_content)) {
				PHALCON_CONCAT_SVS(&exception_message, "Cache file ", &cache_file, " could not be opened");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_cache_exception_ce, &exception_message);
				goto end;
			}

			if (phalcon_is_numeric(&cached_content)) {
				ZVAL_COPY(return_value, &cached_content);
			} else {
				/**
				 * Use the frontend to process the content of the cache
				 */
				PHALCON_RETURN_CALL_METHOD(&frontend, "afterretrieve", &cached_content);
			}
			zval_ptr_dtor(&cached_content);
		} else {
			phalcon_unlink(return_value, &cache_file);
			ZVAL_NULL(return_value);
		}
	} else {
		ZVAL_NULL(return_value);
	}
end:
	zval_ptr_dtor(&cache_file);
}

/**
 * Stores cached content into the file backend and stops the frontend
 *
 * @param string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_File, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL, frontend = {};
	zval key = {}, prefix = {}, prefixed_key = {}, cache_dir = {}, cache_file = {}, cached_content = {}, prepared_content = {}, status = {}, is_buffering = {};

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

	phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cache_dir, getThis(), SL("_cacheDir"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(&cache_file, &cache_dir, &prefixed_key);
	zval_ptr_dtor(&prefixed_key);

	if (!content || !zend_is_true(content)) {
		PHALCON_CALL_METHOD(&cached_content, &frontend, "getcontent");
	} else {
		ZVAL_COPY(&cached_content, content);
	}

	/**
	 * We use file_put_contents to respect open-base-dir directive
	 */
	if (!phalcon_is_numeric(&cached_content)) {
		PHALCON_CALL_METHOD(&prepared_content, &frontend, "beforestore", &cached_content);
		phalcon_file_put_contents(&status, &cache_file, &prepared_content);
		zval_ptr_dtor(&prepared_content);
	} else {
		phalcon_file_put_contents(&status, &cache_file, &cached_content);
	}

	if (PHALCON_IS_FALSE(&status)) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_cache_exception_ce, "Cache directory is not writable: %s", &cache_file);
		zval_ptr_dtor(&cache_file);
		zval_ptr_dtor(&cached_content);
		return;
	}
	zval_ptr_dtor(&cache_file);

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
PHP_METHOD(Phalcon_Cache_Backend_File, delete){

	zval *key_name, prefix = {}, prefixed_key = {}, cache_dir = {}, cache_file = {};

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cache_dir, getThis(), SL("_cacheDir"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	PHALCON_CONCAT_VV(&cache_file, &cache_dir, &prefixed_key);
	zval_ptr_dtor(&prefixed_key);

	if (phalcon_file_exists(&cache_file) == SUCCESS) {
		phalcon_unlink(return_value, &cache_file);
		zval_ptr_dtor(&cache_file);
		return;
	}
	zval_ptr_dtor(&cache_file);

	RETURN_FALSE;
}

/**
 * Query the existing cached keys
 *
 * @param string $prefix
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_File, queryKeys){

	zval *prefix = NULL, cache_dir = {}, iterator = {};
	zend_object_iterator *it;

	phalcon_fetch_params(0, 0, 1, &prefix);

	array_init(return_value);

	phalcon_read_property(&cache_dir, getThis(), SL("_cacheDir"), PH_NOISY|PH_READONLY);

	/**
	 * We use a directory iterator to traverse the cache dir directory
	 */
	object_init_ex(&iterator, spl_ce_DirectoryIterator);
	PHALCON_CALL_METHOD(NULL, &iterator, "__construct", &cache_dir);

	/* DirectoryIterator implements Iterator */
	assert(instanceof_function_ex(spl_ce_DirectoryIterator, zend_ce_iterator, 1));

	it = spl_ce_DirectoryIterator->get_iterator(spl_ce_DirectoryIterator, &iterator, 0);

	/* DirectoryIterator is an iterator */
	assert(it != NULL);

	/* DirectoryIterator has rewind() method */
	assert(it->funcs->rewind != NULL);

	it->funcs->rewind(it);
	while (it->funcs->valid(it) == SUCCESS && !EG(exception)) {
		zval *item, is_directory = {}, key = {};

		item = it->funcs->get_current_data(it);

		if (FAILURE == phalcon_call_method(&is_directory, item, "isdir", 0, NULL)) {
			break;
		}

		if (!EG(exception) && PHALCON_IS_FALSE(&is_directory)) {
			if (FAILURE == phalcon_call_method(&key, item, "getfilename", 0, NULL)) {
				break;
			}

			if (!EG(exception) && (!prefix || phalcon_start_with(&key, prefix, NULL))) {
				phalcon_array_append(return_value, &key, PH_COPY);
			}
			zval_ptr_dtor(&key);
		}

		it->funcs->move_forward(it);
	}

	it->funcs->dtor(it);
	//efree(it);
	zval_ptr_dtor(&iterator);
}

/**
 * Checks if cache exists and it isn't expired
 *
 * @param string $keyName
 * @param  long $lifetime
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_File, exists){

	zval *key_name, prefix = {}, prefixed_key = {}, cache_dir = {}, cache_file = {}, frontend = {}, lifetime = {}, modified_time = {};
	long int mtime, ttl;

	phalcon_fetch_params(0, 1, 0, &key_name);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_READONLY);
	phalcon_read_property(&cache_dir, getThis(), SL("_cacheDir"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	PHALCON_CONCAT_VV(&cache_file, &cache_dir, &prefixed_key);
	zval_ptr_dtor(&prefixed_key);

	if (phalcon_file_exists(&cache_file) == SUCCESS) {
		phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_READONLY);

		/**
		 * Check if the file has expired
		 */
		PHALCON_CALL_METHOD(&lifetime, &frontend, "getlifetime");

		ttl = Z_LVAL(lifetime) ;
		zval_ptr_dtor(&lifetime);

		phalcon_filemtime(&modified_time, &cache_file);
		mtime = likely(Z_TYPE(modified_time) == IS_LONG) ? Z_LVAL(modified_time) : phalcon_get_intval(&modified_time);
		zval_ptr_dtor(&modified_time);

		if (mtime + ttl > (long int)time(NULL)) {
			RETVAL_TRUE;
		}
	} else {
		RETVAL_FALSE;
	}
	zval_ptr_dtor(&cache_file);
}

/**
 * Increment of a given key, by number $value
 *
 * @param string $keyName
 * @param long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_File, increment){

	zval *key_name, *value = NULL, prefix = {}, prefixed_key = {}, status = {};
	zval cache_dir = {}, cache_file = {}, frontend = {}, lifetime = {}, modified_time = {}, cached_content = {};
	long int now, ttl, mtime, diff;
	int expired;

	phalcon_fetch_params(0, 1, 1, &key_name, &value);

	if (!value) {
		value = &PHALCON_GLOBAL(z_one);
	}

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cache_dir, getThis(), SL("_cacheDir"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);

	PHALCON_CONCAT_VV(&cache_file, &cache_dir, &prefixed_key);
	zval_ptr_dtor(&prefixed_key);
	assert(Z_TYPE(cache_file) == IS_STRING);

	if (phalcon_file_exists(&cache_file) == SUCCESS) {
		phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_NOISY|PH_READONLY);
		/**
		 * Check if the file has expired
		 */
		now = (long int)time(NULL);

		/**
		 * Take the lifetime from the frontend or read it from the set in start()
		 */
		PHALCON_CALL_METHOD(&lifetime, getThis(), "getlifetime");
		ttl = phalcon_get_intval(&lifetime);
		zval_ptr_dtor(&lifetime);

		phalcon_filemtime(&modified_time, &cache_file);
		if (unlikely(Z_TYPE(modified_time) != IS_LONG)) {
			convert_to_long(&modified_time);
		}

		mtime   = Z_LVAL(modified_time);
		diff    = now - ttl;
		expired = diff > mtime;
		zval_ptr_dtor(&modified_time);

		/**
		 * The content is only retrieved if the content has not expired
		 */
		if (!expired) {
			/**
			 * Use file-get-contents to control that the openbase_dir can't be skipped
			 */
			phalcon_file_get_contents(&cached_content, &cache_file);
			if (PHALCON_IS_FALSE(&cached_content)) {
				zend_throw_exception_ex(phalcon_cache_exception_ce, 0, "Failed to open cache file %s", Z_STRVAL(cache_file));
				zval_ptr_dtor(&cache_file);
				return;
			}

			phalcon_add_function(return_value, &cached_content, value);
			zval_ptr_dtor(&cached_content);

			phalcon_file_put_contents(&status, &cache_file, return_value);

			if (PHALCON_IS_FALSE(&status)) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Cache directory can't be written");
			}
		}
	} else {
		RETVAL_FALSE;
	}
	zval_ptr_dtor(&cache_file);
}

/**
 * Decrement of a given key, by number $value
 *
 * @param string $keyName
 * @param long $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_File, decrement){

	zval *key_name, *value = NULL, prefix = {}, prefixed_key = {}, status = {};
	zval cache_dir = {}, cache_file = {}, frontend = {}, lifetime = {}, modified_time = {}, cached_content = {};
	long int now, ttl, mtime, diff;
	int expired;

	phalcon_fetch_params(0, 1, 1, &key_name, &value);
	if (!value) {
		value = &PHALCON_GLOBAL(z_one);
	}

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cache_dir, getThis(), SL("_cacheDir"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(&prefixed_key, &prefix, key_name);
	PHALCON_CONCAT_VV(&cache_file, &cache_dir, &prefixed_key);
	zval_ptr_dtor(&prefixed_key);
	assert(Z_TYPE(cache_file) == IS_STRING);

	if (phalcon_file_exists(&cache_file) == SUCCESS) {
		phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_NOISY|PH_READONLY);
		/**
		 * Check if the file has expired
		 */
		now = (long int)time(NULL);

		/**
		 * Take the lifetime from the frontend or read it from the set in start()
		 */
		PHALCON_CALL_METHOD(&lifetime, getThis(), "getlifetime");
		ttl = phalcon_get_intval(&lifetime);
		zval_ptr_dtor(&lifetime);

		phalcon_filemtime(&modified_time, &cache_file);
		if (unlikely(Z_TYPE(modified_time) != IS_LONG)) {
			convert_to_long(&modified_time);
		}

		mtime   = Z_LVAL(modified_time);
		diff    = now - ttl;
		expired = diff > mtime;
		zval_ptr_dtor(&modified_time);

		/**
		 * The content is only retrieved if the content has not expired
		 */
		if (!expired) {
			/**
			 * Use file-get-contents to control that the openbase_dir can't be skipped
			 */
			phalcon_file_get_contents(&cached_content, &cache_file);
			if (PHALCON_IS_FALSE(&cached_content)) {
				zend_throw_exception_ex(phalcon_cache_exception_ce, 0, "Failed to open cache file %s", Z_STRVAL(cache_file));
				zval_ptr_dtor(&cache_file);
				return;
			}

			phalcon_sub_function(return_value, &cached_content, value);
			zval_ptr_dtor(&cached_content);

			phalcon_file_put_contents(&status, &cache_file, return_value);

			if (PHALCON_IS_FALSE(&status)) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "Cache directory can't be written");
			}
		}
	} else {
		RETVAL_FALSE;
	}
	zval_ptr_dtor(&cache_file);
}

/**
 * Immediately invalidates all existing items.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_File, flush){

	zval prefix = {}, cache_dir = {}, iterator = {};
	zend_object_iterator *it;

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cache_dir, getThis(), SL("_cacheDir"), PH_NOISY|PH_READONLY);

	object_init_ex(&iterator, spl_ce_DirectoryIterator);
	assert(phalcon_has_constructor(&iterator));
	PHALCON_CALL_METHOD(NULL, &iterator, "__construct", &cache_dir);

	/* DirectoryIterator implements Iterator */
	assert(instanceof_function_ex(spl_ce_DirectoryIterator, zend_ce_iterator, 1));

	it = spl_ce_DirectoryIterator->get_iterator(spl_ce_DirectoryIterator, &iterator, 0);

	/* DirectoryIterator is an iterator */
	assert(it != NULL);

	/* DirectoryIterator has rewind() method */
	assert(it->funcs->rewind != NULL);

	it->funcs->rewind(it);
	while (it->funcs->valid(it) == SUCCESS && !EG(exception)) {
		zval *item, is_file = {}, key = {}, cache_file = {};
		item = it->funcs->get_current_data(it);

		if (FAILURE == phalcon_call_method(&is_file, item, "isfile", 0, NULL)) {
			break;
		}

		if (PHALCON_IS_TRUE(&is_file)) {
			if (FAILURE == phalcon_call_method(&key, item, "getfilename", 0, NULL)) {
				break;
			}

			if (FAILURE == phalcon_call_method(&cache_file, item, "getpathname", 0, NULL)) {
				zval_ptr_dtor(&key);
				break;
			}

			if (PHALCON_IS_EMPTY(&prefix) || phalcon_start_with(&key, &prefix, NULL)) {
				phalcon_unlink(return_value, &cache_file);
			}
			zval_ptr_dtor(&key);
			zval_ptr_dtor(&cache_file);
		}

		it->funcs->move_forward(it);
	}

	it->funcs->dtor(it);
	//efree(it);
	zval_ptr_dtor(&iterator);

	RETURN_TRUE;
}
