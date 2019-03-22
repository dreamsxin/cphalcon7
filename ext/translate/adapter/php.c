
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

#include "translate/adapter/php.h"
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
#include "kernel/require.h"

/**
 * Phalcon\Translate\Adapter\Php
 *
 * Allows to define translation lists using PHP file
 *
 *<code>
 *  $t = new \Phalcon\Translate\Adapter\Php(array(
 *      'locale' => 'en_US.utf8',
 *      'directory' => __DIR__ . DIRECTORY_SEPARATOR . 'locale'
 * ));
 *</code>
 */
zend_class_entry *phalcon_translate_adapter_php_ce;

PHP_METHOD(Phalcon_Translate_Adapter_Php, __construct);
PHP_METHOD(Phalcon_Translate_Adapter_Php, query);
PHP_METHOD(Phalcon_Translate_Adapter_Php, exists);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_translate_adapter_php___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_translate_adapter_php_method_entry[] = {
	PHP_ME(Phalcon_Translate_Adapter_Php, __construct, arginfo_phalcon_translate_adapter_php___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Translate_Adapter_Php, query, arginfo_phalcon_translate_adapterinterface_query, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Translate_Adapter_Php, exists, arginfo_phalcon_translate_adapterinterface_exists, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Translate\Adapter\Php initializer
 */
PHALCON_INIT_CLASS(Phalcon_Translate_Adapter_Php){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Translate\\Adapter, Php, translate_adapter_php, phalcon_translate_adapter_ce, phalcon_translate_adapter_php_method_entry, 0);

	zend_declare_property_null(phalcon_translate_adapter_php_ce, SL("_translate"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_translate_adapter_php_ce, 1, phalcon_translate_adapterinterface_ce);
	return SUCCESS;
}

/**
 * Phalcon\Translate\Adapter\Php constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Translate_Adapter_Php, __construct){

	zval *options, locale = {}, directory = {}, file = {}, translate = {};

	phalcon_fetch_params(1, 1, 0, &options);

	if (Z_TYPE_P(options) != IS_ARRAY) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_translate_exception_ce, "Invalid options");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&locale, options, SL("locale"), PH_READONLY)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_translate_exception_ce, "Translation locale was not provided");
		return;
	}

	if (phalcon_array_isset_fetch_str(&directory, options, SL("directory"), PH_READONLY)) {
		zval path = {};
		phalcon_add_trailing_slash(&path, &directory);
		PHALCON_MM_ADD_ENTRY(&path);
		PHALCON_CONCAT_VVS(&file, &path, &locale, ".php");
	} else {
		PHALCON_CONCAT_VS(&file, &locale, ".php");
	}
	PHALCON_MM_ADD_ENTRY(&file);
	if (phalcon_require_ret(&translate, Z_STRVAL(file)) == FAILURE) {
		zend_throw_exception_ex(phalcon_translate_exception_ce, 0, "Translation file '%s' cannot be read", Z_STRVAL(file));
		RETURN_MM();
	}
	PHALCON_MM_ADD_ENTRY(&translate);

	if (Z_TYPE(translate) != IS_ARRAY) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_translate_exception_ce, "Translation data must be an array");
		return;
	}

	phalcon_update_property(getThis(), SL("_translate"), &translate);
	RETURN_MM();
}

/**
 * Returns the translation related to the given key
 *
 * @param string $index
 * @param array $placeholders
 * @return string
 */
PHP_METHOD(Phalcon_Translate_Adapter_Php, query){

	zval *index, *placeholders = NULL, translate = {}, translation = {};
	zval *value;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 1, 1, &index, &placeholders);

	if (!placeholders) {
		placeholders = &PHALCON_GLOBAL(z_null);
	}
	phalcon_read_property(&translate, getThis(), SL("_translate"), PH_NOISY|PH_READONLY);

	if (!phalcon_array_isset_fetch(&translation, &translate, index, PH_READONLY)) {
		ZVAL_COPY_VALUE(&translation, index);
	}

	if (Z_TYPE_P(placeholders) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(placeholders), idx, str_key, value) {
			zval key = {}, key_placeholder = {}, replaced = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}

			PHALCON_CONCAT_SVS(&key_placeholder, "%", &key, "%");
			PHALCON_MM_ADD_ENTRY(&key_placeholder);

			PHALCON_STR_REPLACE(&replaced, &key_placeholder, value, &translation);
			PHALCON_MM_ADD_ENTRY(&replaced);

			ZVAL_COPY_VALUE(&translation, &replaced);
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_MM_CTOR(&translation);
}

/**
 * Check whether is defined a translation key in the internal array
 *
 * @param string $index
 * @return boolean
 */
PHP_METHOD(Phalcon_Translate_Adapter_Php, exists){

	zval *index, translate = {};

	phalcon_fetch_params(0, 1, 0, &index);

	phalcon_read_property(&translate, getThis(), SL("_translate"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset(&translate, index)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
