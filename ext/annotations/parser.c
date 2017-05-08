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
/* #line 36 "parser.y" */


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

/* #line 132 "parser.c" */
/**************** End of %include directives **********************************/
/* These constants specify the various numeric values for terminal symbols
** in a format understandable to "makeheaders".  This section is blank unless
** "lemon" is run with the "-m" command-line option.
***************** Begin makeheaders token definitions *************************/
/**************** End makeheaders token definitions ***************************/

/* The next sections is a series of control #defines.
** various aspects of the generated parser.
**    JJCODETYPE         is the data type used to store the integer codes
**                       that represent terminal and non-terminal symbols.
**                       "unsigned char" is used if there are fewer than
**                       256 symbols.  Larger types otherwise.
**    JJNOCODE           is a number of type JJCODETYPE that is not used for
**                       any terminal or nonterminal symbol.
**    JJFALLBACK         If defined, this indicates that one or more tokens
**                       (also known as: "terminal symbols") have fall-back
**                       values which should be used if the original symbol
**                       would not parse.  This permits keywords to sometimes
**                       be used as identifiers, for example.
**    JJACTIONTYPE       is the data type used for "action codes" - numbers
**                       that indicate what to do in response to the next
**                       token.
**    phannot_JTOKENTYPE     is the data type used for minor type for terminal
**                       symbols.  Background: A "minor type" is a semantic
**                       value associated with a terminal or non-terminal
**                       symbols.  For example, for an "ID" terminal symbol,
**                       the minor type might be the name of the identifier.
**                       Each non-terminal can have a different minor type.
**                       Terminal symbols all have the same minor type, though.
**                       This macros defines the minor type for terminal 
**                       symbols.
**    JJMINORTYPE        is the data type used for all minor types.
**                       This is typically a union of many types, one of
**                       which is phannot_JTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "jj0".
**    JJSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    phannot_ARG_SDECL     A static variable declaration for the %extra_argument
**    phannot_ARG_PDECL     A parameter declaration for the %extra_argument
**    phannot_ARG_STORE     Code to store %extra_argument into jjpParser
**    phannot_ARG_FETCH     Code to extract %extra_argument from jjpParser
**    JJERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
**    JJNSTATE           the combined number of states.
**    JJNRULE            the number of rules in the grammar
**    JJ_MAX_SHIFT       Maximum value for shift actions
**    JJ_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    JJ_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    JJ_MIN_REDUCE      Maximum value for reduce actions
**    JJ_ERROR_ACTION    The jj_action[] code for syntax error
**    JJ_ACCEPT_ACTION   The jj_action[] code for accept
**    JJ_NO_ACTION       The jj_action[] code for no-op
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define JJCODETYPE unsigned char
#define JJNOCODE 28
#define JJACTIONTYPE unsigned char
#define phannot_JTOKENTYPE phannot_parser_token*
typedef union {
  int jjinit;
  phannot_JTOKENTYPE jj0;
  zval jj8;
} JJMINORTYPE;
#ifndef JJSTACKDEPTH
#define JJSTACKDEPTH 100
#endif
#define phannot_ARG_SDECL phannot_parser_status *status;
#define phannot_ARG_PDECL ,phannot_parser_status *status
#define phannot_ARG_FETCH phannot_parser_status *status = jjpParser->status
#define phannot_ARG_STORE jjpParser->status = status
#define JJNSTATE             18
#define JJNRULE              25
#define JJ_MAX_SHIFT         17
#define JJ_MIN_SHIFTREDUCE   40
#define JJ_MAX_SHIFTREDUCE   64
#define JJ_MIN_REDUCE        65
#define JJ_MAX_REDUCE        89
#define JJ_ERROR_ACTION      90
#define JJ_ACCEPT_ACTION     91
#define JJ_NO_ACTION         92
/************* End control #defines *******************************************/

/* Define the jjtestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define jjtestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the jjtestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef jjtestcase
# define jjtestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N <= JJ_MAX_SHIFT             Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   N between JJ_MIN_SHIFTREDUCE       Shift to an arbitrary state then
**     and JJ_MAX_SHIFTREDUCE           reduce by rule N-JJ_MIN_SHIFTREDUCE.
**
**   N between JJ_MIN_REDUCE            Reduce by rule N-JJ_MIN_REDUCE
**     and JJ_MAX_REDUCE

**   N == JJ_ERROR_ACTION               A syntax error has occurred.
**
**   N == JJ_ACCEPT_ACTION              The parser accepts its input.
**
**   N == JJ_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the jj_action[] table.
**
** The action table is constructed as a single large table named jj_action[].
** Given state S and lookahead X, the action is computed as
**
**      jj_action[ jj_shift_ofst[S] + X ]
**
** If the index value jj_shift_ofst[S]+X is out of range or if the value
** jj_lookahead[jj_shift_ofst[S]+X] is not equal to X or if jj_shift_ofst[S]
** is equal to JJ_SHIFT_USE_DFLT, it means that the action is not in the table
** and that jj_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the jj_reduce_ofst[] array is used in place of
** the jj_shift_ofst[] array and JJ_REDUCE_USE_DFLT is used in place of
** JJ_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  jj_action[]        A single table containing all actions.
**  jj_lookahead[]     A table containing the lookahead for each entry in
**                     jj_action.  Used to detect hash collisions.
**  jj_shift_ofst[]    For each state, the offset into jj_action for
**                     shifting terminals.
**  jj_reduce_ofst[]   For each state, the offset into jj_action for
**                     shifting non-terminals after a reduce.
**  jj_default[]       Default action for each state.
**
*********** Begin parsing tables **********************************************/
#define JJ_ACTTAB_COUNT (84)
static const JJACTIONTYPE jj_action[] = {
 /*     0 */    16,   11,   54,   45,   13,   51,   55,   57,   59,   60,
 /*    10 */    61,   62,    3,   16,    2,   16,   11,   54,   42,   13,
 /*    20 */    53,   55,   57,   59,   60,   61,   62,    3,    1,    2,
 /*    30 */    16,   56,   54,   65,   58,   52,   55,   57,   59,   60,
 /*    40 */    61,   62,    3,   15,    2,   54,   14,   48,   49,   55,
 /*    50 */     4,   67,   54,   10,   48,   49,   55,   54,   12,   48,
 /*    60 */    49,   55,   91,   17,    9,   43,   64,   54,    4,   47,
 /*    70 */    49,   55,   54,    7,    6,   50,   55,    8,    5,    4,
 /*    80 */    67,   67,   63,   44,
};
static const JJCODETYPE jj_lookahead[] = {
 /*     0 */     2,    3,   22,    5,    6,   25,   26,    9,   10,   11,
 /*    10 */    12,   13,   14,    2,   16,    2,    3,   22,   22,    6,
 /*    20 */    25,   26,    9,   10,   11,   12,   13,   14,    4,   16,
 /*    30 */     2,    3,   22,    0,    6,   25,   26,    9,   10,   11,
 /*    40 */    12,   13,   14,    3,   16,   22,   23,   24,   25,   26,
 /*    50 */     1,   27,   22,   23,   24,   25,   26,   22,   23,   24,
 /*    60 */    25,   26,   19,   20,   21,   22,   17,   22,    1,   24,
 /*    70 */    25,   26,   22,    7,    8,   25,   26,    7,    8,    1,
 /*    80 */    27,   27,   15,    5,
};
#define JJ_SHIFT_USE_DFLT (-3)
#define JJ_SHIFT_COUNT (17)
#define JJ_SHIFT_MIN   (-2)
#define JJ_SHIFT_MAX   (78)
static const signed char jj_shift_ofst[] = {
 /*     0 */    11,   -2,   13,   13,   13,   28,   28,   28,   28,   11,
 /*    10 */    49,   66,   67,   70,   78,   24,   40,   33,
};
#define JJ_REDUCE_USE_DFLT (-21)
#define JJ_REDUCE_COUNT (9)
#define JJ_REDUCE_MIN   (-20)
#define JJ_REDUCE_MAX   (50)
static const signed char jj_reduce_ofst[] = {
 /*     0 */    43,   23,   30,   35,   45,  -20,   -5,   10,   50,   -4,
};
static const JJACTIONTYPE jj_default[] = {
 /*     0 */    90,   90,   90,   90,   90,   90,   90,   90,   90,   66,
 /*    10 */    90,   81,   90,   83,   90,   71,   90,   90,
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
#ifdef JJFALLBACK
static const JJCODETYPE jjFallback[] = {
};
#endif /* JJFALLBACK */

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
struct jjStackEntry {
  JJACTIONTYPE stateno;  /* The state-number, or reduce action in SHIFTREDUCE */
  JJCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  JJMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct jjStackEntry jjStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct jjParser {
  int jjidx;                    /* Index of top element in stack */
#ifdef JJTRACKMAXSTACKDEPTH
  int jjidxMax;                 /* Maximum value of jjidx */
#endif
#ifndef JJNOERRORRECOVERY
  int jjerrcnt;                 /* Shifts left before out of the error */
#endif
  phannot_ARG_SDECL                /* A place to hold %extra_argument */
#if JJSTACKDEPTH<=0
  int jjstksz;                  /* Current side of the stack */
  jjStackEntry *jjstack;        /* The parser's stack */
#else
  jjStackEntry jjstack[JJSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct jjParser jjParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *jjTraceFILE = 0;
static char *jjTracePrompt = 0;
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
void phannot_Trace(FILE *TraceFILE, char *zTracePrompt){
  jjTraceFILE = TraceFILE;
  jjTracePrompt = zTracePrompt;
  if( jjTraceFILE==0 ) jjTracePrompt = 0;
  else if( jjTracePrompt==0 ) jjTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const jjTokenName[] = { 
  "$",             "COMMA",         "AT",            "IDENTIFIER",  
  "PARENTHESES_OPEN",  "PARENTHESES_CLOSE",  "STRING",        "EQUALS",      
  "COLON",         "INTEGER",       "DOUBLE",        "NULL",        
  "FALSE",         "TRUE",          "BRACKET_OPEN",  "BRACKET_CLOSE",
  "SBRACKET_OPEN",  "SBRACKET_CLOSE",  "error",         "program",     
  "annotation_language",  "annotation_list",  "annotation",    "argument_list",
  "argument_item",  "expr",          "array",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const jjRuleName[] = {
 /*   0 */ "program ::= annotation_language",
 /*   1 */ "annotation_language ::= annotation_list",
 /*   2 */ "annotation_list ::= annotation_list annotation",
 /*   3 */ "annotation_list ::= annotation",
 /*   4 */ "annotation ::= AT IDENTIFIER PARENTHESES_OPEN argument_list PARENTHESES_CLOSE",
 /*   5 */ "annotation ::= AT IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /*   6 */ "annotation ::= AT IDENTIFIER",
 /*   7 */ "argument_list ::= argument_list COMMA argument_item",
 /*   8 */ "argument_list ::= argument_item",
 /*   9 */ "argument_item ::= expr",
 /*  10 */ "argument_item ::= STRING EQUALS expr",
 /*  11 */ "argument_item ::= STRING COLON expr",
 /*  12 */ "argument_item ::= IDENTIFIER EQUALS expr",
 /*  13 */ "argument_item ::= IDENTIFIER COLON expr",
 /*  14 */ "expr ::= annotation",
 /*  15 */ "expr ::= array",
 /*  16 */ "expr ::= IDENTIFIER",
 /*  17 */ "expr ::= INTEGER",
 /*  18 */ "expr ::= STRING",
 /*  19 */ "expr ::= DOUBLE",
 /*  20 */ "expr ::= NULL",
 /*  21 */ "expr ::= FALSE",
 /*  22 */ "expr ::= TRUE",
 /*  23 */ "array ::= BRACKET_OPEN argument_list BRACKET_CLOSE",
 /*  24 */ "array ::= SBRACKET_OPEN argument_list SBRACKET_CLOSE",
};
#endif /* NDEBUG */


#if JJSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void jjGrowStack(jjParser *p){
  int newSize;
  jjStackEntry *pNew;

  newSize = p->jjstksz*2 + 100;
  pNew = realloc(p->jjstack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->jjstack = pNew;
    p->jjstksz = newSize;
#ifndef NDEBUG
    if( jjTraceFILE ){
      fprintf(jjTraceFILE,"%sStack grows to %d entries!\n",
              jjTracePrompt, p->jjstksz);
    }
#endif
  }
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to phannot_Alloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef JJMALLOCARGTYPE
# define JJMALLOCARGTYPE size_t
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
** to phannot_ and phannot_Free.
*/
void *phannot_Alloc(void *(*mallocProc)(JJMALLOCARGTYPE)){
  jjParser *pParser;
  pParser = (jjParser*)(*mallocProc)( (JJMALLOCARGTYPE)sizeof(jjParser) );
  if( pParser ){
    pParser->jjidx = -1;
#ifdef JJTRACKMAXSTACKDEPTH
    pParser->jjidxMax = 0;
#endif
#if JJSTACKDEPTH<=0
    pParser->jjstack = NULL;
    pParser->jjstksz = 0;
    jjGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the "minor type" or semantic value
** associated with a symbol.  The symbol can be either a terminal
** or nonterminal. "jjmajor" is the symbol code, and "jjpminor" is
** a pointer to the value to be deleted.  The code used to do the 
** deletions is derived from the %destructor and/or %token_destructor
** directives of the input grammar.
*/
static void jj_destructor(
  jjParser *jjpParser,    /* The parser */
  JJCODETYPE jjmajor,     /* Type code for object to destroy */
  JJMINORTYPE *jjpminor   /* The object to be destroyed */
){
  phannot_ARG_FETCH;
  switch( jjmajor ){
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
    case 1: /* COMMA */
    case 2: /* AT */
    case 3: /* IDENTIFIER */
    case 4: /* PARENTHESES_OPEN */
    case 5: /* PARENTHESES_CLOSE */
    case 6: /* STRING */
    case 7: /* EQUALS */
    case 8: /* COLON */
    case 9: /* INTEGER */
    case 10: /* DOUBLE */
    case 11: /* NULL */
    case 12: /* FALSE */
    case 13: /* TRUE */
    case 14: /* BRACKET_OPEN */
    case 15: /* BRACKET_CLOSE */
    case 16: /* SBRACKET_OPEN */
    case 17: /* SBRACKET_CLOSE */
{
/* #line 186 "parser.y" */

	if ((jjpminor->jj0)) {
		if ((jjpminor->jj0)->free_flag) {
			efree((jjpminor->jj0)->token);
		}
		efree((jjpminor->jj0));
	}

/* #line 585 "parser.c" */
}
      break;
      /* Default NON-TERMINAL Destructor */
    case 18: /* error */
    case 19: /* program */
    case 26: /* array */
{
/* #line 23 "parser.y" */

	if (status) {
		// TODO:
	}
	if (&(jjpminor->jj8)) {
		zval_ptr_dtor(&(jjpminor->jj8));
	}

/* #line 602 "parser.c" */
}
      break;
    case 20: /* annotation_language */
    case 21: /* annotation_list */
    case 22: /* annotation */
    case 23: /* argument_list */
    case 24: /* argument_item */
    case 25: /* expr */
{
/* #line 199 "parser.y" */

    zval_ptr_dtor(&(jjpminor->jj8));

/* #line 616 "parser.c" */
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
static void jj_pop_parser_stack(jjParser *pParser){
  jjStackEntry *jjtos;
  assert( pParser->jjidx>=0 );
  jjtos = &pParser->jjstack[pParser->jjidx--];
#ifndef NDEBUG
  if( jjTraceFILE ){
    fprintf(jjTraceFILE,"%sPopping %s\n",
      jjTracePrompt,
      jjTokenName[jjtos->major]);
  }
#endif
  jj_destructor(pParser, jjtos->major, &jjtos->minor);
}

/* 
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the JJPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void phannot_Free(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  jjParser *pParser = (jjParser*)p;
#ifndef JJPARSEFREENEVERNULL
  if( pParser==0 ) return;
#endif
  while( pParser->jjidx>=0 ) jj_pop_parser_stack(pParser);
#if JJSTACKDEPTH<=0
  free(pParser->jjstack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef JJTRACKMAXSTACKDEPTH
int phannot_StackPeak(void *p){
  jjParser *pParser = (jjParser*)p;
  return pParser->jjidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
*/
static unsigned int jj_find_shift_action(
  jjParser *pParser,        /* The parser */
  JJCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->jjstack[pParser->jjidx].stateno;
 
  if( stateno>=JJ_MIN_REDUCE ) return stateno;
  assert( stateno <= JJ_SHIFT_COUNT );
  do{
    i = jj_shift_ofst[stateno];
    if( i==JJ_SHIFT_USE_DFLT ) return jj_default[stateno];
    assert( iLookAhead!=JJNOCODE );
    i += iLookAhead;
    if( i<0 || i>=JJ_ACTTAB_COUNT || jj_lookahead[i]!=iLookAhead ){
      if( iLookAhead>0 ){
#ifdef JJFALLBACK
        JJCODETYPE iFallback;            /* Fallback token */
        if( iLookAhead<sizeof(jjFallback)/sizeof(jjFallback[0])
               && (iFallback = jjFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
          if( jjTraceFILE ){
            fprintf(jjTraceFILE, "%sFALLBACK %s => %s\n",
               jjTracePrompt, jjTokenName[iLookAhead], jjTokenName[iFallback]);
          }
#endif
          assert( jjFallback[iFallback]==0 ); /* Fallback loop must terminate */
          iLookAhead = iFallback;
          continue;
        }
#endif
#ifdef JJWILDCARD
        {
          int j = i - iLookAhead + JJWILDCARD;
          if( 
#if JJ_SHIFT_MIN+JJWILDCARD<0
            j>=0 &&
#endif
#if JJ_SHIFT_MAX+JJWILDCARD>=JJ_ACTTAB_COUNT
            j<JJ_ACTTAB_COUNT &&
#endif
            jj_lookahead[j]==JJWILDCARD
          ){
#ifndef NDEBUG
            if( jjTraceFILE ){
              fprintf(jjTraceFILE, "%sWILDCARD %s => %s\n",
                 jjTracePrompt, jjTokenName[iLookAhead],
                 jjTokenName[JJWILDCARD]);
            }
#endif /* NDEBUG */
            return jj_action[j];
          }
        }
#endif /* JJWILDCARD */
      }
      return jj_default[stateno];
    }else{
      return jj_action[i];
    }
  }while(1);
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
*/
static int jj_find_reduce_action(
  int stateno,              /* Current state number */
  JJCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef JJERRORSYMBOL
  if( stateno>JJ_REDUCE_COUNT ){
    return jj_default[stateno];
  }
#else
  assert( stateno<=JJ_REDUCE_COUNT );
#endif
  i = jj_reduce_ofst[stateno];
  assert( i!=JJ_REDUCE_USE_DFLT );
  assert( iLookAhead!=JJNOCODE );
  i += iLookAhead;
#ifdef JJERRORSYMBOL
  if( i<0 || i>=JJ_ACTTAB_COUNT || jj_lookahead[i]!=iLookAhead ){
    return jj_default[stateno];
  }
#else
  assert( i>=0 && i<JJ_ACTTAB_COUNT );
  assert( jj_lookahead[i]==iLookAhead );
#endif
  return jj_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void jjStackOverflow(jjParser *jjpParser){
   phannot_ARG_FETCH;
   jjpParser->jjidx--;
#ifndef NDEBUG
   if( jjTraceFILE ){
     fprintf(jjTraceFILE,"%sStack Overflow!\n",jjTracePrompt);
   }
#endif
   while( jjpParser->jjidx>=0 ) jj_pop_parser_stack(jjpParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
/******** End %stack_overflow code ********************************************/
   phannot_ARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void jjTraceShift(jjParser *jjpParser, int jjNewState){
  if( jjTraceFILE ){
    if( jjNewState<JJNSTATE ){
      fprintf(jjTraceFILE,"%sShift '%s', go to state %d\n",
         jjTracePrompt,jjTokenName[jjpParser->jjstack[jjpParser->jjidx].major],
         jjNewState);
    }else{
      fprintf(jjTraceFILE,"%sShift '%s'\n",
         jjTracePrompt,jjTokenName[jjpParser->jjstack[jjpParser->jjidx].major]);
    }
  }
}
#else
# define jjTraceShift(X,Y)
#endif

/*
** Perform a shift action.
*/
static void jj_shift(
  jjParser *jjpParser,          /* The parser to be shifted */
  int jjNewState,               /* The new state to shift in */
  int jjMajor,                  /* The major token to shift in */
  phannot_JTOKENTYPE jjMinor        /* The minor token to shift in */
){
  jjStackEntry *jjtos;
  jjpParser->jjidx++;
#ifdef JJTRACKMAXSTACKDEPTH
  if( jjpParser->jjidx>jjpParser->jjidxMax ){
    jjpParser->jjidxMax = jjpParser->jjidx;
  }
#endif
#if JJSTACKDEPTH>0 
  if( jjpParser->jjidx>=JJSTACKDEPTH ){
    jjStackOverflow(jjpParser);
    return;
  }
#else
  if( jjpParser->jjidx>=jjpParser->jjstksz ){
    jjGrowStack(jjpParser);
    if( jjpParser->jjidx>=jjpParser->jjstksz ){
      jjStackOverflow(jjpParser);
      return;
    }
  }
#endif
  jjtos = &jjpParser->jjstack[jjpParser->jjidx];
  jjtos->stateno = (JJACTIONTYPE)jjNewState;
  jjtos->major = (JJCODETYPE)jjMajor;
  jjtos->minor.jj0 = jjMinor;
  jjTraceShift(jjpParser, jjNewState);
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  JJCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} jjRuleInfo[] = {
  { 19, 1 },
  { 20, 1 },
  { 21, 2 },
  { 21, 1 },
  { 22, 5 },
  { 22, 4 },
  { 22, 2 },
  { 23, 3 },
  { 23, 1 },
  { 24, 1 },
  { 24, 3 },
  { 24, 3 },
  { 24, 3 },
  { 24, 3 },
  { 25, 1 },
  { 25, 1 },
  { 25, 1 },
  { 25, 1 },
  { 25, 1 },
  { 25, 1 },
  { 25, 1 },
  { 25, 1 },
  { 25, 1 },
  { 26, 3 },
  { 26, 3 },
};

static void jj_accept(jjParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void jj_reduce(
  jjParser *jjpParser,         /* The parser */
  unsigned int jjruleno        /* Number of the rule by which to reduce */
){
  int jjgoto;                     /* The next state */
  int jjact;                      /* The next action */
  jjStackEntry *jjmsp;            /* The top of the parser's stack */
  int jjsize;                     /* Amount to pop the stack */
  phannot_ARG_FETCH;
  jjmsp = &jjpParser->jjstack[jjpParser->jjidx];
#ifndef NDEBUG
  if( jjTraceFILE && jjruleno<(int)(sizeof(jjRuleName)/sizeof(jjRuleName[0])) ){
    jjsize = jjRuleInfo[jjruleno].nrhs;
    fprintf(jjTraceFILE, "%sReduce [%s], go to state %d.\n", jjTracePrompt,
      jjRuleName[jjruleno], jjmsp[-jjsize].stateno);
  }
#endif /* NDEBUG */

  /* Check that the stack is large enough to grow by a single entry
  ** if the RHS of the rule is empty.  This ensures that there is room
  ** enough on the stack to push the LHS value */
  if( jjRuleInfo[jjruleno].nrhs==0 ){
#ifdef JJTRACKMAXSTACKDEPTH
    if( jjpParser->jjidx>jjpParser->jjidxMax ){
      jjpParser->jjidxMax = jjpParser->jjidx;
    }
#endif
#if JJSTACKDEPTH>0 
    if( jjpParser->jjidx>=JJSTACKDEPTH-1 ){
      jjStackOverflow(jjpParser);
      return;
    }
#else
    if( jjpParser->jjidx>=jjpParser->jjstksz-1 ){
      jjGrowStack(jjpParser);
      if( jjpParser->jjidx>=jjpParser->jjstksz-1 ){
        jjStackOverflow(jjpParser);
        return;
      }
    }
#endif
  }

  switch( jjruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
/********** Begin reduce actions **********************************************/
        JJMINORTYPE jjlhsminor;
      case 0: /* program ::= annotation_language */
/* #line 195 "parser.y" */
{
	ZVAL_ZVAL(&status->ret, &jjmsp[0].minor.jj8, 1, 1);
}
/* #line 949 "parser.c" */
        break;
      case 1: /* annotation_language ::= annotation_list */
      case 14: /* expr ::= annotation */ jjtestcase(jjruleno==14);
      case 15: /* expr ::= array */ jjtestcase(jjruleno==15);
/* #line 203 "parser.y" */
{
	jjlhsminor.jj8 = jjmsp[0].minor.jj8;
}
/* #line 958 "parser.c" */
  jjmsp[0].minor.jj8 = jjlhsminor.jj8;
        break;
      case 2: /* annotation_list ::= annotation_list annotation */
/* #line 211 "parser.y" */
{
	phannot_ret_zval_list(&jjlhsminor.jj8, &jjmsp[-1].minor.jj8, &jjmsp[0].minor.jj8);
}
/* #line 966 "parser.c" */
  jjmsp[-1].minor.jj8 = jjlhsminor.jj8;
        break;
      case 3: /* annotation_list ::= annotation */
      case 8: /* argument_list ::= argument_item */ jjtestcase(jjruleno==8);
/* #line 215 "parser.y" */
{
	phannot_ret_zval_list(&jjlhsminor.jj8, NULL, &jjmsp[0].minor.jj8);
}
/* #line 975 "parser.c" */
  jjmsp[0].minor.jj8 = jjlhsminor.jj8;
        break;
      case 4: /* annotation ::= AT IDENTIFIER PARENTHESES_OPEN argument_list PARENTHESES_CLOSE */
{  jj_destructor(jjpParser,2,&jjmsp[-4].minor);
/* #line 223 "parser.y" */
{
	phannot_ret_annotation(&jjmsp[-4].minor.jj8, jjmsp[-3].minor.jj0, &jjmsp[-1].minor.jj8, status->scanner_state);
}
/* #line 984 "parser.c" */
  jj_destructor(jjpParser,4,&jjmsp[-2].minor);
  jj_destructor(jjpParser,5,&jjmsp[0].minor);
}
        break;
      case 5: /* annotation ::= AT IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
{  jj_destructor(jjpParser,2,&jjmsp[-3].minor);
/* #line 227 "parser.y" */
{
	phannot_ret_annotation(&jjmsp[-3].minor.jj8, jjmsp[-2].minor.jj0, NULL, status->scanner_state);
}
/* #line 995 "parser.c" */
  jj_destructor(jjpParser,4,&jjmsp[-1].minor);
  jj_destructor(jjpParser,5,&jjmsp[0].minor);
}
        break;
      case 6: /* annotation ::= AT IDENTIFIER */
{  jj_destructor(jjpParser,2,&jjmsp[-1].minor);
/* #line 231 "parser.y" */
{
	phannot_ret_annotation(&jjmsp[-1].minor.jj8, jjmsp[0].minor.jj0, NULL, status->scanner_state);
}
/* #line 1006 "parser.c" */
}
        break;
      case 7: /* argument_list ::= argument_list COMMA argument_item */
/* #line 239 "parser.y" */
{
	phannot_ret_zval_list(&jjlhsminor.jj8, &jjmsp[-2].minor.jj8, &jjmsp[0].minor.jj8);
}
/* #line 1014 "parser.c" */
  jj_destructor(jjpParser,1,&jjmsp[-1].minor);
  jjmsp[-2].minor.jj8 = jjlhsminor.jj8;
        break;
      case 9: /* argument_item ::= expr */
/* #line 251 "parser.y" */
{
	phannot_ret_named_item(&jjlhsminor.jj8, NULL, &jjmsp[0].minor.jj8);
}
/* #line 1023 "parser.c" */
  jjmsp[0].minor.jj8 = jjlhsminor.jj8;
        break;
      case 10: /* argument_item ::= STRING EQUALS expr */
      case 11: /* argument_item ::= STRING COLON expr */ jjtestcase(jjruleno==11);
      case 12: /* argument_item ::= IDENTIFIER EQUALS expr */ jjtestcase(jjruleno==12);
      case 13: /* argument_item ::= IDENTIFIER COLON expr */ jjtestcase(jjruleno==13);
/* #line 255 "parser.y" */
{
	phannot_ret_named_item(&jjlhsminor.jj8, jjmsp[-2].minor.jj0, &jjmsp[0].minor.jj8);
}
/* #line 1034 "parser.c" */
  jj_destructor(jjpParser,7,&jjmsp[-1].minor);
  jjmsp[-2].minor.jj8 = jjlhsminor.jj8;
        break;
      case 16: /* expr ::= IDENTIFIER */
/* #line 283 "parser.y" */
{
	phannot_ret_literal_zval(&jjlhsminor.jj8, PHANNOT_T_IDENTIFIER, jjmsp[0].minor.jj0);
}
/* #line 1043 "parser.c" */
  jjmsp[0].minor.jj8 = jjlhsminor.jj8;
        break;
      case 17: /* expr ::= INTEGER */
/* #line 287 "parser.y" */
{
	phannot_ret_literal_zval(&jjlhsminor.jj8, PHANNOT_T_INTEGER, jjmsp[0].minor.jj0);
}
/* #line 1051 "parser.c" */
  jjmsp[0].minor.jj8 = jjlhsminor.jj8;
        break;
      case 18: /* expr ::= STRING */
/* #line 291 "parser.y" */
{
	phannot_ret_literal_zval(&jjlhsminor.jj8, PHANNOT_T_STRING, jjmsp[0].minor.jj0);
}
/* #line 1059 "parser.c" */
  jjmsp[0].minor.jj8 = jjlhsminor.jj8;
        break;
      case 19: /* expr ::= DOUBLE */
/* #line 295 "parser.y" */
{
	phannot_ret_literal_zval(&jjlhsminor.jj8, PHANNOT_T_DOUBLE, jjmsp[0].minor.jj0);
}
/* #line 1067 "parser.c" */
  jjmsp[0].minor.jj8 = jjlhsminor.jj8;
        break;
      case 20: /* expr ::= NULL */
{  jj_destructor(jjpParser,11,&jjmsp[0].minor);
/* #line 299 "parser.y" */
{
	phannot_ret_literal_zval(&jjmsp[0].minor.jj8, PHANNOT_T_NULL, NULL);
}
/* #line 1076 "parser.c" */
}
        break;
      case 21: /* expr ::= FALSE */
{  jj_destructor(jjpParser,12,&jjmsp[0].minor);
/* #line 303 "parser.y" */
{
	phannot_ret_literal_zval(&jjmsp[0].minor.jj8, PHANNOT_T_FALSE, NULL);
}
/* #line 1085 "parser.c" */
}
        break;
      case 22: /* expr ::= TRUE */
{  jj_destructor(jjpParser,13,&jjmsp[0].minor);
/* #line 307 "parser.y" */
{
	phannot_ret_literal_zval(&jjmsp[0].minor.jj8, PHANNOT_T_TRUE, NULL);
}
/* #line 1094 "parser.c" */
}
        break;
      case 23: /* array ::= BRACKET_OPEN argument_list BRACKET_CLOSE */
      case 24: /* array ::= SBRACKET_OPEN argument_list SBRACKET_CLOSE */ jjtestcase(jjruleno==24);
{  jj_destructor(jjpParser,14,&jjmsp[-2].minor);
/* #line 311 "parser.y" */
{
	phannot_ret_array(&jjmsp[-2].minor.jj8, &jjmsp[-1].minor.jj8);
}
/* #line 1104 "parser.c" */
  jj_destructor(jjpParser,15,&jjmsp[0].minor);
}
        break;
      default:
        break;
/********** End reduce actions ************************************************/
  };
  assert( jjruleno<sizeof(jjRuleInfo)/sizeof(jjRuleInfo[0]) );
  jjgoto = jjRuleInfo[jjruleno].lhs;
  jjsize = jjRuleInfo[jjruleno].nrhs;
  jjact = jj_find_reduce_action(jjmsp[-jjsize].stateno,(JJCODETYPE)jjgoto);
  if( jjact <= JJ_MAX_SHIFTREDUCE ){
    if( jjact>JJ_MAX_SHIFT ) jjact += JJ_MIN_REDUCE - JJ_MIN_SHIFTREDUCE;
    jjpParser->jjidx -= jjsize - 1;
    jjmsp -= jjsize-1;
    jjmsp->stateno = (JJACTIONTYPE)jjact;
    jjmsp->major = (JJCODETYPE)jjgoto;
    jjTraceShift(jjpParser, jjact);
  }else{
    assert( jjact == JJ_ACCEPT_ACTION );
    jjpParser->jjidx -= jjsize;
    jj_accept(jjpParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef JJNOERRORRECOVERY
static void jj_parse_failed(
  jjParser *jjpParser           /* The parser */
){
  phannot_ARG_FETCH;
#ifndef NDEBUG
  if( jjTraceFILE ){
    fprintf(jjTraceFILE,"%sFail!\n",jjTracePrompt);
  }
#endif
  while( jjpParser->jjidx>=0 ) jj_pop_parser_stack(jjpParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
/************ End %parse_failure code *****************************************/
  phannot_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* JJNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void jj_syntax_error(
  jjParser *jjpParser,           /* The parser */
  int jjmajor,                   /* The major type of the error token */
  phannot_JTOKENTYPE jjminor         /* The minor type of the error token */
){
  phannot_ARG_FETCH;
#define JTOKEN jjminor
/************ Begin %syntax_error code ****************************************/
/* #line 141 "parser.y" */

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
/* #line 1207 "parser.c" */
/************ End %syntax_error code ******************************************/
  phannot_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void jj_accept(
  jjParser *jjpParser           /* The parser */
){
  phannot_ARG_FETCH;
#ifndef NDEBUG
  if( jjTraceFILE ){
    fprintf(jjTraceFILE,"%sAccept!\n",jjTracePrompt);
  }
#endif
  while( jjpParser->jjidx>=0 ) jj_pop_parser_stack(jjpParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/
/*********** End %parse_accept code *******************************************/
  phannot_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "phannot_Alloc" which describes the current state of the parser.
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
void phannot_(
  void *jjp,                   /* The parser */
  int jjmajor,                 /* The major token code number */
  phannot_JTOKENTYPE jjminor       /* The value for the token */
  phannot_ARG_PDECL               /* Optional %extra_argument parameter */
){
  JJMINORTYPE jjminorunion;
  unsigned int jjact;   /* The parser action. */
#if !defined(JJERRORSYMBOL) && !defined(JJNOERRORRECOVERY)
  int jjendofinput;     /* True if we are at the end of input */
#endif
#ifdef JJERRORSYMBOL
  int jjerrorhit = 0;   /* True if jjmajor has invoked an error */
#endif
  jjParser *jjpParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  jjpParser = (jjParser*)jjp;
  if( jjpParser->jjidx<0 ){
#if JJSTACKDEPTH<=0
    if( jjpParser->jjstksz <=0 ){
      jjStackOverflow(jjpParser);
      return;
    }
#endif
    jjpParser->jjidx = 0;
#ifndef JJNOERRORRECOVERY
    jjpParser->jjerrcnt = -1;
#endif
    jjpParser->jjstack[0].stateno = 0;
    jjpParser->jjstack[0].major = 0;
#ifndef NDEBUG
    if( jjTraceFILE ){
      fprintf(jjTraceFILE,"%sInitialize. Empty stack. State 0\n",
              jjTracePrompt);
    }
#endif
  }
#if !defined(JJERRORSYMBOL) && !defined(JJNOERRORRECOVERY)
  jjendofinput = (jjmajor==0);
#endif
  phannot_ARG_STORE;

#ifndef NDEBUG
  if( jjTraceFILE ){
    fprintf(jjTraceFILE,"%sInput '%s'\n",jjTracePrompt,jjTokenName[jjmajor]);
  }
#endif

  do{
    jjact = jj_find_shift_action(jjpParser,(JJCODETYPE)jjmajor);
    if( jjact <= JJ_MAX_SHIFTREDUCE ){
      if( jjact > JJ_MAX_SHIFT ) jjact += JJ_MIN_REDUCE - JJ_MIN_SHIFTREDUCE;
      jj_shift(jjpParser,jjact,jjmajor,jjminor);
#ifndef JJNOERRORRECOVERY
      jjpParser->jjerrcnt--;
#endif
      jjmajor = JJNOCODE;
    }else if( jjact <= JJ_MAX_REDUCE ){
      jj_reduce(jjpParser,jjact-JJ_MIN_REDUCE);
    }else{
      assert( jjact == JJ_ERROR_ACTION );
      jjminorunion.jj0 = jjminor;
#ifdef JJERRORSYMBOL
      int jjmx;
#endif
#ifndef NDEBUG
      if( jjTraceFILE ){
        fprintf(jjTraceFILE,"%sSyntax Error!\n",jjTracePrompt);
      }
#endif
#ifdef JJERRORSYMBOL
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
      if( jjpParser->jjerrcnt<0 ){
        jj_syntax_error(jjpParser,jjmajor,jjminor);
      }
      jjmx = jjpParser->jjstack[jjpParser->jjidx].major;
      if( jjmx==JJERRORSYMBOL || jjerrorhit ){
#ifndef NDEBUG
        if( jjTraceFILE ){
          fprintf(jjTraceFILE,"%sDiscard input token %s\n",
             jjTracePrompt,jjTokenName[jjmajor]);
        }
#endif
        jj_destructor(jjpParser, (JJCODETYPE)jjmajor, &jjminorunion);
        jjmajor = JJNOCODE;
      }else{
        while(
          jjpParser->jjidx >= 0 &&
          jjmx != JJERRORSYMBOL &&
          (jjact = jj_find_reduce_action(
                        jjpParser->jjstack[jjpParser->jjidx].stateno,
                        JJERRORSYMBOL)) >= JJ_MIN_REDUCE
        ){
          jj_pop_parser_stack(jjpParser);
        }
        if( jjpParser->jjidx < 0 || jjmajor==0 ){
          jj_destructor(jjpParser,(JJCODETYPE)jjmajor,&jjminorunion);
          jj_parse_failed(jjpParser);
          jjmajor = JJNOCODE;
        }else if( jjmx!=JJERRORSYMBOL ){
          jj_shift(jjpParser,jjact,JJERRORSYMBOL,jjminor);
        }
      }
      jjpParser->jjerrcnt = 3;
      jjerrorhit = 1;
#elif defined(JJNOERRORRECOVERY)
      /* If the JJNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      jj_syntax_error(jjpParser,jjmajor, jjminor);
      jj_destructor(jjpParser,(JJCODETYPE)jjmajor,&jjminorunion);
      jjmajor = JJNOCODE;
      
#else  /* JJERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( jjpParser->jjerrcnt<=0 ){
        jj_syntax_error(jjpParser,jjmajor, jjminor);
      }
      jjpParser->jjerrcnt = 3;
      jj_destructor(jjpParser,(JJCODETYPE)jjmajor,&jjminorunion);
      if( jjendofinput ){
        jj_parse_failed(jjpParser);
      }
      jjmajor = JJNOCODE;
#endif
    }
  }while( jjmajor!=JJNOCODE && jjpParser->jjidx>=0 );
#ifndef NDEBUG
  if( jjTraceFILE ){
    int i;
    fprintf(jjTraceFILE,"%sReturn. Stack=",jjTracePrompt);
    for(i=1; i<=jjpParser->jjidx; i++)
      fprintf(jjTraceFILE,"%c%s", i==1 ? '[' : ' ', 
              jjTokenName[jjpParser->jjstack[i].major]);
    fprintf(jjTraceFILE,"]\n");
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

#ifndef PHP_PHALCON_H
#include "php_phalcon.h"
#include "annotations/annot.h"
#include "annotations/parser.h"
#include "annotations/scanner.h"
#endif

const phannot_token_names phannot_tokens[] =
{
	{ "INTEGER",        PHANNOT_T_INTEGER },
	{ "DOUBLE",         PHANNOT_T_DOUBLE },
	{ "STRING",         PHANNOT_T_STRING },
	{ "IDENTIFIER",     PHANNOT_T_IDENTIFIER },
	{ "@",              PHANNOT_T_AT },
	{ ",",              PHANNOT_T_COMMA },
	{ "=",              PHANNOT_T_EQUALS },
	{ ":",              PHANNOT_T_COLON },
	{ "(",              PHANNOT_T_PARENTHESES_OPEN },
	{ ")",              PHANNOT_T_PARENTHESES_CLOSE },
	{ "{",              PHANNOT_T_BRACKET_OPEN },
	{ "}",              PHANNOT_T_BRACKET_CLOSE },
 	{ "[",              PHANNOT_T_SBRACKET_OPEN },
	{ "]",              PHANNOT_T_SBRACKET_CLOSE },
	{ "ARBITRARY TEXT", PHANNOT_T_ARBITRARY_TEXT },
	{ NULL, 0 }
};

/**
 * Wrapper to alloc memory within the parser
 */
static void *phannot_wrapper_alloc(size_t bytes){
	return emalloc(bytes);
}

/**
 * Wrapper to free memory within the parser
 */
static void phannot_wrapper_free(void *pointer){
	efree(pointer);
}

/**
 * Creates a parser_token to be passed to the parser
 */
static void phannot_parse_with_token(void* phannot_parser, int opcode, int parsercode, phannot_scanner_token *token, phannot_parser_status *parser_status){

	phannot_parser_token *pToken;

	pToken = emalloc(sizeof(phannot_parser_token));
	pToken->opcode = opcode;
	pToken->token = token->value;
	pToken->token_len = token->len;
	pToken->free_flag = 1;

	phannot_(phannot_parser, parsercode, pToken, parser_status);

	token->value = NULL;
	token->len = 0;
}

/**
 * Creates an error message when it's triggered by the scanner
 */
static void phannot_scanner_error_msg(phannot_parser_status *parser_status, char **error_msg){

	phannot_scanner_state *state = parser_status->scanner_state;

	if (state->start) {
		if (state->start_length > 16) {
			spprintf(error_msg, 0, "Scanning error before '%.16s...' in %s on line %d", state->start, state->active_file, state->active_line);
		} else {
			spprintf(error_msg, 0, "Scanning error before '%s' in %s on line %d", state->start, state->active_file, state->active_line);
		}
	} else {
		spprintf(error_msg, 0, "Scanning error near to EOF in %s", state->active_file);
	}
}

/**
 * Receives the comment tokenizes and parses it
 */
int phannot_parse_annotations(zval *result, zend_string *comment, const char *file_path, uint32_t line){

	char *error_msg = NULL;

	ZVAL_NULL(result);

	if (phannot_internal_parse_annotations(&result, comment, file_path, line, &error_msg) == FAILURE) {
		if (likely(error_msg != NULL)) {
			zend_throw_exception_ex(phalcon_annotations_exception_ce, 0, "%s", error_msg);
			efree(error_msg);
		}
		else {
			zend_throw_exception_ex(phalcon_annotations_exception_ce, 0, "Error parsing annotation");
		}

		return FAILURE;
	}

	return SUCCESS;
}

/**
 * Remove comment separators from a docblock
 */
static zend_string* phannot_remove_comment_separators(zend_string *comment, uint32_t *start_lines)
{
	int start_mode = 1, j, i, open_parentheses;
	smart_str processed_str = {0};
	char ch;

	(*start_lines) = 0;

	for (i = 0; i < comment->len; i++) {

		ch = comment->val[i];

		if (start_mode) {
			if (ch == ' ' || ch == '*' || ch == '/' || ch == '\t' || ch == 11) {
				continue;
			}
			start_mode = 0;
		}

		if (ch == '@') {

			smart_str_appendc(&processed_str, ch);
			i++;

			open_parentheses = 0;
			for (j = i; j < comment->len; j++) {

				ch = comment->val[j];

				if (start_mode) {
					if (ch == ' ' || ch == '*' || ch == '/' || ch == '\t' || ch == 11) {
						continue;
					}
					start_mode = 0;
				}

				if (open_parentheses == 0) {

					if (isalnum(ch) || '_' == ch || '\\' == ch) {
						smart_str_appendc(&processed_str, ch);
						continue;
					}

					if (ch == '(') {
						smart_str_appendc(&processed_str, ch);
						open_parentheses++;
						continue;
					}

				} else {

					smart_str_appendc(&processed_str, ch);

					if (ch == '(') {
						open_parentheses++;
					} else if (ch == ')') {
						open_parentheses--;
					} else if (ch == '\n') {
						(*start_lines)++;
						start_mode = 1;
					}

					if (open_parentheses > 0) {
						continue;
					}
				}

				i = j;
				smart_str_appendc(&processed_str, ' ');
				break;
			}
		}

		if (ch == '\n') {
			(*start_lines)++;
			start_mode = 1;
		}
	}

	smart_str_0(&processed_str);

	if (processed_str.s) {
		return processed_str.s;
	} else {
		return NULL;
	}
}

/**
 * Parses a comment returning an intermediate array representation
 */
int phannot_internal_parse_annotations(zval **result, zend_string *comment, const char *file_path, uint32_t line, char **error_msg)
{
	phannot_scanner_state *state;
	phannot_scanner_token token;
	uint32_t start_lines;
	int scanner_status, status = SUCCESS;
	phannot_parser_status *parser_status = NULL;
	void* phannot_parser;
	zend_string *processed_comment;

	*error_msg = NULL;

	/**
	 * Check if the comment has content
	 */
	if (UNEXPECTED(!comment)) {
		ZVAL_BOOL(*result, 0);
		spprintf(error_msg, 0, "Empty annotation");
		return FAILURE;
	}

	if (comment->len < 2) {
		ZVAL_BOOL(*result, 0);
		return SUCCESS;
	}

	/**
	 * Remove comment separators
	 */
	processed_comment = phannot_remove_comment_separators(comment, &start_lines);

	if (!processed_comment) {
		return SUCCESS;
	} else if (processed_comment->len < 2) {
		ZVAL_BOOL(*result, 0);
		zend_string_release(processed_comment);
		return SUCCESS;
	}

	/**
	 * Start the reentrant parser
	 */
	phannot_parser = phannot_Alloc(phannot_wrapper_alloc);
	if (unlikely(!phannot_parser)) {
		ZVAL_BOOL(*result, 0);
		return FAILURE;
	}

	parser_status = emalloc(sizeof(phannot_parser_status) + sizeof(phannot_scanner_state));
	state         = (phannot_scanner_state*)((char*)parser_status + sizeof(phannot_parser_status));

	parser_status->status = PHANNOT_PARSING_OK;
	parser_status->scanner_state = state;
	parser_status->token = &token;
	parser_status->syntax_error = NULL;

	/**
	 * Initialize the scanner state
	 */
	state->active_token = 0;
	state->start = processed_comment->val;
	state->start_length = 0;
	state->mode = PHANNOT_MODE_RAW;
	state->active_file = file_path;

	token.value = NULL;
	token.len = 0;

	/**
	 * Possible start line
	 */
	if (line) {
		state->active_line = line - start_lines;
	} else {
		state->active_line = 1;
	}

	state->end = state->start;

	while(0 <= (scanner_status = phannot_get_token(state, &token))) {

		state->active_token = token.opcode;

		state->start_length = processed_comment->val + processed_comment->len - state->start;

		switch (token.opcode) {

			case PHANNOT_T_IGNORE:
				break;

			case PHANNOT_T_AT:
				phannot_(phannot_parser, PHANNOT_AT, NULL, parser_status);
				break;
			case PHANNOT_T_COMMA:
				phannot_(phannot_parser, PHANNOT_COMMA, NULL, parser_status);
				break;
			case PHANNOT_T_EQUALS:
				phannot_(phannot_parser, PHANNOT_EQUALS, NULL, parser_status);
				break;
			case PHANNOT_T_COLON:
				phannot_(phannot_parser, PHANNOT_COLON, NULL, parser_status);
				break;

			case PHANNOT_T_PARENTHESES_OPEN:
				phannot_(phannot_parser, PHANNOT_PARENTHESES_OPEN, NULL, parser_status);
				break;
			case PHANNOT_T_PARENTHESES_CLOSE:
				phannot_(phannot_parser, PHANNOT_PARENTHESES_CLOSE, NULL, parser_status);
				break;

			case PHANNOT_T_BRACKET_OPEN:
				phannot_(phannot_parser, PHANNOT_BRACKET_OPEN, NULL, parser_status);
				break;
			case PHANNOT_T_BRACKET_CLOSE:
				phannot_(phannot_parser, PHANNOT_BRACKET_CLOSE, NULL, parser_status);
				break;

			case PHANNOT_T_SBRACKET_OPEN:
				phannot_(phannot_parser, PHANNOT_SBRACKET_OPEN, NULL, parser_status);
				break;
			case PHANNOT_T_SBRACKET_CLOSE:
				phannot_(phannot_parser, PHANNOT_SBRACKET_CLOSE, NULL, parser_status);
				break;

			case PHANNOT_T_NULL:
				phannot_(phannot_parser, PHANNOT_NULL, NULL, parser_status);
				break;
			case PHANNOT_T_TRUE:
				phannot_(phannot_parser, PHANNOT_TRUE, NULL, parser_status);
				break;
			case PHANNOT_T_FALSE:
				phannot_(phannot_parser, PHANNOT_FALSE, NULL, parser_status);
				break;

			case PHANNOT_T_INTEGER:
				phannot_parse_with_token(phannot_parser, PHANNOT_T_INTEGER, PHANNOT_INTEGER, &token, parser_status);
				break;
			case PHANNOT_T_DOUBLE:
				phannot_parse_with_token(phannot_parser, PHANNOT_T_DOUBLE, PHANNOT_DOUBLE, &token, parser_status);
				break;
			case PHANNOT_T_STRING:
				phannot_parse_with_token(phannot_parser, PHANNOT_T_STRING, PHANNOT_STRING, &token, parser_status);
				break;
			case PHANNOT_T_IDENTIFIER:
				phannot_parse_with_token(phannot_parser, PHANNOT_T_IDENTIFIER, PHANNOT_IDENTIFIER, &token, parser_status);
				break;
			/*case PHANNOT_T_ARBITRARY_TEXT:
				phannot_parse_with_token(phannot_parser, PHANNOT_T_ARBITRARY_TEXT, PHANNOT_ARBITRARY_TEXT, &token, parser_status);
				break;*/

			default:
				parser_status->status = PHANNOT_PARSING_FAILED;
				if (!*error_msg) {
					spprintf(error_msg, 0, "Scanner: unknown opcode %d on in %s line %d", token.opcode, state->active_file, state->active_line);
				}
				break;
		}

		if (parser_status->status != PHANNOT_PARSING_OK) {
			status = FAILURE;
			break;
		}

		state->end = state->start;
	}

	if (status != FAILURE) {
		switch (scanner_status) {
			case PHANNOT_SCANNER_RETCODE_ERR:
			case PHANNOT_SCANNER_RETCODE_IMPOSSIBLE:
				if (!*error_msg) {
					phannot_scanner_error_msg(parser_status, error_msg);
				}
				status = FAILURE;
				break;
			default:
				phannot_(phannot_parser, 0, NULL, parser_status);
		}
	}

	state->active_token = 0;
	state->start = NULL;

	if (parser_status->status != PHANNOT_PARSING_OK) {
		status = FAILURE;
		if (parser_status->syntax_error) {
			if (!*error_msg) {
				*error_msg = parser_status->syntax_error;
			}
			else {
				efree(parser_status->syntax_error);
			}
		}
	}

	phannot_Free(phannot_parser, phannot_wrapper_free);

	if (status != FAILURE) {
		if (parser_status->status == PHANNOT_PARSING_OK) {
			ZVAL_COPY_VALUE(*result, &parser_status->ret);
		}
	}

	efree(parser_status);
	if (processed_comment) {
		zend_string_release(processed_comment);
	}

	return status;
}
