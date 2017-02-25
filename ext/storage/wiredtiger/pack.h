
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
  +------------------------------------------------------------------------+
*/

#include "storage/wiredtiger/cursor.h"

#include <unistd.h>

typedef struct _phalcon_storage_wiredtiger_pack_item {
	char *data;
	size_t size;
	size_t asize;
} phalcon_storage_wiredtiger_pack_item;

void phalcon_storage_wiredtiger_pack_item_free(phalcon_storage_wiredtiger_pack_item *item);
int phalcon_storage_wiredtiger_pack_key(phalcon_storage_wiredtiger_cursor_object *intern, phalcon_storage_wiredtiger_pack_item *item, zval *key);
int phalcon_storage_wiredtiger_pack_value(phalcon_storage_wiredtiger_cursor_object *intern, phalcon_storage_wiredtiger_pack_item *item, zval *value);
int phalcon_storage_wiredtiger_unpack_key(phalcon_storage_wiredtiger_cursor_object *intern, zval *return_value, WT_ITEM *item);
int phalcon_storage_wiredtiger_unpack_value(phalcon_storage_wiredtiger_cursor_object *intern, zval *return_value, WT_ITEM *item);
