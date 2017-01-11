
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

#include "kernel/avltree.h"

phalcon_avltree_node *phalcon_avltree_first(phalcon_avltree *tree)
{
	return phalcon_memory_void_get(&tree->first);
}

phalcon_avltree_node *phalcon_avltree_last(phalcon_avltree *tree)
{
	return phalcon_memory_void_get(&tree->last);
}

phalcon_avltree_node *phalcon_avltree_next(phalcon_avltree_node *node)
{
	phalcon_avltree_node* r;

	if (phalcon_memory_void_get(&node->right))
		return phalcon_avltree_get_first_avl(phalcon_memory_void_get(&node->right));

	while ((r = phalcon_avltree_get_parent_avl(node)) &&
				 (phalcon_memory_void_get(&r->right) == node) ) {
		node = r;
	}

	return r;
}

phalcon_avltree_node *phalcon_avltree_prev(phalcon_avltree_node *node)
{
	phalcon_avltree_node* r;

	if (phalcon_memory_void_get(&node->left))
		return phalcon_avltree_get_last_avl(phalcon_memory_void_get(&node->left));

	while ((r = phalcon_avltree_get_parent_avl(node)) &&
				 (phalcon_memory_void_get(&r->left) == node)) {
		node = r;
	}
	return r;
}

phalcon_avltree_node *phalcon_avltree_lookup(phalcon_avltree_node *key, phalcon_avltree_node_compare cmp, phalcon_avltree *tree)
{
	phalcon_avltree_node *parent, *unbalanced;
	int is_left;

	return phalcon_avltree_do_lookup_avl(key, cmp, tree, &parent, &unbalanced, &is_left);
}

/* Insertion never needs more than 2 rotations */
phalcon_avltree_node *phalcon_avltree_insert(phalcon_avltree_node *node, phalcon_avltree_node_compare cmp, phalcon_avltree *tree)
{
	phalcon_avltree_node *key, *parent, *unbalanced;
	int is_left;

	key = phalcon_avltree_do_lookup_avl(node, cmp, tree, &parent, &unbalanced, &is_left);

	if (key)
		return key;

	phalcon_avltree_init_node(node);

	if (!parent) {

		phalcon_memory_void_set(&tree->root, node);
		phalcon_memory_void_set(&tree->first, node);
		phalcon_memory_void_set(&tree->last, node);
		tree->height++;
		return NULL;
	}

	if (is_left) {

		if (phalcon_memory_void_get(&tree->first) == parent)
			phalcon_memory_void_set(&tree->first, node);

	} else {

		if (phalcon_memory_void_get(&tree->last) == parent)
			phalcon_memory_void_set(&tree->last, node);
	}

	phalcon_avltree_set_parent_avl(parent, node);
	phalcon_avltree_set_child_avl(node, parent, is_left);

	for (;;) {

		if (phalcon_memory_void_get(&parent->left) == node)
			phalcon_avltree_dec_balance(parent);

		else
			phalcon_avltree_inc_balance(parent);

		if (parent == unbalanced)
			break;

		node = parent;
		parent = phalcon_avltree_get_parent_avl(parent);
	}

	switch (phalcon_avltree_get_balance(unbalanced)) {

	case 1:
	case -1:
		tree->height++;
		/* fall through */
	case 0:
		break;
	case 2:
	{
		phalcon_avltree_node* right = phalcon_memory_void_get(&unbalanced->right);

		if (phalcon_avltree_get_balance(right) == 1) {

			phalcon_avltree_set_balance(0, unbalanced);
			phalcon_avltree_set_balance(0, right);
		} else {

			switch (phalcon_avltree_get_balance(phalcon_memory_void_get(&right->left))) {
			case 1:
				phalcon_avltree_set_balance(-1, unbalanced);
				phalcon_avltree_set_balance( 0, right);
				break;
			case 0:
				phalcon_avltree_set_balance(0, unbalanced);
				phalcon_avltree_set_balance(0, right);
				break;
			case -1:
				phalcon_avltree_set_balance(0, unbalanced);
				phalcon_avltree_set_balance(1, right);
				break;
			}
			phalcon_avltree_set_balance(0, phalcon_memory_void_get(&right->left));
			phalcon_avltree_rotate_right_avl(right, tree);
		}
		phalcon_avltree_rotate_left_avl(unbalanced, tree);
		break;
	}

	case -2: {
		phalcon_avltree_node *left = phalcon_memory_void_get(&unbalanced->left);

		if (phalcon_avltree_get_balance(left) == -1) {
			phalcon_avltree_set_balance(0, unbalanced);
			phalcon_avltree_set_balance(0, left);
		} else {
			switch (phalcon_avltree_get_balance(phalcon_memory_void_get(&left->right))) {
			case 1:
				phalcon_avltree_set_balance( 0, unbalanced);
				phalcon_avltree_set_balance(-1, left);
				break;
			case 0:
				phalcon_avltree_set_balance(0, unbalanced);
				phalcon_avltree_set_balance(0, left);
				break;
			case -1:
				phalcon_avltree_set_balance(1, unbalanced);
				phalcon_avltree_set_balance(0, left);
				break;
			}
			phalcon_avltree_set_balance(0, phalcon_memory_void_get(&left->right));

			phalcon_avltree_rotate_left_avl(left, tree);
		}

		phalcon_avltree_rotate_right_avl(unbalanced, tree);
		break;
	}
	}
	return NULL;
}

/* Deletion might require up to log(n) rotations */
void phalcon_avltree_remove(phalcon_avltree_node *node, phalcon_avltree *tree)
{
	phalcon_avltree_node *parent = phalcon_avltree_get_parent_avl(node);
	phalcon_avltree_node *left = phalcon_memory_void_get(&node->left);
	phalcon_avltree_node *right = phalcon_memory_void_get(&node->right);
	phalcon_avltree_node *next;
	int is_left = 0;

	if (node == phalcon_memory_void_get(&tree->first))
		phalcon_memory_void_set(&tree->first, phalcon_avltree_next(node));
	if (node == phalcon_memory_void_get(&tree->last))
		phalcon_memory_void_set(&tree->last, phalcon_avltree_prev(node));

	if (!left)
		next = right;
	else if (!right)
		next = left;
	else
		next = phalcon_avltree_get_first_avl(right);

	if (parent) {
		is_left = phalcon_memory_void_get(&parent->left) == node;
		phalcon_avltree_set_child_avl(next, parent, is_left);
	} else
		phalcon_memory_void_set(&tree->root, next);

	if (left && right) {
		phalcon_avltree_set_balance(phalcon_avltree_get_balance(node), next);

		phalcon_memory_void_set(&next->left, left);
		phalcon_avltree_set_parent_avl(next, left);

		if (next != right) {
			parent = phalcon_avltree_get_parent_avl(next);
			phalcon_avltree_set_parent_avl(phalcon_avltree_get_parent_avl(node), next);

			node = phalcon_memory_void_get(&next->right);
			phalcon_memory_void_set(&parent->left, node);
			is_left = 1;

			phalcon_memory_void_set(&next->right, right);
			phalcon_avltree_set_parent_avl(next, right);
		} else {
			phalcon_avltree_set_parent_avl(parent, next);
			parent = next;
			node = phalcon_memory_void_get(&parent->right);
			is_left = 0;
		}
		assert(parent != NULL);
	} else
		node = next;

	if (node)
		phalcon_avltree_set_parent_avl(parent, node);

	/*
	 * At this point, 'parent' can only be null, if 'node' is the
	 * tree's root and has at most one child.
	 *
	 * case 1: the subtree is now balanced but its height has
	 * decreased.
	 *
	 * case 2: the subtree is mostly balanced and its height is
	 * unchanged.
	 *
	 * case 3: the subtree is unbalanced and its height may have
	 * been changed during the rebalancing process, see below.
	 *
	 * case 3.1: after a left rotation, the subtree becomes mostly
	 * balanced and its height is unchanged.
	 *
	 * case 3.2: after a left rotation, the subtree becomes
	 * balanced but its height has decreased.
	 *
	 * case 3.3: after a left and a right rotation, the subtree
	 * becomes balanced or mostly balanced but its height has
	 * decreased for all cases.
	 */
	while (parent) {
		int balance;
		node	 = parent;
		parent = phalcon_avltree_get_parent_avl(parent);

		if (is_left) {
			is_left = parent && phalcon_memory_void_get(&parent->left) == node;

			balance = phalcon_avltree_inc_balance(node);
			if (balance == 0)		/* case 1 */
				continue;
			if (balance == 1)		/* case 2 */
				return;
			right = phalcon_memory_void_get(&node->right);		/* case 3 */
			switch (phalcon_avltree_get_balance(right)) {
			case 0:				/* case 3.1 */
				phalcon_avltree_set_balance( 1, node);
				phalcon_avltree_set_balance(-1, right);
				phalcon_avltree_rotate_left_avl(node, tree);
				return;
			case 1:				/* case 3.2 */
				phalcon_avltree_set_balance(0, node);
				phalcon_avltree_set_balance(0, right);
				break;
			case -1:			/* case 3.3 */
				switch (phalcon_avltree_get_balance(phalcon_memory_void_get(&right->left))) {
				case 1:
					phalcon_avltree_set_balance(-1, node);
					phalcon_avltree_set_balance( 0, right);
					break;
				case 0:
					phalcon_avltree_set_balance(0, node);
					phalcon_avltree_set_balance(0, right);
					break;
				case -1:
					phalcon_avltree_set_balance(0, node);
					phalcon_avltree_set_balance(1, right);
					break;
				}
				phalcon_avltree_set_balance(0, phalcon_memory_void_get(&right->left));

				phalcon_avltree_rotate_right_avl(right, tree);
			}
			phalcon_avltree_rotate_left_avl(node, tree);
		} else {
			is_left = parent && phalcon_memory_void_get(&parent->left) == node;

			balance = phalcon_avltree_dec_balance(node);
			if (balance == 0)
				continue;
			if (balance == -1)
				return;
			left = phalcon_memory_void_get(&node->left);
			switch (phalcon_avltree_get_balance(left)) {
			case 0:
				phalcon_avltree_set_balance(-1, node);
				phalcon_avltree_set_balance(1, left);
				phalcon_avltree_rotate_right_avl(node, tree);
				return;
			case -1:
				phalcon_avltree_set_balance(0, node);
				phalcon_avltree_set_balance(0, left);
				break;
			case 1:
				switch (phalcon_avltree_get_balance(phalcon_memory_void_get(&left->right))) {
				case 1:
					phalcon_avltree_set_balance(0, node);
					phalcon_avltree_set_balance(-1, left);
					break;
				case 0:
					phalcon_avltree_set_balance(0, node);
					phalcon_avltree_set_balance(0, left);
					break;
				case -1:
					phalcon_avltree_set_balance(1, node);
					phalcon_avltree_set_balance(0, left);
					break;
				}
				phalcon_avltree_set_balance(0, phalcon_memory_void_get(&left->right));

				phalcon_avltree_rotate_left_avl(left, tree);
			}
			phalcon_avltree_rotate_right_avl(node, tree);
		}
	}
	tree->height--;
}

void phalcon_avltree_replace(phalcon_avltree_node *old, phalcon_avltree_node *n, phalcon_avltree *tree)
{
	phalcon_avltree_node *parent = phalcon_avltree_get_parent_avl(old);

	if (parent) {
		phalcon_avltree_set_child_avl(n, parent, phalcon_memory_void_get(&parent->left) == old);
	} else {
		phalcon_memory_void_set(&tree->root, n);
	}

	if (phalcon_memory_void_get(&old->left))
		phalcon_avltree_set_parent_avl(n, phalcon_memory_void_get(&old->left));

	if (phalcon_memory_void_get(&old->right))
		phalcon_avltree_set_parent_avl(n, phalcon_memory_void_get(&old->right));

	if (phalcon_memory_void_get(&old->left))
		phalcon_avltree_set_parent_avl(n, phalcon_memory_void_get(&old->left));
	if (phalcon_memory_void_get(&old->right))
		phalcon_avltree_set_parent_avl(n, phalcon_memory_void_get(&old->right));

	if (phalcon_memory_void_get(&tree->first) == old)
		phalcon_memory_void_set(&tree->first, n);
	if (phalcon_memory_void_get(&tree->last) == old)
		phalcon_memory_void_set(&tree->last, n);

	n->balance = old->balance;
	phalcon_memory_void_set(&n->parent, phalcon_memory_void_get(&old->parent));
	phalcon_memory_void_set(&n->left, phalcon_memory_void_get(&old->left));
	phalcon_memory_void_set(&n->right, phalcon_memory_void_get(&old->right));
}

void phalcon_avltree_init(phalcon_avltree *tree)
{
	phalcon_memory_void_set(&tree->root, NULL);
	tree->height = -1;
	phalcon_memory_void_set(&tree->first, NULL);
	phalcon_memory_void_set(&tree->last, NULL);
}
