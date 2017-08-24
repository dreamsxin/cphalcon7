
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

#ifndef PHALCON_SYNC_READERWRITER_H
#define PHALCON_SYNC_READERWRITER_H

#include "php_phalcon.h"
#if PHALCON_USE_SHM_OPEN
#include "sync/common.h"

/* Reader-Writer */
typedef struct _phalcon_sync_readerwriter_object {
	int MxNamed;
	char *MxMem;
	phalcon_semaphore_wrapper MxPthreadRCountMutex;
	volatile uint32_t *MxRCount;
	phalcon_event_wrapper MxPthreadRWaitEvent;
	phalcon_semaphore_wrapper MxPthreadWWaitMutex;

	int MxAutoUnlock;
	volatile unsigned int MxReadLocks, MxWriteLock;

	zend_object std;
} phalcon_sync_readerwriter_object;

static inline phalcon_sync_readerwriter_object *phalcon_sync_readerwriter_object_from_obj(zend_object *obj) {
	return (phalcon_sync_readerwriter_object*)((char*)(obj) - XtOffsetOf(phalcon_sync_readerwriter_object, std));
}

static inline int phalcon_readerwriter_readunlock_internal(phalcon_sync_readerwriter_object *obj)
{
	if (obj->MxMem == NULL)  return 0;

	/* Acquire the counter mutex. */
	if (!phalcon_semaphore_wait(&obj->MxPthreadRCountMutex, INFINITE))  return 0;

	if (obj->MxReadLocks)  obj->MxReadLocks--;

	/* Decrease the number of readers. */
	if (obj->MxRCount[0])  obj->MxRCount[0]--;
	else
	{
		phalcon_semaphore_release(&obj->MxPthreadRCountMutex, NULL);

		return 0;
	}

	/* Update the event state. */
	if (!obj->MxRCount[0] && !phalcon_event_fire(&obj->MxPthreadRWaitEvent))
	{
		phalcon_semaphore_release(&obj->MxPthreadRCountMutex, NULL);

		return 0;
	}

	/* Release the counter mutex. */
	phalcon_semaphore_release(&obj->MxPthreadRCountMutex, NULL);

	return 1;
}

static inline int phalcon_readerwriter_writeunlock_internal(phalcon_sync_readerwriter_object *obj)
{

	if (obj->MxMem == NULL)  return 0;

	obj->MxWriteLock = 0;

	/* Release the write lock. */
	phalcon_semaphore_release(&obj->MxPthreadWWaitMutex, NULL);

	return 1;
}

extern zend_class_entry *phalcon_sync_readerwriter_ce;

PHALCON_INIT_CLASS(Phalcon_Sync_Readerwriter);

#endif
#endif /* PHALCON_SYNC_READERWRITER_H */
