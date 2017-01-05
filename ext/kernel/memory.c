
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
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
  +------------------------------------------------------------------------+
*/

#include "kernel/memory.h"

#include <Zend/zend_alloc.h>

#include "kernel/fcall.h"
#include "kernel/backtrace.h"
#include "kernel/framework/orm.h"

/*
 * Memory Frames/Virtual Symbol Scopes
 *------------------------------------
 *
 * Phalcon uses memory frames to track the variables used within a method
 * in order to free them or reduce their reference counting accordingly before
 * exit the method in execution.
 *
 * This adds a minimum overhead to execution but save us the work of
 * free memory in each method.
 *
 * The whole memory frame is an open double-linked list which start is an
 * allocated empty frame that points to the real first frame. The start
 * memory frame is globally accesed using PHALCON_GLOBAL(start_frame)
 *
 * Not all methods must grow/restore the phalcon_memory_entry.
 */

void phalcon_initialize_memory(zend_phalcon_globals *phalcon_globals_ptr)
{
	ZVAL_NULL(&phalcon_globals_ptr->z_null);
	ZVAL_FALSE(&phalcon_globals_ptr->z_false);
	ZVAL_TRUE(&phalcon_globals_ptr->z_true);
	ZVAL_LONG(&phalcon_globals_ptr->z_zero, 0);
	ZVAL_LONG(&phalcon_globals_ptr->z_one, 1);
	ZVAL_LONG(&phalcon_globals_ptr->z_two, 2);

	phalcon_globals_ptr->initialized = 1;
}

void phalcon_deinitialize_memory()
{
	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;

	if (phalcon_globals_ptr->initialized != 1) {
		phalcon_globals_ptr->initialized = 0;
		return;
	}

	phalcon_orm_destroy_cache();

	phalcon_globals_ptr->initialized = 0;
}

/**
 * Exports symbols to the active symbol table
 */
int phalcon_set_symbol(zend_array *symbol_table, zval *key_name, zval *value)
{
	if (!symbol_table) {
		php_error_docref(NULL, E_WARNING, "Cannot find a valid symbol_table");
		return FAILURE;
	}

	if (Z_TYPE_P(key_name) == IS_STRING) {
		Z_TRY_ADDREF_P(value);
		zend_hash_update(symbol_table, Z_STR_P(key_name), value);
	}

	return SUCCESS;
}

/**
 * Exports a string symbol to the active symbol table
 */
int phalcon_set_symbol_str(zend_array *symbol_table, char *key_name, unsigned int key_length, zval *value)
{
	if (!symbol_table) {
		php_error_docref(NULL, E_WARNING, "Cannot find a valid symbol_table");
		return FAILURE;
	}

	Z_TRY_ADDREF_P(value);
	zend_hash_str_update(symbol_table, key_name, key_length, value);

	return SUCCESS;
}

int phalcon_del_symbol(zend_array *symbol_table, zval *key_name)
{
	if (!symbol_table) {
		php_error_docref(NULL, E_WARNING, "Cannot find a valid symbol_table");
		return FAILURE;
	}

	if (Z_TYPE_P(key_name) == IS_STRING) {
		zend_hash_del(symbol_table, Z_STR_P(key_name));
	}

	return SUCCESS;
}

int phalcon_del_symbol_str(zend_array *symbol_table, char *key_name, unsigned int key_length)
{
	if (!symbol_table) {
		php_error_docref(NULL, E_WARNING, "Cannot find a valid symbol_table");
		return FAILURE;
	}

	zend_hash_str_del(symbol_table, key_name, key_length);

	return SUCCESS;
}
