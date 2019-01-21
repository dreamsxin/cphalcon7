
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

#include "session/bag.h"
#include "session/baginterface.h"
#include "session/exception.h"
#include "session/adapterinterface.h"
#include "di.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "internal/arginfo.h"

#include <ext/spl/spl_array.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/array.h"

#include "interned-strings.h"

/**
 * Phalcon\Session\Bag
 *
 * This component helps to separate session data into "namespaces". Working by this way
 * you can easily create groups of session variables into the application
 *
 *<code>
 *	$user = new \Phalcon\Session\Bag('user');
 *	$user->name = "Kimbra Johnson";
 *	$user->age = 22;
 *</code>
 */
zend_class_entry *phalcon_session_bag_ce;

PHP_METHOD(Phalcon_Session_Bag, __construct);
PHP_METHOD(Phalcon_Session_Bag, initialize);
PHP_METHOD(Phalcon_Session_Bag, destroy);
PHP_METHOD(Phalcon_Session_Bag, set);
PHP_METHOD(Phalcon_Session_Bag, get);
PHP_METHOD(Phalcon_Session_Bag, has);
PHP_METHOD(Phalcon_Session_Bag, __get);
PHP_METHOD(Phalcon_Session_Bag, remove);
PHP_METHOD(Phalcon_Session_Bag, getIterator);
PHP_METHOD(Phalcon_Session_Bag, count);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_session_bag___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_session_bag_method_entry[] = {
	PHP_ME(Phalcon_Session_Bag, __construct, arginfo_phalcon_session_bag___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Session_Bag, initialize, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Bag, destroy, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Bag, set, arginfo_phalcon_session_baginterface_set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Bag, get, arginfo_phalcon_session_baginterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Bag, has, arginfo_phalcon_session_baginterface_has, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Bag, remove, arginfo_phalcon_session_baginterface_remove, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Bag, getIterator, arginfo_iteratoraggregate_getiterator, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Bag, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Bag, __set, set, arginfo___set, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Bag, __isset, has, arginfo___isset, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Bag, __unset, remove, arginfo___unset, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Bag, offsetGet, __get, arginfo_arrayaccess_offsetget, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Bag, offsetSet, set, arginfo_arrayaccess_offsetset, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Bag, offsetExists, has, arginfo_arrayaccess_offsetexists, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Session_Bag, offsetUnset, remove, arginfo_arrayaccess_offsetunset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Session_Bag, count, arginfo_countable_count, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static int phalcon_session_bag_maybe_initialize(zval *this_ptr)
{
	zval initialized = {};

	phalcon_read_property(&initialized, this_ptr, SL("_initialized"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_FALSE(&initialized)) {
		return phalcon_call_method(NULL, this_ptr, "initialize", 0, NULL);
	}

	return SUCCESS;
}

static zend_object_iterator* phalcon_session_bag_get_iterator(zend_class_entry *ce, zval *object, int by_ref)
{
	zval iterator = {}, data = {}, *params[1];
	zend_object_iterator *ret;

	if (FAILURE == phalcon_session_bag_maybe_initialize(object)) {
		return NULL;
	}

	phalcon_read_property(&data, object, SL("_data"), PH_NOISY|PH_READONLY);

	object_init_ex(&iterator, spl_ce_ArrayIterator);
	params[0] = &data;
	if (FAILURE == phalcon_call_method(NULL, &iterator, "__construct", 1, params)) {
		ret = NULL;
	}
	else if (Z_TYPE(iterator) == IS_OBJECT) {
		ret = spl_ce_ArrayIterator->get_iterator(spl_ce_ArrayIterator, &iterator, by_ref);
	} else {
		ret = NULL;
	}
	zval_ptr_dtor(&iterator);

	return ret;
}

/**
 * Phalcon\Session\Bag initializer
 */
PHALCON_INIT_CLASS(Phalcon_Session_Bag){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Session, Bag, session_bag, phalcon_di_injectable_ce, phalcon_session_bag_method_entry, 0);

	zend_declare_property_null(phalcon_session_bag_ce, SL("_name"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_bag_ce, SL("_data"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_session_bag_ce, SL("_initialized"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_session_bag_ce, SL("_session"), ZEND_ACC_PROTECTED);

	phalcon_session_bag_ce->get_iterator = phalcon_session_bag_get_iterator;

	zend_class_implements(phalcon_session_bag_ce, 4, phalcon_session_baginterface_ce, zend_ce_aggregate, zend_ce_arrayaccess, spl_ce_Countable);

	return SUCCESS;
}

/**
 * Phalcon\Session\Bag constructor
 *
 * @param string $name
 */
PHP_METHOD(Phalcon_Session_Bag, __construct){

	zval *name;

	phalcon_fetch_params(0, 1, 0, &name);
	PHALCON_ENSURE_IS_STRING(name);
	phalcon_update_property(getThis(), SL("_name"), name);
}

/**
 * Initializes the session bag. This method must not be called directly, the class calls it when its internal data is accesed
 */
PHP_METHOD(Phalcon_Session_Bag, initialize){

	zval session = {}, dependency_injector = {}, service = {}, name = {}, data = {};

	PHALCON_MM_INIT();

	phalcon_read_property(&session, getThis(), SL("_session"), PH_READONLY);
	if (Z_TYPE(session) != IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);
		if (Z_TYPE(dependency_injector) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_session_exception_ce, "A dependency injection object is required to access the 'session' service");
			return;
		}

		ZVAL_STR(&service, IS(session));

		PHALCON_MM_CALL_METHOD(&session, &dependency_injector, "getshared", &service);
		PHALCON_MM_ADD_ENTRY(&session);
		PHALCON_MM_VERIFY_INTERFACE(&session, phalcon_session_adapterinterface_ce);
		phalcon_update_property(getThis(), SL("_session"), &session);
	}

	phalcon_read_property(&name, getThis(), SL("_name"), PH_READONLY);

	PHALCON_MM_CALL_METHOD(&data, &session, "__get", &name);
	PHALCON_MM_ADD_ENTRY(&data);

	if (Z_TYPE(data) != IS_ARRAY) {
		phalcon_update_property_empty_array(getThis(), SL("_data"));
	} else {
		phalcon_update_property(getThis(), SL("_data"), &data);
	}

	phalcon_update_property_bool(getThis(), SL("_initialized"), 1);
	RETURN_MM();
}

/**
 * Destroys the session bag
 *
 *<code>
 * $user->destroy();
 *</code>
 */
PHP_METHOD(Phalcon_Session_Bag, destroy){

	zval name = {}, session = {};

	RETURN_ON_FAILURE(phalcon_session_bag_maybe_initialize(getThis()));

	phalcon_read_property(&name, getThis(), SL("_name"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&session, getThis(), SL("_session"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &session, "__unset", &name);
}

/**
 * Sets a value in the session bag
 *
 *<code>
 * $user->set('name', 'Kimbra');
 *</code>
 *
 * @param string $property
 * @param string $value
 */
PHP_METHOD(Phalcon_Session_Bag, set){

	zval *property, *value, session = {}, name = {}, data = {};

	phalcon_fetch_params(0, 2, 0, &property, &value);

	RETURN_ON_FAILURE(phalcon_session_bag_maybe_initialize(getThis()));

	phalcon_update_property_array(getThis(), SL("_data"), property, value);

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&name, getThis(), SL("_name"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&session, getThis(), SL("_session"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &session, "__set", &name, &data);
}

/**
 * Magic setter to assign values to the session bag.
 * Alias for Phalcon\Session\Bag::set()
 *
 *<code>
 * $user->name = "Kimbra";
 *</code>
 *
 * @param string $property
 * @param string $value
 */
PHALCON_DOC_METHOD(Phalcon_Session_Bag, __set);

/**
 * Obtains a value from the session bag optionally setting a default value
 *
 *<code>
 * echo $user->get('name', 'Kimbra');
 *</code>
 *
 * @param string $property
 * @param string $defaultValue
 * @return mixed
 */
PHP_METHOD(Phalcon_Session_Bag, get){

	zval *property, *default_value = NULL, data = {}, value = {};

	phalcon_fetch_params(0, 1, 1, &property, &default_value);

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	/* Check first if the bag is initialized */
	RETURN_ON_FAILURE(phalcon_session_bag_maybe_initialize(getThis()));

	/* Retrieve the data */
	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset_fetch(&value, &data, property, PH_READONLY)) {
		RETURN_CTOR(&value);
	}

	RETURN_CTOR(default_value);
}

/**
 * Magic getter to obtain values from the session bag.
 *
 *<code>
 * echo $user->name;
 *</code>
 *
 * @param string $property
 * @return string
 */
PHP_METHOD(Phalcon_Session_Bag, __get)
{
	zval *property, data = {}, name = {}, session = {};

	phalcon_fetch_params(0, 1, 0, &property);

	/* Check first if the bag is initialized */
	RETURN_ON_FAILURE(phalcon_session_bag_maybe_initialize(getThis()));

	/* Retrieve the data */
	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);

	if (!phalcon_array_isset_fetch(return_value, &data, property, 0)) {
		ZVAL_NULL(return_value);
		ZVAL_MAKE_REF(return_value);

		phalcon_update_property_array(getThis(), SL("_data"), property, return_value);

		phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
		phalcon_read_property(&name, getThis(), SL("_name"), PH_NOISY|PH_READONLY);
		phalcon_read_property(&session, getThis(), SL("_session"), PH_NOISY|PH_READONLY);

		PHALCON_CALL_METHOD(NULL, &session, "__set", &name, &data);
	} else {
		ZVAL_MAKE_REF(return_value);
	}

}


/**
 * Check whether a property is defined in the internal bag
 *
 *<code>
 * var_dump($user->has('name'));
 *</code>
 *
 * @param string $property
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Bag, has){

	zval *property, data = {};

	phalcon_fetch_params(0, 1, 0, &property);

	RETURN_ON_FAILURE(phalcon_session_bag_maybe_initialize(getThis()));

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	RETURN_BOOL(phalcon_array_isset(&data, property));
}

/**
 * Magic isset to check whether a property is defined in the bag.
 * Alias for Phalcon\Session\Bag::has()
 *
 *<code>
 * var_dump(isset($user['name']));
 *</code>
 *
 * @param string $property
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Session_Bag, __isset);

/**
 * Removes a property from the internal bag
 *
 *<code>
 * $user->remove('name');
 *</code>
 *
 * @param string $property
 * @return boolean
 */
PHP_METHOD(Phalcon_Session_Bag, remove){

	zval *property, data = {}, name = {}, session = {};

	phalcon_fetch_params(0, 1, 0, &property);

	RETURN_ON_FAILURE(phalcon_session_bag_maybe_initialize(getThis()));

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset(&data, property)) {
		phalcon_unset_property_array(getThis(), SL("_data"), property);

		phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
		phalcon_read_property(&name, getThis(), SL("_name"), PH_NOISY|PH_READONLY);
		phalcon_read_property(&session, getThis(), SL("_session"), PH_NOISY|PH_READONLY);

		PHALCON_CALL_METHOD(NULL, &session, "__set", &name, &data);

		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Magic unset to remove items using the property syntax.
 * Alias for Phalcon\Session\Bag::remove()
 *
 *<code>
 * unset($user['name']);
 *</code>
 *
 * @param string $property
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Session_Bag, __unset);

PHP_METHOD(Phalcon_Session_Bag, getIterator)
{
	zval data = {};

	RETURN_ON_FAILURE(phalcon_session_bag_maybe_initialize(getThis()));

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	object_init_ex(return_value, spl_ce_ArrayIterator);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", &data);
}

PHP_METHOD(Phalcon_Session_Bag, count)
{
	zval data = {};
	long int count;

	RETURN_ON_FAILURE(phalcon_session_bag_maybe_initialize(getThis()));

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	count = (Z_TYPE(data) == IS_ARRAY) ? zend_hash_num_elements(Z_ARRVAL(data)) : 0;
	RETURN_LONG(count);
}
