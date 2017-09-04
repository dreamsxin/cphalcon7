/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2013 Phalcon Team (http://www.phalconphp.com)       |
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

#include "py/matplot.h"
#include "py/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/output.h"

/**
 * Phalcon\Py\Matplot
 *
 */
zend_class_entry *phalcon_py_matplot_ce;

PHP_METHOD(Phalcon_Py_Matplot, backend);
PHP_METHOD(Phalcon_Py_Matplot, annotate);
PHP_METHOD(Phalcon_Py_Matplot, plot);
PHP_METHOD(Phalcon_Py_Matplot, fillBetween);
PHP_METHOD(Phalcon_Py_Matplot, hist);
PHP_METHOD(Phalcon_Py_Matplot, errorbar);
PHP_METHOD(Phalcon_Py_Matplot, figure);
PHP_METHOD(Phalcon_Py_Matplot, legend);
PHP_METHOD(Phalcon_Py_Matplot, ylim);
PHP_METHOD(Phalcon_Py_Matplot, getYlim);
PHP_METHOD(Phalcon_Py_Matplot, xlim);
PHP_METHOD(Phalcon_Py_Matplot, getXlim);
PHP_METHOD(Phalcon_Py_Matplot, subplot);
PHP_METHOD(Phalcon_Py_Matplot, title);
PHP_METHOD(Phalcon_Py_Matplot, axis);
PHP_METHOD(Phalcon_Py_Matplot, xlabel);
PHP_METHOD(Phalcon_Py_Matplot, ylabel);
PHP_METHOD(Phalcon_Py_Matplot, grid);
PHP_METHOD(Phalcon_Py_Matplot, show);
PHP_METHOD(Phalcon_Py_Matplot, draw);
PHP_METHOD(Phalcon_Py_Matplot, pause);
PHP_METHOD(Phalcon_Py_Matplot, save);
PHP_METHOD(Phalcon_Py_Matplot, clf);
PHP_METHOD(Phalcon_Py_Matplot, tightLayout);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_backend, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, backend, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_annotate, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, annotation, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_plot, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_fillbetween, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, x, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, y1, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, y2, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, keywords, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_hist, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_errorbar, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, x, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, yerr, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_ylim, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, left, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, right, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_xlim, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, left, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, right, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_title, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, title, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_axis, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, axis, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_xlabel, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, xlabel, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_ylabel, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, ylabel, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_grid, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, flag, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_pause, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, interval, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_save, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_py_matplot_method_entry[] = {
	PHP_ME(Phalcon_Py_Matplot, backend, arginfo_phalcon_py_matplot_backend, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Py_Matplot, annotate, arginfo_phalcon_py_matplot_annotate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, plot, arginfo_phalcon_py_matplot_plot, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, fillBetween, arginfo_phalcon_py_matplot_fillbetween, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, hist, arginfo_phalcon_py_matplot_hist, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, errorbar, arginfo_phalcon_py_matplot_errorbar, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, figure, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, legend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, ylim, arginfo_phalcon_py_matplot_ylim, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, getYlim, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, xlim, arginfo_phalcon_py_matplot_xlim, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, getXlim, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, subplot, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, title, arginfo_phalcon_py_matplot_title, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, axis, arginfo_phalcon_py_matplot_axis, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, xlabel, arginfo_phalcon_py_matplot_xlabel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, ylabel, arginfo_phalcon_py_matplot_ylabel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, grid, arginfo_phalcon_py_matplot_grid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, show, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, draw, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, pause, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, clf, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, tightLayout, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Py\Matplot initializer
 */
PHALCON_INIT_CLASS(Phalcon_Py_Matplot){

	PHALCON_REGISTER_CLASS(Phalcon\\Py, Matplot, py_matplot, phalcon_py_matplot_method_entry, 0);

	return SUCCESS;
}

PHP_METHOD(Phalcon_Py_Matplot, backend){

}

PHP_METHOD(Phalcon_Py_Matplot, annotate){

}

PHP_METHOD(Phalcon_Py_Matplot, plot){

}

PHP_METHOD(Phalcon_Py_Matplot, fillBetween){

}

PHP_METHOD(Phalcon_Py_Matplot, hist){

}

PHP_METHOD(Phalcon_Py_Matplot, errorbar){

}

PHP_METHOD(Phalcon_Py_Matplot, figure){

}

PHP_METHOD(Phalcon_Py_Matplot, legend){

}

PHP_METHOD(Phalcon_Py_Matplot, ylim){

}

PHP_METHOD(Phalcon_Py_Matplot, getYlim){

}

PHP_METHOD(Phalcon_Py_Matplot, xlim){

}

PHP_METHOD(Phalcon_Py_Matplot, getXlim){

}

PHP_METHOD(Phalcon_Py_Matplot, subplot){

}

PHP_METHOD(Phalcon_Py_Matplot, title){

}

PHP_METHOD(Phalcon_Py_Matplot, axis){

}

PHP_METHOD(Phalcon_Py_Matplot, xlabel){

}

PHP_METHOD(Phalcon_Py_Matplot, ylabel){

}

PHP_METHOD(Phalcon_Py_Matplot, grid){

}

PHP_METHOD(Phalcon_Py_Matplot, show){

}

PHP_METHOD(Phalcon_Py_Matplot, draw){

}

PHP_METHOD(Phalcon_Py_Matplot, pause){

}

PHP_METHOD(Phalcon_Py_Matplot, clf){

}

PHP_METHOD(Phalcon_Py_Matplot, tightLayout){

}
