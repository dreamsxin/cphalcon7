
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

#include "socket/exception.h"
#include "socket/../exception.h"

#include "kernel/main.h"

/**
 * Phalcon\Socket\Exception
 *
 * Exceptions thrown in Phalcon\Socket will use this class
 *
 */
zend_class_entry *phalcon_socket_exception_ce;

/**
 * Phalcon\Socket\Exception initializer
 */
PHALCON_INIT_CLASS(Phalcon_Socket_Exception){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Socket, Exception, socket_exception, phalcon_exception_ce, NULL, 0);

	return SUCCESS;
}
