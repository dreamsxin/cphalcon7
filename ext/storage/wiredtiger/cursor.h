
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

#ifndef PHALCON_STORAGE_WIREDTIGER_CURSOR_H
#define PHALCON_STORAGE_WIREDTIGER_CURSOR_H

#include "php_phalcon.h"
# if PHALCON_USE_WIREDTIGER

#include <wiredtiger.h>

typedef struct {
    WT_ITEM key;
    WT_ITEM value;
} phalcon_storage_wiredtiger_cursor_current;

typedef struct {
    zend_object *db;
    WT_CURSOR *cursor;
    phalcon_storage_wiredtiger_cursor_current current;
	zend_bool started_iterating;
	zend_object std;
} phalcon_storage_wiredtiger_cursor_object;

static inline phalcon_storage_wiredtiger_cursor_object *phalcon_storage_wiredtiger_cursor_object_from_obj(zend_object *obj) {
	return (phalcon_storage_wiredtiger_cursor_object*)((char*)(obj) - XtOffsetOf(phalcon_storage_wiredtiger_cursor_object, std));
}

extern zend_class_entry *phalcon_storage_wiredtiger_cursor_ce;

PHALCON_INIT_CLASS(Phalcon_Storage_Wiredtiger_Cursor);

# endif
#endif /* PHALCON_STORAGE_WIREDTIGER_CURSOR_H */
