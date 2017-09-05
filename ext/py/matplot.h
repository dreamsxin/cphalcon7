
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
#ifndef PHALCON_PY_MATPLOT_H
#define PHALCON_PY_MATPLOT_H

#include "php_phalcon.h"

#include "py.h"

#if PHALCON_USE_PYTHON

typedef struct {
	PyObject *matplotlib;
	PyObject *pyplot;
	PyObject *pylab;

	PyObject *s_python_function_show;
	PyObject *s_python_function_close;
	PyObject *s_python_function_draw;
	PyObject *s_python_function_pause;
	PyObject *s_python_function_save;
	PyObject *s_python_function_figure;
	PyObject *s_python_function_plot;
	PyObject *s_python_function_fill_between;
	PyObject *s_python_function_hist;
	PyObject *s_python_function_subplot;
	PyObject *s_python_function_legend;
	PyObject *s_python_function_xlim;
	PyObject *s_python_function_ylim;
	PyObject *s_python_function_title;
	PyObject *s_python_function_axis;
	PyObject *s_python_function_xlabel;
	PyObject *s_python_function_ylabel;
	PyObject *s_python_function_grid;
	PyObject *s_python_function_clf;
	PyObject *s_python_function_errorbar;
	PyObject *s_python_function_annotate;
	PyObject *s_python_function_tight_layout;
	PyObject *s_python_empty_tuple;

	zend_object std;
} phalcon_py_matplot_object;

static inline phalcon_py_matplot_object *phalcon_py_matplot_object_from_obj(zend_object *obj) {
	return (phalcon_py_matplot_object*)((char*)(obj) - XtOffsetOf(phalcon_py_matplot_object, std));
}

#endif

extern zend_class_entry *phalcon_py_matplot_ce;

PHALCON_INIT_CLASS(Phalcon_Py_Matplot);

#endif	/* PHALCON_PY_MATPLOT_H */
