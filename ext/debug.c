
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

#include "debug.h"
#include "exception.h"
#include "version.h"

#include <ext/standard/php_string.h>
#include <Zend/zend_builtin_functions.h>

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
PHP_METHOD(Phalcon_Debug, getCharset);
PHP_METHOD(Phalcon_Debug, setCharset);
PHP_METHOD(Phalcon_Debug, getLinesBeforeContext);
PHP_METHOD(Phalcon_Debug, setLinesBeforeContext);
PHP_METHOD(Phalcon_Debug, getLinesAfterContext);
PHP_METHOD(Phalcon_Debug, setLinesAfterContext);
PHP_METHOD(Phalcon_Debug, getFileLink);
PHP_METHOD(Phalcon_Debug, enable);

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
	PHP_ME(Phalcon_Debug, getCharset, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Debug, setCharset, arginfo_phalcon_debug_setcharset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, getLinesBeforeContext, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, setLinesBeforeContext, arginfo_phalcon_debug_setlines, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, getLinesAfterContext, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, setLinesAfterContext, arginfo_phalcon_debug_setlines, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Debug, getFileLink, arginfo_phalcon_debug_getfilelink, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Debug, enable, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Debug initializer
 */
PHALCON_INIT_CLASS(Phalcon_Debug){

	PHALCON_REGISTER_CLASS(Phalcon, Debug, debug, phalcon_debug_method_entry, 0);

	zend_declare_property_string(phalcon_debug_ce, SL("_uri"), "//d2yyr506dy8ck0.cloudfront.net/debug/1.2.0/", ZEND_ACC_PUBLIC);
	zend_declare_property_string(phalcon_debug_ce, SL("_theme"), "default", ZEND_ACC_PUBLIC);
	zend_declare_property_bool(phalcon_debug_ce, SL("_hideDocumentRoot"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_debug_ce, SL("_showBackTrace"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_debug_ce, SL("_showFiles"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_debug_ce, SL("_showFileFragment"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_debug_ce, SL("_data"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_debug_ce, SL("_isActive"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_debug_ce, SL("_charset"), "utf-8", ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_long(phalcon_debug_ce, SL("_beforeContext"), 7, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_debug_ce, SL("_afterContext"), 5, ZEND_ACC_PROTECTED);

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

	phalcon_update_property_this(getThis(), SL("_uri"), uri);
	RETURN_THISW();
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

	phalcon_update_property_this(getThis(), SL("_showBackTrace"), show_back_trace);
	RETURN_THISW();
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

	phalcon_update_property_this(getThis(), SL("_showFiles"), show_files);
	RETURN_THISW();
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

	phalcon_update_property_this(getThis(), SL("_showFileFragment"), show_file_fragment);
	RETURN_THISW();
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
		PHALCON_CALL_METHODW(NULL, getThis(), "listenexceptions");
	}

	if (low_severity && zend_is_true(low_severity)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "listenlowseverity");
	}

	RETURN_THISW();
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
	PHALCON_CALL_FUNCTIONW(NULL, "set_exception_handler", &handler);
	RETURN_THISW();
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
	PHALCON_CALL_FUNCTIONW(NULL, "set_error_handler", &handler);

	array_init_size(&handler, 2);
	phalcon_array_append(&handler, getThis(), PH_COPY);
	add_next_index_stringl(&handler, SL("onShutdown"));
	PHALCON_CALL_FUNCTIONW(NULL, "register_shutdown_function", &handler);

	RETURN_THISW();
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
	RETURN_THISW();
}

/**
 * Clears are variables added previously
 *
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, clearVars){


	phalcon_update_property_null(getThis(), SL("_data"));
	RETURN_THISW();
}

/**
 * Escapes a string with htmlentities
 *
 * @param string $value
 * @return string
 */
PHP_METHOD(Phalcon_Debug, _escapeString){

	zval *value, *charset, replaced_value = {};

	phalcon_fetch_params(0, 1, 0, &value);

	if (Z_TYPE_P(value) == IS_STRING) {
		zval line_break = {}, escaped_line_break = {};

		charset = phalcon_read_static_property_ce(phalcon_debug_ce, SL("_charset"));

		ZVAL_STRING(&line_break, "\n");
		ZVAL_STRING(&escaped_line_break, "\\n");

		PHALCON_STR_REPLACEW(&replaced_value, &line_break, &escaped_line_break, value);
		phalcon_htmlentities(return_value, &replaced_value, NULL, charset);
		return;
	}

	RETURN_CTORW(value);
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

	phalcon_fast_count(&number_arguments, argument);

	if (PHALCON_LT_LONG(n, 3)) {
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
							PHALCON_CALL_METHODW(&escaped_string, getThis(), "_escapestring", v);
							PHALCON_CONCAT_SVSV(&var_dump, "[", &tmp, "] =&gt; ", &escaped_string);
						}
						phalcon_array_append(&dump, &var_dump, PH_COPY);
					} else {
						if (Z_TYPE_P(v) == IS_ARRAY) { 
							phalcon_add_function(&next, n, &PHALCON_GLOBAL(z_one));

							PHALCON_CALL_METHODW(&array_dump, getThis(), "_getarraydump", v, &next);

							PHALCON_CONCAT_SVSVS(&var_dump, "[", &tmp, "] =&gt; Array(", &array_dump, ")");
							phalcon_array_append(&dump, &var_dump, PH_COPY);
							continue;
						}
						if (Z_TYPE_P(v) == IS_OBJECT) {
							ce = Z_OBJCE_P(v);
							ZVAL_NEW_STR(&class_name, ce->name);

							PHALCON_CONCAT_SVSVS(&var_dump, "[", &tmp, "] =&gt; Object(", &class_name, ")");
							phalcon_array_append(&dump, &var_dump, PH_COPY);
							continue;
						}

						if (Z_TYPE_P(v) == IS_NULL) {
							PHALCON_CONCAT_SVS(&var_dump, "[", &tmp, "] =&gt; null");
							phalcon_array_append(&dump, &var_dump, PH_COPY);
							continue;
						}

						PHALCON_CONCAT_SVSV(&var_dump, "[", &tmp, "] =&gt; ", v);
						phalcon_array_append(&dump, &var_dump, PH_COPY);
					}
				} ZEND_HASH_FOREACH_END();

				phalcon_fast_join_str(&joined_dump, SL(", "), &dump);

				RETURN_CTORW(&joined_dump);
			}

			RETURN_CTORW(&number_arguments);
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
			PHALCON_RETURN_CALL_METHODW(getThis(), "_escapestring", variable);
			return;
		}

		/** 
		 * Other scalar variables are just converted to strings
		 */
		RETURN_CTORW(variable);
	}

	/** 
	 * If the variable is an object print its class name
	 */
	if (Z_TYPE_P(variable) == IS_OBJECT) {
		const zend_class_entry *ce = Z_OBJCE_P(variable);

		ZVAL_NEW_STR(&class_name, ce->name);

		/** 
		 * Try to check for a 'dump' method, this surely produces a better printable
		 * representation
		 */
		if (phalcon_method_exists_ex(variable, SL("dump")) == SUCCESS) {
			PHALCON_CALL_METHODW(&dumped_object, variable, "dump");

			/** 
			 * dump() must return an array, generate a recursive representation using
			 * getArrayDump
			 */
			PHALCON_CALL_METHODW(&array_dump, getThis(), "_getarraydump", &dumped_object);

			PHALCON_CONCAT_SVSVS(&dump, "Object(", &class_name, ": ", &array_dump, ")");
		} else {
			/** 
			 * If dump() is not available just print the class name
			 */
			PHALCON_CONCAT_SVS(&dump, "Object(", &class_name, ")</span>");
		}

		RETURN_CTORW(&dump);
	}

	/** 
	 * Recursively process the array and enclose it in Array()
	 */
	if (Z_TYPE_P(variable) == IS_ARRAY) { 
		PHALCON_CALL_METHODW(&array_dump, getThis(), "_getarraydump", variable);
		PHALCON_CONCAT_SVS(return_value, "Array(", &array_dump, ")");
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

	PHALCON_CALL_CE_STATICW(&version, phalcon_version_ce, "get");

	phalcon_fast_explode_str(&parts, SL(" "), &version);

	phalcon_array_fetch_long(return_value, &parts, 0, PH_NOISY);
}

/**
 * Generates a link to the current version documentation
 *
 * @return string
 */
PHP_METHOD(Phalcon_Debug, getVersion){

	zval version = {};

	PHALCON_CALL_METHODW(&version, getThis(), "getmajorversion");
	PHALCON_CONCAT_SVSVS(return_value, "<div class=\"version\">Phalcon Framework <a target=\"_new\" href=\"http://docs.myleftstudio.com/", &version, "/\">", &version, "</a></div>");
}

/**
 * Returns the css sources
 *
 * @return string
 */
PHP_METHOD(Phalcon_Debug, getCssSources){

	zval *uri;

	uri = phalcon_read_property(getThis(), SL("_uri"), PH_NOISY);

	PHALCON_CONCAT_SVS(return_value, "<link href=\"", uri, "jquery/jquery-ui.css\" type=\"text/css\" rel=\"stylesheet\" />");
	PHALCON_SCONCAT_SVS(return_value, "<link href=\"", uri, "themes/default/style.css\" type=\"text/css\" rel=\"stylesheet\" />");
}

/**
 * Returns the javascript sources
 *
 * @return string
 */
PHP_METHOD(Phalcon_Debug, getJsSources){

	zval *uri;

	uri = phalcon_read_property(getThis(), SL("_uri"), PH_NOISY);

	PHALCON_CONCAT_SVS(return_value, "<script type=\"text/javascript\" src=\"", uri, "jquery/jquery.js\"></script>");
	PHALCON_SCONCAT_SVS(return_value, "<script type=\"text/javascript\" src=\"", uri, "jquery/jquery-ui.js\"></script>");
	PHALCON_SCONCAT_SVS(return_value, "<script type=\"text/javascript\" src=\"", uri, "jquery/jquery.scrollTo.js\"></script>");
	PHALCON_SCONCAT_SVS(return_value, "<script type=\"text/javascript\" src=\"", uri, "prettify/prettify.js\"></script>");
	PHALCON_SCONCAT_SVS(return_value, "<script type=\"text/javascript\" src=\"", uri, "pretty.js\"></script>");
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

	zval *n, *trace, *link_format, space = {}, two_spaces = {}, underscore = {}, minus = {}, html = {}, class_name = {}, namespace_separator = {}, prepare_uri_class = {}, lower_class_name = {}, prepared_function_name = {};
	zval prepare_internal_class = {}, type = {}, function_name = {}, trace_args = {}, arguments = {}, *argument, joined_arguments = {}, file = {}, line = {}, show_files = {}, lines = {}, number_lines = {};
	zval show_file_fragment = {}, before_context = {}, before_line = {}, first_line = {}, after_context = {}, after_line = {}, last_line = {}, comment_pattern = {}, charset = {}, tab = {}, comment = {}, i = {}, formatted_file = {};
	zend_class_entry *class_ce;

	phalcon_fetch_params(0, 3, 0, &n, &trace, &link_format);

	ZVAL_STRING(&space, " ");
	ZVAL_STRING(&two_spaces, "  ");
	ZVAL_STRING(&underscore, "_");
	ZVAL_STRING(&minus, "-");

	/** 
	 * Every trace in the backtrace have a unique number
	 */
	PHALCON_CONCAT_SVS(&html, "<tr><td align=\"right\" valign=\"top\" class=\"error-number\">#", n, "</td><td>");
	if (phalcon_array_isset_fetch_str(&class_name, trace, SL("class"))) {
		class_ce = zend_fetch_class(Z_STR(class_name), ZEND_FETCH_CLASS_AUTO | ZEND_FETCH_CLASS_SILENT);

		if (!class_ce) {
			/* Unable to load the class, should never happen */
		} else if (is_phalcon_class(class_ce)) {
			ZVAL_STRING(&namespace_separator, "\\");

			/* Prepare the class name according to the Phalcon's conventions */
			PHALCON_STR_REPLACE(&prepare_uri_class, &namespace_separator, &underscore, &class_name);

			/* Generate a link to the official docs */
			PHALCON_SCONCAT_SVSVS(&html, "<span class=\"error-class\"><a target=\"_new\" href=\"http://docs.phalconphp.com/en/latest/api/", &prepare_uri_class, ".html\">", &class_name, "</a></span>");
		} else if (class_ce->type == ZEND_INTERNAL_CLASS) {
			phalcon_fast_strtolower(&lower_class_name, &class_name);

			PHALCON_STR_REPLACE(&prepare_internal_class, &underscore, &minus, &lower_class_name);

			/* Generate a link to the official docs */
			PHALCON_SCONCAT_SVSVS(&html, "<span class=\"error-class\"><a target=\"_new\" href=\"http://php.net/manual/en/class.", &prepare_internal_class, ".php\">", &class_name, "</a></span>");
		} else {
			PHALCON_SCONCAT_SVS(&html, "<span class=\"error-class\">", &class_name, "</span>");
		}

		/** 
		 * Object access operator: static/instance
		 */
		phalcon_array_fetch_str(&type, trace, SL("type"), PH_NOISY);
		phalcon_concat_self(&html, &type);
	}

	/** 
	 * Normally the backtrace contains only classes
	 */

	if (phalcon_array_isset_fetch_str(&function_name, trace, SL("function"))) {
		convert_to_string(&function_name);
		if (phalcon_array_isset_str(trace, SL("class"))) {
			PHALCON_SCONCAT_SVS(&html, "<span class=\"error-function\">", &function_name, "</span>");
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
					/** 
					 * Prepare function's name according to the conventions in the docs
					 */
					PHALCON_STR_REPLACE(&prepared_function_name, &underscore, &minus, &function_name);
					PHALCON_SCONCAT_SVSVS(&html, "<span class=\"error-function\"><a target=\"_new\" href=\"http://php.net/manual/en/function.", &prepared_function_name, ".php\">", &function_name, "</a></span>");
				} else {
					PHALCON_SCONCAT_SVS(&html, "<span class=\"error-function\">", &function_name, "</span>");
				}
			} else {
				PHALCON_SCONCAT_SVS(&html, "<span class=\"error-function\">", &function_name, "</span>");
			}
		}
	}

	/** 
	 * Check for arguments in the function
	 */
	if (phalcon_array_isset_fetch_str(&trace_args, trace, SL("args"))) {
		if (phalcon_fast_count_ev(&trace_args)) {
			array_init(&arguments);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(trace_args), argument) {
				zval dumped_argument = {}, span_argument = {};
				/** 
				 * Every argument is generated using _getVarDump
				 */
				PHALCON_CALL_METHODW(&dumped_argument, getThis(), "_getvardump", argument);
				PHALCON_CONCAT_SVS(&span_argument, "<span class=\"error-parameter\">", &dumped_argument, "</span>");

				/** 
				 * Append the HTML generated to the argument's list
				 */
				phalcon_array_append(&arguments, &span_argument, PH_COPY);
			} ZEND_HASH_FOREACH_END();

			/** 
			 * Join all the arguments
			 */
			phalcon_fast_join_str(&joined_arguments, SL(", "), &arguments);
			PHALCON_SCONCAT_SVS(&html, "(", &joined_arguments, ")");
		} else {
			phalcon_concat_self_str(&html, SL("()"));
		}
	}

	/** 
	 * When 'file' is present, it usually means the function is provided by the user
	 */
	if (phalcon_array_isset_fetch_str(&file, trace, SL("file"))) {
		phalcon_array_fetch_str(&line, trace, SL("line"), PH_NOISY);

		PHALCON_CALL_METHODW(&formatted_file, getThis(), "getfilelink", &file, &line, link_format);

		/** 
		 * Realpath to the file and its line using a special header
		 */
		PHALCON_SCONCAT_SVSVS(&html, "<br/><div class=\"error-file\">", &formatted_file, " (", &line, ")</div>");

		phalcon_return_property(&show_files, getThis(), SL("_showFiles"));

		/** 
		 * The developer can change if the files must be opened or not
		 */
		if (zend_is_true(&show_files)) {
			/** 
			 * Open the file to an array using 'file', this respects the openbase-dir directive
			 */
			PHALCON_CALL_FUNCTIONW(&lines, "file", &file);
			phalcon_fast_count(&number_lines, &lines);

			phalcon_return_property(&show_file_fragment, getThis(), SL("_showFileFragment"));

			/** 
			 * File fragments just show a piece of the file where the exception is located
			 */
			if (zend_is_true(&show_file_fragment)) {

				/** 
				 * Take lines back to the current exception's line
				 */
				phalcon_return_property(&before_context, getThis(), SL("_beforeContext"));

				phalcon_sub_function(&before_line, &line, &before_context);

				/** 
				 * Check for overflows
				 */
				if (PHALCON_LT_LONG(&before_line, 1)) {
					PHALCON_CPY_WRT_CTOR(&first_line, &PHALCON_GLOBAL(z_one));
				} else {
					PHALCON_CPY_WRT_CTOR(&first_line, &before_line);
				}

				/** 
				 * Take lines after the current exception's line
				 */
				phalcon_return_property(&after_context, getThis(), SL("_afterContext"));

				phalcon_add_function(&after_line, &line, &after_context);

				/** 
				 * Check for overflows
				 */
				if (PHALCON_GT(&after_line, &number_lines)) {
					PHALCON_CPY_WRT_CTOR(&last_line, &number_lines);
				} else {
					PHALCON_CPY_WRT_CTOR(&last_line, &after_line);
				}

				PHALCON_SCONCAT_SVSVSVS(&html, "<pre class='prettyprint highlight:", &first_line, ":", &line, " linenums:", &first_line, "'>");
			} else {
				PHALCON_CPY_WRT_CTOR(&first_line, &PHALCON_GLOBAL(z_one));
				PHALCON_CPY_WRT_CTOR(&last_line, &number_lines);
				PHALCON_SCONCAT_SVSVS(&html, "<pre class='prettyprint highlight:", &first_line, ":", &line, " linenums error-scroll'>");
			}

			ZVAL_STRING(&comment_pattern, "#\\*\\/$#");

			phalcon_return_static_property_ce(&charset, phalcon_debug_ce, SL("_charset"));

			ZVAL_STRING(&tab, "\t");
			ZVAL_STRING(&comment, "* /");

			PHALCON_CPY_WRT_CTOR(&i, &first_line);

			while (PHALCON_LE(&i, &last_line)) {
				zval line_position = {}, current_line = {}, trimmed = {}, is_comment = {}, spaced_current_line = {}, escaped_line = {};
				/**
				 * Current line in the file
				 */
				phalcon_sub_function(&line_position, &i, &PHALCON_GLOBAL(z_one));

				/** 
				 * Current line content in the piece of file
				 */
				phalcon_array_fetch(&current_line, &lines, &line_position, PH_NOISY);

				/** 
				 * File fragments are cleaned, removing tabs and comments
				 */
				if (zend_is_true(&show_file_fragment)) {
					if (PHALCON_IS_EQUAL(&i, &first_line)) {
						ZVAL_STR(&trimmed, phalcon_trim(&current_line, NULL, PHALCON_TRIM_RIGHT));

						RETURN_ON_FAILURE(phalcon_preg_match(&is_comment, &comment_pattern, &current_line, NULL));

						if (zend_is_true(&is_comment)) {
							PHALCON_STR_REPLACE(&spaced_current_line, &comment, &space, &current_line);
							PHALCON_CPY_WRT_CTOR(&current_line, &spaced_current_line);
						}
					}
				}

				/** 
				 * Print a non break space if the current line is a line break, this allows to show
				 * the html zebra properly
				 */
				if (PHALCON_IS_STRING(&current_line, "\n")) {
					phalcon_concat_self_str(&html, SL("&nbsp;\n"));
				} else {
					if (PHALCON_IS_STRING(&current_line, "\r\n")) {
						phalcon_concat_self_str(&html, SL("&nbsp;\n"));
					} else {
						PHALCON_STR_REPLACE(&spaced_current_line, &tab, &two_spaces, &current_line);

						phalcon_htmlentities(&escaped_line, &spaced_current_line, NULL, &charset);
						phalcon_concat_self(&html, &escaped_line);
					}
				}

				phalcon_increment(&i);
			}
			phalcon_concat_self_str(&html, SL("</pre>"));
		}
	}

	phalcon_concat_self_str(&html, SL("</td></tr>"));

	RETURN_CTORW(&html);
}

/**
 * Handles uncaught exceptions
 *
 * @param \Exception $exception
 * @return boolean
 */
PHP_METHOD(Phalcon_Debug, onUncaughtException){

	zval *exception, *is_active, message = {}, class_name = {}, css_sources = {}, escaped_message = {}, html = {}, version = {}, file = {}, line = {}, show_back_trace = {};
	zval *data_vars, trace = {}, *trace_item, *_REQUEST, *value = NULL, *_SERVER, files = {}, memory = {}, *data_var, js_sources = {}, formatted_file = {}, z_link_format = {};
	zend_bool ini_exists = 1;
	zend_class_entry *ce;
	zend_string *str_key;
	ulong idx;
	char* link_format;

	phalcon_fetch_params(0, 1, 0, &exception);
	PHALCON_VERIFY_CLASS_EX(exception, zend_exception_get_default(), phalcon_exception_ce, 0);

	/** 
	 * Cancel the output buffer if active
	 */
	if (phalcon_ob_get_level() > 0) {
		phalcon_ob_end_clean();
	}

	is_active = phalcon_read_static_property_ce(phalcon_debug_ce, SL("_isActive"));

	/** 
	 * Avoid that multiple exceptions being showed
	 */
	if (zend_is_true(is_active)) {
		PHALCON_CALL_METHODW(&message, exception, "getmessage");
		zend_print_zval(&message, 0);
	}

	/** 
	 * Globally block the debug component to avoid other exceptions must be shown
	 */
	zend_update_static_property_bool(phalcon_debug_ce, SL("_isActive"), 1);

	ce = Z_OBJCE_P(exception);
	ZVAL_NEW_STR(&class_name, ce->name);

	PHALCON_CALL_METHODW(&message, exception, "getmessage");

	/** 
	 * CSS static sources to style the error presentation
	 */
	PHALCON_CALL_METHODW(&css_sources, getThis(), "getcsssources");

	/** 
	 * Escape the exception's message avoiding possible XSS injections?
	 */
	PHALCON_CPY_WRT_CTOR(&escaped_message, &message);

	/** 
	 * Use the exception info as document's title
	 */
	PHALCON_CONCAT_SVSVS(&html, "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/><title>", &class_name, ": ", &escaped_message, "</title>");
	PHALCON_SCONCAT_VS(&html, &css_sources, "</head><body>");

	/** 
	 * Get the version link
	 */
	PHALCON_CALL_METHODW(&version, getThis(), "getversion");
	phalcon_concat_self(&html, &version);

	PHALCON_CALL_METHODW(&file, exception, "getfile");
	PHALCON_CALL_METHODW(&line, exception, "getline");

	link_format = zend_ini_string_ex(SL("xdebug.file_link_format"), 0, &ini_exists);
	if (!link_format || !ini_exists || !strlen(link_format)) {
		link_format = "file://%f#%l";
	}

	ZVAL_STRING(&z_link_format, link_format);

	PHALCON_CALL_METHODW(&formatted_file, getThis(), "getfilelink", &file, &line, &z_link_format);

	/** 
	 * Main exception info
	 */
	phalcon_concat_self_str(&html, SL("<div align=\"center\"><div class=\"error-main\">"));
	PHALCON_SCONCAT_SVSVS(&html, "<h1>", &class_name, ": ", &escaped_message, "</h1>");
	PHALCON_SCONCAT_SVSVS(&html, "<span class=\"error-file\">", &formatted_file, " (", &line, ")</span>");
	phalcon_concat_self_str(&html, SL("</div>"));

	phalcon_return_property(&show_back_trace, getThis(), SL("_showBackTrace"));

	/**
	 * Check if the developer wants to show the backtrace or not
	 */
	if (zend_is_true(&show_back_trace)) {
		data_vars = phalcon_read_property(getThis(), SL("_data"), PH_NOISY);

		/** 
		 * Create the tabs in the page
		 */
		phalcon_concat_self_str(&html, SL("<div class=\"error-info\"><div id=\"tabs\"><ul>"));
		phalcon_concat_self_str(&html, SL("<li><a href=\"#error-tabs-1\">Backtrace</a></li>"));
		phalcon_concat_self_str(&html, SL("<li><a href=\"#error-tabs-2\">Request</a></li>"));
		phalcon_concat_self_str(&html, SL("<li><a href=\"#error-tabs-3\">Server</a></li>"));
		phalcon_concat_self_str(&html, SL("<li><a href=\"#error-tabs-4\">Included Files</a></li>"));
		phalcon_concat_self_str(&html, SL("<li><a href=\"#error-tabs-5\">Memory</a></li>"));
		if (Z_TYPE_P(data_vars) == IS_ARRAY) { 
			phalcon_concat_self_str(&html, SL("<li><a href=\"#error-tabs-6\">Variables</a></li>"));
		}

		phalcon_concat_self_str(&html, SL("</ul>"));

		/** 
		 * Print backtrace
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-1\"><table cellspacing=\"0\" align=\"center\" width=\"100%\">"));

		PHALCON_CALL_METHODW(&trace, exception, "gettrace");

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
			PHALCON_CALL_METHODW(&html_item, getThis(), "showtraceitem", &tmp, trace_item, &z_link_format);
			phalcon_concat_self(&html, &html_item);
		} ZEND_HASH_FOREACH_END();

		phalcon_concat_self_str(&html, SL("</table></div>"));

		/** 
		 * Print _REQUEST superglobal
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-2\"><table cellspacing=\"0\" align=\"center\" class=\"superglobal-detail\">"));
		phalcon_concat_self_str(&html, SL("<tr><th>Key</th><th>Value</th></tr>"));
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
					PHALCON_CALL_METHODW(&joined_value, getThis(), "_getvardump", value);
					PHALCON_SCONCAT_SVSVS(&html, "<tr><td class=\"key\">", &tmp, "</td><td>", &joined_value, "</td></tr>");
				} else {
					PHALCON_SCONCAT_SVSVS(&html, "<tr><td class=\"key\">", &tmp, "</td><td>", value, "</td></tr>");
				}
			} ZEND_HASH_FOREACH_END();
		}

		phalcon_concat_self_str(&html, SL("</table></div>"));

		/** 
		 * Print _SERVER superglobal
		 */
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-3\"><table cellspacing=\"0\" align=\"center\" class=\"superglobal-detail\">"));
		phalcon_concat_self_str(&html, SL("<tr><th>Key</th><th>Value</th></tr>"));
		_SERVER = phalcon_get_global_str(SL("_SERVER"));

		if (Z_TYPE_P(_SERVER) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(_SERVER), idx, str_key, value) {
				zval tmp = {}, dumped_argument = {};
				if (str_key) {
					ZVAL_STR(&tmp, str_key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}
				PHALCON_CALL_METHODW(&dumped_argument, getThis(), "_getvardump", value);
				PHALCON_SCONCAT_SVSVS(&html, "<tr><td class=\"key\">", &tmp, "</td><td>", &dumped_argument, "</td></tr>");
			} ZEND_HASH_FOREACH_END();
		}

		phalcon_concat_self_str(&html, SL("</table></div>"));

		/** 
		 * Show included files
		 */
		PHALCON_CALL_FUNCTIONW(&files, "get_included_files");

		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-4\"><table cellspacing=\"0\" align=\"center\" class=\"superglobal-detail\">"));
		phalcon_concat_self_str(&html, SL("<tr><th>#</th><th>Path</th></tr>"));

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(files), idx, str_key, value) {
			zval tmp = {};
			if (str_key) {
				ZVAL_STR(&tmp, str_key);
			} else {
				ZVAL_LONG(&tmp, idx);
			}

			PHALCON_SCONCAT_SVSVS(&html, "<tr><td>", &tmp, "</th><td>", value, "</td></tr>");
		} ZEND_HASH_FOREACH_END();

		phalcon_concat_self_str(&html, SL("</table></div>"));

		/** 
		 * Memory usage
		 */
		ZVAL_LONG(&memory, zend_memory_usage(1));
		phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-5\"><table cellspacing=\"0\" align=\"center\" class=\"superglobal-detail\">"));
		PHALCON_SCONCAT_SVS(&html, "<tr><th colspan=\"2\">Memory</th></tr><tr><td>Usage</td><td>", &memory, "</td></tr>");
		phalcon_concat_self_str(&html, SL("</table></div>"));

		/** 
		 * Print extra variables passed to the component
		 */
		if (Z_TYPE_P(data_vars) == IS_ARRAY) { 
			phalcon_concat_self_str(&html, SL("<div id=\"error-tabs-6\"><table cellspacing=\"0\" align=\"center\" class=\"superglobal-detail\">"));
			phalcon_concat_self_str(&html, SL("<tr><th>Key</th><th>Value</th></tr>"));

			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data_vars), idx, str_key, data_var) {
				zval tmp = {}, variable = {}, dumped_argument = {};
				if (str_key) {
					ZVAL_STR(&tmp, str_key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}

				phalcon_array_fetch_long(&variable, data_var, 0, PH_NOISY);

				PHALCON_CALL_METHODW(&dumped_argument, getThis(), "_getvardump", &variable);
				PHALCON_SCONCAT_SVSVS(&html, "<tr><td class=\"key\">", &tmp, "</td><td>", &dumped_argument, "</td></tr>");
			} ZEND_HASH_FOREACH_END();

			phalcon_concat_self_str(&html, SL("</table></div>"));
		}

		phalcon_concat_self_str(&html, SL("</div>"));
	}

	/** 
	 * Get Javascript sources
	 */
	PHALCON_CALL_METHODW(&js_sources, getThis(), "getjssources");
	PHALCON_SCONCAT_VS(&html, &js_sources, "</div></body></html>");

	/** 
	 * Print the HTML, @TODO, add an option to store the html
	 */
	zend_print_zval(&html, 0);

	/** 
	 * Unlock the exception renderer
	 */
	zend_update_static_property_bool(phalcon_debug_ce, SL("_isActive"), 0);
	RETURN_TRUE;
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
			!phalcon_array_isset_fetch_str(&previous, context, SL("e")) ||
			Z_TYPE_P(&previous) != IS_OBJECT ||
			!instanceof_function_ex(Z_OBJCE_P(&previous), zend_exception_get_default(), 1)
		) {
			ZVAL_NULL(&previous);
		}
	} else {
		ZVAL_NULL(&previous);
	}

	default_exception_ce = zend_get_error_exception();

	object_init_ex(&exception, default_exception_ce);

	PHALCON_CALL_METHODW(NULL, &exception, "__construct", message, &PHALCON_GLOBAL(z_zero), severity, file, line, &previous);

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

	PHALCON_CALL_FUNCTIONW(&error, "error_get_last");

	if (
		!phalcon_array_isset_fetch_str(&message, &error, SL("message")) ||
		!phalcon_array_isset_fetch_str(&type, &error, SL("type")) ||
		!phalcon_array_isset_fetch_str(&file, &error, SL("file")) ||
		!phalcon_array_isset_fetch_str(&line, &error, SL("line"))
		
	) {
		default_exception_ce = zend_get_error_exception();

		object_init_ex(&exception, default_exception_ce);

		PHALCON_CALL_METHODW(NULL, &exception, "__construct", &message, &PHALCON_GLOBAL(z_zero), &type, &file, &line);

		zend_throw_exception_object(&exception);
	}
}

/**
 * Returns the character set used to display the HTML
 *
 * @brief string \Phalcon\Debug::getCharset(void)
 * @return string
 */
PHP_METHOD(Phalcon_Debug, getCharset) {
	zval *charset = phalcon_read_static_property_ce(phalcon_debug_ce, SL("_charset"));
	RETURN_ZVAL(charset, 1, 0);
}

/**
 * Sets the character set used to display the HTML
 *
 * @brief \Phalcon\Debug \Phalcon\Debug::setCharset(string $charset)
 * @param string $charset
 * @return \Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, setCharset) {

	zval *charset;

	phalcon_fetch_params(0, 1, 0, &charset);
	PHALCON_ENSURE_IS_STRING(charset);

	phalcon_update_static_property_ce(phalcon_debug_ce, SL("_charset"), charset);
	RETURN_THISW();
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

	phalcon_update_property_this(getThis(), SL("_beforeContext"), lines);
	RETURN_THISW();
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

	phalcon_update_property_this(getThis(), SL("_afterContext"), lines);
	RETURN_THISW();
}

/**
 * Enable simple debug mode
 *
 * @return Phalcon\Debug
 */
PHP_METHOD(Phalcon_Debug, enable){

	PHALCON_GLOBAL(debug).enable_debug = 1;
}
