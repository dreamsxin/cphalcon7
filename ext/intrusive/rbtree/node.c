
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

#include "intrusive/rbtree/node.h"

#include "kernel/main.h"
#include "kernel/rbtree.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/operators.h"

/**
 * Phalcon\Intrusive\Rbtree\Node
 *
 * This class defines rbtree node entity
 *
 */
zend_class_entry *phalcon_intrusive_rbtree_node_ce;

PHP_METHOD(Phalcon_Intrusive_Rbtree_Node, __construct);
PHP_METHOD(Phalcon_Intrusive_Rbtree_Node, setValue);
PHP_METHOD(Phalcon_Intrusive_Rbtree_Node, getValue);
PHP_METHOD(Phalcon_Intrusive_Rbtree_Node, prev);
PHP_METHOD(Phalcon_Intrusive_Rbtree_Node, next);

ZEND_BEGIN_ARG_INFO_EX(phalcon_intrusive_rbtree_node___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(phalcon_intrusive_rbtree_node_setvalue, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_intrusive_rbtree_node_method_entry[] = {
	PHP_ME(Phalcon_Intrusive_Rbtree_Node, __construct, phalcon_intrusive_rbtree_node___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Intrusive_Rbtree_Node, setValue, phalcon_intrusive_rbtree_node_setvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Rbtree_Node, getValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Rbtree_Node, prev, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Intrusive_Rbtree_Node, next, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_intrusive_rbtree_node_object_handlers;
zend_object* phalcon_intrusive_rbtree_node_object_create_handler(zend_class_entry *ce)
{
	phalcon_intrusive_rbtree_node_object *intern = ecalloc(1, sizeof(phalcon_intrusive_rbtree_node_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_intrusive_rbtree_node_object_handlers;
	return &intern->std;
}

void phalcon_intrusive_rbtree_node_object_free_handler(zend_object *object)
{
	zend_object_std_dtor(object);
}

/**
 * Phalcon\Intrusive\Rbtree\Node initializer
 */
PHALCON_INIT_CLASS(Phalcon_Intrusive_Rbtree_Node){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Intrusive\\Rbtree, Node, intrusive_rbtree_node, phalcon_intrusive_rbtree_node_method_entry, 0);

	zend_declare_property_null(phalcon_intrusive_rbtree_node_ce, SL("_value"), ZEND_ACC_PROTECTED);
	return SUCCESS;
}

/**
 * Phalcon\Intrusive\Rbtree\Node constructor
 *
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree_Node, __construct){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_update_property_zval(getThis(), SL("_value"), value);
}

/**
 * Sets the value
 *
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree_Node, setValue){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_update_property_zval(getThis(), SL("_value"), value);
	RETURN_THIS();
}

/**
 * Gets the value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree_Node, getValue){


	RETURN_MEMBER(getThis(), "_value");
}

/**
 * Gets prev node
 *
 * @return Phalcon\Intrusive\Rbtree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree_Node, prev){

	phalcon_intrusive_rbtree_node_object *intern, *node_object;
	struct phalcon_rbtree_node *n;

	intern = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ_P(getThis()));
	n = phalcon_rbtree_prev(&intern->node);
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_rbtree_node_object_from_node(n);
	RETURN_OBJ(&node_object->std);
}

/**
 * Gets next node
 *
 * @return Phalcon\Intrusive\Rbtree\Node
 */
PHP_METHOD(Phalcon_Intrusive_Rbtree_Node, next){

	phalcon_intrusive_rbtree_node_object *intern, *node_object;
	struct phalcon_rbtree_node *n;

	intern = phalcon_intrusive_rbtree_node_object_from_obj(Z_OBJ_P(getThis()));
	n = phalcon_rbtree_next(&intern->node);
	if (!n) {
		RETURN_FALSE;
	}
	node_object = phalcon_intrusive_rbtree_node_object_from_node(n);
	RETURN_OBJ(&node_object->std);
}
