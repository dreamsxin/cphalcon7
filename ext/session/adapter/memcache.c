
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

#include "session/adapter/memcache.h"
#include "session/adapter.h"
#include "session/adapterinterface.h"
#include "session/exception.h"
#include "cache/backend/memcache.h"
#include "cache/frontend/data.h"

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
 * Phalcon\Session\Adapter\Memcache
 *
 * This adapter store sessions in memcache
 *
 *<code>
 * $session = new Phalcon\Session\Adapter\Memcache(array(
 *    'host' => '127.0.0.1',
 *    'port' => 11211,
 *    'lifetime' => 3600,
 *    'persistent' => TRUE,
 *    'prefix' => 'my_'
 * ));
 *
 * $session->start();
 *
 * $session->set('var', 'some-value');
 *
 * echo $session->get('var');
 *</code>
 */
zend_class_entry *phalcon_session_adapter_memcache_ce;

PHP_METHOD(Phalcon_Session_Adapter_Memcache, start);
PHP_METHOD(Phalcon_Session_Adapter_Memcache, open);
PHP_METHOD(Phalcon_Session_Adapter_Memcache, close);
PHP_METHOD(Phalcon_Session_Adapter_Memcache, read);
PHP_METHOD(Phalcon_Session_Adapter_Memcache, write);
PHP_METHOD(Phalcon_Session_Adapter_Memcache, destroy);
PHP_METHOD(Phalcon_Session_Adapter_Memcache, gc);

static const zend_function_entry phalcon_session_adapter_memcache_method_entry[] = {
	PHP_ME(Phalcon_Session_Adapter_Memcache, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Memcache, open, arginfo_phalcon_session_adapterinterface_open, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Memcache, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Memcache, read, arginfo_phalcon_session_adapterinterface_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Memcache, write, arginfo_phalcon_session_adapterinterface_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Memcache, destroy, arginfo_phalcon_session_adapterinterface_destroy, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Memcache, gc, arginfo_phalcon_session_adapterinterface_gc, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Session\Adapter\Memcache initializer
 */
PHALCON_INIT_CLASS(Phalcon_Session_Adapter_Memcache){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Session\\Adapter, Memcache, session_adapter_memcache, phalcon_session_adapter_ce, phalcon_session_adapter_memcache_method_entry, 0);

	zend_declare_property_long(phalcon_session_adapter_memcache_ce, SL("_lifetime"), 8600, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_memcache_ce, SL("_memcache"), ZEND_ACC_PROTECTED);

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
PHP_METHOD(Phalcon_Session_Adapter_Memcache, start){

	zval options = {}, host = {}, port = {}, lifetime = {}, persistent = {}, prefix = {};
	zval frontend_option = {}, backend_option = {}, frontend_data = {}, memcache = {};
//#ifndef PHALCON_USE_PHP_SESSION
	zval callable_open = {}, callable_close = {}, callable_read = {}, callable_write = {}, callable_destroy = {}, callable_gc = {};
//#endif

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY);

	if (Z_TYPE(options) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_session_exception_ce, "The options must be an array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&host, &options, SL("host"))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_session_exception_ce, "No session host given in options");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&port, &options, SL("port"))) {
		ZVAL_LONG(&port, 11211);
	}

	if (!phalcon_array_isset_fetch_str(&lifetime, &options, SL("lifetime"))) {
		ZVAL_LONG(&lifetime, 8600);
	} else {
		phalcon_update_property_zval(getThis(), SL("_lifetime"), &lifetime);
	}

	if (!phalcon_array_isset_fetch_str(&persistent, &options, SL("persistent"))) {
		ZVAL_FALSE(&persistent);
	}

	if (!phalcon_array_isset_fetch_str(&prefix, &options, SL("prefix"))) {
		ZVAL_EMPTY_STRING(&prefix);
	}

	/* create memcache instance */
	array_init_size(&frontend_option, 1);

	phalcon_array_update_str(&frontend_option, SL("lifetime"), &lifetime, PH_COPY);

	object_init_ex(&frontend_data, phalcon_cache_frontend_data_ce);

	PHALCON_CALL_METHODW(NULL, &frontend_data, "__construct", &frontend_option);

	array_init_size(&backend_option, 3);

	phalcon_array_update_str(&backend_option, SL("host"), &host, PH_COPY);
	phalcon_array_update_str(&backend_option, SL("port"), &port, PH_COPY);
	phalcon_array_update_str(&backend_option, SL("persistent"), &persistent, PH_COPY);
	phalcon_array_update_str(&backend_option, SL("prefix"), &prefix, PH_COPY);

	object_init_ex(&memcache, phalcon_cache_backend_memcache_ce);

	PHALCON_CALL_METHODW(NULL, &memcache, "__construct", &frontend_data, &backend_option);

	phalcon_update_property_zval(getThis(), SL("_memcache"), &memcache);

//#ifdef PHALCON_USE_PHP_SESSION
//	PHALCON_CALL_FUNCTIONW(return_value, "session_set_save_handler", getThis(), &PHALCON_GLOBAL(z_true));
//#else
	/* open callback */
	array_init_size(&callable_open, 2);
	phalcon_array_append(&callable_open, getThis(), 0);
	phalcon_array_append_string(&callable_open, SL("open"), 0);

	/* close callback */
	array_init_size(&callable_close, 2);
	phalcon_array_append(&callable_close, getThis(), 0);
	phalcon_array_append_string(&callable_close, SL("close"), 0);

	/* read callback */
	array_init_size(&callable_read, 2);
	phalcon_array_append(&callable_read, getThis(), 0);
	phalcon_array_append_string(&callable_read, SL("read"), 0);

	/* write callback */
	array_init_size(&callable_write, 2);
	phalcon_array_append(&callable_write, getThis(), 0);
	phalcon_array_append_string(&callable_write, SL("write"), 0);

	/* destroy callback */
	array_init_size(&callable_destroy, 2);
	phalcon_array_append(&callable_destroy, getThis(), 0);
	phalcon_array_append_string(&callable_destroy, SL("destroy"), 0);

	/* gc callback */
	array_init_size(&callable_gc, 2);
	phalcon_array_append(&callable_gc, getThis(), 0);
	phalcon_array_append_string(&callable_gc, SL("gc"), 0);

	PHALCON_CALL_FUNCTIONW(return_value, "session_set_save_handler", &callable_open, &callable_close, &callable_read, &callable_write, &callable_destroy, &callable_gc);
	PHALCON_CALL_FUNCTIONW(NULL, "session_register_shutdown");
//#endif
	if (!zend_is_true(return_value)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_session_exception_ce, "Sets user-level session storage functions failed");
		RETURN_FALSE;
	}
	PHALCON_CALL_PARENTW(return_value, phalcon_session_adapter_memcache_ce, getThis(), "start");
}

/**
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Memcache, open){

	RETURN_TRUE;
}

/**
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Memcache, close){

	RETURN_TRUE;
}

/**
 *
 * @param string $sessionId
 * @return mixed
 */
PHP_METHOD(Phalcon_Session_Adapter_Memcache, read){

	zval *sid, lifetime = {}, memcache = {};

	phalcon_fetch_params(0, 1, 0, &sid);

	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY);
	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_NOISY);

	if (Z_TYPE(memcache) == IS_OBJECT) {
		PHALCON_RETURN_CALL_METHODW(&memcache, "get", sid, &lifetime);
		return;
	} else {
		RETURN_FALSE;
	}
}

/**
 *
 * @param string $sessionId
 * @param string $data
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Memcache, write){

	zval *sid, *data, lifetime = {}, memcache = {};

	phalcon_fetch_params(0, 2, 0, &sid, &data);

	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY);
	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_NOISY);

	if (Z_TYPE(memcache) == IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, &memcache, "save", sid, data, &lifetime);
	}
}

/**
 *
 * @param string $session_id optional, session id
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Memcache, destroy){

	zval *_sid = NULL, sid = {}, memcache = {};

	phalcon_fetch_params(0, 0, 1, &_sid);

	if (!_sid) {
		PHALCON_CALL_SELFW(&sid, "getid");
	} else {
		PHALCON_CPY_WRT(&sid, _sid);
	}

	phalcon_read_property(&memcache, getThis(), SL("_memcache"), PH_NOISY);

	if (Z_TYPE(memcache) == IS_OBJECT) {
		PHALCON_RETURN_CALL_METHODW(&memcache, "delete", &sid);
		return;
	} else {
		RETURN_FALSE;
	}
}

/**
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Memcache, gc){

	RETURN_TRUE;
}
