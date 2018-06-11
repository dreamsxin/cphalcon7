
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
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


#include "php_phalcon.h"

#include <ext/standard/php_math.h>
#include <ext/standard/php_lcg.h>
#include <ext/standard/php_rand.h>
#include <Zend/zend_operators.h>

#include "kernel/math.h"
#include "kernel/memory.h"
#include "kernel/string.h"
#include "kernel/operators.h"

double phalcon_floor(zval *op1)
{
	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return (double) Z_LVAL_P(op1);
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return floor(phalcon_get_numberval(op1));
}


double phalcon_ceil(zval *op1)
{
	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return (double) Z_LVAL_P(op1);
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ceil(phalcon_get_numberval(op1));
}

extern double _php_math_round(double value, int places, int mode);

void phalcon_round(zval *return_value, zval *op1, zval *op2, zval *op3)
{
	int places = 0;
	long mode = PHP_ROUND_HALF_UP;
	double return_val;

	convert_scalar_to_number_ex(op1);

	if (op2) {
		places = phalcon_get_intval_ex(op2);
	}
	if (op3) {
		mode = phalcon_get_intval_ex(op3);
	}

	switch (Z_TYPE_P(*&op1)) {
		case IS_LONG:
			/* Simple case - long that doesn't need to be rounded. */
			if (places >= 0) {
				RETURN_DOUBLE((double) Z_LVAL_P(*&op1));
			}
			/* break omitted intentionally */

		case IS_DOUBLE:
			return_val = (Z_TYPE_P(*&op1) == IS_LONG) ? (double)Z_LVAL_P(*&op1) : Z_DVAL_P(*&op1);
			return_val = _php_math_round(return_val, places, mode);
			RETURN_DOUBLE(return_val);
			break;

		default:
			RETURN_FALSE;
			break;
	}
}

long phalcon_mt_rand(long min, long max) {

	long number;

	if (max < min) {
		php_error_docref(NULL, E_WARNING, "max(%ld) is smaller than min(%ld)", max, min);
		return 0;
	}

	if (!BG(mt_rand_is_seeded)) {
		php_mt_srand(GENERATE_SEED());
	}

#if PHP_VERSION_ID >= 70200
	number = php_mt_rand_range(min, max);
#else
	number = (long) (php_mt_rand() >> 1);
	RAND_RANGE(number, min, max, PHP_MT_RAND_MAX);
#endif
	return number;
}
