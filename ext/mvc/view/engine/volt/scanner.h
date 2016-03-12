
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

#ifndef PHALCON_MVC_VIEW_ENGINE_VOLT_SCANNER_H
#define PHALCON_MVC_VIEW_ENGINE_VOLT_SCANNER_H

#include "php_phalcon.h"

#define PHVOLT_RAW_BUFFER_SIZE 256

#define PHVOLT_SCANNER_RETCODE_EOF -1
#define PHVOLT_SCANNER_RETCODE_ERR -2
#define PHVOLT_SCANNER_RETCODE_IMPOSSIBLE -3

/** Modes */
#define PHVOLT_MODE_RAW 0
#define PHVOLT_MODE_CODE 1
#define PHVOLT_MODE_COMMENT 2

#define PHVOLT_T_IGNORE 257

/* Literals & Identifiers */
#define PHVOLT_T_INTEGER 258
#define PHVOLT_T_DOUBLE 259
#define PHVOLT_T_STRING 260
#define PHVOLT_T_NULL 261
#define PHVOLT_T_FALSE 262
#define PHVOLT_T_TRUE 263
#define PHVOLT_T_IDENTIFIER 264

/* Operators */
#define PHVOLT_T_ADD 265
#define PHVOLT_T_SUB 266
#define PHVOLT_T_MUL 267
#define PHVOLT_T_DIV 268
#define PHVOLT_T_MOD 269
#define PHVOLT_T_AND 270
#define PHVOLT_T_OR 271
#define PHVOLT_T_CONCAT 272
#define PHVOLT_T_PIPE 273

#define PHVOLT_T_DOT 274
#define PHVOLT_T_COMMA 275

#define PHVOLT_T_NOT 276
#define PHVOLT_T_LESS 277
#define PHVOLT_T_LESSEQUAL 278
#define PHVOLT_T_GREATER 279
#define PHVOLT_T_GREATEREQUAL 280
#define PHVOLT_T_EQUALS 281
#define PHVOLT_T_NOTEQUALS 282
#define PHVOLT_T_IDENTICAL 283
#define PHVOLT_T_NOTIDENTICAL 284
#define PHVOLT_T_RANGE 285
#define PHVOLT_T_ASSIGN 286
#define PHVOLT_T_COLON 287
#define PHVOLT_T_QUESTION 288
#define PHVOLT_T_POW 289
#define PHVOLT_T_INCR 290
#define PHVOLT_T_DECR 291
#define PHVOLT_T_ADD_ASSIGN 292
#define PHVOLT_T_SUB_ASSIGN 293
#define PHVOLT_T_MUL_ASSIGN 294
#define PHVOLT_T_DIV_ASSIGN 295

#define PHVOLT_T_PARENTHESES_OPEN 296
#define PHVOLT_T_PARENTHESES_CLOSE 297
#define PHVOLT_T_SBRACKET_OPEN 298
#define PHVOLT_T_SBRACKET_CLOSE 299
#define PHVOLT_T_CBRACKET_OPEN 300
#define PHVOLT_T_CBRACKET_CLOSE 301

/** Reserved words */
#define PHVOLT_T_IF 302
#define PHVOLT_T_ELSE 303
#define PHVOLT_T_ELSEIF 304
#define PHVOLT_T_ENDIF 305
#define PHVOLT_T_FOR 306
#define PHVOLT_T_ENDFOR 307
#define PHVOLT_T_SET 308
#define PHVOLT_T_BLOCK 309
#define PHVOLT_T_ENDBLOCK 310
#define PHVOLT_T_IN 311
#define PHVOLT_T_EXTENDS 312
#define PHVOLT_T_IS 313
#define PHVOLT_T_DEFINED 314
#define PHVOLT_T_INCLUDE 315
#define PHVOLT_T_CACHE 316
#define PHVOLT_T_ENDCACHE 317
#define PHVOLT_T_DO 318
#define PHVOLT_T_AUTOESCAPE 319
#define PHVOLT_T_ENDAUTOESCAPE 320
#define PHVOLT_T_CONTINUE 321
#define PHVOLT_T_BREAK 322
#define PHVOLT_T_ELSEFOR 323
#define PHVOLT_T_MACRO 324
#define PHVOLT_T_ENDMACRO 325
#define PHVOLT_T_WITH 326
#define PHVOLT_T_CALL 327
#define PHVOLT_T_ENDCALL 328
#define PHVOLT_T_RETURN 329

/** Delimiters */
#define PHVOLT_T_OPEN_DELIMITER  340
#define PHVOLT_T_CLOSE_DELIMITER  341
#define PHVOLT_T_OPEN_EDELIMITER  342
#define PHVOLT_T_CLOSE_EDELIMITER  343

/** Special Tokens */
#define PHVOLT_T_FCALL 350
#define PHVOLT_T_EXPR 354
#define PHVOLT_T_QUALIFIED 355
#define PHVOLT_T_ENCLOSED 356
#define PHVOLT_T_RAW_FRAGMENT 357
#define PHVOLT_T_EMPTY_STATEMENT 358
#define PHVOLT_T_ECHO 359
#define PHVOLT_T_ARRAY 360
#define PHVOLT_T_ARRAYACCESS 361
#define PHVOLT_T_NOT_ISSET 362
#define PHVOLT_T_ISSET 363
#define PHVOLT_T_RESOLVED_EXPR 364
#define PHVOLT_T_SLICE 365
#define PHVOLT_T_TERNARY 366
#define PHVOLT_T_NOT_IN 367

#define PHVOLT_T_MINUS 368
#define PHVOLT_T_PLUS 369

#define PHVOLT_T_EMPTY 380
#define PHVOLT_T_EVEN 381
#define PHVOLT_T_ODD 382
#define PHVOLT_T_NUMERIC 383
#define PHVOLT_T_SCALAR 384
#define PHVOLT_T_ITERABLE 385

#define PHVOLT_T_ISEMPTY 386
#define PHVOLT_T_ISEVEN 387
#define PHVOLT_T_ISODD 388
#define PHVOLT_T_ISNUMERIC 389
#define PHVOLT_T_ISSCALAR 390
#define PHVOLT_T_ISITERABLE 391

#define PHVOLT_T_NOT_ISEMPTY 392
#define PHVOLT_T_NOT_ISEVEN 393
#define PHVOLT_T_NOT_ISODD 394
#define PHVOLT_T_NOT_ISNUMERIC 395
#define PHVOLT_T_NOT_ISSCALAR 396
#define PHVOLT_T_NOT_ISITERABLE 397

/* List of tokens and their names */
typedef struct _phvolt_token_names {
	char *name;
	int len;
	unsigned int code;
} phvolt_token_names;

/* Active token state */
typedef struct _phvolt_scanner_state {
	int active_token;
	int mode;
	char* start;
	char* end;
	unsigned int start_length;
	unsigned int active_line;
	zval *active_file;
	unsigned int statement_position;
	unsigned int extends_mode;
	unsigned int block_level;
	unsigned int macro_level;
	char *raw_buffer;
	unsigned int raw_buffer_cursor;
	unsigned int raw_buffer_size;
	unsigned int old_if_level;
	unsigned int if_level;
	unsigned int for_level;
	int whitespace_control;
} phvolt_scanner_state;

/* Extra information tokens */
typedef struct _phvolt_scanner_token {
	int opcode;
	int len;
	char *value;
} phvolt_scanner_token;

int phvolt_get_token(phvolt_scanner_state *s, phvolt_scanner_token *token);

extern const phvolt_token_names phvolt_tokens[];

#endif  /* PHALCON_MVC_VIEW_ENGINE_VOLT_SCANNER_H */

