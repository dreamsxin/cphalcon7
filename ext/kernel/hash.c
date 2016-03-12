
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

#include "kernel/hash.h"
#include "kernel/memory.h"

/**
 * Initialize an array to start an iteration over it
 */
int phalcon_is_iterable_ex(zval *arr, HashTable **arr_hash, HashPosition *hash_position, int duplicate, int reverse) {

	if (unlikely(Z_TYPE_P(arr) != IS_ARRAY)) {
		return 0;
	}

	if (duplicate) {
		ALLOC_HASHTABLE(*arr_hash);
		zend_hash_init(*arr_hash, 0, NULL, NULL, 0);
		zend_hash_copy(*arr_hash, Z_ARRVAL_P(arr), NULL);
	} else {
		*arr_hash = Z_ARRVAL_P(arr);
	}

	if (reverse) {
		zend_hash_internal_pointer_end_ex(*arr_hash, hash_position);
	} else {
		zend_hash_internal_pointer_reset_ex(*arr_hash, hash_position);
	}

	return 1;
}

/**
int phalcon_has_numeric_keys(const zval *data)
{
	zend_long idx;
	zend_string *key;

	if (Z_TYPE_P(data) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(data), idx, key) {
			if (!key) {
				return 1;
			}
		} ZEND_HASH_FOREACH_END();
	}

	return 0;
}
*/

/**
 * @brief Adds or updates item @a key in the hash table @a ht
 * @param ht Hash table
 * @param[in] key Key
 * @param[in] value Value
 * @return Whether the operation succeeded
 * @retval SUCCESS
 * @retval FAILURE
 * @note @a value's reference count in not updated
 * @note If @a key is @c NULL or is @c IS_NULL, @a value is appended to @a ht
 * @throw E_WARNING if @a key type is not supported
 */
int phalcon_hash_update_or_insert(HashTable *ht, const zval *key, zval *value)
{
	if (!key || Z_TYPE_P(key) == IS_NULL) {
		return zend_hash_next_index_insert(ht, value) ? SUCCESS : FAILURE;
	}

	switch (Z_TYPE_P(key)) {
		case IS_STRING:
			return zend_symtable_update(ht, Z_STR_P(key), value) ? SUCCESS : FAILURE;

		case IS_TRUE:
			return zend_hash_index_update(ht, 1, value) ? SUCCESS : FAILURE;

		case IS_FALSE:
			return zend_hash_index_update(ht, 0, value) ? SUCCESS : FAILURE;

		case IS_RESOURCE:
		case IS_DOUBLE:
		case IS_LONG:
			return zend_hash_index_update(ht, ((Z_TYPE_P(key) == IS_DOUBLE) ? (ulong)Z_DVAL_P(key) : (ulong)Z_LVAL_P(key)), value) ? SUCCESS : FAILURE;

		default:
			zend_error(E_WARNING, "Illegal offset type");
			return FAILURE;
	}
}

/**
 * @brief Returns the entry @a ht identified by @a key
 * @param[in] ht Hash table
 * @param[in] key
 * @param[in] type One of @c BP_VAR_XXX values
 * @return Pointer to the stored value or a pointer to the special variable / @c NULL if @a key was not found
 * @retval <tt>&EG(error_zval)</tt> when @a key was not found and @a type is one of @c BP_VAR_W, @c BP_VAR_RW
 * @retval <tt>&EG(uninitialized_zval)</tt> when @a key was not found and @a type is one of @c BP_VAR_R, @c BP_VAR_UNSET, @c BP_VAR_IS
 * @retval @c NULL when @a key was not found and @a type is not any of the above
 * @throw @c E_WARNING when @a key is of not supported type; in this case the function never returns @c NULL
 * @throw @c E_STRICT when @a key is a resource
 * @throw @c E_NOTICE if @a key was not found and @a type is @c BP_VAR_R or @c BP_VAR_RW
 * @note Reference count of the returned item is not modified
 * @note The implementation is suitable for @c read_property, @c get_property_ptr_ptr and @c read_dimension object handlers
 * @warning If @a type is @c BP_VAR_W or @c BP_VAR_RW and @a key was not found, it is added to @a ht and its value is set to @c IS_NULL
 */
zval* phalcon_hash_get(HashTable *ht, const zval *key, int type)
{
	zval *ret = NULL, value = {};

	switch (Z_TYPE_P(key)) {
		case IS_RESOURCE:
			zend_error(E_STRICT, "Resource ID#%ld used as offset, casting to integer (%ld)", Z_LVAL_P(key), Z_LVAL_P(key));
			/* no break */
		case IS_LONG:
		case IS_DOUBLE:
		case IS_TRUE:
		case IS_FALSE: {
			ulong index = 0;
			if ((Z_TYPE_P(key) == IS_TRUE)) {
				index = 1;
			} else if ((Z_TYPE_P(key) == IS_FALSE)) {
				index = 0;
			} else {
				index = (Z_TYPE_P(key) == IS_DOUBLE) ? ((long int)Z_DVAL_P(key)) : Z_LVAL_P(key);
			}

			if ((ret = zend_hash_index_find(ht, index)) == NULL) {
				switch (type) {
					case BP_VAR_R:
						zend_error(E_NOTICE, "Undefined offset: %ld", index);
						/* no break */
					case BP_VAR_UNSET:
					case BP_VAR_IS: {
						TSRMLS_FETCH();
						ret = &EG(uninitialized_zval);
						break;
					}

					case BP_VAR_RW:
						zend_error(E_NOTICE, "Undefined offset: %ld", index);
						/* no break */
					case BP_VAR_W: {
						ZVAL_NULL(&value);
						zend_hash_index_update(ht, index, &value);
						break;
					}
				}
			}

			return ret;
		}

		case IS_STRING:
			if ((ret = zend_symtable_find(ht, Z_STR_P(key)))  == NULL) {
				switch (type) {
					case BP_VAR_R:
						zend_error(E_NOTICE, "Undefined offset: %s", Z_STRVAL_P(key));
						/* no break */
					case BP_VAR_UNSET:
					case BP_VAR_IS: {
						TSRMLS_FETCH();
						ret = &EG(uninitialized_zval);
						break;
					}

					case BP_VAR_RW:
						zend_error(E_NOTICE, "Undefined offset: %s", Z_STRVAL_P(key));
						/* no break */
					case BP_VAR_W: {
						ZVAL_NULL(&value);
						zend_symtable_update(ht, Z_STR_P(key), &value);
						break;
					}
				}
			}

			return ret;

		default: {
			TSRMLS_FETCH();
			zend_error(E_WARNING, "Illegal offset type");
			return (type == BP_VAR_W || type == BP_VAR_RW) ? &EG(error_zval) : &EG(uninitialized_zval);
		}
	}
}

/**
 * @brief Unset @a key from @a ht
 * @param ht
 * @param key
 * @return
 */
int phalcon_hash_unset(HashTable *ht, const zval *key)
{
	switch (Z_TYPE_P(key)) {
		case IS_TRUE:
			return (zend_hash_index_del(ht, 1) == SUCCESS);

		case IS_FALSE:
			return (zend_hash_index_del(ht, 0) == SUCCESS);

		case IS_LONG:
		case IS_DOUBLE:
		case IS_RESOURCE:
			return (zend_hash_index_del(ht, (Z_TYPE_P(key) == IS_DOUBLE) ? ((long int)Z_DVAL_P(key)) : Z_LVAL_P(key)) == SUCCESS);

		case IS_STRING:
			return (zend_symtable_del(ht, Z_STR_P(key)) == SUCCESS);

		default:
			zend_error(E_WARNING, "Illegal offset type");
			return 0;
	}
}
