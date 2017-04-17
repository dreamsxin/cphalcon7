
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

#include "user/module.h"
#include "di/injectable.h"
#include "kernel/main.h"

/**
 * Phalcon\User\Module
 *
 * This class can be used to provide user modules easy access to services
 * in the application
 */
zend_class_entry *phalcon_user_module_ce;

/**
 * Phalcon\User\Module initializer
 */
PHALCON_INIT_CLASS(Phalcon_User_Module){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\User, Module, user_module, phalcon_di_injectable_ce, NULL, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	return SUCCESS;
}
