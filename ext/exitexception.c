
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

#include "exitexception.h"

#include <Zend/zend_exceptions.h>

#include "kernel/main.h"

/**
 * Phalcon\ExitException
 */
zend_class_entry *phalcon_exitexception_ce;

/**
 * Phalcon\ExitException initializer
 */
PHALCON_INIT_CLASS(Phalcon_ExitException){

	PHALCON_REGISTER_CLASS_EX(Phalcon, ExitException, exitexception, zend_ce_exception, NULL, 0);

	return SUCCESS;
}
