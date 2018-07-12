
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

#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/frontendinterface.h"
#include "cache/exception.h"
#include "di/injectable.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/string.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/exception.h"

/**
 * Phalcon\Cache\Backend
 *
 * This class implements common functionality for backend adapters. A backend cache adapter may extend this class
 */
zend_class_entry *phalcon_cache_backend_ce;

PHP_METHOD(Phalcon_Cache_Backend, __construct);
PHP_METHOD(Phalcon_Cache_Backend, start);
PHP_METHOD(Phalcon_Cache_Backend, stop);
PHP_METHOD(Phalcon_Cache_Backend, getFrontend);
PHP_METHOD(Phalcon_Cache_Backend, getOptions);
PHP_METHOD(Phalcon_Cache_Backend, isFresh);
PHP_METHOD(Phalcon_Cache_Backend, isStarted);
PHP_METHOD(Phalcon_Cache_Backend, getLifetime);
PHP_METHOD(Phalcon_Cache_Backend, setPrefix);
PHP_METHOD(Phalcon_Cache_Backend, getPrefix);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, frontend)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cache_backend_setprefix, 0, 0, 1)
	ZEND_ARG_INFO(0, prefix)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cache_backend_method_entry[] = {
	PHP_ME(Phalcon_Cache_Backend, __construct, arginfo_phalcon_cache_backend___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cache_Backend, start, arginfo_phalcon_cache_backendinterface_start, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend, stop, arginfo_phalcon_cache_backendinterface_stop, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend, getFrontend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend, getOptions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend, isFresh, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend, isStarted, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend, getLifetime, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend, setPrefix, arginfo_phalcon_cache_backend_setprefix, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cache_Backend, getPrefix, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cache\Backend initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache, Backend, cache_backend, phalcon_di_injectable_ce, phalcon_cache_backend_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_cache_backend_ce, SL("_frontend"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cache_backend_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_cache_backend_ce, SL("_prefix"), "", ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_cache_backend_ce, SL("_lastKey"), "", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cache_backend_ce, SL("_lastLifetime"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_cache_backend_ce, SL("_fresh"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_cache_backend_ce, SL("_started"), 0, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_cache_backend_ce, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Cache\Backend constructor
 *
 * @param Phalcon\Cache\FrontendInterface $frontend
 * @param array $options
 */
PHP_METHOD(Phalcon_Cache_Backend, __construct){

	zval *frontend, *options = NULL, prefix = {};

	phalcon_fetch_params(0, 1, 1, &frontend, &options);
	PHALCON_VERIFY_INTERFACE_EX(frontend, phalcon_cache_frontendinterface_ce, phalcon_cache_exception_ce);

	phalcon_update_property(getThis(), SL("_frontend"), frontend);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		/**
		 * A common option is the prefix
		 */
		if (phalcon_array_isset_fetch_str(&prefix, options, SL("prefix"), PH_READONLY)) {
			if (phalcon_fast_strpos_str(NULL, &prefix, SL("#"))) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The prefix is currupted, can't contain `#`");
				return;
			}
			phalcon_update_property(getThis(), SL("_prefix"), &prefix);
		}

		phalcon_update_property(getThis(), SL("_options"), options);
	}
}

/**
 * Starts a cache. The $keyname allows to identify the created fragment
 *
 * @param int|string $keyName
 * @param long $lifetime
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend, start){

	zval *key_name, *lifetime = NULL, *nobuffer = NULL, *fresh = NULL, frontend = {};

	phalcon_fetch_params(0, 1, 2, &key_name, &lifetime, &nobuffer);

	phalcon_update_property(getThis(), SL("_lastKey"), key_name);

	if (lifetime && Z_TYPE_P(lifetime) == IS_LONG) {
		phalcon_update_property(getThis(), SL("_lastLifetime"), lifetime);
	}
	PHALCON_CALL_METHOD(return_value, getThis(), "get", key_name);

	if (nobuffer && Z_TYPE_P(nobuffer) == IS_TRUE) {
		return;
	}

	if (Z_TYPE_P(return_value) == IS_NULL) {
		fresh = &PHALCON_GLOBAL(z_true);

		phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_NOISY|PH_READONLY);
		PHALCON_CALL_METHOD(NULL, &frontend, "start");
	} else {
		fresh = &PHALCON_GLOBAL(z_false);
	}

	phalcon_update_property(getThis(), SL("_fresh"), fresh);
	phalcon_update_property(getThis(), SL("_started"), &PHALCON_GLOBAL(z_true));
}

/**
 * Stops the frontend without store any cached content
 *
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend, stop){

	zval *stop_buffer = NULL, frontend = {};

	phalcon_fetch_params(0, 0, 1, &stop_buffer);

	if (!stop_buffer || PHALCON_IS_TRUE(stop_buffer)) {
		phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_NOISY|PH_READONLY);
		PHALCON_CALL_METHOD(NULL, &frontend, "stop");
	}

	phalcon_update_property_bool(getThis(), SL("_started"), 0);
}

/**
 * Returns front-end instance adapter related to the back-end
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Cache_Backend, getFrontend){


	RETURN_MEMBER(getThis(), "_frontend");
}

/**
 * Returns the backend options
 *
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend, getOptions){


	RETURN_MEMBER(getThis(), "_options");
}

/**
 * Checks whether the last cache is fresh or cached
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend, isFresh){


	RETURN_MEMBER(getThis(), "_fresh");
}

/**
 * Checks whether the cache has starting buffering or not
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend, isStarted){


	RETURN_MEMBER(getThis(), "_started");
}

/**
 * Gets the last lifetime set
 *
 * @return int
 */
PHP_METHOD(Phalcon_Cache_Backend, getLifetime){

	zval lifetime = {}, frontend = {};

	phalcon_read_property(&lifetime, getThis(), SL("_lastLifetime"), PH_NOISY|PH_COPY);
	if (Z_TYPE(lifetime) != IS_LONG) {
		phalcon_read_property(&frontend, getThis(), SL("_frontend"), PH_NOISY|PH_READONLY);
		PHALCON_CALL_METHOD(&lifetime, &frontend, "getlifetime");
	}
	RETURN_ZVAL(&lifetime, 0, 0);
}

/**
 * Sets prefix
 *
 * @param string $prefix
 * @return Phalcon\Cache\Backend
 */
PHP_METHOD(Phalcon_Cache_Backend, setPrefix){

	zval *prefix;

	phalcon_fetch_params(0, 1, 0, &prefix);

	phalcon_update_property(getThis(), SL("_prefix"), prefix);

	RETURN_THIS();
}

/**
 * Gets prefix
 *
 * @return string
 */
PHP_METHOD(Phalcon_Cache_Backend, getPrefix){

	RETURN_MEMBER(getThis(), "_prefix");
}
