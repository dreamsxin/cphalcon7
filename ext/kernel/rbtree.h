
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

#ifndef	PHALCON_KERNEL_RBTREE_H
#define	PHALCON_KERNEL_RBTREE_H

#include "kernel/memory.h"

#define phalcon_rbtree_container_of(ptr, type, member) ({		\
 	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
 	(type *)( (char *)__mptr - phalcon_rbtree_offsetof(type,member) );})

#define phalcon_rbtree_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define PHALCON_RBTREE_RED 0
#define PHALCON_RBTREE_BLACK 1

struct phalcon_rbtree_node
{
	unsigned long parent_color;
	struct phalcon_rbtree_node *right;
	struct phalcon_rbtree_node *left;
} __attribute__((aligned(sizeof(long))));

struct phalcon_rbtree
{
	struct phalcon_rbtree_node *node;
};

#define phalcon_rbtree_parent(r)   ((struct phalcon_rbtree_node *)((r)->parent_color & ~3))
#define phalcon_rbtree_color(r)   ((r)->parent_color & 1)
#define phalcon_rbtree_is_red(r)   (!phalcon_rbtree_color(r))
#define phalcon_rbtree_is_black(r) phalcon_rbtree_color(r)
#define phalcon_rbtree_set_red(r)  do { (r)->parent_color &= ~1; } while (0)
#define phalcon_rbtree_set_black(r)  do { (r)->parent_color |= 1; } while (0)

static inline void phalcon_rbtree_init(struct phalcon_rbtree *tree)
{
	tree->node = NULL;
}

static inline void phalcon_rbtree_set_parent(struct phalcon_rbtree_node *rb, struct phalcon_rbtree_node *p)
{
	rb->parent_color = (rb->parent_color & 3) | (unsigned long)p;
}

static inline void phalcon_rbtree_set_color(struct phalcon_rbtree_node *rb, int color)
{
	rb->parent_color = (rb->parent_color & ~1) | color;
}

#define PHALCON_RBTREE_ROOT	(struct phalcon_rbtree) { NULL, }
#define PHALCON_RBTREE_EMPTY_ROOT(root)	((root)->node == NULL)
#define PHALCON_RBTREE_EMPTY_NODE(node)	(phalcon_rbtree_parent(node) == node)
#define PHALCON_RBTREE_CLEAR_NODE(node)	(phalcon_rbtree_set_parent(node, node))

#define	phalcon_rbtree_entry(ptr, type, member) phalcon_rbtree_container_of(ptr, type, member)

static inline void phalcon_rbtree_init_node(struct phalcon_rbtree_node *rb)
{
	rb->parent_color = 0;
	rb->right = NULL;
	rb->left = NULL;
	phalcon_rbtree_set_parent(rb, rb);
}

void phalcon_rbtree_insert_color(struct phalcon_rbtree_node *, struct phalcon_rbtree *);
void phalcon_rbtree_remove(struct phalcon_rbtree_node *, struct phalcon_rbtree *);

//typedef void (*phalcon_rbtree_augment_f)(struct phalcon_rbtree_node *node, void *data);

//extern void phalcon_rbtree_augment_insert(struct phalcon_rbtree_node *node, phalcon_rbtree_augment_f func, void *data);
//extern struct phalcon_rbtree_node *phalcon_rbtree_augment_erase_begin(struct phalcon_rbtree_node *node);
//extern void phalcon_rbtree_augment_erase_end(struct phalcon_rbtree_node *node, phalcon_rbtree_augment_f func, void *data);

/* Find logical next and previous nodes in a tree */
struct phalcon_rbtree_node *phalcon_rbtree_next(const struct phalcon_rbtree_node *);
struct phalcon_rbtree_node *phalcon_rbtree_prev(const struct phalcon_rbtree_node *);
struct phalcon_rbtree_node *phalcon_rbtree_first(const struct phalcon_rbtree *);
struct phalcon_rbtree_node *phalcon_rbtree_last(const struct phalcon_rbtree *);

/* Fast replacement of a single node without remove/rebalance/add/rebalance */
void phalcon_rbtree_replace(struct phalcon_rbtree_node *old, struct phalcon_rbtree_node *new, struct phalcon_rbtree *root);

static inline void phalcon_rbtree_link_node(struct phalcon_rbtree_node * node, struct phalcon_rbtree_node * parent, struct phalcon_rbtree_node ** link)
{
	node->parent_color = (unsigned long )parent;
	node->left = node->right = NULL;

	*link = node;
}

typedef int (*phalcon_rbtree_node_compare)(struct phalcon_rbtree_node * , struct phalcon_rbtree_node *);

struct phalcon_rbtree_node* phalcon_rbtree_lookup(struct phalcon_rbtree_node* key, phalcon_rbtree_node_compare cmp, struct phalcon_rbtree* root);
struct phalcon_rbtree_node *phalcon_rbtree_insert(struct phalcon_rbtree_node* node, phalcon_rbtree_node_compare cmp, struct phalcon_rbtree* root);

#endif	/* PHALCON_KERNEL_RBTREE_H */
