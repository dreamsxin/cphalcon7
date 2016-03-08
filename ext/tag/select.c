
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

#include "tag/select.h"
#include "tag.h"
#include "tag/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/fcall.h"
#include "kernel/hash.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/exception.h"
#include "kernel/object.h"

/**
 * Phalcon\Tag\Select
 *
 * Generates a SELECT html tag using a static array of values or a Phalcon\Mvc\Model resultset
 */
zend_class_entry *phalcon_tag_select_ce;

PHP_METHOD(Phalcon_Tag_Select, selectField);
PHP_METHOD(Phalcon_Tag_Select, _optionsFromResultset);
PHP_METHOD(Phalcon_Tag_Select, _optionsFromArray);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_select_selectfield, 0, 0, 1)
	ZEND_ARG_INFO(0, parameters)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_tag_select_method_entry[] = {
	PHP_ME(Phalcon_Tag_Select, selectField, arginfo_phalcon_tag_select_selectfield, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag_Select, _optionsFromResultset, NULL, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag_Select, _optionsFromArray, NULL, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Tag\Select initializer
 */
PHALCON_INIT_CLASS(Phalcon_Tag_Select){

	PHALCON_REGISTER_CLASS(Phalcon\\Tag, Select, tag_select, phalcon_tag_select_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	return SUCCESS;
}

/**
 * Generates a SELECT tag
 *
 * @param array $parameters
 * @param array $data
 */
PHP_METHOD(Phalcon_Tag_Select, selectField){

	zval *parameters, *data = NULL, params, default_params, id, name, value;
	zval use_empty, empty_value, empty_text, code;
	zval close_option, options, using;
	zval resultset_options, array_options;

	phalcon_fetch_params(0, 1, 1, &parameters, &data);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(parameters) != IS_ARRAY) { 
		array_init_size(&params, 2);
		phalcon_array_append(&params, parameters, PH_COPY);
		phalcon_array_append(&params, data, PH_COPY);
	} else {
		PHALCON_CPY_WRT_CTOR(&params, parameters);
	}

	phalcon_return_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"));
	if (Z_TYPE(default_params) == IS_ARRAY) { 
		phalcon_array_merge_recursive_n2(&params, &default_params);
	}

	if (!phalcon_array_isset_fetch_long(&id, &params, 0)) {
		phalcon_array_fetch_str(&id, &params, SL("id"), PH_NOISY);
	}

	if (!phalcon_array_isset_fetch_str(&name, &params, SL("name"))) {
		phalcon_array_update_str(&params, SL("name"), &id, PH_COPY);
	} else {
		if (!zend_is_true(&name)) {
			phalcon_array_update_str(&params, SL("name"), &id, PH_COPY);
		}
	}

	/** 
	 * Automatically assign the id if the name is not an array
	 */
	if (!phalcon_memnstr_str(&id, SL("["))) {
		if (!phalcon_array_isset_str(&params, SL("id"))) {
			phalcon_array_update_str(&params, SL("id"), &id, PH_COPY);
		}
	}

	if (!phalcon_array_isset_fetch_str(&value, &params, SL("value"))) {
		PHALCON_CALL_CE_STATIC(&value, phalcon_tag_ce, "getvalue", &id, &params);
	} else {
		phalcon_array_unset_str(&params, SL("value"), PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&use_empty, &params, SL("useEmpty"))) {
		if (!phalcon_array_isset_fetch_str(&empty_value, &params, SL("emptyValue"))) {
			ZVAL_EMPTY_STRING(&empty_value);
		} else {
			phalcon_array_unset_str(&params, SL("emptyValue"), PH_COPY);
		}
		if (!phalcon_array_isset_fetch_str(&empty_text, &params, SL("emptyText"))) {
			ZVAL_STRING(&empty_text, "Choose...");
		} else {
			phalcon_array_unset_str(&params, SL("emptyText"), PH_COPY);
		}

		phalcon_array_unset_str(&params, SL("useEmpty"), PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&using, &params, SL("using"))) {
		phalcon_array_unset_str(&params, SL("using"), PH_COPY);
	}

	ZVAL_STRING(&code, "<select");

	phalcon_tag_render_attributes(&code, &params);

	phalcon_concat_self_str(&code, SL(">" PHP_EOL));

	ZVAL_STRING(&close_option, "</option>" PHP_EOL);

	if (zend_is_true(&use_empty)) {
		/** 
		 * Create an empty value
		 */
		PHALCON_SCONCAT_SVSVV(&code, "\t<option value=\"", &empty_value, "\">", &empty_text, &close_option);
	}

	if (!phalcon_array_isset_fetch_long(&options, &params, 1)) {
		PHALCON_CPY_WRT_CTOR(&options, data);
	}

	if (Z_TYPE(options) == IS_OBJECT) {
		/** 
		 * The options is a resultset
		 */
		if (Z_TYPE(using) < IS_NULL) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_tag_exception_ce, "The 'using' parameter is required");
			return;
		}

		if (Z_TYPE(using) != IS_ARRAY && Z_TYPE(using) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_tag_exception_ce, "The 'using' parameter should be an Array");
			return;
		}

		/** 
		 * Create the SELECT's option from a resultset
		 */
		PHALCON_CALL_SELFW(&resultset_options, "_optionsfromresultset", &options, &using, &value, &close_option);
		phalcon_concat_self(&code, &resultset_options);
	} else if (Z_TYPE(options) == IS_ARRAY) {
		/**
		 * Create the SELECT's option from an array
		 */
		PHALCON_CALL_SELFW(&array_options, "_optionsfromarray", &options, &value, &close_option);
		phalcon_concat_self(&code, &array_options);
	} else {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_tag_exception_ce, "Invalid data provided to SELECT helper");
		return;
	}

	phalcon_concat_self_str(&code, SL("</select>"));

	RETURN_CTORW(&code);
}

/**
 * Generate the OPTION tags based on a resulset
 *
 * @param Phalcon\Mvc\Model $resultset
 * @param array $using
 * @param mixed value
 * @param string $closeOption
 */
PHP_METHOD(Phalcon_Tag_Select, _optionsFromResultset){

	zval *resultset, *using, *value, *close_option, code;

	phalcon_fetch_params(0, 4, 0, &resultset, &using, &value, &close_option);

	if (Z_TYPE_P(value) != IS_ARRAY) { 
		PHALCON_SEPARATE_PARAM(value);
		convert_to_string(value);
	}

	PHALCON_CALL_METHODW(NULL, resultset, "rewind");

	while (1) {
		zval r0, option, using_zero, using_one, option_value, option_text, escaped, params, code_option;

		PHALCON_CALL_METHODW(&r0, resultset, "valid");
		if (PHALCON_IS_NOT_FALSE(&r0)) {
		} else {
			break;
		}

		/** 
		 * Get the current option
		 */
		PHALCON_CALL_METHODW(&option, resultset, "current");
		if (Z_TYPE_P(using) == IS_ARRAY) {
			phalcon_array_fetch_long(&using_zero, using, 0, PH_NOISY);
			phalcon_array_fetch_long(&using_one, using, 1, PH_NOISY);
			if (Z_TYPE(option) == IS_OBJECT) {
				if (phalcon_method_exists_ex(&option, SL("readattribute")) == SUCCESS) {
					/** 
					 * Read the value attribute from the model
					 */
					PHALCON_CALL_METHODW(&option_value, &option, "readattribute", &using_zero);

					/** 
					 * Read the text attribute from the model
					 */
					PHALCON_CALL_METHOD(&option_text, &option, "readattribute", &using_one);
				} else {
					/** 
					 * Read the variable directly from the model/object
					 */
					phalcon_return_property_zval(&option_value, &option, &using_zero);

					/** 
					 * Read the text directly from the model/object
					 */
					phalcon_return_property_zval(&option_text, &option, &using_one);
				}
			} else {
				if (Z_TYPE(option) == IS_ARRAY) { 
					/** 
					 * Read the variable directly from the model/object
					 */
					phalcon_array_fetch(&option_value, &option, &using_zero, PH_NOISY);

					/** 
					 * Read the text directly from the model/object
					 */
					phalcon_array_fetch(&option_text, &option, &using_one, PH_NOISY);
				} else {
					PHALCON_THROW_EXCEPTION_STRW(phalcon_tag_exception_ce, "Resultset returned an invalid value");
					return;
				}
			}

			/** 
			 * If the value is equal to the option's value we mark it as selected
			 */
			phalcon_htmlspecialchars(&escaped, &option_value, NULL, NULL);
			if (Z_TYPE_P(value) == IS_ARRAY) { 
				if (phalcon_fast_in_array(&option_value, value)) {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option selected=\"selected\" value=\"", &escaped, "\">", &option_text, close_option);
				} else {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option value=\"", &escaped, "\">", &option_text, close_option);
				}
			} else {
				convert_to_string(&option_value);
				if (PHALCON_IS_IDENTICAL(&option_value, value)) {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option selected=\"selected\" value=\"", &escaped, "\">", &option_text, close_option);
				} else {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option value=\"", &escaped, "\">", &option_text, close_option);
				}
			}
		} else {
			/** 
			 * Check if using is a closure
			 */
			if (Z_TYPE_P(using) == IS_OBJECT) {
				if (Z_TYPE(params) <= IS_NULL) {
					array_init(&params);
				}
				phalcon_array_update_long(&params, 0, &option, PH_COPY);

				PHALCON_CALL_USER_FUNC_ARRAYW(&code_option, using, &params);
				phalcon_concat_self(&code, &code_option);
			}
		}

		PHALCON_CALL_METHODW(NULL, resultset, "next");
	}

	RETURN_CTORW(&code);
}

/**
 * Generate the OPTION tags based on an array
 *
 * @param Phalcon\Mvc\ModelInterface $resultset
 * @param array $using
 * @param mixed value
 * @param string $closeOption
 */
PHP_METHOD(Phalcon_Tag_Select, _optionsFromArray){

	zval *data, *value, *close_option, code, *option_text;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 3, 0, &data, &value, &close_option);

	if (Z_TYPE_P(value) != IS_ARRAY) { 
		PHALCON_SEPARATE_PARAM(value);
		convert_to_string(value);
	}

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, str_key, option_text) {
		zval option_value, escaped, array_options;
		if (str_key) {
			ZVAL_STR(&option_value, str_key);
		} else {
			ZVAL_LONG(&option_value, idx);
		}

		if (Z_TYPE_P(option_text) == IS_ARRAY) {
			phalcon_htmlspecialchars(&escaped, &option_value, NULL, NULL);

			PHALCON_CALL_SELFW(&array_options, "_optionsfromarray", option_text, value, close_option);

			PHALCON_SCONCAT_SVSVS(&code, "\t<optgroup label=\"", &escaped, "\">" PHP_EOL, &array_options, "\t</optgroup>" PHP_EOL);
		} else {
			phalcon_htmlspecialchars(&escaped, &option_value, NULL, NULL);
		
			if (Z_TYPE_P(value) == IS_ARRAY) { 
				if (phalcon_fast_in_array(&option_value, value)) {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option selected=\"selected\" value=\"", &escaped, "\">", option_text, close_option);
				} else {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option value=\"", &escaped, "\">", option_text, close_option);
				}
			} else {
				convert_to_string(&option_value);
				if (PHALCON_IS_EQUAL(&option_value, value)) {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option selected=\"selected\" value=\"", &escaped, "\">", option_text, close_option);
				} else {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option value=\"", &escaped, "\">", option_text, close_option);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_CTORW(&code);
}
