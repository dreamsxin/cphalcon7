
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

#ifndef PHALCON_SYNC_SHAREDMEMORY_H
#define PHALCON_SYNC_SHAREDMEMORY_H

#include "php_phalcon.h"
#if PHALCON_USE_SHM_OPEN
#include "sync/common.h"

/* Named shared memory */
typedef struct _phalcon_sync_sharedmemory_object {
	int MxFirst;
	size_t MxSize;
	char *MxMem;
	char *MxMemInternal;

	zend_object std;
} phalcon_sync_sharedmemory_object;

static inline phalcon_sync_sharedmemory_object *phalcon_sync_sharedmemory_object_from_obj(zend_object *obj) {
	return (phalcon_sync_sharedmemory_object*)((char*)(obj) - XtOffsetOf(phalcon_sync_sharedmemory_object, std));
}

extern zend_class_entry *phalcon_sync_sharedmemory_ce;

PHALCON_INIT_CLASS(Phalcon_Sync_Sharedmemory);

#endif
#endif /* PHALCON_SYNC_SHAREDMEMORY_H */
