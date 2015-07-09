
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
int phalcon_get_class_constant(zval *return_value, const zend_class_entry *ce, const char *constant_name, uint32_t constant_length) {

	zval *result;

	if ((result = zend_hash_str_find(&ce->constants_table, constant_name, constant_length)) == NULL) {
		php_error_docref(NULL, E_ERROR, "Undefined class constant '%s::%s'", ce->name->val, constant_name);
		return FAILURE;
	}

	ZVAL_ZVAL(return_value, result, 1, 0);
	return SUCCESS;
}

/*
 * Multiple array-offset update
 */
int phalcon_update_static_property_array_multi_ce(zend_class_entry *ce, const char *property, uint32_t property_length, zval *value, const char *types, int types_length, int types_count, ...) {

	int i, l, ll; char *s;
	va_list ap;
	zval *fetched, *tmp_arr, *tmp, *p, *item;
	int separated = 0;

	tmp_arr = phalcon_read_static_property_ce(ce, property, property_length);

	Z_DELREF_P(tmp_arr);

	/** Separation only when refcount > 1 */
	if (Z_REFCOUNT_P(tmp_arr) > 1) {
		if (!Z_ISREF_P(tmp_arr)) {
			SEPARATE_ZVAL(tmp_arr);
			separated = 1;
		}
	}

	/** Convert the value to array if not is an array */
	if (Z_TYPE_P(tmp_arr) != IS_ARRAY) {
		if (separated) {
			convert_to_array(tmp_arr);
		} else {
			if (!Z_ISREF_P(tmp_arr)) {
				SEPARATE_ZVAL(tmp_arr);
				separated = 1;
			}
			convert_to_array(tmp_arr);
		}
	}

	va_start(ap, types_count);

	p = tmp_arr;
	for (i = 0; i < types_length; ++i) {
		switch (types[i]) {

			case 's':
				s = va_arg(ap, char*);
				l = va_arg(ap, int);
				if (phalcon_array_isset_str_fetch(&fetched, p, s, l + 1)) {
					if (Z_TYPE_P(fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							phalcon_array_update_string(fetched, s, l, value, PH_COPY | PH_SEPARATE);
						} else {
							p = fetched;
						}
						continue;
					}
				}
				if (i == (types_length - 1)) {
					phalcon_array_update_string(p, s, l, value, PH_COPY | PH_SEPARATE);
				} else {
					PHALCON_ALLOC_GHOST_ZVAL(tmp);
					array_init(tmp);
					phalcon_array_update_string(p, s, l, tmp, PH_SEPARATE);
					p = tmp;
				}
				break;

			case 'l':
				ll = va_arg(ap, long);
				if (phalcon_array_isset_long_fetch(&fetched, p, ll)) {
					if (Z_TYPE_P(fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							phalcon_array_update_long(fetched, ll, value, PH_COPY | PH_SEPARATE);
						} else {
							p = fetched;
						}
						continue;
					}
				}
				if (i == (types_length - 1)) {
					phalcon_array_update_long(p, ll, value, PH_COPY | PH_SEPARATE);
				} else {
					PHALCON_ALLOC_GHOST_ZVAL(tmp);
					array_init(tmp);
					phalcon_array_update_long(p, ll, tmp, PH_SEPARATE);
					p = tmp;
				}
				break;

			case 'z':
				item = va_arg(ap, zval*);
				if (phalcon_array_isset_fetch(&fetched, p, item)) {
					if (Z_TYPE_P(fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							phalcon_array_update_zval(fetched, item, value, PH_COPY | PH_SEPARATE);
						} else {
							p = fetched;
						}
						continue;
					}
				}
				if (i == (types_length - 1)) {
					phalcon_array_update_zval(p, item, value, PH_COPY | PH_SEPARATE);
				} else {
					PHALCON_ALLOC_GHOST_ZVAL(tmp);
					array_init(tmp);
					phalcon_array_update_zval(p, item, tmp, PH_SEPARATE);
					p = tmp;
				}
				break;

			case 'a':
				phalcon_array_append(p, value, PH_SEPARATE);
				break;
		}
	}

	va_end(ap);

	if (separated) {
		phalcon_update_static_property_ce(ce, property, property_length, tmp_arr);
	}

	return SUCCESS;
}

/**
 * Returns a class name into a zval result
 */
void phalcon_get_class(zval *result, const zval *object, int lower) {

	if (Z_TYPE_P(object) == IS_OBJECT) {
		const zend_class_entry *ce = Z_OBJCE_P(object);
		ZVAL_NEW_STR(result, ce->name);

		if (lower) {
			zend_str_tolower(Z_STRVAL_P(result), Z_STRLEN_P(result));
		}

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
		zend_str_tolower(Z_STRVAL_P(result), Z_STRLEN_P(result));
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
	zend_execute_data *ex = EG(current_execute_data);
	zend_class_entry *called_scope = ex->called_scope;
	if (called_scope) {
		RETURN_NEW_STR(called_scope->name);
	}

	if (!EG(scope))  {
		php_error_docref(NULL, E_WARNING, "phalcon_get_called_class() called from outside a class");
	}

}

/**
 * Returns a parent class name into a zval result
 */
void phalcon_get_parent_class(zval *result, const zval *object, int lower) {

	if (Z_TYPE_P(object) == IS_OBJECT) {
		const zend_class_entry *ce = Z_OBJCE_P(object);
		if (ce && ce->parent) {
			ZVAL_NEW_STR(result, ce->parent->name);

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

	zval *value;
	HashTable *properties;
	zend_string *key;
	const char *prop_name, *class_name;

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

		ZEND_HASH_FOREACH_STR_KEY_VAL(properties, key, value) {
			if (key) {
				if (!check_access || zend_check_property_access(zobj, key) == SUCCESS) {
					zend_unmangle_property_name(key, &class_name, &prop_name);
					/* Not separating references */
					Z_ADDREF_P(value);
					add_assoc_zval_ex(result, prop_name, strlen(prop_name)+1, value);
				}
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		php_error_docref(NULL, E_WARNING, "phalcon_get_object_vars expects an object");
	}
}

/**
 * Returns an array of method names for class or class instance
 */
void phalcon_get_class_methods(zval *return_value, zval *object, int check_access) {

	zval method_name;
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
		PHALCON_CALL_FUNCTION(&return_value, "get_class_methods", object);
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
zend_class_entry *phalcon_fetch_class(const zval *class_name) {

	if (Z_TYPE_P(class_name) == IS_STRING) {
		return zend_fetch_class(Z_STR_P(class_name), ZEND_FETCH_CLASS_DEFAULT);
	}

	php_error_docref(NULL, E_WARNING, "class name must be a string");
	return zend_fetch_class(SSL("stdclass"), ZEND_FETCH_CLASS_DEFAULT);
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

/**
 * Checks if a class exist
 */
zend_class_entry *phalcon_class_exists(const zval *class_name, int autoload) {

	zend_class_entry *ce;

	if ((ce = zend_lookup_class_ex(Z_STR_P(class_name), NULL, autoload)) != NULL) {
		return (ce->ce_flags & (ZEND_ACC_INTERFACE | (ZEND_ACC_TRAIT - ZEND_ACC_EXPLICIT_ABSTRACT_CLASS))) == 0 ? ce : NULL;
	}

	return 0;
}

zend_class_entry *phalcon_class_exists_ex(const zval *class_name, int autoload) {

	if (Z_TYPE_P(class_name) == IS_STRING) {
		return phalcon_class_exists(class_name, autoload);
	}

	php_error_docref(NULL, E_WARNING, "class name must be a string");
	return NULL;
}

zend_class_entry *phalcon_class_str_exists(const char *class_name, uint32_t class_len, int autoload) {

	zend_class_entry *ce;

	if ((ce = zend_lookup_class_ex(zend_string_init(class_name, class_len, 0), NULL, autoload)) != NULL) {
		return (ce->ce_flags & (ZEND_ACC_INTERFACE | (ZEND_ACC_TRAIT - ZEND_ACC_EXPLICIT_ABSTRACT_CLASS))) == 0 ? ce : NULL;
	}

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

	int status = SUCCESS;
	zend_class_entry *ce;
	zend_object_clone_obj_t clone_call;

	if (Z_TYPE_P(obj) != IS_OBJECT) {
		php_error_docref(NULL, E_ERROR, "__clone method called on non-object");
		status = FAILURE;
	} else {
		ce = Z_OBJCE_P(obj);
		clone_call =  Z_OBJ_HT_P(obj)->clone_obj;
		if (!clone_call) {
			if (ce) {
				php_error_docref(NULL, E_ERROR, "Trying to clone an uncloneable object of class %s", ce->name->val);
			} else {
				php_error_docref(NULL, E_ERROR, "Trying to clone an uncloneable object");
			}
			status = FAILURE;
		} else {
			if (!EG(exception)) {
				ZVAL_OBJ(destination, clone_call(obj));
				Z_SET_REFCOUNT_P(destination, 1);
				ZVAL_UNREF(destination);
				if (EG(exception)) {
					zval_ptr_dtor(destination);
					ZVAL_NULL(destination);
				}
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

		temp_ce = zend_fetch_class(zend_string_init(class_name, class_length, 0), ZEND_FETCH_CLASS_DEFAULT);
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
int phalcon_isset_property(zval *object, const char *property_name, uint32_t property_length)
{
	if (Z_TYPE_P(object) == IS_OBJECT) {
		if (likely(zend_hash_str_exists(&Z_OBJCE_P(object)->properties_info, property_name, property_length))) {
			return 1;
		}

		return zend_hash_str_exists(Z_OBJ_HT_P(object)->get_properties(object), property_name, property_length);
	}

	return 0;
}

/**
 * Lookup exact class where a property is defined
 *
 */
static inline zend_class_entry *phalcon_lookup_class_ce(zend_class_entry *ce, const char *property_name, uint32_t property_length) {

	zend_class_entry *original_ce = ce;

	while (ce) {
		if (zend_hash_str_exists(&ce->properties_info, property_name, property_length + 1)) {
			return ce;
		}
		ce = ce->parent;
	}
	return original_ce;
}

static inline zend_class_entry *phalcon_lookup_str_class_ce(zend_class_entry *ce, zend_string *property) {

	zend_class_entry *original_ce = ce;

	while (ce) {
		if (zend_hash_exists(&ce->properties_info, property)) {
			return ce;
		}
		ce = ce->parent;
	}
	return original_ce;
}

/**
 * Reads a property from an object
 */
zval* phalcon_read_property(zval *object, const char *property_name, size_t property_length, int silent)
{
	zend_class_entry *ce;

	if (Z_TYPE_P(object) != IS_OBJECT) {
		if (silent == PH_NOISY) {
			php_error_docref(NULL, E_NOTICE, "Trying to get property of non-object");
		}
		return &PHALCON_GLOBAL(z_null);
	}

	ce = Z_OBJCE_P(object);
	if (ce->parent) {
		ce = phalcon_lookup_class_ce(ce, property_name, property_length);
	}

	return zend_read_property(ce, object, property_name, property_length, silent, NULL);
}

/**
 * Checks whether obj is an object and updates property with long value
 */
int phalcon_update_property_long(zval *object, const char *property_name, uint32_t property_length, long value)
{

	zval *v;

	PHALCON_INIT_ZVAL_NREF(v);
	ZVAL_LONG(v, value);

	return phalcon_update_property_zval(object, property_name, property_length, v);
}

/**
 * Checks whether obj is an object and updates property with string value
 */
int phalcon_update_property_str(zval *object, const char *property_name, uint32_t property_length, const char *str, uint32_t str_length)
{
	zval *value;

	PHALCON_INIT_ZVAL_NREF(value);
	ZVAL_STRINGL(value, str, str_length);

	return phalcon_update_property_zval(object, property_name, property_length, value);
}

/**
 * Checks whether obj is an object and updates property with bool value
 */
int phalcon_update_property_bool(zval *object, const char *property_name, uint32_t property_length, int value) {

	zval *v = value ? &PHALCON_GLOBAL(z_true) : &PHALCON_GLOBAL(z_false);
	return phalcon_update_property_zval(object, property_name, property_length, v);
}

/**
 * Checks whether obj is an object and updates property with null value
 */
int phalcon_update_property_null(zval *object, const char *property_name, uint32_t property_length) {
	zval *v = &PHALCON_GLOBAL(z_null);
	return phalcon_update_property_zval(object, property_name, property_length, v);
}

/**
 * Checks whether obj is an object and updates property with another zval
 */
int phalcon_update_property_zval(zval *object, const char *property_name, uint32_t property_length, zval *value){
	zend_class_entry *ce, *old_scope;
	zval property;

	if (!object) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object (1)");
		return FAILURE;
	}

	old_scope = EG(scope);
	if (Z_TYPE_P(object) != IS_OBJECT) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object (2)");
		return FAILURE;
	}

	ce = Z_OBJCE_P(object);
	if (ce->parent) {
		ce = phalcon_lookup_class_ce(ce, property_name, property_length);
	}

	EG(scope) = ce;

	if (!Z_OBJ_HT_P(object)->write_property) {
		zend_error(E_CORE_ERROR, "Property %s of class %s cannot be updated", property_name, ce->name->val);
	}

	ZVAL_STRINGL(&property, property_name, property_length);

	Z_OBJ_HT_P(object)->write_property(object, &property, value, 0);

	zval_dtor(&property);

	EG(scope) = old_scope;
	return SUCCESS;
}

int phalcon_update_property_string_zval(zval *object, zend_string *property, zval *value){
	zend_class_entry *ce;

	if (!object) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object (1)");
		return FAILURE;
	}

	if (Z_TYPE_P(object) != IS_OBJECT) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object (2)");
		return FAILURE;
	}

	ce = Z_OBJCE_P(object);
	if (ce->parent) {
		ce = phalcon_lookup_str_class_ce(ce, property);
	}

	zend_update_property_ex(ce, object, property, value);

	return SUCCESS;
}

/**
 * Updates properties on this_ptr (quick)
 * Variables must be defined in the class definition. This function ignores magic methods or dynamic properties
 */
int phalcon_update_property_this(zval *object, const char *property_name, uint32_t property_length, zval *value){
	zval property;

	if (unlikely(Z_TYPE_P(object) != IS_OBJECT)) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object");
		return FAILURE;
	}

	ZVAL_STR(&property, zend_string_init(property_name, property_length, 0));
	zend_std_write_property(object, &property, value, NULL); 

	return SUCCESS;
}

int phalcon_update_property_zval_null(zval *object, const zval *property){

	zval *v = &PHALCON_GLOBAL(z_null);
	return phalcon_update_property_zval(object, Z_STRVAL_P(property), Z_STRLEN_P(property), v);
}

/**
 * Checks whether obj is an object and updates zval property with long value
 */
int phalcon_update_property_zval_long(zval *object, const zval *property, int value){

	if (Z_TYPE_P(property) != IS_STRING) {
		php_error_docref(NULL, E_WARNING, "Property should be string");
		return FAILURE;
	}

	return phalcon_update_property_long(object, Z_STRVAL_P(property), Z_STRLEN_P(property), value);
}

/**
 * Checks whether obj is an object and updates zval property with another zval
 */
int phalcon_update_property_zval_zval(zval *object, const zval *property, zval *value){

	if (Z_TYPE_P(property) != IS_STRING) {
		php_error_docref(NULL, E_WARNING, "Property should be string");
		return FAILURE;
	}

	return phalcon_update_property_zval(object, Z_STRVAL_P(property), Z_STRLEN_P(property), value);
}

/**
 * Updates an array property
 */
int phalcon_update_property_array(zval *object, const char *property, uint32_t property_length, const zval *index, zval *value) {

	zval *tmp;
	int separated = 0;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		tmp = phalcon_read_property(object, property, property_length, PH_NOISY);

		Z_DELREF_P(tmp);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNT_P(tmp) > 1) {
			if (!Z_ISREF_P(tmp)) {
				zval *new_zv;
				ALLOC_ZVAL(new_zv);
				INIT_PZVAL_COPY(new_zv, tmp);
				tmp = new_zv;
				zval_copy_ctor(new_zv);
				Z_SET_REFCOUNT_P(tmp, 0);
				ZVAL_UNREF(tmp);
				separated = 1;
			}
		}

		/** Convert the value to array if not is an array */
		if (Z_TYPE_P(tmp) != IS_ARRAY) {
			if (separated) {
				convert_to_array(tmp);
			} else {
				zval *new_zv;
				ALLOC_ZVAL(new_zv);
				INIT_PZVAL_COPY(new_zv, tmp);
				tmp = new_zv;
				zval_copy_ctor(new_zv);
				Z_SET_REFCOUNT_P(tmp, 0);
				ZVAL_UNREF(tmp);
				array_init(tmp);
				separated = 1;
			}
		}

		Z_ADDREF_P(value);

		if (Z_TYPE_P(index) == IS_STRING) {
			zend_symtable_update(Z_ARRVAL_P(tmp), Z_STR_P(index), value);
		} else if (Z_TYPE_P(index) == IS_LONG) {
			zend_hash_index_update(Z_ARRVAL_P(tmp), Z_LVAL_P(index), value);
		} else if (Z_TYPE_P(index) == IS_NULL) {
			zend_hash_next_index_insert(Z_ARRVAL_P(tmp), value);
		}

		if (separated) {
			phalcon_update_property_zval(object, property, property_length, tmp);
		}
	}

	return SUCCESS;
}

/**
 * Multiple array-offset update
 */
int phalcon_update_property_array_multi(zval *object, const char *property, uint32_t property_length, zval *value, const char *types, int types_length, int types_count, ...) {
	va_list ap;
	zval *tmp_arr;
	int separated = 0;

	if (Z_TYPE_P(object) == IS_OBJECT) {

		tmp_arr = phalcon_read_property(object, property, property_length, PH_NOISY);

		Z_DELREF_P(tmp_arr);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNT_P(tmp_arr) > 1) {
			if (!Z_ISREF_P(tmp_arr)) {
				zval *new_zv;
				ALLOC_ZVAL(new_zv);
				INIT_PZVAL_COPY(new_zv, tmp_arr);
				tmp_arr = new_zv;
				zval_copy_ctor(new_zv);
				Z_SET_REFCOUNT_P(tmp_arr, 0);
				ZVAL_UNREF(tmp_arr);
				separated = 1;
			}
		}

		/** Convert the value to array if not is an array */
		if (Z_TYPE_P(tmp_arr) != IS_ARRAY) {
			if (separated) {
				convert_to_array(tmp_arr);
			} else {
				zval *new_zv;
				ALLOC_ZVAL(new_zv);
				INIT_PZVAL_COPY(new_zv, tmp_arr);
				tmp_arr = new_zv;
				zval_copy_ctor(new_zv);
				Z_SET_REFCOUNT_P(tmp_arr, 0);
				ZVAL_UNREF(tmp_arr);
				array_init(tmp_arr);
				separated = 1;
			}
		}

		va_start(ap, types_count);
		phalcon_array_update_multi_ex(tmp_arr, value, types, types_length, types_count, ap);
		va_end(ap);

		if (separated) {
			phalcon_update_property_zval(object, property, property_length, tmp_arr);
		}
	}

	return SUCCESS;
}

/**
 * Updates an array property using a string index
 */
int phalcon_update_property_array_string(zval *object, const char *property, uint32_t property_length, const char *index, uint32_t index_length, zval *value) {

	zval *tmp;
	int separated = 0;

	if (likely(Z_TYPE_P(object) == IS_OBJECT)) {

		tmp = phalcon_read_property(object, property, property_length, PH_NOISY);

		Z_DELREF_P(tmp);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNT_P(tmp) > 1) {
			if (!Z_ISREF_P(tmp)) {
				zval *new_zv;
				ALLOC_ZVAL(new_zv);
				INIT_PZVAL_COPY(new_zv, tmp);
				tmp = new_zv;
				zval_copy_ctor(new_zv);
				Z_SET_REFCOUNT_P(tmp, 0);
				ZVAL_UNREF(tmp);
				separated = 1;
			}
		}

		/** Convert the value to array if not is an array */
		if (Z_TYPE_P(tmp) != IS_ARRAY) {
			if (separated) {
				convert_to_array(tmp);
			} else {
				zval *new_zv;
				ALLOC_ZVAL(new_zv);
				INIT_PZVAL_COPY(new_zv, tmp);
				tmp = new_zv;
				zval_copy_ctor(new_zv);
				Z_SET_REFCOUNT_P(tmp, 0);
				ZVAL_UNREF(tmp);
				array_init(tmp);
				separated = 1;
			}
		}

		Z_ADDREF_P(value);

		zend_hash_str_update(Z_ARRVAL_P(tmp), index, index_length, value);

		if (separated) {
			phalcon_update_property_zval(object, property, property_length, tmp);
		}

	}

	return SUCCESS;
}

/**
 * Appends a zval value to an array property
 */
int phalcon_update_property_array_append(zval *object, const char *property, uint32_t property_length, zval *value) {

	zval *tmp;
	int separated = 0;

	if (Z_TYPE_P(object) != IS_OBJECT) {
		return SUCCESS;
	}

	tmp = phalcon_read_property(object, property, property_length, PH_NOISY);

	Z_DELREF_P(tmp);

	/** Separation only when refcount > 1 */
	if (Z_REFCOUNT_P(tmp) > 1) {
		if (!Z_ISREF_P(tmp)) {
			zval *new_zv;
			ALLOC_ZVAL(new_zv);
			INIT_PZVAL_COPY(new_zv, tmp);
			tmp = new_zv;
			zval_copy_ctor(new_zv);
			Z_SET_REFCOUNT_P(tmp, 0);
			ZVAL_UNREF(tmp);
			separated = 1;
		}
	}

	/** Convert the value to array if not is an array */
	if (Z_TYPE_P(tmp) != IS_ARRAY) {
		if (separated) {
			convert_to_array(tmp);
		} else {
			zval *new_zv;
			ALLOC_ZVAL(new_zv);
			INIT_PZVAL_COPY(new_zv, tmp);
			tmp = new_zv;
			zval_copy_ctor(new_zv);
			Z_SET_REFCOUNT_P(tmp, 0);
			ZVAL_UNREF(tmp);
			array_init(tmp);
			separated = 1;
		}
	}

	Z_ADDREF_P(value);
	add_next_index_zval(tmp, value);

	if (separated) {
		phalcon_update_property_zval(object, property, property_length, tmp);
	}

	return SUCCESS;
}

int phalcon_update_property_array_merge(zval *object, const char *property, uint32_t property_length, zval *values) {

	zval *tmp;
	int separated = 0;

	if (Z_TYPE_P(object) != IS_OBJECT) {
		return SUCCESS;
	}

	tmp = phalcon_read_property(object, property, property_length, PH_NOISY);

	Z_DELREF_P(tmp);

	/** Separation only when refcount > 1 */
	if (Z_REFCOUNT_P(tmp) > 1) {
		zval *new_zv;
		ALLOC_ZVAL(new_zv);
		INIT_PZVAL_COPY(new_zv, tmp);
		tmp = new_zv;
		zval_copy_ctor(new_zv);
		Z_SET_REFCOUNT_P(tmp, 0);
		separated = 1;
	}

	/** Convert the value to array if not is an array */
	if (Z_TYPE_P(tmp) != IS_ARRAY) {
		if (separated) {
			convert_to_array(tmp);
		} else {
			zval *new_zv;
			ALLOC_ZVAL(new_zv);
			INIT_PZVAL_COPY(new_zv, tmp);
			tmp = new_zv;
			zval_copy_ctor(new_zv);
			Z_SET_REFCOUNT_P(tmp, 0);
			array_init(tmp);
			separated = 1;
		}
	}

	phalcon_array_merge_recursive_n(tmp, values);

	if (separated) {
		phalcon_update_property_zval(object, property, property_length, tmp);
	}

	return SUCCESS;
}

/**
 * Appends every element of an array at the end of the array property
 */
int phalcon_update_property_array_merge_append(zval *object, const char *property, uint32_t property_length, zval *values) {

	zval *tmp;
	int separated = 0;

	if (Z_TYPE_P(object) != IS_OBJECT) {
		return SUCCESS;
	}

	tmp = phalcon_read_property(object, property, property_length, PH_NOISY);

	Z_DELREF_P(tmp);

	/** Separation only when refcount > 1 */
	if (Z_REFCOUNT_P(tmp) > 1) {
		zval *new_zv;
		ALLOC_ZVAL(new_zv);
		INIT_PZVAL_COPY(new_zv, tmp);
		tmp = new_zv;
		zval_copy_ctor(new_zv);
		Z_SET_REFCOUNT_P(tmp, 0);
		separated = 1;
	}

	/** Convert the value to array if not is an array */
	if (Z_TYPE_P(tmp) != IS_ARRAY) {
		if (separated) {
			convert_to_array(tmp);
		} else {
			zval *new_zv;
			ALLOC_ZVAL(new_zv);
			INIT_PZVAL_COPY(new_zv, tmp);
			tmp = new_zv;
			zval_copy_ctor(new_zv);
			Z_SET_REFCOUNT_P(tmp, 0);
			array_init(tmp);
			separated = 1;
		}
	}

	phalcon_merge_append(tmp, values);

	if (separated) {
		phalcon_update_property_zval(object, property, property_length, tmp);
	}

	return SUCCESS;
}

/**
 * Intializes an object property with an empty array
 */
int phalcon_update_property_empty_array(zval *object, const char *property_name, uint32_t property_length) {

	zval *empty_array;

	PHALCON_ALLOC_GHOST_ZVAL(empty_array);
	array_init(empty_array);

	return phalcon_update_property_zval(object, property_name, property_length, empty_array);
}

/**
 * Checks if an array property exists on object
 */
int phalcon_isset_property_array(zval *object, const char *property, uint32_t property_length, const zval *index) {

	zval *tmp;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		tmp = phalcon_read_property(object, property, property_length, PH_NOISY);
		return phalcon_array_isset(tmp, index);
	}

	return 0;
}

/**
 * Reads a array property from an object
 */
zval *phalcon_read_property_array(zval *object, const char *property, size_t property_length, const zval *index) {

	zval *tmp, *retval;

	if ((tmp = phalcon_read_property(object, property, property_length, PH_NOISY)) == NULL || !phalcon_array_isset_fetch(&retval, tmp, index)) {
		retval = &EG(uninitialized_zval);
	}

	return retval;
}

/**
 * Unsets an index in an array property
 */
int phalcon_unset_property_array(zval *object, const char *property, uint32_t property_length, const zval *index) {

	zval *tmp;
	int separated = 0;

	if (Z_TYPE_P(object) == IS_OBJECT) {
		tmp = phalcon_read_property(object, property, property_length, PH_NOISY);
		Z_DELREF_P(tmp);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNT_P(tmp) > 1) {
			if (!Z_ISREF_P(tmp)) {
				zval *new_zv;
				ALLOC_ZVAL(new_zv);
				INIT_PZVAL_COPY(new_zv, tmp);
				tmp = new_zv;
				zval_copy_ctor(new_zv);
				Z_SET_REFCOUNT_P(tmp, 0);
				ZVAL_UNREF(tmp);
				separated = 1;
			}
		}

		phalcon_array_unset(tmp, index, PH_SEPARATE);

		if (separated) {
			phalcon_update_property_zval(object, property, property_length, tmp);
		}
	}

	return SUCCESS;
}

/**
 * Check if a method is implemented on certain object
 */
int phalcon_method_exists(const zval *object, const zval *method_name){

	char *lcname = zend_str_tolower_dup(Z_STRVAL_P(method_name), Z_STRLEN_P(method_name));
	int res = phalcon_method_exists_ex(object, lcname, Z_STRLEN_P(method_name)+1);
	efree(lcname);
	return res;
}

/**
 * Check if method exists on certain object using explicit char param
 *
 * @param object
 * @param method_name
 * @param method_length strlen(method_name)+1
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
 * @param method_length strlen(method_name)+1
 */
int phalcon_method_exists_ce_ex(const zend_class_entry *ce, const char *method_name, uint32_t method_len)
{
	return (ce && zend_hash_str_exists(&ce->function_table, method_name, method_len)) ? SUCCESS : FAILURE;
}

/**
 * Query a static property value from a zend_class_entry
 */
int phalcon_read_static_property(zval *result, const char *class_name, uint32_t class_length, const char *property_name, uint32_t property_length){
	zend_class_entry *ce;
	if ((ce = zend_lookup_class(zend_string_init(class_name, class_length, 0))) != NULL) {
		ZVAL_ZVAL(result, phalcon_read_static_property_ce(ce, property_name, property_length), 1, 0);
	}

	return FAILURE;
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
			HashPosition pos;
			zval *item;
			int i = 0;

			if (likely(param_count) <= 10) {
				params_ptr = static_params;
			} else {
				params_arr = emalloc(param_count * sizeof(zval*));
				params_ptr = params_arr;
			}

			for (
				zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(params), &pos);
				(item = zend_hash_get_current_data_ex(Z_ARRVAL_P(params), &pos)) != NULL;
				zend_hash_move_forward_ex(Z_ARRVAL_P(params), &pos), ++i
			) {
				params_ptr[i] = item;
			}
		} else {
			params_ptr = NULL;
		}

		outcome = phalcon_call_class_method_aparams(NULL, return_value, Z_OBJCE_P(return_value), phalcon_fcall_method, "__construct", 11, param_count, params_ptr);

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
		return FAILURE;
	}

	return phalcon_create_instance_params_ce(return_value, ce, params);
}

/**
 * Increments an object property
 */
int phalcon_property_incr(zval *object, const char *property_name, uint32_t property_length){

	zval *tmp = NULL;
	zend_class_entry *ce;
	int separated = 0;

	if (unlikely(Z_TYPE_P(object) != IS_OBJECT)) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object");
		return FAILURE;
	}

	ce = Z_OBJCE_P(object);
	if (ce->parent) {
		ce = phalcon_lookup_class_ce(ce, property_name, property_length);
	}

	tmp = phalcon_read_property(object, property_name, property_length, 0);
	if (tmp) {

		Z_DELREF_P(tmp);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNT_P(tmp) > 1) {
			if (!Z_ISREF_P(tmp)) {
				zval *new_zv;
				ALLOC_ZVAL(new_zv);
				INIT_PZVAL_COPY(new_zv, tmp);
				tmp = new_zv;
				zval_copy_ctor(new_zv);
				Z_SET_REFCOUNT_P(tmp, 0);
				ZVAL_UNREF(tmp);
				separated = 1;
			}
		}

		phalcon_increment(tmp);

		if (separated) {
			phalcon_update_property_zval(object, property_name, property_length, tmp);
		}
	}

	return SUCCESS;
}

/**
 * Decrements an object property
 */
int phalcon_property_decr(zval *object, const char *property_name, uint32_t property_length){

	zval *tmp = NULL;
	zend_class_entry *ce;
	int separated = 0;

	if (unlikely(Z_TYPE_P(object) != IS_OBJECT)) {
		php_error_docref(NULL, E_WARNING, "Attempt to assign property of non-object");
		return FAILURE;
	}

	ce = Z_OBJCE_P(object);
	if (ce->parent) {
		ce = phalcon_lookup_class_ce(ce, property_name, property_length);
	}

	tmp = phalcon_read_property(object, property_name, property_length, 0);
	if (tmp) {

		Z_DELREF_P(tmp);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNT_P(tmp) > 1) {
			if (!Z_ISREF_P(tmp)) {
				SEPARATE_ZVAL(tmp);
				separated = 1;
			}
		}

		phalcon_decrement(tmp);

		if (separated) {
			phalcon_update_property_zval(object, property_name, property_length, tmp);
		}
	}

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
