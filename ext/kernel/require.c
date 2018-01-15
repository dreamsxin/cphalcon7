
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

#include "kernel/require.h"
#include "kernel/memory.h"
#include "kernel/backtrace.h"

#include <main/php_main.h>
#include <Zend/zend_hash.h>

static int phalcon_valid_var_name(char *var_name, int len) /* {{{ */
{
	int i, ch;

	if (!var_name)
		return 0;

	/* These are allowed as first char: [a-zA-Z_\x7f-\xff] */
	ch = (int)((unsigned char *)var_name)[0];
	if (var_name[0] != '_' &&
			(ch < 65  /* A    */ || /* Z    */ ch > 90)  &&
			(ch < 97  /* a    */ || /* z    */ ch > 122) &&
			(ch < 127 /* 0x7f */ || /* 0xff */ ch > 255)
	   ) {
		return 0;
	}

	/* And these as the rest: [a-zA-Z0-9_\x7f-\xff] */
	if (len > 1) {
		for (i = 1; i < len; i++) {
			ch = (int)((unsigned char *)var_name)[i];
			if (var_name[i] != '_' &&
					(ch < 48  /* 0    */ || /* 9    */ ch > 57)  &&
					(ch < 65  /* A    */ || /* Z    */ ch > 90)  &&
					(ch < 97  /* a    */ || /* z    */ ch > 122) &&
					(ch < 127 /* 0x7f */ || /* 0xff */ ch > 255)
			   ) {
				return 0;
			}
		}
	}
	return 1;
}

zend_array *phalcon_build_symtable(zval *vars) {
	zval *entry;
	zend_string *var_name;
	zend_array *symbol_table;
#if PHP_VERSION_ID < 70100
	zend_class_entry *scope = EG(scope);
#else
	zend_class_entry *scope = zend_get_executed_scope();
#endif

	symbol_table = emalloc(sizeof(zend_array));

	zend_hash_init(symbol_table, 8, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_real_init(symbol_table, 0);

	if (vars && Z_TYPE_P(vars) == IS_ARRAY) {
	    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(vars), var_name, entry) {
			/* GLOBALS protection */
			if (zend_string_equals_literal(var_name, "GLOBALS")) {
				continue;
			}

			if (zend_string_equals_literal(var_name, "this") && scope && ZSTR_LEN(scope->name) != 0) {
				continue;
			}

			if (phalcon_valid_var_name(ZSTR_VAL(var_name), ZSTR_LEN(var_name))) {
				if (EXPECTED(zend_hash_add_new(symbol_table, var_name, entry))) {
					Z_TRY_ADDREF_P(entry);
				}
			}
		} ZEND_HASH_FOREACH_END();
	}

	return symbol_table;
}

int _phalcon_exec(zval* ret, zval *object, zend_op_array *op_array, zend_array *symbol_table) {
	zend_execute_data *call;
	zval result;

	ZVAL_UNDEF(&result);

	if (object && Z_TYPE_P(object) == IS_OBJECT) {
		op_array->scope = Z_OBJCE_P(object);
	} else {
#if PHP_VERSION_ID >= 70100
		op_array->scope = EG(fake_scope);
#else
		op_array->scope = EG(scope);
#endif
	}

	call = zend_vm_stack_push_call_frame(ZEND_CALL_NESTED_CODE
#if PHP_VERSION_ID >= 70100
		    | ZEND_CALL_HAS_SYMBOL_TABLE
#endif
			,
			(zend_function*)op_array, 0, op_array->scope, object ? Z_OBJ_P(object) : NULL);

	call->symbol_table = symbol_table;

	if (ret && php_output_start_user(NULL, 0, PHP_OUTPUT_HANDLER_STDFLAGS) == FAILURE) {
		php_error_docref("ref.outcontrol", E_WARNING, "failed to create buffer");
		return 0;
	}

	zend_init_execute_data(call, op_array, &result);

	ZEND_ADD_CALL_FLAG(call, ZEND_CALL_TOP);
	zend_execute_ex(call);
	zend_vm_stack_free_call_frame(call);

	zval_ptr_dtor(&result);

	if (UNEXPECTED(EG(exception) != NULL)) {
		if (ret) {
			php_output_discard();
		}
		return 0;
	}

	if (ret) {
		if (php_output_get_contents(ret) == FAILURE) {
			php_output_end();
			php_error_docref(NULL, E_WARNING, "Unable to fetch ob content");
			return 0;
		}

		if (php_output_discard() != SUCCESS ) {
			return 0;
		}
	}

	return 1;
}

int phalcon_exec_file(zval *ret, zval *object, zval *file, zval *vars) {
	int status = 0;
	zend_string *filename;
	zend_array *symbol_table;
	zend_file_handle file_handle;
	zend_op_array 	*op_array;
	char realpath[MAXPATHLEN];

	if (IS_STRING != Z_TYPE_P(file)) {
		return 0;
	}

	filename = Z_STR_P(file);

	if (unlikely(!VCWD_REALPATH(ZSTR_VAL(filename), realpath))) {
		zend_error_noreturn(E_CORE_ERROR, "Failed opening file %s: %s", ZSTR_VAL(filename), strerror(errno));
		return 0;
	}
	symbol_table = phalcon_build_symtable(vars);

	file_handle.filename = ZSTR_VAL(filename);
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	op_array = zend_compile_file(&file_handle, ZEND_INCLUDE);

	if (op_array) {
		if (file_handle.handle.stream.handle) {
			if (!file_handle.opened_path) {
				file_handle.opened_path = zend_string_copy(filename);
			}
			zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path);
		}

		status = _phalcon_exec(ret, object, op_array, symbol_table);

		destroy_op_array(op_array);
		efree_size(op_array, sizeof(zend_op_array));
	}

	zend_destroy_file_handle(&file_handle);

	phalcon_destroy_symtable(symbol_table);
	return status;
}

int phalcon_exec_code(zval *ret, zval *object, zval *code, zval * vars) {
	zend_array *symbol_table;

	if (IS_STRING != Z_TYPE_P(code)) {
		return 0;
	}

	symbol_table = phalcon_build_symtable(vars);

	if (Z_STRLEN_P(code)) {
		zval phtml;
		zend_op_array *op_array;
		char *eval_desc = zend_make_compiled_string_description("template code");

		/* eval require code mustn't be wrapped in opening and closing PHP tags */
		ZVAL_STR(&phtml, strpprintf(0, "?>%s", Z_STRVAL_P(code)));

		op_array = zend_compile_string(&phtml, eval_desc);

		zval_ptr_dtor(&phtml);
		efree(eval_desc);

		if (op_array) {
			(void)_phalcon_exec(ret, object, op_array, symbol_table);
			destroy_op_array(op_array);
			efree(op_array);
		}
	}

	phalcon_destroy_symtable(symbol_table);

	return 1;
}

/**
 * Do an internal require to a plain php file taking care of the value returned by the file
 */
int phalcon_require_ret(zval *return_value_ptr, const char *require_path)
{
	zend_file_handle file_handle;
	zend_op_array *new_op_array;
	zval dummy, local_retval;
	char realpath[MAXPATHLEN];
	int ret;

	if (unlikely(!VCWD_REALPATH(require_path, realpath))) {
		//zend_error_noreturn(E_CORE_ERROR, "Failed opening file %s: %s", require_path, strerror(errno));
		return FAILURE;
	}

	ZVAL_UNDEF(&local_retval);

	file_handle.filename = require_path;
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	new_op_array = zend_compile_file(&file_handle, ZEND_REQUIRE);
	if (new_op_array) {

		if (file_handle.handle.stream.handle) {
			ZVAL_NULL(&dummy);
			if (!file_handle.opened_path) {
				file_handle.opened_path = zend_string_init(require_path, strlen(require_path), 0);
			}

			zend_hash_add(&EG(included_files), file_handle.opened_path, &dummy);
			zend_destroy_file_handle(&file_handle);
		}

#if PHP_VERSION_ID >= 70100
		new_op_array->scope = EG(fake_scope);
#else
		new_op_array->scope = EG(scope);
#endif
		zend_execute(new_op_array, &local_retval);

		if (return_value_ptr) {
			zval_ptr_dtor(return_value_ptr);
			ZVAL_COPY_VALUE(return_value_ptr, &local_retval);
		} else {
			zval_ptr_dtor(&local_retval);
		}

		destroy_op_array(new_op_array);
		efree_size(new_op_array, sizeof(zend_op_array));

		if (EG(exception)) {
			ret = FAILURE;
		} else {
			ret = SUCCESS;
		}

		return ret;
	} else {
		zend_destroy_file_handle(&file_handle);
	}

	return FAILURE;
}
