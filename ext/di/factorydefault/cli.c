
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

#include "di.h"
#include "di/factorydefault/cli.h"
#include "di/factorydefault.h"
#include "di/service.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/string.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/object.h"

#include "interned-strings.h"

/**
 * Phalcon\Di\FactoryDefault\CLI
 *
 * This is a variant of the standard Phalcon\Di. By default it automatically
 * registers all the services provided by the framework.
 * Thanks to this, the developer does not need to register each service individually.
 * This class is specially suitable for CLI applications
 */
zend_class_entry *phalcon_di_factorydefault_cli_ce;

PHP_METHOD(Phalcon_Di_FactoryDefault_CLI, __construct);

static const zend_function_entry phalcon_di_factorydefault_cli_method_entry[] = {
	PHP_ME(Phalcon_Di_FactoryDefault_CLI, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_FE_END
};

/**
 * Phalcon\Di\FactoryDefault\CLI initializer
 */
PHALCON_INIT_CLASS(Phalcon_Di_FactoryDefault_CLI){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Di\\FactoryDefault, CLI, di_factorydefault_cli, phalcon_di_factorydefault_ce, phalcon_di_factorydefault_cli_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Di\FactoryDefault\CLI constructor
 */
PHP_METHOD(Phalcon_Di_FactoryDefault_CLI, __construct){

	zval *shared, name = {}, definition = {};

	PHALCON_CALL_PARENT(NULL, phalcon_di_factorydefault_cli_ce, getThis(), "__construct");

	shared = &PHALCON_GLOBAL(z_true);

	ZVAL_STRING(&name, ISV(router));
	ZVAL_STRING(&definition, "Phalcon\\CLI\\Router");
	PHALCON_CALL_SELF(NULL, "set", &name, &definition, shared);

	ZVAL_STRING(&name, ISV(dispatcher));
	ZVAL_STRING(&definition, "Phalcon\\CLI\\Dispatcher");
	PHALCON_CALL_SELF(NULL, "set", &name, &definition, shared);
}
