
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

static zend_object_handlers phalcon_session_adapter_object_handlers;

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

static zval* phalcon_session_adapter_get_property_ptr_ptr_internal(zval *object, zval *member, int type)
{
	zval *unique_id, *_SESSION, key = zval_used_for_init, *pkey = &key;
	zval *value;

	unique_id = phalcon_fetch_nproperty_this(object, SL("_uniqueId"), PH_NOISY);

	_SESSION = phalcon_get_global(SS("_SESSION"));
	if (Z_TYPE_P(_SESSION) != IS_ARRAY) {
		if (type == BP_VAR_R || type == BP_VAR_RW) {
			php_error_docref(NULL, E_WARNING, "Session is not started or $_SESSION is invalid");
		}
		return (type == BP_VAR_W || type == BP_VAR_RW) ? &EG(error_zval_ptr) : &EG(uninitialized_zval_ptr);
	}

	phalcon_concat_vv(&pkey, unique_id, member, 0);
	value = phalcon_hash_get(Z_ARRVAL_P(_SESSION), pkey, type);
	zval_dtor(&key);

	return value;
}

static int phalcon_session_adapter_has_property_internal(zval *object, zval *member, int has_set_exists)
{
	zval *unique_id, *_SESSION, *tmp;
	zval key = zval_used_for_init, *pkey = &key;

	unique_id = phalcon_fetch_nproperty_this(object, SL("_uniqueId"), PH_NOISY);

	_SESSION = phalcon_get_global(SS("_SESSION"));
	if (Z_TYPE_P(_SESSION) != IS_ARRAY) {
		return 0;
	}

	phalcon_concat_vv(&pkey, unique_id, member, 0);
	tmp = phalcon_hash_get(Z_ARRVAL_P(_SESSION), pkey, BP_VAR_NA);
	zval_dtor(&key);

	if (!tmp) {
		return 0;
	}

	if (0 == has_set_exists) {
		return Z_TYPE_P(tmp) != IS_NULL;
	}

	if (1 == has_set_exists) {
		return zend_is_true(tmp);
	}

	return 1;
}

static void phalcon_session_adapter_write_property_internal(zval *object, zval *member, zval *value)
{
	zval *unique_id, *_SESSION;
	zval key = zval_used_for_init, *pkey = &key;

	unique_id = phalcon_fetch_nproperty_this(object, SL("_uniqueId"), PH_NOISY);

	_SESSION = phalcon_get_global(SS("_SESSION"));
	if (Z_TYPE_P(_SESSION) == IS_ARRAY) {
		phalcon_concat_vv(&pkey, unique_id, member, 0);
		Z_ADDREF_P(value);
		phalcon_hash_update_or_insert(Z_ARRVAL_P(_SESSION), pkey, value);
		zval_dtor(&key);
	}
}

static void phalcon_session_adapter_unset_property_internal(zval *object, zval *member)
{
	zval *unique_id, *_SESSION;
	zval key = zval_used_for_init, *pkey = &key;

	unique_id = phalcon_fetch_nproperty_this(object, SL("_uniqueId"), PH_NOISY);

	_SESSION = phalcon_get_global(SS("_SESSION"));
	if (Z_TYPE_P(_SESSION) == IS_ARRAY) {
		phalcon_concat_vv(&pkey, unique_id, member, 0);
		phalcon_hash_unset(Z_ARRVAL_P(_SESSION), pkey);
		zval_dtor(&key);
	}
}

static zval* phalcon_session_adapter_get_property_ptr_ptr(zval *object, zval *member, int type, void **cache_slot)
{
	if (!is_phalcon_class(Z_OBJCE_P(object))) {
		return zend_get_std_object_handlers()->get_property_ptr_ptr(object, member, type, cache_slot);
	}

	return phalcon_session_adapter_get_property_ptr_ptr_internal(object, member, type);
}

static int phalcon_session_adapter_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
	if (!is_phalcon_class(Z_OBJCE_P(object))) {
		return zend_get_std_object_handlers()->has_property(object, member, has_set_exists, cache_slot);
	}

	return phalcon_session_adapter_has_property_internal(object, member, has_set_exists);
}

static void phalcon_session_adapter_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
	if (!is_phalcon_class(Z_OBJCE_P(object)) || phalcon_isset_property_zval(object, member)) {
		zend_get_std_object_handlers()->write_property(object, member, value, cache_slot);
	}
	else {
		phalcon_session_adapter_write_property_internal(object, member, value);
	}
}

static void phalcon_session_adapter_unset_property(zval *object, zval *member, void **cache_slot)
{
	if (!is_phalcon_class(Z_OBJCE_P(object))) {
		zend_get_std_object_handlers()->unset_property(object, member, cache_slot);
	}
	else {
		phalcon_session_adapter_unset_property_internal(object, member);
	}
}

static zval* phalcon_session_adapter_read_dimension(zval *object, zval *offset, int type, zval *rv)
{
	zval *ret;

	if (!is_phalcon_class(Z_OBJCE_P(object))) {
		return zend_get_std_object_handlers()->read_dimension(object, offset, type, rv);
	}

	if (UNEXPECTED(!offset)) {
		return EG(uninitialized_zval_ptr);
	}

	ret = phalcon_session_adapter_get_property_ptr_ptr_internal(object, offset, type);

	/* For write context we need to return a reference */
	if ((type == BP_VAR_W || type == BP_VAR_RW || type == BP_VAR_UNSET) && !Z_ISREF_P(*ret)) {
		if (Z_REFCOUNT_P(ret) > 1) {
			zval *newval;

			PHALCON_ALLOC_GHOST_ZVAL(newval);
			*newval = *ret;
			zval_copy_ctor(newval);
			Z_SET_REFCOUNT_P(newval, 1);

			Z_DELREF_P(ret);
			ret = newval;
		}

		Z_SET_ISREF_P(*ret);
	}

	return ret;
}

static void phalcon_session_adapter_write_dimension(zval *object, zval *offset, zval *value)
{
	if (!is_phalcon_class(Z_OBJCE_P(object))) {
		zend_get_std_object_handlers()->write_dimension(object, offset, value);
		return;
	}

	if (!offset) {
		offset = PHALCON_GLOBAL(z_null);
	}

	phalcon_session_adapter_write_property_internal(object, offset, value);
}

static int phalcon_session_adapter_has_dimension(zval *object, zval *member, int check_empty)
{
	zval *tmp;

	if (!is_phalcon_class(Z_OBJCE_P(object))) {
		return zend_get_std_object_handlers()->has_dimension(object, member, check_empty);
	}

	tmp = phalcon_session_adapter_get_property_ptr_ptr_internal(object, member, BP_VAR_NA);

	if (!tmp) {
		return 0;
	}

	if (0 == check_empty) {
		return Z_TYPE_P(tmp) != IS_NULL;
	}

	if (1 == check_empty) {
		return zend_is_true(tmp);
	}

	return 1;
}

static void phalcon_session_adapter_unset_dimension(zval *object, zval *offset)
{
	if (!is_phalcon_class(Z_OBJCE_P(object))) {
		zend_get_std_object_handlers()->unset_dimension(object, offset);
		return;
	}

	phalcon_session_adapter_unset_property_internal(object, offset);
}

static int phalcon_session_adapter_count_elements(zval *object, long *count)
{
	int res;
	zval *cnt = NULL;

	if (is_phalcon_class(Z_OBJCE_P(object))) {
		zval *_SESSION = phalcon_get_global(SS("_SESSION"));
		if (Z_TYPE_P(_SESSION) == IS_ARRAY) {
			*count = zend_hash_num_elements(Z_ARRVAL_P(_SESSION));
			return SUCCESS;
		}

		return FAILURE;
	}

	res = phalcon_call_method(&cnt, object, "count", 0, NULL);
	if (res == SUCCESS) {
		*count = (Z_TYPE_P(cnt) == IS_LONG) ? Z_LVAL_P(cnt) : phalcon_get_intval(cnt);
		zval_ptr_dtor(&cnt);
	}

	return res;
}

static zend_object *phalcon_session_adapter_object_ctor(zend_class_entry *ce)
{
	zend_object *obj = emalloc(sizeof(zend_object));
	zend_object_value retval;

	zend_object_std_init(obj, ce);
	object_properties_init(obj, ce);

	phalcon_session_adapter_object_handlers.offset = 0;
    phalcon_session_adapter_object_handlers.free_obj = zend_objects_free_object_storage;

	return obj;
}

static zend_object_iterator* phalcon_session_adapter_get_iterator(zend_class_entry *ce, zval *object, int by_ref)
{
	zval *iterator;
	zval *data;
	zval *params[1];
	zend_object_iterator *ret;

	data = phalcon_get_global(SS("_SESSION"));
	if (Z_TYPE_P(data) != IS_ARRAY) {
		return NULL;
	}

	PHALCON_ALLOC_GHOST_ZVAL(iterator);
	object_init_ex(iterator, spl_ce_ArrayIterator);
	params[0] = data;
	if (FAILURE == phalcon_call_method(NULL, iterator, "__construct", 1, params)) {
		ret = NULL;
	}
	else if (Z_TYPE_P(iterator) == IS_OBJECT) {
		ret = spl_ce_ArrayIterator->get_iterator(spl_ce_ArrayIterator, iterator, by_ref);
	}
	else {
		ret = NULL;
	}

	zval_ptr_dtor(&iterator);
	return ret;
}

/**
 * Phalcon\Session\Adapter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Session_Adapter){

	PHALCON_REGISTER_CLASS(Phalcon\\Session, Adapter, session_adapter, phalcon_session_adapter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	phalcon_session_adapter_ce->create_object = phalcon_session_adapter_object_ctor;

	zend_declare_property_null(phalcon_session_adapter_ce, SL("_uniqueId"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_session_adapter_ce, SL("_started"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_ce, SL("_expire"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_session_adapter_ce, SL("_path"), "/", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_ce, SL("_secure"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_adapter_ce, SL("_domain"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_session_adapter_ce, SL("_httpOnly"), 0, ZEND_ACC_PROTECTED);

	/**
	 * T2414 - niden - Removed if statement for nuSphere Debugger
	   if (!nusphere_dbg_present) {
	 */
	phalcon_session_adapter_object_handlers = *zend_get_std_object_handlers();
	phalcon_session_adapter_object_handlers.get_property_ptr_ptr = phalcon_session_adapter_get_property_ptr_ptr;
	phalcon_session_adapter_object_handlers.has_property         = phalcon_session_adapter_has_property;
	phalcon_session_adapter_object_handlers.write_property       = phalcon_session_adapter_write_property;
	phalcon_session_adapter_object_handlers.unset_property       = phalcon_session_adapter_unset_property;
	phalcon_session_adapter_object_handlers.count_elements       = phalcon_session_adapter_count_elements;
	phalcon_session_adapter_object_handlers.read_dimension       = phalcon_session_adapter_read_dimension;
	phalcon_session_adapter_object_handlers.write_dimension      = phalcon_session_adapter_write_dimension;
	phalcon_session_adapter_object_handlers.has_dimension        = phalcon_session_adapter_has_dimension;
	phalcon_session_adapter_object_handlers.unset_dimension      = phalcon_session_adapter_unset_dimension;
	/**
	 * T2414 - niden - Removed if statement for nuSphere Debugger
       }
	 */

	phalcon_session_adapter_ce->get_iterator = phalcon_session_adapter_get_iterator;

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

	phalcon_fetch_params(0, 0, 0, 6, &options, &expire, &path, &secure, &domain, &http_only);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		PHALCON_CALL_METHODW(NULL, this_ptr, "setoptions", options);
	}

	if (expire || path || secure || domain || http_only || http_only) {
		if (!expire) {
			expire = phalcon_fetch_nproperty_this(getThis(), SL("_expire"), PH_NOISY);
		}

		if (!path) {
			path = phalcon_fetch_nproperty_this(getThis(), SL("_path"), PH_NOISY);
		}

		if (!secure) {
			secure = phalcon_fetch_nproperty_this(getThis(), SL("_secure"), PH_NOISY);
		}

		if (!domain) {
			domain = phalcon_fetch_nproperty_this(getThis(), SL("_domain"), PH_NOISY);
		}

		if (!http_only) {
			http_only = phalcon_fetch_nproperty_this(getThis(), SL("_httpOnly"), PH_NOISY);
		}

		PHALCON_CALL_FUNCTIONW(NULL, "session_set_cookie_params", expire, path, secure, domain, http_only);
	}
}

PHP_METHOD(Phalcon_Session_Adapter, __destruct) {

	zval *started;

	started = phalcon_fetch_nproperty_this(getThis(), SL("_started"), PH_NOISY);
	if (zend_is_true(started)) {
		RETURN_ON_FAILURE(phalcon_session_write_close());
		phalcon_update_property_bool(this_ptr, SL("_started"), 0);
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
		phalcon_update_property_bool(this_ptr, SL("_started"), 1);
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

	phalcon_fetch_params(0, 0, 1, 0, &options);

	if (Z_TYPE_P(options) == IS_ARRAY) {
		if (phalcon_array_isset_string_fetch(&unique_id, options, SS("uniqueId"))) {
			phalcon_update_property_this(this_ptr, SL("_uniqueId"), unique_id);
		}

		phalcon_update_property_this(this_ptr, SL("_options"), options);
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

	phalcon_fetch_params(0, 0, 1, 2, &index, &default_value, &remove);
	if (!default_value) {
		default_value = PHALCON_GLOBAL(z_null);
	}

	if (!remove || !zend_is_true(remove)) {
		/* Fast path */
		zval *value = phalcon_session_adapter_get_property_ptr_ptr_internal(getThis(), index, BP_VAR_NA);
		if (value) {
			RETURN_ZVAL(value, 1, 0);
		}

		RETURN_ZVAL(default_value, 1, 0);
	}

	unique_id = phalcon_fetch_nproperty_this(this_ptr, SL("_uniqueId"), PH_NOISY);

	PHALCON_MM_GROW();
	PHALCON_INIT_VAR(key);
	PHALCON_CONCAT_VV(key, unique_id, index);

	_SESSION = phalcon_get_global(SS("_SESSION"));
	if (phalcon_array_isset_fetch(&value, _SESSION, key)) {
		RETVAL_ZVAL(value, 1, 0);
		phalcon_array_unset(&_SESSION, key, 0);
	}
	else {
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

	zval *index, *value;

	phalcon_fetch_params(0, 0, 2, 0, &index, &value);
	phalcon_session_adapter_write_property_internal(getThis(), index, value);
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

	zval *data, *index = NULL, *value = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	phalcon_fetch_params(0, 1, 1, 0, &data);

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

	zval *index;

	phalcon_fetch_params(0, 0, 1, 0, &index);
	RETURN_BOOL(phalcon_session_adapter_has_property_internal(getThis(), index, 2));
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

	zval *index;

	phalcon_fetch_params(0, 0, 1, 0, &index);
	phalcon_session_adapter_unset_property_internal(getThis(), index);
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

	RETURN_ON_FAILURE(phalcon_get_session_id(return_value, return_value_ptr));
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


	RETURN_MEMBER(this_ptr, "_started");
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

	phalcon_update_property_bool(this_ptr, SL("_started"), 0);
	RETURN_ON_FAILURE(phalcon_session_destroy());
	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Session_Adapter, __get)
{
	zval **property, *retval;

	assert(return_value_ptr != NULL);

	phalcon_fetch_params(0, 1, 0, &property);
	retval = phalcon_session_adapter_get_property_ptr_ptr_internal(getThis(), *property, BP_VAR_W);

	zval_ptr_dtor(return_value_ptr);
	*return_value_ptr = retval;
	Z_ADDREF_P(*return_value_ptr);
	Z_SET_ISREF_P(*return_value_ptr);
}

PHP_METHOD(Phalcon_Session_Adapter, count)
{
	long int count;

	if (SUCCESS == phalcon_session_adapter_count_elements(getThis(), &count)) {
		RETURN_LONG(count);
	}

	RETURN_NULL();
}

PHP_METHOD(Phalcon_Session_Adapter, getIterator)
{
	zval *data;

	data = phalcon_get_global(SS("_SESSION"));
	object_init_ex(return_value, spl_ce_ArrayIterator);
	PHALCON_CALL_METHODW(NULL, return_value, "__construct", data);
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

	phalcon_fetch_params(0, 0, 1, 0, &sid);

	RETURN_ON_FAILURE(phalcon_set_session_id(sid));
}
