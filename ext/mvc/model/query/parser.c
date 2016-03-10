/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include <stdio.h>
/* #line 40 "parser.y" */


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

static zval *phql_ret_literal_zval(int type, phql_parser_token *T)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 2);
	add_assoc_long(ret, ISV(type), type);
	if (T) {
		add_assoc_stringl(ret, ISV(value), T->token, T->token_len);
		efree(T);
	}

	return ret;
}

static zval *phql_ret_placeholder_zval(int type, phql_parser_token *T)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 2);
	add_assoc_long(ret, ISV(type), type);
	add_assoc_stringl(ret, ISV(value), T->token, T->token_len);
	efree(T);

	return ret;
}

static zval *phql_ret_qualified_name(phql_parser_token *A, phql_parser_token *B, phql_parser_token *C)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHQL_T_QUALIFIED);

	if (A != NULL) {
		add_assoc_stringl(ret, ISV(ns_alias), A->token, A->token_len);
		efree(A);
	}

	if (B != NULL) {
		add_assoc_stringl(ret, ISV(domain), B->token, B->token_len);
		efree(B);
	}

	add_assoc_stringl(ret, ISV(name), C->token, C->token_len);
	efree(C);

	return ret;
}

static zval *phql_ret_raw_qualified_name(phql_parser_token *A, phql_parser_token *B)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHQL_T_RAW_QUALIFIED);
	if (B != NULL) {
		add_assoc_stringl(ret, ISV(domain), A->token, A->token_len);
		add_assoc_stringl(ret, ISV(name), B->token, B->token_len);
		efree(B);
	} else {
		add_assoc_stringl(ret, ISV(name), A->token, A->token_len);
	}
	efree(A);

	return ret;
}

static zval *phql_ret_select_statement(zval *S, zval *W, zval *O, zval *G, zval *H, zval *L, zval *F)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHQL_T_SELECT);
	add_assoc_zval(ret, ISV(select), S);

	if (W != NULL) {
		add_assoc_zval(ret, ISV(where), W);
	}
	if (O != NULL) {
		add_assoc_zval(ret, ISV(orderBy), O);
	}
	if (G != NULL) {
		add_assoc_zval(ret, ISV(groupBy), G);
	}
	if (H != NULL) {
		add_assoc_zval(ret, ISV(having), H);
	}
	if (L != NULL) {
		add_assoc_zval(ret, ISV(limit), L);
	}
	if (F != NULL) {
		add_assoc_zval(ret, ISV(forupdate), F);
	}

	return ret;
}

static zval *phql_ret_select_clause(zval *distinct, zval *columns, zval *tables, zval *join_list)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 4);

	if (distinct) {
		add_assoc_zval(ret, ISV(distinct), distinct);
	}

	add_assoc_zval(ret, ISV(columns), columns);
	add_assoc_zval(ret, ISV(tables), tables);
	if (join_list) {
		add_assoc_zval(ret, ISV(joins), join_list);
	}

	return ret;
}

static zval *phql_ret_distinct_all(int distinct)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	ZVAL_LONG(ret, distinct);

	return ret;
}

static zval *phql_ret_distinct(void)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	ZVAL_TRUE(ret);

	return ret;
}

static zval *phql_ret_order_item(zval *column, int sort){

	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);
	add_assoc_zval(ret, ISV(column), column);
	if (sort != 0 ) {
		add_assoc_long(ret, ISV(sort), sort);
	}

	return ret;
}

static zval *phql_ret_limit_clause(zval *L, zval *O)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 2);

	add_assoc_zval(ret, ISV(number), L);

	if (O != NULL) {
		add_assoc_zval(ret, ISV(offset), O);
	}

	return ret;
}

static zval *phql_ret_for_update_clause()
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	ZVAL_TRUE(ret);

	return ret;
}

static zval *phql_ret_insert_statement(zval *Q, zval *F, zval *V)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 4);

	add_assoc_long(ret, ISV(type), PHQL_T_INSERT);
	add_assoc_zval(ret, ISV(qualifiedName), Q);
	if (F != NULL) {
		add_assoc_zval(ret, ISV(fields), F);
	}
	add_assoc_zval(ret, ISV(values), V);

	return ret;
}

static zval *phql_ret_insert_statement2(zval *ret, zval *F, zval *V)
{
	zval key1, key2, rows, values;

	ZVAL_STR(&key1, IS(rows));

	if (!phalcon_array_isset_fetch(&rows, ret, &key1)) {
		array_init_size(&rows, 1);		

		ZVAL_STR(&key2, IS(values));

		if (phalcon_array_isset_fetch(&values, ret, &key2)) {
			Z_TRY_ADDREF_P(&values);
			add_next_index_zval(&rows, &values);	
		}
	}

	add_next_index_zval(&rows, V);
	Z_TRY_ADDREF(rows);
	add_assoc_zval(ret, ISV(rows), &rows);

	return ret;
}

static zval *phql_ret_update_statement(zval *U, zval *W, zval *L)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHQL_T_UPDATE);
	add_assoc_zval(ret, ISV(update), U);
	if (W != NULL) {
		add_assoc_zval(ret, ISV(where), W);
	}
	if (L != NULL) {
		add_assoc_zval(ret, ISV(limit), L);
	}

	return ret;
}

static zval *phql_ret_update_clause(zval *tables, zval *values)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 2);
	add_assoc_zval(ret, ISV(tables), tables);
	add_assoc_zval(ret, ISV(values), values);

	return ret;
}

static zval *phql_ret_update_item(zval *column, zval *expr)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 2);
	add_assoc_zval(ret, ISV(column), column);
	add_assoc_zval(ret, ISV(expr), expr);

	return ret;
}

static zval *phql_ret_delete_statement(zval *D, zval *W, zval *L)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);

	add_assoc_long(ret, ISV(type), PHQL_T_DELETE);
	add_assoc_zval(ret, ISV(delete), D);
	if (W != NULL) {
		add_assoc_zval(ret, ISV(where), W);
	}
	if (L != NULL) {
		add_assoc_zval(ret, ISV(limit), L);
	}

	return ret;
}

static zval *phql_ret_delete_clause(zval *tables)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 1);
	add_assoc_zval(ret, ISV(tables), tables);

	return ret;
}

static zval *phql_ret_zval_list(zval *list_left, zval *right_list)
{
	zval *ret;
	HashTable *list;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init(ret);

	list = Z_ARRVAL_P(list_left);
	if (zend_hash_index_exists(list, 0)) {
		zval *item;
		ZEND_HASH_FOREACH_VAL(list, item) {
			Z_TRY_ADDREF_P(item);
			add_next_index_zval(ret, item);
		} ZEND_HASH_FOREACH_END();

		zval_ptr_dtor(list_left);
	} else {
		add_next_index_zval(ret, list_left);
	}

	if (right_list) {
		add_next_index_zval(ret, right_list);
	}

	return ret;
}

static zval *phql_ret_column_item(int type, zval *column, phql_parser_token *identifier_column, phql_parser_token *alias)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 4);
	add_assoc_long(ret, ISV(type), type);
	if (column) {
		add_assoc_zval(ret, ISV(column), column);
	}
	if (identifier_column) {
		add_assoc_stringl(ret, ISV(column), identifier_column->token, identifier_column->token_len);
		efree(identifier_column);
	}
	if (alias) {
		add_assoc_stringl(ret, ISV(alias), alias->token, alias->token_len);
		efree(alias);
	}

	return ret;
}

static zval *phql_ret_assoc_name(zval *qualified_name, phql_parser_token *alias)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 2);
	add_assoc_zval(ret, ISV(qualifiedName), qualified_name);
	if (alias) {
		add_assoc_stringl(ret, ISV(alias), alias->token, alias->token_len);
		efree(alias);
	}

	return ret;
}

static zval *phql_ret_join_type(int type)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	ZVAL_LONG(ret, type);

	return ret;
}

static zval *phql_ret_join_item(zval *type, zval *qualified, zval *alias, zval *conditions)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 4);
	add_assoc_zval(ret, ISV(type), type);

	if (qualified) {
		add_assoc_zval(ret, ISV(qualified), qualified);
	}

	if (alias) {
		add_assoc_zval(ret, ISV(alias), alias);
	}

	if (conditions) {
		add_assoc_zval(ret, ISV(conditions), conditions);
	}

	return ret;
}

static zval *phql_ret_expr(int type, zval *left, zval *right)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 2);
	add_assoc_long(ret, ISV(type), type);
	if (left) {
		add_assoc_zval(ret, ISV(left), left);
	}
	if (right) {
		add_assoc_zval(ret, ISV(right), right);
	}

	return ret;
}

static zval *phql_ret_func_call(phql_parser_token *name, zval *arguments, zval *distinct)
{
	zval *ret;

	PHALCON_ALLOC_INIT_ZVAL(ret);
	array_init_size(ret, 4);
	add_assoc_long(ret, ISV(type), PHQL_T_FCALL);
	add_assoc_stringl(ret, ISV(name), name->token, name->token_len);
	efree(name);

	if (arguments) {
		add_assoc_zval(ret, ISV(arguments), arguments);
	}
	
	if (distinct) {
		add_assoc_zval(ret, ISV(distinct), distinct);
	}

	return ret;
}


/* #line 464 "parser.c" */
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    phql_TOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is phql_TOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.
**    phql_ARG_SDECL     A static variable declaration for the %extra_argument
**    phql_ARG_PDECL     A parameter declaration for the %extra_argument
**    phql_ARG_STORE     Code to store %extra_argument into yypParser
**    phql_ARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 138
#define YYACTIONTYPE unsigned short int
#define phql_TOKENTYPE phql_parser_token*
typedef union {
  phql_TOKENTYPE yy0;
  zval* yy68;
  int yy275;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define phql_ARG_SDECL phql_parser_status *status;
#define phql_ARG_PDECL ,phql_parser_status *status
#define phql_ARG_FETCH phql_parser_status *status = yypParser->status
#define phql_ARG_STORE yypParser->status = status
#define YYNSTATE 293
#define YYNRULE 160
#define YYERRORSYMBOL 85
#define YYERRSYMDT yy275
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* Next are that tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
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
*/
static YYACTIONTYPE yy_action[] = {
 /*     0 */    92,   98,   32,   34,   36,   38,   40,   54,   42,   44,
 /*    10 */    46,   48,   50,   52,   22,   24,   56,   61,   26,   28,
 /*    20 */    30,   18,   16,   20,   14,   11,   94,   63,   58,   32,
 /*    30 */    34,   36,   38,   40,   54,   42,   44,   46,   48,   50,
 /*    40 */    52,   22,   24,   56,   61,   26,   28,   30,   18,   16,
 /*    50 */    20,   14,   11,   94,   63,   58,  293,   90,   91,   92,
 /*    60 */    98,   32,   34,   36,   38,   40,   54,   42,   44,   46,
 /*    70 */    48,   50,   52,   22,   24,   56,   61,   26,   28,   30,
 /*    80 */    18,   16,   20,   14,   11,   94,   63,   58,   92,   98,
 /*    90 */    32,   34,   36,   38,   40,   54,   42,   44,   46,   48,
 /*   100 */    50,   52,   22,   24,   56,   61,   26,   28,   30,   18,
 /*   110 */    16,   20,   14,   11,   94,   63,   58,   14,   11,   94,
 /*   120 */    63,   58,  106,  217,  294,  215,  121,  206,  123,  124,
 /*   130 */    94,   63,   58,  151,  112,  183,  110,   92,   98,   32,
 /*   140 */    34,   36,   38,   40,   54,   42,   44,   46,   48,   50,
 /*   150 */    52,   22,   24,   56,   61,   26,   28,   30,   18,   16,
 /*   160 */    20,   14,   11,   94,   63,   58,   18,   16,   20,   14,
 /*   170 */    11,   94,   63,   58,  137,    5,   92,   98,   32,   34,
 /*   180 */    36,   38,   40,   54,   42,   44,   46,   48,   50,   52,
 /*   190 */    22,   24,   56,   61,   26,   28,   30,   18,   16,   20,
 /*   200 */    14,   11,   94,   63,   58,   92,   98,   32,   34,   36,
 /*   210 */    38,   40,   54,   42,   44,   46,   48,   50,   52,   22,
 /*   220 */    24,   56,   61,   26,   28,   30,   18,   16,   20,   14,
 /*   230 */    11,   94,   63,   58,   56,   61,   26,   28,   30,   18,
 /*   240 */    16,   20,   14,   11,   94,   63,   58,  143,   96,   92,
 /*   250 */    98,   32,   34,   36,   38,   40,   54,   42,   44,   46,
 /*   260 */    48,   50,   52,   22,   24,   56,   61,   26,   28,   30,
 /*   270 */    18,   16,   20,   14,   11,   94,   63,   58,  219,  220,
 /*   280 */   153,   22,   24,   56,   61,   26,   28,   30,   18,   16,
 /*   290 */    20,   14,   11,   94,   63,   58,  222,   95,   92,   98,
 /*   300 */    32,   34,   36,   38,   40,   54,   42,   44,   46,   48,
 /*   310 */    50,   52,   22,   24,   56,   61,   26,   28,   30,   18,
 /*   320 */    16,   20,   14,   11,   94,   63,   58,   42,   44,   46,
 /*   330 */    48,   50,   52,   22,   24,   56,   61,   26,   28,   30,
 /*   340 */    18,   16,   20,   14,   11,   94,   63,   58,  165,   64,
 /*   350 */   125,   12,   59,  241,  166,  168,  196,  101,   89,  194,
 /*   360 */   107,  156,  113,  243,   78,   79,   80,   81,  189,  205,
 /*   370 */   191,  193,  100,  197,  201,  165,  170,   76,   12,   86,
 /*   380 */   225,  166,  168,  122,  123,  124,   58,  155,  156,  255,
 /*   390 */   171,  172,  178,  179,  130,  134,  140,  108,  146,  100,
 /*   400 */   427,  113,  109,  175,  173,  174,  176,  177,  180,  132,
 /*   410 */    82,  239,  295,   65,  113,   65,  119,  171,  172,  178,
 /*   420 */   179,  130,  134,  140,  247,  146,  157,  159,   65,    8,
 /*   430 */   175,  173,  174,  176,  177,  180,  189,  205,  191,  193,
 /*   440 */   159,  197,  201,    4,   12,  170,    7,  166,  168,   10,
 /*   450 */   101,  148,  185,  237,  156,  235,  155,  119,  170,  181,
 /*   460 */   101,    9,  208,  115,  104,  100,  245,  170,  210,  155,
 /*   470 */    66,   12,  181,  163,  166,  168,  159,  253,  155,  228,
 /*   480 */   276,  211,  290,  171,  172,  178,  179,  130,  134,  140,
 /*   490 */   426,  146,  100,  188,  170,  186,  175,  173,  174,  176,
 /*   500 */   177,  180,  162,  200,  115,  155,  198,  160,  181,  212,
 /*   510 */   171,  172,  178,  179,  130,  134,  140,  223,  146,  157,
 /*   520 */    10,   65,  213,  175,  173,  174,  176,  177,  180,    6,
 /*   530 */   233,   67,  250,  221,  165,  120,  226,   12,  170,  103,
 /*   540 */   166,  168,  218,  214,  279,  286,  282,  156,  170,  155,
 /*   550 */   376,  170,  204,  230,  234,  202,  115,  261,  100,  155,
 /*   560 */   270,  170,  155,   68,   12,  260,   84,  166,  168,  256,
 /*   570 */   159,   74,  155,   69,  156,   70,  171,  172,  178,  179,
 /*   580 */   130,  134,  140,  249,  146,  100,  281,  282,  170,  175,
 /*   590 */   173,  174,  176,  177,  180,  149,  151,    8,  183,  155,
 /*   600 */    71,  170,  164,  171,  172,  178,  179,  130,  134,  140,
 /*   610 */    73,  146,  155,  274,  259,   72,  175,  173,  174,  176,
 /*   620 */   177,  180,   75,   26,   28,   30,   18,   16,   20,   14,
 /*   630 */    11,   94,   63,   58,  454,    1,    2,    3,  251,  252,
 /*   640 */    65,   85,   77,  163,   10,   10,   89,   87,   83,   97,
 /*   650 */   102,  114,  163,  258,  265,  117,  250,  250,  209,  214,
 /*   660 */   238,  105,  170,  170,  170,  207,  271,  233,   88,  246,
 /*   670 */   109,  111,  287,  155,  155,  155,  116,  170,  267,  285,
 /*   680 */   118,   13,  113,   15,   17,  170,   19,   21,  155,   23,
 /*   690 */   120,  232,  126,   25,  128,  262,  155,  170,  284,  170,
 /*   700 */   127,  170,  170,  131,  170,  170,   27,  170,  155,   29,
 /*   710 */   155,  170,  155,  155,   31,  155,  155,    8,  155,  101,
 /*   720 */   133,   33,  155,   35,  170,  135,   37,  170,   39,   41,
 /*   730 */    43,   45,  170,   47,  266,  155,   49,  138,  155,  170,
 /*   740 */   139,  170,  141,  155,  170,   51,  170,  170,  170,  170,
 /*   750 */   155,  170,  155,  144,  170,  155,  145,  155,  155,  155,
 /*   760 */   155,   53,  155,  170,   55,  155,   57,   60,   62,   93,
 /*   770 */    99,  129,  182,  136,  155,  150,  158,  187,  161,  170,
 /*   780 */   190,  321,  170,  192,  170,  170,  170,  170,  170,  170,
 /*   790 */   155,  170,  142,  155,  147,  155,  155,  155,  155,  155,
 /*   800 */   155,  152,  155,  277,  154,  322,  167,  169,  109,  184,
 /*   810 */   170,  227,  170,  236,  242,  195,  323,  324,  199,  170,
 /*   820 */   113,  155,  170,  155,  170,  170,  325,  170,  326,  170,
 /*   830 */   155,  170,  170,  155,  292,  155,  155,  203,  155,  109,
 /*   840 */   155,  327,  155,  155,  328,  329,  216,  224,  229,  231,
 /*   850 */   240,  113,  248,  244,  296,  297,  269,  254,  257,  268,
 /*   860 */   264,  272,  341,  263,  275,  273,  375,  278,  280,  288,
 /*   870 */   283,  289,  347,  291,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*    10 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*    20 */    21,   22,   23,   24,   25,   26,   27,   28,   29,    3,
 /*    30 */     4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
 /*    40 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*    50 */    24,   25,   26,   27,   28,   29,    0,   58,   59,    1,
 /*    60 */     2,    3,    4,    5,    6,    7,    8,    9,   10,   11,
 /*    70 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /*    80 */    22,   23,   24,   25,   26,   27,   28,   29,    1,    2,
 /*    90 */     3,    4,    5,    6,    7,    8,    9,   10,   11,   12,
 /*   100 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   110 */    23,   24,   25,   26,   27,   28,   29,   25,   26,   27,
 /*   120 */    28,   29,  102,   36,    0,   38,  106,  107,  108,  109,
 /*   130 */    27,   28,   29,   75,   36,   77,   38,    1,    2,    3,
 /*   140 */     4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   150 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   160 */    24,   25,   26,   27,   28,   29,   22,   23,   24,   25,
 /*   170 */    26,   27,   28,   29,   38,   47,    1,    2,    3,    4,
 /*   180 */     5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
 /*   190 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*   200 */    25,   26,   27,   28,   29,    1,    2,    3,    4,    5,
 /*   210 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   220 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*   230 */    26,   27,   28,   29,   17,   18,   19,   20,   21,   22,
 /*   240 */    23,   24,   25,   26,   27,   28,   29,   72,   29,    1,
 /*   250 */     2,    3,    4,    5,    6,    7,    8,    9,   10,   11,
 /*   260 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /*   270 */    22,   23,   24,   25,   26,   27,   28,   29,   34,   35,
 /*   280 */    76,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   290 */    24,   25,   26,   27,   28,   29,   48,   78,    1,    2,
 /*   300 */     3,    4,    5,    6,    7,    8,    9,   10,   11,   12,
 /*   310 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   320 */    23,   24,   25,   26,   27,   28,   29,    9,   10,   11,
 /*   330 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /*   340 */    22,   23,   24,   25,   26,   27,   28,   29,   23,   47,
 /*   350 */   110,   26,   17,   18,   29,   30,   40,   32,  104,   43,
 /*   360 */    31,   36,  122,   28,   65,   66,   67,   68,   39,   40,
 /*   370 */    41,   42,   47,   44,   45,   23,  122,   31,   26,  125,
 /*   380 */   126,   29,   30,  107,  108,  109,   29,  133,   36,  110,
 /*   390 */    65,   66,   67,   68,   69,   70,   71,  105,   73,   47,
 /*   400 */    48,  122,  110,   78,   79,   80,   81,   82,   83,   88,
 /*   410 */    64,   88,    0,   92,  122,   92,   37,   65,   66,   67,
 /*   420 */    68,   69,   70,   71,   88,   73,   47,  104,   92,   31,
 /*   430 */    78,   79,   80,   81,   82,   83,   39,   40,   41,   42,
 /*   440 */   104,   44,   45,   31,   26,  122,   48,   29,   30,  104,
 /*   450 */    32,  131,  132,  130,   36,   55,  133,   37,  122,  136,
 /*   460 */    32,  116,   31,   84,   33,   47,  130,  122,   23,  133,
 /*   470 */    93,   26,  136,   31,   29,   30,  104,   49,  133,   60,
 /*   480 */    52,   36,   54,   65,   66,   67,   68,   69,   70,   71,
 /*   490 */    48,   73,   47,   36,  122,   38,   78,   79,   80,   81,
 /*   500 */    82,   83,  130,   40,   84,  133,   43,  135,  136,   37,
 /*   510 */    65,   66,   67,   68,   69,   70,   71,   88,   73,   47,
 /*   520 */   104,   92,   23,   78,   79,   80,   81,   82,   83,  113,
 /*   530 */   104,   94,  116,  104,   23,   36,   61,   26,  122,  100,
 /*   540 */    29,   30,  103,  104,  120,  121,  122,   36,  122,  133,
 /*   550 */     0,  122,   40,  127,  128,   43,   84,  114,   47,  133,
 /*   560 */   117,  122,  133,   95,   26,   47,   56,   29,   30,   51,
 /*   570 */   104,   63,  133,   96,   36,   97,   65,   66,   67,   68,
 /*   580 */    69,   70,   71,  104,   73,   47,  121,  122,  122,   78,
 /*   590 */    79,   80,   81,   82,   83,   74,   75,   31,   77,  133,
 /*   600 */    98,  122,  136,   65,   66,   67,   68,   69,   70,   71,
 /*   610 */    52,   73,  133,   63,   48,   62,   78,   79,   80,   81,
 /*   620 */    82,   83,  129,   19,   20,   21,   22,   23,   24,   25,
 /*   630 */    26,   27,   28,   29,   86,   87,   88,   89,   90,   91,
 /*   640 */    92,   57,  129,   31,  104,  104,  104,   31,  129,   78,
 /*   650 */    99,   36,   31,  113,  113,   37,  116,  116,  103,  104,
 /*   660 */    48,  101,  122,  122,  122,  105,  118,  104,  126,   48,
 /*   670 */   110,   36,  124,  133,  133,  133,   36,  122,   31,  104,
 /*   680 */    36,  104,  122,  104,  104,  122,  104,  104,  133,  104,
 /*   690 */    36,  128,  111,  104,   46,   48,  133,  122,  123,  122,
 /*   700 */   112,  122,  122,   47,  122,  122,  104,  122,  133,  104,
 /*   710 */   133,  122,  133,  133,  104,  133,  133,   31,  133,   32,
 /*   720 */    48,  104,  133,  104,  122,   47,  104,  122,  104,  104,
 /*   730 */   104,  104,  122,  104,   48,  133,  104,   36,  133,  122,
 /*   740 */    48,  122,   47,  133,  122,  104,  122,  122,  122,  122,
 /*   750 */   133,  122,  133,   36,  122,  133,   48,  133,  133,  133,
 /*   760 */   133,  104,  133,  122,  104,  133,  104,  104,  104,  104,
 /*   770 */   104,  104,   34,  104,  133,  132,  134,   36,   48,  122,
 /*   780 */    40,   36,  122,   40,  122,  122,  122,  122,  122,  122,
 /*   790 */   133,  122,  104,  133,  104,  133,  133,  133,  133,  133,
 /*   800 */   133,  104,  133,  105,  104,   36,  104,  104,  110,  104,
 /*   810 */   122,  104,  122,  104,  104,   40,   36,   36,   40,  122,
 /*   820 */   122,  133,  122,  133,  122,  122,   36,  122,   36,  122,
 /*   830 */   133,  122,  122,  133,  105,  133,  133,   40,  133,  110,
 /*   840 */   133,   36,  133,  133,   36,   36,   36,   48,   57,   31,
 /*   850 */    48,  122,   48,   47,    0,    0,   36,   50,   47,  117,
 /*   860 */    47,   93,    0,   51,  129,  119,    0,   53,   31,   93,
 /*   870 */     3,  119,    0,   33,
};
#define YY_SHIFT_USE_DFLT (-2)
static short yy_shift_ofst[] = {
 /*     0 */   428,   56,  124,  412,  128,  538,  398,   -2,  538,   -2,
 /*    10 */   297,  538,  538,  103,  538,  103,  538,   92,  538,   92,
 /*    20 */   538,   92,  538,  217,  538,  217,  538,  144,  538,  144,
 /*    30 */   538,  144,  538,  318,  538,  318,  538,  318,  538,  318,
 /*    40 */   538,  318,  538,  266,  538,  266,  538,  266,  538,  266,
 /*    50 */   538,  266,  538,  266,  538,  318,  538,  604,  335,  538,
 /*    60 */   357,  538,  604,  302,  325,  400,  419,  475,  510,  508,
 /*    70 */   553,   -2,  558,   -2,  299,  346,  299,   -2,   -2,   -2,
 /*    80 */    -2,   -2,  299,   -2,  584,  538,  616,  538,   -2,   -1,
 /*    90 */    -2,   -2,  538,  297,  219,   -2,  571,   -2,  538,   26,
 /*   100 */   418,  244,  445,  431,  615,  329,   -2,  615,   -2,   98,
 /*   110 */   635,   -2,   -2,   -2,  420,  640,  618,  644,   -2,  654,
 /*   120 */    -2,  397,   -2,   -2,  615,  457,  648,   -2,  538,  297,
 /*   130 */   656,  687,  672,   -2,  678,  538,  136,  701,  692,   -2,
 /*   140 */   695,  538,  175,  717,  708,   -2,  538,   58,  521,   -2,
 /*   150 */    -2,  538,  204,  538,  297,   -2,  379,  738,  352,  297,
 /*   160 */   730,   -2,  442,  511,   -2,   -2,  538,  357,  538,  357,
 /*   170 */    -2,   -2,   -2,   -2,   -2,   -2,   -2,   -2,   -2,   -2,
 /*   180 */    -2,   -2,   -2,  538,  297,   -2,  741,   -2,   -2,  740,
 /*   190 */   745,  743,  769,  316,  775,  780,  781,  463,  778,  790,
 /*   200 */   792,  512,  797,  805,  808,  809,   -2,   -2,  445,   -2,
 /*   210 */    -2,  472,  499,   -2,   87,  810,   -2,   -2,   -2,   -2,
 /*   220 */    -2,  248,   -2,  799,   -2,   -2,  538,  297,  791,  538,
 /*   230 */   818,  538,   -2,  297,   -2,  538,  297,  612,   -2,  802,
 /*   240 */    -2,  538,  357,  806,  325,  621,   -2,  804,   -2,  103,
 /*   250 */    -2,  854,  855,  807,  615,  518,  811,  538,  566,   -2,
 /*   260 */   820,  647,  812,  813,  538,  686,   -2,  820,   -2,   -2,
 /*   270 */    -2,  400,  550,  862,  299,  866,  615,  814,  615,  837,
 /*   280 */   615,   -2,  867,  538,   -2,  297,   -2,  400,  550,  872,
 /*   290 */   840,  615,   -2,
};
#define YY_REDUCE_USE_DFLT (-1)
static short yy_reduce_ofst[] = {
 /*     0 */   548,   -1,   -1,   -1,   -1,  416,   -1,   -1,  345,   -1,
 /*    10 */    -1,  479,  577,   -1,  579,   -1,  580,   -1,  582,   -1,
 /*    20 */   583,   -1,  585,   -1,  589,   -1,  602,   -1,  605,   -1,
 /*    30 */   610,   -1,  617,   -1,  619,   -1,  622,   -1,  624,   -1,
 /*    40 */   625,   -1,  626,   -1,  627,   -1,  629,   -1,  632,   -1,
 /*    50 */   641,   -1,  657,   -1,  660,   -1,  662,   -1,   -1,  663,
 /*    60 */    -1,  664,   -1,   -1,  323,  377,  437,  468,  477,  478,
 /*    70 */   502,   -1,   -1,   -1,  493,   -1,  513,   -1,   -1,   -1,
 /*    80 */    -1,   -1,  519,   -1,   -1,  254,   -1,  542,   -1,   -1,
 /*    90 */    -1,   -1,  665,   -1,   -1,   -1,   -1,   -1,  666,   -1,
 /*   100 */   429,  551,  439,   -1,  560,   20,   -1,  292,   -1,   -1,
 /*   110 */    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
 /*   120 */    -1,  276,   -1,   -1,  240,  581,  588,   -1,  667,   -1,
 /*   130 */    -1,  321,   -1,   -1,   -1,  669,   -1,   -1,   -1,   -1,
 /*   140 */    -1,  688,   -1,   -1,   -1,   -1,  690,  320,  643,   -1,
 /*   150 */    -1,  697,   -1,  700,   -1,   -1,   -1,  642,  372,   -1,
 /*   160 */    -1,   -1,   -1,  466,   -1,   -1,  702,   -1,  703,   -1,
 /*   170 */    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
 /*   180 */    -1,   -1,   -1,  705,   -1,   -1,   -1,   -1,   -1,   -1,
 /*   190 */    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
 /*   200 */    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  555,   -1,
 /*   210 */    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
 /*   220 */    -1,   -1,   -1,   -1,   -1,   -1,  707,   -1,   -1,  426,
 /*   230 */    -1,  563,   -1,   -1,   -1,  709,   -1,   -1,   -1,   -1,
 /*   240 */    -1,  710,   -1,   -1,  336,   -1,   -1,   -1,   -1,   -1,
 /*   250 */    -1,   -1,   -1,   -1,  279,   -1,   -1,  540,   -1,   -1,
 /*   260 */   443,   -1,   -1,   -1,  541,   -1,   -1,  742,   -1,   -1,
 /*   270 */    -1,  768,  746,   -1,  735,   -1,  698,   -1,  424,   -1,
 /*   280 */   465,   -1,   -1,  575,   -1,   -1,   -1,  776,  752,   -1,
 /*   290 */    -1,  729,   -1,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */   453,  453,  453,  453,  453,  453,  453,  332,  453,  335,
 /*    10 */   337,  453,  453,  381,  453,  383,  453,  384,  453,  385,
 /*    20 */   453,  386,  453,  387,  453,  388,  453,  389,  453,  390,
 /*    30 */   453,  391,  453,  392,  453,  393,  453,  394,  453,  395,
 /*    40 */   453,  396,  453,  397,  453,  398,  453,  399,  453,  400,
 /*    50 */   453,  401,  453,  402,  453,  403,  453,  404,  453,  453,
 /*    60 */   405,  453,  406,  453,  453,  354,  363,  368,  356,  374,
 /*    70 */   370,  298,  453,  369,  453,  371,  453,  372,  377,  378,
 /*    80 */   379,  380,  453,  373,  453,  453,  355,  453,  357,  359,
 /*    90 */   360,  361,  453,  414,  453,  432,  453,  433,  453,  434,
 /*   100 */   453,  302,  453,  453,  453,  313,  299,  453,  310,  351,
 /*   110 */   453,  349,  350,  352,  452,  453,  450,  453,  449,  453,
 /*   120 */   451,  312,  314,  316,  453,  320,  331,  317,  453,  330,
 /*   130 */   453,  453,  453,  413,  453,  453,  453,  453,  453,  415,
 /*   140 */   453,  453,  453,  453,  453,  416,  453,  453,  453,  417,
 /*   150 */   418,  453,  453,  453,  420,  422,  452,  425,  453,  431,
 /*   160 */   453,  423,  453,  453,  428,  430,  453,  435,  453,  436,
 /*   170 */   438,  439,  440,  441,  442,  443,  444,  445,  446,  447,
 /*   180 */   448,  429,  424,  453,  421,  419,  453,  318,  319,  453,
 /*   190 */   453,  453,  453,  453,  453,  453,  453,  453,  453,  453,
 /*   200 */   453,  453,  453,  453,  453,  453,  315,  311,  453,  303,
 /*   210 */   305,  452,  453,  306,  309,  453,  307,  308,  304,  300,
 /*   220 */   301,  453,  437,  453,  410,  358,  453,  367,  453,  453,
 /*   230 */   362,  453,  364,  366,  365,  453,  353,  453,  408,  453,
 /*   240 */   411,  453,  407,  453,  453,  453,  409,  453,  412,  382,
 /*   250 */   336,  453,  453,  453,  453,  453,  453,  453,  453,  333,
 /*   260 */   453,  453,  453,  453,  453,  453,  334,  453,  338,  340,
 /*   270 */   339,  354,  453,  453,  453,  453,  453,  453,  453,  342,
 /*   280 */   453,  343,  453,  453,  345,  346,  344,  354,  453,  453,
 /*   290 */   453,  453,  348,
};
#define YY_SZ_ACTTAB (sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammer, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
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
*/
struct yyStackEntry {
  int stateno;       /* The state-number */
  int major;         /* The major token value.  This is the code
                     ** number for the token at this stack level */
  YYMINORTYPE minor; /* The user-supplied minor token value.  This
                     ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
  int yyerrcnt;                 /* Shifts left before out of the error */
  phql_ARG_SDECL                /* A place to hold %extra_argument */
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
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
static const char *yyTokenName[] = { 
  "$",             "AGAINST",       "BETWEEN",       "EQUALS",      
  "NOTEQUALS",     "LESS",          "GREATER",       "GREATEREQUAL",
  "LESSEQUAL",     "TS_MATCHES",    "TS_OR",         "TS_AND",      
  "TS_NEGATE",     "TS_CONTAINS_ANOTHER",  "TS_CONTAINS_IN",  "AND",         
  "OR",            "LIKE",          "ILIKE",         "BITWISE_AND", 
  "BITWISE_OR",    "BITWISE_XOR",   "DIVIDE",        "TIMES",       
  "MOD",           "PLUS",          "MINUS",         "IS",          
  "IN",            "NOT",           "BITWISE_NOT",   "COMMA",       
  "SELECT",        "FROM",          "DISTINCT",      "ALL",         
  "IDENTIFIER",    "DOT",           "AS",            "INNER",       
  "JOIN",          "CROSS",         "LEFT",          "OUTER",       
  "RIGHT",         "FULL",          "ON",            "PARENTHESES_OPEN",
  "PARENTHESES_CLOSE",  "INSERT",        "INTO",          "VALUES",      
  "UPDATE",        "SET",           "DELETE",        "WHERE",       
  "ORDER",         "BY",            "ASC",           "DESC",        
  "GROUP",         "HAVING",        "FOR",           "LIMIT",       
  "OFFSET",        "INTEGER",       "HINTEGER",      "NPLACEHOLDER",
  "SPLACEHOLDER",  "EXISTS",        "CAST",          "CONVERT",     
  "USING",         "CASE",          "END",           "WHEN",        
  "THEN",          "ELSE",          "NULL",          "STRING",      
  "DOUBLE",        "TRUE",          "FALSE",         "BPLACEHOLDER",
  "COLON",         "error",         "program",       "query_language",
  "select_statement",  "insert_statement",  "update_statement",  "delete_statement",
  "select_clause",  "where_clause",  "group_clause",  "having_clause",
  "order_clause",  "select_limit_clause",  "for_update_clause",  "distinct_all",
  "column_list",   "associated_name_list",  "join_list_or_null",  "column_item", 
  "expr",          "associated_name",  "join_list",     "join_item",   
  "join_clause",   "join_type",     "aliased_or_qualified_name",  "join_associated_name",
  "join_conditions",  "values_list",   "field_list",    "value_list",  
  "value_item",    "field_item",    "update_clause",  "limit_clause",
  "update_item_list",  "update_item",   "qualified_name",  "new_value",   
  "delete_clause",  "order_list",    "order_item",    "group_list",  
  "group_item",    "integer_or_placeholder",  "argument_list",  "when_clauses",
  "when_clause",   "function_call",  "distinct_or_null",  "argument_list_or_null",
  "argument_item",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *yyRuleName[] = {
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
 /*  56 */ "associated_name ::= aliased_or_qualified_name AS IDENTIFIER",
 /*  57 */ "associated_name ::= aliased_or_qualified_name IDENTIFIER",
 /*  58 */ "associated_name ::= aliased_or_qualified_name",
 /*  59 */ "aliased_or_qualified_name ::= qualified_name",
 /*  60 */ "where_clause ::= WHERE expr",
 /*  61 */ "where_clause ::=",
 /*  62 */ "order_clause ::= ORDER BY order_list",
 /*  63 */ "order_clause ::=",
 /*  64 */ "order_list ::= order_list COMMA order_item",
 /*  65 */ "order_list ::= order_item",
 /*  66 */ "order_item ::= expr",
 /*  67 */ "order_item ::= expr ASC",
 /*  68 */ "order_item ::= expr DESC",
 /*  69 */ "group_clause ::= GROUP BY group_list",
 /*  70 */ "group_clause ::=",
 /*  71 */ "group_list ::= group_list COMMA group_item",
 /*  72 */ "group_list ::= group_item",
 /*  73 */ "group_item ::= expr",
 /*  74 */ "having_clause ::= HAVING expr",
 /*  75 */ "having_clause ::=",
 /*  76 */ "for_update_clause ::= FOR UPDATE",
 /*  77 */ "for_update_clause ::=",
 /*  78 */ "select_limit_clause ::= LIMIT integer_or_placeholder",
 /*  79 */ "select_limit_clause ::= LIMIT integer_or_placeholder COMMA integer_or_placeholder",
 /*  80 */ "select_limit_clause ::= LIMIT integer_or_placeholder OFFSET integer_or_placeholder",
 /*  81 */ "select_limit_clause ::=",
 /*  82 */ "limit_clause ::= LIMIT integer_or_placeholder",
 /*  83 */ "limit_clause ::=",
 /*  84 */ "integer_or_placeholder ::= INTEGER",
 /*  85 */ "integer_or_placeholder ::= HINTEGER",
 /*  86 */ "integer_or_placeholder ::= NPLACEHOLDER",
 /*  87 */ "integer_or_placeholder ::= SPLACEHOLDER",
 /*  88 */ "expr ::= MINUS expr",
 /*  89 */ "expr ::= expr MINUS expr",
 /*  90 */ "expr ::= expr PLUS expr",
 /*  91 */ "expr ::= expr TIMES expr",
 /*  92 */ "expr ::= expr DIVIDE expr",
 /*  93 */ "expr ::= expr MOD expr",
 /*  94 */ "expr ::= expr AND expr",
 /*  95 */ "expr ::= expr OR expr",
 /*  96 */ "expr ::= expr BITWISE_AND expr",
 /*  97 */ "expr ::= expr BITWISE_OR expr",
 /*  98 */ "expr ::= expr BITWISE_XOR expr",
 /*  99 */ "expr ::= expr EQUALS expr",
 /* 100 */ "expr ::= expr NOTEQUALS expr",
 /* 101 */ "expr ::= expr LESS expr",
 /* 102 */ "expr ::= expr GREATER expr",
 /* 103 */ "expr ::= expr GREATEREQUAL expr",
 /* 104 */ "expr ::= expr TS_MATCHES expr",
 /* 105 */ "expr ::= expr TS_OR expr",
 /* 106 */ "expr ::= expr TS_AND expr",
 /* 107 */ "expr ::= expr TS_NEGATE expr",
 /* 108 */ "expr ::= expr TS_CONTAINS_ANOTHER expr",
 /* 109 */ "expr ::= expr TS_CONTAINS_IN expr",
 /* 110 */ "expr ::= expr LESSEQUAL expr",
 /* 111 */ "expr ::= expr LIKE expr",
 /* 112 */ "expr ::= expr NOT LIKE expr",
 /* 113 */ "expr ::= expr ILIKE expr",
 /* 114 */ "expr ::= expr NOT ILIKE expr",
 /* 115 */ "expr ::= expr IN PARENTHESES_OPEN argument_list PARENTHESES_CLOSE",
 /* 116 */ "expr ::= expr NOT IN PARENTHESES_OPEN argument_list PARENTHESES_CLOSE",
 /* 117 */ "expr ::= PARENTHESES_OPEN select_statement PARENTHESES_CLOSE",
 /* 118 */ "expr ::= expr IN PARENTHESES_OPEN select_statement PARENTHESES_CLOSE",
 /* 119 */ "expr ::= expr NOT IN PARENTHESES_OPEN select_statement PARENTHESES_CLOSE",
 /* 120 */ "expr ::= EXISTS PARENTHESES_OPEN select_statement PARENTHESES_CLOSE",
 /* 121 */ "expr ::= expr AGAINST expr",
 /* 122 */ "expr ::= CAST PARENTHESES_OPEN expr AS IDENTIFIER PARENTHESES_CLOSE",
 /* 123 */ "expr ::= CONVERT PARENTHESES_OPEN expr USING IDENTIFIER PARENTHESES_CLOSE",
 /* 124 */ "expr ::= CASE expr when_clauses END",
 /* 125 */ "when_clauses ::= when_clauses when_clause",
 /* 126 */ "when_clauses ::= when_clause",
 /* 127 */ "when_clause ::= WHEN expr THEN expr",
 /* 128 */ "when_clause ::= ELSE expr",
 /* 129 */ "expr ::= function_call",
 /* 130 */ "function_call ::= IDENTIFIER PARENTHESES_OPEN distinct_or_null argument_list_or_null PARENTHESES_CLOSE",
 /* 131 */ "distinct_or_null ::= DISTINCT",
 /* 132 */ "distinct_or_null ::=",
 /* 133 */ "argument_list_or_null ::= argument_list",
 /* 134 */ "argument_list_or_null ::=",
 /* 135 */ "argument_list ::= argument_list COMMA argument_item",
 /* 136 */ "argument_list ::= argument_item",
 /* 137 */ "argument_item ::= TIMES",
 /* 138 */ "argument_item ::= expr",
 /* 139 */ "expr ::= expr IS NULL",
 /* 140 */ "expr ::= expr IS NOT NULL",
 /* 141 */ "expr ::= expr BETWEEN expr",
 /* 142 */ "expr ::= NOT expr",
 /* 143 */ "expr ::= BITWISE_NOT expr",
 /* 144 */ "expr ::= PARENTHESES_OPEN expr PARENTHESES_CLOSE",
 /* 145 */ "expr ::= qualified_name",
 /* 146 */ "expr ::= INTEGER",
 /* 147 */ "expr ::= HINTEGER",
 /* 148 */ "expr ::= STRING",
 /* 149 */ "expr ::= DOUBLE",
 /* 150 */ "expr ::= NULL",
 /* 151 */ "expr ::= TRUE",
 /* 152 */ "expr ::= FALSE",
 /* 153 */ "expr ::= NPLACEHOLDER",
 /* 154 */ "expr ::= SPLACEHOLDER",
 /* 155 */ "expr ::= BPLACEHOLDER",
 /* 156 */ "qualified_name ::= IDENTIFIER COLON IDENTIFIER DOT IDENTIFIER",
 /* 157 */ "qualified_name ::= IDENTIFIER COLON IDENTIFIER",
 /* 158 */ "qualified_name ::= IDENTIFIER DOT IDENTIFIER",
 /* 159 */ "qualified_name ::= IDENTIFIER",
};
#endif /* NDEBUG */

/*
** This function returns the symbolic name associated with a token
** value.
*/
const char *phql_TokenName(int tokenType){
#ifndef NDEBUG
  if( tokenType>0 && (size_t)tokenType<(sizeof(yyTokenName)/sizeof(yyTokenName[0])) ){
    return yyTokenName[tokenType];
  }else{
    return "Unknown";
  }
#else
  return "";
#endif
}

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
void *phql_Alloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(YYCODETYPE yymajor, YYMINORTYPE *yypminor){
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
/* #line 563 "parser.y" */
{
	if ((yypminor->yy0)) {
		if ((yypminor->yy0)->free_flag) {
			efree((yypminor->yy0)->token);
		}
		efree((yypminor->yy0));
	}
}
/* #line 1296 "parser.c" */
      break;
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 100:
    case 101:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 114:
    case 115:
    case 117:
    case 118:
    case 120:
    case 121:
    case 122:
    case 124:
    case 125:
    case 126:
    case 127:
    case 128:
    case 130:
    case 133:
    case 136:
/* #line 576 "parser.y" */
{ zval_ptr_dtor((yypminor->yy68)); }
/* #line 1331 "parser.c" */
      break;
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
    case 111:
    case 112:
    case 119:
    case 134:
    case 135:
/* #line 868 "parser.y" */
{ phalcon_safe_zval_ptr_dtor((yypminor->yy68)); }
/* #line 1347 "parser.c" */
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor( yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from phql_Alloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void phql_Free(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
  (*freeProc)((void*)pParser);
}

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  /* if( pParser->yyidx<0 ) return YY_NO_ACTION;  */
  i = yy_shift_ofst[stateno];
  if( i==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=(int)YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
#ifdef YYFALLBACK
    int iFallback;            /* Fallback token */
    if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
           && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
           yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
      }
#endif
      return yy_find_shift_action(pParser, iFallback);
    }
#endif
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  i = yy_reduce_ofst[stateno];
  if( i==YY_REDUCE_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=(int)YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer ot the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
  if( yypParser->yyidx>=YYSTACKDEPTH ){
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
     phql_ARG_STORE; /* Suppress warning about unused %extra_argument var */
     return;
  }
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = yyNewState;
  yytos->major = yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 86, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 87, 1 },
  { 88, 7 },
  { 92, 6 },
  { 99, 1 },
  { 99, 1 },
  { 99, 0 },
  { 100, 3 },
  { 100, 1 },
  { 103, 1 },
  { 103, 3 },
  { 103, 3 },
  { 103, 2 },
  { 103, 1 },
  { 101, 3 },
  { 101, 1 },
  { 102, 1 },
  { 102, 0 },
  { 106, 2 },
  { 106, 1 },
  { 107, 1 },
  { 108, 4 },
  { 111, 2 },
  { 111, 1 },
  { 111, 0 },
  { 109, 2 },
  { 109, 2 },
  { 109, 3 },
  { 109, 2 },
  { 109, 3 },
  { 109, 2 },
  { 109, 3 },
  { 109, 2 },
  { 109, 1 },
  { 112, 2 },
  { 112, 0 },
  { 89, 5 },
  { 89, 7 },
  { 89, 10 },
  { 113, 3 },
  { 113, 1 },
  { 116, 1 },
  { 114, 3 },
  { 114, 1 },
  { 117, 1 },
  { 90, 3 },
  { 118, 4 },
  { 120, 3 },
  { 120, 1 },
  { 121, 3 },
  { 123, 1 },
  { 91, 3 },
  { 124, 3 },
  { 105, 3 },
  { 105, 2 },
  { 105, 1 },
  { 110, 1 },
  { 93, 2 },
  { 93, 0 },
  { 96, 3 },
  { 96, 0 },
  { 125, 3 },
  { 125, 1 },
  { 126, 1 },
  { 126, 2 },
  { 126, 2 },
  { 94, 3 },
  { 94, 0 },
  { 127, 3 },
  { 127, 1 },
  { 128, 1 },
  { 95, 2 },
  { 95, 0 },
  { 98, 2 },
  { 98, 0 },
  { 97, 2 },
  { 97, 4 },
  { 97, 4 },
  { 97, 0 },
  { 119, 2 },
  { 119, 0 },
  { 129, 1 },
  { 129, 1 },
  { 129, 1 },
  { 129, 1 },
  { 104, 2 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 3 },
  { 104, 4 },
  { 104, 3 },
  { 104, 4 },
  { 104, 5 },
  { 104, 6 },
  { 104, 3 },
  { 104, 5 },
  { 104, 6 },
  { 104, 4 },
  { 104, 3 },
  { 104, 6 },
  { 104, 6 },
  { 104, 4 },
  { 131, 2 },
  { 131, 1 },
  { 132, 4 },
  { 132, 2 },
  { 104, 1 },
  { 133, 5 },
  { 134, 1 },
  { 134, 0 },
  { 135, 1 },
  { 135, 0 },
  { 130, 3 },
  { 130, 1 },
  { 136, 1 },
  { 136, 1 },
  { 104, 3 },
  { 104, 4 },
  { 104, 3 },
  { 104, 2 },
  { 104, 2 },
  { 104, 3 },
  { 104, 1 },
  { 104, 1 },
  { 104, 1 },
  { 104, 1 },
  { 104, 1 },
  { 104, 1 },
  { 104, 1 },
  { 104, 1 },
  { 104, 1 },
  { 104, 1 },
  { 104, 1 },
  { 122, 5 },
  { 122, 3 },
  { 122, 3 },
  { 122, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  phql_ARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0:
/* #line 572 "parser.y" */
{
	status->ret = yymsp[0].minor.yy68;
}
/* #line 1724 "parser.c" */
        break;
      case 1:
      case 2:
      case 3:
      case 4:
      case 18:
      case 19:
      case 22:
      case 23:
      case 44:
      case 51:
      case 53:
      case 59:
      case 65:
      case 72:
      case 73:
      case 129:
      case 133:
      case 138:
      case 145:
/* #line 578 "parser.y" */
{
	yygotominor.yy68 = yymsp[0].minor.yy68;
}
/* #line 1749 "parser.c" */
        break;
      case 5:
/* #line 596 "parser.y" */
{
	yygotominor.yy68 = phql_ret_select_statement(yymsp[-6].minor.yy68, yymsp[-5].minor.yy68, yymsp[-2].minor.yy68, yymsp[-4].minor.yy68, yymsp[-3].minor.yy68, yymsp[-1].minor.yy68, yymsp[0].minor.yy68);
}
/* #line 1756 "parser.c" */
        break;
      case 6:
/* #line 602 "parser.y" */
{
	yygotominor.yy68 = phql_ret_select_clause(yymsp[-4].minor.yy68, yymsp[-3].minor.yy68, yymsp[-1].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(32,&yymsp[-5].minor);
  yy_destructor(33,&yymsp[-2].minor);
}
/* #line 1765 "parser.c" */
        break;
      case 7:
/* #line 608 "parser.y" */
{
	yygotominor.yy68 = phql_ret_distinct_all(1);
  yy_destructor(34,&yymsp[0].minor);
}
/* #line 1773 "parser.c" */
        break;
      case 8:
/* #line 612 "parser.y" */
{
	yygotominor.yy68 = phql_ret_distinct_all(0);
  yy_destructor(35,&yymsp[0].minor);
}
/* #line 1781 "parser.c" */
        break;
      case 9:
      case 20:
      case 27:
      case 38:
      case 61:
      case 63:
      case 70:
      case 75:
      case 77:
      case 81:
      case 83:
      case 132:
      case 134:
/* #line 616 "parser.y" */
{
	yygotominor.yy68 = NULL;
}
/* #line 1800 "parser.c" */
        break;
      case 10:
      case 17:
      case 42:
      case 45:
      case 50:
      case 64:
      case 71:
      case 135:
/* #line 622 "parser.y" */
{
	yygotominor.yy68 = phql_ret_zval_list(yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(31,&yymsp[-1].minor);
}
/* #line 1815 "parser.c" */
        break;
      case 11:
      case 43:
      case 46:
      case 126:
      case 136:
/* #line 626 "parser.y" */
{
	yygotominor.yy68 = phql_ret_zval_list(yymsp[0].minor.yy68, NULL);
}
/* #line 1826 "parser.c" */
        break;
      case 12:
      case 137:
/* #line 632 "parser.y" */
{
	yygotominor.yy68 = phql_ret_column_item(PHQL_T_STARALL, NULL, NULL, NULL);
  yy_destructor(23,&yymsp[0].minor);
}
/* #line 1835 "parser.c" */
        break;
      case 13:
/* #line 636 "parser.y" */
{
	yygotominor.yy68 = phql_ret_column_item(PHQL_T_DOMAINALL, NULL, yymsp[-2].minor.yy0, NULL);
  yy_destructor(37,&yymsp[-1].minor);
  yy_destructor(23,&yymsp[0].minor);
}
/* #line 1844 "parser.c" */
        break;
      case 14:
/* #line 640 "parser.y" */
{
	yygotominor.yy68 = phql_ret_column_item(PHQL_T_EXPR, yymsp[-2].minor.yy68, NULL, yymsp[0].minor.yy0);
  yy_destructor(38,&yymsp[-1].minor);
}
/* #line 1852 "parser.c" */
        break;
      case 15:
/* #line 644 "parser.y" */
{
	yygotominor.yy68 = phql_ret_column_item(PHQL_T_EXPR, yymsp[-1].minor.yy68, NULL, yymsp[0].minor.yy0);
}
/* #line 1859 "parser.c" */
        break;
      case 16:
/* #line 648 "parser.y" */
{
	yygotominor.yy68 = phql_ret_column_item(PHQL_T_EXPR, yymsp[0].minor.yy68, NULL, NULL);
}
/* #line 1866 "parser.c" */
        break;
      case 21:
      case 125:
/* #line 672 "parser.y" */
{
	yygotominor.yy68 = phql_ret_zval_list(yymsp[-1].minor.yy68, yymsp[0].minor.yy68);
}
/* #line 1874 "parser.c" */
        break;
      case 24:
/* #line 689 "parser.y" */
{
	yygotominor.yy68 = phql_ret_join_item(yymsp[-3].minor.yy68, yymsp[-2].minor.yy68, yymsp[-1].minor.yy68, yymsp[0].minor.yy68);
}
/* #line 1881 "parser.c" */
        break;
      case 25:
/* #line 695 "parser.y" */
{
	yygotominor.yy68 = phql_ret_qualified_name(NULL, NULL, yymsp[0].minor.yy0);
  yy_destructor(38,&yymsp[-1].minor);
}
/* #line 1889 "parser.c" */
        break;
      case 26:
      case 47:
      case 159:
/* #line 699 "parser.y" */
{
	yygotominor.yy68 = phql_ret_qualified_name(NULL, NULL, yymsp[0].minor.yy0);
}
/* #line 1898 "parser.c" */
        break;
      case 28:
/* #line 709 "parser.y" */
{
	yygotominor.yy68 = phql_ret_join_type(PHQL_T_INNERJOIN);
  yy_destructor(39,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
/* #line 1907 "parser.c" */
        break;
      case 29:
/* #line 713 "parser.y" */
{
	yygotominor.yy68 = phql_ret_join_type(PHQL_T_CROSSJOIN);
  yy_destructor(41,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
/* #line 1916 "parser.c" */
        break;
      case 30:
/* #line 717 "parser.y" */
{
	yygotominor.yy68 = phql_ret_join_type(PHQL_T_LEFTJOIN);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(43,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
/* #line 1926 "parser.c" */
        break;
      case 31:
/* #line 721 "parser.y" */
{
	yygotominor.yy68 = phql_ret_join_type(PHQL_T_LEFTJOIN);
  yy_destructor(42,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
/* #line 1935 "parser.c" */
        break;
      case 32:
/* #line 725 "parser.y" */
{
	yygotominor.yy68 = phql_ret_join_type(PHQL_T_RIGHTJOIN);
  yy_destructor(44,&yymsp[-2].minor);
  yy_destructor(43,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
/* #line 1945 "parser.c" */
        break;
      case 33:
/* #line 729 "parser.y" */
{
	yygotominor.yy68 = phql_ret_join_type(PHQL_T_RIGHTJOIN);
  yy_destructor(44,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
/* #line 1954 "parser.c" */
        break;
      case 34:
/* #line 733 "parser.y" */
{
	yygotominor.yy68 = phql_ret_join_type(PHQL_T_FULLJOIN);
  yy_destructor(45,&yymsp[-2].minor);
  yy_destructor(43,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
/* #line 1964 "parser.c" */
        break;
      case 35:
/* #line 737 "parser.y" */
{
	yygotominor.yy68 = phql_ret_join_type(PHQL_T_FULLJOIN);
  yy_destructor(45,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
/* #line 1973 "parser.c" */
        break;
      case 36:
/* #line 741 "parser.y" */
{
	yygotominor.yy68 = phql_ret_join_type(PHQL_T_INNERJOIN);
  yy_destructor(40,&yymsp[0].minor);
}
/* #line 1981 "parser.c" */
        break;
      case 37:
/* #line 747 "parser.y" */
{
	yygotominor.yy68 = yymsp[0].minor.yy68;
  yy_destructor(46,&yymsp[-1].minor);
}
/* #line 1989 "parser.c" */
        break;
      case 39:
/* #line 758 "parser.y" */
{
	yygotominor.yy68 = phql_ret_insert_statement2(yymsp[-4].minor.yy68, NULL, yymsp[-1].minor.yy68);
  yy_destructor(31,&yymsp[-3].minor);
  yy_destructor(47,&yymsp[-2].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 1999 "parser.c" */
        break;
      case 40:
/* #line 762 "parser.y" */
{
	yygotominor.yy68 = phql_ret_insert_statement(yymsp[-4].minor.yy68, NULL, yymsp[-1].minor.yy68);
  yy_destructor(49,&yymsp[-6].minor);
  yy_destructor(50,&yymsp[-5].minor);
  yy_destructor(51,&yymsp[-3].minor);
  yy_destructor(47,&yymsp[-2].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 2011 "parser.c" */
        break;
      case 41:
/* #line 766 "parser.y" */
{
	yygotominor.yy68 = phql_ret_insert_statement(yymsp[-7].minor.yy68, yymsp[-5].minor.yy68, yymsp[-1].minor.yy68);
  yy_destructor(49,&yymsp[-9].minor);
  yy_destructor(50,&yymsp[-8].minor);
  yy_destructor(47,&yymsp[-6].minor);
  yy_destructor(48,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-3].minor);
  yy_destructor(47,&yymsp[-2].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 2025 "parser.c" */
        break;
      case 48:
/* #line 804 "parser.y" */
{
	yygotominor.yy68 = phql_ret_update_statement(yymsp[-2].minor.yy68, yymsp[-1].minor.yy68, yymsp[0].minor.yy68);
}
/* #line 2032 "parser.c" */
        break;
      case 49:
/* #line 810 "parser.y" */
{
	yygotominor.yy68 = phql_ret_update_clause(yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(52,&yymsp[-3].minor);
  yy_destructor(53,&yymsp[-1].minor);
}
/* #line 2041 "parser.c" */
        break;
      case 52:
/* #line 826 "parser.y" */
{
	yygotominor.yy68 = phql_ret_update_item(yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(3,&yymsp[-1].minor);
}
/* #line 2049 "parser.c" */
        break;
      case 54:
/* #line 838 "parser.y" */
{
	yygotominor.yy68 = phql_ret_delete_statement(yymsp[-2].minor.yy68, yymsp[-1].minor.yy68, yymsp[0].minor.yy68);
}
/* #line 2056 "parser.c" */
        break;
      case 55:
/* #line 844 "parser.y" */
{
	yygotominor.yy68 = phql_ret_delete_clause(yymsp[0].minor.yy68);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(33,&yymsp[-1].minor);
}
/* #line 2065 "parser.c" */
        break;
      case 56:
/* #line 850 "parser.y" */
{
	yygotominor.yy68 = phql_ret_assoc_name(yymsp[-2].minor.yy68, yymsp[0].minor.yy0);
  yy_destructor(38,&yymsp[-1].minor);
}
/* #line 2073 "parser.c" */
        break;
      case 57:
/* #line 854 "parser.y" */
{
	yygotominor.yy68 = phql_ret_assoc_name(yymsp[-1].minor.yy68, yymsp[0].minor.yy0);
}
/* #line 2080 "parser.c" */
        break;
      case 58:
/* #line 858 "parser.y" */
{
	yygotominor.yy68 = phql_ret_assoc_name(yymsp[0].minor.yy68, NULL);
}
/* #line 2087 "parser.c" */
        break;
      case 60:
/* #line 870 "parser.y" */
{
	yygotominor.yy68 = yymsp[0].minor.yy68;
  yy_destructor(55,&yymsp[-1].minor);
}
/* #line 2095 "parser.c" */
        break;
      case 62:
/* #line 880 "parser.y" */
{
	yygotominor.yy68 = yymsp[0].minor.yy68;
  yy_destructor(56,&yymsp[-2].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
/* #line 2104 "parser.c" */
        break;
      case 66:
/* #line 900 "parser.y" */
{
	yygotominor.yy68 = phql_ret_order_item(yymsp[0].minor.yy68, 0);
}
/* #line 2111 "parser.c" */
        break;
      case 67:
/* #line 904 "parser.y" */
{
	yygotominor.yy68 = phql_ret_order_item(yymsp[-1].minor.yy68, PHQL_T_ASC);
  yy_destructor(58,&yymsp[0].minor);
}
/* #line 2119 "parser.c" */
        break;
      case 68:
/* #line 908 "parser.y" */
{
	yygotominor.yy68 = phql_ret_order_item(yymsp[-1].minor.yy68, PHQL_T_DESC);
  yy_destructor(59,&yymsp[0].minor);
}
/* #line 2127 "parser.c" */
        break;
      case 69:
/* #line 914 "parser.y" */
{
	yygotominor.yy68 = yymsp[0].minor.yy68;
  yy_destructor(60,&yymsp[-2].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
/* #line 2136 "parser.c" */
        break;
      case 74:
/* #line 940 "parser.y" */
{
	yygotominor.yy68 = yymsp[0].minor.yy68;
  yy_destructor(61,&yymsp[-1].minor);
}
/* #line 2144 "parser.c" */
        break;
      case 76:
/* #line 950 "parser.y" */
{
	yygotominor.yy68 = phql_ret_for_update_clause();
  yy_destructor(62,&yymsp[-1].minor);
  yy_destructor(52,&yymsp[0].minor);
}
/* #line 2153 "parser.c" */
        break;
      case 78:
      case 82:
/* #line 960 "parser.y" */
{
	yygotominor.yy68 = phql_ret_limit_clause(yymsp[0].minor.yy68, NULL);
  yy_destructor(63,&yymsp[-1].minor);
}
/* #line 2162 "parser.c" */
        break;
      case 79:
/* #line 964 "parser.y" */
{
	yygotominor.yy68 = phql_ret_limit_clause(yymsp[0].minor.yy68, yymsp[-2].minor.yy68);
  yy_destructor(63,&yymsp[-3].minor);
  yy_destructor(31,&yymsp[-1].minor);
}
/* #line 2171 "parser.c" */
        break;
      case 80:
/* #line 968 "parser.y" */
{
	yygotominor.yy68 = phql_ret_limit_clause(yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(63,&yymsp[-3].minor);
  yy_destructor(64,&yymsp[-1].minor);
}
/* #line 2180 "parser.c" */
        break;
      case 84:
      case 146:
/* #line 986 "parser.y" */
{
	yygotominor.yy68 = phql_ret_literal_zval(PHQL_T_INTEGER, yymsp[0].minor.yy0);
}
/* #line 2188 "parser.c" */
        break;
      case 85:
      case 147:
/* #line 990 "parser.y" */
{
	yygotominor.yy68 = phql_ret_literal_zval(PHQL_T_HINTEGER, yymsp[0].minor.yy0);
}
/* #line 2196 "parser.c" */
        break;
      case 86:
      case 153:
/* #line 994 "parser.y" */
{
	yygotominor.yy68 = phql_ret_placeholder_zval(PHQL_T_NPLACEHOLDER, yymsp[0].minor.yy0);
}
/* #line 2204 "parser.c" */
        break;
      case 87:
      case 154:
/* #line 998 "parser.y" */
{
	yygotominor.yy68 = phql_ret_placeholder_zval(PHQL_T_SPLACEHOLDER, yymsp[0].minor.yy0);
}
/* #line 2212 "parser.c" */
        break;
      case 88:
/* #line 1004 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_MINUS, NULL, yymsp[0].minor.yy68);
  yy_destructor(26,&yymsp[-1].minor);
}
/* #line 2220 "parser.c" */
        break;
      case 89:
/* #line 1008 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_SUB, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(26,&yymsp[-1].minor);
}
/* #line 2228 "parser.c" */
        break;
      case 90:
/* #line 1012 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_ADD, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(25,&yymsp[-1].minor);
}
/* #line 2236 "parser.c" */
        break;
      case 91:
/* #line 1016 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_MUL, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(23,&yymsp[-1].minor);
}
/* #line 2244 "parser.c" */
        break;
      case 92:
/* #line 1020 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_DIV, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(22,&yymsp[-1].minor);
}
/* #line 2252 "parser.c" */
        break;
      case 93:
/* #line 1024 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_MOD, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(24,&yymsp[-1].minor);
}
/* #line 2260 "parser.c" */
        break;
      case 94:
/* #line 1028 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_AND, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(15,&yymsp[-1].minor);
}
/* #line 2268 "parser.c" */
        break;
      case 95:
/* #line 1032 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_OR, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(16,&yymsp[-1].minor);
}
/* #line 2276 "parser.c" */
        break;
      case 96:
/* #line 1036 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_BITWISE_AND, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(19,&yymsp[-1].minor);
}
/* #line 2284 "parser.c" */
        break;
      case 97:
/* #line 1040 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_BITWISE_OR, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(20,&yymsp[-1].minor);
}
/* #line 2292 "parser.c" */
        break;
      case 98:
/* #line 1044 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_BITWISE_XOR, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(21,&yymsp[-1].minor);
}
/* #line 2300 "parser.c" */
        break;
      case 99:
/* #line 1048 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_EQUALS, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(3,&yymsp[-1].minor);
}
/* #line 2308 "parser.c" */
        break;
      case 100:
/* #line 1052 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_NOTEQUALS, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(4,&yymsp[-1].minor);
}
/* #line 2316 "parser.c" */
        break;
      case 101:
/* #line 1056 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_LESS, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(5,&yymsp[-1].minor);
}
/* #line 2324 "parser.c" */
        break;
      case 102:
/* #line 1060 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_GREATER, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(6,&yymsp[-1].minor);
}
/* #line 2332 "parser.c" */
        break;
      case 103:
/* #line 1064 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_GREATEREQUAL, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(7,&yymsp[-1].minor);
}
/* #line 2340 "parser.c" */
        break;
      case 104:
/* #line 1068 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_TS_MATCHES, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(9,&yymsp[-1].minor);
}
/* #line 2348 "parser.c" */
        break;
      case 105:
/* #line 1072 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_TS_OR, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(10,&yymsp[-1].minor);
}
/* #line 2356 "parser.c" */
        break;
      case 106:
/* #line 1076 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_TS_AND, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(11,&yymsp[-1].minor);
}
/* #line 2364 "parser.c" */
        break;
      case 107:
/* #line 1080 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_TS_NEGATE, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(12,&yymsp[-1].minor);
}
/* #line 2372 "parser.c" */
        break;
      case 108:
/* #line 1084 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_TS_CONTAINS_ANOTHER, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(13,&yymsp[-1].minor);
}
/* #line 2380 "parser.c" */
        break;
      case 109:
/* #line 1088 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_TS_CONTAINS_IN, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(14,&yymsp[-1].minor);
}
/* #line 2388 "parser.c" */
        break;
      case 110:
/* #line 1092 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_LESSEQUAL, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(8,&yymsp[-1].minor);
}
/* #line 2396 "parser.c" */
        break;
      case 111:
/* #line 1096 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_LIKE, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(17,&yymsp[-1].minor);
}
/* #line 2404 "parser.c" */
        break;
      case 112:
/* #line 1100 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_NLIKE, yymsp[-3].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(29,&yymsp[-2].minor);
  yy_destructor(17,&yymsp[-1].minor);
}
/* #line 2413 "parser.c" */
        break;
      case 113:
/* #line 1104 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_ILIKE, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(18,&yymsp[-1].minor);
}
/* #line 2421 "parser.c" */
        break;
      case 114:
/* #line 1108 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_NILIKE, yymsp[-3].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(29,&yymsp[-2].minor);
  yy_destructor(18,&yymsp[-1].minor);
}
/* #line 2430 "parser.c" */
        break;
      case 115:
      case 118:
/* #line 1112 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_IN, yymsp[-4].minor.yy68, yymsp[-1].minor.yy68);
  yy_destructor(28,&yymsp[-3].minor);
  yy_destructor(47,&yymsp[-2].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 2441 "parser.c" */
        break;
      case 116:
      case 119:
/* #line 1116 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_NOTIN, yymsp[-5].minor.yy68, yymsp[-1].minor.yy68);
  yy_destructor(29,&yymsp[-4].minor);
  yy_destructor(28,&yymsp[-3].minor);
  yy_destructor(47,&yymsp[-2].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 2453 "parser.c" */
        break;
      case 117:
/* #line 1120 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_SUBQUERY, yymsp[-1].minor.yy68, NULL);
  yy_destructor(47,&yymsp[-2].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 2462 "parser.c" */
        break;
      case 120:
/* #line 1132 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_EXISTS, NULL, yymsp[-1].minor.yy68);
  yy_destructor(69,&yymsp[-3].minor);
  yy_destructor(47,&yymsp[-2].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 2472 "parser.c" */
        break;
      case 121:
/* #line 1136 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_AGAINST, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(1,&yymsp[-1].minor);
}
/* #line 2480 "parser.c" */
        break;
      case 122:
/* #line 1140 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_CAST, yymsp[-3].minor.yy68, phql_ret_raw_qualified_name(yymsp[-1].minor.yy0, NULL));
  yy_destructor(70,&yymsp[-5].minor);
  yy_destructor(47,&yymsp[-4].minor);
  yy_destructor(38,&yymsp[-2].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 2491 "parser.c" */
        break;
      case 123:
/* #line 1144 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_CONVERT, yymsp[-3].minor.yy68, phql_ret_raw_qualified_name(yymsp[-1].minor.yy0, NULL));
  yy_destructor(71,&yymsp[-5].minor);
  yy_destructor(47,&yymsp[-4].minor);
  yy_destructor(72,&yymsp[-2].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 2502 "parser.c" */
        break;
      case 124:
/* #line 1148 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_CASE, yymsp[-2].minor.yy68, yymsp[-1].minor.yy68);
  yy_destructor(73,&yymsp[-3].minor);
  yy_destructor(74,&yymsp[0].minor);
}
/* #line 2511 "parser.c" */
        break;
      case 127:
/* #line 1160 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_WHEN, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(75,&yymsp[-3].minor);
  yy_destructor(76,&yymsp[-1].minor);
}
/* #line 2520 "parser.c" */
        break;
      case 128:
/* #line 1164 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_ELSE, yymsp[0].minor.yy68, NULL);
  yy_destructor(77,&yymsp[-1].minor);
}
/* #line 2528 "parser.c" */
        break;
      case 130:
/* #line 1174 "parser.y" */
{
	yygotominor.yy68 = phql_ret_func_call(yymsp[-4].minor.yy0, yymsp[-1].minor.yy68, yymsp[-2].minor.yy68);
  yy_destructor(47,&yymsp[-3].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 2537 "parser.c" */
        break;
      case 131:
/* #line 1180 "parser.y" */
{
	yygotominor.yy68 = phql_ret_distinct();
  yy_destructor(34,&yymsp[0].minor);
}
/* #line 2545 "parser.c" */
        break;
      case 139:
/* #line 1218 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_ISNULL, yymsp[-2].minor.yy68, NULL);
  yy_destructor(27,&yymsp[-1].minor);
  yy_destructor(78,&yymsp[0].minor);
}
/* #line 2554 "parser.c" */
        break;
      case 140:
/* #line 1222 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_ISNOTNULL, yymsp[-3].minor.yy68, NULL);
  yy_destructor(27,&yymsp[-2].minor);
  yy_destructor(29,&yymsp[-1].minor);
  yy_destructor(78,&yymsp[0].minor);
}
/* #line 2564 "parser.c" */
        break;
      case 141:
/* #line 1226 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_BETWEEN, yymsp[-2].minor.yy68, yymsp[0].minor.yy68);
  yy_destructor(2,&yymsp[-1].minor);
}
/* #line 2572 "parser.c" */
        break;
      case 142:
/* #line 1230 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_NOT, NULL, yymsp[0].minor.yy68);
  yy_destructor(29,&yymsp[-1].minor);
}
/* #line 2580 "parser.c" */
        break;
      case 143:
/* #line 1234 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_BITWISE_NOT, NULL, yymsp[0].minor.yy68);
  yy_destructor(30,&yymsp[-1].minor);
}
/* #line 2588 "parser.c" */
        break;
      case 144:
/* #line 1238 "parser.y" */
{
	yygotominor.yy68 = phql_ret_expr(PHQL_T_ENCLOSED, yymsp[-1].minor.yy68, NULL);
  yy_destructor(47,&yymsp[-2].minor);
  yy_destructor(48,&yymsp[0].minor);
}
/* #line 2597 "parser.c" */
        break;
      case 148:
/* #line 1254 "parser.y" */
{
	yygotominor.yy68 = phql_ret_literal_zval(PHQL_T_STRING, yymsp[0].minor.yy0);
}
/* #line 2604 "parser.c" */
        break;
      case 149:
/* #line 1258 "parser.y" */
{
	yygotominor.yy68 = phql_ret_literal_zval(PHQL_T_DOUBLE, yymsp[0].minor.yy0);
}
/* #line 2611 "parser.c" */
        break;
      case 150:
/* #line 1262 "parser.y" */
{
	yygotominor.yy68 = phql_ret_literal_zval(PHQL_T_NULL, NULL);
  yy_destructor(78,&yymsp[0].minor);
}
/* #line 2619 "parser.c" */
        break;
      case 151:
/* #line 1266 "parser.y" */
{
	yygotominor.yy68 = phql_ret_literal_zval(PHQL_T_TRUE, NULL);
  yy_destructor(81,&yymsp[0].minor);
}
/* #line 2627 "parser.c" */
        break;
      case 152:
/* #line 1270 "parser.y" */
{
	yygotominor.yy68 = phql_ret_literal_zval(PHQL_T_FALSE, NULL);
  yy_destructor(82,&yymsp[0].minor);
}
/* #line 2635 "parser.c" */
        break;
      case 155:
/* #line 1285 "parser.y" */
{
	yygotominor.yy68 = phql_ret_placeholder_zval(PHQL_T_BPLACEHOLDER, yymsp[0].minor.yy0);
}
/* #line 2642 "parser.c" */
        break;
      case 156:
/* #line 1291 "parser.y" */
{
	yygotominor.yy68 = phql_ret_qualified_name(yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, yymsp[0].minor.yy0);
  yy_destructor(84,&yymsp[-3].minor);
  yy_destructor(37,&yymsp[-1].minor);
}
/* #line 2651 "parser.c" */
        break;
      case 157:
/* #line 1295 "parser.y" */
{
	yygotominor.yy68 = phql_ret_qualified_name(yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy0);
  yy_destructor(84,&yymsp[-1].minor);
}
/* #line 2659 "parser.c" */
        break;
      case 158:
/* #line 1299 "parser.y" */
{
	yygotominor.yy68 = phql_ret_qualified_name(NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy0);
  yy_destructor(37,&yymsp[-1].minor);
}
/* #line 2667 "parser.c" */
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yypParser,yygoto);
  if( yyact < YYNSTATE ){
    yy_shift(yypParser,yyact,yygoto,&yygotominor);
  }else if( yyact == YYNSTATE + YYNRULE + 1 ){
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
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
  phql_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  phql_ARG_FETCH;
#define TOKEN (yyminor.yy0)
/* #line 496 "parser.y" */

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

/* #line 2776 "parser.c" */
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
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
    if( yymajor==0 ) return;
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  phql_ARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,yymajor);
    if( yyact<YYNSTATE ){
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      if( yyendofinput && yypParser->yyidx>=0 ){
        yymajor = 0;
      }else{
        yymajor = YYNOCODE;
      }
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else if( yyact == YY_ERROR_ACTION ){
      int yymx;
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
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_shift_action(yypParser,YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
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
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }else{
      yy_accept(yypParser);
      yymajor = YYNOCODE;
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
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
static void phql_scanner_error_msg(phql_parser_status *parser_status, zval **error_msg){

	char *error = NULL, *error_part;
	unsigned int length;
	phql_scanner_state *state = parser_status->scanner_state;

	PHALCON_ALLOC_INIT_ZVAL(*error_msg);
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
		ZVAL_STRING(*error_msg, error);
	} else {
		ZVAL_STRING(*error_msg, "Scanning error near to EOF");
	}

	if (error) {
		efree(error);
	}
}

/**
 * Executes the internal PHQL parser/tokenizer
 */
int phql_parse_phql(zval *result, zval *phql) {

	zval *error_msg = NULL;

	ZVAL_NULL(result);

	if (phql_internal_parse_phql(&result, Z_STRVAL_P(phql), Z_STRLEN_P(phql), &error_msg) == FAILURE) {
		if (likely(error_msg != NULL)) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, Z_STRVAL_P(error_msg));
			zval_ptr_dtor(error_msg);
		}
		else {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "There was an error parsing PHQL");
		}

		return FAILURE;
	}

	return SUCCESS;
}

/**
 * Executes a PHQL parser/tokenizer
 */
int phql_internal_parse_phql(zval **result, char *phql, unsigned int phql_length, zval **error_msg) {

	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;
	phql_parser_status *parser_status = NULL;
	int scanner_status, status = SUCCESS, error_length;
	phql_scanner_state *state;
	phql_scanner_token token;
	void* phql_parser;
	char *error;
	zval unique_id;

	if (!phql) {
		PHALCON_ALLOC_INIT_ZVAL(*error_msg);
		ZVAL_STRING(*error_msg, "PHQL statement cannot be NULL");
		return FAILURE;
	}

	ZVAL_LONG(&unique_id, zend_inline_hash_func(phql, phql_length));

	phalcon_orm_get_prepared_ast(result, &unique_id);

	if (Z_TYPE_P(*result) == IS_ARRAY) {
		zval_ptr_dtor(&unique_id);
		return SUCCESS;
	}

	phql_parser = phql_Alloc(phql_wrapper_alloc);
	if (unlikely(!phql_parser)) {
		PHALCON_ALLOC_INIT_ZVAL(*error_msg);
		ZVAL_STRING(*error_msg, "Memory allocation error");
		return FAILURE;
	}

	parser_status = emalloc(sizeof(phql_parser_status));
	state = emalloc(sizeof(phql_scanner_state));

	parser_status->status = PHQL_PARSING_OK;
	parser_status->scanner_state = state;
	parser_status->ret = NULL;
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
					PHALCON_ALLOC_INIT_ZVAL(*error_msg);
					ZVAL_STRING(*error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_DOUBLE:
				if (parser_status->enable_literals) {
					phql_parse_with_token(phql_parser, PHQL_T_DOUBLE, PHQL_DOUBLE, &token, parser_status);
				} else {
					PHALCON_ALLOC_INIT_ZVAL(*error_msg);
					ZVAL_STRING(*error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_STRING:
				if (parser_status->enable_literals) {
					phql_parse_with_token(phql_parser, PHQL_T_STRING, PHQL_STRING, &token, parser_status);
				} else {
					PHALCON_ALLOC_INIT_ZVAL(*error_msg);
					ZVAL_STRING(*error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_TRUE:
				if (parser_status->enable_literals) {
					phql_(phql_parser, PHQL_TRUE, NULL, parser_status);
				} else {
					PHALCON_ALLOC_INIT_ZVAL(*error_msg);
					ZVAL_STRING(*error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_FALSE:
				if (parser_status->enable_literals) {
					phql_(phql_parser, PHQL_FALSE, NULL, parser_status);
				} else {
					PHALCON_ALLOC_INIT_ZVAL(*error_msg);
					ZVAL_STRING(*error_msg, "Literals are disabled in PHQL statements");
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
				PHALCON_ALLOC_INIT_ZVAL(*error_msg);
				ZVAL_STRING(*error_msg, error);
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
				if (!*error_msg) {
					if (!*error_msg) {
						phql_scanner_error_msg(parser_status, error_msg);
					}
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
			if (!*error_msg) {
				PHALCON_ALLOC_INIT_ZVAL(*error_msg);
				ZVAL_STRING(*error_msg, parser_status->syntax_error);
			}
			efree(parser_status->syntax_error);
		}
	}

	phql_Free(phql_parser, phql_wrapper_free);

	if (status != FAILURE) {
		if (parser_status->status == PHQL_PARSING_OK) {
			if (parser_status->ret) {

				/**
				 * Set a unique id for the parsed ast
				 */
				if (phalcon_globals_ptr->orm.cache_level >= 1) {
					if (Z_TYPE_P(parser_status->ret) == IS_ARRAY) {
						add_assoc_long(parser_status->ret, "id", Z_LVAL(unique_id));
					}
				}

				ZVAL_ZVAL(*result, parser_status->ret, 0, 0);
				ZVAL_NULL(parser_status->ret);
				zval_ptr_dtor(parser_status->ret);

				/**
				 * Store the parsed definition in the cache
				 */
				phalcon_orm_set_prepared_ast(&unique_id, *result);

			} else {
				efree(parser_status->ret);
			}
		}
	}

	zval_ptr_dtor(&unique_id);

	efree(parser_status);
	efree(state);

	return status;
}
