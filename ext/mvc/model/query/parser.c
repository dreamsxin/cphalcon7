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
/* #line 58 "parser.y" */


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

static void phql_ret_func_call2(zval *ret, char *name, zval *arguments, zval *distinct)
{
	array_init_size(ret, 4);
	add_assoc_long(ret, ISV(type), PHQL_T_FCALL);
	add_assoc_stringl(ret, ISV(name), name, strlen(name));

	if (arguments && Z_TYPE_P(arguments) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(arguments), arguments);
	}

	if (distinct && Z_TYPE_P(distinct) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(distinct), distinct);
	}
}

/* #line 427 "parser.c" */
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
**    phql_TOKENTYPE     is the data type used for minor type for terminal
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
**                       which is phql_TOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    phql_ARG_SDECL     A static variable declaration for the %extra_argument
**    phql_ARG_PDECL     A parameter declaration for the %extra_argument
**    phql_ARG_STORE     Code to store %extra_argument into yypParser
**    phql_ARG_FETCH     Code to extract %extra_argument from yypParser
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
#define YYNOCODE 145
#define YYACTIONTYPE unsigned short int
#define phql_TOKENTYPE phql_parser_token*
typedef union {
  int yyinit;
  phql_TOKENTYPE yy0;
  zval yy26;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define phql_ARG_SDECL phql_parser_status *status;
#define phql_ARG_PDECL ,phql_parser_status *status
#define phql_ARG_FETCH phql_parser_status *status = yypParser->status
#define phql_ARG_STORE yypParser->status = status
#define YYNSTATE             224
#define YYNRULE              170
#define YY_MAX_SHIFT         223
#define YY_MIN_SHIFTREDUCE   326
#define YY_MAX_SHIFTREDUCE   495
#define YY_MIN_REDUCE        496
#define YY_MAX_REDUCE        665
#define YY_ERROR_ACTION      666
#define YY_ACCEPT_ACTION     667
#define YY_NO_ACTION         668
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
#define YY_ACTTAB_COUNT (919)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    35,   34,   50,   49,   48,   47,   46,   39,   45,   44,
 /*    10 */    43,   42,   41,   40,   55,   54,   38,   36,   53,   52,
 /*    20 */    51,   57,   58,   56,   59,   61,  163,   33,  219,  146,
 /*    30 */    35,   34,   50,   49,   48,   47,   46,   39,   45,   44,
 /*    40 */    43,   42,   41,   40,   55,   54,   38,   36,   53,   52,
 /*    50 */    51,   57,   58,   56,   59,   61,  163,   33,  219,  146,
 /*    60 */   186,  362,  185,  156,   69,  155,  154,  148,  341,  372,
 /*    70 */   181,  150,  186,  362,  185,  156,  202,  155,  154,   28,
 /*    80 */   101,   24,  385,   35,   34,   50,   49,   48,   47,   46,
 /*    90 */    39,   45,   44,   43,   42,   41,   40,   55,   54,   38,
 /*   100 */    36,   53,   52,   51,   57,   58,   56,   59,   61,  163,
 /*   110 */    33,  219,  146,   35,   34,   50,   49,   48,   47,   46,
 /*   120 */    39,   45,   44,   43,   42,   41,   40,   55,   54,   38,
 /*   130 */    36,   53,   52,   51,   57,   58,   56,   59,   61,  163,
 /*   140 */    33,  219,  146,   83,  459,  400,  401,   55,   54,   38,
 /*   150 */    36,   53,   52,   51,   57,   58,   56,   59,   61,  163,
 /*   160 */    33,  219,  146,  480,  157,   35,   34,   50,   49,   48,
 /*   170 */    47,   46,   39,   45,   44,   43,   42,   41,   40,   55,
 /*   180 */    54,   38,   36,   53,   52,   51,   57,   58,   56,   59,
 /*   190 */    61,  163,   33,  219,  146,   35,   34,   50,   49,   48,
 /*   200 */    47,   46,   39,   45,   44,   43,   42,   41,   40,   55,
 /*   210 */    54,   38,   36,   53,   52,   51,   57,   58,   56,   59,
 /*   220 */    61,  163,   33,  219,  146,   38,   36,   53,   52,   51,
 /*   230 */    57,   58,   56,   59,   61,  163,   33,  219,  146,   59,
 /*   240 */    61,  163,   33,  219,  146,   27,   53,   52,   51,   57,
 /*   250 */    58,   56,   59,   61,  163,   33,  219,  146,   57,   58,
 /*   260 */    56,   59,   61,  163,   33,  219,  146,  417,  418,  419,
 /*   270 */   420,  197,  373,   35,   34,   50,   49,   48,   47,   46,
 /*   280 */    39,   45,   44,   43,   42,   41,   40,   55,   54,   38,
 /*   290 */    36,   53,   52,   51,   57,   58,   56,   59,   61,  163,
 /*   300 */    33,  219,  146,  332,  347,  349,   75,   62,  348,  349,
 /*   310 */    75,  457,   28,  200,   24,   35,   34,   50,   49,   48,
 /*   320 */    47,   46,   39,   45,   44,   43,   42,   41,   40,   55,
 /*   330 */    54,   38,   36,   53,   52,   51,   57,   58,   56,   59,
 /*   340 */    61,  163,   33,  219,  146,   50,   49,   48,   47,   46,
 /*   350 */    39,   45,   44,   43,   42,   41,   40,   55,   54,   38,
 /*   360 */    36,   53,   52,   51,   57,   58,   56,   59,   61,  163,
 /*   370 */    33,  219,  146,  472,   86,  103,   60,  384,   84,  389,
 /*   380 */    26,   25,   85,  161,  458,   86,  337,  102,  158,  145,
 /*   390 */   372,  173,  481,  385,   68,  195,  166,  193,  216,  398,
 /*   400 */     7,  209,  211,  159,  481,  372,  462,   86,  667,  223,
 /*   410 */   327,  222,  329,  330,  101,  174,  376,  167,  462,  101,
 /*   420 */   482,  483,  489,  490,  203,  201,  198,  213,   29,  215,
 /*   430 */   153,  113,  117,  486,  484,  485,  487,  488,  491,   90,
 /*   440 */    91,  147,  180,  369,  472,   88,  101,   60,  481,  481,
 /*   450 */   113,   26,   25,  333,  334,  336,  102,   72,  104,  151,
 /*   460 */   145,  172,  462,  462,  381,  471,  195,  481,  193,   63,
 /*   470 */   177,    7,  169,  481,  101,  481,  415,   63,  190,  207,
 /*   480 */   385,  462,  474,  191,  471,  110,  113,  462,  385,  462,
 /*   490 */   388,  482,  483,  489,  490,  203,  201,  198,  160,   29,
 /*   500 */   372,  413,  481,  481,  486,  484,  485,  487,  488,  491,
 /*   510 */   113,  404,   37,   21,  152,  338,  462,  462,   60,  113,
 /*   520 */   471,   94,   26,   25,  175,  209,  211,  481,  176,  110,
 /*   530 */   412,  144,   64,  352,  214,  187,  481,  195,  190,  193,
 /*   540 */   164,  462,    7,  192,  471,  481,  481,  190,  219,  146,
 /*   550 */   462,  213,  194,  471,  178,  405,  383,   84,  389,  462,
 /*   560 */   462,    6,  482,  483,  489,  490,  203,  201,  198,  113,
 /*   570 */    29,  339,   14,  117,   20,  486,  484,  485,  487,  488,
 /*   580 */   491,   79,  149,   22,  369,  380,  481,  494,  117,   60,
 /*   590 */   481,  343,  374,   26,   25,  204,   63,  165,   86,  369,
 /*   600 */   462,  466,  145,  470,  462,  481,   87,  385,  195,  108,
 /*   610 */   193,   13,  117,    7,   89,  344,  382,   84,  389,  462,
 /*   620 */    63,  361,   32,  368,  182,    3,  481,  378,   19,  481,
 /*   630 */     4,  385,  204,  482,  483,  489,  490,  203,  201,  198,
 /*   640 */   462,   29,   91,  462,  207,  367,  486,  484,  485,  487,
 /*   650 */   488,  491,  168,  377,  167,    5,   45,   44,   43,   42,
 /*   660 */    41,   40,   55,   54,   38,   36,   53,   52,   51,   57,
 /*   670 */    58,   56,   59,   61,  163,   33,  219,  146,   60,  103,
 /*   680 */   371,  207,   26,   25,  163,   33,  219,  146,  109,  111,
 /*   690 */   112,  145,   95,  350,  218,   19,  481,  195,  188,  193,
 /*   700 */   189,   15,    7,  397,  331,  481,  481,  481,  359,  171,
 /*   710 */   462,  183,  366,  357,   82,  481,  184,  481,  449,  462,
 /*   720 */   462,  462,  482,  483,  489,  490,  203,  201,  198,  462,
 /*   730 */    29,  462,   81,  114,  217,  486,  484,  485,  487,  488,
 /*   740 */   491,  105,   97,   70,  106,  107,  115,   96,  162,  118,
 /*   750 */   481,  116,   23,  133,   98,   99,  179,  100,  481,   66,
 /*   760 */   481,  481,  481,  481,  462,  481,  481,   80,  481,  220,
 /*   770 */   481,  134,  462,  119,  462,  462,  462,  462,  125,  462,
 /*   780 */   462,  126,  462,   16,  462,   71,  481,    8,  481,  127,
 /*   790 */   481,   67,  170,  128,  129,  481,  130,  120,  481,  121,
 /*   800 */   462,  122,  462,    9,  462,   15,  481,  452,   73,  462,
 /*   810 */   481,  481,  462,  481,  481,    1,  481,  123,  481,  146,
 /*   820 */   462,  451,  448,   10,  462,  462,   17,  462,  462,  124,
 /*   830 */   462,  360,  462,  135,  481,  136,  137,  131,  132,  138,
 /*   840 */   139,  140,  450,  340,  358,  142,  481,   15,  462,  356,
 /*   850 */   481,  355,  481,  481,  481,  481,  481,  481,  481,  354,
 /*   860 */   462,  143,  481,  141,  462,   95,  462,  462,  462,  462,
 /*   870 */   462,  462,  462,   95,   95,  351,  462,  465,  481,  464,
 /*   880 */   481,   19,  392,   92,  196,  463,  456,   93,   30,  199,
 /*   890 */   391,  390,  462,  455,  462,  453,   31,  494,  365,   74,
 /*   900 */   492,   76,  205,  206,   77,   78,   65,  208,  475,   18,
 /*   910 */     2,  210,  212,   12,  409,  221,  496,  498,   11,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*    10 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*    20 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /*    30 */     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*    40 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*    50 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /*    60 */    42,   43,   44,   45,   34,   47,   48,  119,   39,  121,
 /*    70 */    41,  115,   42,   43,   44,   45,   93,   47,   48,   80,
 /*    80 */    97,   82,  126,    1,    2,    3,    4,    5,    6,    7,
 /*    90 */     8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
 /*   100 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*   110 */    28,   29,   30,    1,    2,    3,    4,    5,    6,    7,
 /*   120 */     8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
 /*   130 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*   140 */    28,   29,   30,  138,  139,   63,   64,   15,   16,   17,
 /*   150 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*   160 */    28,   29,   30,   51,   39,    1,    2,    3,    4,    5,
 /*   170 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   180 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*   190 */    26,   27,   28,   29,   30,    1,    2,    3,    4,    5,
 /*   200 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   210 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*   220 */    26,   27,   28,   29,   30,   17,   18,   19,   20,   21,
 /*   230 */    22,   23,   24,   25,   26,   27,   28,   29,   30,   25,
 /*   240 */    26,   27,   28,   29,   30,   81,   19,   20,   21,   22,
 /*   250 */    23,   24,   25,   26,   27,   28,   29,   30,   22,   23,
 /*   260 */    24,   25,   26,   27,   28,   29,   30,   70,   71,   72,
 /*   270 */    73,   77,   39,    1,    2,    3,    4,    5,    6,    7,
 /*   280 */     8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
 /*   290 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*   300 */    28,   29,   30,  107,  112,  113,  114,  111,  112,  113,
 /*   310 */   114,   79,   80,   41,   82,    1,    2,    3,    4,    5,
 /*   320 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   330 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*   340 */    26,   27,   28,   29,   30,    3,    4,    5,    6,    7,
 /*   350 */     8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
 /*   360 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*   370 */    28,   29,   30,   23,   35,  109,   26,  129,  130,  131,
 /*   380 */    30,   31,  115,  105,  139,   35,  108,  109,  119,   39,
 /*   390 */   121,   52,  126,  126,   55,   45,   57,   47,  132,  133,
 /*   400 */    50,   32,   33,  119,  126,  121,  140,   35,   91,   92,
 /*   410 */    93,   94,   95,   96,   97,   93,  125,  126,  140,   97,
 /*   420 */    70,   71,   72,   73,   74,   75,   76,   58,   78,   30,
 /*   430 */    40,  109,  109,   83,   84,   85,   86,   87,   88,  122,
 /*   440 */    50,  118,   93,  120,   23,  128,   97,   26,  126,  126,
 /*   450 */   109,   30,   31,   37,   38,  108,  109,   50,  109,  137,
 /*   460 */    39,   54,  140,  140,  110,  143,   45,  126,   47,  115,
 /*   470 */    93,   50,  110,  126,   97,  126,  136,  115,  137,   89,
 /*   480 */   126,  140,   83,  142,  143,  109,  109,  140,  126,  140,
 /*   490 */   131,   70,   71,   72,   73,   74,   75,   76,  119,   78,
 /*   500 */   121,  136,  126,  126,   83,   84,   85,   86,   87,   88,
 /*   510 */   109,  135,   17,   18,  137,   23,  140,  140,   26,  109,
 /*   520 */   143,  116,   30,   31,   29,   32,   33,  126,  109,  109,
 /*   530 */   136,   39,   39,   39,   41,   41,  126,   45,  137,   47,
 /*   540 */   136,  140,   50,  142,  143,  126,  126,  137,   29,   30,
 /*   550 */   140,   58,  142,  143,  134,  135,  129,  130,  131,  140,
 /*   560 */   140,  104,   70,   71,   72,   73,   74,   75,   76,  109,
 /*   570 */    78,   23,   34,  109,   36,   83,   84,   85,   86,   87,
 /*   580 */    88,   68,  118,   60,  120,  123,  126,   39,  109,   26,
 /*   590 */   126,  110,  123,   30,   31,   40,  115,  118,   35,  120,
 /*   600 */   140,   37,   39,  143,  140,  126,   98,  126,   45,  109,
 /*   610 */    47,  106,  109,   50,   98,  110,  129,  130,  131,  140,
 /*   620 */   115,   43,   49,  120,   46,  141,  126,  127,   34,  126,
 /*   630 */   141,  126,   40,   70,   71,   72,   73,   74,   75,   76,
 /*   640 */   140,   78,   50,  140,   89,   51,   83,   84,   85,   86,
 /*   650 */    87,   88,  124,  125,  126,  141,    9,   10,   11,   12,
 /*   660 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   670 */    23,   24,   25,   26,   27,   28,   29,   30,   26,  109,
 /*   680 */   121,   89,   30,   31,   27,   28,   29,   30,  109,  109,
 /*   690 */   109,   39,   34,  117,   67,   34,  126,   45,  109,   47,
 /*   700 */   109,   34,   50,  133,  103,  126,  126,  126,   43,   51,
 /*   710 */   140,   46,   51,   43,   68,  126,   46,  126,   51,  140,
 /*   720 */   140,  140,   70,   71,   72,   73,   74,   75,   76,  140,
 /*   730 */    78,  140,   34,  109,   61,   83,   84,   85,   86,   87,
 /*   740 */    88,  109,  101,  109,  109,  109,  109,  102,  109,  109,
 /*   750 */   126,  109,   66,  109,  100,   99,   65,   98,  126,   36,
 /*   760 */   126,  126,  126,  126,  140,  126,  126,   69,  126,  109,
 /*   770 */   126,  109,  140,  109,  140,  140,  140,  140,  109,  140,
 /*   780 */   140,  109,  140,    3,  140,   34,  126,   50,  126,  109,
 /*   790 */   126,   56,   54,  109,  109,  126,  109,  109,  126,  109,
 /*   800 */   140,  109,  140,   50,  140,   34,  126,   51,   53,  140,
 /*   810 */   126,  126,  140,  126,  126,   50,  126,  109,  126,   30,
 /*   820 */   140,   51,   51,   62,  140,  140,   34,  140,  140,  109,
 /*   830 */   140,   43,  140,  109,  126,  109,  109,  109,  109,  109,
 /*   840 */   109,  109,   51,   39,   43,  109,  126,   34,  140,   43,
 /*   850 */   126,   43,  126,  126,  126,  126,  126,  126,  126,   43,
 /*   860 */   140,  109,  126,  109,  140,   34,  140,  140,  140,  140,
 /*   870 */   140,  140,  140,   34,   34,   39,  140,   51,  126,   51,
 /*   880 */   126,   34,   51,   50,   39,   51,   51,   50,   50,   39,
 /*   890 */    51,   51,  140,   51,  140,   51,   50,   39,   51,   50,
 /*   900 */    39,   50,   40,   39,   50,   50,   39,   59,   83,   34,
 /*   910 */    50,   59,   59,   50,   55,   34,    0,  144,   62,
};
#define YY_SHIFT_USE_DFLT (-2)
#define YY_SHIFT_COUNT (223)
#define YY_SHIFT_MIN   (-1)
#define YY_SHIFT_MAX   (916)
static const short yy_shift_ofst[] = {
 /*     0 */   339,  350,  350,  421,  421,  421,  492,  563,  652,  652,
 /*    10 */   652,  652,  652,   30,  492,  421,  652,  652,  652,  652,
 /*    20 */   125,  652,  652,  652,  652,  652,  652,  652,  652,  652,
 /*    30 */   652,  652,  652,  652,  652,  652,  652,  652,  652,  652,
 /*    40 */   652,  652,  652,  652,  652,  652,  652,  652,  652,  652,
 /*    50 */   652,  652,  652,  652,  652,  652,  652,  652,  652,  652,
 /*    60 */   652,  652,   18,  493,  369,  369,  125,  125,  125,  125,
 /*    70 */    -1,  125,  233,  125,  372,  125,  233,  233,  233,  197,
 /*    80 */   197,  197,  197,  232,  369,  494,  416,  513,  523,  513,
 /*    90 */   523,  564,  564,  564,  573,  233,  627,  646,  673,  686,
 /*   100 */   691,  523,   29,   82,  112,  164,  194,  272,  314,  314,
 /*   110 */   314,  314,  314,  314,  314,  314,  314,  314,  342,  647,
 /*   120 */   647,  647,  647,  647,  647,  132,  132,  132,  132,  132,
 /*   130 */   132,  208,  208,  227,  227,  236,  236,  236,  214,  214,
 /*   140 */   214,  657,  657,  657,  390,  592,  495,  594,  658,  661,
 /*   150 */   407,  667,  771,  548,  578,  665,  670,  555,  831,  839,
 /*   160 */   840,  538,  519,  399,  698,  847,  723,  780,  751,  735,
 /*   170 */   737,  738,  753,  755,  756,  765,  789,  770,  792,  761,
 /*   180 */   791,  804,  788,  801,  806,  808,  816,  836,  789,  789,
 /*   190 */   813,  826,  828,  833,  834,  837,  835,  845,  838,  842,
 /*   200 */   850,  846,  844,  849,  858,  861,  862,  864,  851,  848,
 /*   210 */   854,  852,  855,  853,  867,  825,  875,  856,  859,  860,
 /*   220 */   789,  863,  881,  916,
};
#define YY_REDUCE_USE_DFLT (-53)
#define YY_REDUCE_COUNT (101)
#define YY_REDUCE_MIN   (-52)
#define YY_REDUCE_MAX   (754)
static const short yy_reduce_ofst[] = {
 /*     0 */   317,  322,  377,  341,  401,  410,  278,  349,  323,  464,
 /*    10 */   420,  266,  479,  196,  347,  460,  500,  376,  570,  503,
 /*    20 */   505,  419,  579,  580,  581,  589,  591,  624,  632,  634,
 /*    30 */   635,  636,  637,  639,  640,  642,  644,  660,  662,  664,
 /*    40 */   669,  672,  680,  684,  685,  687,  688,  690,  692,  708,
 /*    50 */   720,  724,  726,  727,  728,  729,  730,  731,  732,  736,
 /*    60 */   752,  754,  192,  248,  427,  487,  354,  528,  362,  481,
 /*    70 */     5,  291,  -52,  -44,  -17,  267,  269,  284,  379,  340,
 /*    80 */   365,  394,  404,  245,  359,  405,  457,  462,  508,  469,
 /*    90 */   516,  484,  489,  514,  576,  559,  601,  645,  641,  654,
 /*   100 */   656,  659,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   666,  666,  666,  639,  639,  639,  666,  666,  666,  666,
 /*    10 */   666,  666,  666,  516,  666,  666,  666,  666,  666,  666,
 /*    20 */   666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
 /*    30 */   666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
 /*    40 */   666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
 /*    50 */   666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
 /*    60 */   666,  666,  515,  557,  557,  557,  666,  666,  666,  666,
 /*    70 */   666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
 /*    80 */   666,  666,  666,  666,  556,  523,  505,  586,  564,  586,
 /*    90 */   564,  637,  637,  637,  534,  666,  580,  584,  566,  578,
 /*   100 */   573,  564,  512,  569,  666,  666,  666,  666,  549,  563,
 /*   110 */   576,  577,  631,  643,  630,  533,  624,  540,  646,  613,
 /*   120 */   606,  605,  604,  603,  602,  612,  611,  610,  609,  608,
 /*   130 */   607,  598,  597,  616,  614,  601,  600,  599,  596,  595,
 /*   140 */   594,  592,  593,  591,  665,  665,  666,  666,  666,  666,
 /*   150 */   666,  666,  666,  666,  666,  666,  666,  665,  666,  666,
 /*   160 */   666,  666,  647,  666,  581,  666,  666,  666,  545,  666,
 /*   170 */   666,  666,  666,  666,  666,  666,  617,  666,  572,  666,
 /*   180 */   666,  666,  666,  666,  666,  666,  666,  666,  649,  648,
 /*   190 */   638,  666,  666,  666,  666,  666,  666,  666,  666,  666,
 /*   200 */   666,  666,  666,  666,  666,  666,  663,  666,  666,  666,
 /*   210 */   666,  666,  666,  666,  666,  666,  565,  666,  666,  666,
 /*   220 */   615,  666,  498,  666,
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
  phql_ARG_SDECL                /* A place to hold %extra_argument */
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
void phql_Trace(FILE *TraceFILE, char *zTracePrompt){
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
  "$",             "AGAINST",       "BETWEEN",       "EQUALS",      
  "NOTEQUALS",     "LESS",          "GREATER",       "GREATEREQUAL",
  "LESSEQUAL",     "TS_MATCHES",    "TS_OR",         "TS_AND",      
  "TS_NEGATE",     "TS_CONTAINS_ANOTHER",  "TS_CONTAINS_IN",  "AND",         
  "OR",            "LIKE",          "ILIKE",         "BITWISE_AND", 
  "BITWISE_OR",    "BITWISE_XOR",   "DIVIDE",        "TIMES",       
  "MOD",           "PLUS",          "MINUS",         "IS",          
  "DOUBLECOLON",   "IN",            "NOT",           "BITWISE_NOT", 
  "FORCE",         "USE",           "COMMA",         "SELECT",      
  "FROM",          "DISTINCT",      "ALL",           "IDENTIFIER",  
  "DOT",           "AS",            "INNER",         "JOIN",        
  "CROSS",         "LEFT",          "OUTER",         "RIGHT",       
  "FULL",          "ON",            "PARENTHESES_OPEN",  "PARENTHESES_CLOSE",
  "INSERT",        "INTO",          "VALUES",        "UPDATE",      
  "SET",           "DELETE",        "IGNORE",        "INDEX",       
  "WHERE",         "ORDER",         "BY",            "ASC",         
  "DESC",          "GROUP",         "HAVING",        "FOR",         
  "LIMIT",         "OFFSET",        "INTEGER",       "HINTEGER",    
  "NPLACEHOLDER",  "SPLACEHOLDER",  "EXISTS",        "CAST",        
  "CONVERT",       "USING",         "CASE",          "END",         
  "WHEN",          "THEN",          "ELSE",          "NULL",        
  "STRING",        "DOUBLE",        "TRUE",          "FALSE",       
  "BPLACEHOLDER",  "COLON",         "error",         "program",     
  "query_language",  "select_statement",  "insert_statement",  "update_statement",
  "delete_statement",  "select_clause",  "where_clause",  "group_clause",
  "having_clause",  "order_clause",  "select_limit_clause",  "for_update_clause",
  "distinct_all",  "column_list",   "associated_name_list",  "join_list_or_null",
  "column_item",   "expr",          "associated_name",  "join_list",   
  "join_item",     "join_clause",   "join_type",     "aliased_or_qualified_name",
  "join_associated_name",  "join_conditions",  "values_list",   "field_list",  
  "value_item",    "field_item",    "update_clause",  "limit_clause",
  "update_item_list",  "update_item",   "qualified_name",  "new_value",   
  "delete_clause",  "index_hints_or_null",  "index_hints",   "index_hint",  
  "order_list",    "order_item",    "group_list",    "group_item",  
  "integer_or_placeholder",  "argument_list",  "when_clauses",  "when_clause", 
  "function_call",  "distinct_or_null",  "argument_list_or_null",  "argument_item",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "program ::= query_language",
 /*   1 */ "query_language ::= select_statement",
 /*   2 */ "query_language ::= insert_statement",
 /*   3 */ "query_language ::= update_statement",
 /*   4 */ "query_language ::= delete_statement",
 /*   5 */ "select_statement ::= select_clause where_clause group_clause having_clause order_clause select_limit_clause for_update_clause",
 /*   6 */ "select_clause ::= SELECT distinct_all column_list FROM associated_name_list join_list_or_null",
 /*   7 */ "distinct_all ::= DISTINCT",
 /*   8 */ "distinct_all ::= ALL",
 /*   9 */ "distinct_all ::=",
 /*  10 */ "column_list ::= column_list COMMA column_item",
 /*  11 */ "column_list ::= column_item",
 /*  12 */ "column_item ::= TIMES",
 /*  13 */ "column_item ::= IDENTIFIER DOT TIMES",
 /*  14 */ "column_item ::= expr AS IDENTIFIER",
 /*  15 */ "column_item ::= expr IDENTIFIER",
 /*  16 */ "column_item ::= expr",
 /*  17 */ "associated_name_list ::= associated_name_list COMMA associated_name",
 /*  18 */ "associated_name_list ::= associated_name",
 /*  19 */ "join_list_or_null ::= join_list",
 /*  20 */ "join_list_or_null ::=",
 /*  21 */ "join_list ::= join_list join_item",
 /*  22 */ "join_list ::= join_item",
 /*  23 */ "join_item ::= join_clause",
 /*  24 */ "join_clause ::= join_type aliased_or_qualified_name join_associated_name join_conditions",
 /*  25 */ "join_associated_name ::= AS IDENTIFIER",
 /*  26 */ "join_associated_name ::= IDENTIFIER",
 /*  27 */ "join_associated_name ::=",
 /*  28 */ "join_type ::= INNER JOIN",
 /*  29 */ "join_type ::= CROSS JOIN",
 /*  30 */ "join_type ::= LEFT OUTER JOIN",
 /*  31 */ "join_type ::= LEFT JOIN",
 /*  32 */ "join_type ::= RIGHT OUTER JOIN",
 /*  33 */ "join_type ::= RIGHT JOIN",
 /*  34 */ "join_type ::= FULL OUTER JOIN",
 /*  35 */ "join_type ::= FULL JOIN",
 /*  36 */ "join_type ::= JOIN",
 /*  37 */ "join_conditions ::= ON expr",
 /*  38 */ "join_conditions ::=",
 /*  39 */ "insert_statement ::= insert_statement COMMA PARENTHESES_OPEN values_list PARENTHESES_CLOSE",
 /*  40 */ "insert_statement ::= INSERT INTO aliased_or_qualified_name VALUES PARENTHESES_OPEN values_list PARENTHESES_CLOSE",
 /*  41 */ "insert_statement ::= INSERT INTO aliased_or_qualified_name PARENTHESES_OPEN field_list PARENTHESES_CLOSE VALUES PARENTHESES_OPEN values_list PARENTHESES_CLOSE",
 /*  42 */ "values_list ::= values_list COMMA value_item",
 /*  43 */ "values_list ::= value_item",
 /*  44 */ "value_item ::= expr",
 /*  45 */ "field_list ::= field_list COMMA field_item",
 /*  46 */ "field_list ::= field_item",
 /*  47 */ "field_item ::= IDENTIFIER",
 /*  48 */ "update_statement ::= update_clause where_clause limit_clause",
 /*  49 */ "update_clause ::= UPDATE associated_name SET update_item_list",
 /*  50 */ "update_item_list ::= update_item_list COMMA update_item",
 /*  51 */ "update_item_list ::= update_item",
 /*  52 */ "update_item ::= qualified_name EQUALS new_value",
 /*  53 */ "new_value ::= expr",
 /*  54 */ "delete_statement ::= delete_clause where_clause limit_clause",
 /*  55 */ "delete_clause ::= DELETE FROM associated_name",
 /*  56 */ "associated_name ::= aliased_or_qualified_name AS IDENTIFIER index_hints_or_null",
 /*  57 */ "associated_name ::= aliased_or_qualified_name IDENTIFIER index_hints_or_null",
 /*  58 */ "associated_name ::= aliased_or_qualified_name index_hints_or_null",
 /*  59 */ "aliased_or_qualified_name ::= qualified_name",
 /*  60 */ "index_hints_or_null ::= index_hints",
 /*  61 */ "index_hints_or_null ::=",
 /*  62 */ "index_hints ::= index_hints index_hint",
 /*  63 */ "index_hints ::= index_hint",
 /*  64 */ "index_hint ::= IGNORE INDEX PARENTHESES_OPEN field_list PARENTHESES_CLOSE",
 /*  65 */ "index_hint ::= USE INDEX PARENTHESES_OPEN field_list PARENTHESES_CLOSE",
 /*  66 */ "index_hint ::= FORCE INDEX PARENTHESES_OPEN field_list PARENTHESES_CLOSE",
 /*  67 */ "where_clause ::= WHERE expr",
 /*  68 */ "where_clause ::=",
 /*  69 */ "order_clause ::= ORDER BY order_list",
 /*  70 */ "order_clause ::=",
 /*  71 */ "order_list ::= order_list COMMA order_item",
 /*  72 */ "order_list ::= order_item",
 /*  73 */ "order_item ::= expr",
 /*  74 */ "order_item ::= expr ASC",
 /*  75 */ "order_item ::= expr DESC",
 /*  76 */ "group_clause ::= GROUP BY group_list",
 /*  77 */ "group_clause ::=",
 /*  78 */ "group_list ::= group_list COMMA group_item",
 /*  79 */ "group_list ::= group_item",
 /*  80 */ "group_item ::= expr",
 /*  81 */ "having_clause ::= HAVING expr",
 /*  82 */ "having_clause ::=",
 /*  83 */ "for_update_clause ::= FOR UPDATE",
 /*  84 */ "for_update_clause ::=",
 /*  85 */ "select_limit_clause ::= LIMIT integer_or_placeholder",
 /*  86 */ "select_limit_clause ::= LIMIT integer_or_placeholder COMMA integer_or_placeholder",
 /*  87 */ "select_limit_clause ::= LIMIT integer_or_placeholder OFFSET integer_or_placeholder",
 /*  88 */ "select_limit_clause ::=",
 /*  89 */ "limit_clause ::= LIMIT integer_or_placeholder",
 /*  90 */ "limit_clause ::=",
 /*  91 */ "integer_or_placeholder ::= INTEGER",
 /*  92 */ "integer_or_placeholder ::= HINTEGER",
 /*  93 */ "integer_or_placeholder ::= NPLACEHOLDER",
 /*  94 */ "integer_or_placeholder ::= SPLACEHOLDER",
 /*  95 */ "expr ::= MINUS expr",
 /*  96 */ "expr ::= expr MINUS expr",
 /*  97 */ "expr ::= expr PLUS expr",
 /*  98 */ "expr ::= expr TIMES expr",
 /*  99 */ "expr ::= expr DIVIDE expr",
 /* 100 */ "expr ::= expr MOD expr",
 /* 101 */ "expr ::= expr AND expr",
 /* 102 */ "expr ::= expr OR expr",
 /* 103 */ "expr ::= expr BITWISE_AND expr",
 /* 104 */ "expr ::= expr BITWISE_OR expr",
 /* 105 */ "expr ::= expr BITWISE_XOR expr",
 /* 106 */ "expr ::= expr EQUALS expr",
 /* 107 */ "expr ::= expr NOTEQUALS expr",
 /* 108 */ "expr ::= expr LESS expr",
 /* 109 */ "expr ::= expr GREATER expr",
 /* 110 */ "expr ::= expr GREATEREQUAL expr",
 /* 111 */ "expr ::= expr TS_MATCHES expr",
 /* 112 */ "expr ::= expr TS_OR expr",
 /* 113 */ "expr ::= expr TS_AND expr",
 /* 114 */ "expr ::= expr TS_NEGATE expr",
 /* 115 */ "expr ::= expr TS_CONTAINS_ANOTHER expr",
 /* 116 */ "expr ::= expr TS_CONTAINS_IN expr",
 /* 117 */ "expr ::= expr LESSEQUAL expr",
 /* 118 */ "expr ::= expr LIKE expr",
 /* 119 */ "expr ::= expr NOT LIKE expr",
 /* 120 */ "expr ::= expr ILIKE expr",
 /* 121 */ "expr ::= expr NOT ILIKE expr",
 /* 122 */ "expr ::= expr IN PARENTHESES_OPEN argument_list PARENTHESES_CLOSE",
 /* 123 */ "expr ::= expr NOT IN PARENTHESES_OPEN argument_list PARENTHESES_CLOSE",
 /* 124 */ "expr ::= PARENTHESES_OPEN select_statement PARENTHESES_CLOSE",
 /* 125 */ "expr ::= expr IN PARENTHESES_OPEN select_statement PARENTHESES_CLOSE",
 /* 126 */ "expr ::= expr NOT IN PARENTHESES_OPEN select_statement PARENTHESES_CLOSE",
 /* 127 */ "expr ::= EXISTS PARENTHESES_OPEN select_statement PARENTHESES_CLOSE",
 /* 128 */ "expr ::= expr AGAINST expr",
 /* 129 */ "expr ::= CAST PARENTHESES_OPEN expr AS IDENTIFIER PARENTHESES_CLOSE",
 /* 130 */ "expr ::= CONVERT PARENTHESES_OPEN expr USING IDENTIFIER PARENTHESES_CLOSE",
 /* 131 */ "expr ::= CASE expr when_clauses END",
 /* 132 */ "when_clauses ::= when_clauses when_clause",
 /* 133 */ "when_clauses ::= when_clause",
 /* 134 */ "when_clause ::= WHEN expr THEN expr",
 /* 135 */ "when_clause ::= ELSE expr",
 /* 136 */ "expr ::= function_call",
 /* 137 */ "function_call ::= LEFT PARENTHESES_OPEN distinct_or_null argument_list_or_null PARENTHESES_CLOSE",
 /* 138 */ "function_call ::= RIGHT PARENTHESES_OPEN distinct_or_null argument_list_or_null PARENTHESES_CLOSE",
 /* 139 */ "function_call ::= IDENTIFIER PARENTHESES_OPEN distinct_or_null argument_list_or_null PARENTHESES_CLOSE",
 /* 140 */ "distinct_or_null ::= DISTINCT",
 /* 141 */ "distinct_or_null ::=",
 /* 142 */ "argument_list_or_null ::= argument_list",
 /* 143 */ "argument_list_or_null ::=",
 /* 144 */ "argument_list ::= argument_list COMMA argument_item",
 /* 145 */ "argument_list ::= argument_item",
 /* 146 */ "argument_item ::= TIMES",
 /* 147 */ "argument_item ::= expr",
 /* 148 */ "expr ::= expr IS NULL",
 /* 149 */ "expr ::= expr IS NOT NULL",
 /* 150 */ "expr ::= expr BETWEEN expr",
 /* 151 */ "expr ::= expr DOUBLECOLON expr",
 /* 152 */ "expr ::= NOT expr",
 /* 153 */ "expr ::= BITWISE_NOT expr",
 /* 154 */ "expr ::= PARENTHESES_OPEN expr PARENTHESES_CLOSE",
 /* 155 */ "expr ::= qualified_name",
 /* 156 */ "expr ::= INTEGER",
 /* 157 */ "expr ::= HINTEGER",
 /* 158 */ "expr ::= STRING",
 /* 159 */ "expr ::= DOUBLE",
 /* 160 */ "expr ::= NULL",
 /* 161 */ "expr ::= TRUE",
 /* 162 */ "expr ::= FALSE",
 /* 163 */ "expr ::= NPLACEHOLDER",
 /* 164 */ "expr ::= SPLACEHOLDER",
 /* 165 */ "expr ::= BPLACEHOLDER",
 /* 166 */ "qualified_name ::= IDENTIFIER COLON IDENTIFIER DOT IDENTIFIER",
 /* 167 */ "qualified_name ::= IDENTIFIER COLON IDENTIFIER",
 /* 168 */ "qualified_name ::= IDENTIFIER DOT IDENTIFIER",
 /* 169 */ "qualified_name ::= IDENTIFIER",
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
** second argument to phql_Alloc() below.  This can be changed by
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
** to phql_ and phql_Free.
*/
void *phql_Alloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
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
  phql_ARG_FETCH;
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
    case 1: /* AGAINST */
    case 2: /* BETWEEN */
    case 3: /* EQUALS */
    case 4: /* NOTEQUALS */
    case 5: /* LESS */
    case 6: /* GREATER */
    case 7: /* GREATEREQUAL */
    case 8: /* LESSEQUAL */
    case 9: /* TS_MATCHES */
    case 10: /* TS_OR */
    case 11: /* TS_AND */
    case 12: /* TS_NEGATE */
    case 13: /* TS_CONTAINS_ANOTHER */
    case 14: /* TS_CONTAINS_IN */
    case 15: /* AND */
    case 16: /* OR */
    case 17: /* LIKE */
    case 18: /* ILIKE */
    case 19: /* BITWISE_AND */
    case 20: /* BITWISE_OR */
    case 21: /* BITWISE_XOR */
    case 22: /* DIVIDE */
    case 23: /* TIMES */
    case 24: /* MOD */
    case 25: /* PLUS */
    case 26: /* MINUS */
    case 27: /* IS */
    case 28: /* DOUBLECOLON */
    case 29: /* IN */
    case 30: /* NOT */
    case 31: /* BITWISE_NOT */
    case 32: /* FORCE */
    case 33: /* USE */
    case 34: /* COMMA */
    case 35: /* SELECT */
    case 36: /* FROM */
    case 37: /* DISTINCT */
    case 38: /* ALL */
    case 39: /* IDENTIFIER */
    case 40: /* DOT */
    case 41: /* AS */
    case 42: /* INNER */
    case 43: /* JOIN */
    case 44: /* CROSS */
    case 45: /* LEFT */
    case 46: /* OUTER */
    case 47: /* RIGHT */
    case 48: /* FULL */
    case 49: /* ON */
    case 50: /* PARENTHESES_OPEN */
    case 51: /* PARENTHESES_CLOSE */
    case 52: /* INSERT */
    case 53: /* INTO */
    case 54: /* VALUES */
    case 55: /* UPDATE */
    case 56: /* SET */
    case 57: /* DELETE */
    case 58: /* IGNORE */
    case 59: /* INDEX */
    case 60: /* WHERE */
    case 61: /* ORDER */
    case 62: /* BY */
    case 63: /* ASC */
    case 64: /* DESC */
    case 65: /* GROUP */
    case 66: /* HAVING */
    case 67: /* FOR */
    case 68: /* LIMIT */
    case 69: /* OFFSET */
    case 70: /* INTEGER */
    case 71: /* HINTEGER */
    case 72: /* NPLACEHOLDER */
    case 73: /* SPLACEHOLDER */
    case 74: /* EXISTS */
    case 75: /* CAST */
    case 76: /* CONVERT */
    case 77: /* USING */
    case 78: /* CASE */
    case 79: /* END */
    case 80: /* WHEN */
    case 81: /* THEN */
    case 82: /* ELSE */
    case 83: /* NULL */
    case 84: /* STRING */
    case 85: /* DOUBLE */
    case 86: /* TRUE */
    case 87: /* FALSE */
    case 88: /* BPLACEHOLDER */
    case 89: /* COLON */
{
/* #line 22 "parser.y" */

	if ((yypminor->yy0)) {
		if ((yypminor->yy0)->free_flag) {
			efree((yypminor->yy0)->token);
		}
		efree((yypminor->yy0));
	}

/* #line 1344 "parser.c" */
}
      break;
      /* Default NON-TERMINAL Destructor */
    case 90: /* error */
    case 91: /* program */
    case 92: /* query_language */
    case 93: /* select_statement */
    case 94: /* insert_statement */
    case 95: /* update_statement */
    case 96: /* delete_statement */
    case 97: /* select_clause */
    case 98: /* where_clause */
    case 99: /* group_clause */
    case 100: /* having_clause */
    case 101: /* order_clause */
    case 102: /* select_limit_clause */
    case 103: /* for_update_clause */
    case 104: /* distinct_all */
    case 105: /* column_list */
    case 106: /* associated_name_list */
    case 107: /* join_list_or_null */
    case 108: /* column_item */
    case 109: /* expr */
    case 110: /* associated_name */
    case 111: /* join_list */
    case 112: /* join_item */
    case 113: /* join_clause */
    case 114: /* join_type */
    case 115: /* aliased_or_qualified_name */
    case 116: /* join_associated_name */
    case 117: /* join_conditions */
    case 118: /* values_list */
    case 119: /* field_list */
    case 120: /* value_item */
    case 121: /* field_item */
    case 122: /* update_clause */
    case 123: /* limit_clause */
    case 124: /* update_item_list */
    case 125: /* update_item */
    case 126: /* qualified_name */
    case 127: /* new_value */
    case 128: /* delete_clause */
    case 129: /* index_hints_or_null */
    case 130: /* index_hints */
    case 131: /* index_hint */
    case 132: /* order_list */
    case 133: /* order_item */
    case 134: /* group_list */
    case 135: /* group_item */
    case 136: /* integer_or_placeholder */
    case 137: /* argument_list */
    case 138: /* when_clauses */
    case 139: /* when_clause */
    case 140: /* function_call */
    case 141: /* distinct_or_null */
    case 142: /* argument_list_or_null */
    case 143: /* argument_item */
{
/* #line 31 "parser.y" */

	if (status) {
		// TODO:
	}
	if (&(yypminor->yy26)) {
		zval_ptr_dtor(&(yypminor->yy26));
	}

/* #line 1412 "parser.c" */
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
void phql_Free(
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
int phql_StackPeak(void *p){
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
   phql_ARG_FETCH;
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
   phql_ARG_STORE; /* Suppress warning about unused %extra_argument var */
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
  phql_TOKENTYPE yyMinor        /* The minor token to shift in */
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
  { 91, 1 },
  { 92, 1 },
  { 92, 1 },
  { 92, 1 },
  { 92, 1 },
  { 93, 7 },
  { 97, 6 },
  { 104, 1 },
  { 104, 1 },
  { 104, 0 },
  { 105, 3 },
  { 105, 1 },
  { 108, 1 },
  { 108, 3 },
  { 108, 3 },
  { 108, 2 },
  { 108, 1 },
  { 106, 3 },
  { 106, 1 },
  { 107, 1 },
  { 107, 0 },
  { 111, 2 },
  { 111, 1 },
  { 112, 1 },
  { 113, 4 },
  { 116, 2 },
  { 116, 1 },
  { 116, 0 },
  { 114, 2 },
  { 114, 2 },
  { 114, 3 },
  { 114, 2 },
  { 114, 3 },
  { 114, 2 },
  { 114, 3 },
  { 114, 2 },
  { 114, 1 },
  { 117, 2 },
  { 117, 0 },
  { 94, 5 },
  { 94, 7 },
  { 94, 10 },
  { 118, 3 },
  { 118, 1 },
  { 120, 1 },
  { 119, 3 },
  { 119, 1 },
  { 121, 1 },
  { 95, 3 },
  { 122, 4 },
  { 124, 3 },
  { 124, 1 },
  { 125, 3 },
  { 127, 1 },
  { 96, 3 },
  { 128, 3 },
  { 110, 4 },
  { 110, 3 },
  { 110, 2 },
  { 115, 1 },
  { 129, 1 },
  { 129, 0 },
  { 130, 2 },
  { 130, 1 },
  { 131, 5 },
  { 131, 5 },
  { 131, 5 },
  { 98, 2 },
  { 98, 0 },
  { 101, 3 },
  { 101, 0 },
  { 132, 3 },
  { 132, 1 },
  { 133, 1 },
  { 133, 2 },
  { 133, 2 },
  { 99, 3 },
  { 99, 0 },
  { 134, 3 },
  { 134, 1 },
  { 135, 1 },
  { 100, 2 },
  { 100, 0 },
  { 103, 2 },
  { 103, 0 },
  { 102, 2 },
  { 102, 4 },
  { 102, 4 },
  { 102, 0 },
  { 123, 2 },
  { 123, 0 },
  { 136, 1 },
  { 136, 1 },
  { 136, 1 },
  { 136, 1 },
  { 109, 2 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 3 },
  { 109, 4 },
  { 109, 3 },
  { 109, 4 },
  { 109, 5 },
  { 109, 6 },
  { 109, 3 },
  { 109, 5 },
  { 109, 6 },
  { 109, 4 },
  { 109, 3 },
  { 109, 6 },
  { 109, 6 },
  { 109, 4 },
  { 138, 2 },
  { 138, 1 },
  { 139, 4 },
  { 139, 2 },
  { 109, 1 },
  { 140, 5 },
  { 140, 5 },
  { 140, 5 },
  { 141, 1 },
  { 141, 0 },
  { 142, 1 },
  { 142, 0 },
  { 137, 3 },
  { 137, 1 },
  { 143, 1 },
  { 143, 1 },
  { 109, 3 },
  { 109, 4 },
  { 109, 3 },
  { 109, 3 },
  { 109, 2 },
  { 109, 2 },
  { 109, 3 },
  { 109, 1 },
  { 109, 1 },
  { 109, 1 },
  { 109, 1 },
  { 109, 1 },
  { 109, 1 },
  { 109, 1 },
  { 109, 1 },
  { 109, 1 },
  { 109, 1 },
  { 109, 1 },
  { 126, 5 },
  { 126, 3 },
  { 126, 3 },
  { 126, 1 },
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
  phql_ARG_FETCH;
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
      case 0: /* program ::= query_language */
/* #line 525 "parser.y" */
{
	ZVAL_ZVAL(&status->ret, &yymsp[0].minor.yy26, 1, 1);
}
/* #line 1890 "parser.c" */
        break;
      case 1: /* query_language ::= select_statement */
      case 2: /* query_language ::= insert_statement */ yytestcase(yyruleno==2);
      case 3: /* query_language ::= update_statement */ yytestcase(yyruleno==3);
      case 4: /* query_language ::= delete_statement */ yytestcase(yyruleno==4);
      case 18: /* associated_name_list ::= associated_name */ yytestcase(yyruleno==18);
      case 19: /* join_list_or_null ::= join_list */ yytestcase(yyruleno==19);
      case 22: /* join_list ::= join_item */ yytestcase(yyruleno==22);
      case 23: /* join_item ::= join_clause */ yytestcase(yyruleno==23);
      case 44: /* value_item ::= expr */ yytestcase(yyruleno==44);
      case 51: /* update_item_list ::= update_item */ yytestcase(yyruleno==51);
      case 53: /* new_value ::= expr */ yytestcase(yyruleno==53);
      case 59: /* aliased_or_qualified_name ::= qualified_name */ yytestcase(yyruleno==59);
      case 60: /* index_hints_or_null ::= index_hints */ yytestcase(yyruleno==60);
      case 72: /* order_list ::= order_item */ yytestcase(yyruleno==72);
      case 79: /* group_list ::= group_item */ yytestcase(yyruleno==79);
      case 80: /* group_item ::= expr */ yytestcase(yyruleno==80);
      case 136: /* expr ::= function_call */ yytestcase(yyruleno==136);
      case 142: /* argument_list_or_null ::= argument_list */ yytestcase(yyruleno==142);
      case 147: /* argument_item ::= expr */ yytestcase(yyruleno==147);
      case 155: /* expr ::= qualified_name */ yytestcase(yyruleno==155);
/* #line 529 "parser.y" */
{
	yylhsminor.yy26 = yymsp[0].minor.yy26;
}
/* #line 1916 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 5: /* select_statement ::= select_clause where_clause group_clause having_clause order_clause select_limit_clause for_update_clause */
/* #line 545 "parser.y" */
{
	phql_ret_select_statement(&yylhsminor.yy26, &yymsp[-6].minor.yy26, &yymsp[-5].minor.yy26, &yymsp[-2].minor.yy26, &yymsp[-4].minor.yy26, &yymsp[-3].minor.yy26, &yymsp[-1].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 1924 "parser.c" */
  yymsp[-6].minor.yy26 = yylhsminor.yy26;
        break;
      case 6: /* select_clause ::= SELECT distinct_all column_list FROM associated_name_list join_list_or_null */
{  yy_destructor(yypParser,35,&yymsp[-5].minor);
/* #line 549 "parser.y" */
{
	phql_ret_select_clause(&yymsp[-5].minor.yy26, &yymsp[-4].minor.yy26, &yymsp[-3].minor.yy26, &yymsp[-1].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 1933 "parser.c" */
  yy_destructor(yypParser,36,&yymsp[-2].minor);
}
        break;
      case 7: /* distinct_all ::= DISTINCT */
{  yy_destructor(yypParser,37,&yymsp[0].minor);
/* #line 553 "parser.y" */
{
	phql_ret_distinct_all(&yymsp[0].minor.yy26, 1);
}
/* #line 1943 "parser.c" */
}
        break;
      case 8: /* distinct_all ::= ALL */
{  yy_destructor(yypParser,38,&yymsp[0].minor);
/* #line 557 "parser.y" */
{
	phql_ret_distinct_all(&yymsp[0].minor.yy26, 0);
}
/* #line 1952 "parser.c" */
}
        break;
      case 9: /* distinct_all ::= */
      case 20: /* join_list_or_null ::= */ yytestcase(yyruleno==20);
      case 27: /* join_associated_name ::= */ yytestcase(yyruleno==27);
      case 38: /* join_conditions ::= */ yytestcase(yyruleno==38);
      case 61: /* index_hints_or_null ::= */ yytestcase(yyruleno==61);
      case 68: /* where_clause ::= */ yytestcase(yyruleno==68);
      case 70: /* order_clause ::= */ yytestcase(yyruleno==70);
      case 77: /* group_clause ::= */ yytestcase(yyruleno==77);
      case 82: /* having_clause ::= */ yytestcase(yyruleno==82);
      case 84: /* for_update_clause ::= */ yytestcase(yyruleno==84);
      case 88: /* select_limit_clause ::= */ yytestcase(yyruleno==88);
      case 90: /* limit_clause ::= */ yytestcase(yyruleno==90);
      case 141: /* distinct_or_null ::= */ yytestcase(yyruleno==141);
      case 143: /* argument_list_or_null ::= */ yytestcase(yyruleno==143);
/* #line 561 "parser.y" */
{
	ZVAL_UNDEF(&yymsp[1].minor.yy26);
}
/* #line 1973 "parser.c" */
        break;
      case 10: /* column_list ::= column_list COMMA column_item */
      case 17: /* associated_name_list ::= associated_name_list COMMA associated_name */ yytestcase(yyruleno==17);
      case 42: /* values_list ::= values_list COMMA value_item */ yytestcase(yyruleno==42);
      case 45: /* field_list ::= field_list COMMA field_item */ yytestcase(yyruleno==45);
      case 50: /* update_item_list ::= update_item_list COMMA update_item */ yytestcase(yyruleno==50);
      case 71: /* order_list ::= order_list COMMA order_item */ yytestcase(yyruleno==71);
      case 78: /* group_list ::= group_list COMMA group_item */ yytestcase(yyruleno==78);
      case 144: /* argument_list ::= argument_list COMMA argument_item */ yytestcase(yyruleno==144);
/* #line 565 "parser.y" */
{
	phql_ret_zval_list(&yylhsminor.yy26, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 1987 "parser.c" */
  yy_destructor(yypParser,34,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 11: /* column_list ::= column_item */
      case 43: /* values_list ::= value_item */ yytestcase(yyruleno==43);
      case 46: /* field_list ::= field_item */ yytestcase(yyruleno==46);
      case 133: /* when_clauses ::= when_clause */ yytestcase(yyruleno==133);
      case 145: /* argument_list ::= argument_item */ yytestcase(yyruleno==145);
/* #line 569 "parser.y" */
{
	phql_ret_zval_list(&yylhsminor.yy26, &yymsp[0].minor.yy26, NULL);
}
/* #line 2000 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 12: /* column_item ::= TIMES */
      case 146: /* argument_item ::= TIMES */ yytestcase(yyruleno==146);
{  yy_destructor(yypParser,23,&yymsp[0].minor);
/* #line 573 "parser.y" */
{
	phql_ret_column_item(&yymsp[0].minor.yy26, PHQL_T_STARALL, NULL, NULL, NULL);
}
/* #line 2010 "parser.c" */
}
        break;
      case 13: /* column_item ::= IDENTIFIER DOT TIMES */
/* #line 577 "parser.y" */
{
	phql_ret_column_item(&yylhsminor.yy26, PHQL_T_DOMAINALL, NULL, yymsp[-2].minor.yy0, NULL);
}
/* #line 2018 "parser.c" */
  yy_destructor(yypParser,40,&yymsp[-1].minor);
  yy_destructor(yypParser,23,&yymsp[0].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 14: /* column_item ::= expr AS IDENTIFIER */
/* #line 581 "parser.y" */
{
	phql_ret_column_item(&yylhsminor.yy26, PHQL_T_EXPR, &yymsp[-2].minor.yy26, NULL, yymsp[0].minor.yy0);
}
/* #line 2028 "parser.c" */
  yy_destructor(yypParser,41,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 15: /* column_item ::= expr IDENTIFIER */
/* #line 585 "parser.y" */
{
	phql_ret_column_item(&yylhsminor.yy26, PHQL_T_EXPR, &yymsp[-1].minor.yy26, NULL, yymsp[0].minor.yy0);
}
/* #line 2037 "parser.c" */
  yymsp[-1].minor.yy26 = yylhsminor.yy26;
        break;
      case 16: /* column_item ::= expr */
/* #line 589 "parser.y" */
{
	phql_ret_column_item(&yylhsminor.yy26, PHQL_T_EXPR, &yymsp[0].minor.yy26, NULL, NULL);
}
/* #line 2045 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 21: /* join_list ::= join_list join_item */
      case 132: /* when_clauses ::= when_clauses when_clause */ yytestcase(yyruleno==132);
/* #line 609 "parser.y" */
{
	phql_ret_zval_list(&yylhsminor.yy26, &yymsp[-1].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2054 "parser.c" */
  yymsp[-1].minor.yy26 = yylhsminor.yy26;
        break;
      case 24: /* join_clause ::= join_type aliased_or_qualified_name join_associated_name join_conditions */
/* #line 622 "parser.y" */
{
	phql_ret_join_item(&yylhsminor.yy26, &yymsp[-3].minor.yy26, &yymsp[-2].minor.yy26, &yymsp[-1].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2062 "parser.c" */
  yymsp[-3].minor.yy26 = yylhsminor.yy26;
        break;
      case 25: /* join_associated_name ::= AS IDENTIFIER */
{  yy_destructor(yypParser,41,&yymsp[-1].minor);
/* #line 626 "parser.y" */
{
	phql_ret_qualified_name(&yymsp[-1].minor.yy26, NULL, NULL, yymsp[0].minor.yy0);
}
/* #line 2071 "parser.c" */
}
        break;
      case 26: /* join_associated_name ::= IDENTIFIER */
      case 47: /* field_item ::= IDENTIFIER */ yytestcase(yyruleno==47);
      case 169: /* qualified_name ::= IDENTIFIER */ yytestcase(yyruleno==169);
/* #line 630 "parser.y" */
{
	phql_ret_qualified_name(&yylhsminor.yy26, NULL, NULL, yymsp[0].minor.yy0);
}
/* #line 2081 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 28: /* join_type ::= INNER JOIN */
{  yy_destructor(yypParser,42,&yymsp[-1].minor);
/* #line 638 "parser.y" */
{
	phql_ret_join_type(&yymsp[-1].minor.yy26, PHQL_T_INNERJOIN);
}
/* #line 2090 "parser.c" */
  yy_destructor(yypParser,43,&yymsp[0].minor);
}
        break;
      case 29: /* join_type ::= CROSS JOIN */
{  yy_destructor(yypParser,44,&yymsp[-1].minor);
/* #line 642 "parser.y" */
{
	phql_ret_join_type(&yymsp[-1].minor.yy26, PHQL_T_CROSSJOIN);
}
/* #line 2100 "parser.c" */
  yy_destructor(yypParser,43,&yymsp[0].minor);
}
        break;
      case 30: /* join_type ::= LEFT OUTER JOIN */
{  yy_destructor(yypParser,45,&yymsp[-2].minor);
/* #line 646 "parser.y" */
{
	phql_ret_join_type(&yymsp[-2].minor.yy26, PHQL_T_LEFTJOIN);
}
/* #line 2110 "parser.c" */
  yy_destructor(yypParser,46,&yymsp[-1].minor);
  yy_destructor(yypParser,43,&yymsp[0].minor);
}
        break;
      case 31: /* join_type ::= LEFT JOIN */
{  yy_destructor(yypParser,45,&yymsp[-1].minor);
/* #line 650 "parser.y" */
{
	phql_ret_join_type(&yymsp[-1].minor.yy26, PHQL_T_LEFTJOIN);
}
/* #line 2121 "parser.c" */
  yy_destructor(yypParser,43,&yymsp[0].minor);
}
        break;
      case 32: /* join_type ::= RIGHT OUTER JOIN */
{  yy_destructor(yypParser,47,&yymsp[-2].minor);
/* #line 654 "parser.y" */
{
	phql_ret_join_type(&yymsp[-2].minor.yy26, PHQL_T_RIGHTJOIN);
}
/* #line 2131 "parser.c" */
  yy_destructor(yypParser,46,&yymsp[-1].minor);
  yy_destructor(yypParser,43,&yymsp[0].minor);
}
        break;
      case 33: /* join_type ::= RIGHT JOIN */
{  yy_destructor(yypParser,47,&yymsp[-1].minor);
/* #line 658 "parser.y" */
{
	phql_ret_join_type(&yymsp[-1].minor.yy26, PHQL_T_RIGHTJOIN);
}
/* #line 2142 "parser.c" */
  yy_destructor(yypParser,43,&yymsp[0].minor);
}
        break;
      case 34: /* join_type ::= FULL OUTER JOIN */
{  yy_destructor(yypParser,48,&yymsp[-2].minor);
/* #line 662 "parser.y" */
{
	phql_ret_join_type(&yymsp[-2].minor.yy26, PHQL_T_FULLJOIN);
}
/* #line 2152 "parser.c" */
  yy_destructor(yypParser,46,&yymsp[-1].minor);
  yy_destructor(yypParser,43,&yymsp[0].minor);
}
        break;
      case 35: /* join_type ::= FULL JOIN */
{  yy_destructor(yypParser,48,&yymsp[-1].minor);
/* #line 666 "parser.y" */
{
	phql_ret_join_type(&yymsp[-1].minor.yy26, PHQL_T_FULLJOIN);
}
/* #line 2163 "parser.c" */
  yy_destructor(yypParser,43,&yymsp[0].minor);
}
        break;
      case 36: /* join_type ::= JOIN */
{  yy_destructor(yypParser,43,&yymsp[0].minor);
/* #line 670 "parser.y" */
{
	phql_ret_join_type(&yymsp[0].minor.yy26, PHQL_T_INNERJOIN);
}
/* #line 2173 "parser.c" */
}
        break;
      case 37: /* join_conditions ::= ON expr */
      case 67: /* where_clause ::= WHERE expr */ yytestcase(yyruleno==67);
      case 81: /* having_clause ::= HAVING expr */ yytestcase(yyruleno==81);
{  yy_destructor(yypParser,49,&yymsp[-1].minor);
/* #line 674 "parser.y" */
{
	yymsp[-1].minor.yy26 = yymsp[0].minor.yy26;
}
/* #line 2184 "parser.c" */
}
        break;
      case 39: /* insert_statement ::= insert_statement COMMA PARENTHESES_OPEN values_list PARENTHESES_CLOSE */
/* #line 683 "parser.y" */
{
	phql_ret_insert_statement2(&yylhsminor.yy26, &yymsp[-4].minor.yy26, &yymsp[-1].minor.yy26);
}
/* #line 2192 "parser.c" */
  yy_destructor(yypParser,34,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
  yymsp[-4].minor.yy26 = yylhsminor.yy26;
        break;
      case 40: /* insert_statement ::= INSERT INTO aliased_or_qualified_name VALUES PARENTHESES_OPEN values_list PARENTHESES_CLOSE */
{  yy_destructor(yypParser,52,&yymsp[-6].minor);
/* #line 687 "parser.y" */
{
	phql_ret_insert_statement(&yymsp[-6].minor.yy26, &yymsp[-4].minor.yy26, NULL, &yymsp[-1].minor.yy26);
}
/* #line 2204 "parser.c" */
  yy_destructor(yypParser,53,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 41: /* insert_statement ::= INSERT INTO aliased_or_qualified_name PARENTHESES_OPEN field_list PARENTHESES_CLOSE VALUES PARENTHESES_OPEN values_list PARENTHESES_CLOSE */
{  yy_destructor(yypParser,52,&yymsp[-9].minor);
/* #line 691 "parser.y" */
{
	phql_ret_insert_statement(&yymsp[-9].minor.yy26, &yymsp[-7].minor.yy26, &yymsp[-5].minor.yy26, &yymsp[-1].minor.yy26);
}
/* #line 2217 "parser.c" */
  yy_destructor(yypParser,53,&yymsp[-8].minor);
  yy_destructor(yypParser,50,&yymsp[-6].minor);
  yy_destructor(yypParser,51,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 48: /* update_statement ::= update_clause where_clause limit_clause */
/* #line 721 "parser.y" */
{
	phql_ret_update_statement(&yylhsminor.yy26, &yymsp[-2].minor.yy26, &yymsp[-1].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2231 "parser.c" */
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 49: /* update_clause ::= UPDATE associated_name SET update_item_list */
{  yy_destructor(yypParser,55,&yymsp[-3].minor);
/* #line 725 "parser.y" */
{
	phql_ret_update_clause(&yymsp[-3].minor.yy26, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2240 "parser.c" */
  yy_destructor(yypParser,56,&yymsp[-1].minor);
}
        break;
      case 52: /* update_item ::= qualified_name EQUALS new_value */
/* #line 737 "parser.y" */
{
	phql_ret_update_item(&yylhsminor.yy26, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2249 "parser.c" */
  yy_destructor(yypParser,3,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 54: /* delete_statement ::= delete_clause where_clause limit_clause */
/* #line 747 "parser.y" */
{
	phql_ret_delete_statement(&yylhsminor.yy26, &yymsp[-2].minor.yy26, &yymsp[-1].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2258 "parser.c" */
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 55: /* delete_clause ::= DELETE FROM associated_name */
{  yy_destructor(yypParser,57,&yymsp[-2].minor);
/* #line 751 "parser.y" */
{
	phql_ret_delete_clause(&yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2267 "parser.c" */
  yy_destructor(yypParser,36,&yymsp[-1].minor);
}
        break;
      case 56: /* associated_name ::= aliased_or_qualified_name AS IDENTIFIER index_hints_or_null */
/* #line 755 "parser.y" */
{
	phql_ret_assoc_name(&yylhsminor.yy26, &yymsp[-3].minor.yy26, yymsp[-1].minor.yy0, &yymsp[0].minor.yy26);
}
/* #line 2276 "parser.c" */
  yy_destructor(yypParser,41,&yymsp[-2].minor);
  yymsp[-3].minor.yy26 = yylhsminor.yy26;
        break;
      case 57: /* associated_name ::= aliased_or_qualified_name IDENTIFIER index_hints_or_null */
/* #line 759 "parser.y" */
{
	phql_ret_assoc_name(&yylhsminor.yy26, &yymsp[-2].minor.yy26, yymsp[-1].minor.yy0, &yymsp[0].minor.yy26);
}
/* #line 2285 "parser.c" */
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 58: /* associated_name ::= aliased_or_qualified_name index_hints_or_null */
/* #line 763 "parser.y" */
{
	phql_ret_assoc_name(&yylhsminor.yy26, &yymsp[-1].minor.yy26, NULL, &yymsp[0].minor.yy26);
}
/* #line 2293 "parser.c" */
  yymsp[-1].minor.yy26 = yylhsminor.yy26;
        break;
      case 62: /* index_hints ::= index_hints index_hint */
/* #line 779 "parser.y" */
{
	phql_ret_index_list(&yylhsminor.yy26, &yymsp[-1].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2301 "parser.c" */
  yymsp[-1].minor.yy26 = yylhsminor.yy26;
        break;
      case 63: /* index_hints ::= index_hint */
/* #line 783 "parser.y" */
{
	phql_ret_index_list(&yylhsminor.yy26, NULL, &yymsp[0].minor.yy26);
}
/* #line 2309 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 64: /* index_hint ::= IGNORE INDEX PARENTHESES_OPEN field_list PARENTHESES_CLOSE */
      case 65: /* index_hint ::= USE INDEX PARENTHESES_OPEN field_list PARENTHESES_CLOSE */ yytestcase(yyruleno==65);
{  yy_destructor(yypParser,58,&yymsp[-4].minor);
/* #line 787 "parser.y" */
{
	phql_ret_index_type(&yymsp[-4].minor.yy26, PHQL_T_USE, &yymsp[-1].minor.yy26);
}
/* #line 2319 "parser.c" */
  yy_destructor(yypParser,59,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 66: /* index_hint ::= FORCE INDEX PARENTHESES_OPEN field_list PARENTHESES_CLOSE */
{  yy_destructor(yypParser,32,&yymsp[-4].minor);
/* #line 795 "parser.y" */
{
	phql_ret_index_type(&yymsp[-4].minor.yy26, PHQL_T_FORCE, &yymsp[-1].minor.yy26);
}
/* #line 2331 "parser.c" */
  yy_destructor(yypParser,59,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 69: /* order_clause ::= ORDER BY order_list */
      case 76: /* group_clause ::= GROUP BY group_list */ yytestcase(yyruleno==76);
{  yy_destructor(yypParser,61,&yymsp[-2].minor);
/* #line 807 "parser.y" */
{
	yymsp[-2].minor.yy26 = yymsp[0].minor.yy26;
}
/* #line 2344 "parser.c" */
  yy_destructor(yypParser,62,&yymsp[-1].minor);
}
        break;
      case 73: /* order_item ::= expr */
/* #line 823 "parser.y" */
{
	phql_ret_order_item(&yylhsminor.yy26, &yymsp[0].minor.yy26, 0);
}
/* #line 2353 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 74: /* order_item ::= expr ASC */
/* #line 827 "parser.y" */
{
	phql_ret_order_item(&yylhsminor.yy26, &yymsp[-1].minor.yy26, PHQL_T_ASC);
}
/* #line 2361 "parser.c" */
  yy_destructor(yypParser,63,&yymsp[0].minor);
  yymsp[-1].minor.yy26 = yylhsminor.yy26;
        break;
      case 75: /* order_item ::= expr DESC */
/* #line 831 "parser.y" */
{
	phql_ret_order_item(&yylhsminor.yy26, &yymsp[-1].minor.yy26, PHQL_T_DESC);
}
/* #line 2370 "parser.c" */
  yy_destructor(yypParser,64,&yymsp[0].minor);
  yymsp[-1].minor.yy26 = yylhsminor.yy26;
        break;
      case 83: /* for_update_clause ::= FOR UPDATE */
{  yy_destructor(yypParser,67,&yymsp[-1].minor);
/* #line 863 "parser.y" */
{
	phql_ret_for_update_clause(&yymsp[-1].minor.yy26);
}
/* #line 2380 "parser.c" */
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
        break;
      case 85: /* select_limit_clause ::= LIMIT integer_or_placeholder */
      case 89: /* limit_clause ::= LIMIT integer_or_placeholder */ yytestcase(yyruleno==89);
{  yy_destructor(yypParser,68,&yymsp[-1].minor);
/* #line 871 "parser.y" */
{
	phql_ret_limit_clause(&yymsp[-1].minor.yy26, &yymsp[0].minor.yy26, NULL);
}
/* #line 2391 "parser.c" */
}
        break;
      case 86: /* select_limit_clause ::= LIMIT integer_or_placeholder COMMA integer_or_placeholder */
{  yy_destructor(yypParser,68,&yymsp[-3].minor);
/* #line 875 "parser.y" */
{
	phql_ret_limit_clause(&yymsp[-3].minor.yy26, &yymsp[0].minor.yy26, &yymsp[-2].minor.yy26);
}
/* #line 2400 "parser.c" */
  yy_destructor(yypParser,34,&yymsp[-1].minor);
}
        break;
      case 87: /* select_limit_clause ::= LIMIT integer_or_placeholder OFFSET integer_or_placeholder */
{  yy_destructor(yypParser,68,&yymsp[-3].minor);
/* #line 879 "parser.y" */
{
	phql_ret_limit_clause(&yymsp[-3].minor.yy26, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2410 "parser.c" */
  yy_destructor(yypParser,69,&yymsp[-1].minor);
}
        break;
      case 91: /* integer_or_placeholder ::= INTEGER */
      case 156: /* expr ::= INTEGER */ yytestcase(yyruleno==156);
/* #line 895 "parser.y" */
{
	phql_ret_literal_zval(&yylhsminor.yy26, PHQL_T_INTEGER, yymsp[0].minor.yy0);
}
/* #line 2420 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 92: /* integer_or_placeholder ::= HINTEGER */
      case 157: /* expr ::= HINTEGER */ yytestcase(yyruleno==157);
/* #line 899 "parser.y" */
{
	phql_ret_literal_zval(&yylhsminor.yy26, PHQL_T_HINTEGER, yymsp[0].minor.yy0);
}
/* #line 2429 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 93: /* integer_or_placeholder ::= NPLACEHOLDER */
      case 163: /* expr ::= NPLACEHOLDER */ yytestcase(yyruleno==163);
/* #line 903 "parser.y" */
{
	phql_ret_placeholder_zval(&yylhsminor.yy26, PHQL_T_NPLACEHOLDER, yymsp[0].minor.yy0);
}
/* #line 2438 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 94: /* integer_or_placeholder ::= SPLACEHOLDER */
      case 164: /* expr ::= SPLACEHOLDER */ yytestcase(yyruleno==164);
/* #line 907 "parser.y" */
{
	phql_ret_placeholder_zval(&yylhsminor.yy26, PHQL_T_SPLACEHOLDER, yymsp[0].minor.yy0);
}
/* #line 2447 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 95: /* expr ::= MINUS expr */
{  yy_destructor(yypParser,26,&yymsp[-1].minor);
/* #line 911 "parser.y" */
{
	phql_ret_expr(&yymsp[-1].minor.yy26, PHQL_T_MINUS, NULL, &yymsp[0].minor.yy26);
}
/* #line 2456 "parser.c" */
}
        break;
      case 96: /* expr ::= expr MINUS expr */
/* #line 915 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_SUB, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2464 "parser.c" */
  yy_destructor(yypParser,26,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 97: /* expr ::= expr PLUS expr */
/* #line 919 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_ADD, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2473 "parser.c" */
  yy_destructor(yypParser,25,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 98: /* expr ::= expr TIMES expr */
/* #line 923 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_MUL, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2482 "parser.c" */
  yy_destructor(yypParser,23,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 99: /* expr ::= expr DIVIDE expr */
/* #line 927 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_DIV, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2491 "parser.c" */
  yy_destructor(yypParser,22,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 100: /* expr ::= expr MOD expr */
/* #line 931 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_MOD, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2500 "parser.c" */
  yy_destructor(yypParser,24,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 101: /* expr ::= expr AND expr */
/* #line 935 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_AND, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2509 "parser.c" */
  yy_destructor(yypParser,15,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 102: /* expr ::= expr OR expr */
/* #line 939 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_OR, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2518 "parser.c" */
  yy_destructor(yypParser,16,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 103: /* expr ::= expr BITWISE_AND expr */
/* #line 943 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_BITWISE_AND, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2527 "parser.c" */
  yy_destructor(yypParser,19,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 104: /* expr ::= expr BITWISE_OR expr */
/* #line 947 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_BITWISE_OR, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2536 "parser.c" */
  yy_destructor(yypParser,20,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 105: /* expr ::= expr BITWISE_XOR expr */
/* #line 951 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_BITWISE_XOR, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2545 "parser.c" */
  yy_destructor(yypParser,21,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 106: /* expr ::= expr EQUALS expr */
/* #line 955 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_EQUALS, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2554 "parser.c" */
  yy_destructor(yypParser,3,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 107: /* expr ::= expr NOTEQUALS expr */
/* #line 959 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_NOTEQUALS, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2563 "parser.c" */
  yy_destructor(yypParser,4,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 108: /* expr ::= expr LESS expr */
/* #line 963 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_LESS, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2572 "parser.c" */
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 109: /* expr ::= expr GREATER expr */
/* #line 967 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_GREATER, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2581 "parser.c" */
  yy_destructor(yypParser,6,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 110: /* expr ::= expr GREATEREQUAL expr */
/* #line 971 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_GREATEREQUAL, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2590 "parser.c" */
  yy_destructor(yypParser,7,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 111: /* expr ::= expr TS_MATCHES expr */
/* #line 975 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_TS_MATCHES, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2599 "parser.c" */
  yy_destructor(yypParser,9,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 112: /* expr ::= expr TS_OR expr */
/* #line 979 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_TS_OR, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2608 "parser.c" */
  yy_destructor(yypParser,10,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 113: /* expr ::= expr TS_AND expr */
/* #line 983 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_TS_AND, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2617 "parser.c" */
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 114: /* expr ::= expr TS_NEGATE expr */
/* #line 987 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_TS_NEGATE, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2626 "parser.c" */
  yy_destructor(yypParser,12,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 115: /* expr ::= expr TS_CONTAINS_ANOTHER expr */
/* #line 991 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_TS_CONTAINS_ANOTHER, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2635 "parser.c" */
  yy_destructor(yypParser,13,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 116: /* expr ::= expr TS_CONTAINS_IN expr */
/* #line 995 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_TS_CONTAINS_IN, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2644 "parser.c" */
  yy_destructor(yypParser,14,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 117: /* expr ::= expr LESSEQUAL expr */
/* #line 999 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_LESSEQUAL, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2653 "parser.c" */
  yy_destructor(yypParser,8,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 118: /* expr ::= expr LIKE expr */
/* #line 1003 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_LIKE, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2662 "parser.c" */
  yy_destructor(yypParser,17,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 119: /* expr ::= expr NOT LIKE expr */
/* #line 1007 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_NLIKE, &yymsp[-3].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2671 "parser.c" */
  yy_destructor(yypParser,30,&yymsp[-2].minor);
  yy_destructor(yypParser,17,&yymsp[-1].minor);
  yymsp[-3].minor.yy26 = yylhsminor.yy26;
        break;
      case 120: /* expr ::= expr ILIKE expr */
/* #line 1011 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_ILIKE, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2681 "parser.c" */
  yy_destructor(yypParser,18,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 121: /* expr ::= expr NOT ILIKE expr */
/* #line 1015 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_NILIKE, &yymsp[-3].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2690 "parser.c" */
  yy_destructor(yypParser,30,&yymsp[-2].minor);
  yy_destructor(yypParser,18,&yymsp[-1].minor);
  yymsp[-3].minor.yy26 = yylhsminor.yy26;
        break;
      case 122: /* expr ::= expr IN PARENTHESES_OPEN argument_list PARENTHESES_CLOSE */
      case 125: /* expr ::= expr IN PARENTHESES_OPEN select_statement PARENTHESES_CLOSE */ yytestcase(yyruleno==125);
/* #line 1019 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_IN, &yymsp[-4].minor.yy26, &yymsp[-1].minor.yy26);
}
/* #line 2701 "parser.c" */
  yy_destructor(yypParser,29,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
  yymsp[-4].minor.yy26 = yylhsminor.yy26;
        break;
      case 123: /* expr ::= expr NOT IN PARENTHESES_OPEN argument_list PARENTHESES_CLOSE */
      case 126: /* expr ::= expr NOT IN PARENTHESES_OPEN select_statement PARENTHESES_CLOSE */ yytestcase(yyruleno==126);
/* #line 1023 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_NOTIN, &yymsp[-5].minor.yy26, &yymsp[-1].minor.yy26);
}
/* #line 2713 "parser.c" */
  yy_destructor(yypParser,30,&yymsp[-4].minor);
  yy_destructor(yypParser,29,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
  yymsp[-5].minor.yy26 = yylhsminor.yy26;
        break;
      case 124: /* expr ::= PARENTHESES_OPEN select_statement PARENTHESES_CLOSE */
{  yy_destructor(yypParser,50,&yymsp[-2].minor);
/* #line 1027 "parser.y" */
{
	phql_ret_expr(&yymsp[-2].minor.yy26, PHQL_T_SUBQUERY, &yymsp[-1].minor.yy26, NULL);
}
/* #line 2726 "parser.c" */
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 127: /* expr ::= EXISTS PARENTHESES_OPEN select_statement PARENTHESES_CLOSE */
{  yy_destructor(yypParser,74,&yymsp[-3].minor);
/* #line 1039 "parser.y" */
{
	phql_ret_expr(&yymsp[-3].minor.yy26, PHQL_T_EXISTS, NULL, &yymsp[-1].minor.yy26);
}
/* #line 2736 "parser.c" */
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 128: /* expr ::= expr AGAINST expr */
/* #line 1043 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_AGAINST, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2746 "parser.c" */
  yy_destructor(yypParser,1,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 129: /* expr ::= CAST PARENTHESES_OPEN expr AS IDENTIFIER PARENTHESES_CLOSE */
{  yy_destructor(yypParser,75,&yymsp[-5].minor);
/* #line 1047 "parser.y" */
{
	{
		zval qualified;
		phql_ret_raw_qualified_name(&qualified, yymsp[-1].minor.yy0, NULL);
		phql_ret_expr(&yymsp[-5].minor.yy26, PHQL_T_CAST, &yymsp[-3].minor.yy26, &qualified);
	}
}
/* #line 2760 "parser.c" */
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 130: /* expr ::= CONVERT PARENTHESES_OPEN expr USING IDENTIFIER PARENTHESES_CLOSE */
{  yy_destructor(yypParser,76,&yymsp[-5].minor);
/* #line 1055 "parser.y" */
{
	{
		zval qualified;
		phql_ret_raw_qualified_name(&qualified, yymsp[-1].minor.yy0, NULL);
		phql_ret_expr(&yymsp[-5].minor.yy26, PHQL_T_CONVERT, &yymsp[-3].minor.yy26, &qualified);
	}
}
/* #line 2776 "parser.c" */
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,77,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 131: /* expr ::= CASE expr when_clauses END */
{  yy_destructor(yypParser,78,&yymsp[-3].minor);
/* #line 1063 "parser.y" */
{
	phql_ret_expr(&yymsp[-3].minor.yy26, PHQL_T_CASE, &yymsp[-2].minor.yy26, &yymsp[-1].minor.yy26);
}
/* #line 2788 "parser.c" */
  yy_destructor(yypParser,79,&yymsp[0].minor);
}
        break;
      case 134: /* when_clause ::= WHEN expr THEN expr */
{  yy_destructor(yypParser,80,&yymsp[-3].minor);
/* #line 1075 "parser.y" */
{
	phql_ret_expr(&yymsp[-3].minor.yy26, PHQL_T_WHEN, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2798 "parser.c" */
  yy_destructor(yypParser,81,&yymsp[-1].minor);
}
        break;
      case 135: /* when_clause ::= ELSE expr */
{  yy_destructor(yypParser,82,&yymsp[-1].minor);
/* #line 1079 "parser.y" */
{
	phql_ret_expr(&yymsp[-1].minor.yy26, PHQL_T_ELSE, &yymsp[0].minor.yy26, NULL);
}
/* #line 2808 "parser.c" */
}
        break;
      case 137: /* function_call ::= LEFT PARENTHESES_OPEN distinct_or_null argument_list_or_null PARENTHESES_CLOSE */
{  yy_destructor(yypParser,45,&yymsp[-4].minor);
/* #line 1087 "parser.y" */
{
	phql_ret_func_call2(&yymsp[-4].minor.yy26, "LEFT", &yymsp[-1].minor.yy26, &yymsp[-2].minor.yy26);
}
/* #line 2817 "parser.c" */
  yy_destructor(yypParser,50,&yymsp[-3].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 138: /* function_call ::= RIGHT PARENTHESES_OPEN distinct_or_null argument_list_or_null PARENTHESES_CLOSE */
{  yy_destructor(yypParser,47,&yymsp[-4].minor);
/* #line 1091 "parser.y" */
{
	phql_ret_func_call2(&yymsp[-4].minor.yy26, "RIGHT", &yymsp[-1].minor.yy26, &yymsp[-2].minor.yy26);
}
/* #line 2828 "parser.c" */
  yy_destructor(yypParser,50,&yymsp[-3].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 139: /* function_call ::= IDENTIFIER PARENTHESES_OPEN distinct_or_null argument_list_or_null PARENTHESES_CLOSE */
/* #line 1095 "parser.y" */
{
	phql_ret_func_call(&yylhsminor.yy26, yymsp[-4].minor.yy0, &yymsp[-1].minor.yy26, &yymsp[-2].minor.yy26);
}
/* #line 2838 "parser.c" */
  yy_destructor(yypParser,50,&yymsp[-3].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
  yymsp[-4].minor.yy26 = yylhsminor.yy26;
        break;
      case 140: /* distinct_or_null ::= DISTINCT */
{  yy_destructor(yypParser,37,&yymsp[0].minor);
/* #line 1099 "parser.y" */
{
	phql_ret_distinct(&yymsp[0].minor.yy26);
}
/* #line 2849 "parser.c" */
}
        break;
      case 148: /* expr ::= expr IS NULL */
/* #line 1131 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_ISNULL, &yymsp[-2].minor.yy26, NULL);
}
/* #line 2857 "parser.c" */
  yy_destructor(yypParser,27,&yymsp[-1].minor);
  yy_destructor(yypParser,83,&yymsp[0].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 149: /* expr ::= expr IS NOT NULL */
/* #line 1135 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_ISNOTNULL, &yymsp[-3].minor.yy26, NULL);
}
/* #line 2867 "parser.c" */
  yy_destructor(yypParser,27,&yymsp[-2].minor);
  yy_destructor(yypParser,30,&yymsp[-1].minor);
  yy_destructor(yypParser,83,&yymsp[0].minor);
  yymsp[-3].minor.yy26 = yylhsminor.yy26;
        break;
      case 150: /* expr ::= expr BETWEEN expr */
/* #line 1139 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_BETWEEN, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2878 "parser.c" */
  yy_destructor(yypParser,2,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 151: /* expr ::= expr DOUBLECOLON expr */
/* #line 1143 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy26, PHQL_T_DOUBLECOLON, &yymsp[-2].minor.yy26, &yymsp[0].minor.yy26);
}
/* #line 2887 "parser.c" */
  yy_destructor(yypParser,28,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 152: /* expr ::= NOT expr */
{  yy_destructor(yypParser,30,&yymsp[-1].minor);
/* #line 1147 "parser.y" */
{
	phql_ret_expr(&yymsp[-1].minor.yy26, PHQL_T_NOT, NULL, &yymsp[0].minor.yy26);
}
/* #line 2897 "parser.c" */
}
        break;
      case 153: /* expr ::= BITWISE_NOT expr */
{  yy_destructor(yypParser,31,&yymsp[-1].minor);
/* #line 1151 "parser.y" */
{
	phql_ret_expr(&yymsp[-1].minor.yy26, PHQL_T_BITWISE_NOT, NULL, &yymsp[0].minor.yy26);
}
/* #line 2906 "parser.c" */
}
        break;
      case 154: /* expr ::= PARENTHESES_OPEN expr PARENTHESES_CLOSE */
{  yy_destructor(yypParser,50,&yymsp[-2].minor);
/* #line 1155 "parser.y" */
{
	phql_ret_expr(&yymsp[-2].minor.yy26, PHQL_T_ENCLOSED, &yymsp[-1].minor.yy26, NULL);
}
/* #line 2915 "parser.c" */
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
        break;
      case 158: /* expr ::= STRING */
/* #line 1171 "parser.y" */
{
	phql_ret_literal_zval(&yylhsminor.yy26, PHQL_T_STRING, yymsp[0].minor.yy0);
}
/* #line 2924 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 159: /* expr ::= DOUBLE */
/* #line 1175 "parser.y" */
{
	phql_ret_literal_zval(&yylhsminor.yy26, PHQL_T_DOUBLE, yymsp[0].minor.yy0);
}
/* #line 2932 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 160: /* expr ::= NULL */
{  yy_destructor(yypParser,83,&yymsp[0].minor);
/* #line 1179 "parser.y" */
{
	phql_ret_literal_zval(&yymsp[0].minor.yy26, PHQL_T_NULL, NULL);
}
/* #line 2941 "parser.c" */
}
        break;
      case 161: /* expr ::= TRUE */
{  yy_destructor(yypParser,86,&yymsp[0].minor);
/* #line 1183 "parser.y" */
{
	phql_ret_literal_zval(&yymsp[0].minor.yy26, PHQL_T_TRUE, NULL);
}
/* #line 2950 "parser.c" */
}
        break;
      case 162: /* expr ::= FALSE */
{  yy_destructor(yypParser,87,&yymsp[0].minor);
/* #line 1187 "parser.y" */
{
	phql_ret_literal_zval(&yymsp[0].minor.yy26, PHQL_T_FALSE, NULL);
}
/* #line 2959 "parser.c" */
}
        break;
      case 165: /* expr ::= BPLACEHOLDER */
/* #line 1202 "parser.y" */
{
	phql_ret_placeholder_zval(&yylhsminor.yy26, PHQL_T_BPLACEHOLDER, yymsp[0].minor.yy0);
}
/* #line 2967 "parser.c" */
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 166: /* qualified_name ::= IDENTIFIER COLON IDENTIFIER DOT IDENTIFIER */
/* #line 1206 "parser.y" */
{
	phql_ret_qualified_name(&yylhsminor.yy26, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, yymsp[0].minor.yy0);
}
/* #line 2975 "parser.c" */
  yy_destructor(yypParser,89,&yymsp[-3].minor);
  yy_destructor(yypParser,40,&yymsp[-1].minor);
  yymsp[-4].minor.yy26 = yylhsminor.yy26;
        break;
      case 167: /* qualified_name ::= IDENTIFIER COLON IDENTIFIER */
/* #line 1210 "parser.y" */
{
	phql_ret_qualified_name(&yylhsminor.yy26, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy0);
}
/* #line 2985 "parser.c" */
  yy_destructor(yypParser,89,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 168: /* qualified_name ::= IDENTIFIER DOT IDENTIFIER */
/* #line 1214 "parser.y" */
{
	phql_ret_qualified_name(&yylhsminor.yy26, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy0);
}
/* #line 2994 "parser.c" */
  yy_destructor(yypParser,40,&yymsp[-1].minor);
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
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
  phql_ARG_FETCH;
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
  phql_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  phql_TOKENTYPE yyminor         /* The minor type of the error token */
){
  phql_ARG_FETCH;
#define TOKEN yyminor
/************ Begin %syntax_error code ****************************************/
/* #line 458 "parser.y" */

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
/* #line 3119 "parser.c" */
/************ End %syntax_error code ******************************************/
  phql_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  phql_ARG_FETCH;
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
  phql_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "phql_Alloc" which describes the current state of the parser.
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
void phql_(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  phql_TOKENTYPE yyminor       /* The value for the token */
  phql_ARG_PDECL               /* Optional %extra_argument parameter */
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
  phql_ARG_STORE;

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

const phql_token_names phql_tokens[] =
{
  { SL("INTEGER"),       PHQL_T_INTEGER },
  { SL("DOUBLE"),        PHQL_T_DOUBLE },
  { SL("STRING"),        PHQL_T_STRING },
  { SL("IDENTIFIER"),    PHQL_T_IDENTIFIER },
  { SL("MINUS"),         PHQL_T_MINUS },
  { SL("+"),             PHQL_T_ADD },
  { SL("-"),             PHQL_T_SUB },
  { SL("*"),             PHQL_T_MUL },
  { SL("/"),             PHQL_T_DIV },
  { SL("&"),             PHQL_T_BITWISE_AND },
  { SL("|"),             PHQL_T_BITWISE_OR },
  { SL("%%"),            PHQL_T_MOD },
  { SL("AND"),           PHQL_T_AND },
  { SL("OR"),            PHQL_T_OR },
  { SL("LIKE"),          PHQL_T_LIKE },
  { SL("ILIKE"),         PHQL_T_ILIKE },
  { SL("DOT"),           PHQL_T_DOT },
  { SL("COLON"),         PHQL_T_COLON },
  { SL("DOUBLECOLON"),   PHQL_T_DOUBLECOLON },
  { SL("COMMA"),         PHQL_T_COMMA },
  { SL("EQUALS"),        PHQL_T_EQUALS },
  { SL("NOT EQUALS"),    PHQL_T_NOTEQUALS },
  { SL("NOT"),           PHQL_T_NOT },
  { SL("<"),             PHQL_T_LESS },
  { SL("<="),            PHQL_T_LESSEQUAL },
  { SL(">"),             PHQL_T_GREATER },
  { SL(">="),            PHQL_T_GREATEREQUAL },
  { SL("@@"),            PHQL_T_TS_MATCHES },
  { SL("||"),            PHQL_T_TS_OR },
  { SL("&&"),            PHQL_T_TS_AND },
  { SL("!!"),            PHQL_T_TS_NEGATE },
  { SL("@>"),            PHQL_T_TS_CONTAINS_ANOTHER },
  { SL("<@"),            PHQL_T_TS_CONTAINS_IN },
  { SL("("),             PHQL_T_PARENTHESES_OPEN },
  { SL(")"),             PHQL_T_PARENTHESES_CLOSE },
  { SL("NUMERIC PLACEHOLDER"), PHQL_T_NPLACEHOLDER },
  { SL("STRING PLACEHOLDER"),  PHQL_T_SPLACEHOLDER },
  { SL("UPDATE"),        PHQL_T_UPDATE },
  { SL("SET"),           PHQL_T_SET },
  { SL("WHERE"),         PHQL_T_WHERE },
  { SL("DELETE"),        PHQL_T_DELETE },
  { SL("FROM"),          PHQL_T_FROM },
  { SL("AS"),            PHQL_T_AS },
  { SL("INSERT"),        PHQL_T_INSERT },
  { SL("INTO"),          PHQL_T_INTO },
  { SL("VALUES"),        PHQL_T_VALUES },
  { SL("SELECT"),        PHQL_T_SELECT },
  { SL("ORDER"),         PHQL_T_ORDER },
  { SL("BY"),            PHQL_T_BY },
  { SL("LIMIT"),         PHQL_T_LIMIT },
  { SL("OFFSET"),        PHQL_T_OFFSET },
  { SL("GROUP"),         PHQL_T_GROUP },
  { SL("HAVING"),        PHQL_T_HAVING },
  { SL("IN"),            PHQL_T_IN },
  { SL("ON"),            PHQL_T_ON },
  { SL("INNER"),         PHQL_T_INNER },
  { SL("JOIN"),          PHQL_T_JOIN },
  { SL("LEFT"),          PHQL_T_LEFT },
  { SL("RIGHT"),         PHQL_T_RIGHT },
  { SL("IS"),            PHQL_T_IS },
  { SL("NULL"),          PHQL_T_NULL },
  { SL("NOT IN"),        PHQL_T_NOTIN },
  { SL("CROSS"),         PHQL_T_CROSS },
  { SL("OUTER"),         PHQL_T_OUTER },
  { SL("FULL"),          PHQL_T_FULL },
  { SL("ASC"),           PHQL_T_ASC },
  { SL("DESC"),          PHQL_T_DESC },
  { SL("BETWEEN"),       PHQL_T_BETWEEN },
  { SL("DISTINCT"),      PHQL_T_DISTINCT },
  { SL("AGAINST"),       PHQL_T_AGAINST },
  { SL("CAST"),          PHQL_T_CAST },
  { SL("CONVERT"),       PHQL_T_CONVERT },
  { SL("USING"),         PHQL_T_USING },
  { SL("ALL"),           PHQL_T_ALL },
  { SL("FOR"),           PHQL_T_FOR },
  { SL("EXISTS"),        PHQL_T_EXISTS },
  { SL("CASE"),          PHQL_T_CASE },
  { SL("WHEN"),          PHQL_T_WHEN },
  { SL("THEN"),          PHQL_T_THEN },
  { SL("ELSE"),          PHQL_T_ELSE },
  { SL("END"),           PHQL_T_END },
  { SL("WITH"),          PHQL_T_WITH },
  { SL("FORCE"),         PHQL_T_FORCE },
  { SL("IGNORE"),        PHQL_T_IGNORE },
  { SL("USE"),           PHQL_T_USE },
  { SL("INDEX"),         PHQL_T_INDEX },
  { NULL, 0, 0 }
};

static void *phql_wrapper_alloc(size_t bytes){
	return emalloc(bytes);
}

static void phql_wrapper_free(void *pointer){
	efree(pointer);
}

static void phql_parse_with_token(void* phql_parser, int opcode, int parsercode, phql_scanner_token *token, phql_parser_status *parser_status){

	phql_parser_token *pToken;

	pToken = emalloc(sizeof(phql_parser_token));
	pToken->opcode = opcode;
	pToken->token = token->value;
	pToken->token_len = token->len;
	pToken->free_flag = 1;

	phql_(phql_parser, parsercode, pToken, parser_status);

	token->value = NULL;
	token->len = 0;
}

/**
 * Creates an error message when it's triggered by the scanner
 */
static void phql_scanner_error_msg(phql_parser_status *parser_status, zval *error_msg){

	char *error = NULL, *error_part;
	unsigned int length;
	phql_scanner_state *state = parser_status->scanner_state;

	if (state->start) {
		length = 64 + state->start_length + parser_status->phql_length;
		error = emalloc(sizeof(char) * length);
		if (state->start_length > 16) {
			error_part = estrndup(state->start, 16);
			snprintf(error, length, "Scanning error before '%s...' when parsing: %s (%d)", error_part, parser_status->phql, parser_status->phql_length);
			efree(error_part);
		} else {
			snprintf(error, length, "Scanning error before '%s' when parsing: %s (%d)", state->start, parser_status->phql, parser_status->phql_length);
		}
		error[length - 1] = '\0';
		ZVAL_STRING(error_msg, error);
	} else {
		ZVAL_STRING(error_msg, "Scanning error near to EOF");
	}

	if (error) {
		efree(error);
	}
}

/**
 * Executes the internal PHQL parser/tokenizer
 */
int phql_parse_phql(zval *result, zval *phql) {

	zval error_msg = {};

	if (Z_TYPE_P(phql) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "PHQL is must be string");
		return FAILURE;
	}

	if (phql_internal_parse_phql(result, Z_STRVAL_P(phql), Z_STRLEN_P(phql), &error_msg) == FAILURE) {
		if (Z_TYPE(error_msg) > IS_NULL) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, Z_STRVAL(error_msg));
			zval_ptr_dtor(&error_msg);
		}
		else {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "There was an error parsing PHQL");
		}

		return FAILURE;
	}

	return SUCCESS;
}

/**
 * Executes a PHQL parser/tokenizer
 */
int phql_internal_parse_phql(zval *result, char *phql, unsigned int phql_length, zval *error_msg) {

	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;
	phql_parser_status *parser_status = NULL;
	int scanner_status, status = SUCCESS, error_length;
	phql_scanner_state *state;
	phql_scanner_token token;
	void* phql_parser;
	char *error;
	zval unique_id = {};

	if (!phql) {
		ZVAL_STRING(error_msg, "PHQL statement cannot be NULL");
		return FAILURE;
	}

	ZVAL_LONG(&unique_id, zend_inline_hash_func(phql, phql_length));

	phalcon_orm_get_prepared_ast(result, &unique_id);

	if (Z_TYPE_P(result) == IS_ARRAY) {
		return SUCCESS;
	}

	phql_parser = phql_Alloc(phql_wrapper_alloc);
	if (unlikely(!phql_parser)) {
		ZVAL_STRING(error_msg, "Memory allocation error");
		return FAILURE;
	}

	parser_status = emalloc(sizeof(phql_parser_status));
	state = emalloc(sizeof(phql_scanner_state));

	parser_status->status = PHQL_PARSING_OK;
	parser_status->scanner_state = state;
	ZVAL_UNDEF(&parser_status->ret);
	parser_status->syntax_error = NULL;
	parser_status->token = &token;
	parser_status->enable_literals = phalcon_globals_ptr->orm.enable_literals;
	parser_status->phql = phql;
	parser_status->phql_length = phql_length;

	state->active_token = 0;
	state->start = phql;
	state->start_length = 0;
	state->end = state->start;

	token.value = NULL;
	token.len = 0;

	while (0 <= (scanner_status = phql_get_token(state, &token))) {

		/* Calculate the 'start' length */
		state->start_length = (phql + phql_length - state->start);
		state->active_token = token.opcode;

		/* Parse the token found */
		switch (token.opcode) {
			case PHQL_T_IGNORE:
				break;

			case PHQL_T_ADD:
				phql_(phql_parser, PHQL_PLUS, NULL, parser_status);
				break;
			case PHQL_T_SUB:
				phql_(phql_parser, PHQL_MINUS, NULL, parser_status);
				break;
			case PHQL_T_MUL:
				phql_(phql_parser, PHQL_TIMES, NULL, parser_status);
				break;
			case PHQL_T_DIV:
				phql_(phql_parser, PHQL_DIVIDE, NULL, parser_status);
				break;
			case PHQL_T_MOD:
				phql_(phql_parser, PHQL_MOD, NULL, parser_status);
				break;
			case PHQL_T_AND:
				phql_(phql_parser, PHQL_AND, NULL, parser_status);
				break;
			case PHQL_T_OR:
				phql_(phql_parser, PHQL_OR, NULL, parser_status);
				break;
			case PHQL_T_EQUALS:
				phql_(phql_parser, PHQL_EQUALS, NULL, parser_status);
				break;
			case PHQL_T_NOTEQUALS:
				phql_(phql_parser, PHQL_NOTEQUALS, NULL, parser_status);
				break;
			case PHQL_T_LESS:
				phql_(phql_parser, PHQL_LESS, NULL, parser_status);
				break;
			case PHQL_T_GREATER:
				phql_(phql_parser, PHQL_GREATER, NULL, parser_status);
				break;
			case PHQL_T_GREATEREQUAL:
				phql_(phql_parser, PHQL_GREATEREQUAL, NULL, parser_status);
				break;
			case PHQL_T_LESSEQUAL:
				phql_(phql_parser, PHQL_LESSEQUAL, NULL, parser_status);
				break;

            case PHQL_T_IDENTIFIER:
    			phql_parse_with_token(phql_parser, PHQL_T_IDENTIFIER, PHQL_IDENTIFIER, &token, parser_status);
    			break;

            case PHQL_T_DOT:
    			phql_(phql_parser, PHQL_DOT, NULL, parser_status);
    			break;
    		case PHQL_T_COMMA:
    			phql_(phql_parser, PHQL_COMMA, NULL, parser_status);
    			break;

    		case PHQL_T_PARENTHESES_OPEN:
    			phql_(phql_parser, PHQL_PARENTHESES_OPEN, NULL, parser_status);
    			break;
    		case PHQL_T_PARENTHESES_CLOSE:
    			phql_(phql_parser, PHQL_PARENTHESES_CLOSE, NULL, parser_status);
    			break;

			case PHQL_T_LIKE:
				phql_(phql_parser, PHQL_LIKE, NULL, parser_status);
				break;
			case PHQL_T_ILIKE:
				phql_(phql_parser, PHQL_ILIKE, NULL, parser_status);
				break;
			case PHQL_T_NOT:
				phql_(phql_parser, PHQL_NOT, NULL, parser_status);
				break;
			case PHQL_T_BITWISE_AND:
				phql_(phql_parser, PHQL_BITWISE_AND, NULL, parser_status);
				break;
			case PHQL_T_BITWISE_OR:
				phql_(phql_parser, PHQL_BITWISE_OR, NULL, parser_status);
				break;
			case PHQL_T_BITWISE_NOT:
				phql_(phql_parser, PHQL_BITWISE_NOT, NULL, parser_status);
				break;
			case PHQL_T_BITWISE_XOR:
				phql_(phql_parser, PHQL_BITWISE_XOR, NULL, parser_status);
				break;
			case PHQL_T_AGAINST:
				phql_(phql_parser, PHQL_AGAINST, NULL, parser_status);
				break;
            case PHQL_T_CASE:
    			phql_(phql_parser, PHQL_CASE, NULL, parser_status);
    			break;
            case PHQL_T_WHEN:
        		phql_(phql_parser, PHQL_WHEN, NULL, parser_status);
        		break;
            case PHQL_T_THEN:
            	phql_(phql_parser, PHQL_THEN, NULL, parser_status);
            	break;
            case PHQL_T_END:
            	phql_(phql_parser, PHQL_END, NULL, parser_status);
            	break;
            case PHQL_T_ELSE:
                phql_(phql_parser, PHQL_ELSE, NULL, parser_status);
                break;

			case PHQL_T_INTEGER:
				if (parser_status->enable_literals) {
					phql_parse_with_token(phql_parser, PHQL_T_INTEGER, PHQL_INTEGER, &token, parser_status);
				} else {
					ZVAL_STRING(error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_DOUBLE:
				if (parser_status->enable_literals) {
					phql_parse_with_token(phql_parser, PHQL_T_DOUBLE, PHQL_DOUBLE, &token, parser_status);
				} else {
					ZVAL_STRING(error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_STRING:
				if (parser_status->enable_literals) {
					phql_parse_with_token(phql_parser, PHQL_T_STRING, PHQL_STRING, &token, parser_status);
				} else {
					ZVAL_STRING(error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_TRUE:
				if (parser_status->enable_literals) {
					phql_(phql_parser, PHQL_TRUE, NULL, parser_status);
				} else {
					ZVAL_STRING(error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_FALSE:
				if (parser_status->enable_literals) {
					phql_(phql_parser, PHQL_FALSE, NULL, parser_status);
				} else {
					ZVAL_STRING(error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;

			case PHQL_T_NPLACEHOLDER:
				phql_parse_with_token(phql_parser, PHQL_T_NPLACEHOLDER, PHQL_NPLACEHOLDER, &token, parser_status);
				break;
			case PHQL_T_SPLACEHOLDER:
				phql_parse_with_token(phql_parser, PHQL_T_SPLACEHOLDER, PHQL_SPLACEHOLDER, &token, parser_status);
				break;
			case PHQL_T_BPLACEHOLDER:
				phql_parse_with_token(phql_parser, PHQL_T_BPLACEHOLDER, PHQL_BPLACEHOLDER, &token, parser_status);
				break;

			case PHQL_T_FROM:
				phql_(phql_parser, PHQL_FROM, NULL, parser_status);
				break;
			case PHQL_T_UPDATE:
				phql_(phql_parser, PHQL_UPDATE, NULL, parser_status);
				break;
			case PHQL_T_SET:
				phql_(phql_parser, PHQL_SET, NULL, parser_status);
				break;
			case PHQL_T_WHERE:
				phql_(phql_parser, PHQL_WHERE, NULL, parser_status);
				break;
			case PHQL_T_DELETE:
				phql_(phql_parser, PHQL_DELETE, NULL, parser_status);
				break;
			case PHQL_T_INSERT:
				phql_(phql_parser, PHQL_INSERT, NULL, parser_status);
				break;
			case PHQL_T_INTO:
				phql_(phql_parser, PHQL_INTO, NULL, parser_status);
				break;
			case PHQL_T_VALUES:
				phql_(phql_parser, PHQL_VALUES, NULL, parser_status);
				break;
			case PHQL_T_SELECT:
				phql_(phql_parser, PHQL_SELECT, NULL, parser_status);
				break;
			case PHQL_T_AS:
				phql_(phql_parser, PHQL_AS, NULL, parser_status);
				break;
			case PHQL_T_ORDER:
				phql_(phql_parser, PHQL_ORDER, NULL, parser_status);
				break;
			case PHQL_T_BY:
				phql_(phql_parser, PHQL_BY, NULL, parser_status);
				break;
			case PHQL_T_LIMIT:
				phql_(phql_parser, PHQL_LIMIT, NULL, parser_status);
				break;
			case PHQL_T_OFFSET:
				phql_(phql_parser, PHQL_OFFSET, NULL, parser_status);
				break;
			case PHQL_T_GROUP:
				phql_(phql_parser, PHQL_GROUP, NULL, parser_status);
				break;
			case PHQL_T_HAVING:
				phql_(phql_parser, PHQL_HAVING, NULL, parser_status);
				break;
			case PHQL_T_ASC:
				phql_(phql_parser, PHQL_ASC, NULL, parser_status);
				break;
			case PHQL_T_DESC:
				phql_(phql_parser, PHQL_DESC, NULL, parser_status);
				break;
			case PHQL_T_IN:
				phql_(phql_parser, PHQL_IN, NULL, parser_status);
				break;
			case PHQL_T_ON:
				phql_(phql_parser, PHQL_ON, NULL, parser_status);
				break;
			case PHQL_T_INNER:
				phql_(phql_parser, PHQL_INNER, NULL, parser_status);
				break;
			case PHQL_T_JOIN:
				phql_(phql_parser, PHQL_JOIN, NULL, parser_status);
				break;
			case PHQL_T_LEFT:
				phql_(phql_parser, PHQL_LEFT, NULL, parser_status);
				break;
			case PHQL_T_RIGHT:
				phql_(phql_parser, PHQL_RIGHT, NULL, parser_status);
				break;
			case PHQL_T_CROSS:
				phql_(phql_parser, PHQL_CROSS, NULL, parser_status);
				break;
			case PHQL_T_FULL:
				phql_(phql_parser, PHQL_FULL, NULL, parser_status);
				break;
			case PHQL_T_OUTER:
				phql_(phql_parser, PHQL_OUTER, NULL, parser_status);
				break;
			case PHQL_T_IS:
				phql_(phql_parser, PHQL_IS, NULL, parser_status);
				break;
			case PHQL_T_NULL:
				phql_(phql_parser, PHQL_NULL, NULL, parser_status);
				break;
			case PHQL_T_BETWEEN:
				phql_(phql_parser, PHQL_BETWEEN, NULL, parser_status);
				break;
			case PHQL_T_DISTINCT:
				phql_(phql_parser, PHQL_DISTINCT, NULL, parser_status);
				break;
			case PHQL_T_ALL:
				phql_(phql_parser, PHQL_ALL, NULL, parser_status);
				break;
			case PHQL_T_CAST:
				phql_(phql_parser, PHQL_CAST, NULL, parser_status);
				break;
			case PHQL_T_CONVERT:
				phql_(phql_parser, PHQL_CONVERT, NULL, parser_status);
				break;
			case PHQL_T_USING:
				phql_(phql_parser, PHQL_USING, NULL, parser_status);
				break;
            case PHQL_T_EXISTS:
    			phql_(phql_parser, PHQL_EXISTS, NULL, parser_status);
    			break;
			case PHQL_T_TS_MATCHES:
				phql_(phql_parser, PHQL_TS_MATCHES, NULL, parser_status);
				break;
			case PHQL_T_TS_OR:
				phql_(phql_parser, PHQL_TS_OR, NULL, parser_status);
				break;
			case PHQL_T_TS_AND:
				phql_(phql_parser, PHQL_TS_AND, NULL, parser_status);
				break;
			case PHQL_T_TS_NEGATE:
				phql_(phql_parser, PHQL_TS_NEGATE, NULL, parser_status);
				break;
			case PHQL_T_TS_CONTAINS_ANOTHER:
				phql_(phql_parser, PHQL_TS_CONTAINS_ANOTHER, NULL, parser_status);
				break;
			case PHQL_T_TS_CONTAINS_IN:
				phql_(phql_parser, PHQL_TS_CONTAINS_IN, NULL, parser_status);
				break;
			case PHQL_T_FOR:
				phql_(phql_parser, PHQL_FOR, NULL, parser_status);
				break;
			default:
				parser_status->status = PHQL_PARSING_FAILED;
				error_length = sizeof(char) * 32;
				error = emalloc(error_length);
				snprintf(error, error_length, "Scanner: Unknown opcode %c", token.opcode);
				error[error_length - 1] = '\0';
				ZVAL_STRING(error_msg, error);
				efree(error);
				break;
		}

		if (parser_status->status != PHQL_PARSING_OK) {
			status = FAILURE;
			break;
		}

		state->end = state->start;
	}

	if (status != FAILURE) {
		switch (scanner_status) {
			case PHQL_SCANNER_RETCODE_ERR:
			case PHQL_SCANNER_RETCODE_IMPOSSIBLE:
				if (Z_TYPE_P(error_msg) <= IS_NULL) {
					phql_scanner_error_msg(parser_status, error_msg);
				}
				status = FAILURE;
				break;
			default:
				phql_(phql_parser, 0, NULL, parser_status);
		}
	}

	state->active_token = 0;
	state->start = NULL;

	if (parser_status->status != PHQL_PARSING_OK) {
		status = FAILURE;
		if (parser_status->syntax_error) {
			if (Z_TYPE_P(error_msg) <= IS_NULL) {
				ZVAL_STRING(error_msg, parser_status->syntax_error);
			}
			efree(parser_status->syntax_error);
		}
	}

	phql_Free(phql_parser, phql_wrapper_free);

	if (status != FAILURE) {
		if (parser_status->status == PHQL_PARSING_OK) {
			if (Z_TYPE(parser_status->ret) == IS_ARRAY) {

				/**
				 * Set a unique id for the parsed ast
				 */
				if (phalcon_globals_ptr->orm.cache_level >= 1) {
					add_assoc_long(&parser_status->ret, "id", Z_LVAL(unique_id));
				}

				ZVAL_COPY_VALUE(result, &parser_status->ret);

				/**
				 * Store the parsed definition in the cache
				 */
				phalcon_orm_set_prepared_ast(&unique_id, result);

			} else {
				array_init(result);
			}
		}
	}

	efree(parser_status);
	efree(state);

	return status;
}
