
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

#ifndef PHALCON_KERNEL_LIST_H
#define PHALCON_KERNEL_LIST_H

#include "kernel/memory.h"

typedef struct _phalcon_list_node {
    phalcon_memory_void_value next;
    phalcon_memory_void_value prev;
} phalcon_list_node;

typedef int (*phalcon_list_node_compare)(phalcon_list_node const* l, phalcon_list_node const* r);

typedef struct {
  phalcon_memory_void_value first;
  phalcon_memory_void_value last;
} phalcon_list;

static inline void phalcon_list_node_init(phalcon_list_node* node)
{
  phalcon_memory_void_set(&node->next, NULL);
  phalcon_memory_void_set(&node->prev, NULL);
}

phalcon_list_node* phalcon_list_lookup(phalcon_list_node const*, phalcon_list_node_compare cmp, phalcon_list const*);

phalcon_list_node* phalcon_list_front(phalcon_list const*);
phalcon_list_node* phalcon_list_back(phalcon_list const*);
phalcon_list_node* phalcon_list_first(phalcon_list_node const* node);
phalcon_list_node* phalcon_list_last(phalcon_list_node const* node);
phalcon_list_node* phalcon_list_next(phalcon_list_node const* node);
phalcon_list_node* phalcon_list_prev(phalcon_list_node const* node);

void phalcon_list_insert_befor(phalcon_list_node* where, phalcon_list_node* node, phalcon_list*);
void phalcon_list_insert_after(phalcon_list_node* where, phalcon_list_node* node, phalcon_list*);
void phalcon_list_push_back(phalcon_list_node* node, phalcon_list*);
void phalcon_list_push_front(phalcon_list_node* node, phalcon_list*);

void phalcon_list_remove(phalcon_list_node* node, phalcon_list*);
void phalcon_list_replace(phalcon_list_node* old, phalcon_list_node* node, phalcon_list*);

void phalcon_list_swap(phalcon_list_node* node1, phalcon_list_node* node2, phalcon_list*);
void phalcon_list_sort(phalcon_list*, phalcon_list_node_compare cmp);
int phalcon_list_init(phalcon_list*);

#endif /* PHALCON_KERNEL_LIST_H */
