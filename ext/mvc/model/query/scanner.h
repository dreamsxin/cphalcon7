
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

#ifndef PHALCON_MVC_MODEL_QUERY_SCANNER_H
#define PHALCON_MVC_MODEL_QUERY_SCANNER_H

#define PHQL_SCANNER_RETCODE_EOF -1
#define PHQL_SCANNER_RETCODE_ERR -2
#define PHQL_SCANNER_RETCODE_IMPOSSIBLE -3

#define PHQL_T_IGNORE 257

/* Literals & Identifiers */
#define PHQL_T_INTEGER 258
#define PHQL_T_DOUBLE 259
#define PHQL_T_STRING 260
#define PHQL_T_HINTEGER 261
#define PHQL_T_IDENTIFIER 265

/* Operators */
#define PHQL_T_ADD 266
#define PHQL_T_SUB 267
#define PHQL_T_MUL 268
#define PHQL_T_DIV 269
#define PHQL_T_MOD 270
#define PHQL_T_BITWISE_AND 271
#define PHQL_T_BITWISE_OR 272
#define PHQL_T_BITWISE_XOR 273
#define PHQL_T_BITWISE_NOT 274
#define PHQL_T_AND 275
#define PHQL_T_OR 276
#define PHQL_T_LIKE 277
#define PHQL_T_ILIKE 278
#define PHQL_T_AGAINST 279

#define PHQL_T_DOT 280
#define PHQL_T_COMMA 281
#define PHQL_T_COLON 282

#define PHQL_T_EQUALS 283
#define PHQL_T_NOTEQUALS 284
#define PHQL_T_NOT 285
#define PHQL_T_LESS 286
#define PHQL_T_LESSEQUAL 287
#define PHQL_T_GREATER 288
#define PHQL_T_GREATEREQUAL 289

#define PHQL_T_PARENTHESES_OPEN 290
#define PHQL_T_PARENTHESES_CLOSE 291

/** Placeholders */
#define PHQL_T_NPLACEHOLDER 292
#define PHQL_T_SPLACEHOLDER 293
#define PHQL_T_BPLACEHOLDER 294

/** Reserved words */
#define PHQL_T_UPDATE 300
#define PHQL_T_SET 301
#define PHQL_T_WHERE 302
#define PHQL_T_DELETE 303
#define PHQL_T_FROM 304
#define PHQL_T_AS 305
#define PHQL_T_INSERT 306
#define PHQL_T_INTO 307
#define PHQL_T_VALUES 308
#define PHQL_T_SELECT 309
#define PHQL_T_ORDER 310
#define PHQL_T_BY 311
#define PHQL_T_LIMIT 312
#define PHQL_T_GROUP 313
#define PHQL_T_HAVING 314
#define PHQL_T_IN 315
#define PHQL_T_ON 316
#define PHQL_T_INNER 317
#define PHQL_T_JOIN 318
#define PHQL_T_LEFT 319
#define PHQL_T_RIGHT 320
#define PHQL_T_IS 321
#define PHQL_T_NULL 322
#define PHQL_T_NOTIN 323
#define PHQL_T_CROSS 324
#define PHQL_T_FULL 325
#define PHQL_T_OUTER 326
#define PHQL_T_ASC 327
#define PHQL_T_DESC 328
#define PHQL_T_OFFSET 329
#define PHQL_T_DISTINCT 330
#define PHQL_T_BETWEEN 331
#define PHQL_T_CAST 332
#define PHQL_T_TRUE 333
#define PHQL_T_FALSE 334
#define PHQL_T_CONVERT 335
#define PHQL_T_USING 336
#define PHQL_T_ALL 337
#define PHQL_T_FOR 338

/** Special Tokens */
#define PHQL_T_FCALL 350
#define PHQL_T_NLIKE 351
#define PHQL_T_STARALL 352
#define PHQL_T_DOMAINALL 353
#define PHQL_T_EXPR 354
#define PHQL_T_QUALIFIED 355
#define PHQL_T_ENCLOSED 356
#define PHQL_T_NILIKE 357
#define PHQL_T_RAW_QUALIFIED 358

#define PHQL_T_INNERJOIN 360
#define PHQL_T_LEFTJOIN 361
#define PHQL_T_RIGHTJOIN 362
#define PHQL_T_CROSSJOIN 363
#define PHQL_T_FULLJOIN 364
#define PHQL_T_ISNULL 365
#define PHQL_T_ISNOTNULL 366
#define PHQL_T_MINUS 367

/** Postgresql Text Search Operators */
#define PHQL_T_TS_MATCHES 401
#define PHQL_T_TS_OR 402
#define PHQL_T_TS_AND 403
#define PHQL_T_TS_NEGATE 404
#define PHQL_T_TS_CONTAINS_ANOTHER 405
#define PHQL_T_TS_CONTAINS_IN 406

#define PHQL_T_SUBQUERY 407
#define PHQL_T_EXISTS 408

#define PHQL_T_CASE 409
#define PHQL_T_WHEN 410
#define PHQL_T_ELSE 411
#define PHQL_T_END 412
#define PHQL_T_THEN 413
#define PHQL_T_WITH 415
#define PHQL_T_FORCE 416
#define PHQL_T_USE 417
#define PHQL_T_INDEX 418
#define PHQL_T_DOUBLECOLON 419

/* list of tokens and their names */
typedef struct _phql_token_names {
	char *name;
	unsigned int length;
	unsigned int code;
} phql_token_names;

/* active token state */
typedef struct _phql_scanner_state {
	int active_token;
	unsigned int start_length;
	char* start;
	char* end;
} phql_scanner_state;

/* extra information tokens */
typedef struct _phql_scanner_token {
	char *value;
	int opcode;
	unsigned int len;
} phql_scanner_token;

int phql_get_token(phql_scanner_state *s, phql_scanner_token *token);

extern const phql_token_names phql_tokens[];

#endif /* PHALCON_MVC_MODEL_QUERY_SCANNER_H */
