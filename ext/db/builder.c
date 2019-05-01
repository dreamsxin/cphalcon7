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

#include "db/builder.h"
#include "db/builderinterface.h"
#include "db/builder/select.h"
#include "db/builder/update.h"
#include "db/builder/insert.h"
#include "db/builder/delete.h"
#include "di/injectable.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/file.h"
#include "kernel/debug.h"

/**
 * Phalcon\Db\Builder
 *
 * Helps to create queries using an OO interface
 */
zend_class_entry *phalcon_db_builder_ce;

PHP_METHOD(Phalcon_Db_Builder, select);
PHP_METHOD(Phalcon_Db_Builder, update);
PHP_METHOD(Phalcon_Db_Builder, insert);
PHP_METHOD(Phalcon_Db_Builder, delete);
PHP_METHOD(Phalcon_Db_Builder, setBindParams);
PHP_METHOD(Phalcon_Db_Builder, getBindParams);
PHP_METHOD(Phalcon_Db_Builder, setBindTypes);
PHP_METHOD(Phalcon_Db_Builder, getBindTypes);
PHP_METHOD(Phalcon_Db_Builder, execute);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_select, 0, 0, 1)
	ZEND_ARG_INFO(0, tables)
	ZEND_ARG_INFO(0, db)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_update, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	ZEND_ARG_INFO(0, db)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_insert, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	ZEND_ARG_INFO(0, db)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_delete, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	ZEND_ARG_INFO(0, db)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_setbindparams, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, bindparams, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_setbindtypes, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, bindtypes, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_builder_method_entry[] = {
	PHP_ME(Phalcon_Db_Builder, select, arginfo_phalcon_db_builder_select, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Db_Builder, update, arginfo_phalcon_db_builder_update, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Db_Builder, insert, arginfo_phalcon_db_builder_insert, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Db_Builder, delete, arginfo_phalcon_db_builder_delete, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Db_Builder, setBindParams, arginfo_phalcon_db_builder_setbindparams, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder, getBindParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder, setBindTypes, arginfo_phalcon_db_builder_setbindtypes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder, getBindTypes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder, execute, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Builder initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Builder){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db, Builder, db_builder, phalcon_di_injectable_ce, phalcon_db_builder_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_string(phalcon_db_builder_ce, SL("_defaultConnectionService"), "db", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_builder_ce, SL("_definition"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_builder_ce, SL("_bindParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_builder_ce, SL("_bindTypes"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_builder_ce, 1, phalcon_db_builderinterface_ce);

	return SUCCESS;
}

/**
 * Create a select builder
 *
 *<code>
 * $resultset = Phalcon\Db\Builder::select('robots')
 * 	  ->join('robots_parts', 'robots.id = robots_parts.robots_id')
 * 	  ->where('robots.id = 1')
 * 	  ->limit(20)
 * 	  ->orderBy('robots.name')
 *    ->execute();
 *</code>
 *
 * @return Phalcon\Db\Builder\Select
 */
PHP_METHOD(Phalcon_Db_Builder, select){
	zval *table, *db = NULL;

	phalcon_fetch_params(0, 1, 1, &table, &db);

	if (!db) {
		db = &PHALCON_GLOBAL(z_null);
	}

	object_init_ex(return_value, phalcon_db_builder_select_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", table, db);
}

/**
 * Create a update builder
 *
 *<code>
 * $ret = Phalcon\Db\Builder::update('robots')
 * 	  ->set(['name' => 'test'])
 * 	  ->where('id = 1')
 *    ->execute();
 *</code>
 *
 * @return Phalcon\Db\Builder\Update
 */
PHP_METHOD(Phalcon_Db_Builder, update){
	zval *table, *db = NULL;

	phalcon_fetch_params(0, 1, 1, &table, &db);

	if (!db) {
		db = &PHALCON_GLOBAL(z_null);
	}

	object_init_ex(return_value, phalcon_db_builder_update_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", table, db);
}

/**
 * Create a insert builder
 *
 *<code>
 * $ret = Phalcon\Db\Builder::insert('robots')
 * 	  ->values(['name' => 'test'])
 *    ->execute();
 *</code>
 *
 * @return Phalcon\Db\Builder\Insert
 */
PHP_METHOD(Phalcon_Db_Builder, insert){
	zval *table = NULL, *db = NULL;

	phalcon_fetch_params(0, 1, 1, &table, &db);

	if (!db) {
		db = &PHALCON_GLOBAL(z_null);
	}

	object_init_ex(return_value, phalcon_db_builder_insert_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", table, db);
}

/**
 * Create a delete builder
 *
 *<code>
 * $ret = Phalcon\Db\Builder::delete('robots')
 * 	  ->where('id = 1')
 *    ->execute();
 *</code>
 *
 * @return Phalcon\Db\Builder\Delete
 */
PHP_METHOD(Phalcon_Db_Builder, delete){
	zval *table, *db = NULL;

	phalcon_fetch_params(0, 1, 1, &table, &db);

	if (!db) {
		db = &PHALCON_GLOBAL(z_null);
	}

	object_init_ex(return_value, phalcon_db_builder_delete_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", table, db);
}

/**
 * Sets the bind parameters
 *
 * @param array $bindParams
 * @return Phalcon\Db\Builder
 */
PHP_METHOD(Phalcon_Db_Builder, setBindParams){

	zval *bind_params, *merge = NULL, current_bind_params = {}, merged_params = {};

	phalcon_fetch_params(0, 1, 1, &bind_params, &merge);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_true);
	}

	if (zend_is_true(merge)) {
		if (Z_TYPE_P(bind_params) == IS_ARRAY) {
			phalcon_read_property(&current_bind_params, getThis(), SL("_bindParams"), PH_NOISY|PH_READONLY);
			if (Z_TYPE(current_bind_params) == IS_ARRAY) {
				phalcon_add_function(&merged_params, bind_params, &current_bind_params);
			} else {
				ZVAL_COPY(&merged_params, bind_params);
			}

			phalcon_update_property(getThis(), SL("_bindParams"), &merged_params);
			zval_ptr_dtor(&merged_params);
		}
	} else {
		phalcon_update_property(getThis(), SL("_bindParams"), bind_params);
	}
}

/**
 * Gets the bind parameters
 *
 * @return Phalcon\Db\Builder
 */
PHP_METHOD(Phalcon_Db_Builder, getBindParams){


	RETURN_MEMBER(getThis(), "_bindParams");
}

/**
 * Sets the bind types
 *
 * @param array $bindTypes
 * @return Phalcon\Db\Builder
 */
PHP_METHOD(Phalcon_Db_Builder, setBindTypes){

	zval *bind_types, *merge = NULL, current_bind_types = {}, merged_types = {};

	phalcon_fetch_params(0, 1, 1, &bind_types, &merge);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_true);
	}

	if (zend_is_true(merge)) {
		if (Z_TYPE_P(bind_types) == IS_ARRAY) {
			phalcon_read_property(&current_bind_types, getThis(), SL("_bindTypes"), PH_NOISY|PH_READONLY);
			if (Z_TYPE(current_bind_types) == IS_ARRAY) {
				phalcon_add_function(&merged_types, bind_types, &current_bind_types);
			} else {
				ZVAL_COPY(&merged_types, bind_types);
			}

			phalcon_update_property(getThis(), SL("_bindTypes"), &merged_types);
			zval_ptr_dtor(&merged_types);
		}
	} else {
		phalcon_update_property(getThis(), SL("_bindTypes"), bind_types);
	}
}

/**
 * Gets the bind types
 *
 * @return Phalcon\Db\Builder
 */
PHP_METHOD(Phalcon_Db_Builder, getBindTypes){


	RETURN_MEMBER(getThis(), "_bindTypes");
}

/**
 * Execute query
 *
 * @return Phalcon\Db\ResultInterface|boolean
 */
PHP_METHOD(Phalcon_Db_Builder, execute){

	PHALCON_CALL_METHOD(return_value, getThis(), "_execute");
}
