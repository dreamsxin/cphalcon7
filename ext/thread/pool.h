
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

#ifndef PHALCON_THREAD_SIMPLE_H
#define PHALCON_THREAD_SIMPLE_H

#include "php_phalcon.h"

#include "kernel/thread/pool.h"

typedef struct _phalcon_thread_pool_object {
	phalcon_thread_pool_t *pool;
	zend_object std;
} phalcon_thread_pool_object;

static inline phalcon_thread_pool_object *phalcon_thread_pool_object_from_obj(zend_object *obj) {
	return (phalcon_thread_pool_object*)((char*)(obj) - XtOffsetOf(phalcon_thread_pool_object, std));
}

extern zend_class_entry *phalcon_thread_pool_ce;

PHALCON_INIT_CLASS(Phalcon_Thread_Pool);

#endif /* PHALCON_ERVER_SIMPLE_H */
