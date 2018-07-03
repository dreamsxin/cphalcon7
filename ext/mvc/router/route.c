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

#include "mvc/router/route.h"
#include "mvc/../router.h"
#include "mvc/router/routeinterface.h"
#include "mvc/router/exception.h"
#include "mvc/router/group.h"
#include "debug.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/framework/router.h"
#include "kernel/hash.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Router\Route
 *
 * This class represents every route added to the router
 */
zend_class_entry *phalcon_mvc_router_route_ce;

PHP_METHOD(Phalcon_Mvc_Router_Route, __construct);
PHP_METHOD(Phalcon_Mvc_Router_Route, compilePattern);
PHP_METHOD(Phalcon_Mvc_Router_Route, via);
PHP_METHOD(Phalcon_Mvc_Router_Route, reConfigure);
PHP_METHOD(Phalcon_Mvc_Router_Route, getName);
PHP_METHOD(Phalcon_Mvc_Router_Route, setName);
PHP_METHOD(Phalcon_Mvc_Router_Route, beforeMatch);
PHP_METHOD(Phalcon_Mvc_Router_Route, getBeforeMatch);
PHP_METHOD(Phalcon_Mvc_Router_Route, getRouteId);
PHP_METHOD(Phalcon_Mvc_Router_Route, getPattern);
PHP_METHOD(Phalcon_Mvc_Router_Route, getCompiledPattern);
PHP_METHOD(Phalcon_Mvc_Router_Route, getPaths);
PHP_METHOD(Phalcon_Mvc_Router_Route, getReversedPaths);
PHP_METHOD(Phalcon_Mvc_Router_Route, setHttpMethods);
PHP_METHOD(Phalcon_Mvc_Router_Route, getHttpMethods);
PHP_METHOD(Phalcon_Mvc_Router_Route, setPrefix);
PHP_METHOD(Phalcon_Mvc_Router_Route, getPrefix);
PHP_METHOD(Phalcon_Mvc_Router_Route, setHostname);
PHP_METHOD(Phalcon_Mvc_Router_Route, getHostname);
PHP_METHOD(Phalcon_Mvc_Router_Route, setGroup);
PHP_METHOD(Phalcon_Mvc_Router_Route, getGroup);
PHP_METHOD(Phalcon_Mvc_Router_Route, convert);
PHP_METHOD(Phalcon_Mvc_Router_Route, getConverters);
PHP_METHOD(Phalcon_Mvc_Router_Route, reset);
PHP_METHOD(Phalcon_Mvc_Router_Route, setDefaults);
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaults);
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaultNamespace);
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaultModule);
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaultController);
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaultAction);
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaultParams);
PHP_METHOD(Phalcon_Mvc_Router_Route, setUrlGenerator);
PHP_METHOD(Phalcon_Mvc_Router_Route, getUrlGenerator);
PHP_METHOD(Phalcon_Mvc_Router_Route, setCaseSensitive);
PHP_METHOD(Phalcon_Mvc_Router_Route, getCaseSensitive);
PHP_METHOD(Phalcon_Mvc_Router_Route, setMode);
PHP_METHOD(Phalcon_Mvc_Router_Route, getMode);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_route___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	ZEND_ARG_INFO(0, paths)
	ZEND_ARG_INFO(0, httpMethods)
	ZEND_ARG_TYPE_INFO(0, regex, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_route_setgroup, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, group, Phalcon\\Mvc\\Router\\Group, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_route_beforematch, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, callback, Closure, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_route_setprefix, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, prefix, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_route_sethostname, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_route_convert, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, converter, Closure, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_route_seturlgenerator, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, handler, Closure, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_route_setcasesensitive, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, caseSensitive, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_router_route_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Router_Route, __construct, arginfo_phalcon_mvc_router_route___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Router_Route, compilePattern, arginfo_phalcon_mvc_router_routeinterface_compilepattern, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, via, arginfo_phalcon_mvc_router_routeinterface_via, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, reConfigure, arginfo_phalcon_mvc_router_routeinterface_reconfigure, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, setName, arginfo_phalcon_mvc_router_routeinterface_setname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, beforeMatch, arginfo_phalcon_mvc_router_route_beforematch, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getBeforeMatch, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getRouteId, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getPattern, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getCompiledPattern, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getPaths, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getReversedPaths, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, setHttpMethods, arginfo_phalcon_mvc_router_routeinterface_sethttpmethods, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getHttpMethods, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, setPrefix, arginfo_phalcon_mvc_router_route_setprefix, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getPrefix, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, setHostname, arginfo_phalcon_mvc_router_route_sethostname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getHostname, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, setGroup, arginfo_phalcon_mvc_router_route_setgroup, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getGroup, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, convert, arginfo_phalcon_mvc_router_route_convert, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getConverters, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, reset, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Router_Route, setDefaults, arginfo_phalcon_mvc_router_routeinterface_setdefaults, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getDefaults, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getDefaultNamespace, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getDefaultModule, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getDefaultController, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getDefaultAction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getDefaultParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, setUrlGenerator, arginfo_phalcon_mvc_router_route_seturlgenerator, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getUrlGenerator, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, setCaseSensitive, arginfo_phalcon_mvc_router_route_setcasesensitive, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getCaseSensitive, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, setMode, arginfo_phalcon_mvc_router_routeinterface_setmode, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Route, getMode, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Router\Route initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Router_Route){

	PHALCON_REGISTER_CLASS(Phalcon\\Mvc\\Router, Route, mvc_router_route, phalcon_mvc_router_route_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_pattern"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_compiledPattern"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_paths"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_methods"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_prefix"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_hostname"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_converters"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_id"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_name"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_beforeMatch"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_group"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_router_route_ce, SL("_uniqueId"), 0, ZEND_ACC_STATIC|ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_defaultNamespace"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_defaultModule"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_defaultController"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_defaultAction"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_defaultParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_urlGenerator"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_route_ce, SL("_caseSensitive"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_router_route_ce, SL("_mode"), PHALCON_ROUTER_MODE_DEFAULT, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_router_route_ce, 1, phalcon_mvc_router_routeinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Router\Route constructor
 *
 * @param string $pattern
 * @param array $paths
 * @param array|string $httpMethods
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, __construct){

	zval *pattern, *paths = NULL, *http_methods = NULL, *regex = NULL, unique_id = {};

	phalcon_fetch_params(0, 1, 3, &pattern, &paths, &http_methods, &regex);

	if (!paths) {
		paths = &PHALCON_GLOBAL(z_null);
	}

	if (!http_methods) {
		http_methods = &PHALCON_GLOBAL(z_null);
	}

	if (!regex) {
		regex = &PHALCON_GLOBAL(z_null);
	}

	/**
	 * Configure the route (extract parameters, paths, etc)
	 */
	PHALCON_CALL_METHOD(NULL, getThis(), "reconfigure", pattern, paths, regex);

	/**
	 * Update the HTTP method constraints
	 */
	phalcon_update_property(getThis(), SL("_methods"), http_methods);

	/**
	 * Get the unique Id from the static member _uniqueId
	 */
	phalcon_read_static_property_ce(&unique_id, phalcon_mvc_router_route_ce, SL("_uniqueId"), PH_READONLY);

	if (Z_TYPE(unique_id) == IS_NULL) {
		ZVAL_LONG(&unique_id, 0);
	}

	phalcon_update_property(getThis(), SL("_id"), &unique_id);

	/* increment_function() will increment the value of the static property as well */
	increment_function(&unique_id);
	phalcon_update_static_property_ce(phalcon_mvc_router_route_ce, SL("_uniqueId"), &unique_id);
}

/**
 * Replaces placeholders from pattern returning a valid PCRE regular expression
 *
 * @param string $pattern
 * @param array $paths
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, compilePattern){

	zval *pattern, *regex = NULL, compiled_pattern = {}, id_pattern = {}, wildcard = {}, pattern_copy = {}, params_pattern = {};

	phalcon_fetch_params(0, 1, 1, &pattern, &regex);

	ZVAL_DUP(&compiled_pattern, pattern);

	if (!regex) {
		regex = &PHALCON_GLOBAL(z_null);
	}
	/**
	 * If a pattern contains ':', maybe there are placeholders to replace
	 */
	if (phalcon_memnstr_str(pattern, SL(":"))) {

		/**
		 * This is a pattern for valid identifiers
		 */
		ZVAL_STRING(&id_pattern, "/([\\w0-9\\_\\-]+)");

		/**
		 * Replace the module part
		 */
		if (phalcon_memnstr_str(pattern, SL("/:module"))) {
			ZVAL_STRING(&wildcard, "/:module");

			ZVAL_COPY_VALUE(&pattern_copy, &compiled_pattern);

			if (Z_TYPE_P(regex) == IS_ARRAY && phalcon_array_isset_fetch_str(&params_pattern, regex, SL("/:module"), PH_READONLY)) {
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &params_pattern, &pattern_copy);
			} else {
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &id_pattern, &pattern_copy);
			}
			zval_ptr_dtor(&wildcard);
			zval_ptr_dtor(&pattern_copy);
		}

		/**
		 * Replace the controller placeholder
		 */
		if (phalcon_memnstr_str(pattern, SL("/:controller"))) {
			ZVAL_STRING(&wildcard, "/:controller");

			ZVAL_COPY_VALUE(&pattern_copy, &compiled_pattern);

			if (Z_TYPE_P(regex) == IS_ARRAY && phalcon_array_isset_fetch_str(&params_pattern, regex, SL("/:controller"), PH_READONLY)) {
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &params_pattern, &pattern_copy);
			} else {
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &id_pattern, &pattern_copy);
			}
			zval_ptr_dtor(&wildcard);
			zval_ptr_dtor(&pattern_copy);
		}

		/**
		 * Replace the namespace placeholder
		 */
		if (phalcon_memnstr_str(pattern, SL("/:namespace"))) {
			ZVAL_STRING(&wildcard, "/:namespace");

			ZVAL_COPY_VALUE(&pattern_copy, &compiled_pattern);

			if (Z_TYPE_P(regex) == IS_ARRAY && phalcon_array_isset_fetch_str(&params_pattern, regex, SL("/:namespace"), PH_READONLY)) {
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &params_pattern, &pattern_copy);
			} else {
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &id_pattern, &pattern_copy);
			}
			zval_ptr_dtor(&wildcard);
			zval_ptr_dtor(&pattern_copy);
		}

		/**
		 * Replace the action placeholder
		 */
		if (phalcon_memnstr_str(pattern, SL("/:action"))) {
			ZVAL_STRING(&wildcard, "/:action");

			ZVAL_COPY_VALUE(&pattern_copy, &compiled_pattern);

			if (Z_TYPE_P(regex) == IS_ARRAY && phalcon_array_isset_fetch_str(&params_pattern, regex, SL("/:action"), PH_READONLY)) {
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &params_pattern, &pattern_copy);
			} else {
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &id_pattern, &pattern_copy);
			}
			zval_ptr_dtor(&wildcard);
			zval_ptr_dtor(&pattern_copy);
		}

		/**
		 * Replace the params placeholder
		 */
		if (phalcon_memnstr_str(pattern, SL("/:params"))) {
			ZVAL_STRING(&wildcard, "/:params");

			ZVAL_COPY_VALUE(&pattern_copy, &compiled_pattern);

			if (Z_TYPE_P(regex) == IS_ARRAY && phalcon_array_isset_fetch_str(&params_pattern, regex, SL("/:params"), PH_READONLY)) {
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &params_pattern, &pattern_copy);
			} else {
				zval_ptr_dtor(&id_pattern);
				ZVAL_STRING(&id_pattern, "(/.*)*");
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &id_pattern, &pattern_copy);
			}
			zval_ptr_dtor(&wildcard);
			zval_ptr_dtor(&pattern_copy);
		}

		/**
		 * Replace the int placeholder
		 */
		if (phalcon_memnstr_str(pattern, SL("/:int"))) {
			ZVAL_STRING(&wildcard, "/:int");

			ZVAL_COPY_VALUE(&pattern_copy, &compiled_pattern);

			if (Z_TYPE_P(regex) == IS_ARRAY && phalcon_array_isset_fetch_str(&params_pattern, regex, SL("/:int"), PH_READONLY)) {
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &params_pattern, &pattern_copy);
			} else {
				zval_ptr_dtor(&id_pattern);
				ZVAL_STRING(&id_pattern, "/([0-9]+)");
				PHALCON_STR_REPLACE(&compiled_pattern, &wildcard, &id_pattern, &pattern_copy);
			}
			zval_ptr_dtor(&wildcard);
			zval_ptr_dtor(&pattern_copy);
		}

		zval_ptr_dtor(&id_pattern);
	}

	/**
	 * Check if the pattern has parantheses in order to add the regex delimiters
	 */
	if (phalcon_memnstr_str(&compiled_pattern, SL("("))) {
		PHALCON_CONCAT_SVS(return_value, "#^", &compiled_pattern, "$#u");
		zval_ptr_dtor(&compiled_pattern);
		return;
	}

	/**
	 * Square brackets are also checked
	 */
	if (phalcon_memnstr_str(&compiled_pattern, SL("["))) {
		PHALCON_CONCAT_SVS(return_value, "#^", &compiled_pattern, "$#u");
		zval_ptr_dtor(&compiled_pattern);
		return;
	}

	/**
	 * Square brackets are also checked
	 */
	if (phalcon_memnstr_str(&compiled_pattern, SL("{"))) {
		PHALCON_CONCAT_SVS(return_value, "#^", &compiled_pattern, "$#u");
		zval_ptr_dtor(&compiled_pattern);
		return;
	}

	RETURN_CTOR(&compiled_pattern);
}

/**
 * Set one or more HTTP methods that constraint the matching of the route
 *
 *<code>
 * $route->via('GET');
 * $route->via(array('GET', 'POST'));
 *</code>
 *
 * @param string|array $httpMethods
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, via){

	zval *http_methods;

	phalcon_fetch_params(0, 1, 0, &http_methods);

	phalcon_update_property(getThis(), SL("_methods"), http_methods);
	RETURN_THIS();
}

/**
 * Reconfigure the route adding a new pattern and a set of paths
 *
 * @param string $pattern
 * @param string|array $paths
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, reConfigure){

	zval *pattern, *paths = NULL, *regex = NULL, module_name = {}, controller_name = {}, action_name = {}, parts = {}, number_parts = {}, route_paths = {};
	zval real_class_name = {}, namespace_name = {}, lower_name = {}, pcre_pattern = {}, compiled_pattern = {}, debug_message = {};

	phalcon_fetch_params(0, 1, 2, &pattern, &paths, &regex);

	if (!paths) {
		paths = &PHALCON_GLOBAL(z_null);
	}

	if (!regex) {
		regex = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(paths) != IS_NULL) {
		if (Z_TYPE_P(paths) == IS_STRING) {
			/**
			 * Explode the short paths using the :: separator
			 */
			phalcon_fast_explode_str(&parts, SL("::"), paths);
			phalcon_fast_count(&number_parts, &parts);

			/**
			 * Create the array paths dynamically
			 */

			switch (phalcon_get_intval(&number_parts)) {

				case 3:
					phalcon_array_fetch_long(&module_name, &parts, 0, PH_NOISY | PH_READONLY);
					phalcon_array_fetch_long(&controller_name, &parts, 1, PH_NOISY | PH_READONLY);
					phalcon_array_fetch_long(&action_name, &parts, 2, PH_NOISY | PH_READONLY);
					break;

				case 2:
					phalcon_array_fetch_long(&controller_name, &parts, 0, PH_NOISY | PH_READONLY);
					phalcon_array_fetch_long(&action_name, &parts, 1, PH_NOISY | PH_READONLY);
					break;

				case 1:
					phalcon_array_fetch_long(&controller_name, &parts, 0, PH_NOISY|PH_READONLY);
					break;

			}

			array_init(&route_paths);

			/**
			 * Process module name
			 */
			if (PHALCON_IS_NOT_EMPTY_STRING(&module_name)) {
				phalcon_array_update_str(&route_paths, SL("module"), &module_name, PH_COPY);
			}

			/**
			 * Process controller name
			 */
			if (PHALCON_IS_NOT_EMPTY_STRING(&controller_name)) {

				/**
				 * Check if we need to obtain the namespace
				 */
				if (phalcon_memnstr_str(&controller_name, SL("\\"))) {

					/**
					 * Extract the real class name from the namespaced class
					 */
					phalcon_get_class_ns(&real_class_name, &controller_name, 0);

					/**
					 * Extract the namespace from the namespaced class
					 */
					phalcon_get_ns_class(&namespace_name, &controller_name, 0);

					/**
					 * Update the namespace
					 */
					if (PHALCON_IS_NOT_EMPTY_STRING(&namespace_name)) {
						phalcon_array_update_str(&route_paths, SL("namespace"), &namespace_name, 0);
					}
				} else {
					ZVAL_COPY(&real_class_name, &controller_name);
				}

				/**
				 * Always pass the controller to lowercase
				 */
				phalcon_uncamelize(&lower_name, &real_class_name);
				zval_ptr_dtor(&real_class_name);

				/**
				 * Update the controller path
				 */
				phalcon_array_update_str(&route_paths, SL("controller"), &lower_name, 0);
			}

			/**
			 * Process action name
			 */
			if (PHALCON_IS_NOT_EMPTY_STRING(&action_name)) {
				phalcon_array_update_str(&route_paths, SL("action"), &action_name, PH_COPY);
			}
			zval_ptr_dtor(&parts);
		} else if (Z_TYPE_P(paths) == IS_ARRAY) {
			if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				ZVAL_STRING(&debug_message, "Add Route paths: ");
				PHALCON_DEBUG_LOG(&debug_message);
				zval_ptr_dtor(&debug_message);
				PHALCON_DEBUG_LOG(paths);
			}

			ZVAL_DUP(&route_paths, paths);
			if (phalcon_array_isset_fetch_str(&controller_name, &route_paths, SL("controller"), PH_READONLY)) {
				if (Z_TYPE(controller_name) == IS_STRING && !phalcon_is_numeric_ex(&controller_name)) {
					phalcon_uncamelize(&lower_name, &controller_name);
					phalcon_array_update_str(&route_paths, SL("controller"), &lower_name, 0);
				}
			}
		} else {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "The route contains invalid paths");
			return;
		}
	} else {
		array_init(&route_paths);
	}

	/**
	 * If the route starts with '#' we assume that it is a regular expression
	 */
	if (!phalcon_start_with_str(pattern, SL("#"))) {
		/**
		 * Transform the route's pattern to a regular expression
		 */
		PHALCON_CALL_METHOD(&pcre_pattern, getThis(), "compilepattern", pattern, regex);
		if (phalcon_memnstr_str(pattern, SL("{"))) {
			/**
			 * The route has named parameters so we need to extract them
			 */
			phalcon_extract_named_params(&compiled_pattern, &pcre_pattern, &route_paths);
		} else {
			ZVAL_COPY(&compiled_pattern, &pcre_pattern);
		}
		zval_ptr_dtor(&pcre_pattern);
	} else {
		ZVAL_COPY(&compiled_pattern, pattern);
	}

	/**
	 * Update the original pattern
	 */
	phalcon_update_property(getThis(), SL("_pattern"), pattern);

	/**
	 * Update the compiled pattern
	 */
	phalcon_update_property(getThis(), SL("_compiledPattern"), &compiled_pattern);
	zval_ptr_dtor(&compiled_pattern);

	/**
	 * Update the route's paths
	 */
	phalcon_update_property(getThis(), SL("_paths"), &route_paths);
	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		ZVAL_STRING(&debug_message, "Update Route paths: ");
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
		PHALCON_DEBUG_LOG(&route_paths);
	}
	zval_ptr_dtor(&route_paths);
}

/**
 * Returns the route's name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getName){


	RETURN_MEMBER(getThis(), "_name");
}

/**
 * Sets the route's name
 *
 *<code>
 * $router->add('/about', array(
 *     'controller' => 'about'
 * ))->setName('about');
 *</code>
 *
 * @param string $name
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, setName){

	zval *name;

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_update_property(getThis(), SL("_name"), name);
	RETURN_THIS();
}

/**
 * Sets a callback that is called if the route is matched.
 * The developer can implement any arbitrary conditions here
 * If the callback returns false the route is treaded as not matched
 *
 * @param callback $callback
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, beforeMatch){

	zval *callback;

	phalcon_fetch_params(0, 1, 0, &callback);

	phalcon_update_property(getThis(), SL("_beforeMatch"), callback);
	RETURN_THIS();
}

/**
 * Returns the 'before match' callback if any
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getBeforeMatch){


	RETURN_MEMBER(getThis(), "_beforeMatch");
}

/**
 * Returns the route's id
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getRouteId){


	RETURN_MEMBER(getThis(), "_id");
}

/**
 * Returns the route's pattern
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getPattern){


	RETURN_MEMBER(getThis(), "_pattern");
}

/**
 * Returns the route's compiled pattern
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getCompiledPattern){


	RETURN_MEMBER(getThis(), "_compiledPattern");
}

/**
 * Returns the paths
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getPaths){


	RETURN_MEMBER(getThis(), "_paths");
}

/**
 * Returns the paths using positions as keys and names as values
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getReversedPaths){

	zval paths = {}, *position;
	zend_string *str_key;
	ulong idx;

	phalcon_read_property(&paths, getThis(), SL("_paths"), PH_NOISY|PH_READONLY);

	array_init(return_value);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(paths), idx, str_key, position) {
		zval path = {};
		if (str_key) {
			ZVAL_STR(&path, str_key);
		} else {
			ZVAL_LONG(&path, idx);
		}
		phalcon_array_update(return_value, position, &path, PH_COPY);
	} ZEND_HASH_FOREACH_END();
}

/**
 * Sets a set of HTTP methods that constraint the matching of the route (alias of via)
 *
 *<code>
 * $route->setHttpMethods('GET');
 * $route->setHttpMethods(array('GET', 'POST'));
 *</code>
 *
 * @param string|array $httpMethods
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, setHttpMethods){

	zval *http_methods;

	phalcon_fetch_params(0, 1, 0, &http_methods);

	phalcon_update_property(getThis(), SL("_methods"), http_methods);
	RETURN_THIS();
}

/**
 * Returns the HTTP methods that constraint matching the route
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getHttpMethods){


	RETURN_MEMBER(getThis(), "_methods");
}

/**
 * Set a common uri prefix for the route
 *
 * @param string $prefix
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, setPrefix){

	zval *prefix;

	phalcon_fetch_params(0, 1, 0, &prefix);

	phalcon_update_property(getThis(), SL("_prefix"), prefix);
	RETURN_THIS();
}

/**
 * Returns the common prefix for the route
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getPrefix){


	RETURN_MEMBER(getThis(), "_prefix");
}

/**
 * Sets a hostname restriction to the route
 *
 *<code>
 * $route->setHostname('localhost');
 *</code>
 *
 * @param string|array $httpMethods
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, setHostname){

	zval *hostname;

	phalcon_fetch_params(0, 1, 0, &hostname);

	phalcon_update_property(getThis(), SL("_hostname"), hostname);
	RETURN_THIS();
}

/**
 * Returns the hostname restriction if any
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getHostname){


	RETURN_MEMBER(getThis(), "_hostname");
}

/**
 * Sets the group associated with the route
 *
 * @param Phalcon\Mvc\Router\Group $group
 * @return Phalcon\Mvc\RouteInterface
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, setGroup) {

	zval *group;

	phalcon_fetch_params(0, 1, 0, &group);
	PHALCON_VERIFY_CLASS_EX(group, phalcon_mvc_router_group_ce, phalcon_mvc_router_exception_ce);

	phalcon_update_property(getThis(), SL("_group"), group);
	RETURN_THIS();
}

/**
 * Returns the group associated with the route
 *
 * @return Phalcon\Mvc\Router\Group|null
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getGroup) {

	RETURN_MEMBER(getThis(), "_group");
}

/**
 * Adds a converter to perform an additional transformation for certain parameter
 *
 * @param string $name
 * @param callable $converter
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, convert){

	zval *name, *converter, callback = {};

	phalcon_fetch_params(0, 2, 0, &name, &converter);

	PHALCON_CALL_CE_STATIC(&callback, zend_ce_closure, "bind", converter, getThis());
	phalcon_update_property_array(getThis(), SL("_converters"), name, &callback);
	zval_ptr_dtor(&callback);
	RETURN_THIS();
}

/**
 * Returns the router converter
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getConverters){


	RETURN_MEMBER(getThis(), "_converters");
}

/**
 * Resets the internal route id generator
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, reset){

	zend_update_static_property_long(phalcon_mvc_router_route_ce, SL("_uniqueId"), 0);
}

/**
 * Sets an array of default paths.
 *
 * @param array $defaults
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, setDefaults){

	zval *defaults, namespace_name = {}, module_name = {}, controller_name = {}, action_name = {}, params = {};

	phalcon_fetch_params(0, 1, 0, &defaults);

	/* Set the default namespace */
	if (phalcon_array_isset_fetch_str(&namespace_name, defaults, SL("namespace"), PH_READONLY)) {
		if (Z_TYPE(namespace_name) != IS_STRING) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "Namespace name must be an string");
			return;
		}
		phalcon_update_property(getThis(), SL("_defaultNamespace"), &namespace_name);
	}

	/* Set the default module */
	if (phalcon_array_isset_fetch_str(&module_name, defaults, SL("module"), PH_READONLY)) {
		if (Z_TYPE(module_name) != IS_STRING) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "Module name must be an string");
			return;
		}
		phalcon_update_property(getThis(), SL("_defaultModule"), &module_name);
	}

	/* Set the default controller */
	if (phalcon_array_isset_fetch_str(&controller_name, defaults, SL("controller"), PH_READONLY)) {
		if (Z_TYPE(controller_name) != IS_STRING) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "Controller name must be an string");
			return;
		}
		phalcon_update_property(getThis(), SL("_defaultController"), &controller_name);
	}

	/* Set the default action */
	if (phalcon_array_isset_fetch_str(&action_name, defaults, SL("action"), PH_READONLY)) {
		if (Z_TYPE(action_name) != IS_STRING) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "Action name must be an string");
			return;
		}
		phalcon_update_property(getThis(), SL("_defaultAction"), &action_name);
	}

	/* Set default parameters */
	if (phalcon_array_isset_fetch_str(&params, defaults, SL("params"), PH_READONLY)) {
		if (Z_TYPE(params) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "Params must be an array");
			return;
		}
		phalcon_update_property(getThis(), SL("_defaultParams"), &params);
	}

	RETURN_THIS();
}

/**
 * Returns an array of default parameters
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaults){

	zval namespace_name = {}, module_name = {}, controller_name = {}, action_name = {}, params = {};

	 phalcon_read_property(&namespace_name, getThis(), SL("_defaultNamespace"), PH_NOISY|PH_READONLY);
	 phalcon_read_property(&module_name, getThis(), SL("_defaultModule"), PH_NOISY|PH_READONLY);
	 phalcon_read_property(&controller_name, getThis(), SL("_defaultController"), PH_NOISY|PH_READONLY);
	 phalcon_read_property(&action_name, getThis(), SL("_defaultAction"), PH_NOISY|PH_READONLY);
	 phalcon_read_property(&params, getThis(), SL("_defaultParams"), PH_NOISY|PH_READONLY);

	array_init_size(return_value, 5);

	phalcon_array_update_string(return_value, IS(namespace),  &namespace_name,  PH_COPY);
	phalcon_array_update_string(return_value, IS(module),     &module_name,     PH_COPY);
	phalcon_array_update_string(return_value, IS(controller), &controller_name, PH_COPY);
	phalcon_array_update_string(return_value, IS(action),     &action_name,     PH_COPY);
	phalcon_array_update_string(return_value, IS(params),     &params,          PH_COPY);
}

/**
 * Returns the name of the default namespace
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaultNamespace){

	RETURN_MEMBER(getThis(), "_defaultNamespace");
}

/**
 * Returns the name of the default module
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaultModule){

	RETURN_MEMBER(getThis(), "_defaultModule");
}

/**
 * Returns the default controller name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaultController){

	RETURN_MEMBER(getThis(), "_defaultController");
}

/**
 * Returns the default action name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaultAction){

	RETURN_MEMBER(getThis(), "_defaultAction");
}

/**
 * Returns the default params
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getDefaultParams){

	RETURN_MEMBER(getThis(), "_defaultParams");
}

/**
 * Sets the Url Generator
 * @param callable $generator
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, setUrlGenerator){

	zval *generator, callback = {};

	phalcon_fetch_params(0, 1, 0, &generator);

	PHALCON_CALL_CE_STATIC(&callback, zend_ce_closure, "bind", generator, getThis());
	phalcon_update_property(getThis(), SL("_urlGenerator"), &callback);

	RETURN_THIS();
}

/**
 * Returns the Url Generator
 *
 * @return callable
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getUrlGenerator){

	RETURN_MEMBER(getThis(), "_urlGenerator");
}

/**
 * Sets the case sensitive
 * @param boolean $caseSensitive
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, setCaseSensitive){

	zval *case_sensitive;

	phalcon_fetch_params(0, 1, 0, &case_sensitive);

	phalcon_update_property_bool(getThis(), SL("_caseSensitive"), zend_is_true(case_sensitive));

	RETURN_THIS();
}

/**
 * Returns the case sensitive
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getCaseSensitive){

	RETURN_MEMBER(getThis(), "_caseSensitive");
}

/**
 * Sets the mode
 *
 * @param int $mode
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, setMode){

	zval *mode;

	phalcon_fetch_params(0, 1, 0, &mode);

	phalcon_update_property(getThis(), SL("_mode"), mode);
	RETURN_THIS();
}

/**
 * Gets the mode
 *
 * @param int|null $mode
 */
PHP_METHOD(Phalcon_Mvc_Router_Route, getMode){


	RETURN_MEMBER(getThis(), "_mode");
}
