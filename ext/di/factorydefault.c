
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

	zval *shared, name = {}, definition = {};

	PHALCON_CALL_PARENTW(NULL, phalcon_di_factorydefault_ce, getThis(), "__construct");

	shared = &PHALCON_GLOBAL(z_true);

	/**
	 * Mvc Router
	 */
	PHALCON_STR(&name, ISV(router));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Router");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Mvc Dispatcher
	 */
	PHALCON_STR(&name, ISV(dispatcher));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Dispatcher");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Mvc Url
	 */
	PHALCON_STR(&name, ISV(url));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Url");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Models manager for ORM
	 */
	PHALCON_STR(&name, ISV(modelsManager));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Models meta-data using the Memory adapter
	 */
	PHALCON_STR(&name, ISV(modelsMetadata));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\MetaData\\Memory");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Models Query for ORM
	 */
	PHALCON_STR(&name, ISV(modelsQuery));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\Query");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, &PHALCON_GLOBAL(z_false));

	/**
	 * Models Query Builder for ORM
	 */
	PHALCON_STR(&name, ISV(modelsQueryBuilder));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\Query\\Builder");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, &PHALCON_GLOBAL(z_false));

	/**
	 * Models Criteria for ORM
	 */
	PHALCON_STR(&name, ISV(modelsCriteria));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\Criteria");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, &PHALCON_GLOBAL(z_false));

	/**
	 * Request/Response are always shared
	 */
	PHALCON_STR(&name, ISV(response));
	PHALCON_STR(&definition, "Phalcon\\Http\\Response");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Http Response Cookies
	 */
	PHALCON_STR(&name, ISV(cookies));
	PHALCON_STR(&definition, "Phalcon\\Http\\Response\\Cookies");

	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);
	/**
	 * Http Request
	 */
	PHALCON_STR(&name, ISV(request));
	PHALCON_STR(&definition, "Phalcon\\Http\\Request");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Filter/Escaper services are always shared
	 */
	PHALCON_STR(&name, ISV(filter));
	PHALCON_STR(&definition, "Phalcon\\Filter");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Escaper
	 */
	PHALCON_STR(&name, ISV(escaper));
	PHALCON_STR(&definition, "Phalcon\\Escaper");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Default annotations service
	 */
	PHALCON_STR(&name, ISV(annotations));
	PHALCON_STR(&definition, "Phalcon\\Annotations\\Adapter\\Memory");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Security doesn't need to be shared, but anyways we register it as shared
	 */
	PHALCON_STR(&name, ISV(security));
	PHALCON_STR(&definition, "Phalcon\\Security");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Crypt Service
	 */
	PHALCON_STR(&name, ISV(crypt));
	PHALCON_STR(&definition, "Phalcon\\Crypt");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Flash services are always shared
	 */
	PHALCON_STR(&name, ISV(flash));
	PHALCON_STR(&definition, "Phalcon\\Flash\\Direct");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Flash Session
	 */
	PHALCON_STR(&name, ISV(flashSession));
	PHALCON_STR(&definition, "Phalcon\\Flash\\Session");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Tag/Helpers
	 */
	PHALCON_STR(&name, ISV(tag));
	PHALCON_STR(&definition, "Phalcon\\Tag");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Session is always shared
	 */
	PHALCON_STR(&name, ISV(session));
	PHALCON_STR(&definition, "Phalcon\\Session\\Adapter\\Files");

	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Session/Bag
	 */
	PHALCON_STR(&name, ISV(sessionBag));
	PHALCON_STR(&definition, "Phalcon\\Session\\Bag");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Events Manager is always shared
	 */
	PHALCON_STR(&name, ISV(eventsManager));
	PHALCON_STR(&definition, "Phalcon\\Events\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Model Transaction Manager
	 */
	PHALCON_STR(&name, ISV(transactionManager));
	PHALCON_STR(&definition, "Phalcon\\Mvc\\Model\\Transaction\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Assets Manager
	 */
	PHALCON_STR(&name, ISV(assets));
	PHALCON_STR(&definition, "Phalcon\\Assets\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);
}
