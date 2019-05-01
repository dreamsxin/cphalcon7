
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

#include "kernel/object.h"

#include "kernel/../exception.h"
#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/hash.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/string.h"

#include <Zend/zend_closures.h>

/**
 * Reads class constant from string name and returns its value
 */
int phalcon_get_class_constant(zval *return_value, const zend_class_entry *ce, const char *constant_name, uint32_t constant_length)
{
#if PHP_VERSION_ID >= 70100
	zend_class_constant *cc;

	if ((cc = zend_hash_str_find_ptr(&ce->constants_table, constant_name, constant_length)) == NULL) {
		php_error_docref(NULL, E_ERROR, "Undefined class constant '%s::%s'", ce->name->val, constant_name);
		return FAILURE;
	}

	ZVAL_COPY_VALUE(return_value, &cc->value);
#else
	zval *result;

	if ((result = zend_hash_str_find(&ce->constants_table, constant_name, constant_length)) == NULL) {
		php_error_docref(NULL, E_ERROR, "Undefined class constant '%s::%s'", ce->name->val, constant_name);
		return FAILURE;
	}

	ZVAL_COPY_VALUE(return_value, result);
#endif
	return SUCCESS;
}

/**
 * Query a static property value from a zend_class_entry
 */
int phalcon_read_static_property(zval *return_value, const char *class_name, uint32_t class_length, const char *property_name, uint32_t property_length, int flags)
{
	zend_class_entry *ce;

	if ((ce = zend_lookup_class(zend_string_init(class_name, class_length, 0))) != NULL) {
		return phalcon_read_static_property_ce(return_value, ce, property_name, property_length, flags);
	}

	ZVAL_NULL(return_value);
	return 0;
}

int phalcon_read_static_property_ce(zval *return_value, zend_class_entry *ce, const char *property, uint32_t len, int flags)
{
	zval *value = zend_read_static_property(ce, property, len, (zend_bool)ZEND_FETCH_CLASS_SILENT);
	if (value) {
		if (EXPECTED(Z_TYPE_P(value) == IS_REFERENCE)) {
			value = Z_REFVAL_P(value);
		}
		if ((flags & PH_SEPARATE) == PH_SEPARATE) {
			ZVAL_DUP(return_value, value);
		} else if ((flags & PH_READONLY) == PH_READONLY) {
			ZVAL_COPY_VALUE(return_value, value);
		} else {
			ZVAL_COPY(return_value, value);
		}
		return 1;
	}
	ZVAL_NULL(return_value);
	return 0;
}

int phalcon_read_static_property_array_ce(zval *return_value, zend_class_entry *ce, const char *property, uint32_t property_length, const zval *index, int flags)
{
	zval arr = {};

	phalcon_read_static_property_ce(&arr, ce, property, property_length, PH_READONLY);

	if (Z_TYPE(arr) != IS_ARRAY || !phalcon_array_isset_fetch(return_value, &arr, index, flags)) {
		ZVAL_NULL(return_value);
		return 0;
	}

	return 1;
}

int phalcon_update_static_property_array_ce(zend_class_entry *ce, const char *property, uint32_t property_length, const zval *index, zval *value)
{
	zval tmp = {};

	phalcon_read_static_property_ce(&tmp, ce, property, property_length, PH_READONLY);

	if (Z_TYPE(tmp) != IS_ARRAY) {
		array_init(&tmp);
		phalcon_array_update(&tmp, index,  value, PH_COPY);
		phalcon_update_static_property_ce(ce, property, property_length, &tmp);
		zval_ptr_dtor(&tmp);
		return SUCCESS;
	}

	phalcon_array_update(&tmp, index,  value, PH_COPY);

	return SUCCESS;
}

/*
 * Multiple array-offset update
 */
int phalcon_update_static_property_array_multi_ce(zend_class_entry *ce, const char *property, uint32_t property_length, zval *value, const char *types, int types_length, int types_count, ...)
{
	zval tmp = {};
	va_list ap;
	int separated = 0, flag = SUCCESS;

	phalcon_read_static_property_ce(&tmp, ce, property, property_length, PH_READONLY);

	/** Convert the value to array if not is an array */
	if (Z_TYPE(tmp) != IS_ARRAY) {
		array_init(&tmp);
		separated = 1;
	} else if (Z_REFCOUNTED(tmp)) {
		if (Z_REFCOUNT(tmp) > 1) {
			if (!Z_ISREF(tmp)) {
				zval new_zv;
				ZVAL_DUP(&new_zv, &tmp);
				ZVAL_COPY_VALUE(&tmp, &new_zv);
				separated = 1;
			}
		}
	}

	va_start(ap, types_count);
	phalcon_array_update_multi_ex(&tmp, value, types, types_length, types_count, ap);
	va_end(ap);

	if (separated) {
		flag = phalcon_update_static_property_ce(ce, property, property_length, &tmp);
		zval_ptr_dtor(&tmp);
	}
	return flag;
}

int phalcon_update_static_property_ce(zend_class_entry *ce, const char *property_name, uint32_t property_length, zval *value)
{
	zval garbage;
	zval *property;
	zend_class_entry *old_scope;
	zend_string *key = zend_string_init(property_name, property_length, 0);

#if PHP_VERSION_ID >= 70100
	old_scope = EG(fake_scope);
	EG(fake_scope) = ce;
#else
	old_scope = EG(scope);
	EG(scope) = ce;
#endif
	property = zend_std_get_static_property(ce, key, 0);
#if PHP_VERSION_ID >= 70100
	EG(fake_scope) = old_scope;
#else
	EG(scope) = old_scope;
#endif
	zend_string_free(key);

	if (!property) {
		return FAILURE;
	} else {
		ZVAL_COPY_VALUE(&garbage, property);
		if (Z_ISREF_P(value)) {
			SEPARATE_ZVAL(value);
		}
		ZVAL_COPY(property, value);
		zval_ptr_dtor(&garbage);
		return SUCCESS;
	}
	//return zend_update_static_property(ce, property_name, property_length, value);
}

int phalcon_update_static_property_bool_ce(zend_class_entry *ce, const char *property_name, uint32_t property_length, int value)
{
	zval v = {};
	ZVAL_BOOL(&v, value);
	return phalcon_update_static_property_ce(ce, property_name, property_length, &v);
}

int phalcon_update_static_property_empty_array_ce(zend_class_entry *ce, const char *name, uint32_t len)
{
	zval empty_array = {};
	int ret;
	array_init(&empty_array);
	ret = zend_update_static_property(ce, name, len, &empty_array);
	return ret;
}

int phalcon_update_static_property_array_append_ce(zend_class_entry *ce, const char *property, uint32_t property_length, zval *value)
{
	zval tmp = {};
	int separated = 0;

	phalcon_read_static_property_ce(&tmp, ce, property, property_length, PH_READONLY);

	/** Convert the value to array if not is an array */
	if (Z_TYPE(tmp) != IS_ARRAY) {
		array_init(&tmp);
		separated = 1;
	} else if (Z_REFCOUNTED(tmp)) {
		if (Z_REFCOUNT(tmp) > 1) {
			if (!Z_ISREF(tmp)) {
				zval new_zv;
				ZVAL_DUP(&new_zv, &tmp);
				ZVAL_COPY_VALUE(&tmp, &new_zv);
				separated = 1;
			}
		}
	}

	phalcon_array_append(&tmp, value, PH_COPY);

	if (separated) {
		phalcon_update_static_property_ce(ce, property, property_length, &tmp);
		zval_ptr_dtor(&tmp);
	}

	return SUCCESS;
}

/**
 * Increments an object property
 */
int phalcon_static_property_incr_ce(zend_class_entry *ce, const char *property, uint32_t len){

	zval value = {};

	phalcon_read_static_property_ce(&value, ce, property, len, PH_READONLY);
	phalcon_increment(&value);
	phalcon_update_static_property_ce(ce, property, len, &value);
	return SUCCESS;
}

/**
 * Decrements an object property
 */
int phalcon_static_property_decr_ce(zend_class_entry *ce, const char *property, uint32_t len){

	zval value = {};

	phalcon_read_static_property_ce(&value, ce, property, len, PH_READONLY);
	phalcon_decrement(&value);
	phalcon_update_static_property_ce(ce, property, len, &value);
	return SUCCESS;
}

/**
 * Returns a class name into a zval result
 */
void phalcon_get_class(zval *result, const zval *object, int lower) {

	zend_class_entry *ce;
	zend_string *class_name;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		ce = Z_OBJCE_P(object);
		class_name = zend_string_init(ZSTR_VAL(ce->name), ZSTR_LEN(ce->name), 0);
		ZVAL_NEW_STR(result, class_name);

		if (lower) {
			phalcon_strtolower_inplace(result);
		}

	} else if (Z_TYPE_P(object) == IS_STRING && phalcon_class_exists_ex(object, 1)) {
		ZVAL_COPY(result, object);
	} else {
		ZVAL_NULL(result);
		php_error_docref(NULL, E_WARNING, "phalcon_get_class expects an object");
	}
}

/**
 * Returns a class name into a zval result
 */
void phalcon_get_class_ns(zval *result, const zval *object, int lower) {

	int found = 0;
	zend_class_entry *ce;
	uint32_t i, class_length;
	const char *cursor, *class_name;

	if (Z_TYPE_P(object) != IS_OBJECT) {
		if (Z_TYPE_P(object) != IS_STRING) {
			ZVAL_NULL(result);
			php_error_docref(NULL, E_WARNING, "phalcon_get_class_ns expects an object");
			return;
		}
	}

	if (Z_TYPE_P(object) == IS_OBJECT) {
		ce = Z_OBJCE_P(object);
		class_name = ce->name->val;
		class_length = ce->name->len;
	} else {
		class_name = Z_STRVAL_P(object);
		class_length = Z_STRLEN_P(object);
	}

	if (!class_length) {
		ZVAL_NULL(result);
		return;
	}

	i = class_length;
	cursor = (char *) (class_name + class_length - 1);

	while (i > 0) {
		if ((*cursor) == '\\') {
			found = 1;
			break;
		}
		cursor--;
		i--;
	}

	if (found) {
		ZVAL_NEW_STR(result, zend_string_init(class_name + i, class_length - i + 1, 0));
	} else {
		ZVAL_STRINGL(result, class_name, class_length);
	}

	if (lower) {
		phalcon_strtolower_inplace(result);
	}
}

/**
 * Returns a namespace from a class name
 */
void phalcon_get_ns_class(zval *result, const zval *object, int lower) {

	zend_class_entry *ce;
	int found = 0;
	uint32_t i, j, class_length;
	const char *cursor, *class_name;

	if (Z_TYPE_P(object) != IS_OBJECT) {
		if (Z_TYPE_P(object) != IS_STRING) {
			php_error_docref(NULL, E_WARNING, "phalcon_get_ns_class expects an object");
			ZVAL_NULL(result);
			return;
		}
	}

	if (Z_TYPE_P(object) == IS_OBJECT) {
		ce = Z_OBJCE_P(object);
		class_name = ce->name->val;
		class_length = ce->name->len;
	} else {
		class_name = Z_STRVAL_P(object);
		class_length = Z_STRLEN_P(object);
	}

	if (!class_length) {
		ZVAL_NULL(result);
		return;
	}

	j = 0;
	i = class_length;
	cursor = (char *) (class_name + class_length - 1);

	while (i > 0) {
		if ((*cursor) == '\\') {
			found = 1;
			break;
		}
		cursor--;
		i--;
		j++;
	}

	if (j > 0) {

		if (found) {
			ZVAL_STRINGL(result, class_name, class_length - j - 1);
		} else {
			ZVAL_EMPTY_STRING(result);
		}

		if (lower) {
			zend_string_tolower(Z_STR_P(result));
		}
	} else {
		ZVAL_NULL(result);
	}

}

/**
 * Returns the called in class in the current scope
 */
void phalcon_get_called_class(zval *return_value)
{
#if PHP_VERSION_ID >= 70100
	zend_class_entry *called_scope = zend_get_called_scope(EG(current_execute_data));
	if (called_scope) {
		ZVAL_STR(return_value, zend_string_dup(called_scope->name, 0));
	}

	if (!zend_get_executed_scope())  {
		php_error_docref(NULL, E_WARNING, "phalcon_get_called_class() called from outside a class");
	}
#else
	if (EG(current_execute_data)->called_scope) {
		zend_string *ret = EG(current_execute_data)->called_scope->name;
		zend_string_addref(ret);
		RETURN_STR(ret);
	}

	if (!EG(scope))  {
		php_error_docref(NULL, E_WARNING, "phalcon_get_called_class() called from outside a class");
	}
#endif
}

/**
 * Returns a parent class name into a zval result
 */
void phalcon_get_parent_class(zval *result, const zval *object, int lower) {

	if (Z_TYPE_P(object) == IS_OBJECT) {
		const zend_class_entry *ce = Z_OBJCE_P(object);
		if (ce && ce->parent) {
			ZVAL_STR(result, zend_string_dup(ce->parent->name, 0));

			if (lower) {
				zend_str_tolower(Z_STRVAL_P(result), Z_STRLEN_P(result));
			}
		} else {
			ZVAL_NULL(result);
		}
	} else {
		php_error_docref(NULL, E_WARNING, "phalcon_get_class expects an object");
	}
}

/**
 * Returns an array of object properties
 */
void phalcon_get_object_vars(zval *result, zval *object, int check_access) {

	HashTable *properties;
	zend_object *zobj;
	zend_ulong index;
	zend_bool fast_copy = 0;
	zval *value;
	zend_string *key;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		if (Z_OBJ_HT_P(object)->get_properties == NULL) {
			ZVAL_NULL(result);
			return;
		}

		properties = Z_OBJ_HT_P(object)->get_properties(object);

		if (properties == NULL) {
			ZVAL_NULL(result);
			return;
		}

		zobj = Z_OBJ_P(object);
#if PHP_VERSION_ID < 70300
		if (!zobj->ce->default_properties_count && properties == zobj->properties && !ZEND_HASH_GET_APPLY_COUNT(properties)) {
#else
		if (!zobj->ce->default_properties_count && properties == zobj->properties && !GC_IS_RECURSIVE(properties)) {
#endif
			fast_copy = 1;
			/* Check if the object has a numeric property, See Bug 73998 */
			ZEND_HASH_FOREACH_STR_KEY(properties, key) {
				if (key && ZEND_HANDLE_NUMERIC(key, index)) {
					fast_copy = 0;
					break;
				}
			} ZEND_HASH_FOREACH_END();
		}

		if (fast_copy) {
			if (EXPECTED(zobj->handlers == &std_object_handlers)) {
				if (EXPECTED(!(GC_FLAGS(properties) & IS_ARRAY_IMMUTABLE))) {
#if PHP_VERSION_ID >= 70300
					GC_ADDREF(properties);
#else
					GC_REFCOUNT(properties)++;
#endif
				}
				ZVAL_ARR(result, properties);
				return;
			}
			ZVAL_ARR(result, zend_array_dup(properties));
			return;
		}

		array_init(result);
		if (check_access) {
			ZEND_HASH_FOREACH_STR_KEY_VAL_IND(properties, key, value) {
				if (key) {
					if (zend_check_property_access(zobj, key) == SUCCESS) {
						if (Z_ISREF_P(value) && Z_REFCOUNT_P(value) == 1) {
							value = Z_REFVAL_P(value);
						}
						if (Z_REFCOUNTED_P(value)) {
							Z_ADDREF_P(value);
						}
						if (ZSTR_VAL(key)[0] == 0) {
							const char *prop_name, *class_name;
							size_t prop_len;
							zend_unmangle_property_name_ex(key, &class_name, &prop_name, &prop_len);
							zend_hash_str_add_new(Z_ARRVAL_P(result), prop_name, prop_len, value);
						} else {
							zend_hash_add_new(Z_ARRVAL_P(result), key, value);
						}
					}
				}
			} ZEND_HASH_FOREACH_END();
		} else {
			ZEND_HASH_FOREACH_STR_KEY_VAL(properties, key, value) {
				if (key) {
					if (Z_ISREF_P(value) && Z_REFCOUNT_P(value) == 1) {
						value = Z_REFVAL_P(value);
					}
					if (Z_REFCOUNTED_P(value)) {
						Z_ADDREF_P(value);
					}
					if (ZSTR_VAL(key)[0] == 0) {
						const char *prop_name, *class_name;
						size_t prop_len;
						zend_unmangle_property_name_ex(key, &class_name, &prop_name, &prop_len);
						zend_hash_str_add_new(Z_ARRVAL_P(result), prop_name, prop_len, value);
					} else {
						zend_hash_add_new(Z_ARRVAL_P(result), key, value);
					}
				}
			} ZEND_HASH_FOREACH_END();
		}
	} else {
		php_error_docref(NULL, E_WARNING, "phalcon_get_object_vars expects an object");
	}
}

/**
 * Returns an array of object propertie names
 */
void phalcon_get_object_members(zval *result, zval *object, int check_access) {

	HashTable *properties;
	zend_string *key;

	zend_object *zobj;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		if (Z_OBJ_HT_P(object)->get_properties == NULL) {
			ZVAL_NULL(result);
			return;
		}

		properties = Z_OBJ_HT_P(object)->get_properties(object);

		if (properties == NULL) {
			ZVAL_NULL(result);
			return;
		}

		zobj = Z_OBJ_P(object);

		array_init(result);

		ZEND_HASH_FOREACH_STR_KEY(properties, key) {
			if (key) {
				if (!check_access || zend_check_property_access(zobj, key) == SUCCESS) {
					if (ZSTR_VAL(key)[0] == 0) {
						const char *prop_name, *class_name;
						size_t prop_len;
						zend_unmangle_property_name_ex(key, &class_name, &prop_name, &prop_len);
						phalcon_array_append_str(result, prop_name, prop_len, 0);
					} else {
						phalcon_array_append_str(result, ZSTR_VAL(key), ZSTR_LEN(key), 0);
					}
				}
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		php_error_docref(NULL, E_WARNING, "phalcon_get_object_members expects an object");
	}
}

static int same_name(zend_string *key, zend_string *name) /* {{{ */
{
	zend_string *lcname;
	int ret;

	if (key == name) {
		return 1;
	}
	if (ZSTR_LEN(key) != ZSTR_LEN(name)) {
		return 0;
	}
	lcname = zend_string_tolower(name);
	ret = memcmp(ZSTR_VAL(lcname), ZSTR_VAL(key), ZSTR_LEN(key)) == 0;
	zend_string_release(lcname);
	return ret;
}

/**
 * Returns an array of method names for class or class instance
 */
void phalcon_get_class_methods(zval *return_value, zval *object, int check_access) {

	zval method_name = {};
	zend_class_entry *ce = NULL, *pce;
	zend_function *mptr;
	zend_string *key;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		ce = Z_OBJCE_P(object);
	} else if (Z_TYPE_P(object) == IS_STRING) {
		if ((pce = zend_lookup_class(Z_STR_P(object))) != NULL) {
			ce = pce;
		}
	}

	if (!ce) {
		RETURN_NULL();
	}

	if (check_access) {
#if PHP_VERSION_ID < 70100
		zend_class_entry *scope = EG(scope);
#else
		zend_class_entry *scope = zend_get_executed_scope();
#endif

		ZEND_HASH_FOREACH_STR_KEY_PTR(&ce->function_table, key, mptr) {

			if ((mptr->common.fn_flags & ZEND_ACC_PUBLIC)
			 || (scope &&
				 (((mptr->common.fn_flags & ZEND_ACC_PROTECTED) &&
				   zend_check_protected(mptr->common.scope, scope))
			   || ((mptr->common.fn_flags & ZEND_ACC_PRIVATE) &&
				   scope == mptr->common.scope)))) {
				size_t len = ZSTR_LEN(mptr->common.function_name);

				/* Do not display old-style inherited constructors */
				if (!key) {
					ZVAL_STR_COPY(&method_name, mptr->common.function_name);
					zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &method_name);
				} else if ((mptr->common.fn_flags & ZEND_ACC_CTOR) == 0 ||
					mptr->common.scope == ce ||
					zend_binary_strcasecmp(ZSTR_VAL(key), ZSTR_LEN(key), ZSTR_VAL(mptr->common.function_name), len) == 0) {

					if (mptr->type == ZEND_USER_FUNCTION &&
						(!mptr->op_array.refcount || *mptr->op_array.refcount > 1) &&
						 !same_name(key, mptr->common.function_name)) {
						ZVAL_STR_COPY(&method_name, zend_find_alias_name(mptr->common.scope, key));
						zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &method_name);
					} else {
						ZVAL_STR_COPY(&method_name, mptr->common.function_name);
						zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &method_name);
					}
				}
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		array_init(return_value);

		ZEND_HASH_FOREACH_STR_KEY_PTR(&ce->function_table, key, mptr) {
			size_t len = ZSTR_LEN(mptr->common.function_name);

			/* Do not display old-style inherited constructors */
			if (!key) {
				ZVAL_STR_COPY(&method_name, mptr->common.function_name);
				zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &method_name);
			} else if ((mptr->common.fn_flags & ZEND_ACC_CTOR) == 0 ||
				mptr->common.scope == ce ||
				zend_binary_strcasecmp(ZSTR_VAL(key), ZSTR_LEN(key), ZSTR_VAL(mptr->common.function_name), len) == 0) {

				if (mptr->type == ZEND_USER_FUNCTION &&
					(!mptr->op_array.refcount || *mptr->op_array.refcount > 1) &&
					 !zend_string_equals_ci(key, mptr->common.function_name)) {
					ZVAL_STR_COPY(&method_name, zend_find_alias_name(mptr->common.scope, key));
					zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &method_name);
				} else {
					ZVAL_STR_COPY(&method_name, mptr->common.function_name);
					zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &method_name);
				}
			}
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * Fetches a zend class entry from a zval value
 */
zend_class_entry* phalcon_fetch_str_class(const char *class_name, uint32_t class_len, int fetch_type) {

	zend_class_entry* ce;
	zend_string *str = zend_string_init(class_name, class_len, 0);

	ce = zend_fetch_class(str, fetch_type);
	zend_string_release(str);

	return ce;
}

/**
 * Fetches a zend class entry from a zval value
 */
zend_class_entry *phalcon_fetch_class(const zval *class_name, int fetch_type) {

	if (Z_TYPE_P(class_name) == IS_STRING) {
		return zend_fetch_class(Z_STR_P(class_name), fetch_type);
	}

	php_error_docref(NULL, E_WARNING, "class name must be a string");
	return phalcon_fetch_str_class(SL("stdclass"), fetch_type);
}

zend_class_entry* phalcon_fetch_self_class() {
	return zend_fetch_class(NULL, ZEND_FETCH_CLASS_SELF);
}

zend_class_entry* phalcon_fetch_parent_class() {
	return zend_fetch_class(NULL, ZEND_FETCH_CLASS_PARENT);
}

zend_class_entry* phalcon_fetch_static_class() {
	return zend_fetch_class(NULL, ZEND_FETCH_CLASS_STATIC);
}

zend_class_entry* phalcon_get_internal_ce(const char *class_name, unsigned int class_name_len)
{
    zend_class_entry* temp_ce;

    if ((temp_ce = zend_hash_str_find_ptr(CG(class_table), class_name, class_name_len)) == NULL) {
        zend_error(E_WARNING, "Class '%s' not found", class_name);
        return NULL;
    }

    return temp_ce;
}

/**
 * Checks if a class exist
 */
zend_class_entry *phalcon_class_exists(const zval *class_name, int autoload) {

	zend_class_entry *ce;

	if (Z_TYPE_P(class_name) == IS_STRING) {
		if ((ce = zend_lookup_class_ex(Z_STR_P(class_name), NULL, autoload)) != NULL) {
			return (ce->ce_flags & (ZEND_ACC_INTERFACE | (ZEND_ACC_TRAIT - ZEND_ACC_EXPLICIT_ABSTRACT_CLASS))) == 0 ? ce : NULL;
		}
	}

	return NULL;
}

zend_class_entry *phalcon_class_exists_ex(const zval *class_name, int autoload) {

	if (Z_TYPE_P(class_name) == IS_STRING) {
		return phalcon_class_exists(class_name, autoload);
	}

	php_error_docref(NULL, E_WARNING, "class name must be a string");
	return NULL;
}

zend_class_entry *phalcon_class_str_exists(const char *class_name, uint32_t class_len, int autoload) {
	zval name = {};
	zend_class_entry *ce;

	ZVAL_STRINGL(&name, class_name, class_len);
	ce = phalcon_class_exists(&name, autoload);
	zval_ptr_dtor(&name);
	return ce;
}

/**
 * Checks if a interface exist
 */
int phalcon_interface_exists(const zval *class_name, int autoload) {

	zend_class_entry *ce;

	if (Z_TYPE_P(class_name) == IS_STRING) {
		if ((ce = zend_lookup_class(Z_STR_P(class_name))) != NULL) {
			return ((ce->ce_flags & ZEND_ACC_INTERFACE) > 0);
		}
		return 0;
	}

	php_error_docref(NULL, E_WARNING, "interface name must be a string");
	return 0;
}

/**
 * Clones an object from obj to destination
 */
int phalcon_clone(zval *destination, zval *obj) {

	int status = FAILURE;
	zend_class_entry *ce;
	zend_object_clone_obj_t clone_call;

	if (Z_TYPE_P(obj) != IS_OBJECT) {
		php_error_docref(NULL, E_ERROR, "__clone method called on non-object");
		status = FAILURE;
	} else {
		clone_call =  Z_OBJ_HT_P(obj)->clone_obj;
		if (!clone_call) {
			ce = Z_OBJCE_P(obj);
			if (ce) {
				php_error_docref(NULL, E_ERROR, "Trying to clone an uncloneable object of class %s", ce->name->val);
			} else {
				php_error_docref(NULL, E_ERROR, "Trying to clone an uncloneable object");
			}
		} else {
			ZVAL_OBJ(destination, clone_call(obj));
			if (EG(exception)) {
				ZVAL_NULL(destination);
			} else {
				status = SUCCESS;
			}
		}
	}

	return status;
}

int phalcon_instance_of(zval *result, const zval *object, const zend_class_entry *ce) {

	if (Z_TYPE_P(object) != IS_OBJECT) {
		php_error_docref(NULL, E_WARNING, "instanceof expects an object instance");
		ZVAL_FALSE(result);
		return FAILURE;
	}

	ZVAL_BOOL(result, instanceof_function(Z_OBJCE_P(object), ce));
	return SUCCESS;
}

int phalcon_instance_of_ev(const zval *object, const zend_class_entry *ce) {

	if (Z_TYPE_P(object) != IS_OBJECT) {
		php_error_docref(NULL, E_WARNING, "instanceof expects an object instance");
		return 0;
	}

	return instanceof_function(Z_OBJCE_P(object), ce);
}

/**
 * Check if an object is instance of a class
 */
int phalcon_is_instance_of(zval *object, const char *class_name, unsigned int class_length) {

	zend_class_entry *ce, *temp_ce;

	if (Z_TYPE_P(object) == IS_OBJECT) {

		ce = Z_OBJCE_P(object);
		if (ce->name->len == class_length) {
			if (!zend_binary_strcasecmp(ce->name->val, ce->name->len, class_name, class_length)) {
				return 1;
			}
		}

		temp_ce = phalcon_fetch_str_class(class_name, class_length, ZEND_FETCH_CLASS_DEFAULT);
		if (temp_ce) {
			return instanceof_function(ce, temp_ce);
		}
	}

	return 0;
}

int phalcon_zval_is_traversable(zval *object) {

	zend_class_entry *ce;
	uint32_t i;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		ce = Z_OBJCE_P(object);

		if (ce->get_iterator || (ce->parent && ce->parent->get_iterator)) {
			return 1;
		}

		for (i = 0; i < ce->num_interfaces; i++) {
			if (ce->interfaces[i] == zend_ce_aggregate ||
				ce->interfaces[i] == zend_ce_iterator ||
				ce->interfaces[i] == zend_ce_traversable
			) {
				return 1;
			}
		}
	}

	return 0;
}

/**
 * Checks if property exists on object
 */
int phalcon_property_exists(zval *object, const char *property_name, uint32_t property_length, int flags)
{
	if (Z_TYPE_P(object) == IS_OBJECT) {
		if (likely((flags & PH_DECLARED) == PH_DECLARED)) {
			if (likely(zend_hash_str_exists(&Z_OBJCE_P(object)->properties_info, property_name, property_length))) {
				return 1;
			}
		}

		if (likely((flags & PH_DYNAMIC) == PH_DYNAMIC)) {
			return zend_hash_str_exists(Z_OBJ_HT_P(object)->get_properties(object), property_name, property_length);
		}
	}

	return 0;
}

/**
 * Reads a property from an object
 */
int phalcon_read_property(zval *result, zval *object, const char *property_name, uint32_t property_length, int flags)
{
	zval property, rv;
	zend_class_entry *ce, *old_scope;
	zval *res;

	if (Z_TYPE_P(object) != IS_OBJECT) {

		if ((flags & PH_NOISY) == PH_NOISY) {
			php_error_docref(NULL, E_NOTICE, "Trying to get property \"%s\" of non-object", property_name);
		}

		ZVAL_NULL(result);
		return FAILURE;
	}

	ce = Z_OBJCE_P(object);
	if (ce->parent) {
		ce = phalcon_lookup_class_ce(ce, property_name, property_length);
	}

#if PHP_VERSION_ID >= 70100
	old_scope = EG(fake_scope);
	EG(fake_scope) = ce;
#else
	old_scope = EG(scope);
	EG(scope) = ce;
#endif
	if (!Z_OBJ_HT_P(object)->read_property) {
		const char *class_name;

		class_name = Z_OBJ_P(object) ? ZSTR_VAL(Z_OBJCE_P(object)->name) : "";
		zend_error(E_CORE_ERROR, "Property %s of class %s cannot be read", property_name, class_name);
	}

	ZVAL_STRINGL(&property, property_name, property_length);

	res = Z_OBJ_HT_P(object)->read_property(object, &property, flags ? BP_VAR_IS : BP_VAR_R, NULL, &rv);
	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		ZVAL_DUP(result, res);
	} else if ((flags & PH_READONLY) == PH_READONLY) {
		ZVAL_COPY_VALUE(result, res);
	} else {
		ZVAL_COPY(result, res);
	}

#if PHP_VERSION_ID >= 70100
	EG(fake_scope) = old_scope;
#else
	EG(scope) = old_scope;
#endif
	zval_ptr_dtor(&property);
	return SUCCESS;
}

/**
 * Checks whether obj is an object and updates property with another zval
 */
int phalcon_update_property(zval *object, const char *property_name, uint32_t property_length, zval *value){

	zend_class_entry *ce, *old_scope;
	zval property;

#if PHP_VERSION_ID >= 70100
	old_scope = EG(fake_scope);
#else
	old_scope = EG(scope);
#endif

	if (Z_TYPE_P(object) != IS_OBJECT) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object");
		return FAILURE;
	}

	ce = Z_OBJCE_P(object);
	if (ce->parent) {
		ce = phalcon_lookup_class_ce(ce, property_name, property_length);
	}

#if PHP_VERSION_ID >= 70100
	EG(fake_scope) = ce;
#else
	EG(scope) = ce;
#endif

	if (!Z_OBJ_HT_P(object)->write_property) {
		const char *class_name;

		class_name = Z_OBJ_P(object) ? ZSTR_VAL(Z_OBJCE_P(object)->name) : "";
		zend_error(E_CORE_ERROR, "Property %s of class %s cannot be updated", property_name, class_name);
	}

	ZVAL_STRINGL(&property, property_name, property_length);

	/* write_property will add 1 to refcount, so no Z_TRY_ADDREF_P(value); is necessary */
	// Z_OBJ_HT_P(object)->write_property(object, &property, value, NULL);
	Z_OBJ_HANDLER_P(object, write_property)(object, &property, value, NULL);

#if PHP_VERSION_ID >= 70100
	EG(fake_scope) = old_scope;
#else
	EG(scope) = old_scope;
#endif
	zval_ptr_dtor(&property);
	return SUCCESS;
}

/**
 * Checks whether obj is an object and updates property with long value
 */
int phalcon_update_property_long(zval *object, const char *property_name, uint32_t property_length, long value)
{
	zval v = {};
	int ret;
	ZVAL_LONG(&v, value);
	ret = phalcon_update_property(object, property_name, property_length, &v);
	zval_ptr_dtor(&v);
	return ret;
}

/**
 * Checks whether obj is an object and updates property with string value
 */
int phalcon_update_property_str(zval *object, const char *property_name, uint32_t property_length, const char *str, uint32_t str_length)
{
	zval tmp = {};
	int status = 0;

	ZVAL_STRINGL(&tmp, str, str_length);
	status = phalcon_update_property(object, property_name, property_length, &tmp);
	zval_ptr_dtor(&tmp);
	return status;
}

/**
 * Checks whether obj is an object and updates property with bool value
 */
int phalcon_update_property_bool(zval *object, const char *property_name, uint32_t property_length, int value) {
	zval v = {};
	int ret;
	ZVAL_BOOL(&v, value);
	ret = phalcon_update_property(object, property_name, property_length, &v);
	zval_ptr_dtor(&v);
	return ret;
}

/**
 * Checks whether obj is an object and updates property with null value
 */
int phalcon_update_property_null(zval *object, const char *property_name, uint32_t property_length) {
	zval v = {};
	int ret;
	ZVAL_NULL(&v);
	ret = phalcon_update_property(object, property_name, property_length, &v);
	zval_ptr_dtor(&v);
	return ret;
}

int phalcon_update_property_string_zval(zval *object, zend_string *property, zval *value){

	return phalcon_update_property(object, ZSTR_VAL(property), ZSTR_LEN(property), value);
}

int phalcon_update_property_zval_null(zval *object, const zval *property)
{
	zval v = {};
	int ret;
	ZVAL_NULL(&v);
	ret = phalcon_update_property(object, Z_STRVAL_P(property), Z_STRLEN_P(property), &v);
	zval_ptr_dtor(&v);
	return ret;
}

/**
 * Checks whether obj is an object and updates zval property with long value
 */
int phalcon_update_property_zval_long(zval *object, const zval *property, int value)
{
	zval v = {};
	int ret;
	ZVAL_LONG(&v, value);
	ret = phalcon_update_property(object, Z_STRVAL_P(property), Z_STRLEN_P(property), &v);
	zval_ptr_dtor(&v);
	return ret;
}

/**
 * Checks whether obj is an object and updates zval property with another zval
 */
int phalcon_update_property_zval_zval(zval *object, const zval *property, zval *value)
{
	if (Z_TYPE_P(property) != IS_STRING) {
		php_error_docref(NULL, E_WARNING, "Property should be string");
		return FAILURE;
	}

	return phalcon_update_property(object, Z_STRVAL_P(property), Z_STRLEN_P(property), value);
}

/**
 * Updates an array property
 */
int phalcon_update_property_array(zval *object, const char *property, uint32_t property_length, const zval *index, zval *value)
{
	zval tmp;
	int separated = 0;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		phalcon_read_property(&tmp, object, property, property_length, PH_NOISY | PH_READONLY);

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp) != IS_ARRAY) {
			array_init(&tmp);
			separated = 1;
		} else if (Z_REFCOUNTED(tmp)) {
			if (Z_REFCOUNT(tmp) > 1) {
				if (!Z_ISREF(tmp)) {
					zval new_zv;
					ZVAL_DUP(&new_zv, &tmp);
					ZVAL_COPY_VALUE(&tmp, &new_zv);
					separated = 1;
				}
			}
		}
		Z_TRY_ADDREF_P(value);

		if (Z_TYPE_P(index) == IS_STRING) {
			zend_symtable_str_update(Z_ARRVAL(tmp), Z_STRVAL_P(index), Z_STRLEN_P(index), value);
		} else if (Z_TYPE_P(index) == IS_LONG) {
			zend_hash_index_update(Z_ARRVAL(tmp), Z_LVAL_P(index), value);
		} else if (Z_TYPE_P(index) == IS_NULL) {
			zend_hash_next_index_insert(Z_ARRVAL(tmp), value);
		}

		if (separated) {
			phalcon_update_property(object, property, property_length, &tmp);
			zval_ptr_dtor(&tmp);
		}
	}

	return SUCCESS;
}

/**
 * Multiple array-offset update
 */
int phalcon_update_property_array_multi(zval *object, const char *property, uint32_t property_length, zval *value, const char *types, int types_length, int types_count, ...)
{
	va_list ap;
	zval tmp;
	int separated = 0;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		phalcon_read_property(&tmp, object, property, property_length, PH_NOISY | PH_READONLY);

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp) != IS_ARRAY) {
			array_init(&tmp);
			separated = 1;
		} else if (Z_REFCOUNTED(tmp)) {
			if (Z_REFCOUNT(tmp) > 1) {
				if (!Z_ISREF(tmp)) {
					zval new_zv;
					ZVAL_DUP(&new_zv, &tmp);
					ZVAL_COPY_VALUE(&tmp, &new_zv);
					separated = 1;
				}
			}
		}

		va_start(ap, types_count);
		phalcon_array_update_multi_ex(&tmp, value, types, types_length, types_count, ap);
		va_end(ap);

		if (separated) {
			phalcon_update_property(object, property, property_length, &tmp);
			zval_ptr_dtor(&tmp);
		}
	}

	return SUCCESS;
}

/**
 * Updates an array property using a string index
 */
int phalcon_update_property_array_str(zval *object, const char *property, uint32_t property_length, const char *index, uint32_t index_length, zval *value)
{
	zval tmp = {};
	int status = 0;

	ZVAL_STRINGL(&tmp, index, index_length);
	status = phalcon_update_property_array(object, property, property_length, &tmp, value);
	zval_ptr_dtor(&tmp);
	return status;
}

int phalcon_update_property_array_string(zval *object, const char *property, uint32_t property_length, zend_string *index, zval *value)
{
	zval tmp = {};
	ZVAL_STR(&tmp, index);
	return phalcon_update_property_array(object, property, property_length, &tmp, value);
}

/**
 * Appends a zval value to an array property
 */
int phalcon_update_property_array_append(zval *object, const char *property, uint32_t property_length, zval *value)
{
	zval tmp = {};
	int separated = 0;

	if (!object) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object (1)");
		return FAILURE;
	}

	if (Z_TYPE_P(object) != IS_OBJECT) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object (2)");
		return FAILURE;
	}

	phalcon_read_property(&tmp, object, property, property_length, PH_NOISY | PH_READONLY);

	/** Convert the value to array if not is an array */
	if (Z_TYPE(tmp) != IS_ARRAY) {
		array_init(&tmp);
		separated = 1;
	} else if (Z_REFCOUNTED(tmp)) {
		if (Z_REFCOUNT(tmp) > 1) {
			if (!Z_ISREF(tmp)) {
				zval new_zv;
				ZVAL_DUP(&new_zv, &tmp);
				ZVAL_COPY_VALUE(&tmp, &new_zv);
				separated = 1;
			}
		}
	}

	phalcon_array_append(&tmp, value, PH_COPY);

	if (separated) {
		phalcon_update_property(object, property, property_length, &tmp);
		zval_ptr_dtor(&tmp);
	}

	return SUCCESS;
}

int phalcon_update_property_array_merge(zval *object, const char *property, uint32_t property_length, zval *values)
{
	zval tmp = {};
	int separated = 0;

	if (!object) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object (1)");
		return FAILURE;
	}

	if (Z_TYPE_P(object) != IS_OBJECT) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object (2)");
		return FAILURE;
	}

	phalcon_read_property(&tmp, object, property, property_length, PH_NOISY | PH_READONLY);

	/** Convert the value to array if not is an array */
	if (Z_TYPE(tmp) != IS_ARRAY) {
		array_init(&tmp);
		separated = 1;
	} else if (Z_REFCOUNTED(tmp)) {
		if (Z_REFCOUNT(tmp) > 1) {
			if (!Z_ISREF(tmp)) {
				zval new_zv;
				ZVAL_DUP(&new_zv, &tmp);
				ZVAL_COPY_VALUE(&tmp, &new_zv);
				separated = 1;
			}
		}
	}

	phalcon_array_merge_recursive_n(&tmp, values);

	if (separated) {
		phalcon_update_property(object, property, property_length, &tmp);
		zval_ptr_dtor(&tmp);
	}

	return SUCCESS;
}

/**
 * Appends every element of an array at the end of the array property
 */
int phalcon_update_property_array_merge_append(zval *object, const char *property, uint32_t property_length, zval *values) {

	zval tmp = {};
	int separated = 0;

	if (!object) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object (1)");
		return FAILURE;
	}

	if (Z_TYPE_P(object) != IS_OBJECT) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object (2)");
		return FAILURE;
	}

	phalcon_read_property(&tmp, object, property, property_length, PH_NOISY | PH_READONLY);

	/** Convert the value to array if not is an array */
	if (Z_TYPE(tmp) != IS_ARRAY) {
		array_init(&tmp);
		separated = 1;
	} else if (Z_REFCOUNTED(tmp)) {
		if (Z_REFCOUNT(tmp) > 1) {
			if (!Z_ISREF(tmp)) {
				zval new_zv;
				ZVAL_DUP(&new_zv, &tmp);
				ZVAL_COPY_VALUE(&tmp, &new_zv);
				separated = 1;
			}
		}
	}

	phalcon_merge_append(&tmp, values);

	if (separated) {
		phalcon_update_property(object, property, property_length, &tmp);
		zval_ptr_dtor(&tmp);
	}

	return SUCCESS;
}

/**
 * Intializes an object property with an empty array
 */
int phalcon_update_property_empty_array(zval *object, const char *property_name, uint32_t property_length) {

	zval v = {};
	int status;

	array_init(&v);
	status = phalcon_update_property(object, property_name, property_length, &v);
	zval_ptr_dtor(&v);
	return status;
}

/**
 * Checks if an array property exists on object
 */
int phalcon_isset_property_array(zval *object, const char *property, uint32_t property_length, const zval *index) {

	zval tmp = {};

	if (Z_TYPE_P(object) == IS_OBJECT) {
		phalcon_read_property(&tmp, object, property, property_length, PH_NOISY | PH_READONLY);
		return phalcon_array_isset(&tmp, index);
	}

	return 0;
}

/**
 * Reads a array property from an object
 */
int phalcon_read_property_array(zval *return_value, zval *object, const char *property, size_t property_length, const zval *index, int flags) {

	zval tmp = {};

	if (phalcon_read_property(&tmp, object, property, property_length, PH_NOISY | PH_READONLY) == FAILURE || !phalcon_array_isset_fetch(return_value, &tmp, index, flags)) {
		ZVAL_NULL(return_value);
		return 0;
	}

	return 1;
}

/**
 * Unsets an index in an array property
 */
int phalcon_unset_property_array(zval *object, const char *property, uint32_t property_length, const zval *index) {

	zval tmp = {};

	if (Z_TYPE_P(object) == IS_OBJECT) {
		phalcon_read_property(&tmp, object, property, property_length, PH_NOISY | PH_READONLY);

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp) == IS_ARRAY) {
			phalcon_array_unset(&tmp, index, 0);
		}
	}

	return SUCCESS;
}

/**
 * Check if a method is implemented on certain object
 */
int phalcon_method_exists(const zval *object, const zval *method_name){

	zend_string *lcname;
	zend_class_entry *ce;

	if (likely(Z_TYPE_P(object) == IS_OBJECT)) {
		ce = Z_OBJCE_P(object);
	} else if (Z_TYPE_P(object) == IS_STRING) {
		ce = zend_fetch_class(Z_STR_P(object), ZEND_FETCH_CLASS_DEFAULT);
	} else {
		return FAILURE;
	}

	lcname = zend_string_tolower(Z_STR_P(method_name));

	while (ce) {
		if (zend_hash_exists(&ce->function_table, lcname)) {
			zend_string_release(lcname);
			return SUCCESS;
		}
		ce = ce->parent;
	}

	zend_string_release(lcname);

	return FAILURE;
}


int phalcon_internal_method_exists(const zval *object, const zval *method_name){

	zend_class_entry *ce;

	ce = Z_OBJCE_P(object);
	while (ce && ce->type != ZEND_INTERNAL_CLASS) {
		ce = ce->parent;
	}
	if (ce) {
		return phalcon_method_exists_ce(ce, method_name);
	}
	return FAILURE;
}

/**
 * Check if method exists on certain object using explicit char param
 *
 * @param object
 * @param method_name
 * @param method_length strlen(method_name)
 */
int phalcon_method_exists_ex(const zval *object, const char *method_name, uint32_t method_len)
{
	zend_class_entry *ce;

	if (likely(Z_TYPE_P(object) == IS_OBJECT)) {
		ce = Z_OBJCE_P(object);
	} else if (Z_TYPE_P(object) == IS_STRING) {
		ce = zend_fetch_class(Z_STR_P(object), ZEND_FETCH_CLASS_DEFAULT);
	} else {
		return FAILURE;
	}

	while (ce) {
		if (zend_hash_str_exists(&ce->function_table, method_name, method_len)) {
			return SUCCESS;
		}
		ce = ce->parent;
	}

	return FAILURE;
}

/**
 * Check if a method is implemented on certain object
 */
int phalcon_method_exists_ce(const zend_class_entry *ce, const zval *method_name){

	char *lcname = zend_str_tolower_dup(Z_STRVAL_P(method_name), Z_STRLEN_P(method_name));
	int res = phalcon_method_exists_ce_ex(ce, lcname, Z_STRLEN_P(method_name)+1);
	efree(lcname);
	return res;
}

/**
 * Check if method exists on certain object using explicit char param
 *
 * @param zend_class_entry
 * @param method_name
 * @param method_length strlen(method_name)
 */
int phalcon_method_exists_ce_ex(const zend_class_entry *ce, const char *method_name, uint32_t method_len)
{
	return (ce && zend_hash_str_exists(&ce->function_table, method_name, method_len)) ? SUCCESS : FAILURE;
}

int phalcon_create_instance_params_ce(zval *return_value, zend_class_entry *ce, zval *params)
{
	int outcome = SUCCESS;

	object_init_ex(return_value, ce);

	if (phalcon_has_constructor_ce(ce)) {
		int param_count = (Z_TYPE_P(params) == IS_ARRAY) ? zend_hash_num_elements(Z_ARRVAL_P(params)) : 0;
		zval *static_params[10];
		zval **params_ptr, **params_arr = NULL;

		if (param_count > 0) {
			zval *item;
			int i = 0;

			if (likely(param_count) <= 10) {
				params_ptr = static_params;
			} else {
				params_arr = emalloc(param_count * sizeof(zval*));
				params_ptr = params_arr;
			}

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(params), item) {
				params_ptr[i++] = item;
			} ZEND_HASH_FOREACH_END();
		} else {
			params_ptr = NULL;
		}

		outcome = phalcon_call_method(NULL, return_value, "__construct", param_count, params_ptr);

		if (unlikely(params_arr != NULL)) {
			efree(params_arr);
		}
	}

	return outcome;
}

/**
 * Creates a new instance dynamically. Call constructor without parameters
 */
int phalcon_create_instance(zval *return_value, const zval *class_name){

	zend_class_entry *ce;

	if (unlikely(Z_TYPE_P(class_name) != IS_STRING)) {
		phalcon_throw_exception_string(phalcon_exception_ce, "Invalid class name");
		return FAILURE;
	}

	ce = zend_fetch_class(Z_STR_P(class_name), ZEND_FETCH_CLASS_DEFAULT);
	if (!ce) {
		return FAILURE;
	}

	return phalcon_create_instance_params_ce(return_value, ce, &PHALCON_GLOBAL(z_null));
}

/**
 * Creates a new instance dynamically calling constructor with parameters
 */
int phalcon_create_instance_params(zval *return_value, const zval *class_name, zval *params){

	zend_class_entry *ce;

	if (unlikely(Z_TYPE_P(class_name) != IS_STRING)) {
		phalcon_throw_exception_string(phalcon_exception_ce, "Invalid class name");
		return FAILURE;
	}

	ce = zend_fetch_class(Z_STR_P(class_name), ZEND_FETCH_CLASS_DEFAULT);
	if (!ce) {
		ZVAL_NULL(return_value);
		return FAILURE;
	}

	return phalcon_create_instance_params_ce(return_value, ce, params);
}

/**
 * Increments an object property
 */
int phalcon_property_incr(zval *object, const char *property_name, uint32_t property_length){

	zval value = {};

	phalcon_read_property(&value, object, property_name, property_length, PH_READONLY);
	phalcon_increment(&value);
	phalcon_update_property(object, property_name, property_length, &value);

	return SUCCESS;
}

/**
 * Decrements an object property
 */
int phalcon_property_decr(zval *object, const char *property_name, uint32_t property_length){

	zval value = {};

	phalcon_read_property(&value, object, property_name, property_length, PH_READONLY);
	phalcon_decrement(&value);
	phalcon_update_property(object, property_name, property_length, &value);

	return SUCCESS;
}

/**
 * Checks if property access on object
 */
int phalcon_check_property_access(zval *object, const char *property_name, uint32_t property_length, int access)
{
	zend_class_entry *ce;
	zend_property_info *property_info;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		ce = Z_OBJCE_P(object);
		if ((property_info = zend_hash_str_find_ptr(&ce->properties_info, property_name, property_length)) == NULL) {
			return (property_info->flags & access) == access;
		}
	}

	return 0;
}

int phalcon_property_isset_fetch(zval *return_value, zval *object, const char *property_name, size_t property_length, int flags)
{
	zval *value;
	zend_class_entry *ce;

	if (!phalcon_isset_property(object, property_name, property_length)) {
		return 0;
	}

	ce = Z_OBJCE_P(object);
	if (ce->parent) {
		ce = phalcon_lookup_class_ce(ce, property_name, property_length);
	}

	value = zend_read_property(ce, object, property_name, property_length, 1, NULL);
	if (EXPECTED(Z_TYPE_P(value) == IS_REFERENCE)) {
		value = Z_REFVAL_P(value);
	}
	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		ZVAL_DUP(return_value, value);
	} else if ((flags & PH_READONLY) == PH_READONLY) {
		ZVAL_COPY_VALUE(return_value, value);
	} else {
		ZVAL_COPY(return_value, value);
	}

	return 1;
}

int phalcon_property_array_isset_fetch(zval *fetched, zval *object, const char *property, size_t property_length, const zval *index, int flags)
{
	zval property_value = {};

	if (!phalcon_property_isset_fetch(&property_value, object, property, property_length, PH_READONLY)) {
		return 0;
	}

	if (!phalcon_array_isset_fetch(fetched, &property_value, index, flags)) {
		return 0;
	}
	return 1;
}

int phalcon_property_array_pop(zval *fetched, zval *object, const char *property, size_t property_length)
{
	zval property_value = {};

	if (!phalcon_property_isset_fetch(&property_value, object, property, property_length, PH_READONLY)) {
		return 0;
	}

	if (!phalcon_array_pop(fetched, &property_value)) {
		return 0;
	}
	return 1;
}

int phalcon_property_array_last(zval *fetched, zval *object, const char *property, size_t property_length, int flags)
{
	zval property_value = {};

	if (!phalcon_property_isset_fetch(&property_value, object, property, property_length, PH_READONLY)) {
		return 0;
	}

	if (phalcon_array_last(fetched, &property_value, flags) == FAILURE) {
		return 0;
	}
	return 1;
}
