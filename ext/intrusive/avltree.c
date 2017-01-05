
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
PHP_METHOD(Phalcon_Intrusive_Avltree, repace);
PHP_METHOD(Phalcon_Intrusive_Avltree, lookup);
PHP_METHOD(Phalcon_Intrusive_Avltree, first);
PHP_METHOD(Phalcon_Intrusive_Avltree, last);
PHP_METHOD(Phalcon_Intrusive_Avltree, prev);
PHP_METHOD(Phalcon_Intrusive_Avltree, next);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_insert, 0, 0, 1)
	ZEND_ARG_INFO(0, nodeValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_remove, 0, 0, 1)
	ZEND_ARG_INFO(0, nodeValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_replace, 0, 0, 2)
	ZEND_ARG_INFO(0, oldNodeValue)
	ZEND_ARG_INFO(0, newNodeValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_lookup, 0, 0, 1)
	ZEND_ARG_INFO(0, nodeValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_prev, 0, 0, 1)
	ZEND_ARG_INFO(0, nodeValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_intrusive_avltree_next, 0, 0, 1)
	ZEND_ARG_INFO(0, nodeValue)
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

static void phalcon_zvltree_dtor(zend_resource *rsrc)
{
    phalcon_avltree *avltree = (phalcon_avltree *) rsrc->ptr;
    efree (avltree);
}

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

	ZVAL_RES(&zid, zend_register_resource(avltree, phalcon_avltree_handle));
	phalcon_update_property_zval(getThis(), SL("_qravltree"), &zid);
}

/**
 * Insert node value
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, insert){


}

/**
 * Remove node value
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, remove){


}

/**
 * Replace node value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, replace){


}

/**
 * Lookup node value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, lookup){


}

/**
 * Returns first node value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, first){


}

/**
 * Returns last node value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, last){


}

/**
 * Returns prev node value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, prev){


}

/**
 * Returns next node value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Intrusive_Avltree, next){


}
