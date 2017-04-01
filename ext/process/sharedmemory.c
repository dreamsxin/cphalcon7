
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
  +------------------------------------------------------------------------+
*/

#include "process/sharedmemory.h"
#include "process/exception.h"

#include "kernel/main.h"
#include "kernel/shm.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/operators.h"

/**
 * Phalcon\Process\Sharedmemory
 *
 * This class defines sharedmemory entity and its description
 *
 */
zend_class_entry *phalcon_process_sharedmemory_ce;

PHP_METHOD(Phalcon_Process_Sharedmemory, __construct);
PHP_METHOD(Phalcon_Process_Sharedmemory, isOpen);
PHP_METHOD(Phalcon_Process_Sharedmemory, getName);
PHP_METHOD(Phalcon_Process_Sharedmemory, getSize);
PHP_METHOD(Phalcon_Process_Sharedmemory, open);
PHP_METHOD(Phalcon_Process_Sharedmemory, create);
PHP_METHOD(Phalcon_Process_Sharedmemory, lock);
PHP_METHOD(Phalcon_Process_Sharedmemory, unlock);
PHP_METHOD(Phalcon_Process_Sharedmemory, read);
PHP_METHOD(Phalcon_Process_Sharedmemory, write);
PHP_METHOD(Phalcon_Process_Sharedmemory, close);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_sharedmemory___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_sharedmemory_create, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_sharedmemory_lock, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, blocking, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_sharedmemory_unlock, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, force, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_sharedmemory_write, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_process_sharedmemory_method_entry[] = {
	PHP_ME(Phalcon_Process_Sharedmemory, __construct, arginfo_phalcon_process_sharedmemory___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Process_Sharedmemory, isOpen, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, getName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, getSize, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, open, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, create, arginfo_phalcon_process_sharedmemory_create, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, lock, arginfo_phalcon_process_sharedmemory_lock, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, unlock, arginfo_phalcon_process_sharedmemory_unlock, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, read, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, write, arginfo_phalcon_process_sharedmemory_write, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static void phalcon_sharedmemory_dtor(zend_resource *rsrc)
{
    phalcon_shared_memory *shm = (phalcon_shared_memory *) rsrc->ptr;
    phalcon_shared_memory_cleanup(shm);
}

int phalcon_sharedmemory_handle;

/**
 * Phalcon\Process\Sharedmemory initializer
 */
PHALCON_INIT_CLASS(Phalcon_Process_Sharedmemory){

	PHALCON_REGISTER_CLASS(Phalcon\\Process, Sharedmemory, process_sharedmemory, phalcon_process_sharedmemory_method_entry, 0);

	phalcon_sharedmemory_handle = zend_register_list_destructors_ex(phalcon_sharedmemory_dtor, NULL, phalcon_sharedmemory_handle_name, module_number);

	zend_declare_property_null(phalcon_process_sharedmemory_ce, SL("_name"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_sharedmemory_ce, SL("_shm"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Process\Sharedmemory constructor
 */
PHP_METHOD(Phalcon_Process_Sharedmemory, __construct){

	zval *name;

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_update_property(getThis(), SL("_name"), name);
}

/**
 * Checks if open a shared memory file
 */
PHP_METHOD(Phalcon_Process_Sharedmemory, isOpen){

	zval shm = {};
	phalcon_shared_memory *m;

	phalcon_read_property(&shm, getThis(), SL("_shm"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(shm) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	if ((m = (phalcon_shared_memory *)zend_fetch_resource(Z_RES(shm), phalcon_sharedmemory_handle_name, phalcon_sharedmemory_handle)) == NULL) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

/**
 * Gets the name
 */
PHP_METHOD(Phalcon_Process_Sharedmemory, getName){

	RETURN_MEMBER(getThis(), "_name");
}

/**
 * Gets the size
 */
PHP_METHOD(Phalcon_Process_Sharedmemory, getSize){

	zval shm = {};
	phalcon_shared_memory *m;

	phalcon_read_property(&shm, getThis(), SL("_shm"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(shm) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	if ((m = (phalcon_shared_memory *)zend_fetch_resource(Z_RES(shm), phalcon_sharedmemory_handle_name, phalcon_sharedmemory_handle)) == NULL) {
		RETURN_FALSE;
	}
	RETURN_LONG(phalcon_shared_memory_size(m));
}

/**
 * Open a shared memory
 */
PHP_METHOD(Phalcon_Process_Sharedmemory, open){

	zval name = {}, shm = {};
	phalcon_shared_memory *m;

	phalcon_read_property(&name, getThis(), SL("_name"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&shm, getThis(), SL("_shm"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(shm) != IS_NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_process_exception_ce, "Already create or open shared memory");
		return;
	}

	m = phalcon_shared_memory_open(Z_STRVAL(name));
	if (NULL == m) {
		RETURN_FALSE;
	}

	ZVAL_RES(&shm, zend_register_resource(m, phalcon_sharedmemory_handle));
	phalcon_update_property(getThis(), SL("_shm"), &shm);
	RETURN_TRUE;
}

/**
 * Create a shared memory
 */
PHP_METHOD(Phalcon_Process_Sharedmemory, create){

	zval *size, name = {}, shm = {};
	phalcon_shared_memory *m;

	phalcon_fetch_params(0, 1, 0, &size);

	phalcon_read_property(&name, getThis(), SL("_name"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&shm, getThis(), SL("_shm"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(shm) != IS_NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_process_exception_ce, "Already create or open shared memory");
		return;
	}

	m = phalcon_shared_memory_create(Z_STRVAL(name), Z_LVAL_P(size));
	if (NULL == m) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_process_exception_ce, "Failed to create shared memory");
		return;
	}

	ZVAL_RES(&shm, zend_register_resource(m, phalcon_sharedmemory_handle));
	phalcon_update_property(getThis(), SL("_shm"), &shm);
	RETURN_TRUE;
}

/**
 * Lock the shared memory
 */
PHP_METHOD(Phalcon_Process_Sharedmemory, lock){

	zval *blocking = NULL, shm = {};
	phalcon_shared_memory *m;

	phalcon_fetch_params(0, 0, 1, &blocking);

	if (!blocking) {
		blocking = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&shm, getThis(), SL("_shm"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(shm) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	if ((m = (phalcon_shared_memory *)zend_fetch_resource(Z_RES(shm), phalcon_sharedmemory_handle_name, phalcon_sharedmemory_handle)) == NULL) {
		RETURN_FALSE;
	}
	if (!zend_is_true(blocking)) {
		RETURN_BOOL(!phalcon_shared_memory_trylock(m));
	} else {
		RETURN_BOOL(!phalcon_shared_memory_lock(m));
	}
}

/**
 * Unock the shared memory
 */
PHP_METHOD(Phalcon_Process_Sharedmemory, unlock){

	zval *force = NULL, shm = {};
	phalcon_shared_memory *m;

	phalcon_fetch_params(0, 0, 1, &force);

	if (!force) {
		force = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&shm, getThis(), SL("_shm"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(shm) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	if ((m = (phalcon_shared_memory *)zend_fetch_resource(Z_RES(shm), phalcon_sharedmemory_handle_name, phalcon_sharedmemory_handle)) == NULL) {
		RETURN_FALSE;
	}
	if (!zend_is_true(force)) {
		RETURN_BOOL(!phalcon_shared_memory_unlock(m));
	} else {
		RETURN_BOOL(!phalcon_shared_memory_unlock_force(m));
	}
}

/**
 * Read the shared memory
 */
PHP_METHOD(Phalcon_Process_Sharedmemory, read){

	zval shm = {};
	phalcon_shared_memory *m;
	char *mem;
	size_t size, value_size;

	phalcon_read_property(&shm, getThis(), SL("_shm"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(shm) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	if ((m = (phalcon_shared_memory *)zend_fetch_resource(Z_RES(shm), phalcon_sharedmemory_handle_name, phalcon_sharedmemory_handle)) == NULL) {
		RETURN_FALSE;
	}
	mem = (char*)phalcon_shared_memory_ptr(m);
  	if (!mem) {
		RETURN_FALSE;
	}
	size = phalcon_shared_memory_size(m);

	if (size <= 0) {
		RETURN_FALSE;
	}

	value_size = strnlen(mem, size);
	if (size <= value_size) {
		RETURN_FALSE;
	}

	RETURN_NEW_STR(zend_string_init(mem, value_size, 0));
}

/**
 * Write the shared memory
 */
PHP_METHOD(Phalcon_Process_Sharedmemory, write){

	zval *value, shm = {};
	phalcon_shared_memory *m;
	char *mem;
	size_t size, value_size;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_read_property(&shm, getThis(), SL("_shm"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(shm) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	if ((m = (phalcon_shared_memory *)zend_fetch_resource(Z_RES(shm), phalcon_sharedmemory_handle_name, phalcon_sharedmemory_handle)) == NULL) {
		RETURN_FALSE;
	}

	mem = (char*)phalcon_shared_memory_ptr(m);
  	if (!mem) {
		RETURN_FALSE;
	}

	size = phalcon_shared_memory_size(m);
	if (size <= 0) {
		RETURN_FALSE;
	}

	value_size = Z_STRLEN_P(value);
	if (size <= value_size) {
		RETURN_FALSE;
	}

	memcpy(mem, Z_STRVAL_P(value), value_size);
	mem[value_size] = 0;
	RETURN_TRUE;
}
