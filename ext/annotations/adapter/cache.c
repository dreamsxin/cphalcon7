
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

#include "annotations/adapter/cache.h"
#include "annotations/adapter.h"
#include "annotations/adapterinterface.h"
#include "annotations/exception.h"
#include "cache/backendinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/exception.h"

/**
 * Phalcon\Annotations\Adapter\Cache
 *
 * Stores the parsed annotations in cache. This adapter is suitable for production
 *
 *<code>
 * $annotations = new \Phalcon\Annotations\Adapter\Cache();
 *</code>
 */
zend_class_entry *phalcon_annotations_adapter_cache_ce;

PHP_METHOD(Phalcon_Annotations_Adapter_Cache, __construct);
PHP_METHOD(Phalcon_Annotations_Adapter_Cache, read);
PHP_METHOD(Phalcon_Annotations_Adapter_Cache, write);


ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_adapter_cache___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_adapter_cache_read, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_adapter_cache_write, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_annotations_adapter_cache_method_entry[] = {
	PHP_ME(Phalcon_Annotations_Adapter_Cache, __construct, arginfo_phalcon_annotations_adapter_cache___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Adapter_Cache, read, arginfo_phalcon_annotations_adapter_cache_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Adapter_Cache, write, arginfo_phalcon_annotations_adapter_cache_write, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Annotations\Adapter\Cache initializer
 */
PHALCON_INIT_CLASS(Phalcon_Annotations_Adapter_Cache){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Annotations\\Adapter, Cache, annotations_adapter_cache, phalcon_annotations_adapter_ce, phalcon_annotations_adapter_cache_method_entry, 0);

	zend_declare_property_long(phalcon_annotations_adapter_cache_ce, SL("_lifetime"), 8600, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_annotations_adapter_cache_ce, SL("_cache"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_annotations_adapter_cache_ce, 1, phalcon_annotations_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Constructor for Phalcon\Session\Adapter\Cache
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Annotations_Adapter_Cache, __construct){

	zval *options, service = {}, cache = {}, lifetime = {};

	phalcon_fetch_params(0, 1, 0, &options);

	if (Z_TYPE_P(options) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_annotations_exception_ce, "The options must be an array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&service, options, SL("service"))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_annotations_exception_ce, "No service given in options");
		return;
	}

	if (Z_TYPE(service) != IS_OBJECT) {
		PHALCON_CALL_METHODW(&cache, getThis(), "getresolveservice", &service);
	} else {
		PHALCON_CPY_WRT(&cache, &service);
	}
	PHALCON_VERIFY_INTERFACEW(&cache, phalcon_cache_backendinterface_ce);

	phalcon_update_property_zval(getThis(), SL("_cache"), &cache);

	if (phalcon_array_isset_fetch_str(&lifetime, options, SL("lifetime"))) {
		phalcon_update_property_zval(getThis(), SL("_lifetime"), &lifetime);
	}

	phalcon_update_property_zval(getThis(), SL("_cache"), &cache);
}

/**
 * Reads parsed annotations from cache
 *
 * @param string $key
 * @return Phalcon\Annotations\Reflection
 */
PHP_METHOD(Phalcon_Annotations_Adapter_Cache, read)
{
	zval *key, lifetime = {}, cache = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY);
	phalcon_read_property(&cache, getThis(), SL("_cache"), PH_NOISY);

	PHALCON_RETURN_CALL_METHODW(&cache, "get", key, &lifetime);
}

/**
 * Writes parsed annotations to cache
 *
 * @param string $key
 * @param Phalcon\Annotations\Reflection $data
 */
PHP_METHOD(Phalcon_Annotations_Adapter_Cache, write){

	zval *key, *data, lifetime = {}, cache = {};

	phalcon_fetch_params(0, 2, 0, &key, &data);

	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY);
	phalcon_read_property(&cache, getThis(), SL("_cache"), PH_NOISY);

	PHALCON_CALL_METHODW(NULL, &cache, "save", key, data, &lifetime);	
}
