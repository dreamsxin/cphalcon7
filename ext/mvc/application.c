
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

#include "mvc/application.h"
#include "mvc/../application.h"
#include "mvc/application/exception.h"
#include "mvc/dispatcherinterface.h"
#include "mvc/../dispatcherinterface.h"
#include "mvc/routerinterface.h"
#include "mvc/moduledefinitioninterface.h"
#include "mvc/viewinterface.h"
#include "mvc/view/modelinterface.h"
#include "di/injectable.h"
#include "diinterface.h"
#include "di.h"
#include "events/managerinterface.h"
#include "http/responseinterface.h"
#include "http/request.h"

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
#include "kernel/string.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Application
 *
 * This component encapsulates all the complex operations behind instantiating every component
 * needed and integrating it with the rest to allow the MVC pattern to operate as desired.
 *
 *<code>
 *
 * class Application extends \Phalcon\Mvc\Application
 * {
 *
 *		/\**
 *		 * Register the services here to make them general or register
 *		 * in the ModuleDefinition to make them module-specific
 *		 *\/
 *		protected function _registerServices()
 *		{
 *
 *		}
 *
 *		/\**
 *		 * This method registers all the modules in the application
 *		 *\/
 *		public function main()
 *		{
 *			$this->registerModules(array(
 *				'frontend' => array(
 *					'className' => 'Multiple\Frontend\Module',
 *					'path' => '../apps/frontend/Module.php'
 *				),
 *				'backend' => array(
 *					'className' => 'Multiple\Backend\Module',
 *					'path' => '../apps/backend/Module.php'
 *				)
 *			));
 *		}
 *	}
 *
 *	$application = new Application();
 *	$application->main();
 *
 *</code>
 */
zend_class_entry *phalcon_mvc_application_ce;

PHP_METHOD(Phalcon_Mvc_Application, __construct);
PHP_METHOD(Phalcon_Mvc_Application, useImplicitView);
PHP_METHOD(Phalcon_Mvc_Application, autoSendHeader);
PHP_METHOD(Phalcon_Mvc_Application, handle);
PHP_METHOD(Phalcon_Mvc_Application, request);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_application___construct, 0, 0, 0)
	ZEND_ARG_OBJ_INFO(0, dependencyInjector, Phalcon\\DiInterface, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_application_useimplicitview, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, implicitView, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_application_autosendheader, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, autoSendHeader, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_application_request, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, uri, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 1)
	ZEND_ARG_OBJ_INFO(0, dependencyInjector, Phalcon\\DiInterface, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_application_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Application, __construct, arginfo_phalcon_mvc_application___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Application, useImplicitView, arginfo_phalcon_mvc_application_useimplicitview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Application, autoSendHeader, arginfo_phalcon_mvc_application_autosendheader, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Application, handle, arginfo_phalcon_application_handle, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Application, request, arginfo_phalcon_mvc_application_request, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Application initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Application){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, Application, mvc_application, phalcon_application_ce, phalcon_mvc_application_method_entry, 0);

	zend_declare_property_bool(phalcon_mvc_application_ce, SL("_implicitView"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_application_ce, SL("_autoSendHeader"), 1, ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Application
 *
 * @param Phalcon\Di $dependencyInjector
 */
PHP_METHOD(Phalcon_Mvc_Application, __construct){

	zval *dependency_injector = NULL;

	phalcon_fetch_params(0, 0, 1, &dependency_injector);

	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHOD(NULL, getThis(), "setdi", dependency_injector);
	}
}

/**
 * By default. The view is implicitly buffering all the output
 * You can full disable the view component using this method
 *
 * @param boolean $implicitView
 * @return Phalcon\Mvc\Application
 */
PHP_METHOD(Phalcon_Mvc_Application, useImplicitView){

	zval *implicit_view;

	phalcon_fetch_params(0, 1, 0, &implicit_view);

	phalcon_update_property(getThis(), SL("_implicitView"), implicit_view);
	RETURN_THIS();
}

/**
 * Enable or disable sending cookies by each request
 *
 * @param boolean $autoSendHeader
 * @return Phalcon\Mvc\Application
 */
PHP_METHOD(Phalcon_Mvc_Application, autoSendHeader){

	zval *auto_send;

	phalcon_fetch_params(0, 1, 0, &auto_send);

	phalcon_update_property(getThis(), SL("_autoSendHeader"), auto_send);
	RETURN_THIS();
}

/**
 * Handles a MVC request
 *
 * @param string $uri
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Mvc_Application, handle){

	zval *uri = NULL, dependency_injector = {}, event_name = {}, status = {}, service = {}, router = {}, module_name = {};
	zval modules = {}, module = {}, module_namespace = {}, module_class = {}, class_name = {}, path = {}, module_object = {}, module_params = {};
	zval implicit_view = {}, view = {}, namespace_name = {}, controller_name = {}, action_name = {}, params = {}, exact = {};
	zval dispatcher = {}, controller = {}, possible_response = {}, returned_response = {}, response = {}, content = {}, auto_send = {};
	int f_implicit_view;

	phalcon_fetch_params(1, 0, 1, &uri);

	if (!uri) {
		uri = &PHALCON_GLOBAL(z_null);
	}

	/* Call boot event, this allows the developer to perform initialization actions */
	PHALCON_MM_ZVAL_STRING(&event_name, "application:boot");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}
	zval_ptr_dtor(&status);

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_application_exception_ce, "A dependency injection object is required to access internal services");
		return;
	}

	ZVAL_STR(&service, IS(app));
	PHALCON_MM_CALL_METHOD(NULL, &dependency_injector, "setshared", &service, getThis());

	ZVAL_STR(&service, IS(router));
	PHALCON_MM_CALL_METHOD(&router, &dependency_injector, "getshared", &service);
	PHALCON_MM_ADD_ENTRY(&router);
	PHALCON_MM_VERIFY_INTERFACE(&router, phalcon_mvc_routerinterface_ce);

	PHALCON_MM_ZVAL_STRING(&event_name, "application:beforeHandleRouter");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &router);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}
	zval_ptr_dtor(&status);

	/* Handle the URI pattern (if any) */
	PHALCON_MM_CALL_METHOD(NULL, &router, "handle", uri);

	PHALCON_MM_ZVAL_STRING(&event_name, "application:afterHandleRouter");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &router);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}
	zval_ptr_dtor(&status);

	/* Load module config */
	PHALCON_MM_CALL_METHOD(&module_name, &router, "getmodulename");
	PHALCON_MM_ADD_ENTRY(&module_name);

	/* If the router doesn't return a valid module we use the default module */
	if (!zend_is_true(&module_name)) {
		 phalcon_read_property(&module_name, getThis(), SL("_defaultModule"), PH_READONLY);
	}

	/**
	 * Process the module definition
	 */
	if (zend_is_true(&module_name)) {
		PHALCON_MM_ZVAL_STRING(&event_name, "application:beforeStartModule");
		PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &module_name);

		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
		zval_ptr_dtor(&status);

		/**
		 * Check if the module passed by the router is registered in the modules container
		 */
		phalcon_read_property(&modules, getThis(), SL("_modules"), PH_NOISY|PH_READONLY);
		if (!phalcon_array_isset_fetch(&module, &modules, &module_name, PH_READONLY)) {
			convert_to_string(&module_name);
			zend_throw_exception_ex(phalcon_mvc_application_exception_ce, 0, "Module %s is not registered in the application container", Z_STRVAL(module_name));
			RETURN_MM();
		}

		/**
		 * A module definition must be an array or an object
		 */
		if (Z_TYPE(module) != IS_ARRAY && Z_TYPE(module) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_application_exception_ce, "Invalid module definition");
			return;
		}

		/* An array module definition contains a path to a module definition class */
		if (unlikely(Z_TYPE(module) == IS_ARRAY)) {
			/* Class name used to load the module definition */
			if (!phalcon_array_isset_fetch_str(&module_namespace, &module, SL("namespaceName"), PH_READONLY)) {
				PHALCON_MM_ZVAL_EMPTY_STRING(&module_namespace);
			}

			/* Class name used to load the module definition */
			if (!phalcon_array_isset_fetch_str(&module_class, &module, SL("className"), PH_READONLY)) {
				PHALCON_MM_ZVAL_STRING(&module_class, "Module");
			}

			if (Z_TYPE(module_class) == IS_STRING && zend_is_true(&module_namespace) && !phalcon_memnstr_str(&module_class, SL("\\"))) {
				if (phalcon_end_with_str(&module_namespace, SL("\\"))) {
					PHALCON_CONCAT_VV(&class_name, &module_namespace, &module_class);
				} else {
					PHALCON_CONCAT_VSV(&class_name, &module_namespace, "\\", &module_class);
				}
				PHALCON_MM_ADD_ENTRY(&class_name);
			} else {
				ZVAL_COPY_VALUE(&class_name, &module_class);
			}

			/* If the developer has specified a path, try to include the file */
			if (phalcon_array_isset_fetch_str(&path, &module, SL("path"), PH_READONLY)) {
				convert_to_string_ex(&path);
				if (Z_TYPE(class_name) != IS_STRING || phalcon_class_exists(&class_name, 0) == NULL) {
					if (phalcon_file_exists(&path) == SUCCESS) {
						RETURN_MM_ON_FAILURE(phalcon_require(Z_STRVAL(path)));
					} else {
						zend_throw_exception_ex(phalcon_mvc_application_exception_ce, 0, "Module definition path '%s' does not exist", Z_STRVAL(path));
						RETURN_MM();
					}
				}
			}

			PHALCON_MM_CALL_METHOD(&module_object, &dependency_injector, "get", &class_name);
			PHALCON_MM_ADD_ENTRY(&module_object);
			if (instanceof_function(Z_OBJCE(module_object), zend_ce_closure)) {
				/* A module definition object, can be a Closure instance */
				array_init_size(&module_params, 1);
				phalcon_array_append(&module_params, &dependency_injector, PH_COPY);
				PHALCON_MM_ADD_ENTRY(&module_params);
				PHALCON_MM_CALL_USER_FUNC_ARRAY(NULL, &module_object, &module_params);
			} else if (instanceof_function(Z_OBJCE(module_object), phalcon_mvc_moduledefinitioninterface_ce)) {
				/**
				 * 'registerAutoloaders' and 'registerServices' are automatically called
				 */
				PHALCON_MM_CALL_METHOD(NULL, &module_object, "registerautoloaders", &dependency_injector);
				PHALCON_MM_CALL_METHOD(NULL, &module_object, "registerservices", &dependency_injector);
			} else {
				PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_application_exception_ce, "Invalid module definition");
				return;
			}
		} else if (Z_TYPE(module) == IS_OBJECT) {
			if (instanceof_function(Z_OBJCE(module), zend_ce_closure)) {
				/* A module definition object, can be a Closure instance */
				array_init_size(&module_params, 1);
				phalcon_array_append(&module_params, &dependency_injector, PH_COPY);
				PHALCON_MM_ADD_ENTRY(&module_params);
				PHALCON_MM_CALL_USER_FUNC_ARRAY(&status, &module, &module_params);
			} else if (instanceof_function(Z_OBJCE(module), phalcon_mvc_moduledefinitioninterface_ce)) {
				PHALCON_MM_CALL_METHOD(NULL, &module, "registerautoloaders", &dependency_injector);
				PHALCON_MM_CALL_METHOD(NULL, &module, "registerservices", &dependency_injector);
			}
		} else {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_application_exception_ce, "Invalid module definition");
			return;
		}

		/* Calling afterStartModule event */
		PHALCON_MM_ZVAL_STRING(&event_name, "application:afterStartModule");
		PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &module_name);

		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
		zval_ptr_dtor(&status);
	}

	/**
	 * Check whether use implicit views or not
	 */
	PHALCON_MM_ZVAL_STRING(&event_name, "application:beforeCheckUseImplicitView");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

	phalcon_read_property(&implicit_view, getThis(), SL("_implicitView"), PH_NOISY|PH_READONLY);

	/*
	 * The safe way is to use a flag because it *might* be possible to alter the value
	 * of _implicitView later which might result in crashes because 'view'
	 * is initialized only when _implicitView evaluates to false
	 */
	f_implicit_view = PHALCON_IS_TRUE(&implicit_view);

	if (f_implicit_view) {
		ZVAL_STR(&service, IS(view));

		PHALCON_MM_CALL_METHOD(&view, &dependency_injector, "getshared", &service);
		PHALCON_MM_ADD_ENTRY(&view);
		PHALCON_MM_VERIFY_INTERFACE(&view, phalcon_mvc_viewinterface_ce);

		/**
		 * Start the view component (start output buffering)
		 */
		PHALCON_MM_CALL_METHOD(NULL, &view, "start");
	}
	PHALCON_MM_ZVAL_STRING(&event_name, "application:afterCheckUseImplicitView");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

	/* We get the parameters from the router and assign them to the dispatcher */
	PHALCON_MM_CALL_METHOD(&module_name, &router, "getmodulename");
	PHALCON_MM_ADD_ENTRY(&module_name);
	PHALCON_MM_CALL_METHOD(&namespace_name, &router, "getnamespacename");
	PHALCON_MM_ADD_ENTRY(&namespace_name);
	PHALCON_MM_CALL_METHOD(&controller_name, &router, "getcontrollername");
	PHALCON_MM_ADD_ENTRY(&controller_name);
	PHALCON_MM_CALL_METHOD(&action_name, &router, "getactionname");
	PHALCON_MM_ADD_ENTRY(&action_name);
	PHALCON_MM_CALL_METHOD(&params, &router, "getparams");
	PHALCON_MM_ADD_ENTRY(&params);
	PHALCON_MM_CALL_METHOD(&exact, &router, "isexactcontrollername");
	PHALCON_MM_ADD_ENTRY(&exact);\

	if (!zend_is_true(&namespace_name) && zend_is_true(&module_namespace)) {
		ZVAL_COPY_VALUE(&namespace_name, &module_namespace);
	}

	ZVAL_STR(&service, IS(dispatcher));

	PHALCON_MM_CALL_METHOD(&dispatcher, &dependency_injector, "getshared", &service);
	PHALCON_MM_ADD_ENTRY(&dispatcher);
	PHALCON_MM_VERIFY_INTERFACE(&dispatcher, phalcon_mvc_dispatcherinterface_ce);

	/* Assign the values passed from the router */
	PHALCON_MM_CALL_METHOD(NULL, &dispatcher, "setmodulename", &module_name);
	PHALCON_MM_CALL_METHOD(NULL, &dispatcher, "setnamespacename", &namespace_name);
	PHALCON_MM_CALL_METHOD(NULL, &dispatcher, "setcontrollername", &controller_name, &exact);
	PHALCON_MM_CALL_METHOD(NULL, &dispatcher, "setactionname", &action_name);
	PHALCON_MM_CALL_METHOD(NULL, &dispatcher, "setparams", &params);

	/* Calling beforeHandleRequest */
	PHALCON_MM_ZVAL_STRING(&event_name, "application:beforeHandleRequest");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &dispatcher);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}
	zval_ptr_dtor(&status);

	/* The dispatcher must return an object */
	PHALCON_MM_CALL_METHOD(&controller, &dispatcher, "dispatch");
	PHALCON_MM_ADD_ENTRY(&controller);

	/* Calling afterHandleRequest */
	PHALCON_MM_ZVAL_STRING(&event_name, "application:afterHandleRequest");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &controller);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}
	zval_ptr_dtor(&status);

	if (f_implicit_view) {
		/* Get the latest value returned by an action */
		PHALCON_MM_CALL_METHOD(&possible_response, &dispatcher, "getreturnedvalue");
		PHALCON_MM_ADD_ENTRY(&possible_response);

		/* Check if the returned object is already a response */
		if (Z_TYPE(possible_response) == IS_OBJECT && instanceof_function_ex(Z_OBJCE(possible_response), phalcon_http_responseinterface_ce, 1)) {
			ZVAL_COPY_VALUE(&response, &possible_response);
			ZVAL_TRUE(&returned_response);
		} else {
			ZVAL_STR(&service, IS(response));

			PHALCON_MM_CALL_METHOD(&response, &dependency_injector, "getshared", &service);
			PHALCON_MM_ADD_ENTRY(&response);
			PHALCON_MM_VERIFY_INTERFACE(&response, phalcon_http_responseinterface_ce);

			if (PHALCON_IS_FALSE(&possible_response)) {
				RETURN_MM_CTOR(&response);
			} else if (Z_TYPE(possible_response) == IS_STRING) {
				PHALCON_MM_CALL_METHOD(NULL, &response, "setcontent", &possible_response);
				RETURN_MM_CTOR(&response);
			}
			ZVAL_FALSE(&returned_response);
		}

		if (PHALCON_IS_FALSE(&returned_response)) {
			/**
			 * This allows to make a custom view render
			 */
			PHALCON_MM_ZVAL_STRING(&event_name, "application:beforeRenderView");
			PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &view);

			if (Z_TYPE(controller) == IS_OBJECT && unlikely(phalcon_method_exists_ex(&controller, SL("beforerenderview")) == SUCCESS)) {
				if (likely(PHALCON_IS_NOT_FALSE(&status))) {
					zval_ptr_dtor(&status);
					PHALCON_MM_CALL_METHOD(&status, &controller, "beforerenderview", &view);
				} else {
					PHALCON_MM_CALL_METHOD(NULL, &controller, "beforerenderview", &view);
				}
			}

			/* Check if the view process has been treated by the developer */
			if (PHALCON_IS_NOT_FALSE(&status)) {
				zval_ptr_dtor(&status);
				PHALCON_MM_CALL_METHOD(&namespace_name, &dispatcher, "getnamespacename");
				PHALCON_MM_ADD_ENTRY(&namespace_name);
				PHALCON_MM_CALL_METHOD(&controller_name, &dispatcher, "getcontrollername");
				PHALCON_MM_ADD_ENTRY(&controller_name);
				PHALCON_MM_CALL_METHOD(&action_name, &dispatcher, "getactionname");
				PHALCON_MM_ADD_ENTRY(&action_name);
				PHALCON_MM_CALL_METHOD(&params, &dispatcher, "getparams");
				PHALCON_MM_ADD_ENTRY(&params);

				/* Automatic render based on the latest controller executed */
				if (Z_TYPE(possible_response) == IS_OBJECT && instanceof_function_ex(Z_OBJCE(possible_response), phalcon_mvc_view_modelinterface_ce, 1)) {
					PHALCON_MM_CALL_METHOD(NULL, &view, "render", &controller_name, &action_name, &params, &namespace_name, &possible_response);
				} else {
					if (Z_TYPE(possible_response) == IS_ARRAY) {
						PHALCON_MM_CALL_METHOD(NULL, &view, "setvars", &possible_response, &PHALCON_GLOBAL(z_true));
					}
					/* Automatic render based on the latest controller executed */
					PHALCON_MM_CALL_METHOD(NULL, &view, "render", &controller_name, &action_name, &params, &namespace_name);
				}
			}
		}
	} else {
		/* Get the latest value returned by an action */
		PHALCON_MM_CALL_METHOD(&possible_response, &dispatcher, "getreturnedvalue");
		PHALCON_MM_ADD_ENTRY(&possible_response);

		/* Check if the returned object is already a response */
		if (Z_TYPE(possible_response) == IS_OBJECT && instanceof_function_ex(Z_OBJCE(possible_response), phalcon_http_responseinterface_ce, 1)) {
			ZVAL_COPY_VALUE(&response, &possible_response);
			ZVAL_TRUE(&returned_response);
		} else {
			ZVAL_STR(&service, IS(response));

			PHALCON_MM_CALL_METHOD(&response, &dependency_injector, "getshared", &service);
			PHALCON_MM_ADD_ENTRY(&response);
			PHALCON_MM_VERIFY_INTERFACE(&response, phalcon_http_responseinterface_ce);
			ZVAL_FALSE(&returned_response);
		}

		if (PHALCON_IS_FALSE(&returned_response)) {
			if (Z_TYPE(possible_response) == IS_STRING) {
				PHALCON_MM_CALL_METHOD(NULL, &response, "setcontent", &possible_response);
			} else if (Z_TYPE(possible_response) == IS_ARRAY) {
				PHALCON_MM_CALL_METHOD(NULL, &response, "setjsoncontent", &possible_response);
			}
		}
	}

	if (f_implicit_view) {
		PHALCON_MM_CALL_METHOD(NULL, &view, "finish");

		if (PHALCON_IS_FALSE(&returned_response)) {
			PHALCON_MM_ZVAL_STRING(&event_name, "application:afterRenderView");
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &view);

			if (Z_TYPE(controller) == IS_OBJECT && unlikely(phalcon_method_exists_ex(&controller, SL("afterrenderview")) == SUCCESS)) {
				PHALCON_MM_CALL_METHOD(NULL, &controller, "afterrenderview", &view);
			}
			/* The content returned by the view is passed to the response service */
			PHALCON_MM_CALL_METHOD(&content, &view, "getcontent");
			PHALCON_MM_ADD_ENTRY(&content);
			PHALCON_MM_CALL_METHOD(NULL, &response, "setcontent", &content);
		}
	}

	/* Calling beforeSendResponse */
	PHALCON_MM_ZVAL_STRING(&event_name, "application:beforeSendResponse");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &response);

	if (Z_TYPE(controller) == IS_OBJECT && unlikely(phalcon_method_exists_ex(&controller, SL("beforesendresponse")) == SUCCESS)) {
		PHALCON_MM_CALL_METHOD(NULL, &controller, "beforesendresponse", &response);
	}

	phalcon_read_property(&auto_send, getThis(), SL("_autoSendHeader"), PH_NOISY|PH_READONLY);

	if (likely(zend_is_true(&auto_send))) {
		/* Headers & Cookies are automatically sent */
		PHALCON_MM_CALL_METHOD(NULL, &response, "sendheaders");
		PHALCON_MM_CALL_METHOD(NULL, &response, "sendcookies");
	}

	PHALCON_MM_ZVAL_STRING(&event_name, "application:afterSendResponse");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &response);

	if (Z_TYPE(controller) == IS_OBJECT && unlikely(phalcon_method_exists_ex(&controller, SL("aftersendresponse")) == SUCCESS)) {
		PHALCON_MM_CALL_METHOD(NULL, &controller, "aftersendresponse", &response);
	}

	/* Return the response */
	RETURN_MM_CTOR(&response);
}

/**
 * Does a HMVC request in the application
 *
 * @param array $location
 * @param array $data
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Application, request){

	zval *uri, *data = NULL, *_dependency_injector = NULL, dependency_injector_new = {}, service = {}, definition = {};
	zval app = {}, response = {}, requset = {};

	phalcon_fetch_params(1, 1, 2, &uri, &data, &_dependency_injector);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}


	if (!_dependency_injector || Z_TYPE_P(_dependency_injector) == IS_NULL) {
		zval dependency_injector = {}, services = {}, *value;
		zend_string *key;
		ulong idx;
		PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);
		PHALCON_MM_CALL_METHOD(&services, &dependency_injector, "getservices");
		PHALCON_MM_ADD_ENTRY(&services);

		object_init_ex(&dependency_injector_new, phalcon_di_ce);
		PHALCON_MM_ADD_ENTRY(&dependency_injector_new);
		
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(services), idx, key, value) {
			zval tmp = {};
			if (key) {
				ZVAL_STR(&tmp, key);
			} else {
				ZVAL_LONG(&tmp, idx);
			}
		PHALCON_MM_CALL_METHOD(NULL, &dependency_injector_new, "setservice", &tmp, value);
		} ZEND_HASH_FOREACH_END();
	} else {
		ZVAL_COPY_VALUE(&dependency_injector_new, _dependency_injector);
	}

	/**
	 * Request
	 */
	ZVAL_STR(&service, IS(request));
	object_init_ex(&requset, phalcon_http_request_ce);
	PHALCON_MM_ADD_ENTRY(&requset);
	PHALCON_MM_CALL_METHOD(NULL, &requset, "__construct", data);
	PHALCON_MM_CALL_METHOD(NULL, &dependency_injector_new, "set", &service, &requset);

	/**
	 * Mvc Router
	 */
	ZVAL_STR(&service, IS(router));
	PHALCON_MM_CALL_METHOD(&definition, &dependency_injector_new, "getraw", &service);
	PHALCON_MM_ADD_ENTRY(&definition);
	PHALCON_MM_CALL_METHOD(NULL, &dependency_injector_new, "set", &service, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Mvc Dispatcher
	 */
	ZVAL_STR(&service, IS(dispatcher));
	PHALCON_CALL_METHOD(&definition, &dependency_injector_new, "getraw", &service);
	PHALCON_MM_ADD_ENTRY(&definition);
	PHALCON_CALL_METHOD(NULL, &dependency_injector_new, "set", &service, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Mvc View
	 */
	ZVAL_STR(&service, IS(view));
	PHALCON_CALL_METHOD(&definition, &dependency_injector_new, "getraw", &service);
	PHALCON_MM_ADD_ENTRY(&definition);
	PHALCON_CALL_METHOD(NULL, &dependency_injector_new, "set", &service, &definition, &PHALCON_GLOBAL(z_true));

	ZVAL_COPY_VALUE(&app, getThis());

	PHALCON_MM_CALL_METHOD(NULL, &app, "setdi", &dependency_injector_new);
	PHALCON_MM_CALL_METHOD(&response, &app, "handle", uri);
	PHALCON_MM_ADD_ENTRY(&response);
	if (Z_TYPE(response) == IS_OBJECT) {
		PHALCON_MM_RETURN_CALL_METHOD(&response, "getcontent");
	}

	RETURN_MM();
}
