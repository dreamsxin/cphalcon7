
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
  +------------------------------------------------------------------------+
*/

%token_prefix PHQL_
%token_type {phql_parser_token*}
%token_destructor {
	if ($$) {
		if ($$->free_flag) {
			efree($$->token);
		}
		efree($$);
	}
}
%default_type {zval}
%default_destructor {
	if (status) {
		// TODO:
	}
	if (&$$) {
		zval_ptr_dtor(&$$);
	}
}
%extra_argument {phql_parser_status *status}
%name phql_

%right AGAINST .
%left BETWEEN .
%left EQUALS NOTEQUALS LESS GREATER GREATEREQUAL LESSEQUAL .
%left TS_MATCHES TS_OR TS_AND TS_NEGATE TS_CONTAINS_ANOTHER TS_CONTAINS_IN .
%left AND OR .
%left LIKE ILIKE .
%left BITWISE_AND BITWISE_OR BITWISE_XOR .
%left DIVIDE TIMES MOD .
%left PLUS MINUS .
%left IS .
%left DOUBLECOLON .
%right IN .
%right NOT BITWISE_NOT .
%right FORCE USE .
%left COMMA .

%include {

#include "php_phalcon.h"

#include "mvc/model/query/parser.h"
#include "mvc/model/query/scanner.h"
#include "mvc/model/query/phql.h"
#include "mvc/model/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/array.h"

#include "kernel/framework/orm.h"

#include "interned-strings.h"

static void phql_ret_literal_zval(zval *ret, int type, phql_parser_token *T)
{
	array_init_size(ret, 2);
	add_assoc_long(ret, ISV(type), type);
	if (T) {
		add_assoc_stringl(ret, ISV(value), T->token, T->token_len);
		efree(T->token);
		efree(T);
	}
}

static void phql_ret_placeholder_zval(zval *ret, int type, phql_parser_token *T)
{
	array_init_size(ret, 2);
	add_assoc_long(ret, ISV(type), type);
	add_assoc_stringl(ret, ISV(value), T->token, T->token_len);
	efree(T->token);
	efree(T);
}

static void phql_ret_qualified_name(zval *ret, phql_parser_token *A, phql_parser_token *B, phql_parser_token *C)
{
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHQL_T_QUALIFIED);

	if (A != NULL) {
		add_assoc_stringl(ret, ISV(ns_alias), A->token, A->token_len);
		efree(A->token);
		efree(A);
	}

	if (B != NULL) {
		add_assoc_stringl(ret, ISV(domain), B->token, B->token_len);
		efree(B->token);
		efree(B);
	}

	add_assoc_stringl(ret, ISV(name), C->token, C->token_len);
	efree(C->token);
	efree(C);
}

static void phql_ret_raw_qualified_name(zval *ret, phql_parser_token *A, phql_parser_token *B)
{
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHQL_T_RAW_QUALIFIED);
	if (B != NULL) {
		add_assoc_stringl(ret, ISV(domain), A->token, A->token_len);
		add_assoc_stringl(ret, ISV(name), B->token, B->token_len);
		efree(B->token);
		efree(B);
	} else {
		add_assoc_stringl(ret, ISV(name), A->token, A->token_len);
	}
	efree(A->token);
	efree(A);
}

static void phql_ret_select_statement(zval *ret, zval *S, zval *W, zval *O, zval *G, zval *H, zval *L, zval *F)
{
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHQL_T_SELECT);
	add_assoc_zval(ret, ISV(select), S);

	if (W && Z_TYPE_P(W) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(where), W);
	}
	if (O && Z_TYPE_P(O) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(orderBy), O);
	}
	if (G && Z_TYPE_P(G) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(groupBy), G);
	}
	if (H && Z_TYPE_P(H) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(having), H);
	}
	if (L && Z_TYPE_P(L) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(limit), L);
	}
	if (F && Z_TYPE_P(F) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(forupdate), F);
	}
}

static void phql_ret_select_clause(zval *ret, zval *distinct, zval *columns, zval *tables, zval *join_list)
{
	array_init_size(ret, 4);

	if (distinct && Z_TYPE_P(distinct) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(distinct), distinct);
	}

	add_assoc_zval(ret, ISV(columns), columns);
	add_assoc_zval(ret, ISV(tables), tables);

	if (join_list && Z_TYPE_P(join_list) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(joins), join_list);
	}
}

static void phql_ret_distinct_all(zval *ret, int distinct)
{
	ZVAL_LONG(ret, distinct);
}

static void phql_ret_distinct(zval *ret)
{
	ZVAL_TRUE(ret);
}

static void phql_ret_order_item(zval *ret, zval *column, int sort)
{
	array_init(ret);
	add_assoc_zval(ret, ISV(column), column);

	if (sort != 0 ) {
		add_assoc_long(ret, ISV(sort), sort);
	}
}

static void phql_ret_limit_clause(zval *ret, zval *L, zval *O)
{
	array_init_size(ret, 2);

	add_assoc_zval(ret, ISV(number), L);

	if (O && Z_TYPE_P(O) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(offset), O);
	}
}

static void phql_ret_for_update_clause(zval *ret)
{
	ZVAL_TRUE(ret);
}

static void phql_ret_insert_statement(zval *ret, zval *Q, zval *F, zval *V)
{
	zval values = {};

	array_init(ret);
	add_assoc_long(ret, ISV(type), PHQL_T_INSERT);
	add_assoc_zval(ret, ISV(qualifiedName), Q);

	if (F && Z_TYPE_P(F) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(fields), F);
	}

	array_init(&values);
	add_next_index_zval(&values, V);

	add_assoc_zval(ret, ISV(values), &values);
}

static void phql_ret_insert_statement2(zval *ret, zval *Q, zval *V)
{
	zval values = {};

	ZVAL_COPY_VALUE(ret, Q);

	if (!phalcon_array_isset_fetch_str(&values, ret, ISL(values), PH_READONLY)) {
		array_init(&values);
		add_assoc_zval(ret, ISV(values), &values);
	}
	add_next_index_zval(&values, V);
}

static void phql_ret_update_statement(zval *ret, zval *U, zval *W, zval *L)
{
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHQL_T_UPDATE);
	add_assoc_zval(ret, ISV(update), U);

	if (W && Z_TYPE_P(W) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(where), W);
	}
	if (L && Z_TYPE_P(L) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(limit), L);
	}
}

static void phql_ret_update_clause(zval *ret, zval *tables, zval *values)
{
	array_init_size(ret, 2);
	add_assoc_zval(ret, ISV(tables), tables);
	add_assoc_zval(ret, ISV(values), values);
}

static void phql_ret_update_item(zval *ret, zval *column, zval *expr)
{
	array_init_size(ret, 2);
	add_assoc_zval(ret, ISV(column), column);
	add_assoc_zval(ret, ISV(expr), expr);
}

static void phql_ret_delete_statement(zval *ret, zval *D, zval *W, zval *L)
{
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHQL_T_DELETE);
	add_assoc_zval(ret, ISV(delete), D);

	if (W && Z_TYPE_P(W) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(where), W);
	}
	if (L && Z_TYPE_P(L) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(limit), L);
	}
}

static void phql_ret_delete_clause(zval *ret, zval *tables)
{
	array_init_size(ret, 1);
	add_assoc_zval(ret, ISV(tables), tables);
}

static void phql_ret_index_list(zval *ret, zval *list_left, zval *right_list)
{
	HashTable *list;

	array_init(ret);

	if (list_left && Z_TYPE_P(list_left) != IS_UNDEF) {
		list = Z_ARRVAL_P(list_left);
		if (zend_hash_index_exists(list, 0)) {
			zval *item;
			ZEND_HASH_FOREACH_VAL(list, item) {
				Z_ADDREF_P(item);
				add_next_index_zval(ret, item);
			} ZEND_HASH_FOREACH_END();
			zval_ptr_dtor(list_left);
		} else {
			add_next_index_zval(ret, list_left);
		}
	}

	if (right_list && Z_TYPE_P(right_list) != IS_UNDEF) {
		add_next_index_zval(ret, right_list);
	}
}

static void phql_ret_index_type(zval *ret, int type, zval *column)
{
	array_init(ret);
	add_assoc_long(ret, ISV(type), type);
	add_assoc_zval(ret, ISV(column), column);
}

static void phql_ret_zval_list(zval *ret, zval *list_left, zval *right_list)
{
	HashTable *list;

	array_init(ret);

	if (list_left && Z_TYPE_P(list_left) != IS_UNDEF) {
		list = Z_ARRVAL_P(list_left);
		if (zend_hash_index_exists(list, 0)) {
			zval *item;
			ZEND_HASH_FOREACH_VAL(list, item) {
				Z_ADDREF_P(item);
				add_next_index_zval(ret, item);
			} ZEND_HASH_FOREACH_END();
			zval_ptr_dtor(list_left);
		} else {
			add_next_index_zval(ret, list_left);
		}
	}

	if (right_list && Z_TYPE_P(right_list) != IS_UNDEF) {
		add_next_index_zval(ret, right_list);
	}
}

static void phql_ret_column_item(zval *ret, int type, zval *column, phql_parser_token *identifier_column, phql_parser_token *alias)
{
	array_init_size(ret, 4);
	add_assoc_long(ret, ISV(type), type);
	if (column && Z_TYPE_P(column) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(column), column);
	}
	if (identifier_column) {
		add_assoc_stringl(ret, ISV(column), identifier_column->token, identifier_column->token_len);
		efree(identifier_column->token);
		efree(identifier_column);
	}
	if (alias) {
		add_assoc_stringl(ret, ISV(alias), alias->token, alias->token_len);
		efree(alias->token);
		efree(alias);
	}
}

static void phql_ret_assoc_name(zval *ret, zval *qualified_name, phql_parser_token *alias, zval *index_list)
{
	array_init_size(ret, 2);
	add_assoc_zval(ret, ISV(qualifiedName), qualified_name);

	if (alias) {
		add_assoc_stringl(ret, ISV(alias), alias->token, alias->token_len);
		efree(alias->token);
		efree(alias);
	}

	if (index_list && Z_TYPE_P(index_list) != IS_UNDEF) {
		add_assoc_zval(ret, "indexs", index_list);
	}
}

static void phql_ret_join_type(zval *ret, int type)
{
	ZVAL_LONG(ret, type);
}

static void phql_ret_join_item(zval *ret, zval *type, zval *qualified, zval *alias, zval *conditions)
{
	array_init_size(ret, 4);
	add_assoc_zval(ret, ISV(type), type);

	if (qualified && Z_TYPE_P(qualified) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(qualified), qualified);
	}

	if (alias && Z_TYPE_P(alias) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(alias), alias);
	}

	if (conditions && Z_TYPE_P(conditions) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(conditions), conditions);
	}
}

static void phql_ret_expr(zval *ret, int type, zval *left, zval *right)
{
	array_init_size(ret, 2);
	add_assoc_long(ret, ISV(type), type);
	if (left && Z_TYPE_P(left) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(left), left);
	}
	if (right && Z_TYPE_P(right) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(right), right);
	}
}

static void phql_ret_func_call(zval *ret, phql_parser_token *name, zval *arguments, zval *distinct)
{
	array_init_size(ret, 4);
	add_assoc_long(ret, ISV(type), PHQL_T_FCALL);
	add_assoc_stringl(ret, ISV(name), name->token, name->token_len);
	efree(name->token);
	efree(name);

	if (arguments && Z_TYPE_P(arguments) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(arguments), arguments);
	}

	if (distinct && Z_TYPE_P(distinct) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(distinct), distinct);
	}
}

}

%syntax_error {
	if (status->scanner_state->start_length) {
		{

			char *token_name = NULL;
			int token_found = 0;
			unsigned int token_length;
			const phql_token_names *tokens = phql_tokens;
			uint active_token = status->scanner_state->active_token;
			uint near_length = status->scanner_state->start_length;

			if (active_token) {

				do {
					if (tokens->code == active_token) {
						token_name = tokens->name;
						token_length = tokens->length;
						token_found = 1;
						break;
					}
					++tokens;
				} while (tokens[0].code != 0);

			}

			if (!token_name) {
				token_length = strlen("UNKNOWN");
				token_name = estrndup("UNKNOWN", token_length);
				token_found = 0;
			}

			status->syntax_error_len = 96 + status->token->len + token_length + near_length + status->phql_length;;
			status->syntax_error = emalloc(sizeof(char) * status->syntax_error_len);

			if (near_length > 0) {
				if (status->token->value) {
					snprintf(status->syntax_error, status->syntax_error_len, "Syntax error, unexpected token %s(%s), near to '%s', when parsing: %s (%d)", token_name, status->token->value, status->scanner_state->start, status->phql, status->phql_length);
				} else {
					snprintf(status->syntax_error, status->syntax_error_len, "Syntax error, unexpected token %s, near to '%s', when parsing: %s (%d)", token_name, status->scanner_state->start, status->phql, status->phql_length);
				}
			} else {
				if (active_token != PHQL_T_IGNORE) {
					if (status->token->value) {
						snprintf(status->syntax_error, status->syntax_error_len, "Syntax error, unexpected token %s(%s), at the end of query, when parsing: %s (%d)", token_name, status->token->value, status->phql, status->phql_length);
					} else {
						snprintf(status->syntax_error, status->syntax_error_len, "Syntax error, unexpected token %s, at the end of query, when parsing: %s (%d)", token_name, status->phql, status->phql_length);
					}
				} else {
					snprintf(status->syntax_error, status->syntax_error_len, "Syntax error, unexpected EOF, at the end of query");
				}
				status->syntax_error[status->syntax_error_len - 1] = '\0';
			}

			if (!token_found) {
				if (token_name) {
					efree(token_name);
				}
			}
		}
	} else {
		status->syntax_error_len = strlen("Syntax error, unexpected EOF");
		status->syntax_error = estrndup("Syntax error, unexpected EOF", status->syntax_error_len);
	}

	status->status = PHQL_PARSING_FAILED;
}

program ::= query_language(Q) . {
	ZVAL_ZVAL(&status->ret, &Q, 1, 1);
}

query_language(R) ::= select_statement(S) . {
	R = S;
}

query_language(R) ::= insert_statement(I) . {
	R = I;
}

query_language(R) ::= update_statement(U) . {
	R = U;
}

query_language(R) ::= delete_statement(D) . {
	R = D;
}

select_statement(R) ::= select_clause(S) where_clause(W) group_clause(G) having_clause(H) order_clause(O) select_limit_clause(L) for_update_clause(F) . {
	phql_ret_select_statement(&R, &S, &W, &O, &G, &H, &L, &F);
}

select_clause(R) ::= SELECT distinct_all(D) column_list(C) FROM associated_name_list(A) join_list_or_null(J) . {
	phql_ret_select_clause(&R, &D, &C, &A, &J);
}

distinct_all(R) ::= DISTINCT . {
	phql_ret_distinct_all(&R, 1);
}

distinct_all(R) ::= ALL . {
	phql_ret_distinct_all(&R, 0);
}

distinct_all(R) ::= . {
	ZVAL_UNDEF(&R);
}

column_list(R) ::= column_list(L) COMMA column_item(C) . {
	phql_ret_zval_list(&R, &L, &C);
}

column_list(R) ::= column_item(I) . {
	phql_ret_zval_list(&R, &I, NULL);
}

column_item(R) ::= TIMES . {
	phql_ret_column_item(&R, PHQL_T_STARALL, NULL, NULL, NULL);
}

column_item(R) ::= IDENTIFIER(I) DOT TIMES . {
	phql_ret_column_item(&R, PHQL_T_DOMAINALL, NULL, I, NULL);
}

column_item(R) ::= expr(E) AS IDENTIFIER(I) . {
	phql_ret_column_item(&R, PHQL_T_EXPR, &E, NULL, I);
}

column_item(R) ::= expr(E) IDENTIFIER(I) . {
	phql_ret_column_item(&R, PHQL_T_EXPR, &E, NULL, I);
}

column_item(R) ::= expr(E) . {
	phql_ret_column_item(&R, PHQL_T_EXPR, &E, NULL, NULL);
}

associated_name_list(R) ::= associated_name_list(L) COMMA associated_name(A) . {
	phql_ret_zval_list(&R, &L, &A);
}

associated_name_list(R) ::= associated_name(L) . {
	R = L;
}

join_list_or_null(R) ::= join_list(L) . {
	R = L;
}

join_list_or_null(R) ::= . {
	ZVAL_UNDEF(&R);
}

join_list(R) ::= join_list(L) join_item(I) . {
	phql_ret_zval_list(&R, &L, &I);
}

join_list(R) ::= join_item(I) . {
	R = I;
}

join_item(R) ::= join_clause(C) . {
	R = C;
}

/** Join + conditions + alias */
join_clause(R) ::= join_type(T) aliased_or_qualified_name(Q) join_associated_name(A) join_conditions(C) . {
	phql_ret_join_item(&R, &T, &Q, &A, &C);
}

join_associated_name(R) ::= AS IDENTIFIER(I) . {
	phql_ret_qualified_name(&R, NULL, NULL, I);
}

join_associated_name(R) ::= IDENTIFIER(I) . {
	phql_ret_qualified_name(&R, NULL, NULL, I);
}

join_associated_name(R) ::= . {
	ZVAL_UNDEF(&R);
}

join_type(R) ::= INNER JOIN . {
	phql_ret_join_type(&R, PHQL_T_INNERJOIN);
}

join_type(R) ::= CROSS JOIN . {
	phql_ret_join_type(&R, PHQL_T_CROSSJOIN);
}

join_type(R) ::= LEFT OUTER JOIN . {
	phql_ret_join_type(&R, PHQL_T_LEFTJOIN);
}

join_type(R) ::= LEFT JOIN . {
	phql_ret_join_type(&R, PHQL_T_LEFTJOIN);
}

join_type(R) ::= RIGHT OUTER JOIN . {
	phql_ret_join_type(&R, PHQL_T_RIGHTJOIN);
}

join_type(R) ::= RIGHT JOIN . {
	phql_ret_join_type(&R, PHQL_T_RIGHTJOIN);
}

join_type(R) ::= FULL OUTER JOIN . {
	phql_ret_join_type(&R, PHQL_T_FULLJOIN);
}

join_type(R) ::= FULL JOIN . {
	phql_ret_join_type(&R, PHQL_T_FULLJOIN);
}

join_type(R) ::= JOIN . {
	phql_ret_join_type(&R, PHQL_T_INNERJOIN);
}

join_conditions(R) ::= ON expr(E) . {
	R = E;
}

join_conditions(R) ::= . {
	ZVAL_UNDEF(&R);
}

/* Insert */
insert_statement(R) ::= insert_statement(Q) COMMA PARENTHESES_OPEN values_list(V) PARENTHESES_CLOSE . {
	phql_ret_insert_statement2(&R, &Q, &V);
}

insert_statement(R) ::= INSERT INTO aliased_or_qualified_name(Q) VALUES PARENTHESES_OPEN values_list(V) PARENTHESES_CLOSE . {
	phql_ret_insert_statement(&R, &Q, NULL, &V);
}

insert_statement(R) ::= INSERT INTO aliased_or_qualified_name(Q) PARENTHESES_OPEN field_list(F) PARENTHESES_CLOSE VALUES PARENTHESES_OPEN values_list(V) PARENTHESES_CLOSE . {
	phql_ret_insert_statement(&R, &Q, &F, &V);
}

values_list(R) ::= values_list(L) COMMA value_item(I) . {
	phql_ret_zval_list(&R, &L, &I);
}

values_list(R) ::= value_item(I) . {
	phql_ret_zval_list(&R, &I, NULL);
}

value_item(R) ::= expr(E) . {
	R = E;
}

field_list(R) ::= field_list(L) COMMA field_item(I) . {
	phql_ret_zval_list(&R, &L, &I);
}

field_list(R) ::= field_item(I) . {
	phql_ret_zval_list(&R, &I, NULL);
}

field_item(R) ::= IDENTIFIER(I) . {
	phql_ret_qualified_name(&R, NULL, NULL, I);
}

/* Update */

update_statement(R) ::= update_clause(U) where_clause(W) limit_clause(L) . {
	phql_ret_update_statement(&R, &U, &W, &L);
}

update_clause(R) ::= UPDATE associated_name(A) SET update_item_list(U) . {
	phql_ret_update_clause(&R, &A, &U);
}

update_item_list(R) ::= update_item_list(L) COMMA update_item(I) . {
	phql_ret_zval_list(&R, &L, &I);
}

update_item_list(R) ::= update_item(I) . {
	R = I;
}

update_item(R) ::= qualified_name(Q) EQUALS new_value(N) . {
	phql_ret_update_item(&R, &Q, &N);
}

new_value(R) ::= expr(E) . {
	R = E;
}

/* Delete */

delete_statement(R) ::= delete_clause(D) where_clause(W) limit_clause(L) . {
	phql_ret_delete_statement(&R, &D, &W, &L);
}

delete_clause(R) ::= DELETE FROM associated_name(A) . {
	phql_ret_delete_clause(&R, &A);
}

associated_name(R) ::= aliased_or_qualified_name(Q) AS IDENTIFIER(I) index_hints_or_null(H) . {
	phql_ret_assoc_name(&R, &Q, I, &H);
}

associated_name(R) ::= aliased_or_qualified_name(Q) IDENTIFIER(I) index_hints_or_null(H) . {
	phql_ret_assoc_name(&R, &Q, I, &H);
}

associated_name(R) ::= aliased_or_qualified_name(Q) index_hints_or_null(H) . {
	phql_ret_assoc_name(&R, &Q, NULL, &H);
}

aliased_or_qualified_name(R) ::= qualified_name(Q) . {
	R = Q;
}

index_hints_or_null(R) ::= index_hints(L) . {
	R = L;
}

index_hints_or_null(R) ::= . {
	ZVAL_UNDEF(&R);
}

index_hints(R) ::= index_hints(L) index_hint(I) . {
	phql_ret_index_list(&R, &L, &I);
}

index_hints(R) ::= index_hint(I) . {
	phql_ret_index_list(&R, NULL, &I);
}

index_hint(R) ::= IGNORE INDEX PARENTHESES_OPEN field_list(F) PARENTHESES_CLOSE . {
	phql_ret_index_type(&R, PHQL_T_USE, &F);
}

index_hint(R) ::= USE INDEX PARENTHESES_OPEN field_list(F) PARENTHESES_CLOSE . {
	phql_ret_index_type(&R, PHQL_T_USE, &F);
}

index_hint(R) ::= FORCE INDEX PARENTHESES_OPEN field_list(F) PARENTHESES_CLOSE  . {
	phql_ret_index_type(&R, PHQL_T_FORCE, &F);
}

where_clause(R) ::= WHERE expr(E) . {
	R = E;
}

where_clause(R) ::= . {
	ZVAL_UNDEF(&R);
}

order_clause(R) ::= ORDER BY order_list(O) . {
	R = O;
}

order_clause(R) ::= . {
	ZVAL_UNDEF(&R);
}

order_list(R) ::= order_list(L) COMMA order_item(I) . {
	phql_ret_zval_list(&R, &L, &I);
}

order_list(R) ::= order_item(I) . {
	R = I;
}

order_item(R) ::= expr(O) . {
	phql_ret_order_item(&R, &O, 0);
}

order_item(R) ::= expr(O) ASC . {
	phql_ret_order_item(&R, &O, PHQL_T_ASC);
}

order_item(R) ::= expr(O) DESC . {
	phql_ret_order_item(&R, &O, PHQL_T_DESC);
}

group_clause(R) ::= GROUP BY group_list(G) . {
	R = G;
}

group_clause(R) ::= . {
	ZVAL_UNDEF(&R);
}

group_list(R) ::= group_list(L) COMMA group_item(I) . {
	phql_ret_zval_list(&R, &L, &I);
}

group_list(R) ::= group_item(I) . {
	R = I;
}

group_item(R) ::= expr(E) . {
	R = E;
}

having_clause(R) ::= HAVING expr(E) . {
	R = E;
}

having_clause(R) ::= . {
	ZVAL_UNDEF(&R);
}

for_update_clause(R) ::= FOR UPDATE . {
	phql_ret_for_update_clause(&R);
}

for_update_clause(R) ::= . {
	ZVAL_UNDEF(&R);
}

select_limit_clause(R) ::= LIMIT integer_or_placeholder(I) . {
	phql_ret_limit_clause(&R, &I, NULL);
}

select_limit_clause(R) ::= LIMIT integer_or_placeholder(O) COMMA integer_or_placeholder(I). {
	phql_ret_limit_clause(&R, &I, &O);
}

select_limit_clause(R) ::= LIMIT integer_or_placeholder(I) OFFSET integer_or_placeholder(O). {
	phql_ret_limit_clause(&R, &I, &O);
}

select_limit_clause(R) ::= . {
	ZVAL_UNDEF(&R);
}

limit_clause(R) ::= LIMIT integer_or_placeholder(I) . {
	phql_ret_limit_clause(&R, &I, NULL);
}

limit_clause(R) ::= . {
	ZVAL_UNDEF(&R);
}

integer_or_placeholder(R) ::= INTEGER(I) . {
	phql_ret_literal_zval(&R, PHQL_T_INTEGER, I);
}

integer_or_placeholder(R) ::= HINTEGER(I) . {
	phql_ret_literal_zval(&R, PHQL_T_HINTEGER, I);
}

integer_or_placeholder(R) ::= NPLACEHOLDER(P) . {
	phql_ret_placeholder_zval(&R, PHQL_T_NPLACEHOLDER, P);
}

integer_or_placeholder(R) ::= SPLACEHOLDER(P) . {
	phql_ret_placeholder_zval(&R, PHQL_T_SPLACEHOLDER, P);
}

expr(R) ::= MINUS expr(E) . {
	phql_ret_expr(&R, PHQL_T_MINUS, NULL, &E);
}

expr(R) ::= expr(O1) MINUS expr(O2) . {
	phql_ret_expr(&R, PHQL_T_SUB, &O1, &O2);
}

expr(R) ::= expr(O1) PLUS expr(O2) . {
	phql_ret_expr(&R, PHQL_T_ADD, &O1, &O2);
}

expr(R) ::= expr(O1) TIMES expr(O2) . {
	phql_ret_expr(&R, PHQL_T_MUL, &O1, &O2);
}

expr(R) ::= expr(O1) DIVIDE expr(O2) . {
	phql_ret_expr(&R, PHQL_T_DIV, &O1, &O2);
}

expr(R) ::= expr(O1) MOD expr(O2) . {
	phql_ret_expr(&R, PHQL_T_MOD, &O1, &O2);
}

expr(R) ::= expr(O1) AND expr(O2) . {
	phql_ret_expr(&R, PHQL_T_AND, &O1, &O2);
}

expr(R) ::= expr(O1) OR expr(O2) . {
	phql_ret_expr(&R, PHQL_T_OR, &O1, &O2);
}

expr(R) ::= expr(O1) BITWISE_AND expr(O2) . {
	phql_ret_expr(&R, PHQL_T_BITWISE_AND, &O1, &O2);
}

expr(R) ::= expr(O1) BITWISE_OR expr(O2) . {
	phql_ret_expr(&R, PHQL_T_BITWISE_OR, &O1, &O2);
}

expr(R) ::= expr(O1) BITWISE_XOR expr(O2) . {
	phql_ret_expr(&R, PHQL_T_BITWISE_XOR, &O1, &O2);
}

expr(R) ::= expr(O1) EQUALS expr(O2) . {
	phql_ret_expr(&R, PHQL_T_EQUALS, &O1, &O2);
}

expr(R) ::= expr(O1) NOTEQUALS expr(O2) . {
	phql_ret_expr(&R, PHQL_T_NOTEQUALS, &O1, &O2);
}

expr(R) ::= expr(O1) LESS expr(O2) . {
	phql_ret_expr(&R, PHQL_T_LESS, &O1, &O2);
}

expr(R) ::= expr(O1) GREATER expr(O2) . {
	phql_ret_expr(&R, PHQL_T_GREATER, &O1, &O2);
}

expr(R) ::= expr(O1) GREATEREQUAL expr(O2) . {
	phql_ret_expr(&R, PHQL_T_GREATEREQUAL, &O1, &O2);
}

expr(R) ::= expr(O1) TS_MATCHES expr(O2) . {
	phql_ret_expr(&R, PHQL_T_TS_MATCHES, &O1, &O2);
}

expr(R) ::= expr(O1) TS_OR expr(O2) . {
	phql_ret_expr(&R, PHQL_T_TS_OR, &O1, &O2);
}

expr(R) ::= expr(O1) TS_AND expr(O2) . {
	phql_ret_expr(&R, PHQL_T_TS_AND, &O1, &O2);
}

expr(R) ::= expr(O1) TS_NEGATE expr(O2) . {
	phql_ret_expr(&R, PHQL_T_TS_NEGATE, &O1, &O2);
}

expr(R) ::= expr(O1) TS_CONTAINS_ANOTHER expr(O2) . {
	phql_ret_expr(&R, PHQL_T_TS_CONTAINS_ANOTHER, &O1, &O2);
}

expr(R) ::= expr(O1) TS_CONTAINS_IN expr(O2) . {
	phql_ret_expr(&R, PHQL_T_TS_CONTAINS_IN, &O1, &O2);
}

expr(R) ::= expr(O1) LESSEQUAL expr(O2) . {
	phql_ret_expr(&R, PHQL_T_LESSEQUAL, &O1, &O2);
}

expr(R) ::= expr(O1) LIKE expr(O2) . {
	phql_ret_expr(&R, PHQL_T_LIKE, &O1, &O2);
}

expr(R) ::= expr(O1) NOT LIKE expr(O2) . {
	phql_ret_expr(&R, PHQL_T_NLIKE, &O1, &O2);
}

expr(R) ::= expr(O1) ILIKE expr(O2) . {
	phql_ret_expr(&R, PHQL_T_ILIKE, &O1, &O2);
}

expr(R) ::= expr(O1) NOT ILIKE expr(O2) . {
	phql_ret_expr(&R, PHQL_T_NILIKE, &O1, &O2);
}

expr(R) ::= expr(E) IN PARENTHESES_OPEN argument_list(L) PARENTHESES_CLOSE . {
	phql_ret_expr(&R, PHQL_T_IN, &E, &L);
}

expr(R) ::= expr(E) NOT IN PARENTHESES_OPEN argument_list(L) PARENTHESES_CLOSE . {
	phql_ret_expr(&R, PHQL_T_NOTIN, &E, &L);
}

expr(R) ::= PARENTHESES_OPEN select_statement(S) PARENTHESES_CLOSE . {
	phql_ret_expr(&R, PHQL_T_SUBQUERY, &S, NULL);
}

expr(R) ::= expr(E) IN PARENTHESES_OPEN select_statement(S) PARENTHESES_CLOSE . {
	phql_ret_expr(&R, PHQL_T_IN, &E, &S);
}

expr(R) ::= expr(E) NOT IN PARENTHESES_OPEN select_statement(S) PARENTHESES_CLOSE . {
	phql_ret_expr(&R, PHQL_T_NOTIN, &E, &S);
}

expr(R) ::= EXISTS PARENTHESES_OPEN select_statement(S) PARENTHESES_CLOSE . {
	phql_ret_expr(&R, PHQL_T_EXISTS, NULL, &S);
}

expr(R) ::= expr(O1) AGAINST expr(O2) . {
	phql_ret_expr(&R, PHQL_T_AGAINST, &O1, &O2);
}

expr(R) ::= CAST PARENTHESES_OPEN expr(E) AS IDENTIFIER(I) PARENTHESES_CLOSE . {
	{
		zval qualified;
		phql_ret_raw_qualified_name(&qualified, I, NULL);
		phql_ret_expr(&R, PHQL_T_CAST, &E, &qualified);
	}
}

expr(R) ::= CONVERT PARENTHESES_OPEN expr(E) USING IDENTIFIER(I) PARENTHESES_CLOSE . {
	{
		zval qualified;
		phql_ret_raw_qualified_name(&qualified, I, NULL);
		phql_ret_expr(&R, PHQL_T_CONVERT, &E, &qualified);
	}
}

expr(R) ::= CASE expr(E) when_clauses(W) END . {
	phql_ret_expr(&R, PHQL_T_CASE, &E, &W);
}

when_clauses(R) ::= when_clauses(L) when_clause(W) . {
	phql_ret_zval_list(&R, &L, &W);
}

when_clauses(R) ::= when_clause(W) . {
	phql_ret_zval_list(&R, &W, NULL);
}

when_clause(R) ::= WHEN expr(E) THEN expr(T) . {
	phql_ret_expr(&R, PHQL_T_WHEN, &E, &T);
}

when_clause(R) ::= ELSE expr(E) . {
	phql_ret_expr(&R, PHQL_T_ELSE, &E, NULL);
}

expr(R) ::= function_call(F) . {
	R = F;
}

function_call(R) ::= IDENTIFIER(I) PARENTHESES_OPEN distinct_or_null(D) argument_list_or_null(L) PARENTHESES_CLOSE . {
	phql_ret_func_call(&R, I, &L, &D);
}

distinct_or_null(R) ::= DISTINCT . {
	phql_ret_distinct(&R);
}

distinct_or_null(R) ::=  . {
	ZVAL_UNDEF(&R);
}

argument_list_or_null(R) ::= argument_list(L) . {
	R = L;
}

argument_list_or_null(R) ::= . {
	ZVAL_UNDEF(&R);
}

argument_list(R) ::= argument_list(L) COMMA argument_item(I) . {
	phql_ret_zval_list(&R, &L, &I);
}

argument_list(R) ::= argument_item(I) . {
	phql_ret_zval_list(&R, &I, NULL);
}

argument_item(R) ::= TIMES . {
	phql_ret_column_item(&R, PHQL_T_STARALL, NULL, NULL, NULL);
}

argument_item(R) ::= expr(E) . {
	R = E;
}

expr(R) ::= expr(E) IS NULL . {
	phql_ret_expr(&R, PHQL_T_ISNULL, &E, NULL);
}

expr(R) ::= expr(E) IS NOT NULL . {
	phql_ret_expr(&R, PHQL_T_ISNOTNULL, &E, NULL);
}

expr(R) ::= expr(E) BETWEEN expr(L) . {
	phql_ret_expr(&R, PHQL_T_BETWEEN, &E, &L);
}

expr(R) ::= expr(E) DOUBLECOLON expr(L) . {
	phql_ret_expr(&R, PHQL_T_DOUBLECOLON, &E, &L);
}

expr(R) ::= NOT expr(E) . {
	phql_ret_expr(&R, PHQL_T_NOT, NULL, &E);
}

expr(R) ::= BITWISE_NOT expr(E) . {
	phql_ret_expr(&R, PHQL_T_BITWISE_NOT, NULL, &E);
}

expr(R) ::= PARENTHESES_OPEN expr(E) PARENTHESES_CLOSE . {
	phql_ret_expr(&R, PHQL_T_ENCLOSED, &E, NULL);
}

expr(R) ::= qualified_name(Q) . {
	R = Q;
}

expr(R) ::= INTEGER(I) . {
	phql_ret_literal_zval(&R, PHQL_T_INTEGER, I);
}

expr(R) ::= HINTEGER(I) . {
	phql_ret_literal_zval(&R, PHQL_T_HINTEGER, I);
}

expr(R) ::= STRING(S) . {
	phql_ret_literal_zval(&R, PHQL_T_STRING, S);
}

expr(R) ::= DOUBLE(D) . {
	phql_ret_literal_zval(&R, PHQL_T_DOUBLE, D);
}

expr(R) ::= NULL . {
	phql_ret_literal_zval(&R, PHQL_T_NULL, NULL);
}

expr(R) ::= TRUE . {
	phql_ret_literal_zval(&R, PHQL_T_TRUE, NULL);
}

expr(R) ::= FALSE . {
	phql_ret_literal_zval(&R, PHQL_T_FALSE, NULL);
}

/* ?0 */
expr(R) ::= NPLACEHOLDER(P) . {
	phql_ret_placeholder_zval(&R, PHQL_T_NPLACEHOLDER, P);
}

/* :placeholder: */
expr(R) ::= SPLACEHOLDER(P) . {
	phql_ret_placeholder_zval(&R, PHQL_T_SPLACEHOLDER, P);
}

/* {placeholder} */
expr(R) ::= BPLACEHOLDER(P) . {
	phql_ret_placeholder_zval(&R, PHQL_T_BPLACEHOLDER, P);
}

qualified_name(R) ::= IDENTIFIER(A) COLON IDENTIFIER(B) DOT IDENTIFIER(C) . {
	phql_ret_qualified_name(&R, A, B, C);
}

qualified_name(R) ::= IDENTIFIER(A) COLON IDENTIFIER(B) . {
	phql_ret_qualified_name(&R, A, NULL, B);
}

qualified_name(R) ::= IDENTIFIER(A) DOT IDENTIFIER(B) . {
	phql_ret_qualified_name(&R, NULL, A, B);
}

qualified_name(R) ::= IDENTIFIER(A) . {
	phql_ret_qualified_name(&R, NULL, NULL, A);
}
