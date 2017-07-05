
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

#include "random.h"
#include "exception.h"

#include <ext/standard/php_rand.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/array.h"

/**
 * Phalcon\Random
 *
 *<code>
 *  $random = new \Phalcon\Random();
 *
 *  // Random alnum string
 *  $alnum = $random->alnum();
 */
zend_class_entry *phalcon_random_ce;

PHP_METHOD(Phalcon_Random, alnum);
PHP_METHOD(Phalcon_Random, alpha);
PHP_METHOD(Phalcon_Random, hexdec);
PHP_METHOD(Phalcon_Random, numeric);
PHP_METHOD(Phalcon_Random, nozero);
PHP_METHOD(Phalcon_Random, color);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_random_alnum, 0, 0, 0)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_random_alpha, 0, 0, 0)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_random_hexdec, 0, 0, 0)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_random_numeric, 0, 0, 0)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_random_nozero, 0, 0, 0)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_random_color, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()


static const zend_function_entry phalcon_random_method_entry[] = {
	PHP_ME(Phalcon_Random, alnum, arginfo_phalcon_random_alnum, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Random, alpha, arginfo_phalcon_random_alpha, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Random, hexdec, arginfo_phalcon_random_hexdec, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Random, numeric, arginfo_phalcon_random_numeric, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Random, nozero, arginfo_phalcon_random_nozero, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Random, color, arginfo_phalcon_random_color, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Random initializer
 */
PHALCON_INIT_CLASS(Phalcon_Random){

	PHALCON_REGISTER_CLASS(Phalcon, Random, random, phalcon_random_method_entry, 0);

	zend_declare_class_constant_long(phalcon_random_ce, SL("COLOR_RGB"), PHALCON_RANDOM_COLOR_RGB);
	zend_declare_class_constant_long(phalcon_random_ce, SL("COLOR_RGBA"), PHALCON_RANDOM_COLOR_RGBA);
	return SUCCESS;
}

/**
 * Generates a random alnum string
 *
 *<code>
 *  $random = new \Phalcon\Random();
 *  $alnum = $random->alnum();
 *</code>
 */
PHP_METHOD(Phalcon_Random, alnum){

	zval *len_param = NULL, len = {}, type = {};
	int l;

	phalcon_fetch_params(0, 0, 1, &len_param);

	if (!len_param) {
		l = 8;
	} else {
		l = phalcon_get_intval(len_param);
		if (l <= 0) {
			l = 8;
		}
	}

	ZVAL_LONG(&len, l);
	ZVAL_LONG(&type, PHALCON_RANDOM_ALNUM);

	phalcon_random_string(return_value, &type, &len);
}

/**
 * Generates a random alpha string
 *
 *<code>
 *  $random = new \Phalcon\Random();
 *  $alpha = $random->alpha();
 *</code>
 */
PHP_METHOD(Phalcon_Random, alpha){

	zval *len_param = NULL, len = {}, type = {};
	int l;

	phalcon_fetch_params(0, 0, 1, &len_param);

	if (!len_param) {
		l = 8;
	} else {
		l = phalcon_get_intval(len_param);
		if (l <= 0) {
			l = 8;
		}
	}

	ZVAL_LONG(&len, l);
	ZVAL_LONG(&type, PHALCON_RANDOM_ALPHA);

	phalcon_random_string(return_value, &type, &len);
}

/**
 * Generates a random hexdec string
 *
 *<code>
 *  $random = new \Phalcon\Random();
 *  $hexdec = $random->hexdec();
 *</code>
 */
PHP_METHOD(Phalcon_Random, hexdec){

	zval *len_param = NULL, len = {}, type = {};
	int l;

	phalcon_fetch_params(0, 0, 1, &len_param);

	if (!len_param) {
		l = 8;
	} else {
		l = phalcon_get_intval(len_param);
		if (l <= 0) {
			l = 8;
		}
	}

	ZVAL_LONG(&len, l);
	ZVAL_LONG(&type, PHALCON_RANDOM_HEXDEC);

	phalcon_random_string(return_value, &type, &len);
}

/**
 * Generates a random numeric string
 *
 *<code>
 *  $random = new \Phalcon\Random();
 *  $numeric = $random->numeric();
 *</code>
 */
PHP_METHOD(Phalcon_Random, numeric){

	zval *len_param = NULL, len = {}, type = {};
	int l;

	phalcon_fetch_params(0, 0, 1, &len_param);

	if (!len_param) {
		l = 8;
	} else {
		l = phalcon_get_intval(len_param);
		if (l <= 0) {
			l = 8;
		}
	}

	ZVAL_LONG(&len, l);
	ZVAL_LONG(&type, PHALCON_RANDOM_NUMERIC);

	phalcon_random_string(return_value, &type, &len);
}

/**
 * Generates a random nozero numeric string
 *
 *<code>
 *  $random = new \Phalcon\Random();
 *  $bytes = $random->nozero();
 *</code>
 */
PHP_METHOD(Phalcon_Random, nozero){

	zval *len_param = NULL, len = {}, type = {};
	int l;

	phalcon_fetch_params(0, 0, 1, &len_param);

	if (!len_param) {
		l = 8;
	} else {
		l = phalcon_get_intval(len_param);
		if (l <= 0) {
			l = 8;
		}
	}

	ZVAL_LONG(&len, l);
	ZVAL_LONG(&type, PHALCON_RANDOM_NOZERO);

	phalcon_random_string(return_value, &type, &len);
}

/**
 * Generates a random color
 */
PHP_METHOD(Phalcon_Random, color) {

	zval *type = NULL, r = {}, g = {}, b = {}, a = {}, r_hex = {}, g_hex = {}, b_hex = {}, pad_type = {}, r_str = {}, g_str = {}, b_str = {};
	zend_long color_type = PHALCON_RANDOM_COLOR_RGBA, rnd_idx = 0;

	phalcon_fetch_params(0, 0, 1, &type);

	if (type && Z_TYPE_P(type) == IS_LONG) {
		color_type = Z_LVAL_P(type);
	}

#if PHP_VERSION_ID >= 70100
		rnd_idx = php_mt_rand_common(0, 255);
#else
		rnd_idx = php_rand();
		RAND_RANGE(rnd_idx, 0, 255, PHP_RAND_MAX);
#endif
	ZVAL_LONG(&r, rnd_idx);

#if PHP_VERSION_ID >= 70100
		rnd_idx = php_mt_rand_common(0, 255);
#else
		rnd_idx = php_rand();
		RAND_RANGE(rnd_idx, 0, 255, PHP_RAND_MAX);
#endif
	ZVAL_LONG(&g, rnd_idx);

#if PHP_VERSION_ID >= 70100
		rnd_idx = php_mt_rand_common(0, 255);
#else
		rnd_idx = php_rand();
		RAND_RANGE(rnd_idx, 0, 255, PHP_RAND_MAX);
#endif
	ZVAL_LONG(&b, rnd_idx);

	if (color_type == PHALCON_RANDOM_COLOR_RGBA) {
#if PHP_VERSION_ID >= 70100
		rnd_idx = php_mt_rand_common(0, 127);
#else
		rnd_idx = php_rand();
		RAND_RANGE(rnd_idx, 0, 127, PHP_RAND_MAX);
#endif
		ZVAL_LONG(&a, rnd_idx);
		PHALCON_CONCAT_SVSVSVSVS(return_value, "rgba(", &r, ",", &g, ",", &b, ",", &a, ")");
	} else {
		PHALCON_CALL_FUNCTION(&r_hex, "dechex", &r);
		PHALCON_CALL_FUNCTION(&g_hex, "dechex", &g);
		PHALCON_CALL_FUNCTION(&b_hex, "dechex", &b);

		if (!phalcon_get_constant(&pad_type, SL("STR_PAD_LEFT"))) {
			ZVAL_LONG(&pad_type, 0);
		}
		PHALCON_CALL_FUNCTION(&r_str, "str_pad", &r_hex, &PHALCON_GLOBAL(z_two), &pad_type);
		PHALCON_CALL_FUNCTION(&g_str, "str_pad", &g_hex, &PHALCON_GLOBAL(z_two), &pad_type);
		PHALCON_CALL_FUNCTION(&b_str, "str_pad", &b_hex, &PHALCON_GLOBAL(z_two), &pad_type);

		PHALCON_CONCAT_SVVV(return_value, "#", &r_str, &g_str, &b_str);
	}
}
