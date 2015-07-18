
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

#include "mvc/router/annotations.h"
#include "mvc/router.h"
#include "mvc/router/exception.h"
#include "annotations/adapterinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/hash.h"
#include "kernel/operators.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Router\Annotations
 *
 * A router that reads routes annotations from classes/resources
 *
 *<code>
 * $di['router'] = function() {
 *
 *		//Use the annotations router
 *		$router = new \Phalcon\Mvc\Router\Annotations(false);
 *
 *		//This will do the same as above but only if the handled uri starts with /robots
 * 		$router->addResource('Robots', '/robots');
 *
 * 		return $router;
 *	};
 *</code>
 */
zend_class_entry *phalcon_mvc_router_annotations_ce;

PHP_METHOD(Phalcon_Mvc_Router_Annotations, addResource);
PHP_METHOD(Phalcon_Mvc_Router_Annotations, addModuleResource);
PHP_METHOD(Phalcon_Mvc_Router_Annotations, handle);
PHP_METHOD(Phalcon_Mvc_Router_Annotations, processControllerAnnotation);
PHP_METHOD(Phalcon_Mvc_Router_Annotations, processActionAnnotation);
PHP_METHOD(Phalcon_Mvc_Router_Annotations, setControllerSuffix);
PHP_METHOD(Phalcon_Mvc_Router_Annotations, setActionSuffix);
PHP_METHOD(Phalcon_Mvc_Router_Annotations, getResources);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_annotations_addresource, 0, 0, 1)
	ZEND_ARG_INFO(0, handler)
	ZEND_ARG_INFO(0, prefix)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_annotations_addmoduleresource, 0, 0, 2)
	ZEND_ARG_INFO(0, module)
	ZEND_ARG_INFO(0, handler)
	ZEND_ARG_INFO(0, prefix)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_annotations_handle, 0, 0, 0)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_annotations_processcontrollerannotation, 0, 0, 2)
	ZEND_ARG_INFO(0, handler)
	ZEND_ARG_INFO(0, annotation)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_annotations_processactionannotation, 0, 0, 5)
	ZEND_ARG_INFO(0, module)
	ZEND_ARG_INFO(0, namespace)
	ZEND_ARG_INFO(0, controller)
	ZEND_ARG_INFO(0, action)
	ZEND_ARG_INFO(0, annotation)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_annotations_setcontrollersuffix, 0, 0, 1)
	ZEND_ARG_INFO(0, controllerSuffix)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_router_annotations_setactionsuffix, 0, 0, 1)
	ZEND_ARG_INFO(0, actionSuffix)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_router_annotations_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Router_Annotations, addResource, arginfo_phalcon_mvc_router_annotations_addresource, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Annotations, addModuleResource, arginfo_phalcon_mvc_router_annotations_addmoduleresource, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Annotations, handle, arginfo_phalcon_mvc_router_annotations_handle, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Annotations, processControllerAnnotation, arginfo_phalcon_mvc_router_annotations_processcontrollerannotation, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Annotations, processActionAnnotation, arginfo_phalcon_mvc_router_annotations_processactionannotation, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Annotations, setControllerSuffix, arginfo_phalcon_mvc_router_annotations_setcontrollersuffix, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Annotations, setActionSuffix, arginfo_phalcon_mvc_router_annotations_setactionsuffix, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Router_Annotations, getResources, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Router\Annotations initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Router_Annotations){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Router, Annotations, mvc_router_annotations, phalcon_mvc_router_ce, phalcon_mvc_router_annotations_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_router_annotations_ce, SL("_handlers"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_router_annotations_ce, SL("_processed"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_mvc_router_annotations_ce, SL("_controllerSuffix"), "Controller", ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_mvc_router_annotations_ce, SL("_actionSuffix"), "Action", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_router_annotations_ce, SL("_routePrefix"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Adds a resource to the annotations handler
 * A resource is a class that contains routing annotations
 *
 * @param string $handler
 * @param string $prefix
 * @return Phalcon\Mvc\Router\Annotations
 */
PHP_METHOD(Phalcon_Mvc_Router_Annotations, addResource){

	zval *handler, *prefix = NULL, *scope;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 1, &handler, &prefix);

	if (!prefix) {
		prefix = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(handler) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "The handler must be a class name");
		return;
	}

	PHALCON_INIT_VAR(scope);
	array_init_size(scope, 2);
	phalcon_array_append(scope, prefix, PH_COPY);
	phalcon_array_append(scope, handler, PH_COPY);
	phalcon_update_property_array_append(getThis(), SL("_handlers"), scope);
	phalcon_update_property_this(getThis(), SL("_processed"), &PHALCON_GLOBAL(z_false));

	RETURN_THIS();
}

/**
 * Adds a resource to the annotations handler
 * A resource is a class that contains routing annotations
 * The class is located in a module
 *
 * @param string $module
 * @param string $handler
 * @param string $prefix
 * @return Phalcon\Mvc\Router\Annotations
 */
PHP_METHOD(Phalcon_Mvc_Router_Annotations, addModuleResource){

	zval *module, *handler, *prefix = NULL, *scope;

	phalcon_fetch_params(0, 2, 1, &module, &handler, &prefix);

	if (!prefix) {
		prefix = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(module) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_router_exception_ce, "The module is not a valid string");
		return;
	}
	if (Z_TYPE_P(handler) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_router_exception_ce, "The handler must be a class name");
		return;
	}

	PHALCON_ALLOC_INIT_ZVAL(scope);
	array_init_size(scope, 3);
	phalcon_array_append(scope, prefix, PH_COPY);
	phalcon_array_append(scope, handler, PH_COPY);
	phalcon_array_append(scope, module, PH_COPY);
	phalcon_update_property_array_append(getThis(), SL("_handlers"), scope);
	zval_ptr_dtor(scope);

	phalcon_update_property_this(getThis(), SL("_processed"), &PHALCON_GLOBAL(z_false));

	RETURN_THISW();
}

/**
 * Produce the routing parameters from the rewrite information
 *
 * @param string $uri
 */
PHP_METHOD(Phalcon_Mvc_Router_Annotations, handle){

	zval *uri = NULL, *real_uri = NULL, *processed, *annotations_service = NULL;
	zval *handlers, *controller_suffix, *scope = NULL, *prefix = NULL;
	zval *dependency_injector = NULL, *service = NULL, *handler = NULL;
	zval *controller_name = NULL;
	zval *namespace_name = NULL, *module_name = NULL, *suffixed = NULL;
	zval *handler_annotations = NULL, *class_annotations = NULL;
	zval *annotations = NULL, *annotation = NULL, *method_annotations = NULL;
	zval *collection = NULL;
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 0, 1, &uri);

	if (!uri) {
		uri = &PHALCON_GLOBAL(z_null);
	}

	if (!zend_is_true(uri)) {
		/** 
		 * If 'uri' isn't passed as parameter it reads $_GET['_url']
		 */
		PHALCON_CALL_METHOD(&real_uri, getThis(), "getrewriteuri");
	} else {
		PHALCON_CPY_WRT(real_uri, uri);
	}

	processed = phalcon_read_property(getThis(), SL("_processed"), PH_NOISY);
	if (!zend_is_true(processed)) {

		PHALCON_INIT_VAR(annotations_service);

		handlers = phalcon_read_property(getThis(), SL("_handlers"), PH_NOISY);
		if (Z_TYPE_P(handlers) == IS_ARRAY) {
			controller_suffix = phalcon_read_property(getThis(), SL("_controllerSuffix"), PH_NOISY);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(handlers), scope) {
				if (Z_TYPE_P(scope) == IS_ARRAY) { 

					/** 
					 * A prefix (if any) must be in position 0
					 */
					PHALCON_OBS_NVAR(prefix);
					phalcon_array_fetch_long(&prefix, scope, 0, PH_NOISY);
					if (Z_TYPE_P(prefix) == IS_STRING) {
						if (!phalcon_start_with(real_uri, prefix, NULL)) {
							continue;
						}
					}

					if (Z_TYPE_P(annotations_service) != IS_OBJECT) {
						dependency_injector = phalcon_read_property(getThis(), SL("_dependencyInjector"), PH_NOISY);
						if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
							PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_router_exception_ce, "A dependency injection container is required to access the 'annotations' service");
							return;
						}

						PHALCON_INIT_NVAR(service);
						ZVAL_STRING(service, "annotations");

						PHALCON_CALL_METHOD(&annotations_service, dependency_injector, "getshared", service);
						PHALCON_VERIFY_INTERFACE(annotations_service, phalcon_annotations_adapterinterface_ce);
					}

					/** 
					 * The controller must be in position 1
					 */
					PHALCON_OBS_NVAR(handler);
					phalcon_array_fetch_long(&handler, scope, 1, PH_NOISY);
					if (phalcon_memnstr_str(handler, SL("\\"))) {
						/** 
						 * Extract the real class name from the namespaced class
						 */
						PHALCON_INIT_NVAR(controller_name);
						phalcon_get_class_ns(controller_name, handler, 0);

						/** 
						 * Extract the namespace from the namespaced class
						 */
						PHALCON_INIT_NVAR(namespace_name);
						phalcon_get_ns_class(namespace_name, handler, 0);
					} else {
						PHALCON_CPY_WRT(controller_name, handler);

						PHALCON_INIT_NVAR(namespace_name);
					}

					phalcon_update_property_null(getThis(), SL("_routePrefix"));

					/** 
					 * Check if the scope has a module associated
					 */
					if (phalcon_array_isset_long(scope, 2)) {
						PHALCON_OBS_NVAR(module_name);
						phalcon_array_fetch_long(&module_name, scope, 2, PH_NOISY);
					} else {
						PHALCON_INIT_NVAR(module_name);
					}

					PHALCON_INIT_NVAR(suffixed);
					PHALCON_CONCAT_VV(suffixed, handler, controller_suffix);

					/** 
					 * Get the annotations from the class
					 */
					PHALCON_CALL_METHOD(&handler_annotations, annotations_service, "get", suffixed);

					/** 
					 * Process class annotations
					 */
					PHALCON_CALL_METHOD(&class_annotations, handler_annotations, "getclassannotations");
					if (Z_TYPE_P(class_annotations) == IS_OBJECT) {

						/** 
						 * Process class annotations
						 */
						PHALCON_CALL_METHOD(&annotations, class_annotations, "getannotations");
						if (Z_TYPE_P(annotations) == IS_ARRAY) {
							ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(annotations), annotation) {
								PHALCON_CALL_METHOD(NULL, getThis(), "processcontrollerannotation", controller_name, annotation);
							} ZEND_HASH_FOREACH_END();

						}
					}

					/** 
					 * Process method annotations
					 */
					PHALCON_CALL_METHOD(&method_annotations, handler_annotations, "getmethodsannotations");
					if (Z_TYPE_P(method_annotations) == IS_ARRAY) {
						ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(method_annotations), idx, str_key, collection) {
							zval method;
							if (str_key) {
								ZVAL_STR(&method, str_key);
							} else {
								ZVAL_LONG(&method, idx);
							}
							if (Z_TYPE_P(collection) == IS_OBJECT) {
								PHALCON_CALL_METHOD(&annotations, collection, "getannotations");
								ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(annotations), annotation) {
									PHALCON_CALL_METHOD(NULL, getThis(), "processactionannotation", module_name, namespace_name, controller_name, &method, annotation);
								} ZEND_HASH_FOREACH_END();

							}
						} ZEND_HASH_FOREACH_END();
					}
				}
			} ZEND_HASH_FOREACH_END();

		}

		phalcon_update_property_bool(getThis(), SL("_processed"), 1);
	}

	/** 
	 * Call the parent handle method()
	 */
	PHALCON_CALL_PARENT(NULL, phalcon_mvc_router_annotations_ce, getThis(), "handle", real_uri);

	PHALCON_MM_RESTORE();
}

/**
 * Checks for annotations in the controller docblock
 *
 * @param string $handler
 * @param Phalcon\Annotations\AdapterInterface
 */
PHP_METHOD(Phalcon_Mvc_Router_Annotations, processControllerAnnotation){

	zval *handler, *annotation, *name = NULL, *position, *value = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 2, 0, &handler, &annotation);

	PHALCON_CALL_METHOD(&name, annotation, "getname");

	/** 
	 * @RoutePrefix add a prefix for all the routes defined in the model
	 */
	if (PHALCON_IS_STRING(name, "RoutePrefix")) {
		position = &PHALCON_GLOBAL(z_zero);

		PHALCON_CALL_METHOD(&value, annotation, "getargument", position);
		phalcon_update_property_this(getThis(), SL("_routePrefix"), value);
	}

	PHALCON_MM_RESTORE();
}

/**
 * Checks for annotations in the public methods of the controller
 *
 * @param string $module
 * @param string $namespace
 * @param string $controller
 * @param string $action
 * @param Phalcon\Annotations\Annotation $annotation
 */
PHP_METHOD(Phalcon_Mvc_Router_Annotations, processActionAnnotation){

	zval *module, *namespace, *controller, *action;
	zval *annotation, *methods = NULL, *name = NULL;
	zval *empty_str, *real_action_name, *action_name;
	zval *parameter = NULL, *paths = NULL, *position;
	zval *value = NULL, *uri = NULL, *route = NULL, *converts = NULL, *convert = NULL;
	zval *route_name = NULL;
	zend_string *str_key;
	ulong idx;
	int is_route;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 5, 0, &module, &namespace, &controller, &action, &annotation);

	PHALCON_CALL_METHOD(&name, annotation, "getname");

	/* Find if the route is for adding routes */
	PHALCON_INIT_VAR(methods);
	if (PHALCON_IS_STRING(name, "Route")) {
		is_route = 1;
	} else if (PHALCON_IS_STRING(name, "Get")) {
		is_route = 1;
		ZVAL_STR(methods, IS(GET));
	} else if (PHALCON_IS_STRING(name, "Post")) {
		is_route = 1;
		ZVAL_STR(methods, IS(POST));
	} else if (PHALCON_IS_STRING(name, "Put")) {
		is_route = 1;
		ZVAL_STR(methods, IS(PUT));
	} else if (PHALCON_IS_STRING(name, "Delete")) {
		is_route = 1;
		ZVAL_STR(methods, IS(DELETE));
	} else if (PHALCON_IS_STRING(name, "Options")) {
		is_route = 1;
		ZVAL_STR(methods, IS(OPTIONS));
	} else {
		is_route = 0;
	}

	if (is_route) {
		zval *action_suffix = phalcon_read_property(getThis(), SL("_actionSuffix"), PH_NOISY);
		zval *route_prefix  = phalcon_read_property(getThis(), SL("_routePrefix"), PH_NOISY);

		PHALCON_INIT_VAR(empty_str);
		ZVAL_EMPTY_STRING(empty_str);

		PHALCON_STR_REPLACE(&real_action_name, action_suffix, empty_str, action);

		PHALCON_INIT_VAR(action_name);
		phalcon_fast_strtolower(action_name, real_action_name);

		PHALCON_INIT_VAR(parameter);
		ZVAL_STR(parameter, IS(paths));

		/** 
		 * Check for existing paths in the annotation
		 */
		PHALCON_CALL_METHOD(&paths, annotation, "getargument", parameter);
		if (Z_TYPE_P(paths) != IS_ARRAY) { 
			PHALCON_INIT_NVAR(paths);
			array_init(paths);
		}

		/** 
		 * Update the module if any
		 */
		if (Z_TYPE_P(module) == IS_STRING) {
			phalcon_array_update_string(paths, IS(module), module, PH_COPY);
		}

		/** 
		 * Update the namespace if any
		 */
		if (Z_TYPE_P(namespace) == IS_STRING) {
			phalcon_array_update_string(paths, IS(namespace), namespace, PH_COPY);
		}

		phalcon_array_update_string(paths, IS(controller), controller, PH_COPY);
		phalcon_array_update_string(paths, IS(action), real_action_name, PH_COPY);
		phalcon_array_update_str(paths, SL("\0exact"), &PHALCON_GLOBAL(z_true), PH_COPY);

		PHALCON_INIT_VAR(position);
		ZVAL_LONG(position, 0);

		PHALCON_CALL_METHOD(&value, annotation, "getargument", position);

		/** 
		 * Create the route using the prefix
		 */
		if (Z_TYPE_P(value) != IS_NULL) {
			if (!PHALCON_IS_STRING(value, "/")) {
				PHALCON_INIT_VAR(uri);
				PHALCON_CONCAT_VV(uri, route_prefix, value);
			} else {
				if (Z_TYPE_P(route_prefix) != IS_NULL) {
					PHALCON_CPY_WRT(uri, route_prefix);
				} else {
					PHALCON_CPY_WRT(uri, value);
				}
			}
		} else {
			PHALCON_INIT_NVAR(uri);
			PHALCON_CONCAT_VV(uri, route_prefix, action_name);
		}

		/** 
		 * Add the route to the router
		 */
		PHALCON_CALL_METHOD(&route, getThis(), "add", uri, paths);
		if (Z_TYPE_P(methods) == IS_NULL) {

			PHALCON_INIT_NVAR(parameter);
			ZVAL_STRING(parameter, "methods");

			PHALCON_CALL_METHOD(&methods, annotation, "getargument", parameter);
			if (Z_TYPE_P(methods) == IS_ARRAY || Z_TYPE_P(methods) == IS_STRING) {
				PHALCON_CALL_METHOD(NULL, route, "via", methods);
			}
		} else {
			PHALCON_CALL_METHOD(NULL, route, "via", methods);
		}

		PHALCON_INIT_NVAR(parameter);
		ZVAL_STRING(parameter, "converts");

		PHALCON_CALL_METHOD(&converts, annotation, "getargument", parameter);
		if (Z_TYPE_P(converts) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(converts), idx, str_key, convert) {
				zval param;
				if (str_key) {
					ZVAL_STR(&param, str_key);
				} else {
					ZVAL_LONG(&param, idx);
				}
				PHALCON_CALL_METHOD(NULL, route, "convert", &param, convert);
			} ZEND_HASH_FOREACH_END();
		}

		PHALCON_INIT_NVAR(parameter);
		ZVAL_STRING(parameter, "conversors");

		PHALCON_CALL_METHOD(&converts, annotation, "getargument", parameter);
		if (Z_TYPE_P(converts) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(converts), idx, str_key, convert) {
				zval conversor_param;
				if (str_key) {
					ZVAL_STR(&conversor_param, str_key);
				} else {
					ZVAL_LONG(&conversor_param, idx);
				}
				PHALCON_CALL_METHOD(NULL, route, "convert", &conversor_param, convert);
			} ZEND_HASH_FOREACH_END();
		}

		PHALCON_INIT_NVAR(parameter);
		ZVAL_STR(parameter, IS(name));

		PHALCON_CALL_METHOD(&route_name, annotation, "getargument", parameter);
		if (Z_TYPE_P(route_name) == IS_STRING) {
			PHALCON_CALL_METHOD(NULL, route, "setname", route_name);
		}

		RETURN_MM_TRUE;
	}

	PHALCON_MM_RESTORE();
}

/**
 * Changes the controller class suffix
 *
 * @param string $controllerSuffix
 */
PHP_METHOD(Phalcon_Mvc_Router_Annotations, setControllerSuffix){

	zval *controller_suffix;

	phalcon_fetch_params(0, 1, 0, &controller_suffix);

	phalcon_update_property_this(getThis(), SL("_controllerSuffix"), controller_suffix);

}

/**
 * Changes the action method suffix
 *
 * @param string $actionSuffix
 */
PHP_METHOD(Phalcon_Mvc_Router_Annotations, setActionSuffix){

	zval *action_suffix;

	phalcon_fetch_params(0, 1, 0, &action_suffix);

	phalcon_update_property_this(getThis(), SL("_actionSuffix"), action_suffix);

}

/**
 * Return the registered resources
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Router_Annotations, getResources){


	RETURN_MEMBER(getThis(), "_handlers");
}
