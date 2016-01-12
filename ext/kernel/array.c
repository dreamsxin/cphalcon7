
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

#include "kernel/array.h"

#include <ext/standard/php_array.h>

#include "kernel/main.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/hash.h"

zval* phalcon_array_return_fetch(const zval *arr, const zval *index)
{
	HashTable *h;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return NULL;
	}

	h = Z_ARRVAL_P(arr);
	switch (Z_TYPE_P(index)) {
		case IS_NULL:
			return zend_hash_str_find(h, SL(""));
			break;

		case IS_DOUBLE:
			return zend_hash_index_find(h, (ulong)Z_DVAL_P(index));
			break;

		case IS_TRUE:
			return zend_hash_index_find(h, 1);
			break;

		case IS_FALSE:
			return zend_hash_index_find(h, 0);
			break;

		case IS_LONG:
		case IS_RESOURCE:
			return zend_hash_index_find(h, Z_LVAL_P(index));
			break;

		case IS_STRING:
			return zend_hash_find(h, Z_STR_P(index));
			break;

		default:
			zend_error(E_WARNING, "Illegal offset type");
	}

	return NULL;
}

int phalcon_array_isset_fetch(zval *fetched, const zval *arr, const zval *index)
{
	zval *val;

	val = phalcon_array_return_fetch(arr, index);

	if (val) {		
		ZVAL_COPY(fetched, val);
		return 1;
	}

	return 0;
}

int phalcon_array_isset_fetch_long(zval *fetched, const zval *arr, ulong index)
{
	zval z_index;
	ZVAL_LONG(&z_index, index);

	return phalcon_array_isset_fetch(fetched, arr, &z_index);
}

int phalcon_array_isset_fetch_str(zval *fetched, const zval *arr, const char *index, uint index_length)
{
	zval z_index;
	ZVAL_STRING(&z_index, index, index_length);

	return phalcon_array_isset_fetch(fetched, arr, &z_index);
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
			return zend_hash_index_exists(h, 1);

		case IS_FALSE:
			return zend_hash_index_exists(h, 0);

		case IS_LONG:
		case IS_RESOURCE:
			return zend_hash_index_exists(h, Z_LVAL_P(index));

		case IS_STRING:
			return zend_hash_exists(h, Z_STR_P(index));

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

int phalcon_array_update_zval(zval *arr, const zval *index, zval *value, int flags)
{
	HashTable *ht;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array (2)");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		zval *new_zv;
		Z_TRY_DELREF_P(value);
		PHALCON_ALLOC_INIT_ZVAL(new_zv);
		INIT_PZVAL_COPY(new_zv, value);
		value = new_zv;
		zval_copy_ctor(new_zv);
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

	if ((flags & PH_COPY) == PH_COPY) {
		Z_TRY_ADDREF_P(value);
	}

	switch (Z_TYPE_P(index)) {
		case IS_NULL:
			status = zend_hash_update(ht, zend_string_init("", 1, 0), value) ? SUCCESS : FAILURE;
			break;

		case IS_DOUBLE:
			status = zend_hash_index_update(ht, (ulong)Z_DVAL_P(index), value) ? SUCCESS : FAILURE;
			break;

		case IS_TRUE:
			status = zend_hash_index_update(ht, 1, value) ? SUCCESS : FAILURE;
			break;

		case IS_FALSE:
			status = zend_hash_index_update(ht, 0, value) ? SUCCESS : FAILURE;
			break;

		case IS_LONG:
		case IS_RESOURCE:
			status = zend_hash_index_update(ht, Z_LVAL_P(index), value) ? SUCCESS : FAILURE;
			break;

		case IS_STRING:
			status = zend_hash_update(ht, Z_STR_P(index), value) ? SUCCESS : FAILURE;
			break;

		default:
			zend_error(E_WARNING, "Illegal offset type");
			status = FAILURE;
			break;
	}

	if (status == FAILURE && ((flags & PH_COPY) == PH_COPY)) {
		Z_TRY_DELREF_P(value);
	}

	return status;
}

int phalcon_array_update_str(zval *arr, const char *index, uint index_length, zval *value, int flags){

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array (3)");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		zval *new_zv;
		Z_TRY_DELREF_P(value);
		PHALCON_ALLOC_INIT_ZVAL(new_zv);
		INIT_PZVAL_COPY(new_zv, value);
		value = new_zv;
		zval_copy_ctor(new_zv);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		Z_TRY_ADDREF_P(value);
	}

	return zend_hash_update(Z_ARRVAL_P(arr), zend_string_init(index, index_length, 0), value) ? SUCCESS : FAILURE;
}

int phalcon_array_update_string(zval *arr, zend_string *index, zval *value, int flags){

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array (3)");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		zval *new_zv;
		Z_TRY_DELREF_P(value);
		PHALCON_ALLOC_INIT_ZVAL(new_zv);
		INIT_PZVAL_COPY(new_zv, value);
		value = new_zv;
		zval_copy_ctor(new_zv);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		Z_TRY_ADDREF_P(value);
	}

	return zend_hash_update(Z_ARRVAL_P(arr), index, value) ? SUCCESS : FAILURE;
}

int phalcon_array_update_long(zval *arr, ulong index, zval *value, int flags){

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		zval *new_zv;
		Z_TRY_DELREF_P(value);
		PHALCON_ALLOC_INIT_ZVAL(new_zv);
		INIT_PZVAL_COPY(new_zv, value);
		value = new_zv;
		zval_copy_ctor(new_zv);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		Z_TRY_ADDREF_P(value);
	}

	return zend_hash_index_update(Z_ARRVAL_P(arr), index, value) ? SUCCESS : FAILURE;
}

int phalcon_array_fetch(zval *return_value, const zval *arr, const zval *index, int flags){

	zval *zv;
	HashTable *ht;
	int result = 0;
	ulong uidx = 0;
	char *sidx = NULL;

	if (Z_TYPE_P(arr) == IS_ARRAY) {
		ht = Z_ARRVAL_P(arr);
		switch (Z_TYPE_P(index)) {
			case IS_NULL:
				result = (zv = zend_hash_str_find(ht, SL(""))) != NULL;
				sidx   = "";
				break;

			case IS_DOUBLE:
				uidx   = (ulong)Z_DVAL_P(index);
				result = (zv = zend_hash_index_find(ht, uidx)) != NULL;
				break;

			case IS_TRUE:
				result = (zv = zend_hash_index_find(ht, 1)) != NULL;
				break;

			case IS_FALSE:
				result = (zv = zend_hash_index_find(ht, 0)) != NULL;
				break;

			case IS_LONG:
			case IS_RESOURCE:
				result = (zv = zend_hash_index_find(ht, Z_LVAL_P(index))) != NULL;
				break;

			case IS_STRING:
				sidx   = Z_STRLEN_P(index) ? Z_STRVAL_P(index) : "";
				result = (zv = zend_symtable_find(ht, Z_STR_P(index))) != NULL;
				break;

			default:
				if ((flags & PH_NOISY) == PH_NOISY) {
					zend_error(E_WARNING, "Illegal offset type");
				}

				result = 0;
				break;
		}

		if (result) {
			ZVAL_COPY(return_value, zv);
			return 1;
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

	return 0;
}

int phalcon_array_fetch_str(zval *return_value, const zval *arr, const char *index, uint index_length, int flags){

	zval *zv;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		if ((zv = zend_hash_str_find(Z_ARRVAL_P(arr), index, index_length)) != NULL) {
			ZVAL_COPY(return_value, zv);
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
			ZVAL_COPY(return_value, zv);
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

int phalcon_array_fetch_long(zval *return_value, const zval *arr, ulong index, int silent)
{
	zval *zv;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		if ((zv = zend_hash_index_find(Z_ARRVAL_P(arr), index)) != NULL) {
			ZVAL_COPY(return_value, zv);
			return SUCCESS;
		}

		if (silent == PH_NOISY) {
			zend_error(E_NOTICE, "Undefined index: %lu", index);
		}
	} else {
		if (silent == PH_NOISY) {
			zend_error(E_NOTICE, "Cannot use a scalar value as an array");
		}
	}

	ZVAL_NULL(return_value);

	return FAILURE;
}

void phalcon_array_append_multi_2(zval *arr, const zval *index, zval *value, int flags)
{
	zval tmp;

	if (Z_TYPE_P(arr) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&tmp, arr, index)) {
			SEPARATE_ZVAL_IF_NOT_REF(&tmp);

			if (Z_TYPE(tmp) != IS_ARRAY) {
				convert_to_array(&tmp);
			}
		} else {
			array_init(&tmp);
		}

		phalcon_array_append(&tmp, value, flags);
		phalcon_array_update_zval(arr, index, &tmp, PH_COPY);
	}
}

void phalcon_array_update_multi_2(zval *arr, const zval *index1, const zval *index2, zval *value, int flags)
{
	zval tmp;

	if (Z_TYPE_P(arr) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&tmp, arr, index1)) {
			SEPARATE_ZVAL_IF_NOT_REF(&tmp);

			if (Z_TYPE(tmp) != IS_ARRAY) {
				convert_to_array(&tmp);
			}
		} else {
			array_init(&tmp);
		}

		phalcon_array_update_zval(&tmp, index2, value, flags | PH_COPY);
		phalcon_array_update_zval(arr, index1, &tmp, PH_COPY);
	}

}

void phalcon_array_update_str_multi_2(zval *arr, const zval *index1, const char *index2, uint index2_length, zval *value, int flags)
{
	zval z_index2;
	ZVAL_STRINGL(&z_index2, index2, index2_length);

	phalcon_array_update_multi_2(arr, index1, &z_index2, value, flags);
}

void phalcon_array_update_long_long_multi_2(zval *arr, ulong index1, ulong index2, zval *value, int flags)
{
	zval z_index1, z_index2;
	ZVAL_LONG(&z_index1, index1);
	ZVAL_LONG(&z_index2, index2);

	phalcon_array_update_multi_2(arr, &z_index1, &z_index2, value, flags);
}

void phalcon_array_update_long_str_multi_2(zval *arr, ulong index1, const char *index2, uint index2_length, zval *value, int flags)
{
	zval z_index1, z_index2;
	ZVAL_LONG(&z_index1, index1);
	ZVAL_STRINGL(&z_index2, index2, index2_length);

	phalcon_array_update_multi_2(arr, &z_index1, &z_index2, value, flags);
}

void phalcon_array_update_zval_str_append_multi_3(zval *arr, const zval *index1, const char *index2, uint index2_length, zval *value, int flags){

	zval tmp1, tmp2;

	if (Z_TYPE_P(arr) == IS_ARRAY) {		
		if (phalcon_array_isset_fetch(&tmp1, arr, index1)) {
			SEPARATE_ZVAL_IF_NOT_REF(&tmp1);

			if (Z_TYPE(tmp1) != IS_ARRAY) {
				convert_to_array(&tmp1);
			}
		} else {
			array_init(&tmp1);
		}

		if (phalcon_array_isset_fetch_str(&tmp2, &tmp1, index2, index2_length)) {
			SEPARATE_ZVAL_IF_NOT_REF(&tmp2);

			if (Z_TYPE(tmp2) != IS_ARRAY) {
				convert_to_array(&tmp2);
			}
		} else {
			array_init(&tmp2);
		}

		phalcon_array_append(&tmp2, value, flags);
		phalcon_array_update_str(&tmp1, index2, index2_length, &tmp2, PH_COPY);
		phalcon_array_update_zval(arr, index1, &tmp1, PH_COPY);
	}
}

void phalcon_array_update_zval_zval_zval_multi_3(zval *arr, const zval *index1, const zval *index2, const zval *index3, zval *value, int flags)
{
	zval tmp1, tmp2;

	if (Z_TYPE_P(arr) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&tmp1, arr, index1)) {
			SEPARATE_ZVAL_IF_NOT_REF(&tmp1);

			if (Z_TYPE(tmp1) != IS_ARRAY) {
				convert_to_array(&tmp1);
			}
		} else {
			array_init(&tmp1);
		}

		if (phalcon_array_isset_fetch(&tmp2, &tmp1, index2)) {
			SEPARATE_ZVAL_IF_NOT_REF(&tmp2);

			if (Z_TYPE(tmp2) != IS_ARRAY) {
				convert_to_array(&tmp2);
			}
		} else {
			array_init(&tmp2);
		}

		phalcon_array_update_zval(&tmp2, index3, value, PH_COPY);
		phalcon_array_update_zval(&tmp1, index2, &tmp2, PH_COPY);
		phalcon_array_update_zval(arr, index1, &tmp1, PH_COPY);
	}
}

void phalcon_array_update_zval_zval_str_multi_3(zval *arr, const zval *index1, const zval *index2, const char *index3, uint index3_length, zval *value, int flags)
{
	zval z_index3;
	ZVAL_STRINGL(&z_index3, index3, index3_length);

	phalcon_array_update_zval_zval_zval_multi_3(arr, index1, index2, &z_index3, value, flags);
}

void phalcon_array_update_zval_str_str_multi_3(zval *arr, const zval *index1, const char *index2, uint index2_length, const char *index3, uint index3_length, zval *value, int flags)
{
	zval z_index2, z_index3;
	ZVAL_STRINGL(&z_index2, index2, index2_length);
	ZVAL_STRINGL(&z_index3, index3, index3_length);

	phalcon_array_update_zval_zval_zval_multi_3(arr, index1, &z_index2, &z_index3, value, flags);
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

int phalcon_fast_in_array(zval *needle, zval *haystack) {

	zval           *tmp;
	unsigned int   numelems;

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
		zval key, *tmp1, *tmp2;
		if (str_key) {
			ZVAL_STR(&key, str_key);
		} else {
			ZVAL_LONG(&key, idx);
		}

		if (!phalcon_array_isset(a1, &key) || Z_TYPE_P(value) != IS_ARRAY) {
			phalcon_array_update_zval(a1, &key, value, PH_COPY);
		} else {
			phalcon_array_fetch(&tmp1, a1, &key, PH_NOISY);
			phalcon_array_fetch(&tmp2, a2, &key, PH_NOISY);
			phalcon_array_merge_recursive_n(tmp1, tmp2);
			zval_ptr_dtor(tmp1);
			zval_ptr_dtor(tmp2);
		}
	} ZEND_HASH_FOREACH_END();
}

void phalcon_array_merge_recursive_n2(zval *a1, zval *a2)
{
	zval *value;
	zend_string *str_key;
	ulong idx;

	assert(Z_TYPE_P(a1) == IS_ARRAY);
	assert(Z_TYPE_P(a2)  == IS_ARRAY);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(a2), idx, str_key, value) {
		zval key, *tmp1, *tmp2;
		if (str_key) {
			ZVAL_STR(&key, str_key);
		} else {
			ZVAL_LONG(&key, idx);
		}

		if (!phalcon_array_isset(a1, &key)) {
			phalcon_array_update_zval(a1, &key, value, PH_COPY);
		} else if (Z_TYPE_P(value) == IS_ARRAY) {
			phalcon_array_fetch(&tmp1, a1,& key, PH_NOISY);
			phalcon_array_fetch(&tmp2, a2, &key, PH_NOISY);
			phalcon_array_merge_recursive_n2(tmp1, tmp2);
			zval_ptr_dtor(tmp1);
			zval_ptr_dtor(tmp2);
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
			zval key;
			if (str_key) {
					ZVAL_STR(&key, str_key);
			} else {
					ZVAL_LONG(&key, idx);
			}

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
	zval fetched, *tmp, *p, *item, *old_item[PHALCON_MAX_ARRAY_LEVELS], *old_p[PHALCON_MAX_ARRAY_LEVELS];
	int i, j, l, ll, re_update, must_continue, wrap_tmp;

	assert(types_length < PHALCON_MAX_ARRAY_LEVELS);

	p = arr;

	for (i = 0; i < types_length; ++i) {

		re_update = 0;
		must_continue = 0;
		wrap_tmp = 0;

		old_p[i] = p;
		switch (types[i]) {

			case 's':
				s = va_arg(ap, char*);
				l = va_arg(ap, int);
				old_s[i] = s;
				old_l[i] = l;
				if (phalcon_array_isset_fetch_str(&fetched, p, s, l + 1)) {
					if (Z_TYPE_P(&fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
							phalcon_array_update_str(p, s, l, value, PH_COPY);
						} else {
							p = &fetched;
						}
						must_continue = 1;
					}
				} else {
					Z_TRY_DELREF_P(&fetched);
				}
				if (!must_continue) {
					re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
					if (i == (types_length - 1)) {
						phalcon_array_update_str(p, s, l, value, PH_COPY);
					} else {
						PHALCON_ALLOC_INIT_ZVAL(tmp);
						array_init(tmp);
						phalcon_array_update_str(p, s, l, tmp, PH_COPY);
						if (re_update) {
							wrap_tmp = 1;
						} else {
							p = tmp;
						}
					}
				}
				break;

			case 'l':
				ll = va_arg(ap, long);
				old_ll[i] = ll;
				if (phalcon_array_isset_fetch_long(&fetched, p, ll)) {
					if (Z_TYPE_P(&fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
							phalcon_array_update_long(p, ll, value, PH_COPY);
						} else {
							p = &fetched;
						}
						must_continue = 1;
					}
				} else {
					Z_TRY_DELREF_P(fetched);
				}
				if (!must_continue) {
					re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
					if (i == (types_length - 1)) {
						phalcon_array_update_long(p, ll, value, PH_COPY);
					} else {
						PHALCON_ALLOC_INIT_ZVAL(tmp);
						array_init(tmp);
						phalcon_array_update_long(p, ll, tmp, PH_COPY);
						if (re_update) {
							wrap_tmp = 1;
						} else {
							p = tmp;
						}
					}
				}
				break;

			case 'z':
				item = va_arg(ap, zval*);
				old_item[i] = item;
				if (phalcon_array_isset_fetch(&fetched, p, item)) {
					if (Z_TYPE_P(&fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
							phalcon_array_update_zval(p, item, value, PH_COPY);
						} else {
							p = &fetched;
						}
						must_continue = 1;
					}
				} else {
					Z_TRY_DELREF_P(&fetched);
				}
				if (!must_continue) {
					re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
					if (i == (types_length - 1)) {
						phalcon_array_update_zval(p, item, value, PH_COPY);
					} else {
						PHALCON_ALLOC_INIT_ZVAL(tmp);
						array_init(tmp);
						phalcon_array_update_zval(p, item, tmp, PH_COPY);
						if (re_update) {
							wrap_tmp = 1;
						} else {
							p = tmp;
						}
					}
				}
				break;

			case 'a':
				re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
				phalcon_array_append(p, value, PH_COPY);
				break;
		}

		if (re_update) {

			for (j = i - 1; j >= 0; j--) {

				if (!re_update) {
					break;
				}

				re_update = Z_REFCOUNT_P(old_p[j]) > 1 && !Z_ISREF_P(old_p[j]);
				switch (old_type[j]) {

					case 's':
						if (j == i - 1) {
							phalcon_array_update_str((old_p[j]), old_s[j], old_l[j], p, PH_COPY);
						} else {
							phalcon_array_update_str((old_p[j]), old_s[j], old_l[j], old_p[j+1], PH_COPY);
						}
						if (wrap_tmp) {
							p = tmp;
							wrap_tmp = 0;
						}
						break;

					case 'l':
						if (j == i - 1) {
							phalcon_array_update_long((old_p[j]), old_ll[j], p, PH_COPY);
						} else {
							phalcon_array_update_long((old_p[j]), old_ll[j], old_p[j+1], PH_COPY);
						}
						if (wrap_tmp) {
							p = tmp;
							wrap_tmp = 0;
						}
						break;

					case 'z':
						if (j == i - 1) {
							phalcon_array_update_zval((old_p[j]), old_item[j], p, PH_COPY);
						} else {
							phalcon_array_update_zval((old_p[j]), old_item[j], old_p[j+1], PH_COPY);
						}
						if (wrap_tmp) {
							p = tmp;
							wrap_tmp = 0;
						}
						break;
				}

			}
		}

		if (i != (types_length - 1)) {
			old_type[i] = types[i];
		}
	}
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
