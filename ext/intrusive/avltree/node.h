
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

#ifndef PHALCON_INTRUSIVE_AVLTREE_NODE_H
#define PHALCON_INTRUSIVE_AVLTREE_NODE_H

#include "php_phalcon.h"

#include "kernel/operators.h"
#include "kernel/object.h"
#include "kernel/avltree.h"

typedef struct {
	phalcon_avltree_node node;
	zend_object std;
} phalcon_intrusive_avltree_node_object;

static inline phalcon_intrusive_avltree_node_object *phalcon_intrusive_avltree_node_object_from_obj(zend_object *obj) {
	return (phalcon_intrusive_avltree_node_object*)((char*)(obj) - XtOffsetOf(phalcon_intrusive_avltree_node_object, std));
}

static inline phalcon_intrusive_avltree_node_object *phalcon_intrusive_avltree_node_object_from_node(phalcon_avltree_node *node) {
	return (phalcon_intrusive_avltree_node_object*)((char*)(node) - XtOffsetOf(phalcon_intrusive_avltree_node_object, node));
}

static inline int phalcon_intrusive_avltree_node_compare(phalcon_avltree_node * a, phalcon_avltree_node * b) {
	zval av = {}, bv = {};
	phalcon_intrusive_avltree_node_object *aobject, *bobject;

	aobject = phalcon_intrusive_avltree_node_object_from_node(a);
	bobject = phalcon_intrusive_avltree_node_object_from_node(b);

	phalcon_read_object_property(&av, &aobject->std, SL("_value"), PH_NOISY|PH_READONLY);
	phalcon_read_object_property(&bv, &bobject->std, SL("_value"), PH_NOISY|PH_READONLY);
	return phalcon_compare(&av, &bv);
}

extern zend_class_entry *phalcon_intrusive_avltree_node_ce;

PHALCON_INIT_CLASS(Phalcon_Intrusive_Avltree_Node);

#endif /* PHALCON_INTRUSIVE_AVLTREE_NODE_H */
