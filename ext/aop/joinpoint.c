
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
  |          Julien Salleyron <julien.salleyron@gmail.com>                 |
  |          <pangudashu@gmail.com>                                        |
  +------------------------------------------------------------------------+
*/

#include "aop/joinpoint.h"
#include "aop.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/exception.h"

/**
 * Phalcon\Aop\Joinpoint
 */
zend_class_entry *phalcon_aop_joinpoint_ce;

PHP_METHOD(Phalcon_Aop_Joinpoint, getArguments);
PHP_METHOD(Phalcon_Aop_Joinpoint, setArguments);
PHP_METHOD(Phalcon_Aop_Joinpoint, getException);
PHP_METHOD(Phalcon_Aop_Joinpoint, getPointcut);
PHP_METHOD(Phalcon_Aop_Joinpoint, process);
PHP_METHOD(Phalcon_Aop_Joinpoint, getKindOfAdvice);
PHP_METHOD(Phalcon_Aop_Joinpoint, getObject);
PHP_METHOD(Phalcon_Aop_Joinpoint, getReturnedValue);
PHP_METHOD(Phalcon_Aop_Joinpoint, setReturnedValue);
PHP_METHOD(Phalcon_Aop_Joinpoint, getClassName);
PHP_METHOD(Phalcon_Aop_Joinpoint, getMethodName);
PHP_METHOD(Phalcon_Aop_Joinpoint, getFunctionName);
PHP_METHOD(Phalcon_Aop_Joinpoint, getAssignedValue);
PHP_METHOD(Phalcon_Aop_Joinpoint, setAssignedValue);
PHP_METHOD(Phalcon_Aop_Joinpoint, getPropertyName);
PHP_METHOD(Phalcon_Aop_Joinpoint, getPropertyValue);
PHP_METHOD(Phalcon_Aop_Joinpoint, setProperty);
PHP_METHOD(Phalcon_Aop_Joinpoint, getProperty);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_aop_joinpoint_setarguments, 0, 0, 1)
	ZEND_ARG_ARRAY_INFO(0, params, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_aop_joinpoint_getreturnedvalue, 0, ZEND_RETURN_REFERENCE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_phalcon_aop_joinpoint_setreturnedvalue, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_aop_joinpoint_getassignedvalue, 0, ZEND_RETURN_REFERENCE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_phalcon_aop_joinpoint_setassignedvalue, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_phalcon_aop_joinpoint_setproperty, 2)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_phalcon_aop_joinpoint_getproperty, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_aop_joinpoint_method_entry[] = {
	PHP_ME(Phalcon_Aop_Joinpoint, getArguments, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, setArguments, arginfo_phalcon_aop_joinpoint_setarguments, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getException, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getPointcut, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, process, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getKindOfAdvice, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getObject, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getReturnedValue, arginfo_phalcon_aop_joinpoint_getreturnedvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, setReturnedValue, arginfo_phalcon_aop_joinpoint_setreturnedvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getClassName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getMethodName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getFunctionName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getAssignedValue, arginfo_phalcon_aop_joinpoint_getassignedvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, setAssignedValue, arginfo_phalcon_aop_joinpoint_setassignedvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getPropertyName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getPropertyValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, setProperty, arginfo_phalcon_aop_joinpoint_setproperty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Aop_Joinpoint, getProperty, arginfo_phalcon_aop_joinpoint_getproperty, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_aop_joinpoint_object_handlers;
zend_object* phalcon_aop_joinpoint_object_create_handler(zend_class_entry *ce)
{
	phalcon_aop_joinpoint_object *intern = ecalloc(1, sizeof(phalcon_aop_joinpoint_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_aop_joinpoint_object_handlers;

	return &intern->std;
}

void phalcon_aop_joinpoint_object_free_handler(zend_object *object)
{
	phalcon_aop_joinpoint_object *intern = phalcon_aop_joinpoint_object_from_obj(object);
	if (intern->args != NULL) {
		zval_ptr_dtor(intern->args);
		efree(intern->args);
	}
	if (intern->return_value != NULL) {
		zval_ptr_dtor(intern->return_value);
		efree(intern->return_value);
	}
	if (Z_TYPE(intern->property_value) != IS_UNDEF) {
		zval_ptr_dtor(&intern->property_value);
	}
	zend_object_std_dtor(object);
}

static inline void _zend_assign_to_variable_reference(zval *variable_ptr, zval *value_ptr)
{
	zend_reference *ref;

	if (EXPECTED(!Z_ISREF_P(value_ptr))) {
		ZVAL_NEW_REF(value_ptr, value_ptr);
	} else if (UNEXPECTED(variable_ptr == value_ptr)) {
		return;
	}

	ref = Z_REF_P(value_ptr);
#if PHP_VERSION_ID >= 70300
	GC_ADDREF(ref);
#else
	GC_REFCOUNT(ref)++;
#endif
	if (Z_REFCOUNTED_P(variable_ptr)) {
		zend_refcounted *garbage = Z_COUNTED_P(variable_ptr);
#if PHP_VERSION_ID >= 70300
		if (GC_DELREF(garbage) == 0) {
#else
		if (--GC_REFCOUNT(garbage) == 0) {
#endif
			ZVAL_REF(variable_ptr, ref);
#if PHP_VERSION_ID >= 70100
			zval_dtor_func(garbage);
#else
			zval_dtor_func_for_ptr(garbage);
#endif
			return;
		} else {
#if PHP_VERSION_ID >= 70200
			gc_check_possible_root(Z_COUNTED_P(variable_ptr));
#else
			GC_ZVAL_CHECK_POSSIBLE_ROOT(variable_ptr);
#endif
		}
	}
	ZVAL_REF(variable_ptr, ref);
}

/**
 * Phalcon\Aop\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Aop_Joinpoint){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Aop, Joinpoint, aop_joinpoint, phalcon_aop_joinpoint_method_entry, 0);

	return SUCCESS;
}

/**
 * Return caught method call arguments
 *
 * @return array
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getArguments){

	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->args == NULL) {
		uint32_t call_num_args, first_extra_arg, i;
		zval *arg, *extra_start;
		zval *ret = emalloc(sizeof(zval));
		zend_op_array *op_array = &intern->ex->func->op_array;
		
		array_init(ret);
		
		first_extra_arg = op_array->num_args;
		call_num_args = ZEND_CALL_NUM_ARGS(intern->ex);

		if (call_num_args <= first_extra_arg) {
			for (i = 0; i < call_num_args; i++){
				arg = ZEND_CALL_VAR_NUM(intern->ex, i);
				if (Z_ISUNDEF_P(arg)) {
					continue;
				}
				Z_TRY_ADDREF_P(arg);
				zend_hash_next_index_insert(Z_ARR_P(ret), arg);
			}
		} else {
			for (i = 0; i < first_extra_arg; i++){
				arg = ZEND_CALL_VAR_NUM(intern->ex, i);
				if (Z_ISUNDEF_P(arg)) {
					continue;
				}
				Z_TRY_ADDREF_P(arg);
				zend_hash_next_index_insert(Z_ARR_P(ret), arg);
			}
			//get extra params
			extra_start = ZEND_CALL_VAR_NUM(intern->ex, op_array->last_var + op_array->T);
			for (i = 0; i < call_num_args - first_extra_arg; i++) {
				Z_TRY_ADDREF_P(extra_start + i);
				zend_hash_next_index_insert(Z_ARR_P(ret), extra_start + i);
			}
		}
		
		intern->args = ret;
	}
	RETURN_ZVAL(intern->args, 1, 0);
}

/**
 * Sets caught method call arguments
 *
 * @param array $params
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, setArguments){

	zval *params;
	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));
	if (intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY) {
		zend_error(E_ERROR, "setArguments is only available when the JoinPoint is a function or ia method call");
		return;
	}

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ARRAY(params)
	ZEND_PARSE_PARAMETERS_END();

	if (intern->args != NULL) {
		zval_ptr_dtor(intern->args);
	}else{
		intern->args = emalloc(sizeof(zval));
	}
	ZVAL_COPY(intern->args, params);

	RETURN_NULL();
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getException){

	zval exception = {};
	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (!(intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_CATCH)){
		zend_error(E_ERROR, "getException is only available when the advice was added with after or afterThrowing");
		return;
	}

	if (intern->exception != NULL) {
		ZVAL_OBJ(&exception, intern->exception);
		RETURN_ZVAL(&exception, 1, 0);
	}
	RETURN_NULL();
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getPointcut){

	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));
	RETURN_STR(intern->current_pointcut->selector);
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, process){

	zval call_ret;
	int is_ret_overloaded = 0;
	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (!intern->current_pointcut || !intern->current_pointcut->kind_of_advice) {
		zend_error(E_ERROR, "Error");
	}
	if (!(intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_AROUND)) {
		zend_error(E_ERROR, "process is only available when the advice was added with around");
		return;
	}
	if (intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY) {
		if (intern->kind_of_advice & PHALCON_AOP_KIND_WRITE) {
			phalcon_aop_do_write_property(intern->pos, intern->advice, getThis());
		} else {
			 phalcon_aop_do_read_property(intern->pos, intern->advice, getThis());
		}
	} else {
		if (intern->ex->return_value == NULL) {
			intern->ex->return_value = &call_ret;
			is_ret_overloaded = 1;
		}
		 phalcon_aop_do_func_execute(intern->pos, intern->advice, intern->ex, getThis());
		if (is_ret_overloaded == 0) {
			if (EG(exception) == NULL) {
				ZVAL_COPY(return_value, intern->ex->return_value);
			}
		} else {
			if (EG(exception) == NULL) {
				ZVAL_COPY_VALUE(return_value, intern->ex->return_value);
			}
			intern->ex->return_value = NULL;
		}
	}
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getKindOfAdvice){

	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));
	RETURN_LONG(intern->kind_of_advice);
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getObject){

	zend_object *call_object = NULL;
	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY) {
		if (intern->object != NULL) {
			RETURN_ZVAL(intern->object, 1, 0);
		}
	} else {
#if PHP_MINOR_VERSION < 1
		call_object = Z_OBJ(intern->ex->This);
#else
		if (Z_TYPE(intern->ex->This) == IS_OBJECT) {
			call_object = Z_OBJ(intern->ex->This);
		}
#endif
		if (call_object != NULL) {
			RETURN_ZVAL(&intern->ex->This, 1, 0);
		}
	}
	RETURN_NULL();
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getReturnedValue){

	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY) {
		zend_error(E_ERROR, "getReturnedValue is not available when the JoinPoint is a property operation (read or write)");
		return;
	}
	if (intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_BEFORE) {
		zend_error(E_ERROR, "getReturnedValue is not available when the advice was added with aop_add_before");
		return;
	}

	if (intern->ex->return_value != NULL) {
		if (EXPECTED(!Z_ISREF_P(intern->ex->return_value))) {
			intern->return_value_changed = 1;
		}
		_zend_assign_to_variable_reference(return_value, intern->ex->return_value);
	}
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, setReturnedValue){

	phalcon_aop_joinpoint_object *intern;
	zval *ret;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->kind_of_advice & PHALCON_AOP_KIND_WRITE) {
		zend_error(E_ERROR, "setReturnedValue is not available when the JoinPoint is a property write operation");
		return;
	}
	
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(ret)
	ZEND_PARSE_PARAMETERS_END();

	if (intern->return_value != NULL) {
		zval_ptr_dtor(intern->return_value);
	} else {
		intern->return_value = emalloc(sizeof(zval));
	}
	ZVAL_COPY(intern->return_value, ret);

	RETURN_NULL();
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getClassName){

	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY) {
		if (intern->object != NULL) {
			zend_class_entry *ce = Z_OBJCE_P(intern->object);
			RETURN_STR(ce->name);
		}
	} else {
		zend_class_entry *ce = NULL;
		zend_object *call_object = NULL;

#if PHP_MINOR_VERSION < 1
		call_object = Z_OBJ(intern->ex->This);
#else
		if (Z_TYPE(intern->ex->This) == IS_OBJECT) {
			call_object = Z_OBJ(intern->ex->This);
		}
#endif
		if (call_object != NULL) {
			ce = Z_OBJCE(intern->ex->This);
			RETURN_STR(ce->name);
		}

		if (ce == NULL && intern->ex->func->common.fn_flags & ZEND_ACC_STATIC) {
			ce = intern->ex->func->common.scope;
			RETURN_STR(ce->name);
		}
	}
	RETURN_NULL();
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getMethodName){

	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY || intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_FUNCTION) {
		zend_error(E_ERROR, "getMethodName is only available when the JoinPoint is a method call");
		return;
	}
	if (intern->ex == NULL) {
		RETURN_NULL();
	}
	RETURN_STR(intern->ex->func->common.function_name);
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getFunctionName){

	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY || intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_METHOD) {
		zend_error(E_ERROR, "getMethodName is only available when the JoinPoint is a function call");
		return;
	}
	if (intern->ex == NULL) {
		RETURN_NULL();
	}
	RETURN_STR(intern->ex->func->common.function_name);
}

/**
 * Gets the assigned value
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getAssignedValue){

	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (!(intern->kind_of_advice & PHALCON_AOP_KIND_WRITE)) {
		zend_error(E_ERROR, "getAssignedValue is only available when the JoinPoint is a property write operation");
		return;
	}

	if (Z_TYPE(intern->property_value) != IS_UNDEF) {
		_zend_assign_to_variable_reference(return_value, &intern->property_value);
	} else {
		RETURN_NULL();
	}
}

/**
 * Sets the assigned value
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, setAssignedValue){

	zval *assigned_value;
	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->kind_of_advice & PHALCON_AOP_KIND_READ) {
		zend_error(E_ERROR, "setAssignedValue is not available when the JoinPoint is a property read operation");
		return;
	}
	//parse prameters
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(assigned_value)
	ZEND_PARSE_PARAMETERS_END_EX(
		zend_error(E_ERROR, "Error");
		return;
	);

	if (Z_TYPE(intern->property_value) != IS_UNDEF) {
		zval_ptr_dtor(&intern->property_value);
	}

	ZVAL_COPY(&intern->property_value, assigned_value);
	RETURN_NULL();
}

/**
 * Gets the property name
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getPropertyName){

	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (!(intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY)) {
		zend_error(E_ERROR, "getPropertyName is only available when the JoinPoint is a property operation (read or write)");
		return;
	}

	if (intern->member != NULL) {
		RETURN_ZVAL(intern->member, 1, 0);
		return;
	}
	RETURN_NULL();
}

/**
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getPropertyValue){

	zval *ret;
	zend_class_entry *old_scope;
	phalcon_aop_joinpoint_object *intern;

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (!(intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY)) {
		zend_error(E_ERROR, "getPropertyValue is only available when the JoinPoint is a property operation (read or write)"); 
		return;
	}

	if (Z_TYPE(intern->property_value) > IS_NULL) {
		RETURN_ZVAL(&intern->property_value, 1, 0);
	}

	if (intern->object != NULL && intern->member != NULL) {
#if PHP_VERSION_ID >= 70100
		old_scope = EG(fake_scope);
		EG(fake_scope) = Z_OBJCE_P(intern->object);
#else
		old_scope = EG(scope);
		EG(scope) = Z_OBJCE_P(intern->object);
#endif
	   ret = original_zend_std_get_property_ptr_ptr(intern->object, intern->member, intern->type, intern->cache_slot);
#if PHP_VERSION_ID >= 70100
		EG(fake_scope) = old_scope;
#else
		EG(scope) = old_scope;
#endif
	}
	if (ret) {
		RETURN_ZVAL(ret, 1, 0);
	}
}

/**
 * Sets the object property
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, setProperty){

	zval *key, *value, *object;
	phalcon_aop_joinpoint_object *intern;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY) {
		if (intern->object != NULL) {
			object = intern->object;
		}
	} else {
		zend_object *call_object = NULL;
#if PHP_MINOR_VERSION < 1
		call_object = Z_OBJ(intern->ex->This);
#else
		if (Z_TYPE(intern->ex->This) == IS_OBJECT) {
			call_object = Z_OBJ(intern->ex->This);
		}
#endif
		if (call_object != NULL) {
			object = &intern->ex->This;
		}
	}

	if (object) {
		phalcon_update_property_zval_zval(object, key, value);
	}
}

/**
 * Gets the object property
 */
PHP_METHOD(Phalcon_Aop_Joinpoint, getProperty){

	zval *key, *object;
	phalcon_aop_joinpoint_object *intern;

	phalcon_fetch_params(0, 1, 0, &key);

	intern = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->current_pointcut->kind_of_advice & PHALCON_AOP_KIND_PROPERTY) {
		if (intern->object != NULL) {
			object = intern->object;
		}
	} else {
		zend_object *call_object = NULL;
#if PHP_MINOR_VERSION < 1
		call_object = Z_OBJ(intern->ex->This);
#else
		if (Z_TYPE(intern->ex->This) == IS_OBJECT) {
			call_object = Z_OBJ(intern->ex->This);
		}
#endif
		if (call_object != NULL) {
			object = &intern->ex->This;
		}
	}
	if (object) {
		phalcon_read_property_zval(return_value, object, key, PH_COPY);
	}
}
