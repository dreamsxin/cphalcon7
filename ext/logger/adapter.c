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

#include "logger/adapter.h"
#include "logger/adapterinterface.h"
#include "logger/exception.h"
#include "logger/formatterinterface.h"
#include "logger/item.h"
#include "logger.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"

/**
 * Phalcon\Logger\Adapter
 *
 * Base class for Phalcon\Logger adapters
 */
zend_class_entry *phalcon_logger_adapter_ce;

PHP_METHOD(Phalcon_Logger_Adapter, setLogLevel);
PHP_METHOD(Phalcon_Logger_Adapter, getLogLevel);
PHP_METHOD(Phalcon_Logger_Adapter, setFormatter);
PHP_METHOD(Phalcon_Logger_Adapter, isTransaction);
PHP_METHOD(Phalcon_Logger_Adapter, begin);
PHP_METHOD(Phalcon_Logger_Adapter, commit);
PHP_METHOD(Phalcon_Logger_Adapter, rollback);
PHP_METHOD(Phalcon_Logger_Adapter, emergency);
PHP_METHOD(Phalcon_Logger_Adapter, alert);
PHP_METHOD(Phalcon_Logger_Adapter, critical);
PHP_METHOD(Phalcon_Logger_Adapter, error);
PHP_METHOD(Phalcon_Logger_Adapter, warning);
PHP_METHOD(Phalcon_Logger_Adapter, notice);
PHP_METHOD(Phalcon_Logger_Adapter, info);
PHP_METHOD(Phalcon_Logger_Adapter, debug);
PHP_METHOD(Phalcon_Logger_Adapter, log);

static const zend_function_entry phalcon_logger_adapter_method_entry[] = {
	PHP_ME(Phalcon_Logger_Adapter, setLogLevel, arginfo_phalcon_logger_adapterinterface_setloglevel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, getLogLevel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, setFormatter, arginfo_phalcon_logger_adapterinterface_setformatter, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, isTransaction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, begin, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, commit, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, rollback, NULL, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Logger_Adapter, emergence, emergency, arginfo_phalcon_logger_adapterinterface_emergency, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
	PHP_ME(Phalcon_Logger_Adapter, log, arginfo_phalcon_logger_adapterinterface_log, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, debug, arginfo_phalcon_logger_adapterinterface_debug, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, info, arginfo_phalcon_logger_adapterinterface_info, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, notice, arginfo_phalcon_logger_adapterinterface_notice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, warning, arginfo_phalcon_logger_adapterinterface_warning, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, error, arginfo_phalcon_logger_adapterinterface_error, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, critical, arginfo_phalcon_logger_adapterinterface_critical, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, alert, arginfo_phalcon_logger_adapterinterface_alert, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Adapter, emergency, arginfo_phalcon_logger_adapterinterface_emergency, ZEND_ACC_PUBLIC)
	ZEND_FENTRY(logInternal, NULL, arginfo_phalcon_logger_adapter_loginternal, ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	PHP_FE_END
};

/**
 * Phalcon\Logger\Adapter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Logger_Adapter){

	PHALCON_REGISTER_CLASS(Phalcon\\Logger, Adapter, logger_adapter, phalcon_logger_adapter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_bool(phalcon_logger_adapter_ce, SL("_transaction"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_logger_adapter_ce, SL("_queue"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_logger_adapter_ce, SL("_formatter"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_logger_adapter_ce, SL("_logLevel"), PHALCON_LOGGER_SPECIAL, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_logger_adapter_ce, 1, phalcon_logger_adapterinterface_ce);

	return SUCCESS;
}

/*
static int phalcon_logger_adapter_string_level_to_int(const zval *level)
{
	const char *s = Z_STRVAL_P(level);
	size_t len    = (size_t)(Z_STRLEN_P(level));
	size_t i;

	struct sl {
		const char *str;
		size_t len;
		int level;
	};

	static struct sl lookup_table[] = {
		{ ZEND_STRL("emergency"), PHALCON_LOGGER_EMERGENCY },
		{ ZEND_STRL("alert"),     PHALCON_LOGGER_ALERT     },
		{ ZEND_STRL("critical"),  PHALCON_LOGGER_CRITICAL  },
		{ ZEND_STRL("error"),     PHALCON_LOGGER_ERROR     },
		{ ZEND_STRL("warning"),   PHALCON_LOGGER_WARNING   },
		{ ZEND_STRL("notice"),    PHALCON_LOGGER_NOTICE    },
		{ ZEND_STRL("info"),      PHALCON_LOGGER_INFO      },
		{ ZEND_STRL("debug"),     PHALCON_LOGGER_DEBUG     }
	};

	assert(Z_TYPE_P(level) == IS_STRING);

	for (i=0; i<sizeof(lookup_table)/sizeof(lookup_table[0]); ++i) {
		if (lookup_table[i].len == len && !memcmp(lookup_table[i].str, s, len)) {
			return lookup_table[i].level;
		}
	}

	{
		TSRMLS_FETCH();
		php_error_docref(NULL, E_NOTICE, "Unrecognized log level '%s'", s);
		return PHALCON_LOGGER_CUSTOM;
	}
}
*/

/**
 * Filters the logs sent to the handlers that are less or equal than a specific level
 *
 * @param int $level
 * @return Phalcon\Logger\Adapter
 */
PHP_METHOD(Phalcon_Logger_Adapter, setLogLevel){

	zval *level;

	phalcon_fetch_params(0, 1, 0, &level);

	phalcon_update_property(getThis(), SL("_logLevel"), level);
	RETURN_THIS();
}

/**
 * Returns the current log level
 *
 * @return int
 */
PHP_METHOD(Phalcon_Logger_Adapter, getLogLevel){


	RETURN_MEMBER(getThis(), "_logLevel");
}

/**
 * Sets the message formatter
 *
 * @param Phalcon\Logger\FormatterInterface $formatter
 * @return Phalcon\Logger\Adapter
 */
PHP_METHOD(Phalcon_Logger_Adapter, setFormatter){

	zval *formatter;

	phalcon_fetch_params(0, 1, 0, &formatter);
	PHALCON_VERIFY_INTERFACE_EX(formatter, phalcon_logger_formatterinterface_ce, phalcon_logger_exception_ce);

	phalcon_update_property(getThis(), SL("_formatter"), formatter);
	RETURN_THIS();
}

/**
 * Returns the current transaction
 *
 * @return Phalcon\Logger\Adapter
 */
PHP_METHOD(Phalcon_Logger_Adapter, isTransaction){

	RETURN_MEMBER(getThis(), "_transaction");
}

/**
 * Starts a transaction
 *
 * @return Phalcon\Logger\Adapter
 */
PHP_METHOD(Phalcon_Logger_Adapter, begin){


	phalcon_update_property_bool(getThis(), SL("_transaction"), 1);
	RETURN_THIS();
}

/**
 * Commits the internal transaction
 *
 * @return Phalcon\Logger\Adapter
 */
PHP_METHOD(Phalcon_Logger_Adapter, commit){

	zval transaction = {}, queue = {}, *message, message_str = {}, type = {}, time = {}, context = {};

	phalcon_read_property(&transaction, getThis(), SL("_transaction"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&transaction)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_logger_exception_ce, "There is no active transaction");
		return;
	}

	phalcon_update_property_bool(getThis(), SL("_transaction"), 0);

	/* Check if the queue has something to log */
	phalcon_read_property(&queue, getThis(), SL("_queue"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(queue) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(queue), message) {
			PHALCON_CALL_METHOD(&message_str, message, "getmessage");
			PHALCON_CALL_METHOD(&type, message, "gettype");
			PHALCON_CALL_METHOD(&time, message, "gettime");
			PHALCON_CALL_METHOD(&context, message, "getcontext");
			PHALCON_CALL_METHOD(NULL, getThis(), "loginternal", &message_str, &type, &time, &context);
		} ZEND_HASH_FOREACH_END();

		phalcon_update_property_empty_array(getThis(), SL("_queue"));
	}

	RETURN_THIS();
}

/**
 * Rollbacks the internal transaction
 *
 * @return Phalcon\Logger\Adapter
 */
PHP_METHOD(Phalcon_Logger_Adapter, rollback){

	zval transaction = {}, queue = {};

	phalcon_read_property(&transaction, getThis(), SL("_transaction"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&transaction)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_logger_exception_ce, "There is no active transaction");
		return;
	}

	phalcon_update_property_bool(getThis(), SL("_transaction"), 0);

	array_init_size(&queue, 0);
	phalcon_update_property(getThis(), SL("_queue"), &queue);

	RETURN_THIS();
}

static void phalcon_logger_adapter_log_helper(INTERNAL_FUNCTION_PARAMETERS, int level)
{
	zval *message, *context = NULL, type = {};

	phalcon_fetch_params(0, 1, 1, &message, &context);

	ZVAL_LONG(&type, level);

	if (!context) {
		context = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(NULL, getThis(), "log", &type, message, context);
	RETURN_ZVAL(getThis(), 1, 0);
}

/**
 * Sends/Writes an emergency message to the log
 *
 * @param string $message
 * @param array $context
 * @return Phalcon\Logger\AdapterInterface
 */
PHP_METHOD(Phalcon_Logger_Adapter, emergency){

	phalcon_logger_adapter_log_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHALCON_LOGGER_EMERGENCY);
}

/**
 * Sends/Writes a debug message to the log
 *
 * @param string $message
 * @param array $context
 * @return Phalcon\Logger\AdapterInterface
 */
PHP_METHOD(Phalcon_Logger_Adapter, debug){

	phalcon_logger_adapter_log_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHALCON_LOGGER_DEBUG);
}

/**
 * Sends/Writes an error message to the log
 *
 * @param string $message
 * @param array $context
 * @return Phalcon\Logger\AdapterInterface
 */
PHP_METHOD(Phalcon_Logger_Adapter, error){

	phalcon_logger_adapter_log_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHALCON_LOGGER_ERROR);
}

/**
 * Sends/Writes an info message to the log
 *
 * @param string $message
 * @param array $context
 * @return Phalcon\Logger\AdapterInterface
 */
PHP_METHOD(Phalcon_Logger_Adapter, info){

	phalcon_logger_adapter_log_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHALCON_LOGGER_INFO);
}

/**
 * Sends/Writes a notice message to the log
 *
 * @param string $message
 * @param array $context
 * @return Phalcon\Logger\AdapterInterface
 */
PHP_METHOD(Phalcon_Logger_Adapter, notice){

	phalcon_logger_adapter_log_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHALCON_LOGGER_NOTICE);
}

/**
 * Sends/Writes a warning message to the log
 *
 * @param string $message
 * @param array $context
 * @return Phalcon\Logger\AdapterInterface
 */
PHP_METHOD(Phalcon_Logger_Adapter, warning){

	phalcon_logger_adapter_log_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHALCON_LOGGER_WARNING);
}

/**
 * Sends/Writes an alert message to the log
 *
 * @param string $message
 * @param array $context
 * @return Phalcon\Logger\AdapterInterface
 */
PHP_METHOD(Phalcon_Logger_Adapter, alert){

	phalcon_logger_adapter_log_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHALCON_LOGGER_ALERT);
}

/**
 * Sends/Writes a critical message to the log
 *
 * @param string $message
 * @param array $context
 * @return Phalcon\Logger\AdapterInterface
 */
PHP_METHOD(Phalcon_Logger_Adapter, critical){

	phalcon_logger_adapter_log_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHALCON_LOGGER_CRITICAL);
}

/**
 * Logs messages to the internal logger. Appends messages to the log
 *
 * @param mixed type
 * @param string $message
 * @param array $context
 * @return Phalcon\Logger\Adapter
 */
PHP_METHOD(Phalcon_Logger_Adapter, log){

	zval *type, *message, *context = NULL, log_level = {}, timestamp = {}, transaction = {}, queue_item = {};
	int i_level;

	phalcon_fetch_params(0, 2, 1, &type, &message, &context);

	if (!context) {
		context = &PHALCON_GLOBAL(z_null);
	}

	i_level = Z_LVAL_P(type);

	phalcon_read_property(&log_level, getThis(), SL("_logLevel"), PH_NOISY|PH_READONLY);

	/* Only log the message if this is allowed by the current log level */
	if (phalcon_get_intval(&log_level) >= i_level) {
		ZVAL_LONG(&timestamp, (long)time(NULL));

		phalcon_read_property(&transaction, getThis(), SL("_transaction"), PH_NOISY|PH_READONLY);
		if (zend_is_true(&transaction)) {
			object_init_ex(&queue_item, phalcon_logger_item_ce);
			PHALCON_CALL_METHOD(NULL, &queue_item, "__construct", message, type, &timestamp, context);

			phalcon_update_property_array_append(getThis(), SL("_queue"), &queue_item);
			zval_ptr_dtor(&queue_item);
		} else {
			PHALCON_CALL_METHOD(NULL, getThis(), "loginternal", message, type, &timestamp, context);
		}
	}

	RETURN_THIS();
}
