
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

int phalcon_array_isset_fetch(zval **fetched, const zval *arr, const zval *index) {

	HashTable *h;
	zval *val;
	int result;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return 0;
	}

	h = Z_ARRVAL_P(arr);
	switch (Z_TYPE_P(index)) {
		case IS_NULL:
			result = (val = zend_hash_str_find(h, SS(""))) != NULL;
			break;

		case IS_DOUBLE:
			result = (val = zend_hash_index_find(h, (ulong)Z_DVAL_P(index))) != NULL;
			break;

		case IS_TRUE:
			result = (val = zend_hash_index_find(h, 1)) != NULL;
			break;

		case IS_FALSE:
			result = (val = zend_hash_index_find(h, 0)) != NULL;
			break;

		case IS_LONG:
		case IS_RESOURCE:
			result = (val = zend_hash_index_find(h, Z_LVAL_P(index))) != NULL;
			break;

		case IS_STRING:
			result = (val = zend_symtable_find(h, Z_STR_P(index))) != NULL;
			break;

		default:
			zend_error(E_WARNING, "Illegal offset type");
			return 0;
	}

	if (result == SUCCESS) {
		*fetched = val;
		return 1;
	}

	return 0;
}

int phalcon_array_isset_string_fetch(zval **fetched, const zval *arr, const char *index, uint index_length) {

	zval *zv;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		if ((zv = zend_hash_str_find(Z_ARRVAL_P(arr), index, index_length)) != NULL) {
			*fetched = zv;
			return 1;
		}
	}

	return 0;
}

int phalcon_array_isset_long_fetch(zval **fetched, const zval *arr, ulong index) {

	zval *zv;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		if ((zv = zend_hash_index_find(Z_ARRVAL_P(arr), index)) != NULL) {
			*fetched = zv;
			return 1;
		}
	}

	return 0;
}

int ZEND_FASTCALL phalcon_array_isset(const zval *arr, const zval *index) {

	HashTable *h;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return 0;
	}

	h = Z_ARRVAL_P(arr);
	switch (Z_TYPE_P(index)) {
		case IS_NULL:
			return zend_hash_str_exists(h, SS(""));

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
			return zend_symtable_exists(h, Z_STR_P(index));

		default:
			zend_error(E_WARNING, "Illegal offset type");
			return 0;
	}
}

int ZEND_FASTCALL phalcon_array_isset_string(const zval *arr, const char *index, uint index_length) {

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		return zend_hash_str_exists(Z_ARRVAL_P(arr), index, index_length);
	}

	return 0;
}

int ZEND_FASTCALL phalcon_array_isset_long(const zval *arr, ulong index) {

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		return zend_hash_index_exists(Z_ARRVAL_P(arr), index);
	}

	return 0;
}

int ZEND_FASTCALL phalcon_array_unset(zval **arr, const zval *index, int flags) {

	HashTable *ht;

	if (Z_TYPE_P(*arr) != IS_ARRAY) {
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(*arr);
	}

	ht = Z_ARRVAL_P(*arr);

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

int ZEND_FASTCALL phalcon_array_unset_string(zval **arr, const char *index, uint index_length, int flags) {

	if (Z_TYPE_P(*arr) != IS_ARRAY) {
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(*arr);
	}

	return zend_hash_str_del(Z_ARRVAL_P(*arr), index, index_length);
}

int ZEND_FASTCALL phalcon_array_unset_long(zval **arr, ulong index, int flags) {

	if (Z_TYPE_P(*arr) != IS_ARRAY) {
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(*arr);
	}

	return zend_hash_index_del(Z_ARRVAL_P(*arr), index);
}

int phalcon_array_append(zval **arr, zval *value, int flags) {

	if (Z_TYPE_P(*arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array");
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(*arr);
	}

	Z_ADDREF_P(value);
	return add_next_index_zval(*arr, value);
}

int phalcon_array_update_zval(zval **arr, const zval *index, zval *value, int flags)
{
	HashTable *ht;

	if (Z_TYPE_P(*arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array (2)");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		zval *new_zv;
		Z_DELREF_P(value);
		ALLOC_ZVAL(new_zv);
		INIT_PZVAL_COPY(new_zv, value);
		value = new_zv;
		zval_copy_ctor(new_zv);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(*arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		Z_ADDREF_P(value);
	}

	ht = Z_ARRVAL_P(*arr);

	return phalcon_array_update_hash(ht, index, value, flags);
}

int phalcon_array_update_hash(HashTable *ht, const zval *index, zval *value, int flags)
{
	int status;

	if ((flags & PH_COPY) == PH_COPY) {
		Z_ADDREF_P(value);
	}

	switch (Z_TYPE_P(index)) {
		case IS_NULL:
			status = zend_symtable_update(ht, zend_string_init("", 1, 0), value) ? SUCCESS : FAILURE;
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
			status = zend_symtable_update(ht, Z_STR_P(index), value) ? SUCCESS : FAILURE;
			break;

		default:
			zend_error(E_WARNING, "Illegal offset type");
			status = FAILURE;
			break;
	}

	if (status == FAILURE && ((flags & PH_COPY) == PH_COPY)) {
		Z_DELREF_P(value);
	}

	return status;
}

int phalcon_array_update_quick_string(zval **arr, const char *index, uint index_length, ulong key, zval *value, int flags){

	if (Z_TYPE_P(*arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array (3)");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		zval *new_zv;
		Z_DELREF_P(value);
		ALLOC_ZVAL(new_zv);
		INIT_PZVAL_COPY(new_zv, value);
		value = new_zv;
		zval_copy_ctor(new_zv);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(*arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		Z_ADDREF_P(value);
	}

	return zend_hash_update(Z_ARRVAL_P(*arr), zend_string_init(index, index_length, 0), value) ? SUCCESS : FAILURE;
}

int phalcon_array_update_long(zval **arr, ulong index, zval *value, int flags){

	if (Z_TYPE_P(*arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		zval *new_zv;
		Z_DELREF_P(value);
		ALLOC_ZVAL(new_zv);
		INIT_PZVAL_COPY(new_zv, value);
		value = new_zv;
		zval_copy_ctor(new_zv);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(*arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		Z_ADDREF_P(value);
	}

	return zend_hash_index_update(Z_ARRVAL_P(*arr), index, value) ? SUCCESS : FAILURE;
}

int phalcon_array_fetch(zval **return_value, const zval *arr, const zval *index, int flags){

	zval *zv;
	HashTable *ht;
	int result;
	ulong uidx = 0;
	char *sidx = NULL;

	if (Z_TYPE_P(arr) == IS_ARRAY) {
		ht = Z_ARRVAL_P(arr);
		switch (Z_TYPE_P(index)) {
			case IS_NULL:
				result = (zv = zend_hash_str_find(ht, SS(""))) != NULL ? SUCCESS : FAILURE;
				sidx   = "";
				break;

			case IS_DOUBLE:
				uidx   = (ulong)Z_DVAL_P(index);
				result = (zv = zend_hash_index_find(ht, uidx)) != NULL ? SUCCESS : FAILURE;
				break;

			case IS_TRUE:
				result = (zv = zend_hash_index_find(ht, 1)) != NULL ? SUCCESS : FAILURE;
				break;

			case IS_FALSE:
				result = (zv = zend_hash_index_find(ht, 0)) != NULL ? SUCCESS : FAILURE;
				break;

			case IS_LONG:
			case IS_RESOURCE:
				result = (zv = zend_hash_index_find(ht, Z_LVAL_P(index))) != NULL ? SUCCESS : FAILURE;
				break;

			case IS_STRING:
				sidx   = Z_STRLEN_P(index) ? Z_STRVAL_P(index) : "";
				result = (zv = zend_symtable_find(ht, Z_STR_P(index))) != NULL ? SUCCESS : FAILURE;
				break;

			default:
				if ((flags & PH_NOISY) == PH_NOISY) {
					zend_error(E_WARNING, "Illegal offset type");
				}

				result = FAILURE;
				break;
		}

		if (result != FAILURE) {
			*return_value = zv;
			Z_ADDREF_P(*return_value);
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

	ALLOC_INIT_ZVAL(*return_value);
	return FAILURE;
}

int phalcon_array_fetch_long(zval **return_value, const zval *arr, ulong index, int silent){

	zval *zv;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		if ((zv = zend_hash_index_find(Z_ARRVAL_P(arr), index)) != NULL) {
			*return_value = zv;
			Z_ADDREF_P(*return_value);
			return SUCCESS;
		}

		if (silent == PH_NOISY) {
			zend_error(E_NOTICE, "Undefined index: %lu", index);
		}
	}
	else {
		if (silent == PH_NOISY) {
			zend_error(E_NOTICE, "Cannot use a scalar value as an array");
		}
	}

	ALLOC_INIT_ZVAL(*return_value);
	return FAILURE;
}

void phalcon_array_append_multi_2(zval **arr, const zval *index, zval *value, int flags){

	zval *temp;

	if (Z_TYPE_P(*arr) == IS_ARRAY) {
		phalcon_array_fetch(&temp, *arr, index, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp);
		if (Z_TYPE_P(temp) != IS_ARRAY) {
			convert_to_array(temp);
		}

		phalcon_array_update_zval(arr, index, temp, 0);
		phalcon_array_append(&temp, value, flags);
	}
}

void phalcon_array_update_multi_2(zval **arr, const zval *index1, const zval *index2, zval *value, int flags){

	zval *temp;

	if (Z_TYPE_P(*arr) == IS_ARRAY) {
		phalcon_array_fetch(&temp, *arr, index1, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp);
		if (Z_TYPE_P(temp) != IS_ARRAY) {
			convert_to_array(temp);
		}

		phalcon_array_update_zval(arr, index1, temp, 0);
		phalcon_array_update_zval(&temp, index2, value, flags | PH_COPY);
	}

}

void phalcon_array_update_string_multi_2(zval **arr, const zval *index1, const char *index2, uint index2_length, zval *value, int flags){

	zval *temp;

	if (Z_TYPE_P(*arr) == IS_ARRAY) {
		phalcon_array_fetch(&temp, *arr, index1, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp);
		if (Z_TYPE_P(temp) != IS_ARRAY) {
			convert_to_array(temp);
		}

		phalcon_array_update_zval(arr, index1, temp, 0);
		phalcon_array_update_string(&temp, index2, index2_length, value, flags | PH_COPY);
	}
}

void phalcon_array_update_long_long_multi_2(zval **arr, ulong index1, ulong index2, zval *value, int flags){

	zval *temp;

	if (Z_TYPE_P(*arr) == IS_ARRAY) {
		phalcon_array_fetch_long(&temp, *arr, index1, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp);
		if (Z_TYPE_P(temp) != IS_ARRAY) {
			convert_to_array(temp);
		}

		phalcon_array_update_long(arr, index1, temp, 0);
		phalcon_array_update_long(&temp, index2, value, flags | PH_COPY);
	}
}

void phalcon_array_update_long_string_multi_2(zval **arr, ulong index1, const char *index2, uint index2_length, zval *value, int flags){

	zval *temp;

	if (Z_TYPE_P(*arr) == IS_ARRAY) {
		phalcon_array_fetch_long(&temp, *arr, index1, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp);
		if (Z_TYPE_P(temp) != IS_ARRAY) {
			convert_to_array(temp);
		}

		phalcon_array_update_long(arr, index1, temp, 0);
		phalcon_array_update_string(&temp, index2, index2_length, value, flags | PH_COPY);
	}
}

void phalcon_array_update_zval_string_append_multi_3(zval **arr, const zval *index1, const char *index2, uint index2_length, zval *value, int flags){

	zval *temp1, *temp2;

	if (Z_TYPE_P(*arr) == IS_ARRAY) {
		phalcon_array_fetch(&temp1, *arr, index1, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp1);
		if (Z_TYPE_P(temp1) != IS_ARRAY) {
			convert_to_array(temp1);
		}

		phalcon_array_update_zval(arr, index1, temp1, 0);

		phalcon_array_fetch_string(&temp2, temp1, index2, index2_length, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp2);
		if (Z_TYPE_P(temp2) != IS_ARRAY) {
			convert_to_array(temp2);
		}

		phalcon_array_update_string(&temp1, index2, index2_length, temp2, 0);
		phalcon_array_append(&temp2, value, flags);
	}
}

void phalcon_array_update_zval_zval_zval_multi_3(zval **arr, const zval *index1, const zval *index2, const zval *index3, zval *value, int flags){

	zval *temp1, *temp2;

	if (Z_TYPE_P(*arr) == IS_ARRAY) {

		phalcon_array_fetch(&temp1, *arr, index1, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp1);
		if (Z_TYPE_P(temp1) != IS_ARRAY) {
			convert_to_array(temp1);
		}

		phalcon_array_update_zval(arr, index1, temp1, 0);

		phalcon_array_fetch(&temp2, temp1, index2, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp2);
		if (Z_TYPE_P(temp2) != IS_ARRAY) {
			convert_to_array(temp2);
		}

		phalcon_array_update_zval(&temp1, index2, temp2, 0);
		phalcon_array_update_zval(&temp2, index3, value, PH_COPY);
	}
}

void phalcon_array_update_string_zval_zval_multi_3(zval **arr, const zval *index1, const zval *index2, const char *index3, uint index3_length, zval *value, int flags){

	zval *temp1, *temp2;

	if (Z_TYPE_P(*arr) == IS_ARRAY) {

		phalcon_array_fetch(&temp1, *arr, index1, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp1);
		if (Z_TYPE_P(temp1) != IS_ARRAY) {
			convert_to_array(temp1);
		}

		phalcon_array_update_zval(arr, index1, temp1, 0);

		phalcon_array_fetch(&temp2, temp1, index2, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp2);
		if (Z_TYPE_P(temp2) != IS_ARRAY) {
			convert_to_array(temp2);
		}

		phalcon_array_update_zval(&temp1, index2, temp2, 0);
		phalcon_array_update_string(&temp2, index3, index3_length, value, PH_COPY);
	}
}

void phalcon_array_update_zval_string_string_multi_3(zval **arr, const zval *index1, const char *index2, uint index2_length, const char *index3, uint index3_length, zval *value, int flags){

	zval *temp1, *temp2;

	if (Z_TYPE_P(*arr) == IS_ARRAY) {

		phalcon_array_fetch(&temp1, *arr, index1, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp1);
		if (Z_TYPE_P(temp1) != IS_ARRAY) {
			convert_to_array(temp1);
		}

		phalcon_array_update_zval(arr, index1, temp1, 0);

		phalcon_array_fetch_string(&temp2, temp1, index2, index2_length, PH_SILENT);

		SEPARATE_ZVAL_IF_NOT_REF(temp2);
		if (Z_TYPE_P(temp2) != IS_ARRAY) {
			convert_to_array(temp2);
		}

		phalcon_array_update_string(&temp1, index2, index2_length, temp2, 0);
		phalcon_array_update_string(&temp2, index3, index3_length, value, PH_COPY);
	}
}


void phalcon_merge_append(zval *left, zval *values){
	zval           *tmp;

	if (Z_TYPE_P(left) != IS_ARRAY) {
		zend_error(E_NOTICE, "The first parameter of phalcon_merge_append must be an array");
		return;
	}

	if (Z_TYPE_P(values) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(values), tmp) {
			Z_ADDREF_P(tmp);
			add_next_index_zval(left, tmp);
		} ZEND_HASH_FOREACH_END();

	} else {
		Z_ADDREF_P(values);
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

void phalcon_fast_array_merge(zval *return_value, zval **array1, zval **array2) {

	int init_size, num;

	if (Z_TYPE_P(*array1) != IS_ARRAY) {
		zend_error(E_WARNING, "First argument is not an array");
		RETURN_NULL();
	}

	if (Z_TYPE_P(*array2) != IS_ARRAY) {
		zend_error(E_WARNING, "Second argument is not an array");
		RETURN_NULL();
	}

	init_size = zend_hash_num_elements(Z_ARRVAL_P(*array1));
	num = zend_hash_num_elements(Z_ARRVAL_P(*array2));
	if (num > init_size) {
		init_size = num;
	}

	array_init_size(return_value, init_size);

	php_array_merge(Z_ARRVAL_P(return_value), Z_ARRVAL_P(*array1), 0);

	php_array_merge(Z_ARRVAL_P(return_value), Z_ARRVAL_P(*array2), 0);

}

void phalcon_array_merge_recursive_n(zval **a1, zval *a2)
{
	HashPosition hp;
	zval *value, key, *tmp1, *tmp2;

	assert(Z_TYPE_P(*a1) == IS_ARRAY);
	assert(Z_TYPE_P(a2)  == IS_ARRAY);

	for (
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(a2), &hp);
		(value = zend_hash_get_current_data_ex(Z_ARRVAL_P(a2), &hp)) != NULL;
		zend_hash_move_forward_ex(Z_ARRVAL_P(a2), &hp)
	) {
		key = phalcon_get_current_key_w(Z_ARRVAL_P(a2), &hp);

		if (!phalcon_array_isset(*a1, &key) || Z_TYPE_P(*value) != IS_ARRAY) {
			phalcon_array_update_zval(a1, &key, value, PH_COPY | PH_SEPARATE);
		} else {
			phalcon_array_fetch(&tmp1, *a1, &key, PH_NOISY);
			phalcon_array_fetch(&tmp2, a2, &key, PH_NOISY);
			phalcon_array_merge_recursive_n(&tmp1, tmp2);
			phalcon_ptr_dtor(tmp1);
			phalcon_ptr_dtor(tmp2);
		}
	}
}

void phalcon_array_merge_recursive_n2(zval **a1, zval *a2)
{
	HashPosition hp;
	zval *value, key, *tmp1, *tmp2;

	assert(Z_TYPE_P(*a1) == IS_ARRAY);
	assert(Z_TYPE_P(a2)  == IS_ARRAY);

	for (
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(a2), &hp);
		(value = zend_hash_get_current_data_ex(Z_ARRVAL_P(a2), &hp)) != NULL;
		zend_hash_move_forward_ex(Z_ARRVAL_P(a2), &hp)
	) {
		key = phalcon_get_current_key_w(Z_ARRVAL_P(a2), &hp);

		if (!phalcon_array_isset(*a1, &key)) {
			phalcon_array_update_zval(a1, &key, value, PH_COPY | PH_SEPARATE);
		} else if (Z_TYPE_P(*value) == IS_ARRAY) {
			phalcon_array_fetch(&tmp1, *a1, &key, PH_NOISY);
			phalcon_array_fetch(&tmp2, a2, &key, PH_NOISY);
			phalcon_array_merge_recursive_n2(&tmp1, tmp2);
			phalcon_ptr_dtor(tmp1);
			phalcon_ptr_dtor(tmp2);
		}
	}
}

HashTable* phalcon_array_splice(HashTable *in_hash, int offset, int length, zval ***list, int list_count, HashTable **removed) /* {{{ */
{
	HashTable 	*out_hash = NULL;	/* Output hashtable */
	int			 num_in,			/* Number of entries in the input hashtable */
				 pos,				/* Current position in the hashtable */
				 i;					/* Loop counter */
	Bucket		*p;					/* Pointer to hash bucket */
	zval		*entry;				/* Hash entry */

	/* If input hash doesn't exist, we have nothing to do */
	if (!in_hash) {
		return NULL;
	}

	/* Get number of entries in the input hash */
	num_in = zend_hash_num_elements(in_hash);

	/* Clamp the offset.. */
	if (offset > num_in) {
		offset = num_in;
	} else if (offset < 0 && (offset = (num_in + offset)) < 0) {
		offset = 0;
	}

	/* ..and the length */
	if (length < 0) {
		length = num_in - offset + length;
	} else if (((unsigned)offset + (unsigned)length) > (unsigned)num_in) {
		length = num_in - offset;
	}

	/* Create and initialize output hash */
	ALLOC_HASHTABLE(out_hash);
	zend_hash_init(out_hash, (length > 0 ? num_in - length : 0) + list_count, NULL, ZVAL_PTR_DTOR, 0);

	/* Start at the beginning of the input hash and copy entries to output hash until offset is reached */
	for (pos = 0, p = in_hash->pListHead; pos < offset && p ; pos++, p = p->pListNext) {
		/* Get entry and increase reference count */
		entry = *((zval **)p->pData);
		Z_ADDREF_P(entry);

		/* Update output hash depending on key type */
		if (p->key->len == 0) {
			zend_hash_next_index_insert(out_hash, entry);
		} else {
			zend_hash_update(out_hash, p->key, entry);
		}
	}

	/* If hash for removed entries exists, go until offset+length and copy the entries to it */
	if (removed != NULL) {
		for ( ; pos < offset + length && p; pos++, p = p->pListNext) {
			entry = *((zval **)p->pData);
			Z_ADDREF_P(entry);
			if (p->key->len == 0) {
				zend_hash_next_index_insert(*removed, entry);
			} else {
				zend_hash_update(*removed, p->key, entry);
			}
		}
	} else { /* otherwise just skip those entries */
		for ( ; pos < offset + length && p; pos++, p = p->pListNext);
	}

	/* If there are entries to insert.. */
	if (list != NULL) {
		/* ..for each one, create a new zval, copy entry into it and copy it into the output hash */
		for (i = 0; i < list_count; i++) {
			entry = *list[i];
			Z_ADDREF_P(entry);
			zend_hash_next_index_insert(out_hash, entry);
		}
	}

	/* Copy the remaining input hash entries to the output hash */
	for ( ; p ; p = p->pListNext) {
		entry = *((zval **)p->pData);
		Z_ADDREF_P(entry);
		if (p->key->len == 0) {
			zend_hash_next_index_insert(out_hash, entry);
		} else {
			zend_hash_update(out_hash, p->key, entry);
		}
	}

	zend_hash_internal_pointer_reset(out_hash);
	return out_hash;
}
/* }}} */

void phalcon_array_unshift(zval *arr, zval *arg)
{
	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		zval** args[1]      = { &arg };
		php_splice(Z_ARRVAL_P(arr), 0, 0, args, 1, NULL);
	}
}

void phalcon_array_keys(zval *return_value, zval *arr)
{

	zval *new_val, *entry;
	char *skey;
	uint skey_len;
	ulong nkey;
	HashPosition pos;
	
	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {

		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(arr)));

		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arr), &pos);
		while ((entry = zend_hash_get_current_data_ex(Z_ARRVAL_P(arr), &pos)) != NULL) {
			PHALCON_ALLOC_GHOST_ZVAL(new_val);

			switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(arr), &skey, &skey_len, &nkey, &pos)) {
				case HASH_KEY_IS_STRING:
					ZVAL_STRINGL(new_val, skey, skey_len - 1);
					zend_hash_next_index_insert(Z_ARRVAL_P(return_value), new_val);
					break;

				case HASH_KEY_IS_LONG:
					ZVAL_LONG(new_val, nkey);
					zend_hash_next_index_insert(Z_ARRVAL_P(return_value), new_val);
					break;
			}

			zend_hash_move_forward_ex(Z_ARRVAL_P(arr), &pos);
		}
	}
}

void phalcon_array_values(zval *return_value, zval *arr)
{
	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		zval *entry;
		HashPosition pos;

		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(arr)));
		for (
			zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arr), &pos);
			(entry = zend_hash_get_current_data_ex(Z_ARRVAL_P(arr), &pos)) != NULL;
			zend_hash_move_forward_ex(Z_ARRVAL_P(arr), &pos)
		) {
			Z_ADDREF_P(entry);
			zend_hash_next_index_insert(Z_ARRVAL_P(return_value), entry);
		}
	}
}

int phalcon_array_key_exists(zval *arr, zval *key)
{
	HashTable *h = HASH_OF(arr);
	if (h) {
		switch (Z_TYPE_P(key)) {
			case IS_STRING:
				return zend_symtable_exists(h, Z_STR_P(key));

			case IS_LONG:
				return zend_hash_index_exists(h, Z_LVAL_P(key));

			case IS_NULL:
				return zend_hash_exists(h, zend_string_init("", 1, 0));

			default:
				zend_error(E_WARNING, "The key should be either a string or an integer");
				return 0;
		}
	}

	return 0;
}

int phalcon_array_is_associative(zval *arr) {

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		HashPosition pos;
		zval *entry;
		char *skey;
		uint skey_len;
		ulong nkey;
		ulong expected = 0;

		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arr), &pos);
		while ((entry = zend_hash_get_current_data_ex(Z_ARRVAL_P(arr), &pos)) != NULL) {
			if (HASH_KEY_IS_LONG == zend_hash_get_current_key_ex(Z_ARRVAL_P(arr), &skey, &skey_len, &nkey, &pos)) {
				if (expected != nkey) {
					return 1;
				}
			} else {
				return 1;
			}

			++expected;
			zend_hash_move_forward_ex(Z_ARRVAL_P(arr), &pos);
		}
	}

	return 0;
}

/**
 * Implementation of Multiple array-offset update
 */
void phalcon_array_update_multi_ex(zval **arr, zval *value, const char *types, int types_length, int types_count, va_list ap)
{
	long old_l[PHALCON_MAX_ARRAY_LEVELS], old_ll[PHALCON_MAX_ARRAY_LEVELS];
	char *s, *old_s[PHALCON_MAX_ARRAY_LEVELS], old_type[PHALCON_MAX_ARRAY_LEVELS];
	zval *fetched, *tmp, *p, *item, *old_item[PHALCON_MAX_ARRAY_LEVELS], *old_p[PHALCON_MAX_ARRAY_LEVELS];
	int i, j, l, ll, re_update, must_continue, wrap_tmp;

	assert(types_length < PHALCON_MAX_ARRAY_LEVELS);

	p = *arr;

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
				if (phalcon_array_isset_string_fetch(&fetched, p, s, l + 1)) {
					if (Z_TYPE_P(fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
							phalcon_array_update_string(&p, s, l, value, PH_COPY | PH_SEPARATE);
						} else {
							p = fetched;
						}
						must_continue = 1;
					}
				} else {
					Z_DELREF_P(fetched);
				}
				if (!must_continue) {
					re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
					if (i == (types_length - 1)) {
						phalcon_array_update_string(&p, s, l, value, PH_COPY | PH_SEPARATE);
					} else {
						PHALCON_ALLOC_GHOST_ZVAL(tmp);
						array_init(tmp);
						phalcon_array_update_string(&p, s, l, tmp, PH_SEPARATE);
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
				if (phalcon_array_isset_long_fetch(&fetched, p, ll)) {
					if (Z_TYPE_P(fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
							phalcon_array_update_long(&p, ll, value, PH_COPY | PH_SEPARATE);
						} else {
							p = fetched;
						}
						must_continue = 1;
					}
				} else {
					Z_DELREF_P(fetched);
				}
				if (!must_continue) {
					re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
					if (i == (types_length - 1)) {
						phalcon_array_update_long(&p, ll, value, PH_COPY | PH_SEPARATE);
					} else {
						PHALCON_ALLOC_GHOST_ZVAL(tmp);
						array_init(tmp);
						phalcon_array_update_long(&p, ll, tmp, PH_SEPARATE);
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
					if (Z_TYPE_P(fetched) == IS_ARRAY) {
						if (i == (types_length - 1)) {
							re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
							phalcon_array_update_zval(&p, item, value, PH_COPY | PH_SEPARATE);
						} else {
							p = fetched;
						}
						must_continue = 1;
					}
				} else {
					Z_DELREF_P(fetched);
				}
				if (!must_continue) {
					re_update = Z_REFCOUNT_P(p) > 1 && !Z_ISREF_P(p);
					if (i == (types_length - 1)) {
						phalcon_array_update_zval(&p, item, value, PH_COPY | PH_SEPARATE);
					} else {
						PHALCON_ALLOC_GHOST_ZVAL(tmp);
						array_init(tmp);
						phalcon_array_update_zval(&p, item, tmp, PH_SEPARATE);
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
				phalcon_array_append(&p, value, PH_SEPARATE);
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
							phalcon_array_update_string(&(old_p[j]), old_s[j], old_l[j], p, PH_SEPARATE);
						} else {
							phalcon_array_update_string(&(old_p[j]), old_s[j], old_l[j], old_p[j+1], PH_SEPARATE);
						}
						if (wrap_tmp) {
							p = tmp;
							wrap_tmp = 0;
						}
						break;

					case 'l':
						if (j == i - 1) {
							phalcon_array_update_long(&(old_p[j]), old_ll[j], p, PH_SEPARATE);
						} else {
							phalcon_array_update_long(&(old_p[j]), old_ll[j], old_p[j+1], PH_SEPARATE);
						}
						if (wrap_tmp) {
							p = tmp;
							wrap_tmp = 0;
						}
						break;

					case 'z':
						if (j == i - 1) {
							phalcon_array_update_zval(&(old_p[j]), old_item[j], p, PH_SEPARATE);
						} else {
							phalcon_array_update_zval(&(old_p[j]), old_item[j], old_p[j+1], PH_SEPARATE);
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

int phalcon_array_update_multi(zval **arr, zval *value, const char *types, int types_length, int types_count, ...)
{
	va_list ap;

	va_start(ap, types_count);
	SEPARATE_ZVAL_IF_NOT_REF(*arr);

	phalcon_array_update_multi_ex(arr, value, types, types_length, types_count, ap);
	va_end(ap);

	return 0;
}
