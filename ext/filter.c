
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

#include "filter.h"
#include "filterinterface.h"
#include "filter/exception.h"
#include "filter/../date.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/hash.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/filter.h"
#include "kernel/concat.h"

/**
 * Phalcon\Filter
 *
 * The Phalcon\Filter component provides a set of commonly needed data filters. It provides
 * object oriented wrappers to the php filter extension. Also allows the developer to
 * define his/her own filters
 *
 *<code>
 *	$filter = new Phalcon\Filter();
 *	$filter->sanitize("some(one)@exa\\mple.com", "email"); // returns "someone@example.com"
 *	$filter->sanitize("hello<<", "string"); // returns "hello"
 *	$filter->sanitize("!100a019", "int"); // returns "100019"
 *	$filter->sanitize("!100a019.01a", "float"); // returns "100019.01"
 *</code>
 */
zend_class_entry *phalcon_filter_ce;

PHP_METHOD(Phalcon_Filter, __construct);
PHP_METHOD(Phalcon_Filter, add);
PHP_METHOD(Phalcon_Filter, sanitize);
PHP_METHOD(Phalcon_Filter, _sanitize);
PHP_METHOD(Phalcon_Filter, getFilters);

static const zend_function_entry phalcon_filter_method_entry[] = {
	PHP_ME(Phalcon_Filter, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Filter, add, arginfo_phalcon_filterinterface_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Filter, sanitize, arginfo_phalcon_filterinterface_sanitize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Filter, _sanitize, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Filter, getFilters, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Filter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Filter){

	PHALCON_REGISTER_CLASS(Phalcon, Filter, filter, phalcon_filter_method_entry, 0);

	zend_declare_property_null(phalcon_filter_ce, SL("_filters"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_filter_ce, SL("_allowTags"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_filter_ce, SL("_allowAttributes"), ZEND_ACC_PROTECTED);

	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_EMAIL"), "email", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_ABSINT"), "absint", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_INT"), "int", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_INT_CAST"), "int!", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_STRING"), "string", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_FLOAT"), "float", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_FLOAT_CAST"), "float!", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_ALPHANUM"), "alphanum", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_TRIM"), "trim", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_STRIPTAGS"), "striptags", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_LOWER"), "lower", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_UPPER"), "upper", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_filter_ce, SL("FILTER_XSS"), "xss", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);

	zend_class_implements(phalcon_filter_ce, 1, phalcon_filterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Filter constructor
 */
PHP_METHOD(Phalcon_Filter, __construct){

	zval allow_tags = {}, allow_attributes = {};

	array_init(&allow_tags);

	phalcon_array_append_string(&allow_tags, SL("a"), 0);
	phalcon_array_append_string(&allow_tags, SL("img"), 0);
	phalcon_array_append_string(&allow_tags, SL("br"), 0);
	phalcon_array_append_string(&allow_tags, SL("hr"), 0);
	phalcon_array_append_string(&allow_tags, SL("strong"), 0);
	phalcon_array_append_string(&allow_tags, SL("strike"), 0);
	phalcon_array_append_string(&allow_tags, SL("b"), 0);
	phalcon_array_append_string(&allow_tags, SL("code"), 0);
	phalcon_array_append_string(&allow_tags, SL("pre"), 0);
	phalcon_array_append_string(&allow_tags, SL("p"), 0);
	phalcon_array_append_string(&allow_tags, SL("div"), 0);
	phalcon_array_append_string(&allow_tags, SL("u"), 0);
	phalcon_array_append_string(&allow_tags, SL("i"), 0);
	phalcon_array_append_string(&allow_tags, SL("em"), 0);
	phalcon_array_append_string(&allow_tags, SL("span"), 0);
	phalcon_array_append_string(&allow_tags, SL("h1"), 0);
	phalcon_array_append_string(&allow_tags, SL("h2"), 0);
	phalcon_array_append_string(&allow_tags, SL("h3"), 0);
	phalcon_array_append_string(&allow_tags, SL("h4"), 0);
	phalcon_array_append_string(&allow_tags, SL("h5"), 0);
	phalcon_array_append_string(&allow_tags, SL("h6"), 0);
	phalcon_array_append_string(&allow_tags, SL("ul"), 0);
	phalcon_array_append_string(&allow_tags, SL("ol"), 0);
	phalcon_array_append_string(&allow_tags, SL("li"), 0);
	phalcon_array_append_string(&allow_tags, SL("table"), 0);
	phalcon_array_append_string(&allow_tags, SL("tr"), 0);
	phalcon_array_append_string(&allow_tags, SL("th"), 0);
	phalcon_array_append_string(&allow_tags, SL("td"), 0);
	phalcon_array_append_string(&allow_tags, SL("u"), 0);
	phalcon_array_append_string(&allow_tags, SL("sub"), 0);
	phalcon_array_append_string(&allow_tags, SL("sup"), 0);
	phalcon_array_append_string(&allow_tags, SL("small"), 0);
	phalcon_array_append_string(&allow_tags, SL("body"), 0);
	phalcon_array_append_string(&allow_tags, SL("html"), 0);

	phalcon_update_property(getThis(), SL("_allowTags"), &allow_tags);

	array_init(&allow_attributes);

	phalcon_array_append_string(&allow_attributes, SL("id"), 0);
	phalcon_array_append_string(&allow_attributes, SL("name"), 0);
	phalcon_array_append_string(&allow_attributes, SL("title"), 0);
	phalcon_array_append_string(&allow_attributes, SL("alt"), 0);
	phalcon_array_append_string(&allow_attributes, SL("src"), 0);
	phalcon_array_append_string(&allow_attributes, SL("style"), 0);
	phalcon_array_append_string(&allow_attributes, SL("href"), 0);
	phalcon_array_append_string(&allow_attributes, SL("class"), 0);
	phalcon_array_append_string(&allow_attributes, SL("width"), 0);
	phalcon_array_append_string(&allow_attributes, SL("height"), 0);
	phalcon_array_append_string(&allow_attributes, SL("target"), 0);
	phalcon_array_append_string(&allow_attributes, SL("align"), 0);

	phalcon_update_property(getThis(), SL("_allowAttributes"), &allow_attributes);
}

/**
 * Adds a user-defined filter
 *
 * @param string $name
 * @param callable $handler
 * @return Phalcon\Filter
 */
PHP_METHOD(Phalcon_Filter, add){

	zval *name, *handler;

	phalcon_fetch_params(0, 2, 0, &name, &handler);

	if (Z_TYPE_P(handler) != IS_OBJECT && !phalcon_is_callable(handler)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_filter_exception_ce, "Filter must be an object or callable");
		return;
	}

	phalcon_update_property_array(getThis(), SL("_filters"), name, handler);
	RETURN_THIS();
}

/**
 * Sanitizes a value with a specified single or set of filters
 *
 * @param  mixed $value
 * @param  mixed $filters
 * @return mixed
 */
PHP_METHOD(Phalcon_Filter, sanitize){

	zval *value, *filters, *norecursive = NULL, *options = NULL, new_value = {}, *item_value, *filter, filter_value = {}, sanizited_value = {};
	zend_string *item_key;
	ulong item_idx;

	phalcon_fetch_params(0, 2, 2, &value, &filters, &norecursive, &options);

	if (!norecursive) {
		norecursive = &PHALCON_GLOBAL(z_false);
	}

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	/**
	 * Apply an array of filters
	 */
	if (Z_TYPE_P(filters) == IS_ARRAY) {
		PHALCON_CPY_WRT_CTOR(&new_value, value);
		if (Z_TYPE_P(value) != IS_NULL) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(filters), filter) {
				zval array_value = {};
				/**
				 * If the value to filter is an array we apply the filters recursively
				 */
				if (Z_TYPE(new_value) == IS_ARRAY && !zend_is_true(norecursive)) {
					array_init(&array_value);

					ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(new_value), item_idx, item_key, item_value) {
						PHALCON_CALL_METHOD(&filter_value, getThis(), "_sanitize", item_value, filter, options);

						if (item_key) {
							phalcon_array_update_string(&array_value, item_key, &filter_value, PH_COPY);
						} else {
							phalcon_array_update_long(&array_value, item_idx, &filter_value, PH_COPY);
						}
					} ZEND_HASH_FOREACH_END();

					PHALCON_CPY_WRT_CTOR(&new_value, &array_value);
				} else {
					PHALCON_CALL_METHOD(&filter_value, getThis(), "_sanitize", &new_value, filter, options);
					PHALCON_CPY_WRT_CTOR(&new_value, &filter_value);
				}
			} ZEND_HASH_FOREACH_END();

		}

		RETURN_CTOR(&new_value);
	}

	/**
	 * Apply a single filter value
	 */
	if (Z_TYPE_P(value) == IS_ARRAY && !zend_is_true(norecursive)) {
		array_init(&sanizited_value);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(value), item_idx, item_key, item_value) {
			PHALCON_CALL_METHOD(&filter_value, getThis(), "_sanitize", item_value, filters, options);
			if (item_key) {
				phalcon_array_update_string(&sanizited_value, item_key, &filter_value, PH_COPY);
			} else {
				phalcon_array_update_long(&sanizited_value, item_idx, &filter_value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();

	} else {
		PHALCON_CALL_METHOD(&sanizited_value, getThis(), "_sanitize", value, filters, options);
	}

	RETURN_CTOR(&sanizited_value);
}

/**
 * Internal sanitize wrapper to filter_var
 *
 * @param  mixed $value
 * @param  string $filter
 * @return mixed
 */
PHP_METHOD(Phalcon_Filter, _sanitize){

	zval *value, *filter, *options = NULL, filters = {}, filter_object = {}, arguments = {}, type = {}, quote = {}, empty_str = {}, escaped = {}, filtered = {};
	zval allow_fraction = {}, allow_tags = {}, allow_attributes = {}, format = {}, exception_message = {};

	phalcon_fetch_params(0, 2, 1, &value, &filter, &options);

	phalcon_read_property(&filters, getThis(), SL("_filters"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset_fetch(&filter_object, &filters, filter, PH_READONLY) && (Z_TYPE(filter_object) == IS_OBJECT || phalcon_is_callable(&filter_object))) {
		/**
		 * If the filter is a closure we call it in the PHP userland
		 */
		if (Z_TYPE(filter_object) == IS_OBJECT && instanceof_function(Z_OBJCE(filter_object), zend_ce_closure)) {
			PHALCON_CALL_METHOD(return_value, &filter_object, "call", getThis(), value);
			return;
		} else if (phalcon_is_callable(&filter_object)) {
			array_init_size(&arguments, 1);
			phalcon_array_append(&arguments, value, PH_COPY);
			PHALCON_CALL_USER_FUNC_ARRAY(return_value, &filter_object, &arguments);
			return;
		}

		PHALCON_RETURN_CALL_METHOD(&filter_object, "filter", value);
		return;
	}

	if (PHALCON_IS_STRING(filter, "email")) {
		/**
		 * The 'email' filter uses the filter extension
		 */
		ZVAL_LONG(&type, 517); /* FILTER_SANITIZE_EMAIL */
		ZVAL_STRING(&quote, "'");
		ZVAL_STRING(&empty_str, "");

		PHALCON_STR_REPLACE(&escaped, &quote, &empty_str, value);

		PHALCON_CALL_FUNCTION(&filtered, "filter_var", &escaped, &type);
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "int")) {
		/**
		 * 'int' filter sanitizes a numeric input
		 */
		ZVAL_LONG(&type, 519); /* FILTER_SANITIZE_NUMBER_INT */

		PHALCON_CALL_FUNCTION(&filtered, "filter_var", value, &type);
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "int!") || (PHALCON_IS_STRING(filter, "int?") && phalcon_is_numeric(value))) {
		PHALCON_CPY_WRT_CTOR(&filtered, value);
		convert_to_long_base(&filtered, 10);
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "abs")) {
		convert_scalar_to_number_ex(value);
		if (Z_TYPE_P(value) == IS_DOUBLE) {
			ZVAL_DOUBLE(&filtered, fabs(Z_DVAL_P(value)));
		} else if (Z_DVAL_P(value) == IS_LONG) {
			if (Z_DVAL_P(value) == LONG_MIN) {
				ZVAL_DOUBLE(&filtered, -(double)LONG_MIN);
			} else {
				ZVAL_LONG(&filtered, Z_LVAL_P(value) < 0 ? -Z_LVAL_P(value) : Z_LVAL_P(value));
			}
		} else {
			ZVAL_FALSE(&filtered);
		}
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "string")) {
		ZVAL_LONG(&type, 513); /* FILTER_SANITIZE_STRING */

		PHALCON_CALL_FUNCTION(&filtered, "filter_var", value, &type);
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "float")) {
		zval opt = {};
		/**
		 * The 'float' filter uses the filter extension
		 */
		ZVAL_LONG(&allow_fraction, 4096); /* FILTER_FLAG_ALLOW_FRACTION */

		array_init_size(&opt, 1);
		phalcon_array_update_str(&opt, SL("flags"), &allow_fraction, PH_COPY);

		ZVAL_LONG(&type, 520); /* FILTER_SANITIZE_NUMBER_FLOAT */

		PHALCON_CALL_FUNCTION(&filtered, "filter_var", value, &type, &opt);
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "float!") || (PHALCON_IS_STRING(filter, "float?") && phalcon_is_numeric(value))) {
		ZVAL_ZVAL(&filtered, value, 1, 0);
		convert_to_double(&filtered);
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "alphanum")) {
		phalcon_filter_alphanum(&filtered, value);
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "trim")) {
		ZVAL_STR(&filtered, phalcon_trim(value, NULL, PHALCON_TRIM_BOTH));
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "striptags")) {
		phalcon_fast_strip_tags(&filtered, value);
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "lower")) {
		if (phalcon_function_exists_ex(SL("mb_strtolower")) == SUCCESS) {
			/**
			 * 'lower' checks for the mbstring extension to make a correct lowercase
			 * transformation
			 */
			PHALCON_CALL_FUNCTION(&filtered, "mb_strtolower", value);
		} else {
			phalcon_fast_strtolower(&filtered, value);
		}
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "upper")) {
		if (phalcon_function_exists_ex(SL("mb_strtoupper")) == SUCCESS) {
			/**
			 * 'upper' checks for the mbstring extension to make a correct lowercase
			 * transformation
			 */
			PHALCON_CALL_FUNCTION(&filtered, "mb_strtoupper", value);
		} else {
			phalcon_fast_strtoupper(&filtered, value);
		}
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "xss") || PHALCON_IS_STRING(filter, "xssclean")) {
		if (Z_TYPE_P(options) == IS_ARRAY) {
			if (!phalcon_array_isset_fetch_str(&allow_tags, options, SL("allowTags"), PH_READONLY) || Z_TYPE(allow_tags) != IS_ARRAY) {
				phalcon_read_property(&allow_tags, getThis(), SL("_allowTags"), PH_READONLY);
			} else {
				phalcon_array_append_string(&allow_tags, SL("body"), 0);
				phalcon_array_append_string(&allow_tags, SL("html"), 0);
			}
			if (!phalcon_array_isset_fetch_str(&allow_attributes, options, SL("allowAttributes"), PH_READONLY) || Z_TYPE(allow_attributes) != IS_ARRAY) {
				phalcon_read_property(&allow_attributes, getThis(), SL("_allowAttributes"), PH_READONLY);
			}
		} else {
			phalcon_read_property(&allow_tags, getThis(), SL("_allowTags"), PH_READONLY);
			phalcon_read_property(&allow_attributes, getThis(), SL("_allowAttributes"), PH_READONLY);
		}

		phalcon_xss_clean(&filtered, value, &allow_tags, &allow_attributes);
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "date")) {
		PHALCON_CALL_CE_STATIC(&filtered, phalcon_date_ce, "filter", value);
		goto ph_end_0;
	}

	if (PHALCON_IS_STRING(filter, "datetime")) {
		ZVAL_STRING(&format, "Y-m-d H:i:s");
		PHALCON_CALL_CE_STATIC(&filtered, phalcon_date_ce, "filter", value, &format);
		zval_ptr_dtor(&format);
		goto ph_end_0;
	}

	PHALCON_CONCAT_SVS(&exception_message, "Sanitize filter ", filter, " is not supported");
	PHALCON_THROW_EXCEPTION_ZVAL(phalcon_filter_exception_ce, &exception_message);
	return;

ph_end_0:

	RETURN_CTOR(&filtered);
}

/**
 * Return the user-defined filters in the instance
 *
 * @return object[]
 */
PHP_METHOD(Phalcon_Filter, getFilters){

	RETURN_MEMBER(getThis(), "_filters");
}
