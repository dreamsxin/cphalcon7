
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

#include "storage/bloomfilter.h"
#include "storage/exception.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/bloomfilter.h"

/**
 * Phalcon\Storage\Bloomfilter
 *
 * This class defines bloomfilter entity and its description
 *
 */
zend_class_entry *phalcon_storage_bloomfilter_ce;

PHP_METHOD(Phalcon_Storage_Bloomfilter, __construct);
PHP_METHOD(Phalcon_Storage_Bloomfilter, add);
PHP_METHOD(Phalcon_Storage_Bloomfilter, check);
PHP_METHOD(Phalcon_Storage_Bloomfilter, reset);
PHP_METHOD(Phalcon_Storage_Bloomfilter, save);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_bloomfilter___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, seed, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, maxItems, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, falsePositive, IS_DOUBLE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_bloomfilter_add, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_bloomfilter_check, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_storage_bloomfilter_method_entry[] = {
	PHP_ME(Phalcon_Storage_Bloomfilter, __construct, arginfo_phalcon_storage_bloomfilter___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Storage_Bloomfilter, add, arginfo_phalcon_storage_bloomfilter_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Bloomfilter, check, arginfo_phalcon_storage_bloomfilter_check, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Bloomfilter, reset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Bloomfilter, save, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_storage_bloomfilter_object_handlers;
zend_object* phalcon_storage_bloomfilter_object_create_handler(zend_class_entry *ce)
{
	phalcon_storage_bloomfilter_object *intern = ecalloc(1, sizeof(phalcon_storage_bloomfilter_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_storage_bloomfilter_object_handlers;

	memset(&intern->bloomfilter, 0, sizeof(phalcon_bloomfilter));

	return &intern->std;
}

void phalcon_storage_bloomfilter_object_free_handler(zend_object *object)
{
	phalcon_storage_bloomfilter_object *intern = phalcon_storage_bloomfilter_object_from_obj(object);

	phalcon_bloomfilter_save(&intern->bloomfilter, Z_STRVAL(intern->filename));
	phalcon_bloomfilter_free(&intern->bloomfilter);
}

/**
 * Phalcon\Storage\Bloomfilter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_Bloomfilter){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Storage, Bloomfilter, storage_bloomfilter, phalcon_storage_bloomfilter_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Storage\Bloomfilter constructor
 */
PHP_METHOD(Phalcon_Storage_Bloomfilter, __construct){

	zval *filename, *_seed = NULL, *_max_items = NULL, *_false_positive = NULL;
	phalcon_storage_bloomfilter_object *intern;
	uint32_t seed = 0, max_items = 100000;
	double false_positive = 0.00001;

	phalcon_fetch_params(0, 1, 3, &filename, &_seed, &_max_items, &_false_positive);

	if (_seed && Z_TYPE_P(_seed) == IS_LONG) {
		seed = Z_LVAL_P(_seed);
	}

	if (_max_items && Z_TYPE_P(_max_items) == IS_LONG && Z_LVAL_P(_max_items) > 0) {
		max_items = Z_LVAL_P(_max_items);
	}

	if (_false_positive && Z_TYPE_P(_false_positive) == IS_DOUBLE && Z_DVAL_P(_false_positive) < 1 && Z_DVAL_P(_false_positive) > 0) {
		false_positive =  Z_DVAL_P(_false_positive);
	}

	intern = phalcon_storage_bloomfilter_object_from_obj(Z_OBJ_P(getThis()));

	PHALCON_ZVAL_DUP(&intern->filename, filename);

	if (phalcon_bloomfilter_init(&intern->bloomfilter, seed, max_items, false_positive) != 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Create bloom filter failed");
		return;
	}

	phalcon_bloomfilter_load(&intern->bloomfilter, Z_STRVAL_P(filename));
}

/**
 * Add value
 *
 * @param string value
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Bloomfilter, add){

	zval *value;
	phalcon_storage_bloomfilter_object *intern;
	int ret;

	phalcon_fetch_params(0, 1, 0, &value);

	intern = phalcon_storage_bloomfilter_object_from_obj(Z_OBJ_P(getThis()));
	ret = phalcon_bloomfilter_add(&intern->bloomfilter, Z_STRVAL_P(value), Z_STRLEN_P(value));
	if (ret < 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Add value error");
		return;
	}
	RETURN_TRUE;
}

/**
 * Check value
 *
 * @param string value
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Bloomfilter, check){

	zval *value;
	phalcon_storage_bloomfilter_object *intern;

	phalcon_fetch_params(0, 1, 0, &value);

	intern = phalcon_storage_bloomfilter_object_from_obj(Z_OBJ_P(getThis()));
	if (!phalcon_bloomfilter_check(&intern->bloomfilter, Z_STRVAL_P(value), Z_STRLEN_P(value))) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

/**
 * Reset
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Bloomfilter, reset){

	phalcon_storage_bloomfilter_object *intern;

	intern = phalcon_storage_bloomfilter_object_from_obj(Z_OBJ_P(getThis()));
	if (!phalcon_bloomfilter_reset(&intern->bloomfilter)) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

/**
 * Save data to file
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Bloomfilter, save){

	phalcon_storage_bloomfilter_object *intern;

	intern = phalcon_storage_bloomfilter_object_from_obj(Z_OBJ_P(getThis()));
	if (!phalcon_bloomfilter_save(&intern->bloomfilter, Z_STRVAL(intern->filename))) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
