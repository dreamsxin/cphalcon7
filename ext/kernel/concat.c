
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

#include "kernel/concat.h"

#include <ext/standard/php_string.h>

#include "kernel/main.h"
#include "kernel/operators.h"
#include "kernel/string.h"

void phalcon_concat_sv(zval *result, const char *op1, uint32_t op1_len, zval *op2, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_svs(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, int self_var){

	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_smart_str_appendl(&implstr, op3, op3_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_svsv(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_smart_str_appendl(&implstr, op3, op3_len);
	phalcon_append_printable_zval(&implstr, op4);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_svsvs(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, const char *op5, uint32_t op5_len, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_smart_str_appendl(&implstr, op3, op3_len);
	phalcon_append_printable_zval(&implstr, op4);
	phalcon_smart_str_appendl(&implstr, op5, op5_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_svsvsv(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, const char *op5, uint32_t op5_len, zval *op6, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_smart_str_appendl(&implstr, op3, op3_len);
	phalcon_append_printable_zval(&implstr, op4);
	phalcon_smart_str_appendl(&implstr, op5, op5_len);
	phalcon_append_printable_zval(&implstr, op6);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_svsvsvs(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, const char *op5, uint32_t op5_len, zval *op6, const char *op7, uint32_t op7_len, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_smart_str_appendl(&implstr, op3, op3_len);
	phalcon_append_printable_zval(&implstr, op4);
	phalcon_smart_str_appendl(&implstr, op5, op5_len);
	phalcon_append_printable_zval(&implstr, op6);
	phalcon_smart_str_appendl(&implstr, op7, op7_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_svsvsvsvs(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, const char *op5, uint32_t op5_len, zval *op6, const char *op7, uint32_t op7_len, zval *op8, const char *op9, uint32_t op9_len, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_smart_str_appendl(&implstr, op3, op3_len);
	phalcon_append_printable_zval(&implstr, op4);
	phalcon_smart_str_appendl(&implstr, op5, op5_len);
	phalcon_append_printable_zval(&implstr, op6);
	phalcon_smart_str_appendl(&implstr, op7, op7_len);
	phalcon_append_printable_zval(&implstr, op8);
	phalcon_smart_str_appendl(&implstr, op9, op9_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_svsvv(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, zval *op4, zval *op5, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_smart_str_appendl(&implstr, op3, op3_len);
	phalcon_append_printable_zval(&implstr, op4);
	phalcon_append_printable_zval(&implstr, op5);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_svv(zval *result, const char *op1, uint32_t op1_len, zval *op2, zval *op3, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_append_printable_zval(&implstr, op3);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_svvv(zval *result, const char *op1, uint32_t op1_len, zval *op2, zval *op3, zval *op4, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_append_printable_zval(&implstr, op4);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_svvs(zval *result, const char *op1, uint32_t op1_len, zval *op2, zval *op3, const char *op4, uint32_t op4_len, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_smart_str_appendl(&implstr, op4, op4_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_ss(zval *result, const char *op1, uint32_t op1_len, const char *op2, uint32_t op2_len, int self_var)
{
	smart_str implstr = {0};
	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_smart_str_appendl(&implstr, op2, op2_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_sss(zval *result, const char *op1, uint32_t op1_len, const char *op2, uint32_t op2_len, const char *op3, uint32_t op3_len, int self_var)
{
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_smart_str_appendl(&implstr, op1, op1_len);
	phalcon_smart_str_appendl(&implstr, op2, op2_len);
	phalcon_smart_str_appendl(&implstr, op3, op3_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vs(zval *result, zval *op1, const char *op2, uint32_t op2_len, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_smart_str_appendl(&implstr, op2, op2_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vsv(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_smart_str_appendl(&implstr, op2, op2_len);
	phalcon_append_printable_zval(&implstr, op3);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vsvs(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, const char *op4, uint32_t op4_len, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_smart_str_appendl(&implstr, op2, op2_len);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_smart_str_appendl(&implstr, op4, op4_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vsvsv(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, const char *op4, uint32_t op4_len, zval *op5, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_smart_str_appendl(&implstr, op2, op2_len);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_smart_str_appendl(&implstr, op4, op4_len);
	phalcon_append_printable_zval(&implstr, op5);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vsvsvs(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, const char *op4, uint32_t op4_len, zval *op5, const char *op6, uint32_t op6_len, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_smart_str_appendl(&implstr, op2, op2_len);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_smart_str_appendl(&implstr, op4, op4_len);
	phalcon_append_printable_zval(&implstr, op5);
	phalcon_smart_str_appendl(&implstr, op6, op6_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vsvsvsv(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, const char *op4, uint32_t op4_len, zval *op5, const char *op6, uint32_t op6_len, zval *op7, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_smart_str_appendl(&implstr, op2, op2_len);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_smart_str_appendl(&implstr, op4, op4_len);
	phalcon_append_printable_zval(&implstr, op5);
	phalcon_smart_str_appendl(&implstr, op6, op6_len);
	phalcon_append_printable_zval(&implstr, op7);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vsvv(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, zval *op4, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_smart_str_appendl(&implstr, op2, op2_len);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_append_printable_zval(&implstr, op4);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vsvvv(zval *result, zval *op1, const char *op2, uint32_t op2_len, zval *op3, zval *op4, zval *op5, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_smart_str_appendl(&implstr, op2, op2_len);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_append_printable_zval(&implstr, op4);
	phalcon_append_printable_zval(&implstr, op5);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vv(zval *result, zval *op1, zval *op2, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_append_printable_zval(&implstr, op2);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vvs(zval *result, zval *op1, zval *op2, const char *op3, uint32_t op3_len, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_smart_str_appendl(&implstr, op3, op3_len);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vvsv(zval *result, zval *op1, zval *op2, const char *op3, uint32_t op3_len, zval *op4, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_smart_str_appendl(&implstr, op3, op3_len);
	phalcon_append_printable_zval(&implstr, op4);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vvv(zval *result, zval *op1, zval *op2, zval *op3, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_append_printable_zval(&implstr, op3);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vvvsv(zval *result, zval *op1, zval *op2, zval *op3, const char *op4, uint32_t op4_len, zval *op5, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_smart_str_appendl(&implstr, op4, op4_len);
	phalcon_append_printable_zval(&implstr, op5);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vvvv(zval *result, zval *op1, zval *op2, zval *op3, zval *op4, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_append_printable_zval(&implstr, op4);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vvvvsvv(zval *result, zval *op1, zval *op2, zval *op3, zval *op4, const char *op5, uint32_t op5_len, zval *op6, zval *op7, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_append_printable_zval(&implstr, op4);
	phalcon_smart_str_appendl(&implstr, op5, op5_len);
	phalcon_append_printable_zval(&implstr, op6);
	phalcon_append_printable_zval(&implstr, op7);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

void phalcon_concat_vvvvv(zval *result, zval *op1, zval *op2, zval *op3, zval *op4, zval *op5, int self_var){
	smart_str implstr = {0};

	if (self_var) {
		phalcon_append_printable_zval(&implstr, result);
	}
	phalcon_append_printable_zval(&implstr, op1);
	phalcon_append_printable_zval(&implstr, op2);
	phalcon_append_printable_zval(&implstr, op3);
	phalcon_append_printable_zval(&implstr, op4);
	phalcon_append_printable_zval(&implstr, op5);
	smart_str_0(&implstr);
	if (implstr.s) {
		ZVAL_NEW_STR(result, implstr.s);
	} else {
		smart_str_free(&implstr);
		ZVAL_NULL(result);
	}
}

/**
 * Appends the content of the right operator to the left operator
 */
void phalcon_concat_self(zval *left, zval *right){
	zval tmp = {};
	concat_function(&tmp, left, right);
	ZVAL_COPY_VALUE(left, &tmp);
}

/**
 * Appends the content of the right operator to the left operator
 */
void phalcon_concat_self_str(zval *left, const char *right, int right_length){
	zval zright = {}, tmp = {};
	ZVAL_STRINGL(&zright, right, right_length);

	concat_function(&tmp, left, &zright);
	ZVAL_COPY_VALUE(left, &tmp);
	zval_ptr_dtor(&zright);
}

/**
* Appends the content of the right operator to the left operator
 */
void phalcon_concat_self_long(zval *left, const long right) {
	zval zright = {}, tmp = {};
	ZVAL_LONG(&zright, right);

	concat_function(&tmp, left, &zright);
	ZVAL_COPY_VALUE(left, &tmp);
}

/**
 * Appends the content of the right operator to the left operator
 */
void phalcon_concat_self_char(zval *left, unsigned char right) {
	zval zright = {}, tmp = {};
	char c[1];
	c[0] = right;
	ZVAL_STRINGL(&zright, c, 1);

	concat_function(&tmp, left, &zright);
	ZVAL_COPY_VALUE(left, &tmp);
	zval_ptr_dtor(&zright);
}
