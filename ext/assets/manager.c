
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

#include "assets/manager.h"
#include "assets/collection.h"
#include "assets/exception.h"
#include "assets/resource.h"
#include "assets/resource/css.h"
#include "assets/resource/js.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/file.h"

/**
 * Phalcon\Assets\Manager
 *
 * Manages collections of CSS/Javascript assets
 */
zend_class_entry *phalcon_assets_manager_ce;

PHP_METHOD(Phalcon_Assets_Manager, __construct);
PHP_METHOD(Phalcon_Assets_Manager, setOptions);
PHP_METHOD(Phalcon_Assets_Manager, getOptions);
PHP_METHOD(Phalcon_Assets_Manager, useImplicitOutput);
PHP_METHOD(Phalcon_Assets_Manager, addCss);
PHP_METHOD(Phalcon_Assets_Manager, addJs);
PHP_METHOD(Phalcon_Assets_Manager, addResourceByType);
PHP_METHOD(Phalcon_Assets_Manager, addResource);
PHP_METHOD(Phalcon_Assets_Manager, set);
PHP_METHOD(Phalcon_Assets_Manager, get);
PHP_METHOD(Phalcon_Assets_Manager, getCss);
PHP_METHOD(Phalcon_Assets_Manager, getJs);
PHP_METHOD(Phalcon_Assets_Manager, collection);
PHP_METHOD(Phalcon_Assets_Manager, output);
PHP_METHOD(Phalcon_Assets_Manager, outputCss);
PHP_METHOD(Phalcon_Assets_Manager, outputJs);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_setoptions, 0, 0, 1)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_useimplicitoutput, 0, 0, 1)
	ZEND_ARG_INFO(0, implicitOutput)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_addcss, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, local)
	ZEND_ARG_INFO(0, filter)
	ZEND_ARG_INFO(0, attributes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_addjs, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, local)
	ZEND_ARG_INFO(0, filter)
	ZEND_ARG_INFO(0, attributes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_addresourcebytype, 0, 0, 2)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_addresource, 0, 0, 1)
	ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_set, 0, 0, 2)
	ZEND_ARG_INFO(0, id)
	ZEND_ARG_INFO(0, collection)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_get, 0, 0, 1)
	ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_collection, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_output, 0, 0, 2)
	ZEND_ARG_INFO(0, collection)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_outputcss, 0, 0, 0)
	ZEND_ARG_INFO(0, collectionName)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_assets_manager_outputjs, 0, 0, 0)
	ZEND_ARG_INFO(0, collectionName)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_assets_manager_method_entry[] = {
	PHP_ME(Phalcon_Assets_Manager, __construct, arginfo_phalcon_assets_manager___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Assets_Manager, setOptions, arginfo_phalcon_assets_manager_setoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, getOptions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, useImplicitOutput, arginfo_phalcon_assets_manager_useimplicitoutput, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, addCss, arginfo_phalcon_assets_manager_addcss, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, addJs, arginfo_phalcon_assets_manager_addjs, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, addResourceByType, arginfo_phalcon_assets_manager_addresourcebytype, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, addResource, arginfo_phalcon_assets_manager_addresource, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, set, arginfo_phalcon_assets_manager_set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, get, arginfo_phalcon_assets_manager_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, getCss, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, getJs, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, collection, arginfo_phalcon_assets_manager_collection, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, output, arginfo_phalcon_assets_manager_output, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, outputCss, arginfo_phalcon_assets_manager_outputcss, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Assets_Manager, outputJs, arginfo_phalcon_assets_manager_outputjs, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Assets\Manager initializer
 */
PHALCON_INIT_CLASS(Phalcon_Assets_Manager){

	PHALCON_REGISTER_CLASS(Phalcon\\Assets, Manager, assets_manager, phalcon_assets_manager_method_entry, 0);

	zend_declare_property_null(phalcon_assets_manager_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_assets_manager_ce, SL("_collections"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_assets_manager_ce, SL("_implicitOutput"), 1, ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Assets\Manager constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Assets_Manager, __construct){

	zval *options = NULL;

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		phalcon_update_property_this(getThis(), SL("_options"), options);
	}
}

/**
 * Sets the manager's options
 *
 * @param array $options
 * @return Phalcon\Assets\Manager
 */
PHP_METHOD(Phalcon_Assets_Manager, setOptions){

	zval *options;

	phalcon_fetch_params(0, 1, 0, &options);

	if (Z_TYPE_P(options) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_assets_exception_ce, "Options must be an array");
		return;
	}
	phalcon_update_property_this(getThis(), SL("_options"), options);

	RETURN_THISW();
}

/**
 * Returns the manager's options
 *
 * @return array
 */
PHP_METHOD(Phalcon_Assets_Manager, getOptions){


	RETURN_MEMBER(getThis(), "_options");
}

/**
 * Sets if the HTML generated must be directly printed or returned
 *
 * @param boolean $implicitOutput
 * @return Phalcon\Assets\Manager
 */
PHP_METHOD(Phalcon_Assets_Manager, useImplicitOutput){

	zval *implicit_output;

	phalcon_fetch_params(0, 1, 0, &implicit_output);

	phalcon_update_property_this(getThis(), SL("_implicitOutput"), implicit_output);
	RETURN_THISW();
}

/**
 * Adds a Css resource to the 'css' collection
 *
 *<code>
 *	$assets->addCss('css/bootstrap.css');
 *	$assets->addCss('http://bootstrap.my-cdn.com/style.css', false);
 *</code>
 *
 * @param string $path
 * @param boolean $local
 * @param boolean $filter
 * @param array $attributes
 * @return Phalcon\Assets\Manager
 */
PHP_METHOD(Phalcon_Assets_Manager, addCss){

	zval *path, *local = NULL, *filter = NULL, *attributes = NULL, type, resource;

	phalcon_fetch_params(0, 1, 3, &path, &local, &filter, &attributes);

	if (!local) {
		local = &PHALCON_GLOBAL(z_true);
	}

	if (!filter) {
		filter = &PHALCON_GLOBAL(z_true);
	}

	if (!attributes) {
		attributes = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "css");

	object_init_ex(&resource, phalcon_assets_resource_css_ce);

	PHALCON_CALL_METHODW(NULL, &resource, "__construct", path, local, filter, attributes);
	PHALCON_CALL_METHODW(NULL, getThis(), "addresourcebytype", &type, &resource);

	RETURN_THISW();
}

/**
 * Adds a javascript resource to the 'js' collection
 *
 *<code>
 *	$assets->addJs('scripts/jquery.js');
 *	$assets->addJs('http://jquery.my-cdn.com/jquery.js', true);
 *</code>
 *
 * @param string $path
 * @param boolean $local
 * @param boolean $filter
 * @param array $attributes
 * @return Phalcon\Assets\Manager
 */
PHP_METHOD(Phalcon_Assets_Manager, addJs){

	zval *path, *local = NULL, *filter = NULL, *attributes = NULL, type, resource;

	phalcon_fetch_params(0, 1, 3, &path, &local, &filter, &attributes);

	if (!local) {
		local = &PHALCON_GLOBAL(z_true);
	}

	if (!filter) {
		filter = &PHALCON_GLOBAL(z_true);
	}

	if (!attributes) {
		attributes = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "js");

	object_init_ex(&resource, phalcon_assets_resource_js_ce);

	PHALCON_CALL_METHODW(NULL, &resource, "__construct", path, local, filter, attributes);
	PHALCON_CALL_METHODW(NULL, getThis(), "addresourcebytype", &type, &resource);

	RETURN_THISW();
}

/**
 * Adds a resource by its type
 *
 *<code>
 *	$assets->addResourceByType('css', new Phalcon\Assets\Resource\Css('css/style.css'));
 *</code>
 *
 * @param string $type
 * @param Phalcon\Assets\Resource $resource
 * @return Phalcon\Assets\Manager
 */
PHP_METHOD(Phalcon_Assets_Manager, addResourceByType){

	zval *type, *resource, *collections, collection;

	phalcon_fetch_params(0, 2, 0, &type, &resource);

	collections = phalcon_read_property(getThis(), SL("_collections"), PH_NOISY);
	if (!phalcon_array_isset_fetch(&collection, collections, type)) {
		object_init_ex(&collection, phalcon_assets_collection_ce);
		phalcon_update_property_array(getThis(), SL("_collections"), type, &collection);
	}

	/** 
	 * Add the resource to the collection
	 */
	PHALCON_CALL_METHODW(NULL, &collection, "add", resource);

	RETURN_THISW();
}

/**
 * Adds a raw resource to the manager
 *
 *<code>
 * $assets->addResource(new Phalcon\Assets\Resource('css', 'css/style.css'));
 *</code>
 *
 * @param Phalcon\Assets\Resource $resource
 * @return Phalcon\Assets\Manager
 */
PHP_METHOD(Phalcon_Assets_Manager, addResource){

	zval *resource, type;

	phalcon_fetch_params(0, 1, 0, &resource);
	PHALCON_VERIFY_CLASS_EX(resource, phalcon_assets_resource_ce, phalcon_assets_exception_ce, 0);

	PHALCON_CALL_METHODW(&type, resource, "gettype");

	/** 
	 * Adds the resource by its type
	 */
	PHALCON_CALL_METHODW(NULL, getThis(), "addresourcebytype", &type, resource);

	RETURN_THISW();
}

/**
 * Sets a collection in the Assets Manager
 *
 *<code>
 * $assets->get('js', $collection);
 *</code>
 *
 * @param string $id
 * @param Phalcon\Assets\Collection $collection
 * @return Phalcon\Assets\Manager
 */
PHP_METHOD(Phalcon_Assets_Manager, set){

	zval *id, *collection;

	phalcon_fetch_params(0, 2, 0, &id, &collection);

	if (unlikely(Z_TYPE_P(id) != IS_STRING)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_assets_exception_ce, "Collection-Id must be a string");
		return;
	}
	if (unlikely(Z_TYPE_P(collection) != IS_OBJECT)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_assets_exception_ce, "Collection must be an object");
		return;
	}

	phalcon_update_property_array(getThis(), SL("_collections"), id, collection);

	RETURN_THISW();
}

/**
 * Returns a collection by its id
 *
 *<code>
 * $scripts = $assets->get('js');
 *</code>
 *
 * @param string $id
 * @return Phalcon\Assets\Collection
 */
PHP_METHOD(Phalcon_Assets_Manager, get){

	zval *id, *collections;

	phalcon_fetch_params(0, 1, 0, &id);

	if (unlikely(Z_TYPE_P(id) != IS_STRING)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_assets_exception_ce, "Collection-Id must be a string");
		return;
	}

	collections = phalcon_read_property(getThis(), SL("_collections"), PH_NOISY);
	if (!phalcon_array_isset_fetch(return_value, collections, id)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_assets_exception_ce, "The collection does not exist in the manager");
		return;
	}
}

/**
 * Returns the CSS collection of assets
 *
 * @return Phalcon\Assets\Collection
 */
PHP_METHOD(Phalcon_Assets_Manager, getCss){

	zval *collections, collection;

	collections = phalcon_read_property(getThis(), SL("_collections"), PH_NOISY);

	/** 
	 * Check if the collection does not exist and create an implicit collection
	 */
	if (!phalcon_array_isset_fetch_str(&collection, collections, SL("css"))) {
		object_init_ex(return_value, phalcon_assets_collection_ce);
		return;
	}

	RETURN_CTORW(&collection);
}

/**
 * Returns the CSS collection of assets
 *
 * @return Phalcon\Assets\Collection
 */
PHP_METHOD(Phalcon_Assets_Manager, getJs){

	zval *collections, collection;

	collections = phalcon_read_property(getThis(), SL("_collections"), PH_NOISY);

	/** 
	 * Check if the collection does not exist and create an implicit collection
	 */
	if (!phalcon_array_isset_fetch_str(&collection, collections, SL("js"))) {
		object_init_ex(return_value, phalcon_assets_collection_ce);
		return;
	}

	RETURN_CTORW(&collection);
}

/**
 * Creates/Returns a collection of resources
 *
 * @param string $name
 * @return Phalcon\Assets\Collection
 */
PHP_METHOD(Phalcon_Assets_Manager, collection){

	zval *name, *collections;

	phalcon_fetch_params(0, 1, 0, &name);

	collections = phalcon_read_property(getThis(), SL("_collections"), PH_NOISY);
	if (!phalcon_array_isset_fetch(return_value, collections, name)) {
		object_init_ex(return_value, phalcon_assets_collection_ce);
		phalcon_update_property_array(getThis(), SL("_collections"), name, return_value);
	}
}

/**
 * Traverses a collection calling the callback to generate its HTML
 *
 * @param Phalcon\Assets\Collection $collection
 * @param callback $callback
 * @param string $type
 * @param array $args
 */
PHP_METHOD(Phalcon_Assets_Manager, output){

	zval *collection, *callback, *z_type = NULL, *args = NULL, type, output, use_implicit_output, exception_message;
	zval resources, filters, prefix, type_css, source_base_path, target_base_path, options, collection_source_path;
	zval complete_source_path, collection_target_path, complete_target_path, join, is_directory, local;
	zval *resource, target_uri, prefixed_path, attributes, parameters, filtered_joined_content, html;

	phalcon_fetch_params(0, 2, 2, &collection, &callback, &type, &args);

	if (z_type) {
		PHALCON_CPY_WRT_CTOR(&type, z_type);
	}

	if (!args) {
		args = &PHALCON_GLOBAL(z_null);
	}

	phalcon_return_property(&use_implicit_output, getThis(), SL("_implicitOutput"));

	/** 
	 * Get the resources as an array
	 */
	PHALCON_CALL_METHODW(&resources, collection, "getresources");

	/** 
	 * Get filters in the collection
	 */
	PHALCON_CALL_METHODW(&filters, collection, "getfilters");

	/** 
	 * Get the collection's prefix
	 */
	PHALCON_CALL_METHODW(&prefix, collection, "getprefix");

	ZVAL_STRING(&type_css, "css");

	/** 
	 * Prepare options if the collection must be filtered
	 */
	if (Z_TYPE(filters) == IS_ARRAY) {
		phalcon_return_property(&options, getThis(), SL("_options"));

		/** 
		 * Check for global options in the assets manager
		 */
		if (Z_TYPE(options) == IS_ARRAY) { 
			/** 
			 * The source base path is a global location where all resources are located
			 */
			if (phalcon_array_isset_str(&options, SL("sourceBasePath"))) {
				phalcon_array_fetch_str(&source_base_path, &options, SL("sourceBasePath"), PH_NOISY);
			}

			/** 
			 * The target base path is a global location where all resources are written
			 */
			if (phalcon_array_isset_str(&options, SL("targetBasePath"))) {
				phalcon_array_fetch_str(&target_base_path, &options, SL("targetBasePath"), PH_NOISY);
			}
		}

		/** 
		 * Check if the collection have its own source base path
		 */
		PHALCON_CALL_METHODW(&collection_source_path, collection, "getsourcepath");

		/** 
		 * Concatenate the global base source path with the collection one
		 */
		if (PHALCON_IS_NOT_EMPTY(&collection_source_path)) {
			PHALCON_CONCAT_VV(&complete_source_path, &source_base_path, &collection_source_path);
		} else {
			PHALCON_CPY_WRT(&complete_source_path, &source_base_path);
		}

		/** 
		 * Check if the collection have its own target base path
		 */
		PHALCON_CALL_METHODW(&collection_target_path, collection, "gettargetpath");

		/** 
		 * Concatenate the global base source path with the collection one
		 */
		if (PHALCON_IS_NOT_EMPTY(&collection_target_path)) {
			PHALCON_CONCAT_VV(&complete_target_path, &target_base_path, &collection_target_path);
		} else {
			PHALCON_CPY_WRT(&complete_target_path, &target_base_path);
		}

		/** 
		 * Check if all the resources in the collection must be joined
		 */
		PHALCON_CALL_METHODW(&join, collection, "getjoin");

		/** 
		 * Check for valid target paths if the collection must be joined
		 */
		if (zend_is_true(&join)) {
			/** 
			 * We need a valid final target path
			 */
			if (PHALCON_IS_EMPTY(&complete_target_path)) {
				PHALCON_CONCAT_SVS(&exception_message, "Path '", &complete_target_path, "' is not a valid target path (1)");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_assets_exception_ce, &exception_message);
				return;
			}

			phalcon_is_dir(&is_directory, &complete_target_path);

			/** 
			 * The targetpath needs to be a valid file
			 */
			if (PHALCON_IS_TRUE(&is_directory)) {
				PHALCON_CONCAT_SVS(&exception_message, "Path '", &complete_target_path, "' is not a valid target path (2)");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_assets_exception_ce, &exception_message);
				return;
			}
		}
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(resources), resource) {
		zval filter_needed, source_path, target_path, path, content, must_filter, *filter;

		ZVAL_FALSE(&filter_needed);

		if (Z_TYPE(type) <= IS_NULL) {
			PHALCON_CALL_METHOD(&type, resource, "gettype");
		}

		/** 
		 * Is the resource local?
		 */
		PHALCON_CALL_METHODW(&local, resource, "getlocal");

		/** 
		 * If the collection must not be joined we must print a HTML for each one
		 */
		if (Z_TYPE(filters) == IS_ARRAY) { 
			if (zend_is_true(&local)) {
				/** 
				 * Get the complete path
				 */
				PHALCON_CALL_METHODW(&source_path, resource, "getrealsourcepath", &complete_source_path);

				/** 
				 * We need a valid source path
				 */
				if (!zend_is_true(&source_path)) {
					PHALCON_CALL_METHODW(&source_path, resource, "getpath");

					PHALCON_CONCAT_SVS(&exception_message, "Resource '", &source_path, "' does not have a valid source path");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_assets_exception_ce, &exception_message);
					return;
				}
			} else {
				/** 
				 * Get the complete source path
				 */
				PHALCON_CALL_METHODW(&source_path, resource, "getpath");

				/** 
				 * resources paths are always filtered
				 */
				ZVAL_TRUE(&filter_needed);
			}

			/** 
			 * Get the target path, we need to write the filtered content to a file
			 */
			PHALCON_CALL_METHODW(&target_path, resource, "getrealtargetpath", &complete_target_path);

			/** 
			 * We need a valid final target path
			 */
			if (PHALCON_IS_EMPTY(&target_path)) {
				PHALCON_CONCAT_SVS(&exception_message, "Resource '", &target_path, "' does not have a valid target path");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_assets_exception_ce, &exception_message);
				return;
			}

			if (zend_is_true(&local)) {
				/** 
				 * Make sure the target path is not the same source path
				 */
				if (PHALCON_IS_EQUAL(&target_path, &source_path)) {
					PHALCON_CONCAT_SVS(&exception_message, "Resource '", &target_path, "' have the same source and target paths");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_assets_exception_ce, &exception_message);
					return;
				}
				if (phalcon_file_exists(&target_path) == SUCCESS) {
					if (phalcon_compare_mtime(&target_path, &source_path)) {
						ZVAL_TRUE(&filter_needed);
					}
				} else {
					ZVAL_TRUE(&filter_needed);
				}
			}
		} else {
			/** 
			 * If there are no filters, just print/buffer the HTML
			 */
			PHALCON_CALL_METHODW(&path, resource, "getrealtargeturi");
			if (Z_TYPE(prefix) != IS_NULL) {
				PHALCON_CONCAT_VV(&prefixed_path, &prefix, &path);
			} else {
				PHALCON_CPY_WRT(&prefixed_path, &path);
			}

			/** 
			 * Gets extra HTML attributes in the resource
			 */
			PHALCON_CALL_METHODW(&attributes, resource, "getattributes");

			/** 
			 * Prepare the parameters for the callback
			 */
			array_init_size(&parameters, 3);
			if (Z_TYPE(attributes) == IS_ARRAY) { 
				phalcon_array_update_long(&attributes, 0, &prefixed_path, PH_COPY);

				phalcon_array_append(&parameters, &attributes, PH_COPY);
			} else {
				phalcon_array_append(&parameters, &prefixed_path, PH_COPY);
			}

			phalcon_array_append(&parameters, &local, PH_COPY);
			phalcon_array_append(&parameters, args, PH_COPY);

			/** 
			 * Call the callback to generate the HTML
			 */
			PHALCON_CALL_USER_FUNC_ARRAYW(&html, callback, &parameters);

			/** 
			 * Implicit output prints the content directly
			 */
			if (zend_is_true(&use_implicit_output)) {
				zend_print_zval(&html, 0);
			} else {
				phalcon_concat_self(&output, &html);
			}

			continue;
		}

		if (zend_is_true(&filter_needed)) {
			/** 
			 * Get the resource's content
			 */
			PHALCON_CALL_METHODW(&content, resource, "getcontent", &complete_source_path);

			/** 
			 * Check if the resource must be filtered
			 */
			PHALCON_CALL_METHODW(&must_filter, resource, "getfilter");

			/** 
			 * Only filter the resource if it's marked as 'filterable'
			 */
			if (zend_is_true(&must_filter)) {
				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(filters), filter) {
					zval filtered_content;
					/** 
					 * Filters must be valid objects
					 */
					if (Z_TYPE_P(filter) != IS_OBJECT) {
						PHALCON_THROW_EXCEPTION_STRW(phalcon_assets_exception_ce, "Filter is invalid");
						return;
					}

					/** 
					 * Calls the method 'filter' which must return a filtered version of the content
					 */
					PHALCON_CALL_METHODW(&filtered_content, filter, "filter", &content);
					PHALCON_CPY_WRT_CTOR(&content, &filtered_content);
				} ZEND_HASH_FOREACH_END();

				/**
				 * Update the joined filtered content
				 */
				if (zend_is_true(&join)) {
					if (PHALCON_IS_EQUAL(&type, &type_css)) {
						if (Z_TYPE(filtered_joined_content) <= IS_NULL) {
							PHALCON_CONCAT_VS(&filtered_joined_content, &content, "");
						} else {
							PHALCON_SCONCAT_VS(&filtered_joined_content, &content, "");
						}
					} else {
						if (Z_TYPE(filtered_joined_content) <= IS_NULL) {
							PHALCON_CONCAT_VS(&filtered_joined_content, &content, ";");
						} else {
							PHALCON_SCONCAT_VS(&filtered_joined_content, &content, ";");
						}
					}
				}
			} else {
				/** 
				 * Update the joined filtered content
				 */
				if (zend_is_true(&join)) {
					if (Z_TYPE(filtered_joined_content) <= IS_NULL) {
						PHALCON_CPY_WRT_CTOR(&filtered_joined_content, &content);
					} else {
						phalcon_concat_self(&filtered_joined_content, &content);
					}
				} else {
					PHALCON_CPY_WRT_CTOR(&filtered_joined_content, &content);
				}
			}

			if (!zend_is_true(&join)) {
				/** 
				 * Write the file using file-put-contents. This respects the openbase-dir also
				 * writes to streams
				 */
				phalcon_file_put_contents(NULL, &target_path, &filtered_joined_content);
			}
		}

		if (!zend_is_true(&join)) {
			/** 
			 * Generate the HTML using the original path in the resource
			 */
			PHALCON_CALL_METHODW(&path, resource, "getrealtargeturi");
			if (Z_TYPE(prefix) != IS_NULL) {
				PHALCON_CONCAT_VV(&prefixed_path, &prefix, &path);
			} else {
				PHALCON_CPY_WRT_CTOR(&prefixed_path, &path);
			}

			/** 
			 * Gets extra HTML attributes in the resource
			 */
			PHALCON_CALL_METHOD(&attributes, resource, "getattributes");

			/** 
			 * Filtered resources are always local
			 */
			ZVAL_TRUE(&local);

			/** 
			 * Prepare the parameters for the callback
			 */
			array_init_size(&parameters, 3);
			if (Z_TYPE(attributes) == IS_ARRAY) { 
				phalcon_array_update_long(&attributes, 0, &prefixed_path, PH_COPY);

				phalcon_array_append(&parameters, &attributes, PH_COPY);
			} else {
				phalcon_array_append(&parameters, &prefixed_path, PH_COPY);
			}

			phalcon_array_append(&parameters, &local, PH_COPY);
			phalcon_array_append(&parameters, args, PH_COPY);

			/** 
			 * Call the callback to generate the HTML
			 */
			PHALCON_CALL_USER_FUNC_ARRAYW(&html, callback, &parameters);

			/** 
			 * Implicit output prints the content directly
			 */
			if (zend_is_true(&use_implicit_output)) {
				zend_print_zval(&html, 0);
			} else {
				phalcon_concat_self(&output, &html);
			}
		}
	} ZEND_HASH_FOREACH_END();

	if (Z_TYPE(filters) == IS_ARRAY) { 
		if (zend_is_true(&join)) {
			/** 
			 * Write the file using file_put_contents. This respects the openbase-dir also
			 * writes to streams
			 */
			phalcon_file_put_contents(NULL, &complete_target_path, &filtered_joined_content);

			/** 
			 * Generate the HTML using the original path in the resource
			 */
			PHALCON_CALL_METHODW(&target_uri, collection, "gettargeturi");
			if (Z_TYPE(prefix) != IS_NULL) {
				PHALCON_CONCAT_VV(&prefixed_path, &prefix, &target_uri);
			} else {
				PHALCON_CPY_WRT_CTOR(&prefixed_path, &target_uri);
			}

			/** 
			 * Gets extra HTML attributes in the resource
			 */
			PHALCON_CALL_METHODW(&attributes, collection, "getattributes");
			PHALCON_CALL_METHODW(&local, collection, "gettargetlocal");

			/** 
			 * Prepare the parameters for the callback
			 */
			array_init_size(&parameters, 3);
			if (Z_TYPE(attributes) == IS_ARRAY) { 
				phalcon_array_update_long(&attributes, 0, &prefixed_path, PH_COPY);

				phalcon_array_append(&parameters, &attributes, PH_COPY);
			} else {
				phalcon_array_append(&parameters, &prefixed_path, PH_COPY);
			}

			phalcon_array_append(&parameters, &local, PH_COPY);
			phalcon_array_append(&parameters, args, PH_COPY);

			/** 
			 * Call the callback to generate the HTML
			 */
			PHALCON_CALL_USER_FUNC_ARRAYW(&html, callback, &parameters);

			/** 
			 * Implicit output prints the content directly
			 */
			if (zend_is_true(&use_implicit_output)) {
				zend_print_zval(&html, 0);
			} else {
				phalcon_concat_self(&output, &html);
			}
		}
	}

	RETURN_CTORW(&output);
}

/**
 * Prints the HTML for CSS resources
 *
 * @param string $collectionName
 * @param array $args
 */
PHP_METHOD(Phalcon_Assets_Manager, outputCss){

	zval *collection_name = NULL, *args = NULL, collection, callback, type;

	phalcon_fetch_params(0, 0, 2, &collection_name, &args);

	if (!collection_name) {
		collection_name = &PHALCON_GLOBAL(z_null);
	}

	if (!args) {
		args = &PHALCON_GLOBAL(z_null);
	}

	if (PHALCON_IS_EMPTY(collection_name)) {
		PHALCON_CALL_METHODW(&collection, getThis(), "getcss");
	} else {
		PHALCON_CALL_METHODW(&collection, getThis(), "get", collection_name);
	}

	array_init_size(&callback, 2);
	add_next_index_stringl(&callback, SL("Phalcon\\Tag"));
	add_next_index_stringl(&callback, SL("stylesheetLink"));

	ZVAL_STRING(&type, "css");

	PHALCON_RETURN_CALL_METHODW(getThis(), "output", &collection, &callback, &type, args);
}

/**
 * Prints the HTML for JS resources
 *
 * @param string $collectionName
 * @param array $args
 */
PHP_METHOD(Phalcon_Assets_Manager, outputJs){

	zval *collection_name = NULL, *args = NULL, collection, callback, type;

	phalcon_fetch_params(0, 0, 2, &collection_name, &args);

	if (!collection_name) {
		collection_name = &PHALCON_GLOBAL(z_null);
	}

	if (!args) {
		args = &PHALCON_GLOBAL(z_null);
	}

	if (PHALCON_IS_EMPTY(collection_name)) {
		PHALCON_CALL_METHODW(&collection, getThis(), "getjs");
	} else {
		PHALCON_CALL_METHODW(&collection, getThis(), "get", collection_name);
	}

	array_init_size(&callback, 2);
	add_next_index_stringl(&callback, SL("Phalcon\\Tag"));
	add_next_index_stringl(&callback, SL("javascriptInclude"));

	ZVAL_STRING(&type, "js");

	PHALCON_RETURN_CALL_METHODW(getThis(), "output", &collection, &callback, &type, args);
}
