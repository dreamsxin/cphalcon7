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

#include "mvc/model/query/builder.h"
#include "mvc/model/query/builderinterface.h"
#include "mvc/model/query/exception.h"
#include "mvc/model/query/builder/select.h"
#include "mvc/model/query/builder/update.h"
#include "mvc/model/query/builder/insert.h"
#include "mvc/model/query/builder/delete.h"
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
#include "kernel/debug.h"
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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, setBindParams);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getBindParams);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getMergeBindParams);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, setBindTypes);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getBindTypes);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getMergeBindTypes);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, setIndex);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getIndex);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, compile);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getPhql);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getQuery);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_create, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_createselectbuilder, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 1)
	ZEND_ARG_OBJ_INFO(0, dependencyInjector, Phalcon\\DiInterface, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_setbindparams, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, bindparams, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_setbindtypes, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, bindtypes, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_setindex, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, index, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_query_builder_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, create, arginfo_phalcon_mvc_model_query_builder_create, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, createSelectBuilder, arginfo_phalcon_mvc_model_query_builder_createselectbuilder, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, createInsertBuilder, arginfo_phalcon_mvc_model_query_builder_createselectbuilder, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, createUpdateBuilder, arginfo_phalcon_mvc_model_query_builder_createselectbuilder, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, createDeleteBuilder, arginfo_phalcon_mvc_model_query_builder_createselectbuilder, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, setBindParams, arginfo_phalcon_mvc_model_query_builder_setbindparams, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getBindParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getMergeBindParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, setBindTypes, arginfo_phalcon_mvc_model_query_builder_setbindtypes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getBindTypes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getMergeBindTypes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, setIndex, arginfo_phalcon_mvc_model_query_builder_setindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getIndex, NULL, ZEND_ACC_PUBLIC)
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
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_bindParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_bindTypes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_mergeBindParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_mergeBindTypes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_phql"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_query_builder_ce, SL("_hiddenParamNumber"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_index"), ZEND_ACC_PROTECTED);

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

	phalcon_fetch_params(1, 1, 2, &type, &params, &_di);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	if (!_di) {
		_di = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(_di) == IS_OBJECT) {
		PHALCON_MM_VERIFY_INTERFACE(_di, phalcon_diinterface_ce);
		ZVAL_COPY_VALUE(&di, _di);
	} else {
		PHALCON_MM_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault", _di);
		PHALCON_MM_ADD_ENTRY(&di);
	}

	switch (phalcon_get_intval(type)) {
		case PHQL_T_SELECT:
			ZVAL_STR(&service_name, IS(modelsQueryBuilderForSelect));
			PHALCON_MM_CALL_METHOD(&has, &di, "has", &service_name);
			if (zend_is_true(&has)) {
				PHALCON_MM_CALL_METHOD(return_value, &di, "get", &service_name);
			} else {
				object_init_ex(return_value, phalcon_mvc_model_query_builder_select_ce);
			}
			PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", params);
			break;

		case PHQL_T_INSERT:
			ZVAL_STR(&service_name, IS(modelsQueryBuilderForInsert));
			PHALCON_MM_CALL_METHOD(&has, &di, "has", &service_name);
			if (zend_is_true(&has)) {
				PHALCON_MM_CALL_METHOD(return_value, &di, "get", &service_name, params);
			} else {
				object_init_ex(return_value, phalcon_mvc_model_query_builder_insert_ce);
			}
			PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", params);
			break;

		case PHQL_T_UPDATE:
			ZVAL_STR(&service_name, IS(modelsQueryBuilderForUpdate));
			PHALCON_MM_CALL_METHOD(&has, &di, "has", &service_name);
			if (zend_is_true(&has)) {
				PHALCON_MM_CALL_METHOD(return_value, &di, "get", &service_name, params);
			} else {
				object_init_ex(return_value, phalcon_mvc_model_query_builder_delete_ce);
			}
			PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", params);
			break;

		case PHQL_T_DELETE:
			ZVAL_STR(&service_name, IS(modelsQueryBuilderForDelete));
			PHALCON_MM_CALL_METHOD(&has, &di, "has", &service_name);
			if (zend_is_true(&has)) {
				PHALCON_MM_CALL_METHOD(return_value, &di, "get", &service_name, params);
			} else {
				object_init_ex(return_value, phalcon_mvc_model_query_builder_delete_ce);
			}
			PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", params);
			break;

		default:
			PHALCON_CONCAT_SV(&exception_message, "Not found builder: ", type);
			PHALCON_MM_ADD_ENTRY(&exception_message);
			PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_query_exception_ce, &exception_message);
			return;
	}

	PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_mvc_model_query_builderinterface_ce);
	RETURN_MM();
}

/**
 * Create a new Query Builder for Select
 *
 *
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createSelectBuilder){

	zval *params = NULL, *_di = NULL, di = {}, service_name = {};

	phalcon_fetch_params(1, 0, 2, &params, &_di);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	if (!_di || Z_TYPE_P(_di) == IS_NULL) {
		PHALCON_MM_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault");
		PHALCON_MM_ADD_ENTRY(&di);
	} else if (Z_TYPE_P(_di) == IS_STRING) {
		PHALCON_MM_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault", _di);
		PHALCON_MM_ADD_ENTRY(&di);
	} else {
		PHALCON_MM_VERIFY_INTERFACE(_di, phalcon_diinterface_ce);
		ZVAL_COPY_VALUE(&di, _di);
	}

	ZVAL_STR(&service_name, IS(modelsQueryBuilderForSelect));
	PHALCON_MM_CALL_METHOD(return_value, &di, "get", &service_name, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		zval_ptr_dtor(return_value);
		object_init_ex(return_value, phalcon_mvc_model_query_builder_select_ce);
		PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", params);
	}

	PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_mvc_model_query_builderinterface_ce);
	RETURN_MM();
}

/**
 * Create a new Query Builder for Insert
 *
 *
 * @return Phalcon\Mvc\Model\Query\Builder\Insert
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createInsertBuilder){
	zval *params = NULL, *_di = NULL, di = {}, service_name = {};

	phalcon_fetch_params(1, 0, 2, &params, &_di);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	if (!_di || Z_TYPE_P(_di) == IS_NULL) {
		PHALCON_MM_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault");
		PHALCON_MM_ADD_ENTRY(&di);
	} else if (Z_TYPE_P(_di) == IS_STRING) {
		PHALCON_MM_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault", _di);
		PHALCON_MM_ADD_ENTRY(&di);
	} else {
		PHALCON_MM_VERIFY_INTERFACE(_di, phalcon_diinterface_ce);
		ZVAL_COPY_VALUE(&di, _di);
	}

	ZVAL_STR(&service_name, IS(modelsQueryBuilderForInsert));
	PHALCON_MM_CALL_METHOD(return_value, &di, "get", &service_name, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		zval_ptr_dtor(return_value);
		object_init_ex(return_value, phalcon_mvc_model_query_builder_insert_ce);
		PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", params);
	}

	PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_mvc_model_query_builderinterface_ce);
	RETURN_MM();
}

/**
 * Create a new Query Builder for Update
 *
 *
 * @return Phalcon\Mvc\Model\Query\Builder\Update
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createUpdateBuilder){
	zval *params = NULL, *_di = NULL, di = {}, service_name = {};

	phalcon_fetch_params(1, 0, 2, &params, &_di);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	if (!_di || Z_TYPE_P(_di) == IS_NULL) {
		PHALCON_MM_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault");
		PHALCON_MM_ADD_ENTRY(&di);
	} else if (Z_TYPE_P(_di) == IS_STRING) {
		PHALCON_MM_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault", _di);
		PHALCON_MM_ADD_ENTRY(&di);
	} else {
		PHALCON_MM_VERIFY_INTERFACE(_di, phalcon_diinterface_ce);
		ZVAL_COPY_VALUE(&di, _di);
	}

	ZVAL_STR(&service_name, IS(modelsQueryBuilderForUpdate));
	PHALCON_MM_CALL_METHOD(return_value, &di, "get", &service_name, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		zval_ptr_dtor(return_value);
		object_init_ex(return_value, phalcon_mvc_model_query_builder_update_ce);
		PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", params);
	}

	PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_mvc_model_query_builderinterface_ce);
	RETURN_MM();
}

/**
 * Create a new Query Builder for Delete
 *
 *
 * @return Phalcon\Mvc\Model\Query\Builder\Delete
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, createDeleteBuilder){
	zval *params = NULL, *_di = NULL, di = {}, service_name = {};

	phalcon_fetch_params(1, 0, 2, &params, &_di);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	if (!_di || Z_TYPE_P(_di) == IS_NULL) {
		PHALCON_MM_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault");
		PHALCON_MM_ADD_ENTRY(&di);
	} else if (Z_TYPE_P(_di) == IS_STRING) {
		PHALCON_MM_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault", _di);
		PHALCON_MM_ADD_ENTRY(&di);
	} else {
		PHALCON_MM_VERIFY_INTERFACE(_di, phalcon_diinterface_ce);
		ZVAL_COPY_VALUE(&di, _di);
	}

	ZVAL_STR(&service_name, IS(modelsQueryBuilderForDelete));
	PHALCON_MM_CALL_METHOD(return_value, &di, "get", &service_name, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		zval_ptr_dtor(return_value);
		object_init_ex(return_value, phalcon_mvc_model_query_builder_delete_ce);
		PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", params);
	}

	PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_mvc_model_query_builderinterface_ce);
	RETURN_MM();
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
 * Sets the bind parameters
 *
 * @param array $bindParams
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, setBindParams){

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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getBindParams){


	RETURN_MEMBER(getThis(), "_bindParams");
}

/**
 * Gets the merge bind parameters
 *
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getMergeBindParams){


	RETURN_MEMBER(getThis(), "_mergeBindParams");
}

/**
 * Sets the bind types
 *
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, setBindTypes){

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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getBindTypes){


	RETURN_MEMBER(getThis(), "_bindTypes");
}

/**
 * Gets the merge bind types
 *
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getMergeBindTypes){


	RETURN_MEMBER(getThis(), "_mergeBindTypes");
}

/**
 * Adds the index
 *
 * @param string $index
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, setIndex) {

	zval *index;

	phalcon_fetch_params(0, 1, 0, &index);

	phalcon_update_property(getThis(), SL("_index"), index);
	RETURN_THIS();
}

/**
 * Gets the index
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getIndex){


	RETURN_MEMBER(getThis(), "_index");
}

/**
 * Compile the PHQL query
 *
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, compile){

	PHALCON_CALL_METHOD(NULL, getThis(), "_compile");

	RETURN_THIS();
}

/**
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getPhql){

	PHALCON_CALL_METHOD(NULL, getThis(), "compile");
	RETURN_MEMBER(getThis(), "_phql");
}

/**
 * Returns the query built
 *
 * @return Phalcon\Mvc\Model\Query
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getQuery){

	zval phql = {}, bind_params = {}, bind_types = {}, index = {}, dependency_injector = {}, service_name = {};
	zval cache = {}, has = {}, args = {};

	PHALCON_MM_INIT();
	/**
	 * Process the PHQL
	 */
	PHALCON_MM_CALL_METHOD(&phql, getThis(), "getphql");
	PHALCON_MM_ADD_ENTRY(&phql);
	PHALCON_MM_CALL_METHOD(&bind_params, getThis(), "getmergebindparams");
	PHALCON_MM_ADD_ENTRY(&bind_params);
	PHALCON_MM_CALL_METHOD(&bind_types, getThis(), "getmergebindtypes");
	PHALCON_MM_ADD_ENTRY(&bind_types);
	PHALCON_MM_CALL_METHOD(&index, getThis(), "getindex");
	PHALCON_MM_ADD_ENTRY(&index);

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi", &PHALCON_GLOBAL(z_true));
	PHALCON_MM_ADD_ENTRY(&dependency_injector);

	ZVAL_STR(&service_name, IS(modelsQuery));

	PHALCON_MM_CALL_METHOD(&has, &dependency_injector, "has", &service_name);

	if (zend_is_true(&has)) {
		array_init(&args);
		phalcon_array_append(&args, &phql, PH_COPY);
		phalcon_array_append(&args, &dependency_injector, PH_COPY);
		PHALCON_MM_ADD_ENTRY(&args);

		PHALCON_MM_CALL_METHOD(return_value, &dependency_injector, "get", &service_name, &args);
	} else {
		object_init_ex(return_value, phalcon_mvc_model_query_ce);
		PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", &phql, &dependency_injector);
	}

	if (phalcon_property_isset_fetch(&cache, getThis(), SL("_cache"), PH_READONLY)) {
		PHALCON_MM_CALL_METHOD(NULL, return_value, "cache", &cache);
	}

	/**
	 * Set default bind params
	 */
	if (Z_TYPE(bind_params) > IS_NULL) {
		PHALCON_MM_CALL_METHOD(NULL, return_value, "setbindparams", &bind_params);
	}

	/**
	 * Set default bind params
	 */
	if (Z_TYPE(bind_types) > IS_NULL) {
		PHALCON_MM_CALL_METHOD(NULL, return_value, "setbindtypes", &bind_types);
	}

	/**
	 * Set index
	 */
	if (Z_TYPE(index) > IS_NULL) {
		PHALCON_MM_CALL_METHOD(NULL, return_value, "setindex", &index);
	}
	RETURN_MM();
}
