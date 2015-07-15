
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

	zval *shared, *name = NULL, *definition = NULL;

	PHALCON_MM_GROW();

	PHALCON_CALL_PARENT(NULL, phalcon_di_factorydefault_ce, getThis(), "__construct");

	shared = &PHALCON_GLOBAL(z_true);

	/**
	 * Mvc Router
	 */
	PHALCON_INIT_VAR(name);
	ZVAL_STR(name, IS(router));

	PHALCON_INIT_VAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Mvc\\Router");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Mvc Dispatcher
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(dispatcher));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Mvc\\Dispatcher");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Mvc Url
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(url));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Mvc\\Url");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Models manager for ORM
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(modelsManager));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Mvc\\Model\\Manager");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Models meta-data using the Memory adapter
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(modelsMetadata));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Mvc\\Model\\MetaData\\Memory");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Models Query for ORM
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(modelsQuery));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Mvc\\Model\\Query");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Models Query Builder for ORM
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(modelsQueryBuilder));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Mvc\\Model\\Query\\Builder");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Models Criteria for ORM
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(modelsCriteria));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Mvc\\Model\\Criteria");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Request/Response are always shared
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(response));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Http\\Response");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Http Response Cookies
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(cookies));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Http\\Response\\Cookies");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);
	/**
	 * Http Request
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(request));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Http\\Request");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Filter/Escaper services are always shared
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(filter));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Filter");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Escaper
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(escaper));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Escaper");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Default annotations service
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(annotations));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Annotations\\Adapter\\Memory");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Security doesn't need to be shared, but anyways we register it as shared
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(security));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Security");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Crypt Service
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(crypt));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Crypt");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Flash services are always shared
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(flash));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Flash\\Direct");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Flash Session
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(flashSession));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Flash\\Session");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Tag/Helpers
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(tag));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Tag");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Session is always shared
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(session));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Session\\Adapter\\Files");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Session/Bag
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(sessionBag));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Session\\Bag");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Events Manager is always shared
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(eventsManager));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Events\\Manager");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Model Transaction Manager
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(transactionManager));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Mvc\\Model\\Transaction\\Manager");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	/**
	 * Assets Manager
	 */
	PHALCON_INIT_NVAR(name);
	ZVAL_STR(name, IS(assets));

	PHALCON_INIT_NVAR(definition);
	ZVAL_STRING(definition, "Phalcon\\Assets\\Manager");

	PHALCON_CALL_SELF(NULL, "set", name, definition, shared);

	PHALCON_MM_RESTORE();
}
