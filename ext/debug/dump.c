/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (debug://www.phalconphp.com)       |
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

#include "debug/dump.h"
#include "debug/exception.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/hash.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"

/**
 * Phalcon\Debug\Dump
 *
 * Dumps information about a variable(s)
 *
 * <code>
 *    $foo = 123;
 *    echo (new \Phalcon\Debug\Dump())->variable($foo, "foo");
 *</code>
 *
 * <code>
 *    $foo = "string";
 *    $bar = ["key" => "value"];
 *    $baz = new stdClass();
 *    echo (new \Phalcon\Debug\Dump())->variables($foo, $bar, $baz);
 *</code>
 */
zend_class_entry *phalcon_debug_dump_ce;

PHP_METHOD(Phalcon_Debug_Dump, __construct);
PHP_METHOD(Phalcon_Debug_Dump, all);
PHP_METHOD(Phalcon_Debug_Dump, getStyle);
PHP_METHOD(Phalcon_Debug_Dump, setStyles);
PHP_METHOD(Phalcon_Debug_Dump, output);
PHP_METHOD(Phalcon_Debug_Dump, variable);
PHP_METHOD(Phalcon_Debug_Dump, variables);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_dump___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, styles)
	ZEND_ARG_INFO(0, detailed)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_dump_getstyle, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_dump_setstyles, 0, 0, 0)
	ZEND_ARG_INFO(0, styles)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_dump_output, 0, 0, 1)
	ZEND_ARG_INFO(0, variable)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, tab)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_dump_variable, 0, 0, 1)
	ZEND_ARG_INFO(0, variable)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_debug_dump_method_entry[] = {
	PHP_ME(Phalcon_Debug_Dump, __construct, arginfo_phalcon_debug_dump___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Debug_Dump, all, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug_Dump, getStyle, arginfo_phalcon_debug_dump_getstyle, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug_Dump, setStyles, arginfo_phalcon_debug_dump_setstyles, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug_Dump, output, arginfo_phalcon_debug_dump_output, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug_Dump, variable, arginfo_phalcon_debug_dump_variable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug_Dump, variables, NULL, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Debug_Dump, one, variable, arginfo_phalcon_debug_dump_variable, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Debug_Dump, var, variable, arginfo_phalcon_debug_dump_variable, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Debug_Dump, vars, variables, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Debug\Dump initializer
 */
PHALCON_INIT_CLASS(Phalcon_Debug_Dump){

	PHALCON_REGISTER_CLASS(Phalcon\\Debug, Dump, debug_dump, phalcon_debug_dump_method_entry, 0);

	zend_declare_property_bool(phalcon_debug_dump_ce, SL("_detailed"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_debug_dump_ce, SL("_styles"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_debug_dump_ce, SL("_objects"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Debug\Dump constructor
 *
 * @param array styles set styles for vars type
 * @param boolean detailed debug object's private and protected properties
 */
PHP_METHOD(Phalcon_Debug_Dump, __construct){

	zval *styles = NULL, *detailed = NULL, default_styles = {};

	phalcon_fetch_params(0, 0, 2, &styles, &detailed);

	array_init(&default_styles);

	phalcon_array_update_str_str(&default_styles, SL("pre"), SL("background-color:#f3f3f3; font-size:11px; padding:10px; border:1px solid #ccc; text-align:left; color:#333"), 0);
	phalcon_array_update_str_str(&default_styles, SL("arr"), SL("color:red"), 0);
	phalcon_array_update_str_str(&default_styles, SL("bool"), SL("color:green"), 0);
	phalcon_array_update_str_str(&default_styles, SL("float"), SL("color:fuchsia"), 0);
	phalcon_array_update_str_str(&default_styles, SL("int"), SL("color:blue"), 0);
	phalcon_array_update_str_str(&default_styles, SL("null"), SL("color:black"), 0);
	phalcon_array_update_str_str(&default_styles, SL("num"), SL("color:navy"), 0);
	phalcon_array_update_str_str(&default_styles, SL("obj"), SL("color:purple"), 0);
	phalcon_array_update_str_str(&default_styles, SL("other"), SL("color:maroon"), 0);
	phalcon_array_update_str_str(&default_styles, SL("res"), SL("color:lime"), 0);
	phalcon_array_update_str_str(&default_styles, SL("str"), SL("color:teal"), 0);

	phalcon_update_property(getThis(), SL("_styles"), &default_styles);

	if (styles && Z_TYPE_P(styles) != IS_NULL) {
		PHALCON_CALL_SELF(NULL, "setstyles", styles);
	}

	if (detailed && zend_is_true(detailed)) {
		phalcon_update_property_bool(getThis(), SL("_detailed"), 1);
	}

	phalcon_update_property_empty_array(getThis(), SL("_objects"));
}

/**
 * Alias of variables() method
 *
 * @param mixed variable
 * @param ...
 */
PHP_METHOD(Phalcon_Debug_Dump, all)
{
	zval *args, method_name = {}, call_object = {};

	args = (zval *)safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval), 0);
	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT;
	}

	ZVAL_STRING(&method_name, "variables");

	array_init_size(&call_object, 2);
	phalcon_array_append(&call_object, getThis(), PH_COPY);
	phalcon_array_append(&call_object, &method_name, 0);

	PHALCON_CALL_USER_FUNC_ARGS(return_value, &call_object, args, ZEND_NUM_ARGS());
	zval_ptr_dtor(&call_object);
}

/**
 * Get style for type
 */
PHP_METHOD(Phalcon_Debug_Dump, getStyle){

	zval *type, styles = {};

	phalcon_fetch_params(0, 1, 0, &type);

	phalcon_read_property(&styles, getThis(), SL("_styles"), PH_NOISY|PH_READONLY);

	if (!phalcon_array_isset_fetch(return_value, &styles, type, PH_COPY)) {
		ZVAL_STRING(return_value, "color:gray");
	}
}

/**
 * Set styles for vars type
 */
PHP_METHOD(Phalcon_Debug_Dump, setStyles){

	zval *styles, default_styles = {}, new_styles = {};

	phalcon_fetch_params(0, 1, 0, &styles);

	if (Z_TYPE_P(styles) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_debug_exception_ce, "The styles must be an array");
		return;
	}

	phalcon_read_property(&default_styles, getThis(), SL("_styles"), PH_NOISY|PH_READONLY);

	phalcon_fast_array_merge(&new_styles, &default_styles, styles);

	phalcon_update_property(getThis(), SL("_styles"), &new_styles);
	zval_ptr_dtor(&new_styles);

	RETURN_THIS();
}

/**
 * Prepare an HTML string of information about a single variable.
 */
PHP_METHOD(Phalcon_Debug_Dump, output){

	zval *variable, *name = NULL, *tab = NULL, space = {}, str = {}, type = {}, style = {}, count = {}, replace_pairs = {}, output = {}, new_tab = {}, tmp = {};
	zval *value, class_name = {}, objects = {}, detailed = {}, properties = {}, methods = {}, *method;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 2, &variable, &name, &tab);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	} else if (!PHALCON_IS_EMPTY(name)) {
		PHALCON_CONCAT_SVS(return_value, "var ", name, " ");
	}

	if (!tab) {
		tab = &PHALCON_GLOBAL(z_one);
	}

	ZVAL_LONG(&new_tab, Z_LVAL_P(tab) - 1);
	ZVAL_STRING(&space, "  ");

	if (Z_TYPE_P(variable) == IS_ARRAY) {
		ZVAL_STRING(&str, "<b style =':style'>Array</b> (<span style =':style'>:count</span>) (\n");
		ZVAL_STRING(&type, "arr");

		PHALCON_CALL_SELF(&style, "getstyle", &type);

		ZVAL_LONG(&count, phalcon_fast_count_int(variable));

		array_init(&replace_pairs);

		phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
		phalcon_array_update_str(&replace_pairs, SL(":count"), &count, PH_COPY);

		PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

		phalcon_concat_self(return_value, &output);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(variable), idx, str_key, value) {
			zval key = {}, repeat = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}

			PHALCON_CALL_FUNCTION(&repeat, "str_repeat", &space, tab);

			phalcon_concat_self(return_value, &repeat);

			ZVAL_STRING(&str, "[<span style=':style'>:key</span>] => ");

			ZVAL_STRING(&type, "arr");

			PHALCON_CALL_SELF(&style, "getstyle", &type);

			array_init(&replace_pairs);

			phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
			phalcon_array_update_str(&replace_pairs, SL(":key"), &key, PH_COPY);

			PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

			phalcon_concat_self(return_value, &output);

			if (PHALCON_IS_LONG(tab, 1) && !PHALCON_IS_EMPTY(name) && !phalcon_is_numeric(&key) && PHALCON_IS_IDENTICAL(name, &key)) {
				continue;
			} else {
				PHALCON_CALL_SELF(&tmp, "output", value, &PHALCON_GLOBAL(z_null), &new_tab);
				PHALCON_SCONCAT_VS(return_value, &tmp, "\n");
			}
		} ZEND_HASH_FOREACH_END();

		PHALCON_CALL_FUNCTION(&tmp, "str_repeat", &space, &new_tab);

		PHALCON_SCONCAT(return_value, &tmp);
	} else if (Z_TYPE_P(variable) == IS_OBJECT) {
		ZVAL_STRING(&str, "<b style=':style'>Object</b> :class");
		ZVAL_STRING(&type, "obj");

		PHALCON_CALL_SELF(&style, "getstyle", &type);

		phalcon_get_class(&class_name, variable, 0);

		array_init(&replace_pairs);

		phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
		phalcon_array_update_str(&replace_pairs, SL(":class"), &class_name, PH_COPY);

		PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

		PHALCON_SCONCAT(return_value, &output);

		phalcon_get_parent_class(&class_name, variable, 0);

		if (zend_is_true(&class_name)) {
			ZVAL_STRING(&str, " <b style=':style'>extends</b> :parent");
			ZVAL_STRING(&type, "obj");

			PHALCON_CALL_SELF(&style, "getstyle", &type);

			array_init(&replace_pairs);

			phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
			phalcon_array_update_str(&replace_pairs, SL(":parent"), &class_name, PH_COPY);

			PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

			PHALCON_SCONCAT(return_value, &output);
		}

		PHALCON_SCONCAT_STR(return_value, " (\n");

		phalcon_read_property(&objects, getThis(), SL("_objects"), PH_NOISY|PH_READONLY);

		if (phalcon_fast_in_array(variable, &objects)) {
			PHALCON_CALL_FUNCTION(&tmp, "str_repeat", &space, tab);
			PHALCON_SCONCAT_VS(return_value, &tmp, "[already listed]\n");

			PHALCON_CALL_FUNCTION(&tmp, "str_repeat", &space, tab);

			PHALCON_SCONCAT_VS(return_value, &tmp, ")");

			return;
		}

		phalcon_update_property_array_append(getThis(), SL("_objects"), variable);

		phalcon_read_property(&detailed, getThis(), SL("_detailed"), PH_NOISY|PH_READONLY);

		phalcon_get_object_vars(&properties, variable, !zend_is_true(&detailed));

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(properties), idx, str_key, value) {
			zval key = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}

			PHALCON_CALL_FUNCTION(&tmp, "str_repeat", &space, tab);

			PHALCON_SCONCAT(return_value, &tmp);

			ZVAL_STRING(&str, "-><span style=':style'>:key</span> (<span style=':style'>:type</span>) = ");
			ZVAL_STRING(&type, "obj");

			PHALCON_CALL_SELF(&style, "getstyle", &type);

			array_init(&replace_pairs);

			phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
			phalcon_array_update_str(&replace_pairs, SL(":key"), &key, PH_COPY);

			if (PHALCON_PROPERTY_IS_PUBLIC_ZVAL(variable, &key)) {
				phalcon_array_update_str_str(&replace_pairs, SL(":type"), SL("public"), 0);
			} else if (PHALCON_PROPERTY_IS_PRIVATE_ZVAL(variable, &key)) {
				phalcon_array_update_str_str(&replace_pairs, SL(":type"), SL("private"), 0);
			} else if (PHALCON_PROPERTY_IS_PROTECTED_ZVAL(variable, &key)) {
				phalcon_array_update_str_str(&replace_pairs, SL(":type"), SL("protected"), 0);
			}

			PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

			PHALCON_SCONCAT(return_value, &output);

			PHALCON_CALL_SELF(&tmp, "output", value, &PHALCON_GLOBAL(z_null), &new_tab);
			PHALCON_SCONCAT_VS(return_value, &tmp, ")\n");
		} ZEND_HASH_FOREACH_END();

		phalcon_get_class_methods(&methods, variable, !zend_is_true(&detailed));

		PHALCON_CALL_FUNCTION(&tmp, "str_repeat", &space, tab);

		PHALCON_SCONCAT(return_value, &tmp);

		ZVAL_STRING(&str, ":class <b style=':style'>methods</b>: (<span style=':style'>:count</span>) (\n");
		ZVAL_STRING(&type, "obj");

		PHALCON_CALL_SELF(&style, "getstyle", &type);

		phalcon_get_class(&class_name, variable, 0);

		ZVAL_LONG(&count, phalcon_fast_count_int(&methods));

		array_init(&replace_pairs);

		phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
		phalcon_array_update_str(&replace_pairs, SL(":class"), &class_name, PH_COPY);
		phalcon_array_update_str(&replace_pairs, SL(":count"), &count, PH_COPY);

		PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

		PHALCON_SCONCAT(return_value, &output);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(methods), method) {
			PHALCON_CALL_FUNCTION(&tmp, "str_repeat", &space, &new_tab);

			PHALCON_SCONCAT(return_value, &tmp);

			ZVAL_STRING(&str, "-><span style=':style'>:method</span>();\n");
			ZVAL_STRING(&type, "obj");

			PHALCON_CALL_SELF(&style, "getstyle", &type);
			array_init(&replace_pairs);

			phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
			phalcon_array_update_str(&replace_pairs, SL(":method"), method, PH_COPY);

			PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

			PHALCON_SCONCAT(return_value, &output);

			PHALCON_CALL_FUNCTION(&tmp, "str_repeat", &space, tab);

			PHALCON_SCONCAT_VS(return_value, &tmp, "\n");
		} ZEND_HASH_FOREACH_END();

		PHALCON_CALL_FUNCTION(&tmp, "str_repeat", &space, tab);

		PHALCON_SCONCAT_VS(return_value, &tmp, ")");
	} else if (Z_TYPE_P(variable) == IS_LONG) {
		ZVAL_STRING(&str, "<b style=':style'>Integer</b> (<span style=':style'>:var</span>)");
		ZVAL_STRING(&type, "int");

		PHALCON_CALL_SELF(&style, "getstyle", &type);

		array_init(&replace_pairs);

		phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
		phalcon_array_update_str(&replace_pairs, SL(":var"), variable, PH_COPY);

		PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

		PHALCON_SCONCAT(return_value, &output);
	} else if (Z_TYPE_P(variable) == IS_DOUBLE) {
		ZVAL_STRING(&str, "<b style=':style'>Float</b> (<span style=':style'>:var</span>)");
		ZVAL_STRING(&type, "float");

		PHALCON_CALL_SELF(&style, "getstyle", &type);

		array_init(&replace_pairs);

		phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
		phalcon_array_update_str(&replace_pairs, SL(":var"), variable, PH_COPY);

		PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

		PHALCON_SCONCAT(return_value, &output);
	} else if (phalcon_is_numeric_ex(variable)) {
		ZVAL_STRING(&str, "<b style=':style'>Numeric string</b> (<span style=':style'>:length</span>) \"<span style=':style'>:var</span>\"");
		ZVAL_STRING(&type, "num");

		PHALCON_CALL_SELF(&style, "getstyle", &type);

		array_init(&replace_pairs);

		phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
		phalcon_array_update_str_long(&replace_pairs, SL(":length"), Z_STRLEN_P(variable), 0);
		phalcon_array_update_str(&replace_pairs, SL(":var"), variable, PH_COPY);

		PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

		PHALCON_SCONCAT(return_value, &output);
	} else if (Z_TYPE_P(variable) == IS_STRING) {
		ZVAL_STRING(&str, "<b style=':style'>String</b> (<span style=':style'>:length</span>) \"<span style=':style'>:var</span>\"");
		ZVAL_STRING(&type, "str");

		PHALCON_CALL_SELF(&style, "getstyle", &type);

		array_init(&replace_pairs);

		phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
		phalcon_array_update_str_long(&replace_pairs, SL(":length"), Z_STRLEN_P(variable), 0);
		phalcon_array_update_str(&replace_pairs, SL(":var"), variable, PH_COPY);

		PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

		PHALCON_SCONCAT(return_value, &output);
	} else if (PHALCON_IS_BOOL(variable)) {
		ZVAL_STRING(&str, "<b style=':style'>Boolean</b> (<span style=':style'>:var</span>)");
		ZVAL_STRING(&type, "bool");

		PHALCON_CALL_SELF(&style, "getstyle", &type);

		array_init(&replace_pairs);

		phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
		if (zend_is_true(variable)) {
			phalcon_array_update_str_str(&replace_pairs, SL(":var"), SL("TRUE") , PH_COPY);
		} else {
			phalcon_array_update_str_str(&replace_pairs, SL(":var"), SL("FALSE") , PH_COPY);
		}

		PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

		PHALCON_SCONCAT(return_value, &output);
	} else if (Z_TYPE_P(variable) == IS_NULL) {
		ZVAL_STRING(&str, "<b style=':style'>NULL</b>");
		ZVAL_STRING(&type, "null");

		PHALCON_CALL_SELF(&style, "getstyle", &type);

		array_init(&replace_pairs);

		phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);

		PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

		PHALCON_SCONCAT(return_value, &output);
	} else {
		ZVAL_STRING(&str, "(<span style=':style'>:var</span>)");
		ZVAL_STRING(&type, "other");

		PHALCON_CALL_SELF(&style, "getstyle", &type);

		array_init(&replace_pairs);

		phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
		phalcon_array_update_str(&replace_pairs, SL(":var"), variable, PH_COPY);

		PHALCON_CALL_FUNCTION(&output, "strtr", &str, &replace_pairs);

		PHALCON_SCONCAT(return_value, &output);
	}
}

/**
 * Returns an HTML string of information about a single variable.
 *
 * <code>
 *    echo (new \Phalcon\Debug\Dump())->variable($foo, "foo");
 * </code>
 */
PHP_METHOD(Phalcon_Debug_Dump, variable){

	zval *variable, *name = NULL, str = {}, type = {}, style = {}, output = {}, replace_pairs = {};

	phalcon_fetch_params(0, 1, 1, &variable, &name);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&str, "<pre style=':style'>:output</pre>");
	ZVAL_STRING(&type, "pre");

	PHALCON_CALL_SELF(&style, "getstyle", &type);
	PHALCON_CALL_SELF(&output, "output", variable, name);

	array_init(&replace_pairs);

	phalcon_array_update_str(&replace_pairs, SL(":style"), &style, PH_COPY);
	phalcon_array_update_str(&replace_pairs, SL(":output"), &output, PH_COPY);

	PHALCON_RETURN_CALL_FUNCTION("strtr", &str, &replace_pairs);
}

/**
 * Returns an HTML string of debugging information about any number of
 * variables, each wrapped in a "pre" tag.
 *
 * <code>
 *    $foo = "string";
 *    $bar = ["key" => "value"];
 *    $baz = new stdClass();
 *    echo (new \Phalcon\Debug\Dump())->variables($foo, $bar, $baz);
 *</code>
 *
 * @param mixed variable
 * @param ...
 */
PHP_METHOD(Phalcon_Debug_Dump, variables){

	zval *args;
	uint32_t i;

	args = (zval *)safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval), 0);
	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT;
	}

	for (i = 0; i < ZEND_NUM_ARGS(); i++) {
		zval output = {};

		PHALCON_CALL_SELF(&output, "variable", &args[i]);
		PHALCON_SCONCAT(return_value, &output);
	};

	return;
}
