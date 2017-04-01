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
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "registry.h"
#include "internal/arginfo.h"

#include <Zend/zend_interfaces.h>
#include <Zend/zend_smart_str.h>
#include <ext/spl/spl_array.h>
#include <ext/standard/php_var.h>

#include "kernel/main.h"
#include "kernel/hash.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"

/**
 * Phalcon\Registry
 *
 * A registry is a container for storing objects and values in the application space.
 * By storing the value in a registry, the same object is always available throughout
 * your application.
 *
 * <code>
 * 	$registry = new \Phalcon\Registry();
 *
 * 	// Set value
 * 	$registry->something = 'something';
 * 	// or
 * 	$registry['something'] = 'something';
 *
 * 	// Get value
 * 	$value = $registry->something;
 * 	// or
 * 	$value = $registry['something'];
 *
 * 	// Check if the key exists
 * 	$exists = isset($registry->something);
 * 	// or
 * 	$exists = isset($registry['something']);
 *
 * 	// Unset
 * 	unset($registry->something);
 * 	// or
 * 	unset($registry['something']);
 * </code>
 *
 * In addition to ArrayAccess, Phalcon\Registry also implements Countable
 * (count($registry) will return the number of elements in the registry),
 * Serializable and Iterator (you can iterate over the registry
 * using a foreach loop) interfaces. For PHP 5.4 and higher, JsonSerializable
 * interface is implemented.
 *
 * Phalcon\Registry is very fast (it is typically faster than any userspace
 * implementation of the registry); however, this comes at a price:
 * Phalcon\Registry is a final class and cannot be inherited from.
 *
 * Though Phalcon\Registry exposes methods like __get(), offsetGet(), count() etc,
 * it is not recommended to invoke them manually (these methods exist mainly to
 * match the interfaces the registry implements): $registry->__get('property')
 * is several times slower than $registry->property.
 *
 * Internally all the magic methods (and interfaces except JsonSerializable)
 * are implemented using object handlers or similar techniques: this allows
 * to bypass relatively slow method calls.
 */
zend_class_entry *phalcon_registry_ce;

PHP_METHOD(Phalcon_Registry, __construct);
PHP_METHOD(Phalcon_Registry, __get);
PHP_METHOD(Phalcon_Registry, __set);
PHP_METHOD(Phalcon_Registry, __isset);
PHP_METHOD(Phalcon_Registry, __unset);
PHP_METHOD(Phalcon_Registry, __call);
PHP_METHOD(Phalcon_Registry, count);
PHP_METHOD(Phalcon_Registry, offsetGet);
PHP_METHOD(Phalcon_Registry, offsetSet);
PHP_METHOD(Phalcon_Registry, offsetUnset);
PHP_METHOD(Phalcon_Registry, offsetExists);
PHP_METHOD(Phalcon_Registry, current);
PHP_METHOD(Phalcon_Registry, key);
PHP_METHOD(Phalcon_Registry, next);
PHP_METHOD(Phalcon_Registry, rewind);
PHP_METHOD(Phalcon_Registry, valid);
PHP_METHOD(Phalcon_Registry, jsonSerialize);
PHP_METHOD(Phalcon_Registry, serialize);
PHP_METHOD(Phalcon_Registry, unserialize);
PHP_METHOD(Phalcon_Registry, __wakeup);

const zend_function_entry phalcon_registry_method_entry[] = {
	PHP_ME(Phalcon_Registry, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Registry, __get, arginfo___getref, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, __set, arginfo___set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, __isset, arginfo___isset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, __unset, arginfo___unset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, __call, arginfo___call, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, count, arginfo_countable_count, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, offsetGet, arginfo_arrayaccess_offsetgetref, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, offsetSet, arginfo_arrayaccess_offsetset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, offsetUnset, arginfo_arrayaccess_offsetunset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, offsetExists, arginfo_arrayaccess_offsetexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, current, arginfo_iterator_current, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, key, arginfo_iterator_key, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, next, arginfo_iterator_next, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, rewind, arginfo_iterator_rewind, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, valid, arginfo_iterator_valid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, serialize, arginfo_serializable_serialize, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Registry, unserialize, arginfo_serializable_unserialize, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(Phalcon_Registry, __wakeup, arginfo_empty, ZEND_ACC_PRIVATE)
	PHP_FE_END
};

PHALCON_INIT_CLASS(Phalcon_Registry){
	PHALCON_REGISTER_CLASS(Phalcon, Registry, registry, phalcon_registry_method_entry, 0);

	zend_declare_property_null(phalcon_registry_ce, SL("_data"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_registry_ce, 4, zend_ce_arrayaccess, zend_ce_iterator, zend_ce_serializable, spl_ce_Countable);

	return SUCCESS;
}

/**
 * Phalcon\Di constructor
 *
 */
PHP_METHOD(Phalcon_Registry, __construct){

	phalcon_update_property_empty_array(getThis(), SL("_data"));
}

/**
 * Returns an index in the registry
 */
PHP_METHOD(Phalcon_Registry, __get){
	zval *property;

	phalcon_fetch_params(0, 1, 0, &property);

	PHALCON_RETURN_CALL_SELF("offsetget", property);
}

/**
 * Sets an element in the registry
 */
PHP_METHOD(Phalcon_Registry, __set){
	zval *property, *value;

	phalcon_fetch_params(0, 2, 0, &property, &value);

	PHALCON_CALL_SELF(NULL, "offsetset", property, value);
}

PHP_METHOD(Phalcon_Registry, __isset){
	zval *key;

	phalcon_fetch_params(0, 1, 0, &key);

	PHALCON_RETURN_CALL_SELF("offsetexists", key);
}

PHP_METHOD(Phalcon_Registry, __unset){
	zval *property;

	phalcon_fetch_params(0, 1, 0, &property);

	PHALCON_CALL_SELF(NULL, "offsetunset", property);
}

/**
 * @brief void Phalcon\Registry::__call(string $name, array $arguments)
 */
PHP_METHOD(Phalcon_Registry, __call){

	zval *name, *arguments, callback = {};

	phalcon_fetch_params(0, 2, 0, &name, &arguments);
	PHALCON_ENSURE_IS_STRING(name);

	if (phalcon_isset_property_array(getThis(), SL("_data"), name)) {
		phalcon_read_property_array(&callback, getThis(), SL("_data"), name, PH_READONLY);
		PHALCON_CALL_ZVAL_FUNCTION(return_value, &callback, arguments);
	} else {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Call to undefined method Phalcon\\Registry::%s", Z_STRVAL_P(name));
	}
}

/**
 * @brief int Phalcon\Registry::count()
 */
PHP_METHOD(Phalcon_Registry, count){

	zval data = {};

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);

	phalcon_fast_count(return_value, &data);
}

/**
 * @brief mixed& Phalcon\Registry::offsetGet(mixed $offset)
 */
PHP_METHOD(Phalcon_Registry, offsetGet){

	zval *property;

	phalcon_fetch_params(0, 1, 0, &property);

	phalcon_read_property_array(return_value, getThis(), SL("_data"), property, PH_COPY);
}

/**
 * @brief void Phalcon\Registry::offsetSet(mixed $offset, mixed $value)
 */
PHP_METHOD(Phalcon_Registry, offsetSet){

	zval *property, *callback;

	phalcon_fetch_params(0, 2, 0, &property, &callback);

	phalcon_update_property_array(getThis(), SL("_data"), property, callback);
}

/**
 * @brief void Phalcon\Registry::offsetUnset(mixed $offset)
 */
PHP_METHOD(Phalcon_Registry, offsetUnset){

	zval *property;

	phalcon_fetch_params(0, 1, 0, &property);

	phalcon_unset_property_array(getThis(), SL("_data"), property);
}

/**
 * @brief void Phalcon\Registry::offsetExists(mixed $offset)
 */
PHP_METHOD(Phalcon_Registry, offsetExists){

	zval *property;

	phalcon_fetch_params(0, 1, 0, &property);

	if (phalcon_isset_property_array(getThis(), SL("_data"), property)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * @brief mixed& Phalcon\Registry::current()
 */
PHP_METHOD(Phalcon_Registry, current){

	zval data = {}, *callback;

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	if ((callback = zend_hash_get_current_data(Z_ARRVAL(data))) != NULL) {
		RETURN_ZVAL(callback, 1, 0);
	}

	RETURN_NULL();
}

/**
 * @brief string|int|null Phalcon\Registry::key()
 */
PHP_METHOD(Phalcon_Registry, key){

	zval data = {};

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	zend_hash_get_current_key_zval(Z_ARRVAL(data), return_value);
}

/**
 * @brief void Phalcon\Registry::next()
 */
PHP_METHOD(Phalcon_Registry, next){

	zval data = {};

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	zend_hash_move_forward(Z_ARRVAL(data));
}

/**
 * @brief void Phalcon\Registry::rewind()
 */
PHP_METHOD(Phalcon_Registry, rewind){

	zval data = {};

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	zend_hash_internal_pointer_reset(Z_ARRVAL(data));
}

/**
 * @brief bool Phalcon\Registry::valid()
 */
PHP_METHOD(Phalcon_Registry, valid){

	zval data = {};

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);

	RETURN_BOOL(zend_hash_has_more_elements(Z_ARRVAL(data)));
}

/**
 * @brief string|null Phalcon\Registry::serialize()
 */
PHP_METHOD(Phalcon_Registry, serialize){

	zval data = {};

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);

	smart_str buf = { 0 };
	php_serialize_data_t var_hash;

	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(&buf, &data, &var_hash);
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.s) {
		RETURN_NEW_STR(buf.s);
	}

	RETURN_NULL();
}

/**
 * @brief Phalcon\Registry Phalcon\Registry::unserialize(string $str)
 */
PHP_METHOD(Phalcon_Registry, unserialize){
	zval data = {}, *str, zv = {};
	php_unserialize_data_t var_hash;
	const unsigned char *buf, *max;

	phalcon_fetch_params(0, 1, 0, &str);
	PHALCON_ENSURE_IS_STRING(str);

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);

	if (zend_hash_num_elements(Z_ARRVAL(data))) {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot call unserialize() on an already constructed object");
		return;
	}

	buf = (unsigned char*)(Z_STRVAL_P(str));
	max = buf + Z_STRLEN_P(str);

	PHP_VAR_UNSERIALIZE_INIT(var_hash);
	if (php_var_unserialize(&zv, &buf, max, &var_hash) && Z_TYPE(zv) == IS_ARRAY) {
		if (zend_hash_num_elements(Z_ARRVAL(zv)) != 0) {
			zend_hash_copy(Z_ARRVAL(data), Z_ARRVAL(zv), (copy_ctor_func_t) zval_add_ref);
		}
	} else {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Bad parameters passed to Phalcon\\Registry::unserialize()");
	}

	zval_dtor(&zv);
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
}

/**
 * @brief void Phalcon\Registry::__wakeup()
 */
PHP_METHOD(Phalcon_Registry, __wakeup)
{
}
