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

#include "mvc/model/query/builder/where.h"
#include "mvc/model/query/builder.h"
#include "mvc/model/query/builderinterface.h"
#include "mvc/model/query/exception.h"
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
 * Helps to create PHQL queries for WHERE statements
 */
zend_class_entry *phalcon_mvc_model_query_builder_where_ce;

PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, setConditions);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, getConditions);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, where);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, andWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, orWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, betweenWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, notBetweenWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, inWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, notInWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, getWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, compile);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, getPhql);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, getQuery);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_where_setconditions, 0, 0, 1)
	ZEND_ARG_INFO(0, conditions)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_where_where, 0, 0, 1)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, bindParams)
	ZEND_ARG_INFO(0, bindTypes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_where_andwhere, 0, 0, 1)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, bindParams)
	ZEND_ARG_INFO(0, bindTypes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_where_orwhere, 0, 0, 1)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, bindParams)
	ZEND_ARG_INFO(0, bindTypes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_where_betweenwhere, 0, 0, 3)
	ZEND_ARG_INFO(0, expr)
	ZEND_ARG_INFO(0, minimum)
	ZEND_ARG_INFO(0, maximum)
	ZEND_ARG_INFO(0, useOrWhere)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_where_notbetweenwhere, 0, 0, 3)
	ZEND_ARG_INFO(0, expr)
	ZEND_ARG_INFO(0, minimum)
	ZEND_ARG_INFO(0, maximum)
	ZEND_ARG_INFO(0, useOrWhere)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_where_inwhere, 0, 0, 2)
	ZEND_ARG_INFO(0, expr)
	ZEND_ARG_INFO(0, values)
	ZEND_ARG_INFO(0, useOrWhere)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_where_notinwhere, 0, 0, 2)
	ZEND_ARG_INFO(0, expr)
	ZEND_ARG_INFO(0, values)
	ZEND_ARG_INFO(0, useOrWhere)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_query_builder_where_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Where, setConditions, arginfo_phalcon_mvc_model_query_builder_where_setconditions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Where, getConditions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Where, where, arginfo_phalcon_mvc_model_query_builder_where_where, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Where, andWhere, arginfo_phalcon_mvc_model_query_builder_where_andwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Where, orWhere, arginfo_phalcon_mvc_model_query_builder_where_orwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Where, betweenWhere, arginfo_phalcon_mvc_model_query_builder_where_betweenwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Where, notBetweenWhere, arginfo_phalcon_mvc_model_query_builder_where_notbetweenwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Where, inWhere, arginfo_phalcon_mvc_model_query_builder_where_inwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Where, notInWhere, arginfo_phalcon_mvc_model_query_builder_where_notinwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Where, getWhere, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Query\Builder initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Query_Builder_Where){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Query\\Builder, Where, mvc_model_query_builder_where, phalcon_mvc_model_query_builder_ce, phalcon_mvc_model_query_builder_where_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_mvc_model_query_builder_where_ce, SL("_conditions"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_query_builder_where_ce, 1, phalcon_mvc_model_query_builderinterface_ce);

	return SUCCESS;
}

/**
 * Gets the type of PHQL queries
 *
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, setConditions){

	zval *conditions, merged_conditions = {}, merged_bind_params = {}, merged_bind_types, new_condition_string = {}, *single_condition_array = NULL;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 0, &conditions);

	if (Z_TYPE_P(conditions) == IS_ARRAY) {
		/* ----------- INITIALIZING LOOP VARIABLES ----------- */

		/*
		 * array containing single condition for example:
		 * array(
		 *      array(
		 *           'status = :status:',
		 *           array('status' => 5),
		 *           array('status' => PDO::PARAM_INT),
		 *      ),
		 *      'name' => 'Dreamsxin',
		 * )
		 */
		array_init(&merged_conditions);
		array_init(&merged_bind_params);
		array_init(&merged_bind_types);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(conditions), idx, str_key, single_condition_array) {
			zval single_condition_key = {}, condition_string = {}, tmp_bind_params = {}, tmp_bind_types = {};
			if (str_key) {
				ZVAL_STR(&single_condition_key, str_key);
			} else {
				ZVAL_LONG(&single_condition_key, idx);
			}
			if (Z_TYPE_P(single_condition_array) == IS_ARRAY
				&& phalcon_array_isset_fetch_long(&condition_string, single_condition_array, 0)
				&& phalcon_array_isset_fetch_long(&tmp_bind_params, single_condition_array, 1)
				&& Z_TYPE(condition_string) == IS_STRING
				&& Z_TYPE(tmp_bind_params) == IS_ARRAY
			) {	
				phalcon_array_update_zval(&merged_conditions, &condition_string, &condition_string, PH_COPY);
				phalcon_array_merge_recursive_n(&merged_bind_params, &tmp_bind_params);

				if (phalcon_array_isset_fetch_long(&tmp_bind_types, single_condition_array, 2) && Z_TYPE(tmp_bind_types) == IS_ARRAY) {
					phalcon_array_merge_recursive_n(&merged_bind_types, &tmp_bind_types);
				}
			} else if (Z_TYPE(single_condition_key) == IS_STRING) {
				PHALCON_CONCAT_VSVS(&condition_string, &single_condition_key, " = :", &single_condition_key, ":");

				phalcon_array_update_zval(&merged_conditions, &single_condition_key, &condition_string, PH_COPY);

				if (Z_TYPE_P(single_condition_array) == IS_ARRAY) {
					phalcon_array_merge_recursive_n(&merged_bind_params, single_condition_array);
				} else {
					phalcon_array_update_zval(&merged_bind_params, &single_condition_key, single_condition_array, PH_COPY);
				}
			}
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&new_condition_string, SL(" AND "), &merged_conditions);

		phalcon_update_property_zval(getThis(), SL("_conditions"), &new_condition_string);

		PHALCON_CALL_METHODW(NULL, getThis(), "setbindparams", &merged_bind_params);
		PHALCON_CALL_METHODW(NULL, getThis(), "setbindtypes", &merged_bind_types);
	} else {
		phalcon_update_property_zval(getThis(), SL("_conditions"), conditions);		
	}
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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, getConditions){

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
				PHALCON_CALL_METHODW(&column_map, &meta_data, "getcolumnmap", &model_instance);

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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, where){

	zval *conditions, *bind_params = NULL, *bind_types = NULL;

	phalcon_fetch_params(0, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_property_zval(getThis(), SL("_conditions"), conditions);


	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		PHALCON_CALL_SELFW(NULL, "setbindparams", bind_params);
	}

	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		PHALCON_CALL_SELFW(NULL, "setbindtypes", bind_types);
	}

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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, andWhere){

	zval *conditions, *bind_params = NULL, *bind_types = NULL, current_conditions = {}, new_conditions = {};

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

	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		PHALCON_CALL_SELFW(NULL, "setbindparams", bind_params);
	}

	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		PHALCON_CALL_SELFW(NULL, "setbindtypes", bind_types);
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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, orWhere){

	zval *conditions, *bind_params = NULL, *bind_types = NULL, current_conditions = {}, new_conditions = {};

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


	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		PHALCON_CALL_SELFW(NULL, "setbindparams", bind_params);
	}

	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		PHALCON_CALL_SELFW(NULL, "setbindtypes", bind_types);
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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, betweenWhere){

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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, notBetweenWhere){

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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, inWhere){

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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, notInWhere){

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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, getWhere){


	RETURN_MEMBER(getThis(), "_conditions");
}

/**
 * Compile the PHQL query
 *
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, compile){

	PHALCON_CALL_METHODW(NULL, getThis(), "_compile");

	RETURN_THISW();
}

/**
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, getPhql){

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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Where, getQuery){

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
