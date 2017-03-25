
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

#include "continueexception.h"

#include <Zend/zend_exceptions.h>

#include "kernel/main.h"

/**
 * Phalcon\ContinueException
 *
 * All framework exceptions should use or extend this exception
 */
zend_class_entry *phalcon_continueexception_ce;

/**
 * Phalcon\ContinueException initializer
 */
PHALCON_INIT_CLASS(Phalcon_ContinueException){

	PHALCON_REGISTER_CLASS_EX(Phalcon, ContinueException, continueexception, zend_ce_exception, NULL, 0);

	return SUCCESS;
}
