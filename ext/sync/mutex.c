
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

#include "sync/mutex.h"
#include "sync/exception.h"

#include "kernel/main.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/exception.h"
#include "kernel/time.h"

/**
 * Phalcon\Sync\Mutex
 *
 */
zend_class_entry *phalcon_sync_mutex_ce;

PHP_METHOD(Phalcon_Sync_Mutex, __construct);
PHP_METHOD(Phalcon_Sync_Mutex, lock);
PHP_METHOD(Phalcon_Sync_Mutex, unlock);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_sync_mutex___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_sync_mutex_lock, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, wait, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_sync_mutex_unlock, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, all, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_sync_mutex_method_entry[] = {
	PHP_ME(Phalcon_Sync_Mutex, __construct, arginfo_phalcon_sync_mutex___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Sync_Mutex, lock, arginfo_phalcon_sync_mutex_lock, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Sync_Mutex, unlock, arginfo_phalcon_sync_mutex_unlock, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_sync_mutex_object_handlers;
zend_object* phalcon_sync_mutex_object_create_handler(zend_class_entry *ce)
{
	phalcon_sync_mutex_object *intern = ecalloc(1, sizeof(phalcon_sync_mutex_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_sync_mutex_object_handlers;

	/* Initialize Mutex information. */
	intern->MxNamed = 0;
	intern->MxMem = NULL;
	pthread_mutex_init(&intern->MxPthreadCritSection, NULL);
	intern->MxOwnerID = 0;
	intern->MxCount = 0;

	return &intern->std;
}

void phalcon_sync_mutex_object_free_handler(zend_object *object)
{
	phalcon_sync_mutex_object *intern = phalcon_sync_mutex_object_from_obj(object);

	phalcon_mutex_unlock_internal(intern, 1);

	if (intern->MxMem != NULL)
	{
		if (intern->MxNamed) {
			phalcon_namedmem_unmap(intern->MxMem, phalcon_semaphore_getsize());
		} else {
			phalcon_semaphore_free(&intern->MxPthreadMutex);

			efree(intern->MxMem);
		}
	}

	pthread_mutex_destroy(&intern->MxPthreadCritSection);

}

/**
 * Phalcon\Sync\Mutex initializer
 */
PHALCON_INIT_CLASS(Phalcon_Sync_Mutex){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Sync, Mutex, sync_mutex, phalcon_sync_mutex_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Sync\Mutex constructor
 *
 * @param string $name
 */
PHP_METHOD(Phalcon_Sync_Mutex, __construct){

	zval *name = NULL;
	phalcon_sync_mutex_object *intern;
	size_t Pos, TempSize;
	int result;

	phalcon_fetch_params(0, 0, 1, &name);

	intern = phalcon_sync_mutex_object_from_obj(Z_OBJ_P(getThis()));

	if (!name || PHALCON_IS_EMPTY(name)) {
		intern->MxNamed = 0;
	} else {
		intern->MxNamed = 1;
	}

	TempSize = phalcon_semaphore_getsize();
	result = phalcon_namedmem_init(&intern->MxMem, &Pos, "/Sync_Mutex", intern->MxNamed ? Z_STRVAL_P(name) : NULL, TempSize);

	if (result < 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_sync_exception_ce, "Mutex could not be created");
		return;
	}

	phalcon_semaphore_get(&intern->MxPthreadMutex, intern->MxMem + Pos);

	/* Handle the first time this mutex has been opened. */
	if (result == 0) {
		phalcon_semaphore_init(&intern->MxPthreadMutex, intern->MxNamed, 1, 1);
		if (intern->MxNamed) phalcon_namedmem_ready(intern->MxMem);
	}
}

/**
 * Locks a mutex object
 */
PHP_METHOD(Phalcon_Sync_Mutex, lock){

	zval *_wait = NULL;
	zend_long wait = -1;
	phalcon_sync_mutex_object *intern;

	phalcon_fetch_params(0, 0, 1, &_wait);

	if (_wait && Z_TYPE_P(_wait) == IS_LONG) {
		wait = Z_LVAL_P(_wait);
	}

	intern = phalcon_sync_mutex_object_from_obj(Z_OBJ_P(getThis()));

	if (pthread_mutex_lock(&intern->MxPthreadCritSection) != 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_sync_exception_ce, "Unable to acquire mutex critical section");
		RETURN_FALSE;
	}

	/* Check to see if this mutex is already owned by the calling thread. */
	if (intern->MxOwnerID == phalcon_getcurrent_threadid()) {
		intern->MxCount++;
		pthread_mutex_unlock(&intern->MxPthreadCritSection);

		RETURN_TRUE;
	}

	pthread_mutex_unlock(&intern->MxPthreadCritSection);

	if (!phalcon_semaphore_wait(&intern->MxPthreadMutex, (uint32_t)(wait > -1 ? wait : INFINITE))) RETURN_FALSE;

	pthread_mutex_lock(&intern->MxPthreadCritSection);
	intern->MxOwnerID = phalcon_getcurrent_threadid();
	intern->MxCount = 1;
	pthread_mutex_unlock(&intern->MxPthreadCritSection);

	RETURN_TRUE;
}

/**
 * Unlocks a mutex object
 *
 * @param boolean $all
 */
PHP_METHOD(Phalcon_Sync_Mutex, unlock){

	zval *all = NULL;
	phalcon_sync_mutex_object *intern;

	phalcon_fetch_params(0, 0, 1, &all);

	if (!all || Z_TYPE_P(all) == IS_NULL) {
		all = &PHALCON_GLOBAL(z_false);
	}

	intern = phalcon_sync_mutex_object_from_obj(Z_OBJ_P(getThis()));

	if (!phalcon_mutex_unlock_internal(intern, zend_is_true(all) ? 1 : 0)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
