
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

#include "kernel/list.h"

phalcon_list_node* phalcon_list_lookup(phalcon_list_node const* key, phalcon_list_node_compare cmp, phalcon_list const* list)
{
	phalcon_list_node* node = phalcon_memory_void_get(&list->first);
	while (node && cmp(node, key))
		node = phalcon_memory_void_get(&node->next);
	return node;
}

phalcon_list_node* phalcon_list_front(phalcon_list const* list)
{
	return phalcon_memory_void_get(&list->first);
}

phalcon_list_node* phalcon_list_back(phalcon_list const* list)
{
	return phalcon_memory_void_get(&list->last);
}

phalcon_list_node* phalcon_list_first(phalcon_list_node const* node)
{
	while (node && phalcon_memory_void_get(&node->prev))
		node = phalcon_memory_void_get(&node->prev);
	return (phalcon_list_node*)node;
}

phalcon_list_node* phalcon_list_last(phalcon_list_node const* node)
{
	while (node && phalcon_memory_void_get(&node->next))
		node = phalcon_memory_void_get(&node->next);
	return (phalcon_list_node*)node;
}

phalcon_list_node* phalcon_list_next(phalcon_list_node const* node)
{
	return phalcon_memory_void_get(&node->next);
}

phalcon_list_node* phalcon_list_prev(phalcon_list_node const* node)
{
	return phalcon_memory_void_get(&node->prev);
}

void phalcon_list_insert_befor(phalcon_list_node* where, phalcon_list_node* node, phalcon_list* list)
{
	phalcon_list_node_init(node);

	phalcon_list_node* prev = phalcon_list_prev(where);
	if (prev) {
		phalcon_memory_void_set(&prev->next, node);
		phalcon_memory_void_set(&node->prev, prev);
	}
	phalcon_memory_void_set(&where->prev, node);
	phalcon_memory_void_set(&node->next, where);

	if (where == phalcon_memory_void_get(&list->first))
		phalcon_memory_void_set(&list->first, node);
}

void phalcon_list_insert_after(phalcon_list_node* where, phalcon_list_node* node, phalcon_list* list)
{
	phalcon_list_node_init(node);

	phalcon_list_node* next = phalcon_list_next(where);

	if (next) {
		phalcon_memory_void_set(&next->prev, node);
		phalcon_memory_void_set(&node->next, next);
	}

	phalcon_memory_void_set(&where->next, node);
	phalcon_memory_void_set(&node->prev, where);

	if (where == phalcon_memory_void_get(&list->last))
		phalcon_memory_void_set(&list->last, node);
}

void phalcon_list_push_back(phalcon_list_node* node, phalcon_list* list)
{
	if (phalcon_memory_void_get(&list->last))
		phalcon_list_insert_after(phalcon_memory_void_get(&list->last), node, list);
	else
		phalcon_list_push_front(node, list);
}

void phalcon_list_push_front(phalcon_list_node* node, phalcon_list* list)
{
	if (phalcon_memory_void_get(&list->first)) {
		phalcon_list_insert_befor(phalcon_memory_void_get(&list->first), node, list);
	} else {
		phalcon_list_node_init(node);
		phalcon_memory_void_set(&list->first, node);
		phalcon_memory_void_set(&list->last, node);
	}
}

void phalcon_list_remove(phalcon_list_node* node, phalcon_list* list)
{
	phalcon_list_node* prev = phalcon_list_prev(node);
	phalcon_list_node* next = phalcon_list_next(node);

	if (prev) phalcon_memory_void_set(&prev->next, next);
	if (next) phalcon_memory_void_set(&next->prev, prev);

	if (phalcon_memory_void_get(&list->first) == node)
		phalcon_memory_void_set(&list->first, next);
	if (phalcon_memory_void_get(&list->last) == node)
		phalcon_memory_void_set(&list->last, prev);

	phalcon_list_node_init(node);
}

void phalcon_list_replace(phalcon_list_node* old, phalcon_list_node* node, phalcon_list* list)
{
	phalcon_list_node_init(node);

	phalcon_list_node* prev = phalcon_list_prev(old);
	phalcon_list_node* next = phalcon_list_next(old);

	if (prev) {
		phalcon_memory_void_set(&prev->next, node);
		phalcon_memory_void_set(&node->prev, prev);
	}

	if (next) {
		phalcon_memory_void_set(&next->prev, node);
		phalcon_memory_void_set(&node->next, next);
	}

	if (phalcon_memory_void_get(&list->first) == old)
		phalcon_memory_void_set(&list->first, node);
	if (phalcon_memory_void_get(&list->last) == old)
		phalcon_memory_void_set(&list->last, node);

	phalcon_list_node_init(old);
}


void phalcon_list_swap(phalcon_list_node* node1, phalcon_list_node* node2, phalcon_list* list)
{
	phalcon_list_node* p1 = phalcon_list_prev(node1);
	phalcon_list_node* n1 = phalcon_list_next(node1);

	phalcon_list_node* p2 = phalcon_list_prev(node2);
	phalcon_list_node* n2 = phalcon_list_next(node2);

	if (n1 == node2) {
		if (p1) phalcon_memory_void_set(&p1->next, node2);
		phalcon_memory_void_set(&node2->prev, p1);
		phalcon_memory_void_set(&node2->next, node1);
		phalcon_memory_void_set(&node1->prev, node2);
		phalcon_memory_void_set(&node1->next, n2);
		if (n2) phalcon_memory_void_set(&n2->prev, node1);
	} else if (p1 == node2) {
		if (p2) phalcon_memory_void_set(&p2->next, node1);
		phalcon_memory_void_set(&node1->prev, p2);
		phalcon_memory_void_set(&node1->next, node2);
		phalcon_memory_void_set(&node2->prev, node1);
		phalcon_memory_void_set(&node2->next, n1);
		if (n1) phalcon_memory_void_set(&n1->prev, node2);
	} else {
		if (p1) phalcon_memory_void_set(&p1->next, node2);
		phalcon_memory_void_set(&node2->prev, p1);
		phalcon_memory_void_set(&node2->next, n1);
		if (n1) phalcon_memory_void_set(&n1->prev, node2);

		if (p2) phalcon_memory_void_set(&p2->next, node1);
		phalcon_memory_void_set(&node1->prev, p2);
		phalcon_memory_void_set(&node1->next, n2);
		if (n2) phalcon_memory_void_set(&n2->prev, node1);
	}

	if (phalcon_memory_void_get(&list->first) == node1)
		phalcon_memory_void_set(&list->first, node2);
	else if (phalcon_memory_void_get(&list->first) == node2)
		phalcon_memory_void_set(&list->first, node1);

	if (phalcon_memory_void_get(&list->last) == node1)
		phalcon_memory_void_set(&list->last, node2);
	else if (phalcon_memory_void_get(&list->last) == node2)
		phalcon_memory_void_set(&list->last, node1);
}

void phalcon_list_sort(phalcon_list* list, phalcon_list_node_compare cmp)
{
	phalcon_list_node* first = phalcon_memory_void_get(&list->first);
	phalcon_list_node *p, *q, *e, *tail;
	int insize, nmerges, psize, qsize, i;

	insize = 1;

	while (1) {

		if (!first)
			break;

		p = first;

		first = NULL;
		tail = NULL;

		nmerges = 0;	/* count number of merges we do in this pass */

		while (p) {

			nmerges++;	/* there exists a merge to be done */
			/* step `insize' places along from p */
			q = p;
			psize = 0;
			for (i = 0; i < insize; i++) {
				psize++;
				q = phalcon_memory_void_get(&q->next);
				if (!q) break;
			}

			/* if q hasn't fallen off end, we have two lists to merge */
			qsize = insize;

			/* now we have two lists; merge them */
			while (psize > 0 || (qsize > 0 && q)) {

				/* decide whether next element of merge comes from p or q */
				if (psize == 0) {
					/* p is empty; e must come from q. */
					e = q;
					q = phalcon_memory_void_get(&q->next);
					qsize--;
				} else if (qsize == 0 || !q) {
					/* q is empty; e must come from p. */
					e = p;
					p = phalcon_memory_void_get(&p->next);
					psize--;
				} else if (cmp(p,q) <= 0) {
					/* First element of p is lower (or same);
										* e must come from p. */
					e = p;
					p = phalcon_memory_void_get(&p->next);
					psize--;
				} else {
					/* First element of q is lower; e must come from q. */
					e = q;
					q = phalcon_memory_void_get(&q->next);
					qsize--;
				}


				/* add the next element to the merged list */
				if (tail) {
					phalcon_memory_void_set(&tail->next, e);
				} else {
					first = e;
				}

				/* Maintain reverse pointers in a doubly linked list. */
				phalcon_memory_void_set(&e->prev, tail);

				tail = e;
			}

			/* now p has stepped `insize' places along, and q has too */
			p = q;
		} // while (p)

		phalcon_memory_void_set(&tail->next, NULL);

		/* If we have done only one merge, we're finished. */
		if (nmerges <= 1)	 /* allow for nmerges==0, the empty list case */
			break;

		/* Otherwise repeat, merging lists twice the size */
		insize *= 2;

	} // while (1)

	phalcon_memory_void_set(&list->first, first);
	phalcon_memory_void_set(&list->last, phalcon_list_last(first));
}

int phalcon_list_init(phalcon_list* list)
{
	phalcon_memory_void_set(&list->first, NULL);
	phalcon_memory_void_set(&list->last, NULL);
	return 0;
}
