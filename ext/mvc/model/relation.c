
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

#include "mvc/model/relation.h"
#include "mvc/model/relationinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/operators.h"

/**
 * Phalcon\Mvc\Model\Relation
 *
 * This class represents a relationship between two models
 */
zend_class_entry *phalcon_mvc_model_relation_ce;

PHP_METHOD(Phalcon_Mvc_Model_Relation, __construct);
PHP_METHOD(Phalcon_Mvc_Model_Relation, setIntermediateRelation);
PHP_METHOD(Phalcon_Mvc_Model_Relation, getType);
PHP_METHOD(Phalcon_Mvc_Model_Relation, getReferencedModel);
PHP_METHOD(Phalcon_Mvc_Model_Relation, getFields);
PHP_METHOD(Phalcon_Mvc_Model_Relation, getReferencedFields);
PHP_METHOD(Phalcon_Mvc_Model_Relation, getOptions);
PHP_METHOD(Phalcon_Mvc_Model_Relation, isForeignKey);
PHP_METHOD(Phalcon_Mvc_Model_Relation, getForeignKey);
PHP_METHOD(Phalcon_Mvc_Model_Relation, isThrough);
PHP_METHOD(Phalcon_Mvc_Model_Relation, isReusable);
PHP_METHOD(Phalcon_Mvc_Model_Relation, getIntermediateFields);
PHP_METHOD(Phalcon_Mvc_Model_Relation, getIntermediateModel);
PHP_METHOD(Phalcon_Mvc_Model_Relation, getIntermediateReferencedFields);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_relation___construct, 0, 0, 4)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, referencedModel)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, referencedFields)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_relation_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Relation, __construct, arginfo_phalcon_mvc_model_relation___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Relation, setIntermediateRelation, arginfo_phalcon_mvc_model_relationinterface_setintermediaterelation, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, getReferencedModel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, getFields, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, getReferencedFields, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, getOptions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, isForeignKey, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, getForeignKey, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, isThrough, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, isReusable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, getIntermediateFields, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, getIntermediateModel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Relation, getIntermediateReferencedFields, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Relation initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Relation){

	PHALCON_REGISTER_CLASS(Phalcon\\Mvc\\Model, Relation, mvc_model_relation, phalcon_mvc_model_relation_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_relation_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_relation_ce, SL("_referencedModel"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_relation_ce, SL("_fields"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_relation_ce, SL("_referencedFields"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_relation_ce, SL("_intermediateModel"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_relation_ce, SL("_intermediateFields"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_relation_ce, SL("_intermediateReferencedFields"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_relation_ce, SL("_options"), ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_mvc_model_relation_ce, SL("BELONGS_TO"), 0);
	zend_declare_class_constant_long(phalcon_mvc_model_relation_ce, SL("HAS_ONE"), 1);
	zend_declare_class_constant_long(phalcon_mvc_model_relation_ce, SL("HAS_MANY"), 2);
	zend_declare_class_constant_long(phalcon_mvc_model_relation_ce, SL("HAS_ONE_THROUGH"), 3);
	zend_declare_class_constant_long(phalcon_mvc_model_relation_ce, SL("HAS_MANY_THROUGH"), 4);
	zend_declare_class_constant_long(phalcon_mvc_model_relation_ce, SL("NO_ACTION"), 0);
	zend_declare_class_constant_long(phalcon_mvc_model_relation_ce, SL("ACTION_RESTRICT"), 1);
	zend_declare_class_constant_long(phalcon_mvc_model_relation_ce, SL("ACTION_CASCADE"), 2);

	zend_class_implements(phalcon_mvc_model_relation_ce, 1, phalcon_mvc_model_relationinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Relation constructor
 *
 * @param int $type
 * @param string $referencedModel
 * @param string|array $fields
 * @param string|array $referencedFields
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, __construct){

	zval *type, *referenced_model, *fields, *referenced_fields;
	zval *options = NULL;

	phalcon_fetch_params(0, 4, 1, &type, &referenced_model, &fields, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_property(getThis(), SL("_type"), type);
	phalcon_update_property(getThis(), SL("_referencedModel"), referenced_model);
	phalcon_update_property(getThis(), SL("_fields"), fields);
	phalcon_update_property(getThis(), SL("_referencedFields"), referenced_fields);
	phalcon_update_property(getThis(), SL("_options"), options);
}

/**
 * Sets the intermediate model data for has-*-through relations
 *
 * @param string|array $intermediateFields
 * @param string $intermediateModel
 * @param string $intermediateReferencedFields
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, setIntermediateRelation){

	zval *intermediate_fields, *intermediate_model;
	zval *intermediate_referenced_fields;

	phalcon_fetch_params(0, 3, 0, &intermediate_fields, &intermediate_model, &intermediate_referenced_fields);

	phalcon_update_property(getThis(), SL("_intermediateFields"), intermediate_fields);
	phalcon_update_property(getThis(), SL("_intermediateModel"), intermediate_model);
	phalcon_update_property(getThis(), SL("_intermediateReferencedFields"), intermediate_referenced_fields);

}

/**
 * Returns the relation type
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, getType){


	RETURN_MEMBER(getThis(), "_type");
}

/**
 * Returns the referenced model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, getReferencedModel){


	RETURN_MEMBER(getThis(), "_referencedModel");
}

/**
 * Returns the fields
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, getFields){


	RETURN_MEMBER(getThis(), "_fields");
}

/**
 * Returns the referenced fields
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, getReferencedFields){


	RETURN_MEMBER(getThis(), "_referencedFields");
}

/**
 * Returns the options
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, getOptions){


	RETURN_MEMBER(getThis(), "_options");
}

/**
 * Check whether the relation act as a foreign key
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, isForeignKey){

	zval options = {};

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(options) == IS_ARRAY) {
		if (phalcon_array_isset_str(&options, SL("foreignKey"))) {
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}

/**
 * Returns the foreign key configuration
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, getForeignKey){

	zval options = {}, foreign_key = {};

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);

	if (phalcon_array_isset_fetch_str(&foreign_key, &options, SL("foreignKey"), PH_READONLY)) {
		if (zend_is_true(&foreign_key)) {
			RETURN_CTOR(&foreign_key);
		}
	}

	RETURN_FALSE;
}

/**
 * Check whether the relation is a 'many-to-many' relation or not
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, isThrough){

	zval type = {};

	phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_LONG(&type, 3) || PHALCON_IS_LONG(&type, 4)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Check if records returned by getting belongs-to/has-many are implicitly cached during the current request
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, isReusable){

	zval options = {}, reusable = {};

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);

	if (phalcon_array_isset_fetch_str(&reusable, &options, SL("reusable"), PH_READONLY)) {
		RETURN_CTOR(&reusable);
	}

	RETURN_FALSE;
}

/**
 * Gets the intermediate fields for has-*-through relations
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, getIntermediateFields){


	RETURN_MEMBER(getThis(), "_intermediateFields");
}

/**
 * Gets the intermediate model for has-*-through relations
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, getIntermediateModel){


	RETURN_MEMBER(getThis(), "_intermediateModel");
}

/**
 * Gets the intermediate referenced fields for has-*-through relations
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Relation, getIntermediateReferencedFields){


	RETURN_MEMBER(getThis(), "_intermediateReferencedFields");
}
