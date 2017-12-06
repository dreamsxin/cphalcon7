
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

#ifndef PHALCON_DEBUG_H
#define PHALCON_DEBUG_H

#include "php_phalcon.h"

extern zend_class_entry *phalcon_debug_ce;

PHALCON_INIT_CLASS(Phalcon_Debug);

#define PHALCON_DEBUG_LOG(message) PHALCON_CALL_CE_STATIC(NULL, phalcon_debug_ce, "log", message);
#define PHALCON_DEBUG_TYPE_LOG(message, type) PHALCON_CALL_CE_STATIC(NULL, phalcon_debug_ce, "log", message, type);

#endif /* PHALCON_DEBUG_H */
