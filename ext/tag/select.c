
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
PHP_METHOD(Phalcon_Tag_Select, selectField)
{
	zval *parameters, *data = NULL, params = {}, default_params = {}, id = {}, name = {}, value = {}, use_empty = {}, empty_value = {};
	zval empty_text = {}, code = {}, close_option = {}, options = {}, using = {}, resultset_options = {}, array_options = {};

	phalcon_fetch_params(0, 1, 1, &parameters, &data);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(parameters) != IS_ARRAY) {
		array_init_size(&params, 2);
		phalcon_array_append(&params, parameters, PH_COPY);
		phalcon_array_append(&params, data, PH_COPY);
	} else {
		ZVAL_DUP(&params, parameters);
	}

	phalcon_read_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"), PH_READONLY);
	if (Z_TYPE(default_params) == IS_ARRAY) {
		phalcon_array_merge_recursive_n(&params, &default_params);
	}

	if (!phalcon_array_isset_fetch_long(&id, &params, 0, PH_READONLY)) {
		phalcon_array_fetch_str(&id, &params, SL("id"), PH_NOISY|PH_READONLY);
	}

	if (!phalcon_array_isset_fetch_str(&name, &params, SL("name"), PH_READONLY)) {
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

	if (!phalcon_array_isset_fetch_str(&value, &params, SL("value"), PH_COPY)) {
		PHALCON_CALL_CE_STATIC(&value, phalcon_tag_ce, "getvalue", &id, &params);
	} else {
		phalcon_array_unset_str(&params, SL("value"), 0);
	}

	if (phalcon_array_isset_fetch_str(&use_empty, &params, SL("useEmpty"), PH_READONLY) && zend_is_true(&use_empty)) {
		if (!phalcon_array_isset_fetch_str(&empty_value, &params, SL("emptyValue"), PH_COPY)) {
			ZVAL_EMPTY_STRING(&empty_value);
		} else {
			phalcon_array_unset_str(&params, SL("emptyValue"), 0);
		}
		if (!phalcon_array_isset_fetch_str(&empty_text, &params, SL("emptyText"), PH_COPY)) {
			ZVAL_STRING(&empty_text, "Choose...");
		} else {
			phalcon_array_unset_str(&params, SL("emptyText"), 0);
		}

		phalcon_array_unset_str(&params, SL("useEmpty"), 0);
	}

	if (phalcon_array_isset_fetch_str(&using, &params, SL("using"), PH_COPY)) {
		phalcon_array_unset_str(&params, SL("using"), 0);
	} else {
		ZVAL_NULL(&using);
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
	zval_ptr_dtor(&empty_value);
	zval_ptr_dtor(&empty_text);

	if (!phalcon_array_isset_fetch_long(&options, &params, 1, PH_READONLY)) {
		ZVAL_COPY_VALUE(&options, data);
	}

	if (Z_TYPE(options) == IS_OBJECT) {
		/**
		 * The options is a resultset
		 */
		if (Z_TYPE(using) < IS_NULL) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_tag_exception_ce, "The 'using' parameter is required");
			zval_ptr_dtor(&params);
			zval_ptr_dtor(&close_option);
			zval_ptr_dtor(&value);
			return;
		}

		if (Z_TYPE(using) != IS_ARRAY && Z_TYPE(using) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_tag_exception_ce, "The 'using' parameter should be an Array");
			zval_ptr_dtor(&params);
			zval_ptr_dtor(&close_option);
			zval_ptr_dtor(&value);
			zval_ptr_dtor(&using);
			return;
		}

		/**
		 * Create the SELECT's option from a resultset
		 */
		PHALCON_CALL_CE_STATIC(&resultset_options, phalcon_tag_select_ce, "_optionsfromresultset", &options, &using, &value, &close_option);
		phalcon_concat_self(&code, &resultset_options);
		zval_ptr_dtor(&resultset_options);
	} else if (Z_TYPE(options) == IS_ARRAY) {
		/**
		 * Create the SELECT's option from an array
		 */
		PHALCON_CALL_CE_STATIC(&array_options, phalcon_tag_select_ce, "_optionsfromarray",  &options, &using, &value, &close_option);
		phalcon_concat_self(&code, &array_options);
		zval_ptr_dtor(&array_options);
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_tag_exception_ce, "Invalid data provided to SELECT helper");
		zval_ptr_dtor(&params);
		zval_ptr_dtor(&close_option);
		zval_ptr_dtor(&value);
		zval_ptr_dtor(&using);
		return;
	}
	zval_ptr_dtor(&params);
	zval_ptr_dtor(&close_option);
	zval_ptr_dtor(&value);
	zval_ptr_dtor(&using);

	phalcon_concat_self_str(&code, SL("</select>"));

	RETURN_ZVAL(&code, 0, 0);
}

/**
 * Generate the OPTION tags based on a resulset
 *
 * @param Phalcon\Mvc\Model $resultset
 * @param array $using
 * @param mixed value
 * @param string $closeOption
 */
PHP_METHOD(Phalcon_Tag_Select, _optionsFromResultset)
{
	zval *resultset, *using, *value, *close_option, using_zero = {}, using_one = {}, code = {};

	phalcon_fetch_params(0, 4, 0, &resultset, &using, &value, &close_option);

	if (Z_TYPE_P(value) != IS_ARRAY) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_string(value);
	}

	if (Z_TYPE_P(using) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_long(&using_zero, using, 0, PH_COPY)) {
			ZVAL_STRING(&using_zero, "id");
		}
		if (!phalcon_array_isset_fetch_long(&using_one, using, 1, PH_READONLY)) {
			ZVAL_COPY_VALUE(&using_one, &using_zero);
		}
	}

	PHALCON_CALL_METHOD(NULL, resultset, "rewind");

	while (1) {
		zval r0 = {}, option = {}, option_value = {}, option_text = {}, escaped = {}, params = {}, code_option = {};

		PHALCON_CALL_METHOD(&r0, resultset, "valid");
		if (!PHALCON_IS_NOT_FALSE(&r0)) {
			break;
		}

		/**
		 * Get the current option
		 */
		PHALCON_CALL_METHOD(&option, resultset, "current");
		if (Z_TYPE_P(using) == IS_ARRAY) {
			if (Z_TYPE(option) == IS_OBJECT) {
				if (phalcon_method_exists_ex(&option, SL("readattribute")) == SUCCESS) {
					if (Z_TYPE(using_zero) == IS_OBJECT) {
						array_init(&params);
						phalcon_array_update_long(&params, 0, &option, PH_COPY);

						PHALCON_CALL_USER_FUNC_ARRAY(&option_value, &using_zero, &params);
						zval_ptr_dtor(&params);
					} else {
						/**
						 * Read the value attribute from the model
						 */
						PHALCON_CALL_METHOD(&option_value, &option, "readattribute", &using_zero);
					}

					if (Z_TYPE(using_one) == IS_OBJECT) {
						array_init(&params);
						phalcon_array_update_long(&params, 0, &option, PH_COPY);

						PHALCON_CALL_USER_FUNC_ARRAY(&option_text, &using_one, &params);
						zval_ptr_dtor(&params);
					} else {
						/**
						 * Read the text attribute from the model
						 */
						PHALCON_CALL_METHOD(&option_text, &option, "readattribute", &using_one);
					}
				} else {
					if (Z_TYPE(using_zero) == IS_OBJECT) {
						array_init(&params);
						phalcon_array_update_long(&params, 0, &option, PH_COPY);

						PHALCON_CALL_USER_FUNC_ARRAY(&option_value, &using_zero, &params);
						zval_ptr_dtor(&params);
					} else {
						/**
						 * Read the variable directly from the model/object
						 */
						phalcon_read_property_zval(&option_value, &option, &using_zero, PH_COPY);
					}

					if (Z_TYPE(using_one) == IS_OBJECT) {
						array_init(&params);
						phalcon_array_update_long(&params, 0, &option, PH_COPY);

						PHALCON_CALL_USER_FUNC_ARRAY(&option_text, &using_one, &params);
						zval_ptr_dtor(&params);
					} else {
						/**
						 * Read the text directly from the model/object
						 */
						phalcon_read_property_zval(&option_text, &option, &using_one, PH_COPY);
					}
				}
			} else if (Z_TYPE(option) == IS_ARRAY) {
				/**
				 * Read the variable directly from the model/object
				 */
				phalcon_array_fetch(&option_value, &option, &using_zero, PH_NOISY|PH_COPY);

				/**
				 * Read the text directly from the model/object
				 */
				phalcon_array_fetch(&option_text, &option, &using_one, PH_NOISY|PH_COPY);
			} else {
				PHALCON_THROW_EXCEPTION_STR(phalcon_tag_exception_ce, "Resultset returned an invalid value");
				zval_ptr_dtor(&option);
				zval_ptr_dtor(&using_zero);
				return;
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
			zval_ptr_dtor(&option_value);
			zval_ptr_dtor(&option_text);
			zval_ptr_dtor(&escaped);
		} else {
			/**
			 * Check if using is a closure
			 */
			if (Z_TYPE_P(using) == IS_OBJECT) {
				array_init(&params);
				phalcon_array_update_long(&params, 0, &option, PH_COPY);

				PHALCON_CALL_USER_FUNC_ARRAY(&code_option, using, &params);
				zval_ptr_dtor(&params);
				phalcon_concat_self(&code, &code_option);
				zval_ptr_dtor(&code_option);
			}
		}
		zval_ptr_dtor(&option);

		PHALCON_CALL_METHOD(NULL, resultset, "next");
	}

	if (Z_TYPE_P(using) == IS_ARRAY) {
		zval_ptr_dtor(&using_zero);
	}

	RETURN_ZVAL(&code, 0, 0);
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

	zval *data, *using, *value, *close_option, using_zero = {}, using_one = {}, code = {}, *option_text;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 4, 0, &data, &using, &value, &close_option);

	if (Z_TYPE_P(using) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_long(&using_zero, using, 0, PH_COPY)) {
			ZVAL_STRING(&using_zero, "id");
		}
		if (!phalcon_array_isset_fetch_long(&using_one, using, 1, PH_READONLY)) {
			ZVAL_COPY_VALUE(&using_one, &using_zero);
		}
	}

	if (Z_TYPE_P(value) != IS_ARRAY) {
		PHALCON_SEPARATE_PARAM(value);
		convert_to_string(value);
	}

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, str_key, option_text) {
		zval option_value = {};
		if (str_key) {
			ZVAL_STR(&option_value, str_key);
		} else {
			ZVAL_LONG(&option_value, idx);
		}

		if (Z_TYPE_P(option_text) == IS_ARRAY) {			
			if (Z_TYPE_P(using) == IS_ARRAY) {
				zval op_text = {}, op_value = {}, escaped = {};

				phalcon_array_fetch(&op_value, option_text, &using_zero, PH_NOISY|PH_READONLY);
				phalcon_array_fetch(&op_text, option_text, &using_one, PH_NOISY|PH_READONLY);

				phalcon_htmlspecialchars(&escaped, &op_value, NULL, NULL);
				if (Z_TYPE_P(value) == IS_ARRAY) {
					if (phalcon_fast_in_array(&op_value, value)) {
						PHALCON_SCONCAT_SVSVV(&code, "\t<option selected=\"selected\" value=\"", &escaped, "\">", &op_text, close_option);
					} else {
						PHALCON_SCONCAT_SVSVV(&code, "\t<option value=\"", &escaped, "\">", &op_text, close_option);
					}
				} else {
					zval v = {};
					ZVAL_DUP(&v, &op_value);
					if (Z_TYPE(v) != IS_STRING) {
						convert_to_string(&v);
					}
					if (PHALCON_IS_EQUAL(&op_value, value)) {
						PHALCON_SCONCAT_SVSVV(&code, "\t<option selected=\"selected\" value=\"", &escaped, "\">", &op_text, close_option);
					} else {
						PHALCON_SCONCAT_SVSVV(&code, "\t<option value=\"", &escaped, "\">", &op_text, close_option);
					}
					zval_ptr_dtor(&v);
				}
				zval_ptr_dtor(&escaped);
			} else {
				zval array_options = {}, escaped = {};
				phalcon_htmlspecialchars(&escaped, &option_value, NULL, NULL);

				PHALCON_CALL_SELF(&array_options, "_optionsfromarray", option_text, using, value, close_option);
				PHALCON_SCONCAT_SVSVS(&code, "\t<optgroup label=\"", &escaped, "\">" PHP_EOL, &array_options, "\t</optgroup>" PHP_EOL);
				zval_ptr_dtor(&array_options);
				zval_ptr_dtor(&escaped);
			}
		} else {
			zval escaped = {};
			phalcon_htmlspecialchars(&escaped, &option_value, NULL, NULL);

			if (Z_TYPE_P(value) == IS_ARRAY) {
				if (phalcon_fast_in_array(&option_value, value)) {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option selected=\"selected\" value=\"", &escaped, "\">", option_text, close_option);
				} else {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option value=\"", &escaped, "\">", option_text, close_option);
				}
			} else {
				zval v = {};
				ZVAL_DUP(&v, &option_value);
				if (Z_TYPE(v) != IS_STRING) {
					convert_to_string(&v);
				}
				if (PHALCON_IS_EQUAL(&option_value, value)) {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option selected=\"selected\" value=\"", &escaped, "\">", option_text, close_option);
				} else {
					PHALCON_SCONCAT_SVSVV(&code, "\t<option value=\"", &escaped, "\">", option_text, close_option);
				}
				zval_ptr_dtor(&v);
			}
			zval_ptr_dtor(&escaped);
		}
	} ZEND_HASH_FOREACH_END();

	if (Z_TYPE_P(using) == IS_ARRAY) {
		zval_ptr_dtor(&using_zero);
	}

	RETURN_ZVAL(&code, 0, 0);
}
