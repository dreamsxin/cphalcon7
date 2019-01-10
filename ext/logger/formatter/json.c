
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

#include "logger/formatter/json.h"
#include "logger/formatter.h"
#include "logger/formatterinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/string.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/object.h"

/**
 * Phalcon\Logger\Formatter\Json
 *
 * Formats messages using JSON encoding
 */
zend_class_entry *phalcon_logger_formatter_json_ce;

PHP_METHOD(Phalcon_Logger_Formatter_Json, __construct);
PHP_METHOD(Phalcon_Logger_Formatter_Json, format);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_logger_formatter_json___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_LONG, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_logger_formatter_json_method_entry[] = {
	PHP_ME(Phalcon_Logger_Formatter_Json, __construct, arginfo_phalcon_logger_formatter_json___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter_Json, format, arginfo_phalcon_logger_formatterinterface_format, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Logger\Formatter\Json initializer
 */
PHALCON_INIT_CLASS(Phalcon_Logger_Formatter_Json){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Logger\\Formatter, Json, logger_formatter_json, phalcon_logger_formatter_ce, phalcon_logger_formatter_json_method_entry, 0);

	zend_declare_property_long(phalcon_logger_formatter_json_ce, SL("_options"), 0, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_logger_formatter_json_ce, 1, phalcon_logger_formatterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Logger\Formatter\Json constructor
 *
 * @param int $options
 */
PHP_METHOD(Phalcon_Logger_Formatter_Json, __construct){

	zval *options = NULL;

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_LONG) {
		phalcon_update_property(getThis(), SL("_options"), options);
	}

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
PHP_METHOD(Phalcon_Logger_Formatter_Json, format){

	zval *message, *type, *timestamp, *context, interpolated = {}, type_str = {}, log = {}, options = {}, json = {};

	phalcon_fetch_params(1, 4, 0, &message, &type, &timestamp, &context);

	if (Z_TYPE_P(context) == IS_ARRAY) {
		PHALCON_MM_CALL_METHOD(&interpolated, getThis(), "interpolate", message, context);
		PHALCON_MM_ADD_ENTRY(&interpolated);
	} else {
		ZVAL_COPY_VALUE(&interpolated, message);
	}

	PHALCON_MM_CALL_METHOD(&type_str, getThis(), "gettypestring", type);
	PHALCON_MM_ADD_ENTRY(&type_str);

	array_init_size(&log, 3);
	PHALCON_MM_ADD_ENTRY(&log);
	phalcon_array_update_str(&log, SL("type"), &type_str, PH_COPY);
	phalcon_array_update_str(&log, SL("message"), &interpolated, PH_COPY);
	phalcon_array_update_str(&log, SL("timestamp"), timestamp, PH_COPY);

	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);

	RETURN_MM_ON_FAILURE(phalcon_json_encode(&json, &log, Z_LVAL(options)));
	PHALCON_MM_ADD_ENTRY(&json);

	PHALCON_CONCAT_VS(return_value, &json, PHP_EOL);
	RETURN_MM();
}
