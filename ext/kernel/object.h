
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

/** Class Retrieving/Checking */
zend_class_entry *phalcon_class_exists(const zval *class_name, int autoload) PHALCON_ATTR_NONNULL;
zend_class_entry *phalcon_class_exists_ex(const zval *class_name, int autoload) PHALCON_ATTR_NONNULL;
zend_class_entry *phalcon_class_str_exists(const char *class_name, uint32_t class_len, int autoload) PHALCON_ATTR_NONNULL;
int phalcon_interface_exists(const zval *interface_name, int autoload) PHALCON_ATTR_NONNULL;
void phalcon_get_class(zval *result, const zval *object, int lower) PHALCON_ATTR_NONNULL;
void phalcon_get_class_ns(zval *result, const zval *object, int lower) PHALCON_ATTR_NONNULL;
void phalcon_get_ns_class(zval *result, const zval *object, int lower) PHALCON_ATTR_NONNULL;
void phalcon_get_called_class(zval *return_value) PHALCON_ATTR_NONNULL;
void phalcon_get_parent_class(zval *result, const zval *object, int lower) PHALCON_ATTR_NONNULL;
void phalcon_get_object_vars(zval *result, zval *object, int check_access) PHALCON_ATTR_NONNULL;
void phalcon_get_class_methods(zval *result, zval *object, int check_access) PHALCON_ATTR_NONNULL;
zend_class_entry* phalcon_fetch_class(const zval *class_name) PHALCON_ATTR_NONNULL;
zend_class_entry* phalcon_fetch_self_class();
zend_class_entry* phalcon_fetch_parent_class();
zend_class_entry* phalcon_fetch_static_class();

#define PHALCON_GET_CLASS_CONSTANT(return_value, ce, const_name) \
	do { \
		if (FAILURE == phalcon_get_class_constant(return_value, ce, const_name, strlen(const_name)+1)) { \
			PHALCON_MM_RESTORE(); \
			return; \
		} \
	} while (0)
/** Class constants */
int phalcon_get_class_constant(zval *return_value, const zend_class_entry *ce, const char *constant_name, uint32_t constant_length) PHALCON_ATTR_NONNULL;

/** Cloning */
int phalcon_clone(zval *destination, zval *obj) PHALCON_ATTR_NONNULL;
int phalcon_instance_of(zval *result, const zval *object, const zend_class_entry *ce);
int phalcon_is_instance_of(zval *object, const char *class_name, unsigned int class_length);
int phalcon_instance_of_ev(const zval *object, const zend_class_entry *ce);
int phalcon_zval_is_traversable(zval *object);

/** Method exists */
int phalcon_method_exists(const zval *object, const zval *method_name) PHALCON_ATTR_NONNULL;
int phalcon_method_exists_ex(const zval *object, const char *method_name, uint32_t method_len) PHALCON_ATTR_NONNULL;
int phalcon_method_exists_ce(const zend_class_entry *ce, const zval *method_name) PHALCON_ATTR_NONNULL;
int phalcon_method_exists_ce_ex(const zend_class_entry *ce, const char *method_name, uint32_t method_len) PHALCON_ATTR_NONNULL;

/** Isset properties */
int phalcon_isset_property(zval *object, const char *property_name, uint32_t property_length) PHALCON_ATTR_NONNULL;

/**
 * Checks if string property exists on object
 */
PHALCON_ATTR_NONNULL static inline int phalcon_isset_property_zval(zval *object, const zval *property)
{
	if (Z_TYPE_P(property) == IS_STRING) {
		return phalcon_isset_property(object, Z_STRVAL_P(property), Z_STRLEN_P(property) + 1);
	}

	return 0;
}

/** Reading properties */
zval* phalcon_fetch_property_this(zval *object, const char *property_name, uint32_t property_length, int silent);
int phalcon_read_property(zval **result, zval *object, const char *property_name, uint32_t property_length, int silent);
int phalcon_read_property_zval(zval **result, zval *object, const zval *property, int silent);

/**
 * Reads a property from this_ptr (with pre-calculated key)
 * Variables must be defined in the class definition. This function ignores magic methods or dynamic properties
 */
PHALCON_ATTR_NONNULL static inline int phalcon_read_property_this(zval **result, zval *object, const char *property_name, uint32_t property_length, int silent)
{
	zval *tmp = phalcon_fetch_property_this(object, property_name, property_length, silent);
	if (EXPECTED(tmp != NULL)) {
		*result = tmp;
		Z_ADDREF_P(*result);
		return SUCCESS;
	}

	ALLOC_INIT_ZVAL(*result);
	return FAILURE;
}

PHALCON_ATTR_NONNULL static inline zval* phalcon_fetch_nproperty_this(zval *object, const char *property_name, uint32_t property_length, int silent)
{
	zval *result = phalcon_fetch_property_this(object, property_name, property_length, silent);
	return result ? result : &EG(uninitialized_zval);
}

PHALCON_ATTR_NONNULL static inline zval* phalcon_fetch_nproperty_this_zval(zval *object, const zval *property, int silent)
{
	return phalcon_fetch_nproperty_this(object, Z_STRVAL_P(property), Z_STRLEN_P(property), silent);
}

/**
 * Returns an object's member
 */
PHALCON_ATTR_NONNULL3(1,2,3)
static inline int phalcon_return_property(zval *return_value, zval *object, const char *property_name, uint32_t property_length)
{
	zval *tmp = phalcon_fetch_nproperty_this(object, property_name, property_length, PH_NOISY);
	if (tmp) {
		ZVAL_ZVAL(return_value, tmp, 1, 0);
	} else {
		ZVAL_NULL(return_value);
	}
	return FAILURE;
}


/** Updating properties */
int phalcon_update_property_long(zval *obj, const char *property_name, uint32_t property_length, long value) PHALCON_ATTR_NONNULL;
int phalcon_update_property_string(zval *object, const char *property_name, uint32_t property_length, const char *str, uint32_t str_length) PHALCON_ATTR_NONNULL;
int phalcon_update_property_bool(zval *obj, const char *property_name, uint32_t property_length, int value) PHALCON_ATTR_NONNULL;
int phalcon_update_property_null(zval *obj, const char *property_name, uint32_t property_length) PHALCON_ATTR_NONNULL;
int phalcon_update_property_zval(zval *obj, const char *property_name, uint32_t property_length, zval *value) PHALCON_ATTR_NONNULL;
int phalcon_update_property_zval_long(zval *obj, const zval *property, int value) PHALCON_ATTR_NONNULL;
int phalcon_update_property_zval_zval(zval *obj, const zval *property, zval *value) PHALCON_ATTR_NONNULL;
int phalcon_update_property_empty_array(zval *object, const char *property, uint32_t property_length) PHALCON_ATTR_NONNULL;

int phalcon_update_property_this(zval *object, const char *property_name, uint32_t property_length, zval *value);

/** Updating array properties */
int phalcon_update_property_array(zval *object, const char *property, uint32_t property_length, const zval *index, zval *value) PHALCON_ATTR_NONNULL;
int phalcon_update_property_array_multi(zval *object, const char *property, uint32_t property_length, zval *value, const char *types, int types_length, int types_count, ...);
int phalcon_update_property_array_string(zval *object, const char *property, uint32_t property_length, const char *index, uint32_t index_length, zval *value) PHALCON_ATTR_NONNULL;
int phalcon_update_property_array_append(zval *object, const char *property, uint32_t property_length, zval *value) PHALCON_ATTR_NONNULL;
int phalcon_update_property_array_merge(zval *object, const char *property, uint32_t property_length, zval *values) PHALCON_ATTR_NONNULL;
int phalcon_update_property_array_merge_append(zval *object, const char *property, uint32_t property_length, zval *values) PHALCON_ATTR_NONNULL;

/** Increment/Decrement properties */
int phalcon_property_incr(zval *object, const char *property_name, uint32_t property_length) PHALCON_ATTR_NONNULL;
int phalcon_property_decr(zval *object, const char *property_name, uint32_t property_length) PHALCON_ATTR_NONNULL;

/** Unset Array properties */
int phalcon_unset_property_array(zval *object, const char *property, uint32_t property_length, const zval *index) PHALCON_ATTR_NONNULL;

/** Static properties */
int phalcon_read_static_property(zval **result, const char *class_name, uint32_t class_length, const char *property_name, uint32_t property_length) PHALCON_ATTR_NONNULL;
int phalcon_read_class_property(zval **result, int type, const char *property, uint32_t len) PHALCON_ATTR_NONNULL;
int phalcon_update_static_property_array_multi_ce(zend_class_entry *ce, const char *property, uint32_t property_length, zval *value, const char *types, int types_length, int types_count, ...);

PHALCON_ATTR_NONNULL static inline zval* phalcon_fetch_static_property_ce(zend_class_entry *ce, const char *property, uint32_t len)
{
	return zend_read_static_property(ce, property, len, (zend_bool)ZEND_FETCH_CLASS_SILENT);
}

PHALCON_ATTR_NONNULL static inline int phalcon_read_static_property_ce(zval **result, zend_class_entry *ce, const char *property, uint32_t len)
{
	*result = phalcon_fetch_static_property_ce(ce, property, len);
	if (*result) {
		Z_ADDREF_P(*result);
		return SUCCESS;
	}

	return FAILURE;
}

PHALCON_ATTR_NONNULL static inline int phalcon_update_static_property_ce(zend_class_entry *ce, const char *name, uint32_t len, zval *value)
{

	return zend_update_static_property(ce, name, len, value);
}

/**
 * Update a static property
 */
PHALCON_ATTR_NONNULL static inline int phalcon_update_static_property(const char *class_name, uint32_t class_length, const char *name, uint32_t name_length, zval *value)
{
	zend_class_entry *ce;
	if ((ce = zend_lookup_class(zend_string_init(class_name, class_length, 0))) != NULL) {
		return phalcon_update_static_property_ce(ce, name, name_length, value);
	}

	return FAILURE;
}


/** Create instances */
int phalcon_create_instance_params_ce(zval *return_value, zend_class_entry *ce, zval *params) PHALCON_ATTR_NONNULL2(1, 2);
int phalcon_create_instance(zval *return_value, const zval *class_name) PHALCON_ATTR_NONNULL;
int phalcon_create_instance_params(zval *return_value, const zval *class_name, zval *params) PHALCON_ATTR_NONNULL2(1, 2);

/** Create closures */
int phalcon_create_closure_ex(zval *return_value, zval *this_ptr, zend_class_entry *ce, const char *method_name, uint32_t method_length);

/** Checks if property access on object */
int phalcon_check_property_access(zval *object, const char *property_name, uint32_t property_length, int access) PHALCON_ATTR_NONNULL;

PHALCON_ATTR_NONNULL static inline int phalcon_check_property_access_zval(zval *object, const zval *property, int access)
{
	if (Z_TYPE_P(property) == IS_STRING) {
		return phalcon_check_property_access(object, Z_STRVAL_P(property), Z_STRLEN_P(property) + 1, access);
	}

	return 0;
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
