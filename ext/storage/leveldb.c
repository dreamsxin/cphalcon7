
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

#include "storage/leveldb.h"
#include "storage/leveldb/iterator.h"
#include "storage/leveldb/writebatch.h"
#include "storage/exception.h"

#include <ext/standard/file.h>
#include <zend_smart_str.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/file.h"
#include "kernel/exception.h"

/**
 * Phalcon\Storage\Leveldb
 *
 */
zend_class_entry *phalcon_storage_leveldb_ce;

PHP_METHOD(Phalcon_Storage_Leveldb, __construct);
PHP_METHOD(Phalcon_Storage_Leveldb, get);
PHP_METHOD(Phalcon_Storage_Leveldb, put);
PHP_METHOD(Phalcon_Storage_Leveldb, write);
PHP_METHOD(Phalcon_Storage_Leveldb, delete);
PHP_METHOD(Phalcon_Storage_Leveldb, iterator);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_leveldb___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_leveldb_get, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_leveldb_put, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_leveldb_write, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, batch, Phalcon\\Storage\\Leveldb\\Writebatch, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_leveldb_delete, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_storage_leveldb_method_entry[] = {
	PHP_ME(Phalcon_Storage_Leveldb, __construct, arginfo_phalcon_storage_leveldb___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Storage_Leveldb, get, arginfo_phalcon_storage_leveldb_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Leveldb, put, arginfo_phalcon_storage_leveldb_put, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Leveldb, write, arginfo_phalcon_storage_leveldb_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Leveldb, delete, arginfo_phalcon_storage_leveldb_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Leveldb, iterator, NULL, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Storage_Leveldb, set, put, arginfo_phalcon_storage_leveldb_put, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_storage_leveldb_object_handlers;
zend_object* phalcon_storage_leveldb_object_create_handler(zend_class_entry *ce)
{
	phalcon_storage_leveldb_object *intern = ecalloc(1, sizeof(phalcon_storage_leveldb_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_storage_leveldb_object_handlers;

	return &intern->std;
}

void phalcon_storage_leveldb_object_free_handler(zend_object *object)
{
	phalcon_storage_leveldb_object *intern;

	intern = phalcon_storage_leveldb_object_from_obj(object);

	if (intern->db) {
		leveldb_close(intern->db);
	}
}

/**
 * Phalcon\Storage\Leveldb initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_Leveldb){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Storage, Leveldb, storage_leveldb, phalcon_storage_leveldb_method_entry, 0);

	zend_declare_property_null(phalcon_storage_leveldb_ce, SL("_path"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_storage_leveldb_ce, SL("_options"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Storage\Leveldb constructor
 *
 * @param string $path
 * @param string $options
 */
PHP_METHOD(Phalcon_Storage_Leveldb, __construct)
{
	zval *path, *_options = NULL;
	leveldb_options_t *options;
	phalcon_storage_leveldb_object *intern;
	char *err = NULL;

	phalcon_fetch_params(0, 1, 1, &path, &_options);

	intern = phalcon_storage_leveldb_object_from_obj(Z_OBJ_P(getThis()));

	options = leveldb_options_create();

	/* default true */
	leveldb_options_set_create_if_missing(options, 1);

	if (_options && Z_TYPE_P(_options) == IS_ARRAY) {
		zval value = {};
		if (phalcon_array_isset_fetch_str(&value, _options, SL("create_if_missing"), PH_READONLY)) {
			leveldb_options_set_create_if_missing(options, zend_is_true(&value));
		}

		if (phalcon_array_isset_fetch_str(&value, _options, SL("error_if_exists"), PH_READONLY)) {
			leveldb_options_set_error_if_exists(options, zend_is_true(&value));
		}

		if (phalcon_array_isset_fetch_str(&value, _options, SL("paranoid_checks"), PH_READONLY)) {
			leveldb_options_set_paranoid_checks(options, zend_is_true(&value));
		}

		if (phalcon_array_isset_fetch_str(&value, _options, SL("write_buffer_size"), PH_READONLY)) {
			convert_to_long(&value);
			leveldb_options_set_write_buffer_size(options, Z_LVAL(value));
		}

		if (phalcon_array_isset_fetch_str(&value, _options, SL("max_open_files"), PH_READONLY)) {
			convert_to_long(&value);
			leveldb_options_set_max_open_files(options, Z_LVAL(value));
		}

		if (phalcon_array_isset_fetch_str(&value, _options, SL("block_size"), PH_READONLY)) {
			convert_to_long(&value);
			leveldb_options_set_block_size(options, Z_LVAL(value));
		}

		if (phalcon_array_isset_fetch_str(&value, _options, SL("block_cache_size"), PH_READONLY)) {
			convert_to_long(&value);
			leveldb_options_set_cache(options, leveldb_cache_create_lru(Z_LVAL(value)));
		}

		if (phalcon_array_isset_fetch_str(&value, _options, SL("block_restart_interval"), PH_READONLY)) {
			convert_to_long(&value);
			leveldb_options_set_block_restart_interval(options, Z_LVAL(value));
		}

		if (phalcon_array_isset_fetch_str(&value, _options, SL("compression"), PH_READONLY)) {
			convert_to_long(&value);
			if (Z_LVAL(value) != leveldb_no_compression && Z_LVAL(value) != leveldb_snappy_compression) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid compression type");
			} else {
				leveldb_options_set_compression(options, Z_LVAL(value));
			}
		}
	}

	intern->db = leveldb_open(options, Z_STRVAL_P(path), &err);
	leveldb_options_destroy(options);
	if (err != NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, err);
		free(err);
		return;
	}
}

/**
 * Returns the value for the given key or false
 *
 * @param string $key
 * @param array $options
 * @return string
 */
PHP_METHOD(Phalcon_Storage_Leveldb, get)
{
	zval *key, *_options = NULL;
	leveldb_readoptions_t *options;
	phalcon_storage_leveldb_object *intern;
	char *value, *err = NULL;
	size_t value_len;

	phalcon_fetch_params(0, 1, 1, &key, &_options);

	intern = phalcon_storage_leveldb_object_from_obj(Z_OBJ_P(getThis()));

	options = leveldb_readoptions_create();

	if (_options && Z_TYPE_P(_options) == IS_ARRAY) {
		zval value = {};

		if (phalcon_array_isset_fetch_str(&value, _options, SL("verify_check_sum"), PH_READONLY)) {
			leveldb_readoptions_set_verify_checksums(options, zend_is_true(&value));
		}

		if (phalcon_array_isset_fetch_str(&value, _options, SL("fill_cache"), PH_READONLY)) {
			leveldb_readoptions_set_fill_cache(options, zend_is_true(&value));
		}
	}

	value = leveldb_get(intern->db, options, Z_STRVAL_P(key), Z_STRLEN_P(key), &value_len, &err);
	leveldb_readoptions_destroy(options);

	if (err != NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, err);
		free(err);
		return;
	}

	if (value == NULL) {
		RETURN_FALSE;
	}

	RETVAL_STRINGL(value, value_len);
	free(value);
}

/**
 * Puts the value for the given key
 *
 * @param string $key
 * @param string $value
 * @param array $options
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Leveldb, put)
{
	zval *key, *value, *_options = NULL;
	leveldb_writeoptions_t *options;
	phalcon_storage_leveldb_object *intern;
	char *err = NULL;

	phalcon_fetch_params(0, 2, 1, &key, &value, &_options);

	intern = phalcon_storage_leveldb_object_from_obj(Z_OBJ_P(getThis()));

	options = leveldb_writeoptions_create();

	if (_options && Z_TYPE_P(_options) == IS_ARRAY) {
		zval value = {};

		if (phalcon_array_isset_fetch_str(&value, _options, SL("sync"), PH_READONLY)) {
			leveldb_writeoptions_set_sync(options, zend_is_true(&value));
		}
	}

	leveldb_put(intern->db, options, Z_STRVAL_P(key), Z_STRLEN_P(key), Z_STRVAL_P(value), Z_STRLEN_P(value), &err);
	leveldb_writeoptions_destroy(options);

	if (err != NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, err);
		free(err);
		return;
	}

	RETURN_TRUE;
}

/**
 * Executes all of the operations added in the write batch
 *
 * @param string $key
 * @param Phalcon\Storage\Leveldb\Writebatch $batch
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Leveldb, write)
{
	zval *batch, *_options = NULL;
	leveldb_writeoptions_t *options;
	phalcon_storage_leveldb_object *intern;
	phalcon_storage_leveldb_writebatch_object *batch_intern;
	char *err = NULL;

	phalcon_fetch_params(0, 1, 1, &batch, &_options);

	intern = phalcon_storage_leveldb_object_from_obj(Z_OBJ_P(getThis()));
	batch_intern = phalcon_storage_leveldb_writebatch_object_from_obj(Z_OBJ_P(batch));

	options = leveldb_writeoptions_create();

	if (_options && Z_TYPE_P(_options) == IS_ARRAY) {
		zval value = {};

		if (phalcon_array_isset_fetch_str(&value, _options, SL("sync"), PH_READONLY)) {
			leveldb_writeoptions_set_sync(options, zend_is_true(&value));
		}
	}

	leveldb_write(intern->db, options, batch_intern->batch, &err);
	leveldb_writeoptions_destroy(options);

	if (err != NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, err);
		free(err);
		return;
	}

	RETURN_TRUE;
}

/**
 * Deletes the given key
 *
 * @param string $key
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Leveldb, delete)
{
	zval *key, *_options = NULL;
	leveldb_writeoptions_t *options;
	phalcon_storage_leveldb_object *intern;
	char *err = NULL;

	phalcon_fetch_params(0, 1, 0, &key, &_options);

	intern = phalcon_storage_leveldb_object_from_obj(Z_OBJ_P(getThis()));

	options = leveldb_writeoptions_create();

	if (_options && Z_TYPE_P(_options) == IS_ARRAY) {
		zval value = {};

		if (phalcon_array_isset_fetch_str(&value, _options, SL("sync"), PH_READONLY)) {
			leveldb_writeoptions_set_sync(options, zend_is_true(&value));
		}
	}

	leveldb_delete(intern->db, options, Z_STRVAL_P(key), Z_STRLEN_P(key), &err);
	leveldb_writeoptions_destroy(options);

	if (err != NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, err);
		free(err);
		return;
	}

	RETURN_TRUE;
}

/**
 * Gets a new iterator for the db
 *
 * @return Phalcon\Storage\Leveldb\Iterator
 */
PHP_METHOD(Phalcon_Storage_Leveldb, iterator)
{
	zval *_options = NULL;
	leveldb_readoptions_t *options;
	phalcon_storage_leveldb_object *intern;
	phalcon_storage_leveldb_iterator_object *iterator_intern;

	phalcon_fetch_params(0, 0, 1, &_options);

	intern = phalcon_storage_leveldb_object_from_obj(Z_OBJ_P(getThis()));

	options = leveldb_readoptions_create();

	if (_options && Z_TYPE_P(_options) == IS_ARRAY) {
		zval value = {};

		if (phalcon_array_isset_fetch_str(&value, _options, SL("verify_check_sum"), PH_READONLY)) {
			leveldb_readoptions_set_verify_checksums(options, zend_is_true(&value));
		}

		if (phalcon_array_isset_fetch_str(&value, _options, SL("fill_cache"), PH_READONLY)) {
			leveldb_readoptions_set_fill_cache(options, zend_is_true(&value));
		}
	}

	object_init_ex(return_value, phalcon_storage_leveldb_iterator_ce);
	iterator_intern = phalcon_storage_leveldb_iterator_object_from_obj(Z_OBJ_P(return_value));

	iterator_intern->iterator = leveldb_create_iterator(intern->db, options);
	leveldb_readoptions_destroy(options);
}
