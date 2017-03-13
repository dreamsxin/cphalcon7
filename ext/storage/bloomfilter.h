
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

#ifndef PHALCON_STORAGE_BLOOMFILTER_H
#define PHALCON_STORAGE_BLOOMFILTER_H

#include "php_phalcon.h"
#include "kernel/shm.h"
#include "kernel/bloomfilter.h"

typedef struct _phalcon_storage_bloomfilter_object {
	phalcon_bloomfilter bloomfilter;
	zval filename;
	zend_object std;
} phalcon_storage_bloomfilter_object;

static inline phalcon_storage_bloomfilter_object *phalcon_storage_bloomfilter_object_from_obj(zend_object *obj) {
	return (phalcon_storage_bloomfilter_object*)((char*)(obj) - XtOffsetOf(phalcon_storage_bloomfilter_object, std));
}

extern zend_class_entry *phalcon_storage_bloomfilter_ce;

PHALCON_INIT_CLASS(Phalcon_Storage_Bloomfilter);

#endif /* PHALCON_STORAGE_BLOOMFILTER_H */
