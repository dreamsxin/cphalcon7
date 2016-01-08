
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

#ifndef PHALCON_KERNEL_MAIN_H
#define PHALCON_KERNEL_MAIN_H

#include "php_phalcon.h"

#include <Zend/zend_exceptions.h>
#include <Zend/zend_interfaces.h>

#include <ext/spl/spl_exceptions.h>
#include <ext/spl/spl_iterators.h>

#include "kernel/memory.h"
#include "kernel/backtrace.h"

/** Main macros */
#define PH_DEBUG 0

#define PH_NOISY 256
#define PH_SILENT 1024
#define PH_READONLY 4096

#define PH_NOISY_CC PH_NOISY
#define PH_SILENT_CC PH_SILENT

#define PH_SEPARATE 256
#define PH_COPY 1024
#define PH_CTOR 4096

#define SL(str)   ZEND_STRL(str)
#define SS(str)   ZEND_STRS(str)
#define IS(str)   (phalcon_interned_##str)
#define ISV(str)  (phalcon_interned_##str)->val
#define ISL(str)  (phalcon_interned_##str)->val, (sizeof(#str)-1)
#define ISS(str)  (phalcon_interned_##str)->val, (sizeof(#str))
#define SSL(str)   zend_string_init(SL(str), 0)
#define SSS(str)   zend_string_init(SS(str), 0)

/* Startup functions */
void php_phalcon_init_globals(zend_phalcon_globals *phalcon_globals);
zend_class_entry *phalcon_register_internal_interface_ex(zend_class_entry *orig_ce, zend_class_entry *parent_ce);

/* Globals functions */
zval* phalcon_get_global_str(const char *global, unsigned int global_length);
zval* phalcon_get_global(zend_string *name);

int phalcon_is_callable(zval *var);
int phalcon_function_exists_ex(const char *func_name, unsigned int func_len);

PHALCON_ATTR_NONNULL static inline zend_function *phalcon_fetch_function_str(const char *function_name, unsigned int function_len)
{
	return zend_hash_str_find_ptr(EG(function_table), function_name, function_len+1);
}

PHALCON_ATTR_NONNULL static inline zend_function *phalcon_fetch_function(zend_string *function_name)
{
	return zend_hash_find_ptr(EG(function_table), function_name);
}

/* Count */
long int phalcon_fast_count_int(zval *value);
void phalcon_fast_count(zval *result, zval *array);
int phalcon_fast_count_ev(zval *array);

static inline int is_phalcon_class(const zend_class_entry *ce)
{
	return
			ce->type == ZEND_INTERNAL_CLASS
		 && ce->info.internal.module->module_number == phalcon_module_entry.module_number;
}

/* Fetch Parameters */
int phalcon_fetch_parameters(int num_args, int required_args, int optional_args, ...);

/* Compatibility macros for PHP 5.3 */
#ifndef INIT_PZVAL_COPY
 #define INIT_PZVAL_COPY(z, v) ZVAL_COPY_VALUE(z, v);\
  Z_SET_REFCOUNT_P(z, 1);\
  ZVAL_UNREF(z);
#endif

#ifndef ZVAL_COPY_VALUE
 #define ZVAL_COPY_VALUE(z, v)\
  (z)->value = (v)->value;\
  Z_TYPE_P(z) = Z_TYPE_P(v);
#endif

/** Return zval checking if it's needed to ctor */
#define RETURN_CCTOR(var) { \
		ZVAL_DUP(return_value, var); \
	} \
	PHALCON_MM_RESTORE(); \
	return;

/** Return zval checking if it's needed to ctor, without restoring the memory stack  */
#define RETURN_CCTORW(var) { \
		ZVAL_DUP(return_value, var); \
	} \
	return;

/** Return zval with always ctor */
#define RETURN_CTOR(var) { \
		RETVAL_ZVAL(var, 1, 0); \
	} \
	PHALCON_MM_RESTORE(); \
	return;

/** Return zval with always ctor, without restoring the memory stack */
#define RETURN_CTORW(var) { \
		RETVAL_ZVAL(var, 1, 0); \
	} \
	return;

/** Return this pointer */
#define RETURN_THIS() { \
		RETVAL_ZVAL(getThis(), 1, 0); \
	} \
	PHALCON_MM_RESTORE(); \
	return;

/** Return zval with always ctor, without restoring the memory stack */
#define RETURN_THISW() \
	RETURN_ZVAL(getThis(), 1, 0);

/**
 * Returns a zval in an object member
 */
#define RETURN_MEMBER(object, member_name) \
	phalcon_return_property(return_value, object, SL(member_name)); \
	return;

/**
 * Returns a zval in an object member
 */
#define RETURN_MM_MEMBER(object, member_name) \
	phalcon_return_property(return_value, object, SL(member_name)); \
	RETURN_MM();

#define RETURN_ON_FAILURE(what) \
	if (FAILURE == what) { \
		return;            \
	}

#define RETURN_MM_ON_FAILURE(what) \
	if (FAILURE == what) {    \
		PHALCON_MM_RESTORE(); \
		return;               \
	}

/** Return without change return_value */
#define RETURN_MM() PHALCON_MM_RESTORE(); return;

/** Return bool restoring memory frame */
#define RETURN_MM_BOOL(value) RETVAL_BOOL(value); RETURN_MM();

/** Return null restoring memory frame */
#define RETURN_MM_NULL() PHALCON_MM_RESTORE(); RETURN_NULL();

/** Return bool restoring memory frame */
#define RETURN_MM_FALSE PHALCON_MM_RESTORE(); RETURN_FALSE;
#define RETURN_MM_TRUE PHALCON_MM_RESTORE(); RETURN_TRUE;

/** Return string restoring memory frame */
#define RETURN_MM_STRING(str) PHALCON_MM_RESTORE(); RETURN_STRING(str);
#define RETURN_MM_EMPTY_STRING() PHALCON_MM_RESTORE(); RETURN_EMPTY_STRING();

/* Return long */
#define RETURN_MM_LONG(value) RETVAL_LONG(value); RETURN_MM();

/* Return double */
#define RETURN_MM_DOUBLE(value) RETVAL_DOUBLE(value); RETURN_MM();

/** Return empty array */
#define RETURN_EMPTY_ARRAY() array_init(return_value); return;
#define RETURN_MM_EMPTY_ARRAY() PHALCON_MM_RESTORE(); RETURN_EMPTY_ARRAY();

/** Get the current hash key without copying the hash key */
#define PHALCON_GET_HKEY(var, hash, hash_position) \
	do { \
		PHALCON_INIT_NVAR_PNULL(var); \
		phalcon_get_current_key(&var, hash, &hash_position); \
	} while (0)

/** Get current hash key copying the iterator if needed */
#define PHALCON_GET_IKEY(var, it) \
	do { \
		PHALCON_INIT_NVAR(var); \
		it->funcs->get_current_key(it, var); \
	} while (0)

#define PHALCON_GET_HVALUE(var) \
	PHALCON_OBS_NVAR(var); \
	var = *hd; \
	Z_TRY_ADDREF_P(var);

/** class/interface registering */
#define PHALCON_REGISTER_CLASS(ns, class_name, name, methods, flags) \
	{ \
		zend_class_entry ce; \
		INIT_NS_CLASS_ENTRY(ce, #ns, #class_name, methods); \
		phalcon_ ##name## _ce = zend_register_internal_class(&ce); \
		phalcon_ ##name## _ce->ce_flags |= flags;  \
	}

#define PHALCON_REGISTER_CLASS_EX(ns, class_name, lcname, parent_ce, methods, flags) \
	{ \
		zend_class_entry ce; \
		INIT_NS_CLASS_ENTRY(ce, #ns, #class_name, methods); \
		phalcon_ ##lcname## _ce = zend_register_internal_class_ex(&ce, parent_ce); \
		if (!phalcon_ ##lcname## _ce) { \
			fprintf(stderr, "Phalcon Error: Class to extend '%s' was not found when registering class '%s'\n", (parent_ce ? parent_ce->name->val : "(null)"), ZEND_NS_NAME(#ns, #class_name)); \
			return FAILURE; \
		} \
		phalcon_ ##lcname## _ce->ce_flags |= flags;  \
	}

#define PHALCON_REGISTER_INTERFACE(ns, classname, name, methods) \
	{ \
		zend_class_entry ce; \
		INIT_NS_CLASS_ENTRY(ce, #ns, #classname, methods); \
		phalcon_ ##name## _ce = zend_register_internal_interface(&ce); \
	}

#define PHALCON_REGISTER_INTERFACE_EX(ns, classname, lcname, parent_ce, methods) \
	{ \
		zend_class_entry ce; \
		INIT_NS_CLASS_ENTRY(ce, #ns, #classname, methods); \
		phalcon_ ##lcname## _ce = phalcon_register_internal_interface_ex(&ce, parent_ce); \
		if (!phalcon_ ##lcname## _ce) { \
			fprintf(stderr, "Can't register interface %s with parent %s\n", ZEND_NS_NAME(#ns, #classname), (parent_ce ? parent_ce->name->val : "(null)")); \
			return FAILURE; \
		} \
	}

/** Method declaration for API generation */
#define PHALCON_DOC_METHOD(class_name, method)

/** Low overhead parse/fetch parameters */
#ifndef PHALCON_RELEASE

#define phalcon_fetch_params(memory_grow, required_params, optional_params, ...) \
	if (memory_grow) { \
		zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL; \
		ASSUME(phalcon_globals_ptr != NULL); \
		if (unlikely(phalcon_globals_ptr->active_memory == NULL)) { \
			fprintf(stderr, "phalcon_fetch_params is called with memory_grow=1 but there is no active memory frame!\n"); \
			phalcon_print_backtrace(); \
		} \
		else if (unlikely(phalcon_globals_ptr->active_memory->func != __func__)) { \
			fprintf(stderr, "phalcon_fetch_params is called with memory_grow=1 but the memory frame was not created!\n"); \
			fprintf(stderr, "The frame was created by %s\n", phalcon_globals_ptr->active_memory->func); \
			fprintf(stderr, "Calling function: %s\n", __func__); \
			phalcon_print_backtrace(); \
		} \
	} \
	if (phalcon_fetch_parameters(ZEND_NUM_ARGS(), required_params, optional_params, __VA_ARGS__) == FAILURE) { \
		if (memory_grow) { \
			RETURN_MM_NULL(); \
		} \
		RETURN_NULL(); \
	} \

#else

#define phalcon_fetch_params(memory_grow, required_params, optional_params, ...) \
	if (phalcon_fetch_parameters(ZEND_NUM_ARGS(), required_params, optional_params, __VA_ARGS__) == FAILURE) { \
		if (memory_grow) { \
			RETURN_MM_NULL(); \
		} \
		RETURN_NULL(); \
	}

#endif

#define PHALCON_VERIFY_INTERFACE_EX(instance, interface_ce, exception_ce, restore_stack) \
	if (Z_TYPE_P(instance) != IS_OBJECT) { \
		zend_throw_exception_ex(exception_ce, 0, "Unexpected value type: expected object implementing %s, %s given", interface_ce->name->val, zend_zval_type_name(instance)); \
		if (restore_stack) { \
			PHALCON_MM_RESTORE(); \
		} \
		return; \
	} else { \
		if (!instanceof_function_ex(Z_OBJCE_P(instance), interface_ce, 1)) { \
			if (Z_TYPE_P(instance) != IS_OBJECT) { \
				zend_throw_exception_ex(exception_ce, 0, "Unexpected value type: expected object implementing %s, %s given", interface_ce->name->val, zend_zval_type_name(instance)); \
			} \
			else { \
				zend_throw_exception_ex(exception_ce, 0, "Unexpected value type: expected object implementing %s, object of type %s given", interface_ce->name->val, Z_OBJCE_P(instance)->name->val); \
			} \
			if (restore_stack) { \
				PHALCON_MM_RESTORE(); \
			} \
			return; \
		} \
	}

#define PHALCON_VERIFY_INTERFACE_OR_NULL_EX(pzv, interface_ce, exception_ce, restore_stack) \
	if (Z_TYPE_P(pzv) != IS_NULL && (Z_TYPE_P(pzv) != IS_OBJECT || !instanceof_function_ex(Z_OBJCE_P(pzv), interface_ce, 1))) { \
		if (Z_TYPE_P(pzv) != IS_OBJECT) { \
			zend_throw_exception_ex(exception_ce, 0, "Unexpected value type: expected object implementing %s or NULL, %s given", interface_ce->name->val, zend_zval_type_name(pzv)); \
		} \
		else { \
			zend_throw_exception_ex(exception_ce, 0, "Unexpected value type: expected object implementing %s or NULL, object of type %s given", interface_ce->name->val, Z_OBJCE_P(pzv)->name->val); \
		} \
		if (restore_stack) { \
			PHALCON_MM_RESTORE(); \
		} \
		return; \
	}

#define PHALCON_VERIFY_CLASS_EX(instance, class_ce, exception_ce, restore_stack) \
	if (Z_TYPE_P(instance) != IS_OBJECT || !instanceof_function_ex(Z_OBJCE_P(instance), class_ce, 0)) { \
		if (Z_TYPE_P(instance) != IS_OBJECT) { \
			zend_throw_exception_ex(exception_ce, 0, "Unexpected value type: expected object of type %s, %s given", class_ce->name->val, zend_zval_type_name(instance)); \
		} \
		else { \
			zend_throw_exception_ex(exception_ce, 0, "Unexpected value type: expected object of type %s, object of type %s given", class_ce->name->val, Z_OBJCE_P(instance)->name->val); \
		} \
		if (restore_stack) { \
			PHALCON_MM_RESTORE(); \
		} \
		return; \
	}

#define PHALCON_VERIFY_CLASS_OR_NULL_EX(pzv, class_ce, exception_ce, restore_stack) \
	if (Z_TYPE_P(pzv) != IS_NULL && (Z_TYPE_P(pzv) != IS_OBJECT || !instanceof_function_ex(Z_OBJCE_P(pzv), class_ce, 0))) { \
		if (Z_TYPE_P(pzv) != IS_OBJECT) { \
			zend_throw_exception_ex(exception_ce, 0, "Unexpected value type: expected object of type %s, %s given", class_ce->name->val, zend_zval_type_name(pzv)); \
		} \
		else { \
			zend_throw_exception_ex(exception_ce, 0, "Unexpected value type: expected object of type %s, object of type %s given", class_ce->name->val, Z_OBJCE_P(pzv)->name->val); \
		} \
		if (restore_stack) { \
			PHALCON_MM_RESTORE(); \
		} \
		return; \
	}

#define PHALCON_VERIFY_INTERFACE(instance, interface_ce)      PHALCON_VERIFY_INTERFACE_EX(instance, interface_ce, spl_ce_LogicException, 1)
#define PHALCON_VERIFY_INTERFACEW(instance, interface_ce)      PHALCON_VERIFY_INTERFACE_EX(instance, interface_ce, spl_ce_LogicException, 0)
#define PHALCON_VERIFY_INTERFACE_OR_NULL(pzv, interface_ce)   PHALCON_VERIFY_INTERFACE_OR_NULL_EX(pzv, interface_ce, spl_ce_LogicException, 1)
#define PHALCON_VERIFY_CLASS(instance, class_ce)              PHALCON_VERIFY_CLASS_EX(instance, class_ce, spl_ce_LogicException, 1)
#define PHALCON_VERIFY_CLASS_OR_NULL(pzv, class_ce)           PHALCON_VERIFY_CLASS_OR_NULL_EX(pzv, class_ce, spl_ce_LogicException, 1)

#define PHALCON_ENSURE_IS_STRING(pzv)    convert_to_string_ex(pzv)
#define PHALCON_ENSURE_IS_LONG(pzv)      convert_to_long_ex(pzv)
#define PHALCON_ENSURE_IS_DOUBLE(pzv)    convert_to_double_ex(pzv)
#define PHALCON_ENSURE_IS_BOOL(pzv)      convert_to_boolean_ex(pzv)
#define PHALCON_ENSURE_IS_ARRAY(pzv)     convert_to_array_ex(pzv)
#define PHALCON_ENSURE_IS_OBJECT(pzv)    convert_to_object_ex(pzv)
#define PHALCON_ENSURE_IS_NULL(pzv)      convert_to_null_ex(pzv)

void phalcon_clean_and_cache_symbol_table(zend_array *symbol_table);

#define PHALCON_CHECK_POINTER(v) if (!v) fprintf(stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__)
#define PHALCON_DEBUG_POINTER() fprintf(stderr, "%s:%d\n", __PRETTY_FUNCTION__, __LINE__)

#define PHALCON_GET_OBJECT_FROM_OBJ(object, object_struct) \
	((object_struct *) ((char *) (object) - XtOffsetOf(object_struct, obj)))

#define PHALCON_GET_OBJECT_FROM_ZVAL(zv, object_struct) \
	PHALCON_GET_OBJECT_FROM_OBJ(Z_OBJ_P(zv), object_struct)

#define phalcon_is_php_version(id) (PHP_VERSION_ID / 10 == id / 10 ?  1 : 0)

#endif /* PHALCON_KERNEL_MAIN_H */
