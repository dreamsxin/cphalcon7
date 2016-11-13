
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

int phalcon_call_user_func_params(zval *retval, zval *handler, zval *params, int params_count)
{
	zval ret = {}, *retval_ptr = (retval != NULL) ? retval : &ret;
	int status;

	if ((status = call_user_function(EG(function_table), NULL, handler, retval_ptr, params_count, params)) == FAILURE || EG(exception)) {
		status = FAILURE;
		ZVAL_NULL(retval_ptr);
	}

	return status;
}

int phalcon_call_user_func_array(zval *retval, zval *handler, zval *params)
{
	zval ret = {}, *retval_ptr = (retval != NULL) ? retval : &ret, *arguments = NULL, *param;
	int param_count = 0, i, status;

	if (params && Z_TYPE_P(params) != IS_ARRAY) {
		status = FAILURE;
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid arguments supplied for phalcon_call_user_func_array()");
		return status;
	}

	if (params && Z_TYPE_P(params) == IS_ARRAY) {
		param_count = zend_hash_num_elements(Z_ARRVAL_P(params));
		arguments = (zval*)emalloc(sizeof(zval) * param_count);
		i = 0;
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(params), param) {
			ZVAL_COPY_VALUE(&arguments[i], param);
			i++;
		} ZEND_HASH_FOREACH_END();
	} else {
		param_count = 0;
		arguments = NULL;
	}

	if ((status = call_user_function(EG(function_table), NULL, handler, retval_ptr, param_count, arguments)) == FAILURE || EG(exception)) {
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
		if (object == NULL) {
			if (zend_get_this_object(EG(current_execute_data))){
				ZVAL_OBJ(&obj, zend_get_this_object(EG(current_execute_data)));
			} else {
				ZVAL_NULL(&obj);
			}
		} else {
			ZVAL_COPY(&obj, object);
		}

		if (Z_TYPE(obj) != IS_NULL && Z_TYPE(obj) != IS_OBJECT) {
			phalcon_throw_exception_format(spl_ce_RuntimeException, "Trying to call method %s on a non-object", method_name);
			return FAILURE;
		}
		
		if (!ce && Z_TYPE(obj) == IS_OBJECT) {
			ce = Z_OBJCE(obj);
		}

		array_init_size(&func_name, 2);
		switch (type) {
			case phalcon_fcall_parent:
				add_next_index_string(&func_name, ISV(parent));
				break;
			case phalcon_fcall_self:
				add_next_index_string(&func_name, ISV(self));
				break;
			case phalcon_fcall_static:
				add_next_index_string(&func_name, ISV(static));
				break;

			case phalcon_fcall_ce:
				assert(ce != NULL);
				add_next_index_string(&func_name, ce->name->val);
				break;

			case phalcon_fcall_method:
			default:
				assert(object != NULL);
				Z_TRY_ADDREF(obj);
				add_next_index_zval(&func_name, &obj);
				break;
		}

		add_next_index_stringl(&func_name, method_name, method_len);
	} else {
		ZVAL_STRINGL(&func_name, method_name, method_len);
	}

	arguments = param_count ? safe_emalloc(sizeof(zval), param_count, 0) : NULL;

	i = 0;
	while(i < param_count) {
		ZVAL_COPY_VALUE(&arguments[i], params[i]);
		i++;
	}

	if ((status = call_user_function_ex(ce ? &(ce)->function_table : EG(function_table), &obj, &func_name, retval_ptr, param_count, arguments, 1, NULL)) == FAILURE || EG(exception)) {
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

	efree(arguments);
	if (retval == NULL) {
		if (!Z_ISUNDEF(ret)) {
			zval_ptr_dtor(&ret);
		}
	}

	return status;
}
