
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

#include "storage/leveldb/writebatch.h"
#include "storage/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/file.h"
#include "kernel/exception.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Storage\Leveldb\Writebatch
 *
 */
zend_class_entry *phalcon_storage_leveldb_writebatch_ce;

PHP_METHOD(Phalcon_Storage_Leveldb_Writebatch, put);
PHP_METHOD(Phalcon_Storage_Leveldb_Writebatch, delete);
PHP_METHOD(Phalcon_Storage_Leveldb_Writebatch, clear);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_leveldb_wirtebatch_put, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_leveldb_wirtebatch_delete, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_storage_leveldb_writebatch_method_entry[] = {
	PHP_ME(Phalcon_Storage_Leveldb_Writebatch, put, arginfo_phalcon_storage_leveldb_wirtebatch_put, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Leveldb_Writebatch, delete, arginfo_phalcon_storage_leveldb_wirtebatch_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Leveldb_Writebatch, clear, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_storage_leveldb_writebatch_object_handlers;
zend_object* phalcon_storage_leveldb_writebatch_object_create_handler(zend_class_entry *ce)
{
	phalcon_storage_leveldb_writebatch_object *intern = ecalloc(1, sizeof(phalcon_storage_leveldb_writebatch_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_storage_leveldb_writebatch_object_handlers;

	intern->batch = leveldb_writebatch_create();

	return &intern->std;
}

void phalcon_storage_leveldb_writebatch_object_free_handler(zend_object *object)
{
	phalcon_storage_leveldb_writebatch_object *intern = phalcon_storage_leveldb_writebatch_object_from_obj(object);
	if (intern->batch) {
		leveldb_writebatch_destroy(intern->batch);
		intern->batch = NULL;
	}
}

/**
 * Phalcon\Storage\Leveldb\Writebatch initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_Leveldb_Writebatch){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Storage\\Leveldb, Writebatch, storage_leveldb_writebatch, phalcon_storage_leveldb_writebatch_method_entry, 0);

	return SUCCESS;
}

/**
 * Adds a put operation for the given key and value to the write batch
 *
 * @param string $key
 * @param string $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Leveldb_Writebatch, put)
{
	zval *key, *value;
	phalcon_storage_leveldb_writebatch_object *intern;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	intern = phalcon_storage_leveldb_writebatch_object_from_obj(Z_OBJ_P(getThis()));

	leveldb_writebatch_put(intern->batch, Z_STRVAL_P(key), Z_STRLEN_P(key), Z_STRVAL_P(value), Z_STRLEN_P(value));

	RETURN_TRUE;
}

/**
 * Adds a deletion operation for the given key to the write batch
 *
 * @param string $key
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Leveldb_Writebatch, delete)
{
	zval *key;
	phalcon_storage_leveldb_writebatch_object *intern;

	phalcon_fetch_params(0, 1, 0, &key);

	intern = phalcon_storage_leveldb_writebatch_object_from_obj(Z_OBJ_P(getThis()));

	leveldb_writebatch_delete(intern->batch, Z_STRVAL_P(key), Z_STRLEN_P(key));

	RETURN_TRUE;
}

/**
 * Clears all of operations in the write batch
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Leveldb_Writebatch, clear)
{
	phalcon_storage_leveldb_writebatch_object *intern;

	intern = phalcon_storage_leveldb_writebatch_object_from_obj(Z_OBJ_P(getThis()));

	leveldb_writebatch_clear(intern->batch);

	RETURN_TRUE;
}
