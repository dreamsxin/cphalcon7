
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

/**
 * Calls a function/method in the PHP userland
 */
int phalcon_call_user_function2(zval **retval_ptr, zval *object, zend_class_entry *ce, phalcon_call_type type, zval *function_name, uint32_t param_count, zval params[])
{
	zval retval;
	HashTable *function_table;
	int status;

	if (type != phalcon_fcall_function && !object) {
		object = &EG(current_execute_data)->This;
	}

	if (!ce && object) {
		ce = Z_OBJCE_P(object);
	}

	function_table = ce ? &ce->function_table : EG(function_table);

	if ((status = call_user_function(function_table, object, function_name, &retval, param_count, params)) == FAILURE || EG(exception)) {
		status = FAILURE;
	}

	if (retval_ptr) {
		if (*retval_ptr == NULL) {
			PHALCON_ALLOC_ZVAL(*retval_ptr);
		}
		ZVAL_COPY(*retval_ptr, &retval);
	}

	return status;
}

int phalcon_call_user_function(zval **retval_ptr, zval *object, zend_class_entry *ce, phalcon_call_type type, zval *function_name, uint32_t param_count, zval *params[])
{
	zval retval, *arguments;
	HashTable *function_table;
	int i, status;

	if (type != phalcon_fcall_function && !object) {
		object = &EG(current_execute_data)->This;
	}

	if (!ce && object) {
		ce = Z_OBJCE_P(object);
	}

	function_table = ce ? &ce->function_table : EG(function_table);

	arguments = emalloc(sizeof(zval) * param_count);

	i = 0;
	while(i < param_count) {
		ZVAL_COPY(&arguments[i], params[i]);
		i++;
	}

	if ((status = call_user_function(function_table, object, function_name, &retval, param_count, arguments)) == FAILURE || EG(exception)) {
		status = FAILURE;
	}

	if (retval_ptr) {
		if (*retval_ptr == NULL) {
			PHALCON_ALLOC_ZVAL(*retval_ptr);
		}
		ZVAL_COPY(*retval_ptr, &retval);
	}

	efree(arguments);

	return status;
}

int phalcon_call_func_aparams(zval **retval_ptr, const char *func_name, uint func_length, uint param_count, zval *params[])
{
	zval func;
	int status;

	ZVAL_STRINGL(&func, func_name, func_length);

	if ((status = phalcon_call_user_function(retval_ptr, NULL, NULL, phalcon_fcall_function, &func, param_count, params)) == FAILURE) {
		if (!EG(exception)) {
			zend_error(E_ERROR, "Call to undefined function %s()", func_name);
		}
	}

	zval_ptr_dtor(&func);

	return status;
}

int phalcon_call_zval_func_aparams(zval **retval_ptr, zval *func, uint param_count, zval *params[])
{
	int status;

	if ((status = phalcon_call_user_function(retval_ptr, NULL, NULL, phalcon_fcall_function, func, param_count, params)) == FAILURE) {
		if (!EG(exception)) {
			phalcon_throw_exception_format(spl_ce_RuntimeException, "Call to undefined function %s()", Z_TYPE_P(func) ? Z_STRVAL_P(func) : "undefined");
		}
	}

	return status;
}



int phalcon_call_class_zval_method_aparams(zval **retval_ptr, zval *object, zend_class_entry *ce, phalcon_call_type type, zval *method, uint param_count, zval *params[])
{
	zval func;
	int status;

	if (object) {
		if (Z_TYPE_P(object) != IS_OBJECT) {
			phalcon_throw_exception_format(spl_ce_RuntimeException, "Trying to call method %s on a non-object", Z_STRVAL_P(method));
			return FAILURE;
		}
	}
	array_init_size(&func, 2);
	switch (type) {
		case phalcon_fcall_parent: add_next_index_str(&func, IS(parent)); break;
		case phalcon_fcall_self:   assert(!ce); add_next_index_str(&func, IS(self)); break;
		case phalcon_fcall_static: assert(!ce); add_next_index_str(&func, IS(static)); break;

		case phalcon_fcall_ce:
			assert(ce != NULL);
			add_next_index_str(&func, ce->name);
			break;

		case phalcon_fcall_method:
		default:
			assert(object != NULL);
			Z_TRY_ADDREF_P(object);
			add_next_index_zval(&func, object);
			break;
	}

	Z_TRY_ADDREF_P(method);
	add_next_index_zval(&func, method);

	if ((status = phalcon_call_user_function(retval_ptr, object ? object : NULL, ce, type, &func, param_count, params)) == FAILURE) {
		if (!EG(exception)) {
			switch (type) {
				case phalcon_fcall_parent:
					zend_error(E_ERROR, "Call to undefined function parent::%s()", Z_STRVAL_P(method));
					break;
				case phalcon_fcall_self:
					zend_error(E_ERROR, "Call to undefined function self::%s()", Z_STRVAL_P(method));
					break;
				case phalcon_fcall_static:
					zend_error(E_ERROR, "Call to undefined function static::%s()", Z_STRVAL_P(method));
					break;
				case phalcon_fcall_ce:
					zend_error(E_ERROR, "Call to undefined function %s::%s()", ce->name->val, Z_STRVAL_P(method));
					break;
				case phalcon_fcall_method:
					zend_error(E_ERROR, "Call to undefined function %s::%s()", Z_OBJCE_P(object)->name->val, Z_STRVAL_P(method));
					break;
				default:
					zend_error(E_ERROR, "Call to undefined function ?::%s()", Z_STRVAL_P(method));
			}
		}
	}

	zval_ptr_dtor(&func);
	return status;
}

int phalcon_call_class_method_aparams(zval **retval_ptr, zval *object, zend_class_entry *ce, phalcon_call_type type, const char *method_name, uint method_len, uint param_count, zval *params[])
{
	zval method;
	ZVAL_STRINGL(&method, method_name, method_len);

	return phalcon_call_class_zval_method_aparams(retval_ptr, object, ce, type, &method, param_count, params);
}

int phalcon_call_class_zval_method_array(zval **retval_ptr, zval *object, zval *method, zval *params)
{
	zval func, *arguments, *param;
	int param_count, i, status;

	if (object) {
		if (Z_TYPE_P(object) != IS_OBJECT) {
			phalcon_throw_exception_format(spl_ce_RuntimeException, "Trying to call method %s on a non-object", Z_STRVAL_P(method));
			return FAILURE;
		}
	}

	array_init_size(&func, 2);
	
	Z_TRY_ADDREF_P(object);
	add_next_index_zval(&func, object);
	add_next_index_zval(&func, method);

	if (params && Z_TYPE_P(params) != IS_ARRAY) {
		status = FAILURE;
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid arguments supplied for phalcon_call_class_method_array()");
		return FAILURE;
	}

	param_count = zend_hash_num_elements(Z_ARRVAL_P(params));

	arguments = emalloc(sizeof(zval) * param_count);

	i = 0;
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(params), param) {
		ZVAL_ZVAL(&arguments[i], param, 1, 0);
		i++;
	} ZEND_HASH_FOREACH_END();

	if ((status = phalcon_call_user_function2(retval_ptr, object, Z_OBJCE_P(object), phalcon_fcall_method, &func, param_count, arguments)) == FAILURE) {
		zend_error(E_ERROR, "Call to undefined function %s::%s()", Z_OBJCE_P(object)->name->val, Z_STRVAL_P(method));
	}
	return status;
}

int phalcon_call_user_func_array(zval **retval_ptr, zval *handler, zval *params)
{
	zval retval, *arguments, *param;
	int param_count, i, status;

	if (params && Z_TYPE_P(params) != IS_ARRAY) {
		status = FAILURE;
		ZVAL_NULL(*retval_ptr);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid arguments supplied for phalcon_call_user_func_array()");
	} else {
		param_count = zend_hash_num_elements(Z_ARRVAL_P(params));

		arguments = emalloc(sizeof(zval) * param_count);

		i = 0;
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(params), param) {
			ZVAL_COPY(&arguments[i], param);
			i++;
		} ZEND_HASH_FOREACH_END();

		if ((status = call_user_function(EG(function_table), NULL, handler, &retval, param_count, arguments)) == FAILURE || EG(exception)) {
			status = FAILURE;
		}
	}

	if (retval_ptr) {
		if (*retval_ptr == NULL) {
			PHALCON_ALLOC_ZVAL(*retval_ptr);
		}
		ZVAL_COPY(*retval_ptr, &retval);
	}

	efree(arguments);

	return status;
}
