
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

#include "translate/adapter.h"
#include "translate/adapterinterface.h"
#include "translate/exception.h"
#include "internal/arginfo.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"

/**
 * Phalcon\Translate\Adapter
 *
 * Base class for Phalcon\Translate adapters
 */
zend_class_entry *phalcon_translate_adapter_ce;

PHP_METHOD(Phalcon_Translate_Adapter, t);
PHP_METHOD(Phalcon_Translate_Adapter, offsetSet);
PHP_METHOD(Phalcon_Translate_Adapter, offsetExists);
PHP_METHOD(Phalcon_Translate_Adapter, offsetUnset);
PHP_METHOD(Phalcon_Translate_Adapter, offsetGet);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_translate_adapter_t, 0, 0, 1)
	ZEND_ARG_INFO(0, translateKey)
	ZEND_ARG_INFO(0, placeholders)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_translate_adapter_method_entry[] = {
	PHP_ME(Phalcon_Translate_Adapter, t, arginfo_phalcon_translate_adapter_t, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Translate_Adapter, offsetSet, arginfo_arrayaccess_offsetset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Translate_Adapter, offsetExists, arginfo_arrayaccess_offsetexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Translate_Adapter, offsetUnset, arginfo_arrayaccess_offsetunset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Translate_Adapter, offsetGet, arginfo_arrayaccess_offsetget, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Translate_Adapter, _, t, arginfo_phalcon_translate_adapter_t, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Translate\Adapter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Translate_Adapter){

	PHALCON_REGISTER_CLASS(Phalcon\\Translate, Adapter, translate_adapter, phalcon_translate_adapter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_class_implements(phalcon_translate_adapter_ce, 2, zend_ce_arrayaccess, phalcon_translate_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Returns the translation string of the given key
 *
 * @param string $translateKey
 * @param array $placeholders
 * @return string
 */
PHP_METHOD(Phalcon_Translate_Adapter, t){

	zval *translate_key, *placeholders = NULL;

	phalcon_fetch_params(0, 1, 1, &translate_key, &placeholders);

	if (!placeholders) {
		placeholders = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_METHOD(getThis(), "query", translate_key, placeholders);
}

/**
 * Sets a translation value
 *
 * @param 	string $offset
 * @param 	string $value
 */
PHP_METHOD(Phalcon_Translate_Adapter, offsetSet){

	zend_throw_exception_ex(phalcon_translate_exception_ce, 0, "'%s' is an immutable ArrayAccess object", "Phalcon\\Translate\\Adapter");
}

/**
 * Check whether a translation key exists
 *
 * @param string $translateKey
 * @return boolean
 */
PHP_METHOD(Phalcon_Translate_Adapter, offsetExists){

	zval *translate_key;

	phalcon_fetch_params(0, 1, 0, &translate_key);

	PHALCON_RETURN_CALL_METHOD(getThis(), "exists", translate_key);
}

/**
 * Unsets a translation from the dictionary
 *
 * @param string $offset
 */
PHP_METHOD(Phalcon_Translate_Adapter, offsetUnset){

	zend_throw_exception_ex(phalcon_translate_exception_ce, 0, "'%s' is an immutable ArrayAccess object", "Phalcon\\Translate\\Adapter");
}

/**
 * Returns the translation related to the given key
 *
 * @param string $translateKey
 * @return string
 */
PHP_METHOD(Phalcon_Translate_Adapter, offsetGet){

	zval *translate_key;

	phalcon_fetch_params(0, 1, 0, &translate_key);
	PHALCON_RETURN_CALL_METHOD(getThis(), "query", translate_key);
}
