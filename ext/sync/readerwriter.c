
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

#include "sync/readerwriter.h"
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
 * Phalcon\Sync\Readerwriter
 *
 */
zend_class_entry *phalcon_sync_readerwriter_ce;

PHP_METHOD(Phalcon_Sync_Readerwriter, __construct);
PHP_METHOD(Phalcon_Sync_Readerwriter, readlock);
PHP_METHOD(Phalcon_Sync_Readerwriter, readunlock);
PHP_METHOD(Phalcon_Sync_Readerwriter, writelock);
PHP_METHOD(Phalcon_Sync_Readerwriter, writeunlock);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_sync_readerwriter___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, autounlock, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_sync_readerwriter_readlock, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, wait, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_sync_readerwriter_writelock, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, wait, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_sync_readerwriter_method_entry[] = {
	PHP_ME(Phalcon_Sync_Readerwriter, __construct, arginfo_phalcon_sync_readerwriter___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Sync_Readerwriter, readlock, arginfo_phalcon_sync_readerwriter_readlock, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Sync_Readerwriter, readunlock, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Sync_Readerwriter, writelock, arginfo_phalcon_sync_readerwriter_writelock, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Sync_Readerwriter, writeunlock, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_sync_readerwriter_object_handlers;
zend_object* phalcon_sync_readerwriter_object_create_handler(zend_class_entry *ce)
{
	phalcon_sync_readerwriter_object *intern = ecalloc(1, sizeof(phalcon_sync_readerwriter_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_sync_readerwriter_object_handlers;

	intern->MxNamed = 0;
	intern->MxMem = NULL;
	intern->MxRCount = NULL;

	intern->MxAutoUnlock = 1;
	intern->MxReadLocks = 0;
	intern->MxWriteLock = 0;

	return &intern->std;
}

void phalcon_sync_readerwriter_object_free_handler(zend_object *object)
{
	phalcon_sync_readerwriter_object *intern = phalcon_sync_readerwriter_object_from_obj(object);

	if (intern->MxAutoUnlock) {
		while (intern->MxReadLocks) {
			phalcon_readerwriter_readunlock_internal(intern);
		}

		if (intern->MxWriteLock) phalcon_readerwriter_writeunlock_internal(intern);
	}
	
	if (intern->MxMem != NULL) {
		if (intern->MxNamed) {
			phalcon_namedmem_unmap(intern->MxMem, phalcon_semaphore_getsize() + phalcon_getalignsize(sizeof(uint32_t)) + phalcon_event_getsize() + phalcon_semaphore_getsize());
		} else {
			phalcon_semaphore_free(&intern->MxPthreadRCountMutex);
			phalcon_event_free(&intern->MxPthreadRWaitEvent);
			phalcon_semaphore_free(&intern->MxPthreadWWaitMutex);

			efree(intern->MxMem);
		}
	}
}

/**
 * Phalcon\Sync\Readerwriter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Sync_Readerwriter){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Sync, Readerwriter, sync_readerwriter, phalcon_sync_readerwriter_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Sync\Readerwriter constructor
 *
 * @param string $name
 * @param boolean $autounlock
 */
PHP_METHOD(Phalcon_Sync_Readerwriter, __construct){

	zval *name = NULL, *autounlock = NULL;
	size_t Pos, TempSize;
	phalcon_sync_readerwriter_object *intern;
	int Result;
	char *MemPtr;

	phalcon_fetch_params(0, 0, 2, &name, &autounlock);

	if (!autounlock) {
		autounlock = &PHALCON_GLOBAL(z_true);
	}

	intern = phalcon_sync_readerwriter_object_from_obj(Z_OBJ_P(getThis()));

	if (!name || PHALCON_IS_EMPTY(name)) {
		intern->MxNamed = 0;
	} else {
		intern->MxNamed = 1;
	}

	intern->MxAutoUnlock = zend_is_true(autounlock) ? 1 : 0;

	TempSize = phalcon_semaphore_getsize() + phalcon_getalignsize(sizeof(uint32_t)) + phalcon_event_getsize() + phalcon_semaphore_getsize();

	Result = phalcon_namedmem_init(&intern->MxMem, &Pos, "/Sync_ReadWrite", intern->MxNamed ? Z_STRVAL_P(name) : NULL, TempSize);

	if (Result < 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_sync_exception_ce, "Reader-Writer object could not be created");
		return;
	}

	/* Load the pointers. */
	MemPtr = intern->MxMem + Pos;
	phalcon_semaphore_get(&intern->MxPthreadRCountMutex, MemPtr);
	MemPtr += phalcon_semaphore_getsize();

	intern->MxRCount = (volatile uint32_t *)(MemPtr);
	MemPtr += phalcon_getalignsize(sizeof(uint32_t));

	phalcon_event_get(&intern->MxPthreadRWaitEvent, MemPtr);
	MemPtr += phalcon_event_getsize();

	phalcon_semaphore_get(&intern->MxPthreadWWaitMutex, MemPtr);

	/* Handle the first time this reader/writer lock has been opened. */
	if (Result == 0) {
		phalcon_semaphore_init(&intern->MxPthreadRCountMutex, intern->MxNamed, 1, 1);
		intern->MxRCount[0] = 0;
		phalcon_event_init(&intern->MxPthreadRWaitEvent, intern->MxNamed, 1, 1);
		phalcon_semaphore_init(&intern->MxPthreadWWaitMutex, intern->MxNamed, 1, 1);

		if (intern->MxNamed) phalcon_namedmem_ready(intern->MxMem);
	}
}

/**
 * Read locks a reader-writer object
 */
PHP_METHOD(Phalcon_Sync_Readerwriter, readlock){

	zval *wait = NULL;
	uint32_t waitamt;
	uint64_t starttime, currtime;
	phalcon_sync_readerwriter_object *intern;

	phalcon_fetch_params(0, 0, 1, &wait);

	intern = phalcon_sync_readerwriter_object_from_obj(Z_OBJ_P(getThis()));

	if (wait && Z_TYPE_P(wait) == IS_LONG && Z_LVAL_P(wait) > -1) {
		waitamt = Z_LVAL_P(wait);
	} else {
		waitamt = INFINITE;
	}

	/* Get current time in milliseconds. */
	starttime = (waitamt == INFINITE ? 0 : phalcon_getmicrosecondtime() / 1000000);

	/* Acquire the write lock mutex.  Guarantees that readers can't starve the writer. */
	if (!phalcon_semaphore_wait(&intern->MxPthreadWWaitMutex, waitamt))  RETURN_FALSE;

	/* Acquire the counter mutex. */
	currtime = (waitamt == INFINITE ? 0 : phalcon_getmicrosecondtime() / 1000000);
	if (waitamt < currtime - starttime || !phalcon_semaphore_wait(&intern->MxPthreadRCountMutex, waitamt - (currtime - starttime)))
	{
		phalcon_semaphore_release(&intern->MxPthreadWWaitMutex, NULL);

		RETURN_FALSE;
	}

	/* Update the event state. */
	if (!phalcon_event_reset(&intern->MxPthreadRWaitEvent)) {
		phalcon_semaphore_release(&intern->MxPthreadRCountMutex, NULL);
		phalcon_semaphore_release(&intern->MxPthreadWWaitMutex, NULL);

		RETURN_FALSE;
	}

	/* Increment the number of readers. */
	intern->MxRCount[0]++;

	intern->MxReadLocks++;

	/* Release the mutexes. */
	phalcon_semaphore_release(&intern->MxPthreadRCountMutex, NULL);
	phalcon_semaphore_release(&intern->MxPthreadWWaitMutex, NULL);

	RETURN_TRUE;
}

/**
 * Read unlocks a reader-writer object
 *
 */
PHP_METHOD(Phalcon_Sync_Readerwriter, readunlock){

	phalcon_sync_readerwriter_object *intern;

	intern = phalcon_sync_readerwriter_object_from_obj(Z_OBJ_P(getThis()));

	if (!phalcon_readerwriter_readunlock_internal(intern)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

/**
 * Write locks a reader-writer object
 */
PHP_METHOD(Phalcon_Sync_Readerwriter, writelock){

	zval *wait = NULL;
	uint32_t waitamt;
	uint64_t starttime, currtime;
	phalcon_sync_readerwriter_object *intern;

	phalcon_fetch_params(0, 0, 1, &wait);

	intern = phalcon_sync_readerwriter_object_from_obj(Z_OBJ_P(getThis()));

	if (wait && Z_TYPE_P(wait) == IS_LONG && Z_LVAL_P(wait) > -1) {
		waitamt = Z_LVAL_P(wait);
	} else {
		waitamt = INFINITE;
	}

	/* Get current time in milliseconds. */
	starttime = (waitamt == INFINITE ? 0 : phalcon_getmicrosecondtime() / 1000000);

	/* Acquire the write lock mutex. */
	if (!phalcon_semaphore_wait(&intern->MxPthreadWWaitMutex, waitamt))  RETURN_FALSE;

	/* Wait for readers to reach zero. */
	currtime = (waitamt == INFINITE ? 0 : phalcon_getmicrosecondtime() / 1000000);
	if (waitamt < currtime - starttime || !phalcon_event_wait(&intern->MxPthreadRWaitEvent, waitamt - (currtime - starttime)))
	{
		phalcon_semaphore_release(&intern->MxPthreadWWaitMutex, NULL);

		RETURN_FALSE;
	}

	intern->MxWriteLock = 1;

	RETURN_TRUE;
}

/**
 * Write unlocks a reader-writer object
 *
 */
PHP_METHOD(Phalcon_Sync_Readerwriter, writeunlock){

	phalcon_sync_readerwriter_object *intern;

	intern = phalcon_sync_readerwriter_object_from_obj(Z_OBJ_P(getThis()));

	if (!phalcon_readerwriter_writeunlock_internal(intern)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
