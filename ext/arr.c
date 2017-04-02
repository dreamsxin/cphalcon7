
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

#include "arr.h"
#include "di.h"
#include "filterinterface.h"

#include <ext/standard/php_array.h>
#include <ext/spl/spl_array.h>

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/hash.h"

#include "interned-strings.h"

/**
 * Phalcon\Arr
 *
 * Provides utilities to work with arrs
 */
zend_class_entry *phalcon_arr_ce;

PHP_METHOD(Phalcon_Arr, is_assoc);
PHP_METHOD(Phalcon_Arr, is_array);
PHP_METHOD(Phalcon_Arr, path);
PHP_METHOD(Phalcon_Arr, set_path);
PHP_METHOD(Phalcon_Arr, range);
PHP_METHOD(Phalcon_Arr, get);
PHP_METHOD(Phalcon_Arr, choice);
PHP_METHOD(Phalcon_Arr, extract);
PHP_METHOD(Phalcon_Arr, pluck);
PHP_METHOD(Phalcon_Arr, unshift);
PHP_METHOD(Phalcon_Arr, map);
PHP_METHOD(Phalcon_Arr, merge);
PHP_METHOD(Phalcon_Arr, overwrite);
PHP_METHOD(Phalcon_Arr, callback);
PHP_METHOD(Phalcon_Arr, flatten);
PHP_METHOD(Phalcon_Arr, arrayobject);
PHP_METHOD(Phalcon_Arr, key);
PHP_METHOD(Phalcon_Arr, filter);
PHP_METHOD(Phalcon_Arr, sum);
PHP_METHOD(Phalcon_Arr, toArray);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_is_assoc, 0, 0, 1)
	ZEND_ARG_INFO(0, array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_is_array, 0, 0, 1)
	ZEND_ARG_INFO(0, array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_path, 0, 0, 2)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, default_value)
	ZEND_ARG_INFO(0, delimiter)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_set_path, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, delimiter)
	ZEND_ARG_INFO(0, flag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_range, 0, 0, 0)
	ZEND_ARG_INFO(0, step)
	ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_get, 0, 0, 2)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, default_value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_choice, 0, 0, 3)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value1)
	ZEND_ARG_INFO(0, value2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_extract, 0, 0, 2)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, paths)
	ZEND_ARG_INFO(0, default_value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_pluck, 0, 0, 2)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_unshift, 0, 0, 3)
	ZEND_ARG_INFO(1, array)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_map, 0, 0, 2)
	ZEND_ARG_INFO(0, callbacks)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, keys)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_merge, 0, 0, 2)
	ZEND_ARG_INFO(0, array1)
	ZEND_ARG_INFO(0, array2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_overwrite, 0, 0, 2)
	ZEND_ARG_INFO(0, array1)
	ZEND_ARG_INFO(0, array2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_callback, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_flatten, 0, 0, 1)
	ZEND_ARG_INFO(0, array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_arrayobject, 0, 0, 1)
	ZEND_ARG_INFO(0, array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_key, 0, 0, 1)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, postion)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_filter, 0, 0, 1)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_sum, 0, 0, 1)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_toarray, 0, 0, 1)
	ZEND_ARG_INFO(0, object)
	ZEND_ARG_INFO(0, recursive)
	ZEND_ARG_INFO(0, properties)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_arr_method_entry[] = {
	PHP_ME(Phalcon_Arr, is_assoc, arginfo_phalcon_arr_is_assoc, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, is_array, arginfo_phalcon_arr_is_array, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, path, arginfo_phalcon_arr_path, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, set_path, arginfo_phalcon_arr_set_path, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, range, arginfo_phalcon_arr_range, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, get, arginfo_phalcon_arr_get, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, choice, arginfo_phalcon_arr_choice, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, extract, arginfo_phalcon_arr_extract, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, pluck, arginfo_phalcon_arr_pluck, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, unshift, arginfo_phalcon_arr_unshift, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, map, arginfo_phalcon_arr_map, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, merge, arginfo_phalcon_arr_merge, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, overwrite, arginfo_phalcon_arr_overwrite, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, callback, arginfo_phalcon_arr_callback, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, flatten, arginfo_phalcon_arr_flatten, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, arrayobject, arginfo_phalcon_arr_arrayobject, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, key, arginfo_phalcon_arr_key, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, filter, arginfo_phalcon_arr_filter, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, sum, arginfo_phalcon_arr_sum, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Arr, toArray, arginfo_phalcon_arr_toarray, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Arr initializer
 */
PHALCON_INIT_CLASS(Phalcon_Arr){

	PHALCON_REGISTER_CLASS(Phalcon, Arr, arr, phalcon_arr_method_entry, 0);

	// zend_declare_class_constant_stringl(phalcon_arr_ce, SL("delimiter"), SL("."));
	zend_declare_property_string(phalcon_arr_ce, SL("delimiter"), ".", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);

	return SUCCESS;
}

/**
 * Tests if an array is associative or not.
 *
 *     // Returns TRUE
 *     \Phalcon\Arr::is_assoc(array('username' => 'john.doe'))
 *
 * @param array $array
 * @return boolean
 */
PHP_METHOD(Phalcon_Arr, is_assoc){

	zval *array;

	phalcon_fetch_params(0, 1, 0, &array);

	if (phalcon_array_is_associative(array)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

/**
 * Test if a value is an array with an additional check for array-like objects.
 *
 *     // Returns TRUE
 *     \Phalcon\Arr::is_array(array());
 *
 * @param mixed $value
 * @return boolean
 */
PHP_METHOD(Phalcon_Arr, is_array){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	if (Z_TYPE_P(value) == IS_ARRAY) {
		RETURN_TRUE;
	}

	 RETURN_BOOL(Z_TYPE_P(value) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(value), zend_ce_traversable, 1));
}

/**
 * Gets a value from an array using a dot separated path.
 *
 *     // Get the value of $array['foo']['bar']
 *     $value = \Phalcon\Arr::path($array, 'foo.bar');
 *
 * Using a wildcard "*" will search intermediate arrays and return an array.
 *
 *     // Get the values of "color" in theme
 *     $colors = \Phalcon\Arr::path($array, 'theme.*.color');
 *
 *     // Using an array of keys
 *     $colors = \Phalcon\Arr::path($array, array('theme', '*', 'color'));
 *
 * @param array $array
 * @param mixed $path
 * @param mixed $default
 * @param string $delimiter
 * @return mixed
 */
PHP_METHOD(Phalcon_Arr, path){

	zval *array, *path, *default_value = NULL, *_delimiter = NULL, delimiter = {}, is_array = {}, keys = {}, key = {}, *arr = NULL;

	phalcon_fetch_params(0, 2, 2, &array, &path, &default_value, &_delimiter);
	PHALCON_SEPARATE_PARAM(array);

	if (!_delimiter || Z_TYPE_P(_delimiter) == IS_NULL) {
		phalcon_read_static_property_ce(&delimiter, phalcon_arr_ce, SL("delimiter"), PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&delimiter, _delimiter);
	}

	PHALCON_CALL_SELF(&is_array, "is_array", array);

	if (!zend_is_true(&is_array)) {
		goto end;
	}

	if (Z_TYPE_P(path) == IS_ARRAY) {
		PHALCON_SEPARATE_PARAM(path);
		ZVAL_COPY_VALUE(&keys, path);
	} else {
		if (phalcon_array_isset_fetch(return_value, array, path, PH_COPY)) {
			return;
		}

		phalcon_fast_explode(&keys, &delimiter, path);
	}

	do {
		zval values = {};
		ZVAL_MAKE_REF(&keys);
		PHALCON_CALL_FUNCTION(&key, "array_shift", &keys);
		ZVAL_UNREF(&keys);

		if (Z_TYPE(key) == IS_NULL) {
			break;
		}

		if (phalcon_array_isset_fetch(&values, array, &key, 0)) {
			if (phalcon_fast_count_ev(&keys) > 0) {
				PHALCON_CALL_SELF(&is_array, "is_array", &values);
				if (zend_is_true(&is_array)) {
					ZVAL_COPY_VALUE(array, &values);
				} else {
					// Unable to dig deeper
					break;
				}
			} else {
				RETURN_CTOR(&values);
			}
		} else if (PHALCON_IS_STRING(&key, "*")) {
			array_init(return_value);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), arr) {
				zval value = {};

				PHALCON_CALL_SELF(&value, "path", arr, &keys);

				if (Z_TYPE(value) != IS_NULL) {
					phalcon_array_append(return_value, &value, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();

			if (phalcon_fast_count_ev(return_value)) {
				// Found the values requested
				return;
			} else {
				// Unable to dig deeper
				break;
			}
		} else {
			// Unable to dig deeper
			break;
		}
	} while (phalcon_fast_count_ev(&keys));

end:
	if (default_value) {
		RETURN_CTOR(default_value);
	}

	RETURN_NULL();
}

/**
 * Set a value on an array by path.
 *
 * Using a wildcard "*" will search intermediate arrays and return an array.
 *
 *     // Set the values of "color" in theme
 *     $array = array('theme' => array('one' => array('color' => 'green'), 'two' => array('size' => 11));
 *     \Phalcon\Arr::set_path($array, 'theme.*.color', 'red');
 *     // Result: array('theme' => array('one' => array('color' => 'red'), 'two' => array('size' => 11, 'color' => 'red'));
 *
 * @param array $array
 * @param string $path
 * @param mixed $value
 * @param string $delimiter
 */
PHP_METHOD(Phalcon_Arr, set_path){

	zval *array, *path, *value, *_delimiter = NULL, *flag = NULL, delimiter = {}, keys = {}, cpy_array = {}, key = {};
	int found = 1;

	phalcon_fetch_params(0, 3, 2, &array, &path, &value, &_delimiter, &flag);
	ZVAL_DEREF(array);

	if (Z_TYPE_P(path) == IS_ARRAY) {
		PHALCON_SEPARATE_PARAM(path);
		ZVAL_COPY_VALUE(&keys, path);
	} else {
		if (!_delimiter || Z_TYPE_P(_delimiter) == IS_NULL) {
			phalcon_read_static_property_ce(&delimiter, phalcon_arr_ce, SL("delimiter"), PH_READONLY);
		} else {
			ZVAL_COPY_VALUE(&delimiter, _delimiter);
		}

		phalcon_fast_explode(&keys, &delimiter, path);
	}

	if (!flag) {
		flag = &PHALCON_GLOBAL(z_false);
	}

	ZVAL_COPY_VALUE(&cpy_array, array);

	// Set current $array to inner-most array  path
	while ((int) zend_hash_num_elements(Z_ARRVAL(keys)) > 1) {

		ZVAL_MAKE_REF(&keys);
		PHALCON_CALL_FUNCTION(&key, "array_shift", &keys);
		ZVAL_UNREF(&keys);

		if (PHALCON_IS_STRING(&key, "*")) {
			zval *arr = NULL;
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(cpy_array), arr) {
				zval is_array = {};
				PHALCON_CALL_SELF(&is_array, "is_array", arr);

				if (zend_is_true(&is_array)) {
					ZVAL_MAKE_REF(arr);
					PHALCON_CALL_SELF(NULL, "set_path", arr, &keys, value, &PHALCON_GLOBAL(z_null), flag);
					ZVAL_UNREF(arr);
				}
			} ZEND_HASH_FOREACH_END();
			found = 0;
			break;
		} else {
			zval v = {};

			if (phalcon_is_long(&key)) {
				convert_to_long(&key);
			}

			if (phalcon_array_isset_fetch(&v, &cpy_array, &key, 0)) {
				ZVAL_COPY_VALUE(&cpy_array, &v);
			} else {
				array_init(&v);
				phalcon_array_update(&cpy_array, &key, &v, PH_COPY);
				zval_ptr_dtor(&v);
			}
		}
	}

	if (found) {
		ZVAL_MAKE_REF(&keys);
		PHALCON_CALL_FUNCTION(&key, "array_shift", &keys);
		ZVAL_UNREF(&keys);

		if (zend_is_true(flag)) {
			zval v = {};
			if (phalcon_array_isset_fetch(&v, &cpy_array, &key, 0)) {
				if (Z_TYPE(v) != IS_ARRAY) {
					convert_to_array(&v);
				}
			} else {
				array_init(&v);
			}
			phalcon_array_append(&v, value, PH_COPY);
			phalcon_array_update(&cpy_array, &key, &v, PH_COPY);
		} else {
			phalcon_array_update(&cpy_array, &key, value, PH_COPY);
		}
	}
}

/**
 * Fill an array with a range of numbers.
 *
 *     // Fill an array with values 5, 10, 15, 20
 *     $values = \Phalcon\Arr::range(5, 20);
 *
 * @param integer $step
 * @param integer $max
 * @return array
 */
PHP_METHOD(Phalcon_Arr, range){

	zval *step = NULL, *max = NULL;
	int i, s, m;

	phalcon_fetch_params(0, 0, 2, &step, &max);

	if (!step) {
		s = 10;
	} else {
		if (Z_TYPE_P(step) != IS_LONG) {
			PHALCON_SEPARATE_PARAM(step);
			convert_to_long(step);
		}
		s = Z_LVAL_P(step);

		if (s < 1) {
			RETURN_EMPTY_ARRAY();
		}
	}

	if (!max) {
		m = 100;
	} else {
		if (Z_TYPE_P(max) != IS_LONG) {
			PHALCON_SEPARATE_PARAM(max);
			convert_to_long(max);
		}

		m = Z_LVAL_P(max);
	}

	array_init(return_value);
	for (i = s; i <= m; i += s) {
		phalcon_array_update_long_long(return_value, i, i, PH_COPY);
	}
}

/**
 * Retrieve a single key from an array. If the key does not exist in the
 * array, the default value will be returned instead.
 *
 *     // Get the value "username" from $_POST, if it exists
 *     $username = \Phalcon\Arr::get($_POST, 'username');
 *
 * @param array $array
 * @param string|array|\Closure $key
 * @param mixed $default_value
 * @return mixed
 */
PHP_METHOD(Phalcon_Arr, get){

	zval *array, *keys, *default_value = NULL, *key, arguments = {}, value = {};

	phalcon_fetch_params(0, 2, 1, &array, &keys, &default_value);

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(keys) == IS_OBJECT && instanceof_function(Z_OBJCE_P(keys), zend_ce_closure)) {
		array_init_size(&arguments, 2);
		phalcon_array_append(&arguments, array, PH_COPY);
		phalcon_array_append(&arguments, default_value, PH_COPY);
		PHALCON_CALL_USER_FUNC_ARRAY(return_value, keys, &arguments);
		zval_ptr_dtor(&arguments);
		return;
	}

	if (Z_TYPE_P(array) == IS_ARRAY) {
		if (Z_TYPE_P(keys) == IS_ARRAY) {
			array_init(return_value);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(keys), key) {
				zval value0 = {};
				if (phalcon_array_isset_fetch(&value0, array, key, 0)) {
					phalcon_array_update(return_value, key, &value0, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
			if (phalcon_fast_count_ev(return_value)) {
				return;
			}
		} else if (phalcon_array_isset_fetch(&value, array, keys, 0)) {
			RETURN_CTOR(&value);
		}
	} else if (Z_TYPE_P(array) == IS_OBJECT) {
		if (Z_TYPE_P(keys) == IS_ARRAY) {
			array_init(return_value);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(keys), key) {
				zval value0 = {};
				if (phalcon_property_isset_fetch_zval(&value0, array, key, PH_READONLY)) {
					phalcon_array_update(return_value, key, &value0, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
			if (phalcon_fast_count_ev(return_value)) {
				return;
			}
		} else if (phalcon_property_isset_fetch_zval(&value, array, keys, PH_READONLY)) {
			RETURN_CTOR(&value);
		}
	}

	RETURN_CTOR(default_value);
}

/**
 * Choice one value, If the key does not exist in the array, the value2 will be returned instead.
 *
 *     // Choice the "value1", if exists the value "email" of $_POST
 *     $username = \Phalcon\Arr::choice($_POST, 'email', 'value1', 'value2');
 *
 * @param array $array
 * @param string $key
 * @param string $value1
 * @param string $value2
 * @return mixed
 */
PHP_METHOD(Phalcon_Arr, choice){

	zval *array, *key, *value1, *value2 = NULL;

	phalcon_fetch_params(0, 3, 1, &array, &key, &value1, &value2);

	if (!value2) {
		value2 = &PHALCON_GLOBAL(z_null);
	}

	if (phalcon_array_isset(array, key)) {
		RETURN_CTOR(value1);
	}

	RETURN_CTOR(value2);
}

/**
 * Retrieves multiple paths from an array. If the path does not exist in the
 * array, the default value will be added instead.
 *
 *     // Get the values "username", "password" from $_POST
 *     $auth = \Phalcon\Arr::extract($_POST, array('username', 'password'));
 *
 *     // Get the value "level1.level2a" from $data
 *     $data = array('level1' => array('level2a' => 'value 1', 'level2b' => 'value 2'));
 *     \Phalcon\Arr::extract($data, array('level1.level2a', 'password'));
 *
 * @param array $array
 * @param array $paths
 * @param mixed $default_value
 * @return array
 */
PHP_METHOD(Phalcon_Arr, extract){

	zval *array, *paths, *default_value = NULL, *path;

	phalcon_fetch_params(0, 2, 1, &array, &paths, &default_value);

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	array_init(return_value);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(paths), path) {
		zval value = {};
		PHALCON_CALL_SELF(&value, "path", array, path, default_value);

		ZVAL_MAKE_REF(return_value);
		PHALCON_CALL_SELF(NULL, "set_path", return_value, path, &value);
		ZVAL_UNREF(return_value);
	} ZEND_HASH_FOREACH_END();
}

/**
 * Retrieves muliple single-key values from a list of arrays.
 *
 *     // Get all of the "id" values from a result
 *     $ids = \Phalcon\Arr::pluck($result, 'id');
 *
 * @param array $array
 * @param string $key
 * @return array
 */
PHP_METHOD(Phalcon_Arr, pluck){

	zval *array, *key, *row;

	phalcon_fetch_params(0, 2, 0, &array, &key);

	array_init(return_value);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), row) {
		zval value = {};
		if (phalcon_array_isset_fetch(&value, row, key, 0)) {
			phalcon_array_append(return_value, &value, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();
}

/**
 * Adds a value to the beginning of an associative array.
 *
 *     // Add an empty value to the start of a select list
 *     \Phalcon\Arr::unshift($array, 'none', 'Select a value');
 *
 * @param array $array
 * @param string $key
 * @param mixed $val
 * @return array
 */
PHP_METHOD(Phalcon_Arr, unshift){

	zval *array, *key, *val, tmp = {};

	phalcon_fetch_params(0, 3, 0, &array, &key, &val);

	PHALCON_CALL_FUNCTION(&tmp, "array_reverse", array, &PHALCON_GLOBAL(z_true));

	phalcon_array_update(&tmp, key, val, PH_COPY);

	PHALCON_RETURN_CALL_FUNCTION("array_reverse", &tmp, &PHALCON_GLOBAL(z_true));
}

/**
 * Recursive version of [array_map](http://php.net/array_map), applies one or more
 * callbacks to all elements in an array, including sub-arrays.
 *
 *     // Apply "strip_tags" to every element in the array
 *     $array = \Phalcon\Arr::map('strip_tags', $array);
 *
 *     // Apply $this->filter to every element in the array
 *     $array = \Phalcon\Arr::map(array(array($this,'filter')), $array);
 *
 * @param mixed $callbacks
 * @param array $array
 * @param array $keys
 * @return array
 */
PHP_METHOD(Phalcon_Arr, map){

	zval *callbacks, *array, *keys = NULL, *val;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 1, &callbacks, &array, &keys);
	PHALCON_SEPARATE_PARAM(array);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(array), idx, str_key, val) {
		zval key = {}, value = {}, *callback, params = {};
		if (str_key) {
			ZVAL_STR(&key, str_key);
		} else {
			ZVAL_LONG(&key, idx);
		}
		if (Z_TYPE_P(val) == IS_ARRAY) {
			PHALCON_CALL_SELF(&value, "map", callbacks, val);
			phalcon_array_update(array, &key, &value, PH_COPY);
		} else if (!keys || Z_TYPE_P(keys) != IS_ARRAY || phalcon_fast_in_array(&key, keys)) {
			if (Z_TYPE_P(callbacks) == IS_ARRAY) {
				ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), callback) {
					array_init(&params);

					phalcon_array_update_long(&params, 0, val, PH_COPY);

					PHALCON_CALL_USER_FUNC_ARRAY(&value, callback, &params);

					phalcon_array_update(array, &key, &value, PH_COPY);
					zval_ptr_dtor(&params);
				} ZEND_HASH_FOREACH_END();
			} else {
				array_init(&params);

				phalcon_array_update_long(&params, 0, val, PH_COPY);

				PHALCON_CALL_USER_FUNC_ARRAY(&value, callbacks, &params);

				phalcon_array_update(array, &key, &value, PH_COPY);
				zval_ptr_dtor(&params);
			}
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_CTOR(array);
}

/**
 * Recursively merge two or more arrays. Values in an associative array
 * overwrite previous values with the same key. Values in an indexed array
 * are appended, but only when they do not already exist in the result.
 *
 * Note that this does not work the same as [array_merge_recursive](http://php.net/array_merge_recursive)!
 *
 *     $john = array('name' => 'john', 'children' => array('fred', 'paul', 'sally', 'jane'));
 *     $mary = array('name' => 'mary', 'children' => array('jane'));
 *
 *     // John and Mary are married, merge them together
 *     $john = \Phalcon\Arr::merge($john, $mary);
 *
 *     // The output of $john will now be:
 *     array('name' => 'mary', 'children' => array('fred', 'paul', 'sally', 'jane'))
 *
 * @param array $array1
 * @param array $array2,...
 * @return array
 */
PHP_METHOD(Phalcon_Arr, merge){

	zval *array1, *array2, *value, *args;
	zend_string *str_key;
	ulong idx;
	uint32_t i;

	phalcon_fetch_params(0, 2, 0, &array1, &array2);
	PHALCON_SEPARATE_PARAM(array1);
	ZVAL_COPY(return_value, array1);

	if (phalcon_array_is_associative(array2)) {
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(array2), idx, str_key, value) {
			zval tmp = {}, arr = {}, value1 = {};
			if (str_key) {
				ZVAL_STR(&tmp, str_key);
			} else {
				ZVAL_LONG(&tmp, idx);
			}
			if (Z_TYPE_P(value) == IS_ARRAY && phalcon_array_isset_fetch(&value1, array1, &tmp, 0)) {
				if (Z_TYPE(value1) == IS_ARRAY) {
					PHALCON_CALL_SELF(&arr, "merge", &value1, value);

					phalcon_array_update(return_value, &tmp, &arr, PH_COPY);
				} else {
					phalcon_array_update(return_value, &tmp, value, PH_COPY);
				}
			} else {
				phalcon_array_update(return_value, &tmp, value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array2), value) {
			if (!phalcon_fast_in_array(value, array1)) {
				phalcon_array_append(return_value, value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}

	if (ZEND_NUM_ARGS() > 2) {
		args = (zval *)safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval), 0);
		if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
			efree(args);
			WRONG_PARAM_COUNT;
		}

		for (i = 2; i < ZEND_NUM_ARGS(); i++) {
			PHALCON_CALL_SELF(return_value, "merge", return_value, &args[i]);
		}
	}
}

/**
 * Overwrites an array with values from input arrays.
 * Keys that do not exist in the first array will not be added!
 *
 *     $a1 = array('name' => 'john', 'mood' => 'happy', 'food' => 'bacon');
 *     $a2 = array('name' => 'jack', 'food' => 'tacos', 'drink' => 'beer');
 *
 *     // Overwrite the values of $a1 with $a2
 *     $array = \Phalcon\Arr::overwrite($a1, $a2);
 *
 *     // The output of $array will now be:
 *     array('name' => 'jack', 'mood' => 'happy', 'food' => 'tacos')
 *
 * @param array $array1
 * @param array $array2
 * @return array
 */
PHP_METHOD(Phalcon_Arr, overwrite){

	zval *array1, *array2, array = {}, *value, *args;
	zend_string *key;
	ulong idx;
	uint32_t i;

	phalcon_fetch_params(0, 2, 0, &array1, &array2);
	PHALCON_SEPARATE_PARAM(array1);
	ZVAL_COPY(return_value, array1);

	if (Z_TYPE_P(array2) != IS_ARRAY) {
		PHALCON_SEPARATE_PARAM(array2);
		convert_to_array(array2);
	}

	PHALCON_CALL_FUNCTION(&array, "array_intersect_key", array2, return_value);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(array), idx, key, value) {
		if (key) {
			phalcon_array_update_string(return_value, key, value, PH_COPY);
		} else {
			phalcon_array_update_long(return_value, idx, value, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();

	if (ZEND_NUM_ARGS() > 2) {
		args = (zval *)safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval), 0);
		if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
			efree(args);
			WRONG_PARAM_COUNT;
		}

		for (i = 2; i < ZEND_NUM_ARGS(); i++) {
			PHALCON_CALL_SELF(return_value, "overwrite", return_value, &args[i]);
		}
	}
}

/**
 * Creates a callable function and parameter list from a string representation.
 * Note that this function does not validate the callback string.
 *
 *     // Get the callback function and parameters
 *     list($func, $params) = \Phalcon\Arr::callback('Foo::bar(apple,orange)');
 *
 *     // Get the result of the callback
 *     $result = call_user_func_array($func, $params);
 *
 * @param string $str
 * @return array function, params
 */
PHP_METHOD(Phalcon_Arr, callback){

	zval *str, pattern = {}, matches = {}, ret = {}, command = {}, match = {}, split = {}, search = {}, replace = {}, params = {}, command_parts = {};
	pcre_cache_entry *pce;

	phalcon_fetch_params(0, 1, 0, &str);

	ZVAL_STRING(&pattern, "#^([^\\(]*+)\\((.*)\\)$#");

	ZVAL_NULL(&matches);
	ZVAL_MAKE_REF(&matches);
	RETURN_ON_FAILURE(phalcon_preg_match(&ret, &pattern, str, &matches));
	ZVAL_UNREF(&matches);

	if (zend_is_true(&ret)) {
		if (!phalcon_array_isset_fetch_long(&command, &matches, 1)) {
			ZVAL_EMPTY_STRING(&command);
		}

		if (phalcon_array_isset_fetch_long(&match, &matches, 2)) {
			if ((pce = pcre_get_compiled_regex_cache(SSL("#(?<!\\\\\\\\),#"))) == NULL) {
				RETURN_FALSE;
			}

			php_pcre_split_impl(pce, Z_STRVAL(match), Z_STRLEN(match), &split, -1, 0);

			ZVAL_STRING(&search, "\\,");
			ZVAL_STRING(&replace, ",");

			PHALCON_CALL_FUNCTION(&params, "str_replace", &search, &replace, &split);
		}
	} else {
		ZVAL_COPY_VALUE(&command, str);
	}

	array_init(return_value);

	if (phalcon_memnstr_str(&command, SL("::"))) {
		phalcon_fast_explode_str(&command_parts, SL("::"), &command);
		phalcon_array_append(return_value, &command_parts, PH_COPY);
	} else {
		phalcon_array_append(return_value, &command, PH_COPY);
	}

	phalcon_array_append(return_value, &params, PH_COPY);
}

/**
 * Convert a multi-dimensional array into a single-dimensional array.
 *
 *     $array = array('set' => array('one' => 'something'), 'two' => 'other');
 *
 *     // Flatten the array
 *     $array = \Phalcon\Arr::flatten($array);
 *
 *     // The array will now be
 *     array('one' => 'something', 'two' => 'other');
 *
 * @param array $array
 * @return array
 */
PHP_METHOD(Phalcon_Arr, flatten){

	zval *array, is_assoc = {}, *value;
	zend_string *key;
	ulong idx;

	phalcon_fetch_params(0, 1, 0, &array);

	PHALCON_CALL_SELF(&is_assoc, "is_assoc", array);

	array_init(return_value);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(array), idx, key, value) {
		zval arr = {};
		if (Z_TYPE_P(value) == IS_ARRAY) {
			PHALCON_CALL_SELF(&arr, "flatten", value);

			php_array_merge(Z_ARRVAL_P(return_value), Z_ARRVAL(arr));
		} else {
			if (zend_is_true(&is_assoc)) {
				if (key) {
					phalcon_array_update_string(return_value, key, value, PH_COPY);
				} else {
					phalcon_array_update_long(return_value, idx, value, PH_COPY);
				}
			} else {
				phalcon_array_append(return_value, value, PH_COPY);
			}
		}
	} ZEND_HASH_FOREACH_END();
}

/**
 * Convert a array to a array object.
 *
 *     $array = array('name' => 'Phalcon7', 'version' => '1.0.x');
 *
 *     $arrayobject = \Phalcon\Arr::arrayobject($array);
 *
 * @param array $array
 * @return ArrayObject
 */
PHP_METHOD(Phalcon_Arr, arrayobject){

	zval *array, arrayobject = {};

	phalcon_fetch_params(0, 1, 0, &array);

	object_init_ex(&arrayobject, spl_ce_ArrayObject);
	PHALCON_CALL_METHOD(NULL, &arrayobject, "__construct", array);

	RETURN_CTOR(&arrayobject);
}

/**
 * Gets array key of the postion
 *
 *     $array = array('name' => 'Phalcon7', 'version' => '1.0.x');
 *
 *     $key = \Phalcon\Arr::key($array, 1);
 *
 * @param array $array
 * @param int $postion
 * @return mixed
 */
PHP_METHOD(Phalcon_Arr, key){

	zval *array, *postion = NULL, arrayobject = {}, arrayiterator = {}, ret = {};

	phalcon_fetch_params(0, 1, 1, &array, &postion);

	object_init_ex(&arrayobject, spl_ce_ArrayObject);
	PHALCON_CALL_METHOD(NULL, &arrayobject, "__construct", array);

	PHALCON_CALL_METHOD(&arrayiterator, &arrayobject, "getIterator");

	PHALCON_CALL_METHOD(&ret, &arrayiterator, "valid");
	if (!zend_is_true(&ret)) {
		RETURN_NULL();
	}

	if (postion) {
		PHALCON_CALL_METHOD(NULL, &arrayiterator, "seek", postion);
	}

	PHALCON_CALL_METHOD(return_value, &arrayiterator, "key");
}

/**
 * Filters elements of an array using a the filter
 *
 *     $array = array('name' => 'Phalcon7', 'version' => '1.0.x');
 *
 *     $key = \Phalcon\Arr::filter($array, 'int');
 *
 * @param array $array
 * @param mixed $filters
 * @return array
 */
PHP_METHOD(Phalcon_Arr, filter){

	zval *array, *filters = NULL, dependency_injector = {}, service = {}, filter = {}, *value;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 1, &array, &filters);

	if (!filters || Z_TYPE_P(filters) == IS_NULL) {
		PHALCON_RETURN_CALL_FUNCTION("array_filter", array);
		return;
	}

	PHALCON_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");

	ZVAL_STRING(&service, ISV(filter));

	PHALCON_CALL_METHOD(&filter, &dependency_injector, "getshared", &service);
	PHALCON_VERIFY_INTERFACE(&filter, phalcon_filterinterface_ce);

	array_init(return_value);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(array), idx, str_key, value) {
		zval filter_value = {};
		PHALCON_CALL_METHOD(&filter_value, &filter, "sanitize", value, filters);
		if (str_key) {
			phalcon_array_update_string(return_value, str_key, &filter_value, PH_COPY);
		} else {
			phalcon_array_update_long(return_value, idx, &filter_value, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();
}

/**
 * Return the sum of all the values in the array using a dot separated path
 *
 * @param array $array
 * @param mixed $path
 * @param mixed $default
 * @param string $delimiter
 * @return mixed
 */
PHP_METHOD(Phalcon_Arr, sum){

	zval *array, *path, *default_value = NULL, *_delimiter = NULL, delimiter = {}, values = {};

	phalcon_fetch_params(0, 2, 2, &array, &path, &default_value, &_delimiter);

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!_delimiter || Z_TYPE_P(_delimiter) == IS_NULL) {
		phalcon_read_static_property_ce(&delimiter, phalcon_arr_ce, SL("delimiter"), PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&delimiter, _delimiter);
	}

	PHALCON_CALL_SELF(&values, "path", array, path, default_value, &delimiter);

	if (Z_TYPE(values) == IS_ARRAY) {
		PHALCON_RETURN_CALL_FUNCTION("array_sum", &values);
		return;
	} else {
		RETURN_CTOR(&values);
	}
}
/**
 * Converts an object or an array of objects into an array
 *
 *<code>
 *	print_r(Phalcon\Arr::toArray($user);
 *</code>
 *
 * @param object|array|string $object
 * @param array $properties
 * @param bool $recursive
 * @return array
 */
PHP_METHOD(Phalcon_Arr, toArray){

	zval *object, *properties = NULL, *recursive = NULL, *value;
	zend_string *key;
	ulong idx;

	phalcon_fetch_params(0, 1, 2, &object, &properties, &recursive);

	phalcon_get_object_vars(return_value, getThis(), 0);

	if (Z_TYPE_P(object) == IS_OBJECT) {
		if (phalcon_method_exists_ex(object, SL("toarray")) == SUCCESS) {
			PHALCON_CALL_METHOD(return_value, object, "toarray");
			return;
		}
		phalcon_get_object_vars(return_value, object, 1);
		if (!recursive || zend_is_true(recursive)) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(return_value), idx, key, value) {
				zval tmp = {}, array_value = {};
				if (key) {
					ZVAL_STR(&tmp, key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}

				if (Z_TYPE_P(value) == IS_OBJECT && Z_TYPE_P(value) == IS_ARRAY) {
					PHALCON_CALL_CE_STATIC(&array_value, phalcon_arr_ce, "toarray", value);
					phalcon_array_update(return_value, &tmp, &array_value, PH_COPY);
				} else {
					phalcon_array_update(return_value, &tmp, value, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
		}
	} else if (Z_TYPE_P(object) == IS_ARRAY) {
		ZVAL_ARR(return_value, zend_array_dup(Z_ARRVAL_P(object)));
		if (!recursive || zend_is_true(recursive)) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(return_value), idx, key, value) {
				zval tmp = {}, array_value = {};
				if (key) {
					ZVAL_STR(&tmp, key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}
				if (Z_TYPE_P(value) == IS_OBJECT && Z_TYPE_P(value) == IS_ARRAY) {
					PHALCON_CALL_CE_STATIC(&array_value, phalcon_arr_ce, "toarray", value);
					phalcon_array_update(return_value, &tmp, &array_value, PH_COPY);
				} else {
					phalcon_array_update(return_value, &tmp, value, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
		}
	} else {
		array_init_size(return_value, 1);
		phalcon_array_append(return_value, object, PH_COPY);
	}
}
