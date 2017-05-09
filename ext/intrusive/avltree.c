
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

#include "intrusive/avltree.h"
#include "intrusive/avltree/node.h"

#include "kernel/main.h"
#include "kernel/avltree.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"

/**
 * Phalcon\Intrusive\Avltree
 *
 * This class defines avltree entity and its description
 *
 */
zend_class_entry *phalcon_intrusive_avltree_ce;

PHP_METHOD(Phalcon_Intrusive_Avltree, __construct);
PHP_METHOD(Phalcon_Intrusive_Avltree, insert);
PHP_METHOD(Phalcon_Intrusive_Avltree, remove);
PHP_METHOD(Phalcon_Intrusive_Avltree, replace);
PHP_METHOD(Phalcon_Intrusive_Avltree, find);
PHP_METHOD(Phalcon_Intrusive_Avltree, first);
PHP_METHOD(Phalcon_Intrusive_Avltree, last);
PHP_METHOD(Phalcon_Intrusive_Avltree, prev);
PHP_METHOD(Phalcon_Intrusive_Avltree, next);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_insert, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_remove, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_replace, 0, 0, 2)
	ZEND_ARG_INFO(0, oldValue)
	ZEND_ARG_INFO(0, newValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_find, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_prev, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_next, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_intrusive_avltree_method_entry[] = {
	PHP_ME(Phalcon_Intrusive_Avltree, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Intrusive_Avltree, insert, arginfo_phalcon_intrusive_avltree_insert, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, remove, arginfo_phalcon_intrusive_avltree_remove, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, replace, arginfo_phalcon_intrusive_avltree_replace, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, find, arginfo_phalcon_intrusive_avltree_find, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, first, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, last, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, prev, arginfo_phalcon_intrusive_avltree_prev, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, next, arginfo_phalcon_intrusive_avltree_next, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static void phalcon_zvltree_dtor(zend_resource *rsrc)
{
    phalcon_avltree *avltree = (phalcon_avltree *) rsrc->ptr;
	phalcon_avltree_node* node;
	while ((node = phalcon_avltree_first(avltree)) != NULL) {
		zval obj = {};
		phalcon_avltree_remove(node, avltree);
		phalcon_intrusive_avltree_node_object *node_object = phalcon_intrusive_avltree_node_object_from_node(node);
		ZVAL_OBJ(&obj, &node_object->std);
		zval_ptr_dtor(&obj);
	}
    efree(avltree);
}

int phalcon_avltree_handle;

/**
 * Phalcon\Intrusive\Avltree initializer
 */
PHALCON_INIT_CLASS(Phalcon_Intrusive_Avltree){

	PHALCON_REGISTER_CLASS(Phalcon\\Intrusive, Avltree, intrusive_avltree, phalcon_intrusive_avltree_method_entry, 0);

	phalcon_avltree_handle = zend_register_list_destructors_ex(phalcon_zvltree_dtor, NULL, phalcon_avltree_handle_name, module_number);

	zend_declare_property_null(phalcon_intrusive_avltree_ce, SL("_avltree"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Intrusive\Avltree constructor
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, __construct){

	zval zid = {};
	phalcon_avltree *avltree;

	avltree = (phalcon_avltree *) emalloc (sizeof (phalcon_avltree));
	phalcon_avltree_init(avltree);

	ZVAL_RES(&zid, zend_register_resource(avltree, phalcon_avltree_handle));
	phalcon_update_property(getThis(), SL("_avltree"), &zid);
}

/**
 * Insert node value
 *
 * @param mixed value
 * @return Phalcon\Intrusive\Avltree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, insert){

	zval *value, zid = {}, node = {};
	phalcon_avltree *avltree;
	phalcon_intrusive_avltree_node_object *node_object;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(value), phalcon_intrusive_avltree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_avltree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", value);
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ(node));
		phalcon_avltree_insert(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
		RETURN_CTOR(&node);
	} else {
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ_P(value));
		phalcon_avltree_insert(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
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
PHP_METHOD(Phalcon_Intrusive_Avltree, remove){

	zval *value, zid = {}, node = {};
	phalcon_avltree *avltree;
	phalcon_intrusive_avltree_node_object *node_object;
	phalcon_avltree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(value), phalcon_intrusive_avltree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_avltree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", value);
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ(node));
		n = phalcon_avltree_lookup(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
		zval_ptr_dtor(&node);
	} else {
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ_P(value));
		n = phalcon_avltree_lookup(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
	}
	if (!n) {
		RETURN_FALSE;
	}
	phalcon_avltree_remove(n, avltree);
	node_object = phalcon_intrusive_avltree_node_object_from_node(n);
	RETURN_OBJ(&node_object->std);
}

/**
 * Replace node value
 *
 * @param mixed oldValue
 * @param mixed newValue
 * @return Phalcon\Intrusive\Avltree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, replace){

	zval *old_value, *new_value, zid = {}, node = {};
	phalcon_avltree *avltree;
	phalcon_intrusive_avltree_node_object *old_node_object,  *node_object;
	phalcon_avltree_node *n;

	phalcon_fetch_params(0, 2, 0, &old_value, &new_value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(old_value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(old_value), phalcon_intrusive_avltree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_avltree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", old_value);
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ(node));
		n = phalcon_avltree_lookup(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
		zval_ptr_dtor(&node);
	} else {
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ_P(old_value));
		n = phalcon_avltree_lookup(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
	}

	if (!n) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(new_value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(new_value), phalcon_intrusive_avltree_node_ce)) {
		zval new_node = {};
		object_init_ex(&new_node, phalcon_intrusive_avltree_node_ce);
		PHALCON_CALL_METHOD(NULL, &new_node, "__construct", new_value);
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ(new_node));
		phalcon_avltree_replace(n, &node_object->node, avltree);
	} else {
		Z_TRY_ADDREF_P(new_value);
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ_P(new_value));
		phalcon_avltree_replace(n, &node_object->node, avltree);
	}
	old_node_object = phalcon_intrusive_avltree_node_object_from_node(n);
	ZVAL_OBJ(&node, &old_node_object->std);
	zval_ptr_dtor(&node);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}

/**
 * Lookup node value
 *
 * @param mixed value
 * @return Phalcon\Intrusive\Avltree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, find){

	zval *value, zid = {}, node = {};
	phalcon_avltree *avltree;
	phalcon_intrusive_avltree_node_object *node_object;
	phalcon_avltree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(value), phalcon_intrusive_avltree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_avltree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", value);
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ(node));
		n = phalcon_avltree_lookup(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
		zval_ptr_dtor(&node);
	} else {
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ_P(value));
		n = phalcon_avltree_lookup(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
	}
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_avltree_node_object_from_node(n);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}

/**
 * Returns first node value
 *
 * @return Phalcon\Intrusive\Avltree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, first){

	zval zid = {}, node = {};
	phalcon_avltree *avltree;
	phalcon_intrusive_avltree_node_object *node_object;
	phalcon_avltree_node *n;

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

 	n = phalcon_avltree_first(avltree);
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_avltree_node_object_from_node(n);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}

/**
 * Returns last node value
 *
 * @return Phalcon\Intrusive\Avltree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, last){

	zval zid = {}, node = {};
	phalcon_avltree *avltree;
	phalcon_intrusive_avltree_node_object *node_object;
	phalcon_avltree_node *n;

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

 	n = phalcon_avltree_last(avltree);
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_avltree_node_object_from_node(n);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}

/**
 * Returns prev node value
 *
 * @param mixed $value
 * @return Phalcon\Intrusive\Avltree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, prev){

	zval *value, zid = {}, node = {};
	phalcon_avltree *avltree;
	phalcon_intrusive_avltree_node_object *node_object;
	phalcon_avltree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(value), phalcon_intrusive_avltree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_avltree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", value);
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ(node));
		n = phalcon_avltree_lookup(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
		zval_ptr_dtor(&node);
	} else {
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ_P(value));
		n = phalcon_avltree_lookup(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
	}
	if (!n) {
		RETURN_FALSE;
	}

 	n = phalcon_avltree_prev(n);
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_avltree_node_object_from_node(n);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}

/**
 * Returns next node value
 *
 * @param mixed $value
 * @return Phalcon\Intrusive\Avltree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, next){

	zval *value, zid = {}, node = {};
	phalcon_avltree *avltree;
	phalcon_intrusive_avltree_node_object *node_object;
	phalcon_avltree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(value), phalcon_intrusive_avltree_node_ce)) {
		object_init_ex(&node, phalcon_intrusive_avltree_node_ce);
		PHALCON_CALL_METHOD(NULL, &node, "__construct", value);
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ(node));
		n = phalcon_avltree_lookup(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
		zval_ptr_dtor(&node);
	} else {
		node_object = phalcon_intrusive_avltree_node_object_from_obj(Z_OBJ_P(value));
		n = phalcon_avltree_lookup(&node_object->node, phalcon_intrusive_avltree_node_compare, avltree);
	}
	if (!n) {
		RETURN_FALSE;
	}

 	n = phalcon_avltree_next(n);
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_avltree_node_object_from_node(n);
	ZVAL_OBJ(&node, &node_object->std);
	RETURN_CTOR(&node);
}
