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

#include "mvc/orm.h"
#include "mvc/model.h"
#include "mvc/model/exception.h"
#include "diinterface.h"
#include "di/injectable.h"

#include "debug.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/hash.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\ORM
 *
 */
zend_class_entry *phalcon_mvc_orm_ce;

PHP_METHOD(Phalcon_Mvc_ORM, factory);
PHP_METHOD(Phalcon_Mvc_ORM, __construct);
PHP_METHOD(Phalcon_Mvc_ORM, new);
PHP_METHOD(Phalcon_Mvc_ORM, find);
PHP_METHOD(Phalcon_Mvc_ORM, findFirst);
PHP_METHOD(Phalcon_Mvc_ORM, group);
PHP_METHOD(Phalcon_Mvc_ORM, count);
PHP_METHOD(Phalcon_Mvc_ORM, sum);
PHP_METHOD(Phalcon_Mvc_ORM, maximum);
PHP_METHOD(Phalcon_Mvc_ORM, minimum);
PHP_METHOD(Phalcon_Mvc_ORM, average);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_orm_factory, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, tableName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, className, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, suffix, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_orm___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, tableName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, className, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, suffix, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_orm_find, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_orm_findfirst, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_orm_group, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_orm_count, 0, 0, 0)
	ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_orm_sum, 0, 0, 0)
	ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_orm_maximum, 0, 0, 0)
	ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_orm_minimum, 0, 0, 0)
	ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_orm_average, 0, 0, 0)
	ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_orm_method_entry[] = {
	PHP_ME(Phalcon_Mvc_ORM, factory, arginfo_phalcon_mvc_orm_factory, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_ORM, __construct, arginfo_phalcon_mvc_orm___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_ORM, new, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_ORM, find, arginfo_phalcon_mvc_orm_find, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_ORM, findFirst, arginfo_phalcon_mvc_orm_findfirst, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_ORM, group, arginfo_phalcon_mvc_orm_group, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_ORM, count, arginfo_phalcon_mvc_orm_count, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_ORM, sum, arginfo_phalcon_mvc_orm_sum, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_ORM, maximum, arginfo_phalcon_mvc_orm_maximum, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_ORM, minimum, arginfo_phalcon_mvc_orm_minimum, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_ORM, average, arginfo_phalcon_mvc_orm_average, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\ORM initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_ORM){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, ORM, mvc_orm, phalcon_di_injectable_ce, phalcon_mvc_orm_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_orm_ce, SL("_tableName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_orm_ce, SL("_className"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Loads an ORM and prepares it for manipulation
 *
 * <code>
 *
 * $robot = ORM::factory('robots')->new();
 * $robot->type = 'mechanical';
 * $robot->name = 'Astro Boy';
 * $robot->year = 1952;
 * if ($robot->save() == false) {
 *  foreach ($robot->getMessages() as $message) {
 *    echo $message;
 *  }
 * }
 *
 * </code>
 *
 * @param string $tableName
 * @param string $className
 * @param boolean $suffix
 * @return Phalcon\Mvc\ORM
 **/
PHP_METHOD(Phalcon_Mvc_ORM, factory)
{
	zval *table_name, *class_name = NULL, *suffix = NULL;

	phalcon_fetch_params(0, 1, 2, &table_name, &class_name, &suffix);

	if (!class_name) {
		class_name = &PHALCON_GLOBAL(z_null);
	}
	if (!suffix) {
		suffix = &PHALCON_GLOBAL(z_false);
	}
	object_init_ex(return_value, phalcon_mvc_orm_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", table_name, class_name, suffix);
}

/**
 * Phalcon\Mvc\ORM constructor
 *
 * @param string $tableName
 * @param string $className
 * @param boolean $suffix
 * @return Phalcon\Mvc\ORM
 */
PHP_METHOD(Phalcon_Mvc_ORM, __construct){

	zval *table_name, *class_name = NULL, *suffix = NULL;
	zval tmp = {};

	phalcon_fetch_params(1, 1, 1, &table_name, &class_name, &suffix);

	if (!class_name) {
		class_name = &PHALCON_GLOBAL(z_null);
	}
	if (!suffix) {
		suffix = &PHALCON_GLOBAL(z_false);
	}

	phalcon_update_property(getThis(), SL("_tableName"), table_name);
	if (PHALCON_IS_NOT_EMPTY(class_name)) {
		phalcon_update_property(getThis(), SL("_className"), class_name);
	} else {
		phalcon_camelize(&tmp, table_name);
		PHALCON_MM_ADD_ENTRY(&tmp);

		if (zend_is_true(suffix)) {
			PHALCON_SCONCAT_STR(&tmp, "Model");
			PHALCON_MM_ADD_ENTRY(&tmp);
		}
		phalcon_update_property(getThis(), SL("_className"), &tmp);

		class_name = &tmp;
	}
	PHALCON_MM_CALL_CE_STATIC(NULL, phalcon_mvc_model_ce, "register", class_name, table_name);

	RETURN_MM_THIS();
}

/**
 * Create a model
 *
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_ORM, new){

	zval class_name = {}, service_name = {}, manager = {};

	PHALCON_MM_INIT();

	phalcon_read_property(&class_name, getThis(), SL("_className"), PH_READONLY);

	ZVAL_STR(&service_name, IS(modelsManager));

	PHALCON_MM_CALL_METHOD(&manager, getThis(), "getresolveservice", &service_name);
	PHALCON_MM_ADD_ENTRY(&manager);

	PHALCON_MM_CALL_METHOD(return_value, &manager, "load", &class_name);

	RETURN_MM();
}

/**
 * Allows to query a set of records that match the specified conditions
 *
 * <code>
 *
 * $robots = ORM::factory('robots')->find()
 *
 * </code>
 *
 * @param array $params
 * @return  Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_ORM, find){

	zval *params = NULL, class_name = {};
	zend_class_entry *ce;

	phalcon_fetch_params(1, 0, 1, &params);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&class_name, getThis(), SL("_className"), PH_READONLY);

	ce = phalcon_class_exists_ex(&class_name, 0);
	PHALCON_VERIFY_CLASS_CE_EX(ce, phalcon_mvc_model_ce, phalcon_mvc_model_exception_ce);

	PHALCON_CALL_CE_STATIC(return_value, ce, "find", params);

	RETURN_MM();
}

/**
 * Allows to query the first record that match the specified conditions
 *
 * <code>
 *
 * $robot = ORM::factory('robots')->findFirst();
 * echo "The robot name is ", $robot->name;
 *
 * </code>
 *
 * @param array $params
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_ORM, findFirst){

	zval *params = NULL, class_name = {};
	zend_class_entry *ce;

	phalcon_fetch_params(1, 0, 1, &params);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&class_name, getThis(), SL("_className"), PH_READONLY);

	ce = phalcon_class_exists_ex(&class_name, 0);
	PHALCON_VERIFY_CLASS_CE_EX(ce, phalcon_mvc_model_ce, phalcon_mvc_model_exception_ce);

	PHALCON_CALL_CE_STATIC(return_value, ce, "findfirst", params);

	RETURN_MM();
}

/**
 * Generate a PHQL SELECT statement for an aggregate
 *
 *<code>
 *
 *	ORM::factory('robots')->group(['aggregators' => array(array('column' => 'id', 'aggregator' => 'sum'), 'sumatory' => array('column' => 'price', 'aggregator' => 'sum'))]);
 *
 *</code>
 *
 * @param array $params
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_ORM, group){

	zval *params = NULL, class_name = {};
	zend_class_entry *ce;

	phalcon_fetch_params(1, 0, 1, &params);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&class_name, getThis(), SL("_className"), PH_READONLY);

	ce = phalcon_class_exists_ex(&class_name, 0);
	PHALCON_VERIFY_CLASS_CE_EX(ce, phalcon_mvc_model_ce, phalcon_mvc_model_exception_ce);

	PHALCON_CALL_CE_STATIC(return_value, ce, "group", params);

	RETURN_MM();
}

/**
 * Allows to count how many records match the specified conditions
 *
 * <code>
 *
 * $number = Robots::count();
 *
 * </code>
 *
 * @param array $params
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_ORM, count){

	zval *params = NULL, class_name = {};
	zend_class_entry *ce;

	phalcon_fetch_params(1, 0, 1, &params);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&class_name, getThis(), SL("_className"), PH_READONLY);

	ce = phalcon_class_exists_ex(&class_name, 0);
	PHALCON_VERIFY_CLASS_CE_EX(ce, phalcon_mvc_model_ce, phalcon_mvc_model_exception_ce);

	PHALCON_CALL_CE_STATIC(return_value, ce, "count", params);

	RETURN_MM();
}

/**
 * Allows to calculate a summatory on a column that match the specified conditions
 *
 * <code>
 *
 * $sum = Robots::sum(array('column' => 'price'));
 *
 * </code>
 *
 * @param array $params
 * @return double
 */
PHP_METHOD(Phalcon_Mvc_ORM, sum){

	zval *params = NULL, class_name = {};
	zend_class_entry *ce;

	phalcon_fetch_params(1, 0, 1, &params);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&class_name, getThis(), SL("_className"), PH_READONLY);

	ce = phalcon_class_exists_ex(&class_name, 0);
	PHALCON_VERIFY_CLASS_CE_EX(ce, phalcon_mvc_model_ce, phalcon_mvc_model_exception_ce);

	PHALCON_CALL_CE_STATIC(return_value, ce, "sum", params);

	RETURN_MM();
}

/**
 * Allows to get the maximum value of a column that match the specified conditions
 *
 * <code>
 *
 * $id = Robots::maximum(array('column' => 'id'));
 *
 * </code>
 *
 * @param array $params
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_ORM, maximum){

	zval *params = NULL, class_name = {};
	zend_class_entry *ce;

	phalcon_fetch_params(1, 0, 1, &params);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&class_name, getThis(), SL("_className"), PH_READONLY);

	ce = phalcon_class_exists_ex(&class_name, 0);
	PHALCON_VERIFY_CLASS_CE_EX(ce, phalcon_mvc_model_ce, phalcon_mvc_model_exception_ce);

	PHALCON_CALL_CE_STATIC(return_value, ce, "maximum", params);

	RETURN_MM();
}

/**
 * Allows to get the minimum value of a column that match the specified conditions
 *
 * <code>
 *
 * $id = Robots::minimum(array('column' => 'id'));
 *
 * </code>
 *
 * @param array $params
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_ORM, minimum){

	zval *params = NULL, class_name = {};
	zend_class_entry *ce;

	phalcon_fetch_params(1, 0, 1, &params);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&class_name, getThis(), SL("_className"), PH_READONLY);

	ce = phalcon_class_exists_ex(&class_name, 0);
	PHALCON_VERIFY_CLASS_CE_EX(ce, phalcon_mvc_model_ce, phalcon_mvc_model_exception_ce);

	PHALCON_CALL_CE_STATIC(return_value, ce, "minimum", params);

	RETURN_MM();
}

/**
 * Allows to calculate the average value on a column matching the specified conditions
 *
 * <code>
 *
 * //What's the average price of robots?
 * $average = Robots::average(array('column' => 'price'));
 * echo "The average price is ", $average, "\n";
 *
 * //What's the average price of mechanical robots?
 * $average = Robots::average(array("type='mechanical'", 'column' => 'price'));
 * echo "The average price of mechanical robots is ", $average, "\n";
 *
 * </code>
 *
 * @param array $parameters
 * @return double
 */
PHP_METHOD(Phalcon_Mvc_ORM, average){

	zval *params = NULL, class_name = {};
	zend_class_entry *ce;

	phalcon_fetch_params(1, 0, 1, &params);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&class_name, getThis(), SL("_className"), PH_READONLY);

	ce = phalcon_class_exists_ex(&class_name, 0);
	PHALCON_VERIFY_CLASS_CE_EX(ce, phalcon_mvc_model_ce, phalcon_mvc_model_exception_ce);

	PHALCON_CALL_CE_STATIC(return_value, ce, "average", params);

	RETURN_MM();
}
