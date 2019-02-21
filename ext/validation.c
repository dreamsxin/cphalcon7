
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

#include "validation.h"
#include "validationinterface.h"
#include "validation/exception.h"
#include "validation/message/group.h"
#include "validation/validator.h"
#include "validation/validatorinterface.h"
#include "translate/adapterinterface.h"
#include "di.h"
#include "di/injectable.h"
#include "filterinterface.h"
#include "kernel.h"
#include "arr.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/require.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Validation
 *
 * Allows to validate data using validators
 */
zend_class_entry *phalcon_validation_ce;

PHP_METHOD(Phalcon_Validation, __construct);
PHP_METHOD(Phalcon_Validation, validate);
PHP_METHOD(Phalcon_Validation, add);
PHP_METHOD(Phalcon_Validation, setFilters);
PHP_METHOD(Phalcon_Validation, getFilters);
PHP_METHOD(Phalcon_Validation, getValidators);
PHP_METHOD(Phalcon_Validation, setEntity);
PHP_METHOD(Phalcon_Validation, getEntity);
PHP_METHOD(Phalcon_Validation, getMessages);
PHP_METHOD(Phalcon_Validation, appendMessage);
PHP_METHOD(Phalcon_Validation, bind);
PHP_METHOD(Phalcon_Validation, getData);
PHP_METHOD(Phalcon_Validation, getValue);
PHP_METHOD(Phalcon_Validation, setDefaultMessages);
PHP_METHOD(Phalcon_Validation, getDefaultMessage);
PHP_METHOD(Phalcon_Validation, setLabels);
PHP_METHOD(Phalcon_Validation, getLabel);
PHP_METHOD(Phalcon_Validation, setLabelDelimiter);
PHP_METHOD(Phalcon_Validation, setMessageFilename);
PHP_METHOD(Phalcon_Validation, getMessage);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, validators, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_setfilters, 0, 0, 2)
	ZEND_ARG_INFO(0, attribute)
	ZEND_ARG_INFO(0, filters)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_getfilters, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, attribute, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_setentity, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, entity, IS_OBJECT, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_bind, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, entity, IS_OBJECT, 0)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_setdefaultmessages, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, messages, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_getdefaultmessage, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_TYPE_INFO(0, defaultValue, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_setlabels, 0, 0, 1)
	ZEND_ARG_INFO(0, labels)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_setlabeldelimiter, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, delimiter, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_setmessagefilename, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_getmessage, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_validation_method_entry[] = {
	PHP_ME(Phalcon_Validation, __construct, arginfo_phalcon_validation___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Validation, validate, arginfo_phalcon_validationinterface_validate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, add, arginfo_phalcon_validationinterface_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, setFilters, arginfo_phalcon_validation_setfilters, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, getFilters, arginfo_phalcon_validation_getfilters, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, getValidators, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, setEntity, arginfo_phalcon_validation_setentity, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, getEntity, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, getMessages, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, appendMessage, arginfo_phalcon_validationinterface_appendmessage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, bind, arginfo_phalcon_validation_bind, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, getData, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, getValue, arginfo_phalcon_validationinterface_getvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, setDefaultMessages, arginfo_phalcon_validation_setdefaultmessages, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, getDefaultMessage, arginfo_phalcon_validation_getdefaultmessage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, setLabels, arginfo_phalcon_validation_setlabels, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, getLabel, arginfo_phalcon_validationinterface_getlabel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation, setLabelDelimiter, arginfo_phalcon_validation_setlabeldelimiter, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Validation, setMessageFilename, arginfo_phalcon_validation_setmessagefilename, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Validation, getMessage, arginfo_phalcon_validation_getmessage, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation){

	PHALCON_REGISTER_CLASS_EX(Phalcon, Validation, validation, phalcon_di_injectable_ce, phalcon_validation_method_entry, 0);

	zend_declare_property_null(phalcon_validation_ce, SL("_data"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_ce, SL("_entity"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_ce, SL("_validators"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_ce, SL("_filters"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_ce, SL("_messages"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_ce, SL("_values"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_ce, SL("_labels"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_ce, SL("_messageFilename"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_validation_ce, SL("_allowEmpty"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_validation_ce, SL("_delimiter"), ", ", ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_validation_ce, SL("_defaultMessageFilename"), "validation", ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	zend_class_implements(phalcon_validation_ce, 1, phalcon_validationinterface_ce);

	return SUCCESS;
}

int phalcon_validation_getdefaultmessage_helper(zval *retval, const zend_class_entry *ce, zval *this_ptr, const char *type)
{
	zval t = {}, *params[1];
	int ret;
	ZVAL_STRING(&t, type);
	params[0] = &t;

	ret = phalcon_call_method(retval, this_ptr, "getdefaultmessage", 1, params);
	zval_ptr_dtor(&t);
	return ret;
}

/**
 * Phalcon\Validation constructor
 *
 * @param array $validators
 * @param array $options
 */
PHP_METHOD(Phalcon_Validation, __construct){

	zval *validators = NULL, *options = NULL;

	phalcon_fetch_params(0, 0, 2, &validators, &options);

	if (validators && Z_TYPE_P(validators) == IS_ARRAY) {
		zval *scope;
		zend_string *str_key;

		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(validators), str_key, scope) {
			zval attribute = {}, validator = {};

			if (str_key) {
				ZVAL_STR(&attribute, str_key);
				PHALCON_CALL_METHOD(NULL, getThis(), "add", &attribute, scope);
			} else {
				if (Z_TYPE_P(scope) != IS_ARRAY) {
					PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "Validators is invalid");
					return;
				}

				phalcon_array_fetch_long(&attribute, scope, 0, PH_NOISY|PH_READONLY);
				phalcon_array_fetch_long(&validator, scope, 1, PH_NOISY|PH_READONLY);
				PHALCON_CALL_METHOD(NULL, getThis(), "add", &attribute, &validator);
			}
		} ZEND_HASH_FOREACH_END();
	}

	/* Check for an 'initialize' method */
	if (phalcon_method_exists_ex(getThis(), SL("initialize")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, getThis(), "initialize");
	}

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		zval filename = {}, allow_empty = {};
		if (phalcon_array_isset_fetch_str(&filename, options, SL("messageFilename"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_messageFilename"), &filename);
		}
		if (phalcon_array_isset_fetch_str(&allow_empty, options, SL("allowEmpty"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_allowEmpty"), &allow_empty);
		}
	}
}

/**
 * Validate a set of data according to a set of rules
 *
 * @param array|object $data
 * @param object $entity
 * @return Phalcon\Validation\Message\Group
 */
PHP_METHOD(Phalcon_Validation, validate){

	zval *data = NULL, *entity = NULL, validators = {}, allow_empty = {}, messages = {}, status = {}, *scope;

	phalcon_fetch_params(1, 0, 2, &data, &entity);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!entity) {
		entity = &PHALCON_GLOBAL(z_null);
	} else {
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setentity", entity);

	}

	phalcon_read_property(&validators, getThis(), SL("_validators"), PH_READONLY);
	if (Z_TYPE(validators) != IS_ARRAY) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "There are no validators to validate");
		return;
	}

	phalcon_read_property(&allow_empty, getThis(), SL("_allowEmpty"), PH_READONLY);

	/**
	 * Clear pre-calculated values
	 */
	phalcon_update_property_null(getThis(), SL("_values"));

	/**
	 * Implicitly creates a Phalcon\Validation\Message\Group object
	 */
	object_init_ex(&messages, phalcon_validation_message_group_ce);
	PHALCON_MM_ADD_ENTRY(&messages);
	PHALCON_MM_CALL_METHOD(NULL, &messages, "__construct");

	phalcon_update_property(getThis(), SL("_messages"), &messages);

	/**
	 * Validation classes can implement the 'beforeValidation' callback
	 */
	if (phalcon_method_exists_ex(getThis(), SL("beforevalidation")) == SUCCESS) {
		PHALCON_MM_CALL_METHOD(&status, getThis(), "beforevalidation", data, entity, &messages);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_NCTOR(&status);
		}
	}

	if (Z_TYPE_P(data) == IS_ARRAY || Z_TYPE_P(data) == IS_OBJECT) {
		phalcon_update_property(getThis(), SL("_data"), data);
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(validators), scope) {
		zval attribute = {}, validator = {}, attribute_validators = {}, *attribute_validator, must_cancel = {};
		if (Z_TYPE_P(scope) != IS_ARRAY) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "The validator scope is not valid");
			return;
		}

		phalcon_array_fetch_long(&attribute, scope, 0, PH_NOISY|PH_READONLY);
		phalcon_array_fetch_long(&validator, scope, 1, PH_NOISY|PH_READONLY);

		if (Z_TYPE(validator) != IS_ARRAY) {
			array_init_size(&attribute_validators, 1);
			phalcon_array_append(&attribute_validators, &validator, PH_COPY);
		} else {
			ZVAL_COPY(&attribute_validators, &validator);
		}
		PHALCON_MM_ADD_ENTRY(&attribute_validators);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attribute_validators), attribute_validator) {
			if (Z_TYPE_P(attribute_validator) != IS_OBJECT) {
				PHALCON_MM_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "One of the validators is not valid");
				return;
			}

			PHALCON_MM_CALL_METHOD(&status, attribute_validator, "validate", getThis(), &attribute, &allow_empty);

			/**
			 * Check if the validation must be canceled if this validator fails
			 */
			if (PHALCON_IS_FALSE(&status)) {
				RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&must_cancel, Z_OBJCE_P(attribute_validator), attribute_validator, "cancelOnFail"));

				if (zend_is_true(&must_cancel)) {
					break;
				}
			}
		} ZEND_HASH_FOREACH_END();

		if (zend_is_true(&must_cancel)) {
			break;
		}
	} ZEND_HASH_FOREACH_END();

	/**
	 * Get the messages generated by the validators
	 */
	phalcon_read_property(&messages, getThis(),  SL("_messages"), PH_READONLY);
	if (phalcon_method_exists_ex(getThis(), SL("aftervalidation")) == SUCCESS) {
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "aftervalidation", data, entity, &messages);
	}

	RETURN_MM_CTOR(&messages);
}

/**
 * Adds a validator to a field
 *
 * @param string|array $attribute
 * @param array|Phalcon\Validation\ValidatorInterface
 * @return Phalcon\Validation
 */
PHP_METHOD(Phalcon_Validation, add){

	zval *attribute, *_validator, validator = {}, validators = {}, scope = {};

	phalcon_fetch_params(1, 2, 0, &attribute, &_validator);

	if (Z_TYPE_P(attribute) != IS_STRING && Z_TYPE_P(attribute) != IS_ARRAY) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "Field must be passed as array of fields or string");
		return;
	}

	if (Z_TYPE_P(_validator) == IS_STRING) {
		array_init_size(&validator, 1);
		phalcon_array_append(&validator, _validator, PH_COPY);
	} else {
		ZVAL_COPY(&validator, _validator);
	}
	PHALCON_MM_ADD_ENTRY(&validator);

	array_init(&validators);
	PHALCON_MM_ADD_ENTRY(&validators);

	if (Z_TYPE(validator) == IS_ARRAY) {
		zval *item;
		zend_string *item_key;

		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(validator), item_key, item) {
			zval name = {}, has = {}, class_name = {}, options = {}, object = {};

			if (!item_key) {
				if (Z_TYPE_P(item)!= IS_STRING) {
					PHALCON_MM_VERIFY_INTERFACE_EX(item, phalcon_validation_validatorinterface_ce, phalcon_validation_exception_ce);
					phalcon_array_append(&validators, item, PH_COPY);
					continue;
				}
				ZVAL_COPY_VALUE(&name, item);
				array_init(&options);
			} else {
				ZVAL_STR(&name, item_key);
				array_init_size(&options, 1);
				phalcon_array_append(&options, item, PH_COPY);
			}
			PHALCON_MM_ADD_ENTRY(&options);

			PHALCON_MM_CALL_METHOD(&has, getThis(), "hasservice", &name);
			if (!zend_is_true(&has)) {
				if (!phalcon_memnstr_str(&name, SL("\\"))) {
					PHALCON_CONCAT_SV(&class_name, "Phalcon\\Validation\\Validator\\", &name);
					PHALCON_MM_ADD_ENTRY(&class_name);
				} else {
					PHALCON_MM_ZVAL_COPY(&class_name, &name);
				}
			} else {
				PHALCON_MM_ZVAL_COPY(&class_name, &name);
			}

			PHALCON_MM_CALL_METHOD(&object, getThis(), "getresolveservice", &class_name, &options, &PHALCON_GLOBAL(z_false), &PHALCON_GLOBAL(z_true));
			PHALCON_MM_ADD_ENTRY(&object);
			if (Z_TYPE(object) != IS_OBJECT) {
				PHALCON_MM_THROW_EXCEPTION_FORMAT(phalcon_validation_exception_ce, "Validator %s is invalid", Z_STRVAL(name));
				return;
			}

			PHALCON_MM_VERIFY_INTERFACE_EX(&object, phalcon_validation_validatorinterface_ce, phalcon_validation_exception_ce);
			phalcon_array_append(&validators, &object, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	} else {
		PHALCON_MM_VERIFY_INTERFACE_EX(&validator, phalcon_validation_validatorinterface_ce, phalcon_validation_exception_ce);
		phalcon_array_append(&validators, &validator, PH_COPY);
	}

	array_init_size(&scope, 2);
	phalcon_array_append(&scope, attribute, PH_COPY);
	phalcon_array_append(&scope, &validators, PH_COPY);
	phalcon_update_property_array_append(getThis(), SL("_validators"), &scope);
	zval_ptr_dtor(&scope);

	RETURN_MM_THIS();
}

/**
 * Adds filters to the field
 *
 * @param string $attribute
 * @param array|string $attribute
 * @return Phalcon\Validation
 */
PHP_METHOD(Phalcon_Validation, setFilters){

	zval *attribute, *filters;

	phalcon_fetch_params(0, 2, 0, &attribute, &filters);

	phalcon_update_property_array(getThis(), SL("_filters"), attribute, filters);
	RETURN_THIS();
}

/**
 * Returns all the filters or a specific one
 *
 * @param string $attribute
 * @return mixed
 */
PHP_METHOD(Phalcon_Validation, getFilters){

	zval *attribute = NULL, filters = {}, attribute_filters = {};

	phalcon_fetch_params(0, 0, 1, &attribute);

	phalcon_read_property(&filters, getThis(), SL("_filters"), PH_READONLY);
	if (attribute && Z_TYPE_P(attribute) == IS_STRING) {
		if (phalcon_array_isset_fetch(&attribute_filters, &filters, attribute, PH_READONLY)) {
			RETURN_CTOR(&attribute_filters);
		}

		RETURN_NULL();
	}

	RETURN_CTOR(&filters);
}

/**
 * Returns the validators added to the validation
 *
 * @return array
 */
PHP_METHOD(Phalcon_Validation, getValidators){


	RETURN_MEMBER(getThis(), "_validators");
}

/**
 * Sets the bound entity
 *
 * @param object entity
 * @return Phalcon\Validation
 */
PHP_METHOD(Phalcon_Validation, setEntity){

	zval *entity;

	phalcon_fetch_params(0, 1, 0, &entity);

	phalcon_update_property(getThis(), SL("_entity"), entity);
	RETURN_THIS();
}

/**
 * Returns the bound entity
 *
 * @return object
 */
PHP_METHOD(Phalcon_Validation, getEntity){


	RETURN_MEMBER(getThis(), "_entity");
}

/**
 * Returns the registered validators
 *
 * @return Phalcon\Validation\Message\Group
 */
PHP_METHOD(Phalcon_Validation, getMessages){


	RETURN_MEMBER(getThis(), "_messages");
}

/**
 * Appends a message to the messages list
 *
 * @param Phalcon\Validation\MessageInterface $message
 * @return Phalcon\Validation
 */
PHP_METHOD(Phalcon_Validation, appendMessage){

	zval *message, messages = {};

	phalcon_fetch_params(0, 1, 0, &message);

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_NOISY|PH_COPY);
	if (Z_TYPE(messages) != IS_OBJECT) {
	   object_init_ex(&messages, phalcon_validation_message_group_ce);
	   PHALCON_CALL_METHOD(NULL, &messages, "__construct");

	   phalcon_update_property(getThis(), SL("_messages"), &messages);
	}

	PHALCON_CALL_METHOD(NULL, &messages, "appendmessage", message);
	zval_ptr_dtor(&messages);

	RETURN_THIS();
}

/**
 * Assigns the data to an entity
 * The entity is used to obtain the validation values
 *
 * @param object $entity
 * @param object|array $data
 * @return Phalcon\Validation
 */
PHP_METHOD(Phalcon_Validation, bind){

	zval *entity, *data;

	phalcon_fetch_params(0, 2, 0, &entity, &data);

	if (Z_TYPE_P(data) != IS_ARRAY && Z_TYPE_P(data) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "The data to validate must be an array or object");
	}

	phalcon_update_property(getThis(), SL("_entity"), entity);
	phalcon_update_property(getThis(), SL("_data"), data);

	RETURN_THIS();
}

/**
 * Gets the a array data source
 *
 * @return array|null
 */
PHP_METHOD(Phalcon_Validation, getData){


	RETURN_MEMBER(getThis(), "_data");
}

/**
 * Gets the a value to validate in the array/object data source
 *
 * @param string $attribute
 * @param object entity
 * @return mixed
 */
PHP_METHOD(Phalcon_Validation, getValue){

	zval *attribute, *_entity = NULL, entity ={}, value = {}, data = {}, values = {}, filters = {}, field_filters = {};

	phalcon_fetch_params(0, 1, 1, &attribute, &_entity);

	/**
	 * Check if there is a calculated value
	 */
	phalcon_read_property(&values, getThis(), SL("_values"), PH_READONLY);
	if (phalcon_array_isset_fetch(&value, &values, attribute, PH_READONLY)) {
		RETURN_CTOR(&value);
	}

	if (!_entity || Z_TYPE_P(_entity) != IS_OBJECT) {
		phalcon_read_property(&entity, getThis(), SL("_entity"), PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&entity, _entity);
	}

	phalcon_read_property(&data, getThis(), SL("_data"), PH_READONLY);
	if (Z_TYPE(data) != IS_ARRAY && Z_TYPE(data) != IS_OBJECT) {
		/**
		 * If the entity is an object use it to retrieve the values
		 */
		if (Z_TYPE(entity) == IS_OBJECT) {
			if (phalcon_method_exists_ex(&entity, SL("readattribute")) == SUCCESS) {
				PHALCON_CALL_METHOD(&value, &entity, "readattribute", attribute);
			} else {
				phalcon_read_property_zval(&value, &entity, attribute, PH_COPY);
			}
			RETURN_ZVAL(&value, 0, 0);
		} else {
			PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "There are no data to validate");
		}
		return;
	}

	if (Z_TYPE(data) == IS_ARRAY) {
		 if (!phalcon_array_isset_fetch(&value, &data, attribute, PH_COPY)) {
			ZVAL_NULL(&value);
		 }
	} else if (Z_TYPE(data) == IS_OBJECT) {
		if (phalcon_isset_property_zval(&data, attribute)) {
			phalcon_read_property_zval(&value, &data, attribute, PH_COPY);
		} else {
			ZVAL_NULL(&value);
		}
	}

	if (Z_TYPE(value) != IS_NULL) {
		phalcon_read_property(&filters, getThis(), SL("_filters"), PH_READONLY);
		if (Z_TYPE(filters) == IS_ARRAY) {
			if (phalcon_array_isset_fetch(&field_filters, &filters, attribute, PH_READONLY)) {
				if (zend_is_true(&field_filters)) {
					zval filter_value = {}, service_name = {}, dependency_injector = {}, filter_service = {};
					ZVAL_STR(&service_name, IS(filter));

					PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi");
					if (Z_TYPE(dependency_injector) != IS_OBJECT) {
						PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "A dependency injector is required to obtain the 'filter' service");
						return;
					}

					PHALCON_CALL_METHOD(&filter_service, &dependency_injector, "getshared", &service_name);
					zval_ptr_dtor(&dependency_injector);
					if (Z_TYPE(filter_service) != IS_OBJECT) {
						PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "Returned 'filter' service is invalid");
						return;
					}

					PHALCON_VERIFY_INTERFACE(&filter_service, phalcon_filterinterface_ce);
					PHALCON_CALL_METHOD(&filter_value, &filter_service, "sanitize", &value, &field_filters);
					zval_ptr_dtor(&filter_service);
					zval_ptr_dtor(&value);
					ZVAL_COPY_VALUE(&value, &filter_value);
				}
			}
		}

	   	if (Z_TYPE(entity) == IS_OBJECT) {
	   		if (phalcon_method_exists_ex(&entity, SL("writeattribute")) == SUCCESS) {
	   			PHALCON_CALL_METHOD(NULL, &entity, "writeattribute", attribute, &value);
	   		} else {
	   			phalcon_update_property_zval_zval(&entity, attribute, &value);
	   		}
	   	}

		/**
		 * Cache the calculated value
		 */
		phalcon_update_property_array(getThis(), SL("_values"), attribute, &value);
	}

	RETURN_ZVAL(&value, 0, 0);
}

PHP_METHOD(Phalcon_Validation, setDefaultMessages)
{
	zval *messages, file = {};

	phalcon_fetch_params(0, 1, 0, &messages);

	phalcon_read_static_property_ce(&file, phalcon_validation_ce, SL("_file"), PH_READONLY);

	phalcon_update_static_property_array_ce(phalcon_kernel_ce, SL("_defaultMessages"), &file, messages);
}

PHP_METHOD(Phalcon_Validation, getDefaultMessage)
{
	zval *type, *default_value = NULL, filename = {}, file = {};

	phalcon_fetch_params(0, 1, 1, &type, &default_value);

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&filename, getThis(), SL("_messageFilename"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_EMPTY(&filename)) {
		phalcon_read_static_property_ce(&file, phalcon_validation_ce, SL("_defaultMessageFilename"), PH_READONLY);
		PHALCON_CALL_CE_STATIC(return_value, phalcon_kernel_ce, "message", &file, type, default_value);
	} else {
		PHALCON_CALL_CE_STATIC(return_value, phalcon_kernel_ce, "message", &filename, type, default_value);
	}

}

/**
 * Adds labels for fields
 *
 * @param array labels
 */
PHP_METHOD(Phalcon_Validation, setLabels) {

	zval *labels;

	phalcon_fetch_params(0, 1, 0, &labels);

	if (Z_TYPE_P(labels) != IS_ARRAY) {
		zend_throw_exception_ex(phalcon_validation_exception_ce, 0, "Labels must be an array");
		return;
	}
	phalcon_update_property(getThis(), SL("_labels"), labels);
}

/**
 * Gets label for field
 *
 * @param string|array field
 * @return string
 */
PHP_METHOD(Phalcon_Validation, getLabel) {

	zval *field_param = NULL, labels = {}, entity = {};
	int exists = 0;

	phalcon_fetch_params(0, 1, 0, &field_param);

	if (Z_TYPE_P(field_param) == IS_NULL) {
		RETURN_NULL();
	}

	if (Z_TYPE_P(field_param) != IS_STRING && Z_TYPE_P(field_param) != IS_ARRAY) {
		zend_throw_exception_ex(phalcon_validation_exception_ce, 0, "Parameter 'field' must be a string or array");
		RETURN_NULL();
	}

	phalcon_read_property(&labels, getThis(), SL("_labels"), PH_READONLY);
	phalcon_read_property(&entity, getThis(), SL("_entity"), PH_READONLY);
	if (Z_TYPE(entity) == IS_OBJECT && phalcon_method_exists_ex(&entity, SL("getlabel")) == SUCCESS) {
		exists = 1;
	}

	if (Z_TYPE_P(field_param) == IS_ARRAY) {
		zval label_values = {}, *field, delimiter = {};
		array_init(&label_values);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(field_param), field) {
			zval label = {};
			if (Z_TYPE(labels) == IS_ARRAY && phalcon_array_isset_fetch(&label, &labels, field, PH_READONLY) && Z_TYPE(label) == IS_STRING) {
				phalcon_array_append(&label_values, &label, PH_COPY);
			} else if (exists) {
				PHALCON_CALL_METHOD(&label, &entity, "getlabel", field_param);
				if (Z_TYPE(label) == IS_STRING) {
					phalcon_array_append(&label_values, &label, 0);
				} else {
					phalcon_array_append(&label_values, field, PH_COPY);
				}
			} else {
				phalcon_array_append(&label_values, field, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
		phalcon_read_static_property_ce(&delimiter, phalcon_validation_ce, SL("_delimiter"), PH_READONLY);
		phalcon_fast_join_str(return_value, Z_STRVAL(delimiter), Z_STRLEN(delimiter), &label_values);
		zval_ptr_dtor(&label_values);
	} else {
		if (Z_TYPE(labels) != IS_ARRAY || !phalcon_array_isset_fetch(return_value, &labels, field_param, PH_COPY) || Z_TYPE_P(return_value) == IS_NULL) {
			if (exists) {
				PHALCON_CALL_METHOD(return_value, &entity, "getlabel", field_param);
				if (Z_TYPE_P(return_value) == IS_NULL) {
					ZVAL_COPY(return_value, field_param);
				}
			} else {
				ZVAL_COPY(return_value, field_param);
			}
		}
	}
}

/**
 * Sets delimiter for label
 *
 * @param string
 */
PHP_METHOD(Phalcon_Validation, setLabelDelimiter)
{
	zval *delimiter;

	phalcon_fetch_params(0, 1, 0, &delimiter);

	phalcon_update_static_property_ce(phalcon_validation_ce, SL("_delimiter"), delimiter);
}

/**
 * Sets validation message file name
 *
 * @param string filename
 */
PHP_METHOD(Phalcon_Validation, setMessageFilename)
{
	zval *filename;

	phalcon_fetch_params(0, 1, 0, &filename);

	phalcon_update_static_property_ce(phalcon_validation_ce, SL("_defaultMessageFilename"), filename);
}

/**
 * Gets message
 *
 * @param string
 */
PHP_METHOD(Phalcon_Validation, getMessage)
{
	zval *type, filename = {};

	phalcon_fetch_params(0, 1, 0, &type);

	phalcon_read_static_property_ce(&filename, phalcon_validation_ce, SL("_defaultMessageFilename"), PH_READONLY);

	PHALCON_CALL_CE_STATIC(return_value, phalcon_kernel_ce, "message", &filename, type);
}
