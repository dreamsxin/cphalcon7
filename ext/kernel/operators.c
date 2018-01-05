
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

#include "kernel/operators.h"

#include <ext/standard/php_string.h>
#include <ext/standard/php_math.h>
#include <ext/spl/php_spl.h>
#include <Zend/zend_operators.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/string.h"


int phalcon_make_printable_zval(zval *expr, zval *expr_copy){
	int use_copy = zend_make_printable_zval(expr, expr_copy);
	if (use_copy) {
		Z_SET_REFCOUNT_P(expr_copy, 1);
		ZVAL_UNREF(expr_copy);
	}
	return use_copy;
}

/**
 * Performs logical AND function operator
 */
int phalcon_and_function(zval *result, zval *left, zval *right){
	int istrue = zend_is_true(left) && zend_is_true(right);
	ZVAL_BOOL(result, istrue);
	return SUCCESS;
}

int phalcon_compare(zval *op1, zval *op2) {
	zval result;

	if (compare_function(&result, op1, op2)==FAILURE) {
		return 1;
	}
	return Z_LVAL(result);
}

/**
 * Natural compare with string operandus on right
 */
int phalcon_compare_strict_string(zval *op1, const char *op2, int op2_length){

	switch (Z_TYPE_P(op1)) {
		case IS_STRING:
			if (!Z_STRLEN_P(op1) && !op2_length) {
				return 1;
			}
			if (Z_STRLEN_P(op1) != op2_length) {
				return 0;
			}
			return !zend_binary_strcmp(Z_STRVAL_P(op1), Z_STRLEN_P(op1), op2, op2_length);
		case IS_NULL:
			return !zend_binary_strcmp("", 0, op2, op2_length);
		case IS_TRUE:
			return !zend_binary_strcmp("1", strlen("1"), op2, op2_length);
		case IS_FALSE:
			return !zend_binary_strcmp("0", strlen("0"), op2, op2_length);
	}

	return 0;
}

/**
 * Natural compare with long operandus on right
 */
int phalcon_compare_strict_long(zval *op1, zend_long op2){
	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return Z_LVAL_P(op1) == op2;
		case IS_DOUBLE:
			return Z_LVAL_P(op1) == (double) op2;
		case IS_NULL:
			return 0 == op2;
		case IS_TRUE:
			return 1 == op2;
		case IS_FALSE:
			return 0 == op2;
		default: {
			zval result = {}, op2_tmp = {};
			ZVAL_LONG(&op2_tmp, op2);
			is_equal_function(&result, op1, &op2_tmp);
			return Z_TYPE(result) == IS_TRUE ? 1 : 0;
		}
	}

	return 0;
}

/**
* Natural compare with double operandus on right
 */
int phalcon_compare_strict_double(zval *op1, double op2) {

	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return Z_LVAL_P(op1) == (zend_long) op2;
		case IS_DOUBLE:
			return Z_DVAL_P(op1) == op2;
		case IS_NULL:
			return 0 == op2;
		case IS_TRUE:
			return 1 == op2;
		case IS_FALSE:
			return 0 == op2;
		default:
			{
				zval result = {}, op2_tmp = {};
				ZVAL_DOUBLE(&op2_tmp, op2);
				is_equal_function(&result, op1, &op2_tmp);
				return Z_TYPE(result) == IS_TRUE ? 1 : 0;
			}
	}

	return 0;
}

/**
 * Natural compare with bool operandus on right
 */
int phalcon_compare_strict_bool(zval *op1, zend_bool op2) {

	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return (Z_LVAL_P(op1) ? 1 : 0) == op2;
		case IS_DOUBLE:
			return (Z_DVAL_P(op1) ? 1 : 0) == op2;
		case IS_NULL:
			return 0 == op2;
		case IS_TRUE:
			return 1 == op2;
		default:
			{
				zval result = {}, op2_tmp = {};
				ZVAL_BOOL(&op2_tmp, op2);
				is_equal_function(&result, op1, &op2_tmp);
				return Z_TYPE(result) == IS_TRUE ? 1 : 0;
			}
	}

	return 0;
}

void phalcon_negate(zval *z) {
	while (1) {
		switch (Z_TYPE_P(z)) {
			case IS_TRUE:
				ZVAL_LONG(z, -1);
				return;

			case IS_FALSE:
				ZVAL_LONG(z, 0);
				return;

			case IS_LONG:
			case IS_DOUBLE:
				ZVAL_DOUBLE(z, -Z_DVAL_P(z));
				return;

			case IS_NULL:
				ZVAL_LONG(z, 0);
				return;

			default:
				convert_scalar_to_number(z);
				assert(Z_TYPE_P(z) == IS_LONG || Z_TYPE_P(z) == IS_DOUBLE);
		}
	}
}

void phalcon_convert_to_object(zval *op) {
    convert_to_object(op);
}
/**
 * Cast variables converting they to other types
 */
void phalcon_cast(zval *result, zval *var, uint32_t type){

	ZVAL_ZVAL(result, var, 1, 0);

	switch (type) {
		case IS_STRING:
			convert_to_string(result);
			break;
		case IS_LONG:
			convert_to_long(result);
			break;
		case IS_DOUBLE:
			convert_to_double(result);
			break;
		/*case IS_BOOL:
			convert_to_bool(result);
			break;*/
		case IS_ARRAY:
			if (Z_TYPE_P(result) != IS_ARRAY) {
				convert_to_array(result);
			}
			break;
	}

}

/**
 * Returns the long value of a zval
 */
zend_long phalcon_get_intval_ex(const zval *op) {

	switch (Z_TYPE_P(op)) {
        case IS_ARRAY:
            return zend_hash_num_elements(Z_ARRVAL_P(op)) ? 1 : 0;
            break;

	    case IS_CALLABLE:
	    case IS_RESOURCE:
	    case IS_OBJECT:
	        return 1;

		case IS_LONG:
			return Z_LVAL_P(op);
		case IS_TRUE:
			return 1;
		case IS_FALSE:
			return 0;
		case IS_DOUBLE:
			return (zend_long) Z_DVAL_P(op);
		case IS_STRING: {
			zend_long long_value;
			double double_value;
			zend_uchar type;

			ASSUME(Z_STRVAL_P(op) != NULL);
			type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), &long_value, &double_value, 0);
			if (type == IS_LONG) {
				return long_value;
			}
			if (type == IS_DOUBLE) {
				return (long)double_value;
			}
			return 0;
		}
	}

	return 0;
}

/**
 * Returns the double value of a zval
 */
double phalcon_get_doubleval_ex(const zval *op) {

	switch (Z_TYPE_P(op)) {
        case IS_ARRAY:
            return zend_hash_num_elements(Z_ARRVAL_P(op)) ? (double) 1 : 0;
            break;
	    case IS_CALLABLE:
	    case IS_RESOURCE:
	    case IS_OBJECT:
	        return (double) 1;
		case IS_LONG:
			return (double) Z_LVAL_P(op);
		case IS_TRUE:
			return (double) 1;
		case IS_FALSE:
			return (double) 0;
		case IS_DOUBLE:
			return Z_DVAL_P(op);
		case IS_STRING: {
			zend_long long_value;
			double double_value;
			zend_uchar type;

			ASSUME(Z_STRVAL_P(op) != NULL);
			type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), &long_value, &double_value, 0);
			if (type == IS_LONG) {
				return long_value;
			}
			if (type == IS_DOUBLE) {
				return double_value;
			}
			return 0;
		}
	}

	return 0;
}

/**
 * Returns the long value of a zval
 */
zend_bool phalcon_get_boolval_ex(const zval *op) {

	int type;
	zend_long long_value = 0;
	double double_value = 0;

	switch (Z_TYPE_P(op)) {
        case IS_ARRAY:
            return zend_hash_num_elements(Z_ARRVAL_P(op)) ? (zend_bool) 1 : 0;
            break;
	    case IS_CALLABLE:
	    case IS_RESOURCE:
	    case IS_OBJECT:
	        return (zend_bool) 1;
		case IS_LONG:
			return (Z_LVAL_P(op) ? (zend_bool) 1 : 0);
		case IS_TRUE:
			return (zend_bool) 1;
		case IS_FALSE:
			return (zend_bool) 0;
		case IS_DOUBLE:
			return (Z_DVAL_P(op) ? (zend_bool) 1 : 0);
		case IS_STRING:
			if ((type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), &long_value, &double_value, 0))) {
				if (type == IS_LONG) {
					return (long_value ? (zend_bool) 1 : 0);
				} else {
					if (type == IS_DOUBLE) {
						return (double_value ? (zend_bool) 1 : 0);
					} else {
						return 0;
					}
				}
			}
	}

	return 0;
}

/**
 * Returns the long value of a zval
 */
int phalcon_is_numeric_ex(const zval *op) {

	int type;

	switch (Z_TYPE_P(op)) {
		case IS_LONG:
			return 1;
		case IS_TRUE:
			return 1;
		case IS_FALSE:
			return 0;
		case IS_DOUBLE:
			return 1;
		case IS_STRING:
			if ((type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), NULL, NULL, 0))) {
				if (type == IS_LONG || type == IS_DOUBLE) {
					return 1;
				}
			}
	}

	return 0;
}

/**
 * Returns the long value of a zval
 */
int phalcon_is_long_ex(const zval *op) {

	int type;

	switch (Z_TYPE_P(op)) {
		case IS_LONG:
			return 1;
		case IS_STRING:
			if ((type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), NULL, NULL, 0))) {
				if (type == IS_LONG) {
					return 1;
				}
			}
	}

	return 0;
}

/**
 * Check if two zvals are equal
 */
int phalcon_is_equal(zval *op1, zval *op2) {
	zval result = {};
	if (Z_TYPE_P(op1) == IS_STRING && Z_TYPE_P(op1) == Z_TYPE_P(op2)) {
		return !zend_binary_strcmp(Z_STRVAL_P(op1), Z_STRLEN_P(op1), Z_STRVAL_P(op2), Z_STRLEN_P(op2));
	}
	is_equal_function(&result, op1, op2);
	return Z_TYPE(result) == IS_TRUE ? 1 : 0;
}

/**
 * Check if a zval is equal than a long value
 */
int phalcon_is_equal_long(zval *op1, zend_long op2) {
	zval op2_zval = {};
	ZVAL_LONG(&op2_zval, op2);
	return phalcon_is_equal(op1, &op2_zval);
}

/**
 * Check if two object are equal
 */
int phalcon_is_equal_object(zval *obj1, zval *obj2) {
	zend_string *md5str;
	zend_string *md5str2;

	if (Z_TYPE_P(obj1) != IS_OBJECT && Z_TYPE_P(obj1) != IS_OBJECT) {
		return 0;
	}

	md5str = php_spl_object_hash(obj1);
	md5str2 = php_spl_object_hash(obj2);

	return zend_string_equals(md5str, md5str2);
}

/**
 * Check if a zval is less than other
 */
int phalcon_less(zval *op1, zval *op2) {
	zval result = {};
	is_smaller_function(&result, op1, op2);
	return Z_TYPE(result) == IS_TRUE ? 1 : 0;
}

/**
 * Check if a zval is less/equal than other
 */
int phalcon_less_equal(zval *op1, zval *op2) {
	zval result = {};
	is_smaller_or_equal_function(&result, op1, op2);
	return Z_TYPE(result) == IS_TRUE ? 1 : 0;
}

/**
 * Check if a zval is less than a long value
 */
int phalcon_less_long(zval *op1, zend_long op2) {
	zval result = {}, op2_zval = {};
	ZVAL_LONG(&op2_zval, op2);
	is_smaller_function(&result, op1, &op2_zval);
	return Z_TYPE(result) == IS_TRUE ? 1 : 0;
}

int phalcon_less_equal_long(zval *op1, zend_long op2) {
	zval result = {}, op2_zval = {};
	ZVAL_LONG(&op2_zval, op2);
	is_smaller_or_equal_function(&result, op1, &op2_zval);
	return Z_TYPE(result) == IS_TRUE ? 1 : 0;
}

/**
 * Check if a zval is greater than other
 */
int phalcon_greater(zval *op1, zval *op2) {
	zval result = {};
	is_smaller_or_equal_function(&result, op1, op2);
	return Z_TYPE(result) == IS_FALSE ? 1 : 0;
}

/**
 * Check if a zval is greater than a long value
 */
int phalcon_greater_long(zval *op1, zend_long op2) {
	zval result = {}, op2_zval = {};
	ZVAL_LONG(&op2_zval, op2);
	is_smaller_or_equal_function(&result, op1, &op2_zval);
	return Z_TYPE(result) == IS_FALSE ? 1 : 0;
}

/**
 * Check if a zval is greater/equal than other
 */
int phalcon_greater_equal(zval *op1, zval *op2) {
	zval result = {};
	is_smaller_function(&result, op1, op2);
	return Z_TYPE(result) == IS_FALSE ? 1 : 0;
}

/**
 * Check for greater/equal
 */
int phalcon_greater_equal_long(zval *op1, zend_long op2) {
	zval result = {}, op2_zval = {};
	ZVAL_LONG(&op2_zval, op2);
	is_smaller_function(&result, op1, &op2_zval);
	return Z_TYPE(result) == IS_FALSE ? 1 : 0;
}

/**
 * Check if two zvals are identical
 */
int phalcon_is_identical(zval *op1, zval *op2) {
	zval result = {};
	is_identical_function(&result, op1, op2);
	return Z_TYPE(result) == IS_TRUE ? 1 : 0;
}

/**
 * Do bitwise_and function keeping ref_count and is_ref
 */
int phalcon_bitwise_and_function(zval *result, zval *op1, zval *op2){
	int status;
	status = bitwise_and_function(result, op1, op2);
	return status;
}

/**
 * Do bitwise_or function keeping ref_count and is_ref
 */
int phalcon_bitwise_or_function(zval *result, zval *op1, zval *op2){
	int status;
	status = bitwise_or_function(result, op1, op2);
	return status;
}

/**
 * Do bitwise_xor function keeping ref_count and is_ref
 */
int phalcon_bitwise_xor_function(zval *result, zval *op1, zval *op2){
	int status;
	int ref_count = Z_REFCOUNT_P(result);
	int is_ref = Z_ISREF_P(result);
	status = bitwise_xor_function(result, op1, op2);
	Z_SET_REFCOUNT_P(result, ref_count);
	if (is_ref) {
		ZVAL_MAKE_REF(result);
	}
	return status;
}

/**
 * Do shiftleft function keeping ref_count and is_ref
 */
int phalcon_shift_left_function(zval *result, zval *op1, zval *op2){
	int status;
	int ref_count = Z_REFCOUNT_P(result);
	int is_ref = Z_ISREF_P(result);
	status = shift_left_function(result, op1, op2);
	Z_SET_REFCOUNT_P(result, ref_count);
	if (is_ref) {
		ZVAL_MAKE_REF(result);
	}
	return status;
}

/**
 * Do shiftright function keeping ref_count and is_ref
 */
int phalcon_shift_right_function(zval *result, zval *op1, zval *op2){
	int status;
	int ref_count = Z_REFCOUNT_P(result);
	int is_ref = Z_ISREF_P(result);
	status = shift_right_function(result, op1, op2);
	Z_SET_REFCOUNT_P(result, ref_count);
	if (is_ref) {
		ZVAL_MAKE_REF(result);
	}
	return status;
}

/**
 * Do safe divisions between two longs
 */
double phalcon_safe_div_long_long(zend_long op1, zend_long op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return (double) op1 / (double) op2;
}

/**
 * Do safe divisions between two long/double
 */
double phalcon_safe_div_long_double(zend_long op1, double op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return (double) op1 / op2;
}

/**
 * Do safe divisions between two double/long
 */
double phalcon_safe_div_double_long(double op1, zend_long op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return op1 / (double) op2;
}

/**
 * Do safe divisions between two doubles
 */
double phalcon_safe_div_double_double(double op1, double op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return op1 / op2;
}

/**
 * Do safe divisions between two zval/long
 */
double phalcon_safe_div_zval_long(zval *op1, zend_long op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op1)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ((double) phalcon_get_numberval(op1)) / (double) op2;
}

/**
 * Do safe divisions between two zval/double
 */
double phalcon_safe_div_zval_double(zval *op1, double op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op1)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ((double) phalcon_get_numberval(op1)) / op2;
}

/**
 * Do safe divisions between two long/zval
 */
double phalcon_safe_div_long_zval(zend_long op1, zval *op2) {
	if (!phalcon_get_numberval(op2)) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op2)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return (double) op1 / ((double) phalcon_get_numberval(op2));
}

/**
 * Do safe divisions between two double/zval
 */
double phalcon_safe_div_double_zval(double op1, zval *op2) {
	if (!phalcon_get_numberval(op2)) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op2)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return op1 / ((double) phalcon_get_numberval(op2));
}

/**
 * Do safe divisions between two longs
 */
zend_long phalcon_safe_mod_long_long(zend_long op1, zend_long op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return op1 % op2;
}

/**
 * Do safe divisions between two long/double
 */
zend_long phalcon_safe_mod_long_double(zend_long op1, double op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return op1 % (zend_long) op2;
}

/**
 * Do safe divisions between two double/long
 */
zend_long phalcon_safe_mod_double_long(double op1, zend_long op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return (zend_long) op1 % op2;
}

/**
 * Do safe divisions between two doubles
 */
zend_long phalcon_safe_mod_double_double(double op1, double op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return (zend_long) op1 % (zend_long) op2;
}

/**
 * Do safe divisions between two zval/long
 */
zend_long phalcon_safe_mod_zval_long(zval *op1, zend_long op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op1)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ((zend_long) phalcon_get_numberval(op1)) % (zend_long) op2;
}

/**
 * Do safe divisions between two zval/double
 */
zend_long phalcon_safe_mod_zval_double(zval *op1, double op2) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op1)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ((zend_long) phalcon_get_numberval(op1)) % (zend_long) op2;
}

/**
 * Do safe divisions between two long/zval
 */
zend_long phalcon_safe_mod_long_zval(zend_long op1, zval *op2) {
	if (!phalcon_get_numberval(op2)) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op2)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return op1 % ((zend_long) phalcon_get_numberval(op2));
}

/**
 * Do safe divisions between two double/zval
 */
zend_long phalcon_safe_mod_double_zval(double op1, zval *op2) {
	if (!phalcon_get_numberval(op2)) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op2)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return (zend_long) op1 % ((zend_long) phalcon_get_numberval(op2));
}

/**
 * Checks whether a variable has a scalar type
 */
int phalcon_is_scalar(zval *var)
{
	switch (Z_TYPE_P(var)) {
		case IS_TRUE:
		case IS_FALSE:
		case IS_DOUBLE:
		case IS_LONG:
		case IS_STRING:
			return 1;
			break;
	}

	return 0;
}

/**
 * Makes fast count on implicit array types
 */
zend_long phalcon_fast_count_int(zval *value)
{
	zval retval = {};
	zend_long result = 0;

	if (Z_TYPE_P(value) == IS_ARRAY) {
		return zend_hash_num_elements(Z_ARRVAL_P(value));
	}

	if (Z_TYPE_P(value) == IS_OBJECT) {
		if (Z_OBJ_HT_P(value)->count_elements) {
			zend_long result;
			if (SUCCESS == Z_OBJ_HT(*value)->count_elements(value, &result)) {
				return result;
			}
		}

		if (instanceof_function_ex(Z_OBJCE_P(value), spl_ce_Countable, 1)) {
			zend_call_method_with_0_params(value, Z_OBJCE_P(value), NULL, "count", &retval);
			if (!Z_ISUNDEF(retval)) {
				convert_to_long_ex(&retval);
				result = Z_LVAL(retval);
				zval_dtor(&retval);
			}

			return result;
		}

		return 0;
	}

	if (Z_TYPE_P(value) == IS_NULL) {
		return 0;
	}

	return 1;
}

/**
 * Makes fast count on implicit array types
 */
void phalcon_fast_count(zval *result, zval *value)
{
	zval retval = {};

	if (Z_TYPE_P(value) == IS_ARRAY) {
		ZVAL_LONG(result, zend_hash_num_elements(Z_ARRVAL_P(value)));
		return;
	}

	if (Z_TYPE_P(value) == IS_OBJECT) {
		if (Z_OBJ_HT_P(value)->count_elements) {
			ZVAL_LONG(result, 1);
			if (SUCCESS == Z_OBJ_HT(*value)->count_elements(value, &Z_LVAL_P(result))) {
				return;
			}
		}

		if (instanceof_function(Z_OBJCE_P(value), spl_ce_Countable)) {
			zend_call_method_with_0_params(value, NULL, NULL, "count", &retval);
			if (!Z_ISUNDEF(retval)) {
				convert_to_long_ex(&retval);
				ZVAL_LONG(result, Z_LVAL(retval));
				zval_dtor(&retval);
			}
			return;
		}

		ZVAL_LONG(result, 0);
		return;
	}

	if (Z_TYPE_P(value) == IS_NULL) {
		ZVAL_LONG(result, 0);
		return;
	}

	ZVAL_LONG(result, 1);
}

/**
 * Makes fast count on implicit array types without creating a return zval value
 */
int phalcon_fast_count_ev(zval *value)
{
	zval retval = {};
	zend_long count = 0;

	if (Z_TYPE_P(value) == IS_ARRAY) {
		return (int) zend_hash_num_elements(Z_ARRVAL_P(value)) > 0;
	}

	if (Z_TYPE_P(value) == IS_OBJECT) {
		if (Z_OBJ_HT_P(value)->count_elements) {
			Z_OBJ_HT(*value)->count_elements(value, &count);
			return (int) count > 0;
		}

		if (instanceof_function(Z_OBJCE_P(value), spl_ce_Countable)) {
			zend_call_method_with_0_params(value, NULL, NULL, "count", &retval);
			if (!Z_ISUNDEF(retval)) {
				convert_to_long_ex(&retval);
				count = Z_LVAL(retval);
				zval_dtor(&retval);
				return (int) count > 0;
			}
			return 0;
		}

		return 0;
	}

	if (Z_TYPE_P(value) == IS_NULL) {
		return 0;
	}

	return 1;
}
