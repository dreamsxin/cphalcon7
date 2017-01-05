
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

#ifndef PHALCON_KERNEL_AVLTREE_H
#define PHALCON_KERNEL_AVLTREE_H

#include "kernel/memory.h"

typedef struct {
    mvoid_t right;
    mvoid_t left;
    mvoid_t parent;
    signed balance:3;		/* balance factor [-2:+2] */
} phalcon_avltree_node;

typedef int (*phalcon_avltree_node_compare)(phalcon_avltree_node const* , phalcon_avltree_node const*);

typedef struct {
    mvoid_t root;
    int     height;
    mvoid_t first;
    mvoid_t last;
} phalcon_avltree;

static inline int phalcon_avltree_is_root_avl(phalcon_avltree_node *node)
{
	return NULL == phalcon_memory_void_get(&node->parent);
}

static inline void phalcon_avltree_init_node(phalcon_avltree_node *node)
{
	phalcon_memory_void_set(&node->left, NULL);
	phalcon_memory_void_set(&node->right, NULL);
	phalcon_memory_void_set(&node->parent, NULL);
	node->balance = 0;;
}

static inline signed phalcon_avltree_get_balance(phalcon_avltree_node *node)
{
	return node->balance;
}

static inline void phalcon_avltree_set_balance(int balance, phalcon_avltree_node *node)
{
	node->balance = balance;
}

static inline int phalcon_avltree_inc_balance(phalcon_avltree_node *node)
{
	return ++node->balance;
}

static inline int phalcon_avltree_dec_balance(phalcon_avltree_node *node)
{
	return --node->balance;
}

static inline phalcon_avltree_node* phalcon_avltree_get_parent_avl(const phalcon_avltree_node *node)
{
	return phalcon_memory_void_get(&node->parent);
}

static inline void phalcon_avltree_set_parent_avl(phalcon_avltree_node *parent, phalcon_avltree_node *node)
{
	phalcon_memory_void_set(&node->parent, parent);
}

/*
 * Iterators
 */
static inline phalcon_avltree_node* phalcon_avltree_get_first_avl(phalcon_avltree_node *node)
{
	while (phalcon_memory_void_get(&node->left))
		node = phalcon_memory_void_get(&node->left);
	return node;
}

static inline phalcon_avltree_node* phalcon_avltree_get_last_avl(phalcon_avltree_node *node)
{
	while (phalcon_memory_void_get(&node->right))
		node = phalcon_memory_void_get(&node->right);
	return node;
}

phalcon_avltree_node* phalcon_avltree_first(phalcon_avltree const* tree);
phalcon_avltree_node* phalcon_avltree_last(phalcon_avltree const* tree);
phalcon_avltree_node* phalcon_avltree_next(phalcon_avltree_node const* node);
phalcon_avltree_node* avltree_prev(phalcon_avltree_node const* node);

phalcon_avltree_node* phalcon_avltree_lookup(phalcon_avltree_node const* key, phalcon_avltree_node_compare cmp, phalcon_avltree const* tree);
phalcon_avltree_node* phalcon_avltree_insert(phalcon_avltree_node* node, phalcon_avltree_node_compare cmp, phalcon_avltree* tree);
void phalcon_avltree_remove(phalcon_avltree_node* node, phalcon_avltree* tree);
void phalcon_avltree_replace(phalcon_avltree_node* old, phalcon_avltree_node* node, phalcon_avltree* tree);
int phalcon_avltree_init(phalcon_avltree* tree);

#endif /* PHALCON_KERNEL_AVLTREE_H */
