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
#include "mvc/routerinterface.h"
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
PHP_METHOD(Phalcon_Mvc_Router, setDefaultNamespace);
PHP_METHOD(Phalcon_Mvc_Router, getDefaultNamespace);
PHP_METHOD(Phalcon_Mvc_Router, setDefaultModule);
PHP_METHOD(Phalcon_Mvc_Router, getDefaultModule);
PHP_METHOD(Phalcon_Mvc_Router, setDefaultController);
PHP_METHOD(Phalcon_Mvc_Router, getDefaultController);
PHP_METHOD(Phalcon_Mvc_Router, setDefaultAction);
PHP_METHOD(Phalcon_Mvc_Router, getDefaultAction);
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
PHP_METHOD(Phalcon_Mvc_Router, setNamespaceName);
PHP_METHOD(Phalcon_Mvc_Router, getNamespaceName);
PHP_METHOD(Phalcon_Mvc_Router, setModuleName);
PHP_METHOD(Phalcon_Mvc_Router, getModuleName);
PHP_METHOD(Phalcon_Mvc_Router, setControllerName);
PHP_METHOD(Phalcon_Mvc_Router, getControllerName);
PHP_METHOD(Phalcon_Mvc_Router, setActionName);
PHP_METHOD(Phalcon_Mvc_Router, getActionName);
PHP_METHOD(Phalcon_Mvc_Router, setParams);
PHP_METHOD(Phalcon_Mvc_Router, getParams);
PHP_METHOD(Phalcon_Mvc_Router, getMatchedRoute);
PHP_METHOD(Phalcon_Mvc_Router, getMatches);
PHP_METHOD(Phalcon_Mvc_Router, wasMatched);
PHP_METHOD(Phalcon_Mvc_Router, getRoutes);
PHP_METHOD(Phalcon_Mvc_Router, getRouteById);
PHP_METHOD(Phalcon_Mvc_Router, getRouteByName);
PHP_METHOD(Phalcon_Mvc_Router, isExactControllerName);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, defaultRoutes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_seturisource, 0, 0, 1)
	ZEND_ARG_INFO(0, uriSource)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_removeextraslashes, 0, 0, 1)
	ZEND_ARG_INFO(0, remove)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_setdefaultnamespace, 0, 0, 1)
	ZEND_ARG_INFO(0, namespaceName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_mount, 0, 0, 1)
	ZEND_ARG_INFO(0, group)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_notfound, 0, 0, 1)
	ZEND_ARG_INFO(0, paths)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_setnamespacename, 0, 0, 1)
	ZEND_ARG_INFO(0, namespaceName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_setmodulename, 0, 0, 1)
	ZEND_ARG_INFO(0, moduleName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_setcontrollername, 0, 0, 1)
	ZEND_ARG_INFO(0, controllerName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_setactionname, 0, 0, 1)
	ZEND_ARG_INFO(0, actionName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_setparams, 0, 0, 1)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_router_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Router, __construct, arginfo_phalcon_mvc_router___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Router, getRewriteUri, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setUriSource, arginfo_phalcon_mvc_router_seturisource, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, removeExtraSlashes, arginfo_phalcon_mvc_router_removeextraslashes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setDefaultNamespace, arginfo_phalcon_mvc_router_setdefaultnamespace, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getDefaultNamespace, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setDefaultModule, arginfo_phalcon_mvc_routerinterface_setdefaultmodule, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getDefaultModule, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setDefaultController, arginfo_phalcon_mvc_routerinterface_setdefaultcontroller, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getDefaultController, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setDefaultAction, arginfo_phalcon_mvc_routerinterface_setdefaultaction, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getDefaultAction, NULL, ZEND_ACC_PUBLIC)
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
	PHP_ME(Phalcon_Mvc_Router, setNamespaceName, arginfo_phalcon_mvc_router_setnamespacename, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getNamespaceName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setModuleName, arginfo_phalcon_mvc_router_setmodulename, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getModuleName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setControllerName, arginfo_phalcon_mvc_router_setcontrollername, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getControllerName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setActionName, arginfo_phalcon_mvc_router_setactionname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getActionName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, setParams, arginfo_phalcon_mvc_router_setparams, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getMatchedRoute, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getMatches, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, wasMatched, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getRoutes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getRouteById, arginfo_phalcon_mvc_routerinterface_getroutebyid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, getRouteByName, arginfo_phalcon_mvc_routerinterface_getroutebyname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router, isExactControllerName, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Router initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Router){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, Router, mvc_router, phalcon_di_injectable_ce, phalcon_mvc_router_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_router_ce, SL("_uriSource"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_namespace"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_module"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_controller"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_action"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_params"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_routes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_routesNameLookup"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_matchedRoute"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_matches"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_router_ce, SL("_wasMatched"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_defaultNamespace"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_defaultModule"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_defaultController"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_defaultAction"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_ce, SL("_defaultParams"), ZEND_ACC_PROTECTED);
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
		add_assoc_long_ex(&paths, ISL(controller), 1);

		PHALCON_STR(&params_pattern, "#^/([a-zA-Z0-9_-]++)/?+$#");

		object_init_ex(&route, phalcon_mvc_router_route_ce);
		PHALCON_CALL_METHODW(NULL, &route, "__construct", &params_pattern, &paths);
		phalcon_array_append(&routes, &route, PH_COPY);


		array_init_size(&paths, 3);
		add_assoc_long_ex(&paths, ISL(controller), 1);
		add_assoc_long_ex(&paths, ISL(action), 2);
		add_assoc_long_ex(&paths, ISL(params), 3);

		PHALCON_STR(&params_pattern, "#^/([a-zA-Z0-9_-]++)/([a-zA-Z0-9\\._]++)(/.*+)?+$#");

		object_init_ex(&route, phalcon_mvc_router_route_ce);
		PHALCON_CALL_METHODW(NULL, &route, "__construct", &params_pattern, &paths);
		phalcon_array_append(&routes, &route, PH_COPY);

	}

	phalcon_update_property_empty_array(getThis(), SL("_params"));
	phalcon_update_property_zval(getThis(), SL("_routes"), &routes);
	phalcon_update_property_empty_array(getThis(), SL("_routesNameLookup"));
}

/**
 * Get rewrite info. This info is read from $_GET['_url']. This returns '/' if the rewrite information cannot be read
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getRewriteUri){

	zval longopts = {}, options = {}, uri_source = {}, *_GET, url = {}, *_SERVER, url_parts = {}, real_uri = {};

	if (unlikely(!strcmp(sapi_module.name, "cli"))) {
		array_init(&longopts);
		phalcon_array_append_string(&longopts, SL("url::"), 0);
		PHALCON_CALL_FUNCTIONW(&options, "getopt", &PHALCON_GLOBAL(z_null), &longopts);

		if (phalcon_array_isset_fetch_str(&url, &options, SL("url"))) {
			RETURN_CTORW(&url);
		}
	}

	/**
	 * The developer can change the URI source
	 */
	phalcon_read_property(&uri_source, getThis(), SL("_uriSource"), PH_NOISY);

	/**
	 * By default we use $_GET['url'] to obtain the rewrite information
	 */
	if (!zend_is_true(&uri_source)) { /* FIXME: Compare with URI_SOURCE_SERVER_REQUEST_URI */
		_GET = phalcon_get_global_str(SL("_GET"));
		if (phalcon_array_isset_fetch_str(&url, _GET, SL("_url"))) {
			if (PHALCON_IS_NOT_EMPTY(&url)) {
				RETURN_CTORW(&url);
			}
		}
	} else {
		/**
		 * Otherwise use the standard $_SERVER['REQUEST_URI']
		 */
		_SERVER = phalcon_get_global_str(SL("_SERVER"));
		if (phalcon_array_isset_fetch_str(&url, _SERVER, SL("REQUEST_URI"))) {
			phalcon_fast_explode_str(&url_parts, SL("?"), &url);

			phalcon_array_fetch_long(&real_uri, &url_parts, 0, PH_NOISY);
			if (PHALCON_IS_NOT_EMPTY(&real_uri)) {
				RETURN_CTORW(&real_uri);
			}
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

	phalcon_update_property_zval(getThis(), SL("_uriSource"), uri_source);
	RETURN_THISW();
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

	phalcon_update_property_zval(getThis(), SL("_removeExtraSlashes"), remove);
	RETURN_THISW();
}

/**
 * Sets the name of the default namespace
 *
 * @param string $namespaceName
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setDefaultNamespace){

	zval *namespace_name;

	phalcon_fetch_params(0, 1, 0, &namespace_name);

	phalcon_update_property_zval(getThis(), SL("_defaultNamespace"), namespace_name);
	RETURN_THISW();
}

/**
 * Returns the name of the default namespace
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getDefaultNamespace){

	RETURN_MEMBER(getThis(), "_defaultNamespace");
}

/**
 * Sets the name of the default module
 *
 * @param string $moduleName
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setDefaultModule){

	zval *module_name;

	phalcon_fetch_params(0, 1, 0, &module_name);

	phalcon_update_property_zval(getThis(), SL("_defaultModule"), module_name);
	RETURN_THISW();
}

/**
 * Returns the name of the default module
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getDefaultModule){

	RETURN_MEMBER(getThis(), "_defaultModule");
}

/**
 * Sets the default controller name
 *
 * @param string $controllerName
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setDefaultController){

	zval *controller_name;

	phalcon_fetch_params(0, 1, 0, &controller_name);

	phalcon_update_property_zval(getThis(), SL("_defaultController"), controller_name);
	RETURN_THISW();
}

/**
 * Returns the default controller name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getDefaultController){

	RETURN_MEMBER(getThis(), "_defaultController");
}

/**
 * Sets the default action name
 *
 * @param string $actionName
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setDefaultAction){

	zval *action_name;

	phalcon_fetch_params(0, 1, 0, &action_name);

	phalcon_update_property_zval(getThis(), SL("_defaultAction"), action_name);
	RETURN_THISW();
}

/**
 * Returns the default action name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getDefaultAction){

	RETURN_MEMBER(getThis(), "_defaultAction");
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

	zval *defaults, namespace_name = {}, module_name = {}, controller_name = {}, action_name = {}, params = {};

	phalcon_fetch_params(0, 1, 0, &defaults);

	if (Z_TYPE_P(defaults) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_router_exception_ce, "Defaults must be an array");
		return;
	}

	/* Set the default namespace */
	if (phalcon_array_isset_fetch_str(&namespace_name, defaults, SL("namespace"))) {
		phalcon_update_property_zval(getThis(), SL("_defaultNamespace"), &namespace_name);
	}

	/* Set the default module */
	if (phalcon_array_isset_fetch_str(&module_name, defaults, SL("module"))) {
		phalcon_update_property_zval(getThis(), SL("_defaultModule"), &module_name);
	}

	/* Set the default controller */
	if (phalcon_array_isset_fetch_str(&controller_name, defaults, SL("controller"))) {
		phalcon_update_property_zval(getThis(), SL("_defaultController"), &controller_name);
	}

	/* Set the default action */
	if (phalcon_array_isset_fetch_str(&action_name, defaults, SL("action"))) {
		phalcon_update_property_zval(getThis(), SL("_defaultAction"), &action_name);
	}

	/* Set default parameters */
	if (phalcon_array_isset_fetch_str(&params, defaults, SL("params"))) {
		phalcon_update_property_zval(getThis(), SL("_defaultParams"), &params);
	}

	RETURN_THISW();
}

/**
 * Returns an array of default parameters
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Router, getDefaults){

	zval namespace_name = {}, module_name = {}, controller_name = {}, action_name = {}, params = {};

	phalcon_read_property(&namespace_name, getThis(), SL("_defaultNamespace"), PH_NOISY);
	phalcon_read_property(&module_name, getThis(), SL("_defaultModule"), PH_NOISY);
	phalcon_read_property(&controller_name, getThis(), SL("_defaultController"), PH_NOISY);
	phalcon_read_property(&action_name, getThis(), SL("_defaultAction"), PH_NOISY);
	phalcon_read_property(&params, getThis(), SL("_defaultParams"), PH_NOISY);

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

	zval *uri = NULL, real_uri = {}, removeextraslashes = {}, handled_uri = {}, route_found = {}, params = {}, service = {}, dependency_injector = {}, request = {}, debug_message = {}, event_name = {};
	zval current_host_name = {}, routes = {}, *route, matches = {}, parts = {}, namespace_name = {}, default_namespace = {}, module = {}, default_module = {}, exact = {};
	zval controller = {}, default_controller = {}, action = {}, default_action = {}, params_str = {}, str_params = {}, params_merge = {}, default_params = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 1, &uri);

	if (!uri || !zend_is_true(uri)) {
		/**
		 * If 'uri' isn't passed as parameter it reads $_GET['_url']
		 */
		PHALCON_CALL_METHODW(&real_uri, getThis(), "getrewriteuri");
	} else {
		PHALCON_CPY_WRT(&real_uri, uri);
	}

	/**
	 * Remove extra slashes in the route
	 */
	phalcon_read_property(&removeextraslashes, getThis(), SL("_removeExtraSlashes"), PH_NOISY);
	if (zend_is_true(&removeextraslashes)) {
		phalcon_remove_extra_slashes(&handled_uri, &real_uri);
	} else {
		PHALCON_CPY_WRT(&handled_uri, &real_uri);
	}

	ZVAL_FALSE(&route_found);

	array_init(&params);

	/**
	 * Retrieve the request service from the container
	 */
	PHALCON_STR(&service, ISV(request));

	PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi");
	PHALCON_VERIFY_INTERFACE_EX(&dependency_injector, phalcon_diinterface_ce, phalcon_mvc_router_exception_ce, 0);

	PHALCON_CALL_METHODW(&request, &dependency_injector, "getshared", &service);
	PHALCON_VERIFY_INTERFACE_EX(&request, phalcon_http_requestinterface_ce, phalcon_mvc_router_exception_ce, 0);

	phalcon_update_property_bool(getThis(), SL("_wasMatched"), 0);
	phalcon_update_property_null(getThis(), SL("_matchedRoute"));

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "Handle the URI: ", &real_uri);
		PHALCON_DEBUG_LOG(&debug_message);
	}


	PHALCON_STR(&event_name, "router:beforeCheckRoutes");

	PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name, &handled_uri);
	PHALCON_CALL_METHODW(&current_host_name, &request, "gethttphost");

	/**
	 * Routes are traversed in reversed order
	 */
	phalcon_read_property(&routes, getThis(), SL("_routes"), PH_NOISY);

	ZEND_HASH_REVERSE_FOREACH_VAL(Z_ARRVAL(routes), route) {
		zval case_sensitive = {}, methods = {}, match_method = {}, hostname = {}, regex_host_name = {}, matched = {};
		zval pattern = {}, case_pattern = {}, before_match = {}, before_match_params = {}, paths = {};
		zval converters = {}, *position;

		PHALCON_CALL_METHODW(&case_sensitive, route, "getcasesensitive");

		/**
		 * Look for HTTP method constraints
		 */
		PHALCON_CALL_METHODW(&methods, route, "gethttpmethods");
		if (Z_TYPE(methods) != IS_NULL) {
			/**
			 * Check if the current method is allowed by the route
			 */
			PHALCON_CALL_METHODW(&match_method, &request, "ismethod", &methods);
			if (PHALCON_IS_FALSE(&match_method)) {
				continue;
			}
		}

		/**
		 * Look for hostname constraints
		 */
		PHALCON_CALL_METHODW(&hostname, route, "gethostname");
		if (Z_TYPE(hostname) != IS_NULL) {
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
				} else {
					PHALCON_CPY_WRT(&regex_host_name, &hostname);
				}

				RETURN_ON_FAILURE(phalcon_preg_match(&matched, &regex_host_name, &current_host_name, NULL));

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

		PHALCON_STR(&event_name, "router:beforeCheckRoute");

		PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);

		/**
		 * If the route has parentheses use preg_match
		 */
		PHALCON_CALL_METHODW(&pattern, route, "getcompiledpattern");

		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_CONCAT_SV(&debug_message, "--Route Pattern: ", &pattern);
			PHALCON_DEBUG_LOG(&debug_message);
		}

		if (Z_TYPE(pattern) == IS_STRING && Z_STRLEN(pattern) > 3 && Z_STRVAL(pattern)[1] == '^') {
			if (zend_is_true(&case_sensitive)) {
				PHALCON_CONCAT_VS(&case_pattern, &pattern, "i");
				ZVAL_NULL(&matches);
				ZVAL_MAKE_REF(&matches);
				RETURN_ON_FAILURE(phalcon_preg_match(&route_found, &case_pattern, &handled_uri, &matches));
				ZVAL_UNREF(&matches);
			} else {
				ZVAL_NULL(&matches);
				ZVAL_MAKE_REF(&matches);
				RETURN_ON_FAILURE(phalcon_preg_match(&route_found, &pattern, &handled_uri, &matches));
				ZVAL_UNREF(&matches);
			}
		} else {
			ZVAL_BOOL(&route_found, phalcon_comparestr(&pattern, &handled_uri, &case_sensitive));
		}

		/**
		 * Check for beforeMatch conditions
		 */
		if (zend_is_true(&route_found)) {
			PHALCON_STR(&event_name, "router:matchedRoute");

			PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name, route);

			PHALCON_CALL_METHODW(&before_match, route, "getbeforematch");
			if (Z_TYPE(before_match) != IS_NULL) {
				/**
				 * Check first if the callback is callable
				 */
				if (!phalcon_is_callable(&before_match)) {
					PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_router_exception_ce, "Before-Match callback is not callable in matched route");
					return;
				}

				/**
				 * Before-Match parameters
				 */
				array_init_size(&before_match_params, 3);
				phalcon_array_append(&before_match_params, &handled_uri, PH_COPY);
				phalcon_array_append(&before_match_params, route, PH_COPY);
				phalcon_array_append(&before_match_params, getThis(), PH_COPY);

				/**
				 * Call the function in the PHP userland
				 */
				PHALCON_CALL_USER_FUNC_ARRAYW(&route_found, &before_match, &before_match_params);
			}

			if (zend_is_true(&route_found)) {
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					PHALCON_STR(&debug_message, "--Found matches: ");
					PHALCON_DEBUG_LOG(&debug_message);
					PHALCON_DEBUG_LOG(&matches);
				}

				/**
				 * Start from the default paths
				 */
				PHALCON_CALL_METHODW(&paths, route, "getpaths");
				PHALCON_CPY_WRT_CTOR(&parts, &paths);

				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					PHALCON_STR(&debug_message, "--Route paths: ");
					PHALCON_DEBUG_LOG(&debug_message);
					PHALCON_DEBUG_LOG(&paths);
				}

				/**
				 * Check if the matches has variables
				 */
				if (Z_TYPE(matches) == IS_ARRAY) {
					/**
					 * Get the route converters if any
					 */
					PHALCON_CALL_METHODW(&converters, route, "getconverters");

					ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(paths), idx, str_key, position) {
						zval tmp = {}, match_position = {}, converter = {}, parameters = {}, converted_part = {};
						if (str_key) {
							ZVAL_STR(&tmp, str_key);
						} else {
							ZVAL_LONG(&tmp, idx);
						}
						if (!str_key || str_key->val[0] != '\0') {
							if (phalcon_array_isset_fetch(&match_position, &matches, position, 0)) {
								/* Check if the part has a converter */
								if (phalcon_array_isset_fetch(&converter, &converters, &tmp, 0)) {
									array_init_size(&parameters, 1);
									phalcon_array_append(&parameters, &match_position, PH_COPY);
									PHALCON_CALL_USER_FUNC_ARRAYW(&converted_part, &converter, &parameters);

									phalcon_array_update_zval(&parts, &tmp, &converted_part, PH_COPY);
								} else {
									/* Update the parts if there is no converter */
									phalcon_array_update_zval(&parts, &tmp, &match_position, PH_COPY);
								}
							} else if (phalcon_array_isset_fetch(&converter, &converters, &tmp, 0)) {
								array_init_size(&parameters, 1);
								phalcon_array_append(&parameters, position, PH_COPY);
								PHALCON_CALL_USER_FUNC_ARRAYW(&converted_part, &converter, &parameters);

								phalcon_array_update_zval(&parts, &tmp, &converted_part, PH_COPY);
							}
						}
					} ZEND_HASH_FOREACH_END();

					/**
					 * Update the matches generated by preg_match
					 */
					phalcon_update_property_zval(getThis(), SL("_matches"), &matches);
				}

				phalcon_update_property_zval(getThis(), SL("_matchedRoute"), route);
				break;
			} else if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				PHALCON_CONCAT_SV(&debug_message, "--Not Found Route: ", &pattern);
				PHALCON_DEBUG_LOG(&debug_message);
			}
		} else {
			PHALCON_STR(&event_name, "router:notMatchedRoute");

			PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name, route);
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
		phalcon_return_property(&parts, getThis(), SL("_notFoundPaths"));
		if (Z_TYPE(parts) != IS_NULL) {
			ZVAL_TRUE(&route_found);
		}
	}

	if (zend_is_true(&route_found)) {
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_STR(&debug_message, "--Route Parts: ");
			PHALCON_DEBUG_LOG(&debug_message);
			PHALCON_DEBUG_LOG(&parts);
		}

		/**
		 * Check for a namespace
		 */
		if (phalcon_array_isset_fetch_str(&namespace_name, &parts, SL("namespace"))) {
			PHALCON_CALL_METHODW(NULL, getThis(), "setnamespacename", &namespace_name);
			phalcon_array_unset_str(&parts, SL("namespace"), 0);
		} else {
			PHALCON_CALL_METHODW(&default_namespace, route, "getdefaultnamespace");
			if (Z_TYPE(default_namespace) == IS_NULL) {
				phalcon_return_property(&default_namespace, getThis(), SL("_defaultNamespace"));
			}
			PHALCON_CALL_METHODW(NULL, getThis(), "setnamespacename", &default_namespace);
		}

		/**
		 * Check for a module
		 */
		if (phalcon_array_isset_fetch_str(&module, &parts, SL("module"))) {
			PHALCON_CALL_METHODW(NULL, getThis(), "setmodulename", &module);
			phalcon_array_unset_str(&parts, SL("module"), 0);
		} else {
			PHALCON_CALL_METHODW(&default_module, route, "getdefaultmodule");
			if (Z_TYPE(default_module) == IS_NULL) {
				phalcon_return_property(&default_module, getThis(), SL("_defaultModule"));
			}
			PHALCON_CALL_METHODW(NULL, getThis(), "setmodulename", &default_module);
		}

		if (phalcon_array_isset_fetch_str(&exact, &parts, SL("\0exact"))) {
			phalcon_update_property_zval(getThis(), SL("_isExactControllerName"), &exact);
			phalcon_array_unset_str(&parts, SL("\0exact"), 0);
		} else {
			ZVAL_FALSE(&exact);
			phalcon_update_property_zval(getThis(), SL("_isExactControllerName"), &exact);
		}

		/**
		 * Check for a controller
		 */
		if (phalcon_array_isset_fetch_str(&controller, &parts, SL("controller"))) {
			PHALCON_CALL_METHODW(NULL, getThis(), "setcontrollername", &controller);
			phalcon_array_unset_str(&parts, SL("controller"), 0);
		} else {
			PHALCON_CALL_METHODW(&default_controller, route, "getdefaultcontroller");
			if (Z_TYPE(default_controller) == IS_NULL) {
				phalcon_return_property(&default_controller, getThis(), SL("_defaultController"));
			}
			PHALCON_CALL_METHODW(NULL, getThis(), "setcontrollername", &default_controller);
		}

		/**
		 * Check for an action
		 */
		if (phalcon_array_isset_fetch_str(&action, &parts, SL("action"))) {
			PHALCON_CALL_METHODW(NULL, getThis(), "setactionname", &action);
			phalcon_array_unset_str(&parts, SL("action"), 0);
		} else {
			PHALCON_CALL_METHODW(&default_action, route, "getdefaultaction");
			if (Z_TYPE(default_action) == IS_NULL) {
				phalcon_return_property(&default_action, getThis(), SL("_defaultAction"));
			}
			PHALCON_CALL_METHODW(NULL, getThis(), "setactionname", &default_action);
		}

		/**
		 * Check for parameters
		 */
		if (phalcon_array_isset_fetch_str(&params_str, &parts, SL("params"))) {
			if (Z_TYPE(params_str) == IS_STRING) {
				if (phalcon_start_with_str(&params_str, SL("/"))) {
					phalcon_substr(&str_params, &params_str, 1, 0);
				} else {
					phalcon_substr(&str_params, &params_str, 0, 0);
				}

				if (zend_is_true(&str_params)) {
					zval slash = {};
					PHALCON_STRL(&slash, "/", 1);
					phalcon_fast_explode(&params, &slash, &str_params);
				} else if (!PHALCON_IS_EMPTY(&str_params)) {
					phalcon_array_append(&params, &str_params, PH_COPY);
				}
			}

			phalcon_array_unset_str(&parts, SL("params"), PH_COPY);
		}

		if (zend_hash_num_elements(Z_ARRVAL(params))) {
			phalcon_fast_array_merge(&params_merge, &params, &parts);
		} else {
			PHALCON_CPY_WRT_CTOR(&params_merge, &parts);
		}

		if (PHALCON_IS_EMPTY(&params_merge)) {
			PHALCON_CALL_METHODW(&default_params, route, "getdefaultparams");

			if (Z_TYPE(default_params) == IS_NULL) {
				phalcon_return_property(&default_params, getThis(), SL("_defaultParams"));
			}
			PHALCON_CALL_METHODW(NULL, getThis(), "setparams", &default_params);
		} else {
			PHALCON_CALL_METHODW(NULL, getThis(), "setparams", &params_merge);
		}
	} else {
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_STR(&debug_message, "--Use Debug");
			PHALCON_DEBUG_LOG(&debug_message);
		}

		/**
		 * Use default values if the route hasn't matched
		 */
		phalcon_return_property(&default_namespace, getThis(), SL("_defaultNamespace"));
		PHALCON_CALL_METHODW(NULL, getThis(), "setnamespacename", &default_namespace);

		phalcon_return_property(&default_module, getThis(), SL("_defaultModule"));
		PHALCON_CALL_METHODW(NULL, getThis(), "setmodulename", &default_module);

		phalcon_return_property(&default_controller, getThis(), SL("_defaultController"));
		PHALCON_CALL_METHODW(NULL, getThis(), "setcontrollername", &default_controller);

		phalcon_return_property(&default_action, getThis(), SL("_defaultAction"));
		PHALCON_CALL_METHODW(NULL, getThis(), "setactionname", &default_action);

		phalcon_return_property(&default_params, getThis(), SL("_defaultParams"));
		PHALCON_CALL_METHODW(NULL, getThis(), "setparams", &default_params);
	}

	PHALCON_STR(&event_name, "router:afterCheckRoutes");
	PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);

	if (zend_is_true(&route_found)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
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
	PHALCON_CALL_METHODW(NULL, return_value, "__construct", pattern, paths, http_methods, regex);

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
	PHALCON_RETURN_CALL_METHODW(getThis(), "add", pattern, paths, &http_method);
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

	zval *group, group_routes = {}, before_match = {}, hostname = {}, converters = {}, *route, routes = {}, new_routes = {};

	phalcon_fetch_params(0, 1, 0, &group);
	PHALCON_VERIFY_CLASS_EX(group, phalcon_mvc_router_group_ce, phalcon_mvc_router_exception_ce, 0);

	PHALCON_CALL_METHODW(&group_routes, group, "getroutes");
	if (Z_TYPE(group_routes) != IS_ARRAY || !zend_hash_num_elements(Z_ARRVAL(group_routes))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_router_exception_ce, "The group of routes does not contain any routes");
		return;
	}

	/* Get the before-match condition */
	PHALCON_CALL_METHODW(&before_match, group, "getbeforematch");

	/* Get the hostname restriction */
	PHALCON_CALL_METHODW(&hostname, group, "gethostname");

	/* Get converters */
	PHALCON_CALL_METHODW(&converters, group, "getconverters");

	if (Z_TYPE(before_match) != IS_NULL || Z_TYPE(hostname) != IS_NULL || Z_TYPE(converters) != IS_NULL) {
		int has_before_match = (Z_TYPE(before_match) != IS_NULL);
		int has_hostname     = (Z_TYPE(hostname) != IS_NULL);
		int has_converters   = (Z_TYPE(converters) != IS_NULL);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(group_routes), route) {
			if (has_before_match) {
				PHALCON_CALL_METHODW(NULL, route, "beforematch", &before_match);
			}

			if (has_hostname) {
				PHALCON_CALL_METHODW(NULL, route, "sethostname", &hostname);
			}

			if (has_converters) {
				zend_hash_apply_with_arguments(Z_ARRVAL(converters), phalcon_router_call_convert, 1, route);
			}
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_read_property(&routes, getThis(), SL("_routes"), PH_NOISY);
	if (Z_TYPE(routes) == IS_ARRAY) {
		phalcon_fast_array_merge(&new_routes, &routes, &group_routes);
		phalcon_update_property_zval(getThis(), SL("_routes"), &new_routes);
	} else {
		phalcon_update_property_zval(getThis(), SL("_routes"), &group_routes);
	}

	RETURN_THISW();
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
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_router_exception_ce, "The not-found paths must be an array or string");
			return;
		}
	}
	phalcon_update_property_zval(getThis(), SL("_notFoundPaths"), paths);

	RETURN_THISW();
}

/**
 * Removes all the pre-defined routes
 */
PHP_METHOD(Phalcon_Mvc_Router, clear){

	phalcon_update_property_empty_array(getThis(), SL("_routes"));
	phalcon_update_property_empty_array(getThis(), SL("_routesNameLookup"));

}

/**
 * Sets the name of the namespace
 *
 * @param string $namespaceName
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setNamespaceName){

	zval *namespace_name;

	phalcon_fetch_params(0, 1, 0, &namespace_name);

	phalcon_update_property_zval(getThis(), SL("_namespace"), namespace_name);
	RETURN_THISW();
}

/**
 * Returns the processed namespace name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getNamespaceName){


	RETURN_MEMBER(getThis(), "_namespace");
}

/**
 * Sets the name of the module
 *
 * @param string $moduleName
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setModuleName){

	zval *module_name;

	phalcon_fetch_params(0, 1, 0, &module_name);

	phalcon_update_property_zval(getThis(), SL("_module"), module_name);
	RETURN_THISW();
}

/**
 * Returns the processed module name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getModuleName){


	RETURN_MEMBER(getThis(), "_module");
}

/**
 * Sets the name of the controller
 *
 * @param string $controllerName
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setControllerName){

	zval *controller_name;

	phalcon_fetch_params(0, 1, 0, &controller_name);

	phalcon_update_property_zval(getThis(), SL("_controller"), controller_name);
	RETURN_THISW();
}

/**
 * Returns the processed controller name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getControllerName){


	RETURN_MEMBER(getThis(), "_controller");
}

/**
 * Sets the name of the action
 *
 * @param string $actionName
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setActionName){

	zval *action_name;

	phalcon_fetch_params(0, 1, 0, &action_name);

	phalcon_update_property_zval(getThis(), SL("_action"), action_name);
	RETURN_THISW();
}

/**
 * Returns the processed action name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Router, getActionName){


	RETURN_MEMBER(getThis(), "_action");
}

/**
 * Sets the params
 *
 * @param string $params
 * @return Phalcon\Mvc\Router
 */
PHP_METHOD(Phalcon_Mvc_Router, setParams){

	zval *params;

	phalcon_fetch_params(0, 1, 0, &params);

	phalcon_update_property_zval(getThis(), SL("_params"), params);
	RETURN_THISW();
}

/**
 * Returns the processed parameters
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Router, getParams){


	RETURN_MEMBER(getThis(), "_params");
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

	phalcon_read_property(&routes, getThis(), SL("_routes"), PH_NOISY);
	if (Z_TYPE(routes) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(routes), route) {
			PHALCON_CALL_METHODW(&route_id, route, "getrouteid");
			if (phalcon_is_equal(&route_id, id)) {
				RETURN_CTORW(route);
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

	phalcon_read_property(&routes_name_lookup, getThis(), SL("_routesNameLookup"), PH_NOISY);
	if (PHALCON_IS_NOT_EMPTY(name) && (route = zend_hash_find(Z_ARRVAL(routes_name_lookup), Z_STR_P(name))) != NULL) {
		RETURN_CTORW(route);
	}

	phalcon_read_property(&routes, getThis(), SL("_routes"), PH_NOISY);
	if (Z_TYPE(routes) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(routes), route) {
			zval route_name = {};
			PHALCON_CALL_METHODW(&route_name, route, "getname");
			convert_to_string(&route_name);
			if (PHALCON_IS_NOT_EMPTY(&route_name)) {
				phalcon_update_property_array_string(getThis(), SL("_routesNameLookup"), Z_STR(route_name), route);
			}

			if (phalcon_is_equal(&route_name, name)) {
				RETURN_CTORW(route);
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
