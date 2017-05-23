
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

#include "kernel/concat.h"

#include <ext/standard/php_string.h>

#include "kernel/main.h"
#include "kernel/operators.h"
#include "kernel/string.h"

void phalcon_concat_sv(zval *result, const char *op1, uint32_t op1_len, zval *op2, int self_var){
	zval zop1 = {};
	ZVAL_STRINGL(&zop1, op1, op1_len);
	if (self_var) {
		concat_function(result, result, &zop1);
		concat_function(result, result, op2);
	} else {
		concat_function(result, &zop1, op2);
	}
	zval_ptr_dtor(&zop1);
}

void phalcon_concat_svs(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, int self_var){
	zval zop1 = {}, zop3 = {};
	ZVAL_STRINGL(&zop1, op1, op1_len);
	ZVAL_STRINGL(&zop3, op3, op3_len);

	if (self_var) {
		concat_function(result, result, &zop1);
		concat_function(result, result, op2);
	} else {
		concat_function(result, &zop1, op2);
	}
	concat_function(result, result, &zop3);
	zval_ptr_dtor(&zop1);
	zval_ptr_dtor(&zop3);
}

void phalcon_concat_svsv(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, int self_var){
	phalcon_concat_sv(result, op1, op1_len, op2, self_var);
	phalcon_concat_sv(result, op3, op3_len, op4, 1);
}

void phalcon_concat_svsvs(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, const char *op5, uint32_t op5_len, int self_var){
	phalcon_concat_svs(result, op1, op1_len, op2, op3, op3_len, self_var);
	phalcon_concat_vs(result, op4, op5, op5_len, 1);
}

void phalcon_concat_svsvsv(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, const char *op5, uint32_t op5_len, zval *op6, int self_var){
	phalcon_concat_svs(result, op1, op1_len, op2, op3, op3_len, self_var);
	phalcon_concat_vsv(result, op4, op5, op5_len, op6, 1);
}

void phalcon_concat_svsvsvs(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, const char *op5, uint32_t op5_len, zval *op6, const char *op7, uint32_t op7_len, int self_var){
	phalcon_concat_sv(result, op1, op1_len, op2, self_var);
	phalcon_concat_sv(result, op3, op3_len, op4, 1);
	phalcon_concat_svs(result, op5, op5_len, op6, op7, op7_len, 1);
}

void phalcon_concat_svsvsvsvs(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, const char *op5, uint32_t op5_len, zval *op6, const char *op7, uint32_t op7_len, zval *op8, const char *op9, uint32_t op9_len, int self_var){
	phalcon_concat_svs(result, op1, op1_len, op2, op3, op3_len, self_var);
	phalcon_concat_vsv(result, op4, op5, op5_len, op6, 1);
	phalcon_concat_svs(result, op7, op7_len, op8, op9, op9_len, 1);
}

void phalcon_concat_svsvv(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, zval *op5, int self_var){
	phalcon_concat_svs(result, op1, op1_len, op2, op3, op3_len, self_var);
	phalcon_concat_vv(result, op4, op5, 1);
}

void phalcon_concat_svv(zval *result, const char *op1, uint32_t op1_len, zval *op2, zval *op3, int self_var){
	zval zop1 = {};
	ZVAL_STRINGL(&zop1, op1, op1_len);
	if (self_var) {
		concat_function(result, result, &zop1);
		concat_function(result, result, op2);
	} else {
		concat_function(result, &zop1, op2);
	}
	concat_function(result, result, op3);
	zval_ptr_dtor(&zop1);
}

void phalcon_concat_svvv(zval *result, const char *op1, uint32_t op1_len, zval *op2, zval *op3, zval *op4, int self_var){
	phalcon_concat_sv(result, op1, op1_len, op2, self_var);
	phalcon_concat_vv(result, op3, op4, 1);
}

void phalcon_concat_svvs(zval *result, const char *op1, uint32_t op1_len, zval *op2, zval *op3, const char *op4, uint32_t op4_len, int self_var){
	phalcon_concat_sv(result, op1, op1_len, op2, self_var);
	phalcon_concat_vs(result, op3, op4, op4_len, 1);
}

void phalcon_concat_ss(zval *result, const char *op1, uint32_t op1_len, const char *op2, uint32_t op2_len, int self_var)
{
	zval zop1 = {}, zop2 = {};
	ZVAL_STRINGL(&zop1, op1, op1_len);
	ZVAL_STRINGL(&zop2, op2, op2_len);
	if (self_var) {
		concat_function(result, result, &zop1);
		concat_function(result, result, &zop2);
	} else {
		concat_function(result, &zop1, &zop2);
	}
	zval_ptr_dtor(&zop1);
	zval_ptr_dtor(&zop2);
}

void phalcon_concat_sss(zval *result, const char *op1, uint32_t op1_len, const char *op2, uint32_t op2_len, const char *op3, uint32_t op3_len, int self_var)
{
	zval zop1 = {}, zop2 = {}, zop3 = {};
	ZVAL_STRINGL(&zop1, op1, op1_len);
	ZVAL_STRINGL(&zop2, op2, op2_len);
	ZVAL_STRINGL(&zop3, op3, op3_len);
	if (self_var) {
		concat_function(result, result, &zop1);
		concat_function(result, result, &zop2);
		concat_function(result, result, &zop3);
	} else {
		concat_function(result, &zop1, &zop2);
		concat_function(result, result, &zop3);
	}
	zval_ptr_dtor(&zop1);
	zval_ptr_dtor(&zop2);
	zval_ptr_dtor(&zop3);
}

void phalcon_concat_vs(zval *result, zval *op1, const char *op2, uint32_t op2_len, int self_var){
	zval zop2 = {};
	ZVAL_STRINGL(&zop2, op2, op2_len);
	if (self_var) {
		concat_function(result, result, op1);
		concat_function(result, result, &zop2);
	} else {
		concat_function(result, op1, &zop2);
	}
	zval_ptr_dtor(&zop2);
}

void phalcon_concat_vsv(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, int self_var){
	zval zop2 = {};
	ZVAL_STRINGL(&zop2, op2, op2_len);
	if (self_var) {
		concat_function(result, result, op1);
		concat_function(result, result, &zop2);
	} else {
		concat_function(result, op1, &zop2);
	}
	concat_function(result, result, op3);
	zval_ptr_dtor(&zop2);
}

void phalcon_concat_vsvs(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, const char *op4, uint32_t op4_len, int self_var){
	phalcon_concat_vs(result, op1, op2, op2_len, self_var);
	phalcon_concat_vs(result, op3, op4, op4_len, 1);
}

void phalcon_concat_vsvsv(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, const char *op4, uint32_t op4_len, zval *op5, int self_var){
	phalcon_concat_vsv(result, op1, op2, op2_len, op3, self_var);
	phalcon_concat_sv(result, op4, op4_len, op5, 1);
}

void phalcon_concat_vsvsvs(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, const char *op4, uint32_t op4_len, zval *op5, const char *op6, uint32_t op6_len, int self_var){
	phalcon_concat_vsv(result, op1, op2, op2_len, op3, self_var);
	phalcon_concat_svs(result, op4, op4_len, op5, op6, op6_len, 1);
}

void phalcon_concat_vsvsvsv(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, const char *op4, uint32_t op4_len, zval *op5, const char *op6, uint32_t op6_len, zval *op7, int self_var){
	phalcon_concat_vs(result, op1, op2, op2_len, self_var);
	phalcon_concat_vs(result, op3, op4, op4_len, 1);
	phalcon_concat_vsv(result, op5, op6, op6_len, op7, 1);
}

void phalcon_concat_vsvv(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, zval *op4, int self_var){
	phalcon_concat_vs(result, op1, op2, op2_len, self_var);
	phalcon_concat_vv(result, op3, op4, 1);
}

void phalcon_concat_vsvvv(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, zval *op4, zval *op5, int self_var){
	phalcon_concat_vs(result, op1, op2, op2_len, self_var);
	phalcon_concat_vvv(result, op3, op4, op5, 1);
}

void phalcon_concat_vv(zval *result, zval *op1, zval *op2, int self_var){
	if (self_var) {
		concat_function(result, result, op1);
		concat_function(result, result, op2);
	} else {
		concat_function(result, op1, op2);
	}
}

void phalcon_concat_vvs(zval *result, zval *op1, zval *op2, const char *op3, uint32_t op3_len, int self_var){
	zval zop3 = {};
	ZVAL_STRINGL(&zop3, op3, op3_len);

	if (self_var) {
		concat_function(result, result, op1);
		concat_function(result, result, op2);
	} else {
		concat_function(result, op1, op2);
	}
	concat_function(result, result, &zop3);
	zval_ptr_dtor(&zop3);
}

void phalcon_concat_vvsv(zval *result, zval *op1, zval *op2, const char *op3, uint32_t op3_len, zval *op4, int self_var){
	phalcon_concat_vv(result, op1, op2, self_var);
	phalcon_concat_sv(result, op3, op3_len, op4, 1);
}

void phalcon_concat_vvv(zval *result, zval *op1, zval *op2, zval *op3, int self_var){
	if (self_var) {
		concat_function(result, result, op1);
		concat_function(result, result, op2);
	} else {
		concat_function(result, op1, op2);
	}
	concat_function(result, result, op3);
}

void phalcon_concat_vvvsv(zval *result, zval *op1, zval *op2, zval *op3, const char *op4, uint32_t op4_len, zval *op5, int self_var){
	phalcon_concat_vvv(result, op1, op2, op3, self_var);
	phalcon_concat_sv(result, op4, op4_len, op5, 1);
}

void phalcon_concat_vvvv(zval *result, zval *op1, zval *op2, zval *op3, zval *op4, int self_var){
	phalcon_concat_vv(result, op1, op2, self_var);
	phalcon_concat_vv(result, op3, op4, 1);
}

void phalcon_concat_vvvvsvv(zval *result, zval *op1, zval *op2, zval *op3, zval *op4, const char *op5, uint32_t op5_len, zval *op6, zval *op7, int self_var){
	phalcon_concat_vvv(result, op1, op2, op3, self_var);
	phalcon_concat_vs(result, op4, op5, op5_len, 1);
	phalcon_concat_vv(result, op6, op7, 1);
}

void phalcon_concat_vvvvv(zval *result, zval *op1, zval *op2, zval *op3, zval *op4, zval *op5, int self_var){
	phalcon_concat_vvvv(result, op1, op2, op3, op4, self_var);
	concat_function(result, result, op5);
}

/**
 * Appends the content of the right operator to the left operator
 */
void phalcon_concat_self(zval *left, zval *right){
	concat_function(left, left, right);
}

/**
 * Appends the content of the right operator to the left operator
 */
void phalcon_concat_self_str(zval *left, const char *right, int right_length){
	zval zright = {};
	ZVAL_STRINGL(&zright, right, right_length);

	concat_function(left, left, &zright);
	zval_ptr_dtor(&zright);
}

/**
* Appends the content of the right operator to the left operator
 */
void phalcon_concat_self_long(zval *left, const long right) {
	zval zright = {};
	ZVAL_LONG(&zright, right);

	concat_function(left, left, &zright);
}

/**
 * Appends the content of the right operator to the left operator
 */
void phalcon_concat_self_char(zval *left, unsigned char right) {
	zval zright = {};
	char c[1];
	c[0] = right;
	ZVAL_STRINGL(&zright, c, 1);

	concat_function(left, left, &zright);
}
