
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

#include "text.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"

/**
 * Phalcon\Text
 *
 * Provides utilities to work with texts
 */
zend_class_entry *phalcon_text_ce;

PHP_METHOD(Phalcon_Text, camelize);
PHP_METHOD(Phalcon_Text, uncamelize);
PHP_METHOD(Phalcon_Text, increment);
PHP_METHOD(Phalcon_Text, random);
PHP_METHOD(Phalcon_Text, startsWith);
PHP_METHOD(Phalcon_Text, endsWith);
PHP_METHOD(Phalcon_Text, lower);
PHP_METHOD(Phalcon_Text, upper);
PHP_METHOD(Phalcon_Text, bytes);
PHP_METHOD(Phalcon_Text, reduceSlashes);
PHP_METHOD(Phalcon_Text, concat);
PHP_METHOD(Phalcon_Text, underscore);
PHP_METHOD(Phalcon_Text, humanize);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_camelize, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_uncamelize, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_increment, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, separator)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_random, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_startswith, 0, 0, 2)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, ignoreCase)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_endswith, 0, 0, 2)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, end)
	ZEND_ARG_INFO(0, ignoreCase)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_lower, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_upper, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_bytes, 0, 0, 1)
	ZEND_ARG_INFO(0, size)
	ZEND_ARG_INFO(0, forceUnit)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, si)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_reduceslashes, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_text_concat, 0, 0, 3)
	ZEND_ARG_INFO(0, separator)
	ZEND_ARG_INFO(0, strA)
	ZEND_ARG_INFO(0, strB)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_text_method_entry[] = {
	PHP_ME(Phalcon_Text, camelize, arginfo_phalcon_text_camelize, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Text, uncamelize, arginfo_phalcon_text_uncamelize, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Text, increment, arginfo_phalcon_text_increment, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Text, random, arginfo_phalcon_text_random, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Text, startsWith, arginfo_phalcon_text_startswith, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Text, endsWith, arginfo_phalcon_text_endswith, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Text, lower, arginfo_phalcon_text_lower, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Text, upper, arginfo_phalcon_text_upper, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Text, bytes, arginfo_phalcon_text_bytes, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Text, reduceSlashes, arginfo_phalcon_text_reduceslashes, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Text, concat, arginfo_phalcon_text_concat, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Text initializer
 */
PHALCON_INIT_CLASS(Phalcon_Text){

	PHALCON_REGISTER_CLASS(Phalcon, Text, text, phalcon_text_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_class_constant_long(phalcon_text_ce, SL("RANDOM_ALNUM"),   PHALCON_TEXT_RANDOM_ALNUM  );
	zend_declare_class_constant_long(phalcon_text_ce, SL("RANDOM_ALPHA"),   PHALCON_TEXT_RANDOM_ALPHA  );
	zend_declare_class_constant_long(phalcon_text_ce, SL("RANDOM_HEXDEC"),  PHALCON_TEXT_RANDOM_HEXDEC );
	zend_declare_class_constant_long(phalcon_text_ce, SL("RANDOM_NUMERIC"), PHALCON_TEXT_RANDOM_NUMERIC);
	zend_declare_class_constant_long(phalcon_text_ce, SL("RANDOM_NOZERO"),  PHALCON_TEXT_RANDOM_NOZERO );

	return SUCCESS;
}

/**
 * Converts strings to camelize style
 *
 *<code>
 *	echo Phalcon\Text::camelize('coco_bongo'); //CocoBongo
 *</code>
 *
 * @param string $str
 * @return string
 */
PHP_METHOD(Phalcon_Text, camelize){

	zval *str;

	phalcon_fetch_params(0, 1, 0, &str);

	phalcon_camelize(return_value, str);
}

/**
 * Uncamelize strings which are camelized
 *
 *<code>
 *	echo Phalcon\Text::uncamelize('CocoBongo'); //coco_bongo
 *</code>
 *
 * @param string $str
 * @return string
 */
PHP_METHOD(Phalcon_Text, uncamelize){

	zval *str;

	phalcon_fetch_params(0, 1, 0, &str);

	phalcon_uncamelize(return_value, str);
}

/**
 * Adds a number to a string or increment that number if it already is defined
 *
 *<code>
 *	echo Phalcon\Text::increment("a"); // "a_1"
 *	echo Phalcon\Text::increment("a_1"); // "a_2"
 *</code>
 *
 * @param string $str
 * @param string $separator
 * @return string
 */
PHP_METHOD(Phalcon_Text, increment){

	zval *str, *separator = NULL, sep = {}, parts = {}, number = {}, first_part = {};

	phalcon_fetch_params(0, 1, 1, &str, &separator);

	if (!separator || Z_TYPE_P(separator) == IS_NULL) {
		ZVAL_STRING(&sep, "_");
	} else {
		PHALCON_CPY_WRT_CTOR(&sep, separator);
	}

	phalcon_fast_explode(&parts, separator, str);
	if (phalcon_array_isset_fetch_long(&number, &parts, 1)) {
		phalcon_increment(&number);
	} else {
		PHALCON_CPY_WRT_CTOR(&number, &PHALCON_GLOBAL(z_one));
	}

	phalcon_array_fetch_long(&first_part, &parts, 0, PH_NOISY);
	PHALCON_CONCAT_VVV(return_value, &first_part, &sep, &number);
}

/**
 * Generates a random string based on the given type. Type is one of the RANDOM_* constants
 *
 *<code>
 *	echo Phalcon\Text::random(Phalcon\Text::RANDOM_ALNUM); //"aloiwkqz"
 *</code>
 *
 * @param int $type
 * @param int $length
 * @return string
 */
PHP_METHOD(Phalcon_Text, random){

	zval *type, *length = NULL, len = {};

	phalcon_fetch_params(0, 1, 1, &type, &length);

	if (!length) {
		ZVAL_LONG(&len, 8);
		phalcon_random_string(return_value, type, &len);
	} else {
		phalcon_random_string(return_value, type, length);
	}
}

/**
 * Check if a string starts with a given string
 *
 *<code>
 *	echo Phalcon\Text::startsWith("Hello", "He"); // true
 *	echo Phalcon\Text::startsWith("Hello", "he"); // false
 *	echo Phalcon\Text::startsWith("Hello", "he", false); // true
 *</code>
 *
 * @param string $str
 * @param string $start
 * @param boolean $ignoreCase
 * @return boolean
 */
PHP_METHOD(Phalcon_Text, startsWith){

	zval *str, *start, *ignore_case = NULL, *case_sensitive;

	phalcon_fetch_params(0, 2, 1, &str, &start, &ignore_case);

	if (!ignore_case) {
		case_sensitive = &PHALCON_GLOBAL(z_false);
	}
	else {
		case_sensitive = zend_is_true(ignore_case) ? &PHALCON_GLOBAL(z_false) : &PHALCON_GLOBAL(z_true);
	}

	RETURN_BOOL(phalcon_start_with(str, start, case_sensitive));
}

/**
 * Check if a string ends with a given string
 *
 *<code>
 *	echo Phalcon\Text::endsWith("Hello", "llo"); // true
 *	echo Phalcon\Text::endsWith("Hello", "LLO"); // false
 *	echo Phalcon\Text::endsWith("Hello", "LLO", false); // true
 *</code>
 *
 * @param string $str
 * @param string $end
 * @param boolean $ignoreCase
 * @return boolean
 */
PHP_METHOD(Phalcon_Text, endsWith){

	zval *str, *end, *ignore_case = NULL, *case_sensitive;

	phalcon_fetch_params(0, 2, 1, &str, &end, &ignore_case);

	if (!ignore_case) {
		case_sensitive = &PHALCON_GLOBAL(z_false);
	}
	else {
		case_sensitive = zend_is_true(ignore_case) ? &PHALCON_GLOBAL(z_false) : &PHALCON_GLOBAL(z_true);
	}

	RETURN_BOOL(phalcon_end_with(str, end, case_sensitive));
}

/**
 * Lowercases a string, this function makes use of the mbstring extension if available
 *
 * @param string $str
 * @return string
 */
PHP_METHOD(Phalcon_Text, lower){

	zval *str;

	phalcon_fetch_params(0, 1, 0, &str);

	/** 
	 * 'lower' checks for the mbstring extension to make a correct lowercase
	 * transformation
	 */
	if (phalcon_function_exists_ex(SL("mb_strtolower")) == SUCCESS) {
		PHALCON_RETURN_CALL_FUNCTIONW("mb_strtolower", str);
		return;
	}

	phalcon_fast_strtolower(return_value, str);
}

/**
 * Uppercases a string, this function makes use of the mbstring extension if available
 *
 * @param string $str
 * @return string
 */
PHP_METHOD(Phalcon_Text, upper){

	zval *str;

	phalcon_fetch_params(0, 1, 0, &str);

	/** 
	 * 'upper' checks for the mbstring extension to make a correct lowercase
	 * transformation
	 */
	if (phalcon_function_exists_ex(SL("mb_strtoupper")) == SUCCESS) {
		PHALCON_RETURN_CALL_FUNCTIONW("mb_strtoupper", str);
		return;
	}

	phalcon_fast_strtoupper(return_value, str);
}

/**
 * Returns human readable sizes
 *
 * @param int $size
 * @param string $forceUnit
 * @param string $format
 * @param boolean $si
 * @return string
 */
PHP_METHOD(Phalcon_Text, bytes){

	zval *_z_size, *_z_force_unit = NULL, *_format = NULL, z_size = {}, z_force_unit = {}, format = {}, *si = NULL;
	char *force_unit;
	const char **units;
	const char *units1[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
	const char *units2[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
	double size;
	int mod, power = 0, found = 0, i, j = 0;

	phalcon_fetch_params(0, 1, 3, &_z_size, &_z_force_unit, &_format, &si);

	PHALCON_CPY_WRT_CTOR(&z_size, _z_size);
	convert_to_double(&z_size);

	size = Z_DVAL(z_size);

	if (_z_force_unit) {
		PHALCON_CPY_WRT_CTOR(&z_force_unit, _z_force_unit);
		convert_to_string(&z_force_unit);
	}

	if (_format) {
		PHALCON_CPY_WRT_CTOR(&format, _format);
		convert_to_string(&format);
	}
		
	if (PHALCON_IS_EMPTY(&format)) {
		ZVAL_STRING(&format, "%01.2f %s");
	}

	if (!si) {
		si = &PHALCON_GLOBAL(z_true);
	}

	if (!zend_is_true(si) || (!PHALCON_IS_EMPTY(&z_force_unit) && phalcon_memnstr_str(&z_force_unit, SL("i")))) {
		units = units2;
		mod = 1024;
	} else {
		units = units1;
		mod = 1000;
	}

	if (!PHALCON_IS_EMPTY(&z_force_unit)) {
		force_unit = Z_STRVAL(z_force_unit);
		for (i = 0; i < sizeof(units); i++)
		{
			if (strcasecmp(force_unit, units[i]) == 0) {
				found = 1;
				power = i;
				break;
			}
		}
	}

	if (found) {
		while(j < power) {
			size /= mod;
			j++;
		}
	} else {
		while(size > mod) {
			size /= mod;
			power++;
		}
	}

	ZVAL_DOUBLE(&z_size, size);
	ZVAL_STRING(&z_force_unit, units[power]);

	PHALCON_RETURN_CALL_FUNCTIONW("sprintf", &format, &z_size, &z_force_unit);
}

/**
 * Reduces multiple slashes in a string to single slashes
 *
 * <code>
 *    echo Phalcon\Text::reduceSlashes("foo//bar/baz"); // foo/bar/baz
 *    echo Phalcon\Text::reduceSlashes("http://foo.bar///baz/buz"); // http://foo.bar/baz/buz
 * </code>
 */
PHP_METHOD(Phalcon_Text, reduceSlashes){

	zval *str, pattern = {}, replacement = {};

	phalcon_fetch_params(0, 1, 0, &str);

	ZVAL_STRING(&pattern, "#(?<!:)//+#");
	ZVAL_STRING(&replacement, "/");

	PHALCON_RETURN_CALL_FUNCTIONW("preg_replace", &pattern, &replacement, str);
}

/**
 * Concatenates strings using the separator only once without duplication in places concatenation
 *
 * <code>
 *    $str = Phalcon\Text::concat("/", "/tmp/", "/folder_1/", "/folder_2", "folder_3/");
 *    echo $str; // /tmp/folder_1/folder_2/folder_3/
 * </code>
 *
 * @param string separator
 * @param string a
 * @param string b
 * @param string ...N
 */
PHP_METHOD(Phalcon_Text, concat){

	zval *separator, *a, *b, arg_num = {}, arg_list = {}, offset = {}, args = {}, *c, str = {}, a_trimmed = {}, str_trimmed = {};

	phalcon_fetch_params(0, 3, 0, &separator, &a, &b);

	PHALCON_CALL_FUNCTIONW(&arg_num, "func_num_args");

	if (Z_LVAL(arg_num) > 3) {
		PHALCON_CALL_FUNCTIONW(&arg_list, "func_get_args");

		ZVAL_LONG(&offset, 3);

		PHALCON_CALL_FUNCTIONW(&args, "array_slice", &arg_list, &offset);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(args), c) {
			zval b_trimmed = {}, c_trimmed = {};

			ZVAL_STR(&b_trimmed, phalcon_trim(b, separator, PHALCON_TRIM_RIGHT));
			ZVAL_STR(&c_trimmed, phalcon_trim(c, separator, PHALCON_TRIM_LEFT));

			PHALCON_CONCAT_VVV(&str, &b_trimmed, separator, &c_trimmed)

		} ZEND_HASH_FOREACH_END();
	} else {
		PHALCON_CPY_WRT_CTOR(&str, b);
	}

	ZVAL_STR(&a_trimmed, phalcon_trim(a, separator, PHALCON_TRIM_RIGHT));
	ZVAL_STR(&str_trimmed, phalcon_trim(&str, separator, PHALCON_TRIM_LEFT));

	PHALCON_CONCAT_VVV(return_value, &a_trimmed, separator, &str_trimmed)
}

/**
 * Makes a phrase underscored instead of spaced
 *
 * <code>
 *   echo Phalcon\Text::underscore('look behind'); // 'look_behind'
 *   echo Phalcon\Text::underscore('Awesome Phalcon'); // 'Awesome_Phalcon'
 * </code>
 */
PHP_METHOD(Phalcon_Text, underscore)
{
	zval *str, trimmed = {}, pattern = {}, replacement = {};

	phalcon_fetch_params(0, 1, 0, &str);

	ZVAL_STR(&trimmed, phalcon_trim(str, NULL, PHALCON_TRIM_BOTH));
	ZVAL_STRING(&pattern, "#\\s+#");
	ZVAL_STRING(&replacement, "_");

	PHALCON_RETURN_CALL_FUNCTIONW("preg_replace", &pattern, &replacement, &trimmed);
}

/**
 * Makes an underscored or dashed phrase human-readable
 *
 * <code>
 *   echo Phalcon\Text::humanize('start-a-horse'); // 'start a horse'
 *   echo Phalcon\Text::humanize('five_cats'); // 'five cats'
 * </code>
 */
PHP_METHOD(Phalcon_Text, humanize)
{
	zval *str, trimmed = {}, pattern = {}, replacement = {};

	phalcon_fetch_params(0, 1, 0, &str);

	ZVAL_STR(&trimmed, phalcon_trim(str, NULL, PHALCON_TRIM_BOTH));
	ZVAL_STRING(&pattern, "#[_-]+#");
	ZVAL_STRING(&replacement, " ");

	PHALCON_RETURN_CALL_FUNCTIONW("preg_replace", &pattern, &replacement, &trimmed);
}
