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

#include "loader.h"
#include "loader/exception.h"
#include "di/injectable.h"
#include "debug.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/require.h"
#include "kernel/file.h"
#include "kernel/hash.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Loader
 *
 * This component helps to load your project classes automatically based on some conventions
 *
 *<code>
 * //Creates the autoloader
 * $loader = new Phalcon\Loader();
 *
 * //Register some namespaces
 * $loader->registerNamespaces(array(
 *   'Example\Base' => 'vendor/example/base/',
 *   'Example\Adapter' => 'vendor/example/adapter/',
 *   'Example' => 'vendor/example/'
 * ));
 *
 * //register autoloader
 * $loader->register();
 *
 * //Requiring this class will automatically include file vendor/example/adapter/Some.php
 * $adapter = Example\Adapter\Some();
 *</code>
 */
zend_class_entry *phalcon_loader_ce;

PHP_METHOD(Phalcon_Loader, __construct);
PHP_METHOD(Phalcon_Loader, setExtensions);
PHP_METHOD(Phalcon_Loader, getExtensions);
PHP_METHOD(Phalcon_Loader, registerNamespaces);
PHP_METHOD(Phalcon_Loader, getNamespaces);
PHP_METHOD(Phalcon_Loader, registerPrefixes);
PHP_METHOD(Phalcon_Loader, getPrefixes);
PHP_METHOD(Phalcon_Loader, registerSufixes);
PHP_METHOD(Phalcon_Loader, getSufixes);
PHP_METHOD(Phalcon_Loader, registerDirs);
PHP_METHOD(Phalcon_Loader, getDirs);
PHP_METHOD(Phalcon_Loader, registerClasses);
PHP_METHOD(Phalcon_Loader, getClasses);
PHP_METHOD(Phalcon_Loader, register);
PHP_METHOD(Phalcon_Loader, unregister);
PHP_METHOD(Phalcon_Loader, findFile);
PHP_METHOD(Phalcon_Loader, autoLoad);
PHP_METHOD(Phalcon_Loader, getFoundPath);
PHP_METHOD(Phalcon_Loader, getCheckedPath);
PHP_METHOD(Phalcon_Loader, getDefault);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_loader_setextensions, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, extensions, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_loader_registernamespaces, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, namespaces, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_loader_registerprefixes, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, prefixes, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_loader_registersufixes, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, sufixes, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_loader_registerdirs, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, directories, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_loader_registerclasses, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, classes, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_loader_findfile, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, className, IS_STRING, 0)
	ZEND_ARG_INFO(0, directory)
	ZEND_ARG_TYPE_INFO(0, extensions, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, ds, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_loader_autoload, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, className, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_loader_method_entry[] = {
	PHP_ME(Phalcon_Loader, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Loader, setExtensions, arginfo_phalcon_loader_setextensions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, getExtensions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, registerNamespaces, arginfo_phalcon_loader_registernamespaces, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, getNamespaces, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, registerPrefixes, arginfo_phalcon_loader_registerprefixes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, getPrefixes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, registerSufixes, arginfo_phalcon_loader_registersufixes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, getSufixes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, registerDirs, arginfo_phalcon_loader_registerdirs, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, getDirs, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, registerClasses, arginfo_phalcon_loader_registerclasses, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, getClasses, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, register, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, unregister, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, findFile, arginfo_phalcon_loader_findfile, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, autoLoad, arginfo_phalcon_loader_autoload, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, getFoundPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, getCheckedPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Loader, getDefault, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Loader initializer
 */
PHALCON_INIT_CLASS(Phalcon_Loader){

	PHALCON_REGISTER_CLASS_EX(Phalcon, Loader, loader, phalcon_di_injectable_ce, phalcon_loader_method_entry, 0);

	zend_declare_property_null(phalcon_loader_ce, SL("_default"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_loader_ce, SL("_foundPath"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_loader_ce, SL("_checkedPath"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_loader_ce, SL("_prefixes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_loader_ce, SL("_sufixes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_loader_ce, SL("_classes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_loader_ce, SL("_extensions"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_loader_ce, SL("_namespaces"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_loader_ce, SL("_directories"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_loader_ce, SL("_registered"), 0, ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Loader constructor
 */
PHP_METHOD(Phalcon_Loader, __construct){

	zval extensions = {}, default_loader = {};

	array_init_size(&extensions, 1);
	phalcon_array_append_string(&extensions, IS(php), PH_COPY);

	phalcon_update_property(getThis(), SL("_extensions"), &extensions);
	zval_ptr_dtor(&extensions);

	phalcon_read_static_property_ce(&default_loader, phalcon_loader_ce, SL("_default"), PH_READONLY);
	if (Z_TYPE(default_loader) == IS_NULL) {
		phalcon_update_static_property_ce(phalcon_loader_ce, SL("_default"), getThis());
	}
}

/**
 * Sets an array of extensions that the loader must try in each attempt to locate the file
 *
 * @param array $extensions
 * @param boolean $merge
 * @return Phalcon\Loader
 */
PHP_METHOD(Phalcon_Loader, setExtensions){

	zval *extensions;

	phalcon_fetch_params(0, 1, 0, &extensions);

	phalcon_update_property(getThis(), SL("_extensions"), extensions);

	RETURN_THIS();
}

/**
 * Return file extensions registered in the loader
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Loader, getExtensions){


	RETURN_MEMBER(getThis(), "_extensions");
}

/**
 * Register namespaces and their related directories
 *
 * @param array $namespaces
 * @param boolean $merge
 * @return Phalcon\Loader
 */
PHP_METHOD(Phalcon_Loader, registerNamespaces){

	zval *namespaces, *merge = NULL, current_namespaces = {}, merged_namespaces = {};

	phalcon_fetch_params(0, 1, 1, &namespaces, &merge);

	if (merge && zend_is_true(merge)) {
		phalcon_read_property(&current_namespaces, getThis(), SL("_namespaces"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_namespaces) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_namespaces, &current_namespaces, namespaces);
		} else {
			ZVAL_COPY_VALUE(&merged_namespaces, namespaces);
		}

		phalcon_update_property(getThis(), SL("_namespaces"), &merged_namespaces);
	} else {
		phalcon_update_property(getThis(), SL("_namespaces"), namespaces);
	}

	RETURN_THIS();
}

/**
 * Return current namespaces registered in the autoloader
 *
 * @return array
 */
PHP_METHOD(Phalcon_Loader, getNamespaces){


	RETURN_MEMBER(getThis(), "_namespaces");
}

/**
 * Register directories on which "not found" classes could be found
 *
 * @param array $prefixes
 * @param boolean $merge
 * @return Phalcon\Loader
 */
PHP_METHOD(Phalcon_Loader, registerPrefixes){

	zval *prefixes, *merge = NULL, current_prefixes = {}, merged_prefixes = {};

	phalcon_fetch_params(0, 1, 1, &prefixes, &merge);

	if (merge && zend_is_true(merge)) {
		phalcon_read_property(&current_prefixes, getThis(), SL("_prefixes"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_prefixes) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_prefixes, &current_prefixes, prefixes);
		} else {
			ZVAL_COPY(&merged_prefixes, prefixes);
		}

		phalcon_update_property(getThis(), SL("_prefixes"), &merged_prefixes);
		zval_ptr_dtor(&merged_prefixes);
	} else {
		phalcon_update_property(getThis(), SL("_prefixes"), prefixes);
	}

	RETURN_THIS();
}

/**
 * Return current prefixes registered in the autoloader
 *
 * @param array
 */
PHP_METHOD(Phalcon_Loader, getPrefixes){


	RETURN_MEMBER(getThis(), "_prefixes");
}

/**
 * Register directories on which "not found" classes could be found
 *
 * @param array $sufixes
 * @param boolean $merge
 * @return Phalcon\Loader
 */
PHP_METHOD(Phalcon_Loader, registerSufixes){

	zval *sufixes, *merge = NULL, current_sufixes = {}, merged_sufixes = {};

	phalcon_fetch_params(0, 1, 1, &sufixes, &merge);

	if (merge && zend_is_true(merge)) {
		phalcon_read_property(&current_sufixes, getThis(), SL("_sufixes"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_sufixes) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_sufixes, &current_sufixes, sufixes);
		} else {
			ZVAL_COPY(&merged_sufixes, sufixes);
		}

		phalcon_update_property(getThis(), SL("_sufixes"), &merged_sufixes);
		zval_ptr_dtor(&merged_sufixes);
	} else {
		phalcon_update_property(getThis(), SL("_sufixes"), sufixes);
	}

	RETURN_THIS();
}

/**
 * Return current prefixes registered in the autoloader
 *
 * @param array
 */
PHP_METHOD(Phalcon_Loader, getSufixes){


	RETURN_MEMBER(getThis(), "_sufixes");
}

/**
 * Register directories on which "not found" classes could be found
 *
 * @param array $directories
 * @param boolean $merge
 * @return Phalcon\Loader
 */
PHP_METHOD(Phalcon_Loader, registerDirs){

	zval *directories, *merge = NULL, current_directories = {}, merged_directories = {};

	phalcon_fetch_params(0, 1, 1, &directories, &merge);

	if (merge && zend_is_true(merge)) {
		phalcon_read_property(&current_directories, getThis(), SL("_directories"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_directories) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_directories, &current_directories, directories);
		} else {
			ZVAL_COPY_VALUE(&merged_directories, directories);
		}

		phalcon_update_property(getThis(), SL("_directories"), &merged_directories);
	} else {
		phalcon_update_property(getThis(), SL("_directories"), directories);
	}

	RETURN_THIS();
}

/**
 * Return current directories registered in the autoloader
 *
 * @param array
 */
PHP_METHOD(Phalcon_Loader, getDirs){


	RETURN_MEMBER(getThis(), "_directories");
}

/**
 * Register classes and their locations
 *
 * @param array $classes
 * @param boolean $merge
 * @return Phalcon\Loader
 */
PHP_METHOD(Phalcon_Loader, registerClasses){

	zval *classes, *merge = NULL, current_classes = {}, merged_classes = {};

	phalcon_fetch_params(0, 1, 1, &classes, &merge);

	if (merge && zend_is_true(merge)) {
		phalcon_read_property(&current_classes, getThis(), SL("_classes"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_classes) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_classes, &current_classes, classes);
		} else {
			ZVAL_COPY_VALUE(&merged_classes, classes);
		}

		phalcon_update_property(getThis(), SL("_classes"), &merged_classes);
	} else {
		phalcon_update_property(getThis(), SL("_classes"), classes);
	}

	RETURN_THIS();
}

/**
 * Return the current class-map registered in the autoloader
 *
 * @param array
 */
PHP_METHOD(Phalcon_Loader, getClasses){


	RETURN_MEMBER(getThis(), "_classes");
}

/**
 * Register the autoload method
 *
 * @return Phalcon\Loader
 */
PHP_METHOD(Phalcon_Loader, register){

	zval registered = {}, autoloader = {};

	phalcon_read_property(&registered, getThis(), SL("_registered"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_FALSE(&registered)) {
		array_init_size(&autoloader, 2);
		phalcon_array_append(&autoloader, getThis(), PH_COPY);
		phalcon_array_append_string(&autoloader, IS(autoLoad), PH_COPY);
		PHALCON_CALL_FUNCTION(NULL, "spl_autoload_register", &autoloader);
		zval_ptr_dtor(&autoloader);

		phalcon_update_property(getThis(), SL("_registered"), &PHALCON_GLOBAL(z_true));
	}

	RETURN_THIS();
}

/**
 * Unregister the autoload method
 *
 * @return Phalcon\Loader
 */
PHP_METHOD(Phalcon_Loader, unregister){

	zval registered = {}, autoloader = {};

	PHALCON_MM_INIT();
	phalcon_read_property(&registered, getThis(), SL("_registered"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_TRUE(&registered)) {
		array_init_size(&autoloader, 2);
		phalcon_array_append(&autoloader, getThis(), PH_COPY);
		add_next_index_stringl(&autoloader, SL("autoLoad"));
		PHALCON_MM_ADD_ENTRY(&autoloader);
		PHALCON_MM_CALL_FUNCTION(NULL, "spl_autoload_unregister", &autoloader);
		phalcon_update_property(getThis(), SL("_registered"), &PHALCON_GLOBAL(z_false));
	}

	RETURN_MM_THIS();
}

/**
 * Makes the work of autoload registered classes
 *
 * @param string $className
 * @param array|string $directory
 * @param array $extensions
 * @param string $ds
 * @return boolean
 */
PHP_METHOD(Phalcon_Loader, findFile){

	zval *class_name, *directory, *extensions, *ds = NULL, ds_slash = {}, events_manager = {}, event_name = {}, directories = {}, *dir, *extension, debug_message = {};
	char slash[2] = {DEFAULT_SLASH, 0};

	phalcon_fetch_params(1, 3, 1, &class_name, &directory, &extensions, &ds);

	phalcon_read_property(&events_manager, getThis(), SL("_eventsManager"), PH_NOISY|PH_READONLY);

	if (Z_TYPE_P(directory) != IS_ARRAY) {
		array_init(&directories);
		phalcon_array_append(&directories, directory, PH_COPY);
		PHALCON_MM_ADD_ENTRY(&directories);
	} else {
		ZVAL_COPY_VALUE(&directories, directory);
	}

	if (!ds) {
		PHALCON_MM_ZVAL_STRING(&ds_slash, slash);
	} else {
		ZVAL_COPY_VALUE(&ds_slash, ds);
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "Find class: ", class_name);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	RETVAL_FALSE;


	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(directories), dir) {
		zval fixed_dir = {};
		if (Z_TYPE_P(dir) != IS_STRING) {
			convert_to_string_ex(dir);
		}
		/**
		 * Add a trailing directory separator if the user forgot to do that
		 */
		phalcon_fix_path(&fixed_dir, dir, &ds_slash);
		PHALCON_MM_ADD_ENTRY(&fixed_dir);
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(extensions), extension) {
			zval file_path = {};
			PHALCON_CONCAT_VVSV(&file_path, &fixed_dir, class_name, ".", extension);
			PHALCON_MM_ADD_ENTRY(&file_path);
			/**
			 * Check if a events manager is available
			 */
			if (Z_TYPE(events_manager) == IS_OBJECT) {
				phalcon_update_property(getThis(), SL("_checkedPath"), &file_path);

				PHALCON_MM_ZVAL_STRING(&event_name, "loader:beforeCheckPath");
				PHALCON_MM_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis());
			}

			/**
			 * This is probably a good path, let's check if the file exist
			 */
			if (phalcon_file_exists(&file_path) == SUCCESS) {
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					PHALCON_CONCAT_SV(&debug_message, "--Found: ", &file_path);
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}

				if (Z_TYPE(events_manager) == IS_OBJECT) {
					phalcon_update_property(getThis(), SL("_foundPath"), &file_path);

					PHALCON_MM_ZVAL_STRING(&event_name, "loader:pathFound");
					PHALCON_MM_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), &file_path);
				}

				/**
				 * Simulate a require
				 */
				assert(Z_TYPE(file_path) == IS_STRING);
				RETURN_MM_ON_FAILURE(phalcon_require(Z_STRVAL(file_path)));

				/**
				 * Return true mean success
				 */
				RETVAL_TRUE;
				break;
			} else if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				PHALCON_CONCAT_SV(&debug_message, "--Not Found: ", &file_path);
				PHALCON_DEBUG_LOG(&debug_message);
				zval_ptr_dtor(&debug_message);
			}
		} ZEND_HASH_FOREACH_END();
		if (zend_is_true(return_value)) {
			break;
		}
	} ZEND_HASH_FOREACH_END();
	RETURN_MM();
}

/**
 * Makes the work of autoload registered classes
 *
 * @param string $className
 * @return boolean
 */
PHP_METHOD(Phalcon_Loader, autoLoad){

	zval *class_name, events_manager = {}, event_name = {}, classes = {}, file_path = {}, found = {}, ds = {}, namespace_separator = {};
	zval extensions = {}, *directory, pseudo_separator = {}, directories = {};
	zend_string *str_key;
	ulong idx;
	char slash[2] = {DEFAULT_SLASH, 0};

	phalcon_fetch_params(1, 1, 0, &class_name);

	ZVAL_FALSE(&found);

	phalcon_read_property(&events_manager, getThis(), SL("_eventsManager"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(events_manager) == IS_OBJECT) {
		PHALCON_MM_ZVAL_STRING(&event_name, "loader:beforeCheckClass");
		PHALCON_MM_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), class_name);
	}

	/**
	 * First we check for static paths for classes
	 */
	phalcon_read_property(&classes, getThis(), SL("_classes"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(classes) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&file_path, &classes, class_name, PH_READONLY)) {
			convert_to_string_ex(&file_path);
			if (Z_TYPE(events_manager) == IS_OBJECT) {
				phalcon_update_property(getThis(), SL("_foundPath"), &file_path);

				PHALCON_MM_ZVAL_STRING(&event_name, "loader:pathFound");
				PHALCON_MM_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), &file_path);
			}

			RETURN_MM_ON_FAILURE(phalcon_require(Z_STRVAL(file_path)));

			ZVAL_TRUE(&found);
		}
	}

	PHALCON_MM_ZVAL_STRING(&ds, slash);
	PHALCON_MM_ZVAL_STRING(&namespace_separator, "\\");
	PHALCON_MM_ZVAL_STRING(&pseudo_separator, "_");
	phalcon_read_property(&extensions, getThis(), SL("_extensions"), PH_NOISY|PH_READONLY);

	if (!zend_is_true(&found)) {
		zval namespaces = {};
		/**
		 * Checking in namespaces
		 */
		phalcon_read_property(&namespaces, getThis(), SL("_namespaces"), PH_READONLY);
		if (Z_TYPE(namespaces) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(namespaces), idx, str_key, directory) {
				zval ns_prefix = {}, ns_prefixed = {}, file_name = {};
				if (str_key) {
					ZVAL_STR(&ns_prefix, str_key);
				} else {
					ZVAL_LONG(&ns_prefix, idx);
				}
				/**
				 * The class name must start with the current namespace
				 */
				PHALCON_CONCAT_VV(&ns_prefixed, &ns_prefix, &namespace_separator);
				PHALCON_MM_ADD_ENTRY(&ns_prefixed);
				if (!phalcon_start_with(class_name, &ns_prefixed, NULL)) {
					continue;
				}

				/**
				 * Get the possible file path
				 */
				phalcon_possible_autoload_filepath(&file_name, &ns_prefix, class_name, &ds, NULL);
				PHALCON_MM_ADD_ENTRY(&file_name);
				if (zend_is_true(&file_name)) {
					PHALCON_MM_CALL_METHOD(&found, getThis(), "findfile", &file_name, directory, &extensions, &ds);

					if (zend_is_true(&found)) {
						break;
					}
				}

				if (phalcon_memnstr_str(class_name, SL("_"))) {
					phalcon_possible_autoload_filepath(&file_name, &ns_prefix, class_name, &ds, &pseudo_separator);
					PHALCON_MM_ADD_ENTRY(&file_name);
					if (zend_is_true(&file_name)) {
						PHALCON_MM_CALL_METHOD(&found, getThis(), "findfile", &file_name, directory, &extensions, &ds);

						if (zend_is_true(&found)) {
							break;
						}
					}
				}
			} ZEND_HASH_FOREACH_END();
		}
	}

	if (!zend_is_true(&found)) {
		zval prefixes = {};
		/**
		 * Checking in prefixes
		 */
		phalcon_read_property(&prefixes, getThis(), SL("_prefixes"), PH_READONLY);
		if (Z_TYPE(prefixes) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(prefixes), idx, str_key, directory) {
				zval prefix = {}, file_name = {};
				if (str_key) {
					ZVAL_STR(&prefix, str_key);
				} else {
					ZVAL_LONG(&prefix, idx);
				}

				/**
				 * The class name starts with the prefix?
				 */
				if (phalcon_start_with(class_name, &prefix, NULL)) {
					/**
					 * Get the possible file path
					 */
					phalcon_possible_autoload_filepath(&file_name, &prefix, class_name, &ds, NULL);
					PHALCON_MM_ADD_ENTRY(&file_name);
					if (zend_is_true(&file_name)) {
						PHALCON_MM_CALL_METHOD(&found, getThis(), "findfile", &file_name, directory, &extensions, &ds);

						if (zend_is_true(&found)) {
							break;
						}
					}

					if (phalcon_memnstr_str(class_name, SL("_"))) {
						phalcon_possible_autoload_filepath(&file_name, &prefix, class_name, &ds, &pseudo_separator);
						PHALCON_MM_ADD_ENTRY(&file_name);
						if (zend_is_true(&file_name)) {
							PHALCON_MM_CALL_METHOD(&found, getThis(), "findfile", &file_name, directory, &extensions, &ds);

							if (zend_is_true(&found)) {
								break;
							}
						}
					}
				}
			} ZEND_HASH_FOREACH_END();
		}
	}

	if (!zend_is_true(&found)) {
		zval sufixes = {};
		/**
		 * Checking in sufixes
		 */
		phalcon_read_property(&sufixes, getThis(), SL("_sufixes"), PH_READONLY);
		if (Z_TYPE(sufixes) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(sufixes), idx, str_key, directory) {
				zval sufix = {}, file_name = {};
				if (str_key) {
					ZVAL_STR(&sufix, str_key);
				} else {
					ZVAL_LONG(&sufix, idx);
				}

				/**
				 * The class name ends with the sufix?
				 */
				if (phalcon_end_with(class_name, &sufix, NULL)) {
					/**
					 * Get the possible file path
					 */
					phalcon_possible_autoload_filepath2(&file_name, class_name, &ds, NULL);
					PHALCON_MM_ADD_ENTRY(&file_name);
					if (zend_is_true(&file_name)) {
						PHALCON_MM_CALL_METHOD(&found, getThis(), "findfile", &file_name, directory, &extensions, &ds);

						if (zend_is_true(&found)) {
							break;
						}
					}

					if (phalcon_memnstr_str(class_name, SL("_"))) {
						phalcon_possible_autoload_filepath2(&file_name, class_name, &ds, &pseudo_separator);
						PHALCON_MM_ADD_ENTRY(&file_name);
						if (zend_is_true(&file_name)) {
							PHALCON_MM_CALL_METHOD(&found, getThis(), "findfile", &file_name, directory, &extensions, &ds);

							if (zend_is_true(&found)) {
								break;
							}
						}
					}
				}
			} ZEND_HASH_FOREACH_END();
		}
	}

	if (!zend_is_true(&found)) {
		zval ns_class_name = {};

		/**
		 * And change the namespace separator by directory separator too
		 */
		PHALCON_STR_REPLACE(&ns_class_name, &namespace_separator, &ds, class_name);
		PHALCON_MM_ADD_ENTRY(&ns_class_name);
		/**
		 * Checking in directories
		 */
		phalcon_read_property(&directories, getThis(), SL("_directories"), PH_READONLY);

		PHALCON_MM_CALL_METHOD(&found, getThis(), "findfile", &ns_class_name, &directories, &extensions, &ds);

		if (phalcon_memnstr_str(class_name, SL("_"))) {
			zval ds_class_name = {};
			/**
			 * Change the pseudo-separator by the directory separator in the class name
			 */
			PHALCON_STR_REPLACE(&ds_class_name, &pseudo_separator, &ds, &ns_class_name);
			PHALCON_MM_ADD_ENTRY(&ds_class_name);
			PHALCON_MM_CALL_METHOD(&found, getThis(), "findfile", &ds_class_name, &directories, &extensions, &ds);
		}
	}

	/**
	 * Call 'afterCheckClass' event
	 */
	if (Z_TYPE(events_manager) == IS_OBJECT) {
		PHALCON_MM_ZVAL_STRING(&event_name, "loader:afterCheckClass");
		PHALCON_MM_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), class_name);
	}

	if (zend_is_true(&found)) {
		RETURN_MM_TRUE;
	}

	/**
	 * Cannot find the class return false
	 */
	RETURN_MM_FALSE;
}

/**
 * Get the path when a class was found
 *
 * @return string
 */
PHP_METHOD(Phalcon_Loader, getFoundPath){


	RETURN_MEMBER(getThis(), "_foundPath");
}

/**
 * Get the path the loader is checking for a path
 *
 * @return string
 */
PHP_METHOD(Phalcon_Loader, getCheckedPath){


	RETURN_MEMBER(getThis(), "_checkedPath");
}

/**
 * Return the default loader
 *
 * @return Phalcon\Loader
 */
PHP_METHOD(Phalcon_Loader, getDefault){

	phalcon_read_static_property_ce(return_value, phalcon_loader_ce, SL("_default"), PH_COPY);
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		object_init_ex(return_value, phalcon_loader_ce);
		PHALCON_CALL_METHOD(NULL, return_value, "__construct");
	}
}
