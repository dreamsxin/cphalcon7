
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
    phalcon_memory_void_value right;
    phalcon_memory_void_value left;
    phalcon_memory_void_value parent;
    signed balance:3;		/* balance factor [-2:+2] */
} phalcon_avltree_node;

typedef int (*phalcon_avltree_node_compare)(phalcon_avltree_node * , phalcon_avltree_node *);

typedef struct {
    phalcon_memory_void_value root;
    int height;
    phalcon_memory_void_value first;
    phalcon_memory_void_value last;
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

static inline phalcon_avltree_node* phalcon_avltree_get_parent_avl(phalcon_avltree_node *node)
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

/*
 * The AVL tree is more rigidly balanced than Red-Black trees, leading
 * to slower insertion and removal but faster retrieval.
 */

/* node->balance = height(node->right) - height(node->left); */
static inline void phalcon_avltree_rotate_left_avl(phalcon_avltree_node *node, phalcon_avltree *tree)
{
	phalcon_avltree_node *p = node;
	phalcon_avltree_node *q = phalcon_memory_void_get(&node->right); /* can't be NULL */
	phalcon_avltree_node *parent = phalcon_avltree_get_parent_avl(p);

	if (!phalcon_avltree_is_root_avl(p)) {

		if (phalcon_memory_void_get(&parent->left) == p)
			phalcon_memory_void_set(&parent->left, q);
		else
			phalcon_memory_void_set(&parent->right, q);

	} else {
		phalcon_memory_void_set(&tree->root, q);
	}

	phalcon_avltree_set_parent_avl(parent, q);
	phalcon_avltree_set_parent_avl(q, p);

	phalcon_memory_void_set(&p->right, phalcon_memory_void_get(&q->left));

	if (phalcon_memory_void_get(&p->right))
		phalcon_avltree_set_parent_avl(p, phalcon_memory_void_get(&p->right));

	phalcon_memory_void_set(&q->left, p);
}

static inline void phalcon_avltree_rotate_right_avl(phalcon_avltree_node *node, phalcon_avltree *tree)
{
	phalcon_avltree_node *p = node;
	phalcon_avltree_node *q = phalcon_memory_void_get(&node->left) ; /* can't be NULL */
	phalcon_avltree_node *parent = phalcon_avltree_get_parent_avl(p);

	if (!phalcon_avltree_is_root_avl(p)) {

		if (phalcon_memory_void_get(&parent->left) == p)
			phalcon_memory_void_set(&parent->left, q);
		else
			phalcon_memory_void_set(&parent->right, q);

	} else {
		phalcon_memory_void_set(&tree->root, q);
	}

	phalcon_avltree_set_parent_avl(parent, q);
	phalcon_avltree_set_parent_avl(q, p);

	phalcon_memory_void_set(&p->left, phalcon_memory_void_get(&q->right));

	if (phalcon_memory_void_get(&p->left))
		phalcon_avltree_set_parent_avl(p, phalcon_memory_void_get(&p->left));

	phalcon_memory_void_set(&q->right, p);
}

static inline void phalcon_avltree_set_child_avl(phalcon_avltree_node *child, phalcon_avltree_node *node, int left)
{
	if (left) phalcon_memory_void_set(&node->left, child);
	else phalcon_memory_void_set(&node->right, child);
}

/*
 * 'pparent', 'unbalanced' and 'is_left' are only used for
 * insertions. Normally GCC will notice this and get rid of them for
 * lookups.
 */
static inline phalcon_avltree_node *phalcon_avltree_do_lookup_avl(phalcon_avltree_node *key, phalcon_avltree_node_compare cmp, phalcon_avltree *tree, phalcon_avltree_node **pparent, phalcon_avltree_node **unbalanced, int *is_left)
{
	phalcon_avltree_node *node = phalcon_memory_void_get(&tree->root);
	int res = 0;

	*pparent = NULL;
	*unbalanced = node;
	*is_left = 0;

	while (node) {
		if (phalcon_avltree_get_balance(node) != 0)
			*unbalanced = node;

		res = cmp(node, key);
		if (res == 0)
			return node;
		*pparent = node;
		if ((*is_left = res > 0))
			node = phalcon_memory_void_get(&node->left);
		else
			node = phalcon_memory_void_get(&node->right);
	}
	return NULL;
}

phalcon_avltree_node* phalcon_avltree_first(phalcon_avltree* tree);
phalcon_avltree_node* phalcon_avltree_last(phalcon_avltree* tree);
phalcon_avltree_node* phalcon_avltree_next(phalcon_avltree_node* node);
phalcon_avltree_node* phalcon_avltree_prev(phalcon_avltree_node* node);

phalcon_avltree_node* phalcon_avltree_lookup(phalcon_avltree_node* key, phalcon_avltree_node_compare cmp, phalcon_avltree* tree);
phalcon_avltree_node* phalcon_avltree_insert(phalcon_avltree_node* node, phalcon_avltree_node_compare cmp, phalcon_avltree* tree);
void phalcon_avltree_remove(phalcon_avltree_node* node, phalcon_avltree* tree);
void phalcon_avltree_replace(phalcon_avltree_node* old, phalcon_avltree_node* node, phalcon_avltree* tree);
void phalcon_avltree_init(phalcon_avltree* tree);

#endif /* PHALCON_KERNEL_AVLTREE_H */
