
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

#include "intrusive/rbtree.h"
#include "intrusive/rbtree/node.h"

#include "kernel/main.h"
#include "kernel/rbtree.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/array.h"
#include "kernel/debug.h"

/**
 * Phalcon\Intrusive\Rbtree
 *
 * This class defines rbtree entity and its description
 *
 */
zend_class_entry *phalcon_intrusive_rbtree_ce;

PHP_METHOD(Phalcon_Intrusive_Rbtree, __construct);
PHP_METHOD(Phalcon_Intrusive_Rbtree, insert);
PHP_METHOD(Phalcon_Intrusive_Rbtree, remove);
PHP_METHOD(Phalcon_Intrusive_Rbtree, replace);
PHP_METHOD(Phalcon_Intrusive_Rbtree, find);
PHP_METHOD(Phalcon_Intrusive_Rbtree, first);
PHP_METHOD(Phalcon_Intrusive_Rbtree, last);
PHP_METHOD(Phalcon_Intrusive_Rbtree, prev);
PHP_METHOD(Phalcon_Intrusive_Rbtree, next);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_rbtree_insert, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_rbtree_remove, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_rbtree_replace, 0, 0, 2)
	ZEND_ARG_INFO(0, oldValue)
	ZEND_ARG_INFO(0, newValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_rbtree_find, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_rbtree_prev, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_rbtree_next, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_intrusive_rbtree_method_entry[] = {
	PHP_ME(Phalcon_Intrusive_Rbtree, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Intrusive_Rbtree, insert, arginfo_phalcon_intrusive_rbtree_insert, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Rbtree, remove, arginfo_phalcon_intrusive_rbtree_remove, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Rbtree, replace, arginfo_phalcon_intrusive_rbtree_replace, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Rbtree, find, arginfo_phalcon_intrusive_rbtree_find, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Rbtree, first, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Rbtree, last, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Rbtree, prev, arginfo_phalcon_intrusive_rbtree_prev, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Rbtree, next, arginfo_phalcon_intrusive_rbtree_next, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static void phalcon_zvltree_dtor(zend_resource *rsrc)
{
    struct phalcon_rbtree *rbtree = (struct phalcon_rbtree *) rsrc->ptr;
	struct phalcon_rbtree_node* node;
	while ((node = phalcon_rbtree_first(rbtree)) != NULL) {
		zval obj = {};
		phalcon_rbtree_remove(node, rbtree);
		phalcon_intrusive_rbtree_node_object *node_object = phalcon_intrusive_rbtree_node_object_from_node(node);
		ZVAL_OBJ(&obj, &node_object->std);
		zval_ptr_dtor(&obj);
	}
    efree (rbtree);
}

int phalcon_rbtree_handle;

/**
 * Phalcon\Intrusive\Rbtree initializer
 */
PHALCON_INIT_CLASS(Phalcon_Intrusive_Rbtree){

	PHALCON_REGISTER_CLASS(Phalcon\\Intrusive, Rbtree, intrusive_rbtree, phalcon_intrusive_rbtree_method_entry, 0);

	phalcon_rbtree_handle = zend_register_list_destructors_ex(phalcon_zvltree_dtor, NULL, phalcon_rbtree_handle_name, module_number);

	zend_declare_property_null(phalcon_intrusive_rbtree_ce, SL("_rbtree"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Intrusive\Rbtree constructor
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree, __construct){

	zval zid = {};
	struct phalcon_rbtree *rbtree;

	rbtree = (struct phalcon_rbtree *) emalloc (sizeof (struct phalcon_rbtree));
	phalcon_rbtree_init(rbtree);

	ZVAL_RES(&zid, zend_register_resource(rbtree, phalcon_rbtree_handle));
	phalcon_update_property(getThis(), SL("_rbtree"), &zid);
}

/**
 * Insert node value
 *
 * @param mixed value
 * @return Phalcon\Intrusive\Rbtree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree, insert){

	zval *value, zid = {}, node = {};
	struct phalcon_rbtree *rbtree;
	phalcon_intrusive_rbtree_node_object *node_object;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_rbtree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((rbtree = (struct phalcon_rbtree *)zend_fetch_resource(Z_RES(zid), phalcon_rbtree_handle_name, phalcon_rbtree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(value), phalcon_intrusive_rbtree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_rbtree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", value);
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ(node));
		phalcon_rbtree_insert(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
		RETURN_CTOR(&node);
	} else {
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ_P(value));
		phalcon_rbtree_insert(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
		Z_TRY_ADDREF_P(value);
		RETURN_CTOR(value);
	}
}

/**
 * Remove node value
 *
 * @param mixed value
 * @return boolean
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree, remove){

	zval *value, zid = {}, node = {};
	struct phalcon_rbtree *rbtree;
	phalcon_intrusive_rbtree_node_object *node_object;
	struct phalcon_rbtree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_rbtree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((rbtree = (struct phalcon_rbtree *)zend_fetch_resource(Z_RES(zid), phalcon_rbtree_handle_name, phalcon_rbtree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(value), phalcon_intrusive_rbtree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_rbtree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", value);
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ(node));
		n = phalcon_rbtree_lookup(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
		zval_ptr_dtor(&node);
	} else {
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ_P(value));
		n = phalcon_rbtree_lookup(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
	}
	if (!n) {
		RETURN_FALSE;
	}
	phalcon_rbtree_remove(n, rbtree);
	node_object = phalcon_intrusive_rbtree_node_object_from_node(n);
	RETURN_OBJ(&node_object->std);
}

/**
 * Replace node value
 *
 * @param mixed oldValue
 * @param mixed newValue
 * @return Phalcon\Intrusive\Rbtree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree, replace){

	zval *old_value, *new_value, zid = {}, node = {};
	struct phalcon_rbtree *rbtree;
	phalcon_intrusive_rbtree_node_object *old_node_object, *node_object;
	struct phalcon_rbtree_node *n;

	phalcon_fetch_params(0, 2, 0, &old_value, &new_value);

	phalcon_read_property(&zid, getThis(), SL("_rbtree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((rbtree = (struct phalcon_rbtree *)zend_fetch_resource(Z_RES(zid), phalcon_rbtree_handle_name, phalcon_rbtree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(old_value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(old_value), phalcon_intrusive_rbtree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_rbtree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", old_value);
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ(node));
		n = phalcon_rbtree_lookup(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
		zval_ptr_dtor(&node);
	} else {
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ_P(old_value));
		n = phalcon_rbtree_lookup(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
	}

	if (!n) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(new_value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(new_value), phalcon_intrusive_rbtree_node_ce)) {
		zval new_node = {};
		object_init_ex(&new_node, phalcon_intrusive_rbtree_node_ce);
		PHALCON_CALL_METHOD(NULL, &new_node, "__construct", new_value);
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ(new_node));
		phalcon_rbtree_replace(n, &node_object->node, rbtree);
	} else {
		zval v = {}, new_node = {};
		PHALCON_CALL_METHOD(&v, new_value, "getvalue");
		object_init_ex(&new_node, phalcon_intrusive_rbtree_node_ce);
		PHALCON_CALL_METHOD(NULL, &new_node, "__construct", &v);
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ(new_node));
		phalcon_rbtree_replace(n, &node_object->node, rbtree);
	}
	old_node_object = phalcon_intrusive_rbtree_node_object_from_node(n);
	ZVAL_OBJ(&node, &old_node_object->std);
	zval_ptr_dtor(&node);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}

/**
 * Lookup node value
 *
 * @param mixed value
 * @return Phalcon\Intrusive\Rbtree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree, find){

	zval *value, zid = {}, node = {};
	struct phalcon_rbtree *rbtree;
	phalcon_intrusive_rbtree_node_object *node_object;
	struct phalcon_rbtree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_rbtree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((rbtree = (struct phalcon_rbtree *)zend_fetch_resource(Z_RES(zid), phalcon_rbtree_handle_name, phalcon_rbtree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(value), phalcon_intrusive_rbtree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_rbtree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", value);
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ(node));
		n = phalcon_rbtree_lookup(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
		zval_ptr_dtor(&node);
	} else {
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ_P(value));
		n = phalcon_rbtree_lookup(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
	}
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_rbtree_node_object_from_node(n);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}

/**
 * Returns first node value
 *
 * @return Phalcon\Intrusive\Rbtree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree, first){

	zval zid = {}, node = {};
	struct phalcon_rbtree *rbtree;
	phalcon_intrusive_rbtree_node_object *node_object;
	struct phalcon_rbtree_node *n;

	phalcon_read_property(&zid, getThis(), SL("_rbtree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((rbtree = (struct phalcon_rbtree *)zend_fetch_resource(Z_RES(zid), phalcon_rbtree_handle_name, phalcon_rbtree_handle)) == NULL) {
		RETURN_FALSE;
	}

 	n = phalcon_rbtree_first(rbtree);
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_rbtree_node_object_from_node(n);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}

/**
 * Returns last node value
 *
 * @return Phalcon\Intrusive\Rbtree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree, last){

	zval zid = {}, node = {};
	struct phalcon_rbtree *rbtree;
	phalcon_intrusive_rbtree_node_object *node_object;
	struct phalcon_rbtree_node *n;

	phalcon_read_property(&zid, getThis(), SL("_rbtree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((rbtree = (struct phalcon_rbtree *)zend_fetch_resource(Z_RES(zid), phalcon_rbtree_handle_name, phalcon_rbtree_handle)) == NULL) {
		RETURN_FALSE;
	}

 	n = phalcon_rbtree_last(rbtree);
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_rbtree_node_object_from_node(n);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}

/**
 * Returns prev node value
 *
 * @param mixed $value
 * @return Phalcon\Intrusive\Rbtree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree, prev){

	zval *value, zid = {}, node = {};
	struct phalcon_rbtree *rbtree;
	phalcon_intrusive_rbtree_node_object *node_object;
	struct phalcon_rbtree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_rbtree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((rbtree = (struct phalcon_rbtree *)zend_fetch_resource(Z_RES(zid), phalcon_rbtree_handle_name, phalcon_rbtree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(value), phalcon_intrusive_rbtree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_rbtree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", value);
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ(node));
		n = phalcon_rbtree_lookup(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
		zval_ptr_dtor(&node);
	} else {
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ_P(value));
		n = phalcon_rbtree_lookup(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
	}
	if (!n) {
		RETURN_FALSE;
	}

 	n = phalcon_rbtree_prev(n);
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_rbtree_node_object_from_node(n);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}

/**
 * Returns next node value
 *
 * @param mixed $value
 * @return Phalcon\Intrusive\Rbtree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree, next){

	zval *value, zid = {}, node = {};
	struct phalcon_rbtree *rbtree;
	phalcon_intrusive_rbtree_node_object *node_object;
	struct phalcon_rbtree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_rbtree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((rbtree = (struct phalcon_rbtree *)zend_fetch_resource(Z_RES(zid), phalcon_rbtree_handle_name, phalcon_rbtree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(value), phalcon_intrusive_rbtree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_rbtree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", value);
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ(node));
		n = phalcon_rbtree_lookup(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
		zval_ptr_dtor(&node);
	} else {
		node_object = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ_P(value));
		n = phalcon_rbtree_lookup(&node_object->node, phalcon_intrusive_rbtree_node_compare, rbtree);
	}
	if (!n) {
		RETURN_FALSE;
	}

 	n = phalcon_rbtree_next(n);
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_rbtree_node_object_from_node(n);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}
