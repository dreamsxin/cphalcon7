
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

static const unsigned char tolower_map[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

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
int phalcon_call_user_function(zval *object, zend_class_entry *ce, phalcon_call_type type, zval *function_name, zval *retval_ptr, uint32_t param_count, zval params[])
{
	HashTable *function_table;

	if (type != phalcon_fcall_function && !object) {
		object = &EG(current_execute_data)->This;
	}

	if (!ce && object) {
		ce = Z_OBJCE_P(object);
	}

	function_table = ce ? &ce->function_table : EG(function_table);

	return call_user_function(function_table, object, function_name, retval_ptr, param_count, params);
}

int phalcon_call_func_aparams(zval *retval_ptr, const char *func_name, uint func_length, uint param_count, zval *params)
{
	zval func;
	int status;

#ifndef PHALCON_RELEASE
	if (!Z_ISUNDEF_P(retval_ptr)) {
		fprintf(stderr, "%s: retval_ptr must be UNDEF\n", __func__);
		phalcon_print_backtrace();
		abort();
	}
#endif

	ZVAL_STRINGL(&func, func_name, func_length);

	if ((status = phalcon_call_user_function(NULL, NULL, phalcon_fcall_function, &func, retval_ptr, param_count, params)) == FAILURE) {
		zval_ptr_dtor(retval_ptr);
		if (!EG(exception)) {
			zend_error(E_ERROR, "Call to undefined function %s()", func_name);
		}
	} else if (EG(exception)) {
		status = FAILURE;
	}

	phalcon_ptr_dtor(&func);

	return status;
}

int phalcon_call_zval_func_aparams(zval *retval_ptr, zval *func, uint param_count, zval *params)
{
	int status;

#ifndef PHALCON_RELEASE
	if (!Z_ISUNDEF_P(retval_ptr)) {
		fprintf(stderr, "%s: retval_ptr must be UNDEF\n", __func__);
		phalcon_print_backtrace();
		abort();
	}
#endif

	if ((status = phalcon_call_user_function(NULL, NULL, phalcon_fcall_function, func, retval_ptr, param_count, params)) == FAILURE) {
		phalcon_ptr_dtor(retval_ptr);
		if (!EG(exception)) {
			phalcon_throw_exception_format(spl_ce_RuntimeException, "Call to undefined function %s()", Z_TYPE_P(func) ? Z_STRVAL_P(func) : "undefined");
		}
	} else if (EG(exception)) {
		status = FAILURE;
	}

	return status;
}

int phalcon_call_class_method_aparams(zval *retval_ptr, zval *object, zend_class_entry *ce, phalcon_call_type type, const char *method_name, uint method_len, uint param_count, zval *params)
{
	zval func;
	int status;

#ifndef PHALCON_RELEASE
	if (!Z_ISUNDEF_P(retval_ptr)) {
		fprintf(stderr, "%s: retval_ptr must be UNDEF\n", __func__);
		phalcon_print_backtrace();
		abort();
	}
#endif

	if (object) {
		if (Z_TYPE_P(object) != IS_OBJECT) {
			phalcon_throw_exception_format(spl_ce_RuntimeException, "Trying to call method %s on a non-object", method_name);
			return FAILURE;
		}
	}
	array_init_size(&func, 2);
	switch (type) {
		case phalcon_fcall_parent: add_next_index_stringl(&func, ISL(parent)); break;
		case phalcon_fcall_self:   assert(!ce); add_next_index_stringl(&func, ISL(self)); break;
		case phalcon_fcall_static: assert(!ce); add_next_index_stringl(&func, ISL(static)); break;

		case phalcon_fcall_ce:
			assert(ce != NULL);
			add_next_index_str(&func, ce->name);
			break;

		case phalcon_fcall_method:
		default:
			assert(object != NULL);
			Z_ADDREF_P(object);
			add_next_index_zval(&func, object);
			break;
	}

	add_next_index_stringl(&func, method_name, method_len);

	if ((status = phalcon_call_user_function(object ? object : NULL, ce, type, &func, retval_ptr, param_count, params)) == FAILURE) {
		phalcon_ptr_dtor(retval_ptr);
		if (!EG(exception)) {
			switch (type) {
				case phalcon_fcall_parent: zend_error(E_ERROR, "Call to undefined function parent::%s()", method_name); break;
				case phalcon_fcall_self:   zend_error(E_ERROR, "Call to undefined function self::%s()", method_name); break;
				case phalcon_fcall_static: zend_error(E_ERROR, "Call to undefined function static::%s()", method_name); break;
				case phalcon_fcall_ce:     zend_error(E_ERROR, "Call to undefined function %s::%s()", ce->name->val, method_name); break;
				case phalcon_fcall_method: zend_error(E_ERROR, "Call to undefined function %s::%s()", Z_OBJCE_P(object)->name->val, method_name); break;
				default:                   zend_error(E_ERROR, "Call to undefined function ?::%s()", method_name);
			}
		}
	} else if (EG(exception)) {
		status = FAILURE;
	}

	phalcon_ptr_dtor(&func);
	return status;
}

/**
 * Replaces call_user_func_array avoiding function lookup
 * This function does not return FAILURE if an exception has ocurred
 */
int phalcon_call_user_func_array_noex(zval *return_value, zval *handler, zval *params){

	zval retval;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	char *is_callable_error = NULL;
	int status = FAILURE;

	if (params && Z_TYPE_P(params) != IS_ARRAY) {
		ZVAL_NULL(return_value);
		php_error_docref(NULL, E_WARNING, "Invalid arguments supplied for phalcon_call_user_func_array_noex()");
		return FAILURE;
	}

	if (zend_fcall_info_init(handler, 0, &fci, &fci_cache, NULL, &is_callable_error) == SUCCESS) {
		if (is_callable_error) {
			zend_error(E_STRICT, "%s", is_callable_error);
			efree(is_callable_error);
		}
		status = SUCCESS;
	} else {
		if (is_callable_error) {
			zend_error(E_WARNING, "%s", is_callable_error);
			efree(is_callable_error);
		} else {
			status = SUCCESS;
		}
	}

	if (status == SUCCESS) {

		zend_fcall_info_args(&fci, params);
		fci.retval = &retval;

		if (zend_call_function(&fci, &fci_cache) == SUCCESS && fci.retval) {
			ZVAL_DUP(return_value, fci.retval);
		}

		if (fci.params) {
			efree(fci.params);
		}
	}

	if (EG(exception)) {
		status = SUCCESS;
	}

	return status;
}

void phalcon_eval_php(zval *str, zval *retval_ptr, char *context)
{
    zend_eval_string_ex(Z_STRVAL_P(str), retval_ptr, context, 1);
}
