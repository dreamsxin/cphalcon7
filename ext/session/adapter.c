
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

#include "session/adapter.h"
#include "session/adapterinterface.h"
#include "internal/arginfo.h"

#include <main/SAPI.h>
#include <ext/spl/spl_array.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/session.h"
#include "kernel/hash.h"

/**
 * Phalcon\Session\Adapter
 *
 * Base class for Phalcon\Session adapters
 */
zend_class_entry *phalcon_session_adapter_ce;

PHP_METHOD(Phalcon_Session_Adapter, __construct);
PHP_METHOD(Phalcon_Session_Adapter, __destruct);
PHP_METHOD(Phalcon_Session_Adapter, start);
PHP_METHOD(Phalcon_Session_Adapter, setOptions);
PHP_METHOD(Phalcon_Session_Adapter, getOptions);
PHP_METHOD(Phalcon_Session_Adapter, get);
PHP_METHOD(Phalcon_Session_Adapter, set);
PHP_METHOD(Phalcon_Session_Adapter, sets);
PHP_METHOD(Phalcon_Session_Adapter, has);
PHP_METHOD(Phalcon_Session_Adapter, remove);
PHP_METHOD(Phalcon_Session_Adapter, getId);
PHP_METHOD(Phalcon_Session_Adapter, isStarted);
PHP_METHOD(Phalcon_Session_Adapter, destroy);
PHP_METHOD(Phalcon_Session_Adapter, __get);
PHP_METHOD(Phalcon_Session_Adapter, count);
PHP_METHOD(Phalcon_Session_Adapter, getIterator);
PHP_METHOD(Phalcon_Session_Adapter, setId);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_session_adapter___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
	ZEND_ARG_INFO(0, expire)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, secure)
	ZEND_ARG_INFO(0, domain)
	ZEND_ARG_INFO(0, httpOnly)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_session_adapter_setid, 0, 0, 1)
	ZEND_ARG_INFO(0, sid)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_session_adapter_method_entry[] = {
	PHP_ME(Phalcon_Session_Adapter, __construct, arginfo_phalcon_session_adapter___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Session_Adapter, __destruct, arginfo___destruct, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(Phalcon_Session_Adapter, start, arginfo_phalcon_session_adapterinterface_start, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, setOptions, arginfo_phalcon_session_adapterinterface_setoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, getOptions, arginfo_phalcon_session_adapterinterface_getoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, get, arginfo_phalcon_session_adapterinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, set, arginfo_phalcon_session_adapterinterface_set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, sets, arginfo_phalcon_session_adapterinterface_sets, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, has, arginfo_phalcon_session_adapterinterface_has, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, remove, arginfo_phalcon_session_adapterinterface_remove, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, getId, arginfo_phalcon_session_adapterinterface_getid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, isStarted, arginfo_phalcon_session_adapterinterface_isstarted, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, destroy, arginfo_phalcon_session_adapterinterface_destroy, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Adapter, __set, set, arginfo___set, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Adapter, __isset, has, arginfo___isset, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Adapter, __unset, remove, arginfo___unset, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Adapter, offsetGet, __get, arginfo_arrayaccess_offsetget, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Adapter, offsetSet, set, arginfo_arrayaccess_offsetset, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Adapter, offsetExists, has, arginfo_arrayaccess_offsetexists, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Adapter, offsetUnset, remove, arginfo_arrayaccess_offsetunset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, count, arginfo_countable_count, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, getIterator, arginfo_iteratoraggregate_getiterator, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Adapter, setId, arginfo_phalcon_session_adapter_setid, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Session\Adapter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Session_Adapter){

	PHALCON_REGISTER_CLASS(Phalcon\\Session, Adapter, session_adapter, phalcon_session_adapter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_session_adapter_ce, SL("_uniqueId"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_session_adapter_ce, SL("_started"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_ce, SL("_expire"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_session_adapter_ce, SL("_path"), "/", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_ce, SL("_secure"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_ce, SL("_domain"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_session_adapter_ce, SL("_httpOnly"), 0, ZEND_ACC_PROTECTED);

	zend_class_implements(
		phalcon_session_adapter_ce, 4,
		phalcon_session_adapterinterface_ce,
		spl_ce_Countable,
		zend_ce_aggregate,
		zend_ce_arrayaccess
	);

	return SUCCESS;
}

/**
 * Phalcon\Session\Adapter constructor
 *
 * @param array $options
 * @param int $expire
 * @param string $path
 * @param boolean $secure
 * @param string $domain
 * @param boolean $httpOnly
 */
PHP_METHOD(Phalcon_Session_Adapter, __construct){

	zval *options = NULL, *expire = NULL, *path = NULL, *secure = NULL, *domain = NULL, *http_only = NULL;

	phalcon_fetch_params(0, 0, 6, &options, &expire, &path, &secure, &domain, &http_only);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		PHALCON_CALL_METHODW(NULL, getThis(), "setoptions", options);
	}

	if (expire || path || secure || domain || http_only || http_only) {
		if (!expire) {
			expire = phalcon_read_property(getThis(), SL("_expire"), PH_NOISY);
		}

		if (!path) {
			path = phalcon_read_property(getThis(), SL("_path"), PH_NOISY);
		}

		if (!secure) {
			secure = phalcon_read_property(getThis(), SL("_secure"), PH_NOISY);
		}

		if (!domain) {
			domain = phalcon_read_property(getThis(), SL("_domain"), PH_NOISY);
		}

		if (!http_only) {
			http_only = phalcon_read_property(getThis(), SL("_httpOnly"), PH_NOISY);
		}

		PHALCON_CALL_FUNCTIONW(NULL, "session_set_cookie_params", expire, path, secure, domain, http_only);
	}
}

PHP_METHOD(Phalcon_Session_Adapter, __destruct) {

	zval *started;

	started = phalcon_read_property(getThis(), SL("_started"), PH_NOISY);
	if (zend_is_true(started)) {
		RETURN_ON_FAILURE(phalcon_session_write_close());
		phalcon_update_property_bool(getThis(), SL("_started"), 0);
	}
}

/**
 * Starts the session (if headers are already sent the session will not be started)
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter, start){

	if (!SG(headers_sent)) {
		RETURN_ON_FAILURE(phalcon_session_start());
		phalcon_update_property_bool(getThis(), SL("_started"), 1);
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Sets session's options
 *
 *<code>
 *	$session->setOptions(array(
 *		'uniqueId' => 'my-private-app'
 *	));
 *</code>
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Session_Adapter, setOptions){

	zval *options, *unique_id;

	phalcon_fetch_params(0, 1, 0, &options);

	if (Z_TYPE_P(options) == IS_ARRAY) {
		if (phalcon_array_isset_string_fetch(&unique_id, options, SS("uniqueId"))) {
			phalcon_update_property_this(getThis(), SL("_uniqueId"), unique_id);
		}

		phalcon_update_property_this(getThis(), SL("_options"), options);
	}
}

/**
 * Get internal options
 *
 * @return array
 */
PHP_METHOD(Phalcon_Session_Adapter, getOptions){

	RETURN_MEMBER(getThis(), "_options");
}

/**
 * Gets a session variable from an application context
 *
 * @param string $index
 * @param mixed $defaultValue
 * @param bool $remove
 * @return mixed
 */
PHP_METHOD(Phalcon_Session_Adapter, get){

	zval *index, *default_value = NULL, *remove = NULL, *unique_id, *key, *_SESSION;
	zval *value;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 2, &index, &default_value, &remove);

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	unique_id = phalcon_read_property(getThis(), SL("_uniqueId"), PH_NOISY);

	PHALCON_INIT_VAR(key);
	PHALCON_CONCAT_VV(key, unique_id, index);

	_SESSION = phalcon_get_global(SS("_SESSION"));
	if (phalcon_array_isset_fetch(&value, _SESSION, key)) {
		RETVAL_ZVAL(value, 1, 0);
		if (remove && zend_is_true(remove)) {
			phalcon_array_unset(_SESSION, key, 0);
		}
	} else {
		RETVAL_ZVAL(default_value, 1, 0);
	}

	PHALCON_MM_RESTORE();
}

/**
 * Sets a session variable in an application context
 *
 *<code>
 *	$session->set('auth', 'yes');
 *</code>
 *
 * @param string $index
 * @param string $value
 */
PHP_METHOD(Phalcon_Session_Adapter, set){

	zval *index, *value, *unique_id, *key, *_SESSION;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 2, 0, &index, &value);
	unique_id = phalcon_read_property(getThis(), SL("_uniqueId"), PH_NOISY);

	PHALCON_INIT_VAR(key);
	PHALCON_CONCAT_VV(key, unique_id, index);

	_SESSION = phalcon_get_global(SS("_SESSION"));
	phalcon_array_update_zval(_SESSION, key, value, PH_COPY);

	PHALCON_MM_RESTORE();
}

/**
 * Sets a session variables in an application context
 *
 *<code>
 *	$session->sets(array('auth', 'yes'));
 *</code>
 *
 * @param array $data
 */
PHP_METHOD(Phalcon_Session_Adapter, sets){

	zval *data, *value = NULL;
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &data);

	if (Z_TYPE_P(data) == IS_ARRAY) { 
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, str_key, value) {
			zval index;
			if (str_key) {
				ZVAL_STR(&index, str_key);
			} else {
				ZVAL_LONG(&index, idx);
			}

			PHALCON_CALL_SELF(NULL, "set", &index, value);
		} ZEND_HASH_FOREACH_END();
	}

	PHALCON_MM_RESTORE();
}

/**
 * Check whether a session variable is set in an application context
 *
 *<code>
 *	var_dump($session->has('auth'));
 *</code>
 *
 * @param string $index
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter, has){

	zval *index, *unique_id, *key, *_SESSION;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &index);
	unique_id = phalcon_read_property(getThis(), SL("_uniqueId"), PH_NOISY);
	
	PHALCON_INIT_VAR(key);
	PHALCON_CONCAT_VV(key, unique_id, index);

	_SESSION = phalcon_get_global(SS("_SESSION"));
	if(phalcon_array_isset(_SESSION, key)) {
		RETURN_MM_TRUE;
	}

	RETURN_MM_FALSE;
}

/**
 * Removes a session variable from an application context
 *
 *<code>
 *	$session->remove('auth');
 *</code>
 *
 * @param string $index
 */
PHP_METHOD(Phalcon_Session_Adapter, remove){

	zval *index, *unique_id, *key, *_SESSION;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &index);
	unique_id = phalcon_read_property(getThis(), SL("_uniqueId"), PH_NOISY);
	
	PHALCON_INIT_VAR(key);
	PHALCON_CONCAT_VV(key, unique_id, index);

	_SESSION = phalcon_get_global(SS("_SESSION"));
	phalcon_array_unset(_SESSION, key, 0);

	RETURN_MM();
}

/**
 * Returns active session id
 *
 *<code>
 *	echo $session->getId();
 *</code>
 *
 * @return string
 */
PHP_METHOD(Phalcon_Session_Adapter, getId){

	RETURN_ON_FAILURE(phalcon_get_session_id(return_value));
}

/**
 * Check whether the session has been started
 *
 *<code>
 *	var_dump($session->isStarted());
 *</code>
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter, isStarted){


	RETURN_MEMBER(getThis(), "_started");
}

/**
 * Destroys the active session
 *
 *<code>
 *	var_dump($session->destroy());
 *</code>
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Adapter, destroy){

	phalcon_update_property_bool(getThis(), SL("_started"), 0);
	RETURN_ON_FAILURE(phalcon_session_destroy());
	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Session_Adapter, __get)
{
	zval *property;

	phalcon_fetch_params(0, 1, 0, &property);
	
	PHALCON_RETURN_CALL_SELFW("get", property);
}

PHP_METHOD(Phalcon_Session_Adapter, count)
{
	zval *_SESSION = phalcon_get_global(SS("_SESSION"));

	RETURN_LONG(phalcon_fast_count_int(_SESSION));
}

PHP_METHOD(Phalcon_Session_Adapter, getIterator)
{
	zval *_SESSION;

	_SESSION = phalcon_get_global(SS("_SESSION"));
	object_init_ex(return_value, spl_ce_ArrayIterator);
	PHALCON_CALL_METHODW(NULL, return_value, "__construct", _SESSION);
}

/**
 * Set the current session id
 *
 *<code>
 *	$session->setId($id);
 *</code>
 */
PHP_METHOD(Phalcon_Session_Adapter, setId){

	zval *sid;

	phalcon_fetch_params(0, 1, 0, &sid);

	RETURN_ON_FAILURE(phalcon_set_session_id(sid));
}
