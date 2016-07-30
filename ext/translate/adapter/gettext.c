
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

#include "translate/adapter/gettext.h"
#include "translate/adapter.h"
#include "translate/adapterinterface.h"
#include "translate/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/string.h"

/**
 * Phalcon\Translate\Adapter\Gettext
 *
 * Allows to define translation lists using PHP arrays
 *
 */
zend_class_entry *phalcon_translate_adapter_gettext_ce;

PHP_METHOD(Phalcon_Translate_Adapter_Gettext, __construct);
PHP_METHOD(Phalcon_Translate_Adapter_Gettext, query);
PHP_METHOD(Phalcon_Translate_Adapter_Gettext, exists);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_translate_adapter_gettext___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_translate_adapter_gettext_method_entry[] = {
	PHP_ME(Phalcon_Translate_Adapter_Gettext, __construct, arginfo_phalcon_translate_adapter_gettext___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Translate_Adapter_Gettext, query, arginfo_phalcon_translate_adapterinterface_query, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Translate_Adapter_Gettext, exists, arginfo_phalcon_translate_adapterinterface_exists, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Translate\Adapter\Gettext initializer
 */
PHALCON_INIT_CLASS(Phalcon_Translate_Adapter_Gettext){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Translate\\Adapter, Gettext, translate_adapter_gettext, phalcon_translate_adapter_ce, phalcon_translate_adapter_gettext_method_entry, 0);

	zend_declare_property_null(phalcon_translate_adapter_gettext_ce, SL("_locale"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_translate_adapter_gettext_ce, SL("_defaultDomain"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_translate_adapter_gettext_ce, SL("_directory"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_translate_adapter_gettext_ce, 1, phalcon_translate_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Translate\Adapter\Gettext constructor
 *
 * @param array $options
 * @throws \Phalcon\Translate\Exception
 */
PHP_METHOD(Phalcon_Translate_Adapter_Gettext, __construct){

	zval *options, locale, default_domain = {}, directory = {}, setting = {}, *constant, *value;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 0, &options);

	if (Z_TYPE_P(options) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_translate_exception_ce, "Invalid options");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&locale, options, SL("locale"))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_translate_exception_ce, "Parameter \"locale\" is required");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&default_domain, options, SL("defaultDomain"))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_translate_exception_ce, "Parameter \"defaultDomain\" is required");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&directory, options, SL("directory"))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_translate_exception_ce, "Parameter \"directory\" is required");
		return;
	}

	phalcon_update_property_zval(getThis(), SL("_locale"), &locale);
	phalcon_update_property_zval(getThis(), SL("_defaultDomain"), &default_domain);
	phalcon_update_property_zval(getThis(), SL("_directory"), &directory);

	PHALCON_CONCAT_SV(&setting, "LC_ALL=", &locale);
	PHALCON_CALL_FUNCTIONW(NULL, "putenv", &setting);

	if ((constant = zend_get_constant_str(SL("LC_ALL"))) != NULL) {
		PHALCON_CALL_FUNCTIONW(NULL, "setlocale", constant, &locale);
	}

	if (Z_TYPE(directory) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(directory), idx, str_key, value) {
			zval key = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}
			PHALCON_CALL_FUNCTIONW(NULL, "bindtextdomain", &key, value);
		} ZEND_HASH_FOREACH_END();
	} else {
		PHALCON_CALL_FUNCTIONW(NULL, "bindtextdomain", &default_domain, &directory);
	}

	PHALCON_CALL_FUNCTIONW(NULL, "textdomain", &default_domain);
}

/**
 * Returns the translation related to the given key
 *
 * @param string $index
 * @param array $placeholders
 * @param string $domain
 * @return string
 */
PHP_METHOD(Phalcon_Translate_Adapter_Gettext, query){

	zval *index, *placeholders = NULL, *domain = NULL, translation = {}, *value;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 2, &index, &placeholders, &domain);

	if (!domain) {
		PHALCON_CALL_FUNCTIONW(&translation, "gettext", index);
	} else {
		PHALCON_CALL_FUNCTIONW(&translation, "dgettext", domain, index);
	}

	if (placeholders && Z_TYPE_P(placeholders) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(placeholders))) {
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(placeholders), idx, str_key, value) {
			zval key_placeholder = {}, replaced = {}, key = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}

			PHALCON_CONCAT_SVS(&key_placeholder, "%", &key, "%");

			PHALCON_STR_REPLACE(&replaced, &key_placeholder, value, &translation);

			PHALCON_CPY_WRT(&translation, &replaced);
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_CTORW(&translation);
}

/**
 * Check whether is defined a translation key in the internal array
 *
 * @param string $index
 * @return bool
 */
PHP_METHOD(Phalcon_Translate_Adapter_Gettext, exists){

	zval *index, *domain = NULL, translation = {};

	phalcon_fetch_params(0, 1, 1, &index, &domain);

	if (!domain) {
		PHALCON_CALL_FUNCTIONW(&translation, "gettext", index);
	} else {
		PHALCON_CALL_FUNCTIONW(&translation, "dgettext", domain, index);
	}

	if (Z_STRLEN(translation) > 0) {
		RETURN_TRUE;
	}
	
	RETURN_FALSE;
}
