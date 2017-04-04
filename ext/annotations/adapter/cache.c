
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
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
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

	if (!phalcon_array_isset_fetch_str(&service, options, SL("service"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_annotations_exception_ce, "No service given in options");
		return;
	}

	if (Z_TYPE(service) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&cache, getThis(), "getresolveservice", &service);
	} else {
		ZVAL_COPY_VALUE(&cache, &service);
	}
	PHALCON_VERIFY_INTERFACE(&cache, phalcon_cache_backendinterface_ce);

	phalcon_update_property(getThis(), SL("_cache"), &cache);

	if (phalcon_array_isset_fetch_str(&lifetime, options, SL("lifetime"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_lifetime"), &lifetime);
	}

	phalcon_update_property(getThis(), SL("_cache"), &cache);
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

	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cache, getThis(), SL("_cache"), PH_NOISY|PH_READONLY);

	PHALCON_RETURN_CALL_METHOD(&cache, "get", key, &lifetime);
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

	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cache, getThis(), SL("_cache"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &cache, "save", key, data, &lifetime);
}
