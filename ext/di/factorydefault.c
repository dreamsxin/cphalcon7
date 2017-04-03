
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
  |          Kenji Minamoto <kenji.minamoto@gmail.com>                     |
  +------------------------------------------------------------------------+
*/

#include "di/factorydefault.h"
#include "di/service.h"
#include "di.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/string.h"

#include "interned-strings.h"

/**
 * Phalcon\Di\FactoryDefault
 *
 * This is a variant of the standard Phalcon\Di. By default it automatically
 * registers all the services provided by the framework. Thanks to this, the developer does not need
 * to register each service individually providing a full stack framework
 */
zend_class_entry *phalcon_di_factorydefault_ce;

PHP_METHOD(Phalcon_Di_FactoryDefault, __construct);

static const zend_function_entry phalcon_di_factorydefault_method_entry[] = {
	PHP_ME(Phalcon_Di_FactoryDefault, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_FE_END
};


/**
 * Phalcon\Di\FactoryDefault initializer
 */
PHALCON_INIT_CLASS(Phalcon_Di_FactoryDefault){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Di, FactoryDefault, di_factorydefault, phalcon_di_ce, phalcon_di_factorydefault_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Di\FactoryDefault constructor
 */
PHP_METHOD(Phalcon_Di_FactoryDefault, __construct){

	zval *name = NULL, servicename = {}, definition = {};

	phalcon_fetch_params(0, 0, 1, &name);

	if (name) {
		PHALCON_CALL_PARENT(NULL, phalcon_di_factorydefault_ce, getThis(), "__construct", name);
	} else {
		PHALCON_CALL_PARENT(NULL, phalcon_di_factorydefault_ce, getThis(), "__construct");
	}

	/**
	 * Mvc Router
	 */
	ZVAL_STR(&servicename, IS(router));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Router");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Mvc Dispatcher
	 */
	ZVAL_STR(&servicename, IS(dispatcher));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Dispatcher");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Mvc Url
	 */
	ZVAL_STR(&servicename, IS(url));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Url");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Models manager for ORM
	 */
	ZVAL_STR(&servicename, IS(modelsManager));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\Manager");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Models meta-data using the Memory adapter
	 */
	ZVAL_STR(&servicename, IS(modelsMetadata));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\MetaData\\Memory");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Models Query for ORM
	 */
	ZVAL_STR(&servicename, IS(modelsQuery));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\Query");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_false));
	zval_ptr_dtor(&definition);

	/**
	 * Models Query Select Builder for ORM
	 */
	ZVAL_STR(&servicename, IS(modelsQueryBuilderForSelect));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\Query\\Builder\\Select");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_false));
	zval_ptr_dtor(&definition);

	/**
	 * Models Criteria for ORM
	 */
	ZVAL_STR(&servicename, IS(modelsCriteria));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\Criteria");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_false));
	zval_ptr_dtor(&definition);

	/**
	 * Request/Response are always &PHALCON_GLOBAL(z_true)
	 */
	ZVAL_STR(&servicename, IS(response));
	ZVAL_STRING(&definition, "Phalcon\\Http\\Response");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Http Response Cookies
	 */
	ZVAL_STR(&servicename, IS(cookies));
	ZVAL_STRING(&definition, "Phalcon\\Http\\Response\\Cookies");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Http Request
	 */
	ZVAL_STR(&servicename, IS(request));
	ZVAL_STRING(&definition, "Phalcon\\Http\\Request");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Filter/Escaper services are always &PHALCON_GLOBAL(z_true)
	 */
	ZVAL_STR(&servicename, IS(filter));
	ZVAL_STRING(&definition, "Phalcon\\Filter");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Escaper
	 */
	ZVAL_STR(&servicename, IS(escaper));
	ZVAL_STRING(&definition, "Phalcon\\Escaper");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Default annotations service
	 */
	ZVAL_STR(&servicename, IS(annotations));
	ZVAL_STRING(&definition, "Phalcon\\Annotations\\Adapter\\Memory");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Security doesn't need to be &PHALCON_GLOBAL(z_true), but anyways we register it as &PHALCON_GLOBAL(z_true)
	 */
	ZVAL_STR(&servicename, IS(security));
	ZVAL_STRING(&definition, "Phalcon\\Security");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Crypt Service
	 */
	ZVAL_STR(&servicename, IS(crypt));
	ZVAL_STRING(&definition, "Phalcon\\Crypt");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Flash services are always &PHALCON_GLOBAL(z_true)
	 */
	ZVAL_STR(&servicename, IS(flash));
	ZVAL_STRING(&definition, "Phalcon\\Flash\\Direct");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Flash Session
	 */
	ZVAL_STR(&servicename, IS(flashSession));
	ZVAL_STRING(&definition, "Phalcon\\Flash\\Session");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Tag/Helpers
	 */
	ZVAL_STR(&servicename, IS(tag));
	ZVAL_STRING(&definition, "Phalcon\\Tag");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Session is always &PHALCON_GLOBAL(z_true)
	 */
	ZVAL_STR(&servicename, IS(session));
	ZVAL_STRING(&definition, "Phalcon\\Session\\Adapter\\Files");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Session/Bag
	 */
	ZVAL_STR(&servicename, IS(sessionBag));
	ZVAL_STRING(&definition, "Phalcon\\Session\\Bag");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Events Manager is always &PHALCON_GLOBAL(z_true)
	 */
	ZVAL_STR(&servicename, IS(eventsManager));
	ZVAL_STRING(&definition, "Phalcon\\Events\\Manager");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Model Transaction Manager
	 */
	ZVAL_STR(&servicename, IS(transactionManager));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\Transaction\\Manager");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);

	/**
	 * Assets Manager
	 */
	ZVAL_STR(&servicename, IS(assets));
	ZVAL_STRING(&definition, "Phalcon\\Assets\\Manager");
	PHALCON_CALL_SELF(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&definition);
}
