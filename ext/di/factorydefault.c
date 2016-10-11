
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
 * Phalcon\DI\FactoryDefault
 *
 * This is a variant of the standard Phalcon\DI. By default it automatically
 * registers all the services provided by the framework. Thanks to this, the developer does not need
 * to register each service individually providing a full stack framework
 */
zend_class_entry *phalcon_di_factorydefault_ce;

PHP_METHOD(Phalcon_DI_FactoryDefault, __construct);

static const zend_function_entry phalcon_di_factorydefault_method_entry[] = {
	PHP_ME(Phalcon_DI_FactoryDefault, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_FE_END
};


/**
 * Phalcon\DI\FactoryDefault initializer
 */
PHALCON_INIT_CLASS(Phalcon_DI_FactoryDefault){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\DI, FactoryDefault, di_factorydefault, phalcon_di_ce, phalcon_di_factorydefault_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\DI\FactoryDefault constructor
 */
PHP_METHOD(Phalcon_DI_FactoryDefault, __construct){

	zval *name = NULL, servicename = {}, definition = {};

	phalcon_fetch_params(0, 0, 1, &name);

	if (name) {
		PHALCON_CALL_PARENTW(NULL, phalcon_di_factorydefault_ce, getThis(), "__construct", name);
	} else {
		PHALCON_CALL_PARENTW(NULL, phalcon_di_factorydefault_ce, getThis(), "__construct");
	}

	/**
	 * Mvc Router
	 */
	PHALCON_STR(&servicename, ISV(router));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Router");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Mvc Dispatcher
	 */
	PHALCON_STR(&servicename, ISV(dispatcher));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Dispatcher");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Mvc Url
	 */
	PHALCON_STR(&servicename, ISV(url));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Url");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Models manager for ORM
	 */
	PHALCON_STR(&servicename, ISV(modelsManager));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Models meta-data using the Memory adapter
	 */
	PHALCON_STR(&servicename, ISV(modelsMetadata));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\MetaData\\Memory");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Models Query for ORM
	 */
	PHALCON_STR(&servicename, ISV(modelsQuery));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\Query");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_false));

	/**
	 * Models Query Select Builder for ORM
	 */
	PHALCON_STR(&servicename, ISV(modelsQueryBuilderForSelect));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\Query\\Builder\\Select");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_false));

	/**
	 * Models Criteria for ORM
	 */
	PHALCON_STR(&servicename, ISV(modelsCriteria));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\Criteria");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_false));

	/**
	 * Request/Response are always &PHALCON_GLOBAL(z_true)
	 */
	PHALCON_STR(&servicename, ISV(response));
	PHALCON_STR(&definition, "Phalcon\\Http\\Response");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Http Response Cookies
	 */
	PHALCON_STR(&servicename, ISV(cookies));
	PHALCON_STR(&definition, "Phalcon\\Http\\Response\\Cookies");

	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
	/**
	 * Http Request
	 */
	PHALCON_STR(&servicename, ISV(request));
	PHALCON_STR(&definition, "Phalcon\\Http\\Request");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Filter/Escaper services are always &PHALCON_GLOBAL(z_true)
	 */
	PHALCON_STR(&servicename, ISV(filter));
	PHALCON_STR(&definition, "Phalcon\\Filter");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Escaper
	 */
	PHALCON_STR(&servicename, ISV(escaper));
	PHALCON_STR(&definition, "Phalcon\\Escaper");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Default annotations service
	 */
	PHALCON_STR(&servicename, ISV(annotations));
	PHALCON_STR(&definition, "Phalcon\\Annotations\\Adapter\\Memory");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Security doesn't need to be &PHALCON_GLOBAL(z_true), but anyways we register it as &PHALCON_GLOBAL(z_true)
	 */
	PHALCON_STR(&servicename, ISV(security));
	PHALCON_STR(&definition, "Phalcon\\Security");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Crypt Service
	 */
	PHALCON_STR(&servicename, ISV(crypt));
	PHALCON_STR(&definition, "Phalcon\\Crypt");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Flash services are always &PHALCON_GLOBAL(z_true)
	 */
	PHALCON_STR(&servicename, ISV(flash));
	PHALCON_STR(&definition, "Phalcon\\Flash\\Direct");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Flash Session
	 */
	PHALCON_STR(&servicename, ISV(flashSession));
	PHALCON_STR(&definition, "Phalcon\\Flash\\Session");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Tag/Helpers
	 */
	PHALCON_STR(&servicename, ISV(tag));
	PHALCON_STR(&definition, "Phalcon\\Tag");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Session is always &PHALCON_GLOBAL(z_true)
	 */
	PHALCON_STR(&servicename, ISV(session));
	PHALCON_STR(&definition, "Phalcon\\Session\\Adapter\\Files");

	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Session/Bag
	 */
	PHALCON_STR(&servicename, ISV(sessionBag));
	PHALCON_STR(&definition, "Phalcon\\Session\\Bag");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Events Manager is always &PHALCON_GLOBAL(z_true)
	 */
	PHALCON_STR(&servicename, ISV(eventsManager));
	PHALCON_STR(&definition, "Phalcon\\Events\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Model Transaction Manager
	 */
	PHALCON_STR(&servicename, ISV(transactionManager));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\Transaction\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));

	/**
	 * Assets Manager
	 */
	PHALCON_STR(&servicename, ISV(assets));
	PHALCON_STR(&definition, "Phalcon\\Assets\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &servicename, &definition, &PHALCON_GLOBAL(z_true));
}
