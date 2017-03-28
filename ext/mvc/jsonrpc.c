
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
  |          Antonio Lopez <alantonilopez@gmail.com>                       |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "mvc/jsonrpc.h"
#include "mvc/jsonrpc/exception.h"
#include "mvc/dispatcherinterface.h"
#include "mvc/../dispatcherinterface.h"
#include "mvc/routerinterface.h"
#include "http/requestinterface.h"
#include "mvc/urlinterface.h"
#include "di/injectable.h"
#include "diinterface.h"
#include "events/managerinterface.h"
#include "http/responseinterface.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/file.h"
#include "kernel/require.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\JsonRpc
 *
 * This component encapsulates all the complex operations behind instantiating every component
 * needed and integrating it with the rest to allow the MVC pattern to operate as desired.
 *
 *<code>
 *
 *	$jsonrpc = new \Phalcon\Mvc\JsonRpc($di);
 *	echo $jsonrpc->handle()->getContent();
 *
 *</code>
 */
zend_class_entry *phalcon_mvc_jsonrpc_ce;

PHP_METHOD(Phalcon_Mvc_JsonRpc, __construct);
PHP_METHOD(Phalcon_Mvc_JsonRpc, registerModules);
PHP_METHOD(Phalcon_Mvc_JsonRpc, getModules);
PHP_METHOD(Phalcon_Mvc_JsonRpc, setDefaultModule);
PHP_METHOD(Phalcon_Mvc_JsonRpc, getDefaultModule);
PHP_METHOD(Phalcon_Mvc_JsonRpc, handle);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_jsonrpc___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, dependencyInjector)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_jsonrpc_registermodules, 0, 0, 1)
	ZEND_ARG_INFO(0, modules)
	ZEND_ARG_INFO(0, merge)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_jsonrpc_setdefaultmodule, 0, 0, 1)
	ZEND_ARG_INFO(0, defaultModule)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_jsonrpc_method_entry[] = {
	PHP_ME(Phalcon_Mvc_JsonRpc, __construct, arginfo_phalcon_mvc_jsonrpc___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_JsonRpc, registerModules, arginfo_phalcon_mvc_jsonrpc_registermodules, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_JsonRpc, getModules, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_JsonRpc, setDefaultModule, arginfo_phalcon_mvc_jsonrpc_setdefaultmodule, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_JsonRpc, getDefaultModule, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_JsonRpc, handle, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\JsonRpc initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_JsonRpc){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, JsonRpc, mvc_jsonrpc, phalcon_di_injectable_ce, phalcon_mvc_jsonrpc_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_jsonrpc_ce, SL("_defaultModule"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_jsonrpc_ce, SL("_modules"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_jsonrpc_ce, SL("_moduleObject"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\JsonRpc
 *
 * @param Phalcon\Di $dependencyInjector
 */
PHP_METHOD(Phalcon_Mvc_JsonRpc, __construct){

	zval *dependency_injector = NULL;

	phalcon_fetch_params(0, 0, 1, &dependency_injector);

	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHOD(NULL, getThis(), "setdi", dependency_injector);
	}
}

/**
 * Register an array of modules present in the jsonrpc
 *
 *<code>
 *	$this->registerModules(array(
 *		'frontend' => array(
 *			'className' => 'Multiple\Frontend\Module',
 *			'path' => '../apps/frontend/Module.php'
 *		),
 *		'backend' => array(
 *			'className' => 'Multiple\Backend\Module',
 *			'path' => '../apps/backend/Module.php'
 *		)
 *	));
 *</code>
 *
 * @param array $modules
 * @param boolean $merge
 * @param Phalcon\Mvc\JsonRpc
 */
PHP_METHOD(Phalcon_Mvc_JsonRpc, registerModules){

	zval *modules, *merge = NULL, registered_modules = {}, merged_modules = {};

	phalcon_fetch_params(0, 1, 1, &modules, &merge);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(modules) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_jsonrpc_exception_ce, "Modules must be an Array");
		return;
	}
	if (PHALCON_IS_FALSE(merge)) {
		phalcon_update_property_zval(getThis(), SL("_modules"), modules);
	} else {
		phalcon_read_property(&registered_modules, getThis(), SL("_modules"), PH_NOISY);
		if (Z_TYPE(registered_modules) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_modules, &registered_modules, modules);
		} else {
			ZVAL_COPY_VALUE(&merged_modules, modules);
		}

		phalcon_update_property_zval(getThis(), SL("_modules"), &merged_modules);
	}

	RETURN_THIS();
}

/**
 * Return the modules registered in the jsonrpc
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_JsonRpc, getModules){


	RETURN_MEMBER(getThis(), "_modules");
}

/**
 * Sets the module name to be used if the router doesn't return a valid module
 *
 * @param string $defaultModule
 * @return Phalcon\Mvc\JsonRpc
 */
PHP_METHOD(Phalcon_Mvc_JsonRpc, setDefaultModule){

	zval *default_module;

	phalcon_fetch_params(0, 1, 0, &default_module);

	phalcon_update_property_zval(getThis(), SL("_defaultModule"), default_module);
	RETURN_THIS();
}

/**
 * Returns the default module name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_JsonRpc, getDefaultModule){


	RETURN_MEMBER(getThis(), "_defaultModule");
}

static int phalcon_mvc_jsonrpc_fire_event(zval *mgr, const char *event, zval *this_ptr, zval *params)
{
	zval event_name = {}, status = {};
	uint params_cnt = 2 + (params != NULL ? 1 : 0);
	zval *p[3];
	if (mgr && Z_TYPE_P(mgr) == IS_OBJECT) {

		ZVAL_STRING(&event_name, event);

		p[0] = &event_name;
		p[1] = this_ptr;
		p[2] = params;

		if (FAILURE == phalcon_call_method(&status, mgr, "fire", params_cnt, p)) {
			return FAILURE;
		}

		return zend_is_true(&status) ? SUCCESS : FAILURE;
	}

	return SUCCESS;
}

/**
 * Handles a MVC request
 *
 * @param string $uri
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Mvc_JsonRpc, handle){

	zval dependency_injector = {}, events_manager = {}, service = {}, request = {}, json = {}, data = {}, response = {}, jsonrpc_message = {}, jsonrpc_error = {}, jsonrpc_method = {}, jsonrpc_params = {};
	zval url = {}, uri = {}, router = {}, module_name = {}, modules = {}, module = {}, class_name = {}, path = {}, module_object = {}, module_params = {}, status = {}, namespace_name = {}, controller_name = {}, action_name = {};
	zval params = {}, exact = {}, dispatcher = {}, controller = {}, jsonrpc_result = {}, jsonrpc_id = {};

	PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_jsonrpc_exception_ce, "A dependency injection object is required to access internal services");
		return;
	}

	phalcon_read_property(&events_manager, getThis(), SL("_eventsManager"), PH_NOISY);
	PHALCON_VERIFY_INTERFACE_OR_NULL_EX(&events_manager, phalcon_events_managerinterface_ce, phalcon_mvc_jsonrpc_exception_ce);

	/* Call boot event, this allows the developer to perform initialization actions */
	if (FAILURE == phalcon_mvc_jsonrpc_fire_event(&events_manager, "jsonrpc:boot", getThis(), NULL)) {
		RETURN_FALSE;
	}

	/* Deserializer Json */

	ZVAL_STRING(&service, ISV(request));

	PHALCON_CALL_METHOD(&request, &dependency_injector, "getshared", &service);
	PHALCON_VERIFY_INTERFACE(&request, phalcon_http_requestinterface_ce);

	PHALCON_CALL_METHOD(&json, &request, "getrawbody");
	PHALCON_CALL_FUNCTION(&data, "json_decode", &json, &PHALCON_GLOBAL(z_true));

	ZVAL_STRING(&service, ISV(response));

	PHALCON_CALL_METHOD(&response, &dependency_injector, "getshared", &service);
	PHALCON_VERIFY_INTERFACE(&response, phalcon_http_responseinterface_ce);

	array_init(&jsonrpc_message);
	array_init(&jsonrpc_error);

	if (PHALCON_IS_EMPTY(&data)) {
		phalcon_array_update_str_long(&jsonrpc_error, SL("code"), __LINE__, 0);
		phalcon_array_update_str_str(&jsonrpc_error, SL("message"), SL("Parse error"), PH_COPY);
	} else if (Z_TYPE(data) != IS_ARRAY) {
		phalcon_array_update_str_long(&jsonrpc_error, SL("code"), __LINE__, 0);
		phalcon_array_update_str_str(&jsonrpc_error, SL("message"), SL("Parse error"), PH_COPY);
	} else if (!phalcon_array_isset_str(&data, SL("jsonrpc"))) {
			phalcon_array_update_str_long(&jsonrpc_error, SL("code"), __LINE__, 0);
			phalcon_array_update_str_str(&jsonrpc_error, SL("message"), SL("Invalid Request"), PH_COPY);
	} else if (!phalcon_array_isset_fetch_str(&jsonrpc_method, &data, SL("method"))) {
			phalcon_array_update_str_long(&jsonrpc_error, SL("code"), __LINE__, 0);
			phalcon_array_update_str_str(&jsonrpc_error, SL("message"), SL("Invalid Request"), PH_COPY);
	} else {
		if (!phalcon_array_isset_fetch_str(&jsonrpc_params, &data, SL("params"))) {
			array_init(&jsonrpc_params);
		}

		ZVAL_STRING(&service, ISV(url));
		PHALCON_CALL_METHOD(&url, &dependency_injector, "getshared", &service);
		PHALCON_VERIFY_INTERFACE(&url, phalcon_mvc_urlinterface_ce);

		PHALCON_CALL_METHOD(&uri, &url, "get", &jsonrpc_method);

		ZVAL_STRING(&service, ISV(router));
		PHALCON_CALL_METHOD(&router, &dependency_injector, "getshared", &service);
		PHALCON_VERIFY_INTERFACE(&router, phalcon_mvc_routerinterface_ce);

		/* Handle the URI pattern (if any) */
		PHALCON_CALL_METHOD(NULL, &router, "handle", &uri);

		/* Load module config */
		PHALCON_CALL_METHOD(&module_name, &router, "getmodulename");

		/* If the router doesn't return a valid module we use the default module */
		if (!zend_is_true(&module_name)) {
			phalcon_return_property(&module_name, getThis(), SL("_defaultModule"));
		}

		/**
		 * Process the module definition
		 */
		if (zend_is_true(&module_name)) {
			if (FAILURE == phalcon_mvc_jsonrpc_fire_event(&events_manager, "jsonrpc:beforeStartModule", getThis(), &module_name)) {
				RETURN_FALSE;
			}

			/**
			 * Check if the module passed by the router is registered in the modules container
			 */
			phalcon_read_property(&modules, getThis(), SL("_modules"), PH_NOISY);
			if (!phalcon_array_isset_fetch(&module, &modules, &module_name, 0)) {
				convert_to_string(&module_name);
				zend_throw_exception_ex(phalcon_mvc_jsonrpc_exception_ce, 0, "Module %s is not registered in the jsonrpc container", Z_STRVAL(module_name));
				return;
			}

			/**
			 * A module definition must be an array or an object
			 */
			if (Z_TYPE(module) != IS_ARRAY && Z_TYPE(module) != IS_OBJECT) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_jsonrpc_exception_ce, "Invalid module definition");
				return;
			}

			/* An array module definition contains a path to a module definition class */
			if (Z_TYPE(module) == IS_ARRAY) {
				/* Class name used to load the module definition */
				if (!phalcon_array_isset_fetch_str(&class_name, &module, SL("className"))) {
					ZVAL_STRING(&class_name, "Module");
				}

				/* If the developer has specified a path, try to include the file */
				if (phalcon_array_isset_fetch_str(&path, &module, SL("path"))) {
					convert_to_string_ex(&path);
					if (Z_TYPE(class_name) != IS_STRING || phalcon_class_exists(&class_name, 0) == NULL) {
						if (phalcon_file_exists(&path) == SUCCESS) {
							RETURN_ON_FAILURE(phalcon_require(Z_STRVAL(path)));
						} else {
							zend_throw_exception_ex(phalcon_mvc_jsonrpc_exception_ce, 0, "Module definition path '%s' does not exist", Z_STRVAL(path));
							return;
						}
					}
				}

				PHALCON_CALL_METHOD(&module_object, &dependency_injector, "get", &class_name);

				/**
				 * 'registerAutoloaders' and 'registerServices' are automatically called
				 */
				PHALCON_CALL_METHOD(NULL, &module_object, "registerautoloaders", &dependency_injector);
				PHALCON_CALL_METHOD(NULL, &module_object, "registerservices", &dependency_injector);
			} else if (Z_TYPE(module) == IS_OBJECT && instanceof_function(Z_OBJCE(module), zend_ce_closure)) {
				/* A module definition object, can be a Closure instance */
				array_init_size(&module_params, 1);
				phalcon_array_append(&module_params, &dependency_injector, PH_COPY);

				PHALCON_CALL_USER_FUNC_ARRAY(&status, &module, &module_params);
			} else {
				PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_jsonrpc_exception_ce, "Invalid module definition");
				return;
			}

			/* Calling afterStartModule event */
			phalcon_update_property_zval(getThis(), SL("_moduleObject"), &module_object);
			if (FAILURE == phalcon_mvc_jsonrpc_fire_event(&events_manager, "jsonrpc:afterStartModule", getThis(), &module_name)) {
				RETURN_FALSE;
			}
		}

		/* We get the parameters from the router and assign them to the dispatcher */
		PHALCON_CALL_METHOD(&module_name, &router, "getmodulename");
		PHALCON_CALL_METHOD(&namespace_name, &router, "getnamespacename");
		PHALCON_CALL_METHOD(&controller_name, &router, "getcontrollername");
		PHALCON_CALL_METHOD(&action_name, &router, "getactionname");
		PHALCON_CALL_METHOD(&params, &router, "getparams");
		PHALCON_CALL_METHOD(&exact, &router, "isexactcontrollername");

		ZVAL_STRING(&service, ISV(dispatcher));

		PHALCON_CALL_METHOD(&dispatcher, &dependency_injector, "getshared", &service);
		PHALCON_VERIFY_INTERFACE(&dispatcher, phalcon_dispatcherinterface_ce);

		/* Assign the values passed from the router */
		PHALCON_CALL_METHOD(NULL, &dispatcher, "setmodulename", &module_name);
		PHALCON_CALL_METHOD(NULL, &dispatcher, "setnamespacename", &namespace_name);
		PHALCON_CALL_METHOD(NULL, &dispatcher, "setcontrollername", &controller_name, &exact);
		PHALCON_CALL_METHOD(NULL, &dispatcher, "setactionname", &action_name);
		PHALCON_CALL_METHOD(NULL, &dispatcher, "setparams", &jsonrpc_params);

		/* Calling beforeHandleRequest */
		RETURN_ON_FAILURE(phalcon_mvc_jsonrpc_fire_event(&events_manager, "jsonrpc:beforeHandleRequest", getThis(), &dispatcher));

		/* The dispatcher must return an object */
		PHALCON_CALL_METHOD(&controller, &dispatcher, "dispatch");

		/* Get the latest value returned by an action */
		PHALCON_CALL_METHOD(&jsonrpc_result, &dispatcher, "getreturnedvalue");
	}

	/* Calling afterHandleRequest */
	if (FAILURE == phalcon_mvc_jsonrpc_fire_event(&events_manager, "jsonrpc:afterHandleRequest", getThis(), &controller) && EG(exception)) {
		return;
	}

	phalcon_array_update_str_str(&jsonrpc_message, SL("jsonrpc"), SL("2.0"), PH_COPY);

	if (PHALCON_IS_NOT_EMPTY(&jsonrpc_error)) {
		phalcon_array_update_str(&jsonrpc_message, SL("error"), &jsonrpc_error, PH_COPY);
	}

	if (Z_TYPE(jsonrpc_result) > IS_NULL) {
		phalcon_array_update_str(&jsonrpc_message, SL("result"), &jsonrpc_result, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&jsonrpc_id, &data, SL("id"))) {
		phalcon_array_update_str(&jsonrpc_message, SL("id"), &jsonrpc_id, PH_COPY);
	} else {
		phalcon_array_update_str(&jsonrpc_message, SL("id"), &PHALCON_GLOBAL(z_null), PH_COPY);
	}

	PHALCON_CALL_METHOD(NULL, &response, "setjsoncontent", &jsonrpc_message);


	/* Calling beforeSendResponse */
	if (FAILURE == phalcon_mvc_jsonrpc_fire_event(&events_manager, "jsonrpc:beforeSendResponse", getThis(), &response) && EG(exception)) {
		return;
	}

	/* Headers are automatically sent */
	PHALCON_CALL_METHOD(NULL, &response, "sendheaders");

	/* Cookies are automatically sent */
	PHALCON_CALL_METHOD(NULL, &response, "sendcookies");

	/* Return the response */
	RETURN_CTOR(&response);
}
