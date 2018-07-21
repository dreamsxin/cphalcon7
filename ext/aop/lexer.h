
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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  |          Julien Salleyron <julien.salleyron@gmail.com>                 |
  +------------------------------------------------------------------------+
*/

#ifndef _LEXER_H
# define _LEXER_H
#define SCANNER_RETCODE_EOF -1
#define SCANNER_RETCODE_ERR -2
#define SCANNER_RETCODE_IMPOSSIBLE -3

#define TOKEN_SPACE 0
#define TOKEN_FUNCTION 1
#define TOKEN_CLASS 2
#define TOKEN_JOKER 3
#define TOKEN_SUPER_JOKER 4
#define TOKEN_PROPERTY 5
#define TOKEN_SCOPE 6
#define TOKEN_STATIC 7
#define TOKEN_OR 8
#define TOKEN_TEXT 9


typedef struct _scanner_state {
	char *start;
	char *end;
	char *marker;
} scanner_state;

typedef struct _scanner_token {
    int TOKEN;
    char *str_val;
    int int_val;

} scanner_token;

int scan(scanner_state *state, scanner_token *token);
//TODO: some functions to manipulate scanner states

#endif
