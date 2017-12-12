
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

#ifndef PHALCON_PY_COMMON_H
#define PHALCON_PY_COMMON_H

#include "php_phalcon.h"

#if PHALCON_USE_PYTHON

#include <Python.h>

#define PHP_PYTHON_THREAD_ASSERT() assert(PyThreadState_GET() == PHALCON_GLOBAL(python).tstate)
#define PHP_PYTHON_THREAD_ACQUIRE() PyEval_AcquireThread(PHALCON_GLOBAL(python).tstate)
#define PHP_PYTHON_THREAD_RELEASE() PyEval_ReleaseThread(PyThreadState_GET())

void python_error(int error_type);

zend_ulong python_get_arg_info(PyObject *callable, zend_internal_arg_info **arg_info) ;

/* PHP to Python Conversion */
PyObject* pip_hash_to_list(zval *hash);
PyObject* pip_hash_to_tuple(zval *hash);
PyObject* pip_hash_to_dict(zval *hash);
PyObject* pip_zobject_to_pyobject(zval *obj);
PyObject* pip_zval_to_pyobject(zval *val);

/* Python to PHP Conversion */
int pip_sequence_to_hash(PyObject *o, HashTable *ht);
int pip_sequence_to_array(PyObject *o, zval *zv);
int pip_mapping_to_hash(PyObject *o, HashTable *ht);
int pip_mapping_to_array(PyObject *o, zval *zv);
int pip_pyobject_to_zobject(PyObject *o, zval *zv);
int pip_pyobject_to_zval(PyObject *o, zval *zv);

/* Argument Conversion */
PyObject* pip_args_to_tuple(int argc, int start);

/* Object Representations */
int python_str(PyObject *o, char **buffer, Py_ssize_t *length);

/* Python Streams */
int python_streams_init();
int python_streams_intercept();

/* Python Modules */
int python_php_init(); 

static inline PyThreadState *interpreter_python_init_thread()
{
	PyInterpreterState *interp = NULL;
	PyThreadState *new_thread_state;

	// create a new interpreter 
	PyEval_AcquireLock();
	new_thread_state = Py_NewInterpreter();
	assert(new_thread_state != NULL);
	python_php_init();
	python_streams_intercept();
	PyEval_ReleaseThread(new_thread_state);
	return new_thread_state;
}

static inline void interpreter_python_shutdown_thread(PyThreadState *thread_state)
{
	// release the interpreter 
	PyEval_AcquireThread(thread_state);
	Py_EndInterpreter(thread_state);
	PyEval_ReleaseLock();
}

#endif

#endif /* PHALCON_PY_COMMON_H */
