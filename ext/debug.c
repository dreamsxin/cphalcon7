
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

#include "debug.h"
#include "debug/exception.h"
#include "logger/adapterinterface.h"
#include "logger.h"
#include "exception.h"
#include "version.h"
#include "di.h"
#include "loader.h"

#include <ext/standard/php_string.h>
#include <ext/standard/php_var.h>
#include <Zend/zend_builtin_functions.h>
#include <Zend/zend_execute.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/operators.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/file.h"
#include "kernel/output.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Debug
 *
 * Provides debug capabilities to Phalcon applications
 */
zend_class_entry *phalcon_debug_ce;

PHP_METHOD(Phalcon_Debug, setUri);
PHP_METHOD(Phalcon_Debug, setShowBackTrace);
PHP_METHOD(Phalcon_Debug, setShowFiles);
PHP_METHOD(Phalcon_Debug, setShowFileFragment);
PHP_METHOD(Phalcon_Debug, listen);
PHP_METHOD(Phalcon_Debug, listenExceptions);
PHP_METHOD(Phalcon_Debug, listenLowSeverity);
PHP_METHOD(Phalcon_Debug, halt);
PHP_METHOD(Phalcon_Debug, debugVar);
PHP_METHOD(Phalcon_Debug, clearVars);
PHP_METHOD(Phalcon_Debug, _escapeString);
PHP_METHOD(Phalcon_Debug, _getArrayDump);
PHP_METHOD(Phalcon_Debug, _getVarDump);
PHP_METHOD(Phalcon_Debug, getMajorVersion);
PHP_METHOD(Phalcon_Debug, getVersion);
PHP_METHOD(Phalcon_Debug, getCssSources);
PHP_METHOD(Phalcon_Debug, getJsSources);
PHP_METHOD(Phalcon_Debug, showTraceItem);
PHP_METHOD(Phalcon_Debug, onUncaughtException);
PHP_METHOD(Phalcon_Debug, onUserDefinedError);
PHP_METHOD(Phalcon_Debug, onShutdown);
PHP_METHOD(Phalcon_Debug, getLinesBeforeContext);
PHP_METHOD(Phalcon_Debug, setLinesBeforeContext);
PHP_METHOD(Phalcon_Debug, getLinesAfterContext);
PHP_METHOD(Phalcon_Debug, setLinesAfterContext);
PHP_METHOD(Phalcon_Debug, getFileLink);
PHP_METHOD(Phalcon_Debug, setLogger);
PHP_METHOD(Phalcon_Debug, enable);
PHP_METHOD(Phalcon_Debug, disable);
PHP_METHOD(Phalcon_Debug, isEnable);
PHP_METHOD(Phalcon_Debug, log);
PHP_METHOD(Phalcon_Debug, dumpVar);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_seturi, 0, 0, 1)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_setshowbacktrace, 0, 0, 1)
	ZEND_ARG_INFO(0, showBackTrace)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_setshowfiles, 0, 0, 1)
	ZEND_ARG_INFO(0, showFiles)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_setshowfilefragment, 0, 0, 1)
	ZEND_ARG_INFO(0, showFileFragment)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_listen, 0, 0, 0)
	ZEND_ARG_INFO(0, exceptions)
	ZEND_ARG_INFO(0, lowSeverity)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_debugvar, 0, 0, 1)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_onuncaughtexception, 0, 0, 1)
	ZEND_ARG_INFO(0, exception)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_onuserdefinederror, 0, 0, 2)
	ZEND_ARG_INFO(0, severity)
	ZEND_ARG_INFO(0, message)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_setcharset, 0, 0, 1)
	ZEND_ARG_INFO(0, charset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_setlines, 0, 0, 1)
	ZEND_ARG_INFO(0, lines)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_getfilelink, 0, 0, 3)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_setlogger, 0, 0, 1)
	ZEND_ARG_INFO(0, logger)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_enable, 0, 0, 0)
	ZEND_ARG_INFO(0, logger)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_log, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_debug_dumpvar, 0, 0, 1)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_debug_method_entry[] = {
	PHP_ME(Phalcon_Debug, setUri, arginfo_phalcon_debug_seturi, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, setShowBackTrace, arginfo_phalcon_debug_setshowbacktrace, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, setShowFiles, arginfo_phalcon_debug_setshowfiles, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, setShowFileFragment, arginfo_phalcon_debug_setshowfilefragment, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, listen, arginfo_phalcon_debug_listen, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, listenExceptions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, listenLowSeverity, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, halt, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, debugVar, arginfo_phalcon_debug_debugvar, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, clearVars, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, _escapeString, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Debug, _getArrayDump, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Debug, _getVarDump, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Debug, getMajorVersion, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, getVersion, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, getCssSources, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, getJsSources, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, showTraceItem, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Debug, onUncaughtException, arginfo_phalcon_debug_onuncaughtexception, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, onUserDefinedError, arginfo_phalcon_debug_onuserdefinederror, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, onShutdown, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, getLinesBeforeContext, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, setLinesBeforeContext, arginfo_phalcon_debug_setlines, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, getLinesAfterContext, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, setLinesAfterContext, arginfo_phalcon_debug_setlines, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, getFileLink, arginfo_phalcon_debug_getfilelink, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Debug, setLogger, arginfo_phalcon_debug_setlogger, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Debug, enable, arginfo_phalcon_debug_enable, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Debug, disable, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Debug, isEnable, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Debug, log, arginfo_phalcon_debug_log, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Debug, dumpVar, arginfo_phalcon_debug_dumpvar, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Debug initializer
 */
PHALCON_INIT_CLASS(Phalcon_Debug){

	PHALCON_REGISTER_CLASS(Phalcon, Debug, debug, phalcon_debug_method_entry, 0);

	zend_declare_property_string(phalcon_debug_ce, SL("_uri"), "//www.myleftstudio.com/debug/", ZEND_ACC_PUBLIC);
	zend_declare_property_bool(phalcon_debug_ce, SL("_hideDocumentRoot"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_debug_ce, SL("_showBackTrace"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_debug_ce, SL("_showFiles"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_debug_ce, SL("_showFileFragment"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_debug_ce, SL("_data"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_debug_ce, SL("_isActive"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_debug_ce, SL("_charset"), "utf-8", ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_long(phalcon_debug_ce, SL("_beforeContext"), 7, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_debug_ce, SL("_afterContext"), 5, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_debug_ce, SL("_logger"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_bool(phalcon_debug_ce, SL("_listen"), 0, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_debug_ce, SL("_logs"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	return SUCCESS;
}

/**
 * Change the base URI for static resources
 *
 * @param string $uri
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, setUri){

	zval *uri;

	phalcon_fetch_params(0, 1, 0, &uri);

	phalcon_update_property(getThis(), SL("_uri"), uri);
	RETURN_THIS();
}

/**
 * Sets if files the exception's backtrace must be showed
 *
 * @param boolean $showBackTrace
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, setShowBackTrace){

	zval *show_back_trace;

	phalcon_fetch_params(0, 1, 0, &show_back_trace);

	phalcon_update_property(getThis(), SL("_showBackTrace"), show_back_trace);
	RETURN_THIS();
}

/**
 * Set if files part of the backtrace must be shown in the output
 *
 * @param boolean $showFiles
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, setShowFiles){

	zval *show_files;

	phalcon_fetch_params(0, 1, 0, &show_files);

	phalcon_update_property(getThis(), SL("_showFiles"), show_files);
	RETURN_THIS();
}

/**
 * Sets if files must be completely opened and showed in the output
 * or just the fragment related to the exception
 *
 * @param boolean $showFileFragment
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, setShowFileFragment){

	zval *show_file_fragment;

	phalcon_fetch_params(0, 1, 0, &show_file_fragment);

	phalcon_update_property(getThis(), SL("_showFileFragment"), show_file_fragment);
	RETURN_THIS();
}

/**
 * Listen for uncaught exceptions and unsilent notices or warnings
 *
 * @param boolean $exceptions
 * @param boolean $lowSeverity
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, listen){

	zval *exceptions = NULL, *low_severity = NULL;

	phalcon_fetch_params(0, 0, 2, &exceptions, &low_severity);

	if (!exceptions || zend_is_true(exceptions)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "listenexceptions");
	}

	if (low_severity && zend_is_true(low_severity)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "listenlowseverity");
	}

	phalcon_update_static_property_bool_ce(phalcon_debug_ce, SL("_listen"), 1);
	RETURN_THIS();
}

/**
 * Listen for uncaught exceptions
 *
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, listenExceptions){

	zval handler = {};

	array_init_size(&handler, 2);
	phalcon_array_append(&handler, getThis(), PH_COPY);
	add_next_index_stringl(&handler, SL("onUncaughtException"));
	PHALCON_CALL_FUNCTION(NULL, "set_exception_handler", &handler);
	zval_ptr_dtor(&handler);
	RETURN_THIS();
}

/**
 * Listen for unsilent notices or warnings or user-defined error
 *
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, listenLowSeverity)
{
	zval handler = {};

	array_init_size(&handler, 2);
	phalcon_array_append(&handler, getThis(), PH_COPY);
	add_next_index_stringl(&handler, SL("onUserDefinedError"));
	PHALCON_CALL_FUNCTION(NULL, "set_error_handler", &handler);
	zval_ptr_dtor(&handler);

	array_init_size(&handler, 2);
	phalcon_array_append(&handler, getThis(), PH_COPY);
	add_next_index_stringl(&handler, SL("onShutdown"));
	PHALCON_CALL_FUNCTION(NULL, "register_shutdown_function", &handler);
	zval_ptr_dtor(&handler);
	RETURN_THIS();
}

/**
 * Halts the request showing a backtrace
 */
PHP_METHOD(Phalcon_Debug, halt){

	zend_throw_exception(NULL, "Halted request", 0);
}

/**
 * Adds a variable to the debug output
 *
 * @param mixed $var
 * @param string $key
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, debugVar){

	zval *var, *key = NULL, ztime = {}, backtrace = {}, data = {};

	phalcon_fetch_params(0, 1, 1, &var, &key);

	if (!key) {
		key = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_LONG(&ztime, (long) time(NULL));

	zend_fetch_debug_backtrace(&backtrace, 0, DEBUG_BACKTRACE_PROVIDE_OBJECT, 0);

	array_init_size(&data, 3);
	phalcon_array_append(&data, var, PH_COPY);
	phalcon_array_append(&data, &backtrace, PH_COPY);
	phalcon_array_append(&data, &ztime, PH_COPY);
	phalcon_update_property_array_append(getThis(), SL("_data"), &data);
	zval_ptr_dtor(&data);
	RETURN_THIS();
}

/**
 * Clears are variables added previously
 *
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, clearVars){


	phalcon_update_property_null(getThis(), SL("_data"));
	RETURN_THIS();
}

/**
 * Escapes a string with htmlentities
 *
 * @param string $value
 * @return string
 */
PHP_METHOD(Phalcon_Debug, _escapeString){

	zval *value, charset = {}, replaced_value = {};

	phalcon_fetch_params(0, 1, 0, &value);

	if (Z_TYPE_P(value) == IS_STRING) {
		zval line_break = {}, escaped_line_break = {};

		phalcon_read_static_property_ce(&charset, phalcon_debug_ce, SL("_charset"), PH_READONLY);

		ZVAL_STRING(&line_break, "\n");
		ZVAL_STRING(&escaped_line_break, "\\n");

		PHALCON_STR_REPLACE(&replaced_value, &line_break, &escaped_line_break, value);
		zval_ptr_dtor(&line_break);
		zval_ptr_dtor(&escaped_line_break);
		phalcon_htmlentities(return_value, &replaced_value, NULL, &charset);
		zval_ptr_dtor(&replaced_value);
		return;
	}

	RETURN_CTOR(value);
}

/**
 * Produces a recursive representation of an array
 *
 * @param array $argument
 * @return string
 */
PHP_METHOD(Phalcon_Debug, _getArrayDump){

	zval *argument, *n = NULL, number_arguments = {}, dump = {}, *v, joined_dump = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 1, &argument, &n);

	if (!n) {
		n = &PHALCON_GLOBAL(z_zero);
	}


	if (PHALCON_LT_LONG(n, 3)) {
		phalcon_fast_count(&number_arguments, argument);
		if (PHALCON_GT_LONG(&number_arguments, 0)) {
			if (PHALCON_LT_LONG(&number_arguments, 10)) {
				array_init(&dump);

				ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(argument), idx, str_key, v) {
					zval tmp = {}, var_dump = {}, escaped_string = {}, next = {}, array_dump = {}, class_name = {};
					zend_class_entry *ce;
					if (str_key) {
						ZVAL_STR(&tmp, str_key);
					} else {
						ZVAL_LONG(&tmp, idx);
					}
					if (PHALCON_IS_SCALAR(v)) {
						if (PHALCON_IS_STRING(v, "")) {
							PHALCON_CONCAT_SVS(&var_dump, "[", &tmp, "] =&gt; (empty string)");
						} else {
							PHALCON_CALL_METHOD(&escaped_string, getThis(), "_escapestring", v);
							PHALCON_CONCAT_SVSV(&var_dump, "[", &tmp, "] =&gt; ", &escaped_string);
							zval_ptr_dtor(&escaped_string);
						}
						phalcon_array_append(&dump, &var_dump, 0);
					} else {
						if (Z_TYPE_P(v) == IS_ARRAY) {
							phalcon_add_function(&next, n, &PHALCON_GLOBAL(z_one));

							PHALCON_CALL_METHOD(&array_dump, getThis(), "_getarraydump", v, &next);
							PHALCON_CONCAT_SVSVS(&var_dump, "[", &tmp, "] =&gt; Array(", &array_dump, ")");
							zval_ptr_dtor(&array_dump);
							phalcon_array_append(&dump, &var_dump, 0);
							continue;
						}
						if (Z_TYPE_P(v) == IS_OBJECT) {
							ce = Z_OBJCE_P(v);
							ZVAL_NEW_STR(&class_name, ce->name);

							PHALCON_CONCAT_SVSVS(&var_dump, "[", &tmp, "] =&gt; Object(", &class_name, ")");
							phalcon_array_append(&dump, &var_dump, 0);
							continue;
						}

						if (Z_TYPE_P(v) == IS_NULL) {
							PHALCON_CONCAT_SVS(&var_dump, "[", &tmp, "] =&gt; null");
							phalcon_array_append(&dump, &var_dump, 0);
							continue;
						}

						PHALCON_CONCAT_SVSV(&var_dump, "[", &tmp, "] =&gt; ", v);
						phalcon_array_append(&dump, &var_dump, 0);
					}
				} ZEND_HASH_FOREACH_END();

				phalcon_fast_join_str(&joined_dump, SL(", "), &dump);
				zval_ptr_dtor(&dump);
				RETURN_NCTOR(&joined_dump);
			}

			RETURN_NCTOR(&number_arguments);
		}
	}

	RETURN_NULL();
}

/**
 * Produces an string representation of a variable
 *
 * @param mixed $variable
 * @return string
 */
PHP_METHOD(Phalcon_Debug, _getVarDump){

	zval *variable, class_name = {}, dumped_object = {}, array_dump = {}, dump = {};

	phalcon_fetch_params(0, 1, 0, &variable);

	if (PHALCON_IS_SCALAR(variable)) {
		/**
		 * Boolean variables are represented as 'true'/'false'
		 */
		if (PHALCON_IS_BOOL(variable)) {
			if (zend_is_true(variable)) {
				RETURN_STRING("true");
			} else {
				RETURN_STRING("false");
			}
		}

		/**
		 * String variables are escaped to avoid XSS injections
		 */
		if (Z_TYPE_P(variable) == IS_STRING) {
			PHALCON_RETURN_CALL_METHOD(getThis(), "_escapestring", variable);
			return;
		}

		/**
		 * Other scalar variables are just converted to strings
		 */
		RETURN_CTOR(variable);
	}

	/**
	 * If the variable is an object print its class name
	 */
	if (Z_TYPE_P(variable) == IS_OBJECT) {
		const zend_class_entry *ce = Z_OBJCE_P(variable);

		ZVAL_STR(&class_name, ce->name);

		/**
		 * Try to check for a 'dump' method, this surely produces a better printable
		 * representation
		 */
		if (phalcon_method_exists_ex(variable, SL("dump")) == SUCCESS) {
			PHALCON_CALL_METHOD(&dumped_object, variable, "dump");

			/**
			 * dump() must return an array, generate a recursive representation using
			 * getArrayDump
			 */
			PHALCON_CALL_METHOD(&array_dump, getThis(), "_getarraydump", &dumped_object);
			zval_ptr_dtor(&dumped_object);

			PHALCON_CONCAT_SVSVS(&dump, "Object(", &class_name, ": ", &array_dump, ")");
			zval_ptr_dtor(&array_dump);
		} else {
			/**
			 * If dump() is not available just print the class name
			 */
			PHALCON_CONCAT_SVS(&dump, "Object(", &class_name, ")</span>");
		}

		RETURN_NCTOR(&dump);
	}

	/**
	 * Recursively process the array and enclose it in Array()
	 */
	if (Z_TYPE_P(variable) == IS_ARRAY) {
		PHALCON_CALL_METHOD(&array_dump, getThis(), "_getarraydump", variable);
		PHALCON_CONCAT_SVS(return_value, "Array(", &array_dump, ")");
		zval_ptr_dtor(&array_dump);
		return;
	}

	/**
	 * Null variables are represented as 'null'
	 * Other types are represented by its type
	 */
	RETURN_STRING(zend_zval_type_name(variable));
}

/**
 * Returns the major framework's version
 *
 * @return string
 */
PHP_METHOD(Phalcon_Debug, getMajorVersion){

	zval version = {}, parts = {};

	PHALCON_CALL_CE_STATIC(&version, phalcon_version_ce, "get");

	phalcon_fast_explode_str(&parts, SL(" "), &version);
	zval_ptr_dtor(&version);

	phalcon_array_fetch_long(return_value, &parts, 0, PH_NOISY|PH_COPY);
	zval_ptr_dtor(&parts);
}

/**
 * Generates a link to the current version documentation
 *
 * @return string
 */
PHP_METHOD(Phalcon_Debug, getVersion){

	zval version = {};

	PHALCON_CALL_METHOD(&version, getThis(), "getmajorversion");
	PHALCON_CONCAT_SVSVS(return_value, "<div class=\"version\">Phalcon7 Framework <a target=\"_new\" href=\"http://docs.myleftstudio.com/", &version, "/\">", &version, "</a></div>");
	zval_ptr_dtor(&version);
}

/**
 * Returns the css sources
 *
 * @return string
 */
PHP_METHOD(Phalcon_Debug, getCssSources){

	zval uri = {}, css = {};

	phalcon_read_property(&uri, getThis(), SL("_uri"), PH_NOISY|PH_READONLY);
	PHALCON_MM_INIT();
	PHALCON_CONCAT_SVS(&css, "<link href=\"", &uri, "bootstrap/css/bootstrap.min.css\" type=\"text/css\" rel=\"stylesheet\" />");
	PHALCON_MM_ADD_ENTRY(&css);
	PHALCON_SCONCAT_SVS(&css, "<link href=\"", &uri, "bower_components/jquery-ui/themes/ui-lightness/theme.css\" type=\"text/css\" rel=\"stylesheet\" />");
	PHALCON_MM_ADD_ENTRY(&css);
	PHALCON_SCONCAT_SVS(&css, "<link href=\"", &uri, "style.css\" type=\"text/css\" rel=\"stylesheet\" />");
	PHALCON_MM_ADD_ENTRY(&css);
	RETURN_MM_CTOR(&css);
}

/**
 * Returns the javascript sources
 *
 * @return string
 */
PHP_METHOD(Phalcon_Debug, getJsSources){

	zval uri = {}, js = {};

	phalcon_read_property(&uri, getThis(), SL("_uri"), PH_NOISY|PH_READONLY);
	PHALCON_MM_INIT();
	PHALCON_CONCAT_SVS(&js, "<script type=\"text/javascript\" src=\"", &uri, "bower_components/jquery/dist/jquery.min.js\"></script>");
	PHALCON_MM_ADD_ENTRY(&js);
	PHALCON_SCONCAT_SVS(&js, "<script type=\"text/javascript\" src=\"", &uri, "bootstrap/js/bootstrap.min.js\"></script>");
	PHALCON_MM_ADD_ENTRY(&js);
	PHALCON_SCONCAT_SVS(&js, "<script type=\"text/javascript\" src=\"", &uri, "bower_components/jquery.scrollTo/jquery.scrollTo.min.js\"></script>");
	PHALCON_MM_ADD_ENTRY(&js);
	PHALCON_SCONCAT_SVS(&js, "<script type=\"text/javascript\" src=\"", &uri, "prettify/prettify.js\"></script>");
	PHALCON_MM_ADD_ENTRY(&js);
	PHALCON_SCONCAT_SVS(&js, "<script type=\"text/javascript\" src=\"", &uri, "pretty.js\"></script>");
	PHALCON_MM_ADD_ENTRY(&js);
	RETURN_MM_CTOR(&js);
}

PHP_METHOD(Phalcon_Debug, getFileLink) {

	zval *file, *line, *format;

	phalcon_fetch_params(0, 3, 0, &file, &line, &format);
	PHALCON_ENSURE_IS_STRING(file);
	PHALCON_ENSURE_IS_STRING(line);

	if (Z_TYPE_P(format) == IS_STRING) {
		zend_string *tmp, *link;
		zval z_link = {};

		tmp  = php_str_to_str(Z_STRVAL_P(format), Z_STRLEN_P(format), SL("%f"), Z_STRVAL_P(file), Z_STRLEN_P(file));
		link = php_str_to_str(ZSTR_VAL(tmp), ZSTR_LEN(tmp), SL("%l"), Z_STRVAL_P(line), Z_STRLEN_P(line));

		ZVAL_STR(&z_link, link);

		PHALCON_CONCAT_SVSVS(return_value, "<a href=\"", &z_link, "\">", file, "</a>");

		zend_string_release(tmp);
		zend_string_release(link);
	} else {
		RETVAL_ZVAL(file, 1, 0);
	}
}

/**
 * Shows a backtrace item
 *
 * @param int $n
 * @param array $trace
 */
PHP_METHOD(Phalcon_Debug, showTraceItem){

	zval *n, *trace, *link_format, space = {}, two_spaces = {}, underscore = {}, minus = {}, html = {}, class_name = {}, prepare_uri_class = {};
	zval type = {}, function_name = {}, trace_args = {}, *argument, file = {}, line = {}, show_files = {}, lines = {}, number_lines = {};
	zval show_file_fragment = {}, before_context = {}, before_line = {}, first_line = {}, after_context = {}, after_line = {}, last_line = {}, comment_pattern = {}, charset = {}, tab = {}, comment = {}, i = {}, formatted_file = {};
	zend_class_entry *class_ce;

	phalcon_fetch_params(1, 3, 0, &n, &trace, &link_format);

	PHALCON_MM_ZVAL_STRING(&space, " ");
	PHALCON_MM_ZVAL_STRING(&two_spaces, "  ");
	PHALCON_MM_ZVAL_STRING(&underscore, "_");
	PHALCON_MM_ZVAL_STRING(&minus, "-");

	/**
	 * Every trace in the backtrace have a unique number
	 */
	PHALCON_CONCAT_SVS(&html, "<tr><td align=\"right\" valign=\"top\" class=\"error-number\">#", n, "</td><td>");
	PHALCON_MM_ADD_ENTRY(&html);
	if (phalcon_array_isset_fetch_str(&class_name, trace, SL("class"), PH_READONLY)) {
		class_ce = phalcon_fetch_class(&class_name, ZEND_FETCH_CLASS_AUTO | ZEND_FETCH_CLASS_SILENT);

		if (!class_ce) {
			/* Unable to load the class, should never happen */
		} else if (is_phalcon_class(class_ce)) {
			zval namespace_separator = {};
			ZVAL_STRING(&namespace_separator, "\\");

			/* Prepare the class name according to the Phalcon's conventions */
			PHALCON_STR_REPLACE(&prepare_uri_class, &namespace_separator, &underscore, &class_name);
			zval_ptr_dtor(&namespace_separator);
			PHALCON_MM_ADD_ENTRY(&prepare_uri_class);

			/* Generate a link to the official docs */
			PHALCON_SCONCAT_SVSVS(&html, "<span class=\"error-class\"><a target=\"_new\" href=\"http://www.myleftstudio.com/api/", &prepare_uri_class, ".html\">", &class_name, "</a></span>");
			PHALCON_MM_ADD_ENTRY(&html);
		} else if (class_ce->type == ZEND_INTERNAL_CLASS) {
			zval lower_class_name = {}, prepare_internal_class = {};
			phalcon_fast_strtolower(&lower_class_name, &class_name);
			PHALCON_STR_REPLACE(&prepare_internal_class, &underscore, &minus, &lower_class_name);
			zval_ptr_dtor(&lower_class_name);

			/* Generate a link to the official docs */
			PHALCON_SCONCAT_SVSVS(&html, "<span class=\"error-class\"><a target=\"_new\" href=\"http://php.net/manual/en/class.", &prepare_internal_class, ".php\">", &class_name, "</a></span>");
			zval_ptr_dtor(&prepare_internal_class);
			PHALCON_MM_ADD_ENTRY(&html);
		} else {
			PHALCON_SCONCAT_SVS(&html, "<span class=\"error-class\">", &class_name, "</span>");
			PHALCON_MM_ADD_ENTRY(&html);
		}

		/**
		 * Object access operator: static/instance
		 */
		phalcon_array_fetch_str(&type, trace, SL("type"), PH_NOISY|PH_READONLY);
		phalcon_concat_self(&html, &type);
		PHALCON_MM_ADD_ENTRY(&html);
	}

	/**
	 * Normally the backtrace contains only classes
	 */

	if (phalcon_array_isset_fetch_str(&function_name, trace, SL("function"), PH_READONLY)) {
		if (phalcon_array_isset_str(trace, SL("class"))) {
			PHALCON_SCONCAT_SVS(&html, "<span class=\"error-function\">", &function_name, "</span>");
			PHALCON_MM_ADD_ENTRY(&html);
		} else {
			zend_function *func;

			/**
			 * Check if the function exists
			 */
			if ((func = phalcon_fetch_function(Z_STR(function_name))) != NULL) {

				/**
				 * Internal functions links to the PHP documentation
				 */
				if (func->type == ZEND_INTERNAL_FUNCTION) {
					zval prepared_function_name = {};
					/**
					 * Prepare function's name according to the conventions in the docs
					 */
					PHALCON_STR_REPLACE(&prepared_function_name, &underscore, &minus, &function_name);
					PHALCON_SCONCAT_SVSVS(&html, "<span class=\"error-function\"><a target=\"_new\" href=\"http://php.net/manual/en/function.", &prepared_function_name, ".php\">", &function_name, "</a></span>");
					zval_ptr_dtor(&prepared_function_name);
					PHALCON_MM_ADD_ENTRY(&html);
				} else {
					PHALCON_SCONCAT_SVS(&html, "<span class=\"error-function\">", &function_name, "</span>");
					PHALCON_MM_ADD_ENTRY(&html);
				}
			} else {
				PHALCON_SCONCAT_SVS(&html, "<span class=\"error-function\">", &function_name, "</span>");
				PHALCON_MM_ADD_ENTRY(&html);
			}
		}
	}

	/**
	 * Check for arguments in the function
	 */
	if (phalcon_array_isset_fetch_str(&trace_args, trace, SL("args"), PH_READONLY)) {
		if (phalcon_fast_count_ev(&trace_args)) {
			zval arguments = {}, joined_arguments = {};
			array_init(&arguments);
			PHALCON_MM_ADD_ENTRY(&html);
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(trace_args), argument) {
				zval dumped_argument = {}, span_argument = {};
				/**
				 * Every argument is generated using _getVarDump
				 */
				PHALCON_MM_CALL_METHOD(&dumped_argument, getThis(), "_getvardump", argument);
				PHALCON_CONCAT_SVS(&span_argument, "<span class=\"error-parameter\">", &dumped_argument, "</span>");
				zval_ptr_dtor(&dumped_argument);

				/**
				 * Append the HTML generated to the argument's list
				 */
				phalcon_array_append(&arguments, &span_argument, 0);
			} ZEND_HASH_FOREACH_END();

			/**
			 * Join all the arguments
			 */
			phalcon_fast_join_str(&joined_arguments, SL(", "), &arguments);
			PHALCON_SCONCAT_SVS(&html, "(", &joined_arguments, ")");
			zval_ptr_dtor(&joined_arguments);
			PHALCON_MM_ADD_ENTRY(&html);
		} else {
			phalcon_concat_self_str(&html, SL("()"));
			PHALCON_MM_ADD_ENTRY(&html);
		}
	}

	/**
	 * When 'file' is present, it usually means the function is provided by the user
	 */
	if (phalcon_array_isset_fetch_str(&file, trace, SL("file"), PH_READONLY)) {
		phalcon_array_fetch_str(&line, trace, SL("line"), PH_NOISY|PH_READONLY);

		PHALCON_MM_CALL_METHOD(&formatted_file, getThis(), "getfilelink", &file, &line, link_format);

		/**
		 * Realpath to the file and its line using a special header
		 */
		PHALCON_SCONCAT_SVSVS(&html, "<br/><div class=\"error-file\">", &formatted_file, " (", &line, ")</div>");
		zval_ptr_dtor(&formatted_file);
		PHALCON_MM_ADD_ENTRY(&html);

		phalcon_read_property(&show_files, getThis(), SL("_showFiles"), PH_READONLY);

		/**
		 * The developer can change if the files must be opened or not
		 */
		if (zend_is_true(&show_files)) {
			/**
			 * Open the file to an array using 'file', this respects the openbase-dir directive
			 */
			PHALCON_MM_CALL_FUNCTION(&lines, "file", &file);
			PHALCON_MM_ADD_ENTRY(&lines);
			phalcon_fast_count(&number_lines, &lines);

			phalcon_read_property(&show_file_fragment, getThis(), SL("_showFileFragment"), PH_READONLY);

			/**
			 * File fragments just show a piece of the file where the exception is located
			 */
			if (zend_is_true(&show_file_fragment)) {

				/**
				 * Take lines back to the current exception's line
				 */
				phalcon_read_property(&before_context, getThis(), SL("_beforeContext"), PH_READONLY);

				phalcon_sub_function(&before_line, &line, &before_context);

				/**
				 * Check for overflows
				 */
				if (PHALCON_LT_LONG(&before_line, 1)) {
					ZVAL_COPY_VALUE(&first_line, &PHALCON_GLOBAL(z_one));
				} else {
					ZVAL_COPY_VALUE(&first_line, &before_line);
				}

				/**
				 * Take lines after the current exception's line
				 */
				phalcon_read_property(&after_context, getThis(), SL("_afterContext"), PH_READONLY);

				phalcon_add_function(&after_line, &line, &after_context);

				/**
				 * Check for overflows
				 */
				if (PHALCON_GT(&after_line, &number_lines)) {
					ZVAL_COPY_VALUE(&last_line, &number_lines);
				} else {
					ZVAL_COPY_VALUE(&last_line, &after_line);
				}

				PHALCON_SCONCAT_SVSVSVS(&html, "<pre class='prettyprint highlight:", &first_line, ":", &line, " linenums:", &first_line, "'>");
				PHALCON_MM_ADD_ENTRY(&html);
			} else {
				ZVAL_COPY_VALUE(&first_line, &PHALCON_GLOBAL(z_one));
				ZVAL_COPY_VALUE(&last_line, &number_lines);
				PHALCON_SCONCAT_SVSVS(&html, "<pre class='prettyprint highlight:", &first_line, ":", &line, " linenums error-scroll'>");
				PHALCON_MM_ADD_ENTRY(&html);
			}

			PHALCON_MM_ZVAL_STRING(&comment_pattern, "#\\*\\/$#");

			phalcon_read_static_property_ce(&charset, phalcon_debug_ce, SL("_charset"), PH_READONLY);

			PHALCON_MM_ZVAL_STRING(&tab, "\t");
			PHALCON_MM_ZVAL_STRING(&comment, "* /");

			ZVAL_COPY_VALUE(&i, &first_line);

			while (PHALCON_LE(&i, &last_line)) {
				zval line_position = {}, current_line = {}, trimmed = {}, is_comment = {}, spaced_current_line = {}, escaped_line = {};
				/**
				 * Current line in the file
				 */
				phalcon_sub_function(&line_position, &i, &PHALCON_GLOBAL(z_one));

				/**
				 * Current line content in the piece of file
				 */
				phalcon_array_fetch(&current_line, &lines, &line_position, PH_NOISY|PH_READONLY);

				/**
				 * File fragments are cleaned, removing tabs and comments
				 */
				if (zend_is_true(&show_file_fragment)) {
					if (PHALCON_IS_EQUAL(&i, &first_line)) {
						ZVAL_STR(&trimmed, phalcon_trim(&current_line, NULL, PHALCON_TRIM_RIGHT));
						PHALCON_MM_ADD_ENTRY(&trimmed);
						phalcon_preg_match(&is_comment, &comment_pattern, &current_line, NULL, 0, 0);

						if (zend_is_true(&is_comment)) {
							PHALCON_STR_REPLACE(&spaced_current_line, &comment, &space, &current_line);
							PHALCON_MM_ADD_ENTRY(&spaced_current_line);
							ZVAL_COPY_VALUE(&current_line, &spaced_current_line);
						}
					}
				}

				/**
				 * Print a non break space if the current line is a line break, this allows to show
				 * the html zebra properly
				 */
				if (PHALCON_IS_STRING(&current_line, "\n")) {
					phalcon_concat_self_str(&html, SL("&nbsp;\n"));
					PHALCON_MM_ADD_ENTRY(&html);
				} else {
					if (PHALCON_IS_STRING(&current_line, "\r\n")) {
						phalcon_concat_self_str(&html, SL("&nbsp;\n"));
						PHALCON_MM_ADD_ENTRY(&html);
					} else {
						PHALCON_STR_REPLACE(&spaced_current_line, &tab, &two_spaces, &current_line);
						PHALCON_MM_ADD_ENTRY(&spaced_current_line);
						phalcon_htmlentities(&escaped_line, &spaced_current_line, NULL, &charset);
						PHALCON_MM_ADD_ENTRY(&escaped_line);
						phalcon_concat_self(&html, &escaped_line);
						PHALCON_MM_ADD_ENTRY(&html);
					}
				}

				phalcon_increment(&i);
			}
			phalcon_concat_self_str(&html, SL("</pre>"));
			PHALCON_MM_ADD_ENTRY(&html);
		}
	}

	phalcon_concat_self_str(&html, SL("</td></tr>"));
	PHALCON_MM_ADD_ENTRY(&html);
	RETURN_MM_CTOR(&html);
}

/**
 * Handles uncaught exceptions
 *
 * @param \Exception $exception
 * @return boolean
 */
PHP_METHOD(Phalcon_Debug, onUncaughtException){

	zval *exception, is_active = {}, message = {}, class_name = {}, css_sources = {}, escaped_message = {}, html = {}, version = {}, file = {}, line = {}, show_back_trace = {};
	zval data_vars = {}, trace = {}, *trace_item, *_REQUEST, *value = NULL, *_SERVER, *_SESSION, *_COOKIE, files = {}, di = {}, service_name = {}, router = {}, routes = {}, *route;
	zval loader = {}, loader_value = {}, dumped_loader_value = {}, memory = {}, *data_var, logs = {}, *log, js_sources = {}, formatted_file = {}, z_link_format = {};
	zend_bool ini_exists = 1;
	zend_class_entry *ce;
	zend_string *str_key;
	ulong idx;
	char* link_format;

	phalcon_fetch_params(1, 1, 0, &exception);
	PHALCON_MM_VERIFY_CLASS_EX(exception, zend_exception_get_default(), phalcon_exception_ce);

	/**
	 * Cancel the output buffer if active
	 */
	if (phalcon_ob_get_level() > 0) {
		phalcon_ob_end_clean();
	}

	phalcon_read_static_property_ce(&is_active, phalcon_debug_ce, SL("_isActive"), PH_READONLY);

	/**
	 * Avoid that multiple exceptions being showed
	 */
	if (zend_is_true(&is_active)) {
		PHALCON_MM_CALL_METHOD(&message, exception, "getmessage");
		zend_print_zval(&message, 0);
		zval_ptr_dtor(&message);
	}

	/**
	 * Globally block the debug component to avoid other exceptions must be shown
	 */
	zend_update_static_property_bool(phalcon_debug_ce, SL("_isActive"), 1);

	ce = Z_OBJCE_P(exception);
	ZVAL_NEW_STR(&class_name, ce->name);
	PHALCON_MM_ADD_ENTRY(&class_name);
	PHALCON_MM_CALL_METHOD(&message, exception, "getmessage");
	PHALCON_MM_ADD_ENTRY(&message);

	/**
	 * CSS static sources to style the error presentation
	 */
	PHALCON_MM_CALL_METHOD(&css_sources, getThis(), "getcsssources");
	PHALCON_MM_ADD_ENTRY(&css_sources);

	/**
	 * Escape the exception's message avoiding possible XSS injections?
	 */
	ZVAL_COPY_VALUE(&escaped_message, &message);

	/**
	 * Use the exception info as document's title
	 */
	PHALCON_CONCAT_SVSVS(&html, "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/><title>", &class_name, ": ", &escaped_message, "</title>");
	PHALCON_MM_ADD_ENTRY(&html);
	PHALCON_SCONCAT_VS(&html, &css_sources, "</head><body>");
	PHALCON_MM_ADD_ENTRY(&html);

	/**
	 * Get the version link
	 */
	PHALCON_MM_CALL_METHOD(&version, getThis(), "getversion");
	PHALCON_MM_ADD_ENTRY(&version);
	phalcon_concat_self(&html, &version);
	PHALCON_MM_ADD_ENTRY(&html);

	PHALCON_MM_CALL_METHOD(&file, exception, "getfile");
	PHALCON_MM_ADD_ENTRY(&file);
	PHALCON_MM_CALL_METHOD(&line, exception, "getline");
	PHALCON_MM_ADD_ENTRY(&line);

	link_format = zend_ini_string_ex(SL("xdebug.file_link_format"), 0, &ini_exists);
	if (!link_format || !ini_exists || !strlen(link_format)) {
		link_format = "file://%f#%l";
	}

	PHALCON_MM_ZVAL_STRING(&z_link_format, link_format);

	PHALCON_MM_CALL_METHOD(&formatted_file, getThis(), "getfilelink", &file, &line, &z_link_format);
	PHALCON_MM_ADD_ENTRY(&formatted_file);

	/**
	 * Main exception info
	 */
	phalcon_concat_self_str(&html, SL("<div class=\"container-fluid\"><div class=\"alert alert-danger\">"));
	PHALCON_MM_ADD_ENTRY(&html);
	PHALCON_SCONCAT_SVSVS(&html, "<h1>", &class_name, ": ", &escaped_message, "</h1>");
	PHALCON_MM_ADD_ENTRY(&html);

	PHALCON_SCONCAT_SVSVS(&html, "<span>", &formatted_file, " (", &line, ")</span>");
	PHALCON_MM_ADD_ENTRY(&html);
	phalcon_concat_self_str(&html, SL("</div>"));
	PHALCON_MM_ADD_ENTRY(&html);

	phalcon_read_property(&show_back_trace, getThis(), SL("_showBackTrace"), PH_READONLY);

	/**
	 * Check if the developer wants to show the backtrace or not
	 */
	if (zend_is_true(&show_back_trace)) {
		phalcon_read_property(&data_vars, getThis(), SL("_data"), PH_NOISY|PH_READONLY);

		/**
		 * Create the tabs in the page
		 */
		phalcon_concat_self_str(&html, SL("<div class=\"col-sm-4 col-md-2\"><ul class=\"nav nav-pills nav-stacked\" role=\"tablist\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\" class=\"active\"><a href=\"#error-tabs-1\" role=\"tab\" data-toggle=\"tab\">Backtrace</a></li>"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\"><a href=\"#error-tabs-2\" role=\"tab\" data-toggle=\"tab\">Request</a></li>"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\"><a href=\"#error-tabs-3\" role=\"tab\" data-toggle=\"tab\">Server</a></li>"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\"><a href=\"#error-tabs-4\" role=\"tab\" data-toggle=\"tab\">Session</a></li>"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\"><a href=\"#error-tabs-5\" role=\"tab\" data-toggle=\"tab\">Cookies</a></li>"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\"><a href=\"#error-tabs-6\" role=\"tab\" data-toggle=\"tab\">Included Files</a></li>"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\"><a href=\"#error-tabs-7\" role=\"tab\" data-toggle=\"tab\">Router</a></li>"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\"><a href=\"#error-tabs-8\" role=\"tab\" data-toggle=\"tab\">Loader</a></li>"));
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\"><a href=\"#error-tabs-9\" role=\"tab\" data-toggle=\"tab\">Memory</a></li>"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\"><a href=\"#error-tabs-10\" role=\"tab\" data-toggle=\"tab\">Variables</a></li>"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<li role=\"presentation\"><a href=\"#error-tabs-11\" role=\"tab\" data-toggle=\"tab\">Logs</a></li>"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("</ul></div><div class=\"col-sm-8 col-md-10 tab-content\">"));
		PHALCON_MM_ADD_ENTRY(&html);

		/**
		 * Print backtrace
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-1\" role=\"tabpanel\" class=\"tab-pane active\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);

		PHALCON_MM_CALL_METHOD(&trace, exception, "gettrace");
		PHALCON_MM_ADD_ENTRY(&trace);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(trace), idx, str_key, trace_item) {
			zval tmp = {}, html_item = {};
			if (str_key) {
				ZVAL_STR(&tmp, str_key);
			} else {
				ZVAL_LONG(&tmp, idx);
			}

			/**
			 * Every line in the trace is rendered using 'showTraceItem'
			 */
			PHALCON_MM_CALL_METHOD(&html_item, getThis(), "showtraceitem", &tmp, trace_item, &z_link_format);
			phalcon_concat_self(&html, &html_item);
			zval_ptr_dtor(&html_item);
			PHALCON_MM_ADD_ENTRY(&html);
		} ZEND_HASH_FOREACH_END();

		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		/**
		 * Print _REQUEST superglobal
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-2\" role=\"tabpanel\" class=\"tab-pane\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<tr><th>Key</th><th>Value</th></tr>"));
		PHALCON_MM_ADD_ENTRY(&html);

		_REQUEST = phalcon_get_global_str(SL("_REQUEST"));

		if (Z_TYPE_P(_REQUEST) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(_REQUEST), idx, str_key, value) {
				zval tmp = {}, joined_value = {};
				if (str_key) {
					ZVAL_STR(&tmp, str_key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}
				if (Z_TYPE_P(value) == IS_ARRAY) {
					PHALCON_MM_CALL_METHOD(&joined_value, getThis(), "_getvardump", value);
					PHALCON_SCONCAT_SVSVS(&html, "<tr><td class=\"key\">", &tmp, "</td><td>", &joined_value, "</td></tr>");
					zval_ptr_dtor(&joined_value);
					PHALCON_MM_ADD_ENTRY(&html);
				} else {
					PHALCON_SCONCAT_SVSVS(&html, "<tr><td class=\"key\">", &tmp, "</td><td>", value, "</td></tr>");
					PHALCON_MM_ADD_ENTRY(&html);
				}
			} ZEND_HASH_FOREACH_END();
		}

		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		/**
		 * Print _SERVER superglobal
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-3\" role=\"tabpanel\" class=\"tab-pane\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<tr><th>Key</th><th>Value</th></tr>"));
		PHALCON_MM_ADD_ENTRY(&html);

		_SERVER = phalcon_get_global_str(SL("_SERVER"));

		if (Z_TYPE_P(_SERVER) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(_SERVER), idx, str_key, value) {
				zval tmp = {}, dumped_argument = {};
				if (str_key) {
					ZVAL_STR(&tmp, str_key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}
				PHALCON_MM_CALL_METHOD(&dumped_argument, getThis(), "_getvardump", value);
				PHALCON_SCONCAT_SVSVS(&html, "<tr><td class=\"key\">", &tmp, "</td><td>", &dumped_argument, "</td></tr>");
				zval_ptr_dtor(&dumped_argument);
				PHALCON_MM_ADD_ENTRY(&html);
			} ZEND_HASH_FOREACH_END();
		}

		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		/**
		 * Print _SESSION superglobal
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-4\" role=\"tabpanel\" class=\"tab-pane\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<tr><th>Key</th><th>Value</th></tr>"));
		PHALCON_MM_ADD_ENTRY(&html);

		_SESSION = phalcon_get_global_str(SL("_SESSION"));

		if (Z_TYPE_P(_SESSION) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(_SESSION), idx, str_key, value) {
				zval tmp = {}, dumped_argument = {};
				if (str_key) {
					ZVAL_STR(&tmp, str_key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}
				PHALCON_MM_CALL_METHOD(&dumped_argument, getThis(), "_getvardump", value);
				PHALCON_SCONCAT_SVSVS(&html, "<tr><td class=\"key\">", &tmp, "</td><td>", &dumped_argument, "</td></tr>");
				zval_ptr_dtor(&dumped_argument);
				PHALCON_MM_ADD_ENTRY(&html);
			} ZEND_HASH_FOREACH_END();
		}

		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		/**
		 * Print _COOKIE superglobal
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-5\" role=\"tabpanel\" class=\"tab-pane\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<tr><th>Key</th><th>Value</th></tr>"));
		PHALCON_MM_ADD_ENTRY(&html);

		_COOKIE = phalcon_get_global_str(SL("_COOKIE"));

		if (Z_TYPE_P(_COOKIE) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(_COOKIE), idx, str_key, value) {
				zval tmp = {}, dumped_argument = {};
				if (str_key) {
					ZVAL_STR(&tmp, str_key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}
				PHALCON_MM_CALL_METHOD(&dumped_argument, getThis(), "_getvardump", value);
				PHALCON_SCONCAT_SVSVS(&html, "<tr><td class=\"key\">", &tmp, "</td><td>", &dumped_argument, "</td></tr>");
				zval_ptr_dtor(&dumped_argument);
				PHALCON_MM_ADD_ENTRY(&html);
			} ZEND_HASH_FOREACH_END();
		}

		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		/**
		 * Show included files
		 */
		PHALCON_MM_CALL_FUNCTION(&files, "get_included_files");
		PHALCON_MM_ADD_ENTRY(&files);
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-6\" role=\"tabpanel\" class=\"tab-pane\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<tr><th>#</th><th>Path</th></tr>"));
		PHALCON_MM_ADD_ENTRY(&html);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(files), idx, str_key, value) {
			zval tmp = {};
			if (str_key) {
				ZVAL_STR(&tmp, str_key);
			} else {
				ZVAL_LONG(&tmp, idx);
			}

			PHALCON_SCONCAT_SVSVS(&html, "<tr><td>", &tmp, "</th><td>", value, "</td></tr>");
			PHALCON_MM_ADD_ENTRY(&html);
		} ZEND_HASH_FOREACH_END();
		zval_ptr_dtor(&files);

		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		PHALCON_MM_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault");
		PHALCON_MM_ADD_ENTRY(&di);

		/**
		 * Router
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-7\" role=\"tabpanel\" class=\"tab-pane\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<tr><th>Pattern</th><th>Paths</th><th>Methods</th></tr>"));
		PHALCON_MM_ADD_ENTRY(&html);

		if (Z_TYPE(di) == IS_OBJECT) {
			ZVAL_STR(&service_name, IS(router));
			PHALCON_MM_CALL_METHOD(&router, &di, "getshared", &service_name);
			PHALCON_MM_ADD_ENTRY(&router);
			if (Z_TYPE(router) == IS_OBJECT) {
				PHALCON_MM_CALL_METHOD(&routes, &router, "getroutes");
				PHALCON_MM_ADD_ENTRY(&routes);

				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(routes), route) {
					zval pattern = {}, paths = {}, methods = {}, dumped_paths = {}, dumped_methods = {};
					PHALCON_MM_CALL_METHOD(&pattern, route, "getpattern");
					PHALCON_MM_ADD_ENTRY(&pattern);
					PHALCON_MM_CALL_METHOD(&paths, route, "getpaths");
					PHALCON_MM_ADD_ENTRY(&paths);
					PHALCON_MM_CALL_METHOD(&methods, route, "gethttpmethods");
					PHALCON_MM_ADD_ENTRY(&methods);

					PHALCON_MM_CALL_METHOD(&dumped_paths, getThis(), "_getvardump", &paths);
					PHALCON_MM_ADD_ENTRY(&dumped_paths);
					PHALCON_MM_CALL_METHOD(&dumped_methods, getThis(), "_getvardump", &methods);

					PHALCON_SCONCAT_SVSVSVS(&html, "<tr><td>", &pattern, "</th><td>", &dumped_paths, "</td><td>", &dumped_methods, "</td></tr>");
					PHALCON_MM_ADD_ENTRY(&html);
					zval_ptr_dtor(&dumped_methods);
				} ZEND_HASH_FOREACH_END();
			}
		}

		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		/**
		 * Loader
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-8\" role=\"tabpanel\" class=\"tab-pane\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<tr><th>#</th><th>Value</th></tr>"));
		PHALCON_MM_ADD_ENTRY(&html);

		phalcon_read_static_property_ce(&loader, phalcon_loader_ce, SL("_default"), PH_READONLY);
		if (Z_TYPE(loader) == IS_OBJECT) {
			PHALCON_MM_CALL_METHOD(&loader_value, &loader, "getdirs");
			PHALCON_MM_ADD_ENTRY(&loader_value);
			PHALCON_MM_CALL_METHOD(&dumped_loader_value, getThis(), "_getvardump", &loader_value);
			PHALCON_SCONCAT_SVS(&html, "<tr><td>Directories Registered</th><td>", &dumped_loader_value, "</td></tr>");
			zval_ptr_dtor(&dumped_loader_value);
			PHALCON_MM_ADD_ENTRY(&html);

			PHALCON_MM_CALL_METHOD(&loader_value, &loader, "getclasses");
			PHALCON_MM_ADD_ENTRY(&loader_value);
			PHALCON_MM_CALL_METHOD(&dumped_loader_value, getThis(), "_getvardump", &loader_value);
			PHALCON_SCONCAT_SVS(&html, "<tr><td>Class-map Registered</th><td>", &dumped_loader_value, "</td></tr>");
			zval_ptr_dtor(&dumped_loader_value);
			PHALCON_MM_ADD_ENTRY(&html);

			PHALCON_MM_CALL_METHOD(&loader_value, &loader, "getprefixes");
			PHALCON_MM_ADD_ENTRY(&loader_value);
			PHALCON_MM_CALL_METHOD(&dumped_loader_value, getThis(), "_getvardump", &loader_value);
			PHALCON_SCONCAT_SVS(&html, "<tr><td>Prefixes Registered</th><td>", &dumped_loader_value, "</td></tr>");
			zval_ptr_dtor(&dumped_loader_value);
			PHALCON_MM_ADD_ENTRY(&html);

			PHALCON_MM_CALL_METHOD(&loader_value, &loader, "getnamespaces");
			PHALCON_MM_ADD_ENTRY(&loader_value);
			PHALCON_MM_CALL_METHOD(&dumped_loader_value, getThis(), "_getvardump", &loader_value);
			PHALCON_SCONCAT_SVS(&html, "<tr><td>Namespaces Registered</th><td>", &dumped_loader_value, "</td></tr>");
			zval_ptr_dtor(&dumped_loader_value);
			PHALCON_MM_ADD_ENTRY(&html);
		}

		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		/**
		 * Memory usage
		 */
		ZVAL_LONG(&memory, zend_memory_usage(1));
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-9\" role=\"tabpanel\" class=\"tab-pane\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		PHALCON_SCONCAT_SVS(&html, "<tr><th colspan=\"2\">Memory</th></tr><tr><td>Usage</td><td>", &memory, "</td></tr>");
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		/**
		 * Print extra variables passed to the component
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-10\" role=\"tabpanel\" class=\"tab-pane\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<tr><th>Key</th><th>Value</th></tr>"));
		PHALCON_MM_ADD_ENTRY(&html);

		if (Z_TYPE(data_vars) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(data_vars), idx, str_key, data_var) {
				zval tmp = {}, variable = {}, dumped_argument = {};
				if (str_key) {
					ZVAL_STR(&tmp, str_key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}

				phalcon_array_fetch_long(&variable, data_var, 0, PH_NOISY|PH_READONLY);

				PHALCON_MM_CALL_METHOD(&dumped_argument, getThis(), "_getvardump", &variable);
				PHALCON_SCONCAT_SVSVS(&html, "<tr><td class=\"key\">", &tmp, "</td><td>", &dumped_argument, "</td></tr>");
				zval_ptr_dtor(&dumped_argument);
				PHALCON_MM_ADD_ENTRY(&html);
			} ZEND_HASH_FOREACH_END();
		}

		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		// Logs
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-11\" role=\"tabpanel\" class=\"tab-pane\"><table class=\"table table-striped\">"));
		PHALCON_MM_ADD_ENTRY(&html);
		phalcon_concat_self_str(&html, SL("<tr><th>Message</th></tr>"));
		PHALCON_MM_ADD_ENTRY(&html);

		phalcon_read_static_property_ce(&logs, phalcon_debug_ce, SL("_logs"), PH_READONLY);
		if (Z_TYPE(logs) == IS_ARRAY) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(logs), log) {
				zval dumped_argument = {};

				PHALCON_MM_CALL_METHOD(&dumped_argument, getThis(), "_getvardump", log);
				PHALCON_SCONCAT_SVS(&html, "<tr><td>", &dumped_argument, "</td></tr>");
				zval_ptr_dtor(&dumped_argument);
				PHALCON_MM_ADD_ENTRY(&html);
			} ZEND_HASH_FOREACH_END();
		}

		phalcon_concat_self_str(&html, SL("</table></div>"));
		PHALCON_MM_ADD_ENTRY(&html);

		phalcon_concat_self_str(&html, SL("</div></div>"));
		PHALCON_MM_ADD_ENTRY(&html);
	}

	/**
	 * Get Javascript sources
	 */
	PHALCON_MM_CALL_METHOD(&js_sources, getThis(), "getjssources");
	PHALCON_SCONCAT_VS(&html, &js_sources, "</div></body></html>");
	zval_ptr_dtor(&js_sources);
	PHALCON_MM_ADD_ENTRY(&html);

	/**
	 * Print the HTML, @TODO, add an option to store the html
	 */
	zend_print_zval(&html, 0);

	/**
	 * Unlock the exception renderer
	 */
	zend_update_static_property_bool(phalcon_debug_ce, SL("_isActive"), 0);
	RETURN_MM_TRUE;
}

/**
 * Handles user-defined error
 *
 * @param int $severity
 * @param string $message
 * @param string $file
 * @param string $line
 * @param array $context
 * @return boolean
 */
PHP_METHOD(Phalcon_Debug, onUserDefinedError){

	zval *severity, *message, *file = NULL, *line = NULL, *context = NULL, previous = {}, exception = {};
	zend_class_entry *default_exception_ce;

	phalcon_fetch_params(0, 2, 3, &severity, &message, &file, &line, &context);

	if (!file) {
		file = &PHALCON_GLOBAL(z_null);
	}

	if (!line) {
		line = &PHALCON_GLOBAL(z_null);
	}

	if (context && Z_TYPE_P(context) == IS_ARRAY) {
		if (
			!phalcon_array_isset_fetch_str(&previous, context, SL("e"), PH_READONLY) ||
			Z_TYPE(previous) != IS_OBJECT ||
			!instanceof_function_ex(Z_OBJCE_P(&previous), zend_exception_get_default(), 1)
		) {
			ZVAL_NULL(&previous);
		}
	} else {
		ZVAL_NULL(&previous);
	}

	default_exception_ce = zend_get_error_exception();

	object_init_ex(&exception, default_exception_ce);

	PHALCON_CALL_METHOD(NULL, &exception, "__construct", message, &PHALCON_GLOBAL(z_zero), severity, file, line, &previous);

	zend_throw_exception_object(&exception);

	RETURN_TRUE;
}

/**
 * Handles user-defined error
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Debug, onShutdown){

	zval error = {}, message = {}, type = {}, file = {}, line = {}, exception = {};
	zend_class_entry *default_exception_ce;

	PHALCON_CALL_FUNCTION(&error, "error_get_last");

	if (
		!phalcon_array_isset_fetch_str(&message, &error, SL("message"), PH_READONLY) ||
		!phalcon_array_isset_fetch_str(&type, &error, SL("type"), PH_READONLY) ||
		!phalcon_array_isset_fetch_str(&file, &error, SL("file"), PH_READONLY) ||
		!phalcon_array_isset_fetch_str(&line, &error, SL("line"), PH_READONLY)

	) {
		default_exception_ce = zend_get_error_exception();

		object_init_ex(&exception, default_exception_ce);

		PHALCON_CALL_METHOD(NULL, &exception, "__construct", &message, &PHALCON_GLOBAL(z_zero), &type, &file, &line);

		zend_throw_exception_object(&exception);
	}
	zval_ptr_dtor(&error);
}

/**
 * Returns the number of lines deplayed before the error line
 *
 * @brief int \Phalcon\Debug::getLinesBeforeContext(void)
 * @return int
 */
PHP_METHOD(Phalcon_Debug, getLinesBeforeContext) {
	RETURN_MEMBER(getThis(), "_beforeContext");
}

/**
 * Sets the number of lines deplayed before the error line
 *
 * @brief \Phalcon\Debug \Phalcon\Debug::setLinesBeforeContext(int $lines)
 * @param int $lines
 * @return \Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, setLinesBeforeContext) {

	zval *lines;

	phalcon_fetch_params(0, 1, 0, &lines);
	PHALCON_ENSURE_IS_LONG(lines);

	phalcon_update_property(getThis(), SL("_beforeContext"), lines);
	RETURN_THIS();
}

/**
 * Returns the number of lines deplayed after the error line
 *
 * @brief int \Phalcon\Debug::getLinesAfterContext(void)
 * @return int
 */
PHP_METHOD(Phalcon_Debug, getLinesAfterContext) {
	RETURN_MEMBER(getThis(), "_afterContext");
}

/**
 * Sets the number of lines deplayed after the error line
 *
 * @brief \Phalcon\Debug \Phalcon\Debug::setLinesAfterContext(int $lines)
 * @param int $lines
 * @return \Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, setLinesAfterContext) {

	zval *lines;

	phalcon_fetch_params(0, 1, 0, &lines);
	PHALCON_ENSURE_IS_LONG(lines);

	phalcon_update_property(getThis(), SL("_afterContext"), lines);
	RETURN_THIS();
}

/**
 * Sets logger
 *
 * @param Phalcon\Logger\AdapterInterface $logger
 */
PHP_METHOD(Phalcon_Debug, setLogger){

	zval *logger;

	phalcon_fetch_params(0, 1, 0, &logger);
	if (logger) {
		PHALCON_VERIFY_INTERFACE_EX(logger, phalcon_logger_adapterinterface_ce, phalcon_debug_exception_ce);
	} else {
		logger = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_static_property_ce(phalcon_debug_ce, SL("_logger"), logger);
}

/**
 * Enable simple debug mode
 *
 * @param Phalcon\Logger\AdapterInterface $logger
 */
PHP_METHOD(Phalcon_Debug, enable){

	zval *logger = NULL;

	phalcon_fetch_params(0, 0, 1, &logger);
	if (logger) {
		PHALCON_VERIFY_INTERFACE_EX(logger, phalcon_logger_adapterinterface_ce, phalcon_debug_exception_ce);
		phalcon_update_static_property_ce(phalcon_debug_ce, SL("_logger"), logger);
	}

	PHALCON_GLOBAL(debug).enable_debug = 1;
}

/**
 * Disable simple debug mode
 *
 */
PHP_METHOD(Phalcon_Debug, disable){

	PHALCON_GLOBAL(debug).enable_debug = 0;
}

/**
 * Check if debug mode
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Debug, isEnable){

	RETURN_BOOL(PHALCON_GLOBAL(debug).enable_debug);
}

/**
 * Logs messages
 *
 * @param string $message
 * @param mixed $type
 * @param array $context
 */
PHP_METHOD(Phalcon_Debug, log){

	zval *message, *_type = NULL, *context = NULL, type = {},  listen = {}, log_type = {}, log = {}, logger = {};

	phalcon_fetch_params(1, 1, 2, &message, &_type, &context);

	if (!_type || Z_TYPE_P(_type) == IS_NULL) {
		ZVAL_LONG(&type, PHALCON_LOGGER_DEBUG);
	} else {
		ZVAL_COPY_VALUE(&type, _type);
	}

	if (!context) {
		context = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_static_property_ce(&listen, phalcon_debug_ce, SL("_listen"), PH_READONLY);
	if (zend_is_true(&listen)) {
		if (Z_TYPE_P(message) == IS_STRING) {
			PHALCON_MM_CALL_CE_STATIC(&log_type, phalcon_logger_ce, "gettypestring", &type);
			PHALCON_MM_ADD_ENTRY(&log_type);
			PHALCON_CONCAT_SVSV(&log, "[", &log_type, "] ", message);
			PHALCON_MM_ADD_ENTRY(&log);
			phalcon_update_static_property_array_append_ce(phalcon_debug_ce, SL("_logs"), &log);
		} else {
			phalcon_update_static_property_array_append_ce(phalcon_debug_ce, SL("_logs"), message);
		}
	}

	phalcon_read_static_property_ce(&logger, phalcon_debug_ce, SL("_logger"), PH_READONLY);
	if (Z_TYPE(logger) != IS_NULL) {
		PHALCON_MM_VERIFY_INTERFACE_EX(&logger, phalcon_logger_adapterinterface_ce, phalcon_debug_exception_ce);
		PHALCON_MM_CALL_METHOD(NULL, &logger, "log", &type, message, context);
	} else if (!zend_is_true(&listen)) {
		phalcon_debug_print_r(message);
	}
	RETURN_MM();
}

/**
 * Dumps a string representation of variable to output
 *
 * @param mixed $var
 */
PHP_METHOD(Phalcon_Debug, dumpVar){

	zval *var, *level = NULL;
	int lvl = 0;

	phalcon_fetch_params(0, 1, 1, &var, &level);

	if (level && Z_LVAL_P(level) > 0) {
		lvl = Z_LVAL_P(level);
	}

	zend_printf("File: %s, Line: %d\n", zend_get_executed_filename(), zend_get_executed_lineno());
	php_debug_zval_dump(var, lvl);
}