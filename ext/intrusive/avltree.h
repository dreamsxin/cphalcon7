
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

#ifndef PHALCON_INTRUSIVE_AVLTREE_H
#define PHALCON_INTRUSIVE_AVLTREE_H

#include "php_phalcon.h"

extern int phalcon_avltree_handle;
#define phalcon_avltree_handle_name "avltree"

extern zend_class_entry *phalcon_intrusive_avltree_ce;

PHALCON_INIT_CLASS(Phalcon_Intrusive_Avltree);

#endif /* PHALCON_INTRUSIVE_AVLTREE_H */
