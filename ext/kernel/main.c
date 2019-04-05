
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

	/* Recursive Lock */
	phalcon_globals->recursive_lock = 0;

	/* ORM options*/
	phalcon_globals->orm.events = 1;
	phalcon_globals->orm.virtual_foreign_keys = 1;
	phalcon_globals->orm.not_null_validations = 1;
	phalcon_globals->orm.length_validations = 1;
	phalcon_globals->orm.use_mb_strlen = 1;
	phalcon_globals->orm.exception_on_failed_save = 0;
	phalcon_globals->orm.enable_literals = 1;
	phalcon_globals->orm.cache_level = 3;
	phalcon_globals->orm.ast_cache = NULL;
	phalcon_globals->orm.enable_property_method = 1;
	phalcon_globals->orm.enable_auto_convert = 1;
	phalcon_globals->orm.allow_update_primary = 0;
	phalcon_globals->orm.enable_strict = 0;
	phalcon_globals->orm.must_column = 1;

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
#if PHALCON_USE_UV
	/* Async options */
	phalcon_globals->async.fs_enabled = 0;
#endif
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
int phalcon_read_global_str(zval *return_value, const char *global, unsigned int global_length) {
	if (PG(auto_globals_jit)) {
		zend_is_auto_global_str((char *)global, global_length);
	}

	if (&EG(symbol_table)) {
		zval *gv;
		if ((gv = zend_hash_str_find(&EG(symbol_table), global, global_length)) != NULL) {
			if (EXPECTED(Z_TYPE_P(gv) == IS_REFERENCE)) {
				gv = Z_REFVAL_P(gv);
			}
			if (Z_TYPE_P(gv) == IS_ARRAY) {
				ZVAL_COPY_VALUE(return_value, gv);
				return 1;
			}
		}
	}

	return 0;
}

zval* phalcon_get_global_str(const char *global, unsigned int global_length) {

	if (PG(auto_globals_jit)) {
		zend_is_auto_global_str((char *)global, global_length);
	}

	if (&EG(symbol_table)) {
		zval *gv;
		if ((gv = zend_hash_str_find(&EG(symbol_table), global, global_length)) != NULL) {
			if (EXPECTED(Z_TYPE_P(gv) == IS_REFERENCE)) {
				gv = Z_REFVAL_P(gv);
			}
			if (Z_TYPE_P(gv) == IS_ARRAY) {
				return gv;
			}
		}
	}

	return &PHALCON_GLOBAL(z_null);
}

/**
 * Gets the global zval into PG macro
 */
zval* phalcon_get_global(zend_string *name) {

	if (PG(auto_globals_jit)) {
		zend_is_auto_global_str((char *)name->val, name->len);
	}
	if (&EG(symbol_table)) {
		zval *gv;
		if ((gv = zend_hash_find(&EG(symbol_table), name)) != NULL) {
			if (EXPECTED(Z_TYPE_P(gv) == IS_REFERENCE)) {
				gv = Z_REFVAL_P(gv);
			}
			if (Z_TYPE_P(gv) == IS_ARRAY) {
				return gv;
			}
		}
	}

	return &PHALCON_GLOBAL(z_null);
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
 * Returns the type of a variable as a string
 */
void phalcon_gettype(zval *return_value, zval *arg)
{
	switch (Z_TYPE_P(arg)) {

		case IS_NULL:
			RETVAL_STRING("NULL");
			break;

		case IS_TRUE:
		case IS_FALSE:
			RETVAL_STRING("boolean");
			break;

		case IS_LONG:
			RETVAL_STRING("integer");
			break;

		case IS_DOUBLE:
			RETVAL_STRING("double");
			break;

		case IS_STRING:
			RETVAL_STRING("string");
			break;

		case IS_ARRAY:
			RETVAL_STRING("array");
			break;

		case IS_OBJECT:
			RETVAL_STRING("object");
			break;

		case IS_RESOURCE:
			{
				const char *type_name = zend_rsrc_list_get_rsrc_type(Z_RES_P(arg));

				if (type_name) {
					RETVAL_STRING("resource");
					break;
				}
			}

		default:
			RETVAL_STRING("unknown type");
	}
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
		phalcon_throw_exception_format(spl_ce_BadMethodCallException, "Wrong number of parameters, num:%d, required:%d", num_args, required_args);
		return FAILURE;
	}

	if (num_args > arg_count) {
		phalcon_throw_exception_format(spl_ce_BadMethodCallException, "Could not obtain parameters for parsing, num:%d, count:%d", num_args, arg_count);
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

int phalcon_has_constant(const char *name, size_t name_len)
{
	zval *constant;
	if ((constant = zend_get_constant_str(name, name_len)) == NULL) {
		return 0;
	}

	return 1;
}

int phalcon_get_constant(zval *retval, const char *name, size_t name_len)
{
	zval *constant;
	if ((constant = zend_get_constant_str(name, name_len)) == NULL) {
		return 0;
	}

	ZVAL_COPY(retval, constant);
	return 1;
}
