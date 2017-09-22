
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

#ifndef PHALCON_MATRIX_H
#define PHALCON_MATRIX_H

#include "php_phalcon.h"

#define PHALCON_MATRIX_TYPE_DOUBLE		0
#define PHALCON_MATRIX_TYPE_LONG		1

typedef struct ZMatrix{
    int m; //i, rows
    int n; //j, columns
    zval *matrix;
} ZMatrix;

typedef struct DMatrix{
    int m; //i, rows
    int n; //j, columns
    double *matrix;
} DMatrix;

typedef struct LMatrix{
    int m; //i, rows
    int n; //j, columns
    long *matrix;
} LMatrix;

void phalcon_lmatrix_to_pzval(LMatrix, zval*);
void phalcon_dmatrix_to_pzval(LMatrix, zval*);

int phalcon_matrix_init_matrix(ZMatrix*, zval*);
void phalcon_matrix_free(ZMatrix);
void phalcon_matrix_free_matrix(ZMatrix);

int phalcon_matrix_init_matrix_double(DMatrix*, zval*);
void phalcon_matrix_free_double(DMatrix);

int phalcon_matrix_init_matrix_long(LMatrix*, zval*);
void phalcon_matrix_free_long(LMatrix);

void phalcon_matrix_elementwise_function(zval*, zval*, zval*, void (*f)(zval*, zval*, zval*));
void phalcon_matrix_scalar_matrix_function(zval*, double, zval*, double (*f)(double, double));
void phalcon_matrix_scalar_matrix_function_long(zval*, long, zval*, long (*f)(long, long));

void phalcon_array_add_sum_long(zval*, zval*, zval*);
void phalcon_array_add_sum_double(zval*, zval*, zval*);
void phalcon_array_add_mul_zvals_to_long(zval*, zval*, zval*);
void phalcon_array_add_mul_zvals_to_double(zval*, zval*, zval*);

double phalcon_matrix_mul_double(double, double);
long phalcon_matrix_mul_long(long, long);

extern zend_class_entry *phalcon_matrix_ce;

PHALCON_INIT_CLASS(Phalcon_Matrix);

#endif /* PHALCON_MATRIX_H */
