
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

#include "forms/form.h"
#include "forms/elementinterface.h"
#include "forms/exception.h"
#include "tag.h"
#include "di/injectable.h"
#include "diinterface.h"
#include "filterinterface.h"
#include "validation.h"
#include "validation/exception.h"
#include "validation/message/group.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/file.h"
#include "kernel/string.h"

#include "interned-strings.h"

/**
 * Phalcon\Forms\Form
 *
 * This component allows to build forms using an object-oriented interface
 */
zend_class_entry *phalcon_forms_form_ce;

PHP_METHOD(Phalcon_Forms_Form, __construct);
PHP_METHOD(Phalcon_Forms_Form, setAction);
PHP_METHOD(Phalcon_Forms_Form, getAction);
PHP_METHOD(Phalcon_Forms_Form, setMethod);
PHP_METHOD(Phalcon_Forms_Form, getMethod);
PHP_METHOD(Phalcon_Forms_Form, setEnctype);
PHP_METHOD(Phalcon_Forms_Form, getEnctype);
PHP_METHOD(Phalcon_Forms_Form, setOption);
PHP_METHOD(Phalcon_Forms_Form, getOption);
PHP_METHOD(Phalcon_Forms_Form, setOptions);
PHP_METHOD(Phalcon_Forms_Form, getOptions);
PHP_METHOD(Phalcon_Forms_Form, setEntity);
PHP_METHOD(Phalcon_Forms_Form, getEntity);
PHP_METHOD(Phalcon_Forms_Form, getElements);
PHP_METHOD(Phalcon_Forms_Form, bind);
PHP_METHOD(Phalcon_Forms_Form, isValid);
PHP_METHOD(Phalcon_Forms_Form, getMessages);
PHP_METHOD(Phalcon_Forms_Form, getMessagesFor);
PHP_METHOD(Phalcon_Forms_Form, hasMessagesFor);
PHP_METHOD(Phalcon_Forms_Form, add);
PHP_METHOD(Phalcon_Forms_Form, render);
PHP_METHOD(Phalcon_Forms_Form, get);
PHP_METHOD(Phalcon_Forms_Form, label);
PHP_METHOD(Phalcon_Forms_Form, getLabel);
PHP_METHOD(Phalcon_Forms_Form, getValue);
PHP_METHOD(Phalcon_Forms_Form, setValue);
PHP_METHOD(Phalcon_Forms_Form, getValues);
PHP_METHOD(Phalcon_Forms_Form, has);
PHP_METHOD(Phalcon_Forms_Form, remove);
PHP_METHOD(Phalcon_Forms_Form, clear);
PHP_METHOD(Phalcon_Forms_Form, count);
PHP_METHOD(Phalcon_Forms_Form, rewind);
PHP_METHOD(Phalcon_Forms_Form, current);
PHP_METHOD(Phalcon_Forms_Form, key);
PHP_METHOD(Phalcon_Forms_Form, next);
PHP_METHOD(Phalcon_Forms_Form, valid);
PHP_METHOD(Phalcon_Forms_Form, appendMessage);
PHP_METHOD(Phalcon_Forms_Form, appendMessages);
PHP_METHOD(Phalcon_Forms_Form, toArray);
PHP_METHOD(Phalcon_Forms_Form, __toString);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, entity)
	ZEND_ARG_INFO(0, userOptions)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_setaction, 0, 0, 1)
	ZEND_ARG_INFO(0, action)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_setmethod, 0, 0, 1)
	ZEND_ARG_INFO(0, method)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_setenctype, 0, 0, 1)
	ZEND_ARG_INFO(0, enctype)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_setoption, 0, 0, 2)
	ZEND_ARG_INFO(0, option)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_getoption, 0, 0, 1)
	ZEND_ARG_INFO(0, option)
	ZEND_ARG_INFO(0, defaultValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_setoptions, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_setentity, 0, 0, 1)
	ZEND_ARG_INFO(0, entity)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_bind, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, entity, IS_OBJECT, 1)
	ZEND_ARG_TYPE_INFO(0, whitelist, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_isvalid, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, entity, IS_OBJECT, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_getmessages, 0, 0, 0)
	ZEND_ARG_INFO(0, byItemName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_getmessagesfor, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_hasmessagesfor, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_add, 0, 0, 1)
	ZEND_ARG_INFO(0, element)
	ZEND_ARG_INFO(0, postion)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_render, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, attributes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_get, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_label, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, attributes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_getlabel, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_getvalue, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, flag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_setvalue, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_getvalues, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, flag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_has, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_remove, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_clear, 0, 0, 0)
	ZEND_ARG_INFO(0, fields)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_appendmessage, 0, 0, 2)
	ZEND_ARG_INFO(0, field)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_forms_form_appendmessages, 0, 0, 2)
	ZEND_ARG_INFO(0, field)
	ZEND_ARG_INFO(0, messages)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_forms_form_method_entry[] = {
	PHP_ME(Phalcon_Forms_Form, __construct, arginfo_phalcon_forms_form___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Forms_Form, setAction, arginfo_phalcon_forms_form_setaction, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getAction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, setMethod, arginfo_phalcon_forms_form_setmethod, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getMethod, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, setEnctype, arginfo_phalcon_forms_form_setenctype, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getEnctype, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, setOption, arginfo_phalcon_forms_form_setoption, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getOption, arginfo_phalcon_forms_form_getoption, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, setOptions, arginfo_phalcon_forms_form_setoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getOptions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, setEntity, arginfo_phalcon_forms_form_setentity, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getEntity, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getElements, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, bind, arginfo_phalcon_forms_form_bind, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, isValid, arginfo_phalcon_forms_form_isvalid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getMessages, arginfo_phalcon_forms_form_getmessages, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getMessagesFor, arginfo_phalcon_forms_form_getmessagesfor, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, hasMessagesFor, arginfo_phalcon_forms_form_hasmessagesfor, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, add, arginfo_phalcon_forms_form_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, render, arginfo_phalcon_forms_form_render, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, get, arginfo_phalcon_forms_form_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, label, arginfo_phalcon_forms_form_label, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getLabel, arginfo_phalcon_forms_form_getlabel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getValue, arginfo_phalcon_forms_form_getvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, setValue, arginfo_phalcon_forms_form_setvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, getValues, arginfo_phalcon_forms_form_getvalues, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, has, arginfo_phalcon_forms_form_has, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, remove, arginfo_phalcon_forms_form_remove, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, clear, arginfo_phalcon_forms_form_clear, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, count, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, rewind, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, current, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, key, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, next, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, appendMessage, arginfo_phalcon_forms_form_appendmessage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, appendMessages, arginfo_phalcon_forms_form_appendmessages, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, toArray, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Forms_Form, __toString, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Forms\Form initializer
 */
PHALCON_INIT_CLASS(Phalcon_Forms_Form){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Forms, Form, forms_form, phalcon_di_injectable_ce, phalcon_forms_form_method_entry, 0);

	zend_declare_property_null(phalcon_forms_form_ce, SL("_position"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_forms_form_ce, SL("_entity"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_forms_form_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_forms_form_ce, SL("_data"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_forms_form_ce, SL("_filterData"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_forms_form_ce, SL("_elements"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_forms_form_ce, SL("_messages"), ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_forms_form_ce, SL("VALUES_RAW"), PHALCON_FROM_VALUES_RAW);
	zend_declare_class_constant_long(phalcon_forms_form_ce, SL("VALUES_AS_ARRAY"), PHALCON_FROM_VALUES_AS_ARRAY);

	zend_class_implements(phalcon_forms_form_ce, 2, spl_ce_Countable, zend_ce_iterator);

	return SUCCESS;
}

/**
 * Phalcon\Forms\Form constructor
 *
 * @param object $entity
 * @param array $userOptions
 */
PHP_METHOD(Phalcon_Forms_Form, __construct){

	zval *entity = NULL, *options = NULL;

	phalcon_fetch_params(0, 0, 2, &entity, &options);

	if (!entity) {
		entity = &PHALCON_GLOBAL(z_null);
	}

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(entity) != IS_NULL) {
		if (Z_TYPE_P(entity) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_forms_exception_ce, "The base entity is not valid");
			return;
		}

		phalcon_update_property(getThis(), SL("_entity"), entity);
	}

	/**
	 * Update the user options
	 */
	if (Z_TYPE_P(options) == IS_ARRAY) {
		phalcon_update_property(getThis(), SL("_options"), options);
	}

	/**
	 * Check for an 'initialize' method and call it
	 */
	if (phalcon_method_exists_ex(getThis(), SL("initialize")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, getThis(), "initialize", entity, options);
	}
}

/**
 * Sets the form's action
 *
 * @param string $action
 * @return Phalcon\Forms\Form
 */
PHP_METHOD(Phalcon_Forms_Form, setAction){

	zval *action, option = {};

	phalcon_fetch_params(0, 1, 0, &action);

	ZVAL_STRING(&option, "action");
	phalcon_update_property_array(getThis(), SL("_options"), &option, action);
	zval_ptr_dtor(&option);

	RETURN_THIS();
}

/**
 * Returns the form's action
 *
 * @return string
 */
PHP_METHOD(Phalcon_Forms_Form, getAction){

	zval options = {}, option = {};

	ZVAL_STRING(&option, "action");
	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(return_value, &options, &option, PH_COPY)) {
		RETVAL_NULL();
	}
	zval_ptr_dtor(&option);
}

/**
 * Sets the form's method
 *
 * @param string $action
 * @return Phalcon\Forms\Form
 */
PHP_METHOD(Phalcon_Forms_Form, setMethod){

	zval *method, option = {};

	phalcon_fetch_params(0, 1, 0, &method);

	ZVAL_STRING(&option, "method");
	phalcon_update_property_array(getThis(), SL("_options"), &option, method);
	zval_ptr_dtor(&option);

	RETURN_THIS();
}

/**
 * Returns the form's method
 *
 * @return string
 */
PHP_METHOD(Phalcon_Forms_Form, getMethod){

	zval options = {}, option = {};

	ZVAL_STRING(&option, "method");
	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(return_value, &options, &option, PH_COPY)) {
		RETVAL_NULL();
	}
	zval_ptr_dtor(&option);
}

/**
 * Sets the form's enctype
 *
 * @param string $action
 * @return Phalcon\Forms\Form
 */
PHP_METHOD(Phalcon_Forms_Form, setEnctype){

	zval *enctype, option = {};

	phalcon_fetch_params(0, 1, 0, &enctype);

	ZVAL_STRING(&option, "enctype");
	phalcon_update_property_array(getThis(), SL("_options"), &option, enctype);
	zval_ptr_dtor(&option);

	RETURN_THIS();
}

/**
 * Returns the form's enctype
 *
 * @return string
 */
PHP_METHOD(Phalcon_Forms_Form, getEnctype){

	zval options = {}, option = {};

	ZVAL_STRING(&option, "enctype");
	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(return_value, &options, &option, PH_COPY)) {
		RETVAL_NULL();
	}
	zval_ptr_dtor(&option);
}

/**
 * Sets an option for the form
 *
 * @param string $option
 * @param mixed $value
 * @return Phalcon\Forms\Form
 */
PHP_METHOD(Phalcon_Forms_Form, setOption){

	zval *option, *value;

	phalcon_fetch_params(0, 2, 0, &option, &value);

	phalcon_update_property_array(getThis(), SL("_options"), option, value);
	RETURN_THIS();
}

/**
 * Returns the value of an option if present
 *
 * @param string $option
 * @param mixed $defaultValue
 * @return mixed
 */
PHP_METHOD(Phalcon_Forms_Form, getOption){

	zval *option, *default_value = NULL, options = {}, value = {};

	phalcon_fetch_params(0, 1, 1, &option, &default_value);

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset_fetch(&value, &options, option, PH_READONLY)) {
		RETURN_CTOR(&value);
	}

	RETURN_CTOR(default_value);
}

/**
 * Sets options for the element
 *
 * @param array $options
 * @return Phalcon\Forms\ElementInterface
 */
PHP_METHOD(Phalcon_Forms_Form, setOptions){

	zval *options;

	phalcon_fetch_params(0, 1, 0, &options);

	phalcon_update_property(getThis(), SL("_options"), options);

	RETURN_THIS();
}

/**
 * Returns the options for the element
 *
 * @return array
 */
PHP_METHOD(Phalcon_Forms_Form, getOptions){


	RETURN_MEMBER(getThis(), "_options");
}

/**
 * Sets the entity related to the model
 *
 * @param object $entity
 * @return Phalcon\Forms\Form
 */
PHP_METHOD(Phalcon_Forms_Form, setEntity){

	zval *entity;

	phalcon_fetch_params(0, 1, 0, &entity);

	if (Z_TYPE_P(entity) != IS_NULL && Z_TYPE_P(entity) != IS_OBJECT) {
		zend_throw_exception_ex(phalcon_forms_exception_ce, 0, "'%s' must be an object or NULL", "entity");
		return;
	}

	phalcon_update_property(getThis(), SL("_entity"), entity);
	RETURN_THIS();
}

/**
 * Returns the entity related to the model
 *
 * @return object
 */
PHP_METHOD(Phalcon_Forms_Form, getEntity){


	RETURN_MEMBER(getThis(), "_entity");
}

/**
 * Returns the form elements added to the form
 *
 * @return Phalcon\Forms\ElementInterface[]
 */
PHP_METHOD(Phalcon_Forms_Form, getElements){


	RETURN_MEMBER(getThis(), "_elements");
}

/**
 * Binds data to the entity
 *
 * @param array $data
 * @param object $entity
 * @param array $whitelist
 * @return Phalcon\Forms\Form
 */
PHP_METHOD(Phalcon_Forms_Form, bind){

	zval *data, *entity, *whitelist = NULL, elements = {}, service_name = {}, dependency_injector = {}, filter = {}, filter_data = {}, *value;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 1, 2, &data, &entity, &whitelist);

	if (!entity) {
		entity = &PHALCON_GLOBAL(z_null);
	}

	if (!whitelist) {
		whitelist = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "setentity", entity);

	phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(elements) != IS_ARRAY) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_forms_exception_ce, "There are no elements in the form");
		return;
	}

	ZVAL_STR(&service_name, IS(filter));

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	PHALCON_MM_VERIFY_INTERFACE(&dependency_injector, phalcon_diinterface_ce);

	PHALCON_MM_CALL_METHOD(&filter, &dependency_injector, "getshared", &service_name);
	PHALCON_MM_ADD_ENTRY(&filter);
	PHALCON_MM_VERIFY_INTERFACE(&filter, phalcon_filterinterface_ce);

	array_init(&filter_data);
	PHALCON_MM_ADD_ENTRY(&filter_data);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, str_key, value) {
		zval key = {}, element = {}, filters = {}, filtered_value = {}, method = {};
		if (str_key) {
			ZVAL_STR(&key, str_key);
		} else {
			ZVAL_LONG(&key, idx);
		}

		if (!phalcon_array_isset_fetch(&element, &elements, &key, PH_READONLY)) {
			continue;
		}

		/**
		 * Check if the item is in the whitelist
		 */
		if (Z_TYPE_P(whitelist) == IS_ARRAY) {
			if (!phalcon_fast_in_array(&key, whitelist)) {
				continue;
			}
		}

		/**
		 * Check if the method has filters
		 */
		PHALCON_MM_CALL_METHOD(&filters, &element, "getfilters");
		PHALCON_MM_ADD_ENTRY(&filters);
		if (zend_is_true(&filters)) {
			/**
			 * Sanitize the filters
			 */
			PHALCON_MM_CALL_METHOD(&filtered_value, &filter, "sanitize", value, &filters);
			PHALCON_MM_ADD_ENTRY(&filtered_value);
		} else {
			ZVAL_COPY_VALUE(&filtered_value, value);
		}

		if (Z_TYPE_P(entity) == IS_OBJECT) {
			PHALCON_CONCAT_SV(&method, "set", &key);
			PHALCON_MM_ADD_ENTRY(&method);
			zend_str_tolower(Z_STRVAL(method), Z_STRLEN(method));

			/**
			 * Use the setter if any available
			 */
			if (phalcon_method_exists(entity, &method) == SUCCESS) {
				PHALCON_MM_CALL_METHOD(NULL, entity, Z_STRVAL(method), &filtered_value);
				continue;
			}

			/**
			 * Use the public property if it doesn't have a setter
			 */
			phalcon_update_property_zval_zval(entity, &key, &filtered_value);
		} else {
			phalcon_array_update(&filter_data, &key, &filtered_value, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();

	phalcon_update_property(getThis(), SL("_filterData"), &filter_data);
	phalcon_update_property(getThis(), SL("_data"), data);
	RETURN_MM();
}

/**
 * Validates the form
 *
 * @param array $data
 * @param object $entity
 * @return boolean
 */
PHP_METHOD(Phalcon_Forms_Form, isValid){

	zval *_data = NULL, *entity = NULL, data = {}, elements = {}, status = {}, not_failed = {}, messages = {}, *element;

	phalcon_fetch_params(0, 0, 2, &_data, &entity);

	if (!entity) {
		entity = &PHALCON_GLOBAL(z_null);
	}

	if (_data) {
		PHALCON_CALL_METHOD(NULL, getThis(), "bind", _data, entity);
	}

	phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(elements) != IS_ARRAY) {
		RETURN_TRUE;
	}

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);

	/**
	 * Check if there is a method 'beforeValidation'
	 */
	if (phalcon_method_exists_ex(getThis(), SL("beforevalidation")) == SUCCESS) {
		PHALCON_CALL_METHOD(&status, getThis(), "beforevalidation", &data, entity);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_CTOR(&status);
		}
	}

	ZVAL_TRUE(&not_failed);

	array_init(&messages);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(elements), element) {
		zval validators = {}, name = {}, prepared_validators = {}, *validator, validation = {}, filters = {}, element_messages = {};
		PHALCON_CALL_METHOD(&validators, element, "getvalidators");
		if (Z_TYPE(validators) == IS_ARRAY) {
			if (phalcon_fast_count_ev(&validators)) {

				/**
				 * Element's name
				 */
				PHALCON_CALL_METHOD(&name, element, "getname");

				/**
				 * Prepare the validators
				 */
				array_init(&prepared_validators);

				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(validators), validator) {
					zval scope = {};
					array_init_size(&scope, 2);
					phalcon_array_append(&scope, &name, PH_COPY);
					phalcon_array_append(&scope, validator, PH_COPY);
					phalcon_array_append(&prepared_validators, &scope, 0);
				} ZEND_HASH_FOREACH_END();

				/**
				 * Create an implicit validation
				 */
				object_init_ex(&validation, phalcon_validation_ce);
				PHALCON_CALL_METHOD(NULL, &validation, "__construct", &prepared_validators);

				/**
				 * Get filters in the element
				 */
				PHALCON_CALL_METHOD(&filters, element, "getfilters");

				/**
				 * Assign the filters to the validation
				 */
				if (Z_TYPE(filters) == IS_ARRAY) {
					PHALCON_CALL_METHOD(NULL, &validation, "setfilters", &name, &filters);
				}
				zval_ptr_dtor(&filters);

				/**
				 * Perform the validation
				 */
				PHALCON_CALL_METHOD(&element_messages, &validation, "validate", &data, entity);
				if (phalcon_fast_count_ev(&element_messages)) {
					phalcon_array_update(&messages, &name, &element_messages, PH_COPY);

					ZVAL_FALSE(&not_failed);
				}
				zval_ptr_dtor(&name);
				zval_ptr_dtor(&validation);
				zval_ptr_dtor(&element_messages);
			}
		}
		zval_ptr_dtor(&validators);
	} ZEND_HASH_FOREACH_END();

	/**
	 * If the validation fails update the messages
	 */
	if (!zend_is_true(&not_failed)) {
		phalcon_update_property(getThis(), SL("_messages"), &messages);
	}

	/**
	 * Check if there is a method 'afterValidation'
	 */
	if (phalcon_method_exists_ex(getThis(), SL("aftervalidation")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, getThis(), "aftervalidation", &messages);
	}
	zval_ptr_dtor(&messages);

	/**
	 * Return the validation status
	 */

	RETURN_CTOR(&not_failed);
}

/**
 * Returns the messages generated in the validation
 *
 * @param boolean $byItemName
 * @return Phalcon\Validation\Message\Group
 */
PHP_METHOD(Phalcon_Forms_Form, getMessages){

	zval *by_item_name = NULL, messages ={};

	phalcon_fetch_params(0, 0, 1, &by_item_name);

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_NOISY|PH_READONLY);
	if (by_item_name && zend_is_true(by_item_name)) {
		if (Z_TYPE(messages) != IS_ARRAY) {
			object_init_ex(return_value, phalcon_validation_message_group_ce);
		} else {
			RETURN_ZVAL(&messages, 1, 0);
		}
	} else {
		object_init_ex(return_value, phalcon_validation_message_group_ce);

		if (Z_TYPE(messages) == IS_ARRAY) {
			zval *v;
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(messages), v) {
				PHALCON_CALL_METHOD(NULL, return_value, "appendmessages", v);
			} ZEND_HASH_FOREACH_END();
		}
	}
}

/**
 * Returns the messages generated for a specific element
 *
 * @return Phalcon\Validation\Message\Group[]
 */
PHP_METHOD(Phalcon_Forms_Form, getMessagesFor){

	zval *name, messages = {}, element_messages = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset_fetch(&element_messages, &messages, name, PH_READONLY)) {
		RETURN_CTOR(&element_messages);
	}

	object_init_ex(return_value, phalcon_validation_message_group_ce);
}

/**
 * Check if messages were generated for a specific element
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Forms_Form, hasMessagesFor){

	zval *name, messages = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_NOISY|PH_READONLY);
	RETURN_BOOL(phalcon_array_isset(&messages, name));
}

/**
 * Adds an element to the form
 *
 * @param Phalcon\Forms\ElementInterface $element
 * @param string $postion
 * @param bool $type If $type is TRUE, the element wile add before $postion, else is after
 * @return Phalcon\Forms\Form
 */
PHP_METHOD(Phalcon_Forms_Form, add){

	zval *element, *pos = NULL, *type = NULL, name = {}, values = {}, elements = {}, elements_indexed = {}, tmp0 = {}, tmp1 = {}, offset = {}, new_elements = {};
	zend_string *str_key;
	ulong idx;
	int found = 0, i = 0;

	phalcon_fetch_params(0, 1, 2, &element, &pos, &type);
	PHALCON_VERIFY_INTERFACE_EX(element, phalcon_forms_elementinterface_ce, phalcon_forms_exception_ce);

	/**
	 * Gets the element's name
	 */
	PHALCON_CALL_METHOD(&name, element, "getname");

	/**
	 * Link the element to the form
	 */
	PHALCON_CALL_METHOD(NULL, element, "setform", getThis());

	if (!pos || Z_TYPE_P(pos) == IS_NULL) {
		/* Append the element by its name */
		phalcon_update_property_array(getThis(), SL("_elements"), &name, element);
		phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);
		phalcon_array_values(&elements_indexed, &elements);
		phalcon_update_property(getThis(), SL("_elementsIndexed"), &elements_indexed);
		zval_ptr_dtor(&elements_indexed);
	} else {
		if (type && zend_is_true(type)) {
			i = -1;
		}

		array_init_size(&values, 1);

		phalcon_array_update(&values, &name, element, PH_COPY);

		phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);

		if (Z_TYPE(elements) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY(Z_ARRVAL(elements), idx, str_key) {
				zval key = {};
				if (str_key) {
					ZVAL_STR(&key, str_key);
				} else {
					ZVAL_LONG(&key, idx);
				}
				++i;
				if (phalcon_is_equal(&key, pos)) {
					found = 1;
					break;
				}
			} ZEND_HASH_FOREACH_END();
		}

		if (!found) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_forms_exception_ce, "Array position does not exist");
			return;
		}

		ZVAL_LONG(&offset, i);

		PHALCON_CALL_FUNCTION(&tmp0, "array_slice", &elements, &PHALCON_GLOBAL(z_zero), &offset, &PHALCON_GLOBAL(z_true));
		PHALCON_CALL_FUNCTION(&tmp1, "array_slice", &elements, &offset, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));

		array_init(&new_elements);

		phalcon_array_merge_recursive_n(&new_elements, &tmp0);
		phalcon_array_merge_recursive_n(&new_elements, &values);
		phalcon_array_merge_recursive_n(&new_elements, &tmp1);
		zval_ptr_dtor(&values);
		zval_ptr_dtor(&tmp0);
		zval_ptr_dtor(&tmp1);

		phalcon_update_property(getThis(), SL("_elements"), &new_elements);
		phalcon_array_values(&elements_indexed, &new_elements);
		zval_ptr_dtor(&new_elements);

		phalcon_update_property(getThis(), SL("_elementsIndexed"), &elements_indexed);
		zval_ptr_dtor(&elements_indexed);
	}
	zval_ptr_dtor(&name);

	RETURN_THIS();
}

/**
 * Renders a specific item in the form
 *
 * @param string $name
 * @param array $attributes
 * @param string $template
 * @return string
 */
PHP_METHOD(Phalcon_Forms_Form, render){

	zval *name = NULL, *attributes = NULL, *template = NULL, elements = {}, element = {}, label ={}, element_str = {};
	zval replace_pairs = {}, options = {}, *el;

	phalcon_fetch_params(0, 0, 3, &name, &attributes, &template);

	if (!attributes) {
		attributes = &PHALCON_GLOBAL(z_null);
	}

	if (!template) {
		template = &PHALCON_GLOBAL(z_null);
	}

	if (name && PHALCON_IS_NOT_EMPTY(name)) {
		PHALCON_ENSURE_IS_STRING(name);

		phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);
		if (!phalcon_array_isset_fetch(&element, &elements, name, PH_READONLY)) {
			zend_throw_exception_ex(phalcon_forms_exception_ce, 0, "Element with ID=%s is not a part of the form", Z_STRVAL_P(name));
			return;
		}

		if (PHALCON_IS_NOT_EMPTY(template)) {
			PHALCON_CALL_METHOD(&label, getThis(), "getlabel", name);
			PHALCON_CALL_METHOD(&element_str, &element, "render", attributes);

			array_init_size(&replace_pairs, 2);
			phalcon_array_update_str(&replace_pairs, SL(":label:"), &label, 0);
			phalcon_array_update_str(&replace_pairs, SL(":element:"), &element_str, 0);

			phalcon_strtr_array(return_value, template, &replace_pairs);
			zval_ptr_dtor(&replace_pairs);
		} else {
			PHALCON_CALL_METHOD(return_value, &element, "render", attributes);
		}
	} else {
		phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);

		PHALCON_CALL_CE_STATIC(return_value, phalcon_tag_ce, "form", &options);

		phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);

		if (Z_TYPE(elements) == IS_ARRAY) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(elements), el) {
				zval str = {}, el_name = {}, el_label = {}, el_replace_pairs = {};

				PHALCON_CALL_METHOD(&str, el, "render", attributes);
				if (PHALCON_IS_NOT_EMPTY(template)) {
					zval tmp = {};
					PHALCON_CALL_METHOD(&el_name, el, "getname");
					PHALCON_CALL_METHOD(&el_label, getThis(), "getlabel", &el_name);
					zval_ptr_dtor(&el_name);

					array_init_size(&el_replace_pairs, 2);
					phalcon_array_update_str(&el_replace_pairs, SL(":label:"), &el_label, 0);
					phalcon_array_update_str(&el_replace_pairs, SL(":element:"), &str, PH_COPY);

					phalcon_strtr_array(&tmp, template, &el_replace_pairs);
					zval_ptr_dtor(&el_replace_pairs);
					phalcon_concat_self(return_value, &tmp);
					zval_ptr_dtor(&tmp);
				} else {
					phalcon_concat_self(return_value, &str);
				}
				zval_ptr_dtor(&str);
			} ZEND_HASH_FOREACH_END();
		}

		phalcon_concat_self_str(return_value, SL("</form>"));
	}
}

/**
 * Returns an element added to the form by its name
 *
 * @param string $name
 * @return Phalcon\Forms\ElementInterface
 */
PHP_METHOD(Phalcon_Forms_Form, get){

	zval *name, elements = {}, element = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&element, &elements, name, PH_READONLY)) {
		PHALCON_ENSURE_IS_STRING(name);
		zend_throw_exception_ex(phalcon_forms_exception_ce, 0, "Element with ID=%s is not a part of the form", Z_STRVAL_P(name));
		return;
	}

	RETURN_CTOR(&element);
}

/**
 * Generate the label of a element added to the form including HTML
 *
 * @param string $name
 * @return string
 */
PHP_METHOD(Phalcon_Forms_Form, label){

	zval *name, *attributes = NULL, elements = {}, element = {};

	phalcon_fetch_params(0, 1, 1, &name, &attributes);

	if (!attributes) {
		attributes = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&element, &elements, name, PH_READONLY)) {
		PHALCON_ENSURE_IS_STRING(name);
		zend_throw_exception_ex(phalcon_forms_exception_ce, 0, "Element with ID=%s is not a part of the form", Z_STRVAL_P(name));
		return;
	}

	PHALCON_RETURN_CALL_METHOD(&element, "label", attributes);
}

/**
 * Returns a label for an element
 *
 * @param string $name
 * @return string
 */
PHP_METHOD(Phalcon_Forms_Form, getLabel){

	zval *name, elements = {}, element = {}, label = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&element, &elements, name, PH_READONLY)) {
		PHALCON_ENSURE_IS_STRING(name);
		zend_throw_exception_ex(phalcon_forms_exception_ce, 0, "Element with ID=%s is not a part of the form", Z_STRVAL_P(name));
		return;
	}

	PHALCON_CALL_METHOD(&label, &element, "getlabel");

	/* Use the element's name as label if the label is not available */
	if (!zend_is_true(&label)) {
		RETURN_CTOR(name);
	}

	RETURN_ZVAL(&label, 0, 0);
}

/**
 * Gets a value from the internal related entity or from the default value
 *
 * @param string $name
 * @return mixed
 */
PHP_METHOD(Phalcon_Forms_Form, getValue){

	zval *name, *flag = NULL;

	phalcon_fetch_params(0, 1, 1, &name, &flag);

	if (!flag) {
		PHALCON_RETURN_CALL_SELF("getvalues", name, &PHALCON_GLOBAL(z_zero));
	} else {
		PHALCON_RETURN_CALL_SELF("getvalues", name, flag);
	}
}

/**
 * Sets a value
 *
 * @param string $name
 * @param mixed $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Forms_Form, setValue){

	zval *name, *value = NULL, entity = {};

	phalcon_fetch_params(0, 1, 1, &name, &value);

	if (!value) {
		value = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&entity, getThis(), SL("_entity"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(entity) == IS_OBJECT) {
		zval method = {};
		/**
		 * Check if the entity has a getter
		 */
		PHALCON_CONCAT_SV(&method, "set", name);
		phalcon_strtolower_inplace(&method);
		if (phalcon_method_exists(&entity, &method) == SUCCESS) {
			PHALCON_CALL_METHOD(NULL, &entity, Z_STRVAL(method), value);
			zval_ptr_dtor(&method);
			return;
		}
		zval_ptr_dtor(&method);

		if (!phalcon_isset_property_zval(&entity, name) && phalcon_method_exists_ex(&entity, SL("__set")) == SUCCESS) {
			PHALCON_CALL_METHOD(NULL, &entity, "__set", name, value);
		} else {
			phalcon_update_property_zval_zval(&entity, name, value);
		}
		return;
	}

	phalcon_update_property_array(getThis(), SL("_filterData"), name, value);
}

/**
 * Gets a values
 *
 * @param string $name
 * @return mixed
 */
PHP_METHOD(Phalcon_Forms_Form, getValues){

	zval *name = NULL, *flag = NULL, data = {}, entity = {}, method = {}, value = {}, filter_data = {};
	int f = 0;

	phalcon_fetch_params(0, 0, 2, &name, &flag);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (flag) {
		f = phalcon_get_intval(flag);
	}

	if ((f & PHALCON_FROM_VALUES_RAW) == PHALCON_FROM_VALUES_RAW) {
		phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
		if (PHALCON_IS_EMPTY(name)) {
			RETURN_CTOR(&data);
		}

		if (Z_TYPE(data) == IS_ARRAY) {
			/**
			 * Check if the data is in the data array
			 */
			if (phalcon_array_isset_fetch(&value, &data, name, PH_READONLY)) {
				RETURN_CTOR(&value);
			}
		}

		RETURN_NULL();
	}

	phalcon_read_property(&entity, getThis(), SL("_entity"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(entity) == IS_OBJECT) {
		if (PHALCON_IS_EMPTY(name)) {
			if ((f & PHALCON_FROM_VALUES_AS_ARRAY) == PHALCON_FROM_VALUES_AS_ARRAY) {
				phalcon_get_object_vars(return_value, &entity, 1);
				return;
			} else {
				RETURN_CTOR(&entity);
			}
		}

		/**
		 * Check if the entity has a getter
		 */
		PHALCON_CONCAT_SV(&method, "get", name);
		phalcon_strtolower_inplace(&method);
		if (phalcon_method_exists(&entity, &method) == SUCCESS) {
			PHALCON_RETURN_CALL_METHOD(&entity, Z_STRVAL(method));
			zval_ptr_dtor(&method);
			return;
		}
		zval_ptr_dtor(&method);

		/**
		 * Check if the entity has a public property
		 */
		if (phalcon_property_isset_fetch_zval(&value, &entity, name, PH_READONLY)) {
			RETURN_CTOR(&value);
		}
	}

	phalcon_read_property(&filter_data, getThis(), SL("_filterData"), PH_NOISY|PH_READONLY);

	if (PHALCON_IS_EMPTY(name)) {
		RETURN_CTOR(&filter_data);
	} else if (Z_TYPE(filter_data) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&value, &filter_data, name, PH_READONLY)) {
			RETURN_CTOR(&value);
		}
	}

	RETURN_NULL();
}

/**
 * Check if the form contains an element
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Forms_Form, has){

	zval *name, elements = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);
	RETURN_BOOL(phalcon_array_isset(&elements, name));
}

/**
 * Removes an element from the form
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Forms_Form, remove){

	zval *name, elements = {}, elements_indexed = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);

	if (phalcon_isset_property_array(getThis(), SL("_elements"), name)) {
		phalcon_unset_property_array(getThis(), SL("_elements"), name);

		phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);
		phalcon_array_values(&elements_indexed, &elements);
		phalcon_update_property(getThis(), SL("_elementsIndexed"), &elements_indexed);
		zval_ptr_dtor(&elements_indexed);
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Clears every element in the form to its default value
 *
 * @param array $fields
 * @return Phalcon\Forms\Form
 */
PHP_METHOD(Phalcon_Forms_Form, clear){

	zval *fields = NULL, elements = {}, *element, name = {};

	phalcon_fetch_params(0, 0, 1, &fields);

	if (!fields) {
		fields = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(elements) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(elements), element) {
			if (Z_TYPE_P(fields) != IS_ARRAY) {
				PHALCON_CALL_METHOD(NULL, element, "clear");
			} else {
				PHALCON_CALL_METHOD(&name, element, "getname");
				if (phalcon_fast_in_array(&name, fields)) {
					PHALCON_CALL_METHOD(NULL, element, "clear");
				}
				zval_ptr_dtor(&name);
			}
		} ZEND_HASH_FOREACH_END();

	}

	RETURN_THIS();
}

/**
 * Returns the number of elements in the form
 *
 * @return int
 */
PHP_METHOD(Phalcon_Forms_Form, count){

	zval elements = {};
	long int count;

	phalcon_read_property(&elements, getThis(), SL("_elementsIndexed"), PH_NOISY|PH_READONLY);
	count = (Z_TYPE(elements) == IS_ARRAY) ? zend_hash_num_elements(Z_ARRVAL(elements)) : 0;

	RETURN_LONG(count);
}

/**
 * Rewinds the internal iterator
 */
PHP_METHOD(Phalcon_Forms_Form, rewind){

	zval elements = {}, elements_indexed = {};

	phalcon_read_property(&elements, getThis(), SL("_elementsIndexed"), PH_NOISY|PH_READONLY);
	phalcon_update_property_long(getThis(), SL("_position"), 0);
	phalcon_array_values(&elements_indexed, &elements);
	phalcon_update_property(getThis(), SL("_elementsIndexed"), &elements_indexed);
	zval_ptr_dtor(&elements_indexed);
}

/**
 * Returns the current element in the iterator
 *
 * @return Phalcon\Forms\ElementInterface
 */
PHP_METHOD(Phalcon_Forms_Form, current){

	zval elements = {}, position = {};

	phalcon_read_property(&elements, getThis(), SL("_elementsIndexed"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(elements) == IS_ARRAY) {
		phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY|PH_READONLY);
		if (phalcon_array_isset_fetch(return_value, &elements, &position, PH_COPY)) {
			return;
		}
	}

	RETURN_NULL();
}

/**
 * Returns the current position/key in the iterator
 *
 * @return int
 */
PHP_METHOD(Phalcon_Forms_Form, key){

	RETURN_MEMBER(getThis(), "_position");
}

/**
 * Moves the internal iteration pointer to the next position
 *
 */
PHP_METHOD(Phalcon_Forms_Form, next){

	phalcon_property_incr(getThis(), SL("_position"));
}

/**
 * Check if the current element in the iterator is valid
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Forms_Form, valid){

	zval elements = {}, position = {};

	phalcon_read_property(&elements, getThis(), SL("_elementsIndexed"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(elements) == IS_ARRAY) {
		phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY|PH_READONLY);
		if (phalcon_array_isset(&elements, &position)) {
			RETURN_TRUE;
		}
	}

	RETURN_FALSE
}

/**
 * Appends a message to the form
 *
 *<code>
 * $form->appendMessage('email', new Phalcon\Validation\Message('Must be not empty '));
 *</code>
 *
 * @param string $field
 * @param Phalcon\Validation\Message $message
 * @return Phalcon\Forms\Form
 */
PHP_METHOD(Phalcon_Forms_Form, appendMessage){

	zval *filed, *message, current_messages = {}, element_messages = {};

	phalcon_fetch_params(0, 2, 0, &filed, &message);

	phalcon_read_property(&current_messages, getThis(), SL("_messages"), PH_COPY);
	if (Z_TYPE(current_messages) != IS_ARRAY) {
		array_init(&current_messages);
	}

	if (!phalcon_array_isset_fetch(&element_messages, &current_messages, filed, PH_COPY)) {
		object_init_ex(&element_messages, phalcon_validation_message_group_ce);
		PHALCON_CALL_METHOD(NULL, &element_messages, "__construct");
	}

	PHALCON_CALL_METHOD(NULL, &element_messages, "appendmessage", message);

	phalcon_array_update(&current_messages, filed, &element_messages, 0);
	phalcon_update_property(getThis(), SL("_messages"), &current_messages);
	zval_ptr_dtor(&current_messages);

	RETURN_THIS();
}

/**
 * Appends a messages to the form
 *
 *<code>
 * $form->appendMessages('email', array(new Phalcon\Validation\Message('Must be not empty '), new Phalcon\Validation\Message('Must be an email address')));
 *</code>
 *
 * @param string $field
 * @param Phalcon\Validation\MessageInterface[] $messages
 * @return Phalcon\Forms\Form
 */
PHP_METHOD(Phalcon_Forms_Form, appendMessages){

	zval *filed, *messages, current_messages = {}, element_messages = {};

	phalcon_fetch_params(0, 2, 0, &filed, &messages);

	phalcon_read_property(&current_messages, getThis(), SL("_messages"), PH_READONLY);
	if (Z_TYPE(current_messages) != IS_ARRAY) {
		array_init(&current_messages);
		phalcon_update_property(getThis(), SL("_messages"), &current_messages);
		zval_ptr_dtor(&current_messages);
	}

	if (!phalcon_array_isset_fetch(&element_messages, &current_messages, filed, PH_READONLY)) {
		object_init_ex(&element_messages, phalcon_validation_message_group_ce);
		PHALCON_CALL_METHOD(NULL, &element_messages, "__construct");
		phalcon_array_update(&current_messages, filed, &element_messages, 0);
	}

	PHALCON_CALL_METHOD(NULL, &element_messages, "appendmessages", messages);

	RETURN_THIS();
}

/**
 * Returns a form elements as an array
 *
 * @return array
 */
PHP_METHOD(Phalcon_Forms_Form, toArray){

	zval options = {}, elements = {}, *element;

	array_init(return_value);

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);
	if (!PHALCON_IS_EMPTY(&options)) {
		phalcon_array_update_str(return_value, SL("options"), &options, PH_COPY);
	}

	phalcon_read_property(&elements, getThis(), SL("_elements"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(elements) == IS_ARRAY) {
		zval index = {};
		ZVAL_STRING(&index, "elements");
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(elements), element) {
			zval name ={}, value = {};

			PHALCON_CALL_METHOD(&name, element, "getname");
			PHALCON_CALL_METHOD(&value, element, "toarray");

			phalcon_array_update_multi_2(return_value, &index, &name, &value, 0);
			zval_ptr_dtor(&name);
		} ZEND_HASH_FOREACH_END();
		zval_ptr_dtor(&index);
	}
}

/**
 * Renders the form html
 *
 * @return string
 */
PHP_METHOD(Phalcon_Forms_Form, __toString){

	PHALCON_CALL_METHOD(return_value, getThis(), "render");
}
