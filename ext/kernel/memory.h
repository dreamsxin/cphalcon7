
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

#ifndef PHALCON_KERNEL_MEMORY_H
#define PHALCON_KERNEL_MEMORY_H

#include "php_phalcon.h"
#include "kernel/main.h"

#include <stdint.h>

typedef struct _phalcon_memory_void_value {
    ptrdiff_t offset;
} phalcon_memory_void_value;

inline static void phalcon_memory_void_set(phalcon_memory_void_value* ptr, void* addr) {
	if (addr == ptr) {
		ptr->offset = INTPTR_MIN;
	} else if (addr) {
		ptr->offset = (char const*)addr - (char const*)ptr;
	} else {
		ptr->offset = 0;
	}
}

inline static void* phalcon_memory_void_get(phalcon_memory_void_value const* ptr) {
	if (INTPTR_MIN == ptr->offset) {
		return (void*)ptr;
	} else if (ptr->offset) {
		return (char*)ptr + ptr->offset;
	}
	return NULL;
}

#define phalcon_memory_container_of(nodepointer, type, member) \
  ((type *)((char *)(nodepointer) - offsetof(type, member)))

void phalcon_initialize_memory(zend_phalcon_globals *phalcon_globals_ptr);
void phalcon_deinitialize_memory();

int phalcon_set_symbol(zend_array *symbol_table, zval *key_name, zval *value);
int phalcon_set_symbol_str(zend_array *symbol_table, char *key_name, unsigned int key_length, zval *value);
int phalcon_del_symbol(zend_array *symbol_table, zval *key_name);
int phalcon_del_symbol_str(zend_array *symbol_table, char *key_name, unsigned int key_length);

#define PHALCON_PTR_DTOR(z) zval_ptr_dtor(z);
#define PHALCON_DTOR(z) zval_dtor(z);

#define PHALCON_INIT_VAR(z) ZVAL_UNDEF(z);

#define PHALCON_INIT_NVAR(z) \
	do { \
		if (Z_TYPE_P(z) > IS_NULL) { \
			PHALCON_PTR_DTOR(z); \
		} \
		PHALCON_INIT_VAR(z); \
	} while (0)

#define PHALCON_CPY_WRT_CTOR(d, v) ZVAL_DUP(d, v);
#define PHALCON_SEPARATE(z) SEPARATE_ZVAL(z);
#define PHALCON_SEPARATE_PARAM(z) SEPARATE_ZVAL_IF_NOT_REF(z)

#endif /* PHALCON_KERNEL_MEMORY_H */
