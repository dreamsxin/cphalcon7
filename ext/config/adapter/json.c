
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
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  +------------------------------------------------------------------------+
*/

#include "config/adapter/json.h"
#include "config/adapter.h"
#include "config/adapterinterface.h"
#include "pconfig.h"

#include "kernel/main.h"
#include "kernel/file.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/fcall.h"

/**
 * Phalcon\Config\Adapter\Json
 *
 * Reads JSON files and converts them to Phalcon\Config objects.
 *
 * Given the following configuration file:
 *
 *<code>
 *{"phalcon":{"baseuri":"\/phalcon\/"},"models":{"metadata":"memory"}}
 *</code>
 *
 * You can read it as follows:
 *
 *<code>
 *	$config = new Phalcon\Config\Adapter\Json("path/config.json");
 *	echo $config->phalcon->baseuri;
 *	echo $config->models->metadata;
 *</code>
 *
 */
zend_class_entry *phalcon_config_adapter_json_ce;

PHP_METHOD(Phalcon_Config_Adapter_Json, read);

static const zend_function_entry phalcon_config_adapter_json_method_entry[] = {
	PHP_ME(Phalcon_Config_Adapter_Json, read, arginfo_phalcon_config_adapter_read, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_FE_END
};


/**
 * Phalcon\Config\Adapter\Json initializer
 */
PHALCON_INIT_CLASS(Phalcon_Config_Adapter_Json){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Config\\Adapter, Json, config_adapter_json, phalcon_config_adapter_ce, phalcon_config_adapter_json_method_entry, 0);

	zend_class_implements(phalcon_config_adapter_json_ce, 1, phalcon_config_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Load config file
 *
 * @param string $filePath
 */
PHP_METHOD(Phalcon_Config_Adapter_Json, read){

	zval *file_path, *absolute_path = NULL, config_dir_path = {}, base_path = {}, contents = {}, config = {};

	phalcon_fetch_params(0, 1, 1, &file_path, &absolute_path);
	PHALCON_ENSURE_IS_STRING(file_path);

	if (absolute_path == NULL) {
		absolute_path = &PHALCON_GLOBAL(z_false);
	}

	if (zend_is_true(absolute_path)) {
		ZVAL_COPY(&config_dir_path, file_path);
	} else {
		phalcon_read_static_property_ce(&base_path, phalcon_config_adapter_ce, SL("_basePath"), PH_READONLY);
		PHALCON_CONCAT_VV(&config_dir_path, &base_path, file_path);
	}

	phalcon_file_get_contents(&contents, &config_dir_path);
	zval_ptr_dtor(&config_dir_path);

	if (Z_TYPE(contents) == IS_STRING) {
		if (phalcon_json_decode(&config, &contents, 1) != FAILURE) {
			if (Z_TYPE(config) == IS_ARRAY) {
				PHALCON_CALL_METHOD(NULL, getThis(), "val", &config);
			}
			zval_ptr_dtor(&config);
		}
	}
	zval_ptr_dtor(&contents);

	RETURN_THIS();
}
