
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
  |          Nikolaos Dimopoulos <nikos@niden.net>                         |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_DI_H
#define PHALCON_DI_H

#include "php_phalcon.h"

extern zend_class_entry *phalcon_di_ce;

PHALCON_INIT_CLASS(Phalcon_Di);

PHALCON_STATIC void phalcon_di_set_service(zval *this_ptr, zval *name, zval *services, int flags);
PHALCON_STATIC void phalcon_di_set_services(zval *this_ptr, zval *services);

#endif /* PHALCON_DI_H */
