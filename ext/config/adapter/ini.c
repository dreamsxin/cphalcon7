
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

#include "config/adapter/ini.h"
#include "config/adapter.h"
#include "config/adapterinterface.h"
#include "config/exception.h"
#include "pconfig.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/exception.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/operators.h"
#include "kernel/object.h"

/**
 * Phalcon\Config\Adapter\Ini
 *
 * Reads ini files and converts them to Phalcon\Config objects.
 *
 * Given the next configuration file:
 *
 *<code>
 *[database]
 *adapter = Mysql
 *host = localhost
 *username = scott
 *password = cheetah
 *dbname = test_db
 *
 *[phalcon]
 *controllersDir = "../app/controllers/"
 *modelsDir = "../app/models/"
 *viewsDir = "../app/views/"
 *</code>
 *
 * You can read it as follows:
 *
 *<code>
 *	$config = new Phalcon\Config\Adapter\Ini("path/config.ini");
 *	echo $config->phalcon->controllersDir;
 *	echo $config->database->username;
 *</code>
 *
 */
zend_class_entry *phalcon_config_adapter_ini_ce;

PHP_METHOD(Phalcon_Config_Adapter_Ini, read);

static const zend_function_entry phalcon_config_adapter_ini_method_entry[] = {
	PHP_ME(Phalcon_Config_Adapter_Ini, read, arginfo_phalcon_config_adapter_read, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Config\Adapter\Ini initializer
 */
PHALCON_INIT_CLASS(Phalcon_Config_Adapter_Ini){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Config\\Adapter, Ini, config_adapter_ini, phalcon_config_adapter_ce, phalcon_config_adapter_ini_method_entry, 0);

	zend_class_implements(phalcon_config_adapter_ini_ce, 1, phalcon_config_adapterinterface_ce);

	return SUCCESS;
}

static void phalcon_config_adapter_ini_update_zval_directive(zval *arr, zval *section, zval *directive, zval *value)
{
	zval t1 = {}, *tmp1 = &t1, t2 = {}, index = {};
	int i, n;

	assert(Z_TYPE_P(arr) == IS_ARRAY);
	assert(Z_TYPE_P(directive) == IS_ARRAY);

	n = zend_hash_num_elements(Z_ARRVAL_P(directive));
	assert(n > 1);

	if (!phalcon_array_isset_fetch(&t1, arr, section, 0)) {
		array_init(&t1);
	} else if (Z_TYPE_P(&t1) != IS_ARRAY) {
		convert_to_array_ex(&t1);
	}

	for (i = 0; i < n - 1; i++) {
		zval index1 = {};
		phalcon_array_fetch_long(&index1, directive, i, PH_NOISY);
		if (!phalcon_array_isset_fetch(&t2, tmp1, &index, 0)) {
			array_init(&t2);
			phalcon_array_update_zval(tmp1, &index1, &t2, PH_COPY);
		} else if (Z_TYPE_P(&t2) != IS_ARRAY) {
			convert_to_array_ex(&t2);
		}

		tmp1 = &t2;
		PHALCON_PTR_DTOR(&index1);
	}

	phalcon_array_fetch_long(&index, directive, n - 1, PH_NOISY);
	phalcon_array_update_zval(&t1, &index, value, PH_COPY);
	phalcon_array_update_zval(arr, section, &t1, PH_COPY);
	PHALCON_PTR_DTOR(&index);
}

/**
 * Load config file
 *
 * @param string $filePath
 */
PHP_METHOD(Phalcon_Config_Adapter_Ini, read){

	zval *file_path, *absolute_path = NULL, config_dir_path = {}, base_path = {}, ini_config = {}, config = {}, *directives;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 1, &file_path, &absolute_path);
	PHALCON_ENSURE_IS_STRING(file_path);

	if (absolute_path == NULL) {
		absolute_path = &PHALCON_GLOBAL(z_false);
	}

	if (zend_is_true(absolute_path)) {
		PHALCON_CPY_WRT_CTOR(&config_dir_path, file_path);
	} else {
		phalcon_return_static_property_ce(&base_path, phalcon_config_adapter_ce, SL("_basePath"));

		PHALCON_CONCAT_VV(&config_dir_path, &base_path, file_path);
	}

	/** 
	 * Use the standard parse_ini_file
	 */
	PHALCON_CALL_FUNCTIONW(&ini_config, "parse_ini_file", &config_dir_path, &PHALCON_GLOBAL(z_true));

	/** 
	 * Check if the file had errors
	 */
	if (Z_TYPE(ini_config) != IS_ARRAY) {
		zend_throw_exception_ex(phalcon_config_exception_ce, 0, "Configuration file '%s' cannot be read", Z_STRVAL(config_dir_path));
		return;
	}

	array_init(&config);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(ini_config), idx, str_key, directives) {
		zval section = {}, *value;
		if (str_key) {
			ZVAL_STR(&section, str_key);
		} else {
			ZVAL_LONG(&section, idx);
		}

		if (unlikely(Z_TYPE_P(directives) != IS_ARRAY) || zend_hash_num_elements(Z_ARRVAL_P(directives)) == 0) {
			phalcon_array_update_zval(&config, &section, directives, PH_COPY);
		} else {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(directives), idx, str_key, value) {
				zval key = {}, directive_parts = {};
				if (str_key) {
					ZVAL_STR(&key, str_key);
				} else {
					ZVAL_LONG(&key, idx);
				}

				if (str_key && memchr(Z_STRVAL(key), '.', Z_STRLEN(key))) {
					phalcon_fast_explode_str(&directive_parts, SL("."), &key);
					phalcon_config_adapter_ini_update_zval_directive(&config, &section, &directive_parts, value);
				} else {
					phalcon_array_update_multi_2(&config, &section, &key, value, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
		}
	} ZEND_HASH_FOREACH_END();

	if (Z_TYPE(config) == IS_ARRAY) {
		PHALCON_CALL_METHODW(NULL, getThis(), "val", &config);
	}

	RETURN_THISW();
}
