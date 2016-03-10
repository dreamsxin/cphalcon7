
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

	zval *shared, name, definition;

	PHALCON_CALL_PARENTW(NULL, phalcon_di_factorydefault_ce, getThis(), "__construct");

	shared = &PHALCON_GLOBAL(z_true);

	/**
	 * Mvc Router
	 */
	ZVAL_STRING(&name, ISV(router));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Router");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Mvc Dispatcher
	 */
	ZVAL_STRING(&name, ISV(dispatcher));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Dispatcher");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Mvc Url
	 */
	ZVAL_STRING(&name, ISV(url));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Url");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Models manager for ORM
	 */
	ZVAL_STRING(&name, ISV(modelsManager));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Models meta-data using the Memory adapter
	 */
	ZVAL_STRING(&name, ISV(modelsMetadata));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\MetaData\\Memory");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Models Query for ORM
	 */
	ZVAL_STRING(&name, ISV(modelsQuery));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\Query");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Models Query Builder for ORM
	 */
	ZVAL_STRING(&name, ISV(modelsQueryBuilder));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\Query\\Builder");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Models Criteria for ORM
	 */
	ZVAL_STRING(&name, ISV(modelsCriteria));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\Criteria");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Request/Response are always shared
	 */
	ZVAL_STRING(&name, ISV(response));
	ZVAL_STRING(&definition, "Phalcon\\Http\\Response");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Http Response Cookies
	 */
	ZVAL_STRING(&name, ISV(cookies));
	ZVAL_STRING(&definition, "Phalcon\\Http\\Response\\Cookies");

	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);
	/**
	 * Http Request
	 */
	ZVAL_STRING(&name, ISV(request));
	ZVAL_STRING(&definition, "Phalcon\\Http\\Request");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Filter/Escaper services are always shared
	 */
	ZVAL_STRING(&name, ISV(filter));
	ZVAL_STRING(&definition, "Phalcon\\Filter");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Escaper
	 */
	ZVAL_STRING(&name, ISV(escaper));
	ZVAL_STRING(&definition, "Phalcon\\Escaper");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Default annotations service
	 */
	ZVAL_STRING(&name, ISV(annotations));
	ZVAL_STRING(&definition, "Phalcon\\Annotations\\Adapter\\Memory");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Security doesn't need to be shared, but anyways we register it as shared
	 */
	ZVAL_STRING(&name, ISV(security));
	ZVAL_STRING(&definition, "Phalcon\\Security");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Crypt Service
	 */
	ZVAL_STRING(&name, ISV(crypt));
	ZVAL_STRING(&definition, "Phalcon\\Crypt");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Flash services are always shared
	 */
	ZVAL_STRING(&name, ISV(flash));
	ZVAL_STRING(&definition, "Phalcon\\Flash\\Direct");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Flash Session
	 */
	ZVAL_STRING(&name, ISV(flashSession));
	ZVAL_STRING(&definition, "Phalcon\\Flash\\Session");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Tag/Helpers
	 */
	ZVAL_STRING(&name, ISV(tag));
	ZVAL_STRING(&definition, "Phalcon\\Tag");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Session is always shared
	 */
	ZVAL_STRING(&name, ISV(session));
	ZVAL_STRING(&definition, "Phalcon\\Session\\Adapter\\Files");

	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Session/Bag
	 */
	ZVAL_STRING(&name, ISV(sessionBag));
	ZVAL_STRING(&definition, "Phalcon\\Session\\Bag");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Events Manager is always shared
	 */
	ZVAL_STRING(&name, ISV(eventsManager));
	ZVAL_STRING(&definition, "Phalcon\\Events\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Model Transaction Manager
	 */
	ZVAL_STRING(&name, ISV(transactionManager));
	ZVAL_STRING(&definition, "Phalcon\\Mvc\\Model\\Transaction\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);

	/**
	 * Assets Manager
	 */
	ZVAL_STRING(&name, ISV(assets));
	ZVAL_STRING(&definition, "Phalcon\\Assets\\Manager");
	PHALCON_CALL_SELFW(NULL, "set", &name, &definition, shared);
}
