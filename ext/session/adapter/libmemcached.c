
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

#include "session/adapter/libmemcached.h"
#include "session/adapter.h"
#include "session/adapterinterface.h"
#include "session/exception.h"
#include "cache/backend/libmemcached.h"
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
 * Phalcon\Session\Adapter\Libmemcached
 *
 * This adapter store sessions in libmemcached
 *
 *<code>
 * $session = new Phalcon\Session\Adapter\Libmemcached(array(
 *     'servers' => array(
 *         array('host' => 'localhost', 'port' => 11211, 'weight' => 1),
 *     ),
 *     'client' => array(
 *         Memcached::OPT_HASH => Memcached::HASH_MD5,
 *         Memcached::OPT_PREFIX_KEY => 'prefix.',
 *     ),
 *    'lifetime' => 3600,
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
zend_class_entry *phalcon_session_adapter_libmemcached_ce;

PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, start);
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, open);
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, close);
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, read);
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, write);
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, destroy);
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, gc);

static const zend_function_entry phalcon_session_adapter_libmemcached_method_entry[] = {
	PHP_ME(Phalcon_Session_Adapter_Libmemcached, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Libmemcached, open, arginfo_phalcon_session_adapterinterface_open, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Libmemcached, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Libmemcached, read, arginfo_phalcon_session_adapterinterface_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Libmemcached, write, arginfo_phalcon_session_adapterinterface_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Libmemcached, destroy, arginfo_phalcon_session_adapterinterface_destroy, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter_Libmemcached, gc, arginfo_phalcon_session_adapterinterface_gc, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Session\Adapter\Libmemcached initializer
 */
PHALCON_INIT_CLASS(Phalcon_Session_Adapter_Libmemcached){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Session\\Adapter, Libmemcached, session_adapter_libmemcached, phalcon_session_adapter_ce, phalcon_session_adapter_libmemcached_method_entry, 0);

	zend_declare_property_long(phalcon_session_adapter_libmemcached_ce, SL("_lifetime"), 8600, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_libmemcached_ce, SL("_libmemcached"), ZEND_ACC_PROTECTED);

//#ifdef PHALCON_USE_PHP_SESSION
//	zend_class_implements(
//		phalcon_session_adapter_libmemcached_ce, 1,
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
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, start){

	zval options = {}, servers = {}, client = {}, lifetime = {}, prefix = {}, backend_option = {}, frontend_option = {}, frontend_data = {}, libmemcached = {};
//#ifndef PHALCON_USE_PHP_SESSION
	zval callable_open = {}, callable_close = {}, callable_read = {}, callable_write = {}, callable_destroy = {}, callable_gc = {};
//#endif

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY);

	if (Z_TYPE(options) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_session_exception_ce, "The options must be an array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&servers, &options, SL("servers"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_session_exception_ce, "No servers given in options");
		return;
	}

	array_init(&backend_option);

	phalcon_array_update_str(&backend_option, SL("servers"), &servers, PH_COPY);

	if (phalcon_array_isset_fetch_str(&client, &options, SL("client"))) {
		phalcon_array_update_str(&backend_option, SL("client"), &client, PH_COPY);
	}

	if (!phalcon_array_isset_fetch_str(&lifetime, &options, SL("lifetime"))) {
		ZVAL_LONG(&lifetime, 8600);
	} else {
		phalcon_update_property_zval(getThis(), SL("_lifetime"), &lifetime);
	}

	if (phalcon_array_isset_fetch_str(&prefix, &options, SL("prefix"))) {
        phalcon_array_update_str(&backend_option, SL("prefix"), &prefix, PH_COPY);
	}

	/* create libmemcached instance */
	array_init_size(&frontend_option, 1);

	phalcon_array_update_str(&frontend_option, SL("lifetime"), &lifetime, PH_COPY);

	object_init_ex(&frontend_data, phalcon_cache_frontend_data_ce);

	PHALCON_CALL_METHOD(NULL, &frontend_data, "__construct", &frontend_option);

	object_init_ex(&libmemcached, phalcon_cache_backend_libmemcached_ce);

	PHALCON_CALL_METHOD(NULL, &libmemcached, "__construct", &frontend_data, &backend_option);

	phalcon_update_property_zval(getThis(), SL("_libmemcached"), &libmemcached);

//#ifdef PHALCON_USE_PHP_SESSION
//	PHALCON_CALL_FUNCTION(return_value, "session_set_save_handler", getThis(), &PHALCON_GLOBAL(z_true));
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

	PHALCON_CALL_FUNCTION(return_value, "session_set_save_handler", &callable_open, &callable_close, &callable_read, &callable_write, &callable_destroy, &callable_gc);
	PHALCON_CALL_FUNCTION(NULL, "session_register_shutdown");
//#endif
	if (!zend_is_true(return_value)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_session_exception_ce, "Sets user-level session storage functions failed");
		RETURN_FALSE;
	}

	PHALCON_CALL_PARENT(return_value, phalcon_session_adapter_libmemcached_ce, getThis(), "start");
}

/**
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, open){

	RETURN_TRUE;
}

/**
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, close){

	RETURN_TRUE;
}

/**
 *
 * @param string $sessionId
 * @return mixed
 */
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, read){

	zval *sid, lifetime = {}, libmemcached = {};

	phalcon_fetch_params(0, 1, 0, &sid);

	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY);
	phalcon_read_property(&libmemcached, getThis(), SL("_libmemcached"), PH_NOISY);

	if (Z_TYPE(libmemcached) == IS_OBJECT) {
		PHALCON_CALL_METHOD(return_value, &libmemcached, "get", sid, &lifetime);
		if (Z_TYPE_P(return_value)!=IS_STRING) {
			RETURN_EMPTY_STRING();
		}
	} else {
		RETURN_EMPTY_STRING();
	}
}

/**
 *
 * @param string $sessionId
 * @param string $data
 */
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, write){

	zval *sid, *data, lifetime = {}, libmemcached = {};

	phalcon_fetch_params(0, 2, 0, &sid, &data);

	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY);
	phalcon_read_property(&libmemcached, getThis(), SL("_libmemcached"), PH_NOISY);

	if (Z_TYPE(libmemcached) == IS_OBJECT) {
		PHALCON_CALL_METHOD(return_value, &libmemcached, "save", sid, data, &lifetime);
	} else {
		RETURN_FALSE;
	}
}

/**
 *
 * @param string $session_id optional, session id
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, destroy){

	zval *_sid, sid, libmemcached = {};

	phalcon_fetch_params(0, 0, 1, &_sid);

	if (!_sid) {
		PHALCON_CALL_SELF(&sid, "getid");
	} else {
		PHALCON_CPY_WRT(&sid, _sid);
	}

	phalcon_read_property(&libmemcached, getThis(), SL("_libmemcached"), PH_NOISY);

	if (Z_TYPE(libmemcached) == IS_OBJECT) {
		PHALCON_RETURN_CALL_METHOD(&libmemcached, "delete", &sid);
	} else {
		RETURN_FALSE;
	}
}

/**
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter_Libmemcached, gc){

	RETURN_TRUE;
}
