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

PHP_METHOD(Phalcon_Py_Matplot, factory);
PHP_METHOD(Phalcon_Py_Matplot, __construct);
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
PHP_METHOD(Phalcon_Py_Matplot, close);
PHP_METHOD(Phalcon_Py_Matplot, draw);
PHP_METHOD(Phalcon_Py_Matplot, pause);
PHP_METHOD(Phalcon_Py_Matplot, save);
PHP_METHOD(Phalcon_Py_Matplot, clf);
PHP_METHOD(Phalcon_Py_Matplot, tightLayout);
PHP_METHOD(Phalcon_Py_Matplot, __call);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_factory, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, backend, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, backend, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_annotate, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, annotation, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_plot, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, x, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, style, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_fillbetween, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, x, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, y1, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, y2, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, keywords, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_hist, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, y, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, bins, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, color, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, alpha, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_errorbar, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, x, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, yerr, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_ylim, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, left, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, right, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot_xlim, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, left, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, right, IS_DOUBLE, 0)
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_matplot___call, 0, 0, 1)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_py_matplot_method_entry[] = {
	PHP_ME(Phalcon_Py_Matplot, factory, arginfo_phalcon_py_matplot_factory, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py_Matplot, __construct, arginfo_phalcon_py_matplot___construct, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
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
	PHP_ME(Phalcon_Py_Matplot, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, draw, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, pause, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, save, arginfo_phalcon_py_matplot_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, clf, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, tightLayout, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Py_Matplot, __call, arginfo_phalcon_py_matplot___call, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_py_matplot_object_handlers;
zend_object* phalcon_py_matplot_object_create_handler(zend_class_entry *ce)
{
	phalcon_py_matplot_object *intern = ecalloc(1, sizeof(phalcon_py_matplot_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_py_matplot_object_handlers;

	return &intern->std;
}

void phalcon_py_matplot_object_free_handler(zend_object *object)
{
	zend_object_std_dtor(object);
}

/**
 * Phalcon\Py\Matplot initializer
 */
PHALCON_INIT_CLASS(Phalcon_Py_Matplot){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Py, Matplot, py_matplot, phalcon_py_matplot_method_entry, 0);

	zend_declare_property_null(phalcon_py_matplot_ce, SL("_instance"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	return SUCCESS;
}

/**
 *
 * @return Phalcon\Py\Matplot
 **/
PHP_METHOD(Phalcon_Py_Matplot, factory)
{
	zval *backend = NULL, instance = {};

	phalcon_fetch_params(0, 0, 1, &backend);

	if (!backend) {
		backend = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_static_property_ce(&instance, phalcon_py_matplot_ce, SL("_instance"), PH_READONLY);
	if (Z_TYPE(instance) == IS_NULL) {
		object_init_ex(return_value, phalcon_py_matplot_ce);
		PHALCON_CALL_METHOD(NULL, return_value, "__construct", backend);

		phalcon_update_static_property_ce(phalcon_py_matplot_ce, SL("_instance"), return_value);
	} else {
		RETURN_CTOR(&instance);
	}
}

/**
 * Phalcon\Py\Matplot constructor
 *
 */
PHP_METHOD(Phalcon_Py_Matplot, __construct)
{
	zval *backend = NULL;
	phalcon_py_matplot_object *intern;

	phalcon_fetch_params(0, 0, 1, &backend);

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	PHP_PYTHON_THREAD_ACQUIRE();

	intern->matplotlib = PyImport_ImportModule("matplotlib");
	if (!intern->matplotlib) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Error loading module matplotlib!");		
		PHP_PYTHON_THREAD_RELEASE();
		return;
	}
	Py_DECREF(intern->matplotlib);

	// matplotlib.use() must be called *before* pylab, matplotlib.pyplot, or matplotlib.backends is imported for the first time
	if (backend && Z_TYPE_P(backend) == IS_STRING) {
		PyObject_CallMethod(intern->matplotlib, "use", Z_STRVAL_P(backend));
	}

	intern->pyplot = PyImport_ImportModule("matplotlib.pyplot");
	if (!intern->pyplot) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Error loading module matplotlib.pyplot!");		
		PHP_PYTHON_THREAD_RELEASE();
		return;
	}
	Py_DECREF(intern->pyplot);

	intern->pylab = PyImport_ImportModule("pylab");
	if (!intern->pylab) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Error loading module pylab!");		
		PHP_PYTHON_THREAD_RELEASE();
		return;
	}
	Py_DECREF(intern->pylab);

	intern->s_python_function_show = PyObject_GetAttrString(intern->pyplot, "show");
	intern->s_python_function_close = PyObject_GetAttrString(intern->pyplot, "close");
	intern->s_python_function_draw = PyObject_GetAttrString(intern->pyplot, "draw");
	intern->s_python_function_pause = PyObject_GetAttrString(intern->pyplot, "pause");
	intern->s_python_function_figure = PyObject_GetAttrString(intern->pyplot, "figure");
	intern->s_python_function_plot = PyObject_GetAttrString(intern->pyplot, "plot");
	intern->s_python_function_fill_between = PyObject_GetAttrString(intern->pyplot, "fill_between");
	intern->s_python_function_hist = PyObject_GetAttrString(intern->pyplot,"hist");
	intern->s_python_function_subplot = PyObject_GetAttrString(intern->pyplot, "subplot");
	intern->s_python_function_legend = PyObject_GetAttrString(intern->pyplot, "legend");
	intern->s_python_function_ylim = PyObject_GetAttrString(intern->pyplot, "ylim");
	intern->s_python_function_title = PyObject_GetAttrString(intern->pyplot, "title");
	intern->s_python_function_axis = PyObject_GetAttrString(intern->pyplot, "axis");
	intern->s_python_function_xlabel = PyObject_GetAttrString(intern->pyplot, "xlabel");
	intern->s_python_function_ylabel = PyObject_GetAttrString(intern->pyplot, "ylabel");
	intern->s_python_function_grid = PyObject_GetAttrString(intern->pyplot, "grid");
	intern->s_python_function_xlim = PyObject_GetAttrString(intern->pyplot, "xlim");
	intern->s_python_function_save = PyObject_GetAttrString(intern->pylab, "savefig");
	intern->s_python_function_annotate = PyObject_GetAttrString(intern->pyplot,"annotate");
	intern->s_python_function_clf = PyObject_GetAttrString(intern->pyplot, "clf");
	intern->s_python_function_errorbar = PyObject_GetAttrString(intern->pyplot, "errorbar");
	intern->s_python_function_tight_layout = PyObject_GetAttrString(intern->pyplot, "tight_layout");

	if(!intern->s_python_function_show
		|| !intern->s_python_function_close
		|| !intern->s_python_function_draw
		|| !intern->s_python_function_pause
		|| !intern->s_python_function_figure
		|| !intern->s_python_function_plot
		|| !intern->s_python_function_fill_between
		|| !intern->s_python_function_subplot
		|| !intern->s_python_function_legend
		|| !intern->s_python_function_ylim
		|| !intern->s_python_function_title
		|| !intern->s_python_function_axis
		|| !intern->s_python_function_xlabel
		|| !intern->s_python_function_ylabel
		|| !intern->s_python_function_grid
		|| !intern->s_python_function_xlim
		|| !intern->s_python_function_save
		|| !intern->s_python_function_clf
		|| !intern->s_python_function_annotate
		|| !intern->s_python_function_errorbar
		|| !intern->s_python_function_errorbar
		|| !intern->s_python_function_tight_layout
	) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Couldn't find required function!");		
		PHP_PYTHON_THREAD_RELEASE();
		return;
	}

	if (!PyFunction_Check(intern->s_python_function_show)
		|| !PyFunction_Check(intern->s_python_function_close)
		|| !PyFunction_Check(intern->s_python_function_draw)
		|| !PyFunction_Check(intern->s_python_function_pause)
		|| !PyFunction_Check(intern->s_python_function_figure)
		|| !PyFunction_Check(intern->s_python_function_plot)
		|| !PyFunction_Check(intern->s_python_function_fill_between)
		|| !PyFunction_Check(intern->s_python_function_subplot)
		|| !PyFunction_Check(intern->s_python_function_legend)
		|| !PyFunction_Check(intern->s_python_function_annotate)
		|| !PyFunction_Check(intern->s_python_function_ylim)
		|| !PyFunction_Check(intern->s_python_function_title)
		|| !PyFunction_Check(intern->s_python_function_axis)
		|| !PyFunction_Check(intern->s_python_function_xlabel)
		|| !PyFunction_Check(intern->s_python_function_ylabel)
		|| !PyFunction_Check(intern->s_python_function_grid)
		|| !PyFunction_Check(intern->s_python_function_xlim)
		|| !PyFunction_Check(intern->s_python_function_save)
		|| !PyFunction_Check(intern->s_python_function_clf)
		|| !PyFunction_Check(intern->s_python_function_tight_layout)
		|| !PyFunction_Check(intern->s_python_function_errorbar)
	) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Python object is unexpectedly not a PyFunction!");		
		PHP_PYTHON_THREAD_RELEASE();
		return;
	}

	intern->s_python_empty_tuple = PyTuple_New(0);

	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, annotate){

	zval *annotation, *x, *y;
	phalcon_py_matplot_object *intern;
	PyObject *pyxy, *pystr, *kwargs, *args, *res;

	phalcon_fetch_params(0, 3, 0, &annotation, &x, &y);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	pyxy = PyTuple_New(2);
	PyTuple_SetItem(pyxy, 0, PyFloat_FromDouble(Z_DVAL_P(x)));
	PyTuple_SetItem(pyxy, 1, PyFloat_FromDouble(Z_DVAL_P(y)));

	kwargs = PyDict_New();
	PyDict_SetItemString(kwargs, "xy", pyxy);

	pystr = PyString_FromString(Z_STRVAL_P(annotation));

	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, pystr);

	res = PyObject_Call(intern->s_python_function_annotate, args, kwargs);
	
	Py_DECREF(args);
	Py_DECREF(kwargs);

	if(res) {
		Py_DECREF(res);
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}

	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, plot){

	zval *_x, *_y = NULL, *format = NULL, *style = NULL, *label = NULL;
	phalcon_py_matplot_object *intern;
	PyObject *xarray, *yarray, *pystring, *plot_args, *kwargs, *res;
	int size = 0;

	phalcon_fetch_params(0, 1, 4, &_x, &_y, &format, &style, &label);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	size = zend_hash_num_elements(Z_ARRVAL_P(_x));

	if (!_y || Z_TYPE_P(_y) == IS_NULL) {
		int i;
		xarray = PyList_New(size);
		for (i = 0; i < size; i++) {
			PyList_SetItem(xarray, (Py_ssize_t)i, PyInt_FromLong(i));
		}
		yarray = pip_hash_to_list(_x);
	} else {
		xarray = pip_hash_to_list(_x);
		yarray = pip_hash_to_list(_y);
	}

	if (format && Z_TYPE_P(format) == IS_STRING) {
		pystring = PyString_FromString(Z_STRVAL_P(format));
	} else {
		pystring = PyString_FromString("");
	}

	plot_args = PyTuple_New(3);
	PyTuple_SetItem(plot_args, 0, xarray);
	PyTuple_SetItem(plot_args, 1, yarray);
	PyTuple_SetItem(plot_args, 2, pystring);

	kwargs = PyDict_New();
	if (style && Z_TYPE_P(style) == IS_STRING) {
		PyDict_SetItemString(kwargs, "style", PyString_FromString(Z_STRVAL_P(style)));
	}
	if (label && Z_TYPE_P(label) == IS_STRING) {
		PyDict_SetItemString(kwargs, "label", PyString_FromString(Z_STRVAL_P(label)));
	}

	res = PyObject_Call(intern->s_python_function_plot, plot_args, kwargs);

	Py_DECREF(plot_args);
	Py_DECREF(kwargs);
	if(res) {
		pip_pyobject_to_zval(res, return_value);
		Py_DECREF(res);
	}

	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, fillBetween){

	zval *x, *y1, *y2, *keywords;
	phalcon_py_matplot_object *intern;
	PyObject *xarray, *y1array, *y2array, *args, *kwargs, *res;

	phalcon_fetch_params(0, 4, 0, &x, &y1, &y2, &keywords);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	// using numpy arrays
	xarray = pip_hash_to_list(x);
	y1array = pip_hash_to_list(y1);
	y2array = pip_hash_to_list(y2);

	// construct positional args
	args = PyTuple_New(3);
	PyTuple_SetItem(args, 0, xarray);
	PyTuple_SetItem(args, 1, y1array);
	PyTuple_SetItem(args, 2, y2array);

	// construct keyword args
	kwargs = pip_hash_to_dict(keywords);

	res = PyObject_Call(intern->s_python_function_fill_between, args, kwargs);

	Py_DECREF(args);
	Py_DECREF(kwargs);
	if(res) {
		Py_DECREF(res);
	}

	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, hist){

	zval *y, *bins = NULL, *color = NULL, *alpha = NULL, *label = NULL;
	phalcon_py_matplot_object *intern;
	PyObject *yarray, *plot_args, *kwargs, *res;

	phalcon_fetch_params(0, 1, 4, &y, &bins, &color, &alpha, &label);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	yarray = pip_hash_to_list(y);

	plot_args = PyTuple_New(1);
	PyTuple_SetItem(plot_args, 0, yarray);

	kwargs = PyDict_New();
	if (bins && Z_TYPE_P(bins) == IS_LONG) {
		PyDict_SetItemString(kwargs, "bins", PyLong_FromLong(Z_LVAL_P(bins)));
	} else {
		PyDict_SetItemString(kwargs, "bins", PyLong_FromLong(10));
	}
	if (color && Z_TYPE_P(color) == IS_STRING) {
		PyDict_SetItemString(kwargs, "color", PyString_FromString(Z_STRVAL_P(color)));
	} else {
		PyDict_SetItemString(kwargs, "color", PyString_FromString("b"));
	}
	if (alpha && Z_TYPE_P(alpha) == IS_DOUBLE) {
		PyDict_SetItemString(kwargs, "alpha", PyFloat_FromDouble(Z_DVAL_P(alpha)));
	} else {
		PyDict_SetItemString(kwargs, "alpha", PyFloat_FromDouble(1.0));
	}
	if (label && Z_TYPE_P(label) == IS_STRING) {
		PyDict_SetItemString(kwargs, "label", PyString_FromString(Z_STRVAL_P(label)));
	}

	res = PyObject_Call(intern->s_python_function_hist, plot_args, kwargs);

	Py_DECREF(plot_args);
	Py_DECREF(kwargs);

	if(res) {
		Py_DECREF(res);
	}

	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, errorbar){

	zval *x, *y, *yerr;
	phalcon_py_matplot_object *intern;
	PyObject *xarray, *yarray, *yerrarray, *plot_args, *kwargs, *res;

	phalcon_fetch_params(0, 3, 0, &x, &y, &yerr);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	xarray = pip_hash_to_list(x);
	yarray = pip_hash_to_list(y);
	yerrarray = pip_hash_to_list(yerr);

	plot_args = PyTuple_New(2);
	PyTuple_SetItem(plot_args, 0, xarray);
	PyTuple_SetItem(plot_args, 1, yarray);

	kwargs = PyDict_New();
	PyDict_SetItemString(kwargs, "yerr", yerrarray);

	res = PyObject_Call(intern->s_python_function_errorbar, plot_args, kwargs);

	Py_DECREF(kwargs);
	Py_DECREF(plot_args);

	if (res) {
		Py_DECREF(res);
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to errorbar() failed.");		
		PHP_PYTHON_THREAD_RELEASE();
	}

	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, figure){

	phalcon_py_matplot_object *intern;
	PyObject *res;

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));
	
	res = PyObject_CallObject(intern->s_python_function_figure, intern->s_python_empty_tuple);
	if(!res) {		
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to figure() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, legend){

	phalcon_py_matplot_object *intern;
	PyObject *res;

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));
	
	res = PyObject_CallObject(intern->s_python_function_legend, intern->s_python_empty_tuple);
	if(!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to legend() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, ylim){

	zval *left, *right;
	phalcon_py_matplot_object *intern;
	PyObject *list, *args, *res;

	phalcon_fetch_params(0, 2, 0, &left, &right);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	list = PyList_New(2);
	PyList_SetItem(list, 0, PyFloat_FromDouble(Z_DVAL_P(left)));
	PyList_SetItem(list, 1, PyFloat_FromDouble(Z_DVAL_P(right)));

	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, list);

	res = PyObject_CallObject(intern->s_python_function_ylim, args);
	Py_DECREF(args);

	if(!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to ylim() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, getYlim){

	phalcon_py_matplot_object *intern;
	PyObject *args, *res, *left, *right;

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	args = PyTuple_New(0);
	res = PyObject_CallObject(intern->s_python_function_ylim, args);
	Py_DECREF(args);

	if(!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to getylim() failed.");
		return;
	}

	left = PyTuple_GetItem(res,0);
	right = PyTuple_GetItem(res,1);

	array_init(return_value);
	phalcon_array_append_double(return_value, PyFloat_AsDouble(left));
	phalcon_array_append_double(return_value, PyFloat_AsDouble(right));

	Py_DECREF(res);

	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, xlim){

	zval *left, *right;
	phalcon_py_matplot_object *intern;
	PyObject *list, *args, *res;

	phalcon_fetch_params(0, 2, 0, &left, &right);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	list = PyList_New(2);
	PyList_SetItem(list, 0, PyFloat_FromDouble(Z_DVAL_P(left)));
	PyList_SetItem(list, 1, PyFloat_FromDouble(Z_DVAL_P(right)));

	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, list);

	res = PyObject_CallObject(intern->s_python_function_xlim, args);
	Py_DECREF(args);

	if(!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to xlim() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, getXlim){

	phalcon_py_matplot_object *intern;
	PyObject *args, *res, *left, *right;

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	args = PyTuple_New(0);
	res = PyObject_CallObject(intern->s_python_function_xlim, args);
	Py_DECREF(args);

	if(!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to getxlim() failed.");
		return;
	}

	left = PyTuple_GetItem(res,0);
	right = PyTuple_GetItem(res,1);

	array_init(return_value);
	phalcon_array_append_double(return_value, PyFloat_AsDouble(left));
	phalcon_array_append_double(return_value, PyFloat_AsDouble(right));

	Py_DECREF(res);

	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, subplot){

	zval *nrows, *ncols, *plot_number;
	phalcon_py_matplot_object *intern;
	PyObject *args, *res;

	phalcon_fetch_params(0, 3, 0, &nrows, &ncols, &plot_number);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	args = PyTuple_New(3);
	PyTuple_SetItem(args, 0, PyFloat_FromDouble(Z_DVAL_P(nrows)));
	PyTuple_SetItem(args, 1, PyFloat_FromDouble(Z_DVAL_P(ncols)));
	PyTuple_SetItem(args, 2, PyFloat_FromDouble(Z_DVAL_P(plot_number)));

	res = PyObject_CallObject(intern->s_python_function_subplot, args);
	Py_DECREF(args);

	if(!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to subplot() failed.");
		return;
	}

	Py_DECREF(res);

	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, title){

	zval *title;
	phalcon_py_matplot_object *intern;
	PyObject *pytitle, *args, *res;

	phalcon_fetch_params(0, 1, 0, &title);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));
	pytitle = PyString_FromString(Z_STRVAL_P(title));

	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, pytitle);

	res = PyObject_CallObject(intern->s_python_function_title, args);
	Py_DECREF(args);
	if (!res) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to title() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
	RETURN_TRUE;

}

PHP_METHOD(Phalcon_Py_Matplot, axis){

	zval *axis;
	phalcon_py_matplot_object *intern;
	PyObject *pyaxis, *args, *res;

	phalcon_fetch_params(0, 1, 0, &axis);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	pyaxis = PyString_FromString(Z_STRVAL_P(axis));
	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, pyaxis);

	res = PyObject_CallObject(intern->s_python_function_axis, args);
	if (!res) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to axis() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Py_Matplot, xlabel){

	zval *xlabel;
	phalcon_py_matplot_object *intern;
	PyObject *pyxlabel, *args, *res;

	phalcon_fetch_params(0, 1, 0, &xlabel);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	pyxlabel = PyString_FromString(Z_STRVAL_P(xlabel));
	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, pyxlabel);

	res = PyObject_CallObject(intern->s_python_function_xlabel, args);
	if (!res) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to xlabel() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Py_Matplot, ylabel){

	zval *ylabel;
	phalcon_py_matplot_object *intern;
	PyObject *pyylabel, *args, *res;

	phalcon_fetch_params(0, 1, 0, &ylabel);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	pyylabel = PyString_FromString(Z_STRVAL_P(ylabel));
	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, pyylabel);

	res = PyObject_CallObject(intern->s_python_function_ylabel, args);
	if (!res) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to ylabel() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Py_Matplot, grid){

	zval *flag;
	phalcon_py_matplot_object *intern;
	PyObject *args, *res;

	phalcon_fetch_params(0, 1, 0, &flag);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, zend_is_true(flag) ? Py_True : Py_False);

	res = PyObject_CallObject(intern->s_python_function_grid, args);
	Py_DECREF(args);
	if (!res) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to grid() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Py_Matplot, show){

	phalcon_py_matplot_object *intern;
	PyObject *res;

	PHP_PYTHON_THREAD_ACQUIRE();
	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));
	res = PyObject_CallObject(intern->s_python_function_show, intern->s_python_empty_tuple);

	if (!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to show() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, close){

	phalcon_py_matplot_object *intern;
	PyObject *res;

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	res = PyObject_CallObject(intern->s_python_function_close, intern->s_python_empty_tuple);

	if (!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to close() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, draw){

	phalcon_py_matplot_object *intern;
	PyObject *res;

	PHP_PYTHON_THREAD_ACQUIRE();
	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));
	res = PyObject_CallObject(intern->s_python_function_draw, intern->s_python_empty_tuple);

	if (!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to draw() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, pause){

	zval *interval;
	phalcon_py_matplot_object *intern;
	PyObject *args, *res;

	phalcon_fetch_params(0, 1, 0, &interval);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, PyFloat_FromDouble(Z_DVAL_P(interval)));

	res = PyObject_CallObject(intern->s_python_function_pause, args);
	Py_DECREF(args);

	if (!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to pause() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, save){

	zval *filename;
	phalcon_py_matplot_object *intern;
	PyObject *pyfilename, *args, *res;

	phalcon_fetch_params(0, 1, 0, &filename);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));
	pyfilename = PyString_FromString(Z_STRVAL_P(filename));

	args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, pyfilename);

	res = PyObject_CallObject(intern->s_python_function_save, args);
	Py_DECREF(args);
	if (!res) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to save() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Py_Matplot, clf){

	phalcon_py_matplot_object *intern;
	PyObject *res;

	PHP_PYTHON_THREAD_ACQUIRE();
	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));
	res = PyObject_CallObject(intern->s_python_function_clf, intern->s_python_empty_tuple);

	if (!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to clf() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
}

PHP_METHOD(Phalcon_Py_Matplot, tightLayout){

	phalcon_py_matplot_object *intern;
	PyObject *res;

	PHP_PYTHON_THREAD_ACQUIRE();
	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));
	res = PyObject_CallObject(intern->s_python_function_tight_layout, intern->s_python_empty_tuple);

	if (!res) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Call to tightLayout() failed.");
		return;
	}

	Py_DECREF(res);
	PHP_PYTHON_THREAD_RELEASE();
}

/**
 * Handles method calls when a method is not implemented
 *
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_Py_Matplot, __call){

	zval *method, *arguments = NULL;
	phalcon_py_matplot_object *intern;
	PyObject *function, *args, *kwargs = NULL, *retval;

	phalcon_fetch_params(0, 1, 1, &method, &arguments);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_matplot_object_from_obj(Z_OBJ_P(getThis()));

	function = PyObject_GetAttrString(intern->pyplot, Z_STRVAL_P(method));

	if (!function) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Couldn't find required function!");		
		PHP_PYTHON_THREAD_RELEASE();
		return;
	}

	if (!PyFunction_Check(function)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Python object is unexpectedly not a PyFunction!");		
		PHP_PYTHON_THREAD_RELEASE();
		return;
	}

	if (!arguments) {
		args = PyTuple_New(0);
	} else if (Z_TYPE_P(arguments) == IS_ARRAY) {
		zval zero = {};
		if (phalcon_array_isset_fetch_long(&zero, arguments, 0, PH_READONLY) && Z_TYPE(zero) == IS_ARRAY) {
			zval _args = {}, _kwargs = {};
			if (phalcon_array_isset_fetch_str(&_args, &zero, SL("args"), PH_READONLY)) {
				args = pip_hash_to_tuple(&_args);
			} else {
				args = PyTuple_New(0);
			}
			if (phalcon_array_isset_fetch_str(&_kwargs, &zero, SL("kwargs"), PH_READONLY)) {
				kwargs = pip_hash_to_dict(&_kwargs);
			}
		} else {
			args = PyTuple_New(0);
		}
	} else {
		args = PyTuple_New(1);
		PyTuple_SetItem(args, 0, pip_zval_to_pyobject(arguments));
	}

	if (kwargs) {
		retval = PyObject_Call(function, args, kwargs);
		Py_DECREF(kwargs);
	} else {
		retval = PyObject_CallObject(function, args);
	}
	Py_DECREF(function);
	Py_DECREF(args);
	if (!retval) {
		PHP_PYTHON_THREAD_RELEASE();
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_py_exception_ce, "Call to %s() failed.", Z_STRVAL_P(method));
		return;
	}
	pip_pyobject_to_zval(retval, return_value);
	Py_DECREF(retval);
	PHP_PYTHON_THREAD_RELEASE();
}
