/*
** 2000-05-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Driver template for the LEMON parser generator.
**
** The "lemon" program processes an LALR(1) input grammar file, then uses
** this template to construct a parser.  The "lemon" program inserts text
** at each "%%" line.  Also, any "P-a-r-s-e" identifer prefix (without the
** interstitial "-" characters) contained in this template is changed into
** the value of the %name directive from the grammar.  Otherwise, the content
** of this template is copied straight through into the generate parser
** source file.
**
** The following is the concatenation of all %include directives from the
** input grammar file:
*/
#include <stdio.h>
/************ Begin %include sections from the grammar ************************/
/* #line 57 "parser.y" */


#include "php_phalcon.h"
#include "mvc/view/engine/volt/parser.h"
#include "mvc/view/engine/volt/scanner.h"
#include "mvc/view/engine/volt/volt.h"
#include "mvc/view/exception.h"

#include <Zend/zend_smart_str.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"


static zval *phvolt_ret_literal_zval(int type, phvolt_parser_token *T, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", type);
	if (T) {
		add_assoc_stringl(ret, "value", T->token, T->token_len);
		efree(T->token);
		efree(T);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_if_statement(zval *expr, zval *true_statements, zval *false_statements, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_IF);
	add_assoc_zval(ret, "expr", expr);
	efree(expr);

	if (true_statements) {
		add_assoc_zval(ret, "true_statements", true_statements);
		efree(true_statements);
	}
	if (false_statements) {
		add_assoc_zval(ret, "false_statements", false_statements);
		efree(false_statements);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_elseif_statement(zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_ELSEIF);
	add_assoc_zval(ret, "expr", expr);
	efree(expr);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_elsefor_statement(phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_ELSEFOR);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_for_statement(phvolt_parser_token *variable, phvolt_parser_token *key, zval *expr, zval *if_expr, zval *block_statements, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_FOR);

	add_assoc_stringl(ret, "variable", variable->token, variable->token_len);
	efree(variable->token);
	efree(variable);

	if (key) {
		add_assoc_stringl(ret, "key", key->token, key->token_len);
		efree(key->token);
		efree(key);
	}

	add_assoc_zval(ret, "expr", expr);
	efree(expr);

	if (if_expr) {
		add_assoc_zval(ret, "if_expr", if_expr);
		efree(if_expr);
	}

	add_assoc_zval(ret, "block_statements", block_statements);
	efree(block_statements);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_cache_statement(zval *expr, zval *lifetime, zval *block_statements, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);

	add_assoc_long(ret, "type", PHVOLT_T_CACHE);
	add_assoc_zval(ret, "expr", expr);

	if (lifetime) {
		add_assoc_zval(ret, "lifetime", lifetime);
		efree(lifetime);
	}
	add_assoc_zval(ret, "block_statements", block_statements);
	efree(block_statements);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_set_statement(zval *assignments)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 2);
	add_assoc_long(ret, "type", PHVOLT_T_SET);

	add_assoc_zval(ret, "assignments", assignments);
	efree(assignments);

	return ret;
}

static zval *phvolt_ret_set_assignment(phvolt_parser_token *variable, int operator, zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 5);

	add_assoc_stringl(ret, "variable", variable->token, variable->token_len);
	efree(variable->token);
	efree(variable);

	add_assoc_long(ret, "op", operator);
	add_assoc_zval(ret, "expr", expr);
	efree(expr);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_echo_statement(zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 4);
	add_assoc_long(ret, "type", PHVOLT_T_ECHO);
	add_assoc_zval(ret, "expr", expr);
	efree(expr);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_block_statement(phvolt_parser_token *name, zval *block_statements, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);

	add_assoc_long(ret, "type", PHVOLT_T_BLOCK);

	add_assoc_stringl(ret, "name", name->token, name->token_len);
	efree(name->token);
	efree(name);

	if (block_statements) {
		add_assoc_zval(ret, "block_statements", block_statements);
		efree(block_statements);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_macro_statement(phvolt_parser_token *macro_name, zval *parameters, zval *block_statements, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_MACRO);

	add_assoc_stringl(ret, "name", macro_name->token, macro_name->token_len);
	efree(macro_name->token);
	efree(macro_name);

	if (parameters) {
		add_assoc_zval(ret, "parameters", parameters);
		efree(parameters);
	}

	if (block_statements) {
		add_assoc_zval(ret, "block_statements", block_statements);
		efree(block_statements);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_macro_parameter(phvolt_parser_token *variable, zval *default_value, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 5);

	add_assoc_stringl(ret, "variable", variable->token, variable->token_len);
	efree(variable->token);
	efree(variable);

	if (default_value) {
		add_assoc_zval(ret, "default", default_value);
		efree(default_value);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_extends_statement(phvolt_parser_token *P, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 4);

	add_assoc_long(ret, "type", PHVOLT_T_EXTENDS);
	add_assoc_stringl(ret, "path", P->token, P->token_len);
	efree(P->token);
	efree(P);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_include_statement(zval *path, zval *params, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 4);

	add_assoc_long(ret, "type", PHVOLT_T_INCLUDE);
	add_assoc_zval(ret, "path", path);
	efree(path);

	if (params) {
		add_assoc_zval(ret, "params", params);
		efree(params);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_do_statement(zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 4);
	add_assoc_long(ret, "type", PHVOLT_T_DO);
	add_assoc_zval(ret, "expr", expr);
	efree(expr);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_return_statement(zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 4);
	add_assoc_long(ret, "type", PHVOLT_T_RETURN);
	add_assoc_zval(ret, "expr", expr);
	efree(expr);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_autoescape_statement(int enable, zval *block_statements, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 5);

	add_assoc_long(ret, "type", PHVOLT_T_AUTOESCAPE);
	add_assoc_long(ret, "enable", enable);
	add_assoc_zval(ret, "block_statements", block_statements);
	efree(block_statements);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_empty_statement(phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 3);
	add_assoc_long(ret, "type", PHVOLT_T_EMPTY_STATEMENT);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_break_statement(phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 3);
	add_assoc_long(ret, "type", PHVOLT_T_BREAK);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_continue_statement(phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 3);
	add_assoc_long(ret, "type", PHVOLT_T_CONTINUE);

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_zval_list(zval *list_left, zval *right_list)
{
	zval *ret, *item;

	PHALCON_ALLOC_ZVAL(ret);
	array_init(ret);

	if (list_left) {
		if (zend_hash_index_exists(Z_ARRVAL_P(list_left), 0)) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(list_left), item) {
				add_next_index_zval(ret, item);
			} ZEND_HASH_FOREACH_END();
		} else {
			add_next_index_zval(ret, list_left);
		}
		efree(list_left);
	}

	add_next_index_zval(ret, right_list);
	efree(right_list);

	return ret;
}

static zval *phvolt_ret_named_item(phvolt_parser_token *name, zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_zval(ret, "expr", expr);
	efree(expr);
	if (name != NULL) {
		add_assoc_stringl(ret, "name", name->token, name->token_len);
		efree(name->token);
		efree(name);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_expr(int type, zval *left, zval *right, zval *ternary, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", type);

	if (ternary) {
		add_assoc_zval(ret, "ternary", ternary);
		efree(ternary);
	}

	if (left) {
		add_assoc_zval(ret, "left", left);
		efree(left);
	}

	if (right) {
		add_assoc_zval(ret, "right", right);
		efree(right);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_slice(zval *left, zval *start, zval *end, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_SLICE);
	add_assoc_zval(ret, "left", left);
	efree(left);

	if (start != NULL) {
		add_assoc_zval(ret, "start", start);
		efree(start);
	}

	if (end != NULL) {
		add_assoc_zval(ret, "end", end);
		efree(end);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_func_call(zval *expr, zval *arguments, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_FCALL);
	add_assoc_zval(ret, "name", expr);
	efree(expr);

	if (arguments) {
		add_assoc_zval(ret, "arguments", arguments);
		efree(arguments);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_macro_call_statement(zval *expr, zval *arguments, zval *caller, phvolt_scanner_state *state)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_CALL);
	add_assoc_zval(ret, "name", expr);
	efree(expr);

	if (arguments) {
		add_assoc_zval(ret, "arguments", arguments);
		efree(arguments);
	}

	if (caller) {
		add_assoc_zval(ret, "caller", caller);
		efree(caller);
	}

	Z_TRY_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

/* #line 604 "parser.c" */
/**************** End of %include directives **********************************/
/* These constants specify the various numeric values for terminal symbols
** in a format understandable to "makeheaders".  This section is blank unless
** "lemon" is run with the "-m" command-line option.
***************** Begin makeheaders token definitions *************************/
/**************** End makeheaders token definitions ***************************/

/* The next sections is a series of control #defines.
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used to store the integer codes
**                       that represent terminal and non-terminal symbols.
**                       "unsigned char" is used if there are fewer than
**                       256 symbols.  Larger types otherwise.
**    YYNOCODE           is a number of type YYCODETYPE that is not used for
**                       any terminal or nonterminal symbol.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       (also known as: "terminal symbols") have fall-back
**                       values which should be used if the original symbol
**                       would not parse.  This permits keywords to sometimes
**                       be used as identifiers, for example.
**    YYACTIONTYPE       is the data type used for "action codes" - numbers
**                       that indicate what to do in response to the next
**                       token.
**    phvolt_TOKENTYPE     is the data type used for minor type for terminal
**                       symbols.  Background: A "minor type" is a semantic
**                       value associated with a terminal or non-terminal
**                       symbols.  For example, for an "ID" terminal symbol,
**                       the minor type might be the name of the identifier.
**                       Each non-terminal can have a different minor type.
**                       Terminal symbols all have the same minor type, though.
**                       This macros defines the minor type for terminal 
**                       symbols.
**    YYMINORTYPE        is the data type used for all minor types.
**                       This is typically a union of many types, one of
**                       which is phvolt_TOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    phvolt_ARG_SDECL     A static variable declaration for the %extra_argument
**    phvolt_ARG_PDECL     A parameter declaration for the %extra_argument
**    phvolt_ARG_STORE     Code to store %extra_argument into yypParser
**    phvolt_ARG_FETCH     Code to extract %extra_argument from yypParser
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_MIN_REDUCE      Maximum value for reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned char
#define YYNOCODE 121
#define YYACTIONTYPE unsigned short int
#define phvolt_TOKENTYPE phvolt_parser_token*
typedef union {
  int yyinit;
  phvolt_TOKENTYPE yy0;
  zval* yy168;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define phvolt_ARG_SDECL phvolt_parser_status *status;
#define phvolt_ARG_PDECL ,phvolt_parser_status *status
#define phvolt_ARG_FETCH phvolt_parser_status *status = yypParser->status
#define phvolt_ARG_STORE yypParser->status = status
#define YYNSTATE             227
#define YYNRULE              147
#define YY_MAX_SHIFT         226
#define YY_MIN_SHIFTREDUCE   335
#define YY_MAX_SHIFTREDUCE   481
#define YY_MIN_REDUCE        482
#define YY_MAX_REDUCE        628
#define YY_ERROR_ACTION      629
#define YY_ACCEPT_ACTION     630
#define YY_NO_ACTION         631
/************* End control #defines *******************************************/

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N <= YY_MAX_SHIFT             Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   N between YY_MIN_SHIFTREDUCE       Shift to an arbitrary state then
**     and YY_MAX_SHIFTREDUCE           reduce by rule N-YY_MIN_SHIFTREDUCE.
**
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE

**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
**
*********** Begin parsing tables **********************************************/
#define YY_ACTTAB_COUNT (2130)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    66,   64,  161,   75,   79,   78,   37,   74,   38,   71,
 /*    10 */    70,   69,   68,   73,   72,   39,   40,   80,   82,   84,
 /*    20 */    77,   34,   76,  222,  450,  451,   33,   67,   36,    8,
 /*    30 */    55,   54,   53,   52,   51,  398,   82,   84,   77,   34,
 /*    40 */    76,  222,  450,  451,   33,   67,  397,  165,  380,  193,
 /*    50 */    66,   64,  382,   75,   79,   78,   37,   74,   38,   71,
 /*    60 */    70,   69,   68,   73,   72,   39,   40,   80,   82,   84,
 /*    70 */    77,   34,   76,  222,  450,  451,   33,   67,   41,    2,
 /*    80 */    66,   64,   94,   75,   79,   78,   37,   74,   38,   71,
 /*    90 */    70,   69,   68,   73,   72,   39,   40,   80,   82,   84,
 /*   100 */    77,   34,   76,  222,  450,  451,   33,   67,  456,  400,
 /*   110 */    66,   64,  369,   75,   79,   78,   37,   74,   38,   71,
 /*   120 */    70,   69,   68,   73,   72,   39,   40,   80,   82,   84,
 /*   130 */    77,   34,   76,  222,  450,  451,   33,   67,   43,   12,
 /*   140 */    66,   64,   47,   75,   79,   78,   37,   74,   38,   71,
 /*   150 */    70,   69,   68,   73,   72,   39,   40,   80,   82,   84,
 /*   160 */    77,   34,   76,  222,  450,  451,   33,   67,  217,    1,
 /*   170 */    66,   64,  219,   75,   79,   78,   37,   74,   38,   71,
 /*   180 */    70,   69,   68,   73,   72,   39,   40,   80,   82,   84,
 /*   190 */    77,   34,   76,  222,  450,  451,   33,   67,  184,    3,
 /*   200 */    66,   64,  379,   75,   79,   78,   37,   74,   38,   71,
 /*   210 */    70,   69,   68,   73,   72,   39,   40,   80,   82,   84,
 /*   220 */    77,   34,   76,  222,  450,  451,   33,   67,   64,  370,
 /*   230 */    75,   79,   78,   37,   74,   38,   71,   70,   69,   68,
 /*   240 */    73,   72,   39,   40,   80,   82,   84,   77,   34,   76,
 /*   250 */   222,  450,  451,   33,   67,  392,  168,  371,  362,   66,
 /*   260 */    64,  173,   75,   79,   78,   37,   74,   38,   71,   70,
 /*   270 */    69,   68,   73,   72,   39,   40,   80,   82,   84,   77,
 /*   280 */    34,   76,  222,  450,  451,   33,   67,  175,  403,   66,
 /*   290 */    64,  174,   75,   79,   78,   37,   74,   38,   71,   70,
 /*   300 */    69,   68,   73,   72,   39,   40,   80,   82,   84,   77,
 /*   310 */    34,   76,  222,  450,  451,   33,   67,  359,  402,   66,
 /*   320 */    64,  361,   75,   79,   78,   37,   74,   38,   71,   70,
 /*   330 */    69,   68,   73,   72,   39,   40,   80,   82,   84,   77,
 /*   340 */    34,   76,  222,  450,  451,   33,   67,   93,  401,   66,
 /*   350 */    64,   35,   75,   79,   78,   37,   74,   38,   71,   70,
 /*   360 */    69,   68,   73,   72,   39,   40,   80,   82,   84,   77,
 /*   370 */    34,   76,  222,  450,  451,   33,   67,   39,   40,   80,
 /*   380 */    82,   84,   77,   34,   76,  222,  450,  451,   33,   67,
 /*   390 */   199,  195,  186,  452,  182,   56,  207,   66,   64,   63,
 /*   400 */    75,   79,   78,   37,   74,   38,   71,   70,   69,   68,
 /*   410 */    73,   72,   39,   40,   80,   82,   84,   77,   34,   76,
 /*   420 */   222,  450,  451,   33,   67,   66,   64,   36,   75,   79,
 /*   430 */    78,   37,   74,   38,   71,   70,   69,   68,   73,   72,
 /*   440 */    39,   40,   80,   82,   84,   77,   34,   76,  222,  450,
 /*   450 */   451,   33,   67,   66,   64,  368,   75,   79,   78,   37,
 /*   460 */    74,   38,   71,   70,   69,   68,   73,   72,   39,   40,
 /*   470 */    80,   82,   84,   77,   34,   76,  222,  450,  451,   33,
 /*   480 */    67,  367,  363,   34,   76,  222,  450,  451,   33,   67,
 /*   490 */   383,  384,  385,  386,  387,  388,  222,  450,  451,   33,
 /*   500 */    67,   42,  457,  181,  366,  454,   66,   64,   90,   75,
 /*   510 */    79,   78,   37,   74,   38,   71,   70,   69,   68,   73,
 /*   520 */    72,   39,   40,   80,   82,   84,   77,   34,   76,  222,
 /*   530 */   450,  451,   33,   67,  378,   14,   66,   64,    4,   75,
 /*   540 */    79,   78,   37,   74,   38,   71,   70,   69,   68,   73,
 /*   550 */    72,   39,   40,   80,   82,   84,   77,   34,   76,  222,
 /*   560 */   450,  451,   30,   67,   66,   64,  390,   75,   79,   78,
 /*   570 */    37,   74,   38,   71,   70,   69,   68,   73,   72,   39,
 /*   580 */    40,   80,   82,   84,   77,   34,   76,  222,  450,  451,
 /*   590 */    33,   67,   75,   79,   78,   37,   74,   38,   71,   70,
 /*   600 */    69,   68,   73,   72,   39,   40,   80,   82,   84,   77,
 /*   610 */    34,   76,  222,  450,  451,   33,   67,   79,   78,   37,
 /*   620 */    74,   38,   71,   70,   69,   68,   73,   72,   39,   40,
 /*   630 */    80,   82,   84,   77,   34,   76,  222,  450,  451,   33,
 /*   640 */    67,  630,  226,   29,  338,  339,  340,  341,  342,  343,
 /*   650 */   344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
 /*   660 */   354,  355,  356,  357,   37,   74,   38,   71,   70,   69,
 /*   670 */    68,   73,   72,   39,   40,   80,   82,   84,   77,   34,
 /*   680 */    76,  222,  450,  451,   33,   67,   15,  338,  339,  340,
 /*   690 */   341,  342,  343,  344,  345,  346,  347,  348,  349,  350,
 /*   700 */   351,  352,  353,  354,  355,  356,  357,   16,  338,  339,
 /*   710 */   340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
 /*   720 */   350,  351,  352,  353,  354,  355,  356,  357,   17,  338,
 /*   730 */   339,  340,  341,  342,  343,  344,  345,  346,  347,  348,
 /*   740 */   349,  350,  351,  352,  353,  354,  355,  356,  357,   18,
 /*   750 */   338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
 /*   760 */   348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
 /*   770 */    19,  338,  339,  340,  341,  342,  343,  344,  345,  346,
 /*   780 */   347,  348,  349,  350,  351,  352,  353,  354,  355,  356,
 /*   790 */   357,   20,  338,  339,  340,  341,  342,  343,  344,  345,
 /*   800 */   346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
 /*   810 */   356,  357,   21,  338,  339,  340,  341,  342,  343,  344,
 /*   820 */   345,  346,  347,  348,  349,  350,  351,  352,  353,  354,
 /*   830 */   355,  356,  357,   22,  338,  339,  340,  341,  342,  343,
 /*   840 */   344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
 /*   850 */   354,  355,  356,  357,   23,  338,  339,  340,  341,  342,
 /*   860 */   343,  344,  345,  346,  347,  348,  349,  350,  351,  352,
 /*   870 */   353,  354,  355,  356,  357,   24,  338,  339,  340,  341,
 /*   880 */   342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
 /*   890 */   352,  353,  354,  355,  356,  357,   25,  338,  339,  340,
 /*   900 */   341,  342,  343,  344,  345,  346,  347,  348,  349,  350,
 /*   910 */   351,  352,  353,  354,  355,  356,  357,   26,  338,  339,
 /*   920 */   340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
 /*   930 */   350,  351,  352,  353,  354,  355,  356,  357,   27,  338,
 /*   940 */   339,  340,  341,  342,  343,  344,  345,  346,  347,  348,
 /*   950 */   349,  350,  351,  352,  353,  354,  355,  356,  357,   28,
 /*   960 */   338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
 /*   970 */   348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
 /*   980 */   337,  339,  340,  341,  342,  343,  344,  345,  346,  347,
 /*   990 */   348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
 /*  1000 */    81,   83,  187,   32,  188,   62,   35,  184,   61,  189,
 /*  1010 */    81,   83,  106,   32,  118,   62,  209,  475,   61,  394,
 /*  1020 */   167,  218,  396,  468,  117,  468,  472,  475,  476,  477,
 /*  1030 */   478,  479,  480,  481,  464,  468,  117,  148,  476,  477,
 /*  1040 */   478,  479,  480,  481,  171,    5,  465,  468,  468,  141,
 /*  1050 */   463,  469,  425,  427,  429,  431,  433,  435,  437,  405,
 /*  1060 */    31,  462,  424,  426,  428,  430,  432,  434,  436,    6,
 /*  1070 */    31,   86,  391,  177,  176,   85,  214,  213,  407,  117,
 /*  1080 */    88,  406,  118,  404,    7,  118,  211,  172,  170,  465,
 /*  1090 */   468,  149,  460,  468,  472,   50,  468,  471,  204,  205,
 /*  1100 */   399,   49,  468,  201,   48,   44,   46,   45,  166,  131,
 /*  1110 */   197,  196,   86,  391,  225,  224,   85,  214,  213,  395,
 /*  1120 */   408,   88,  154,  155,  200,  393,   98,  211,   95,   87,
 /*  1130 */   463,  389,   99,  468,  468,  100,   50,  468,    9,  468,
 /*  1140 */   205,  462,   49,  468,  201,   48,  468,   46,   45,  166,
 /*  1150 */   126,  197,  196,   86,  391,  178,   10,   85,  214,  213,
 /*  1160 */   101,  124,   88,  102,  103,   44,  377,  127,  211,   11,
 /*  1170 */   210,  468,  365,   96,  468,  468,   89,   50,  364,  169,
 /*  1180 */   408,  205,  610,   49,  468,  201,   48,  468,   46,   45,
 /*  1190 */   166,  128,  197,  196,   86,  391,  109,  609,   85,  214,
 /*  1200 */   213,  110,  179,   88,  111,  112,   44,  468,  129,  211,
 /*  1210 */   461,   91,  468,  459,  113,  468,  468,   44,   50,   59,
 /*  1220 */    60,  408,  205,   44,   49,  468,  201,   48,   65,   46,
 /*  1230 */    45,  166,  408,  197,  196,   86,  391,  360,  408,   85,
 /*  1240 */   214,  213,  114,  180,   88,   97,  482,   44,   13,  130,
 /*  1250 */   211,  358,  484,  468,  484,  151,  468,  484,  484,   50,
 /*  1260 */   484,  484,  408,  205,   44,   49,  468,  201,   48,  132,
 /*  1270 */    46,   45,  166,  133,  197,  196,   86,  391,  484,  408,
 /*  1280 */    85,  214,  213,  152,  183,   88,  484,  484,  115,  484,
 /*  1290 */   484,  211,  116,  484,  468,  484,  104,  484,  484,  468,
 /*  1300 */    50,  484,  484,  468,  205,   44,   49,  468,  201,   48,
 /*  1310 */   134,   46,   45,  166,  135,  197,  196,   86,  391,  484,
 /*  1320 */   408,   85,  214,  213,  162,   44,   88,  484,  484,   44,
 /*  1330 */   484,  120,  211,  484,  185,  468,  136,  105,  484,  484,
 /*  1340 */   408,   50,  468,  484,  408,  205,  137,   49,  468,  201,
 /*  1350 */    48,  484,   46,   45,  166,  138,  197,  196,   86,  391,
 /*  1360 */   484,  484,   85,  214,  213,  484,   44,   88,  484,  484,
 /*  1370 */    44,  484,  139,  211,  484,  484,  484,  484,  163,  484,
 /*  1380 */   484,  408,   50,  484,  484,  408,  205,  191,   49,  468,
 /*  1390 */   201,   48,   44,   46,   45,  166,  140,  197,  196,   86,
 /*  1400 */   391,  484,   44,   85,  214,  213,  484,  408,   88,  484,
 /*  1410 */   484,   44,  484,  125,  211,  484,  484,  408,  484,  484,
 /*  1420 */   484,  484,  119,   50,  484,  484,  408,  205,   44,   49,
 /*  1430 */   192,  201,   48,  468,   46,   45,  166,  446,  197,  196,
 /*  1440 */    86,  391,  484,  408,   85,  214,  213,  484,  468,   88,
 /*  1450 */   142,  484,   44,  484,  143,  211,  484,  484,  484,  484,
 /*  1460 */   144,  468,  484,  145,   50,  468,  484,  408,  205,   44,
 /*  1470 */    49,  468,  201,   48,  468,   46,   45,  166,  194,  197,
 /*  1480 */   196,   86,  391,  484,  408,   85,  214,  213,  484,  146,
 /*  1490 */    88,  484,  484,  147,  484,  150,  211,  121,  484,  164,
 /*  1500 */   468,  158,  484,  484,  468,   50,  468,  484,  468,  205,
 /*  1510 */   468,   49,  468,  201,   48,  484,   46,   45,  166,  198,
 /*  1520 */   197,  196,   86,  391,  484,  122,   85,  214,  213,  484,
 /*  1530 */   123,   88,  484,  484,  153,  484,  468,  211,  159,  484,
 /*  1540 */   157,  468,  484,  484,  484,  468,   50,  484,  484,  468,
 /*  1550 */   205,  468,   49,  202,  201,   48,  484,   46,   45,  166,
 /*  1560 */   160,  197,  196,   86,  391,  484,  484,   85,  214,  213,
 /*  1570 */   484,  468,   88,  156,  484,  107,  484,  108,  211,  484,
 /*  1580 */   484,  484,  484,  484,  468,  484,  468,   50,  468,  484,
 /*  1590 */   484,  205,  203,   49,  484,  201,   48,  484,   46,   45,
 /*  1600 */   166,  484,  197,  196,   86,  391,  484,  484,   85,  214,
 /*  1610 */   213,  484,  484,   88,  484,  484,  484,  484,  484,  211,
 /*  1620 */   484,  484,  484,  484,  484,  484,  484,  484,   50,  206,
 /*  1630 */   484,  484,  205,  484,   49,  484,  201,   48,  484,   46,
 /*  1640 */    45,  166,  484,  197,  196,   86,  391,  484,  484,   85,
 /*  1650 */   214,  213,  484,  484,   88,  484,  484,  484,  484,  484,
 /*  1660 */   211,  484,  208,  484,  484,  484,  484,  484,  484,   50,
 /*  1670 */   484,  484,  484,  205,  484,   49,  484,  201,   48,  484,
 /*  1680 */    46,   45,  166,  484,  197,  196,   86,  391,  484,  484,
 /*  1690 */    85,  214,  213,  484,  212,   88,  484,  484,  484,  484,
 /*  1700 */   484,  211,  484,  484,  484,  484,  484,  484,  484,  484,
 /*  1710 */    50,  484,  484,  484,  205,  484,   49,  484,  201,   48,
 /*  1720 */   484,   46,   45,  166,  484,  197,  196,   86,  391,  223,
 /*  1730 */   484,   85,  214,  213,  484,  484,   88,  484,  484,  484,
 /*  1740 */   484,  484,  211,  484,  484,  484,  484,  484,  484,  484,
 /*  1750 */   484,   50,  484,  484,  484,  205,  484,   49,  484,  201,
 /*  1760 */    48,  484,   46,   45,  166,  484,  197,  196,   86,  391,
 /*  1770 */   484,  484,   85,  214,  213,  484,  484,   88,  484,  484,
 /*  1780 */   484,  484,  484,  211,  484,   81,   83,  484,   32,  484,
 /*  1790 */    62,  484,   50,   61,  484,  484,  205,  484,   49,  484,
 /*  1800 */   201,   48,  475,   46,   45,  166,  484,  197,  196,  484,
 /*  1810 */   484,  190,  484,  476,  220,  478,  479,  480,  481,  484,
 /*  1820 */    81,   83,  484,   32,  484,   62,   81,   83,   61,   32,
 /*  1830 */   484,   62,  484,  484,   61,  484,  484,  475,  484,  484,
 /*  1840 */   484,  484,  484,  475,  484,   31,  484,  484,  476,  221,
 /*  1850 */   478,  479,  480,  481,  476,  221,  478,  479,  480,  481,
 /*  1860 */   484,  484,  484,  484,  484,   81,   83,  484,   32,  484,
 /*  1870 */    62,  484,  484,   61,  484,  484,  484,  484,  484,  484,
 /*  1880 */    31,  455,  475,  484,  484,  453,   31,   92,  484,  484,
 /*  1890 */   484,  470,  484,  476,  220,  478,  479,  480,  481,  484,
 /*  1900 */   484,  484,  484,   81,   83,  484,   32,  484,   62,  484,
 /*  1910 */    58,   61,  484,   81,   83,  484,   32,  484,   62,  484,
 /*  1920 */   215,   61,  484,  484,  484,   31,  484,  484,  484,  484,
 /*  1930 */   475,  216,  477,  478,  479,  480,  481,  484,  484,  484,
 /*  1940 */   484,  476,  477,  478,  479,  480,  481,  484,  484,  484,
 /*  1950 */    57,  484,   81,   83,  484,   32,  484,   62,  484,  484,
 /*  1960 */    61,  484,  484,   31,  484,  484,  484,  484,  484,  475,
 /*  1970 */   484,  484,  484,   31,  484,  484,  484,  484,  484,  484,
 /*  1980 */   476,  477,  478,  479,  480,  481,  484,  484,  484,  484,
 /*  1990 */   484,   81,   83,  484,   32,  484,   62,  484,  484,   61,
 /*  2000 */   484,  484,  484,  484,  484,  484,  484,  484,  475,  484,
 /*  2010 */   484,  484,   31,  484,  484,  484,  484,  484,  484,  476,
 /*  2020 */   220,  478,  479,  480,  481,  484,  484,  484,  484,  484,
 /*  2030 */    81,   83,  484,   32,  484,   62,  484,  484,   61,  484,
 /*  2040 */   484,  484,  484,  484,  484,  484,  484,  475,  484,  484,
 /*  2050 */   484,   31,  484,  484,  484,  484,  484,  484,  476,  221,
 /*  2060 */   478,  479,  480,  481,  484,  484,  484,  484,  484,   81,
 /*  2070 */    83,  484,   32,  484,   62,  484,  484,   61,  484,  484,
 /*  2080 */   484,  484,  484,  484,  484,  484,  475,  484,  484,  484,
 /*  2090 */    31,  484,  484,  484,  484,  484,  484,  476,  477,  478,
 /*  2100 */   479,  480,  481,  484,  484,  484,  484,  484,  484,  484,
 /*  2110 */   484,  484,  484,  484,  484,  484,  484,  484,  484,  484,
 /*  2120 */   484,  484,  484,  484,  484,  484,  484,  484,  484,   31,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     3,    4,   38,    6,    7,    8,    9,   10,   11,   12,
 /*    10 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*    20 */    23,   24,   25,   26,   27,   28,   29,   30,    2,   32,
 /*    30 */    41,   42,   43,   44,   45,   38,   21,   22,   23,   24,
 /*    40 */    25,   26,   27,   28,   29,   30,   49,  110,  111,  114,
 /*    50 */     3,    4,  112,    6,    7,    8,    9,   10,   11,   12,
 /*    60 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*    70 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /*    80 */     3,    4,    2,    6,    7,    8,    9,   10,   11,   12,
 /*    90 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   100 */    23,   24,   25,   26,   27,   28,   29,   30,   82,   32,
 /*   110 */     3,    4,   32,    6,    7,    8,    9,   10,   11,   12,
 /*   120 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   130 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /*   140 */     3,    4,   65,    6,    7,    8,    9,   10,   11,   12,
 /*   150 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   160 */    23,   24,   25,   26,   27,   28,   29,   30,  116,   32,
 /*   170 */     3,    4,  116,    6,    7,    8,    9,   10,   11,   12,
 /*   180 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   190 */    23,   24,   25,   26,   27,   28,   29,   30,   38,   32,
 /*   200 */     3,    4,  111,    6,    7,    8,    9,   10,   11,   12,
 /*   210 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   220 */    23,   24,   25,   26,   27,   28,   29,   30,    4,  109,
 /*   230 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   240 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*   250 */    26,   27,   28,   29,   30,   58,  108,  109,   32,    3,
 /*   260 */     4,   33,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   270 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   280 */    24,   25,   26,   27,   28,   29,   30,   32,   32,    3,
 /*   290 */     4,    1,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   300 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   310 */    24,   25,   26,   27,   28,   29,   30,   32,   32,    3,
 /*   320 */     4,   32,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   330 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   340 */    24,   25,   26,   27,   28,   29,   30,    2,   32,    3,
 /*   350 */     4,    2,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   360 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   370 */    24,   25,   26,   27,   28,   29,   30,   18,   19,   20,
 /*   380 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /*   390 */    53,   54,   47,   47,    2,    3,   47,    3,    4,    5,
 /*   400 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   410 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*   420 */    26,   27,   28,   29,   30,    3,    4,    2,    6,    7,
 /*   430 */     8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
 /*   440 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*   450 */    28,   29,   30,    3,    4,   32,    6,    7,    8,    9,
 /*   460 */    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /*   470 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /*   480 */    30,   32,   32,   24,   25,   26,   27,   28,   29,   30,
 /*   490 */    49,   50,   51,   52,   53,   54,   26,   27,   28,   29,
 /*   500 */    30,    3,   80,   38,   32,   80,    3,    4,   41,    6,
 /*   510 */     7,    8,    9,   10,   11,   12,   13,   14,   15,   16,
 /*   520 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /*   530 */    27,   28,   29,   30,   32,   32,    3,    4,   32,    6,
 /*   540 */     7,    8,    9,   10,   11,   12,   13,   14,   15,   16,
 /*   550 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /*   560 */    27,   28,   29,   30,    3,    4,   32,    6,    7,    8,
 /*   570 */     9,   10,   11,   12,   13,   14,   15,   16,   17,   18,
 /*   580 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /*   590 */    29,   30,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   600 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   610 */    24,   25,   26,   27,   28,   29,   30,    7,    8,    9,
 /*   620 */    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /*   630 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /*   640 */    30,   84,   85,   86,   87,   88,   89,   90,   91,   92,
 /*   650 */    93,   94,   95,   96,   97,   98,   99,  100,  101,  102,
 /*   660 */   103,  104,  105,  106,    9,   10,   11,   12,   13,   14,
 /*   670 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*   680 */    25,   26,   27,   28,   29,   30,   86,   87,   88,   89,
 /*   690 */    90,   91,   92,   93,   94,   95,   96,   97,   98,   99,
 /*   700 */   100,  101,  102,  103,  104,  105,  106,   86,   87,   88,
 /*   710 */    89,   90,   91,   92,   93,   94,   95,   96,   97,   98,
 /*   720 */    99,  100,  101,  102,  103,  104,  105,  106,   86,   87,
 /*   730 */    88,   89,   90,   91,   92,   93,   94,   95,   96,   97,
 /*   740 */    98,   99,  100,  101,  102,  103,  104,  105,  106,   86,
 /*   750 */    87,   88,   89,   90,   91,   92,   93,   94,   95,   96,
 /*   760 */    97,   98,   99,  100,  101,  102,  103,  104,  105,  106,
 /*   770 */    86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
 /*   780 */    96,   97,   98,   99,  100,  101,  102,  103,  104,  105,
 /*   790 */   106,   86,   87,   88,   89,   90,   91,   92,   93,   94,
 /*   800 */    95,   96,   97,   98,   99,  100,  101,  102,  103,  104,
 /*   810 */   105,  106,   86,   87,   88,   89,   90,   91,   92,   93,
 /*   820 */    94,   95,   96,   97,   98,   99,  100,  101,  102,  103,
 /*   830 */   104,  105,  106,   86,   87,   88,   89,   90,   91,   92,
 /*   840 */    93,   94,   95,   96,   97,   98,   99,  100,  101,  102,
 /*   850 */   103,  104,  105,  106,   86,   87,   88,   89,   90,   91,
 /*   860 */    92,   93,   94,   95,   96,   97,   98,   99,  100,  101,
 /*   870 */   102,  103,  104,  105,  106,   86,   87,   88,   89,   90,
 /*   880 */    91,   92,   93,   94,   95,   96,   97,   98,   99,  100,
 /*   890 */   101,  102,  103,  104,  105,  106,   86,   87,   88,   89,
 /*   900 */    90,   91,   92,   93,   94,   95,   96,   97,   98,   99,
 /*   910 */   100,  101,  102,  103,  104,  105,  106,   86,   87,   88,
 /*   920 */    89,   90,   91,   92,   93,   94,   95,   96,   97,   98,
 /*   930 */    99,  100,  101,  102,  103,  104,  105,  106,   86,   87,
 /*   940 */    88,   89,   90,   91,   92,   93,   94,   95,   96,   97,
 /*   950 */    98,   99,  100,  101,  102,  103,  104,  105,  106,   86,
 /*   960 */    87,   88,   89,   90,   91,   92,   93,   94,   95,   96,
 /*   970 */    97,   98,   99,  100,  101,  102,  103,  104,  105,  106,
 /*   980 */    87,   88,   89,   90,   91,   92,   93,   94,   95,   96,
 /*   990 */    97,   98,   99,  100,  101,  102,  103,  104,  105,  106,
 /*  1000 */    21,   22,   56,   24,    1,   26,    2,   38,   29,   32,
 /*  1010 */    21,   22,  107,   24,  107,   26,   47,   38,   29,   32,
 /*  1020 */   113,  116,   32,  118,  107,  118,  119,   38,   49,   50,
 /*  1030 */    51,   52,   53,   54,  117,  118,  107,  107,   49,   50,
 /*  1040 */    51,   52,   53,   54,  115,   32,  117,  118,  118,    1,
 /*  1050 */    38,   47,   73,   74,   75,   76,   77,   78,   79,   32,
 /*  1060 */    81,   49,   73,   74,   75,   76,   77,   78,   79,   32,
 /*  1070 */    81,   31,   32,   33,   34,   35,   36,   37,   32,  107,
 /*  1080 */    40,   32,  107,   32,   32,  107,   46,  115,  113,  117,
 /*  1090 */   118,  107,   80,  118,  119,   55,  118,  119,   38,   59,
 /*  1100 */    32,   61,  118,   63,   64,   57,   66,   67,   68,    1,
 /*  1110 */    70,   71,   31,   32,   33,   34,   35,   36,   37,   32,
 /*  1120 */    72,   40,  107,  107,   50,   32,  107,   46,  107,   29,
 /*  1130 */    38,   32,  107,  118,  118,  107,   55,  118,   32,  118,
 /*  1140 */    59,   49,   61,  118,   63,   64,  118,   66,   67,   68,
 /*  1150 */     1,   70,   71,   31,   32,   33,   32,   35,   36,   37,
 /*  1160 */   107,    1,   40,  107,  107,   57,   32,    1,   46,   32,
 /*  1170 */    38,  118,   32,  107,  118,  118,  107,   55,   32,   38,
 /*  1180 */    72,   59,    5,   61,  118,   63,   64,  118,   66,   67,
 /*  1190 */    68,    1,   70,   71,   31,   32,  107,    5,   35,   36,
 /*  1200 */    37,  107,   39,   40,  107,  107,   57,  118,    1,   46,
 /*  1210 */    80,    5,  118,   80,  107,  118,  118,   57,   55,    5,
 /*  1220 */     5,   72,   59,   57,   61,  118,   63,   64,    3,   66,
 /*  1230 */    67,   68,   72,   70,   71,   31,   32,   32,   72,   35,
 /*  1240 */    36,   37,  107,   39,   40,  107,    0,   57,   32,    1,
 /*  1250 */    46,   32,  120,  118,  120,  107,  118,  120,  120,   55,
 /*  1260 */   120,  120,   72,   59,   57,   61,  118,   63,   64,    1,
 /*  1270 */    66,   67,   68,    1,   70,   71,   31,   32,  120,   72,
 /*  1280 */    35,   36,   37,  107,   39,   40,  120,  120,  107,  120,
 /*  1290 */   120,   46,  107,  120,  118,  120,  107,  120,  120,  118,
 /*  1300 */    55,  120,  120,  118,   59,   57,   61,  118,   63,   64,
 /*  1310 */     1,   66,   67,   68,    1,   70,   71,   31,   32,  120,
 /*  1320 */    72,   35,   36,   37,  107,   57,   40,  120,  120,   57,
 /*  1330 */   120,  107,   46,  120,   48,  118,    1,  107,  120,  120,
 /*  1340 */    72,   55,  118,  120,   72,   59,    1,   61,  118,   63,
 /*  1350 */    64,  120,   66,   67,   68,    1,   70,   71,   31,   32,
 /*  1360 */   120,  120,   35,   36,   37,  120,   57,   40,  120,  120,
 /*  1370 */    57,  120,    1,   46,  120,  120,  120,  120,  107,  120,
 /*  1380 */   120,   72,   55,  120,  120,   72,   59,   60,   61,  118,
 /*  1390 */    63,   64,   57,   66,   67,   68,    1,   70,   71,   31,
 /*  1400 */    32,  120,   57,   35,   36,   37,  120,   72,   40,  120,
 /*  1410 */   120,   57,  120,    1,   46,  120,  120,   72,  120,  120,
 /*  1420 */   120,  120,  107,   55,  120,  120,   72,   59,   57,   61,
 /*  1430 */    62,   63,   64,  118,   66,   67,   68,  107,   70,   71,
 /*  1440 */    31,   32,  120,   72,   35,   36,   37,  120,  118,   40,
 /*  1450 */   107,  120,   57,  120,  107,   46,  120,  120,  120,  120,
 /*  1460 */   107,  118,  120,  107,   55,  118,  120,   72,   59,   57,
 /*  1470 */    61,  118,   63,   64,  118,   66,   67,   68,   69,   70,
 /*  1480 */    71,   31,   32,  120,   72,   35,   36,   37,  120,  107,
 /*  1490 */    40,  120,  120,  107,  120,  107,   46,  107,  120,  107,
 /*  1500 */   118,  107,  120,  120,  118,   55,  118,  120,  118,   59,
 /*  1510 */   118,   61,  118,   63,   64,  120,   66,   67,   68,   69,
 /*  1520 */    70,   71,   31,   32,  120,  107,   35,   36,   37,  120,
 /*  1530 */   107,   40,  120,  120,  107,  120,  118,   46,  107,  120,
 /*  1540 */   107,  118,  120,  120,  120,  118,   55,  120,  120,  118,
 /*  1550 */    59,  118,   61,   62,   63,   64,  120,   66,   67,   68,
 /*  1560 */   107,   70,   71,   31,   32,  120,  120,   35,   36,   37,
 /*  1570 */   120,  118,   40,  107,  120,  107,  120,  107,   46,  120,
 /*  1580 */   120,  120,  120,  120,  118,  120,  118,   55,  118,  120,
 /*  1590 */   120,   59,   60,   61,  120,   63,   64,  120,   66,   67,
 /*  1600 */    68,  120,   70,   71,   31,   32,  120,  120,   35,   36,
 /*  1610 */    37,  120,  120,   40,  120,  120,  120,  120,  120,   46,
 /*  1620 */   120,  120,  120,  120,  120,  120,  120,  120,   55,   56,
 /*  1630 */   120,  120,   59,  120,   61,  120,   63,   64,  120,   66,
 /*  1640 */    67,   68,  120,   70,   71,   31,   32,  120,  120,   35,
 /*  1650 */    36,   37,  120,  120,   40,  120,  120,  120,  120,  120,
 /*  1660 */    46,  120,   48,  120,  120,  120,  120,  120,  120,   55,
 /*  1670 */   120,  120,  120,   59,  120,   61,  120,   63,   64,  120,
 /*  1680 */    66,   67,   68,  120,   70,   71,   31,   32,  120,  120,
 /*  1690 */    35,   36,   37,  120,   39,   40,  120,  120,  120,  120,
 /*  1700 */   120,   46,  120,  120,  120,  120,  120,  120,  120,  120,
 /*  1710 */    55,  120,  120,  120,   59,  120,   61,  120,   63,   64,
 /*  1720 */   120,   66,   67,   68,  120,   70,   71,   31,   32,   33,
 /*  1730 */   120,   35,   36,   37,  120,  120,   40,  120,  120,  120,
 /*  1740 */   120,  120,   46,  120,  120,  120,  120,  120,  120,  120,
 /*  1750 */   120,   55,  120,  120,  120,   59,  120,   61,  120,   63,
 /*  1760 */    64,  120,   66,   67,   68,  120,   70,   71,   31,   32,
 /*  1770 */   120,  120,   35,   36,   37,  120,  120,   40,  120,  120,
 /*  1780 */   120,  120,  120,   46,  120,   21,   22,  120,   24,  120,
 /*  1790 */    26,  120,   55,   29,  120,  120,   59,  120,   61,  120,
 /*  1800 */    63,   64,   38,   66,   67,   68,  120,   70,   71,  120,
 /*  1810 */   120,   47,  120,   49,   50,   51,   52,   53,   54,  120,
 /*  1820 */    21,   22,  120,   24,  120,   26,   21,   22,   29,   24,
 /*  1830 */   120,   26,  120,  120,   29,  120,  120,   38,  120,  120,
 /*  1840 */   120,  120,  120,   38,  120,   81,  120,  120,   49,   50,
 /*  1850 */    51,   52,   53,   54,   49,   50,   51,   52,   53,   54,
 /*  1860 */   120,  120,  120,  120,  120,   21,   22,  120,   24,  120,
 /*  1870 */    26,  120,  120,   29,  120,  120,  120,  120,  120,  120,
 /*  1880 */    81,   82,   38,  120,  120,   80,   81,    5,  120,  120,
 /*  1890 */   120,   47,  120,   49,   50,   51,   52,   53,   54,  120,
 /*  1900 */   120,  120,  120,   21,   22,  120,   24,  120,   26,  120,
 /*  1910 */    18,   29,  120,   21,   22,  120,   24,  120,   26,  120,
 /*  1920 */    38,   29,  120,  120,  120,   81,  120,  120,  120,  120,
 /*  1930 */    38,   49,   50,   51,   52,   53,   54,  120,  120,  120,
 /*  1940 */   120,   49,   50,   51,   52,   53,   54,  120,  120,  120,
 /*  1950 */    19,  120,   21,   22,  120,   24,  120,   26,  120,  120,
 /*  1960 */    29,  120,  120,   81,  120,  120,  120,  120,  120,   38,
 /*  1970 */   120,  120,  120,   81,  120,  120,  120,  120,  120,  120,
 /*  1980 */    49,   50,   51,   52,   53,   54,  120,  120,  120,  120,
 /*  1990 */   120,   21,   22,  120,   24,  120,   26,  120,  120,   29,
 /*  2000 */   120,  120,  120,  120,  120,  120,  120,  120,   38,  120,
 /*  2010 */   120,  120,   81,  120,  120,  120,  120,  120,  120,   49,
 /*  2020 */    50,   51,   52,   53,   54,  120,  120,  120,  120,  120,
 /*  2030 */    21,   22,  120,   24,  120,   26,  120,  120,   29,  120,
 /*  2040 */   120,  120,  120,  120,  120,  120,  120,   38,  120,  120,
 /*  2050 */   120,   81,  120,  120,  120,  120,  120,  120,   49,   50,
 /*  2060 */    51,   52,   53,   54,  120,  120,  120,  120,  120,   21,
 /*  2070 */    22,  120,   24,  120,   26,  120,  120,   29,  120,  120,
 /*  2080 */   120,  120,  120,  120,  120,  120,   38,  120,  120,  120,
 /*  2090 */    81,  120,  120,  120,  120,  120,  120,   49,   50,   51,
 /*  2100 */    52,   53,   54,  120,  120,  120,  120,  120,  120,  120,
 /*  2110 */   120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
 /*  2120 */   120,  120,  120,  120,  120,  120,  120,  120,  120,   81,
};
#define YY_SHIFT_USE_DFLT (-37)
#define YY_SHIFT_COUNT (226)
#define YY_SHIFT_MIN   (-36)
#define YY_SHIFT_MAX   (2048)
static const short yy_shift_ofst[] = {
 /*     0 */  1048, 1048, 1048, 1048, 1048, 1048, 1048, 1048, 1048, 1108,
 /*    10 */  1048, 1048, 1048, 1149, 1160, 1166, 1190, 1207, 1248, 1268,
 /*    20 */  1272, 1309, 1313, 1335, 1345, 1354, 1371, 1395, 1412, 1048,
 /*    30 */  1764, 1799, 1805, 1844, 1882, 1970, 2009,  979,  989, 1892,
 /*    40 */  1931, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048,
 /*    50 */  2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048,
 /*    60 */  2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048,
 /*    70 */  2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048,
 /*    80 */  2048, 2048, 2048, 2048, 2048, 2048, 2048,  969,  -36,   -3,
 /*    90 */   441, 1012, 1092,  160,  -36,   47,   77,  107,  137,  167,
 /*   100 */   197,  256,  286,  316,  346,  394,  422,  450,  503,  533,
 /*   110 */   561,  561,  561,  561,  561,  561,  561,  561,  561,  224,
 /*   120 */   586,  610,  655,  655, 1040, 1081, 1122, 1163, 1204, 1245,
 /*   130 */  1286, 1327, 1368, 1409, 1450, 1491, 1532, 1573, 1614, 1655,
 /*   140 */  1696, 1737,  359,  359,  359,  359,  359,  359,  359,  359,
 /*   150 */   359,   15,   15,   15,   15,   15,  459,  459,  459,  459,
 /*   160 */   459,  -11,  470,  470,  470,  345,  337,  349,   80,  392,
 /*   170 */  1004,   26,  425,  226,  228,  290,  255,  285,  289,  423,
 /*   180 */   449,  498,  465,  472,  467,  502,  506,  534,  946, 1003,
 /*   190 */   977,  987,  990, 1013, 1027, 1037, 1046, 1049, 1051, 1052,
 /*   200 */  1068, 1074, 1087, 1093, 1106, 1060, 1099, 1124, 1134, 1137,
 /*   210 */  1100, 1132, 1140, 1141, 1146, 1177, 1192, 1130, 1206, 1133,
 /*   220 */  1214, 1215, 1225, 1205, 1216, 1219, 1246,
};
#define YY_REDUCE_USE_DFLT (-66)
#define YY_REDUCE_COUNT (94)
#define YY_REDUCE_MIN   (-65)
#define YY_REDUCE_MAX   (1470)
static const short yy_reduce_ofst[] = {
 /*     0 */   557,  600,  621,  642,  663,  684,  705,  726,  747,  768,
 /*    10 */   789,  810,  831,  852,  873,  893,  893,  893,  893,  893,
 /*    20 */   893,  893,  893,  893,  893,  893,  893,  893,  893,  893,
 /*    30 */   907,  929,  972,  975,  905,  978,  917,  930,  984, 1015,
 /*    40 */  1016, 1019, 1021, 1025, 1028, 1053, 1056, 1057, 1066, 1069,
 /*    50 */  1089, 1094, 1097, 1098, 1107, 1135, 1138, 1148, 1176, 1181,
 /*    60 */  1185, 1189, 1217, 1224, 1230, 1271, 1315, 1330, 1343, 1347,
 /*    70 */  1353, 1356, 1382, 1386, 1388, 1390, 1392, 1394, 1418, 1423,
 /*    80 */  1427, 1431, 1433, 1453, 1466, 1468, 1470,  -63,  148,  -65,
 /*    90 */   -60,   52,   56,   91,  120,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*    10 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*    20 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  483,
 /*    30 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*    40 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*    50 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*    60 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*    70 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*    80 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*    90 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*   100 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*   110 */   523,  522,  521,  520,  519,  621,  613,  614,  620,  594,
 /*   120 */   605,  569,  566,  565,  629,  629,  629,  629,  629,  629,
 /*   130 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*   140 */   629,  629,  592,  591,  590,  589,  588,  587,  585,  586,
 /*   150 */   570,  561,  563,  564,  562,  560,  558,  559,  567,  557,
 /*   160 */   556,  629,  596,  595,  568,  629,  629,  629,  629,  629,
 /*   170 */   629,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*   180 */   629,  629,  629,  629,  528,  629,  629,  629,  629,  629,
 /*   190 */   617,  629,  629,  629,  629,  629,  629,  629,  629,  629,
 /*   200 */   629,  629,  629,  629,  629,  629,  629,  616,  629,  629,
 /*   210 */   629,  629,  629,  629,  629,  622,  623,  629,  629,  629,
 /*   220 */   624,  624,  629,  629,  629,  629,  629,
};
/********** End of lemon-generated parsing tables *****************************/

/* The next table maps tokens (terminal symbols) into fallback tokens.  
** If a construct like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
**
** This feature can be used, for example, to cause some keywords in a language
** to revert to identifiers if they keyword does not apply in the context where
** it appears.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
**
** After the "shift" half of a SHIFTREDUCE action, the stateno field
** actually contains the reduce action for the second half of the
** SHIFTREDUCE.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number, or reduce action in SHIFTREDUCE */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
#ifndef YYNOERRORRECOVERY
  int yyerrcnt;                 /* Shifts left before out of the error */
#endif
  phvolt_ARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void phvolt_Trace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "OPEN_DELIMITER",  "COMMA",         "IN",          
  "QUESTION",      "COLON",         "RANGE",         "AND",         
  "OR",            "IS",            "EQUALS",        "NOTEQUALS",   
  "LESS",          "GREATER",       "GREATEREQUAL",  "LESSEQUAL",   
  "IDENTICAL",     "NOTIDENTICAL",  "DIVIDE",        "TIMES",       
  "MOD",           "PLUS",          "MINUS",         "CONCAT",      
  "SBRACKET_OPEN",  "PIPE",          "NOT",           "INCR",        
  "DECR",          "PARENTHESES_OPEN",  "DOT",           "IF",          
  "CLOSE_DELIMITER",  "ENDIF",         "ELSE",          "ELSEIF",      
  "ELSEFOR",       "FOR",           "IDENTIFIER",    "ENDFOR",      
  "SET",           "ASSIGN",        "ADD_ASSIGN",    "SUB_ASSIGN",  
  "MUL_ASSIGN",    "DIV_ASSIGN",    "MACRO",         "PARENTHESES_CLOSE",
  "ENDMACRO",      "INTEGER",       "STRING",        "DOUBLE",      
  "NULL",          "FALSE",         "TRUE",          "CALL",        
  "ENDCALL",       "OPEN_EDELIMITER",  "CLOSE_EDELIMITER",  "BLOCK",       
  "ENDBLOCK",      "CACHE",         "ENDCACHE",      "EXTENDS",     
  "INCLUDE",       "WITH",          "DO",            "RETURN",      
  "AUTOESCAPE",    "ENDAUTOESCAPE",  "BREAK",         "CONTINUE",    
  "RAW_FRAGMENT",  "DEFINED",       "EMPTY",         "EVEN",        
  "ODD",           "NUMERIC",       "SCALAR",        "ITERABLE",    
  "SBRACKET_CLOSE",  "CBRACKET_OPEN",  "CBRACKET_CLOSE",  "error",       
  "program",       "volt_language",  "statement_list",  "statement",   
  "raw_fragment",  "if_statement",  "elseif_statement",  "elsefor_statement",
  "for_statement",  "set_statement",  "echo_statement",  "block_statement",
  "cache_statement",  "extends_statement",  "include_statement",  "do_statement",
  "return_statement",  "autoescape_statement",  "break_statement",  "continue_statement",
  "macro_statement",  "empty_statement",  "macro_call_statement",  "expr",        
  "set_assignments",  "set_assignment",  "macro_parameters",  "macro_parameter",
  "macro_parameter_default",  "argument_list",  "cache_lifetime",  "array_list",  
  "slice_offset",  "array_item",    "function_call",  "argument_item",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "program ::= volt_language",
 /*   1 */ "volt_language ::= statement_list",
 /*   2 */ "statement_list ::= statement_list statement",
 /*   3 */ "statement_list ::= statement",
 /*   4 */ "statement ::= raw_fragment",
 /*   5 */ "statement ::= if_statement",
 /*   6 */ "statement ::= elseif_statement",
 /*   7 */ "statement ::= elsefor_statement",
 /*   8 */ "statement ::= for_statement",
 /*   9 */ "statement ::= set_statement",
 /*  10 */ "statement ::= echo_statement",
 /*  11 */ "statement ::= block_statement",
 /*  12 */ "statement ::= cache_statement",
 /*  13 */ "statement ::= extends_statement",
 /*  14 */ "statement ::= include_statement",
 /*  15 */ "statement ::= do_statement",
 /*  16 */ "statement ::= return_statement",
 /*  17 */ "statement ::= autoescape_statement",
 /*  18 */ "statement ::= break_statement",
 /*  19 */ "statement ::= continue_statement",
 /*  20 */ "statement ::= macro_statement",
 /*  21 */ "statement ::= empty_statement",
 /*  22 */ "statement ::= macro_call_statement",
 /*  23 */ "if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDIF CLOSE_DELIMITER",
 /*  24 */ "if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER OPEN_DELIMITER ENDIF CLOSE_DELIMITER",
 /*  25 */ "if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ELSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDIF CLOSE_DELIMITER",
 /*  26 */ "if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ELSE CLOSE_DELIMITER OPEN_DELIMITER ENDIF CLOSE_DELIMITER",
 /*  27 */ "if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER OPEN_DELIMITER ELSE CLOSE_DELIMITER OPEN_DELIMITER ENDIF CLOSE_DELIMITER",
 /*  28 */ "elseif_statement ::= OPEN_DELIMITER ELSEIF expr CLOSE_DELIMITER",
 /*  29 */ "elsefor_statement ::= OPEN_DELIMITER ELSEFOR CLOSE_DELIMITER",
 /*  30 */ "for_statement ::= OPEN_DELIMITER FOR IDENTIFIER IN expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER",
 /*  31 */ "for_statement ::= OPEN_DELIMITER FOR IDENTIFIER IN expr IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER",
 /*  32 */ "for_statement ::= OPEN_DELIMITER FOR IDENTIFIER COMMA IDENTIFIER IN expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER",
 /*  33 */ "for_statement ::= OPEN_DELIMITER FOR IDENTIFIER COMMA IDENTIFIER IN expr IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER",
 /*  34 */ "set_statement ::= OPEN_DELIMITER SET set_assignments CLOSE_DELIMITER",
 /*  35 */ "set_assignments ::= set_assignments COMMA set_assignment",
 /*  36 */ "set_assignments ::= set_assignment",
 /*  37 */ "set_assignment ::= IDENTIFIER ASSIGN expr",
 /*  38 */ "set_assignment ::= IDENTIFIER ADD_ASSIGN expr",
 /*  39 */ "set_assignment ::= IDENTIFIER SUB_ASSIGN expr",
 /*  40 */ "set_assignment ::= IDENTIFIER MUL_ASSIGN expr",
 /*  41 */ "set_assignment ::= IDENTIFIER DIV_ASSIGN expr",
 /*  42 */ "macro_statement ::= OPEN_DELIMITER MACRO IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDMACRO CLOSE_DELIMITER",
 /*  43 */ "macro_statement ::= OPEN_DELIMITER MACRO IDENTIFIER PARENTHESES_OPEN macro_parameters PARENTHESES_CLOSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDMACRO CLOSE_DELIMITER",
 /*  44 */ "macro_parameters ::= macro_parameters COMMA macro_parameter",
 /*  45 */ "macro_parameters ::= macro_parameter",
 /*  46 */ "macro_parameter ::= IDENTIFIER",
 /*  47 */ "macro_parameter ::= IDENTIFIER ASSIGN macro_parameter_default",
 /*  48 */ "macro_parameter_default ::= INTEGER",
 /*  49 */ "macro_parameter_default ::= STRING",
 /*  50 */ "macro_parameter_default ::= DOUBLE",
 /*  51 */ "macro_parameter_default ::= NULL",
 /*  52 */ "macro_parameter_default ::= FALSE",
 /*  53 */ "macro_parameter_default ::= TRUE",
 /*  54 */ "macro_call_statement ::= OPEN_DELIMITER CALL expr PARENTHESES_OPEN argument_list PARENTHESES_CLOSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDCALL CLOSE_DELIMITER",
 /*  55 */ "macro_call_statement ::= OPEN_DELIMITER CALL expr PARENTHESES_OPEN PARENTHESES_CLOSE CLOSE_DELIMITER OPEN_DELIMITER ENDCALL CLOSE_DELIMITER",
 /*  56 */ "empty_statement ::= OPEN_DELIMITER CLOSE_DELIMITER",
 /*  57 */ "echo_statement ::= OPEN_EDELIMITER expr CLOSE_EDELIMITER",
 /*  58 */ "block_statement ::= OPEN_DELIMITER BLOCK IDENTIFIER CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDBLOCK CLOSE_DELIMITER",
 /*  59 */ "block_statement ::= OPEN_DELIMITER BLOCK IDENTIFIER CLOSE_DELIMITER OPEN_DELIMITER ENDBLOCK CLOSE_DELIMITER",
 /*  60 */ "cache_statement ::= OPEN_DELIMITER CACHE expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDCACHE CLOSE_DELIMITER",
 /*  61 */ "cache_statement ::= OPEN_DELIMITER CACHE expr cache_lifetime CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDCACHE CLOSE_DELIMITER",
 /*  62 */ "cache_lifetime ::= INTEGER",
 /*  63 */ "cache_lifetime ::= IDENTIFIER",
 /*  64 */ "extends_statement ::= OPEN_DELIMITER EXTENDS STRING CLOSE_DELIMITER",
 /*  65 */ "include_statement ::= OPEN_DELIMITER INCLUDE expr CLOSE_DELIMITER",
 /*  66 */ "include_statement ::= OPEN_DELIMITER INCLUDE expr WITH expr CLOSE_DELIMITER",
 /*  67 */ "do_statement ::= OPEN_DELIMITER DO expr CLOSE_DELIMITER",
 /*  68 */ "return_statement ::= OPEN_DELIMITER RETURN expr CLOSE_DELIMITER",
 /*  69 */ "autoescape_statement ::= OPEN_DELIMITER AUTOESCAPE FALSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDAUTOESCAPE CLOSE_DELIMITER",
 /*  70 */ "autoescape_statement ::= OPEN_DELIMITER AUTOESCAPE TRUE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDAUTOESCAPE CLOSE_DELIMITER",
 /*  71 */ "break_statement ::= OPEN_DELIMITER BREAK CLOSE_DELIMITER",
 /*  72 */ "continue_statement ::= OPEN_DELIMITER CONTINUE CLOSE_DELIMITER",
 /*  73 */ "raw_fragment ::= RAW_FRAGMENT",
 /*  74 */ "expr ::= MINUS expr",
 /*  75 */ "expr ::= PLUS expr",
 /*  76 */ "expr ::= expr MINUS expr",
 /*  77 */ "expr ::= expr PLUS expr",
 /*  78 */ "expr ::= expr TIMES expr",
 /*  79 */ "expr ::= expr TIMES TIMES expr",
 /*  80 */ "expr ::= expr DIVIDE expr",
 /*  81 */ "expr ::= expr DIVIDE DIVIDE expr",
 /*  82 */ "expr ::= expr MOD expr",
 /*  83 */ "expr ::= expr AND expr",
 /*  84 */ "expr ::= expr OR expr",
 /*  85 */ "expr ::= expr CONCAT expr",
 /*  86 */ "expr ::= expr PIPE expr",
 /*  87 */ "expr ::= expr RANGE expr",
 /*  88 */ "expr ::= expr EQUALS expr",
 /*  89 */ "expr ::= expr NOTEQUALS DEFINED",
 /*  90 */ "expr ::= expr IS DEFINED",
 /*  91 */ "expr ::= expr NOTEQUALS EMPTY",
 /*  92 */ "expr ::= expr IS EMPTY",
 /*  93 */ "expr ::= expr NOTEQUALS EVEN",
 /*  94 */ "expr ::= expr IS EVEN",
 /*  95 */ "expr ::= expr NOTEQUALS ODD",
 /*  96 */ "expr ::= expr IS ODD",
 /*  97 */ "expr ::= expr NOTEQUALS NUMERIC",
 /*  98 */ "expr ::= expr IS NUMERIC",
 /*  99 */ "expr ::= expr NOTEQUALS SCALAR",
 /* 100 */ "expr ::= expr IS SCALAR",
 /* 101 */ "expr ::= expr NOTEQUALS ITERABLE",
 /* 102 */ "expr ::= expr IS ITERABLE",
 /* 103 */ "expr ::= expr IS expr",
 /* 104 */ "expr ::= expr NOTEQUALS expr",
 /* 105 */ "expr ::= expr IDENTICAL expr",
 /* 106 */ "expr ::= expr NOTIDENTICAL expr",
 /* 107 */ "expr ::= expr LESS expr",
 /* 108 */ "expr ::= expr GREATER expr",
 /* 109 */ "expr ::= expr GREATEREQUAL expr",
 /* 110 */ "expr ::= expr LESSEQUAL expr",
 /* 111 */ "expr ::= expr DOT expr",
 /* 112 */ "expr ::= expr IN expr",
 /* 113 */ "expr ::= expr NOT IN expr",
 /* 114 */ "expr ::= NOT expr",
 /* 115 */ "expr ::= expr INCR",
 /* 116 */ "expr ::= expr DECR",
 /* 117 */ "expr ::= PARENTHESES_OPEN expr PARENTHESES_CLOSE",
 /* 118 */ "expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 119 */ "expr ::= SBRACKET_OPEN array_list SBRACKET_CLOSE",
 /* 120 */ "expr ::= CBRACKET_OPEN CBRACKET_CLOSE",
 /* 121 */ "expr ::= CBRACKET_OPEN array_list CBRACKET_CLOSE",
 /* 122 */ "expr ::= expr SBRACKET_OPEN expr SBRACKET_CLOSE",
 /* 123 */ "expr ::= expr QUESTION expr COLON expr",
 /* 124 */ "expr ::= expr SBRACKET_OPEN COLON slice_offset SBRACKET_CLOSE",
 /* 125 */ "expr ::= expr SBRACKET_OPEN slice_offset COLON SBRACKET_CLOSE",
 /* 126 */ "expr ::= expr SBRACKET_OPEN slice_offset COLON slice_offset SBRACKET_CLOSE",
 /* 127 */ "slice_offset ::= INTEGER",
 /* 128 */ "slice_offset ::= IDENTIFIER",
 /* 129 */ "array_list ::= array_list COMMA array_item",
 /* 130 */ "array_list ::= array_item",
 /* 131 */ "array_item ::= STRING COLON expr",
 /* 132 */ "array_item ::= expr",
 /* 133 */ "expr ::= function_call",
 /* 134 */ "function_call ::= expr PARENTHESES_OPEN argument_list PARENTHESES_CLOSE",
 /* 135 */ "function_call ::= expr PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 136 */ "argument_list ::= argument_list COMMA argument_item",
 /* 137 */ "argument_list ::= argument_item",
 /* 138 */ "argument_item ::= expr",
 /* 139 */ "argument_item ::= STRING COLON expr",
 /* 140 */ "expr ::= IDENTIFIER",
 /* 141 */ "expr ::= INTEGER",
 /* 142 */ "expr ::= STRING",
 /* 143 */ "expr ::= DOUBLE",
 /* 144 */ "expr ::= NULL",
 /* 145 */ "expr ::= FALSE",
 /* 146 */ "expr ::= TRUE",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to phvolt_Alloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to phvolt_ and phvolt_Free.
*/
void *phvolt_Alloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (YYMALLOCARGTYPE)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the "minor type" or semantic value
** associated with a symbol.  The symbol can be either a terminal
** or nonterminal. "yymajor" is the symbol code, and "yypminor" is
** a pointer to the value to be deleted.  The code used to do the 
** deletions is derived from the %destructor and/or %token_destructor
** directives of the input grammar.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  phvolt_ARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are *not* used
    ** inside the C code.
    */
/********* Begin destructor definitions ***************************************/
      /* TERMINAL Destructor */
    case 1: /* OPEN_DELIMITER */
    case 2: /* COMMA */
    case 3: /* IN */
    case 4: /* QUESTION */
    case 5: /* COLON */
    case 6: /* RANGE */
    case 7: /* AND */
    case 8: /* OR */
    case 9: /* IS */
    case 10: /* EQUALS */
    case 11: /* NOTEQUALS */
    case 12: /* LESS */
    case 13: /* GREATER */
    case 14: /* GREATEREQUAL */
    case 15: /* LESSEQUAL */
    case 16: /* IDENTICAL */
    case 17: /* NOTIDENTICAL */
    case 18: /* DIVIDE */
    case 19: /* TIMES */
    case 20: /* MOD */
    case 21: /* PLUS */
    case 22: /* MINUS */
    case 23: /* CONCAT */
    case 24: /* SBRACKET_OPEN */
    case 25: /* PIPE */
    case 26: /* NOT */
    case 27: /* INCR */
    case 28: /* DECR */
    case 29: /* PARENTHESES_OPEN */
    case 30: /* DOT */
    case 31: /* IF */
    case 32: /* CLOSE_DELIMITER */
    case 33: /* ENDIF */
    case 34: /* ELSE */
    case 35: /* ELSEIF */
    case 36: /* ELSEFOR */
    case 37: /* FOR */
    case 38: /* IDENTIFIER */
    case 39: /* ENDFOR */
    case 40: /* SET */
    case 41: /* ASSIGN */
    case 42: /* ADD_ASSIGN */
    case 43: /* SUB_ASSIGN */
    case 44: /* MUL_ASSIGN */
    case 45: /* DIV_ASSIGN */
    case 46: /* MACRO */
    case 47: /* PARENTHESES_CLOSE */
    case 48: /* ENDMACRO */
    case 49: /* INTEGER */
    case 50: /* STRING */
    case 51: /* DOUBLE */
    case 52: /* NULL */
    case 53: /* FALSE */
    case 54: /* TRUE */
    case 55: /* CALL */
    case 56: /* ENDCALL */
    case 57: /* OPEN_EDELIMITER */
    case 58: /* CLOSE_EDELIMITER */
    case 59: /* BLOCK */
    case 60: /* ENDBLOCK */
    case 61: /* CACHE */
    case 62: /* ENDCACHE */
    case 63: /* EXTENDS */
    case 64: /* INCLUDE */
    case 65: /* WITH */
    case 66: /* DO */
    case 67: /* RETURN */
    case 68: /* AUTOESCAPE */
    case 69: /* ENDAUTOESCAPE */
    case 70: /* BREAK */
    case 71: /* CONTINUE */
    case 72: /* RAW_FRAGMENT */
    case 73: /* DEFINED */
    case 74: /* EMPTY */
    case 75: /* EVEN */
    case 76: /* ODD */
    case 77: /* NUMERIC */
    case 78: /* SCALAR */
    case 79: /* ITERABLE */
    case 80: /* SBRACKET_CLOSE */
    case 81: /* CBRACKET_OPEN */
    case 82: /* CBRACKET_CLOSE */
{
/* #line 22 "parser.y" */

	if ((yypminor->yy0)) {
		if ((yypminor->yy0)->free_flag) {
			efree((yypminor->yy0)->token);
		}
		efree((yypminor->yy0));
	}

/* #line 1726 "parser.c" */
}
      break;
      /* Default NON-TERMINAL Destructor */
    case 83: /* error */
    case 84: /* program */
    case 85: /* volt_language */
    case 86: /* statement_list */
    case 87: /* statement */
    case 88: /* raw_fragment */
    case 89: /* if_statement */
    case 90: /* elseif_statement */
    case 91: /* elsefor_statement */
    case 92: /* for_statement */
    case 93: /* set_statement */
    case 94: /* echo_statement */
    case 95: /* block_statement */
    case 96: /* cache_statement */
    case 97: /* extends_statement */
    case 98: /* include_statement */
    case 99: /* do_statement */
    case 100: /* return_statement */
    case 101: /* autoescape_statement */
    case 102: /* break_statement */
    case 103: /* continue_statement */
    case 104: /* macro_statement */
    case 105: /* empty_statement */
    case 106: /* macro_call_statement */
    case 107: /* expr */
    case 108: /* set_assignments */
    case 109: /* set_assignment */
    case 110: /* macro_parameters */
    case 111: /* macro_parameter */
    case 112: /* macro_parameter_default */
    case 113: /* argument_list */
    case 114: /* cache_lifetime */
    case 115: /* array_list */
    case 116: /* slice_offset */
    case 117: /* array_item */
    case 118: /* function_call */
    case 119: /* argument_item */
{
/* #line 31 "parser.y" */

	if (status) {
		// TODO:
	}
	zval_ptr_dtor((yypminor->yy168));
	efree((yypminor->yy168));

/* #line 1776 "parser.c" */
}
      break;
/********* End destructor definitions *****************************************/
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
*/
static void yy_pop_parser_stack(yyParser *pParser){
  yyStackEntry *yytos;
  assert( pParser->yyidx>=0 );
  yytos = &pParser->yystack[pParser->yyidx--];
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yy_destructor(pParser, yytos->major, &yytos->minor);
}

/* 
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the YYPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void phvolt_Free(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
#ifndef YYPARSEFREENEVERNULL
  if( pParser==0 ) return;
#endif
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int phvolt_StackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
*/
static unsigned int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>=YY_MIN_REDUCE ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
  do{
    i = yy_shift_ofst[stateno];
    if( i==YY_SHIFT_USE_DFLT ) return yy_default[stateno];
    assert( iLookAhead!=YYNOCODE );
    i += iLookAhead;
    if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
      if( iLookAhead>0 ){
#ifdef YYFALLBACK
        YYCODETYPE iFallback;            /* Fallback token */
        if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
               && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
          }
#endif
          assert( yyFallback[iFallback]==0 ); /* Fallback loop must terminate */
          iLookAhead = iFallback;
          continue;
        }
#endif
#ifdef YYWILDCARD
        {
          int j = i - iLookAhead + YYWILDCARD;
          if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
            j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
            j<YY_ACTTAB_COUNT &&
#endif
            yy_lookahead[j]==YYWILDCARD
          ){
#ifndef NDEBUG
            if( yyTraceFILE ){
              fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
                 yyTracePrompt, yyTokenName[iLookAhead],
                 yyTokenName[YYWILDCARD]);
            }
#endif /* NDEBUG */
            return yy_action[j];
          }
        }
#endif /* YYWILDCARD */
      }
      return yy_default[stateno];
    }else{
      return yy_action[i];
    }
  }while(1);
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser){
   phvolt_ARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
/******** End %stack_overflow code ********************************************/
   phvolt_ARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void yyTraceShift(yyParser *yypParser, int yyNewState){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%sShift '%s', go to state %d\n",
         yyTracePrompt,yyTokenName[yypParser->yystack[yypParser->yyidx].major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%sShift '%s'\n",
         yyTracePrompt,yyTokenName[yypParser->yystack[yypParser->yyidx].major]);
    }
  }
}
#else
# define yyTraceShift(X,Y)
#endif

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  phvolt_TOKENTYPE yyMinor        /* The minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor.yy0 = yyMinor;
  yyTraceShift(yypParser, yyNewState);
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 84, 1 },
  { 85, 1 },
  { 86, 2 },
  { 86, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 89, 8 },
  { 89, 7 },
  { 89, 12 },
  { 89, 11 },
  { 89, 10 },
  { 90, 4 },
  { 91, 3 },
  { 92, 10 },
  { 92, 12 },
  { 92, 12 },
  { 92, 14 },
  { 93, 4 },
  { 108, 3 },
  { 108, 1 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 104, 10 },
  { 104, 11 },
  { 110, 3 },
  { 110, 1 },
  { 111, 1 },
  { 111, 3 },
  { 112, 1 },
  { 112, 1 },
  { 112, 1 },
  { 112, 1 },
  { 112, 1 },
  { 112, 1 },
  { 106, 11 },
  { 106, 9 },
  { 105, 2 },
  { 94, 3 },
  { 95, 8 },
  { 95, 7 },
  { 96, 8 },
  { 96, 9 },
  { 114, 1 },
  { 114, 1 },
  { 97, 4 },
  { 98, 4 },
  { 98, 6 },
  { 99, 4 },
  { 100, 4 },
  { 101, 8 },
  { 101, 8 },
  { 102, 3 },
  { 103, 3 },
  { 88, 1 },
  { 107, 2 },
  { 107, 2 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 4 },
  { 107, 3 },
  { 107, 4 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 3 },
  { 107, 4 },
  { 107, 2 },
  { 107, 2 },
  { 107, 2 },
  { 107, 3 },
  { 107, 2 },
  { 107, 3 },
  { 107, 2 },
  { 107, 3 },
  { 107, 4 },
  { 107, 5 },
  { 107, 5 },
  { 107, 5 },
  { 107, 6 },
  { 116, 1 },
  { 116, 1 },
  { 115, 3 },
  { 115, 1 },
  { 117, 3 },
  { 117, 1 },
  { 107, 1 },
  { 118, 4 },
  { 118, 3 },
  { 113, 3 },
  { 113, 1 },
  { 119, 1 },
  { 119, 3 },
  { 107, 1 },
  { 107, 1 },
  { 107, 1 },
  { 107, 1 },
  { 107, 1 },
  { 107, 1 },
  { 107, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  unsigned int yyruleno        /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  phvolt_ARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    fprintf(yyTraceFILE, "%sReduce [%s], go to state %d.\n", yyTracePrompt,
      yyRuleName[yyruleno], yymsp[-yysize].stateno);
  }
#endif /* NDEBUG */

  /* Check that the stack is large enough to grow by a single entry
  ** if the RHS of the rule is empty.  This ensures that there is room
  ** enough on the stack to push the LHS value */
  if( yyRuleInfo[yyruleno].nrhs==0 ){
#ifdef YYTRACKMAXSTACKDEPTH
    if( yypParser->yyidx>yypParser->yyidxMax ){
      yypParser->yyidxMax = yypParser->yyidx;
    }
#endif
#if YYSTACKDEPTH>0 
    if( yypParser->yyidx>=YYSTACKDEPTH-1 ){
      yyStackOverflow(yypParser);
      return;
    }
#else
    if( yypParser->yyidx>=yypParser->yystksz-1 ){
      yyGrowStack(yypParser);
      if( yypParser->yyidx>=yypParser->yystksz-1 ){
        yyStackOverflow(yypParser);
        return;
      }
    }
#endif
  }

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
/********** Begin reduce actions **********************************************/
        YYMINORTYPE yylhsminor;
      case 0: /* program ::= volt_language */
/* #line 733 "parser.y" */
{
	status->ret = yymsp[0].minor.yy168;
}
/* #line 2231 "parser.c" */
        break;
      case 1: /* volt_language ::= statement_list */
      case 4: /* statement ::= raw_fragment */ yytestcase(yyruleno==4);
      case 5: /* statement ::= if_statement */ yytestcase(yyruleno==5);
      case 6: /* statement ::= elseif_statement */ yytestcase(yyruleno==6);
      case 7: /* statement ::= elsefor_statement */ yytestcase(yyruleno==7);
      case 8: /* statement ::= for_statement */ yytestcase(yyruleno==8);
      case 9: /* statement ::= set_statement */ yytestcase(yyruleno==9);
      case 10: /* statement ::= echo_statement */ yytestcase(yyruleno==10);
      case 11: /* statement ::= block_statement */ yytestcase(yyruleno==11);
      case 12: /* statement ::= cache_statement */ yytestcase(yyruleno==12);
      case 13: /* statement ::= extends_statement */ yytestcase(yyruleno==13);
      case 14: /* statement ::= include_statement */ yytestcase(yyruleno==14);
      case 15: /* statement ::= do_statement */ yytestcase(yyruleno==15);
      case 16: /* statement ::= return_statement */ yytestcase(yyruleno==16);
      case 17: /* statement ::= autoescape_statement */ yytestcase(yyruleno==17);
      case 18: /* statement ::= break_statement */ yytestcase(yyruleno==18);
      case 19: /* statement ::= continue_statement */ yytestcase(yyruleno==19);
      case 20: /* statement ::= macro_statement */ yytestcase(yyruleno==20);
      case 21: /* statement ::= empty_statement */ yytestcase(yyruleno==21);
      case 22: /* statement ::= macro_call_statement */ yytestcase(yyruleno==22);
      case 133: /* expr ::= function_call */ yytestcase(yyruleno==133);
/* #line 737 "parser.y" */
{
	yylhsminor.yy168 = yymsp[0].minor.yy168;
}
/* #line 2258 "parser.c" */
  yymsp[0].minor.yy168 = yylhsminor.yy168;
        break;
      case 2: /* statement_list ::= statement_list statement */
/* #line 741 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_zval_list(yymsp[-1].minor.yy168, yymsp[0].minor.yy168);
}
/* #line 2266 "parser.c" */
  yymsp[-1].minor.yy168 = yylhsminor.yy168;
        break;
      case 3: /* statement_list ::= statement */
      case 36: /* set_assignments ::= set_assignment */ yytestcase(yyruleno==36);
      case 45: /* macro_parameters ::= macro_parameter */ yytestcase(yyruleno==45);
      case 130: /* array_list ::= array_item */ yytestcase(yyruleno==130);
      case 137: /* argument_list ::= argument_item */ yytestcase(yyruleno==137);
/* #line 745 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_zval_list(NULL, yymsp[0].minor.yy168);
}
/* #line 2278 "parser.c" */
  yymsp[0].minor.yy168 = yylhsminor.yy168;
        break;
      case 23: /* if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDIF CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-7].minor);
/* #line 825 "parser.y" */
{
	yymsp[-7].minor.yy168 = phvolt_ret_if_statement(yymsp[-5].minor.yy168, yymsp[-3].minor.yy168, NULL, status->scanner_state);
}
/* #line 2287 "parser.c" */
  yy_destructor(yypParser,31,&yymsp[-6].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,33,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 24: /* if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER OPEN_DELIMITER ENDIF CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-6].minor);
/* #line 829 "parser.y" */
{
	yymsp[-6].minor.yy168 = phvolt_ret_if_statement(yymsp[-4].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 2301 "parser.c" */
  yy_destructor(yypParser,31,&yymsp[-5].minor);
  yy_destructor(yypParser,32,&yymsp[-3].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,33,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 25: /* if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ELSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDIF CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-11].minor);
/* #line 833 "parser.y" */
{
	yymsp[-11].minor.yy168 = phvolt_ret_if_statement(yymsp[-9].minor.yy168, yymsp[-7].minor.yy168, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2315 "parser.c" */
  yy_destructor(yypParser,31,&yymsp[-10].minor);
  yy_destructor(yypParser,32,&yymsp[-8].minor);
  yy_destructor(yypParser,1,&yymsp[-6].minor);
  yy_destructor(yypParser,34,&yymsp[-5].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,33,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 26: /* if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ELSE CLOSE_DELIMITER OPEN_DELIMITER ENDIF CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-10].minor);
/* #line 837 "parser.y" */
{
	yymsp[-10].minor.yy168 = phvolt_ret_if_statement(yymsp[-8].minor.yy168, yymsp[-6].minor.yy168, NULL, status->scanner_state);
}
/* #line 2332 "parser.c" */
  yy_destructor(yypParser,31,&yymsp[-9].minor);
  yy_destructor(yypParser,32,&yymsp[-7].minor);
  yy_destructor(yypParser,1,&yymsp[-5].minor);
  yy_destructor(yypParser,34,&yymsp[-4].minor);
  yy_destructor(yypParser,32,&yymsp[-3].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,33,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 27: /* if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER OPEN_DELIMITER ELSE CLOSE_DELIMITER OPEN_DELIMITER ENDIF CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-9].minor);
/* #line 841 "parser.y" */
{
	yymsp[-9].minor.yy168 = phvolt_ret_if_statement(yymsp[-7].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 2349 "parser.c" */
  yy_destructor(yypParser,31,&yymsp[-8].minor);
  yy_destructor(yypParser,32,&yymsp[-6].minor);
  yy_destructor(yypParser,1,&yymsp[-5].minor);
  yy_destructor(yypParser,34,&yymsp[-4].minor);
  yy_destructor(yypParser,32,&yymsp[-3].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,33,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 28: /* elseif_statement ::= OPEN_DELIMITER ELSEIF expr CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-3].minor);
/* #line 845 "parser.y" */
{
	yymsp[-3].minor.yy168 = phvolt_ret_elseif_statement(yymsp[-1].minor.yy168, status->scanner_state);
}
/* #line 2366 "parser.c" */
  yy_destructor(yypParser,35,&yymsp[-2].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 29: /* elsefor_statement ::= OPEN_DELIMITER ELSEFOR CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-2].minor);
/* #line 849 "parser.y" */
{
	yymsp[-2].minor.yy168 = phvolt_ret_elsefor_statement(status->scanner_state);
}
/* #line 2377 "parser.c" */
  yy_destructor(yypParser,36,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 30: /* for_statement ::= OPEN_DELIMITER FOR IDENTIFIER IN expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-9].minor);
/* #line 853 "parser.y" */
{
	yymsp[-9].minor.yy168 = phvolt_ret_for_statement(yymsp[-7].minor.yy0, NULL, yymsp[-5].minor.yy168, NULL, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2388 "parser.c" */
  yy_destructor(yypParser,37,&yymsp[-8].minor);
  yy_destructor(yypParser,3,&yymsp[-6].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,39,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 31: /* for_statement ::= OPEN_DELIMITER FOR IDENTIFIER IN expr IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-11].minor);
/* #line 857 "parser.y" */
{
	yymsp[-11].minor.yy168 = phvolt_ret_for_statement(yymsp[-9].minor.yy0, NULL, yymsp[-7].minor.yy168, yymsp[-5].minor.yy168, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2403 "parser.c" */
  yy_destructor(yypParser,37,&yymsp[-10].minor);
  yy_destructor(yypParser,3,&yymsp[-8].minor);
  yy_destructor(yypParser,31,&yymsp[-6].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,39,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 32: /* for_statement ::= OPEN_DELIMITER FOR IDENTIFIER COMMA IDENTIFIER IN expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-11].minor);
/* #line 861 "parser.y" */
{
	yymsp[-11].minor.yy168 = phvolt_ret_for_statement(yymsp[-7].minor.yy0, yymsp[-9].minor.yy0, yymsp[-5].minor.yy168, NULL, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2419 "parser.c" */
  yy_destructor(yypParser,37,&yymsp[-10].minor);
  yy_destructor(yypParser,2,&yymsp[-8].minor);
  yy_destructor(yypParser,3,&yymsp[-6].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,39,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 33: /* for_statement ::= OPEN_DELIMITER FOR IDENTIFIER COMMA IDENTIFIER IN expr IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-13].minor);
/* #line 865 "parser.y" */
{
	yymsp[-13].minor.yy168 = phvolt_ret_for_statement(yymsp[-9].minor.yy0, yymsp[-11].minor.yy0, yymsp[-7].minor.yy168, yymsp[-5].minor.yy168, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2435 "parser.c" */
  yy_destructor(yypParser,37,&yymsp[-12].minor);
  yy_destructor(yypParser,2,&yymsp[-10].minor);
  yy_destructor(yypParser,3,&yymsp[-8].minor);
  yy_destructor(yypParser,31,&yymsp[-6].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,39,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 34: /* set_statement ::= OPEN_DELIMITER SET set_assignments CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-3].minor);
/* #line 869 "parser.y" */
{
	yymsp[-3].minor.yy168 = phvolt_ret_set_statement(yymsp[-1].minor.yy168);
}
/* #line 2452 "parser.c" */
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 35: /* set_assignments ::= set_assignments COMMA set_assignment */
      case 44: /* macro_parameters ::= macro_parameters COMMA macro_parameter */ yytestcase(yyruleno==44);
      case 129: /* array_list ::= array_list COMMA array_item */ yytestcase(yyruleno==129);
      case 136: /* argument_list ::= argument_list COMMA argument_item */ yytestcase(yyruleno==136);
/* #line 873 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_zval_list(yymsp[-2].minor.yy168, yymsp[0].minor.yy168);
}
/* #line 2465 "parser.c" */
  yy_destructor(yypParser,2,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 37: /* set_assignment ::= IDENTIFIER ASSIGN expr */
/* #line 881 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_set_assignment(yymsp[-2].minor.yy0, PHVOLT_T_ASSIGN, yymsp[0].minor.yy168, status->scanner_state);
}
/* #line 2474 "parser.c" */
  yy_destructor(yypParser,41,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 38: /* set_assignment ::= IDENTIFIER ADD_ASSIGN expr */
/* #line 885 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_set_assignment(yymsp[-2].minor.yy0, PHVOLT_T_ADD_ASSIGN, yymsp[0].minor.yy168, status->scanner_state);
}
/* #line 2483 "parser.c" */
  yy_destructor(yypParser,42,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 39: /* set_assignment ::= IDENTIFIER SUB_ASSIGN expr */
/* #line 889 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_set_assignment(yymsp[-2].minor.yy0, PHVOLT_T_SUB_ASSIGN, yymsp[0].minor.yy168, status->scanner_state);
}
/* #line 2492 "parser.c" */
  yy_destructor(yypParser,43,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 40: /* set_assignment ::= IDENTIFIER MUL_ASSIGN expr */
/* #line 893 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_set_assignment(yymsp[-2].minor.yy0, PHVOLT_T_MUL_ASSIGN, yymsp[0].minor.yy168, status->scanner_state);
}
/* #line 2501 "parser.c" */
  yy_destructor(yypParser,44,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 41: /* set_assignment ::= IDENTIFIER DIV_ASSIGN expr */
/* #line 897 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_set_assignment(yymsp[-2].minor.yy0, PHVOLT_T_DIV_ASSIGN, yymsp[0].minor.yy168, status->scanner_state);
}
/* #line 2510 "parser.c" */
  yy_destructor(yypParser,45,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 42: /* macro_statement ::= OPEN_DELIMITER MACRO IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDMACRO CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-9].minor);
/* #line 901 "parser.y" */
{
	yymsp[-9].minor.yy168 = phvolt_ret_macro_statement(yymsp[-7].minor.yy0, NULL, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2520 "parser.c" */
  yy_destructor(yypParser,46,&yymsp[-8].minor);
  yy_destructor(yypParser,29,&yymsp[-6].minor);
  yy_destructor(yypParser,47,&yymsp[-5].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 43: /* macro_statement ::= OPEN_DELIMITER MACRO IDENTIFIER PARENTHESES_OPEN macro_parameters PARENTHESES_CLOSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDMACRO CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-10].minor);
/* #line 905 "parser.y" */
{
	yymsp[-10].minor.yy168 = phvolt_ret_macro_statement(yymsp[-8].minor.yy0, yymsp[-6].minor.yy168, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2536 "parser.c" */
  yy_destructor(yypParser,46,&yymsp[-9].minor);
  yy_destructor(yypParser,29,&yymsp[-7].minor);
  yy_destructor(yypParser,47,&yymsp[-5].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 46: /* macro_parameter ::= IDENTIFIER */
/* #line 917 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_macro_parameter(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
/* #line 2551 "parser.c" */
  yymsp[0].minor.yy168 = yylhsminor.yy168;
        break;
      case 47: /* macro_parameter ::= IDENTIFIER ASSIGN macro_parameter_default */
/* #line 921 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_macro_parameter(yymsp[-2].minor.yy0, yymsp[0].minor.yy168, status->scanner_state);
}
/* #line 2559 "parser.c" */
  yy_destructor(yypParser,41,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 48: /* macro_parameter_default ::= INTEGER */
      case 62: /* cache_lifetime ::= INTEGER */ yytestcase(yyruleno==62);
      case 127: /* slice_offset ::= INTEGER */ yytestcase(yyruleno==127);
      case 141: /* expr ::= INTEGER */ yytestcase(yyruleno==141);
/* #line 925 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_literal_zval(PHVOLT_T_INTEGER, yymsp[0].minor.yy0, status->scanner_state);
}
/* #line 2571 "parser.c" */
  yymsp[0].minor.yy168 = yylhsminor.yy168;
        break;
      case 49: /* macro_parameter_default ::= STRING */
      case 142: /* expr ::= STRING */ yytestcase(yyruleno==142);
/* #line 929 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_literal_zval(PHVOLT_T_STRING, yymsp[0].minor.yy0, status->scanner_state);
}
/* #line 2580 "parser.c" */
  yymsp[0].minor.yy168 = yylhsminor.yy168;
        break;
      case 50: /* macro_parameter_default ::= DOUBLE */
      case 143: /* expr ::= DOUBLE */ yytestcase(yyruleno==143);
/* #line 933 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_literal_zval(PHVOLT_T_DOUBLE, yymsp[0].minor.yy0, status->scanner_state);
}
/* #line 2589 "parser.c" */
  yymsp[0].minor.yy168 = yylhsminor.yy168;
        break;
      case 51: /* macro_parameter_default ::= NULL */
      case 144: /* expr ::= NULL */ yytestcase(yyruleno==144);
{  yy_destructor(yypParser,52,&yymsp[0].minor);
/* #line 937 "parser.y" */
{
	yymsp[0].minor.yy168 = phvolt_ret_literal_zval(PHVOLT_T_NULL, NULL, status->scanner_state);
}
/* #line 2599 "parser.c" */
}
        break;
      case 52: /* macro_parameter_default ::= FALSE */
      case 145: /* expr ::= FALSE */ yytestcase(yyruleno==145);
{  yy_destructor(yypParser,53,&yymsp[0].minor);
/* #line 941 "parser.y" */
{
	yymsp[0].minor.yy168 = phvolt_ret_literal_zval(PHVOLT_T_FALSE, NULL, status->scanner_state);
}
/* #line 2609 "parser.c" */
}
        break;
      case 53: /* macro_parameter_default ::= TRUE */
      case 146: /* expr ::= TRUE */ yytestcase(yyruleno==146);
{  yy_destructor(yypParser,54,&yymsp[0].minor);
/* #line 945 "parser.y" */
{
	yymsp[0].minor.yy168 = phvolt_ret_literal_zval(PHVOLT_T_TRUE, NULL, status->scanner_state);
}
/* #line 2619 "parser.c" */
}
        break;
      case 54: /* macro_call_statement ::= OPEN_DELIMITER CALL expr PARENTHESES_OPEN argument_list PARENTHESES_CLOSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDCALL CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-10].minor);
/* #line 949 "parser.y" */
{
	yymsp[-10].minor.yy168 = phvolt_ret_macro_call_statement(yymsp[-8].minor.yy168, yymsp[-6].minor.yy168, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2628 "parser.c" */
  yy_destructor(yypParser,55,&yymsp[-9].minor);
  yy_destructor(yypParser,29,&yymsp[-7].minor);
  yy_destructor(yypParser,47,&yymsp[-5].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,56,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 55: /* macro_call_statement ::= OPEN_DELIMITER CALL expr PARENTHESES_OPEN PARENTHESES_CLOSE CLOSE_DELIMITER OPEN_DELIMITER ENDCALL CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-8].minor);
/* #line 953 "parser.y" */
{
	yymsp[-8].minor.yy168 = phvolt_ret_macro_call_statement(yymsp[-6].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 2644 "parser.c" */
  yy_destructor(yypParser,55,&yymsp[-7].minor);
  yy_destructor(yypParser,29,&yymsp[-5].minor);
  yy_destructor(yypParser,47,&yymsp[-4].minor);
  yy_destructor(yypParser,32,&yymsp[-3].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,56,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 56: /* empty_statement ::= OPEN_DELIMITER CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-1].minor);
/* #line 957 "parser.y" */
{
	yymsp[-1].minor.yy168 = phvolt_ret_empty_statement(status->scanner_state);
}
/* #line 2660 "parser.c" */
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 57: /* echo_statement ::= OPEN_EDELIMITER expr CLOSE_EDELIMITER */
{  yy_destructor(yypParser,57,&yymsp[-2].minor);
/* #line 961 "parser.y" */
{
	yymsp[-2].minor.yy168 = phvolt_ret_echo_statement(yymsp[-1].minor.yy168, status->scanner_state);
}
/* #line 2670 "parser.c" */
  yy_destructor(yypParser,58,&yymsp[0].minor);
}
        break;
      case 58: /* block_statement ::= OPEN_DELIMITER BLOCK IDENTIFIER CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDBLOCK CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-7].minor);
/* #line 965 "parser.y" */
{
	yymsp[-7].minor.yy168 = phvolt_ret_block_statement(yymsp[-5].minor.yy0, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2680 "parser.c" */
  yy_destructor(yypParser,59,&yymsp[-6].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,60,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 59: /* block_statement ::= OPEN_DELIMITER BLOCK IDENTIFIER CLOSE_DELIMITER OPEN_DELIMITER ENDBLOCK CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-6].minor);
/* #line 969 "parser.y" */
{
	yymsp[-6].minor.yy168 = phvolt_ret_block_statement(yymsp[-4].minor.yy0, NULL, status->scanner_state);
}
/* #line 2694 "parser.c" */
  yy_destructor(yypParser,59,&yymsp[-5].minor);
  yy_destructor(yypParser,32,&yymsp[-3].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,60,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 60: /* cache_statement ::= OPEN_DELIMITER CACHE expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDCACHE CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-7].minor);
/* #line 973 "parser.y" */
{
	yymsp[-7].minor.yy168 = phvolt_ret_cache_statement(yymsp[-5].minor.yy168, NULL, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2708 "parser.c" */
  yy_destructor(yypParser,61,&yymsp[-6].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,62,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 61: /* cache_statement ::= OPEN_DELIMITER CACHE expr cache_lifetime CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDCACHE CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-8].minor);
/* #line 977 "parser.y" */
{
	yymsp[-8].minor.yy168 = phvolt_ret_cache_statement(yymsp[-6].minor.yy168, yymsp[-5].minor.yy168, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2722 "parser.c" */
  yy_destructor(yypParser,61,&yymsp[-7].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,62,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 63: /* cache_lifetime ::= IDENTIFIER */
      case 128: /* slice_offset ::= IDENTIFIER */ yytestcase(yyruleno==128);
      case 140: /* expr ::= IDENTIFIER */ yytestcase(yyruleno==140);
/* #line 985 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_literal_zval(PHVOLT_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state);
}
/* #line 2737 "parser.c" */
  yymsp[0].minor.yy168 = yylhsminor.yy168;
        break;
      case 64: /* extends_statement ::= OPEN_DELIMITER EXTENDS STRING CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-3].minor);
/* #line 989 "parser.y" */
{
	yymsp[-3].minor.yy168 = phvolt_ret_extends_statement(yymsp[-1].minor.yy0, status->scanner_state);
}
/* #line 2746 "parser.c" */
  yy_destructor(yypParser,63,&yymsp[-2].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 65: /* include_statement ::= OPEN_DELIMITER INCLUDE expr CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-3].minor);
/* #line 993 "parser.y" */
{
	yymsp[-3].minor.yy168 = phvolt_ret_include_statement(yymsp[-1].minor.yy168, NULL, status->scanner_state);
}
/* #line 2757 "parser.c" */
  yy_destructor(yypParser,64,&yymsp[-2].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 66: /* include_statement ::= OPEN_DELIMITER INCLUDE expr WITH expr CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-5].minor);
/* #line 997 "parser.y" */
{
	yymsp[-5].minor.yy168 = phvolt_ret_include_statement(yymsp[-3].minor.yy168, yymsp[-1].minor.yy168, status->scanner_state);
}
/* #line 2768 "parser.c" */
  yy_destructor(yypParser,64,&yymsp[-4].minor);
  yy_destructor(yypParser,65,&yymsp[-2].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 67: /* do_statement ::= OPEN_DELIMITER DO expr CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-3].minor);
/* #line 1001 "parser.y" */
{
	yymsp[-3].minor.yy168 = phvolt_ret_do_statement(yymsp[-1].minor.yy168, status->scanner_state);
}
/* #line 2780 "parser.c" */
  yy_destructor(yypParser,66,&yymsp[-2].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 68: /* return_statement ::= OPEN_DELIMITER RETURN expr CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-3].minor);
/* #line 1005 "parser.y" */
{
	yymsp[-3].minor.yy168 = phvolt_ret_return_statement(yymsp[-1].minor.yy168, status->scanner_state);
}
/* #line 2791 "parser.c" */
  yy_destructor(yypParser,67,&yymsp[-2].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 69: /* autoescape_statement ::= OPEN_DELIMITER AUTOESCAPE FALSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDAUTOESCAPE CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-7].minor);
/* #line 1009 "parser.y" */
{
	yymsp[-7].minor.yy168 = phvolt_ret_autoescape_statement(0, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2802 "parser.c" */
  yy_destructor(yypParser,68,&yymsp[-6].minor);
  yy_destructor(yypParser,53,&yymsp[-5].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,69,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 70: /* autoescape_statement ::= OPEN_DELIMITER AUTOESCAPE TRUE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDAUTOESCAPE CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-7].minor);
/* #line 1013 "parser.y" */
{
	yymsp[-7].minor.yy168 = phvolt_ret_autoescape_statement(1, yymsp[-3].minor.yy168, status->scanner_state);
}
/* #line 2817 "parser.c" */
  yy_destructor(yypParser,68,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,69,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 71: /* break_statement ::= OPEN_DELIMITER BREAK CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-2].minor);
/* #line 1017 "parser.y" */
{
	yymsp[-2].minor.yy168 = phvolt_ret_break_statement(status->scanner_state);
}
/* #line 2832 "parser.c" */
  yy_destructor(yypParser,70,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 72: /* continue_statement ::= OPEN_DELIMITER CONTINUE CLOSE_DELIMITER */
{  yy_destructor(yypParser,1,&yymsp[-2].minor);
/* #line 1021 "parser.y" */
{
	yymsp[-2].minor.yy168 = phvolt_ret_continue_statement(status->scanner_state);
}
/* #line 2843 "parser.c" */
  yy_destructor(yypParser,71,&yymsp[-1].minor);
  yy_destructor(yypParser,32,&yymsp[0].minor);
}
        break;
      case 73: /* raw_fragment ::= RAW_FRAGMENT */
/* #line 1025 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_literal_zval(PHVOLT_T_RAW_FRAGMENT, yymsp[0].minor.yy0, status->scanner_state);
}
/* #line 2853 "parser.c" */
  yymsp[0].minor.yy168 = yylhsminor.yy168;
        break;
      case 74: /* expr ::= MINUS expr */
{  yy_destructor(yypParser,22,&yymsp[-1].minor);
/* #line 1029 "parser.y" */
{
	yymsp[-1].minor.yy168 = phvolt_ret_expr(PHVOLT_T_MINUS, NULL, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2862 "parser.c" */
}
        break;
      case 75: /* expr ::= PLUS expr */
{  yy_destructor(yypParser,21,&yymsp[-1].minor);
/* #line 1033 "parser.y" */
{
	yymsp[-1].minor.yy168 = phvolt_ret_expr(PHVOLT_T_PLUS, NULL, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2871 "parser.c" */
}
        break;
      case 76: /* expr ::= expr MINUS expr */
/* #line 1037 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_SUB, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2879 "parser.c" */
  yy_destructor(yypParser,22,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 77: /* expr ::= expr PLUS expr */
/* #line 1041 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_ADD, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2888 "parser.c" */
  yy_destructor(yypParser,21,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 78: /* expr ::= expr TIMES expr */
/* #line 1045 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_MUL, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2897 "parser.c" */
  yy_destructor(yypParser,19,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 79: /* expr ::= expr TIMES TIMES expr */
/* #line 1049 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_POW, yymsp[-3].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2906 "parser.c" */
  yy_destructor(yypParser,19,&yymsp[-2].minor);
  yy_destructor(yypParser,19,&yymsp[-1].minor);
  yymsp[-3].minor.yy168 = yylhsminor.yy168;
        break;
      case 80: /* expr ::= expr DIVIDE expr */
/* #line 1053 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_DIV, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2916 "parser.c" */
  yy_destructor(yypParser,18,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 81: /* expr ::= expr DIVIDE DIVIDE expr */
/* #line 1057 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_MOD, yymsp[-3].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2925 "parser.c" */
  yy_destructor(yypParser,18,&yymsp[-2].minor);
  yy_destructor(yypParser,18,&yymsp[-1].minor);
  yymsp[-3].minor.yy168 = yylhsminor.yy168;
        break;
      case 82: /* expr ::= expr MOD expr */
/* #line 1061 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_MOD, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2935 "parser.c" */
  yy_destructor(yypParser,20,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 83: /* expr ::= expr AND expr */
/* #line 1065 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_AND, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2944 "parser.c" */
  yy_destructor(yypParser,7,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 84: /* expr ::= expr OR expr */
/* #line 1069 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_OR, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2953 "parser.c" */
  yy_destructor(yypParser,8,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 85: /* expr ::= expr CONCAT expr */
/* #line 1073 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_CONCAT, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2962 "parser.c" */
  yy_destructor(yypParser,23,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 86: /* expr ::= expr PIPE expr */
/* #line 1077 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_PIPE, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2971 "parser.c" */
  yy_destructor(yypParser,25,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 87: /* expr ::= expr RANGE expr */
/* #line 1081 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_RANGE, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2980 "parser.c" */
  yy_destructor(yypParser,6,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 88: /* expr ::= expr EQUALS expr */
/* #line 1085 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_EQUALS, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 2989 "parser.c" */
  yy_destructor(yypParser,10,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 89: /* expr ::= expr NOTEQUALS DEFINED */
/* #line 1089 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_NOT_ISSET, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 2998 "parser.c" */
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,73,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 90: /* expr ::= expr IS DEFINED */
/* #line 1093 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_ISSET, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3008 "parser.c" */
  yy_destructor(yypParser,9,&yymsp[-1].minor);
  yy_destructor(yypParser,73,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 91: /* expr ::= expr NOTEQUALS EMPTY */
/* #line 1097 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_NOT_ISEMPTY, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3018 "parser.c" */
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,74,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 92: /* expr ::= expr IS EMPTY */
/* #line 1101 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_ISEMPTY, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3028 "parser.c" */
  yy_destructor(yypParser,9,&yymsp[-1].minor);
  yy_destructor(yypParser,74,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 93: /* expr ::= expr NOTEQUALS EVEN */
/* #line 1105 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_NOT_ISEVEN, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3038 "parser.c" */
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,75,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 94: /* expr ::= expr IS EVEN */
/* #line 1109 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_ISEVEN, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3048 "parser.c" */
  yy_destructor(yypParser,9,&yymsp[-1].minor);
  yy_destructor(yypParser,75,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 95: /* expr ::= expr NOTEQUALS ODD */
/* #line 1113 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_NOT_ISODD, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3058 "parser.c" */
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,76,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 96: /* expr ::= expr IS ODD */
/* #line 1117 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_ISODD, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3068 "parser.c" */
  yy_destructor(yypParser,9,&yymsp[-1].minor);
  yy_destructor(yypParser,76,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 97: /* expr ::= expr NOTEQUALS NUMERIC */
/* #line 1121 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_NOT_ISNUMERIC, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3078 "parser.c" */
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,77,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 98: /* expr ::= expr IS NUMERIC */
/* #line 1125 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_ISNUMERIC, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3088 "parser.c" */
  yy_destructor(yypParser,9,&yymsp[-1].minor);
  yy_destructor(yypParser,77,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 99: /* expr ::= expr NOTEQUALS SCALAR */
/* #line 1129 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_NOT_ISSCALAR, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3098 "parser.c" */
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,78,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 100: /* expr ::= expr IS SCALAR */
/* #line 1133 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_ISSCALAR, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3108 "parser.c" */
  yy_destructor(yypParser,9,&yymsp[-1].minor);
  yy_destructor(yypParser,78,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 101: /* expr ::= expr NOTEQUALS ITERABLE */
/* #line 1137 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_NOT_ISITERABLE, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3118 "parser.c" */
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,79,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 102: /* expr ::= expr IS ITERABLE */
/* #line 1141 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_ISITERABLE, yymsp[-2].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3128 "parser.c" */
  yy_destructor(yypParser,9,&yymsp[-1].minor);
  yy_destructor(yypParser,79,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 103: /* expr ::= expr IS expr */
/* #line 1145 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_IS, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3138 "parser.c" */
  yy_destructor(yypParser,9,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 104: /* expr ::= expr NOTEQUALS expr */
/* #line 1149 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_NOTEQUALS, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3147 "parser.c" */
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 105: /* expr ::= expr IDENTICAL expr */
/* #line 1153 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_IDENTICAL, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3156 "parser.c" */
  yy_destructor(yypParser,16,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 106: /* expr ::= expr NOTIDENTICAL expr */
/* #line 1157 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_NOTIDENTICAL, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3165 "parser.c" */
  yy_destructor(yypParser,17,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 107: /* expr ::= expr LESS expr */
/* #line 1161 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_LESS, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3174 "parser.c" */
  yy_destructor(yypParser,12,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 108: /* expr ::= expr GREATER expr */
/* #line 1165 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_GREATER, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3183 "parser.c" */
  yy_destructor(yypParser,13,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 109: /* expr ::= expr GREATEREQUAL expr */
/* #line 1169 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_GREATEREQUAL, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3192 "parser.c" */
  yy_destructor(yypParser,14,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 110: /* expr ::= expr LESSEQUAL expr */
/* #line 1173 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_LESSEQUAL, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3201 "parser.c" */
  yy_destructor(yypParser,15,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 111: /* expr ::= expr DOT expr */
/* #line 1177 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_DOT, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3210 "parser.c" */
  yy_destructor(yypParser,30,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 112: /* expr ::= expr IN expr */
/* #line 1181 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_IN, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3219 "parser.c" */
  yy_destructor(yypParser,3,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 113: /* expr ::= expr NOT IN expr */
/* #line 1185 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_NOT_IN, yymsp[-3].minor.yy168, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3228 "parser.c" */
  yy_destructor(yypParser,26,&yymsp[-2].minor);
  yy_destructor(yypParser,3,&yymsp[-1].minor);
  yymsp[-3].minor.yy168 = yylhsminor.yy168;
        break;
      case 114: /* expr ::= NOT expr */
{  yy_destructor(yypParser,26,&yymsp[-1].minor);
/* #line 1189 "parser.y" */
{
	yymsp[-1].minor.yy168 = phvolt_ret_expr(PHVOLT_T_NOT, NULL, yymsp[0].minor.yy168, NULL, status->scanner_state);
}
/* #line 3239 "parser.c" */
}
        break;
      case 115: /* expr ::= expr INCR */
/* #line 1193 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_INCR, yymsp[-1].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3247 "parser.c" */
  yy_destructor(yypParser,27,&yymsp[0].minor);
  yymsp[-1].minor.yy168 = yylhsminor.yy168;
        break;
      case 116: /* expr ::= expr DECR */
/* #line 1197 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_DECR, yymsp[-1].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3256 "parser.c" */
  yy_destructor(yypParser,28,&yymsp[0].minor);
  yymsp[-1].minor.yy168 = yylhsminor.yy168;
        break;
      case 117: /* expr ::= PARENTHESES_OPEN expr PARENTHESES_CLOSE */
{  yy_destructor(yypParser,29,&yymsp[-2].minor);
/* #line 1201 "parser.y" */
{
	yymsp[-2].minor.yy168 = phvolt_ret_expr(PHVOLT_T_ENCLOSED, yymsp[-1].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3266 "parser.c" */
  yy_destructor(yypParser,47,&yymsp[0].minor);
}
        break;
      case 118: /* expr ::= SBRACKET_OPEN SBRACKET_CLOSE */
      case 120: /* expr ::= CBRACKET_OPEN CBRACKET_CLOSE */ yytestcase(yyruleno==120);
{  yy_destructor(yypParser,24,&yymsp[-1].minor);
/* #line 1205 "parser.y" */
{
	yymsp[-1].minor.yy168 = phvolt_ret_expr(PHVOLT_T_ARRAY, NULL, NULL, NULL, status->scanner_state);
}
/* #line 3277 "parser.c" */
  yy_destructor(yypParser,80,&yymsp[0].minor);
}
        break;
      case 119: /* expr ::= SBRACKET_OPEN array_list SBRACKET_CLOSE */
      case 121: /* expr ::= CBRACKET_OPEN array_list CBRACKET_CLOSE */ yytestcase(yyruleno==121);
{  yy_destructor(yypParser,24,&yymsp[-2].minor);
/* #line 1209 "parser.y" */
{
	yymsp[-2].minor.yy168 = phvolt_ret_expr(PHVOLT_T_ARRAY, yymsp[-1].minor.yy168, NULL, NULL, status->scanner_state);
}
/* #line 3288 "parser.c" */
  yy_destructor(yypParser,80,&yymsp[0].minor);
}
        break;
      case 122: /* expr ::= expr SBRACKET_OPEN expr SBRACKET_CLOSE */
/* #line 1221 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_ARRAYACCESS, yymsp[-3].minor.yy168, yymsp[-1].minor.yy168, NULL, status->scanner_state);
}
/* #line 3297 "parser.c" */
  yy_destructor(yypParser,24,&yymsp[-2].minor);
  yy_destructor(yypParser,80,&yymsp[0].minor);
  yymsp[-3].minor.yy168 = yylhsminor.yy168;
        break;
      case 123: /* expr ::= expr QUESTION expr COLON expr */
/* #line 1225 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_expr(PHVOLT_T_TERNARY, yymsp[-2].minor.yy168, yymsp[0].minor.yy168, yymsp[-4].minor.yy168, status->scanner_state);
}
/* #line 3307 "parser.c" */
  yy_destructor(yypParser,4,&yymsp[-3].minor);
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yymsp[-4].minor.yy168 = yylhsminor.yy168;
        break;
      case 124: /* expr ::= expr SBRACKET_OPEN COLON slice_offset SBRACKET_CLOSE */
/* #line 1229 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_slice(yymsp[-4].minor.yy168, NULL, yymsp[-1].minor.yy168, status->scanner_state);
}
/* #line 3317 "parser.c" */
  yy_destructor(yypParser,24,&yymsp[-3].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,80,&yymsp[0].minor);
  yymsp[-4].minor.yy168 = yylhsminor.yy168;
        break;
      case 125: /* expr ::= expr SBRACKET_OPEN slice_offset COLON SBRACKET_CLOSE */
/* #line 1233 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_slice(yymsp[-4].minor.yy168, yymsp[-2].minor.yy168, NULL, status->scanner_state);
}
/* #line 3328 "parser.c" */
  yy_destructor(yypParser,24,&yymsp[-3].minor);
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yy_destructor(yypParser,80,&yymsp[0].minor);
  yymsp[-4].minor.yy168 = yylhsminor.yy168;
        break;
      case 126: /* expr ::= expr SBRACKET_OPEN slice_offset COLON slice_offset SBRACKET_CLOSE */
/* #line 1237 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_slice(yymsp[-5].minor.yy168, yymsp[-3].minor.yy168, yymsp[-1].minor.yy168, status->scanner_state);
}
/* #line 3339 "parser.c" */
  yy_destructor(yypParser,24,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,80,&yymsp[0].minor);
  yymsp[-5].minor.yy168 = yylhsminor.yy168;
        break;
      case 131: /* array_item ::= STRING COLON expr */
      case 139: /* argument_item ::= STRING COLON expr */ yytestcase(yyruleno==139);
/* #line 1257 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_named_item(yymsp[-2].minor.yy0, yymsp[0].minor.yy168, status->scanner_state);
}
/* #line 3351 "parser.c" */
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 132: /* array_item ::= expr */
      case 138: /* argument_item ::= expr */ yytestcase(yyruleno==138);
/* #line 1261 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_named_item(NULL, yymsp[0].minor.yy168, status->scanner_state);
}
/* #line 3361 "parser.c" */
  yymsp[0].minor.yy168 = yylhsminor.yy168;
        break;
      case 134: /* function_call ::= expr PARENTHESES_OPEN argument_list PARENTHESES_CLOSE */
/* #line 1269 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_func_call(yymsp[-3].minor.yy168, yymsp[-1].minor.yy168, status->scanner_state);
}
/* #line 3369 "parser.c" */
  yy_destructor(yypParser,29,&yymsp[-2].minor);
  yy_destructor(yypParser,47,&yymsp[0].minor);
  yymsp[-3].minor.yy168 = yylhsminor.yy168;
        break;
      case 135: /* function_call ::= expr PARENTHESES_OPEN PARENTHESES_CLOSE */
/* #line 1273 "parser.y" */
{
	yylhsminor.yy168 = phvolt_ret_func_call(yymsp[-2].minor.yy168, NULL, status->scanner_state);
}
/* #line 3379 "parser.c" */
  yy_destructor(yypParser,29,&yymsp[-1].minor);
  yy_destructor(yypParser,47,&yymsp[0].minor);
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      default:
        break;
/********** End reduce actions ************************************************/
  };
  assert( yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact <= YY_MAX_SHIFTREDUCE ){
    if( yyact>YY_MAX_SHIFT ) yyact += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
    yypParser->yyidx -= yysize - 1;
    yymsp -= yysize-1;
    yymsp->stateno = (YYACTIONTYPE)yyact;
    yymsp->major = (YYCODETYPE)yygoto;
    yyTraceShift(yypParser, yyact);
  }else{
    assert( yyact == YY_ACCEPT_ACTION );
    yypParser->yyidx -= yysize;
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  phvolt_ARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
/************ End %parse_failure code *****************************************/
  phvolt_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  phvolt_TOKENTYPE yyminor         /* The minor type of the error token */
){
  phvolt_ARG_FETCH;
#define TOKEN yyminor
/************ Begin %syntax_error code ****************************************/
/* #line 634 "parser.y" */

	{

		smart_str error_str = {0};

		char *token_name = NULL;
		const phvolt_token_names *tokens = phvolt_tokens;
		int token_len = 0;
		uint active_token = status->scanner_state->active_token;

		if (status->scanner_state->start_length) {

			if (active_token) {

				do {
					if (tokens->code == active_token) {
						token_name = tokens->name;
						token_len = tokens->len;
						break;
					}
					++tokens;
				} while (tokens[0].code != 0);

			}

			smart_str_appendl(&error_str, "Syntax error, unexpected token ", sizeof("Syntax error, unexpected token ") - 1);
			if (!token_name) {
				smart_str_appendl(&error_str, "UNKNOWN", sizeof("UNKNOWN") - 1);
			} else {
				smart_str_appendl(&error_str, token_name, token_len);
			}
			if (status->token->value) {
				smart_str_appendc(&error_str, '(');
				smart_str_appendl(&error_str, status->token->value, status->token->len);
				smart_str_appendc(&error_str, ')');
			}
			smart_str_appendl(&error_str, " in ", sizeof(" in ") - 1);
			smart_str_appendl(&error_str, Z_STRVAL_P(status->scanner_state->active_file), Z_STRLEN_P(status->scanner_state->active_file));
			smart_str_appendl(&error_str, " on line ", sizeof(" on line ") - 1);
			{
				char stmp[MAX_LENGTH_OF_LONG + 1];
				int str_len;
				str_len = slprintf(stmp, sizeof(stmp), "%ld", status->scanner_state->active_line);
				smart_str_appendl(&error_str, stmp, str_len);
			}

		} else {

			smart_str_appendl(&error_str, "Syntax error, unexpected EOF in ", sizeof("Syntax error, unexpected EOF in ") - 1);
			smart_str_appendl(&error_str, Z_STRVAL_P(status->scanner_state->active_file), Z_STRLEN_P(status->scanner_state->active_file));

			/* Report unclosed 'if' blocks */
			if ((status->scanner_state->if_level + status->scanner_state->old_if_level) > 0) {
				if ((status->scanner_state->if_level + status->scanner_state->old_if_level) == 1) {
					smart_str_appendl(&error_str, ", there is one 'if' block without close", sizeof(", there is one 'if' block without close") - 1);
				} else {
					smart_str_appendl(&error_str, ", there are ", sizeof(", there are ") - 1);
					{
						char stmp[MAX_LENGTH_OF_LONG + 1];
						int str_len;
						str_len = slprintf(stmp, sizeof(stmp), "%ld", status->scanner_state->if_level + status->scanner_state->old_if_level);
						smart_str_appendl(&error_str, stmp, str_len);
					}
					smart_str_appendl(&error_str, " 'if' blocks without close", sizeof(" 'if' blocks without close") - 1);
				}
			}

			/* Report unclosed 'for' blocks */
			if (status->scanner_state->for_level > 0) {
				if (status->scanner_state->for_level == 1) {
					smart_str_appendl(&error_str, ", there is one 'for' block without close", sizeof(", there is one 'for' block without close") - 1);
				} else {
					smart_str_appendl(&error_str, ", there are ", sizeof(", there are ") - 1);
					{
						char stmp[MAX_LENGTH_OF_LONG + 1];
						int str_len;
						str_len = slprintf(stmp, sizeof(stmp), "%ld", status->scanner_state->if_level);
						smart_str_appendl(&error_str, stmp, str_len);
					}
					smart_str_appendl(&error_str, " 'for' blocks without close", sizeof(" 'for' blocks without close") - 1);
				}
			}
		}

		smart_str_0(&error_str);

		if (error_str.s) {
			status->syntax_error = estrndup(error_str.s->val, error_str.s->len);
			status->syntax_error_len = error_str.s->len;
		} else {
			status->syntax_error = NULL;
		}
		smart_str_free(&error_str);

	}

	status->status = PHVOLT_PARSING_FAILED;
/* #line 3537 "parser.c" */
/************ End %syntax_error code ******************************************/
  phvolt_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  phvolt_ARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/
/*********** End %parse_accept code *******************************************/
  phvolt_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "phvolt_Alloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void phvolt_(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  phvolt_TOKENTYPE yyminor       /* The value for the token */
  phvolt_ARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  unsigned int yyact;   /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      yyStackOverflow(yypParser);
      return;
    }
#endif
    yypParser->yyidx = 0;
#ifndef YYNOERRORRECOVERY
    yypParser->yyerrcnt = -1;
#endif
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sInitialize. Empty stack. State 0\n",
              yyTracePrompt);
    }
#endif
  }
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif
  phvolt_ARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput '%s'\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact <= YY_MAX_SHIFTREDUCE ){
      if( yyact > YY_MAX_SHIFT ) yyact += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
      yy_shift(yypParser,yyact,yymajor,yyminor);
#ifndef YYNOERRORRECOVERY
      yypParser->yyerrcnt--;
#endif
      yymajor = YYNOCODE;
    }else if( yyact <= YY_MAX_REDUCE ){
      yy_reduce(yypParser,yyact-YY_MIN_REDUCE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
      yyminorunion.yy0 = yyminor;
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminor);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor, &yyminorunion);
        yymajor = YYNOCODE;
      }else{
        while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YY_MIN_REDUCE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          yy_shift(yypParser,yyact,YYERRORSYMBOL,yyminor);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor, yyminor);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor, yyminor);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
#ifndef NDEBUG
  if( yyTraceFILE ){
    int i;
    fprintf(yyTraceFILE,"%sReturn. Stack=",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE,"%c%s", i==1 ? '[' : ' ', 
              yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"]\n");
  }
#endif
  return;
}
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

const phvolt_token_names phvolt_tokens[] =
{
  { SL("INTEGER"),        PHVOLT_T_INTEGER },
  { SL("DOUBLE"),         PHVOLT_T_DOUBLE },
  { SL("STRING"),         PHVOLT_T_STRING },
  { SL("IDENTIFIER"),     PHVOLT_T_IDENTIFIER },
  { SL("MINUS"),          PHVOLT_T_MINUS },
  { SL("+"),              PHVOLT_T_ADD },
  { SL("-"),              PHVOLT_T_SUB },
  { SL("*"),              PHVOLT_T_MUL },
  { SL("/"),              PHVOLT_T_DIV },
  { SL("%%"),             PHVOLT_T_MOD },
  { SL("!"),              PHVOLT_T_NOT },
  { SL("~"),              PHVOLT_T_CONCAT },
  { SL("AND"),            PHVOLT_T_AND },
  { SL("OR"),             PHVOLT_T_OR },
  { SL("DOT"),            PHVOLT_T_DOT },
  { SL("COMMA"),          PHVOLT_T_COMMA },
  { SL("EQUALS"),         PHVOLT_T_EQUALS },
  { SL("NOT EQUALS"),     PHVOLT_T_NOTEQUALS },
  { SL("IDENTICAL"),      PHVOLT_T_IDENTICAL },
  { SL("NOT IDENTICAL"),  PHVOLT_T_NOTIDENTICAL },
  { SL("NOT"),            PHVOLT_T_NOT },
  { SL("RANGE"),          PHVOLT_T_RANGE },
  { SL("COLON"),          PHVOLT_T_COLON },
  { SL("QUESTION MARK"),  PHVOLT_T_QUESTION },
  { SL("<"),              PHVOLT_T_LESS },
  { SL("<="),             PHVOLT_T_LESSEQUAL },
  { SL(">"),              PHVOLT_T_GREATER },
  { SL(">="),             PHVOLT_T_GREATEREQUAL },
  { SL("("),              PHVOLT_T_PARENTHESES_OPEN },
  { SL(")"),              PHVOLT_T_PARENTHESES_CLOSE },
  { SL("["),              PHVOLT_T_SBRACKET_OPEN },
  { SL("]"),              PHVOLT_T_SBRACKET_CLOSE },
  { SL("{"),              PHVOLT_T_CBRACKET_OPEN },
  { SL("}"),              PHVOLT_T_CBRACKET_CLOSE },
  { SL("{%"),             PHVOLT_T_OPEN_DELIMITER },
  { SL("%}"),             PHVOLT_T_CLOSE_DELIMITER },
  { SL("{{"),             PHVOLT_T_OPEN_EDELIMITER },
  { SL("}}"),             PHVOLT_T_CLOSE_EDELIMITER },
  { SL("IF"),             PHVOLT_T_IF },
  { SL("ELSE"),           PHVOLT_T_ELSE },
  { SL("ELSEIF"),         PHVOLT_T_ELSEIF },
  { SL("ELSEFOR"),        PHVOLT_T_ELSEFOR },
  { SL("ENDIF"),          PHVOLT_T_ENDIF },
  { SL("FOR"),            PHVOLT_T_FOR },
  { SL("IN"),             PHVOLT_T_IN },
  { SL("ENDFOR"),         PHVOLT_T_ENDFOR },
  { SL("SET"),            PHVOLT_T_SET },
  { SL("ASSIGN"),         PHVOLT_T_ASSIGN },
  { SL("+="),             PHVOLT_T_ADD_ASSIGN },
  { SL("-="),             PHVOLT_T_SUB_ASSIGN },
  { SL("*="),             PHVOLT_T_MUL_ASSIGN },
  { SL("/="),             PHVOLT_T_DIV_ASSIGN },
  { SL("++"),             PHVOLT_T_INCR },
  { SL("--"),             PHVOLT_T_DECR },
  { SL("BLOCK"),          PHVOLT_T_BLOCK },
  { SL("ENDBLOCK"),       PHVOLT_T_ENDBLOCK },
  { SL("CACHE"),          PHVOLT_T_CACHE },
  { SL("ENDCACHE"),       PHVOLT_T_ENDCACHE },
  { SL("EXTENDS"),        PHVOLT_T_EXTENDS },
  { SL("IS"),             PHVOLT_T_IS },
  { SL("DEFINED"),        PHVOLT_T_DEFINED },
  { SL("EMPTY"),          PHVOLT_T_EMPTY },
  { SL("EVEN"),           PHVOLT_T_EVEN },
  { SL("ODD"),            PHVOLT_T_ODD },
  { SL("NUMERIC"),        PHVOLT_T_NUMERIC },
  { SL("SCALAR"),         PHVOLT_T_SCALAR },
  { SL("ITERABLE"),       PHVOLT_T_ITERABLE },
  { SL("INCLUDE"),        PHVOLT_T_INCLUDE },
  { SL("DO"),             PHVOLT_T_DO },
  { SL("WHITESPACE"),     PHVOLT_T_IGNORE },
  { SL("AUTOESCAPE"),     PHVOLT_T_AUTOESCAPE },
  { SL("ENDAUTOESCAPE"),  PHVOLT_T_ENDAUTOESCAPE },
  { SL("CONTINUE"),       PHVOLT_T_CONTINUE },
  { SL("BREAK"),          PHVOLT_T_BREAK },
  { SL("WITH"),           PHVOLT_T_WITH },
  { SL("RETURN"),         PHVOLT_T_RETURN },
  { SL("MACRO"),          PHVOLT_T_MACRO },
  { SL("ENDMACRO"),       PHVOLT_T_ENDMACRO },
  { SL("CALL"),           PHVOLT_T_CALL },
  { SL("WITH"),           PHVOLT_T_WITH },
  { NULL, 0, 0 }
};

/**
 * Wrapper to alloc memory within the parser
 */
static void *phvolt_wrapper_alloc(size_t bytes){
	return emalloc(bytes);
}

/**
 * Wrapper to free memory within the parser
 */
static void phvolt_wrapper_free(void *pointer){
	efree(pointer);
}

/**
 * Creates a parser_token to be passed to the parser
 */
static void phvolt_parse_with_token(void* phvolt_parser, int opcode, int parsercode, phvolt_scanner_token *token, phvolt_parser_status *parser_status){

	phvolt_parser_token *pToken;

	pToken = emalloc(sizeof(phvolt_parser_token));
	pToken->opcode = opcode;
	pToken->token = token->value;
	pToken->token_len = token->len;
	pToken->free_flag = 1;

	phvolt_(phvolt_parser, parsercode, pToken, parser_status);

	token->value = NULL;
	token->len = 0;
}

/**
 * Creates an error message
 */
static void phvolt_create_error_msg(phvolt_parser_status *parser_status, char *message){

	unsigned int length = (128 + Z_STRLEN_P(parser_status->scanner_state->active_file));
	char *str = emalloc(sizeof(char) * length);

	snprintf(str, length, "%s in %s on line %d", message, Z_STRVAL_P(parser_status->scanner_state->active_file), parser_status->scanner_state->active_line);
	str[length - 1] = '\0';

	parser_status->syntax_error = str;
}

/**
 * Creates an error message when it's triggered by the scanner
 */
static void phvolt_scanner_error_msg(phvolt_parser_status *parser_status, zval *error_msg){

	char *error, *error_part;
	int length;
	phvolt_scanner_state *state = parser_status->scanner_state;

	if (state->start) {
		error = emalloc(sizeof(char) * 72 + state->start_length +  Z_STRLEN_P(state->active_file));
		if (state->start_length > 16) {
			length = 72 + Z_STRLEN_P(state->active_file);
			error_part = estrndup(state->start, 16);
			snprintf(error, length, "Scanning error before '%s...' in %s on line %d", error_part, Z_STRVAL_P(state->active_file), state->active_line);
			error[length - 1] = '\0';
		} else {
			length = 48 + state->start_length + Z_STRLEN_P(state->active_file);
			snprintf(error, length, "Scanning error before '%s' in %s on line %d", state->start, Z_STRVAL_P(state->active_file), state->active_line);
		}
	} else {
		error = emalloc(sizeof(char) * (32 + Z_STRLEN_P(state->active_file)));
		length = 32 + Z_STRLEN_P(state->active_file);
		snprintf(error, length, "Scanning error near to EOF in %s", Z_STRVAL_P(state->active_file));
	}

	error[length - 1] = '\0';
	ZVAL_STRING(error_msg, error);
	efree(error);
}

/**
 * Receives the volt code tokenizes and parses it
 */
int phvolt_parse_view(zval *result, zval *view_code, zval *template_path){

	zval error_msg = {};

	ZVAL_NULL(result);

	if (Z_TYPE_P(view_code) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "View code must be a string");
		return FAILURE;
	}

	if (phvolt_internal_parse_view(result, view_code, template_path, &error_msg) == FAILURE) {
		if (likely(Z_TYPE(error_msg) > IS_NULL)) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, Z_STRVAL(error_msg));
			PHALCON_PTR_DTOR(&error_msg);
		} else {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, "Error parsing the view");
		}

		return FAILURE;
	}

	return SUCCESS;
}

/**
 * Checks whether the token has only blank characters
 */
int phvolt_is_blank_string(phvolt_scanner_token *token){

	char *marker = token->value;
	int i;

	for (i = 0; i < token->len; i++) {
		char ch = *marker;
		if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r' && ch != 11) {
			return 0;
		}
		marker++;
	}

	return 1;
}

/**
 * Parses a volt template returning an intermediate array representation
 */
int phvolt_internal_parse_view(zval *result, zval *view_code, zval *template_path, zval *error_msg) {

	char *error;
	phvolt_scanner_state *state;
	phvolt_scanner_token token;
	int scanner_status, status = SUCCESS;
	phvolt_parser_status *parser_status = NULL;
	void* phvolt_parser;

	/** Check if the view has code */
	if (!Z_STRVAL_P(view_code)) {
		ZVAL_STRING(error_msg, "View code cannot be null");
		return FAILURE;
	}

	if (!Z_STRLEN_P(view_code)) {
		array_init(result);
		return SUCCESS;
	}

	/** Start the reentrant parser */
	phvolt_parser = phvolt_Alloc(phvolt_wrapper_alloc);
	if (unlikely(!phvolt_parser)) {
		ZVAL_STRING(error_msg, "Memory allocation error");
		return FAILURE;
	}

	parser_status = emalloc(sizeof(phvolt_parser_status));
	state = emalloc(sizeof(phvolt_scanner_state));

	parser_status->status = PHVOLT_PARSING_OK;
	parser_status->scanner_state = state;
	parser_status->ret = NULL;
	parser_status->token = &token;
	parser_status->syntax_error = NULL;

	/** Initialize the scanner state */
	state->active_token = 0;
	state->start = Z_STRVAL_P(view_code);
	state->mode = PHVOLT_MODE_RAW;
	state->raw_buffer = emalloc(sizeof(char) * PHVOLT_RAW_BUFFER_SIZE);
	state->raw_buffer_size = PHVOLT_RAW_BUFFER_SIZE;
	state->raw_buffer_cursor = 0;
	state->active_file = template_path;
	state->active_line = 1;
	state->statement_position = 0;
	state->extends_mode = 0;
	state->block_level = 0;
	state->macro_level = 0;
	state->start_length = 0;
	state->old_if_level = 0;
	state->if_level = 0;
	state->for_level = 0;
	state->whitespace_control = 0;

	state->end = state->start;

	token.value = NULL;
	token.len = 0;

	while (0 <= (scanner_status = phvolt_get_token(state, &token))) {

		state->active_token = token.opcode;

		state->start_length = (Z_STRVAL_P(view_code) + Z_STRLEN_P(view_code) - state->start);

		switch (token.opcode) {

			case PHVOLT_T_IGNORE:
				break;

			case PHVOLT_T_ADD:
				phvolt_(phvolt_parser, PHVOLT_PLUS, NULL, parser_status);
				break;
			case PHVOLT_T_SUB:
				phvolt_(phvolt_parser, PHVOLT_MINUS, NULL, parser_status);
				break;
			case PHVOLT_T_MUL:
				phvolt_(phvolt_parser, PHVOLT_TIMES, NULL, parser_status);
				break;
			case PHVOLT_T_DIV:
				phvolt_(phvolt_parser, PHVOLT_DIVIDE, NULL, parser_status);
				break;
			case PHVOLT_T_MOD:
				phvolt_(phvolt_parser, PHVOLT_MOD, NULL, parser_status);
				break;
			case PHVOLT_T_AND:
				phvolt_(phvolt_parser, PHVOLT_AND, NULL, parser_status);
				break;
			case PHVOLT_T_OR:
				phvolt_(phvolt_parser, PHVOLT_OR, NULL, parser_status);
				break;
			case PHVOLT_T_IS:
				phvolt_(phvolt_parser, PHVOLT_IS, NULL, parser_status);
				break;
			case PHVOLT_T_EQUALS:
				phvolt_(phvolt_parser, PHVOLT_EQUALS, NULL, parser_status);
				break;
			case PHVOLT_T_NOTEQUALS:
				phvolt_(phvolt_parser, PHVOLT_NOTEQUALS, NULL, parser_status);
				break;
			case PHVOLT_T_LESS:
				phvolt_(phvolt_parser, PHVOLT_LESS, NULL, parser_status);
				break;
			case PHVOLT_T_GREATER:
				phvolt_(phvolt_parser, PHVOLT_GREATER, NULL, parser_status);
				break;
			case PHVOLT_T_GREATEREQUAL:
				phvolt_(phvolt_parser, PHVOLT_GREATEREQUAL, NULL, parser_status);
				break;
			case PHVOLT_T_LESSEQUAL:
				phvolt_(phvolt_parser, PHVOLT_LESSEQUAL, NULL, parser_status);
				break;
			case PHVOLT_T_IDENTICAL:
				phvolt_(phvolt_parser, PHVOLT_IDENTICAL, NULL, parser_status);
				break;
			case PHVOLT_T_NOTIDENTICAL:
				phvolt_(phvolt_parser, PHVOLT_NOTIDENTICAL, NULL, parser_status);
				break;
			case PHVOLT_T_NOT:
				phvolt_(phvolt_parser, PHVOLT_NOT, NULL, parser_status);
				break;
			case PHVOLT_T_DOT:
				phvolt_(phvolt_parser, PHVOLT_DOT, NULL, parser_status);
				break;
			case PHVOLT_T_CONCAT:
				phvolt_(phvolt_parser, PHVOLT_CONCAT, NULL, parser_status);
				break;
			case PHVOLT_T_RANGE:
				phvolt_(phvolt_parser, PHVOLT_RANGE, NULL, parser_status);
				break;
			case PHVOLT_T_PIPE:
				phvolt_(phvolt_parser, PHVOLT_PIPE, NULL, parser_status);
				break;
			case PHVOLT_T_COMMA:
				phvolt_(phvolt_parser, PHVOLT_COMMA, NULL, parser_status);
				break;
			case PHVOLT_T_COLON:
				phvolt_(phvolt_parser, PHVOLT_COLON, NULL, parser_status);
				break;
			case PHVOLT_T_QUESTION:
				phvolt_(phvolt_parser, PHVOLT_QUESTION, NULL, parser_status);
				break;

			case PHVOLT_T_PARENTHESES_OPEN:
				phvolt_(phvolt_parser, PHVOLT_PARENTHESES_OPEN, NULL, parser_status);
				break;
			case PHVOLT_T_PARENTHESES_CLOSE:
				phvolt_(phvolt_parser, PHVOLT_PARENTHESES_CLOSE, NULL, parser_status);
				break;
			case PHVOLT_T_SBRACKET_OPEN:
				phvolt_(phvolt_parser, PHVOLT_SBRACKET_OPEN, NULL, parser_status);
				break;
			case PHVOLT_T_SBRACKET_CLOSE:
				phvolt_(phvolt_parser, PHVOLT_SBRACKET_CLOSE, NULL, parser_status);
				break;
			case PHVOLT_T_CBRACKET_OPEN:
				phvolt_(phvolt_parser, PHVOLT_CBRACKET_OPEN, NULL, parser_status);
				break;
			case PHVOLT_T_CBRACKET_CLOSE:
				phvolt_(phvolt_parser, PHVOLT_CBRACKET_CLOSE, NULL, parser_status);
				break;

			case PHVOLT_T_OPEN_DELIMITER:
				phvolt_(phvolt_parser, PHVOLT_OPEN_DELIMITER, NULL, parser_status);
				break;
			case PHVOLT_T_CLOSE_DELIMITER:
				phvolt_(phvolt_parser, PHVOLT_CLOSE_DELIMITER, NULL, parser_status);
				break;

			case PHVOLT_T_OPEN_EDELIMITER:
				if (state->extends_mode == 1 && state->block_level == 0) {
					phvolt_create_error_msg(parser_status, "Child templates only may contain blocks");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				}
				phvolt_(phvolt_parser, PHVOLT_OPEN_EDELIMITER, NULL, parser_status);
				break;
			case PHVOLT_T_CLOSE_EDELIMITER:
				phvolt_(phvolt_parser, PHVOLT_CLOSE_EDELIMITER, NULL, parser_status);
				break;

			case PHVOLT_T_NULL:
				phvolt_(phvolt_parser, PHVOLT_NULL, NULL, parser_status);
				break;
			case PHVOLT_T_TRUE:
				phvolt_(phvolt_parser, PHVOLT_TRUE, NULL, parser_status);
				break;
			case PHVOLT_T_FALSE:
				phvolt_(phvolt_parser, PHVOLT_FALSE, NULL, parser_status);
				break;

			case PHVOLT_T_INTEGER:
				phvolt_parse_with_token(phvolt_parser, PHVOLT_T_INTEGER, PHVOLT_INTEGER, &token, parser_status);
				break;
			case PHVOLT_T_DOUBLE:
				phvolt_parse_with_token(phvolt_parser, PHVOLT_T_DOUBLE, PHVOLT_DOUBLE, &token, parser_status);
				break;
			case PHVOLT_T_STRING:
				phvolt_parse_with_token(phvolt_parser, PHVOLT_T_STRING, PHVOLT_STRING, &token, parser_status);
				break;
			case PHVOLT_T_IDENTIFIER:
				phvolt_parse_with_token(phvolt_parser, PHVOLT_T_IDENTIFIER, PHVOLT_IDENTIFIER, &token, parser_status);
				break;

			case PHVOLT_T_IF:
				if (state->extends_mode == 1 && state->block_level == 0){
					phvolt_create_error_msg(parser_status, "Child templates only may contain blocks");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				} else {
					state->if_level++;
					state->block_level++;
				}
				phvolt_(phvolt_parser, PHVOLT_IF, NULL, parser_status);
				break;

			case PHVOLT_T_ELSE:
				if (state->if_level == 0 && state->for_level > 0) {
					phvolt_(phvolt_parser, PHVOLT_ELSEFOR, NULL, parser_status);
				} else {
					phvolt_(phvolt_parser, PHVOLT_ELSE, NULL, parser_status);
				}
				break;

			case PHVOLT_T_ELSEFOR:
				phvolt_(phvolt_parser, PHVOLT_ELSEFOR, NULL, parser_status);
				break;

			case PHVOLT_T_ELSEIF:
				if (state->if_level == 0) {
					phvolt_create_error_msg(parser_status, "Unexpected ENDIF");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				}
				phvolt_(phvolt_parser, PHVOLT_ELSEIF, NULL, parser_status);
				break;

			case PHVOLT_T_ENDIF:
				state->block_level--;
				state->if_level--;
				phvolt_(phvolt_parser, PHVOLT_ENDIF, NULL, parser_status);
				break;

			case PHVOLT_T_FOR:
				if (state->extends_mode == 1 && state->block_level == 0){
					phvolt_create_error_msg(parser_status, "Child templates only may contain blocks");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				} else {
					state->old_if_level = state->if_level;
					state->if_level = 0;
					state->for_level++;
					state->block_level++;
				}
				phvolt_(phvolt_parser, PHVOLT_FOR, NULL, parser_status);
				break;

			case PHVOLT_T_IN:
				phvolt_(phvolt_parser, PHVOLT_IN, NULL, parser_status);
				break;

			case PHVOLT_T_ENDFOR:
				state->block_level--;
				state->for_level--;
				state->if_level = state->old_if_level;
				phvolt_(phvolt_parser, PHVOLT_ENDFOR, NULL, parser_status);
				break;

			case PHVOLT_T_RAW_FRAGMENT:
				if (token.len > 0) {
					if (state->extends_mode == 1 && state->block_level == 0){
						if (!phvolt_is_blank_string(&token)) {
							phvolt_create_error_msg(parser_status, "Child templates only may contain blocks");
							parser_status->status = PHVOLT_PARSING_FAILED;
						}
						efree(token.value);
						break;
					} else {
						if (!phvolt_is_blank_string(&token)) {
							state->statement_position++;
						}
					}
					phvolt_parse_with_token(phvolt_parser, PHVOLT_T_RAW_FRAGMENT, PHVOLT_RAW_FRAGMENT, &token, parser_status);
				} else {
					efree(token.value);
				}
				break;

			case PHVOLT_T_SET:
				if (state->extends_mode == 1 && state->block_level == 0){
					phvolt_create_error_msg(parser_status, "Child templates only may contain blocks");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				}
				phvolt_(phvolt_parser, PHVOLT_SET, NULL, parser_status);
				break;
			case PHVOLT_T_ASSIGN:
				phvolt_(phvolt_parser, PHVOLT_ASSIGN, NULL, parser_status);
				break;
			case PHVOLT_T_ADD_ASSIGN:
				phvolt_(phvolt_parser, PHVOLT_ADD_ASSIGN, NULL, parser_status);
				break;
			case PHVOLT_T_SUB_ASSIGN:
				phvolt_(phvolt_parser, PHVOLT_SUB_ASSIGN, NULL, parser_status);
				break;
			case PHVOLT_T_MUL_ASSIGN:
				phvolt_(phvolt_parser, PHVOLT_MUL_ASSIGN, NULL, parser_status);
				break;
			case PHVOLT_T_DIV_ASSIGN:
				phvolt_(phvolt_parser, PHVOLT_DIV_ASSIGN, NULL, parser_status);
				break;

			case PHVOLT_T_INCR:
				phvolt_(phvolt_parser, PHVOLT_INCR, NULL, parser_status);
				break;
			case PHVOLT_T_DECR:
				phvolt_(phvolt_parser, PHVOLT_DECR, NULL, parser_status);
				break;

			case PHVOLT_T_BLOCK:
				if (state->block_level > 0) {
					phvolt_create_error_msg(parser_status, "Embedding blocks into other blocks is not supported");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				} else {
					state->block_level++;
				}
				phvolt_(phvolt_parser, PHVOLT_BLOCK, NULL, parser_status);
				break;
			case PHVOLT_T_ENDBLOCK:
				state->block_level--;
				phvolt_(phvolt_parser, PHVOLT_ENDBLOCK, NULL, parser_status);
				break;

			case PHVOLT_T_MACRO:
				if (state->macro_level > 0) {
					phvolt_create_error_msg(parser_status, "Embedding macros into other macros is not allowed");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				} else {
					state->macro_level++;
				}
				phvolt_(phvolt_parser, PHVOLT_MACRO, NULL, parser_status);
				break;
			case PHVOLT_T_ENDMACRO:
				state->macro_level--;
				phvolt_(phvolt_parser, PHVOLT_ENDMACRO, NULL, parser_status);
				break;

			case PHVOLT_T_CALL:
				phvolt_(phvolt_parser, PHVOLT_CALL, NULL, parser_status);
				break;
			case PHVOLT_T_ENDCALL:
				phvolt_(phvolt_parser, PHVOLT_ENDCALL, NULL, parser_status);
				break;

			case PHVOLT_T_CACHE:
				phvolt_(phvolt_parser, PHVOLT_CACHE, NULL, parser_status);
				break;
			case PHVOLT_T_ENDCACHE:
				phvolt_(phvolt_parser, PHVOLT_ENDCACHE, NULL, parser_status);
				break;

			case PHVOLT_T_INCLUDE:
				phvolt_(phvolt_parser, PHVOLT_INCLUDE, NULL, parser_status);
				break;

			case PHVOLT_T_WITH:
				phvolt_(phvolt_parser, PHVOLT_WITH, NULL, parser_status);
				break;

			case PHVOLT_T_DEFINED:
				phvolt_(phvolt_parser, PHVOLT_DEFINED, NULL, parser_status);
				break;

			case PHVOLT_T_EMPTY:
				phvolt_(phvolt_parser, PHVOLT_EMPTY, NULL, parser_status);
				break;

			case PHVOLT_T_EVEN:
				phvolt_(phvolt_parser, PHVOLT_EVEN, NULL, parser_status);
				break;

			case PHVOLT_T_ODD:
				phvolt_(phvolt_parser, PHVOLT_ODD, NULL, parser_status);
				break;

			case PHVOLT_T_NUMERIC:
				phvolt_(phvolt_parser, PHVOLT_NUMERIC, NULL, parser_status);
				break;

			case PHVOLT_T_SCALAR:
				phvolt_(phvolt_parser, PHVOLT_SCALAR, NULL, parser_status);
				break;

			case PHVOLT_T_ITERABLE:
				phvolt_(phvolt_parser, PHVOLT_ITERABLE, NULL, parser_status);
				break;

			case PHVOLT_T_DO:
				phvolt_(phvolt_parser, PHVOLT_DO, NULL, parser_status);
				break;
			case PHVOLT_T_RETURN:
				phvolt_(phvolt_parser, PHVOLT_RETURN, NULL, parser_status);
				break;

			case PHVOLT_T_AUTOESCAPE:
				phvolt_(phvolt_parser, PHVOLT_AUTOESCAPE, NULL, parser_status);
				break;

			case PHVOLT_T_ENDAUTOESCAPE:
				phvolt_(phvolt_parser, PHVOLT_ENDAUTOESCAPE, NULL, parser_status);
				break;

			case PHVOLT_T_BREAK:
				phvolt_(phvolt_parser, PHVOLT_BREAK, NULL, parser_status);
				break;

			case PHVOLT_T_CONTINUE:
				phvolt_(phvolt_parser, PHVOLT_CONTINUE, NULL, parser_status);
				break;

			case PHVOLT_T_EXTENDS:
				if (state->statement_position != 1) {
					phvolt_create_error_msg(parser_status, "Extends statement must be placed at the first line in the template");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				} else {
					state->extends_mode = 1;
				}
				phvolt_(phvolt_parser, PHVOLT_EXTENDS, NULL, parser_status);
				break;

			default:
				parser_status->status = PHVOLT_PARSING_FAILED;
				if (Z_TYPE_P(error_msg) <= IS_NULL) {
					error = emalloc(sizeof(char) * (48 + Z_STRLEN_P(state->active_file)));
					snprintf(error, 48 + Z_STRLEN_P(state->active_file) + state->active_line, "Scanner: unknown opcode %d on in %s line %d", token.opcode, Z_STRVAL_P(state->active_file), state->active_line);
					ZVAL_STRING(error_msg, error);
					efree(error);
				}
				break;
		}

		if (parser_status->status != PHVOLT_PARSING_OK) {
			status = FAILURE;
			break;
		}

		state->end = state->start;
	}

	if (status != FAILURE) {
		switch (scanner_status) {
			case PHVOLT_SCANNER_RETCODE_ERR:
			case PHVOLT_SCANNER_RETCODE_IMPOSSIBLE:
				if (Z_TYPE_P(error_msg) <= IS_NULL) {
					phvolt_scanner_error_msg(parser_status, error_msg);
				}
				status = FAILURE;
				break;
			default:
				phvolt_(phvolt_parser, 0, NULL, parser_status);
		}
	}

	state->active_token = 0;
	state->start = NULL;
	efree(state->raw_buffer);

	if (parser_status->status != PHVOLT_PARSING_OK) {
		status = FAILURE;
		if (parser_status->syntax_error) {
			if (Z_TYPE_P(error_msg) <= IS_NULL) {
				ZVAL_STRING(error_msg, parser_status->syntax_error);
			}
			efree(parser_status->syntax_error);
		}
	}

	phvolt_Free(phvolt_parser, phvolt_wrapper_free);

	if (status != FAILURE) {
		if (parser_status->status == PHVOLT_PARSING_OK) {
			if (parser_status->ret) {
				ZVAL_COPY(result, parser_status->ret);
				efree(parser_status->ret);
				parser_status->ret = NULL;
			} else {
				array_init(result);
			}
		}
	}

	efree(parser_status);
	efree(state);

	return status;
}
