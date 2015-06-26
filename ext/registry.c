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
  +------------------------------------------------------------------------+
*/

#include "registry.h"
#include "internal/arginfo.h"

#include <Zend/zend_interfaces.h>
#include <ext/spl/spl_array.h>
#include <ext/standard/php_var.h>
#include <ext/standard/php_smart_str.h>

#include "kernel/main.h"
#include "kernel/hash.h"
#include "kernel/object.h"
#include "kernel/fcall.h"

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

static zend_object_handlers phalcon_registry_object_handlers;

/**
 * @brief Internal registry object
 */
typedef struct _phalcon_registry_object {
	zend_object obj;   /**< Zend Object */
	zval *properties;  /**< The registry itself */
	HashPosition pos;  /**< Current position used by the iterator */
} phalcon_registry_object;

/**
 * @brief Registry destructor
 * @param v Registry object
 * @see phalcon_registry_object
 */
static void phalcon_registry_dtor(void *v)
{
	phalcon_registry_object *obj = v;

	phalcon_ptr_dtor(&obj->properties);
	zend_object_std_dtor(&obj->obj);
	efree(obj);
}

/**
 * @brief Registry constructor
 * @details Allocates and initializes a Phalcon\Registry object
 * @brief ce Phalcon\Registry class entry
 * @return Zend Object Value corresponding to the newly constructed object
 */
static zend_object *phalcon_registry_ctor(zend_class_entry* ce)
{
	phalcon_registry_object *obj = ecalloc(1, sizeof(phalcon_registry_object));

	zend_object_std_init(&obj->obj, ce TSRMLS_CC);
	object_properties_init(&obj->obj, ce);

	MAKE_STD_ZVAL(obj->properties);
	array_init(obj->properties);
	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(obj->properties), &obj->pos);

	phalcon_registry_object_handlers.offset = XtOffsetof(phalcon_registry_object, obj);
    phalcon_registry_object_handlers.free_obj = phalcon_registry_dtor;

	return &obj->obj;
}

static PHP_FUNCTION(phalcon_registry_method_handler)
{
	Z_OBJ_HANDLER_P(getThis(), call_method)(((zend_internal_function*)EG(current_execute_data)->function_state.function)->function_name, INTERNAL_FUNCTION_PARAM_PASSTHRU);
	efree(((zend_internal_function*)EG(current_execute_data)->function_state.function));
}

static union _zend_function* phalcon_registry_get_method(zval **object_ptr, zend_string *method, const zval *key)
{
	zend_function *fbc;
	char *lc_method_name         = emalloc(method_len + 1);
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(*object_ptr, phalcon_registry_object);

	zend_str_tolower_copy(lc_method_name, method, method_len);

	if ((fbc = zend_hash_str_find_ptr(&obj->obj.ce->function_table, lc_method_name, method_len+1)) == NULL) {
		zend_internal_function *f = emalloc(sizeof(zend_internal_function));

		f->type          = ZEND_INTERNAL_FUNCTION;
		f->handler       = ZEND_FN(phalcon_registry_method_handler);
		f->arg_info      = NULL;
		f->num_args      = 0;
		f->scope         = obj->obj.ce;
		f->fn_flags      = ZEND_ACC_CALL_VIA_HANDLER;
		f->function_name = method;
		f->module        = obj->obj.ce->info.internal.module;

		efree(lc_method_name);
		return (union _zend_function*)f;
	}

	efree(lc_method_name);
	return fbc;
}

static int phalcon_registry_call_method(const char *method, INTERNAL_FUNCTION_PARAMETERS)
{
	zval ***args, *params, *callback;
	int argc, result;
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);

	if (!ZEND_NUM_ARGS()) {
		params = NULL;
	}
	else {
		int i;

		if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &argc)) {
			return FAILURE;
		}

		MAKE_STD_ZVAL(params);
		array_init_size(params, argc);

		for (i=0; i<argc; ++i) {
			Z_ADDREF_P(*args[i]);
			add_next_index_zval(params, *(args[i]));
		}
	}

	if ((callback = zend_hash_str_find(Z_ARRVAL_P(obj->properties), method, strlen(method)+1)) != NULL) {
		result = phalcon_call_user_func_array_noex(return_value, callback, params TSRMLS_CC);
	} else {
		result = FAILURE;
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Call to undefined method Phalcon\\Registry::%s", method);
	}

	if (params) {
		phalcon_ptr_dtor(&params);
		efree(args);
	}

	return result;
}

/**
 * @brief Create pointer to the property @a member of @a object, for future direct r/w access (<tt>&__get()</tt>)
 * @param object Object
 * @param member Property
 * @param type Access type
 * @param key Literal key
 * @return Pointer to @a member
 */
static zval** phalcon_registry_get_property_ptr_ptr(zval *object, zval *member, int type, void **cache_slot)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(object, phalcon_registry_object);

	return phalcon_hash_get(Z_ARRVAL_P(obj->properties), member, type);
}

/**
 * @brief Fetch property @a member from @a object, read-only (<tt>__get()</tt>)
 * @param object Object
 * @param member Property
 * @param type Access type
 * @param key Literal key
 * @return <tt>$object->$member</tt>
 */
static zval* phalcon_registry_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
	zval **result;
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(object, phalcon_registry_object);

	result = phalcon_hash_get(Z_ARRVAL_P(obj->properties), member, type);

	return result ? *result : NULL;
}

/**
 * @brief Set property @a member of @a object (<tt>__set()</tt>)
 * @param object Object
 * @param member Property
 * @param value Value
 * @param type Access type
 * @param key Literal key
 */
static void phalcon_registry_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(object, phalcon_registry_object);

	Z_ADDREF_P(value);

	phalcon_hash_update_or_insert(Z_ARRVAL_P(obj->properties), member, value);
}

/**
 * @brief Remove a property @a member of @a object (<tt>__unset()</tt>)
 * @param object Object
 * @param member Property
 * @param key Literal key
 */
static void phalcon_registry_unset_property(zval *object, zval *member, void **cache_slot)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(object, phalcon_registry_object);

	phalcon_hash_fast_unset(Z_ARRVAL_P(obj->properties), key);
}

/**
 * @brief Check if a property @a member of @a object exists
 * @param object Object
 * @param member Property
 * @param has_set_exists (0 (has): whether property exists and is not @c NULL; 1 (set): whether property exists and is @c true); 2 (exists): whether property exists
 * @param key Literal key
 * @return Whether the property exists
 */
static int phalcon_registry_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(object, phalcon_registry_object);
	zval **tmp;

	tmp = phalcon_hash_get(Z_ARRVAL_P(obj->properties), member, BP_VAR_NA);

	if (!tmp) {
		return 0;
	}

	if (0 == has_set_exists) {
		return Z_TYPE_P(*tmp) != IS_NULL;
	}

	if (1 == has_set_exists) {
		return zend_is_true(*tmp);
	}

	return 1;
}

/**
 * @brief Fetch dimension @a offset from @a object, read-only
 */
static zval* phalcon_registry_read_dimension(zval *object, zval *offset, int type, zval *rv)
{
	zval **ret;
	phalcon_registry_object *obj;

	if (UNEXPECTED(!offset)) {
		return EG(uninitialized_zval_ptr);
	}

	obj = PHALCON_GET_OBJECT_FROM_ZVAL(object, phalcon_registry_object);
	ret = phalcon_hash_get(Z_ARRVAL_P(obj->properties), offset, type);

	/* For write context we need to return a reference */
	if ((type == BP_VAR_W || type == BP_VAR_RW || type == BP_VAR_UNSET) && !Z_ISREF_P(*ret)) {
		if (Z_REFCOUNT_P(*ret) > 1) {
			zval *newval;

			MAKE_STD_ZVAL(newval);
			*newval = **ret;
			zval_copy_ctor(newval);
			Z_SET_REFCOUNT_P(newval, 1);

			Z_DELREF_P(*ret);
			*ret = newval;
		}

		Z_SET_ISREF_P(*ret);
	}

	return *ret;
}

/**
 * @brief Set dimension @a offset of @a object
 */
static void phalcon_registry_write_dimension(zval *object, zval *offset, zval *value)
{
	phalcon_registry_write_property(object, offset, value, NULL);
}

/**
 * @brief Check if a dimension @a offset of the @a object exists
 */
static int phalcon_registry_has_dimension(zval *object, zval *offset, int check_empty)
{
	return phalcon_registry_has_property(object, offset, check_empty, NULL);
}

static void phalcon_registry_unset_dimension(zval *object, zval *offset)
{
	phalcon_registry_unset_property(object, offset, NULL);
}

/**
 * @brief Updates @a count to hold the number of elements present and returns @c SUCCESS.
 */
static int phalcon_registry_count_elements(zval *object, long int *count TSRMLS_DC)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(object, phalcon_registry_object);
	*count = zend_hash_num_elements(Z_ARRVAL_P(obj->properties));
	return SUCCESS;
}

/**
 * @brief Get hash of the properties of @a object, as hash of zval's
 */
static HashTable* phalcon_registry_get_properties(zval *object TSRMLS_DC)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(object, phalcon_registry_object);
	return Z_ARRVAL_P(obj->properties);
}

static int phalcon_registry_compare_objects(zval *object1, zval *object2 TSRMLS_DC)
{
	phalcon_registry_object *zobj1, *zobj2;
	zval result;

	zobj1 = PHALCON_GET_OBJECT_FROM_ZVAL(object1, phalcon_registry_object);
	zobj2 = PHALCON_GET_OBJECT_FROM_ZVAL(object2, phalcon_registry_object);

	if (zobj1->obj.ce != zobj2->obj.ce) {
		return 1;
	}

	if (zobj1->properties == zobj2->properties) {
		return 0;
	}

	zend_compare_symbol_tables(&result, Z_ARRVAL_P(zobj1->properties), Z_ARRVAL_P(zobj2->properties) TSRMLS_CC);
	assert(Z_TYPE_P(&result) == IS_LONG);
	return Z_LVAL_P(&result);
}

/**
 * @brief <tt>Serializable::serialize()</tt>
 */
static int phalcon_registry_serialize(zval *object, unsigned char **buffer, uint32_t *buf_len, zend_serialize_data *data TSRMLS_DC)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(object, phalcon_registry_object);
	smart_str buf = { NULL, 0, 0 };
	php_serialize_data_t var_hash;

	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(&buf, &obj->properties, &var_hash TSRMLS_CC);
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.c) {
		*buffer  = (unsigned char*)buf.c;
		*buf_len = buf.len;
		return SUCCESS;
	}

	return FAILURE;
}

/**
 * @brief <tt>Serializable::unserialize()</tt>
 */
static int phalcon_registry_unserialize(zval **object, zend_class_entry *ce, const unsigned char *buf, uint32_t buf_len, zend_unserialize_data *data TSRMLS_DC)
{
	phalcon_registry_object *obj;
	php_unserialize_data_t var_hash;
	zval zv = zval_used_for_init, *pzv = &zv;
	const unsigned char *max;
	int retval;

	object_init_ex(*object, ce);
	obj = PHALCON_GET_OBJECT_FROM_ZVAL(*object, phalcon_registry_object);

	max = buf + buf_len;

	PHP_VAR_UNSERIALIZE_INIT(var_hash);
	retval = (php_var_unserialize(&pzv, &buf, max, &var_hash TSRMLS_CC) && Z_TYPE(zv) == IS_ARRAY) ? SUCCESS : FAILURE;
	if (SUCCESS == retval) {
		if (zend_hash_num_elements(Z_ARRVAL(zv)) != 0) {
			zend_hash_copy(Z_ARRVAL_P(obj->properties), Z_ARRVAL(zv), (copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval*));
		}
	}
	else {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Bad parameters passed to Phalcon\\Registry::unserialize()");
	}

	phalcon_dtor(pzv);
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);

	return retval;
}

/**
 * @brief Iterator destructor
 */
static void phalcon_registry_iterator_dtor(zend_object_iterator *it TSRMLS_DC)
{
	phalcon_ptr_dtor((zval**)&it->data);
	efree(it);
}

/**
 * @brief Checks whether the iterator @a it is valid (<tt>Iterator::valid()</tt>)
 */
static int phalcon_registry_iterator_valid(zend_object_iterator *it TSRMLS_DC)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL((zval*)it->data, phalcon_registry_object);
	return obj->pos != NULL ? SUCCESS : FAILURE;
}

/**
 * @brief Get current data from the iterator @a it into @ data (<tt>Iterator::current()</tt>)
 */
static void phalcon_registry_iterator_get_current_data(zend_object_iterator *it, zval ***data TSRMLS_DC)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL((zval*)it->data, phalcon_registry_object);

	if (FAILURE == zend_hash_get_current_data_ex(Z_ARRVAL_P(obj->properties), (void**)data, &obj->pos)) {
		*data = NULL;
	}
}

/**
 * @brief Get current key from the iterator @a it into @ key (<tt>Iterator::key()</tt>)
 */
static void phalcon_registry_iterator_get_current_key(zend_object_iterator *it, zval *key TSRMLS_DC)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL((zval*)it->data, phalcon_registry_object);

	zend_hash_get_current_key_zval_ex(Z_ARRVAL_P(obj->properties), key, &obj->pos);
}

/**
 * @brief Adavance the iterator @a it forward (<tt>Iterator::next()</tt>)
 */
static void phalcon_registry_iterator_move_forward(zend_object_iterator *it TSRMLS_DC)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL((zval*)it->data, phalcon_registry_object);
	zend_hash_move_forward_ex(Z_ARRVAL_P(obj->properties), &obj->pos);
}

/**
 * @brief Rewind the iterator @a it (<tt>Iterator::rewind()</tt>)
 */
static void phalcon_registry_iterator_rewind(zend_object_iterator *it TSRMLS_DC)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL((zval*)it->data, phalcon_registry_object);
	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(obj->properties), &obj->pos);
}

static zend_object_iterator_funcs phalcon_registry_iterator_funcs = {
	phalcon_registry_iterator_dtor,
	phalcon_registry_iterator_valid,
	phalcon_registry_iterator_get_current_data,
	phalcon_registry_iterator_get_current_key,
	phalcon_registry_iterator_move_forward,
	phalcon_registry_iterator_rewind,
	NULL
};

/**
 * @brief Iterator constructor
 */
static zend_object_iterator* phalcon_registry_get_iterator(zend_class_entry *ce, zval *object, int by_ref TSRMLS_DC)
{
	zend_object_iterator *result;

	result = emalloc(sizeof(zend_object_iterator));

	Z_ADDREF_P(object);
	result->data  = object;
	result->funcs = &phalcon_registry_iterator_funcs;
	return result;
}

/**
 * @brief mixed& Phalcon\Registry::__get(mixed $property)
 */
static PHP_METHOD(Phalcon_Registry, __get)
{
	zval **property, **result;
	phalcon_registry_object *obj;

	assert(return_value_ptr != NULL);

	phalcon_fetch_params(0, 1, 0, &property);

	obj    = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);
	result = phalcon_hash_get(Z_ARRVAL_P(obj->properties), *property, BP_VAR_W);

	phalcon_ptr_dtor(return_value_ptr);
	*return_value_ptr = *result;
	Z_ADDREF_P(*return_value_ptr);
	Z_SET_ISREF_P(*return_value_ptr);
}

/**
 * @brief void Phalcon\Registry::__set(mixed $property, mixed $value)
 */
static PHP_METHOD(Phalcon_Registry, __set)
{
	zval **property, **value;

	phalcon_fetch_params(0, 2, 0, &property, &value);
	phalcon_registry_write_property(getThis(), *property, *value, NULL);
}

/**
 * @brief bool Phalcon\Registry::__isset(mixed $property)
 */
static PHP_METHOD(Phalcon_Registry, __isset)
{
	zval **property;

	phalcon_fetch_params(0, 1, 0, &property);
	phalcon_registry_has_property(getThis(), *property, 0, NULL);
}

/**
 * @brief void Phalcon\Registry::__unset(mixed $property)
 */
static PHP_METHOD(Phalcon_Registry, __unset)
{
	zval **property;

	phalcon_fetch_params(0, 1, 0, &property);
	phalcon_registry_unset_property(getThis(), *property, NULL);
}

/**
 * @brief void Phalcon\Registry::__call(string $name, array $arguments)
 */
static PHP_METHOD(Phalcon_Registry, __call)
{
	zval **name, **arguments, *callback;
	phalcon_registry_object *obj;

	phalcon_fetch_params(0, 2, 0, &name, &arguments);
	PHALCON_ENSURE_IS_STRING(name);

	obj = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);

	if ((callback = zend_hash_str_find(Z_ARRVAL_P(obj->properties), Z_STRVAL_P(*name), Z_STRLEN_P(*name)+1)) != NULL) {
		RETURN_ON_FAILURE(phalcon_call_user_func_array(return_value, callback, *arguments TSRMLS_CC));
	}
	else {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Call to undefined method Phalcon\\Registry::%s", Z_STRVAL_P(*name));
	}
}

/**
 * @brief int Phalcon\Registry::count()
 */
static PHP_METHOD(Phalcon_Registry, count)
{
	long int result;

	phalcon_registry_count_elements(getThis(), &result TSRMLS_CC);
	RETURN_LONG(result);
}

/**
 * @brief mixed& Phalcon\Registry::offsetGet(mixed $offset)
 */
static PHP_METHOD(Phalcon_Registry, offsetGet)
{
	if (return_value_ptr) {
		ZEND_MN(Phalcon_Registry___get)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	}
	else {
		zval **offset, **result;
		phalcon_registry_object *obj;

		phalcon_fetch_params(0, 1, 0, &offset);

		obj    = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);
		result = phalcon_hash_get(Z_ARRVAL_P(obj->properties), *offset, BP_VAR_R);

		RETURN_ZVAL(*result, 1, 0);
	}
}

/**
 * @brief void Phalcon\Registry::offsetSet(mixed $offset, mixed $value)
 */
static PHP_METHOD(Phalcon_Registry, offsetSet)
{
	zval **offset, **value;

	phalcon_fetch_params(0, 2, 0, &offset, &value);
	phalcon_registry_write_dimension(getThis(), *offset, *value);
}

/**
 * @brief void Phalcon\Registry::offsetUnset(mixed $offset)
 */
static PHP_METHOD(Phalcon_Registry, offsetUnset)
{
	zval **offset;

	phalcon_fetch_params(0, 1, 0, &offset);
	phalcon_registry_unset_dimension(getThis(), *offset TSRMLS_CC);
}

/**
 * @brief void Phalcon\Registry::offsetExists(mixed $offset)
 */
static PHP_METHOD(Phalcon_Registry, offsetExists)
{
	zval **offset;

	phalcon_fetch_params(0, 1, 0, &offset);
	phalcon_registry_has_dimension(getThis(), *offset, 0 TSRMLS_CC);
}

/**
 * @brief mixed& Phalcon\Registry::current()
 */
static PHP_METHOD(Phalcon_Registry, current)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);
	zval **data;

	if (SUCCESS == zend_hash_get_current_data_ex(Z_ARRVAL_P(obj->properties), (void**)&data, &obj->pos)) {
		if (return_value_ptr) {
			phalcon_ptr_dtor(return_value_ptr);
			*return_value_ptr = *data;
			Z_ADDREF_P(*data);
			Z_SET_ISREF_P(*data);
			return;
		}

		RETURN_ZVAL(*data, 1, 0);
	}
}

/**
 * @brief string|int|null Phalcon\Registry::key()
 */
static PHP_METHOD(Phalcon_Registry, key)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);
	zval key   = phalcon_get_current_key_w(Z_ARRVAL_P(obj->properties), &obj->pos);
	zval *pkey = &key;
	RETURN_ZVAL(pkey, 1, 0);
}

/**
 * @brief void Phalcon\Registry::next()
 */
static PHP_METHOD(Phalcon_Registry, next)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);
	zend_hash_move_forward_ex(Z_ARRVAL_P(obj->properties), &obj->pos);
}

/**
 * @brief void Phalcon\Registry::rewind()
 */
static PHP_METHOD(Phalcon_Registry, rewind)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);
	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(obj->properties), &obj->pos);
}

/**
 * @brief bool Phalcon\Registry::valid()
 */
static PHP_METHOD(Phalcon_Registry, valid)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);
	RETURN_BOOL(obj->pos != NULL);
}

/**
 * @brief array Phalcon\Registry::jsonSerialize()
 */
static PHP_METHOD(Phalcon_Registry, jsonSerialize)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);
	RETURN_ZVAL(obj->properties, 1, 0);
}

/**
 * @brief string|null Phalcon\Registry::serialize()
 */
static PHP_METHOD(Phalcon_Registry, serialize)
{
	phalcon_registry_object *obj = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);
	smart_str buf = { NULL, 0, 0 };
	php_serialize_data_t var_hash;

	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(&buf, &obj->properties, &var_hash TSRMLS_CC);
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.c) {
		RETURN_STRINGL(buf.c, buf.len, 0);
	}
}

/**
 * @brief Phalcon\Registry Phalcon\Registry::unserialize(string $str)
 */
static PHP_METHOD(Phalcon_Registry, unserialize)
{
	zval **str;
	phalcon_registry_object *obj;
	php_unserialize_data_t var_hash;
	zval zv = zval_used_for_init, *pzv = &zv;
	const unsigned char *buf, *max;

	phalcon_fetch_params(0, 1, 0, &str);
	PHALCON_ENSURE_IS_STRING(str);

	obj = PHALCON_GET_OBJECT_FROM_ZVAL(getThis(), phalcon_registry_object);

	if (zend_hash_num_elements(Z_ARRVAL_P(obj->properties))) {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot call unserialize() on an already constructed object");
		return;
	}

	buf = (unsigned char*)(Z_STRVAL_P(*str));
	max = buf + Z_STRLEN_P(*str);

	PHP_VAR_UNSERIALIZE_INIT(var_hash);
	if (php_var_unserialize(&pzv, &buf, max, &var_hash TSRMLS_CC) && Z_TYPE(zv) == IS_ARRAY) {
		if (zend_hash_num_elements(Z_ARRVAL(zv)) != 0) {
			zend_hash_copy(Z_ARRVAL_P(obj->properties), Z_ARRVAL(zv), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval*));
		}
	}
	else {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Bad parameters passed to Phalcon\\Registry::unserialize()");
	}

	phalcon_dtor(pzv);
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
}

/**
 * @brief void Phalcon\Registry::__wakeup()
 */
static PHP_METHOD(Phalcon_Registry, __wakeup)
{
}

static const zend_function_entry phalcon_registry_method_entry[] = {
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
	PHP_ME(Phalcon_Registry, jsonSerialize, arginfo_jsonserializable_jsonserialize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Registry, serialize, arginfo_serializable_serialize, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Registry, unserialize, arginfo_serializable_unserialize, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(Phalcon_Registry, __wakeup, arginfo_empty, ZEND_ACC_PRIVATE)
	PHP_FE_END
};

PHALCON_INIT_CLASS(Phalcon_Registry)
{
	zend_class_entry **jsonserializable_ce = NULL;

	PHALCON_REGISTER_CLASS(Phalcon, Registry, registry, phalcon_registry_method_entry, ZEND_ACC_FINAL_CLASS);

	phalcon_registry_ce->create_object = phalcon_registry_ctor;
	phalcon_registry_ce->serialize     = phalcon_registry_serialize;
	phalcon_registry_ce->unserialize   = phalcon_registry_unserialize;

	phalcon_registry_object_handlers = *zend_get_std_object_handlers();
	phalcon_registry_object_handlers.get_property_ptr_ptr = phalcon_registry_get_property_ptr_ptr;
	phalcon_registry_object_handlers.read_property        = phalcon_registry_read_property;
	phalcon_registry_object_handlers.write_property       = phalcon_registry_write_property;
	phalcon_registry_object_handlers.unset_property       = phalcon_registry_unset_property;
	phalcon_registry_object_handlers.has_property         = phalcon_registry_has_property;
	phalcon_registry_object_handlers.read_dimension       = phalcon_registry_read_dimension;
	phalcon_registry_object_handlers.has_dimension        = phalcon_registry_has_dimension;
	phalcon_registry_object_handlers.write_dimension      = phalcon_registry_write_dimension;
	phalcon_registry_object_handlers.unset_dimension      = phalcon_registry_unset_dimension;
	phalcon_registry_object_handlers.get_properties       = phalcon_registry_get_properties;
	phalcon_registry_object_handlers.count_elements       = phalcon_registry_count_elements;
	phalcon_registry_object_handlers.compare_objects      = phalcon_registry_compare_objects;
	phalcon_registry_object_handlers.get_method           = phalcon_registry_get_method;
	phalcon_registry_object_handlers.call_method          = (zend_object_call_method_t)phalcon_registry_call_method;

	phalcon_registry_ce->get_iterator = phalcon_registry_get_iterator;

	jsonserializable_ce = zend_hash_str_find_ptr(CG(class_table), ZEND_STRS("jsonserializable"));

	zend_class_implements(
		phalcon_registry_ce TSRMLS_CC,
		(jsonserializable_ce ? 5 : 4),
		zend_ce_arrayaccess,
		zend_ce_iterator,
		zend_ce_serializable,
		spl_ce_Countable,
		(jsonserializable_ce ? *jsonserializable_ce : NULL) /* must be the last one */
	);

	return SUCCESS;
}
