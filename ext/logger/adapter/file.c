
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

#include "logger/adapter/file.h"
#include "logger/adapter.h"
#include "logger/adapterinterface.h"
#include "logger/exception.h"
#include "logger/formatter/line.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/object.h"

/**
 * Phalcon\Logger\Adapter\File
 *
 * Adapter to store logs in plain text files
 *
 *<code>
 *	$logger = new \Phalcon\Logger\Adapter\File("app/logs/test.log");
 *	$logger->log("This is a message");
 *	$logger->log("This is an error", \Phalcon\Logger::ERROR);
 *	$logger->error("This is another error");
 *	$logger->close();
 *</code>
 */
zend_class_entry *phalcon_logger_adapter_file_ce;

PHP_METHOD(Phalcon_Logger_Adapter_File, __construct);
PHP_METHOD(Phalcon_Logger_Adapter_File, getFormatter);
PHP_METHOD(Phalcon_Logger_Adapter_File, logInternal);
PHP_METHOD(Phalcon_Logger_Adapter_File, close);
PHP_METHOD(Phalcon_Logger_Adapter_File, getPath);
PHP_METHOD(Phalcon_Logger_Adapter_File, __wakeup);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_logger_adapter_file___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_logger_adapter_file_method_entry[] = {
	PHP_ME(Phalcon_Logger_Adapter_File, __construct, arginfo_phalcon_logger_adapter_file___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Logger_Adapter_File, getFormatter, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter_File, logInternal, arginfo_phalcon_logger_adapter_loginternal, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Logger_Adapter_File, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter_File, getPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter_File, __wakeup, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Logger\Adapter\File initializer
 */
PHALCON_INIT_CLASS(Phalcon_Logger_Adapter_File){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Logger\\Adapter, File, logger_adapter_file, phalcon_logger_adapter_ce, phalcon_logger_adapter_file_method_entry, 0);

	zend_declare_property_null(phalcon_logger_adapter_file_ce, SL("_fileHandler"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_logger_adapter_file_ce, SL("_path"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_logger_adapter_file_ce, SL("_options"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_logger_adapter_file_ce, 1, phalcon_logger_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Logger\Adapter\File constructor
 *
 * @param string $name
 * @param array $options
 */
PHP_METHOD(Phalcon_Logger_Adapter_File, __construct){

	zval *name, *options = NULL, mode, handler;

	phalcon_fetch_params(0, 1, 1, &name, &options);
	PHALCON_ENSURE_IS_STRING(name);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	if (phalcon_array_isset_fetch_str(&mode, options, SL("mode"))) {
		if (phalcon_memnstr_str(&mode, SL("r"))) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_logger_exception_ce, "Logger must be opened in append or write mode");
			return;
		}
	} else {
		ZVAL_STRING(&mode, "ab");
	}

	/** 
	 * We use 'fopen' to respect to open-basedir directive
	 */
	PHALCON_CALL_FUNCTIONW(&handler, "fopen", name, &mode);
	if (Z_TYPE(handler) != IS_RESOURCE) {
		zend_throw_exception_ex(phalcon_logger_exception_ce, 0, "Cannot open log file '%s'", Z_STRVAL_P(name));
	} else {
		phalcon_update_property_this(getThis(), SL("_path"), name);
		phalcon_update_property_this(getThis(), SL("_options"), options);
		phalcon_update_property_this(getThis(), SL("_fileHandler"), &handler);
	}
}

/**
 * Returns the internal formatter
 *
 * @return Phalcon\Logger\Formatter\Line
 */
PHP_METHOD(Phalcon_Logger_Adapter_File, getFormatter){

	zval formatter;

	phalcon_return_property(&formatter, getThis(), SL("_formatter"));
	if (Z_TYPE(formatter) != IS_OBJECT) {
		object_init_ex(&formatter, phalcon_logger_formatter_line_ce);
		PHALCON_CALL_METHODW(NULL, &formatter, "__construct");

		phalcon_update_property_this(getThis(), SL("_formatter"), &formatter);
	}

	RETURN_CTORW(&formatter);
}

/**
 * Writes the log to the file itself
 *
 * @param string $message
 * @param int $type
 * @param int $time
 * @param array $context
 */
PHP_METHOD(Phalcon_Logger_Adapter_File, logInternal){

	zval *message, *type, *time, *context, *file_handler, formatter, applied_format;

	phalcon_fetch_params(0, 4, 0, &message, &type, &time, &context);

	file_handler = phalcon_read_property(getThis(), SL("_fileHandler"), PH_NOISY);
	if (Z_TYPE_P(file_handler) != IS_RESOURCE) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_logger_exception_ce, "Cannot send message to the log because it is invalid");
		return;
	}

	PHALCON_CALL_METHODW(&formatter, getThis(), "getformatter");
	PHALCON_CALL_METHODW(&applied_format, &formatter, "format", message, type, time, context);
	PHALCON_CALL_FUNCTIONW(NULL, "fwrite", file_handler, &applied_format);
}

/**
 * Closes the logger
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Logger_Adapter_File, close){

	zval *file_handler;

	file_handler = phalcon_read_property(getThis(), SL("_fileHandler"), PH_NOISY);
	PHALCON_RETURN_CALL_FUNCTIONW("fclose", file_handler);
}

/**
 * Returns the file path
 *
 */
PHP_METHOD(Phalcon_Logger_Adapter_File, getPath) {

	RETURN_MEMBER(getThis(), "_path");
}

/**
 * Opens the internal file handler after unserialization
 *
 */
PHP_METHOD(Phalcon_Logger_Adapter_File, __wakeup){

	zval *path, *options, mode, file_handler;

	path = phalcon_read_property(getThis(), SL("_path"), PH_NOISY);
	if (Z_TYPE_P(path) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_logger_exception_ce, "Invalid data passed to Phalcon\\Logger\\Adapter\\File::__wakeup()");
		return;
	}

	options = phalcon_read_property(getThis(), SL("_options"), PH_NOISY);
	if (phalcon_array_isset_fetch_str(&mode, options, SL("mode"))) {
		if (Z_TYPE(mode) != IS_STRING) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_logger_exception_ce, "Invalid data passed to Phalcon\\Logger\\Adapter\\File::__wakeup()");
			return;
		}
	} else {
		ZVAL_STRING(&mode, "ab");
	}

	/**
	 * Re-open the file handler if the logger was serialized
	 */
	PHALCON_CALL_FUNCTIONW(&file_handler, "fopen", path, &mode);
	phalcon_update_property_this(getThis(), SL("_fileHandler"), &file_handler);
}
