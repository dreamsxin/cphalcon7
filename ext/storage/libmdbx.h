
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

#ifndef PHALCON_STORAGE_LIBMDBX_H
#define PHALCON_STORAGE_LIBMDBX_H

#include "php_phalcon.h"

#ifdef PHALCON_USE_LIBMDBX

#include "mdbx.h"

typedef struct {
	MDBX_env *env;
	MDBX_dbi dbi;
	MDBX_txn *txn;
	zend_object std;
} phalcon_storage_libmdbx_object;

static inline phalcon_storage_libmdbx_object *phalcon_storage_libmdbx_object_from_obj(zend_object *obj) {
	return (phalcon_storage_libmdbx_object*)((char*)(obj) - XtOffsetOf(phalcon_storage_libmdbx_object, std));
}

extern zend_class_entry *phalcon_storage_libmdbx_ce;

PHALCON_INIT_CLASS(Phalcon_Storage_Libmdbx);

#endif
#endif /* PHALCON_STORAGE_LIBMDBX_H */
