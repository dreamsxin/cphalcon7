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

#include "mvc/router.h"
#include "mvc/../router.h"
#include "mvc/routerinterface.h"
#include "mvc/../routerinterface.h"
#include "mvc/router/exception.h"
#include "mvc/router/group.h"
#include "mvc/router/route.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "http/requestinterface.h"
#include "debug.h"

#include <main/SAPI.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/exception.h"
#include "kernel/concat.h"
#include "kernel/file.h"
#include "kernel/hash.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Router
 *
 * <p>Phalcon\Mvc\Router is the standard framework router. Routing is the
 * process of taking a URI endpoint (that part of the URI which comes after the base URL) and
 * decomposing it into parameters to determine which module, controller, and
 * action of that controller should receive the request</p>
 *
 *<code>
 *
 *	$router = new Phalcon\Mvc\Router();
 *
 *  $router->add(
 *		"/documentation/{chapter}/{name}.{type:[a-z]+}",
 *		array(
 *			"controller" => "documentation",
 *			"action"     => "show"
 *		)
 *	);
 *
 *	$router->handle();
 *
 *	echo $router->getControllerName();
 *</code>
 *
 */
zend_class_entry *phalcon_mvc_router_ce;

PHP_METHOD(Phalcon_Mvc_Router, __construct);
PHP_METHOD(Phalcon_Mvc_Router, getRewriteUri);
PHP_METHOD(Phalcon_Mvc_Router, setUriSource);
PHP_METHOD(Phalcon_Mvc_Router, removeExtraSlashes);
PHP_METHOD(Phalcon_Mvc_Router, setDefaults);
PHP_METHOD(Phalcon_Mvc_Router, getDefaults);
PHP_METHOD(Phalcon_Mvc_Router, handle);
PHP_METHOD(Phalcon_Mvc_Router, add);
PHP_METHOD(Phalcon_Mvc_Router, addGet);
PHP_METHOD(Phalcon_Mvc_Router, addPost);
PHP_METHOD(Phalcon_Mvc_Router, addPut);
PHP_METHOD(Phalcon_Mvc_Router, addPatch);
PHP_METHOD(Phalcon_Mvc_Router, addDelete);
PHP_METHOD(Phalcon_Mvc_Router, addOptions);
PHP_METHOD(Phalcon_Mvc_Router, addHead);
PHP_METHOD(Phalcon_Mvc_Router, mount);
PHP_METHOD(Phalcon_Mvc_Router, notFound);
PHP_METHOD(Phalcon_Mvc_Router, clear);
PHP_METHOD(Phalcon_Mvc_Router, getMatchedRoute);
PHP_METHOD(Phalcon_Mvc_Router, getMatches);
PHP_METHOD(Phalcon_Mvc_Router, wasMatched);
PHP_METHOD(Phalcon_Mvc_Router, getRoutes);
PHP_METHOD(Phalcon_Mvc_Router, getRouteById);
PHP_METHOD(Phalcon_Mvc_Router, getRouteByName);
PHP_METHOD(Phalcon_Mvc_Router, isExactControllerName);
PHP_METHOD(Phalcon_Mvc_Router, setDefaultController);
PHP_METHOD(Phalcon_Mvc_Router, getDefaultController);
PHP_METHOD(Phalcon_Mvc_Router, setControllerName);
PHP_METHOD(Phalcon_Mvc_Router, getControllerName);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, defaultRoutes, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_seturisource, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, uriSource, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_removeextraslashes, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, remove, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_mount, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, group, Phalcon\\Mvc\\Router\\Group, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_notfound, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, paths, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_router_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Router, __construct, arginfo_phalcon_mvc_router___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Router, getRewriteUri, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setUriSource, arginfo_phalcon_mvc_router_seturisource, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, removeExtraSlashes, arginfo_phalcon_mvc_router_removeextraslashes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setDefaults, arginfo_phalcon_mvc_routerinterface_setdefaults, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getDefaults, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, handle, arginfo_phalcon_mvc_routerinterface_handle, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, add, arginfo_phalcon_mvc_routerinterface_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, addGet, arginfo_phalcon_mvc_routerinterface_addget, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, addPost, arginfo_phalcon_mvc_routerinterface_addpost, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, addPut, arginfo_phalcon_mvc_routerinterface_addput, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, addPatch, arginfo_phalcon_mvc_routerinterface_addpatch, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, addDelete, arginfo_phalcon_mvc_routerinterface_adddelete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, addOptions, arginfo_phalcon_mvc_routerinterface_addoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, addHead, arginfo_phalcon_mvc_routerinterface_addhead, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, mount, arginfo_phalcon_mvc_router_mount, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, notFound, arginfo_phalcon_mvc_router_notfound, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, clear, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getMatchedRoute, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getMatches, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, wasMatched, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getRoutes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getRouteById, arginfo_phalcon_mvc_routerinterface_getroutebyid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getRouteByName, arginfo_phalcon_mvc_routerinterface_getroutebyname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, isExactControllerName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setDefaultController, arginfo_phalcon_routerinterface_setdefaulthandler, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getDefaultController, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setControllerName, arginfo_phalcon_routerinterface_sethandlername, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getControllerName, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Router initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Router){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, Router, mvc_router, phalcon_router_ce, phalcon_mvc_router_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_router_ce, SL("_uriSource"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_routes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_routesNameLookup"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_matchedRoute"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_matches"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_router_ce, SL("_wasMatched"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_removeExtraSlashes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_notFoundPaths"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_router_ce, SL("_isExactControllerName"), 0, ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_mvc_router_ce, SL("URI_SOURCE_GET_URL"), 0);
	zend_declare_class_constant_long(phalcon_mvc_router_ce, SL("URI_SOURCE_SERVER_REQUEST_URI"), 1);

	zend_class_implements(phalcon_mvc_router_ce, 1, phalcon_mvc_routerinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Router constructor
 *
 * @param boolean $defaultRoutes
 */
PHP_METHOD(Phalcon_Mvc_Router, __construct){

	zval *default_routes = NULL, routes = {}, paths = {}, route = {}, params_pattern = {};

	phalcon_fetch_params(0, 0, 1, &default_routes);

	phalcon_update_property_empty_array(getThis(), SL("_defaultParams"));

	if (!default_routes) {
		default_routes = &PHALCON_GLOBAL(z_true);
	}

	array_init(&routes);
	if (PHALCON_IS_TRUE(default_routes)) {
		/**
		 * Two routes are added by default to match /:controller/:action and
		 * /:controller/:action/:params
		 */
		array_init_size(&paths, 1);
		phalcon_array_update_string_long(&paths, IS(controller), 1, 0);

		ZVAL_STRING(&params_pattern, "#^/([a-zA-Z0-9_-]++)/?+$#");

		object_init_ex(&route, phalcon_mvc_router_route_ce);
		PHALCON_CALL_METHOD(NULL, &route, "__construct", &params_pattern, &paths);
		phalcon_array_append(&routes, &route, 0);

		zval_ptr_dtor(&paths);
		zval_ptr_dtor(&params_pattern);


		array_init_size(&paths, 3);
		phalcon_array_update_string_long(&paths, IS(controller), 1, 0);
		phalcon_array_update_string_long(&paths, IS(action), 2, 0);
		phalcon_array_update_string_long(&paths, IS(params), 3, 0);

		ZVAL_STRING(&params_pattern, "#^/([a-zA-Z0-9_-]++)/([a-zA-Z0-9\\._]++)(/.*+)?+$#");

		object_init_ex(&route, phalcon_mvc_router_route_ce);
		PHALCON_CALL_METHOD(NULL, &route, "__construct", &params_pattern, &paths);
		phalcon_array_append(&routes, &route, 0);

		zval_ptr_dtor(&paths);
		zval_ptr_dtor(&params_pattern);
	}

	phalcon_update_property_empty_array(getThis(), SL("_params"));
	phalcon_update_property(getThis(), SL("_routes"), &routes);
	phalcon_update_property_empty_array(getThis(), SL("_routesNameLookup"));
	zval_ptr_dtor(&routes);
}

/**
 * Get rewrite info. This info is read from $_GET['_url']. This returns '/' if the rewrite information cannot be read
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getRewriteUri){

	zval longopts = {}, options = {}, uri_source = {}, *_GET, url = {}, *_SERVER, real_uri = {};

	if (unlikely(!strcmp(sapi_module.name, "cli"))) {
		array_init(&longopts);
		phalcon_array_append_str(&longopts, SL("url::"), 0);
		phalcon_array_append_str(&longopts, SL("uri::"), 0);
		PHALCON_CALL_FUNCTION(&options, "getopt", &PHALCON_GLOBAL(z_null), &longopts);
		zval_ptr_dtor(&longopts);

		if (phalcon_array_isset_fetch_str(&url, &options, SL("url"), PH_COPY)
			|| phalcon_array_isset_fetch_str(&url, &options, SL("uri"), PH_COPY)) {
			zval_ptr_dtor(&options);
			RETVAL_ZVAL(&url, 0, 0);
			return;
		}
		zval_ptr_dtor(&options);
	}

	/**
	 * The developer can change the URI source
	 */
	phalcon_read_property(&uri_source, getThis(), SL("_uriSource"), PH_NOISY|PH_READONLY);

	/**
	 * By default we use $_GET['url'] to obtain the rewrite information
	 */
	if (!zend_is_true(&uri_source)) { /* FIXME: Compare with URI_SOURCE_SERVER_REQUEST_URI */
		_GET = phalcon_get_global_str(SL("_GET"));
		if (phalcon_array_isset_fetch_str(&url, _GET, SL("_url"), PH_READONLY)) {
			if (PHALCON_IS_NOT_EMPTY(&url)) {
				RETURN_CTOR(&url);
			}
		}
	} else {
		/**
		 * Otherwise use the standard $_SERVER['REQUEST_URI']
		 */
		_SERVER = phalcon_get_global_str(SL("_SERVER"));
		if (phalcon_array_isset_fetch_str(&url, _SERVER, SL("REQUEST_URI"), PH_READONLY)) {
			zval url_parts = {};
			phalcon_fast_explode_str(&url_parts, SL("?"), &url);

			phalcon_array_fetch_long(&real_uri, &url_parts, 0, PH_NOISY|PH_COPY);
			if (PHALCON_IS_NOT_EMPTY(&real_uri)) {
				zval_ptr_dtor(&url_parts);
				RETVAL_ZVAL(&real_uri, 0, 0);
				return;
			}
			zval_ptr_dtor(&url_parts);
		}
	}

	RETURN_STRING("/");
}

/**
 * Sets the URI source. One of the URI_SOURCE_* constants
 *
 *<code>
 *	$router->setUriSource(Router::URI_SOURCE_SERVER_REQUEST_URI);
 *</code>
 *
 * @param int $uriSource
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setUriSource){

	zval *uri_source;

	phalcon_fetch_params(0, 1, 0, &uri_source);

	phalcon_update_property(getThis(), SL("_uriSource"), uri_source);
	RETURN_THIS();
}

/**
 * Set whether router must remove the extra slashes in the handled routes
 *
 * @param boolean $remove
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, removeExtraSlashes){

	zval *remove;

	phalcon_fetch_params(0, 1, 0, &remove);

	phalcon_update_property(getThis(), SL("_removeExtraSlashes"), remove);
	RETURN_THIS();
}

/**
 * Sets an array of default paths. If a route is missing a path the router will use the defined here
 * This method must not be used to set a 404 route
 *
 *<code>
 * $router->setDefaults(array(
 *		'module' => 'common',
 *		'action' => 'index'
 * ));
 *</code>
 *
 * @param array $defaults
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setDefaults){

	zval *defaults, namespace_name = {}, module_name = {}, controller_name = {}, action_name = {}, params = {}, mode = {};

	phalcon_fetch_params(0, 1, 0, &defaults);

	if (Z_TYPE_P(defaults) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "Defaults must be an array");
		return;
	}

	/* Set the default namespace */
	if (phalcon_array_isset_fetch_str(&namespace_name, defaults, SL("namespace"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_defaultNamespace"), &namespace_name);
	}

	/* Set the default module */
	if (phalcon_array_isset_fetch_str(&module_name, defaults, SL("module"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_defaultModule"), &module_name);
	}

	/* Set the default controller */
	if (phalcon_array_isset_fetch_str(&controller_name, defaults, SL("controller"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_defaultHandler"), &controller_name);
	}

	/* Set the default action */
	if (phalcon_array_isset_fetch_str(&action_name, defaults, SL("action"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_defaultAction"), &action_name);
	}

	/* Set default parameters */
	if (phalcon_array_isset_fetch_str(&params, defaults, SL("params"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_defaultParams"), &params);
	}

	/* Set default parameters */
	if (phalcon_array_isset_fetch_str(&mode, defaults, SL("mode"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_mode"), &mode);
	}

	RETURN_THIS();
}

/**
 * Returns an array of default parameters
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Router, getDefaults){

	zval namespace_name = {}, module_name = {}, controller_name = {}, action_name = {}, params = {};

	phalcon_read_property(&namespace_name, getThis(), SL("_defaultNamespace"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&module_name, getThis(), SL("_defaultModule"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&controller_name, getThis(), SL("_defaultHandler"), PH_NOISY|PH_READONLY);
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
 * Handles routing information received from the rewrite engine
 *
 *<code>
 * //Read the info from the rewrite engine
 * $router->handle();
 *
 * //Manually passing an URL
 * $router->handle('/posts/edit/1');
 *</code>
 *
 * @param string $uri
 */
PHP_METHOD(Phalcon_Mvc_Router, handle){

	zval *uri = NULL, real_uri = {}, status = {}, removeextraslashes = {}, handled_uri = {}, route_found = {}, params = {}, service = {}, dependency_injector = {}, request = {}, debug_message = {}, event_name = {};
	zval all_case_sensitive = {}, current_host_name = {}, routes = {}, *route, matches = {}, parts = {}, namespace_name = {}, default_namespace = {}, module = {}, default_module = {}, exact = {};
	zval controller = {}, default_handler = {}, action = {}, default_action = {}, mode = {}, http_method = {}, action_name = {}, params_str = {}, str_params = {}, params_merge = {}, default_params = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 0, 1, &uri);

	if (!uri || !zend_is_true(uri)) {
		/**
		 * If 'uri' isn't passed as parameter it reads $_GET['_url']
		 */
		PHALCON_MM_CALL_METHOD(&real_uri, getThis(), "getrewriteuri");
		PHALCON_MM_ADD_ENTRY(&real_uri);
	} else {
		ZVAL_COPY_VALUE(&real_uri, uri);
	}

	PHALCON_MM_ZVAL_STRING(&event_name, "router:beforeHandle");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &real_uri);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}
	PHALCON_MM_ADD_ENTRY(&status);
	if (PHALCON_IS_NOT_EMPTY_STRING(&status)) {
		ZVAL_COPY_VALUE(&real_uri, &status);
	}

	/**
	 * Remove extra slashes in the route
	 */
	phalcon_read_property(&removeextraslashes, getThis(), SL("_removeExtraSlashes"), PH_NOISY|PH_READONLY);
	if (zend_is_true(&removeextraslashes)) {
		phalcon_remove_extra_slashes(&handled_uri, &real_uri);
		PHALCON_MM_ADD_ENTRY(&handled_uri);
	} else {
		ZVAL_COPY_VALUE(&handled_uri, &real_uri);
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "Handle the URI: ", &real_uri);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	ZVAL_FALSE(&route_found);

	/**
	 * Retrieve the request service from the container
	 */
	ZVAL_STR(&service, IS(request));

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	PHALCON_MM_VERIFY_INTERFACE_EX(&dependency_injector, phalcon_diinterface_ce, phalcon_mvc_router_exception_ce);

	PHALCON_MM_CALL_METHOD(&request, &dependency_injector, "getshared", &service);
	PHALCON_MM_ADD_ENTRY(&request);
	PHALCON_MM_VERIFY_INTERFACE_EX(&request, phalcon_http_requestinterface_ce, phalcon_mvc_router_exception_ce);

	phalcon_update_property_bool(getThis(), SL("_wasMatched"), 0);
	phalcon_update_property_null(getThis(), SL("_matchedRoute"));

	PHALCON_MM_ZVAL_STRING(&event_name, "router:beforeCheckRoutes");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &handled_uri);

	PHALCON_MM_CALL_METHOD(&current_host_name, &request, "gethttphost");
	PHALCON_MM_ADD_ENTRY(&current_host_name);
	PHALCON_MM_CALL_METHOD(&http_method, &request, "getmethod");
	PHALCON_MM_ADD_ENTRY(&http_method);

	/**
	 * Routes are traversed in reversed order
	 */
	phalcon_read_property(&routes, getThis(), SL("_routes"), PH_NOISY|PH_READONLY);

	PHALCON_MM_CALL_METHOD(&all_case_sensitive, getThis(), "getcasesensitive");
	PHALCON_MM_ADD_ENTRY(&all_case_sensitive);

	ZEND_HASH_REVERSE_FOREACH_VAL(Z_ARRVAL(routes), route) {
		zval case_sensitive = {}, methods = {}, match_method = {}, hostname = {}, prefix = {}, regex_host_name = {}, matched = {};
		zval pattern = {}, case_pattern = {}, before_match = {}, before_match_params = {}, paths = {};
		zval converters = {}, *position;

		PHALCON_MM_CALL_METHOD(&case_sensitive, route, "getcasesensitive");
		PHALCON_MM_ADD_ENTRY(&case_sensitive);
		if (Z_TYPE(case_sensitive) == IS_NULL) {
			ZVAL_COPY_VALUE(&case_sensitive, &all_case_sensitive);
		}

		/**
		 * Look for HTTP method constraints
		 */
		PHALCON_MM_CALL_METHOD(&methods, route, "gethttpmethods");
		if (Z_TYPE(methods) != IS_NULL) {
			PHALCON_MM_ADD_ENTRY(&methods);
			/**
			 * Check if the current method is allowed by the route
			 */
			PHALCON_MM_CALL_METHOD(&match_method, &request, "ismethod", &methods);
			if (PHALCON_IS_FALSE(&match_method)) {
				continue;
			}
			PHALCON_MM_ADD_ENTRY(&match_method);
		}

		/**
		 * Look for hostname constraints
		 */
		PHALCON_MM_CALL_METHOD(&hostname, route, "gethostname");
		if (Z_TYPE(hostname) != IS_NULL) {
			PHALCON_MM_ADD_ENTRY(&hostname);
			/**
			 * No HTTP_HOST, maybe in CLI mode?
			 */
			if (Z_TYPE(current_host_name) == IS_NULL) {
				continue;
			}

			/**
			 * Check if the hostname restriction is the same as the current in the route
			 */
			if (phalcon_memnstr_str(&hostname, SL("("))) {
				if (!phalcon_memnstr_str(&hostname, SL("#"))) {
					/* FIXME: handle mixed case */
					PHALCON_CONCAT_SVS(&regex_host_name, "#^", &hostname, "$#");
					PHALCON_MM_ADD_ENTRY(&regex_host_name);
				} else {
					ZVAL_COPY_VALUE(&regex_host_name, &hostname);
				}

				phalcon_preg_match(&matched, &regex_host_name, &current_host_name, NULL, 0, 0);

				if (!zend_is_true(&matched)) {
					continue;
				}
			} else {
				/* FIXME: handle mixed case */
				is_equal_function(&matched, &current_host_name, &hostname);
			}

			if (!zend_is_true(&matched)) {
				continue;
			}
		}

		/**
		 * Look for hostname constraints
		 */
		PHALCON_MM_CALL_METHOD(&prefix, route, "getprefix");
		PHALCON_MM_ADD_ENTRY(&prefix);
		if (PHALCON_IS_NOT_EMPTY(&prefix)) {
			if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				PHALCON_CONCAT_SV(&debug_message, "--Route prefix: ", &prefix);
				PHALCON_DEBUG_LOG(&debug_message);
				zval_ptr_dtor(&debug_message);
			}
			if (!phalcon_start_with(&handled_uri, &prefix, &case_sensitive)) {
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "match failure");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}
				continue;
			}
		}

		PHALCON_MM_ZVAL_STRING(&event_name, "router:beforeCheckRoute");
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

		/**
		 * If the route has parentheses use preg_match
		 */
		PHALCON_MM_CALL_METHOD(&pattern, route, "getcompiledpattern");
		PHALCON_MM_ADD_ENTRY(&pattern);

		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_CONCAT_SV(&debug_message, "--Route Pattern: ", &pattern);
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
		}

		ZVAL_NULL(&matches);
		if (Z_TYPE(pattern) == IS_STRING && Z_STRLEN(pattern) > 3 && Z_STRVAL(pattern)[1] == '^') {
			if (zend_is_true(&case_sensitive)) {
				PHALCON_CONCAT_VS(&case_pattern, &pattern, "i");
				phalcon_preg_match(&route_found, &case_pattern, &handled_uri, &matches, 0, 0);
				zval_ptr_dtor(&case_pattern);
			} else {
				phalcon_preg_match(&route_found, &pattern, &handled_uri, &matches, 0, 0);
			}
			PHALCON_MM_ADD_ENTRY(&matches);
		} else {
			ZVAL_BOOL(&route_found, phalcon_comparestr(&pattern, &handled_uri, &case_sensitive));
		}

		/**
		 * Check for beforeMatch conditions
		 */
		if (zend_is_true(&route_found)) {
			PHALCON_MM_ZVAL_STRING(&event_name, "router:matchedRoute");
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, route);
			PHALCON_MM_CALL_METHOD(&before_match, route, "getbeforematch");
			if (Z_TYPE(before_match) != IS_NULL) {
				PHALCON_MM_ADD_ENTRY(&before_match);
				/**
				 * Check first if the callback is callable
				 */
				if (!phalcon_is_callable(&before_match)) {
					PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "Before-Match callback is not callable in matched route");
					return;
				}

				/**
				 * Before-Match parameters
				 */
				array_init_size(&before_match_params, 3);
				PHALCON_MM_ADD_ENTRY(&before_match_params);
				phalcon_array_append(&before_match_params, &handled_uri, PH_COPY);
				phalcon_array_append(&before_match_params, route, PH_COPY);
				phalcon_array_append(&before_match_params, getThis(), PH_COPY);

				/**
				 * Call the function in the PHP userland
				 */
				PHALCON_MM_CALL_USER_FUNC_ARRAY(&route_found, &before_match, &before_match_params);
				PHALCON_MM_ADD_ENTRY(&route_found);
			}

			if (zend_is_true(&route_found)) {
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "--Found matches: ");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
					PHALCON_DEBUG_LOG(&matches);
				}

				/**
				 * Start from the default paths
				 */
				PHALCON_MM_CALL_METHOD(&paths, route, "getpaths");
				PHALCON_MM_ADD_ENTRY(&paths);
				PHALCON_MM_ZVAL_DUP(&parts, &paths);

				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "--Route paths: ");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
					PHALCON_DEBUG_LOG(&paths);
				}

				/**
				 * Check if the matches has variables
				 */
				if (Z_TYPE(matches) == IS_ARRAY && Z_TYPE(paths) == IS_ARRAY) {
					/**
					 * Get the route converters if any
					 */
					PHALCON_MM_CALL_METHOD(&converters, route, "getconverters");
					PHALCON_MM_ADD_ENTRY(&converters);

					ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(paths), idx, str_key, position) {
						zval tmp = {}, match_position = {}, converter = {}, parameters = {}, converted_part = {};
						if (str_key) {
							ZVAL_STR(&tmp, str_key);
						} else {
							ZVAL_LONG(&tmp, idx);
						}
						if (!str_key || str_key->val[0] != '\0') {
							if (phalcon_array_isset_fetch(&match_position, &matches, position, PH_READONLY)) {
								/* Check if the part has a converter */
								if (phalcon_array_isset_fetch(&converter, &converters, &tmp, PH_READONLY)) {
									array_init_size(&parameters, 1);
									phalcon_array_append(&parameters, &match_position, PH_COPY);
									PHALCON_MM_ADD_ENTRY(&parameters);
									PHALCON_MM_CALL_USER_FUNC_ARRAY(&converted_part, &converter, &parameters);

									phalcon_array_update(&parts, &tmp, &converted_part, 0);
								} else {
									/* Update the parts if there is no converter */
									phalcon_array_update(&parts, &tmp, &match_position, PH_COPY);
								}
							} else if (phalcon_array_isset_fetch(&converter, &converters, &tmp, PH_READONLY)) {
								array_init_size(&parameters, 1);
								phalcon_array_append(&parameters, position, PH_COPY);
								PHALCON_MM_ADD_ENTRY(&parameters);
								PHALCON_MM_CALL_USER_FUNC_ARRAY(&converted_part, &converter, &parameters);

								phalcon_array_update(&parts, &tmp, &converted_part, 0);
							}
						}
					} ZEND_HASH_FOREACH_END();

					/**
					 * Update the matches generated by preg_match
					 */
					phalcon_update_property(getThis(), SL("_matches"), &matches);
				} else {
					/**
					 * Get the route converters if any
					 */
					PHALCON_MM_CALL_METHOD(&converters, route, "getconverters");
					PHALCON_MM_ADD_ENTRY(&converters);

					ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(paths), idx, str_key, position) {
						zval tmp = {}, converter = {}, parameters = {}, converted_part = {};
						if (str_key) {
							ZVAL_STR(&tmp, str_key);
						} else {
							ZVAL_LONG(&tmp, idx);
						}
						if (!str_key || str_key->val[0] != '\0') {
							if (phalcon_array_isset_fetch(&converter, &converters, &tmp, PH_READONLY)) {
								array_init_size(&parameters, 1);
								phalcon_array_append(&parameters, position, PH_COPY);
								PHALCON_MM_ADD_ENTRY(&parameters);
								PHALCON_MM_CALL_USER_FUNC_ARRAY(&converted_part, &converter, &parameters);

								phalcon_array_update(&parts, &tmp, &converted_part, 0);
							}
						}
					} ZEND_HASH_FOREACH_END();
				}
				phalcon_update_property(getThis(), SL("_matchedRoute"), route);
				break;
			}

			if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				PHALCON_CONCAT_SV(&debug_message, "--Not Found Route: ", &pattern);
				PHALCON_DEBUG_LOG(&debug_message);
				zval_ptr_dtor(&debug_message);
			}
		} else {
			PHALCON_MM_ZVAL_STRING(&event_name, "router:notMatchedRoute");
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, route);
		}
	} ZEND_HASH_FOREACH_END();

	/**
	 * Update the wasMatched property indicating if the route was matched
	 */
	phalcon_update_property_bool(getThis(), SL("_wasMatched"), zend_is_true(&route_found));

	/**
	 * The route wasn't found, try to use the not-found paths
	 */
	if (!zend_is_true(&route_found)) {
		phalcon_update_property_null(getThis(), SL("_matches"));
		phalcon_update_property_null(getThis(), SL("_matchedRoute"));
		phalcon_read_property(&parts, getThis(), SL("_notFoundPaths"), PH_READONLY);
		if (Z_TYPE(parts) != IS_NULL) {
			ZVAL_TRUE(&route_found);
		}
	}

	if (zend_is_true(&route_found)) {
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			ZVAL_STRING(&debug_message, "--Route Parts: ");
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
			PHALCON_DEBUG_LOG(&parts);
		}

		/**
		 * Check for a namespace
		 */
		if (phalcon_array_isset_fetch_str(&namespace_name, &parts, SL("namespace"), PH_READONLY)) {
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setnamespacename", &namespace_name);
			phalcon_array_unset_str(&parts, SL("namespace"), 0);
		} else {
			PHALCON_MM_CALL_METHOD(&default_namespace, route, "getdefaultnamespace");
			PHALCON_MM_ADD_ENTRY(&default_namespace);
			if (Z_TYPE(default_namespace) == IS_NULL) {
				phalcon_read_property(&default_namespace, getThis(), SL("_defaultNamespace"), PH_READONLY);
			}
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setnamespacename", &default_namespace);
		}

		/**
		 * Check for a module
		 */
		if (phalcon_array_isset_fetch_str(&module, &parts, SL("module"), PH_READONLY)) {
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setmodulename", &module);
			phalcon_array_unset_str(&parts, SL("module"), 0);
		} else {
			PHALCON_MM_CALL_METHOD(&default_module, route, "getdefaultmodule");
			PHALCON_MM_ADD_ENTRY(&default_module);
			if (Z_TYPE(default_module) == IS_NULL) {
				phalcon_read_property(&default_module, getThis(), SL("_defaultModule"), PH_READONLY);
			}
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setmodulename", &default_module);
		}

		if (phalcon_array_isset_fetch_str(&exact, &parts, SL("\0exact"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_isExactControllerName"), &exact);
			phalcon_array_unset_str(&parts, SL("\0exact"), 0);
		} else {
			ZVAL_FALSE(&exact);
			phalcon_update_property(getThis(), SL("_isExactControllerName"), &exact);
		}

		/**
		 * Check for a controller
		 */
		if (phalcon_array_isset_fetch_str(&controller, &parts, SL("controller"), PH_READONLY)) {
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setcontrollername", &controller);
			phalcon_array_unset_str(&parts, SL("controller"), 0);
		} else {
			PHALCON_MM_CALL_METHOD(&default_handler, route, "getdefaultcontroller");
			PHALCON_MM_ADD_ENTRY(&default_handler);
			if (Z_TYPE(default_handler) == IS_NULL) {
				phalcon_read_property(&default_handler, getThis(), SL("_defaultHandler"), PH_READONLY);
			}
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setcontrollername", &default_handler);
		}

		/**
		 * Check for an action
		 */
		if (phalcon_array_isset_fetch_str(&action, &parts, SL("action"), PH_COPY)) {
			PHALCON_MM_ADD_ENTRY(&action);
			phalcon_array_unset_str(&parts, SL("action"), 0);
		} else {
			PHALCON_MM_CALL_METHOD(&action, route, "getdefaultaction");
			PHALCON_MM_ADD_ENTRY(&action);
			if (Z_TYPE(action) == IS_NULL) {
				phalcon_read_property(&action, getThis(), SL("_defaultAction"), PH_READONLY);
			}
		}

		PHALCON_MM_CALL_METHOD(&mode, route, "getmode");
		if (Z_LVAL(mode) <= PHALCON_ROUTER_MODE_DEFAULT) {
			PHALCON_MM_CALL_METHOD(&mode, getThis(), "getmode");
		}
		if (unlikely(Z_LVAL(mode) == PHALCON_ROUTER_MODE_REST)) {
			zval camelized_method = {};
			phalcon_camelize(&camelized_method, &http_method);
			PHALCON_CONCAT_VV(&action_name, &action, &camelized_method);
			zval_ptr_dtor(&camelized_method);
			PHALCON_MM_ADD_ENTRY(&action_name);
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setactionname", &action_name);
		} else {
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setactionname", &action);
		}

		/**
		 * Check for parameters
		 */
		if (phalcon_array_isset_fetch_str(&params_str, &parts, SL("params"), PH_READONLY)) {
			if (Z_TYPE(params_str) == IS_STRING) {
				if (phalcon_start_with_str(&params_str, SL("/"))) {
					phalcon_substr(&str_params, &params_str, 1, 0);
				} else {
					phalcon_substr(&str_params, &params_str, 0, 0);
				}

				if (zend_is_true(&str_params)) {
					zval slash = {};
					ZVAL_STRINGL(&slash, "/", 1);
					phalcon_fast_explode(&params, &slash, &str_params);
					zval_ptr_dtor(&slash);
				} else if (!PHALCON_IS_EMPTY(&str_params)) {
					array_init(&params);
					phalcon_array_append(&params, &str_params, PH_COPY);
				} else {
					array_init(&params);
				}
				zval_ptr_dtor(&str_params);
			} else {
				array_init(&params);
			}

			phalcon_array_unset_str(&parts, SL("params"), 0);
		} else {
			array_init(&params);
		}

		if (zend_hash_num_elements(Z_ARRVAL(params))) {
			phalcon_fast_array_merge(&params_merge, &params, &parts);
		} else {
			ZVAL_COPY(&params_merge, &parts);
		}

		zval_ptr_dtor(&params);
		PHALCON_MM_ADD_ENTRY(&params_merge);
		if (PHALCON_IS_EMPTY(&params_merge)) {
			PHALCON_MM_CALL_METHOD(&default_params, route, "getdefaultparams");
			PHALCON_MM_ADD_ENTRY(&default_params);

			if (Z_TYPE(default_params) == IS_NULL) {
				phalcon_read_property(&default_params, getThis(), SL("_defaultParams"), PH_READONLY);
			}
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setparams", &default_params);
		} else {
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setparams", &params_merge);
		}
	} else {
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			ZVAL_STRING(&debug_message, "--Use default values");
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
		}

		/**
		 * Use default values if the route hasn't matched
		 */
		phalcon_read_property(&default_namespace, getThis(), SL("_defaultNamespace"), PH_READONLY);
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setnamespacename", &default_namespace);

		phalcon_read_property(&default_module, getThis(), SL("_defaultModule"), PH_READONLY);
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setmodulename", &default_module);

		phalcon_read_property(&default_handler, getThis(), SL("_defaultHandler"), PH_READONLY);
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setcontrollername", &default_handler);

		phalcon_read_property(&default_action, getThis(), SL("_defaultAction"), PH_READONLY);

		PHALCON_MM_CALL_METHOD(&mode, getThis(), "getmode");
		if (unlikely(Z_LVAL(mode) == PHALCON_ROUTER_MODE_REST)) {
			zval camelized_method = {};
			phalcon_camelize(&camelized_method, &http_method);
			PHALCON_CONCAT_VV(&action_name, &default_action, &camelized_method);
			zval_ptr_dtor(&camelized_method);
			PHALCON_MM_ADD_ENTRY(&action_name);
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setactionname", &action_name);
		} else {
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setactionname", &default_action);
		}

		phalcon_read_property(&default_params, getThis(), SL("_defaultParams"), PH_READONLY);
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setparams", &default_params);
	}

	PHALCON_MM_ZVAL_STRING(&event_name, "router:afterCheckRoutes");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

	PHALCON_MM_ZVAL_STRING(&event_name, "router:afterHandle");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &route_found);

	if (zend_is_true(&route_found)) {
		RETURN_MM_TRUE;
	}

	RETURN_MM_FALSE;
}

/**
 * Adds a route to the router without any HTTP constraint
 *
 *<code>
 * $router->add('/about', 'About::index');
 *</code>
 *
 * @param string $pattern
 * @param string/array $paths
 * @param array $regex
 * @param string $httpMethods
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router, add){

	zval *pattern, *paths = NULL, *regex = NULL, *http_methods = NULL;

	phalcon_fetch_params(0, 1, 3, &pattern, &paths, &regex, &http_methods);

	if (!paths) {
		paths = &PHALCON_GLOBAL(z_null);
	}

	if (!regex) {
		regex = &PHALCON_GLOBAL(z_null);
	}

	if (!http_methods) {
		http_methods = &PHALCON_GLOBAL(z_null);
	}

	/**
	 * Every route is internally stored as a Phalcon\Mvc\Router\Route
	 */
	object_init_ex(return_value, phalcon_mvc_router_route_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", pattern, paths, http_methods, regex);

	phalcon_update_property_array_append(getThis(), SL("_routes"), return_value);
}

static void phalcon_mvc_router_add_helper(INTERNAL_FUNCTION_PARAMETERS, zend_string *method)
{
	zval *pattern, *paths = NULL, http_method = {};

	phalcon_fetch_params(0, 1, 1, &pattern, &paths);

	if (!paths) {
		paths = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STR(&http_method, method);
	PHALCON_RETURN_CALL_METHOD(getThis(), "add", pattern, paths, &http_method);
}

/**
 * Adds a route to the router that only match if the HTTP method is GET
 *
 * @param string $pattern
 * @param string/array $paths
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router, addGet){

	phalcon_mvc_router_add_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS(GET));
}

/**
 * Adds a route to the router that only match if the HTTP method is POST
 *
 * @param string $pattern
 * @param string/array $paths
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router, addPost){

	phalcon_mvc_router_add_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS(POST));
}

/**
 * Adds a route to the router that only match if the HTTP method is PUT
 *
 * @param string $pattern
 * @param string/array $paths
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router, addPut){

	phalcon_mvc_router_add_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS(PUT));
}

/**
 * Adds a route to the router that only match if the HTTP method is PATCH
 *
 * @param string $pattern
 * @param string/array $paths
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router, addPatch){

	phalcon_mvc_router_add_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS(PATCH));
}

/**
 * Adds a route to the router that only match if the HTTP method is DELETE
 *
 * @param string $pattern
 * @param string/array $paths
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router, addDelete){

	phalcon_mvc_router_add_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS(DELETE));
}

/**
 * Add a route to the router that only match if the HTTP method is OPTIONS
 *
 * @param string $pattern
 * @param string/array $paths
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router, addOptions){

	phalcon_mvc_router_add_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS(OPTIONS));
}

/**
 * Adds a route to the router that only match if the HTTP method is HEAD
 *
 * @param string $pattern
 * @param string/array $paths
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router, addHead){

	phalcon_mvc_router_add_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS(HEAD));
}

static int phalcon_router_call_convert(zval *pDest, int num_args, va_list args, zend_hash_key *hash_key)
{
	zval *route, key = {}, *params[2];

	assert(num_args == 1);

	route = va_arg(args, zval*);
	if (hash_key->key->len) {
		ZVAL_STR(&key, zend_string_dup(hash_key->key, 0));
	}
	else {
		ZVAL_LONG(&key, hash_key->h);
	}

	params[0] = &key;
	params[1] = pDest;

	if (FAILURE == phalcon_call_method(NULL, route, "convert", 2, params)) {
		return ZEND_HASH_APPLY_STOP;
	}

	return ZEND_HASH_APPLY_KEEP;
}

/**
 * Mounts a group of routes in the router
 *
 * @param Phalcon\Mvc\Router\Group $route
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, mount){

	zval *group, group_routes = {}, before_match = {}, prefix = {}, hostname = {}, converters = {}, *route, routes = {};

	phalcon_fetch_params(0, 1, 0, &group);
	PHALCON_VERIFY_CLASS_EX(group, phalcon_mvc_router_group_ce, phalcon_mvc_router_exception_ce);

	PHALCON_CALL_METHOD(&group_routes, group, "getroutes");
	if (Z_TYPE(group_routes) != IS_ARRAY || !zend_hash_num_elements(Z_ARRVAL(group_routes))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "The group of routes does not contain any routes");
		return;
	}

	/* Get the before-match condition */
	PHALCON_CALL_METHOD(&before_match, group, "getbeforematch");

	/* Get the hostname restriction */
	PHALCON_CALL_METHOD(&hostname, group, "gethostname");

	/* Get the prefix restriction */
	PHALCON_CALL_METHOD(&prefix, group, "getprefix");

	/* Get converters */
	PHALCON_CALL_METHOD(&converters, group, "getconverters");

	if (Z_TYPE(before_match) != IS_NULL || Z_TYPE(hostname) != IS_NULL || Z_TYPE(converters) != IS_NULL) {
		int has_before_match = (Z_TYPE(before_match) != IS_NULL);
		int has_hostname     = (Z_TYPE(hostname) != IS_NULL);
		int has_prefix     = (Z_TYPE(prefix) != IS_NULL);
		int has_converters   = (Z_TYPE(converters) != IS_NULL);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(group_routes), route) {
			if (has_before_match) {
				PHALCON_CALL_METHOD(NULL, route, "beforematch", &before_match);
			}

			if (has_hostname) {
				PHALCON_CALL_METHOD(NULL, route, "sethostname", &hostname);
			}

			if (has_prefix) {
				PHALCON_CALL_METHOD(NULL, route, "setprefix", &prefix);
			}

			if (has_converters) {
				zend_hash_apply_with_arguments(Z_ARRVAL(converters), phalcon_router_call_convert, 1, route);
			}
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_read_property(&routes, getThis(), SL("_routes"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(routes) == IS_ARRAY) {
		zval new_routes = {};
		phalcon_fast_array_merge(&new_routes, &routes, &group_routes);
		phalcon_update_property(getThis(), SL("_routes"), &new_routes);
		zval_ptr_dtor(&new_routes);
	} else {
		phalcon_update_property(getThis(), SL("_routes"), &group_routes);
	}
	zval_ptr_dtor(&group_routes);
	zval_ptr_dtor(&before_match);
	zval_ptr_dtor(&hostname);
	zval_ptr_dtor(&prefix);
	zval_ptr_dtor(&converters);

	RETURN_THIS();
}

/**
 * Set a group of paths to be returned when none of the defined routes are matched
 *
 * @param array|string $paths
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, notFound){

	zval *paths;

	phalcon_fetch_params(0, 1, 0, &paths);

	if (Z_TYPE_P(paths) != IS_ARRAY) {
		if (Z_TYPE_P(paths) != IS_STRING) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "The not-found paths must be an array or string");
			return;
		}
	}
	PHALCON_SEPARATE_PARAM(paths);
	phalcon_update_property(getThis(), SL("_notFoundPaths"), paths);

	RETURN_THIS();
}

/**
 * Removes all the pre-defined routes
 */
PHP_METHOD(Phalcon_Mvc_Router, clear){

	phalcon_update_property_empty_array(getThis(), SL("_routes"));
	phalcon_update_property_empty_array(getThis(), SL("_routesNameLookup"));

}

/**
 * Returns the route that matchs the handled URI
 *
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router, getMatchedRoute){


	RETURN_MEMBER(getThis(), "_matchedRoute");
}

/**
 * Returns the sub expressions in the regular expression matched
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Router, getMatches){


	RETURN_MEMBER(getThis(), "_matches");
}

/**
 * Checks if the router macthes any of the defined routes
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Router, wasMatched){


	RETURN_MEMBER(getThis(), "_wasMatched");
}

/**
 * Returns all the routes defined in the router
 *
 * @return Phalcon\Mvc\Router\Route[]
 */
PHP_METHOD(Phalcon_Mvc_Router, getRoutes){


	RETURN_MEMBER(getThis(), "_routes");
}

/**
 * Returns a route object by its id
 *
 * @param string $id
 * @return Phalcon\Mvc\Router\Route | false
 */
PHP_METHOD(Phalcon_Mvc_Router, getRouteById){

	zval *id, routes = {}, *route, route_id = {};

	phalcon_fetch_params(0, 1, 0, &id);

	phalcon_read_property(&routes, getThis(), SL("_routes"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(routes) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(routes), route) {
			PHALCON_CALL_METHOD(&route_id, route, "getrouteid");
			if (phalcon_is_equal(&route_id, id)) {
				RETURN_CTOR(route);
			}
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_FALSE;
}

/**
 * Returns a route object by its name
 *
 * @param string $name
 * @return Phalcon\Mvc\Router\Route
 */
PHP_METHOD(Phalcon_Mvc_Router, getRouteByName){

	zval *name, routes = {}, *route, routes_name_lookup = {};

	phalcon_fetch_params(0, 1, 0, &name);

	if (UNEXPECTED(Z_TYPE_P(name) != IS_STRING)) {
		PHALCON_SEPARATE_PARAM(name);
		convert_to_string(name);
	}

	phalcon_read_property(&routes_name_lookup, getThis(), SL("_routesNameLookup"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(name) && (route = zend_hash_find(Z_ARRVAL(routes_name_lookup), Z_STR_P(name))) != NULL) {
		RETURN_CTOR(route);
	}

	phalcon_read_property(&routes, getThis(), SL("_routes"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(routes) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(routes), route) {
			zval route_name = {};
			PHALCON_CALL_METHOD(&route_name, route, "getname");
			convert_to_string(&route_name);
			if (PHALCON_IS_NOT_EMPTY(&route_name)) {
				phalcon_update_property_array_string(getThis(), SL("_routesNameLookup"), Z_STR(route_name), route);
			}

			if (phalcon_is_equal(&route_name, name)) {
				RETURN_CTOR(route);
			}
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_FALSE;
}

/**
 * Returns whether controller name should not be mangled
 */
PHP_METHOD(Phalcon_Mvc_Router, isExactControllerName) {
	RETURN_MEMBER(getThis(), "_isExactControllerName");
}

/**
 * Sets the default controller name
 *
 * @param string $controllerName
 */
PHP_METHOD(Phalcon_Mvc_Router, setDefaultController){

	zval *controller_name;

	phalcon_fetch_params(0, 1, 0, &controller_name);

	phalcon_update_property(getThis(), SL("_defaultHandler"), controller_name);
	RETURN_THIS();
}

/**
 * Gets the default controller name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getDefaultController){


	RETURN_MEMBER(getThis(), "_defaultHandler");
}

/**
 * Sets the controller name
 *
 * @param string $controllerName
 */
PHP_METHOD(Phalcon_Mvc_Router, setControllerName){

	zval *controller_name;

	phalcon_fetch_params(0, 1, 0, &controller_name);

	phalcon_update_property(getThis(), SL("_handler"), controller_name);
	RETURN_THIS();
}

/**
 * Gets the controller name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getControllerName){


	RETURN_MEMBER(getThis(), "_handler");
}
