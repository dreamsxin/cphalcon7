
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

#include "kernel/fcall.h"

#include <Zend/zend_API.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_execute.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/hash.h"
#include "kernel/exception.h"
#include "kernel/backtrace.h"
#include "kernel/string.h"

#include "interned-strings.h"

int phalcon_has_constructor_ce(const zend_class_entry *ce)
{
	while (ce) {
		if (ce->constructor) {
			return 1;
		}

		ce = ce->parent;
	}

	return 0;
}

zval* _phalcon_call(zval *retval_ptr, zval *object, zend_class_entry *obj_ce, zend_function **fn_proxy, const char *function_name, size_t function_name_len, int param_count, zval* args[])
{
	int i, result;
	zend_fcall_info fci;
	zval retval;
	zval params[50];

	while(i < param_count && i < 50) {
		ZVAL_COPY_VALUE(&params[i], args[i]);
		i++;
	}

	fci.size = sizeof(fci);
	fci.object = object ? Z_OBJ_P(object) : NULL;
	fci.retval = retval_ptr ? retval_ptr : &retval;
	fci.param_count = param_count;
	fci.params = params;
	fci.no_separation = 1;

	if (!fn_proxy && !obj_ce) {
		/* no interest in caching and no information already present that is
		 * needed later inside zend_call_function. */
		ZVAL_STRINGL(&fci.function_name, function_name, function_name_len);
		result = zend_call_function(&fci, NULL);
		zval_ptr_dtor(&fci.function_name);
	} else {
		zend_fcall_info_cache fcic;
		ZVAL_UNDEF(&fci.function_name); /* Unused */
#if PHP_VERSION_ID < 70300
		fcic.initialized = 1;
#endif
		if (!obj_ce) {
			obj_ce = object ? Z_OBJCE_P(object) : NULL;
		}
		if (!fn_proxy || !*fn_proxy) {
			HashTable *function_table = obj_ce ? &obj_ce->function_table : EG(function_table);
			fcic.function_handler = zend_hash_str_find_ptr(
				function_table, function_name, function_name_len);
			if (fcic.function_handler == NULL) {
				/* error at c-level */
				zend_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method %s%s%s", obj_ce ? ZSTR_VAL(obj_ce->name) : "", obj_ce ? "::" : "", function_name);
			}
			if (fn_proxy) {
				*fn_proxy = fcic.function_handler;
			}
		} else {
			fcic.function_handler = *fn_proxy;
		}

		fcic.calling_scope = obj_ce;
		if (object) {
			fcic.called_scope = Z_OBJCE_P(object);
		} else {
			zend_class_entry *called_scope = zend_get_called_scope(EG(current_execute_data));

			if (obj_ce &&
			    (!called_scope ||
			     !instanceof_function(called_scope, obj_ce))) {
				fcic.called_scope = obj_ce;
			} else {
				fcic.called_scope = called_scope;
			}
		}
		fcic.object = object ? Z_OBJ_P(object) : NULL;
		result = zend_call_function(&fci, &fcic);
	}
	if (result == FAILURE) {
		/* error at c-level */
		if (!obj_ce) {
			obj_ce = object ? Z_OBJCE_P(object) : NULL;
		}
		if (!EG(exception)) {
			zend_error_noreturn(E_CORE_ERROR, "Couldn't execute method %s%s%s", obj_ce ? ZSTR_VAL(obj_ce->name) : "", obj_ce ? "::" : "", function_name);
		}
	}
	if (!retval_ptr) {
		zval_ptr_dtor(&retval);
		return NULL;
	}
	return retval_ptr;
}

int phalcon_call_user_func_args(zval *retval, zval *handler, zval *params, int param_count)
{
	zval ret = {}, *retval_ptr = (retval != NULL) ? retval : &ret;
	int status;

	if (
#if PHP_VERSION_ID >= 70100
		(status = _call_user_function_ex(NULL, handler, retval_ptr, param_count, params, 1)) == FAILURE || EG(exception)
#else
		(status = call_user_function(EG(function_table), NULL, handler, retval_ptr, param_count, params)) == FAILURE || EG(exception)
#endif
	) {
		status = FAILURE;
		ZVAL_NULL(retval_ptr);
	}

	return status;
}

int phalcon_call_user_func_params(zval *retval, zval *handler, int param_count, zval *params[])
{
	zval ret = {}, *retval_ptr = (retval != NULL) ? retval : &ret;
	zval *arguments;
	int i, status;

	arguments = param_count ? safe_emalloc(sizeof(zval), param_count, 0) : NULL;

	i = 0;
	while(i < param_count) {
		if (params[i]) {
			ZVAL_COPY_VALUE(&arguments[i], params[i]);
		} else {
			ZVAL_NULL(&arguments[i]);
		}
		i++;
	}

	if (
#if PHP_VERSION_ID >= 70100
		(status = _call_user_function_ex(NULL, handler, retval_ptr, param_count, arguments, 1)) == FAILURE || EG(exception)
#else
		(status = call_user_function(EG(function_table), NULL, handler, retval_ptr, param_count, arguments)) == FAILURE || EG(exception)
#endif
	) {
		status = FAILURE;
		ZVAL_NULL(retval_ptr);
	}

	efree(arguments);

	return status;
}

int phalcon_call_user_func_array(zval *retval, zval *handler, zval *params)
{
	zval ret = {}, *retval_ptr = (retval != NULL) ? retval : &ret, *arguments = NULL, *param;
	int params_count = 0, i, status;

	if (params && Z_TYPE_P(params) != IS_ARRAY && Z_TYPE_P(params) > IS_NULL) {
		status = FAILURE;
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid arguments supplied for phalcon_call_user_func_array()");
		return status;
	}

	if (params && Z_TYPE_P(params) == IS_ARRAY) {
		params_count = zend_hash_num_elements(Z_ARRVAL_P(params));
		arguments = (zval*)emalloc(sizeof(zval) * params_count);
		i = 0;
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(params), param) {
			ZVAL_COPY_VALUE(&arguments[i], param);
			i++;
		} ZEND_HASH_FOREACH_END();
	} else {
		params_count = 0;
		arguments = NULL;
	}
	if (
#if PHP_VERSION_ID >= 70100
	(status = _call_user_function_ex(NULL, handler, retval_ptr, params_count, arguments, 1)) == FAILURE || EG(exception)
#else
	(status = call_user_function(EG(function_table), NULL, handler, retval_ptr, params_count, arguments)) == FAILURE || EG(exception)
#endif
	) {
		status = FAILURE;
		ZVAL_NULL(retval_ptr);
	}

	efree(arguments);

	return status;
}

int phalcon_call_method_with_params(zval *retval, zval *object, zend_class_entry *ce, phalcon_call_type type, const char *method_name, uint method_len, uint param_count, zval *params[])
{
	zval func_name = {}, ret = {}, *retval_ptr = (retval != NULL) ? retval : &ret, obj = {};
	zval *arguments;
	int i, status;

	if (type != phalcon_fcall_function) {
		if (type != phalcon_fcall_ce && type != phalcon_fcall_self && type != phalcon_fcall_static) {
			if (object == NULL || Z_TYPE_P(object) != IS_OBJECT) {
				phalcon_throw_exception_format(spl_ce_RuntimeException, "Trying to call method %s on a non-object", method_name);
				return FAILURE;
			}
		}

		if (object == NULL || Z_TYPE_P(object) != IS_OBJECT) {
			if (zend_get_this_object(EG(current_execute_data))){
				ZVAL_OBJ(&obj, zend_get_this_object(EG(current_execute_data)));
				object = &obj;
			}
		}

		switch (type) {
			case phalcon_fcall_ce:
				assert(ce != NULL);
				array_init_size(&func_name, 2);
				add_next_index_string(&func_name, ce->name->val);
				add_next_index_stringl(&func_name, method_name, method_len);
				break;
			case phalcon_fcall_parent:
				assert(ce != NULL);
				array_init_size(&func_name, 2);
				add_next_index_string(&func_name, ISV(parent));
				add_next_index_stringl(&func_name, method_name, method_len);
				break;
			case phalcon_fcall_self:
				array_init_size(&func_name, 2);
				add_next_index_string(&func_name, ISV(self));
				add_next_index_stringl(&func_name, method_name, method_len);
				break;
			case phalcon_fcall_static:
				if (phalcon_memnstr_str_str(method_name, method_len, SL("::"))) {
					phalcon_fast_explode_str_str(&func_name, SL("::"), method_name, method_len);
				} else {
					array_init_size(&func_name, 2);
					add_next_index_string(&func_name, ISV(static));
					add_next_index_stringl(&func_name, method_name, method_len);
				}
				break;
			case phalcon_fcall_method:
				array_init_size(&func_name, 2);
				Z_TRY_ADDREF_P(object);
				add_next_index_zval(&func_name, object);
				add_next_index_stringl(&func_name, method_name, method_len);
				break;
			default:
				phalcon_throw_exception_format(spl_ce_RuntimeException, "Error call type %d for cmethod %s", type, method_name);
				return FAILURE;
		}

		if (!ce && object && Z_TYPE_P(object) == IS_OBJECT) {
			ce = Z_OBJCE_P(object);
		}
	} else {
		ZVAL_STRINGL(&func_name, method_name, method_len);
	}

	arguments = param_count ? safe_emalloc(sizeof(zval), param_count, 0) : NULL;

	i = 0;
	while(i < param_count) {
		if (params[i]) {
			ZVAL_COPY_VALUE(&arguments[i], params[i]);
		} else {
			ZVAL_NULL(&arguments[i]);
		}
		i++;
	}

	if (
#if PHP_VERSION_ID >= 70100
	(status = _call_user_function_ex(object, &func_name, retval_ptr, param_count, arguments, 1)) == FAILURE || EG(exception)
#else
	(status = call_user_function_ex(ce ? &(ce)->function_table : EG(function_table), object, &func_name, retval_ptr, param_count, arguments, 1, NULL)) == FAILURE || EG(exception)
#endif
	) {
		status = FAILURE;
		ZVAL_NULL(retval_ptr);
		if (!EG(exception)) {
			switch (type) {
				case phalcon_fcall_function:
					zend_error(E_ERROR, "Call to undefined function %s()", method_name);
					break;
				case phalcon_fcall_parent:
					zend_error(E_ERROR, "Call to undefined method parent::%s()", method_name);
					break;
				case phalcon_fcall_self:
					zend_error(E_ERROR, "Call to undefined method self::%s()", method_name);
					break;
				case phalcon_fcall_static:
					zend_error(E_ERROR, "Call to undefined function static::%s()", method_name);
					break;
				case phalcon_fcall_ce:
					zend_error(E_ERROR, "Call to undefined method %s::%s()", ce->name->val, method_name);
					break;
				case phalcon_fcall_method:
					zend_error(E_ERROR, "Call to undefined method %s::%s()", Z_OBJCE_P(object)->name->val, method_name);
					break;
				default:
					zend_error(E_ERROR, "Call to undefined method ?::%s()", method_name);
			}
		}
	}
	zval_ptr_dtor(&func_name);
	efree(arguments);
	if (retval == NULL) {
		if (!Z_ISUNDEF(ret)) {
			zval_ptr_dtor(&ret);
		}
	}

	return status;
}
