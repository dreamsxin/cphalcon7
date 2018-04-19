
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

#ifndef PHALCON_MVC_VIEW_H
#define PHALCON_MVC_VIEW_H

#include "php_phalcon.h"

/* Render level constraints */
#define PHALCON_VIEW_LEVEL_NO_RENDER        0
#define PHALCON_VIEW_LEVEL_ACTION           1
#define PHALCON_VIEW_LEVEL_BEFORE_TEMPLATE  2
#define PHALCON_VIEW_LEVEL_CONTROLLER       3
#define PHALCON_VIEW_LEVEL_NAMESPACE        4
#define PHALCON_VIEW_LEVEL_AFTER_TEMPLATE   5
#define PHALCON_VIEW_LEVEL_MAIN             6

extern zend_class_entry *phalcon_mvc_view_ce;

PHALCON_INIT_CLASS(Phalcon_Mvc_View);

#endif /* PHALCON_MVC_VIEW_H */
