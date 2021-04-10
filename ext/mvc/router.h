
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

#ifndef PHALCON_MVC_ROUTER_H
#define PHALCON_MVC_ROUTER_H

#include "php_phalcon.h"

#if PHALCON_TREEROUTER
#include "kernel/r3/r3.h"

typedef struct {
	R3Node *tree;
	zend_object std;
} phalcon_mvc_router_object;

static inline phalcon_mvc_router_object *phalcon_mvc_router_object_from_obj(zend_object *obj) {
	return (phalcon_mvc_router_object*)((char*)(obj) - XtOffsetOf(phalcon_mvc_router_object, std));
}
#endif

extern zend_class_entry *phalcon_mvc_router_ce;

PHALCON_INIT_CLASS(Phalcon_Mvc_Router);

#endif /* PHALCON_MVC_ROUTER_H */
