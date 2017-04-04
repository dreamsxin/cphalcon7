
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

#include "annotations/adapter/files.h"
#include "annotations/adapterinterface.h"
#include "annotations/adapter.h"
#include "annotations/exception.h"

#include <Zend/zend_smart_str.h>
#include <ext/standard/php_var.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/file.h"
#include "kernel/concat.h"
#include "kernel/require.h"
#include "kernel/operators.h"
#include "kernel/exception.h"

/**
 * Phalcon\Annotations\Adapter\Files
 *
 * Stores the parsed annotations in files. This adapter is suitable for production
 *
 *<code>
 * $annotations = new \Phalcon\Annotations\Adapter\Files(array(
 *    'annotationsDir' => 'app/cache/annotations/'
 * ));
 *</code>
 */
zend_class_entry *phalcon_annotations_adapter_files_ce;

PHP_METHOD(Phalcon_Annotations_Adapter_Files, __construct);
PHP_METHOD(Phalcon_Annotations_Adapter_Files, read);
PHP_METHOD(Phalcon_Annotations_Adapter_Files, write);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_adapter_files___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_adapter_files_read, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_adapter_files_write, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_annotations_adapter_files_method_entry[] = {
	PHP_ME(Phalcon_Annotations_Adapter_Files, __construct, arginfo_phalcon_annotations_adapter_files___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Annotations_Adapter_Files, read, arginfo_phalcon_annotations_adapter_files_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Adapter_Files, write, arginfo_phalcon_annotations_adapter_files_write, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Annotations\Adapter\Files initializer
 */
PHALCON_INIT_CLASS(Phalcon_Annotations_Adapter_Files){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Annotations\\Adapter, Files, annotations_adapter_files, phalcon_annotations_adapter_ce, phalcon_annotations_adapter_files_method_entry, 0);

	zend_declare_property_string(phalcon_annotations_adapter_files_ce, SL("_annotationsDir"), "./", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_annotations_adapter_files_ce, 1, phalcon_annotations_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Annotations\Adapter\Files constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Annotations_Adapter_Files, __construct){

	zval *options = NULL, annotations_dir = {};

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		if (phalcon_array_isset_fetch_str(&annotations_dir, options, SL("annotationsDir"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_annotationsDir"), &annotations_dir);
		}
	}
}

/**
 * Reads parsed annotations from files
 *
 * @param string $key
 * @return Phalcon\Annotations\Reflection
 */
PHP_METHOD(Phalcon_Annotations_Adapter_Files, read){

	zval *key, annotations_dir = {}, virtual_key = {}, path = {};

	phalcon_fetch_params(0, 1, 0, &key);
	PHALCON_ENSURE_IS_STRING(key);

	phalcon_read_property(&annotations_dir, getThis(), SL("_annotationsDir"), PH_READONLY);

	/**
	 * Paths must be normalized before be used as keys
	 */
	phalcon_prepare_virtual_path_ex(&virtual_key, Z_STRVAL_P(key), Z_STRLEN_P(key), '_');

	PHALCON_CONCAT_VVS(&path, &annotations_dir, &virtual_key, ".php");

	if (phalcon_file_exists(&path) == SUCCESS) {
		RETURN_ON_FAILURE(phalcon_require_ret(return_value, Z_STRVAL(path)));
		return;
	}
}

/**
 * Writes parsed annotations to files
 *
 * @param string $key
 * @param Phalcon\Annotations\Reflection $data
 */
PHP_METHOD(Phalcon_Annotations_Adapter_Files, write){

	zval *key, *data, annotations_dir = {}, virtual_key = {}, path = {}, php_export = {}, status = {};
	smart_str exp = { 0 };

	phalcon_fetch_params(0, 2, 0, &key, &data);
	PHALCON_ENSURE_IS_STRING(key);

	phalcon_read_property(&annotations_dir, getThis(), SL("_annotationsDir"), PH_READONLY);

	/**
	 * Paths must be normalized before be used as keys
	 */
	phalcon_prepare_virtual_path_ex(&virtual_key, Z_STRVAL_P(key), Z_STRLEN_P(key), '_');

	PHALCON_CONCAT_VVS(&path, &annotations_dir, &virtual_key, ".php");

	smart_str_appends(&exp, "<?php return ");
	php_var_export_ex(data, 0, &exp);
	smart_str_appendc(&exp, ';');
	smart_str_0(&exp);

	ZVAL_STR(&php_export, exp.s);

	phalcon_file_put_contents(&status, &path, &php_export);
	if (PHALCON_IS_FALSE(&status)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_annotations_exception_ce, "Annotations directory cannot be written");
		return;
	}
}
