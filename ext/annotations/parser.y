
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

%token_prefix PHANNOT_
%token_type {phannot_parser_token*}
%default_type {zval}
%default_destructor {
	if (status) {
		// TODO:
	}
	if (&$$) {
		zval_ptr_dtor(&$$);
	}
}
%extra_argument {phannot_parser_status *status}
%name phannot_

%left COMMA .

%include {

#include "php_phalcon.h"

#include <Zend/zend_smart_str.h>
#include <main/spprintf.h>

#include "annotations/parser.h"
#include "annotations/scanner.h"
#include "annotations/annot.h"
#include "annotations/exception.h"

#include "kernel/main.h"
#include "kernel/exception.h"

#include "interned-strings.h"

#define phannot_add_assoc_stringl(var, index, str, len) add_assoc_stringl(var, index, str, len);
#define phannot_add_assoc_string(var, index, str) add_assoc_string(var, index, str);

static void phannot_ret_literal_zval(zval *ret, int type, phannot_parser_token *T)
{
	array_init(ret);

	add_assoc_long(ret, ISV(type), type);
	if (T) {
		phannot_add_assoc_stringl(ret, "value", T->token, T->token_len);
		efree(T->token);
		efree(T);
	}
}

static void phannot_ret_array(zval *ret, zval *items)
{
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHANNOT_T_ARRAY);

	if (items) {
		add_assoc_zval(ret, ISV(items), items);
	}
}

static void phannot_ret_zval_list(zval *ret, zval *list_left, zval *right_list)
{
	HashTable *list;

	array_init(ret);

	if (list_left) {

		list = Z_ARRVAL_P(list_left);
		if (zend_hash_index_exists(list, 0)) {
            {
                zval *item;
                ZEND_HASH_FOREACH_VAL(list, item) {

                    Z_TRY_ADDREF_P(item);
                    add_next_index_zval(ret, item);

                } ZEND_HASH_FOREACH_END();
            }
            zval_dtor(list_left);
		} else {
			add_next_index_zval(ret, list_left);
		}
	}

	add_next_index_zval(ret, right_list);
}

static void phannot_ret_named_item(zval *ret, phannot_parser_token *name, zval *expr)
{
	array_init(ret);

	add_assoc_zval(ret, ISV(expr), expr);
	if (name != NULL) {
		phannot_add_assoc_stringl(ret, "name", name->token, name->token_len);
        efree(name->token);
		efree(name);
	}
}

static void phannot_ret_annotation(zval *ret, phannot_parser_token *name, zval *arguments, phannot_scanner_state *state)
{
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHANNOT_T_ANNOTATION);

	if (name) {
		phannot_add_assoc_stringl(ret, ISV(name), name->token, name->token_len);
        efree(name->token);
		efree(name);
	}

	if (arguments) {
		add_assoc_zval(ret, ISV(arguments), arguments);
	}

	phannot_add_assoc_string(ret, ISV(file), (char*) state->active_file);
	add_assoc_long(ret, "line", state->active_line);
}

}

%syntax_error {
	if (status->scanner_state->start_length) {
		char *token_name = NULL;
		const phannot_token_names *tokens = phannot_tokens;
		uint active_token = status->scanner_state->active_token;
		uint near_length = status->scanner_state->start_length;

		if (active_token) {
			do {
				if (tokens->code == active_token) {
					token_name = tokens->name;
					break;
				}
				++tokens;
			} while (tokens[0].code != 0);
		}

		if (!token_name) {
			token_name  = "UNKNOWN";
		}

		if (near_length > 0) {
			if (status->token->value) {
				spprintf(&status->syntax_error, 0, "Syntax error, unexpected token %s(%s), near to '%s' in %s on line %d", token_name, status->token->value, status->scanner_state->start, status->scanner_state->active_file, status->scanner_state->active_line);
			} else {
				spprintf(&status->syntax_error, 0, "Syntax error, unexpected token %s, near to '%s' in %s on line %d", token_name, status->scanner_state->start, status->scanner_state->active_file, status->scanner_state->active_line);
			}
		} else {
			if (active_token != PHANNOT_T_IGNORE) {
				if (status->token->value) {
					spprintf(&status->syntax_error, 0, "Syntax error, unexpected token %s(%s), at the end of docblock in %s on line %d", token_name, status->token->value, status->scanner_state->active_file, status->scanner_state->active_line);
				} else {
					spprintf(&status->syntax_error, 0, "Syntax error, unexpected token %s, at the end of docblock in %s on line %d", token_name, status->scanner_state->active_file, status->scanner_state->active_line);
				}
			} else {
				spprintf(&status->syntax_error, 0, "Syntax error, unexpected EOF, at the end of docblock in %s on line %d", status->scanner_state->active_file, status->scanner_state->active_line);
			}
		}
	} else {
		spprintf(&status->syntax_error, 0, "Syntax error, unexpected EOF in %s", status->scanner_state->active_file);
	}

	status->status = PHANNOT_PARSING_FAILED;
}

%token_destructor {
	if ($$) {
		if ($$->free_flag) {
			efree($$->token);
		}
		efree($$);
	}
}

program ::= annotation_language(Q) . {
	ZVAL_ZVAL(&status->ret, &Q, 1, 1);
}

%destructor annotation_language {
    zval_ptr_dtor(&$$);
}

annotation_language(R) ::= annotation_list(L) . {
	R = L;
}

%destructor annotation_list {
    zval_ptr_dtor(&$$);
}

annotation_list(R) ::= annotation_list(L) annotation(S) . {
	phannot_ret_zval_list(&R, &L, &S);
}

annotation_list(R) ::= annotation(S) . {
	phannot_ret_zval_list(&R, NULL, &S);
}

%destructor annotation {
    zval_ptr_dtor(&$$);
}

annotation(R) ::= AT IDENTIFIER(I) PARENTHESES_OPEN argument_list(L) PARENTHESES_CLOSE . {
	phannot_ret_annotation(&R, I, &L, status->scanner_state);
}

annotation(R) ::= AT IDENTIFIER(I) PARENTHESES_OPEN PARENTHESES_CLOSE . {
	phannot_ret_annotation(&R, I, NULL, status->scanner_state);
}

annotation(R) ::= AT IDENTIFIER(I) . {
	phannot_ret_annotation(&R, I, NULL, status->scanner_state);
}

%destructor argument_list {
    zval_ptr_dtor(&$$);
}

argument_list(R) ::= argument_list(L) COMMA argument_item(I) . {
	phannot_ret_zval_list(&R, &L, &I);
}

argument_list(R) ::= argument_item(I) . {
	phannot_ret_zval_list(&R, NULL, &I);
}

%destructor argument_item {
    zval_ptr_dtor(&$$);
}

argument_item(R) ::= expr(E) . {
	phannot_ret_named_item(&R, NULL, &E);
}

argument_item(R) ::= STRING(S) EQUALS expr(E) . {
	phannot_ret_named_item(&R, S, &E);
}

argument_item(R) ::= STRING(S) COLON expr(E) . {
	phannot_ret_named_item(&R, S, &E);
}

argument_item(R) ::= IDENTIFIER(I) EQUALS expr(E) . {
	phannot_ret_named_item(&R, I, &E);
}

argument_item(R) ::= IDENTIFIER(I) COLON expr(E) . {
	phannot_ret_named_item(&R, I, &E);
}

%destructor expr {
    zval_ptr_dtor(&$$);
}

expr(R) ::= annotation(S) . {
	R = S;
}

expr(R) ::= array(A) . {
	R = A;
}

expr(R) ::= IDENTIFIER(I) . {
	phannot_ret_literal_zval(&R, PHANNOT_T_IDENTIFIER, I);
}

expr(R) ::= INTEGER(I) . {
	phannot_ret_literal_zval(&R, PHANNOT_T_INTEGER, I);
}

expr(R) ::= STRING(S) . {
	phannot_ret_literal_zval(&R, PHANNOT_T_STRING, S);
}

expr(R) ::= DOUBLE(D) . {
	phannot_ret_literal_zval(&R, PHANNOT_T_DOUBLE, D);
}

expr(R) ::= NULL . {
	phannot_ret_literal_zval(&R, PHANNOT_T_NULL, NULL);
}

expr(R) ::= FALSE . {
	phannot_ret_literal_zval(&R, PHANNOT_T_FALSE, NULL);
}

expr(R) ::= TRUE . {
	phannot_ret_literal_zval(&R, PHANNOT_T_TRUE, NULL);
}

array(R) ::= BRACKET_OPEN argument_list(A) BRACKET_CLOSE . {
	phannot_ret_array(&R, &A);
}

array(R) ::= SBRACKET_OPEN argument_list(A) SBRACKET_CLOSE . {
	phannot_ret_array(&R, &A);
}
