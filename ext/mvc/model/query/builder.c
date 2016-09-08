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

#include "mvc/model/query/builder.h"
#include "mvc/model/query/builderinterface.h"
#include "mvc/model/query/exception.h"
#include "mvc/model/query/builder/select.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/model/metadata/memory.h"
#include "mvc/model/query.h"
#include "mvc/model/query/scanner.h"
#include "di.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "db/rawvalue.h"

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
#include "kernel/hash.h"
#include "kernel/framework/orm.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Query\Builder
 *
 * Helps to create PHQL queries using an OO interface
 *
 *<code>
 *$resultset = Phalcon\Mvc\Model\Query\Builder::create(Phalcon\Mvc\Model\Query::TYPE_SELECT)
 *   ->from('Robots')
 *   ->join('RobotsParts')
 *   ->limit(20)
 *   ->orderBy('Robots.name')
 *   ->getQuery()
 *   ->execute();
 *</code>
 */
zend_class_entry *phalcon_mvc_model_query_builder_ce;

PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, create);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createSelectBuilder);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createInsertBuilder);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createUpdateBuilder);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createDeleteBuilder);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getType);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getConditions);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, where);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, andWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, orWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, betweenWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, notBetweenWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, inWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, notInWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, compile);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getPhql);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getQuery);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_create, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_where, 0, 0, 1)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, bindParams)
	ZEND_ARG_INFO(0, bindTypes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_andwhere, 0, 0, 1)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, bindParams)
	ZEND_ARG_INFO(0, bindTypes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_orwhere, 0, 0, 1)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, bindParams)
	ZEND_ARG_INFO(0, bindTypes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_betweenwhere, 0, 0, 3)
	ZEND_ARG_INFO(0, expr)
	ZEND_ARG_INFO(0, minimum)
	ZEND_ARG_INFO(0, maximum)
	ZEND_ARG_INFO(0, useOrWhere)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_notbetweenwhere, 0, 0, 3)
	ZEND_ARG_INFO(0, expr)
	ZEND_ARG_INFO(0, minimum)
	ZEND_ARG_INFO(0, maximum)
	ZEND_ARG_INFO(0, useOrWhere)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_inwhere, 0, 0, 2)
	ZEND_ARG_INFO(0, expr)
	ZEND_ARG_INFO(0, values)
	ZEND_ARG_INFO(0, useOrWhere)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_notinwhere, 0, 0, 2)
	ZEND_ARG_INFO(0, expr)
	ZEND_ARG_INFO(0, values)
	ZEND_ARG_INFO(0, useOrWhere)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_query_builder_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, create, arginfo_phalcon_mvc_model_query_builder_create, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, createSelectBuilder, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, createInsertBuilder, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, createUpdateBuilder, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, createDeleteBuilder, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getConditions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, where, arginfo_phalcon_mvc_model_query_builder_where, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, andWhere, arginfo_phalcon_mvc_model_query_builder_andwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, orWhere, arginfo_phalcon_mvc_model_query_builder_orwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, betweenWhere, arginfo_phalcon_mvc_model_query_builder_betweenwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, notBetweenWhere, arginfo_phalcon_mvc_model_query_builder_notbetweenwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, inWhere, arginfo_phalcon_mvc_model_query_builder_inwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, notInWhere, arginfo_phalcon_mvc_model_query_builder_notinwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getWhere, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, compile, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getPhql, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getQuery, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Query\Builder initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Query_Builder){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Query, Builder, mvc_model_query_builder, phalcon_di_injectable_ce, phalcon_mvc_model_query_builder_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_phql"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_conditions"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_bindParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_bindTypes"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_query_builder_ce, SL("_hiddenParamNumber"), 0, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_query_builder_ce, 1, phalcon_mvc_model_query_builderinterface_ce);

	return SUCCESS;
}

/**
 * Create a new Query Builder of the given type.
 *
 *<code>
 *	Phalcon\Mvc\Model\Query\Builder::create(Phalcon\Mvc\Model\Query::TYPE_SELECT);
 *</code>
 *
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, create){

	zval *type, *params = NULL, *_di = NULL, di = {}, service_name = {}, has = {}, exception_message = {};

	phalcon_fetch_params(0, 1, 2, &type, &params, &_di);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	if (!_di) {
		_di = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(_di) == IS_OBJECT) {
		PHALCON_VERIFY_INTERFACEW(_di, phalcon_diinterface_ce);
		PHALCON_CPY_WRT(&di, _di);
	} else {
		PHALCON_CALL_CE_STATICW(&di, phalcon_di_ce, "getdefault", _di);
	}

	switch (phalcon_get_intval(type)) {
		case PHQL_T_SELECT:
			PHALCON_STR(&service_name, ISV(modelsQueryBuilderForSelect));
			PHALCON_CALL_METHODW(&has, &di, "has", &service_name);
			if (zend_is_true(&has)) {
				PHALCON_CALL_METHODW(return_value, &di, "get", &service_name);
			} else {
				object_init_ex(return_value, phalcon_mvc_model_query_builder_select_ce);
				PHALCON_CALL_METHODW(NULL, return_value, "__construct", params);
			}
			break;

		case PHQL_T_INSERT:
			PHALCON_STR(&service_name, ISV(modelsQueryBuilderForInsert));
			PHALCON_CALL_METHODW(&has, &di, "has", &service_name);
			if (zend_is_true(&has)) {
				PHALCON_CALL_METHODW(return_value, &di, "get", &service_name);
			} else {
				//object_init_ex(return_value, phalcon_mvc_model_query_builder_insert_ce);
			}
			break;

		case PHQL_T_UPDATE:
			PHALCON_STR(&service_name, ISV(modelsQueryBuilderForUpdate));
			PHALCON_CALL_METHODW(&has, &di, "has", &service_name);
			if (zend_is_true(&has)) {
				PHALCON_CALL_METHODW(return_value, &di, "get", &service_name);
			} else {
				//object_init_ex(return_value, phalcon_mvc_model_query_builder_delete_ce);
			}
			break;

		case PHQL_T_DELETE:
			PHALCON_STR(&service_name, ISV(modelsQueryBuilderForDelete));
			PHALCON_CALL_METHODW(&has, &di, "has", &service_name);
			if (zend_is_true(&has)) {
				PHALCON_CALL_METHODW(return_value, &di, "get", &service_name);
			} else {
				//object_init_ex(return_value, phalcon_mvc_model_query_builder_delete_ce);
			}
			break;

		default:
			PHALCON_CONCAT_SV(&exception_message, "Not found builder: ", type);
			PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_query_exception_ce, &exception_message);
			return;
	}

	PHALCON_VERIFY_INTERFACEW(return_value, phalcon_mvc_model_query_builderinterface_ce);
}

/**
 * Create a new Query Builder for Select 
 *
 *
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createSelectBuilder){

	zval *params = NULL, *_di = NULL, di = {}, service_name = {};

	phalcon_fetch_params(0, 0, 2, &params, &_di);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	if (!_di) {
		_di = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(_di) == IS_OBJECT) {
		PHALCON_VERIFY_INTERFACEW(_di, phalcon_diinterface_ce);
		PHALCON_CPY_WRT(&di, _di);
	} else {
		PHALCON_CALL_CE_STATICW(&di, phalcon_di_ce, "getdefault", _di);
	}

	PHALCON_STR(&service_name, ISV(modelsQueryBuilderForSelect));
	PHALCON_CALL_METHODW(return_value, &di, "get", &service_name, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		object_init_ex(return_value, phalcon_mvc_model_query_builder_select_ce);
		PHALCON_CALL_METHODW(NULL, return_value, "__construct", params);
	}

	PHALCON_VERIFY_INTERFACEW(return_value, phalcon_mvc_model_query_builderinterface_ce);
}

/**
 * Create a new Query Builder for Insert
 *
 *
 * @return Phalcon\Mvc\Model\Query\Builder\Insert
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createInsertBuilder){

}

/**
 * Create a new Query Builder for Update
 *
 *
 * @return Phalcon\Mvc\Model\Query\Builder\Update
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createUpdateBuilder){

}

/**
 * Create a new Query Builder for Delete
 *
 *
 * @return Phalcon\Mvc\Model\Query\Builder\Delete
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createDeleteBuilder){

}

/**
 * Gets the type of PHQL queries
 *
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getType){

	RETURN_MEMBER(getThis(), "_type");
}

/**
 * Returns the conditions, If the conditions is a single numeric field. We internally create a condition
 * using the related primary key
 *
 *<code>
 *	$builder->getConditions();
 *</code>
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getConditions){

	zval conditions = {}, dependency_injector = {}, models = {}, number_models = {}, invalid_condition = {}, model = {}, service_name = {}, has = {}, meta_data = {};
	zval model_instance = {}, primary_keys = {}, first_primary_key = {}, column_map = {}, attribute_field = {}, exception_message = {};
	zend_class_entry *ce0;

	phalcon_return_property(&conditions, getThis(), SL("_conditions"));

	if (phalcon_is_numeric(&conditions)) {
		PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi", &PHALCON_GLOBAL(z_true));

		phalcon_read_property(&models, getThis(), SL("_models"), PH_NOISY);
		if (Z_TYPE(models) == IS_ARRAY) { 
			if (!phalcon_fast_count_ev(&models)) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_query_exception_ce, "At least one model is required to build the query");
				return;
			}
		} else if (!zend_is_true(&models)) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_query_exception_ce, "At least one model is required to build the query");
			return;
		}

		/** 
		 * If the conditions is a single numeric field. We internally create a condition
		 * using the related primary key
		 */
		if (Z_TYPE(models) == IS_ARRAY) {
			phalcon_fast_count(&number_models, &models);
			is_smaller_function(&invalid_condition, &PHALCON_GLOBAL(z_one), &number_models);
			if (PHALCON_IS_TRUE(&invalid_condition)) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_query_exception_ce, "Cannot build the query. Invalid condition");
				return;
			}

			phalcon_array_fetch_long(&model, &models, 0, PH_NOISY);
		} else {
			PHALCON_CPY_WRT(&model, &models);
		}

		PHALCON_STR(&service_name, ISV(modelsMetadata));

		PHALCON_CALL_METHODW(&has, &dependency_injector, "has", &service_name);
		if (zend_is_true(&has)) {
			/** 
			 * Get the models metadata service to obtain the column names, column map and
			 * primary key
			 */
			PHALCON_CALL_METHODW(&meta_data, &dependency_injector, "getshared", &service_name);
			PHALCON_VERIFY_INTERFACEW(&meta_data, phalcon_mvc_model_metadatainterface_ce);
		} else {
			object_init_ex(&meta_data, phalcon_mvc_model_metadata_memory_ce);
		}

		ce0 = phalcon_fetch_class(&model, ZEND_FETCH_CLASS_DEFAULT);

		object_init_ex(&model_instance, ce0);
		if (phalcon_has_constructor(&model_instance)) {
			PHALCON_CALL_METHODW(NULL, &model_instance, "__construct", &dependency_injector);
		}

		PHALCON_CALL_METHODW(&primary_keys, &meta_data, "getprimarykeyattributes", &model_instance);
		if (phalcon_fast_count_ev(&primary_keys)) {
			if (phalcon_array_isset_fetch_long(&first_primary_key, &primary_keys, 0)) {
				/** 
				 * The PHQL contains the renamed columns if available
				 */
				if (PHALCON_GLOBAL(orm).column_renaming) {
					PHALCON_CALL_METHODW(&column_map, &meta_data, "getcolumnmap", &model_instance);
				}

				if (Z_TYPE(column_map) == IS_ARRAY) { 
					if (!phalcon_array_isset_fetch(&attribute_field, &column_map, &first_primary_key, 0)) {
						PHALCON_CONCAT_SVS(&exception_message, "Column '", &first_primary_key, "\" isn't part of the column map");
						PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_query_exception_ce, &exception_message);
						return;
					}
				} else {
					PHALCON_CPY_WRT(&attribute_field, &first_primary_key);
				}

				PHALCON_CONCAT_SVSVSV(return_value, "[", &model, "].[", &attribute_field, "] = ", &conditions);
				phalcon_update_property_zval(getThis(), SL("_conditions"), return_value);
				return;
			}
		}

		/** 
		 * A primary key is mandatory in these cases
		 */
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_query_exception_ce, "Source related to this model does not have a primary key defined");
		return;
	}

	RETURN_CTORW(&conditions);
}

/**
 * Sets the query conditions
 *
 *<code>
 *	$builder->where('name = "Peter"');
 *	$builder->where('name = :name: AND id > :id:', array('name' => 'Peter', 'id' => 100));
 *</code>
 *
 * @param string $conditions
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, where){

	zval *conditions, *bind_params = NULL, *bind_types = NULL;

	phalcon_fetch_params(0, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_property_zval(getThis(), SL("_conditions"), conditions);

	/** 
	 * Override the bind params and bind types to the current ones
	 */
	phalcon_update_property_zval(getThis(), SL("_bindParams"), bind_params);
	phalcon_update_property_zval(getThis(), SL("_bindTypes"), bind_types);

	RETURN_THISW();
}

/**
 * Appends a condition to the current conditions using a AND operator
 *
 *<code>
 *	$builder->andWhere('name = "Peter"');
 *	$builder->andWhere('name = :name: AND id > :id:', array('name' => 'Peter', 'id' => 100));
 *</code>
 *
 * @param string $conditions
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, andWhere){

	zval *conditions, *bind_params = NULL, *bind_types = NULL, current_conditions = {}, new_conditions = {}, current_bind_params = {}, merged_params = {};
	zval current_bind_types = {}, merged_types = {};

	phalcon_fetch_params(0, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_SELFW(&current_conditions, "getConditions");

	/** 
	 * Nest the condition to current ones or set as unique
	 */
	if (zend_is_true(&current_conditions)) {
		PHALCON_CONCAT_SVSVS(&new_conditions, "(", &current_conditions, ") AND (", conditions, ")");
	} else {
		PHALCON_CPY_WRT(&new_conditions, conditions);
	}

	phalcon_update_property_zval(getThis(), SL("_conditions"), &new_conditions);

	/** 
	 * Merge the bind params to the current ones
	 */
	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		phalcon_read_property(&current_bind_params, getThis(), SL("_bindParams"), PH_NOISY);
		if (Z_TYPE(current_bind_params) == IS_ARRAY) { 
			phalcon_add_function(&merged_params, bind_params, &current_bind_params);
		} else {
			PHALCON_CPY_WRT(&merged_params, bind_params);
		}

		phalcon_update_property_zval(getThis(), SL("_bindParams"), &merged_params);
	}

	/** 
	 * Merge the bind types to the current ones
	 */
	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		phalcon_read_property(&current_bind_types, getThis(), SL("_bindTypes"), PH_NOISY);
		if (Z_TYPE(current_bind_types) == IS_ARRAY) { 
			phalcon_add_function(&merged_params, bind_types, &current_bind_types);
		} else {
			PHALCON_CPY_WRT(&merged_types, bind_types);
		}

		phalcon_update_property_zval(getThis(), SL("_bindTypes"), &merged_types);
	}

	RETURN_THISW();
}

/**
 * Appends a condition to the current conditions using a OR operator
 *
 *<code>
 *	$builder->orWhere('name = "Peter"');
 *	$builder->orWhere('name = :name: AND id > :id:', array('name' => 'Peter', 'id' => 100));
 *</code>
 *
 * @param string $conditions
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, orWhere){

	zval *conditions, *bind_params = NULL, *bind_types = NULL, current_conditions = {}, new_conditions = {}, current_bind_params = {}, merged_params = {};
	zval current_bind_types = {}, merged_types = {};

	phalcon_fetch_params(0, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_SELFW(&current_conditions, "getConditions");

	/** 
	 * Nest the condition to current ones or set as unique
	 */
	if (zend_is_true(&current_conditions)) {
		PHALCON_CONCAT_SVSVS(&new_conditions, "(", &current_conditions, ") OR (", conditions, ")");
	} else {
		PHALCON_CPY_WRT(&new_conditions, conditions);
	}

	phalcon_update_property_zval(getThis(), SL("_conditions"), &new_conditions);

	/** 
	 * Merge the bind params to the current ones
	 */
	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		phalcon_read_property(&current_bind_params, getThis(), SL("_bindParams"), PH_NOISY);
		if (Z_TYPE(current_bind_params) == IS_ARRAY) { 
			phalcon_add_function(&merged_params, bind_params, &current_bind_params);
		} else {
			PHALCON_CPY_WRT(&merged_params, bind_params);
		}

		phalcon_update_property_zval(getThis(), SL("_bindParams"), &merged_params);
	}

	/** 
	 * Merge the bind types to the current ones
	 */
	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		phalcon_read_property(&current_bind_types, getThis(), SL("_bindTypes"), PH_NOISY);
		if (Z_TYPE(current_bind_types) == IS_ARRAY) {
			phalcon_add_function(&merged_types, bind_types, &current_bind_types);
		} else {
			PHALCON_CPY_WRT(&merged_types, bind_types);
		}

		phalcon_update_property_zval(getThis(), SL("_bindTypes"), &merged_types);
	}

	RETURN_THISW();
}

/**
 * Appends a BETWEEN condition to the current conditions
 *
 *<code>
 *	$builder->betweenWhere('price', 100.25, 200.50);
 *</code>
 *
 * @param string $expr
 * @param mixed $minimum
 * @param mixed $maximum
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, betweenWhere){

	zval *expr, *minimum, *maximum, *use_orwhere = NULL, hidden_param = {}, next_hidden_param = {}, minimum_key = {}, maximum_key = {}, conditions = {}, bind_params = {};

	phalcon_fetch_params(0, 3, 1, &expr, &minimum, &maximum, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_NOISY);
	phalcon_add_function(&next_hidden_param, &hidden_param, &PHALCON_GLOBAL(z_one));

	/** 
	 * Minimum key with auto bind-params
	 */
	PHALCON_CONCAT_SV(&minimum_key, "phb", &hidden_param);

	/** 
	 * Maximum key with auto bind-params
	 */
	PHALCON_CONCAT_SV(&maximum_key, "phb", &next_hidden_param);

	/** 
	 * Create a standard BETWEEN condition with bind params
	 */
	PHALCON_CONCAT_VSVSVS(&conditions, expr, " BETWEEN :", &minimum_key, ": AND :", &maximum_key, ":");

	array_init_size(&bind_params, 2);
	phalcon_array_update_zval(&bind_params, &minimum_key, minimum, PH_COPY);
	phalcon_array_update_zval(&bind_params, &maximum_key, maximum, PH_COPY);

	/** 
	 * Append the BETWEEN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHODW(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}

	phalcon_increment(&next_hidden_param);
	phalcon_update_property_zval(getThis(), SL("_hiddenParamNumber"), &next_hidden_param);
	RETURN_THISW();
}

/**
 * Appends a NOT BETWEEN condition to the current conditions
 *
 *<code>
 *	$builder->notBetweenWhere('price', 100.25, 200.50);
 *</code>
 *
 * @param string $expr
 * @param mixed $minimum
 * @param mixed $maximum
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, notBetweenWhere){

	zval *expr, *minimum, *maximum, *use_orwhere = NULL, hidden_param = {}, next_hidden_param = {}, minimum_key = {}, maximum_key = {}, conditions = {}, bind_params = {};

	phalcon_fetch_params(0, 3, 1, &expr, &minimum, &maximum, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_NOISY);
	phalcon_add_function(&next_hidden_param, &hidden_param, &PHALCON_GLOBAL(z_one));

	/** 
	 * Minimum key with auto bind-params
	 */
	PHALCON_CONCAT_SV(&minimum_key, "phb", &hidden_param);

	/** 
	 * Maximum key with auto bind-params
	 */
	PHALCON_CONCAT_SV(&maximum_key, "phb", &next_hidden_param);

	/** 
	 * Create a standard BETWEEN condition with bind params
	 */
	PHALCON_CONCAT_VSVSVS(&conditions, expr, " NOT BETWEEN :", &minimum_key, ": AND :", &maximum_key, ":");

	array_init_size(&bind_params, 2);
	phalcon_array_update_zval(&bind_params, &minimum_key, minimum, PH_COPY);
	phalcon_array_update_zval(&bind_params, &maximum_key, maximum, PH_COPY);

	/** 
	 * Append the BETWEEN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHODW(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}

	phalcon_increment(&next_hidden_param);
	phalcon_update_property_zval(getThis(), SL("_hiddenParamNumber"), &next_hidden_param);
	RETURN_THISW();
}

/**
 * Appends an IN condition to the current conditions
 *
 *<code>
 *	$builder->inWhere('id', [1, 2, 3]);
 *</code>
 *
 * @param string $expr
 * @param array $values
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, inWhere){

	zval *expr, *values, *use_orwhere = NULL, hidden_param = {}, bind_params = {}, bind_keys = {}, *value, joined_keys = {}, conditions = {};

	phalcon_fetch_params(0, 2, 1, &expr, &values, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(values) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_query_exception_ce, "Values must be an array");
		return;
	}

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_READONLY);

	array_init(&bind_params);
	array_init(&bind_keys);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(values), value) {
		zval key = {}, query_key = {};
		/** 
		 * Key with auto bind-params
		 */
		PHALCON_CONCAT_SV(&key, "phi", &hidden_param);

		PHALCON_CONCAT_SVS(&query_key, ":", &key, ":");
		phalcon_array_append(&bind_keys, &query_key, PH_COPY);
		phalcon_array_update_zval(&bind_params, &key, value, PH_COPY);
		phalcon_increment(&hidden_param);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&joined_keys, SL(", "), &bind_keys);

	/** 
	 * Create a standard IN condition with bind params
	 */
	PHALCON_CONCAT_VSVS(&conditions, expr, " IN (", &joined_keys, ")");

	/** 
	 * Append the IN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHODW(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}
	phalcon_update_property_zval(getThis(), SL("_hiddenParamNumber"), &hidden_param);

	RETURN_THISW();
}

/**
 * Appends a NOT IN condition to the current conditions
 *
 *<code>
 *	$builder->notInWhere('id', [1, 2, 3]);
 *</code>
 *
 * @param string $expr
 * @param array $values
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, notInWhere){

	zval *expr, *values, *use_orwhere = NULL, hidden_param = {}, bind_params = {}, bind_keys = {}, *value, joined_keys = {}, conditions = {};

	phalcon_fetch_params(0, 2, 1, &expr, &values, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(values) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_query_exception_ce, "Values must be an array");
		return;
	}

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_NOISY);

	array_init(&bind_params);
	array_init(&bind_keys);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(values), value) {
		zval key = {}, query_key = {};
		/** 
		 * Key with auto bind-params
		 */
		PHALCON_CONCAT_SV(&key, "phi", &hidden_param);

		PHALCON_CONCAT_SVS(&query_key, ":", &key, ":");
		phalcon_array_append(&bind_keys, &query_key, PH_COPY);
		phalcon_array_update_zval(&bind_params, &key, value, PH_COPY);
		phalcon_increment(&hidden_param);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&joined_keys, SL(", "), &bind_keys);

	/** 
	 * Create a standard IN condition with bind params
	 */
	PHALCON_CONCAT_VSVS(&conditions, expr, " NOT IN (", &joined_keys, ")");

	/** 
	 * Append the IN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHODW(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}
	phalcon_update_property_zval(getThis(), SL("_hiddenParamNumber"), &hidden_param);

	RETURN_THISW();
}

/**
 * Return the conditions for the query
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getWhere){


	RETURN_MEMBER(getThis(), "_conditions");
}

/**
 * Compile the PHQL query
 *
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, compile){

	PHALCON_CALL_METHODW(NULL, getThis(), "_compile");

	RETURN_THISW();
}

/**
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getPhql){

	phalcon_read_property(return_value, getThis(), SL("_phql"), PH_NOISY);
	if (PHALCON_IS_EMPTY(return_value)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "compile");
		phalcon_read_property(return_value, getThis(), SL("_phql"), PH_NOISY);
	}
}

/**
 * Returns the query built
 *
 * @return Phalcon\Mvc\Model\Query
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getQuery){

	zval phql = {}, bind_params = {}, bind_types = {}, dependency_injector = {}, service_name = {}, has = {}, args = {}, query = {};

	/** 
	 * Process the PHQL
	 */
	PHALCON_CALL_METHODW(&phql, getThis(), "getphql");

	phalcon_read_property(&bind_params, getThis(), SL("_bindParams"), PH_NOISY);
	phalcon_read_property(&bind_types, getThis(), SL("_bindTypes"), PH_NOISY);

	PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi", &PHALCON_GLOBAL(z_true));

	PHALCON_STR(&service_name, ISV(modelsQuery));

	PHALCON_CALL_METHODW(&has, &dependency_injector, "has", &service_name);

	if (zend_is_true(&has)) {;
		array_init(&args);
		phalcon_array_append(&args, &phql, PH_COPY);
		phalcon_array_append(&args, &dependency_injector, PH_COPY);

		PHALCON_CALL_METHODW(&query, &dependency_injector, "get", &service_name, &args);
	} else {
		object_init_ex(&query, phalcon_mvc_model_query_ce);
		PHALCON_CALL_METHODW(NULL, &query, "__construct", &phql, &dependency_injector);
	}

	/** 
	 * Set default bind params
	 */
	if (Z_TYPE(bind_params) == IS_ARRAY) { 
		PHALCON_CALL_METHODW(NULL, &query, "setbindparams", &bind_params);
	}

	/** 
	 * Set default bind params
	 */
	if (Z_TYPE(bind_types) == IS_ARRAY) { 
		PHALCON_CALL_METHODW(NULL, &query, "setbindtypes", &bind_types);
	}

	RETURN_CTORW(&query);
}
