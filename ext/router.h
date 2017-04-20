
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

#ifndef PHALCON_ROUTER_H
#define PHALCON_ROUTER_H

#include "php_phalcon.h"

#define PHALCON_ROUTER_MODE_DEFAULT		0
#define PHALCON_ROUTER_MODE_NONE		1
#define PHALCON_ROUTER_MODE_REST		2

extern zend_class_entry *phalcon_router_ce;

PHALCON_INIT_CLASS(Phalcon_Router);

#endif /* PHALCON_ROUTER_H */
