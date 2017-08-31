
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

#ifndef PHALCON_KERNEL_DEBUG_H
#define PHALCON_KERNEL_DEBUG_H

#include "php_phalcon.h"

#include <stdio.h>

#include <ext/standard/php_var.h>

#define PHV(v) phalcon_vdump(v)
#define PHPR(v) phalcon_print_r(v)
#define PHALCON_DEBUG_ZVAL(v) PHALCON_DEBUG_SIMPLE(); php_debug_zval_dump(v, 1)
#define PHALCON_DEBUG_SIMPLE() zend_printf("\nFile:%s, Line:%d\n", __FILE__, __LINE__)
#define PHALCON_DEBUG_SIMPLE_ZVAL(v) zend_printf("\nVar:%s, Refcount:%d, File:%s, Line:%d\n", #v, Z_REFCOUNT_P(v), __FILE__, __LINE__)

#define PHALCON_DEBUG_PRINT_ADDR(var) printf("Addr:  0x%p  \'"#var"\'\n",&(var))
#define PHALCON_DEBUG_PRINT_SIZE(var) printf("Size of \'"#var"\': %dByte\n",(int)sizeof(var))
#define PHALCON_DEBUG_PRINT_VALUE_F(var, format) printf("Value: "#format"  \'"#var"\'\n",var)
#define PHALCON_DEBUG_PRINT_VALUE(var) PHALCON_DEBUG_PRINT_VALUE_F(var, 0x%p)

typedef struct _phalcon_debug_entry {
	struct _phalcon_debug_entry *prev;
	struct _phalcon_debug_entry *next;
	char *class_name;
	char *method_name;
	int lineno;
} phalcon_debug_entry;

int phalcon_start_debug();
int phalcon_stop_debug();

int phalcon_print_r(zval *userval);
int phalcon_debug_print_r(zval *message);
int phalcon_vdump(zval *uservar);
int phalcon_debug_assign(char *name, zval *value);
int phalcon_vpdump(const zval **uservar);
int phalcon_dump_ce(zend_class_entry *ce);
int phalcon_class_debug(zval *val);

int phalcon_debug_backtrace_internal();
int phalcon_debug_str(char *what, char *message);
int phalcon_debug_long(char *what, uint vlong);
int phalcon_debug_screen(char *message);

int phalcon_step_over(char *message);
int phalcon_step_into(char *message);
int phalcon_step_out(char *message);

int phalcon_step_over_zval(zval *message);
int phalcon_step_into_zval(zval *message);
int phalcon_step_out_zval(zval *message);

int phalcon_step_into_entry(char *class_name, char *method_name, int lineno);
int phalcon_step_out_entry();

int phalcon_debug_method_call(zval *obj, char *method_name);
int phalcon_debug_vdump(char *preffix, zval *value);
int phalcon_debug_param(zval *param);

int phalcon_error_space();
int phalcon_debug_space();

extern FILE *phalcon_log;
extern int phalcon_debug_trace;
extern phalcon_debug_entry *start;
extern phalcon_debug_entry *active;

#endif /* PHALCON_KERNEL_DEBUG_H */
