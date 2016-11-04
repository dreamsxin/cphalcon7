
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

#ifndef PHALCON_KERNEL_MEMORY_H
#define PHALCON_KERNEL_MEMORY_H

#include "php_phalcon.h"
#include "kernel/main.h"

void phalcon_initialize_memory(zend_phalcon_globals *phalcon_globals_ptr);
void phalcon_deinitialize_memory();

int phalcon_set_symbol(zend_array *symbol_table, zval *key_name, zval *value);
int phalcon_set_symbol_str(zend_array *symbol_table, char *key_name, unsigned int key_length, zval *value);
int phalcon_del_symbol(zend_array *symbol_table, zval *key_name);
int phalcon_del_symbol_str(zend_array *symbol_table, char *key_name, unsigned int key_length);

#define PHALCON_PTR_DTOR(z) zval_ptr_dtor(z);
#define PHALCON_DTOR(z) zval_dtor(z);

#define PHALCON_INIT_ZVAL_NREF(z) \
	PHALCON_ALLOC_ZVAL(z); \
	Z_SET_REFCOUNT_P(z, 0); \
	ZVAL_UNREF(z);

#define PHALCON_INIT_VAR(z) \
	PHALCON_MEMORY_ALLOC(&z);

#define PHALCON_INIT_NVAR(z) \
	do { \
		if (z) { \
			if (Z_REFCOUNTED_P(z)) { \
				if (Z_REFCOUNT_P(z) <= 1) { \
					PHALCON_PTR_DTOR(z); \
					ZVAL_NULL(z); \
				} else { \
					Z_DELREF_P(z); \
				} \
			} \
		} else { \
			PHALCON_INIT_VAR(z); \
		} \
	} while (0)

#define PHALCON_CPY_WRT(d, v)  \
	Z_TRY_ADDREF_P(v);  \
	ZVAL_COPY_VALUE(d, v);

#define PHALCON_CPY_WRT_CTOR(d, v) ZVAL_DUP(d, v);

#define PHALCON_STR(z, str) \
	do { \
		ZVAL_STRING(z, str); \
	} while (0)

#define PHALCON_STRL(z, str, len) \
	do { \
		ZVAL_STRINGL(z, str, len); \
	} while (0)

/* */
#define PHALCON_OBS_VAR(z) \
	PHALCON_MEMORY_OBSERVE(&z)

#define PHALCON_OBS_NVAR(z) \
	do { \
		if (z) { \
			if (Z_REFCOUNTED_P(z) && Z_REFCOUNT_P(z) > 1) { \
				Z_DELREF_P(z); \
			} else { \
				PHALCON_PTR_DTOR(z); \
				z = NULL; \
			} \
		} else { \
			PHALCON_MEMORY_OBSERVE(&z); \
		} \
	} while (0)

#define PHALCON_OBSERVE_OR_NULLIFY_PPZV(ppzv) \
	do { \
		zval ** restrict tmp_ = (ppzv); \
		if (tmp_ != NULL) { \
			if (*tmp_) { \
				PHALCON_PTR_DTOR(*tmp_); \
				ZVAL_NULL(*tmp_); \
			} \
			else { \
				PHALCON_MEMORY_OBSERVE((ppzv)); \
			} \
		} \
	} while (0)


#define PHALCON_SEPARATE_ARRAY(a) SEPARATE_ARRAY(a)
#define PHALCON_SEPARATE(z) SEPARATE_ZVAL(z)
#define PHALCON_SEPARATE_PARAM(z) SEPARATE_ZVAL_IF_NOT_REF(z)

#endif /* PHALCON_KERNEL_MEMORY_H */
