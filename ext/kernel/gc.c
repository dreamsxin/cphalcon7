
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

#include "kernel/gc.h"

static inline phalcon_gc_node* phalcon_gc_list_createnode(zval* value) {
	phalcon_gc_node* node = malloc(sizeof(phalcon_gc_node));
	node->value = value;
	node->next = NULL;
	return node;
}

struct phalcon_gc_list* phalcon_gc_list_init() {
	struct phalcon_gc_list* list = malloc(sizeof(struct phalcon_gc_list));
	list->head = NULL;
	return list;
}

void phalcon_gc_list_display(struct phalcon_gc_list* list) {
	phalcon_gc_node* current = list->head;
	if(list->head == NULL) {
		return;
	}
	while(current->next != NULL){
		zend_print_zval_r(current->value, 0);
		current = current->next;
	}
	zend_print_zval_r(current->value, 0); 
}

void phalcon_gc_list_add(struct phalcon_gc_list* list, zval* value){
	phalcon_gc_node* current = NULL;
	if(list->head == NULL){
		list->head = phalcon_gc_list_createnode(value);
	} else {
		current = list->head; 
		while (current->next!=NULL){
			current = current->next;
		}
		current->next = phalcon_gc_list_createnode(value);
	}
}

void phalcon_gc_list_delete(struct phalcon_gc_list* list, zval* value){
	phalcon_gc_node* current = list->head;
	phalcon_gc_node* previous = current;
	while(current != NULL){
		if(current->value == value){
			previous->next = current->next;
			if(current == list->head) {
				list->head = current->next;
			}
			free(current);
			return;
		}
		previous = current;
		current = current->next;
	}                                 
}                                   

void phalcon_gc_list_reverse(struct phalcon_gc_list* list){
	phalcon_gc_node* reversed = NULL;
	phalcon_gc_node* current = list->head;
	phalcon_gc_node* temp = NULL;
	while(current != NULL){
		temp = current;
		current = current->next;
		temp->next = reversed;
		reversed = temp;
	}
	list->head = reversed;
}

void phalcon_gc_list_destroy(struct phalcon_gc_list* list){
	phalcon_gc_node* current = list->head;
	phalcon_gc_node* next = current;
	while(current != NULL){
		zval_ptr_dtor(current->value);
		next = current->next;
		free(current);
		current = next;
	}
	free(list);
}
