
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

#include "kernel/main.h"
#include "kernel/avltree.h"
#include "kernel/exception.h"
#include "kernel/object.h"
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
PHP_METHOD(Phalcon_Intrusive_Avltree, lookup);
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_lookup, 0, 0, 1)
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
	PHP_ME(Phalcon_Intrusive_Avltree, lookup, arginfo_phalcon_intrusive_avltree_lookup, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, first, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, last, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, prev, arginfo_phalcon_intrusive_avltree_prev, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Avltree, next, arginfo_phalcon_intrusive_avltree_next, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

typedef struct {
	zval v;
	phalcon_avltree_node n;
} node_value;

static int node_compare(phalcon_avltree_node * a, phalcon_avltree_node * b) {
  node_value* pa = phalcon_memory_container_of(a, node_value, n);
  node_value* pb = phalcon_memory_container_of(b, node_value, n);
  return phalcon_compare(&pa->v, &pb->v);
}

static void phalcon_zvltree_dtor(zend_resource *rsrc)
{
    phalcon_avltree *avltree = (phalcon_avltree *) rsrc->ptr;
    efree (avltree);
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
	phalcon_update_property_zval(getThis(), SL("_avltree"), &zid);
}

/**
 * Insert node value
 *
 * @param mixed value
 * @return boolean
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, insert){

	zval *value, zid = {};
	phalcon_avltree *avltree;
	node_value *node;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	node = (node_value *) emalloc (sizeof (node_value));

	ZVAL_COPY(&node->v, value);
	phalcon_avltree_insert(&node->n, node_compare, avltree);
}

/**
 * Remove node value
 *
 * @param mixed value
 * @return boolean
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, remove){

	zval *value, zid = {};
	phalcon_avltree *avltree;
	node_value p = {{},{}};
	phalcon_avltree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	ZVAL_COPY(&p.v, value);
 	n = phalcon_avltree_lookup(&p.n, node_compare, avltree);
	if (!n) {
		RETURN_FALSE;
	}
	phalcon_avltree_remove(n, avltree);
}

/**
 * Replace node value
 *
 * @param mixed oldValue
 * @param mixed newValue
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, replace){

	zval *old_value, *new_value, zid = {};
	phalcon_avltree *avltree;
	node_value p = {{},{}}, *new_node;
	phalcon_avltree_node *n;

	phalcon_fetch_params(0, 2, 0, &old_value, &new_value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	ZVAL_COPY(&p.v, old_value);
 	n = phalcon_avltree_lookup(&p.n, node_compare, avltree);
	if (!n) {
		RETURN_FALSE;
	}

	new_node = (node_value *) emalloc (sizeof (node_value));
	ZVAL_COPY(&new_node->v, new_value);
	phalcon_avltree_replace(n, &new_node->n, avltree);
}

/**
 * Lookup node value
 *
 * @param mixed value
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, lookup){

	zval *value, zid = {};
	phalcon_avltree *avltree;
	node_value p = {{},{}}, *node;
	phalcon_avltree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	ZVAL_COPY(&p.v, value);
 	n = phalcon_avltree_lookup(&p.n, node_compare, avltree);
	if (!n) {
		RETURN_FALSE;
	}
	node = phalcon_memory_container_of(n, node_value, n);
	RETURN_CTORW(&node->v);
}

/**
 * Returns first node value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, first){

	zval zid = {};
	phalcon_avltree *avltree;
	node_value *node;
	phalcon_avltree_node *n;

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY);

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
	node = phalcon_memory_container_of(n, node_value, n);
	RETURN_CTORW(&node->v);
}

/**
 * Returns last node value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, last){

	zval zid = {};
	phalcon_avltree *avltree;
	node_value *node;
	phalcon_avltree_node *n;

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY);

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
	node = phalcon_memory_container_of(n, node_value, n);
	RETURN_CTORW(&node->v);
}

/**
 * Returns prev node value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, prev){

	zval *value, zid = {};
	phalcon_avltree *avltree;
	node_value p = {{},{}}, *node;
	phalcon_avltree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	ZVAL_COPY(&p.v, value);
	n = phalcon_avltree_lookup(&p.n, node_compare, avltree);
	if (!n) {
		RETURN_FALSE;
	}

 	n = phalcon_avltree_prev(n);
	if (!n) {
		RETURN_FALSE;
	}
	node = phalcon_memory_container_of(n, node_value, n);
	RETURN_CTORW(&node->v);
}

/**
 * Returns next node value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, next){

	zval *value, zid = {};
	phalcon_avltree *avltree;
	node_value p = {{},{}}, *node;
	phalcon_avltree_node *n;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&zid, getThis(), SL("_avltree"), PH_NOISY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((avltree = (phalcon_avltree *)zend_fetch_resource(Z_RES(zid), phalcon_avltree_handle_name, phalcon_avltree_handle)) == NULL) {
		RETURN_FALSE;
	}

	ZVAL_COPY(&p.v, value);
	n = phalcon_avltree_lookup(&p.n, node_compare, avltree);
	if (!n) {
		RETURN_FALSE;
	}

 	n = phalcon_avltree_next(n);
	if (!n) {
		RETURN_FALSE;
	}
	node = phalcon_memory_container_of(n, node_value, n);
	RETURN_CTORW(&node->v);
}
