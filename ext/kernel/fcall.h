
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

#ifndef PHALCON_KERNEL_FCALL_H
#define PHALCON_KERNEL_FCALL_H

#include "php_phalcon.h"
#include "kernel/main.h"
#include "kernel/memory.h"

#include <Zend/zend_hash.h>
#include <Zend/zend.h>

typedef enum _phalcon_call_type {
	phalcon_fcall_parent,
	phalcon_fcall_self,
	phalcon_fcall_static,
	phalcon_fcall_ce,
	phalcon_fcall_method,
	phalcon_fcall_function
} phalcon_call_type;

/**
 * @addtogroup callfuncs Calling Functions
 * @{
 */
#define PHALCON_FUNC_STRLEN(x) (__builtin_constant_p(x) ? (sizeof(x)-1) : strlen(x))

#if defined(_MSC_VER)
#define PHALCON_PASS_CALL_PARAMS(x) x + 1
#define PHALCON_CALL_NUM_PARAMS(x) ((sizeof(x) - sizeof(x[0]))/sizeof(x[0]))
#define PHALCON_FETCH_VA_ARGS NULL,
#else
#define PHALCON_PASS_CALL_PARAMS(x) x
#define PHALCON_CALL_NUM_PARAMS(x) sizeof(x)/sizeof(zval *)
#define PHALCON_FETCH_VA_ARGS
#endif

#define PHALCON_CALL_ZVAL_FUNCTIONW(return_value_ptr, func_name, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_ON_FAILURE(phalcon_call_zval_func_aparams(return_value_ptr, func_name, PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_CALL_ZVAL_FUNCTION(return_value_ptr, func_name, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		PHALCON_OBSERVE_OR_NULLIFY_PPZV(return_value_ptr); \
		RETURN_MM_ON_FAILURE(phalcon_call_zval_func_aparams(return_value_ptr, func_name, PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_RETURN_CALL_ZVAL_FUNCTIONW(func, ...) \
	do { \
		zval *params_[] = {__VA_ARGS__}; \
		RETURN_ON_FAILURE(phalcon_call_zval_func_aparams(&return_value, func, PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_RETURN_CALL_ZVAL_FUNCTION(func, ...) \
	do { \
		zval *params_[] = {__VA_ARGS__}; \
		RETURN_MM_ON_FAILURE(phalcon_call_zval_func_aparams(&return_value, func, PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_CALL_METHOD_ARRAYW(return_value_ptr, object, method, params) \
	do { \
		RETURN_ON_FAILURE(phalcon_call_class_method_array(return_value_ptr, object, method, PHALCON_FUNC_STRLEN(method), params)); \
	} while (0)

#define PHALCON_CALL_METHOD_ARRAY(return_value_ptr, object, method, params) \
	do { \
		PHALCON_OBSERVE_OR_NULLIFY_PPZV(return_value_ptr); \
		RETURN_MM_ON_FAILURE(phalcon_call_class_method_array(return_value_ptr, object, method, PHALCON_FUNC_STRLEN(method), params)); \
	} while (0)

#define PHALCON_RETURN_CALL_PARENTW(class_entry, object, method, ...) \
	do { \
		RETURN_ON_FAILURE(phalcon_call_class_method_aparams(&return_value, object, class_entry, phalcon_fcall_parent, method, PHALCON_FUNC_STRLEN(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_RETURN_CALL_PARENT(class_entry, object, method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_MM_ON_FAILURE(phalcon_call_class_method_aparams(&return_value, object, class_entry, phalcon_fcall_parent, method, PHALCON_FUNC_STRLEN(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_CALL_STATICW(return_value_ptr, method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_ON_FAILURE(phalcon_call_class_method_aparams(return_value_ptr, NULL, NULL, phalcon_fcall_static, method, PHALCON_FUNC_STRLEN(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_CALL_STATIC(return_value_ptr, method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		PHALCON_OBSERVE_OR_NULLIFY_PPZV(return_value_ptr); \
		RETURN_MM_ON_FAILURE(phalcon_call_class_method_aparams(return_value_ptr, NULL, NULL, phalcon_fcall_static, method, PHALCON_FUNC_STRLEN(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_RETURN_CALL_STATICW(method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_ON_FAILURE(phalcon_call_class_method_aparams(&return_value, NULL, NULL, phalcon_fcall_static, method, PHALCON_FUNC_STRLEN(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_RETURN_CALL_STATIC(method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_MM_ON_FAILURE(phalcon_call_class_method_aparams(&return_value, NULL, NULL, phalcon_fcall_static, method, PHALCON_FUNC_STRLEN(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_RETURN_CALL_CE_STATICW(class_entry, method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_ON_FAILURE(phalcon_call_class_method_aparams(&return_value, NULL, class_entry, phalcon_fcall_ce, method, PHALCON_FUNC_STRLEN(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_RETURN_CALL_CE_STATIC(class_entry, method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_MM_ON_FAILURE(phalcon_call_class_method_aparams(&return_value, NULL, class_entry, phalcon_fcall_ce, method, PHALCON_FUNC_STRLEN(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

/** Use these functions to call functions in the PHP userland using an arbitrary zval as callable */
#define PHALCON_CALL_USER_FUNC_ARRAY(retval, handler, params) \
	do { \
		RETURN_MM_ON_FAILURE(phalcon_call_user_func_array(retval, handler, params)); \
	} while (0)

#define PHALCON_CALL_USER_FUNC_ARRAYW(retval, handler, params) \
	do { \
		RETURN_ON_FAILURE(phalcon_call_user_func_array(retval, handler, params)); \
	} while (0)

#define PHALCON_CALL_USER_FUNC_ARRAY_NOEX(retval, handler, params) \
	do { \
		RETURN_MM_ON_FAILURE(phalcon_call_user_func_array_noex(retval, handler, params)); \
	} while (0)

#define PHALCON_CALL_USER_FUNC_ARRAY_NOEXW(retval, handler, params) \
	do { \
		RETURN_ON_FAILURE(phalcon_call_user_func_array_noex(retval, handler, params)); \
	} while (0)

#define PHALCON_CALL_USER_FUNC(retval, handler) PHALCON_CALL_USER_FUNC_ARRAY(retval, handler, NULL)
#define PHALCON_CALL_USER_FUNCW(retval, handler) PHALCON_CALL_USER_FUNC_ARRAYW(retval, handler, NULL)

/**
 * @brief Checks if the class defines a constructor
 * @param ce Class entry
 * @return Whether the class defines a constructor
 */
int phalcon_has_constructor_ce(const zend_class_entry *ce);
int phalcon_call_user_function(zval **retval_ptr, zval *object, zend_class_entry *ce, phalcon_call_type type, zval *function_name, uint32_t param_count, zval *params[]);
int phalcon_call_func_aparams(zval **return_value, const char *func_name, uint func_length, uint param_count, zval *params[]);
int phalcon_call_zval_func_aparams(zval **return_value, zval *func_name, uint param_count, zval *params[]);

int phalcon_call_class_zval_method_aparams(zval **return_value, zval *object, zend_class_entry *ce, phalcon_call_type type, zval *method, uint param_count, zval *params[]);
int phalcon_call_class_method_aparams(zval **return_value, zval *object, zend_class_entry *ce, phalcon_call_type type, const char *method_name, uint method_len, uint param_count, zval *params[]);

int phalcon_call_class_zval_method_array(zval **retval_ptr, zval *object, zval *method, zval *params);

static inline int phalcon_call_class_method_array(zval **retval_ptr, zval *object, const char *method_name, uint method_length, zval *params)
{
	zval method;
	ZVAL_STRINGL(&method, method_name, method_length);

	return phalcon_call_class_zval_method_array(retval_ptr, object, &method, params);
}

/**
 * Replaces call_user_func_array avoiding function lookup
 */
int phalcon_call_user_func_array(zval *retval, zval *handler, zval *params);

/** Fast call_user_func_array/call_user_func */
static inline int phalcon_call_user_func_array_noex(zval *retval, zval *handler, zval *params){

	if (zend_is_callable(handler, 0, NULL)) {
		return phalcon_call_user_func_array(retval, handler, params);
	}

	return FAILURE;
}

/**
 * @brief Checks if an object has a constructor
 * @param object Object to check
 * @return Whether @a object has a constructor
 * @retval 0 @a object is not an object or does not have a constructor
 * @retval 1 @a object has a constructor
 */
static inline int phalcon_has_constructor(const zval *object)
{
	return Z_TYPE_P(object) == IS_OBJECT ? phalcon_has_constructor_ce(Z_OBJCE_P(object)) : 0;
}

/**
 * @brief $object->$method()
 */
static inline int phalcon_call_method(zval **retval_ptr, zval *object, const char *method, uint nparams, zval **params)
{
	return phalcon_call_class_method_aparams(retval_ptr, object, Z_OBJCE_P(object), phalcon_fcall_method, method, strlen(method), nparams, params);
}

/**
 * @brief static::$method()
 */
static inline int phalcon_call_static(zval **retval_ptr, const char *method, uint nparams, zval **params)
{
	return phalcon_call_class_method_aparams(retval_ptr, NULL, NULL, phalcon_fcall_static, method, strlen(method), nparams, params);
}

static inline int phalcon_return_call_static(zval **retval_ptr, const char *method, uint nparams, zval **params)
{
	return phalcon_call_class_method_aparams(retval_ptr, NULL, NULL, phalcon_fcall_static, method, strlen(method), nparams, params);
}

/**
 * @brief self::$method()
 */
static inline int phalcon_call_self(zval **retval_ptr, const char *method, uint nparams, zval **params)
{
	return phalcon_call_class_method_aparams(retval_ptr, NULL, NULL, phalcon_fcall_self, method, strlen(method), nparams, params);
}

static inline int phalcon_return_call_self(zval **retval_ptr, const char *method, uint nparams, zval **params)
{
	return phalcon_call_class_method_aparams(retval_ptr, NULL, NULL, phalcon_fcall_self, method, strlen(method), nparams, params);
}

/**
 * @brief $ce::$method()
 */
static inline int phalcon_call_ce(zval **retval_ptr, zend_class_entry *ce, const char *method, uint nparams, zval **params)
{
	return phalcon_call_class_method_aparams(retval_ptr, NULL, ce, phalcon_fcall_ce, method, strlen(method), nparams, params);
}

static inline int phalcon_return_call_ce(zval **retval_ptr, zend_class_entry *ce, const char *method, uint nparams, zval **params)
{
	return phalcon_call_class_method_aparams(retval_ptr, NULL, ce, phalcon_fcall_ce, method, strlen(method), nparams, params);
}

#define PHALCON_CALL_METHOD_WITH_PARAMSW(retval, obj, obj_ce, call_type, method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_ON_FAILURE(phalcon_call_method_with_params(retval, obj, obj_ce, call_type, method, PHALCON_FUNC_STRLEN(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_CALL_FUNCTIONW(retval, method, ...) PHALCON_CALL_METHOD_WITH_PARAMSW(retval, NULL, NULL, phalcon_fcall_function, method, __VA_ARGS__)

#define PHALCON_CALL_METHODW(retval, object, method, ...) PHALCON_CALL_METHOD_WITH_PARAMSW(retval, object, Z_OBJCE_P(object), phalcon_fcall_method, method, __VA_ARGS__)

#define PHALCON_CALL_SELFW(retval, method, ...) PHALCON_CALL_METHOD_WITH_PARAMSW(retval, NULL, NULL, phalcon_fcall_self, method, __VA_ARGS__)

#define PHALCON_CALL_PARENTW(retval, class_entry, object, method, ...) PHALCON_CALL_METHOD_WITH_PARAMSW(retval, object, class_entry, phalcon_fcall_parent, method, __VA_ARGS__)

#define PHALCON_RETURN_CALL_FUNCTIONW(method, ...) PHALCON_CALL_METHOD_WITH_PARAMSW(return_value, NULL, NULL, phalcon_fcall_function, method, __VA_ARGS__)

#define PHALCON_RETURN_CALL_METHODW(object, method, ...) PHALCON_CALL_METHOD_WITH_PARAMSW(return_value, object, Z_OBJCE_P(object), phalcon_fcall_method, method, __VA_ARGS__)

#define PHALCON_RETURN_CALL_SELFW(method, ...) PHALCON_CALL_METHOD_WITH_PARAMSW(return_value, NULL, NULL, phalcon_fcall_self, method, __VA_ARGS__)

#define PHALCON_CALL_CE_STATICW(retval, obj_ce, method, ...) PHALCON_CALL_METHOD_WITH_PARAMSW(retval, NULL, obj_ce, phalcon_fcall_static, method, __VA_ARGS__)

#define PHALCON_CALL_METHOD_WITH_PARAMS(retval, obj, obj_ce, call_type, method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_MM_ON_FAILURE(phalcon_call_method_with_params(retval, obj, obj_ce, call_type, method, PHALCON_FUNC_STRLEN(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_CALL_FUNCTION(retval, method, ...) PHALCON_CALL_METHOD_WITH_PARAMS(retval, NULL, NULL, phalcon_fcall_function, method, __VA_ARGS__)

#define PHALCON_CALL_METHOD(retval, object, method, ...) PHALCON_CALL_METHOD_WITH_PARAMS(retval, object, Z_OBJCE_P(object), phalcon_fcall_method, method, __VA_ARGS__)

#define PHALCON_CALL_SELF(retval, method, ...) PHALCON_CALL_METHOD_WITH_PARAMS(retval, NULL, NULL, phalcon_fcall_self, method, __VA_ARGS__)

#define PHALCON_CALL_PARENT(retval, class_entry, object, method, ...) PHALCON_CALL_METHOD_WITH_PARAMS(retval, object, class_entry, phalcon_fcall_parent, method, __VA_ARGS__)

#define PHALCON_RETURN_CALL_FUNCTION(method, ...) PHALCON_CALL_METHOD_WITH_PARAMS(return_value, NULL, NULL, phalcon_fcall_function, method, __VA_ARGS__)

#define PHALCON_RETURN_CALL_METHOD(object, method, ...) PHALCON_CALL_METHOD_WITH_PARAMS(return_value, object, Z_OBJCE_P(object), phalcon_fcall_method, method, __VA_ARGS__)

#define PHALCON_RETURN_CALL_SELF(method, ...) PHALCON_CALL_METHOD_WITH_PARAMS(return_value, NULL, NULL, phalcon_fcall_self, method, __VA_ARGS__)

#define PHALCON_CALL_CE_STATIC(retval, obj_ce, method, ...) PHALCON_CALL_METHOD_WITH_PARAMS(retval, NULL, obj_ce, phalcon_fcall_static, method, __VA_ARGS__)

#define PHALCON_CALL_ZVAL_METHOD_WITH_PARAMSW(retval, obj, obj_ce, call_type, method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_ON_FAILURE(phalcon_call_method_with_params(retval, obj, obj_ce, call_type, Z_STRVAL_P(method), Z_STRLEN_P(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_CALL_ZVAL_METHOD_WITH_PARAMS(retval, obj, obj_ce, call_type, method, ...) \
	do { \
		zval *params_[] = {PHALCON_FETCH_VA_ARGS __VA_ARGS__}; \
		RETURN_MM_ON_FAILURE(phalcon_call_method_with_params(retval, obj, obj_ce, call_type, Z_STRVAL_P(method), Z_STRLEN_P(method), PHALCON_CALL_NUM_PARAMS(params_), PHALCON_PASS_CALL_PARAMS(params_))); \
	} while (0)

#define PHALCON_CALL_ZVAL_METHODW(retval, object, method, ...) PHALCON_CALL_ZVAL_METHOD_WITH_PARAMSW(retval, object, Z_OBJCE_P(object), phalcon_fcall_method, method, __VA_ARGS__)
#define PHALCON_CALL_ZVAL_METHOD(retval, object, method, ...) PHALCON_CALL_ZVAL_METHOD_WITH_PARAMS(retval, object, Z_OBJCE_P(object), phalcon_fcall_method, method, __VA_ARGS__)

int phalcon_call_method_with_params(zval *retval, zval *object, zend_class_entry *ce, phalcon_call_type type, const char *method_name, uint method_len, uint param_count, zval *params[]);

static inline int phalcon_return_call_method(zval *retval, zval *object, const char *method, uint nparams, zval **params)
{
	return phalcon_call_method_with_params(retval, object, Z_OBJCE_P(object), phalcon_fcall_method, method, strlen(method), nparams, params);
}

#endif /* PHALCON_KERNEL_FCALL_H */
