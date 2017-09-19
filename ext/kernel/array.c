
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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "kernel/array.h"

#include <ext/standard/php_array.h>

#include "kernel/main.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/hash.h"

int ZEND_FASTCALL phalcon_array_isset_fetch(zval *fetched, const zval *arr, const zval *index, int flags)
{
	return phalcon_array_fetch(fetched, arr, index, flags) == SUCCESS ? 1 : 0;
}

int ZEND_FASTCALL phalcon_array_isset_fetch_long(zval *fetched, const zval *arr, ulong index, int flags)
{
	zval z_index = {};
	int status;
	ZVAL_LONG(&z_index, index);

	status = phalcon_array_isset_fetch(fetched, arr, &z_index, flags);
	return status;
}

int ZEND_FASTCALL phalcon_array_isset_fetch_str(zval *fetched, const zval *arr, const char *index, uint index_length, int flags)
{
	zval z_index = {};
	int status;
	ZVAL_STRINGL(&z_index, index, index_length);

	status = phalcon_array_isset_fetch(fetched, arr, &z_index, flags);
	zval_ptr_dtor(&z_index);
	return status;
}

int ZEND_FASTCALL phalcon_array_isset_fetch_string(zval *fetched, const zval *arr, zend_string *index, int flags)
{
	zval z_index = {};
	int status;
	ZVAL_STR(&z_index, index);

	status = phalcon_array_isset_fetch(fetched, arr, &z_index, flags);
	return status;
}

int ZEND_FASTCALL phalcon_array_isset(const zval *arr, const zval *index)
{
	HashTable *h;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return 0;
	}

	h = Z_ARRVAL_P(arr);
	switch (Z_TYPE_P(index)) {
		case IS_NULL:
			return zend_hash_str_exists(h, SL(""));

		case IS_DOUBLE:
			return zend_hash_index_exists(h, (ulong)Z_DVAL_P(index));

		case IS_TRUE:
		case IS_FALSE:
			return zend_hash_index_exists(h, Z_TYPE_P(index) == IS_TRUE ? 1 : 0);

		case IS_LONG:
		case IS_RESOURCE:
			return zend_hash_index_exists(h, Z_LVAL_P(index));

		case IS_STRING:
			return zend_symtable_exists(h, Z_STR_P(index));

		default:
			zend_error(E_WARNING, "Illegal offset type");
			return 0;
	}
}

int ZEND_FASTCALL phalcon_array_isset_str(const zval *arr, const char *index, uint index_length) {

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		return zend_hash_str_exists(Z_ARRVAL_P(arr), index, index_length);
	}

	return 0;
}

int ZEND_FASTCALL phalcon_array_isset_string(const zval *arr, zend_string *index) {

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		return zend_hash_exists(Z_ARRVAL_P(arr), index);
	}

	return 0;
}

int ZEND_FASTCALL phalcon_array_isset_long(const zval *arr, ulong index) {

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		return zend_hash_index_exists(Z_ARRVAL_P(arr), index);
	}

	return 0;
}

int ZEND_FASTCALL phalcon_array_unset(zval *arr, const zval *index, int flags) {

	HashTable *ht;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	ht = Z_ARRVAL_P(arr);

	switch (Z_TYPE_P(index)) {
		case IS_NULL:
			return (zend_hash_str_del(ht, "", 1) == SUCCESS);

		case IS_DOUBLE:
			return (zend_hash_index_del(ht, (ulong)Z_DVAL_P(index)) == SUCCESS);

		case IS_TRUE:
			return (zend_hash_index_del(ht, 1) == SUCCESS);

		case IS_FALSE:
			return (zend_hash_index_del(ht, 0) == SUCCESS);

		case IS_LONG:
		case IS_RESOURCE:
			return (zend_hash_index_del(ht, Z_LVAL_P(index)) == SUCCESS);

		case IS_STRING:
			return (zend_symtable_del(ht, Z_STR_P(index)) == SUCCESS);

		default:
			zend_error(E_WARNING, "Illegal offset type");
			return 0;
	}
}

int ZEND_FASTCALL phalcon_array_unset_str(zval *arr, const char *index, uint index_length, int flags) {

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	return zend_hash_str_del(Z_ARRVAL_P(arr), index, index_length);
}

int ZEND_FASTCALL phalcon_array_unset_long(zval *arr, ulong index, int flags) {

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	return zend_hash_index_del(Z_ARRVAL_P(arr), index);
}

int phalcon_array_append(zval *arr, zval *value, int flags) {

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array");
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		Z_TRY_ADDREF_P(value);
	}

	return add_next_index_zval(arr, value);
}

int phalcon_array_update(zval *arr, const zval *index, zval *value, int flags)
{
	zval new_value = {};
	HashTable *ht;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array (2)");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		ZVAL_DUP(&new_value, value);
		value = &new_value;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		Z_TRY_ADDREF_P(value);
	}

	ht = Z_ARRVAL_P(arr);

	return phalcon_array_update_hash(ht, index, value, flags);
}

int phalcon_array_update_hash(HashTable *ht, const zval *index, zval *value, int flags)
{
	int status;

	switch (Z_TYPE_P(index)) {
		case IS_NULL:
			status = zend_symtable_update(ht, ZSTR_EMPTY_ALLOC(), value) ? SUCCESS : FAILURE;
			break;

		case IS_DOUBLE:
			status = zend_hash_index_update(ht, zend_dval_to_lval(Z_DVAL_P(index)), value) ? SUCCESS : FAILURE;
			break;

		case IS_TRUE:
			status = zend_hash_index_update(ht, 1, value) ? SUCCESS : FAILURE;
			break;

		case IS_FALSE:
			status = zend_hash_index_update(ht, 0, value) ? SUCCESS : FAILURE;
			break;

		case IS_LONG:
			status = zend_hash_index_update(ht, Z_LVAL_P(index), value) ? SUCCESS : FAILURE;
			break;

		case IS_RESOURCE:
			zend_error(E_NOTICE, "Resource ID#%d used as offset, casting to integer (%d)", Z_RES_HANDLE_P(index), Z_RES_HANDLE_P(index));
			status = zend_hash_index_update(ht, Z_RES_HANDLE_P(index), value) ? SUCCESS : FAILURE;
			break;

		case IS_STRING:
			status = zend_symtable_update(ht, Z_STR_P(index), value) ? SUCCESS : FAILURE;
			break;

		default:
			zend_error(E_WARNING, "Illegal offset type");
			status = FAILURE;
			break;
	}

	return status;
	// return array_set_zval_key(ht, (zval *)index, value);
}

int phalcon_array_update_str(zval *arr, const char *index, uint index_length, zval *value, int flags)
{
	zval new_value = {}, key = {};
	int status;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array (3)");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		ZVAL_DUP(&new_value, value);
		value = &new_value;
		Z_TRY_DELREF(new_value);
	} else if ((flags & PH_COPY) == PH_COPY) {
		Z_TRY_ADDREF_P(value);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	ZVAL_STRINGL(&key, index, index_length);
	status = phalcon_array_update_hash(Z_ARRVAL_P(arr), &key, value, flags);
	zval_ptr_dtor(&key);
	return status;
}

int phalcon_array_update_string(zval *arr, zend_string *index, zval *value, int flags)
{
	zval new_value = {};

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array (3)");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		ZVAL_DUP(&new_value, value);
		value = &new_value;
	} else if ((flags & PH_COPY) == PH_COPY) {
		Z_TRY_ADDREF_P(value);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	return zend_hash_update(Z_ARRVAL_P(arr), index, value) ? SUCCESS : FAILURE;
}

int phalcon_array_update_long(zval *arr, ulong index, zval *value, int flags)
{
	zval new_value = {};

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		ZVAL_DUP(&new_value, value);
		value = &new_value;
	} else if ((flags & PH_COPY) == PH_COPY) {
		Z_TRY_ADDREF_P(value);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	return zend_hash_index_update(Z_ARRVAL_P(arr), index, value) ? SUCCESS : FAILURE;
}

int phalcon_array_fetch(zval *return_value, const zval *arr, const zval *index, int flags){

	zval *zv;
	HashTable *ht;
	int result = SUCCESS, found = 0;
	ulong uidx = 0;
	char *sidx = NULL;
/*
	if (!phalcon_array_isset(arr, index)) {
		ZVAL_NULL(return_value);
		return FAILURE;
	}
*/
	if (Z_TYPE_P(arr) == IS_ARRAY) {
		ht = Z_ARRVAL_P(arr);
		switch (Z_TYPE_P(index)) {
			case IS_NULL:
				found = (zv = zend_hash_str_find(ht, SL(""))) != NULL;
				sidx   = "";
				break;

			case IS_DOUBLE:
				uidx   = (ulong)Z_DVAL_P(index);
				found  = (zv = zend_hash_index_find(ht, uidx)) != NULL;
				break;

			case IS_LONG:
			case IS_RESOURCE:
				uidx   = Z_LVAL_P(index);
				found  = (zv = zend_hash_index_find(ht, uidx)) != NULL;
				break;

			case IS_FALSE:
				uidx = 0;
				found  = (zv = zend_hash_index_find(ht, uidx)) != NULL;
				break;

			case IS_TRUE:
				uidx = 1;
				found  = (zv = zend_hash_index_find(ht, uidx)) != NULL;
				break;

			case IS_STRING:
				sidx   = Z_STRLEN_P(index) ? Z_STRVAL_P(index) : "";
				found  = (zv = zend_symtable_str_find(ht, Z_STRVAL_P(index), Z_STRLEN_P(index))) != NULL;
				break;

			default:
				if ((flags & PH_NOISY) == PH_NOISY) {
					zend_error(E_WARNING, "Illegal offset type");
				}
				result = FAILURE;
				break;
		}

		if (result != FAILURE && found == 1) {
			/*
			if (EXPECTED(Z_TYPE_P(zv) == IS_REFERENCE)) {
				zv = Z_REFVAL_P(zv);
			}
			if (Z_TYPE_P(zv) == IS_INDIRECT) {
				zv = Z_INDIRECT_P(zv);
			}
			*/
			if ((flags & PH_SEPARATE) == PH_SEPARATE) {
				SEPARATE_ZVAL_IF_NOT_REF(zv);
				ZVAL_COPY_VALUE(return_value, zv);
			} else if ((flags & PH_CTOR) == PH_CTOR) {
				ZVAL_DUP(return_value, zv);
			} else if ((flags & PH_READONLY) == PH_READONLY) {
				ZVAL_COPY_VALUE(return_value, zv);
			} else {
				ZVAL_COPY(return_value, zv);
			}
			return SUCCESS;
		}

		if ((flags & PH_NOISY) == PH_NOISY) {
			if (sidx == NULL) {
				zend_error(E_NOTICE, "Undefined index: %ld", uidx);
			} else {
				zend_error(E_NOTICE, "Undefined index: %s", sidx);
			}
		}
	}

	ZVAL_NULL(return_value);
	return FAILURE;
}

int phalcon_array_pop(zval *return_value, const zval *stack)
{
	zval *val;		/* Value to be popped */
	uint32_t idx;
	Bucket *p;

	if (zend_hash_num_elements(Z_ARRVAL_P(stack)) == 0) {
		ZVAL_NULL(return_value);
		return FAILURE;
	}

	/* Get the last value and copy it into the return value */
	idx = Z_ARRVAL_P(stack)->nNumUsed;
	while (1) {
		if (idx == 0) {
			ZVAL_NULL(return_value);
			return FAILURE;
		}
		idx--;
		p = Z_ARRVAL_P(stack)->arData + idx;
		val = &p->val;
		if (Z_TYPE_P(val) == IS_INDIRECT) {
			val = Z_INDIRECT_P(val);
		}
		if (Z_TYPE_P(val) != IS_UNDEF) {
			break;
		}
	}
	ZVAL_DEREF(val);
	ZVAL_COPY(return_value, val);

	if (!p->key && Z_ARRVAL_P(stack)->nNextFreeElement > 0 && p->h >= (zend_ulong)(Z_ARRVAL_P(stack)->nNextFreeElement - 1)) {
		Z_ARRVAL_P(stack)->nNextFreeElement = Z_ARRVAL_P(stack)->nNextFreeElement - 1;
	}

	/* Delete the last value */
	if (p->key) {
		zend_hash_del(Z_ARRVAL_P(stack), p->key);
	} else {
		zend_hash_index_del(Z_ARRVAL_P(stack), p->h);
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(stack));
	return SUCCESS;
}

int phalcon_array_last(zval *return_value, const zval *stack, int flags)
{
	zval *val;		/* Value to be popped */
	uint32_t idx;
	Bucket *p;

	if (zend_hash_num_elements(Z_ARRVAL_P(stack)) == 0) {
		ZVAL_NULL(return_value);
		return FAILURE;
	}

	/* Get the last value and copy it into the return value */
	idx = Z_ARRVAL_P(stack)->nNumUsed;
	while (1) {
		if (idx == 0) {
			ZVAL_NULL(return_value);
			return FAILURE;
		}
		idx--;
		p = Z_ARRVAL_P(stack)->arData + idx;
		val = &p->val;
		if (Z_TYPE_P(val) == IS_INDIRECT) {
			val = Z_INDIRECT_P(val);
		}
		if (Z_TYPE_P(val) != IS_UNDEF) {
			break;
		}
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(val);
		ZVAL_COPY_VALUE(return_value, val);
	} else if ((flags & PH_CTOR) == PH_CTOR) {
		ZVAL_DUP(return_value, val);
	} else if ((flags & PH_READONLY) == PH_READONLY) {
		ZVAL_COPY_VALUE(return_value, val);
	} else {
		ZVAL_COPY(return_value, val);
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(stack));
	return SUCCESS;
}

int phalcon_array_fetch_str(zval *return_value, const zval *arr, const char *index, uint index_length, int flags){

	zval *zv;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		if ((zv = zend_hash_str_find(Z_ARRVAL_P(arr), index, index_length)) != NULL) {
			if ((flags & PH_SEPARATE) == PH_SEPARATE) {
				SEPARATE_ZVAL_IF_NOT_REF(zv);
				ZVAL_COPY_VALUE(return_value, zv);
			} else if ((flags & PH_READONLY) == PH_READONLY) {
				ZVAL_COPY_VALUE(return_value, zv);
			} else {
				ZVAL_COPY(return_value, zv);
			}
			return SUCCESS;
		}

		if ((flags & PH_NOISY) == PH_NOISY) {
			zend_error(E_NOTICE, "Undefined index: %s", index);
		}
	} else {
		if ((flags & PH_NOISY) == PH_NOISY) {
			zend_error(E_NOTICE, "Cannot use a scalar value as an array");
		}
	}

	ZVAL_NULL(return_value);

	return FAILURE;
}

int phalcon_array_fetch_string(zval *return_value, const zval *arr, zend_string *index, int flags){

	zval *zv;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		if ((zv = zend_hash_find(Z_ARRVAL_P(arr), index)) != NULL) {
			if ((flags & PH_SEPARATE) == PH_SEPARATE) {
				SEPARATE_ZVAL_IF_NOT_REF(zv);
				ZVAL_COPY_VALUE(return_value, zv);
			} else if ((flags & PH_READONLY) == PH_READONLY) {
				ZVAL_COPY_VALUE(return_value, zv);
			} else {
				ZVAL_COPY(return_value, zv);
			}
			return SUCCESS;
		}

		if ((flags & PH_NOISY) == PH_NOISY) {
			zend_error(E_NOTICE, "Undefined index: %s", index->val);
		}
	} else {
		if ((flags & PH_NOISY) == PH_NOISY) {
			zend_error(E_NOTICE, "Cannot use a scalar value as an array");
		}
	}

	ZVAL_NULL(return_value);

	return FAILURE;
}

int phalcon_array_fetch_long(zval *return_value, const zval *arr, ulong index, int flags)
{
	zval *zv;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		if ((zv = zend_hash_index_find(Z_ARRVAL_P(arr), index)) != NULL) {
			if ((flags & PH_SEPARATE) == PH_SEPARATE) {
				SEPARATE_ZVAL_IF_NOT_REF(zv);
				ZVAL_COPY_VALUE(return_value, zv);
			} else if ((flags & PH_READONLY) == PH_READONLY) {
				ZVAL_COPY_VALUE(return_value, zv);
			} else {
				ZVAL_COPY(return_value, zv);
			}
			return SUCCESS;
		}

		if (flags == PH_NOISY) {
			zend_error(E_NOTICE, "Undefined index: %lu", index);
		}
	} else {
		if (flags == PH_NOISY) {
			zend_error(E_NOTICE, "Cannot use a scalar value as an array");
		}
	}

	ZVAL_NULL(return_value);

	return FAILURE;
}

void phalcon_array_append_multi_2(zval *arr, zval *index, zval *value, int flags)
{
	zval tmp = {};
	int separated = 0;

	if (Z_TYPE_P(arr) == IS_ARRAY) {
		phalcon_array_fetch(&tmp, arr, index, PH_READONLY);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNTED(tmp)) {
			if (Z_REFCOUNT(tmp) > 1) {
				if (!Z_ISREF(tmp)) {
					zval new_zv;
					ZVAL_DUP(&new_zv, &tmp);
					ZVAL_COPY_VALUE(&tmp, &new_zv);
					Z_TRY_DELREF(new_zv);
					separated = 1;
				}
			}
		} else {
			zval new_zv;
			ZVAL_DUP(&new_zv, &tmp);
			ZVAL_COPY_VALUE(&tmp, &new_zv);
			Z_TRY_DELREF(new_zv);
			separated = 1;
		}

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp) != IS_ARRAY) {
			if (separated) {
				convert_to_array(&tmp);
			} else {
				array_init(&tmp);
				separated = 1;
			}
			Z_DELREF(tmp);
		}

		phalcon_array_append(&tmp, value, flags);
		if (separated) {
			phalcon_array_update(arr, index, &tmp, PH_COPY);
		}
	}
}

void phalcon_array_update_multi_2(zval *arr, const zval *index1, const zval *index2, zval *value, int flags)
{
	zval tmp = {};
	int separated = 0;

	if (Z_TYPE_P(arr) == IS_ARRAY) {
		phalcon_array_fetch(&tmp, arr, index1, PH_READONLY);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNTED(tmp)) {
			if (Z_REFCOUNT(tmp) > 1) {
				if (!Z_ISREF(tmp)) {
					zval new_zv;
					ZVAL_DUP(&new_zv, &tmp);
					ZVAL_COPY_VALUE(&tmp, &new_zv);
					Z_TRY_DELREF(new_zv);
					separated = 1;
				}
			}
		} else {
			zval new_zv;
			ZVAL_DUP(&new_zv, &tmp);
			ZVAL_COPY_VALUE(&tmp, &new_zv);
			Z_TRY_DELREF(new_zv);
			separated = 1;
		}

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp) != IS_ARRAY) {
			if (separated) {
				convert_to_array(&tmp);
			} else {
				array_init(&tmp);
				separated = 1;
			}
			Z_DELREF(tmp);
		}

		phalcon_array_update(&tmp, index2, value, flags);
		if (separated) {
			phalcon_array_update(arr, index1, &tmp, PH_COPY);
		}
	}

}

void phalcon_array_update_str_multi_2(zval *arr, const zval *index1, const char *index2, uint index2_length, zval *value, int flags)
{
	zval z_index2 = {};
	ZVAL_STRINGL(&z_index2, index2, index2_length);

	phalcon_array_update_multi_2(arr, index1, &z_index2, value, flags);
	zval_ptr_dtor(&z_index2);
}

void phalcon_array_update_long_long_multi_2(zval *arr, ulong index1, ulong index2, zval *value, int flags)
{
	zval z_index1 = {}, z_index2 = {};
	ZVAL_LONG(&z_index1, index1);
	ZVAL_LONG(&z_index2, index2);

	phalcon_array_update_multi_2(arr, &z_index1, &z_index2, value, flags);
}

void phalcon_array_update_long_str_multi_2(zval *arr, ulong index1, const char *index2, uint index2_length, zval *value, int flags)
{
	zval z_index1 = {}, z_index2 = {};
	ZVAL_LONG(&z_index1, index1);
	ZVAL_STRINGL(&z_index2, index2, index2_length);

	phalcon_array_update_multi_2(arr, &z_index1, &z_index2, value, flags);
	zval_ptr_dtor(&z_index2);
}

void phalcon_array_update_zval_str_append_multi_3(zval *arr, const zval *index1, const char *index2, uint index2_length, zval *value, int flags)
{
	zval tmp1 = {}, tmp2 = {};
	int separated1 = 0, separated2 = 0;

	if (Z_TYPE_P(arr) == IS_ARRAY) {
		phalcon_array_fetch(&tmp1, arr, index1, PH_READONLY);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNTED(tmp1)) {
			if (Z_REFCOUNT(tmp1) > 1) {
				if (!Z_ISREF(tmp1)) {
					zval new_zv;
					ZVAL_DUP(&new_zv, &tmp1);
					ZVAL_COPY_VALUE(&tmp1, &new_zv);
					Z_TRY_DELREF(new_zv);
					separated1 = 1;
				}
			}
		} else {
			zval new_zv;
			ZVAL_DUP(&new_zv, &tmp1);
			ZVAL_COPY_VALUE(&tmp1, &new_zv);
			Z_TRY_DELREF(new_zv);
			separated1 = 1;
		}

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp1) != IS_ARRAY) {
			if (separated1) {
				convert_to_array(&tmp1);
			} else {
				array_init(&tmp1);
				separated1 = 1;
			}
			Z_DELREF(tmp1);
		}

		phalcon_array_fetch_str(&tmp2, &tmp1, index2, index2_length, PH_READONLY);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNTED(tmp2)) {
			if (Z_REFCOUNT(tmp2) > 1) {
				if (!Z_ISREF(tmp2)) {
					zval new_zv;
					ZVAL_DUP(&new_zv, &tmp2);
					ZVAL_COPY_VALUE(&tmp2, &new_zv);
					Z_TRY_DELREF(new_zv);
					separated2 = 1;
				}
			}
		} else {
			zval new_zv;
			ZVAL_DUP(&new_zv, &tmp2);
			ZVAL_COPY_VALUE(&tmp2, &new_zv);
			Z_TRY_DELREF(new_zv);
			separated2 = 1;
		}

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp2) != IS_ARRAY) {
			if (separated2) {
				convert_to_array(&tmp2);
			} else {
				array_init(&tmp2);
				separated2 = 1;
			}
			Z_DELREF(tmp2);
		}

		phalcon_array_append(&tmp2, value, flags);
		if (separated2) {
			phalcon_array_update_str(&tmp1, index2, index2_length, &tmp2, PH_COPY);
		}
		if (separated1) {
			phalcon_array_update(arr, index1, &tmp1, PH_COPY);
		}
	}
}

void phalcon_array_update_zval_zval_zval_multi_3(zval *arr, const zval *index1, const zval *index2, const zval *index3, zval *value, int flags)
{
	zval tmp1 = {}, tmp2 = {};
	int separated1 = 0, separated2 = 0;

	if (Z_TYPE_P(arr) == IS_ARRAY) {
		phalcon_array_fetch(&tmp1, arr, index1, PH_READONLY);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNTED(tmp1)) {
			if (Z_REFCOUNT(tmp1) > 1) {
				if (!Z_ISREF(tmp1)) {
					zval new_zv;
					ZVAL_DUP(&new_zv, &tmp1);
					ZVAL_COPY_VALUE(&tmp1, &new_zv);
					Z_TRY_DELREF(new_zv);
					separated1 = 1;
				}
			}
		} else {
			zval new_zv;
			ZVAL_DUP(&new_zv, &tmp1);
			ZVAL_COPY_VALUE(&tmp1, &new_zv);
			Z_TRY_DELREF(new_zv);
			separated1 = 1;
		}

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp1) != IS_ARRAY) {
			if (separated1) {
				convert_to_array(&tmp1);
			} else {
				array_init(&tmp1);
				separated1 = 1;
			}
			Z_DELREF(tmp1);
		}

		phalcon_array_fetch(&tmp2, &tmp1, index2, PH_READONLY);

		/** Separation only when refcount > 1 */
		if (Z_REFCOUNTED(tmp2)) {
			if (Z_REFCOUNT(tmp2) > 1) {
				if (!Z_ISREF(tmp2)) {
					zval new_zv;
					ZVAL_DUP(&new_zv, &tmp2);
					ZVAL_COPY_VALUE(&tmp2, &new_zv);
					Z_TRY_DELREF(new_zv);
					separated2 = 1;
				}
			}
		} else {
			zval new_zv;
			ZVAL_DUP(&new_zv, &tmp2);
			ZVAL_COPY_VALUE(&tmp2, &new_zv);
			Z_TRY_DELREF(new_zv);
			separated2 = 1;
		}

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp2) != IS_ARRAY) {
			if (separated2) {
				convert_to_array(&tmp2);
			} else {
				array_init(&tmp2);
				separated2 = 1;
			}
			Z_DELREF(tmp2);
		}

		phalcon_array_update(&tmp2, index3, value, flags);
		if (separated2) {
			phalcon_array_update(&tmp1, index2, &tmp2, PH_COPY);
		}
		if (separated1) {
			phalcon_array_update(arr, index1, &tmp1, PH_COPY);
		}
	}
}

void phalcon_array_update_zval_zval_str_multi_3(zval *arr, const zval *index1, const zval *index2, const char *index3, uint index3_length, zval *value, int flags)
{
	zval z_index3 = {};
	ZVAL_STRINGL(&z_index3, index3, index3_length);

	phalcon_array_update_zval_zval_zval_multi_3(arr, index1, index2, &z_index3, value, flags);
	zval_ptr_dtor(&z_index3);
}

void phalcon_array_update_zval_str_str_multi_3(zval *arr, const zval *index1, const char *index2, uint index2_length, const char *index3, uint index3_length, zval *value, int flags)
{
	zval z_index2 = {}, z_index3 = {};
	ZVAL_STRINGL(&z_index2, index2, index2_length);
	ZVAL_STRINGL(&z_index3, index3, index3_length);

	phalcon_array_update_zval_zval_zval_multi_3(arr, index1, &z_index2, &z_index3, value, flags);
	zval_ptr_dtor(&z_index2);
	zval_ptr_dtor(&z_index3);
}

void phalcon_merge_append(zval *left, zval *values){
	zval *tmp;

	if (Z_TYPE_P(left) != IS_ARRAY) {
		zend_error(E_NOTICE, "The first parameter of phalcon_merge_append must be an array");
		return;
	}

	if (Z_TYPE_P(values) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(values), tmp) {
			Z_TRY_ADDREF_P(tmp);
			add_next_index_zval(left, tmp);
		} ZEND_HASH_FOREACH_END();

	} else {
		Z_TRY_ADDREF_P(values);
		add_next_index_zval(left, values);
	}
}

void phalcon_array_replace(zval *left, zval *values){

	zval *value;
	zend_string *str_key;
	ulong idx;

	if (Z_TYPE_P(left) != IS_ARRAY) {
		zend_error(E_NOTICE, "The first parameter of phalcon_merge_append must be an array");
		return;
	}

	if (Z_TYPE_P(values) != IS_ARRAY) {
		zend_error(E_WARNING, "Second argument is not an array");
		return;
	}

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(values), idx, str_key, value) {
		zval key = {};
		if (str_key) {
				ZVAL_STR(&key, str_key);
		} else {
				ZVAL_LONG(&key, idx);
		}

		phalcon_array_update(left, &key, value, PH_COPY);
	} ZEND_HASH_FOREACH_END();
}

void phalcon_array_get_current(zval *return_value, zval *array){

	zval *entry;

	if (Z_TYPE_P(array) == IS_ARRAY) {
		if ((entry = zend_hash_get_current_data(Z_ARRVAL_P(array))) == NULL) {
			RETURN_FALSE;
		}
		RETURN_ZVAL(entry, 1, 0);
	}

	RETURN_FALSE;
}

void phalcon_array_get_key(zval *return_value, zval *array){

	if (Z_TYPE_P(array) == IS_ARRAY) {
		zend_hash_get_current_key_zval(Z_ARRVAL_P(array), return_value);
	}

	RETURN_FALSE;
}

int phalcon_fast_in_array(zval *needle, zval *haystack) {

	zval *tmp;
	unsigned int numelems;

	if (Z_TYPE_P(haystack) != IS_ARRAY) {
		return 0;
	}

	numelems = zend_hash_num_elements(Z_ARRVAL_P(haystack));

	if (numelems == 0) {
		return 0;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(haystack), tmp) {
		if (phalcon_is_equal(needle, tmp)) {
			return 1;
		}
	} ZEND_HASH_FOREACH_END();

	return 0;
}

void phalcon_fast_array_merge(zval *return_value, zval *array1, zval *array2) {

	int init_size, num;

	if (Z_TYPE_P(array1) != IS_ARRAY) {
		zend_error(E_WARNING, "First argument is not an array");
		RETURN_NULL();
	}

	if (Z_TYPE_P(array2) != IS_ARRAY) {
		zend_error(E_WARNING, "Second argument is not an array");
		RETURN_NULL();
	}

	init_size = zend_hash_num_elements(Z_ARRVAL_P(array1));
	num = zend_hash_num_elements(Z_ARRVAL_P(array2));
	if (num > init_size) {
		init_size = num;
	}

	array_init_size(return_value, init_size);

	php_array_merge(Z_ARRVAL_P(return_value), Z_ARRVAL_P(array1));
	php_array_merge(Z_ARRVAL_P(return_value), Z_ARRVAL_P(array2));
}

void phalcon_array_merge_recursive_n(zval *a1, zval *a2)
{
	zval *value;
	zend_string *str_key;
	ulong idx;

	assert(Z_TYPE_P(a1) == IS_ARRAY);
	assert(Z_TYPE_P(a2)  == IS_ARRAY);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(a2), idx, str_key, value) {
		zval key = {}, tmp = {};
		if (str_key) {
			ZVAL_STR(&key, str_key);
		} else {
			ZVAL_LONG(&key, idx);
		}

		if (!phalcon_array_isset_fetch(&tmp, a1, &key, PH_READONLY) || Z_TYPE_P(value) != IS_ARRAY) {
			phalcon_array_update(a1, &key, value, PH_COPY);
		} else {
			phalcon_array_merge_recursive_n(&tmp, value);
		}
	} ZEND_HASH_FOREACH_END();
}

void phalcon_array_merge_recursive_n2(zval *a1, zval *a2, int flags)
{
	zval *value;
	zend_string *str_key;
	ulong idx;

	assert(Z_TYPE_P(a1) == IS_ARRAY);
	assert(Z_TYPE_P(a2)  == IS_ARRAY);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(a2), idx, str_key, value) {
		zval key = {}, tmp = {};
		if (str_key) {
			ZVAL_STR(&key, str_key);
		} else {
			ZVAL_LONG(&key, idx);
		}

		if (!phalcon_array_isset_fetch(&tmp, a1, &key, PH_READONLY)) {
			phalcon_array_update(a1, &key, value, flags);
		} else if (Z_TYPE_P(value) == IS_ARRAY) {
			phalcon_array_merge_recursive_n2(&tmp, value, flags);
		}
	} ZEND_HASH_FOREACH_END();
}

void phalcon_array_keys(zval *return_value, zval *arr)
{
	zend_string *str_key;
	ulong idx;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(arr)));

		ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(arr), idx, str_key) {
			zval key = {};
			if (str_key) {
					ZVAL_STR(&key, str_key);
			} else {
					ZVAL_LONG(&key, idx);
			}
			Z_TRY_ADDREF(key);
			zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &key);
		} ZEND_HASH_FOREACH_END();
	}
}

void phalcon_array_values(zval *return_value, zval *arr)
{
	zval *entry;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(arr)));

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(arr), entry) {
			Z_TRY_ADDREF_P(entry);
			zend_hash_next_index_insert(Z_ARRVAL_P(return_value), entry);
		} ZEND_HASH_FOREACH_END();
	}
}

int phalcon_array_key_exists(zval *arr, zval *key)
{
	HashTable *h = HASH_OF(arr);
	if (h) {
		switch (Z_TYPE_P(key)) {
			case IS_STRING:
				return zend_hash_exists(h, Z_STR_P(key));

			case IS_LONG:
				return zend_hash_index_exists(h, Z_LVAL_P(key));

			case IS_NULL:
				return zend_hash_str_exists(h, "", 0);

			default:
				zend_error(E_WARNING, "The key should be either a string or an integer");
				return 0;
		}
	}

	return 0;
}

int phalcon_array_is_associative(zval *arr) {

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		zend_string *str_key;

		ZEND_HASH_FOREACH_STR_KEY(Z_ARRVAL_P(arr), str_key) {
			if (str_key) {
				return 1;
			}
		} ZEND_HASH_FOREACH_END();
	}

	return 0;
}

/**
 * Implementation of Multiple array-offset update
 */
void phalcon_array_update_multi_ex(zval *arr, zval *value, const char *types, int types_length, int types_count, va_list ap)
{
	long old_l[PHALCON_MAX_ARRAY_LEVELS], old_ll[PHALCON_MAX_ARRAY_LEVELS];
	char *s, *old_s[PHALCON_MAX_ARRAY_LEVELS], old_type[PHALCON_MAX_ARRAY_LEVELS];
	zval *item, *old_item[PHALCON_MAX_ARRAY_LEVELS];
	zval pzv;
	zend_array *p, *old_p[PHALCON_MAX_ARRAY_LEVELS];
	zval tmp;
	int i, j, l, ll, re_update, must_continue, wrap_tmp;

	assert(types_length < PHALCON_MAX_ARRAY_LEVELS);
	ZVAL_UNDEF(&tmp);
	ZVAL_UNDEF(&pzv);

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_ERROR, "Cannot use a scalar value as an array (multi)");
		return;
	}
	p = Z_ARRVAL_P(arr);

	for (i = 0; i < types_length; ++i) {
		zval fetched;
		ZVAL_UNDEF(&fetched);

		re_update = 0;
		must_continue = 0;
		wrap_tmp = 0;

		old_p[i] = p;
		ZVAL_ARR(&pzv, p);
		switch (types[i]) {

			case 's':
				s = va_arg(ap, char*);
				l = va_arg(ap, int);
				old_s[i] = s;
				old_l[i] = l;
				if (phalcon_array_isset_fetch_str(&fetched, &pzv, s, l, PH_READONLY)) {
					if (Z_TYPE(fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							re_update = !Z_REFCOUNTED(pzv) || (Z_REFCOUNT(pzv) > 1 && !Z_ISREF(pzv));
							phalcon_array_update_str(&pzv, s, l, value, PH_COPY | PH_SEPARATE);
							p = Z_ARRVAL(pzv);
						} else {
							p = Z_ARRVAL(fetched);
							Z_ADDREF(fetched);
						}
						must_continue = 1;
					}
				}

				if (!must_continue) {
					ZVAL_ARR(&pzv, p);
					re_update = !Z_REFCOUNTED(pzv) || (Z_REFCOUNT(pzv) > 1 && !Z_ISREF(pzv));
					if (i == (types_length - 1)) {
						phalcon_array_update_str(&pzv, s, l, value, PH_COPY | PH_SEPARATE);
						p = Z_ARRVAL(pzv);
					} else {
						array_init(&tmp);
						phalcon_array_update_str(&pzv, s, l, &tmp, PH_SEPARATE);
						p = Z_ARRVAL(pzv);
						if (re_update) {
							wrap_tmp = 1;
						} else {
							p = Z_ARRVAL(tmp);
						}
					}
				}
				break;

			case 'l':
				ll = va_arg(ap, long);
				old_ll[i] = ll;
				if (phalcon_array_isset_fetch_long(&fetched, &pzv, ll, PH_READONLY)) {
					if (Z_TYPE(fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							re_update = !Z_REFCOUNTED(pzv) || (Z_REFCOUNT(pzv) > 1 && !Z_ISREF(pzv));
							phalcon_array_update_long(&pzv, ll, value, PH_COPY | PH_SEPARATE);
							p = Z_ARRVAL(pzv);
						} else {
							p = Z_ARRVAL(fetched);
							Z_ADDREF(fetched);
						}
						must_continue = 1;
					}
				}

				if (!must_continue) {
					ZVAL_ARR(&pzv, p);
					re_update = !Z_REFCOUNTED(pzv) || (Z_REFCOUNT(pzv) > 1 && !Z_ISREF(pzv));
					if (i == (types_length - 1)) {
						phalcon_array_update_long(&pzv, ll, value, PH_COPY | PH_SEPARATE);
						p = Z_ARRVAL(pzv);
					} else {
						array_init(&tmp);
						phalcon_array_update_long(&pzv, ll, &tmp, PH_SEPARATE);
						p = Z_ARRVAL(pzv);
						if (re_update) {
							wrap_tmp = 1;
						} else {
							p = Z_ARRVAL(tmp);
						}
					}
				}
				break;

			case 'z':
				item = va_arg(ap, zval*);
				old_item[i] = item;
				if (phalcon_array_isset_fetch(&fetched, &pzv, item, 1)) {
					if (Z_TYPE(fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							re_update = !Z_REFCOUNTED(pzv) || (Z_REFCOUNT(pzv) > 1 && !Z_ISREF(pzv));
							phalcon_array_update(&pzv, item, value, PH_COPY | PH_SEPARATE);
							p = Z_ARRVAL(pzv);
						} else {
							p = Z_ARRVAL(fetched);
							Z_ADDREF(fetched);
						}
						must_continue = 1;
					}
				}

				if (!must_continue) {
					ZVAL_ARR(&pzv, p);
					re_update = !Z_REFCOUNTED(pzv) || (Z_REFCOUNT(pzv) > 1 && !Z_ISREF(pzv));
					if (i == (types_length - 1)) {
						phalcon_array_update(&pzv, item, value, PH_COPY | PH_SEPARATE);
						p = Z_ARRVAL(pzv);
					} else {
						array_init(&tmp);
						phalcon_array_update(&pzv, item, &tmp, PH_SEPARATE);
						p = Z_ARRVAL(pzv);
						if (re_update) {
							wrap_tmp = 1;
						} else {
							p = Z_ARRVAL(tmp);
						}
					}
				}
				break;

			case 'a':
				re_update = !Z_REFCOUNTED(pzv) || (Z_REFCOUNT(pzv) > 1 && !Z_ISREF(pzv));
				phalcon_array_append(&pzv, value, PH_SEPARATE);
				p = Z_ARRVAL(pzv);
				break;
		}

		if (re_update) {
			for (j = i - 1; j >= 0; j--) {
				zval old_pzv;

				if (!re_update) {
					break;
				}

				ZVAL_ARR(&pzv, old_p[j]);
				re_update = !Z_REFCOUNTED(pzv) || (Z_REFCOUNT(pzv) > 1 && !Z_ISREF(pzv));

				if (j == i - 1) {
					ZVAL_ARR(&old_pzv, p);
				} else {
					ZVAL_ARR(&old_pzv, old_p[j + 1]);
				}

				switch (old_type[j])
				{
					case 's':
						phalcon_array_update_str(&pzv, old_s[j], old_l[j], &old_pzv, PH_SEPARATE);
						break;
					case 'l':
						phalcon_array_update_long(&pzv, old_ll[j], &old_pzv, PH_SEPARATE);
						break;
					case 'z':
						phalcon_array_update(&pzv, old_item[j], &old_pzv, PH_SEPARATE);
						break;
				}
				old_p[j] = Z_ARRVAL(pzv);
				if (wrap_tmp) {
					p = Z_ARRVAL(tmp);
					wrap_tmp = 0;
				}
			}
		}

		if (i != (types_length - 1)) {
			old_type[i] = types[i];
		}
	}
	ZVAL_COPY_VALUE(arr, &pzv);
}

int phalcon_array_update_multi(zval *arr, zval *value, const char *types, int types_length, int types_count, ...)
{
	va_list ap;

	va_start(ap, types_count);
	SEPARATE_ZVAL_IF_NOT_REF(arr);

	phalcon_array_update_multi_ex(arr, value, types, types_length, types_count, ap);
	va_end(ap);

	return 0;
}

static int phalcon_array_key_compare(const void *a, const void *b)
{
	Bucket *f = (Bucket *) a;
	Bucket *s = (Bucket *) b;
	zend_uchar t;
	zend_long l1, l2;
	double d;

	if (f->key == NULL) {
		if (s->key == NULL) {
			return (zend_long)f->h > (zend_long)s->h ? 1 : -1;
		} else {
			l1 = (zend_long)f->h;
			t = is_numeric_string(s->key->val, s->key->len, &l2, &d, 1);
			if (t == IS_LONG) {
				/* pass */
			} else if (t == IS_DOUBLE) {
				return ZEND_NORMALIZE_BOOL((double)l1 - d);
			} else {
				l2 = 0;
			}
		}
	} else {
		if (s->key) {
			return zendi_smart_strcmp(f->key, s->key);
		} else {
			l2 = (zend_long)s->h;
			t = is_numeric_string(f->key->val, f->key->len, &l1, &d, 1);
			if (t == IS_LONG) {
				/* pass */
			} else if (t == IS_DOUBLE) {
				return ZEND_NORMALIZE_BOOL(d - (double)l2);
			} else {
				l1 = 0;
			}
		}
	}
	return l1 > l2 ? 1 : (l1 < l2 ? -1 : 0);
}

int phalcon_array_ksort(zval *arr, int reverse) {

	compare_func_t cmp = phalcon_array_key_compare;
	if (zend_hash_sort(Z_ARRVAL_P(arr), cmp, 0) == FAILURE) {
		return 0;
	}
	return 1;
}
