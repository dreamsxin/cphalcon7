
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

#ifndef PHALCON_KERNEL_OBJECT_H
#define PHALCON_KERNEL_OBJECT_H

#include "php_phalcon.h"
#include "kernel/main.h"

static inline int is_phalcon_class(const zend_class_entry *ce)
{
	return ce->type == ZEND_INTERNAL_CLASS
		 && ce->info.internal.module->module_number == phalcon_module_entry.module_number;
}

/** Class Retrieving/Checking */
zend_class_entry *phalcon_class_exists(const zval *class_name, int autoload);
zend_class_entry *phalcon_class_exists_ex(const zval *class_name, int autoload);
zend_class_entry *phalcon_class_str_exists(const char *class_name, uint32_t class_len, int autoload);
int phalcon_interface_exists(const zval *interface_name, int autoload);
void phalcon_get_class(zval *result, const zval *object, int lower);
void phalcon_get_class_ns(zval *result, const zval *object, int lower);
void phalcon_get_ns_class(zval *result, const zval *object, int lower);
void phalcon_get_called_class(zval *return_value);
void phalcon_get_parent_class(zval *result, const zval *object, int lower);
void phalcon_get_object_vars(zval *result, zval *object, int check_access);
void phalcon_get_object_members(zval *result, zval *object, int check_access);
void phalcon_get_class_methods(zval *result, zval *object, int check_access);
zend_class_entry *phalcon_fetch_str_class(const char *class_name, uint32_t class_len, int fetch_type);
zend_class_entry *phalcon_fetch_class(const zval *class_name, int fetch_type);
zend_class_entry *phalcon_fetch_self_class();
zend_class_entry *phalcon_fetch_parent_class();
zend_class_entry *phalcon_fetch_static_class();
zend_class_entry *phalcon_get_internal_ce(const char *class_name, unsigned int class_name_len);

#define PHALCON_GET_CLASS_CONSTANT(return_value, ce, const_name) \
	do { \
		if (FAILURE == phalcon_get_class_constant(return_value, ce, const_name, strlen(const_name)+1)) { \
			PHALCON_MM_RESTORE(); \
			return; \
		} \
	} while (0)

/** Class constants */
int phalcon_get_class_constant(zval *return_value, const zend_class_entry *ce, const char *constant_name, uint32_t constant_length);

/** Cloning */
int phalcon_clone(zval *destination, zval *obj);
int phalcon_instance_of(zval *result, const zval *object, const zend_class_entry *ce);
int phalcon_is_instance_of(zval *object, const char *class_name, unsigned int class_length);
int phalcon_instance_of_ev(const zval *object, const zend_class_entry *ce);
int phalcon_zval_is_traversable(zval *object);

/** Method exists */
int phalcon_method_exists(const zval *object, const zval *method_name);
int phalcon_internal_method_exists(const zval *object, const zval *method_name);
int phalcon_method_exists_ex(const zval *object, const char *method_name, uint32_t method_len);
int phalcon_method_exists_ce(const zend_class_entry *ce, const zval *method_name);
int phalcon_method_exists_ce_ex(const zend_class_entry *ce, const char *method_name, uint32_t method_len);

/** Isset properties */
int phalcon_property_exists(zval *object, const char *property_name, uint32_t property_length, int flags);
#define phalcon_isset_property(object, property_name, property_length) phalcon_property_exists(object, property_name, property_length, PH_BOTH)

/**
 * Checks if string property exists on object
 */
static inline int phalcon_isset_property_zval(zval *object, const zval *property)
{
	if (Z_TYPE_P(property) == IS_STRING) {
		return phalcon_isset_property(object, Z_STRVAL_P(property), Z_STRLEN_P(property));
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
                if (zend_hash_str_exists(&ce->properties_info, property_name, property_length)) {
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

/** Reading properties */
int phalcon_read_property(zval *result, zval *object, const char *property_name, uint32_t property_length, int flags);

static inline int phalcon_return_property(zval *result, zval *object, const char *property_name, uint32_t property_length)
{
	return phalcon_read_property(result, object, property_name, property_length, PH_COPY);
}

static inline int phalcon_read_object_property(zval *result, zend_object *object, const char *property_name, uint32_t property_length, int flags)
{
	zval obj = {};
	ZVAL_OBJ(&obj, object);

	return phalcon_read_property(result, &obj, property_name, property_length, flags);
}

static inline int phalcon_read_property_zval(zval *result, zval *object, zval *property, int flags)
{
	if (unlikely(Z_TYPE_P(property) != IS_STRING)) {
		if ((flags & PH_NOISY) == PH_NOISY) {
			php_error_docref(NULL, E_NOTICE, "Cannot access empty property %d", Z_TYPE_P(property));
		}

		ZVAL_NULL(result);
		return FAILURE;
	}

	return phalcon_read_property(result, object, Z_STRVAL_P(property), Z_STRLEN_P(property), flags);
}

static inline int phalcon_return_property_zval(zval *result, zval *object, zval *property)
{
	if (unlikely(Z_TYPE_P(property) != IS_STRING)) {
		ZVAL_NULL(result);
		return FAILURE;
	}

	return phalcon_read_property(result, object, Z_STRVAL_P(property), Z_STRLEN_P(property), PH_COPY);
}

/** Updating properties */
int phalcon_update_property(zval *obj, const char *property_name, uint32_t property_length, zval *value);
int phalcon_update_property_long(zval *obj, const char *property_name, uint32_t property_length, long value);
int phalcon_update_property_str(zval *object, const char *property_name, uint32_t property_length, const char *str, uint32_t str_length);
int phalcon_update_property_bool(zval *obj, const char *property_name, uint32_t property_length, int value);
int phalcon_update_property_null(zval *obj, const char *property_name, uint32_t property_length);
int phalcon_update_property_string_zval(zval *obj, zend_string *property, zval *value);
int phalcon_update_property_zval_null(zval *obj, const zval *property);
int phalcon_update_property_zval_long(zval *obj, const zval *property, int value);
int phalcon_update_property_zval_zval(zval *obj, const zval *property, zval *value);
int phalcon_update_property_empty_array(zval *object, const char *property, uint32_t property_length);

/** Updating array properties */
int phalcon_update_property_array(zval *object, const char *property, uint32_t property_length, const zval *index, zval *value);
int phalcon_update_property_array_multi(zval *object, const char *property, uint32_t property_length, zval *value, const char *types, int types_length, int types_count, ...);
int phalcon_update_property_array_str(zval *object, const char *property, uint32_t property_length, const char *index, uint32_t index_length, zval *value);
int phalcon_update_property_array_string(zval *object, const char *property, uint32_t property_length, zend_string *index, zval *value);
int phalcon_update_property_array_append(zval *object, const char *property, uint32_t property_length, zval *value);
int phalcon_update_property_array_merge(zval *object, const char *property, uint32_t property_length, zval *values);
int phalcon_update_property_array_merge_append(zval *object, const char *property, uint32_t property_length, zval *values);

/** Increment/Decrement properties */
int phalcon_property_incr(zval *object, const char *property_name, uint32_t property_length);
int phalcon_property_decr(zval *object, const char *property_name, uint32_t property_length);

/** Unset Array properties */
int phalcon_isset_property_array(zval *object, const char *property, uint32_t property_length, const zval *index);
int phalcon_read_property_array(zval *return_value, zval *object, const char *property_name, size_t property_length, const zval *index, int flags);
int phalcon_unset_property_array(zval *object, const char *property, uint32_t property_length, const zval *index);

static inline int phalcon_isset_property_zval_array(zval *object, const zval *property, const zval *index)
{
	if (Z_TYPE_P(property) == IS_STRING) {
		return phalcon_isset_property_array(object, Z_STRVAL_P(property), Z_STRLEN_P(property), index);
	}

	return 0;
}

/** Static properties */
int phalcon_read_static_property(zval *return_value, const char *class_name, uint32_t class_length, const char *property_name, uint32_t property_length, int flags);
int phalcon_read_static_property_ce(zval *return_value, zend_class_entry *ce, const char *property, uint32_t len, int flags);
int phalcon_read_static_property_array_ce(zval *return_value, zend_class_entry *ce, const char *property_name, uint32_t property_length, const zval *index, int flags);
int phalcon_update_static_property_array_ce(zend_class_entry *ce, const char *property, uint32_t property_length, const zval *index, zval *value);
int phalcon_update_static_property_array_multi_ce(zend_class_entry *ce, const char *property, uint32_t property_length, zval *value, const char *types, int types_length, int types_count, ...);
int phalcon_update_static_property_ce(zend_class_entry *ce, const char *name, uint32_t len, zval *value);
int phalcon_update_static_property_bool_ce(zend_class_entry *ce, const char *property, uint32_t len, int value);
int phalcon_update_static_property_empty_array_ce(zend_class_entry *ce, const char *name, uint32_t len);
int phalcon_update_static_property_array_append_ce(zend_class_entry *ce, const char *name, uint32_t len, zval *value);
int phalcon_static_property_incr_ce(zend_class_entry *ce, const char *property, uint32_t len);
int phalcon_static_property_decr_ce(zend_class_entry *ce, const char *property, uint32_t len);

/**
 * Update a static property
 */
static inline int phalcon_update_static_property(const char *class_name, uint32_t class_length, const char *name, uint32_t name_length, zval *value)
{
	zend_class_entry *ce;
	if ((ce = zend_lookup_class(zend_string_init(class_name, class_length, 0))) != NULL) {
		return phalcon_update_static_property_ce(ce, name, name_length, value);
	}

	return FAILURE;
}


/** Create instances */
int phalcon_create_instance_params_ce(zval *return_value, zend_class_entry *ce, zval *params);
int phalcon_create_instance(zval *return_value, const zval *class_name);
int phalcon_create_instance_params(zval *return_value, const zval *class_name, zval *params);

#define PHALCON_OBJECT_INIT(object, ce) \
	if (UNEXPECTED(object_init_ex(object, ce) != SUCCESS)) {  \
		return;  \
	}

/** Checks if property access on object */
int phalcon_check_property_access(zval *object, const char *property_name, uint32_t property_length, int access);

static inline int phalcon_check_property_access_zval(zval *object, const zval *property, int access)
{
	if (Z_TYPE_P(property) == IS_STRING) {
		return phalcon_check_property_access(object, Z_STRVAL_P(property), Z_STRLEN_P(property) + 1, access);
	}

	return 0;
}

int phalcon_property_isset_fetch(zval *fetched, zval *object, const char *property_name, size_t property_length, int flags);
int phalcon_property_array_isset_fetch(zval *fetched, zval *object, const char *property_name, size_t property_length, const zval *index, int flags);
int phalcon_property_array_pop(zval *fetched, zval *object, const char *property, size_t property_length);
int phalcon_property_array_last(zval *fetched, zval *object, const char *property, size_t property_length, int flags);

static inline int phalcon_property_isset_fetch_zval(zval *fetched, zval *object, zval *property, int flags)
{
	return phalcon_property_isset_fetch(fetched, object, Z_STRVAL_P(property), Z_STRLEN_P(property), flags);
}

static inline int phalcon_property_array_isset_fetch_zval(zval *fetched, zval *object, zval *property, const zval *index, int flags)
{
	return phalcon_property_array_isset_fetch(fetched, object, Z_STRVAL_P(property), Z_STRLEN_P(property), index, flags);
}

#define PHALCON_PROPERTY_IS_PUBLIC(object, property) \
	 phalcon_check_property_access(object, property, strlen(property), ZEND_ACC_PUBLIC)

#define PHALCON_PROPERTY_IS_PROTECTED(object, property) \
	 phalcon_check_property_access(object, property, strlen(property), ZEND_ACC_PROTECTED)

#define PHALCON_PROPERTY_IS_PRIVATE(object, property) \
	 phalcon_check_property_access(object, property, strlen(property), ZEND_ACC_PRIVATE)

#define PHALCON_PROPERTY_IS_PUBLIC_ZVAL(object, property) \
	 phalcon_check_property_access_zval(object, property, ZEND_ACC_PUBLIC)

#define PHALCON_PROPERTY_IS_PROTECTED_ZVAL(object, property) \
	 phalcon_check_property_access_zval(object, property, ZEND_ACC_PROTECTED)

#define PHALCON_PROPERTY_IS_PRIVATE_ZVAL(object, property) \
	 phalcon_check_property_access_zval(object, property, ZEND_ACC_PRIVATE)

#endif /* PHALCON_KERNEL_OBJECT_H */
