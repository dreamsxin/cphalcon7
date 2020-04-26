
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

#include "logger.h"
#include "logger/exception.h"
#include "logger/../exception.h"

#include "di.h"
#include "diinterface.h"

#include "kernel/main.h"
#include "kernel/fcall.h"

#include "interned-strings.h"

/**
 * Phalcon\Logger
 *
 * Phalcon\Logger is a component whose purpose is create logs using
 * different backends via adapters, generating options, formats and filters
 * also implementing transactions.
 *
 *<code>
 *	$logger = new Phalcon\Logger\Adapter\File("app/logs/test.log");
 *	$logger->log(Phalcon\Logger::INFO, "This is a message");
 *	$logger->log(Phalcon\Logger::ERROR, "This is an error");
 *	$logger->error("This is another error");
 *</code>
 */
zend_class_entry *phalcon_logger_ce;

PHP_METHOD(Phalcon_Logger, getTypeString);
PHP_METHOD(Phalcon_Logger, __callStatic);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_logger_gettypestring, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_logger___callstatic, 0, 0, 1)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_logger_method_entry[] = {
	PHP_ME(Phalcon_Logger, getTypeString, arginfo_phalcon_logger_gettypestring, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Logger, __callStatic, arginfo_phalcon_logger___callstatic, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Logger initializer
 */
PHALCON_INIT_CLASS(Phalcon_Logger){

	PHALCON_REGISTER_CLASS(Phalcon, Logger, logger, phalcon_logger_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_class_constant_long(phalcon_logger_ce, SL("SPECIAL"),   PHALCON_LOGGER_SPECIAL  );
	zend_declare_class_constant_long(phalcon_logger_ce, SL("CUSTOM"),    PHALCON_LOGGER_CUSTOM   );
	zend_declare_class_constant_long(phalcon_logger_ce, SL("DEBUG"),     PHALCON_LOGGER_DEBUG    );
	zend_declare_class_constant_long(phalcon_logger_ce, SL("INFO"),      PHALCON_LOGGER_INFO     );
	zend_declare_class_constant_long(phalcon_logger_ce, SL("NOTICE"),    PHALCON_LOGGER_NOTICE   );
	zend_declare_class_constant_long(phalcon_logger_ce, SL("WARNING"),   PHALCON_LOGGER_WARNING  );
	zend_declare_class_constant_long(phalcon_logger_ce, SL("ERROR"),     PHALCON_LOGGER_ERROR    );
	zend_declare_class_constant_long(phalcon_logger_ce, SL("ALERT"),     PHALCON_LOGGER_ALERT    );
	zend_declare_class_constant_long(phalcon_logger_ce, SL("CRITICAL"),  PHALCON_LOGGER_CRITICAL );
	zend_declare_class_constant_long(phalcon_logger_ce, SL("EMERGENCE"), PHALCON_LOGGER_EMERGENCY);
	zend_declare_class_constant_long(phalcon_logger_ce, SL("EMERGENCY"), PHALCON_LOGGER_EMERGENCY);

	return SUCCESS;
}

/**
 * Returns the string meaning of a logger constant
 *
 * @param  integer $type
 * @return string
 */
PHP_METHOD(Phalcon_Logger, getTypeString){

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
 * Handles method calls when a static method is not implemented
 *
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_Logger, __callStatic){

	zval *method, *arguments = NULL, dependency_injector = {}, service_name = {}, logger = {}, message = {}, context = {};

	phalcon_fetch_params(1, 1, 1, &method, &arguments);
	
	PHALCON_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);

	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_logger_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	ZVAL_STR(&service_name, IS(logger));

	PHALCON_MM_CALL_METHOD(&logger, &dependency_injector, "getshared", &service_name);
	PHALCON_MM_ADD_ENTRY(&logger);

	if (!phalcon_array_isset_fetch_long(&message, arguments, 0, PH_READONLY)) {
		RETURN_MM_FALSE;
	}

	if (!phalcon_array_isset_fetch_long(&context, arguments, 1, PH_READONLY)) {
		PHALCON_CALL_METHOD(NULL, &logger, "log", method, &message, &context);
	} else {
		PHALCON_CALL_METHOD(NULL, &logger, "log", method, &message);
	}
	RETURN_MM_TRUE;
}
