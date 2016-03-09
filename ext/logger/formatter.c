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

#include "logger/formatter.h"
#include "logger/formatterinterface.h"

#include <main/spprintf.h>

#include "kernel/main.h"
#include "kernel/fcall.h"

/**
 * Phalcon\Logger\Formatter
 *
 * This is a base class for logger formatters
 */
zend_class_entry *phalcon_logger_formatter_ce;

PHP_METHOD(Phalcon_Logger_Formatter, getTypeString);
PHP_METHOD(Phalcon_Logger_Formatter, interpolate);

static const zend_function_entry phalcon_logger_formatter_method_entry[] = {
	PHP_ME(Phalcon_Logger_Formatter, getTypeString, arginfo_phalcon_logger_formatter_gettypestring, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter, interpolate, arginfo_phalcon_logger_formatter_interpolate, ZEND_ACC_PROTECTED)
	PHP_FE_END
};

/**
 * Phalcon\Logger\Formatter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Logger_Formatter){

	PHALCON_REGISTER_CLASS(Phalcon\\Logger, Formatter, logger_formatter, phalcon_logger_formatter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_class_implements(phalcon_logger_formatter_ce, 1, phalcon_logger_formatterinterface_ce);

	return SUCCESS;
}

/**
 * Returns the string meaning of a logger constant
 *
 * @param  integer $type
 * @return string
 */
PHP_METHOD(Phalcon_Logger_Formatter, getTypeString){

	static const char *lut[10] = {
		"EMERGENCY", "CRITICAL", "ALERT", "ERROR",  "WARNING",
		"NOTICE",    "INFO",     "DEBUG", "CUSTOM", "SPECIAL"
	};

	zval *type;
	int itype;

	phalcon_fetch_params(0, 1, 0, &type);
	PHALCON_ENSURE_IS_LONG(type);
	
	itype = Z_LVAL_P(type);
	if (itype >= 0 && itype < 10) {
		RETURN_STRING(lut[itype]);
	}
	
	RETURN_STRING("CUSTOM");
}

/**
 * Interpolates context values into the message placeholders
 *
 * @see http://www.php-fig.org/psr/psr-3/ Section 1.2 Message
 * @param string $message
 * @param array $context
 */
PHP_METHOD(Phalcon_Logger_Formatter, interpolate)
{
	zval *message, *context;
	zval replace, *val;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 0, &message, &context);

	if (Z_TYPE_P(context) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(context)) > 0) {
		array_init(&replace);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(context), idx, str_key, val) {
			zval index;
			char *tmp;
			uint str_length;

			if (str_key) {;
				str_length = spprintf(&tmp, 0, "{%s}", str_key->val);
				PHALCON_STRL(&index, tmp, str_length);
			} else {
				str_length = spprintf(&tmp, 0, "{%ld}", idx);
				PHALCON_STRL(&index, tmp, str_length);
			}

			Z_TRY_ADDREF_P(val);
			zend_hash_add(Z_ARRVAL(replace), Z_STR(index), val);
			efree(tmp);
		} ZEND_HASH_FOREACH_END();

		PHALCON_RETURN_CALL_FUNCTIONW("strtr", message, &replace);
		return;
	}

	RETURN_ZVAL(message, 1, 0);
}
