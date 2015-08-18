
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
#include "php_phalcon.h"

#include <ext/spl/spl_exceptions.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_inheritance.h>

#include "kernel/main.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"


/**
 * Initialize globals on each request or each thread started
 */
void php_phalcon_init_globals(zend_phalcon_globals *phalcon_globals) {

	HashTable *constants = EG(zend_constants);

	phalcon_globals->initialized = 0;

	/* Memory options */
	phalcon_globals->active_memory = NULL;

	/* Recursive Lock */
	phalcon_globals->recursive_lock = 0;

	/* ORM options*/
	phalcon_globals->orm.events = 1;
	phalcon_globals->orm.virtual_foreign_keys = 1;
	phalcon_globals->orm.column_renaming = 1;
	phalcon_globals->orm.not_null_validations = 1;
	phalcon_globals->orm.length_validations = 1;
	phalcon_globals->orm.exception_on_failed_save = 0;
	phalcon_globals->orm.enable_literals = 1;
	phalcon_globals->orm.enable_ast_cache = 1;
	phalcon_globals->orm.cache_level = 3;
	phalcon_globals->orm.ast_cache = NULL;
	phalcon_globals->orm.enable_property_method = 1;
	phalcon_globals->orm.enable_auto_convert = 1;
	phalcon_globals->orm.allow_update_primary = 0;
	phalcon_globals->orm.enable_strict = 0;

	/* Security options */
	phalcon_globals->security.crypt_std_des_supported  = zend_hash_str_exists(constants, SL("CRYPT_STD_DES"));
	phalcon_globals->security.crypt_ext_des_supported  = zend_hash_str_exists(constants, SL("CRYPT_EXT_DES"));
	phalcon_globals->security.crypt_md5_supported      = zend_hash_str_exists(constants, SL("CRYPT_MD5"));
	phalcon_globals->security.crypt_blowfish_supported = zend_hash_str_exists(constants, SL("CRYPT_BLOWFISH"));
	phalcon_globals->security.crypt_sha256_supported   = zend_hash_str_exists(constants, SL("CRYPT_SHA256"));
	phalcon_globals->security.crypt_sha512_supported   = zend_hash_str_exists(constants, SL("CRYPT_SHA512"));

	phalcon_globals->security.crypt_blowfish_y_supported = phalcon_globals->security.crypt_blowfish_supported;

	/* DB options */
	phalcon_globals->db.escape_identifiers = 1;
}

/**
 * Initializes internal interface with extends
 */
zend_class_entry *phalcon_register_internal_interface_ex(zend_class_entry *orig_ce, zend_class_entry *parent_ce) {

	zend_class_entry *ce;

	ce = zend_register_internal_interface(orig_ce);
	if (parent_ce) {
		zend_do_inheritance(ce, parent_ce);
	}

	return ce;
}

/**
 * Gets the global zval into PG macro
 */
zval* phalcon_get_global(const char *global, unsigned int global_length) {

	zend_bool jit_initialization = PG(auto_globals_jit);
	if (jit_initialization) {
		zend_is_auto_global_str((char *)global, global_length - 1);
	}

	if (&EG(symbol_table)) {
		zval *gv;
		if ((gv = zend_hash_str_find(&EG(symbol_table), global, global_length)) != NULL) {
			if (gv && Z_TYPE_P(gv) == IS_ARRAY) {
				return gv;
			}
		}
	}

	return &PHALCON_GLOBAL(z_null);
}

/**
 * Makes fast count on implicit array types
 */
long int phalcon_fast_count_int(zval *value) {

	if (Z_TYPE_P(value) == IS_ARRAY) {
		return zend_hash_num_elements(Z_ARRVAL_P(value));
	}

	if (Z_TYPE_P(value) == IS_OBJECT) {
		if (Z_OBJ_HT_P(value)->count_elements) {
			long int result;
			if (SUCCESS == Z_OBJ_HT(*value)->count_elements(value, &result)) {
				return result;
			}
		}

		if (instanceof_function_ex(Z_OBJCE_P(value), spl_ce_Countable, 1)) {
			zval retval;
			long int result = 0;

			zend_call_method_with_0_params(value, Z_OBJCE_P(value), NULL, "count", &retval);
			if (!Z_ISUNDEF(retval)) {
				convert_to_long_ex(&retval);
				result = Z_LVAL(retval);
				zval_dtor(&retval);
			}

			return result;
		}

		return 0;
	}

	if (Z_TYPE_P(value) == IS_NULL) {
		return 0;
	}

	return 1;
}

/**
 * Makes fast count on implicit array types
 */
void phalcon_fast_count(zval *result, zval *value) {

	if (Z_TYPE_P(value) == IS_ARRAY) {
		ZVAL_LONG(result, zend_hash_num_elements(Z_ARRVAL_P(value)));
		return;
	}

	if (Z_TYPE_P(value) == IS_OBJECT) {
		zval retval;

		if (Z_OBJ_HT_P(value)->count_elements) {
			ZVAL_LONG(result, 1);
			if (SUCCESS == Z_OBJ_HT(*value)->count_elements(value, &Z_LVAL_P(result))) {
				return;
			}
		}

		if (instanceof_function(Z_OBJCE_P(value), spl_ce_Countable)) {
			zend_call_method_with_0_params(value, NULL, NULL, "count", &retval);
			if (!Z_ISUNDEF(retval)) {
				convert_to_long_ex(&retval);
				ZVAL_LONG(result, Z_LVAL(retval));
				zval_dtor(&retval);
			}
			return;
		}

		ZVAL_LONG(result, 0);
		return;
	}

	if (Z_TYPE_P(value) == IS_NULL) {
		ZVAL_LONG(result, 0);
		return;
	}

	ZVAL_LONG(result, 1);
}

/**
 * Makes fast count on implicit array types without creating a return zval value
 */
int phalcon_fast_count_ev(zval *value) {

	long count = 0;

	if (Z_TYPE_P(value) == IS_ARRAY) {
		return (int) zend_hash_num_elements(Z_ARRVAL_P(value)) > 0;
	}

	if (Z_TYPE_P(value) == IS_OBJECT) {
		zval retval;

		if (Z_OBJ_HT_P(value)->count_elements) {
			Z_OBJ_HT(*value)->count_elements(value, &count);
			return (int) count > 0;
		}

		if (instanceof_function(Z_OBJCE_P(value), spl_ce_Countable)) {
			zend_call_method_with_0_params(value, NULL, NULL, "count", &retval);
			if (!Z_ISUNDEF(retval)) {
				convert_to_long_ex(&retval);
				count = Z_LVAL(retval);
				zval_dtor(&retval);
				return (int) count > 0;
			}
			return 0;
		}

		return 0;
	}

	if (Z_TYPE_P(value) == IS_NULL) {
		return 0;
	}

	return 1;
}

/**
 * Check if a function exists using explicit char param (using precomputed hash key)
 */
int phalcon_function_exists_ex(const char *method_name, unsigned int method_len) {

	return (zend_hash_str_exists(CG(function_table), method_name, method_len)) ? SUCCESS : FAILURE;
}

/**
 * Checks if a zval is callable
 */
int phalcon_is_callable(zval *var) {

	char *error = NULL;
	zend_bool retval;

	retval = zend_is_callable_ex(var, NULL, 0, NULL, NULL, &error);
	if (error) {
		efree(error);
	}

	return (int) retval;
}

/**
 * Parses method parameters with minimum overhead
 */
int phalcon_fetch_parameters(int num_args, int required_args, int optional_args, ...)
{
	va_list va;
	zval **arg, *param;
	int arg_count;
	int use_args_num;

	param = ZEND_CALL_ARG(EG(current_execute_data), 1);
	arg_count = ZEND_CALL_NUM_ARGS(EG(current_execute_data));

	if (num_args < required_args) {
		phalcon_throw_exception_string(spl_ce_BadMethodCallException, "Wrong number of parameters");
		return FAILURE;
	}

	if (num_args > arg_count) {
		phalcon_throw_exception_string(spl_ce_BadMethodCallException, "Could not obtain parameters for parsing");
		return FAILURE;
	}

	if (!num_args) {
		return SUCCESS;
	}

	va_start(va, optional_args);

	use_args_num = required_args + optional_args;
	num_args = num_args > use_args_num ? use_args_num : num_args;

	while (num_args-- > 0) {
		arg = va_arg(va, zval **);
		*arg = param;
		param++;
	}

	va_end(va);

	return SUCCESS;
}

void phalcon_clean_and_cache_symbol_table(zend_array *symbol_table)
{
	if (EG(symtable_cache_ptr) >= EG(symtable_cache_limit)) {
		zend_array_destroy(symbol_table);
	} else {
		zend_symtable_clean(symbol_table);
		*(++EG(symtable_cache_ptr)) = symbol_table;
	}
}
