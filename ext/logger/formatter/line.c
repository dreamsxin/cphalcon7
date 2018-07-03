
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

#include "logger/formatter/line.h"
#include "logger/formatter.h"
#include "logger/formatterinterface.h"

#include <ext/date/php_date.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/string.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"

/**
 * Phalcon\Logger\Formatter\Line
 *
 * Formats messages using an one-line string
 */
zend_class_entry *phalcon_logger_formatter_line_ce;

PHP_METHOD(Phalcon_Logger_Formatter_Line, __construct);
PHP_METHOD(Phalcon_Logger_Formatter_Line, setFormat);
PHP_METHOD(Phalcon_Logger_Formatter_Line, getFormat);
PHP_METHOD(Phalcon_Logger_Formatter_Line, setDateFormat);
PHP_METHOD(Phalcon_Logger_Formatter_Line, getDateFormat);
PHP_METHOD(Phalcon_Logger_Formatter_Line, format);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_logger_formatter_line___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, dateFormat)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_logger_formatter_line_setformat, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_logger_formatter_line_setdateformat, 0, 0, 1)
	ZEND_ARG_INFO(0, date)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_logger_formatter_line_method_entry[] = {
	PHP_ME(Phalcon_Logger_Formatter_Line, __construct, arginfo_phalcon_logger_formatter_line___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Logger_Formatter_Line, setFormat, arginfo_phalcon_logger_formatter_line_setformat, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter_Line, getFormat, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter_Line, setDateFormat, arginfo_phalcon_logger_formatter_line_setdateformat, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter_Line, getDateFormat, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter_Line, format, arginfo_phalcon_logger_formatterinterface_format, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Logger\Formatter\Line initializer
 */
PHALCON_INIT_CLASS(Phalcon_Logger_Formatter_Line){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Logger\\Formatter, Line, logger_formatter_line, phalcon_logger_formatter_ce, phalcon_logger_formatter_line_method_entry, 0);

	zend_declare_property_string(phalcon_logger_formatter_line_ce, SL("_dateFormat"), "D, d M y H:i:s O", ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_logger_formatter_line_ce, SL("_format"), "[%date%][%type%] %message%", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_logger_formatter_line_ce, 1, phalcon_logger_formatterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Logger\Formatter\Line construct
 *
 * @param string $format
 * @param string $dateFormat
 */
PHP_METHOD(Phalcon_Logger_Formatter_Line, __construct){

	zval *format = NULL, *date_format = NULL;

	phalcon_fetch_params(0, 0, 2, &format, &date_format);

	if (format && Z_TYPE_P(format) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_format"), format);
	}

	if (date_format && Z_TYPE_P(date_format) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_dateFormat"), date_format);
	}
}

/**
 * Set the log format
 *
 * @param string $format
 */
PHP_METHOD(Phalcon_Logger_Formatter_Line, setFormat){

	zval *format;

	phalcon_fetch_params(0, 1, 0, &format);

	phalcon_update_property(getThis(), SL("_format"), format);

}

/**
 * Returns the log format
 *
 * @return format
 */
PHP_METHOD(Phalcon_Logger_Formatter_Line, getFormat){


	RETURN_MEMBER(getThis(), "_format");
}

/**
 * Sets the internal date format
 *
 * @param string $date
 */
PHP_METHOD(Phalcon_Logger_Formatter_Line, setDateFormat){

	zval *date;

	phalcon_fetch_params(0, 1, 0, &date);

	phalcon_update_property(getThis(), SL("_dateFormat"), date);

}

/**
 * Returns the internal date format
 *
 * @return string
 */
PHP_METHOD(Phalcon_Logger_Formatter_Line, getDateFormat){


	RETURN_MEMBER(getThis(), "_dateFormat");
}

/**
 * Applies a format to a message before sent it to the internal log
 *
 * @param string $message
 * @param int $type
 * @param int $timestamp
 * @param array $context
 * @return string
 */
PHP_METHOD(Phalcon_Logger_Formatter_Line, format){

	zval *message, *type, *timestamp, *context, format = {}, date_format = {}, date = {}, date_wildcard = {}, new_format = {}, type_string = {}, type_wildcard = {}, message_wildcard = {};

	phalcon_fetch_params(0, 4, 0, &message, &type, &timestamp, &context);

	phalcon_read_property(&format, getThis(), SL("_format"), PH_COPY);

	/**
	 * Check if the format has the %date% placeholder
	 */
	if (phalcon_memnstr_str(&format, SL("%date%"))) {
		phalcon_read_property(&date_format, getThis(), SL("_dateFormat"), PH_READONLY);

		phalcon_date(&date, &date_format, timestamp);

		ZVAL_STRING(&date_wildcard, "%date%");

		PHALCON_STR_REPLACE(&new_format, &date_wildcard, &date, &format);

		zval_ptr_dtor(&date);
		zval_ptr_dtor(&date_wildcard);
	} else {
		ZVAL_COPY(&new_format, &format);
	}
	zval_ptr_dtor(&format);

	/**
	 * Check if the format has the %type% placeholder
	 */
	if (phalcon_memnstr_str(&new_format, SL("%type%"))) {
		PHALCON_CALL_METHOD(&type_string, getThis(), "gettypestring", type);

		ZVAL_STRING(&type_wildcard, "%type%");

		PHALCON_STR_REPLACE(&format, &type_wildcard, &type_string, &new_format);
		zval_ptr_dtor(&type_string);
		zval_ptr_dtor(&type_wildcard);
	} else {
		ZVAL_COPY(&format, &new_format);
	}
	zval_ptr_dtor(&new_format);

	ZVAL_STRING(&message_wildcard, "%message%");

	if (Z_TYPE_P(message) != IS_STRING) {
		zval tmp = {};
		PHALCON_CALL_FUNCTION(&tmp, "var_export", message, &PHALCON_GLOBAL(z_true));
		PHALCON_STR_REPLACE(&new_format, &message_wildcard, &tmp, &format);
		zval_ptr_dtor(&tmp);
	} else {
		PHALCON_STR_REPLACE(&new_format, &message_wildcard, message, &format);
	}
	zval_ptr_dtor(&format);
	zval_ptr_dtor(&message_wildcard);

	if (Z_TYPE_P(context) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(context)) > 0) {
		PHALCON_CALL_METHOD(&format, getThis(), "interpolate", &new_format, context);
	} else {
		ZVAL_COPY(&format, &new_format);
	}
	zval_ptr_dtor(&new_format);

	PHALCON_CONCAT_VS(return_value, &format, PHP_EOL);
	zval_ptr_dtor(&format);
}
