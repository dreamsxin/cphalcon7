
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

#include "storage/btree.h"
#include "storage/exception.h"

#include "zend_smart_str.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/file.h"
#include "kernel/exception.h"

/**
 * Phalcon\Storage\Btree
 *
 * It can be used to replace APC or local memstoraged.
 */
zend_class_entry *phalcon_storage_btree_ce;

PHP_METHOD(Phalcon_Storage_Btree, __construct);
PHP_METHOD(Phalcon_Storage_Btree, set);
PHP_METHOD(Phalcon_Storage_Btree, get);
PHP_METHOD(Phalcon_Storage_Btree, delete);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_btree___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, db, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_btree_set, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_btree_get, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_btree_delete, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_storage_btree_method_entry[] = {
	PHP_ME(Phalcon_Storage_Btree, __construct, arginfo_phalcon_storage_btree___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Storage_Btree, set, arginfo_phalcon_storage_btree_set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Btree, get, arginfo_phalcon_storage_btree_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Btree, delete, arginfo_phalcon_storage_btree_delete, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_storage_btree_object_handlers;
zend_object* phalcon_storage_btree_object_create_handler(zend_class_entry *ce)
{
	phalcon_storage_btree_object *intern = ecalloc(1, sizeof(phalcon_storage_btree_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_storage_btree_object_handlers;

	return &intern->std;
}

void phalcon_storage_btree_object_free_handler(zend_object *object)
{
	phalcon_storage_btree_object *intern = phalcon_storage_btree_object_from_obj(object);

	phalcon_storage_btree_close(&intern->db);
}

/**
 * Phalcon\Storage\Btree initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_Btree){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Storage, Btree, storage_btree, phalcon_storage_btree_method_entry, 0);

	zend_declare_property_null(phalcon_storage_btree_ce, SL("_db"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Storage\Btree constructor
 *
 * @param string $db
 */
PHP_METHOD(Phalcon_Storage_Btree, __construct)
{
	zval *db;
	phalcon_storage_btree_object *intern;

	phalcon_fetch_params(0, 1, 0, &db);

	phalcon_update_property(getThis(), SL("_db"), db);

	intern = phalcon_storage_btree_object_from_obj(Z_OBJ_P(getThis()));

	if (phalcon_storage_btree_open(&intern->db, Z_STRVAL_P(db)) != PHALCON_STORAGE_BTREE_OK){
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to open Db %s", Z_STRVAL_P(db));
		return;
	}
}

/**
 * Stores storaged content
 *
 * @param string $key
 * @param string $value
 * @return string
 */
PHP_METHOD(Phalcon_Storage_Btree, set)
{
	zval *key, *value;
	phalcon_storage_btree_object *intern;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	intern = phalcon_storage_btree_object_from_obj(Z_OBJ_P(getThis()));

	if (phalcon_storage_btree_sets(&intern->db, Z_STRVAL_P(key), Z_STRVAL_P(value)) != PHALCON_STORAGE_BTREE_OK){
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

/**
 * Returns a storaged content
 *
 * @param string $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Btree, get)
{
	zval *key;
	phalcon_storage_btree_object *intern;
	char* value;

	phalcon_fetch_params(0, 1, 0, &key);

	intern = phalcon_storage_btree_object_from_obj(Z_OBJ_P(getThis()));

	if (phalcon_storage_btree_gets(&intern->db, Z_STRVAL_P(key), &value) == PHALCON_STORAGE_BTREE_OK){
		RETURN_STRING(value);
		return;
	}

	RETURN_NULL();
}

/**
 * Returns a storaged content
 *
 * @param string|array $keys
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Btree, delete)
{
	zval *key;
	phalcon_storage_btree_object *intern;

	phalcon_fetch_params(0, 1, 0, &key);

	intern = phalcon_storage_btree_object_from_obj(Z_OBJ_P(getThis()));

	if (phalcon_storage_btree_removes(&intern->db, Z_STRVAL_P(key)) != PHALCON_STORAGE_BTREE_OK){
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
