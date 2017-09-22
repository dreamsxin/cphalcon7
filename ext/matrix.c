
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework													  |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)	   |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled	 |
  | with this package in the file docs/LICENSE.txt.						|
  |																		|
  | If you did not receive a copy of the license and are unable to		 |
  | obtain it through the world-wide-web, please send an email			 |
  | to license@phalconphp.com so we can send you a copy immediately.	   |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>					  |
  |		  Eduar Carvajal <eduar@phalconphp.com>						 |
  |		  ZhuZongXin <dreamsxin@qq.com>								 |
  +------------------------------------------------------------------------+
*/

#include "matrix.h"
#include "exception.h"

#include <ext/standard/php_array.h>
#include <ext/spl/spl_array.h>

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/exception.h"

/**
 * Phalcon\Matrix
 *
 */
zend_class_entry *phalcon_matrix_ce;

PHP_METHOD(Phalcon_Matrix, valid);
PHP_METHOD(Phalcon_Matrix, dump);
PHP_METHOD(Phalcon_Matrix, transpose);
PHP_METHOD(Phalcon_Matrix, add);
PHP_METHOD(Phalcon_Matrix, mul);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_matrix_valid, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, matrix, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_matrix_dump, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, matrix, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_matrix_transpose, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, matrix, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_matrix_add, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, matrix, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, matrix2)
	ZEND_ARG_TYPE_INFO(0, elementwise, _IS_BOOL, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_matrix_mul, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, matrix, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, matrix2)
	ZEND_ARG_TYPE_INFO(0, elementwise, _IS_BOOL, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_matrix_method_entry[] = {
	PHP_ME(Phalcon_Matrix, valid, arginfo_phalcon_matrix_valid, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Matrix, dump, arginfo_phalcon_matrix_dump, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Matrix, transpose, arginfo_phalcon_matrix_transpose, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Matrix, add, arginfo_phalcon_matrix_add, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Matrix, mul, arginfo_phalcon_matrix_mul, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Matrix initializer
 */
PHALCON_INIT_CLASS(Phalcon_Matrix){

	PHALCON_REGISTER_CLASS(Phalcon, Matrix, matrix, phalcon_matrix_method_entry, 0);

	zend_declare_class_constant_long(phalcon_matrix_ce, SL("TYPE_DOUBLE"),		PHALCON_MATRIX_TYPE_DOUBLE);
	zend_declare_class_constant_long(phalcon_matrix_ce, SL("TYPE_LONG"),		PHALCON_MATRIX_TYPE_LONG);

	return SUCCESS;
}

int phalcon_matrix_init_matrix(ZMatrix* out, zval* phalcon_matrix)
{
	zval *data, *data2;
	int n, m, i, j, idx, i_mem;
	HashTable *j_hash, *i_hash;
	HashPosition pointer, pointer2;
	zval *matrix;

	out->n = 0;
	out->m = 0;
	out->matrix = NULL;

	if (Z_TYPE_P(phalcon_matrix) != IS_ARRAY)
	{
		return FAILURE;
	}

	i_hash = Z_ARRVAL_P(phalcon_matrix);
	zend_hash_internal_pointer_reset_ex(i_hash, &pointer);
	m = zend_hash_num_elements(i_hash);
	if ((data = zend_hash_get_current_data_ex(i_hash, &pointer)) != NULL)
	{
		if (Z_TYPE_P(data) != IS_ARRAY) {
			return FAILURE;
		}
		j_hash = Z_ARRVAL_P(data);
		n = zend_hash_num_elements(j_hash);
	}
	else
	{
		return FAILURE;
	}
	matrix = emalloc(sizeof(zval) * m * n);

	idx = 0;
	for(
		i = 0, zend_hash_internal_pointer_reset_ex(i_hash, &pointer);
		i < m;
		++i, zend_hash_move_forward_ex(i_hash, &pointer)
	)
	{
		if (
			(data = zend_hash_get_current_data_ex(i_hash, &pointer)) == NULL
			||
			Z_TYPE_P(data) != IS_ARRAY
		) {
			for(i_mem = 0; i_mem < idx; ++i_mem)
			{
				zval_ptr_dtor(&matrix[i_mem]);
			}
			efree(matrix);
			return FAILURE;
		}

		j_hash = Z_ARRVAL_P(data);
		if (zend_hash_num_elements(j_hash) != n) {
			for(i_mem = 0; i_mem < idx; ++i_mem)
			{
				zval_ptr_dtor(&matrix[i_mem]);
			}
			efree(matrix);
			return FAILURE;
		}

		for(
			j = 0, zend_hash_internal_pointer_reset_ex(j_hash, &pointer2);
			j < n;
			++j, zend_hash_move_forward_ex(j_hash, &pointer2)
		)
		{
			if ((data2 = zend_hash_get_current_data_ex(j_hash, &pointer2)) == NULL) {
				for(i_mem = 0; i_mem < idx; ++i_mem)
				{
					zval_ptr_dtor(&matrix[i_mem]);
				}
				efree(matrix);
				return FAILURE;
			}
			ZVAL_COPY_VALUE(&matrix[idx], data2);

			++idx;
		}
	}

	out->n = n;
	out->m = m;
	out->matrix = matrix;

	return SUCCESS;
}

int phalcon_matrix_init_matrix_double(DMatrix* out, zval* phalcon_matrix)
{
	zval *data, *data2;
	int n, m, i, j, idx;
	HashTable *j_hash, *i_hash;
	HashPosition pointer, pointer2;
	double *matrix;

	zval temp;

	out->n = 0;
	out->m = 0;
	out->matrix = NULL;

	if (Z_TYPE_P(phalcon_matrix) != IS_ARRAY)
	{
		return FAILURE;
	}

	i_hash = Z_ARRVAL_P(phalcon_matrix);
	zend_hash_internal_pointer_reset_ex(i_hash, &pointer);
	m = zend_hash_num_elements(i_hash);
	if ((data = zend_hash_get_current_data_ex(i_hash, &pointer)) != NULL)
	{
		if (Z_TYPE_P(data) != IS_ARRAY)
		{
			//PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, each row of data types must be an array (%d)", __LINE__);
			return FAILURE;
		}
		j_hash = Z_ARRVAL_P(data);
		n = zend_hash_num_elements(j_hash);
	}
	else
	{
		return FAILURE;
	}
	matrix = emalloc(sizeof(*(out->matrix)) * m * n);

	idx = 0;
	for(
		i = 0, zend_hash_internal_pointer_reset_ex(i_hash, &pointer);
		i < m;
		++i, zend_hash_move_forward_ex(i_hash, &pointer)
	)
	{
		if
		(
			(data = zend_hash_get_current_data_ex(i_hash, &pointer)) == NULL
			||
			Z_TYPE_P(data) != IS_ARRAY
		)
		{
			efree(matrix);
			return FAILURE;
		}

		j_hash = Z_ARRVAL_P(data);
		if (zend_hash_num_elements(j_hash) != n)
		{
			//PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, the length of each row must be consistent (%d)", __LINE__);
			efree(matrix);
			return FAILURE;
		}

		for(
			j = 0, zend_hash_internal_pointer_reset_ex(j_hash, &pointer2);
			j < n;
			++j, zend_hash_move_forward_ex(j_hash, &pointer2)
		)
		{
			if ((data2 = zend_hash_get_current_data_ex(j_hash, &pointer2)) == NULL)
			{
				//PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "Geet current data error (%d)", __LINE__);
				efree(matrix);
				return FAILURE;
			}

			ZVAL_DUP(&temp, data2);
			convert_to_double(&temp);
			matrix[idx] = Z_DVAL(temp);
			zval_dtor(&temp);

			++idx;
		}
	}

	out->n = n;
	out->m = m;
	out->matrix = matrix;

	return SUCCESS;
}

void php_dmatrix_to_pzval(DMatrix matrix, zval* ret)
{
	int i, j, idx = 0;

	array_init(ret);
	for(i = 0; i < matrix.m; ++i)
	{
		zval row = {};
		array_init(&row);
		for(j = 0; j < matrix.n; ++j)
		{
			add_next_index_double(&row, matrix.matrix[idx]);
			++idx;
		}
		add_next_index_zval(ret, &row);
	}
}

int phalcon_matrix_init_matrix_long(LMatrix* out, zval* phalcon_matrix)
{
	zval *data, *data2;
	int n, m, i, j;
	HashTable *j_hash, *i_hash;
	HashPosition pointer, pointer2;
	long *matrix;

	out->n = 0;
	out->m = 0;
	out->matrix = NULL;

	if (Z_TYPE_P(phalcon_matrix) != IS_ARRAY)
	{
		return FAILURE;
	}

	i_hash = Z_ARRVAL_P(phalcon_matrix);
	zend_hash_internal_pointer_reset_ex(i_hash, &pointer);
	m = zend_hash_num_elements(i_hash);
	if ((data =zend_hash_get_current_data_ex(i_hash, &pointer)) != NULL)
	{
		if (Z_TYPE_P(data) != IS_ARRAY)
		{
			//PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, each row of data types must be an array (%d)", __LINE__);
			return FAILURE;
		}
		j_hash = Z_ARRVAL_P(data);
		n = zend_hash_num_elements(j_hash);
	}
	else
	{
		return FAILURE;
	}
	matrix = emalloc(sizeof(*(out->matrix)) * m * n);

	int idx = 0;
	for(
		i = 0, zend_hash_internal_pointer_reset_ex(i_hash, &pointer);
		i < m;
		++i, zend_hash_move_forward_ex(i_hash, &pointer)
	)
	{
		if
		(
			(data =zend_hash_get_current_data_ex(i_hash, &pointer)) == NULL
			||
			Z_TYPE_P(data) != IS_ARRAY
		)
		{
			efree(matrix);
			return FAILURE;
		}

		j_hash = Z_ARRVAL_P(data);
		if (zend_hash_num_elements(j_hash) != n)
		{
			efree(matrix);
			return FAILURE;
		}

		for(
			j = 0, zend_hash_internal_pointer_reset_ex(j_hash, &pointer2);
			j < n;
			++j, zend_hash_move_forward_ex(j_hash, &pointer2)
		)
		{
			zval temp = {};
			if ((data2 = zend_hash_get_current_data_ex(j_hash, &pointer2)) == NULL) {
				//PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, the length of each row must be consistent (%d)", __LINE__);
				efree(matrix);
				return FAILURE;
			}
			
			ZVAL_DUP(&temp, data2);
			convert_to_long(&temp);
			matrix[idx] = Z_LVAL(temp);
			zval_dtor(&temp);

			++idx;
		}
	}

	out->n = n;
	out->m = m;
	out->matrix = matrix;

	return SUCCESS;
}

void phalcon_lmatrix_to_pzval(LMatrix matrix, zval* ret)
{
	int i, j, idx = 0;

	array_init(ret);
	for(i = 0; i < matrix.m; ++i)
	{
		zval row = {};
		array_init(&row);
		for(j = 0; j < matrix.n; ++j)
		{
			add_next_index_long(&row, matrix.matrix[idx]);
			++idx;
		}
		add_next_index_zval(ret, &row);
	}
}

void phalcon_matrix_free(ZMatrix matrix)
{
	int eCnt, idx;
	eCnt = matrix.m * matrix.n;
	for(idx = 0; idx < eCnt; ++idx)
	{
		zval_ptr_dtor(&matrix.matrix[idx]);
	}
	efree(matrix.matrix);
}

void phalcon_matrix_free_matrix(ZMatrix matrix)
{
	efree(matrix.matrix);
}

void phalcon_matrix_free_double(DMatrix matrix)
{
	efree(matrix.matrix);
}

void phalcon_matrix_free_long(LMatrix matrix)
{
	efree(matrix.matrix);
}

void phalcon_matrix_elementwise_function(zval *return_value, zval *arg_matrix1, zval *arg_matrix2, void (*f)(zval*, zval*, zval*))
{
	zval *data1_1, *data1_2;
	zval *data2_1, *data2_2;

	HashTable *j1_hash, *i1_hash;
	HashTable *j2_hash, *i2_hash;
	HashPosition pointer1_1, pointer1_2;
	HashPosition pointer2_1, pointer2_2;
	int i, j, m, n;

	i1_hash = Z_ARRVAL_P(arg_matrix1);
	i2_hash = Z_ARRVAL_P(arg_matrix2);
	m = zend_hash_num_elements(i1_hash);
	if (zend_hash_num_elements(i2_hash) != m) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "The matrix1 rows must be equal to matrix2 rows (%d)", __LINE__);
		RETURN_NULL();
	}

	zend_hash_internal_pointer_reset_ex(i1_hash, &pointer1_1);
	if ((data1_1 = zend_hash_get_current_data_ex(i1_hash, &pointer1_1)) != NULL)
	{
		if (Z_TYPE_P(data1_1) != IS_ARRAY)
		{
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, each row of data types must be an array (%d)", __LINE__);
			RETURN_NULL();
		}
		j1_hash = Z_ARRVAL_P(data1_1);
		n = zend_hash_num_elements(j1_hash);
	}
	else
	{
		RETURN_NULL();
	}

	array_init(return_value);
	for(
		i = 0, zend_hash_internal_pointer_reset_ex(i1_hash, &pointer1_1), zend_hash_internal_pointer_reset_ex(i2_hash, &pointer2_1);
		i < m;
		++i, zend_hash_move_forward_ex(i1_hash, &pointer1_1), zend_hash_move_forward_ex(i2_hash, &pointer2_1)
	)
	{
		zval row = {};
		if
		(
			(data1_1 = zend_hash_get_current_data_ex(i1_hash, &pointer1_1)) == NULL
			||
			Z_TYPE_P(data1_1) != IS_ARRAY
			||
			(data2_1 = zend_hash_get_current_data_ex(i2_hash, &pointer2_1)) == NULL
			||
			Z_TYPE_P(data2_1) != IS_ARRAY
		)
		{
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, each row of data types must be an array (%d)", __LINE__);
			zval_dtor(return_value);
			RETURN_NULL();
		}

		j1_hash = Z_ARRVAL_P(data1_1);
		j2_hash = Z_ARRVAL_P(data2_1);
		if (zend_hash_num_elements(j1_hash) != n || zend_hash_num_elements(j2_hash) != n)
		{
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, the length of each row must be consistent (%d)", __LINE__);
			zval_dtor(return_value);
			RETURN_NULL();
		}

		array_init(&row);
		for(
			j = 0, zend_hash_internal_pointer_reset_ex(j1_hash, &pointer1_2), zend_hash_internal_pointer_reset_ex(j2_hash, &pointer2_2);
			j < n;
			++j, zend_hash_move_forward_ex(j1_hash, &pointer1_2), zend_hash_move_forward_ex(j2_hash, &pointer2_2)
		)
		{
			if ((data1_2 = zend_hash_get_current_data_ex(j1_hash, &pointer1_2)) == NULL)
			{
				/*
				zval_dtor(&row);
				zval_dtor(return_value);
				RETURN_NULL();
				*/
				data1_2 = &PHALCON_GLOBAL(z_zero);
			}
			if ((data2_2 = zend_hash_get_current_data_ex(j2_hash, &pointer2_2)) == NULL)
			{
				/*
				zval_dtor(&row);
				zval_dtor(return_value);
				RETURN_NULL();
				*/
				data2_2 = &PHALCON_GLOBAL(z_zero);
			}

			f(&row, data1_2, data2_2);
		}
		add_next_index_zval(return_value, &row);
	}
}

void phalcon_matrix_scalar_matrix_function(zval *return_value, double arg_scalar, zval *arg_matrix, double (*f)(double, double))
{
	zval *data1_1, *data1_2;

	HashTable *j1_hash, *i1_hash;
	HashPosition pointer1_1, pointer1_2;
	int i, j, m, n;

	double a;

	i1_hash = Z_ARRVAL_P(arg_matrix);
	m = zend_hash_num_elements(i1_hash);

	zend_hash_internal_pointer_reset_ex(i1_hash, &pointer1_1);
	if ((data1_1 = zend_hash_get_current_data_ex(i1_hash, &pointer1_1)) != NULL)
	{
		if (Z_TYPE_P(data1_1) != IS_ARRAY)
		{
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, each row of data types must be an array (%d)", __LINE__);
			RETURN_NULL();
		}
		j1_hash = Z_ARRVAL_P(data1_1);
		n = zend_hash_num_elements(j1_hash);
	}
	else
	{
		RETURN_NULL();
	}

	array_init(return_value);
	for(
		i = 0, zend_hash_internal_pointer_reset_ex(i1_hash, &pointer1_1);
		i < m;
		++i, zend_hash_move_forward_ex(i1_hash, &pointer1_1)
	)
	{
		zval row = {}, temp = {};
		if
		(
			(data1_1 = zend_hash_get_current_data_ex(i1_hash, &pointer1_1)) == NULL
			||
			Z_TYPE_P(data1_1) != IS_ARRAY
		)
		{
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, each row of data types must be an array (%d)", __LINE__);
			zval_dtor(return_value);
			RETURN_NULL();
		}

		j1_hash = Z_ARRVAL_P(data1_1);
		if (zend_hash_num_elements(j1_hash) != n)
		{
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, the length of each row must be consistent (%d)", __LINE__);
			zval_dtor(return_value);
			RETURN_NULL();
		}

		array_init(&row);
		for(
			j = 0, zend_hash_internal_pointer_reset_ex(j1_hash, &pointer1_2);
			j < n;
			++j, zend_hash_move_forward_ex(j1_hash, &pointer1_2)
		)
		{
			if ((data1_2 = zend_hash_get_current_data_ex(j1_hash, &pointer1_2)) == NULL) {
				/*
				zval_dtor(&row);
				zval_dtor(return_value);
				RETURN_NULL();
				*/
				add_next_index_double(&row, f(arg_scalar, 0.00));
			} else {
				ZVAL_DUP(&temp, data1_2);
				convert_to_double(&temp);
				a = Z_DVAL(temp);
				zval_dtor(&temp);

				add_next_index_double(&row, f(arg_scalar, a));
			}
		}
		add_next_index_zval(return_value, &row);
	}
}

void phalcon_matrix_scalar_matrix_function_long(zval *return_value, long arg_scalar, zval *arg_matrix, long (*f)(long, long))
{
	zval *data1_1, *data1_2;

	HashTable *j1_hash, *i1_hash;
	HashPosition pointer1_1, pointer1_2;
	int i, j, m, n;

	long a;

	i1_hash = Z_ARRVAL_P(arg_matrix);
	m = zend_hash_num_elements(i1_hash);

	zend_hash_internal_pointer_reset_ex(i1_hash, &pointer1_1);
	if ((data1_1 = zend_hash_get_current_data_ex(i1_hash, &pointer1_1)) != NULL)
	{
		if (Z_TYPE_P(data1_1) != IS_ARRAY)
		{
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, each row of data types must be an array (%d)", __LINE__);
			RETURN_NULL();
		}
		j1_hash = Z_ARRVAL_P(data1_1);
		n = zend_hash_num_elements(j1_hash);
	}
	else
	{
		RETURN_NULL();
	}

	array_init(return_value);
	for(
		i = 0, zend_hash_internal_pointer_reset_ex(i1_hash, &pointer1_1);
		i < m;
		++i, zend_hash_move_forward_ex(i1_hash, &pointer1_1)
	)
	{
		zval row = {}, temp = {};
		if
		(
			(data1_1 = zend_hash_get_current_data_ex(i1_hash, &pointer1_1)) == NULL
			||
			Z_TYPE_P(data1_1) != IS_ARRAY
		)
		{
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, each row of data types must be an array (%d)", __LINE__);
			zval_dtor(return_value);
			RETURN_NULL();
		}

		j1_hash = Z_ARRVAL_P(data1_1);
		if (zend_hash_num_elements(j1_hash) != n)
		{
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_exception_ce, "In a matrix, the length of each row must be consistent (%d)", __LINE__);
			zval_dtor(return_value);
			RETURN_NULL();
		}

		array_init(&row);
		for(
			j = 0, zend_hash_internal_pointer_reset_ex(j1_hash, &pointer1_2);
			j < n;
			++j, zend_hash_move_forward_ex(j1_hash, &pointer1_2)
		)
		{
			if ((data1_2 = zend_hash_get_current_data_ex(j1_hash, &pointer1_2)) == NULL) {
				/*
				zval_dtor(&row);
				zval_dtor(return_value);
				RETURN_NULL();
				*/
				add_next_index_long(&row, f(arg_scalar, 0));
			} else {
				ZVAL_DUP(&temp, data1_2);
				convert_to_long(&temp);
				a = Z_LVAL(temp);
				zval_dtor(&temp);

				add_next_index_long(&row, f(arg_scalar, a));
			}
		}
		add_next_index_zval(return_value, &row);
	}
}

double phalcon_matrix_mul_double(double x1, double x2)
{
	return x1 * x2;
}

long phalcon_matrix_mul_long(long x1, long x2)
{
	return x1 * x2;
}

/**
 * 
 *
 * @param array $matrix
 * @return boolean
 */
PHP_METHOD(Phalcon_Matrix, valid){

	zval *matrix;
	ZMatrix m;

	phalcon_fetch_params(0, 1, 0, &matrix);

	if (phalcon_matrix_init_matrix(&m, matrix) == FAILURE) {
		RETURN_FALSE;
	}

	phalcon_matrix_free(m);
	RETURN_TRUE;
}

/**
 * 
 *
 * @param array $matrix
 */
PHP_METHOD(Phalcon_Matrix, dump){

	zval *matrix;
	ZMatrix m;
	int i, j, idx=0;

	phalcon_fetch_params(0, 1, 0, &matrix);

	if (phalcon_matrix_init_matrix(&m, matrix) == FAILURE) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix format error (%d)");
		return;
	}

	php_printf("matrix[%dx%d]:\n", m.m, m.n);

	for(i = 0; i < m.m; ++i)
	{
		php_printf("|\t");

		for(j = 0; j < m.n; ++j)
		{
			switch(Z_TYPE(m.matrix[idx]))
			{
				case IS_LONG:
					php_printf("%ld\t", Z_LVAL(m.matrix[idx]));
					break;
				case IS_DOUBLE:
					php_printf("%.3f\t", Z_DVAL(m.matrix[idx]));
					break;
				case IS_STRING:
					php_printf("\"%s\"\t", Z_STRVAL(m.matrix[idx]));
					break;
				default:
					php_printf("<undef>\t");
			}
			++idx;
		}

		php_printf("|\n");
	}

	phalcon_matrix_free(m);
}

/**
 * 
 *
 * @param array $matrix
 * @return array
 */
PHP_METHOD(Phalcon_Matrix, transpose){

	zval *matrix, row = {};
	ZMatrix m;
	int i, j;

	phalcon_fetch_params(0, 1, 0, &matrix);

	if (phalcon_matrix_init_matrix(&m, matrix) == FAILURE) {
		RETURN_FALSE;
	}

	array_init(return_value);
	for(j = 0; j < m.n; ++j)
	{
		array_init(&row);
		for(i = 0; i < m.m; ++i)
		{
			add_next_index_zval(&row, &m.matrix[i*m.n + j]);
		}
		add_next_index_zval(return_value, &row);
	}
	phalcon_matrix_free_matrix(m);
}

void phalcon_array_add_sum_long(zval *row, zval *data1, zval *data2)
{
	long x1, x2;
	zval temp;

	ZVAL_DUP(&temp, data1);
	convert_to_long(&temp);
	x1 = Z_LVAL(temp);
	zval_dtor(&temp);

	ZVAL_DUP(&temp, data2);
	convert_to_long(&temp);
	x2 = Z_LVAL(temp);
	zval_dtor(&temp);

	add_next_index_long(row, x1 + x2);
}

void phalcon_array_add_sum_double(zval *row, zval *data1, zval *data2)
{
	double x1, x2;
	zval temp;

	ZVAL_DUP(&temp, data1);
	convert_to_double(&temp);
	x1 = Z_DVAL(temp);
	zval_dtor(&temp);

	ZVAL_DUP(&temp, data2);
	convert_to_double(&temp);
	x2 = Z_DVAL(temp);
	zval_dtor(&temp);

	add_next_index_double(row, x1 + x2);
}



double phalcon_matrix_add_double(double x1, double x2)
{
	return x1 + x2;
}

long phalcon_matrix_add_long(long x1, long x2)
{
	return x1 + x2;
}

/**
 * 
 *
 * @param array $matrix
 * @param array $matrix2
 * @return array
 */
PHP_METHOD(Phalcon_Matrix, add){

	zval *matrix, *matrix2, *elementwise = NULL, *type = NULL;
	int t = PHALCON_MATRIX_TYPE_DOUBLE;

	phalcon_fetch_params(0, 2, 2, &matrix, &matrix2, &elementwise, &type);

	if (!elementwise) {
		elementwise = &PHALCON_GLOBAL(z_false);
	}

	if (type && Z_TYPE_P(type) == IS_LONG) {
		t = Z_LVAL_P(type);
	}

	if (zend_is_true(elementwise)) {
		if (Z_TYPE_P(matrix2) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "When use element-wise the matrix2 must be array");
			return;
		}
		if (t == PHALCON_MATRIX_TYPE_LONG) {
			phalcon_matrix_elementwise_function(return_value, matrix, matrix2, &phalcon_array_add_sum_long);
		} else {
			phalcon_matrix_elementwise_function(return_value, matrix, matrix2, &phalcon_array_add_sum_double);
		}
		return;
	}

	if (Z_TYPE_P(matrix2) != IS_ARRAY) {
		if (Z_TYPE_P(matrix2) == IS_LONG) {
			phalcon_matrix_scalar_matrix_function_long(return_value, Z_LVAL_P(matrix2), matrix, &phalcon_matrix_add_long);
		} else if (Z_TYPE_P(matrix2) == IS_DOUBLE) {
			phalcon_matrix_scalar_matrix_function(return_value, Z_DVAL_P(matrix2), matrix, &phalcon_matrix_add_double);
		} else {
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix2 must be array, int or float");
			return;
		}

		return;
	}

	if (t == PHALCON_MATRIX_TYPE_LONG) {
		LMatrix a, b;
		long elem;

		int m, n, p, i, j, r;
		int a_idx, a_idx_start, b_idx;
		if (phalcon_matrix_init_matrix_long(&a, matrix) != SUCCESS) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix elements must be int");
			RETURN_NULL();
		}

		if (phalcon_matrix_init_matrix_long(&b, matrix2) != SUCCESS) {
			phalcon_matrix_free_long(a);
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix2 elements must be int");
			RETURN_NULL();
		}

		if (a.n != b.m) {
			phalcon_matrix_free_long(a);
			phalcon_matrix_free_long(b);
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix1 columns must be equal to matrix2 rows");
			RETURN_NULL();
		}

		/*** all correct now ***/
		m = a.m;
		n = a.n;
		p = b.n;

		array_init(return_value);
		for(
			i = a_idx_start = 0;
			i < m;
			++i, a_idx_start += n
		)
		{
			zval row = {};
			array_init(&row);
			for(j = 0; j < p; ++j)
			{
				for(
					r = 0, elem = 0, a_idx = a_idx_start, b_idx = j;
					r < n;
					++r, ++a_idx, b_idx += p
				)
				{
					elem = elem + a.matrix[a_idx] + b.matrix[b_idx];
				}
				add_next_index_long(&row, elem);
			}
			add_next_index_zval(return_value, &row);
		}

		phalcon_matrix_free_long(a);
		phalcon_matrix_free_long(b);
	} else {
		DMatrix a, b;
		double elem;

		int m, n, p, i, j, r;
		int a_idx, a_idx_start, b_idx;

		if (phalcon_matrix_init_matrix_double(&a, matrix) != SUCCESS) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix elements must be float");
			RETURN_NULL();
		}

		if (phalcon_matrix_init_matrix_double(&b, matrix2) != SUCCESS) {
			phalcon_matrix_free_double(a);
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix2 elements must be float");
			RETURN_NULL();
		}

		if (a.n != b.m) {
			phalcon_matrix_free_double(a);
			phalcon_matrix_free_double(b);
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix1 columns must be equal to matrix2 rows");
			RETURN_NULL();
		}

		/*** all correct now ***/
		m = a.m;
		n = a.n;
		p = b.n;

		array_init(return_value);
		for(
			i = a_idx_start = 0;
			i < m;
			++i, a_idx_start += n
		)
		{
			zval row = {};
			array_init(&row);
			for(j = 0; j < p; ++j)
			{
				for(
					r = 0, elem = 0, a_idx = a_idx_start, b_idx = j;
					r < n;
					++r, ++a_idx, b_idx += p
				)
				{
					elem = elem + a.matrix[a_idx] + b.matrix[b_idx];
				}
				add_next_index_double(&row, elem);
			}
			add_next_index_zval(return_value, &row);
		}

		phalcon_matrix_free_double(a);
		phalcon_matrix_free_double(b);
	}
}

void phalcon_array_add_mul_zvals_to_long(zval *row, zval *data1, zval *data2)
{
	long x1, x2;
	zval temp;

	ZVAL_DUP(&temp, data1);
	convert_to_long(&temp);
	x1 = Z_LVAL(temp);
	zval_dtor(&temp);

	ZVAL_DUP(&temp, data2);
	convert_to_long(&temp);
	x2 = Z_LVAL(temp);
	zval_dtor(&temp);

	add_next_index_long(row, x1 * x2);
}

void phalcon_array_add_mul_zvals_to_double(zval *row, zval *data1, zval *data2)
{
	double x1, x2;
	zval temp = {};

	ZVAL_DUP(&temp, data1);
	convert_to_double(&temp);
	x1 = Z_DVAL(temp);
	zval_dtor(&temp);

	ZVAL_DUP(&temp, data2);
	convert_to_double(&temp);
	x2 = Z_DVAL(temp);
	zval_dtor(&temp);

	add_next_index_double(row, x1 * x2);
}

/**
 * 
 *
 * @param array $matrix
 * @param mixed $matrix2
 * @return array
 */
PHP_METHOD(Phalcon_Matrix, mul){

	zval *matrix, *matrix2, *elementwise = NULL, *type = NULL;
	int t = PHALCON_MATRIX_TYPE_DOUBLE;

	phalcon_fetch_params(0, 2, 2, &matrix, &matrix2, &elementwise, &type);

	if (!elementwise) {
		elementwise = &PHALCON_GLOBAL(z_false);
	}

	if (type && Z_TYPE_P(type) == IS_LONG) {
		t = Z_LVAL_P(type);
	}

	if (zend_is_true(elementwise)) {
		if (Z_TYPE_P(matrix2) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "When use element-wise the matrix2 must be array");
			return;
		}
		if (t == PHALCON_MATRIX_TYPE_LONG) {
			phalcon_matrix_elementwise_function(return_value, matrix, matrix2, &phalcon_array_add_mul_zvals_to_long);
		} else {
			phalcon_matrix_elementwise_function(return_value, matrix, matrix2, &phalcon_array_add_mul_zvals_to_double);
		}
		return;
	}

	if (Z_TYPE_P(matrix2) != IS_ARRAY) {
		if (Z_TYPE_P(matrix2) == IS_LONG) {
			phalcon_matrix_scalar_matrix_function_long(return_value, Z_LVAL_P(matrix2), matrix, &phalcon_matrix_mul_long);
		} else if (Z_TYPE_P(matrix2) == IS_DOUBLE) {
			phalcon_matrix_scalar_matrix_function(return_value, Z_DVAL_P(matrix2), matrix, &phalcon_matrix_mul_double);
		} else {
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix2 must be array, int or float");
			return;
		}

		return;
	}

	if (t == PHALCON_MATRIX_TYPE_LONG) {
		LMatrix a, b;
		long elem;

		int m, n, p, i, j, r;
		int a_idx, a_idx_start, b_idx;
		if (phalcon_matrix_init_matrix_long(&a, matrix) != SUCCESS) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix elements must be int");
			RETURN_NULL();
		}

		if (phalcon_matrix_init_matrix_long(&b, matrix2) != SUCCESS) {
			phalcon_matrix_free_long(a);
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix2 elements must be int");
			RETURN_NULL();
		}

		if (a.n != b.m) {
			phalcon_matrix_free_long(a);
			phalcon_matrix_free_long(b);
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix1 columns must be equal to matrix2 rows");
			RETURN_NULL();
		}

		/*** all correct now ***/
		m = a.m;
		n = a.n;
		p = b.n;

		array_init(return_value);
		for(
			i = a_idx_start = 0;
			i < m;
			++i, a_idx_start += n
		)
		{
			zval row = {};
			array_init(&row);
			for(j = 0; j < p; ++j)
			{
				for(
					r = 0, elem = 0, a_idx = a_idx_start, b_idx = j;
					r < n;
					++r, ++a_idx, b_idx += p
				)
				{
					elem = elem + a.matrix[a_idx] * b.matrix[b_idx];
				}
				add_next_index_long(&row, elem);
			}
			add_next_index_zval(return_value, &row);
		}

		phalcon_matrix_free_long(a);
		phalcon_matrix_free_long(b);
	} else {
		DMatrix a, b;
		double elem;

		int m, n, p, i, j, r;
		int a_idx, a_idx_start, b_idx;

		if (phalcon_matrix_init_matrix_double(&a, matrix) != SUCCESS) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix elements must be float");
			RETURN_NULL();
		}

		if (phalcon_matrix_init_matrix_double(&b, matrix2) != SUCCESS) {
			phalcon_matrix_free_double(a);
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix2 elements must be float");
			RETURN_NULL();
		}

		if (a.n != b.m) {
			phalcon_matrix_free_double(a);
			phalcon_matrix_free_double(b);
			PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "The matrix1 columns must be equal to matrix2 rows");
			RETURN_NULL();
		}

		/*** all correct now ***/
		m = a.m;
		n = a.n;
		p = b.n;

		array_init(return_value);
		for(
			i = a_idx_start = 0;
			i < m;
			++i, a_idx_start += n
		)
		{
			zval row = {};
			array_init(&row);
			for(j = 0; j < p; ++j)
			{
				for(
					r = 0, elem = 0, a_idx = a_idx_start, b_idx = j;
					r < n;
					++r, ++a_idx, b_idx += p
				)
				{
					elem = elem + a.matrix[a_idx] * b.matrix[b_idx];
				}
				add_next_index_double(&row, elem);
			}
			add_next_index_zval(return_value, &row);
		}

		phalcon_matrix_free_double(a);
		phalcon_matrix_free_double(b);
	}
}
