
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

#include "acl/adapter.h"
#include "acl/adapterinterface.h"
#include "di/injectable.h"

#include "kernel/main.h"
#include "kernel/object.h"

/**
 * Phalcon\Acl\Adapter
 *
 * Adapter for Phalcon\Acl adapters
 */
PHP_METHOD(Phalcon_Acl_Adapter, setDefaultAction);
PHP_METHOD(Phalcon_Acl_Adapter, getDefaultAction);
PHP_METHOD(Phalcon_Acl_Adapter, getActiveRole);
PHP_METHOD(Phalcon_Acl_Adapter, getActiveResource);
PHP_METHOD(Phalcon_Acl_Adapter, getActiveAccess);

zend_class_entry *phalcon_acl_adapter_ce;

static const zend_function_entry phalcon_acl_adapter_method_entry[] = {
	PHP_ME(Phalcon_Acl_Adapter, setDefaultAction, arginfo_phalcon_acl_adapter_setdefaultaction, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Acl_Adapter, getDefaultAction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Acl_Adapter, getActiveRole, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Acl_Adapter, getActiveResource, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Acl_Adapter, getActiveAccess, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Acl\Adapter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Acl_Adapter){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Acl, Adapter, acl_adapter, phalcon_di_injectable_ce, phalcon_acl_adapter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_long(phalcon_acl_adapter_ce, SL("_defaultAccess"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_acl_adapter_ce, SL("_accessGranted"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_acl_adapter_ce, SL("_activeRole"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_acl_adapter_ce, SL("_activeResource"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_acl_adapter_ce, SL("_activeAccess"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_acl_adapter_ce, 1, phalcon_acl_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Sets the default access level (Phalcon\Acl::ALLOW or Phalcon\Acl::DENY)
 *
 * @param int $defaultAccess
 */
PHP_METHOD(Phalcon_Acl_Adapter, setDefaultAction){

	zval *default_access;

	phalcon_fetch_params(0, 1, 0, &default_access);
	
	phalcon_update_property(getThis(), SL("_defaultAccess"), default_access);
	
}

/**
 * Returns the default ACL access level
 *
 * @return int
 */
PHP_METHOD(Phalcon_Acl_Adapter, getDefaultAction){


	RETURN_MEMBER(getThis(), "_defaultAccess");
}

/**
 * Returns the role which the list is checking if it's allowed to certain resource/access
 *
 * @return string
 */
PHP_METHOD(Phalcon_Acl_Adapter, getActiveRole){


	RETURN_MEMBER(getThis(), "_activeRole");
}

/**
 * Returns the resource which the list is checking if some role can access it
 *
 * @return string
 */
PHP_METHOD(Phalcon_Acl_Adapter, getActiveResource){


	RETURN_MEMBER(getThis(), "_activeResource");
}

/**
 * Returns the access which the list is checking if some role can access it
 *
 * @return string
 */
PHP_METHOD(Phalcon_Acl_Adapter, getActiveAccess){


	RETURN_MEMBER(getThis(), "_activeAccess");
}
