
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
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  +------------------------------------------------------------------------+
*/

#include "logger/formatter/firephp.h"
#include "logger/formatter.h"
#include "logger/formatterinterface.h"

#include <Zend/zend_smart_str.h>
#include <Zend/zend_builtin_functions.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/array.h"


/**
 * Phalcon\Logger\Formatter\Firephp
 *
 * Formats messages so that they can be sent to FirePHP
 */
zend_class_entry *phalcon_logger_formatter_firephp_ce;

PHP_METHOD(Phalcon_Logger_Formatter_Firephp, getTypeString);
PHP_METHOD(Phalcon_Logger_Formatter_Firephp, getShowBacktrace);
PHP_METHOD(Phalcon_Logger_Formatter_Firephp, setShowBacktrace);
PHP_METHOD(Phalcon_Logger_Formatter_Firephp, enableLabels);
PHP_METHOD(Phalcon_Logger_Formatter_Firephp, labelsEnabled);
PHP_METHOD(Phalcon_Logger_Formatter_Firephp, format);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_logger_formatter_firephp_empty, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_logger_formatter_firephp_setshowbacktrace, 0, 0, 0)
	ZEND_ARG_INFO(0, show)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_logger_formatter_firephp_enablelabels, 0, 0, 0)
	ZEND_ARG_INFO(0, enable)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_logger_formatter_firephp_method_entry[] = {
	PHP_ME(Phalcon_Logger_Formatter_Firephp, getTypeString, arginfo_phalcon_logger_formatter_gettypestring, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter_Firephp, getShowBacktrace, arginfo_phalcon_logger_formatter_firephp_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter_Firephp, setShowBacktrace, arginfo_phalcon_logger_formatter_firephp_setshowbacktrace, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter_Firephp, enableLabels, arginfo_phalcon_logger_formatter_firephp_enablelabels, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter_Firephp, labelsEnabled, arginfo_phalcon_logger_formatter_firephp_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Logger_Formatter_Firephp, format, arginfo_phalcon_logger_formatterinterface_format, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Logger\Formatter\Firephp initializer
 */
PHALCON_INIT_CLASS(Phalcon_Logger_Formatter_Firephp){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Logger\\Formatter, Firephp, logger_formatter_firephp, phalcon_logger_formatter_ce, phalcon_logger_formatter_firephp_method_entry, 0);

	zend_declare_property_bool(phalcon_logger_formatter_firephp_ce, SL("_showBacktrace"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_logger_formatter_firephp_ce, SL("_enableLabels"), 1, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_logger_formatter_firephp_ce, 1, phalcon_logger_formatterinterface_ce);

	return SUCCESS;
}

PHP_METHOD(Phalcon_Logger_Formatter_Firephp, getShowBacktrace) {

	RETURN_MEMBER(getThis(), "_showBacktrace");
}

PHP_METHOD(Phalcon_Logger_Formatter_Firephp, setShowBacktrace) {

	zval *show;

	phalcon_fetch_params(0, 1, 0, &show);

	PHALCON_ENSURE_IS_BOOL(show);
	phalcon_update_property(getThis(), SL("_showBacktrace"), show);
}

PHP_METHOD(Phalcon_Logger_Formatter_Firephp, enableLabels) {

	zval *enable;

	phalcon_fetch_params(0, 1, 0, &enable);

	PHALCON_ENSURE_IS_BOOL(enable);
	phalcon_update_property(getThis(), SL("_enableLabels"), enable);
}

PHP_METHOD(Phalcon_Logger_Formatter_Firephp, labelsEnabled) {

	RETURN_MEMBER(getThis(), "_enableLabels");
}

/**
 * Returns the string meaning of a logger constant
 *
 * @param  integer $type
 * @return string
 */
PHP_METHOD(Phalcon_Logger_Formatter_Firephp, getTypeString) {

	static const char* lut[10] = {
		"ERROR", "ERROR", "WARN", "ERROR", "WARN",
		"INFO",  "INFO",  "LOG",  "INFO",  "LOG"
	};

	zval *type, types = {};
	int itype;

	phalcon_fetch_params(0, 1, 0, &type);
	PHALCON_ENSURE_IS_LONG(type);

	phalcon_read_property(&types, getThis(), SL("_typeStrings"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(return_value, &types, type, PH_COPY)) {
		itype = Z_LVAL_P(type);
		if (itype > 0 && itype < 10) {
			RETURN_STRING(lut[itype]);
		}

		RETURN_STRING("CUSTOM");
	}
}

/**
 * Applies a format to a message before sending it to the log
 *
 * @param string $message
 * @param int $type
 * @param int $timestamp
 * @param array $context
 * @return string
 */
PHP_METHOD(Phalcon_Logger_Formatter_Firephp, format) {

	zval *message, *type, *timestamp, *context, interpolated = {}, type_str = {}, show_backtrace = {}, enable_labels = {};
	zval backtrace = {}, *pzval, payload = {}, meta = {}, *file, *line, body = {}, encoded = {};
	int i_show_backtrace, i_enable_labels, found = 0;
	ulong idx;
	smart_str result = { 0 };

	phalcon_fetch_params(0, 4, 0, &message, &type, &timestamp, &context);

	if (Z_TYPE_P(context) == IS_ARRAY) {
		PHALCON_CALL_METHOD(&interpolated, getThis(), "interpolate", message, context);
	} else {
		ZVAL_COPY_VALUE(&interpolated, message);
	}

	PHALCON_CALL_METHOD(&type_str, getThis(), "gettypestring", type);

	phalcon_read_property(&show_backtrace, getThis(), SL("_showBacktrace"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&enable_labels, getThis(), SL("_enableLabels"), PH_NOISY|PH_READONLY);

	i_show_backtrace = zend_is_true(&show_backtrace);
	i_enable_labels  = zend_is_true(&enable_labels);

	/*
	 * Get the backtrace. This differs for different PHP versions.
	 * 5.3.6+ allows us to skip the function arguments which will save some memory
	 * For 5.4+ there is an extra argument.
	 */
	if (i_show_backtrace) {
		zend_fetch_debug_backtrace(&backtrace, 1, DEBUG_BACKTRACE_IGNORE_ARGS, 0);

		if (Z_TYPE(backtrace) == IS_ARRAY) {
			/*
			 * At this point we know that the backtrace is the array.
			 * Again, we intentionally do not use Phalcon's API because we know
			 * that we are working with the array / hash table and thus we can
			 * save some time by omitting Z_TYPE_P(x) == IS_ARRAY checks
			 */
			ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL(backtrace), idx, pzval) {
				if (Z_TYPE_P(pzval) == IS_ARRAY) {
					/*
					 * Here we need to skip the latest calls into Phalcon's core.
					 * Calls to Zend internal functions will have "file" index not set.
					 * We remove these entries from the array.
					 */
					if (!found && !zend_hash_str_exists(Z_ARRVAL_P(pzval), SL("file"))) {
						zend_hash_index_del(Z_ARRVAL(backtrace), idx);
					} else {
						/*
						 * Remove args and object indices. They usually give
						 * too much information; this is not suitable to send
						 * in the HTTP headers
						 */
						zend_hash_str_del(Z_ARRVAL_P(pzval), SL("args"));
						zend_hash_str_del(Z_ARRVAL_P(pzval), SL("object"));
						found = 1;
					}
				}
			} ZEND_HASH_FOREACH_END();
		}
	}

	/*
	 * The result will looks like this:
	 *
	 * array(
	 *     array('Type' => 'message type', 'Label' => 'message'),
	 *     array('backtrace' => array(backtrace goes here)
	 * )
	 */
	array_init_size(&payload, 2);

	array_init_size(&meta, 4);
	add_assoc_zval_ex(&meta, SL("Type"), &type_str);

	if (i_show_backtrace && Z_TYPE(backtrace) == IS_ARRAY) {
		if (likely((pzval = zend_hash_index_find(Z_ARRVAL(backtrace), 0)) != NULL) && likely(Z_TYPE_P(pzval) == IS_ARRAY)) {
			file = zend_hash_str_find(Z_ARRVAL_P(pzval), SL("file"));
			line = zend_hash_str_find(Z_ARRVAL_P(pzval), SL("line"));

			if (likely(file != NULL)) {
				Z_TRY_ADDREF_P(file);
				add_assoc_zval_ex(&meta, SL("File"), file);
			}

			if (likely(line != NULL)) {
				Z_TRY_ADDREF_P(line);
				add_assoc_zval_ex(&meta, SL("Line"), line);
			}
		}
	}

	if (i_enable_labels) {
		add_assoc_zval_ex(&meta, SL("Label"), &interpolated);
	}

	if (!i_enable_labels && !i_show_backtrace) {
		ZVAL_COPY_VALUE(&body, &interpolated);
	} else if (i_enable_labels && !i_show_backtrace) {
		ZVAL_EMPTY_STRING(&body);
	} else {
		array_init_size(&body, 2);

		if (i_show_backtrace) {
			add_assoc_zval_ex(&body, SL("backtrace"), &backtrace);
		}

		if (!i_enable_labels) {
			add_assoc_zval_ex(&body, SL("message"), &interpolated);
		}
	}

	add_next_index_zval(&payload, &meta);
	add_next_index_zval(&payload, &body);

	/* Convert everything to JSON */
	if (FAILURE == phalcon_json_encode(&encoded, &payload, 0)) {
		return;
	}

	/*
	 * We don't want to use Phalcon's concatenation API because it
	 * requires the memory manager. Therefore we fall back to using smart strings.
	 * smart_str_alloc4() will allocate all required memory amount (plus some more)
	 * in one go and this allows us to avoid performance penalties due to
	 * memory reallocations.
	 */
	if (Z_TYPE(encoded) == IS_STRING && Z_STRVAL(encoded) != NULL) {
		smart_str_alloc(&result, (uint)(Z_STRLEN(encoded) + 2 + 5), 0);

		/*
		 * The format is:
		 *
		 * <size>|[meta,body]|
		 *
		 * Meta and body are contained in encoded inside the array, as required
		 * by the protocol specification
		 * @see http://www.firephp.org/Wiki/Reference/Protocol
		 */
		smart_str_append_long(&result, Z_STRLEN(encoded));
		smart_str_appendc(&result, '|');
		smart_str_appendl(&result, Z_STRVAL(encoded), Z_STRLEN(encoded));
		smart_str_appendc(&result, '|');
		smart_str_0(&result);
	}

	/* We don't need the JSON message anymore */
	/* Do not free the smart string because we steal its data for zval */
	RETURN_STR(result.s);
}
