
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

#include "cli/dispatcher/exception.h"
#include "cli/../exception.h"

#include "kernel/main.h"

/**
 * Phalcon\Cli\Dispatcher\Exception
 *
 * Exceptions thrown in Phalcon\Cli\Dispatcher will use this class
 *
 */
zend_class_entry *phalcon_cli_dispatcher_exception_ce;

/**
 * Phalcon\Cli\Dispatcher\Exception initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cli_Dispatcher_Exception){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cli\\Dispatcher, Exception, cli_dispatcher_exception, phalcon_exception_ce, NULL, 0);

	return SUCCESS;
}
