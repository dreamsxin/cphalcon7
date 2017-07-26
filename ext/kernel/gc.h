
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
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

#ifndef PHALCON_KERNEL_GC_H
#define PHALCON_KERNEL_GC_H

#include "kernel/memory.h"

typedef struct phalcon_gc_node {
	zval* value;
	struct phalcon_gc_node* next;
} phalcon_gc_node;

struct phalcon_gc_list {
	phalcon_gc_node* head; 
};

struct phalcon_gc_list* phalcon_gc_list_init();
void phalcon_gc_list_add(struct phalcon_gc_list* list, zval* value);
void phalcon_gc_list_delete(struct phalcon_gc_list* list, zval* value);
void phalcon_gc_list_display(struct phalcon_gc_list* list);
void phalcon_gc_list_reverse(struct phalcon_gc_list* list);
void phalcon_gc_list_destroy(struct phalcon_gc_list* list);

#endif /* PHALCON_KERNEL_GC_H */
