
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

#include "php_phalcon.h"

#include "kernel/memory.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/array.h"

#include <Zend/zend_smart_str.h>

/**
 * Destroyes the prepared ASTs
 */
void phalcon_orm_destroy_cache() {

	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;

	if (phalcon_globals_ptr->orm.ast_cache != NULL) {
		zend_hash_destroy(phalcon_globals_ptr->orm.ast_cache);
		FREE_HASHTABLE(phalcon_globals_ptr->orm.ast_cache);
		phalcon_globals_ptr->orm.ast_cache = NULL;
	}
}

/**
 * Obtains a prepared ast in the phalcon's superglobals
 */
void phalcon_orm_get_prepared_ast(zval *return_value, zval *unique_id) {

	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;
	zval *temp_ast;

	if (Z_TYPE_P(unique_id) == IS_LONG) {
		if (phalcon_globals_ptr->orm.cache_level >= 0) {
			if (phalcon_globals_ptr->orm.ast_cache != NULL) {
				if ((temp_ast = zend_hash_index_find(phalcon_globals_ptr->orm.ast_cache, Z_LVAL_P(unique_id))) != NULL) {
					ZVAL_ZVAL(return_value, temp_ast, 1, 0);
					return;
				}
			}
		}
	}
}

/**
 * Stores a prepared ast in the phalcon's superglobals
 */
void phalcon_orm_set_prepared_ast(zval *unique_id, zval *prepared_ast) {

	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;
	zval copy_ast = {};

	if (Z_TYPE_P(unique_id) == IS_LONG) {
		if (phalcon_globals_ptr->orm.cache_level >= 0) {

			if (!phalcon_globals_ptr->orm.ast_cache) {
				ALLOC_HASHTABLE(phalcon_globals_ptr->orm.ast_cache);
				zend_hash_init(phalcon_globals_ptr->orm.ast_cache, 0, NULL, ZVAL_PTR_DTOR, 0);
			}

			array_init(&copy_ast);

			zend_hash_copy(Z_ARRVAL(copy_ast), Z_ARRVAL_P(prepared_ast), (copy_ctor_func_t)zval_add_ref);

			zend_hash_index_update(phalcon_globals_ptr->orm.ast_cache, Z_LVAL_P(unique_id), &copy_ast);
		}
	}

}

/**
 * Escapes single quotes into database single quotes
 */
void phalcon_orm_singlequotes(zval *return_value, zval *str) {

	int i;
	smart_str  escaped_str = {0};
	char *marker;

	if (Z_TYPE_P(str) != IS_STRING) {
		RETURN_ZVAL(str, 1, 0);
	}

	marker = Z_STRVAL_P(str);

	for (i = 0; i < Z_STRLEN_P(str); i++) {
		if ((*marker) == '\0') {
			break;
		}
		if ((*marker) == '\'') {
			if (i > 0) {
				if (*(marker - 1) != '\\') {
					smart_str_appendc(&escaped_str, '\'');
				}
			} else {
				smart_str_appendc(&escaped_str, '\'');
			}
		}
		smart_str_appendc(&escaped_str, (*marker));
		marker++;
	}

	smart_str_0(&escaped_str);

	if (escaped_str.s) {
		RETURN_STR(escaped_str.s);
	} else {
		smart_str_free(&escaped_str);
		RETURN_EMPTY_STRING();
	}
}

void phalcon_orm_phql_build_group(zval *return_value, zval *group) {

	zval group_items = {}, joined_items = {}, *group_item = NULL;

	if (PHALCON_IS_NOT_EMPTY(group)) {
		PHALCON_MM_INIT();
		if (phalcon_is_numeric(group)) {
			PHALCON_SCONCAT_SV(return_value, " GROUP BY ", group);
			PHALCON_MM_ADD_ENTRY(return_value);
		} else if (Z_TYPE_P(group) == IS_ARRAY) {
			array_init(&group_items);
			PHALCON_MM_ADD_ENTRY(&group_items);
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(group), group_item) {
				zval escaped_item = {};
				if (
					phalcon_is_numeric(group_item)
					|| phalcon_memnstr_str(group_item, SL("."))
					|| phalcon_memnstr_str(group_item, SL("["))
					|| phalcon_memnstr_str(group_item, SL("("))
				) {
					phalcon_array_append(&group_items, group_item, PH_COPY);
				} else {
					PHALCON_CONCAT_SVS(&escaped_item, "[", group_item, "]");
					phalcon_array_append(&group_items, &escaped_item, 0);
				}
			} ZEND_HASH_FOREACH_END();

			phalcon_fast_join_str(&joined_items, SL(", "), &group_items);
			PHALCON_SCONCAT_SV(return_value, " GROUP BY ", &joined_items);
			zval_ptr_dtor(&joined_items);
			PHALCON_MM_ADD_ENTRY(return_value);
		} else if (phalcon_memnstr_str(group, SL(","))) {
			zval escaped_items = {};
			array_init(&escaped_items);
			PHALCON_MM_ADD_ENTRY(&escaped_items);

			phalcon_fast_explode_str(&group_items, SL(", "), group);
			PHALCON_MM_ADD_ENTRY(&group_items);
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(group_items), group_item) {
				zval escaped_item = {};
				if (
					phalcon_is_numeric(group_item)
					|| phalcon_memnstr_str(group_item, SL("."))
					|| phalcon_memnstr_str(group_item, SL("["))
					|| phalcon_memnstr_str(group_item, SL("("))
				) {
					phalcon_array_append(&escaped_items, group_item, PH_COPY);
				} else {
					PHALCON_CONCAT_SVS(&escaped_item, "[", group_item, "]");
					phalcon_array_append(&escaped_items, &escaped_item, 0);
				}
			} ZEND_HASH_FOREACH_END();

			phalcon_fast_join_str(&joined_items, SL(", "), &escaped_items);
			PHALCON_SCONCAT_SV(return_value, " GROUP BY ", &joined_items);
			zval_ptr_dtor(&joined_items);
			PHALCON_MM_ADD_ENTRY(return_value);
		} else if (
			phalcon_memnstr_str(group, SL("."))
			|| phalcon_memnstr_str(group, SL("["))
			|| phalcon_memnstr_str(group, SL("("))
		) {
			PHALCON_SCONCAT_SV(return_value, " GROUP BY ", group);
			PHALCON_MM_ADD_ENTRY(return_value);
		} else {
			PHALCON_SCONCAT_SVS(return_value, " GROUP BY [", group, "]");
			PHALCON_MM_ADD_ENTRY(return_value);
		}

		Z_TRY_ADDREF_P(return_value);
		RETURN_MM();
	}
	Z_TRY_ADDREF_P(return_value);
}

void phalcon_orm_phql_build_order(zval *return_value, zval *order) {

	zval order_items = {}, joined_items = {}, *order_item = NULL;

	if (PHALCON_IS_NOT_EMPTY(order)) {
		PHALCON_MM_INIT();
		if (Z_TYPE_P(order) == IS_ARRAY) {
			array_init(&order_items);
			PHALCON_MM_ADD_ENTRY(&order_items);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(order), order_item) {
				zval escaped_item = {};
				if (phalcon_is_numeric(order_item)) {
					phalcon_array_append(&order_items, order_item, PH_COPY);
				} else {
					if (phalcon_memnstr_str(order_item, SL("."))) {
						phalcon_array_append(&order_items, order_item, PH_COPY);
					} else {
						PHALCON_CONCAT_SVS(&escaped_item, "[", order_item, "]");
						phalcon_array_append(&order_items, &escaped_item, 0);
					}
				}
			} ZEND_HASH_FOREACH_END();

			phalcon_fast_join_str(&joined_items, SL(", "), &order_items);
			PHALCON_SCONCAT_SV(return_value, " ORDER BY ", &joined_items);
			zval_ptr_dtor(&joined_items);
			PHALCON_MM_ADD_ENTRY(return_value);
		} else {
			PHALCON_SCONCAT_SV(return_value, " ORDER BY ", order);
			PHALCON_MM_ADD_ENTRY(return_value);
		}
		Z_TRY_ADDREF_P(return_value);
		RETURN_MM();
	}
	Z_TRY_ADDREF_P(return_value);
}

void phalcon_orm_phql_build_limit(zval *return_value, zval *limit) {

	zval number = {}, offset = {};

	if (PHALCON_IS_NOT_EMPTY(limit)) {
		PHALCON_MM_INIT();
		if (Z_TYPE_P(limit) == IS_ARRAY) {
			phalcon_array_fetch_str(&number, limit, SL("number"), PH_NOISY|PH_READONLY);
			if (phalcon_array_isset_fetch_str(&offset, limit, SL("offset"), PH_READONLY) && Z_TYPE(offset) != IS_NULL) {
				PHALCON_SCONCAT_SVSV(return_value, " LIMIT ", &number, " OFFSET ", &offset);
				PHALCON_MM_ADD_ENTRY(return_value);
			} else {
				PHALCON_SCONCAT_SV(return_value, " LIMIT ", &number);
				PHALCON_MM_ADD_ENTRY(return_value);
			}
		} else {
			PHALCON_SCONCAT_SV(return_value, " LIMIT ", limit);
			PHALCON_MM_ADD_ENTRY(return_value);
		}
		Z_TRY_ADDREF_P(return_value);
		RETURN_MM();
	}
	Z_TRY_ADDREF_P(return_value);
}
