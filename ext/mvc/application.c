
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
PHP_METHOD(Phalcon_Mvc_Application, registerModules);
PHP_METHOD(Phalcon_Mvc_Application, getModules);
PHP_METHOD(Phalcon_Mvc_Application, setDefaultModule);
PHP_METHOD(Phalcon_Mvc_Application, getDefaultModule);
PHP_METHOD(Phalcon_Mvc_Application, handle);
PHP_METHOD(Phalcon_Mvc_Application, request);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_application___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, dependencyInjector)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_application_useimplicitview, 0, 0, 1)
	ZEND_ARG_INFO(0, implicitView)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_application_registermodules, 0, 0, 1)
	ZEND_ARG_INFO(0, modules)
	ZEND_ARG_INFO(0, merge)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_application_setdefaultmodule, 0, 0, 1)
	ZEND_ARG_INFO(0, defaultModule)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_application_handle, 0, 0, 0)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_application_request, 0, 0, 1)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_application_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Application, __construct, arginfo_phalcon_mvc_application___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Application, useImplicitView, arginfo_phalcon_mvc_application_useimplicitview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Application, registerModules, arginfo_phalcon_mvc_application_registermodules, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Application, getModules, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Application, setDefaultModule, arginfo_phalcon_mvc_application_setdefaultmodule, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Application, getDefaultModule, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Application, handle, arginfo_phalcon_mvc_application_handle, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Application, request, arginfo_phalcon_mvc_application_request, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Application initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Application){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, Application, mvc_application, phalcon_di_injectable_ce, phalcon_mvc_application_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_application_ce, SL("_defaultModule"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_application_ce, SL("_modules"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_application_ce, SL("_moduleObject"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_application_ce, SL("_implicitView"), 1, ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Application
 *
 * @param Phalcon\DI $dependencyInjector
 */
PHP_METHOD(Phalcon_Mvc_Application, __construct){

	zval *dependency_injector = NULL;

	phalcon_fetch_params(0, 0, 1, &dependency_injector);

	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHODW(NULL, getThis(), "setdi", dependency_injector);
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

	phalcon_update_property_zval(getThis(), SL("_implicitView"), implicit_view);
	RETURN_THISW();
}

/**
 * Register an array of modules present in the application
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
 * @param Phalcon\Mvc\Application
 */
PHP_METHOD(Phalcon_Mvc_Application, registerModules){

	zval *modules, *merge = NULL, registered_modules = {}, merged_modules = {};

	phalcon_fetch_params(0, 1, 1, &modules, &merge);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(modules) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_application_exception_ce, "Modules must be an Array");
		return;
	}
	if (PHALCON_IS_FALSE(merge)) {
		phalcon_update_property_zval(getThis(), SL("_modules"), modules);
	} else {
		phalcon_read_property(&registered_modules, getThis(), SL("_modules"), PH_NOISY);
		if (Z_TYPE(registered_modules) == IS_ARRAY) { 
			phalcon_fast_array_merge(&merged_modules, &registered_modules, modules);
		} else {
			PHALCON_CPY_WRT(&merged_modules, modules);
		}

		phalcon_update_property_zval(getThis(), SL("_modules"), &merged_modules);
	}

	RETURN_THISW();
}

/**
 * Return the modules registered in the application
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Application, getModules){


	RETURN_MEMBER(getThis(), "_modules");
}

/**
 * Sets the module name to be used if the router doesn't return a valid module
 *
 * @param string $defaultModule
 * @return Phalcon\Mvc\Application
 */
PHP_METHOD(Phalcon_Mvc_Application, setDefaultModule){

	zval *default_module;

	phalcon_fetch_params(0, 1, 0, &default_module);

	phalcon_update_property_zval(getThis(), SL("_defaultModule"), default_module);
	RETURN_THISW();
}

/**
 * Returns the default module name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Application, getDefaultModule){


	RETURN_MEMBER(getThis(), "_defaultModule");
}

/**
 * Handles a MVC request
 *
 * @param string $uri
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Mvc_Application, handle){

	zval *uri = NULL, dependency_injector = {}, event_name = {}, status = {}, service = {}, router = {}, module_name = {};
	zval modules = {}, module = {}, class_name = {}, path = {}, module_object = {}, module_params = {};
	zval implicit_view = {}, view = {}, namespace_name = {}, controller_name = {}, action_name = {}, params = {}, exact = {};
	zval dispatcher = {}, controller = {}, possible_response = {}, returned_response = {}, response = {}, content = {};
	int f_implicit_view;

	phalcon_fetch_params(0, 0, 1, &uri);

	if (!uri) {
		uri = &PHALCON_GLOBAL(z_null);
	}

	/* Call boot event, this allows the developer to perform initialization actions */
	PHALCON_STR(&event_name, "application:boot");
	PHALCON_CALL_METHODW(&status, getThis(), "fireevent", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}

	PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi");
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_application_exception_ce, "A dependency injection object is required to access internal services");
		return;
	}

	PHALCON_STR(&service, ISV(app));
	PHALCON_CALL_METHODW(NULL, &dependency_injector, "setShared", &service, getThis());

	PHALCON_STR(&service, ISV(router));
	PHALCON_CALL_METHODW(&router, &dependency_injector, "getshared", &service);
	PHALCON_VERIFY_INTERFACEW(&router, phalcon_mvc_routerinterface_ce);

	/* Handle the URI pattern (if any) */
	PHALCON_CALL_METHODW(NULL, &router, "handle", uri);

	/* Load module config */
	PHALCON_CALL_METHODW(&module_name, &router, "getmodulename");

	/* If the router doesn't return a valid module we use the default module */
	if (!zend_is_true(&module_name)) {
		 phalcon_return_property(&module_name, getThis(), SL("_defaultModule"));
	}

	/** 
	 * Process the module definition
	 */
	if (zend_is_true(&module_name)) {
		PHALCON_STR(&event_name, "application:beforeStartModule");
		PHALCON_CALL_METHODW(&status, getThis(), "fireevent", &event_name, &module_name);

		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}

		/** 
		 * Check if the module passed by the router is registered in the modules container
		 */
		phalcon_read_property(&modules, getThis(), SL("_modules"), PH_NOISY);
		if (!phalcon_array_isset_fetch(&module, &modules, &module_name, 0)) {
			convert_to_string(&module_name);
			zend_throw_exception_ex(phalcon_mvc_application_exception_ce, 0, "Module %s is not registered in the application container", Z_STRVAL(module_name));
			return;
		}

		/** 
		 * A module definition must be an array or an object
		 */
		if (Z_TYPE(module) != IS_ARRAY && Z_TYPE(module) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_application_exception_ce, "Invalid module definition");
			return;
		}

		/* An array module definition contains a path to a module definition class */
		if (Z_TYPE_P(&module) == IS_ARRAY) { 
			/* Class name used to load the module definition */
			if (!phalcon_array_isset_fetch_str(&class_name, &module, SL("className"))) {
				PHALCON_STR(&class_name, "Module");
			}

			/* If the developer has specified a path, try to include the file */
			if (phalcon_array_isset_fetch_str(&path, &module, SL("path"))) {
				convert_to_string_ex(&path);
				if (Z_TYPE(class_name) != IS_STRING || phalcon_class_exists(&class_name, 0) == NULL) {
					if (phalcon_file_exists(&path) == SUCCESS) {
						RETURN_ON_FAILURE(phalcon_require(Z_STRVAL(path)));
					} else {
						zend_throw_exception_ex(phalcon_mvc_application_exception_ce, 0, "Module definition path '%s' does not exist", Z_STRVAL(path));
						return;
					}
				}
			}

			PHALCON_CALL_METHODW(&module_object, &dependency_injector, "get", &class_name);

			/** 
			 * 'registerAutoloaders' and 'registerServices' are automatically called
			 */
			PHALCON_CALL_METHODW(NULL, &module_object, "registerautoloaders", &dependency_injector);
			PHALCON_CALL_METHODW(NULL, &module_object, "registerservices", &dependency_injector);
		} else if (Z_TYPE(module) == IS_OBJECT) {
			if (instanceof_function(Z_OBJCE(module), zend_ce_closure)) {
				/* A module definition object, can be a Closure instance */
				array_init_size(&module_params, 1);
				phalcon_array_append(&module_params, &dependency_injector, PH_COPY);

				PHALCON_CALL_USER_FUNC_ARRAYW(&status, &module, &module_params);
			} else if (instanceof_function(Z_OBJCE(module), phalcon_mvc_moduledefinitioninterface_ce)) {
				PHALCON_CALL_METHODW(NULL, &module, "registerautoloaders", &dependency_injector);
				PHALCON_CALL_METHODW(NULL, &module, "registerservices", &dependency_injector);
			}
		} else {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_application_exception_ce, "Invalid module definition");
			return;
		}

		/* Calling afterStartModule event */
		PHALCON_STR(&event_name, "application:afterStartModule");

		PHALCON_CALL_METHODW(&status, getThis(), "fireevent", &event_name, &module_name);

		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	/** 
	 * Check whether use implicit views or not
	 */
	phalcon_read_property(&implicit_view, getThis(), SL("_implicitView"), PH_NOISY);

	/*
	 * The safe way is to use a flag because it *might* be possible to alter the value
	 * of _implicitView later which might result in crashes because 'view'
	 * is initialized only when _implicitView evaluates to false
	 */
	f_implicit_view = PHALCON_IS_TRUE(&implicit_view);

	if (f_implicit_view) {
		PHALCON_STR(&service, "view");

		PHALCON_CALL_METHODW(&view, &dependency_injector, "getshared", &service);
		PHALCON_VERIFY_INTERFACEW(&view, phalcon_mvc_viewinterface_ce);
	}

	/* We get the parameters from the router and assign them to the dispatcher */
	PHALCON_CALL_METHODW(&module_name, &router, "getmodulename");
	PHALCON_CALL_METHODW(&namespace_name, &router, "getnamespacename");
	PHALCON_CALL_METHODW(&controller_name, &router, "getcontrollername");
	PHALCON_CALL_METHODW(&action_name, &router, "getactionname");
	PHALCON_CALL_METHODW(&params, &router, "getparams");
	PHALCON_CALL_METHODW(&exact, &router, "isexactcontrollername");

	PHALCON_STR(&service, ISV(dispatcher));

	PHALCON_CALL_METHODW(&dispatcher, &dependency_injector, "getshared", &service);
	PHALCON_VERIFY_INTERFACEW(&dispatcher, phalcon_dispatcherinterface_ce);

	/* Assign the values passed from the router */
	PHALCON_CALL_METHODW(NULL, &dispatcher, "setmodulename", &module_name);
	PHALCON_CALL_METHODW(NULL, &dispatcher, "setnamespacename", &namespace_name);
	PHALCON_CALL_METHODW(NULL, &dispatcher, "setcontrollername", &controller_name, &exact);
	PHALCON_CALL_METHODW(NULL, &dispatcher, "setactionname", &action_name);
	PHALCON_CALL_METHODW(NULL, &dispatcher, "setparams", &params);

	if (f_implicit_view) {
		/** 
		 * Start the view component (start output buffering)
		 */
		PHALCON_CALL_METHODW(NULL, &view, "start");
	}

	/* Calling beforeHandleRequest */
	PHALCON_STR(&event_name, "application:beforeHandleRequest");

	PHALCON_CALL_METHODW(&status, getThis(), "fireevent", &event_name, &dispatcher);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}

	/* The dispatcher must return an object */
	PHALCON_CALL_METHODW(&controller, &dispatcher, "dispatch");

	/* Calling afterHandleRequest */
	PHALCON_STR(&event_name, "application:afterHandleRequest");

	PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name, &controller);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}

	if (f_implicit_view) {
		/* Get the latest value returned by an action */
		PHALCON_CALL_METHODW(&possible_response, &dispatcher, "getreturnedvalue");

		/* Check if the returned object is already a response */
		if (Z_TYPE(possible_response) == IS_OBJECT && instanceof_function_ex(Z_OBJCE(possible_response), phalcon_http_responseinterface_ce, 1)) {
			PHALCON_CPY_WRT(&response, &possible_response);
			ZVAL_TRUE(&returned_response);
		} else {
			PHALCON_STR(&service, ISV(response));

			PHALCON_CALL_METHODW(&response, &dependency_injector, "getshared", &service);
			PHALCON_VERIFY_INTERFACEW(&response, phalcon_http_responseinterface_ce);

			if (PHALCON_IS_FALSE(&possible_response)) {
				RETURN_CTORW(&response);
			} else if (Z_TYPE(possible_response) == IS_STRING) {
				PHALCON_CALL_METHODW(&response, &possible_response, "setcontent", &possible_response);
				RETURN_CTORW(&response);
			}
			ZVAL_FALSE(&returned_response);
		}

		if (PHALCON_IS_FALSE(&returned_response)) {
			if (Z_TYPE(controller) == IS_OBJECT) {
				/** 
				 * This allows to make a custom view render
				 */
				PHALCON_STR(&event_name, "application:beforeRenderView");

				PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name, &view);

				/* Check if the view process has been treated by the developer */
				if (PHALCON_IS_NOT_FALSE(&status)) {
					PHALCON_CALL_METHODW(&namespace_name, &dispatcher, "getnamespacename");
					PHALCON_CALL_METHODW(&controller_name, &dispatcher, "getcontrollername");
					PHALCON_CALL_METHODW(&action_name, &dispatcher, "getactionname");
					PHALCON_CALL_METHODW(&params, &dispatcher, "getparams");

					/* Automatic render based on the latest controller executed */
					if (Z_TYPE(possible_response) == IS_OBJECT && instanceof_function_ex(Z_OBJCE(possible_response), phalcon_mvc_view_modelinterface_ce, 1)) {
						PHALCON_CALL_METHODW(NULL, &view, "render", &controller_name, &action_name, &params, &namespace_name, &possible_response);
					} else {
						if (Z_TYPE(possible_response) == IS_ARRAY) {
							PHALCON_CALL_METHODW(NULL, &view, "setvars", &possible_response, &PHALCON_GLOBAL(z_true));
						}
						/* Automatic render based on the latest controller executed */
						PHALCON_CALL_METHODW(NULL, &view, "render", &controller_name, &action_name, &params, &namespace_name);
					}
				}

				PHALCON_STR(&event_name, "application:afterRenderView");

				PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name, &view);
			}
		}
	} else {
		PHALCON_STR(&service, ISV(response));

		PHALCON_CALL_METHODW(&response, &dependency_injector, "getshared", &service);
		PHALCON_VERIFY_INTERFACEW(&response, phalcon_http_responseinterface_ce);
	}

	/* Calling beforeSendResponse */
	PHALCON_STR(&event_name, "application:beforeSendResponse");

	PHALCON_CALL_METHODW(&status, getThis(), "fireevent", &event_name, &response);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}

	if (f_implicit_view) {
		PHALCON_CALL_METHODW(NULL, &view, "finish");

		if (PHALCON_IS_FALSE(&returned_response)) {
			/* The content returned by the view is passed to the response service */
			PHALCON_CALL_METHODW(&content, &view, "getcontent");
			PHALCON_CALL_METHODW(NULL, &response, "setcontent", &content);
		}
	}

	/* Headers & Cookies are automatically sent */
	PHALCON_CALL_METHODW(NULL, &response, "sendheaders");
	PHALCON_CALL_METHODW(NULL, &response, "sendcookies");

	PHALCON_STR(&event_name, "application:afterSendResponse");

	PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name, &response);

	/* Return the response */
	RETURN_CTORW(&response);
}

/**
 * Does a HMVC request in the application
 *
 * @param array $location
 * @param array $data
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Application, request){

	zval *uri, dependency_injector = {}, dependency_injector_new = {}, service = {}, definition = {}, app = {}, response = {};

	phalcon_fetch_params(0, 1, 0, &uri);

	PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi");

	ZVAL_OBJ(&dependency_injector_new, zend_objects_clone_obj(&dependency_injector));

	PHALCON_CALL_CE_STATICW(NULL, phalcon_di_ce, "setdefault", &dependency_injector_new);

	/**
	 * Mvc Dispatcher
	 */
	PHALCON_STR(&service, ISV(dispatcher));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Dispatcher");
	PHALCON_CALL_METHODW(NULL, &dependency_injector_new, "set", &service, &definition, &PHALCON_GLOBAL(z_true));

	ZVAL_OBJ(&app, zend_objects_clone_obj(getThis()));

	PHALCON_CALL_METHODW(&response, &app, "setdi", &dependency_injector_new);
	PHALCON_CALL_METHODW(&response, &app, "handle", uri);
	if (Z_TYPE(response) == IS_OBJECT) {
		PHALCON_RETURN_CALL_METHODW(&response, "getcontent");
	}

	PHALCON_CALL_CE_STATICW(NULL, phalcon_di_ce, "setdefault", &dependency_injector);
}
