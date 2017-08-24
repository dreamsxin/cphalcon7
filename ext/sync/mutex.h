
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

#ifndef PHALCON_SYNC_MUTEX_H
#define PHALCON_SYNC_MUTEX_H

#include "php_phalcon.h"
#if PHALCON_USE_SHM_OPEN
#include "sync/common.h"

typedef struct _phalcon_sync_mutex_object {
	pthread_mutex_t MxPthreadCritSection;

	int MxNamed;
	char *MxMem;
	phalcon_semaphore_wrapper MxPthreadMutex;

	volatile pthread_t MxOwnerID;
	volatile unsigned int MxCount;

	zend_object std;
} phalcon_sync_mutex_object;

static inline phalcon_sync_mutex_object *phalcon_sync_mutex_object_from_obj(zend_object *obj) {
	return (phalcon_sync_mutex_object*)((char*)(obj) - XtOffsetOf(phalcon_sync_mutex_object, std));
}

static inline int phalcon_mutex_unlock_internal(phalcon_sync_mutex_object *obj, int all)
{
	if (pthread_mutex_lock(&obj->MxPthreadCritSection) != 0)  return 0;

	/* Make sure the mutex exists and make sure it is owned by the calling thread. */
	if (obj->MxMem == NULL || obj->MxOwnerID != phalcon_getcurrent_threadid())
	{
		pthread_mutex_unlock(&obj->MxPthreadCritSection);

		return 0;
	}

	if (all)  obj->MxCount = 1;

	obj->MxCount--;
	if (!obj->MxCount)
	{
		obj->MxOwnerID = 0;

		/* Release the mutex. */
		phalcon_semaphore_release(&obj->MxPthreadMutex, NULL);
	}

	pthread_mutex_unlock(&obj->MxPthreadCritSection);

	return 1;
}

extern zend_class_entry *phalcon_sync_mutex_ce;

PHALCON_INIT_CLASS(Phalcon_Sync_Mutex);

#endif
#endif /* PHALCON_SYNC_MUTEX_H */
