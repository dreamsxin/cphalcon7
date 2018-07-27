
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

#ifndef PHALCON_KERNEL_OPERATORS_H
#define PHALCON_KERNEL_OPERATORS_H

#include "php_phalcon.h"
#include "kernel/main.h"

#include <ctype.h>

#define PHALCON_IS_SPACE(x) isspace((int)(x))

#define PHALCON_IS_TYPE(var, type)   	(PHALCON_TYPE_P(var) == type)
#define PHALCON_IS_NOT_TYPE(var, type)   !(PHALCON_IS_TYPE(var, type))

/** Strict comparing */
#define PHALCON_IS_LONG(op1, op2)   ((Z_TYPE_P(op1) == IS_LONG && Z_LVAL_P(op1) == op2) || phalcon_compare_strict_long(op1, op2))
#define PHALCON_IS_DOUBLE(op1, op2) ((Z_TYPE_P(op1) == IS_DOUBLE && Z_DVAL_P(op1) == op2) || phalcon_compare_strict_double(op1, op2))
#define PHALCON_IS_STRING(op1, op2) phalcon_compare_strict_string(op1, op2, strlen(op2))

#define PHALCON_IS_LONG_IDENTICAL(op1, op2)   (Z_TYPE_P(op1) == IS_LONG && Z_LVAL_P(op1) == op2)
#define PHALCON_IS_DOUBLE_IDENTICAL(op1, op2) (Z_TYPE_P(op1) == IS_DOUBLE && Z_DVAL_P(op1) == op2)
#define PHALCON_IS_STRING_IDENTICAL(op1, op2) (Z_TYPE_P(op1) == IS_STRING && phalcon_compare_strict_string(op1, op2, strlen(op2)))
/** strict boolean comparison */
#define PHALCON_IS_FALSE(var)       (Z_TYPE_P(var) == IS_FALSE)
#define PHALCON_IS_TRUE(var)        (Z_TYPE_P(var) == IS_TRUE)

#define PHALCON_IS_NOT_FALSE(var)   (Z_TYPE_P(var) != IS_FALSE)
#define PHALCON_IS_NOT_TRUE(var)    (Z_TYPE_P(var) != IS_TRUE)
#define PHALCON_IS_BOOL(var)        (Z_TYPE_P(var) == IS_FALSE || Z_TYPE_P(var) == IS_TRUE)

/** SQL null empty **/
#define PHALCON_IS_EMPTY_STRING(var)		(Z_TYPE_P(var) <= IS_NULL || (Z_TYPE_P(var) == IS_STRING && !Z_STRLEN_P(var)))
#define PHALCON_IS_NOT_EMPTY_STRING(var)	(Z_TYPE_P(var) == IS_STRING && Z_STRLEN_P(var))
#define PHALCON_IS_EMPTY_ARR(var)			(Z_TYPE_P(var) == IS_ARRAY && !zend_hash_num_elements(Z_ARRVAL_P(var)))
#define PHALCON_IS_NOT_EMPTY_ARR(var)		(Z_TYPE_P(var) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(var)))
#define PHALCON_IS_EMPTY(var)				(PHALCON_IS_EMPTY_STRING(var) || PHALCON_IS_EMPTY_ARR(var))
#define PHALCON_IS_NOT_EMPTY(var)			(!PHALCON_IS_EMPTY(var))

/** Is scalar */
#define PHALCON_IS_SCALAR(var)      (!(Z_TYPE_P(var) == IS_NULL || Z_TYPE_P(var) == IS_ARRAY || Z_TYPE_P(var) == IS_OBJECT || Z_TYPE_P(var) == IS_RESOURCE))
#define PHALCON_IS_NOT_SCALAR(var)  (Z_TYPE_P(var) == IS_NULL || Z_TYPE_P(var) == IS_ARRAY || Z_TYPE_P(var) == IS_OBJECT || Z_TYPE_P(var) == IS_RESOURCE)

/** Equals/Identical */
#define PHALCON_IS_EQUAL(op1, op2)      phalcon_is_equal(op1, op2)
#define PHALCON_IS_IDENTICAL(op1, op2)  phalcon_is_identical(op1, op2)

/** Greater/Smaller equals */
#define PHALCON_LE(op1, op2)       phalcon_less_equal(op1, op2)
#define PHALCON_LE_LONG(op1, op2)  phalcon_less_equal_long(op1, op2)
#define PHALCON_LE_DOUBLE(op1, op2)  ((Z_TYPE_P(op1) == IS_DOUBLE && Z_DVAL_P(op1) <= op2) || phalcon_less_equal_double(op1, op2))
#define PHALCON_GE(op1, op2)       phalcon_greater_equal(op1, op2)
#define PHALCON_GE_LONG(op1, op2)  phalcon_greater_equal_long(op1, op2)
#define PHALCON_LT(op1, op2)       phalcon_less(op1, op2)
#define PHALCON_LT_LONG(op1, op2)  phalcon_less_long(op1, op2)
#define PHALCON_LT_DOUBLE(op1, op2)  ((Z_TYPE_P(op1) == IS_DOUBLE && Z_DVAL_P(op1) < op2) || phalcon_less_double(op1, op2))
#define PHALCON_GT(op1, op2)       phalcon_greater(op1, op2)
#define PHALCON_GT_LONG(op1, op2)  phalcon_greater_long(op1, op2)
#define PHALCON_GT_DOUBLE(op1, op2)  ((Z_TYPE_P(op1) == IS_DOUBLE && Z_DVAL_P(op1) > op2) || phalcon_greater_double(op1, op2))

#define ZVAL_STRINGING_OFFSET(op1, index) ((index >= 0 && index < Z_STRLEN_P(op1)) ? Z_STRVAL_P(op1)[index] : '\0')
#define CHAR_STRINGING_OFFSET(op1, index) ((index >= 0 && index < strlen(op1)) ? op1[index] : '\0')

#define phalcon_increment(var) increment_function(var)
#define phalcon_decrement(var) decrement_function(var)

int phalcon_make_printable_zval(zval *expr, zval *expr_copy);

#define phalcon_sub_function(result, left, right) sub_function(result, left, right)
#define phalcon_add_function(result, left, right) add_function(result, left, right)
#define phalcon_div_function(result, left, right) div_function(result, left, right)

/** Operator functions */
int phalcon_and_function(zval *result, zval *left, zval *right);
void phalcon_negate(zval *z);

/** Bitwise functions */
int phalcon_bitwise_and_function(zval *result, zval *op1, zval *op2);
int phalcon_bitwise_or_function(zval *result, zval *op1, zval *op2);
int phalcon_bitwise_xor_function(zval *result, zval *op1, zval *op2);
int phalcon_shift_left_function(zval *result, zval *op1, zval *op2);
int phalcon_shift_right_function(zval *result, zval *op1, zval *op2);

/** Comparing */
int phalcon_compare(zval *op1, zval *op2);

/** Strict comparing */
int phalcon_compare_strict_string(zval *op1, const char *op2, int op2_length);
int phalcon_compare_strict_long(zval *op1, zend_long op2);
int phalcon_compare_strict_double(zval *op1, double op2);
int phalcon_compare_strict_bool(zval *op1, zend_bool op2);

void phalcon_cast(zval *result, zval *var, uint32_t type);
void phalcon_convert_to_object(zval *op);
zend_long phalcon_get_intval_ex(const zval *op);
double phalcon_get_doubleval_ex(const zval *op);
zend_bool phalcon_get_boolval_ex(const zval *op);

#define phalcon_get_numberval(z) (Z_TYPE_P(z) == IS_LONG ? Z_LVAL_P(z) : phalcon_get_doubleval(z))
#define phalcon_get_intval(z) (Z_TYPE_P(z) == IS_LONG ? Z_LVAL_P(z) : phalcon_get_intval_ex(z))
#define phalcon_get_doubleval(z) (Z_TYPE_P(z) == IS_DOUBLE ? Z_DVAL_P(z) : phalcon_get_doubleval_ex(z))
#define phalcon_get_boolval(z) (zend_is_true(z) ? 1 : 0);

int phalcon_is_numeric_ex(const zval *op);

#define phalcon_is_numeric(value) (Z_TYPE_P(value) == IS_LONG || Z_TYPE_P(value) == IS_DOUBLE || phalcon_is_numeric_ex(value))

int phalcon_is_long_ex(const zval *op);

#define phalcon_is_long(value) (Z_TYPE_P(value) == IS_LONG || phalcon_is_long_ex(value))

int phalcon_is_equal(zval *op1, zval *op2);
int phalcon_is_identical(zval *op1, zval *op2);
int phalcon_is_equal_long(zval *op1, zend_long op2);
int phalcon_is_equal_object(zval *obj1, zval *obj2);

int phalcon_less(zval *op1, zval *op2);
int phalcon_less_long(zval *op1, zend_long op2);

int phalcon_greater(zval *op1, zval *op2);
int phalcon_greater_long(zval *op1, zend_long op2);

int phalcon_less_equal(zval *op1, zval *op2);
int phalcon_less_equal_long(zval *op1, zend_long op2);

int phalcon_greater_equal(zval *op1, zval *op2);
int phalcon_greater_equal_long(zval *op1, zend_long op2);

zend_long phalcon_safe_mod_long_long(zend_long op1, zend_long op2);
zend_long phalcon_safe_mod_long_double(zend_long op1, double op2);
zend_long phalcon_safe_mod_double_long(double op1, zend_long op2);
zend_long phalcon_safe_mod_double_double(double op1, double op2);
zend_long phalcon_safe_mod_zval_long(zval *op1, zend_long op2);
zend_long phalcon_safe_mod_zval_double(zval *op1, double op2);
zend_long phalcon_safe_mod_long_zval(zend_long op1, zval *op2);
zend_long phalcon_safe_mod_double_zval(double op1, zval *op2);

int phalcon_is_scalar(zval *var);

/* Count */
zend_long phalcon_fast_count_int(zval *value);
void phalcon_fast_count(zval *result, zval *array);
int phalcon_fast_count_ev(zval *array);

#endif /* PHALCON_KERNEL_OPERATORS_H */
