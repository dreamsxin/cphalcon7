
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

#include "storage/bloomfilter/counting.h"
#include "storage/exception.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"

#ifdef ZEND_ENABLE_ZVAL_LONG64

/**
 * Phalcon\Storage\Bloomfilter\Counting
 *
 */
zend_class_entry *phalcon_storage_bloomfilter_counting_ce;

PHP_METHOD(Phalcon_Storage_Bloomfilter_Counting, __construct);
PHP_METHOD(Phalcon_Storage_Bloomfilter_Counting, add);
PHP_METHOD(Phalcon_Storage_Bloomfilter_Counting, remove);
PHP_METHOD(Phalcon_Storage_Bloomfilter_Counting, check);
PHP_METHOD(Phalcon_Storage_Bloomfilter_Counting, save);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_bloomfilter_counting___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, seed, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, maxItems, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, falsePositive, IS_DOUBLE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_bloomfilter_counting_add, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_bloomfilter_counting_remove, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_bloomfilter_counting_check, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_storage_bloomfilter_counting_method_entry[] = {
	PHP_ME(Phalcon_Storage_Bloomfilter_Counting, __construct, arginfo_phalcon_storage_bloomfilter_counting___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Storage_Bloomfilter_Counting, add, arginfo_phalcon_storage_bloomfilter_counting_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Bloomfilter_Counting, remove, arginfo_phalcon_storage_bloomfilter_counting_remove, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Bloomfilter_Counting, check, arginfo_phalcon_storage_bloomfilter_counting_check, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_storage_bloomfilter_counting_object_handlers;
zend_object* phalcon_storage_bloomfilter_counting_object_create_handler(zend_class_entry *ce)
{
	phalcon_storage_bloomfilter_counting_object *intern = ecalloc(1, sizeof(phalcon_storage_bloomfilter_counting_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_storage_bloomfilter_counting_object_handlers;

	return &intern->std;
}

void phalcon_storage_bloomfilter_counting_object_free_handler(zend_object *object)
{
	phalcon_storage_bloomfilter_counting_object *intern = phalcon_storage_bloomfilter_counting_object_from_obj(object);

	if (intern->bloomfilter) {
		free_counting_bloom(intern->bloomfilter);
	}
}

/**
 * Phalcon\Storage\Bloomfilter\Counting initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_Bloomfilter_Counting){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Storage\\Bloomfilter, Counting, storage_bloomfilter_counting, phalcon_storage_bloomfilter_counting_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Storage\Bloomfilter\Counting constructor
 */
PHP_METHOD(Phalcon_Storage_Bloomfilter_Counting, __construct){

	zval *filename, *_capacity = NULL, *_error_rate = NULL;
	phalcon_storage_bloomfilter_counting_object *intern;
	unsigned int capacity = 100000;
	double error_rate = 0.05;

	phalcon_fetch_params(0, 1, 4, &filename, &_capacity, &_error_rate);

	if (_capacity && Z_TYPE_P(_capacity) == IS_LONG && Z_LVAL_P(_capacity) > capacity) {
		capacity = Z_LVAL_P(_capacity);
	}

	if (_error_rate && Z_TYPE_P(_error_rate) == IS_LONG && Z_LVAL_P(_error_rate) > 0 &&  Z_LVAL_P(_error_rate) < error_rate) {
		error_rate = Z_LVAL_P(_error_rate);
	}

	intern = phalcon_storage_bloomfilter_counting_object_from_obj(Z_OBJ_P(getThis()));

	PHALCON_ZVAL_DUP(&intern->filename, filename);

	intern->bloomfilter = autocreate_counting_bloom_from_file(capacity, error_rate, Z_STRVAL_P(filename));
	if (!intern->bloomfilter) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Create counting bloom filter failed");
		return;
	}
}

/**
 * Add value
 *
 * @param string value
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Bloomfilter_Counting, add){

	zval *value;
	phalcon_storage_bloomfilter_counting_object *intern;
	int ret;

	phalcon_fetch_params(0, 1, 0, &value);

	intern = phalcon_storage_bloomfilter_counting_object_from_obj(Z_OBJ_P(getThis()));
	ret = counting_bloom_add(intern->bloomfilter, Z_STRVAL_P(value), Z_STRLEN_P(value));
	if (ret < 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Add value error");
		return;
	}
	RETURN_TRUE;
}

/**
 * Remove value
 *
 * @param string value
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Bloomfilter_Counting, remove){

	zval *value;
	phalcon_storage_bloomfilter_counting_object *intern;
	int ret;

	phalcon_fetch_params(0, 1, 0, &value);

	intern = phalcon_storage_bloomfilter_counting_object_from_obj(Z_OBJ_P(getThis()));
	ret = counting_bloom_remove(intern->bloomfilter, Z_STRVAL_P(value), Z_STRLEN_P(value));
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
PHP_METHOD(Phalcon_Storage_Bloomfilter_Counting, check){

	zval *value;
	phalcon_storage_bloomfilter_counting_object *intern;

	phalcon_fetch_params(0, 1, 0, &value);

	intern = phalcon_storage_bloomfilter_counting_object_from_obj(Z_OBJ_P(getThis()));
	if (!counting_bloom_check(intern->bloomfilter, Z_STRVAL_P(value), Z_STRLEN_P(value))) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

#endif