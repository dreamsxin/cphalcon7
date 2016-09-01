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
/* #line 56 "parser.y" */


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
	array_init(ret);
	add_assoc_long(ret, ISV(type), PHQL_T_INSERT);
	add_assoc_zval(ret, ISV(qualifiedName), Q);

	if (F && Z_TYPE_P(F) != IS_UNDEF) {
		add_assoc_zval(ret, ISV(fields), F);
	}
	add_assoc_zval(ret, ISV(values), V);
}

static void phql_ret_insert_statement2(zval *ret, zval *Q, zval *F, zval *V)
{
	zval key1, key2, rows, values;

	ZVAL_STR(&key1, IS(rows));

	if (!phalcon_array_isset_fetch(&rows, ret, &key1, 0)) {
		array_init_size(&rows, 1);		

		ZVAL_STR(&key2, IS(values));

		if (phalcon_array_isset_fetch(&values, ret, &key2, 0)) {
			add_next_index_zval(&rows, &values);	
		}
	}

	add_next_index_zval(&rows, V);

	add_assoc_zval(Q, ISV(rows), &rows);

	ZVAL_ZVAL(ret, Q, 1, 1);
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

static void phql_ret_zval_list(zval *ret, zval *list_left, zval *right_list)
{
	HashTable *list;

	array_init(ret);

	if (list_left && Z_TYPE_P(list_left) != IS_UNDEF) {
		list = Z_ARRVAL_P(list_left);
		if (zend_hash_index_exists(list, 0)) {
			zval *item;
			ZEND_HASH_FOREACH_VAL(list, item) {
				add_next_index_zval(ret, item);
			} ZEND_HASH_FOREACH_END();
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

static void phql_ret_assoc_name(zval *ret, zval *qualified_name, phql_parser_token *alias)
{
	array_init_size(ret, 2);
	add_assoc_zval(ret, ISV(qualifiedName), qualified_name);

	if (alias) {
		add_assoc_stringl(ret, ISV(alias), alias->token, alias->token_len);
		efree(alias->token);
		efree(alias);
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

/* #line 378 "parser.c" */
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
#define YYNOCODE 137
#define YYACTIONTYPE unsigned short int
#define phql_TOKENTYPE phql_parser_token*
typedef union {
  int yyinit;
  phql_TOKENTYPE yy0;
  zval yy170;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define phql_ARG_SDECL phql_parser_status *status;
#define phql_ARG_PDECL ,phql_parser_status *status
#define phql_ARG_FETCH phql_parser_status *status = yypParser->status
#define phql_ARG_STORE yypParser->status = status
#define YYNSTATE             199
#define YYNRULE              160
#define YY_MAX_SHIFT         198
#define YY_MIN_SHIFTREDUCE   293
#define YY_MAX_SHIFTREDUCE   452
#define YY_MIN_REDUCE        453
#define YY_MAX_REDUCE        612
#define YY_ERROR_ACTION      613
#define YY_ACCEPT_ACTION     614
#define YY_NO_ACTION         615
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
#define YY_ACTTAB_COUNT (840)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    32,   31,   47,   46,   45,   44,   43,   36,   42,   41,
 /*    10 */    40,   39,   38,   37,   52,   51,   35,   33,   50,   49,
 /*    20 */    48,   54,   55,   53,   56,   58,  148,  194,  132,   32,
 /*    30 */    31,   47,   46,   45,   44,   43,   36,   42,   41,   40,
 /*    40 */    39,   38,   37,   52,   51,   35,   33,   50,   49,   48,
 /*    50 */    54,   55,   53,   56,   58,  148,  194,  132,   56,   58,
 /*    60 */   148,  194,  132,  299,  308,  145,  166,   59,  315,  316,
 /*    70 */    69,  314,  316,   69,   26,  319,   22,  172,   32,   31,
 /*    80 */    47,   46,   45,   44,   43,   36,   42,   41,   40,   39,
 /*    90 */    38,   37,   52,   51,   35,   33,   50,   49,   48,   54,
 /*   100 */    55,   53,   56,   58,  148,  194,  132,   47,   46,   45,
 /*   110 */    44,   43,   36,   42,   41,   40,   39,   38,   37,   52,
 /*   120 */    51,   35,   33,   50,   49,   48,   54,   55,   53,   56,
 /*   130 */    58,  148,  194,  132,  340,  360,  361,   32,   31,   47,
 /*   140 */    46,   45,   44,   43,   36,   42,   41,   40,   39,   38,
 /*   150 */    37,   52,   51,   35,   33,   50,   49,   48,   54,   55,
 /*   160 */    53,   56,   58,  148,  194,  132,   52,   51,   35,   33,
 /*   170 */    50,   49,   48,   54,   55,   53,   56,   58,  148,  194,
 /*   180 */   132,  153,  344,  152,  437,   76,   32,   31,   47,   46,
 /*   190 */    45,   44,   43,   36,   42,   41,   40,   39,   38,   37,
 /*   200 */    52,   51,   35,   33,   50,   49,   48,   54,   55,   53,
 /*   210 */    56,   58,  148,  194,  132,   32,   31,   47,   46,   45,
 /*   220 */    44,   43,   36,   42,   41,   40,   39,   38,   37,   52,
 /*   230 */    51,   35,   33,   50,   49,   48,   54,   55,   53,   56,
 /*   240 */    58,  148,  194,  132,   35,   33,   50,   49,   48,   54,
 /*   250 */    55,   53,   56,   58,  148,  194,  132,  377,  378,  379,
 /*   260 */   380,   25,   50,   49,   48,   54,   55,   53,   56,   58,
 /*   270 */   148,  194,  132,   54,   55,   53,   56,   58,  148,  194,
 /*   280 */   132,  417,   26,  136,   22,  339,  178,  375,   32,   31,
 /*   290 */    47,   46,   45,   44,   43,   36,   42,   41,   40,   39,
 /*   300 */    38,   37,   52,   51,   35,   33,   50,   49,   48,   54,
 /*   310 */    55,   53,   56,   58,  148,  194,  132,  171,  329,  170,
 /*   320 */   144,  328,  143,  142,  167,  181,  373,   32,   31,   47,
 /*   330 */    46,   45,   44,   43,   36,   42,   41,   40,   39,   38,
 /*   340 */    37,   52,   51,   35,   33,   50,   49,   48,   54,   55,
 /*   350 */    53,   56,   58,  148,  194,  132,  430,  161,  101,   57,
 /*   360 */    74,  419,   24,   23,  372,   76,  303,   90,   63,  131,
 /*   370 */    76,  148,  194,  132,  438,  438,  171,  329,  170,  144,
 /*   380 */     5,  143,  142,  175,  438,  422,  422,  158,  176,  429,
 /*   390 */    62,  418,  151,  343,  152,  422,  149,  141,  439,  440,
 /*   400 */   446,  447,  184,  182,  179,   83,   27,   82,   34,   19,
 /*   410 */     4,  443,  441,  442,  444,  445,  448,  300,  301,  160,
 /*   420 */    70,   42,   41,   40,   39,   38,   37,   52,   51,   35,
 /*   430 */    33,   50,   49,   48,   54,   55,   53,   56,   58,  148,
 /*   440 */   194,  132,  347,  183,  188,  430,  105,   89,   57,   91,
 /*   450 */   147,   24,   23,  304,   90,  135,  105,  336,  131,   17,
 /*   460 */   350,   20,  189,  438,  165,  137,  438,  336,   89,    5,
 /*   470 */   357,  438,  305,  438,  422,   57,  334,  422,   24,   23,
 /*   480 */    92,  101,  422,   77,  422,  130,  341,  439,  440,  446,
 /*   490 */   447,  184,  182,  179,  348,   27,    5,  438,  438,  146,
 /*   500 */   443,  441,  442,  444,  445,  448,   96,   66,  422,  422,
 /*   510 */   352,  157,  428,  306,  439,  440,  446,  447,  184,  182,
 /*   520 */   179,   97,   27,  438,  345,   81,  451,  443,  441,  442,
 /*   530 */   444,  445,  448,   98,  422,  105,   98,   91,  438,   79,
 /*   540 */    72,   57,  156,   13,   24,   23,  335,   76,  424,  422,
 /*   550 */   438,  131,  438,  438,  438,  163,  365,  191,  358,  364,
 /*   560 */   409,  422,    5,  422,  422,  422,  326,  185,   57,  168,
 /*   570 */    11,   24,   23,   71,  311,   99,  138,   82,  131,  146,
 /*   580 */   439,  440,  446,  447,  184,  182,  179,  352,   27,    5,
 /*   590 */   352,  338,  438,  443,  441,  442,  444,  445,  448,  185,
 /*   600 */   100,  173,  174,  422,  190,   75,    3,  439,  440,  446,
 /*   610 */   447,  184,  182,  179,  188,   27,  352,  438,  438,  438,
 /*   620 */   443,  441,  442,  444,  445,  448,  102,   30,  422,  422,
 /*   630 */   422,  614,  198,  294,  197,  296,  297,   89,  105,  317,
 /*   640 */   159,   93,   64,  438,   89,  193,  188,  150,   17,  336,
 /*   650 */    94,   73,  298,  432,  422,  438,  101,   95,  438,  438,
 /*   660 */   162,   84,   80,  103,   89,  333,  422,  438,   78,  422,
 /*   670 */   422,  192,   85,  438,  438,  106,  101,  104,  422,   13,
 /*   680 */   438,  139,   21,  121,  422,  422,   12,  429,   18,  195,
 /*   690 */    87,  422,  438,  438,  438,  122,  408,  107,  113,  114,
 /*   700 */   438,  140,  115,  422,  422,  422,  438,  429,   86,  164,
 /*   710 */   324,  422,  438,  169,  438,  438,  438,  422,  116,  438,
 /*   720 */   117,   88,  118,  422,  108,  422,  422,  422,  109,   60,
 /*   730 */   422,   14,   17,   65,  110,  438,  111,  438,  112,  438,
 /*   740 */   123,  438,  124,  125,   61,  438,  422,    6,  422,  332,
 /*   750 */   422,  438,  422,  438,  155,  438,  422,  438,    7,  438,
 /*   760 */   438,  119,  422,  120,  422,  126,  422,  127,  422,  128,
 /*   770 */   422,  422,  154,  133,   67,  134,    1,  146,  438,  129,
 /*   780 */   438,  412,  438,  132,  438,  411,  438,   15,  352,  422,
 /*   790 */   438,  422,  438,  422,  410,  422,  438,  422,    8,  310,
 /*   800 */   307,  422,  327,  422,  146,  325,  323,  422,  322,  321,
 /*   810 */   318,   13,  177,  180,   28,  352,  186,  451,  423,  416,
 /*   820 */   415,  413,   29,  449,  187,   68,  349,  433,   16,  196,
 /*   830 */   453,    2,   10,  455,  455,  369,  455,  455,  455,    9,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*    10 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*    20 */    21,   22,   23,   24,   25,   26,   27,   28,   29,    1,
 /*    30 */     2,    3,    4,    5,    6,    7,    8,    9,   10,   11,
 /*    40 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /*    50 */    22,   23,   24,   25,   26,   27,   28,   29,   25,   26,
 /*    60 */    27,   28,   29,  102,   36,   36,   38,  106,  107,  108,
 /*    70 */   109,  107,  108,  109,   75,   36,   77,   38,    1,    2,
 /*    80 */     3,    4,    5,    6,    7,    8,    9,   10,   11,   12,
 /*    90 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   100 */    23,   24,   25,   26,   27,   28,   29,    3,    4,    5,
 /*   110 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   120 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*   130 */    26,   27,   28,   29,   36,   58,   59,    1,    2,    3,
 /*   140 */     4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   150 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   160 */    24,   25,   26,   27,   28,   29,   15,   16,   17,   18,
 /*   170 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /*   180 */    29,  119,  120,  121,   48,   32,    1,    2,    3,    4,
 /*   190 */     5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
 /*   200 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*   210 */    25,   26,   27,   28,   29,    1,    2,    3,    4,    5,
 /*   220 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   230 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*   240 */    26,   27,   28,   29,   17,   18,   19,   20,   21,   22,
 /*   250 */    23,   24,   25,   26,   27,   28,   29,   65,   66,   67,
 /*   260 */    68,   76,   19,   20,   21,   22,   23,   24,   25,   26,
 /*   270 */    27,   28,   29,   22,   23,   24,   25,   26,   27,   28,
 /*   280 */    29,   74,   75,  114,   77,  116,   72,  128,    1,    2,
 /*   290 */     3,    4,    5,    6,    7,    8,    9,   10,   11,   12,
 /*   300 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   310 */    23,   24,   25,   26,   27,   28,   29,   39,   40,   41,
 /*   320 */    42,   40,   44,   45,   43,   38,  128,    1,    2,    3,
 /*   330 */     4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   340 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   350 */    24,   25,   26,   27,   28,   29,   23,  104,  104,   26,
 /*   360 */   130,  131,   29,   30,  128,   32,  103,  104,   31,   36,
 /*   370 */    32,   27,   28,   29,  121,  121,   39,   40,   41,   42,
 /*   380 */    47,   44,   45,  129,  121,  132,  132,   49,  134,  135,
 /*   390 */    52,  131,   54,  120,  121,  132,  128,   37,   65,   66,
 /*   400 */    67,   68,   69,   70,   71,  111,   73,   47,   17,   18,
 /*   410 */    99,   78,   79,   80,   81,   82,   83,   34,   35,   28,
 /*   420 */    63,    9,   10,   11,   12,   13,   14,   15,   16,   17,
 /*   430 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*   440 */    28,   29,  118,   88,   84,   23,  104,   92,   26,  104,
 /*   450 */   100,   29,   30,  103,  104,  113,  104,  115,   36,   31,
 /*   460 */    36,   55,   38,  121,   88,  113,  121,  115,   92,   47,
 /*   470 */   125,  121,   23,  121,  132,   26,   48,  132,   29,   30,
 /*   480 */   104,  104,  132,   93,  132,   36,  118,   65,   66,   67,
 /*   490 */    68,   69,   70,   71,  105,   73,   47,  121,  121,  110,
 /*   500 */    78,   79,   80,   81,   82,   83,  104,   47,  132,  132,
 /*   510 */   121,   51,  135,   23,   65,   66,   67,   68,   69,   70,
 /*   520 */    71,  104,   73,  121,  122,   31,   36,   78,   79,   80,
 /*   530 */    81,   82,   83,  104,  132,  104,  104,  104,  121,   93,
 /*   540 */    31,   26,   48,   31,   29,   30,  115,   32,   34,  132,
 /*   550 */   121,   36,  121,  121,  121,  126,  127,  124,  125,  127,
 /*   560 */    48,  132,   47,  132,  132,  132,   40,   37,   26,   43,
 /*   570 */   101,   29,   30,   64,  105,  104,  110,   47,   36,  110,
 /*   580 */    65,   66,   67,   68,   69,   70,   71,  121,   73,   47,
 /*   590 */   121,  116,  121,   78,   79,   80,   81,   82,   83,   37,
 /*   600 */   104,  104,  104,  132,   29,  110,  133,   65,   66,   67,
 /*   610 */    68,   69,   70,   71,   84,   73,  121,  121,  121,  121,
 /*   620 */    78,   79,   80,   81,   82,   83,  104,   46,  132,  132,
 /*   630 */   132,   86,   87,   88,   89,   90,   91,   92,  104,  112,
 /*   640 */    88,  104,  104,  121,   92,   62,   84,  113,   31,  115,
 /*   650 */   104,   63,   98,   78,  132,  121,  104,  104,  121,  121,
 /*   660 */    88,   97,  117,  104,   92,   48,  132,  121,  123,  132,
 /*   670 */   132,   56,   96,  121,  121,  104,  104,  104,  132,   31,
 /*   680 */   121,  129,   61,  104,  132,  132,   31,  135,   33,  104,
 /*   690 */    94,  132,  121,  121,  121,  104,   48,  104,  104,  104,
 /*   700 */   121,  129,  104,  132,  132,  132,  121,  135,   95,   60,
 /*   710 */    40,  132,  121,   43,  121,  121,  121,  132,  104,  121,
 /*   720 */   104,   93,  104,  132,  104,  132,  132,  132,  104,   33,
 /*   730 */   132,    3,   31,   31,  104,  121,  104,  121,  104,  121,
 /*   740 */   104,  121,  104,  104,   53,  121,  132,   47,  132,   48,
 /*   750 */   132,  121,  132,  121,   51,  121,  132,  121,   47,  121,
 /*   760 */   121,  104,  132,  104,  132,  104,  132,  104,  132,  104,
 /*   770 */   132,  132,  105,  104,   50,  104,   47,  110,  121,  104,
 /*   780 */   121,   48,  121,   29,  121,   48,  121,   31,  121,  132,
 /*   790 */   121,  132,  121,  132,   48,  132,  121,  132,   57,  105,
 /*   800 */    36,  132,   40,  132,  110,   40,   40,  132,   40,   40,
 /*   810 */    36,   31,   36,   36,   47,  121,   37,   36,   48,   48,
 /*   820 */    48,   48,   47,   36,   36,   47,   36,   78,   31,   31,
 /*   830 */     0,   47,   47,  136,  136,   52,  136,  136,  136,   57,
};
#define YY_SHIFT_USE_DFLT (-2)
#define YY_SHIFT_COUNT (198)
#define YY_SHIFT_MIN   (-1)
#define YY_SHIFT_MAX   (830)
static const short yy_shift_ofst[] = {
 /*     0 */   338,  333,  333,  422,  449,  515,  542,  542,  542,  542,
 /*    10 */   542,  337,  449,  422,  542,  542,  542,  542,   29,  542,
 /*    20 */   542,  542,  542,  542,  542,  542,  542,  542,  542,  542,
 /*    30 */   542,  542,  542,  542,  542,  542,  542,  542,  542,  542,
 /*    40 */   542,  542,  542,  542,  542,  542,  542,  542,  542,  542,
 /*    50 */   542,  542,  542,  542,  542,  542,  542,  542,  542,  278,
 /*    60 */    29,   29,   29,   29,   -1,   29,   98,   29,  153,   29,
 /*    70 */   192,  192,  192,  192,  207,   39,  383,  357,  406,  357,
 /*    80 */   406,   98,  514,  581,  583,  588,  615,  621,  649,  406,
 /*    90 */    28,   77,  136,  185,  214,  287,  326,  326,  326,  326,
 /*   100 */   326,  326,  326,  326,  326,  326,  104,  412,  412,  412,
 /*   110 */   412,  412,  412,  151,  151,  151,  151,  151,  151,  227,
 /*   120 */   227,  243,  243,  251,  251,  251,   33,   33,   33,  344,
 /*   130 */   360,  530,  391,  344,  344,  428,  494,  617,  460,  512,
 /*   140 */   648,  490,  281,  526,  670,  562,  424,  655,  575,  509,
 /*   150 */   701,  696,  728,  702,  691,  700,  703,  711,  724,  733,
 /*   160 */   729,  754,  737,  756,  741,  746,  764,  762,  765,  766,
 /*   170 */   768,  769,  774,  754,  754,  780,  770,  771,  776,  767,
 /*   180 */   772,  777,  775,  773,  778,  781,  787,  779,  788,  790,
 /*   190 */   749,  797,  782,  783,  784,  754,  785,  798,  830,
};
#define YY_REDUCE_USE_DFLT (-40)
#define YY_REDUCE_COUNT (89)
#define YY_REDUCE_MIN   (-39)
#define YY_REDUCE_MAX   (694)
static const short yy_reduce_ofst[] = {
 /*     0 */   545,  552,  572,  254,  350,  376,  342,  352,  429,  433,
 /*    10 */   534,  -39,  263,  377,  402,  432,  345,  431,  469,  253,
 /*    20 */   417,  471,  496,  497,  498,  522,  537,  538,  546,  553,
 /*    30 */   559,  571,  573,  579,  585,  591,  593,  594,  595,  598,
 /*    40 */   614,  616,  618,  620,  624,  630,  632,  634,  636,  638,
 /*    50 */   639,  657,  659,  661,  663,  665,  669,  671,  675,  -36,
 /*    60 */   389,   62,  667,  694,  230,  273,  169,  466,  355,  495,
 /*    70 */   159,  198,  236,  268,  260,  294,  311,  324,  390,  368,
 /*    80 */   446,  475,  473,  527,  554,  564,  576,  613,  596,  628,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   613,  613,  613,  587,  613,  613,  613,  613,  613,  613,
 /*    10 */   613,  473,  613,  613,  613,  613,  613,  613,  613,  613,
 /*    20 */   613,  613,  613,  613,  613,  613,  613,  613,  613,  613,
 /*    30 */   613,  613,  613,  613,  613,  613,  613,  613,  613,  613,
 /*    40 */   613,  613,  613,  613,  613,  613,  613,  613,  613,  613,
 /*    50 */   613,  613,  613,  613,  613,  613,  613,  613,  613,  472,
 /*    60 */   613,  613,  613,  613,  613,  613,  613,  613,  613,  613,
 /*    70 */   613,  613,  613,  613,  613,  480,  462,  536,  514,  536,
 /*    80 */   514,  613,  585,  491,  530,  534,  516,  528,  523,  514,
 /*    90 */   469,  519,  613,  613,  613,  613,  506,  513,  526,  527,
 /*   100 */   581,  591,  580,  490,  574,  497,  594,  563,  556,  555,
 /*   110 */   554,  553,  552,  562,  561,  560,  559,  558,  557,  548,
 /*   120 */   547,  566,  564,  551,  550,  549,  546,  545,  544,  542,
 /*   130 */   612,  612,  613,  543,  541,  613,  613,  613,  613,  613,
 /*   140 */   613,  613,  613,  613,  613,  612,  511,  613,  613,  531,
 /*   150 */   613,  613,  613,  502,  613,  613,  613,  613,  613,  613,
 /*   160 */   613,  567,  613,  522,  613,  613,  613,  613,  613,  613,
 /*   170 */   613,  613,  613,  596,  595,  586,  613,  613,  613,  613,
 /*   180 */   613,  613,  613,  613,  613,  613,  613,  610,  613,  613,
 /*   190 */   613,  515,  613,  613,  613,  565,  613,  455,  613,
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
  "join_conditions",  "values_list",   "field_list",    "value_item",  
  "field_item",    "update_clause",  "limit_clause",  "update_item_list",
  "update_item",   "qualified_name",  "new_value",     "delete_clause",
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
    case 28: /* IN */
    case 29: /* NOT */
    case 30: /* BITWISE_NOT */
    case 31: /* COMMA */
    case 32: /* SELECT */
    case 33: /* FROM */
    case 34: /* DISTINCT */
    case 35: /* ALL */
    case 36: /* IDENTIFIER */
    case 37: /* DOT */
    case 38: /* AS */
    case 39: /* INNER */
    case 40: /* JOIN */
    case 41: /* CROSS */
    case 42: /* LEFT */
    case 43: /* OUTER */
    case 44: /* RIGHT */
    case 45: /* FULL */
    case 46: /* ON */
    case 47: /* PARENTHESES_OPEN */
    case 48: /* PARENTHESES_CLOSE */
    case 49: /* INSERT */
    case 50: /* INTO */
    case 51: /* VALUES */
    case 52: /* UPDATE */
    case 53: /* SET */
    case 54: /* DELETE */
    case 55: /* WHERE */
    case 56: /* ORDER */
    case 57: /* BY */
    case 58: /* ASC */
    case 59: /* DESC */
    case 60: /* GROUP */
    case 61: /* HAVING */
    case 62: /* FOR */
    case 63: /* LIMIT */
    case 64: /* OFFSET */
    case 65: /* INTEGER */
    case 66: /* HINTEGER */
    case 67: /* NPLACEHOLDER */
    case 68: /* SPLACEHOLDER */
    case 69: /* EXISTS */
    case 70: /* CAST */
    case 71: /* CONVERT */
    case 72: /* USING */
    case 73: /* CASE */
    case 74: /* END */
    case 75: /* WHEN */
    case 76: /* THEN */
    case 77: /* ELSE */
    case 78: /* NULL */
    case 79: /* STRING */
    case 80: /* DOUBLE */
    case 81: /* TRUE */
    case 82: /* FALSE */
    case 83: /* BPLACEHOLDER */
    case 84: /* COLON */
{
/* #line 22 "parser.y" */

	if ((yypminor->yy0)) {
		if ((yypminor->yy0)->free_flag) {
			efree((yypminor->yy0)->token);
		}
		efree((yypminor->yy0));
	}

/* #line 1254 "parser.c" */
}
      break;
      /* Default NON-TERMINAL Destructor */
    case 85: /* error */
    case 86: /* program */
    case 87: /* query_language */
    case 88: /* select_statement */
    case 89: /* insert_statement */
    case 90: /* update_statement */
    case 91: /* delete_statement */
    case 92: /* select_clause */
    case 93: /* where_clause */
    case 94: /* group_clause */
    case 95: /* having_clause */
    case 96: /* order_clause */
    case 97: /* select_limit_clause */
    case 98: /* for_update_clause */
    case 99: /* distinct_all */
    case 100: /* column_list */
    case 101: /* associated_name_list */
    case 102: /* join_list_or_null */
    case 103: /* column_item */
    case 104: /* expr */
    case 105: /* associated_name */
    case 106: /* join_list */
    case 107: /* join_item */
    case 108: /* join_clause */
    case 109: /* join_type */
    case 110: /* aliased_or_qualified_name */
    case 111: /* join_associated_name */
    case 112: /* join_conditions */
    case 113: /* values_list */
    case 114: /* field_list */
    case 115: /* value_item */
    case 116: /* field_item */
    case 117: /* update_clause */
    case 118: /* limit_clause */
    case 119: /* update_item_list */
    case 120: /* update_item */
    case 121: /* qualified_name */
    case 122: /* new_value */
    case 123: /* delete_clause */
    case 124: /* order_list */
    case 125: /* order_item */
    case 126: /* group_list */
    case 127: /* group_item */
    case 128: /* integer_or_placeholder */
    case 129: /* argument_list */
    case 130: /* when_clauses */
    case 131: /* when_clause */
    case 132: /* function_call */
    case 133: /* distinct_or_null */
    case 134: /* argument_list_or_null */
    case 135: /* argument_item */
{
/* #line 31 "parser.y" */

	if (status) {
		// TODO:
	}
	if (&(yypminor->yy170)) {
		zval_ptr_dtor(&(yypminor->yy170));
	}

/* #line 1319 "parser.c" */
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
  { 115, 1 },
  { 114, 3 },
  { 114, 1 },
  { 116, 1 },
  { 90, 3 },
  { 117, 4 },
  { 119, 3 },
  { 119, 1 },
  { 120, 3 },
  { 122, 1 },
  { 91, 3 },
  { 123, 3 },
  { 105, 3 },
  { 105, 2 },
  { 105, 1 },
  { 110, 1 },
  { 93, 2 },
  { 93, 0 },
  { 96, 3 },
  { 96, 0 },
  { 124, 3 },
  { 124, 1 },
  { 125, 1 },
  { 125, 2 },
  { 125, 2 },
  { 94, 3 },
  { 94, 0 },
  { 126, 3 },
  { 126, 1 },
  { 127, 1 },
  { 95, 2 },
  { 95, 0 },
  { 98, 2 },
  { 98, 0 },
  { 97, 2 },
  { 97, 4 },
  { 97, 4 },
  { 97, 0 },
  { 118, 2 },
  { 118, 0 },
  { 128, 1 },
  { 128, 1 },
  { 128, 1 },
  { 128, 1 },
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
  { 130, 2 },
  { 130, 1 },
  { 131, 4 },
  { 131, 2 },
  { 104, 1 },
  { 132, 5 },
  { 133, 1 },
  { 133, 0 },
  { 134, 1 },
  { 134, 0 },
  { 129, 3 },
  { 129, 1 },
  { 135, 1 },
  { 135, 1 },
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
  { 121, 5 },
  { 121, 3 },
  { 121, 3 },
  { 121, 1 },
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
/* #line 474 "parser.y" */
{
	ZVAL_ZVAL(&status->ret, &yymsp[0].minor.yy170, 1, 1);
}
/* #line 1787 "parser.c" */
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
      case 65: /* order_list ::= order_item */ yytestcase(yyruleno==65);
      case 72: /* group_list ::= group_item */ yytestcase(yyruleno==72);
      case 73: /* group_item ::= expr */ yytestcase(yyruleno==73);
      case 129: /* expr ::= function_call */ yytestcase(yyruleno==129);
      case 133: /* argument_list_or_null ::= argument_list */ yytestcase(yyruleno==133);
      case 138: /* argument_item ::= expr */ yytestcase(yyruleno==138);
      case 145: /* expr ::= qualified_name */ yytestcase(yyruleno==145);
/* #line 478 "parser.y" */
{
	yylhsminor.yy170 = yymsp[0].minor.yy170;
}
/* #line 1812 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 5: /* select_statement ::= select_clause where_clause group_clause having_clause order_clause select_limit_clause for_update_clause */
/* #line 494 "parser.y" */
{
	phql_ret_select_statement(&yylhsminor.yy170, &yymsp[-6].minor.yy170, &yymsp[-5].minor.yy170, &yymsp[-2].minor.yy170, &yymsp[-4].minor.yy170, &yymsp[-3].minor.yy170, &yymsp[-1].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 1820 "parser.c" */
  yymsp[-6].minor.yy170 = yylhsminor.yy170;
        break;
      case 6: /* select_clause ::= SELECT distinct_all column_list FROM associated_name_list join_list_or_null */
{  yy_destructor(yypParser,32,&yymsp[-5].minor);
/* #line 498 "parser.y" */
{
	phql_ret_select_clause(&yymsp[-5].minor.yy170, &yymsp[-4].minor.yy170, &yymsp[-3].minor.yy170, &yymsp[-1].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 1829 "parser.c" */
  yy_destructor(yypParser,33,&yymsp[-2].minor);
}
        break;
      case 7: /* distinct_all ::= DISTINCT */
{  yy_destructor(yypParser,34,&yymsp[0].minor);
/* #line 502 "parser.y" */
{
	phql_ret_distinct_all(&yymsp[0].minor.yy170, 1);
}
/* #line 1839 "parser.c" */
}
        break;
      case 8: /* distinct_all ::= ALL */
{  yy_destructor(yypParser,35,&yymsp[0].minor);
/* #line 506 "parser.y" */
{
	phql_ret_distinct_all(&yymsp[0].minor.yy170, 0);
}
/* #line 1848 "parser.c" */
}
        break;
      case 9: /* distinct_all ::= */
      case 20: /* join_list_or_null ::= */ yytestcase(yyruleno==20);
      case 27: /* join_associated_name ::= */ yytestcase(yyruleno==27);
      case 38: /* join_conditions ::= */ yytestcase(yyruleno==38);
      case 61: /* where_clause ::= */ yytestcase(yyruleno==61);
      case 63: /* order_clause ::= */ yytestcase(yyruleno==63);
      case 70: /* group_clause ::= */ yytestcase(yyruleno==70);
      case 75: /* having_clause ::= */ yytestcase(yyruleno==75);
      case 77: /* for_update_clause ::= */ yytestcase(yyruleno==77);
      case 81: /* select_limit_clause ::= */ yytestcase(yyruleno==81);
      case 83: /* limit_clause ::= */ yytestcase(yyruleno==83);
      case 132: /* distinct_or_null ::= */ yytestcase(yyruleno==132);
      case 134: /* argument_list_or_null ::= */ yytestcase(yyruleno==134);
/* #line 510 "parser.y" */
{
	ZVAL_UNDEF(&yymsp[1].minor.yy170);
}
/* #line 1868 "parser.c" */
        break;
      case 10: /* column_list ::= column_list COMMA column_item */
      case 17: /* associated_name_list ::= associated_name_list COMMA associated_name */ yytestcase(yyruleno==17);
      case 42: /* values_list ::= values_list COMMA value_item */ yytestcase(yyruleno==42);
      case 45: /* field_list ::= field_list COMMA field_item */ yytestcase(yyruleno==45);
      case 50: /* update_item_list ::= update_item_list COMMA update_item */ yytestcase(yyruleno==50);
      case 64: /* order_list ::= order_list COMMA order_item */ yytestcase(yyruleno==64);
      case 71: /* group_list ::= group_list COMMA group_item */ yytestcase(yyruleno==71);
      case 135: /* argument_list ::= argument_list COMMA argument_item */ yytestcase(yyruleno==135);
/* #line 514 "parser.y" */
{
	phql_ret_zval_list(&yylhsminor.yy170, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 1882 "parser.c" */
  yy_destructor(yypParser,31,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 11: /* column_list ::= column_item */
      case 43: /* values_list ::= value_item */ yytestcase(yyruleno==43);
      case 46: /* field_list ::= field_item */ yytestcase(yyruleno==46);
      case 126: /* when_clauses ::= when_clause */ yytestcase(yyruleno==126);
      case 136: /* argument_list ::= argument_item */ yytestcase(yyruleno==136);
/* #line 518 "parser.y" */
{
	phql_ret_zval_list(&yylhsminor.yy170, &yymsp[0].minor.yy170, NULL);
}
/* #line 1895 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 12: /* column_item ::= TIMES */
      case 137: /* argument_item ::= TIMES */ yytestcase(yyruleno==137);
{  yy_destructor(yypParser,23,&yymsp[0].minor);
/* #line 522 "parser.y" */
{
	phql_ret_column_item(&yymsp[0].minor.yy170, PHQL_T_STARALL, NULL, NULL, NULL);
}
/* #line 1905 "parser.c" */
}
        break;
      case 13: /* column_item ::= IDENTIFIER DOT TIMES */
/* #line 526 "parser.y" */
{
	phql_ret_column_item(&yylhsminor.yy170, PHQL_T_DOMAINALL, NULL, yymsp[-2].minor.yy0, NULL);
}
/* #line 1913 "parser.c" */
  yy_destructor(yypParser,37,&yymsp[-1].minor);
  yy_destructor(yypParser,23,&yymsp[0].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 14: /* column_item ::= expr AS IDENTIFIER */
/* #line 530 "parser.y" */
{
	phql_ret_column_item(&yylhsminor.yy170, PHQL_T_EXPR, &yymsp[-2].minor.yy170, NULL, yymsp[0].minor.yy0);
}
/* #line 1923 "parser.c" */
  yy_destructor(yypParser,38,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 15: /* column_item ::= expr IDENTIFIER */
/* #line 534 "parser.y" */
{
	phql_ret_column_item(&yylhsminor.yy170, PHQL_T_EXPR, &yymsp[-1].minor.yy170, NULL, yymsp[0].minor.yy0);
}
/* #line 1932 "parser.c" */
  yymsp[-1].minor.yy170 = yylhsminor.yy170;
        break;
      case 16: /* column_item ::= expr */
/* #line 538 "parser.y" */
{
	phql_ret_column_item(&yylhsminor.yy170, PHQL_T_EXPR, &yymsp[0].minor.yy170, NULL, NULL);
}
/* #line 1940 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 21: /* join_list ::= join_list join_item */
      case 125: /* when_clauses ::= when_clauses when_clause */ yytestcase(yyruleno==125);
/* #line 558 "parser.y" */
{
	phql_ret_zval_list(&yylhsminor.yy170, &yymsp[-1].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 1949 "parser.c" */
  yymsp[-1].minor.yy170 = yylhsminor.yy170;
        break;
      case 24: /* join_clause ::= join_type aliased_or_qualified_name join_associated_name join_conditions */
/* #line 571 "parser.y" */
{
	phql_ret_join_item(&yylhsminor.yy170, &yymsp[-3].minor.yy170, &yymsp[-2].minor.yy170, &yymsp[-1].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 1957 "parser.c" */
  yymsp[-3].minor.yy170 = yylhsminor.yy170;
        break;
      case 25: /* join_associated_name ::= AS IDENTIFIER */
{  yy_destructor(yypParser,38,&yymsp[-1].minor);
/* #line 575 "parser.y" */
{
	phql_ret_qualified_name(&yymsp[-1].minor.yy170, NULL, NULL, yymsp[0].minor.yy0);
}
/* #line 1966 "parser.c" */
}
        break;
      case 26: /* join_associated_name ::= IDENTIFIER */
      case 47: /* field_item ::= IDENTIFIER */ yytestcase(yyruleno==47);
      case 159: /* qualified_name ::= IDENTIFIER */ yytestcase(yyruleno==159);
/* #line 579 "parser.y" */
{
	phql_ret_qualified_name(&yylhsminor.yy170, NULL, NULL, yymsp[0].minor.yy0);
}
/* #line 1976 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 28: /* join_type ::= INNER JOIN */
{  yy_destructor(yypParser,39,&yymsp[-1].minor);
/* #line 587 "parser.y" */
{
	phql_ret_join_type(&yymsp[-1].minor.yy170, PHQL_T_INNERJOIN);
}
/* #line 1985 "parser.c" */
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
        break;
      case 29: /* join_type ::= CROSS JOIN */
{  yy_destructor(yypParser,41,&yymsp[-1].minor);
/* #line 591 "parser.y" */
{
	phql_ret_join_type(&yymsp[-1].minor.yy170, PHQL_T_CROSSJOIN);
}
/* #line 1995 "parser.c" */
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
        break;
      case 30: /* join_type ::= LEFT OUTER JOIN */
{  yy_destructor(yypParser,42,&yymsp[-2].minor);
/* #line 595 "parser.y" */
{
	phql_ret_join_type(&yymsp[-2].minor.yy170, PHQL_T_LEFTJOIN);
}
/* #line 2005 "parser.c" */
  yy_destructor(yypParser,43,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
        break;
      case 31: /* join_type ::= LEFT JOIN */
{  yy_destructor(yypParser,42,&yymsp[-1].minor);
/* #line 599 "parser.y" */
{
	phql_ret_join_type(&yymsp[-1].minor.yy170, PHQL_T_LEFTJOIN);
}
/* #line 2016 "parser.c" */
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
        break;
      case 32: /* join_type ::= RIGHT OUTER JOIN */
{  yy_destructor(yypParser,44,&yymsp[-2].minor);
/* #line 603 "parser.y" */
{
	phql_ret_join_type(&yymsp[-2].minor.yy170, PHQL_T_RIGHTJOIN);
}
/* #line 2026 "parser.c" */
  yy_destructor(yypParser,43,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
        break;
      case 33: /* join_type ::= RIGHT JOIN */
{  yy_destructor(yypParser,44,&yymsp[-1].minor);
/* #line 607 "parser.y" */
{
	phql_ret_join_type(&yymsp[-1].minor.yy170, PHQL_T_RIGHTJOIN);
}
/* #line 2037 "parser.c" */
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
        break;
      case 34: /* join_type ::= FULL OUTER JOIN */
{  yy_destructor(yypParser,45,&yymsp[-2].minor);
/* #line 611 "parser.y" */
{
	phql_ret_join_type(&yymsp[-2].minor.yy170, PHQL_T_FULLJOIN);
}
/* #line 2047 "parser.c" */
  yy_destructor(yypParser,43,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
        break;
      case 35: /* join_type ::= FULL JOIN */
{  yy_destructor(yypParser,45,&yymsp[-1].minor);
/* #line 615 "parser.y" */
{
	phql_ret_join_type(&yymsp[-1].minor.yy170, PHQL_T_FULLJOIN);
}
/* #line 2058 "parser.c" */
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
        break;
      case 36: /* join_type ::= JOIN */
{  yy_destructor(yypParser,40,&yymsp[0].minor);
/* #line 619 "parser.y" */
{
	phql_ret_join_type(&yymsp[0].minor.yy170, PHQL_T_INNERJOIN);
}
/* #line 2068 "parser.c" */
}
        break;
      case 37: /* join_conditions ::= ON expr */
      case 60: /* where_clause ::= WHERE expr */ yytestcase(yyruleno==60);
      case 74: /* having_clause ::= HAVING expr */ yytestcase(yyruleno==74);
{  yy_destructor(yypParser,46,&yymsp[-1].minor);
/* #line 623 "parser.y" */
{
	yymsp[-1].minor.yy170 = yymsp[0].minor.yy170;
}
/* #line 2079 "parser.c" */
}
        break;
      case 39: /* insert_statement ::= insert_statement COMMA PARENTHESES_OPEN values_list PARENTHESES_CLOSE */
/* #line 632 "parser.y" */
{
	phql_ret_insert_statement2(&yylhsminor.yy170, &yymsp[-4].minor.yy170, NULL, &yymsp[-1].minor.yy170);
}
/* #line 2087 "parser.c" */
  yy_destructor(yypParser,31,&yymsp[-3].minor);
  yy_destructor(yypParser,47,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[0].minor);
  yymsp[-4].minor.yy170 = yylhsminor.yy170;
        break;
      case 40: /* insert_statement ::= INSERT INTO aliased_or_qualified_name VALUES PARENTHESES_OPEN values_list PARENTHESES_CLOSE */
{  yy_destructor(yypParser,49,&yymsp[-6].minor);
/* #line 636 "parser.y" */
{
	phql_ret_insert_statement(&yymsp[-6].minor.yy170, &yymsp[-4].minor.yy170, NULL, &yymsp[-1].minor.yy170);
}
/* #line 2099 "parser.c" */
  yy_destructor(yypParser,50,&yymsp[-5].minor);
  yy_destructor(yypParser,51,&yymsp[-3].minor);
  yy_destructor(yypParser,47,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[0].minor);
}
        break;
      case 41: /* insert_statement ::= INSERT INTO aliased_or_qualified_name PARENTHESES_OPEN field_list PARENTHESES_CLOSE VALUES PARENTHESES_OPEN values_list PARENTHESES_CLOSE */
{  yy_destructor(yypParser,49,&yymsp[-9].minor);
/* #line 640 "parser.y" */
{
	phql_ret_insert_statement(&yymsp[-9].minor.yy170, &yymsp[-7].minor.yy170, &yymsp[-5].minor.yy170, &yymsp[-1].minor.yy170);
}
/* #line 2112 "parser.c" */
  yy_destructor(yypParser,50,&yymsp[-8].minor);
  yy_destructor(yypParser,47,&yymsp[-6].minor);
  yy_destructor(yypParser,48,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-3].minor);
  yy_destructor(yypParser,47,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[0].minor);
}
        break;
      case 48: /* update_statement ::= update_clause where_clause limit_clause */
/* #line 670 "parser.y" */
{
	phql_ret_update_statement(&yylhsminor.yy170, &yymsp[-2].minor.yy170, &yymsp[-1].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2126 "parser.c" */
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 49: /* update_clause ::= UPDATE associated_name SET update_item_list */
{  yy_destructor(yypParser,52,&yymsp[-3].minor);
/* #line 674 "parser.y" */
{
	phql_ret_update_clause(&yymsp[-3].minor.yy170, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2135 "parser.c" */
  yy_destructor(yypParser,53,&yymsp[-1].minor);
}
        break;
      case 52: /* update_item ::= qualified_name EQUALS new_value */
/* #line 686 "parser.y" */
{
	phql_ret_update_item(&yylhsminor.yy170, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2144 "parser.c" */
  yy_destructor(yypParser,3,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 54: /* delete_statement ::= delete_clause where_clause limit_clause */
/* #line 696 "parser.y" */
{
	phql_ret_delete_statement(&yylhsminor.yy170, &yymsp[-2].minor.yy170, &yymsp[-1].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2153 "parser.c" */
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 55: /* delete_clause ::= DELETE FROM associated_name */
{  yy_destructor(yypParser,54,&yymsp[-2].minor);
/* #line 700 "parser.y" */
{
	phql_ret_delete_clause(&yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2162 "parser.c" */
  yy_destructor(yypParser,33,&yymsp[-1].minor);
}
        break;
      case 56: /* associated_name ::= aliased_or_qualified_name AS IDENTIFIER */
/* #line 704 "parser.y" */
{
	phql_ret_assoc_name(&yylhsminor.yy170, &yymsp[-2].minor.yy170, yymsp[0].minor.yy0);
}
/* #line 2171 "parser.c" */
  yy_destructor(yypParser,38,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 57: /* associated_name ::= aliased_or_qualified_name IDENTIFIER */
/* #line 708 "parser.y" */
{
	phql_ret_assoc_name(&yylhsminor.yy170, &yymsp[-1].minor.yy170, yymsp[0].minor.yy0);
}
/* #line 2180 "parser.c" */
  yymsp[-1].minor.yy170 = yylhsminor.yy170;
        break;
      case 58: /* associated_name ::= aliased_or_qualified_name */
/* #line 712 "parser.y" */
{
	phql_ret_assoc_name(&yylhsminor.yy170, &yymsp[0].minor.yy170, NULL);
}
/* #line 2188 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 62: /* order_clause ::= ORDER BY order_list */
      case 69: /* group_clause ::= GROUP BY group_list */ yytestcase(yyruleno==69);
{  yy_destructor(yypParser,56,&yymsp[-2].minor);
/* #line 728 "parser.y" */
{
	yymsp[-2].minor.yy170 = yymsp[0].minor.yy170;
}
/* #line 2198 "parser.c" */
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
        break;
      case 66: /* order_item ::= expr */
/* #line 744 "parser.y" */
{
	phql_ret_order_item(&yylhsminor.yy170, &yymsp[0].minor.yy170, 0);
}
/* #line 2207 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 67: /* order_item ::= expr ASC */
/* #line 748 "parser.y" */
{
	phql_ret_order_item(&yylhsminor.yy170, &yymsp[-1].minor.yy170, PHQL_T_ASC);
}
/* #line 2215 "parser.c" */
  yy_destructor(yypParser,58,&yymsp[0].minor);
  yymsp[-1].minor.yy170 = yylhsminor.yy170;
        break;
      case 68: /* order_item ::= expr DESC */
/* #line 752 "parser.y" */
{
	phql_ret_order_item(&yylhsminor.yy170, &yymsp[-1].minor.yy170, PHQL_T_DESC);
}
/* #line 2224 "parser.c" */
  yy_destructor(yypParser,59,&yymsp[0].minor);
  yymsp[-1].minor.yy170 = yylhsminor.yy170;
        break;
      case 76: /* for_update_clause ::= FOR UPDATE */
{  yy_destructor(yypParser,62,&yymsp[-1].minor);
/* #line 784 "parser.y" */
{
	phql_ret_for_update_clause(&yymsp[-1].minor.yy170);
}
/* #line 2234 "parser.c" */
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
        break;
      case 78: /* select_limit_clause ::= LIMIT integer_or_placeholder */
      case 82: /* limit_clause ::= LIMIT integer_or_placeholder */ yytestcase(yyruleno==82);
{  yy_destructor(yypParser,63,&yymsp[-1].minor);
/* #line 792 "parser.y" */
{
	phql_ret_limit_clause(&yymsp[-1].minor.yy170, &yymsp[0].minor.yy170, NULL);
}
/* #line 2245 "parser.c" */
}
        break;
      case 79: /* select_limit_clause ::= LIMIT integer_or_placeholder COMMA integer_or_placeholder */
{  yy_destructor(yypParser,63,&yymsp[-3].minor);
/* #line 796 "parser.y" */
{
	phql_ret_limit_clause(&yymsp[-3].minor.yy170, &yymsp[0].minor.yy170, &yymsp[-2].minor.yy170);
}
/* #line 2254 "parser.c" */
  yy_destructor(yypParser,31,&yymsp[-1].minor);
}
        break;
      case 80: /* select_limit_clause ::= LIMIT integer_or_placeholder OFFSET integer_or_placeholder */
{  yy_destructor(yypParser,63,&yymsp[-3].minor);
/* #line 800 "parser.y" */
{
	phql_ret_limit_clause(&yymsp[-3].minor.yy170, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2264 "parser.c" */
  yy_destructor(yypParser,64,&yymsp[-1].minor);
}
        break;
      case 84: /* integer_or_placeholder ::= INTEGER */
      case 146: /* expr ::= INTEGER */ yytestcase(yyruleno==146);
/* #line 816 "parser.y" */
{
	phql_ret_literal_zval(&yylhsminor.yy170, PHQL_T_INTEGER, yymsp[0].minor.yy0);
}
/* #line 2274 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 85: /* integer_or_placeholder ::= HINTEGER */
      case 147: /* expr ::= HINTEGER */ yytestcase(yyruleno==147);
/* #line 820 "parser.y" */
{
	phql_ret_literal_zval(&yylhsminor.yy170, PHQL_T_HINTEGER, yymsp[0].minor.yy0);
}
/* #line 2283 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 86: /* integer_or_placeholder ::= NPLACEHOLDER */
      case 153: /* expr ::= NPLACEHOLDER */ yytestcase(yyruleno==153);
/* #line 824 "parser.y" */
{
	phql_ret_placeholder_zval(&yylhsminor.yy170, PHQL_T_NPLACEHOLDER, yymsp[0].minor.yy0);
}
/* #line 2292 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 87: /* integer_or_placeholder ::= SPLACEHOLDER */
      case 154: /* expr ::= SPLACEHOLDER */ yytestcase(yyruleno==154);
/* #line 828 "parser.y" */
{
	phql_ret_placeholder_zval(&yylhsminor.yy170, PHQL_T_SPLACEHOLDER, yymsp[0].minor.yy0);
}
/* #line 2301 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 88: /* expr ::= MINUS expr */
{  yy_destructor(yypParser,26,&yymsp[-1].minor);
/* #line 832 "parser.y" */
{
	phql_ret_expr(&yymsp[-1].minor.yy170, PHQL_T_MINUS, NULL, &yymsp[0].minor.yy170);
}
/* #line 2310 "parser.c" */
}
        break;
      case 89: /* expr ::= expr MINUS expr */
/* #line 836 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_SUB, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2318 "parser.c" */
  yy_destructor(yypParser,26,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 90: /* expr ::= expr PLUS expr */
/* #line 840 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_ADD, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2327 "parser.c" */
  yy_destructor(yypParser,25,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 91: /* expr ::= expr TIMES expr */
/* #line 844 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_MUL, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2336 "parser.c" */
  yy_destructor(yypParser,23,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 92: /* expr ::= expr DIVIDE expr */
/* #line 848 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_DIV, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2345 "parser.c" */
  yy_destructor(yypParser,22,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 93: /* expr ::= expr MOD expr */
/* #line 852 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_MOD, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2354 "parser.c" */
  yy_destructor(yypParser,24,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 94: /* expr ::= expr AND expr */
/* #line 856 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_AND, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2363 "parser.c" */
  yy_destructor(yypParser,15,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 95: /* expr ::= expr OR expr */
/* #line 860 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_OR, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2372 "parser.c" */
  yy_destructor(yypParser,16,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 96: /* expr ::= expr BITWISE_AND expr */
/* #line 864 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_BITWISE_AND, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2381 "parser.c" */
  yy_destructor(yypParser,19,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 97: /* expr ::= expr BITWISE_OR expr */
/* #line 868 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_BITWISE_OR, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2390 "parser.c" */
  yy_destructor(yypParser,20,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 98: /* expr ::= expr BITWISE_XOR expr */
/* #line 872 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_BITWISE_XOR, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2399 "parser.c" */
  yy_destructor(yypParser,21,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 99: /* expr ::= expr EQUALS expr */
/* #line 876 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_EQUALS, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2408 "parser.c" */
  yy_destructor(yypParser,3,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 100: /* expr ::= expr NOTEQUALS expr */
/* #line 880 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_NOTEQUALS, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2417 "parser.c" */
  yy_destructor(yypParser,4,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 101: /* expr ::= expr LESS expr */
/* #line 884 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_LESS, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2426 "parser.c" */
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 102: /* expr ::= expr GREATER expr */
/* #line 888 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_GREATER, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2435 "parser.c" */
  yy_destructor(yypParser,6,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 103: /* expr ::= expr GREATEREQUAL expr */
/* #line 892 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_GREATEREQUAL, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2444 "parser.c" */
  yy_destructor(yypParser,7,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 104: /* expr ::= expr TS_MATCHES expr */
/* #line 896 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_TS_MATCHES, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2453 "parser.c" */
  yy_destructor(yypParser,9,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 105: /* expr ::= expr TS_OR expr */
/* #line 900 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_TS_OR, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2462 "parser.c" */
  yy_destructor(yypParser,10,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 106: /* expr ::= expr TS_AND expr */
/* #line 904 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_TS_AND, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2471 "parser.c" */
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 107: /* expr ::= expr TS_NEGATE expr */
/* #line 908 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_TS_NEGATE, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2480 "parser.c" */
  yy_destructor(yypParser,12,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 108: /* expr ::= expr TS_CONTAINS_ANOTHER expr */
/* #line 912 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_TS_CONTAINS_ANOTHER, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2489 "parser.c" */
  yy_destructor(yypParser,13,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 109: /* expr ::= expr TS_CONTAINS_IN expr */
/* #line 916 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_TS_CONTAINS_IN, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2498 "parser.c" */
  yy_destructor(yypParser,14,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 110: /* expr ::= expr LESSEQUAL expr */
/* #line 920 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_LESSEQUAL, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2507 "parser.c" */
  yy_destructor(yypParser,8,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 111: /* expr ::= expr LIKE expr */
/* #line 924 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_LIKE, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2516 "parser.c" */
  yy_destructor(yypParser,17,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 112: /* expr ::= expr NOT LIKE expr */
/* #line 928 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_NLIKE, &yymsp[-3].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2525 "parser.c" */
  yy_destructor(yypParser,29,&yymsp[-2].minor);
  yy_destructor(yypParser,17,&yymsp[-1].minor);
  yymsp[-3].minor.yy170 = yylhsminor.yy170;
        break;
      case 113: /* expr ::= expr ILIKE expr */
/* #line 932 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_ILIKE, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2535 "parser.c" */
  yy_destructor(yypParser,18,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 114: /* expr ::= expr NOT ILIKE expr */
/* #line 936 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_NILIKE, &yymsp[-3].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2544 "parser.c" */
  yy_destructor(yypParser,29,&yymsp[-2].minor);
  yy_destructor(yypParser,18,&yymsp[-1].minor);
  yymsp[-3].minor.yy170 = yylhsminor.yy170;
        break;
      case 115: /* expr ::= expr IN PARENTHESES_OPEN argument_list PARENTHESES_CLOSE */
      case 118: /* expr ::= expr IN PARENTHESES_OPEN select_statement PARENTHESES_CLOSE */ yytestcase(yyruleno==118);
/* #line 940 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_IN, &yymsp[-4].minor.yy170, &yymsp[-1].minor.yy170);
}
/* #line 2555 "parser.c" */
  yy_destructor(yypParser,28,&yymsp[-3].minor);
  yy_destructor(yypParser,47,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[0].minor);
  yymsp[-4].minor.yy170 = yylhsminor.yy170;
        break;
      case 116: /* expr ::= expr NOT IN PARENTHESES_OPEN argument_list PARENTHESES_CLOSE */
      case 119: /* expr ::= expr NOT IN PARENTHESES_OPEN select_statement PARENTHESES_CLOSE */ yytestcase(yyruleno==119);
/* #line 944 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_NOTIN, &yymsp[-5].minor.yy170, &yymsp[-1].minor.yy170);
}
/* #line 2567 "parser.c" */
  yy_destructor(yypParser,29,&yymsp[-4].minor);
  yy_destructor(yypParser,28,&yymsp[-3].minor);
  yy_destructor(yypParser,47,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[0].minor);
  yymsp[-5].minor.yy170 = yylhsminor.yy170;
        break;
      case 117: /* expr ::= PARENTHESES_OPEN select_statement PARENTHESES_CLOSE */
{  yy_destructor(yypParser,47,&yymsp[-2].minor);
/* #line 948 "parser.y" */
{
	phql_ret_expr(&yymsp[-2].minor.yy170, PHQL_T_SUBQUERY, &yymsp[-1].minor.yy170, NULL);
}
/* #line 2580 "parser.c" */
  yy_destructor(yypParser,48,&yymsp[0].minor);
}
        break;
      case 120: /* expr ::= EXISTS PARENTHESES_OPEN select_statement PARENTHESES_CLOSE */
{  yy_destructor(yypParser,69,&yymsp[-3].minor);
/* #line 960 "parser.y" */
{
	phql_ret_expr(&yymsp[-3].minor.yy170, PHQL_T_EXISTS, NULL, &yymsp[-1].minor.yy170);
}
/* #line 2590 "parser.c" */
  yy_destructor(yypParser,47,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[0].minor);
}
        break;
      case 121: /* expr ::= expr AGAINST expr */
/* #line 964 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_AGAINST, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2600 "parser.c" */
  yy_destructor(yypParser,1,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 122: /* expr ::= CAST PARENTHESES_OPEN expr AS IDENTIFIER PARENTHESES_CLOSE */
{  yy_destructor(yypParser,70,&yymsp[-5].minor);
/* #line 968 "parser.y" */
{
	{
		zval qualified;
		phql_ret_raw_qualified_name(&qualified, yymsp[-1].minor.yy0, NULL);
		phql_ret_expr(&yymsp[-5].minor.yy170, PHQL_T_CAST, &yymsp[-3].minor.yy170, &qualified);
	}
}
/* #line 2614 "parser.c" */
  yy_destructor(yypParser,47,&yymsp[-4].minor);
  yy_destructor(yypParser,38,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[0].minor);
}
        break;
      case 123: /* expr ::= CONVERT PARENTHESES_OPEN expr USING IDENTIFIER PARENTHESES_CLOSE */
{  yy_destructor(yypParser,71,&yymsp[-5].minor);
/* #line 976 "parser.y" */
{
	{
		zval qualified;
		phql_ret_raw_qualified_name(&qualified, yymsp[-1].minor.yy0, NULL);
		phql_ret_expr(&yymsp[-5].minor.yy170, PHQL_T_CONVERT, &yymsp[-3].minor.yy170, &qualified);
	}
}
/* #line 2630 "parser.c" */
  yy_destructor(yypParser,47,&yymsp[-4].minor);
  yy_destructor(yypParser,72,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[0].minor);
}
        break;
      case 124: /* expr ::= CASE expr when_clauses END */
{  yy_destructor(yypParser,73,&yymsp[-3].minor);
/* #line 984 "parser.y" */
{
	phql_ret_expr(&yymsp[-3].minor.yy170, PHQL_T_CASE, &yymsp[-2].minor.yy170, &yymsp[-1].minor.yy170);
}
/* #line 2642 "parser.c" */
  yy_destructor(yypParser,74,&yymsp[0].minor);
}
        break;
      case 127: /* when_clause ::= WHEN expr THEN expr */
{  yy_destructor(yypParser,75,&yymsp[-3].minor);
/* #line 996 "parser.y" */
{
	phql_ret_expr(&yymsp[-3].minor.yy170, PHQL_T_WHEN, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2652 "parser.c" */
  yy_destructor(yypParser,76,&yymsp[-1].minor);
}
        break;
      case 128: /* when_clause ::= ELSE expr */
{  yy_destructor(yypParser,77,&yymsp[-1].minor);
/* #line 1000 "parser.y" */
{
	phql_ret_expr(&yymsp[-1].minor.yy170, PHQL_T_ELSE, &yymsp[0].minor.yy170, NULL);
}
/* #line 2662 "parser.c" */
}
        break;
      case 130: /* function_call ::= IDENTIFIER PARENTHESES_OPEN distinct_or_null argument_list_or_null PARENTHESES_CLOSE */
/* #line 1008 "parser.y" */
{
	phql_ret_func_call(&yylhsminor.yy170, yymsp[-4].minor.yy0, &yymsp[-1].minor.yy170, &yymsp[-2].minor.yy170);
}
/* #line 2670 "parser.c" */
  yy_destructor(yypParser,47,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[0].minor);
  yymsp[-4].minor.yy170 = yylhsminor.yy170;
        break;
      case 131: /* distinct_or_null ::= DISTINCT */
{  yy_destructor(yypParser,34,&yymsp[0].minor);
/* #line 1012 "parser.y" */
{
	phql_ret_distinct(&yymsp[0].minor.yy170);
}
/* #line 2681 "parser.c" */
}
        break;
      case 139: /* expr ::= expr IS NULL */
/* #line 1044 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_ISNULL, &yymsp[-2].minor.yy170, NULL);
}
/* #line 2689 "parser.c" */
  yy_destructor(yypParser,27,&yymsp[-1].minor);
  yy_destructor(yypParser,78,&yymsp[0].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 140: /* expr ::= expr IS NOT NULL */
/* #line 1048 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_ISNOTNULL, &yymsp[-3].minor.yy170, NULL);
}
/* #line 2699 "parser.c" */
  yy_destructor(yypParser,27,&yymsp[-2].minor);
  yy_destructor(yypParser,29,&yymsp[-1].minor);
  yy_destructor(yypParser,78,&yymsp[0].minor);
  yymsp[-3].minor.yy170 = yylhsminor.yy170;
        break;
      case 141: /* expr ::= expr BETWEEN expr */
/* #line 1052 "parser.y" */
{
	phql_ret_expr(&yylhsminor.yy170, PHQL_T_BETWEEN, &yymsp[-2].minor.yy170, &yymsp[0].minor.yy170);
}
/* #line 2710 "parser.c" */
  yy_destructor(yypParser,2,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 142: /* expr ::= NOT expr */
{  yy_destructor(yypParser,29,&yymsp[-1].minor);
/* #line 1056 "parser.y" */
{
	phql_ret_expr(&yymsp[-1].minor.yy170, PHQL_T_NOT, NULL, &yymsp[0].minor.yy170);
}
/* #line 2720 "parser.c" */
}
        break;
      case 143: /* expr ::= BITWISE_NOT expr */
{  yy_destructor(yypParser,30,&yymsp[-1].minor);
/* #line 1060 "parser.y" */
{
	phql_ret_expr(&yymsp[-1].minor.yy170, PHQL_T_BITWISE_NOT, NULL, &yymsp[0].minor.yy170);
}
/* #line 2729 "parser.c" */
}
        break;
      case 144: /* expr ::= PARENTHESES_OPEN expr PARENTHESES_CLOSE */
{  yy_destructor(yypParser,47,&yymsp[-2].minor);
/* #line 1064 "parser.y" */
{
	phql_ret_expr(&yymsp[-2].minor.yy170, PHQL_T_ENCLOSED, &yymsp[-1].minor.yy170, NULL);
}
/* #line 2738 "parser.c" */
  yy_destructor(yypParser,48,&yymsp[0].minor);
}
        break;
      case 148: /* expr ::= STRING */
/* #line 1080 "parser.y" */
{
	phql_ret_literal_zval(&yylhsminor.yy170, PHQL_T_STRING, yymsp[0].minor.yy0);
}
/* #line 2747 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 149: /* expr ::= DOUBLE */
/* #line 1084 "parser.y" */
{
	phql_ret_literal_zval(&yylhsminor.yy170, PHQL_T_DOUBLE, yymsp[0].minor.yy0);
}
/* #line 2755 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 150: /* expr ::= NULL */
{  yy_destructor(yypParser,78,&yymsp[0].minor);
/* #line 1088 "parser.y" */
{
	phql_ret_literal_zval(&yymsp[0].minor.yy170, PHQL_T_NULL, NULL);
}
/* #line 2764 "parser.c" */
}
        break;
      case 151: /* expr ::= TRUE */
{  yy_destructor(yypParser,81,&yymsp[0].minor);
/* #line 1092 "parser.y" */
{
	phql_ret_literal_zval(&yymsp[0].minor.yy170, PHQL_T_TRUE, NULL);
}
/* #line 2773 "parser.c" */
}
        break;
      case 152: /* expr ::= FALSE */
{  yy_destructor(yypParser,82,&yymsp[0].minor);
/* #line 1096 "parser.y" */
{
	phql_ret_literal_zval(&yymsp[0].minor.yy170, PHQL_T_FALSE, NULL);
}
/* #line 2782 "parser.c" */
}
        break;
      case 155: /* expr ::= BPLACEHOLDER */
/* #line 1111 "parser.y" */
{
	phql_ret_placeholder_zval(&yylhsminor.yy170, PHQL_T_BPLACEHOLDER, yymsp[0].minor.yy0);
}
/* #line 2790 "parser.c" */
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 156: /* qualified_name ::= IDENTIFIER COLON IDENTIFIER DOT IDENTIFIER */
/* #line 1115 "parser.y" */
{
	phql_ret_qualified_name(&yylhsminor.yy170, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, yymsp[0].minor.yy0);
}
/* #line 2798 "parser.c" */
  yy_destructor(yypParser,84,&yymsp[-3].minor);
  yy_destructor(yypParser,37,&yymsp[-1].minor);
  yymsp[-4].minor.yy170 = yylhsminor.yy170;
        break;
      case 157: /* qualified_name ::= IDENTIFIER COLON IDENTIFIER */
/* #line 1119 "parser.y" */
{
	phql_ret_qualified_name(&yylhsminor.yy170, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy0);
}
/* #line 2808 "parser.c" */
  yy_destructor(yypParser,84,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 158: /* qualified_name ::= IDENTIFIER DOT IDENTIFIER */
/* #line 1123 "parser.y" */
{
	phql_ret_qualified_name(&yylhsminor.yy170, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy0);
}
/* #line 2817 "parser.c" */
  yy_destructor(yypParser,37,&yymsp[-1].minor);
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
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
/* #line 407 "parser.y" */

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
/* #line 2942 "parser.c" */
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
		PHALCON_STR(error_msg, error);
	} else {
		PHALCON_STR(error_msg, "Scanning error near to EOF");
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
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "PHQL is must be string");
		return FAILURE;
	}

	if (phql_internal_parse_phql(result, Z_STRVAL_P(phql), Z_STRLEN_P(phql), &error_msg) == FAILURE) {
		if (Z_TYPE(error_msg) > IS_NULL) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, Z_STRVAL(error_msg));
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
		PHALCON_STR(error_msg, "PHQL statement cannot be NULL");
		return FAILURE;
	}

	ZVAL_LONG(&unique_id, zend_inline_hash_func(phql, phql_length));

	phalcon_orm_get_prepared_ast(result, &unique_id);

	if (Z_TYPE_P(result) == IS_ARRAY) {
		return SUCCESS;
	}

	phql_parser = phql_Alloc(phql_wrapper_alloc);
	if (unlikely(!phql_parser)) {
		PHALCON_STR(error_msg, "Memory allocation error");
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
					PHALCON_STR(error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_DOUBLE:
				if (parser_status->enable_literals) {
					phql_parse_with_token(phql_parser, PHQL_T_DOUBLE, PHQL_DOUBLE, &token, parser_status);
				} else {
					PHALCON_STR(error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_STRING:
				if (parser_status->enable_literals) {
					phql_parse_with_token(phql_parser, PHQL_T_STRING, PHQL_STRING, &token, parser_status);
				} else {
					PHALCON_STR(error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_TRUE:
				if (parser_status->enable_literals) {
					phql_(phql_parser, PHQL_TRUE, NULL, parser_status);
				} else {
					PHALCON_STR(error_msg, "Literals are disabled in PHQL statements");
					parser_status->status = PHQL_PARSING_FAILED;
				}
				break;
			case PHQL_T_FALSE:
				if (parser_status->enable_literals) {
					phql_(phql_parser, PHQL_FALSE, NULL, parser_status);
				} else {
					PHALCON_STR(error_msg, "Literals are disabled in PHQL statements");
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
				PHALCON_STR(error_msg, error);
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
				if (Z_TYPE_P(error_msg) > IS_NULL) {
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
				PHALCON_STR(error_msg, parser_status->syntax_error);
			}
			efree(parser_status->syntax_error);
		}
	}

	phql_Free(phql_parser, phql_wrapper_free);

	if (status != FAILURE) {
		if (parser_status->status == PHQL_PARSING_OK) {
			if (Z_TYPE_P(&parser_status->ret) == IS_ARRAY) {

				/**
				 * Set a unique id for the parsed ast
				 */
				if (phalcon_globals_ptr->orm.cache_level >= 1) {
					add_assoc_long(&parser_status->ret, "id", Z_LVAL(unique_id));
				}

				ZVAL_COPY(result, &parser_status->ret);

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
