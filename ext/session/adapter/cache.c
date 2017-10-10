
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

#include "session/adapter/cache.h"
#include "session/adapter.h"
#include "session/adapterinterface.h"
#include "session/exception.h"
#include "cache/backendinterface.h"

#ifdef PHALCON_USE_PHP_SESSION
#include <ext/session/php_session.h>
#endif

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/array.h"
#include "kernel/object.h"

/**
 * Phalcon\Session\Adapter\Cache
 *
 * This adapter store sessions in cache
 *
 *<code>
 * $frontCache = new Phalcon\Cache\Frontend\Data(array(
 *    "lifetime" => 3600
 * ));
 *
 * $cache = new Phalcon\Cache\Backend\Redis($frontCache, array(
 *		'host' => 'localhost',
 *		'port' => 6379,
 *		'auth' => 'foobared',
 *  	'persistent' => false
 * ));
 *
 * $session = new Phalcon\Session\Adapter\Cache(array(
 *     'service' => $cache // or service name
 * ));
 *
 * $session->start();
 *
 * $session->set('var', 'some-value');
 *
 * echo $session->get('var');
 *</code>
 */
zend_class_entry *phalcon_session_adapter_cache_ce;

PHP_METHOD(Phalcon_Session_Adapter_Cache, start);
PHP_METHOD(Phalcon_Session_Adapter_Cache, open);
PHP_METHOD(Phalcon_Session_Adapter_Cache, close);
PHP_METHOD(Phalcon_Session_Adapter_Cache, read);
PHP_METHOD(Phalcon_Session_Adapter_Cache, write);
PHP_METHOD(Phalcon_Session_Adapter_Cache, destroy);
PHP_METHOD(Phalcon_Session_Adapter_Cache, gc);

static const zend_function_entry phalcon_session_adapter_cache_method_entry[] = {
	PHP_ME(Phalcon_Session_Adapter_Cache, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Cache, open, arginfo_phalcon_session_adapterinterface_open, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Cache, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Cache, read, arginfo_phalcon_session_adapterinterface_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Cache, write, arginfo_phalcon_session_adapterinterface_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Cache, destroy, arginfo_phalcon_session_adapterinterface_destroy, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Cache, gc, arginfo_phalcon_session_adapterinterface_gc, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Session\Adapter\Cache initializer
 */
PHALCON_INIT_CLASS(Phalcon_Session_Adapter_Cache){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Session\\Adapter, Cache, session_adapter_cache, phalcon_session_adapter_ce, phalcon_session_adapter_cache_method_entry, 0);

	zend_declare_property_long(phalcon_session_adapter_cache_ce, SL("_lifetime"), 8600, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_cache_ce, SL("_cache"), ZEND_ACC_PROTECTED);

//#ifdef PHALCON_USE_PHP_SESSION
//	zend_class_implements(
//		phalcon_session_adapter_cache_ce, 1,
//		php_session_iface_entry
//	);
//#endif
	return SUCCESS;
}

/**
 * Starts the session (if headers are already sent the session will not be started)
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Cache, start){

	zval options = {}, service = {}, cache = {}, lifetime = {};
//#ifndef PHALCON_USE_PHP_SESSION
	zval callable_open = {}, callable_close = {}, callable_read = {}, callable_write = {}, callable_destroy = {}, callable_gc = {};
//#endif

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(options) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_session_exception_ce, "The options must be an array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&service, &options, SL("service"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_session_exception_ce, "No service given in options");
		return;
	}

	if (Z_TYPE(service) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&cache, getThis(), "getresolveservice", &service);
	} else {
		ZVAL_COPY(&cache, &service);
	}
	PHALCON_VERIFY_INTERFACE(&cache, phalcon_cache_backendinterface_ce);

	phalcon_update_property(getThis(), SL("_cache"), &cache);

	if (phalcon_array_isset_fetch_str(&lifetime, &options, SL("lifetime"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_lifetime"), &lifetime);
	}

	phalcon_update_property(getThis(), SL("_cache"), &cache);
	zval_ptr_dtor(&cache);

	/* open callback */
	array_init_size(&callable_open, 2);
	phalcon_array_append(&callable_open, getThis(), PH_COPY);
	phalcon_array_append_str(&callable_open, SL("open"), 0);

	/* close callback */
	array_init_size(&callable_close, 2);
	phalcon_array_append(&callable_close, getThis(), PH_COPY);
	phalcon_array_append_str(&callable_close, SL("close"), 0);

	/* read callback */
	array_init_size(&callable_read, 2);
	phalcon_array_append(&callable_read, getThis(), PH_COPY);
	phalcon_array_append_str(&callable_read, SL("read"), 0);

	/* write callback */
	array_init_size(&callable_write, 2);
	phalcon_array_append(&callable_write, getThis(), PH_COPY);
	phalcon_array_append_str(&callable_write, SL("write"), 0);

	/* destroy callback */
	array_init_size(&callable_destroy, 2);
	phalcon_array_append(&callable_destroy, getThis(), PH_COPY);
	phalcon_array_append_str(&callable_destroy, SL("destroy"), 0);

	/* gc callback */
	array_init_size(&callable_gc, 2);
	phalcon_array_append(&callable_gc, getThis(), PH_COPY);
	phalcon_array_append_str(&callable_gc, SL("gc"), 0);

	PHALCON_CALL_FUNCTION(return_value, "session_set_save_handler", &callable_open, &callable_close, &callable_read, &callable_write, &callable_destroy, &callable_gc);
	zval_ptr_dtor(&callable_open);
	zval_ptr_dtor(&callable_close);
	zval_ptr_dtor(&callable_read);
	zval_ptr_dtor(&callable_write);
	zval_ptr_dtor(&callable_destroy);
	zval_ptr_dtor(&callable_gc);
	PHALCON_CALL_FUNCTION(NULL, "session_register_shutdown");

	if (!zend_is_true(return_value)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_session_exception_ce, "Sets user-level session storage functions failed");
		RETURN_FALSE;
	}
	PHALCON_CALL_PARENT(return_value, phalcon_session_adapter_cache_ce, getThis(), "start");
}

/**
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Cache, open){

	RETURN_TRUE;
}

/**
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Cache, close){

	RETURN_TRUE;
}

/**
 *
 * @param string $sessionId
 * @return mixed
 */
PHP_METHOD(Phalcon_Session_Adapter_Cache, read){

	zval *sid, lifetime = {}, cache = {};

	phalcon_fetch_params(0, 1, 0, &sid);

	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cache, getThis(), SL("_cache"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(return_value, &cache, "get", sid, &lifetime);
	if (Z_TYPE_P(return_value)!=IS_STRING) {
		RETURN_EMPTY_STRING();
	}
}

/**
 *
 * @param string $sessionId
 * @param string $data
 */
PHP_METHOD(Phalcon_Session_Adapter_Cache, write){

	zval *sid, *data, lifetime = {}, cache = {};

	phalcon_fetch_params(0, 2, 0, &sid, &data);

	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cache, getThis(), SL("_cache"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(return_value, &cache, "save", sid, data, &lifetime);
}

/**
 *
 * @param string $session_id optional, session id
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Cache, destroy){

	zval *_sid = NULL, sid = {}, cache = {};

	phalcon_fetch_params(0, 0, 1, &_sid);

	if (!_sid) {
		PHALCON_CALL_SELF(&sid, "getid");
	} else {
		ZVAL_COPY(&sid, _sid);
	}

	phalcon_read_property(&cache, getThis(), SL("_cache"), PH_NOISY|PH_READONLY);

	PHALCON_RETURN_CALL_METHOD(&cache, "delete", &sid);
	zval_ptr_dtor(&sid);
}

/**
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Cache, gc){

	RETURN_TRUE;
}
