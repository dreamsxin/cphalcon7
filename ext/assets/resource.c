
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

#include "assets/resource.h"
#include "assets/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/exception.h"
#include "kernel/file.h"
#include "kernel/fcall.h"

/**
 * Phalcon\Assets\Resource
 *
 * Represents an asset resource
 *
 *<code>
 * $resource = new Phalcon\Assets\Resource('js', 'javascripts/jquery.js');
 *</code>
 *
 */
zend_class_entry *phalcon_assets_resource_ce;

PHP_METHOD(Phalcon_Assets_Resource, __construct);
PHP_METHOD(Phalcon_Assets_Resource, setType);
PHP_METHOD(Phalcon_Assets_Resource, getType);
PHP_METHOD(Phalcon_Assets_Resource, setPath);
PHP_METHOD(Phalcon_Assets_Resource, getPath);
PHP_METHOD(Phalcon_Assets_Resource, setLocal);
PHP_METHOD(Phalcon_Assets_Resource, getLocal);
PHP_METHOD(Phalcon_Assets_Resource, setFilter);
PHP_METHOD(Phalcon_Assets_Resource, getFilter);
PHP_METHOD(Phalcon_Assets_Resource, setAttributes);
PHP_METHOD(Phalcon_Assets_Resource, getAttributes);
PHP_METHOD(Phalcon_Assets_Resource, setTargetUri);
PHP_METHOD(Phalcon_Assets_Resource, getTargetUri);
PHP_METHOD(Phalcon_Assets_Resource, setSourcePath);
PHP_METHOD(Phalcon_Assets_Resource, getSourcePath);
PHP_METHOD(Phalcon_Assets_Resource, setTargetPath);
PHP_METHOD(Phalcon_Assets_Resource, getTargetPath);
PHP_METHOD(Phalcon_Assets_Resource, getContent);
PHP_METHOD(Phalcon_Assets_Resource, getRealTargetUri);
PHP_METHOD(Phalcon_Assets_Resource, getRealSourcePath);
PHP_METHOD(Phalcon_Assets_Resource, getRealTargetPath);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, local)
	ZEND_ARG_INFO(0, filter)
	ZEND_ARG_INFO(0, attributes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_settype, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_setpath, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_setlocal, 0, 0, 1)
	ZEND_ARG_INFO(0, local)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_setfilter, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filter, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_setattributes, 0, 0, 1)
	ZEND_ARG_INFO(0, attributes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_settargeturi, 0, 0, 1)
	ZEND_ARG_INFO(0, targetUri)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_setsourcepath, 0, 0, 1)
	ZEND_ARG_INFO(0, sourcePath)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_settargetpath, 0, 0, 1)
	ZEND_ARG_INFO(0, targetPath)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_getcontent, 0, 0, 0)
	ZEND_ARG_INFO(0, basePath)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_getrealsourcepath, 0, 0, 0)
	ZEND_ARG_INFO(0, basePath)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_resource_getrealtargetpath, 0, 0, 0)
	ZEND_ARG_INFO(0, basePath)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_assets_resource_method_entry[] = {
	PHP_ME(Phalcon_Assets_Resource, __construct, arginfo_phalcon_assets_resource___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Assets_Resource, setType, arginfo_phalcon_assets_resource_settype, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, setPath, arginfo_phalcon_assets_resource_setpath, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, setLocal, arginfo_phalcon_assets_resource_setlocal, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getLocal, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, setFilter, arginfo_phalcon_assets_resource_setfilter, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getFilter, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, setAttributes, arginfo_phalcon_assets_resource_setattributes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getAttributes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, setTargetUri, arginfo_phalcon_assets_resource_settargeturi, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getTargetUri, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, setSourcePath, arginfo_phalcon_assets_resource_setsourcepath, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getSourcePath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, setTargetPath, arginfo_phalcon_assets_resource_settargetpath, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getTargetPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getContent, arginfo_phalcon_assets_resource_getcontent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getRealTargetUri, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getRealSourcePath, arginfo_phalcon_assets_resource_getrealsourcepath, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Resource, getRealTargetPath, arginfo_phalcon_assets_resource_getrealtargetpath, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Assets\Resource initializer
 */
PHALCON_INIT_CLASS(Phalcon_Assets_Resource){

	PHALCON_REGISTER_CLASS(Phalcon\\Assets, Resource, assets_resource, phalcon_assets_resource_method_entry, 0);

	zend_declare_property_null(phalcon_assets_resource_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_assets_resource_ce, SL("_path"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_assets_resource_ce, SL("_local"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_assets_resource_ce, SL("_filter"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_assets_resource_ce, SL("_attributes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_assets_resource_ce, SL("_sourcePath"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_assets_resource_ce, SL("_targetPath"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_assets_resource_ce, SL("_targetUri"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Assets\Resource constructor
 *
 * @param string $type
 * @param string $path
 * @param boolean $local
 * @param boolean $filter
 * @param array $attributes
 */
PHP_METHOD(Phalcon_Assets_Resource, __construct){

	zval *type, *path, *local = NULL, *filter = NULL, *attributes = NULL;

	phalcon_fetch_params(0, 2, 3, &type, &path, &local, &filter, &attributes);

	if (!local) {
		local = &PHALCON_GLOBAL(z_true);
	}

	if (!filter) {
		filter = &PHALCON_GLOBAL(z_true);
	}

	if (!attributes) {
		attributes = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_property(getThis(), SL("_type"), type);
	phalcon_update_property(getThis(), SL("_path"), path);
	phalcon_update_property(getThis(), SL("_local"), local);
	phalcon_update_property(getThis(), SL("_filter"), filter);
	if (Z_TYPE_P(attributes) == IS_ARRAY) {
		phalcon_update_property(getThis(), SL("_attributes"), attributes);
	}
}

/**
 * Sets the resource's type
 *
 * @param string $type
 * @return Phalcon\Assets\Resource
 */
PHP_METHOD(Phalcon_Assets_Resource, setType){

	zval *type;

	phalcon_fetch_params(0, 1, 0, &type);

	phalcon_update_property(getThis(), SL("_type"), type);
	RETURN_THIS();
}

/**
 * Returns the type of resource
 *
 * @return string
 */
PHP_METHOD(Phalcon_Assets_Resource, getType){


	RETURN_MEMBER(getThis(), "_type");
}

/**
 * Sets the resource's path
 *
 * @param string $path
 * @return Phalcon\Assets\Resource
 */
PHP_METHOD(Phalcon_Assets_Resource, setPath){

	zval *path;

	phalcon_fetch_params(0, 1, 0, &path);

	phalcon_update_property(getThis(), SL("_path"), path);
	RETURN_THIS();
}

/**
 * Returns the URI/URL path to the resource
 *
 * @return string
 */
PHP_METHOD(Phalcon_Assets_Resource, getPath){


	RETURN_MEMBER(getThis(), "_path");
}

/**
 * Sets if the resource is local or external
 *
 * @param boolean $local
 * @return Phalcon\Assets\Resource
 */
PHP_METHOD(Phalcon_Assets_Resource, setLocal){

	zval *local;

	phalcon_fetch_params(0, 1, 0, &local);

	phalcon_update_property(getThis(), SL("_local"), local);
	RETURN_THIS();
}

/**
 * Returns whether the resource is local or external
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Assets_Resource, getLocal){


	RETURN_MEMBER(getThis(), "_local");
}

/**
 * Sets if the resource must be filtered or not
 *
 * @param boolean $filter
 * @return Phalcon\Assets\Resource
 */
PHP_METHOD(Phalcon_Assets_Resource, setFilter){

	zval *filter;

	phalcon_fetch_params(0, 1, 0, &filter);

	phalcon_update_property(getThis(), SL("_filter"), filter);
	RETURN_THIS();
}

/**
 * Returns whether the resource must be filtered or not
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Assets_Resource, getFilter){


	RETURN_MEMBER(getThis(), "_filter");
}

/**
 * Sets extra HTML attributes
 *
 * @param array $attributes
 * @return Phalcon\Assets\Resource
 */
PHP_METHOD(Phalcon_Assets_Resource, setAttributes){

	zval *attributes;

	phalcon_fetch_params(0, 1, 0, &attributes);

	phalcon_update_property(getThis(), SL("_attributes"), attributes);
	RETURN_THIS();
}

/**
 * Returns extra HTML attributes set in the resource
 *
 * @return array
 */
PHP_METHOD(Phalcon_Assets_Resource, getAttributes){


	RETURN_MEMBER(getThis(), "_attributes");
}

/**
 * Sets a target uri for the generated HTML
 *
 * @param string $targetUri
 * @return Phalcon\Assets\Resource
 */
PHP_METHOD(Phalcon_Assets_Resource, setTargetUri){

	zval *target_uri;

	phalcon_fetch_params(0, 1, 0, &target_uri);

	phalcon_update_property(getThis(), SL("_targetUri"), target_uri);
	RETURN_THIS();
}

/**
 * Returns the target uri for the generated HTML
 *
 * @return string
 */
PHP_METHOD(Phalcon_Assets_Resource, getTargetUri){


	RETURN_MEMBER(getThis(), "_targetUri");
}

/**
 * Sets the resource's source path
 *
 * @param string $sourcePath
 * @return Phalcon\Assets\Resource
 */
PHP_METHOD(Phalcon_Assets_Resource, setSourcePath){

	zval *source_path;

	phalcon_fetch_params(0, 1, 0, &source_path);

	phalcon_update_property(getThis(), SL("_sourcePath"), source_path);
	RETURN_THIS();
}

/**
 * Returns the resource's target path
 *
 * @return string
 */
PHP_METHOD(Phalcon_Assets_Resource, getSourcePath){


	RETURN_MEMBER(getThis(), "_sourcePath");
}

/**
 * Sets the resource's target path
 *
 * @param string $targetPath
 * @return Phalcon\Assets\Resource
 */
PHP_METHOD(Phalcon_Assets_Resource, setTargetPath){

	zval *target_path;

	phalcon_fetch_params(0, 1, 0, &target_path);

	phalcon_update_property(getThis(), SL("_targetPath"), target_path);
	RETURN_THIS();
}

/**
 * Returns the resource's target path
 *
 * @return string
 */
PHP_METHOD(Phalcon_Assets_Resource, getTargetPath){


	RETURN_MEMBER(getThis(), "_targetPath");
}

/**
 * Returns the content of the resource as an string
 * Optionally a base path where the resource is located can be set
 *
 * @param string $basePath
 * @return string
 */
PHP_METHOD(Phalcon_Assets_Resource, getContent){

	zval *base_path = NULL, source_path = {}, complete_path = {}, local = {}, exception_message = {};

	phalcon_fetch_params(0, 0, 1, &base_path);

	if (!base_path) {
		base_path = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&source_path, getThis(), SL("_sourcePath"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_EMPTY(&source_path)) {
		phalcon_read_property(&source_path, getThis(), SL("_path"), PH_NOISY|PH_READONLY);
	}

	/**
	 * A base path for resources can be set in the assets manager
	 */
	PHALCON_CONCAT_VV(&complete_path, base_path, &source_path);

	phalcon_read_property(&local, getThis(), SL("_local"), PH_NOISY|PH_READONLY);

	/**
	 * Local resources are loaded from the local disk
	 */
	if (zend_is_true(&local)) {

		/**
		 * Check first if the file is readable
		 */
		if (phalcon_file_exists(&complete_path) == FAILURE) {
			PHALCON_CONCAT_SVS(&exception_message, "Resource's content for \"", &complete_path, "\" cannot be loaded");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_assets_exception_ce, &exception_message);
			zval_ptr_dtor(&complete_path);
			return;
		}
	}

	/**
	 * Use file_get_contents to respect the openbase_dir. Access urls must be enabled
	 */
	phalcon_file_get_contents(return_value, &complete_path);
	if (PHALCON_IS_FALSE(return_value)) {
		PHALCON_CONCAT_SVS(&exception_message, "Resource's content for \"", &complete_path, "\" cannot be read");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_assets_exception_ce, &exception_message);
		zval_ptr_dtor(&complete_path);
		zval_ptr_dtor(&exception_message);
		return;
	}
	zval_ptr_dtor(&complete_path);
}

/**
 * Returns the real target uri for the generated HTML
 *
 * @return string
 */
PHP_METHOD(Phalcon_Assets_Resource, getRealTargetUri){

	zval *base_uri = NULL, target_uri = {};

	phalcon_fetch_params(0, 0, 1, &base_uri);

	if (!base_uri) {
		base_uri = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&target_uri, getThis(), SL("_targetUri"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_EMPTY(&target_uri)) {
		phalcon_read_property(&target_uri, getThis(), SL("_path"), PH_NOISY|PH_READONLY);
	}

	PHALCON_CONCAT_VV(return_value, base_uri, &target_uri);
}

/**
 * Returns the complete location where the resource is located
 *
 * @param string $basePath
 * @return string
 */
PHP_METHOD(Phalcon_Assets_Resource, getRealSourcePath){

	zval *base_path = NULL, source_path = {}, local = {};

	phalcon_fetch_params(0, 0, 1, &base_path);

	if (!base_path) {
		base_path = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&source_path, getThis(), SL("_sourcePath"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_EMPTY(&source_path)) {
		phalcon_read_property(&source_path, getThis(), SL("_path"), PH_NOISY|PH_READONLY);
	}

	phalcon_read_property(&local, getThis(), SL("_local"), PH_NOISY|PH_READONLY);
	if (zend_is_true(&local)) {
		zval complete_path = {};
		/**
		 * A base path for resources can be set in the assets manager
		 */
		PHALCON_CONCAT_VV(&complete_path, base_path, &source_path);

		/**
		 * Get the real template path
		 */
		phalcon_file_realpath(return_value, &complete_path);
		zval_ptr_dtor(&complete_path);
		return;
	}

	RETURN_CTOR(&source_path);
}

/**
 * Returns the complete location where the resource must be written
 *
 * @param string $basePath
 * @return string
 */
PHP_METHOD(Phalcon_Assets_Resource, getRealTargetPath){

	zval *base_path = NULL, target_path = {}, local = {};

	phalcon_fetch_params(0, 0, 1, &base_path);

	if (!base_path) {
		base_path = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&target_path, getThis(), SL("_targetPath"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_EMPTY(&target_path)) {
		phalcon_read_property(&target_path, getThis(), SL("_path"), PH_NOISY|PH_READONLY);
	}

	phalcon_read_property(&local, getThis(), SL("_local"), PH_NOISY|PH_READONLY);
	if (zend_is_true(&local)) {
		zval complete_path = {};
		/**
		 * A base path for resources can be set in the assets manager
		 */
		PHALCON_CONCAT_VV(&complete_path, base_path, &target_path);

		/**
		 * Get the real template path, the target path can optionally don't exist
		 */
		if (phalcon_file_exists(&complete_path) == SUCCESS) {
			phalcon_file_realpath(return_value, &complete_path);
			zval_ptr_dtor(&complete_path);
			return;
		}

		RETURN_ZVAL(&complete_path, 0, 0);
	}

	RETURN_CTOR(&target_path);
}
