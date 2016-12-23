
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
  +------------------------------------------------------------------------+
*/

#ifndef  PHALCON_CLI_OPT_PARSER_H
#define  PHALCON_CLI_OPT_PARSER_H

#include  <stdio.h>

typedef struct _phalcon_option_value* phalcon_option_value;
typedef struct _phalcon_option_value_ops* phalcon_option_value_ops;

struct _phalcon_option_value
{
    void* pointer;
    int required;
    int has_default;
    phalcon_option_value_ops ops;
};
struct _phalcon_option_value_ops
{
    char const* (*validate)(phalcon_option_value self, char const* arg);
    char const* (*parse)(phalcon_option_value self, char const* arg);
    char const* (*display_default)(phalcon_option_value self);
    void (*store_default)(phalcon_option_value self);
    void (*free)(phalcon_option_value self);
};

typedef struct __phalcon_optparser
{
    int argc;
    char const * const* argv;
    void* _private;
}* phalcon_optparser;

typedef struct _phalcon_option_cmd_chain* phalcon_option_cmd_chain;
struct _phalcon_option_cmd_chain
{
    phalcon_option_cmd_chain (*add)(char const* option_name, char const* help, void* pvalue);
    phalcon_option_cmd_chain (*text)(char const* txt);
    phalcon_option_cmd_chain (*group)(char const* group_name);
    phalcon_option_cmd_chain (*more_help)(char const* help_name, char const* help, void* pvalue, void(*printer)(void*), void* context);
    phalcon_option_cmd_chain (*help)(char const* help);
    void (*parse_into)(int argc, char const * const* argv, phalcon_optparser* pparser);
};
phalcon_option_cmd_chain phalcon_opt_init(char const* desc);
int phalcon_opt_has(phalcon_optparser parser, char const* key);
char const* phalcon_opt_get_arg(phalcon_optparser parser, char const* key);
void phalcon_opt_fprint(FILE*, phalcon_optparser parser);
void phalcon_opt_print(phalcon_optparser parser);
void phalcon_opt_free(phalcon_optparser);

// option value builders {{{
typedef struct _phalcon_opt_value_user_builder_interface* phalcon_opt_value_user_builder_interface;
struct _phalcon_opt_value_user_builder_interface
{
    phalcon_option_value _data;
    phalcon_opt_value_user_builder_interface (*required)();
    phalcon_opt_value_user_builder_interface (*context)(void* ctx);
    phalcon_opt_value_user_builder_interface (*validator)( char const* (*user_validate)(char const* arg, void* ctx) );
    phalcon_opt_value_user_builder_interface (*parser)(char const* (*user_parse)(char const* arg, void* out, void* ctxj));
    phalcon_opt_value_user_builder_interface (*default_store)(void (*copy)(void* default_value, void* out));
    phalcon_opt_value_user_builder_interface (*default_value)(void*);
    phalcon_opt_value_user_builder_interface (*default_display)(char const*);
    phalcon_opt_value_user_builder_interface (*free)(void (*ctx_free)(void*));
};
phalcon_opt_value_user_builder_interface phalcon_opt_arg(void* pointer);
//======================================================
typedef struct _phalcon_opt_int_builder_interface* phalcon_opt_int_builder_interface;
struct _phalcon_opt_int_builder_interface
{
    phalcon_option_value _data;
    phalcon_opt_int_builder_interface (*required)();
    phalcon_opt_int_builder_interface (*validator)(char const* (*)(int,void*));
    phalcon_opt_int_builder_interface (*default_value)(int);
    phalcon_opt_int_builder_interface (*base)(int);
    phalcon_opt_int_builder_interface (*context)(void*);
    phalcon_opt_int_builder_interface (*free)(void (*ctx)(void*));
};
phalcon_opt_int_builder_interface phalcon_opt_int(int* pointer);
//======================================================
typedef struct _phalcon_opt_double_builder_interface* phalcon_opt_double_builder_interface;
struct _phalcon_opt_double_builder_interface
{
    phalcon_option_value _data;
    phalcon_opt_double_builder_interface (*required)();
    phalcon_opt_double_builder_interface (*validator)(char const*(*)(double d, void*));
    phalcon_opt_double_builder_interface (*default_value)(double);
    phalcon_opt_double_builder_interface (*context)(void*);
    phalcon_opt_double_builder_interface (*free)(void(*)(void*));
};
phalcon_opt_double_builder_interface phalcon_opt_double(double* pointer);
//=====================================================
typedef struct _phalcon_opt_string_builder_interface* phalcon_opt_string_builder_interface;
struct _phalcon_opt_string_builder_interface
{
    phalcon_option_value _data;
    phalcon_opt_string_builder_interface (*required)();
    phalcon_opt_string_builder_interface (*default_value)(char const*);
};
phalcon_opt_string_builder_interface phalcon_opt_string(char const** p);
//=====================================================
typedef struct _phalcon_opt_ints_builder_interface* phalcon_opt_ints_builder_interface;
struct _phalcon_opt_ints_builder_interface
{
    phalcon_option_value _data;
    phalcon_opt_ints_builder_interface (*required)();
    phalcon_opt_ints_builder_interface (*default_value)(int const*, int);
    phalcon_opt_ints_builder_interface (*delimiters)(char const*);
    phalcon_opt_ints_builder_interface (*validator)(char const* (*)(int const*, int, void*));
    phalcon_opt_ints_builder_interface (*context)(void*);
    phalcon_opt_ints_builder_interface (*free)(void(*)(void*));
    phalcon_opt_ints_builder_interface (*base)(int n);
};
phalcon_opt_ints_builder_interface phalcon_opt_ints(int* pointer, int* n);
//=====================================================
typedef struct _phalcon_opt_doubles_builder_interface* phalcon_opt_doubles_builder_interface;
struct _phalcon_opt_doubles_builder_interface
{
    phalcon_option_value _data;
    phalcon_opt_doubles_builder_interface (*required)();
    phalcon_opt_doubles_builder_interface (*default_value)(double const*, int);
    phalcon_opt_doubles_builder_interface (*delimiters)(char const*);
    phalcon_opt_doubles_builder_interface (*validator)(char const* (*)(double const*, int, void*));
    phalcon_opt_doubles_builder_interface (*context)(void*);
    phalcon_opt_doubles_builder_interface (*free)(void(*)(void*));
};
phalcon_opt_doubles_builder_interface phalcon_opt_doubles(double* pointer, int* n);
//=====================================================
typedef struct _phalcon_opt_strings_builder_interface* phalcon_opt_strings_builder_interface;
struct _phalcon_opt_strings_builder_interface
{
    phalcon_option_value _data;
    phalcon_opt_strings_builder_interface (*required)();
    phalcon_opt_strings_builder_interface (*default_value)(char const*, ...);
    phalcon_opt_strings_builder_interface (*delimiters)(char const*);
};

phalcon_opt_strings_builder_interface phalcon_opt_strings(char const* pointer[], int* n);

#endif  /* PHALCON_CLI_OPT_PARSER_H */
