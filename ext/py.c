
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

#include "py.h"
#include "py/object.h"
#include "py/exception.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/object.h"

/**
 * Phalcon\Py
 *
 */
zend_class_entry *phalcon_py_ce;

PHP_METHOD(Phalcon_Py, import);
PHP_METHOD(Phalcon_Py, construct);
PHP_METHOD(Phalcon_Py, callFunction);
PHP_METHOD(Phalcon_Py, callMethod);
PHP_METHOD(Phalcon_Py, eval);
PHP_METHOD(Phalcon_Py, exec);
PHP_METHOD(Phalcon_Py, list);
PHP_METHOD(Phalcon_Py, tuple);
PHP_METHOD(Phalcon_Py, dict);
PHP_METHOD(Phalcon_Py, val);
PHP_METHOD(Phalcon_Py, version);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_import, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_construct, 0, 0, 2)
	ZEND_ARG_INFO(0, moduleName)
	ZEND_ARG_TYPE_INFO(0, className, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_callfunction, 0, 0, 2)
	ZEND_ARG_INFO(0, moduleName)
	ZEND_ARG_TYPE_INFO(0, functionName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_callmethod, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, object, Phalcon\\Py\\Object, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, args, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_eval, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, expr, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_exec, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_list, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, val, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_tuple, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, val, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_dict, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, val, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_val, 0, 0, 1)
	ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_py_method_entry[] = {
	PHP_ME(Phalcon_Py, import, arginfo_phalcon_py_import, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py, construct, arginfo_phalcon_py_construct, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py, callFunction, arginfo_phalcon_py_callfunction, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py, callMethod, arginfo_phalcon_py_callmethod, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py, eval, arginfo_phalcon_py_eval, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py, exec, arginfo_phalcon_py_exec, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py, list, arginfo_phalcon_py_list, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py, tuple, arginfo_phalcon_py_tuple, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py, dict, arginfo_phalcon_py_dict, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py, val, arginfo_phalcon_py_val, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Py, version, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Py initializer
 */
PHALCON_INIT_CLASS(Phalcon_Py){

	PHALCON_REGISTER_CLASS(Phalcon, Py, py, phalcon_py_method_entry, 0);

	return SUCCESS;
}

/**
 * Importing Modules
 *
 * @param string $moduleName
 */
PHP_METHOD(Phalcon_Py, import){

	zval *name;
	PyObject *modulename;
	PyObject *module;

	phalcon_fetch_params(0, 1, 0, &name);

	PHP_PYTHON_THREAD_ACQUIRE();
	modulename = PyString_FromString(Z_STRVAL_P(name));
	if (!modulename) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_py_exception_ce, "Could't create string %s!", Z_STRVAL_P(name));		
		PHP_PYTHON_THREAD_RELEASE();
		return;
	}
	module = PyImport_Import(modulename);
	Py_DECREF(modulename);
	if (!module) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_py_exception_ce, "Error import module %s!", Z_STRVAL_P(name));		
		PHP_PYTHON_THREAD_RELEASE();
		return;
	}
	pip_pyobject_to_zval(module, return_value);
	Py_DECREF(module);

	PHP_PYTHON_THREAD_RELEASE();
}

/**
 * Class constructor
 *
 * @param string $moduleName
 * @param string $className
 */
PHP_METHOD(Phalcon_Py, construct){

	zval *module_name, *class_name;
	phalcon_py_object_object* intern;
	PyObject *module;

	phalcon_fetch_params(0, 2, 0, &module_name, &class_name);

	PHP_PYTHON_THREAD_ACQUIRE();

	if (Z_TYPE_P(module_name) == IS_OBJECT && phalcon_instance_of_ev(module_name, phalcon_py_object_ce)) {
		intern = phalcon_py_object_object_from_obj(Z_OBJ_P(module_name));
		module = intern->obj;
		Py_INCREF(module);
	} else if (Z_TYPE_P(module_name) == IS_STRING) {
		module = PyImport_ImportModule(Z_STRVAL_P(module_name));
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "The module paramerter must be string or Phalcon\\Py\\Object");
		PHP_PYTHON_THREAD_RELEASE();
		return;
	}

	if (module) {
		PyObject *dict, *class;

		/*
		 * The module's dictionary holds references to all of its members.  Use
		 * it to acquire a pointer to the requested class.
		 */
		dict = PyModule_GetDict(module);
		class = PyDict_GetItemString(dict, Z_STRVAL_P(class_name));

		/* If the class exists and is callable ... */
		if (class && PyCallable_Check(class)) {
			phalcon_py_object_object *object_intern;
			PyObject *object, *args = NULL;

			/*
			 * Convert our PHP arguments into a Python-digestable tuple.  We
			 * skip the first two arguments (module name, class name) and pass
			 * the rest to the Python constructor.
			 */
			args = pip_args_to_tuple(ZEND_NUM_ARGS(), 2);

			/*
			 * Call the class's constructor and store the resulting object.  If
			 * we have a tuple of arguments, remember to free (decref) it.
			 */
			object = PyObject_CallObject(class, args);
			if (args)
				Py_DECREF(args);

			if (object == NULL)
				python_error(E_ERROR);

			/* Our new object should be an instance of the requested class. */
			assert(PyObject_IsInstance(object, class));

			object_init_ex(return_value, phalcon_py_object_ce);
			object_intern = phalcon_py_object_object_from_obj(Z_OBJ_P(return_value));
			object_intern->obj = object;
		} else {
			php_error(E_ERROR, "Python: '%s.%s' is not a callable object", Z_STRVAL_P(module_name), Z_STRVAL_P(class_name));
		}

		Py_DECREF(module);
	} else {
		php_error(E_ERROR, "Python: '%s' is not a valid module", Z_STRVAL_P(module_name));
	}

	PHP_PYTHON_THREAD_RELEASE();
}

/**
 * Call the requested function in the requested module
 *
 * @param string $moduleName
 * @param string $functionName
 */
PHP_METHOD(Phalcon_Py, callFunction){

	zval *module_name, *function_name;
	phalcon_py_object_object* intern;
	PyObject *module;

	phalcon_fetch_params(0, 2, 0, &module_name, &function_name);

	PHP_PYTHON_THREAD_ACQUIRE();

	/* Attempt to import the requested module. */
	if (Z_TYPE_P(module_name) == IS_OBJECT && phalcon_instance_of_ev(module_name, phalcon_py_object_ce)) {
		intern = phalcon_py_object_object_from_obj(Z_OBJ_P(module_name));
		module = intern->obj;
		Py_INCREF(module);
	} else if (Z_TYPE_P(module_name) == IS_STRING) {
		module = PyImport_ImportModule(Z_STRVAL_P(module_name));
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "The module paramerter must be string or Phalcon\\Py\\Object");
		return;
	}

	if (module) {
		PyObject *dict, *function;

		/* Look up the function name in the module's dictionary. */
		dict = PyModule_GetDict(module);
		function = PyDict_GetItemString(dict, Z_STRVAL_P(function_name));
		if (function) {
			PyObject *args, *retval;

			/*
			 * Call the function with a tuple of arguments.  We skip the first
			 * two arguments to python_call (the module name and function name)
			 * and pack the remaining values into the 'args' tuple.
			 */
			args = pip_args_to_tuple(ZEND_NUM_ARGS(), 2);
			retval = PyObject_CallObject(function, args);
			if (args)
				Py_DECREF(args);

			if (retval) {
				/* Convert the Python result to its PHP equivalent. */
				pip_pyobject_to_zval(retval, return_value);
				Py_DECREF(retval);
			} else {
				python_error(E_ERROR);
			}
		} else {
			python_error(E_ERROR);
		}
		Py_DECREF(module);
	} else {
		python_error(E_ERROR);
	}

	PHP_PYTHON_THREAD_RELEASE();
}

/**
 * Calls a method of the object
 *
 * @param Phalcon\Py\Object $object
 * @param string $name
 */
PHP_METHOD(Phalcon_Py, callMethod){

	zval *object, *name;
	phalcon_py_object_object* intern;
	PyObject *methodname, *callable, *args, *retval;

	phalcon_fetch_params(0, 2, 0, &object, &name);

	PHP_PYTHON_THREAD_ACQUIRE();

	intern = phalcon_py_object_object_from_obj(Z_OBJ_P(object));
	methodname = PyString_FromString(Z_STRVAL_P(name));

    if (intern->obj == NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_py_exception_ce, "Object error");
		return;
    }

    callable = PyObject_GetAttr(intern->obj, methodname);
    if (callable == NULL) {
        RETURN_NULL();
    }

	args = pip_args_to_tuple(ZEND_NUM_ARGS(), 2);

	retval = PyObject_CallObject(callable, args);
	if (retval) {
		/* Convert the Python result to its PHP equivalent. */
		pip_pyobject_to_zval(retval, return_value);
		Py_DECREF(retval);
	} else {
		python_error(E_ERROR);
	}

	Py_DECREF(callable);

	PHP_PYTHON_THREAD_RELEASE();
}

/**
 * Evaluate a string of code by passing it to the Python interpreter
 *
 * @param string $expr
 */
PHP_METHOD(Phalcon_Py, eval){

	zval *expr;
	PyObject *m, *d, *v;

	phalcon_fetch_params(0, 1, 0, &expr);

	PHP_PYTHON_THREAD_ACQUIRE();

	/*
	 * The command will be evaluated in __main__'s context (for both
	 * globals and locals).  If __main__ doesn't already exist, it will be
	 * created.
	 */
	m = PyImport_AddModule("__main__");
	if (m == NULL) {
		PHP_PYTHON_THREAD_RELEASE();
		RETURN_NULL();
	}
	d = PyModule_GetDict(m);

	/*
	 * The string is evaluated as a single, isolated expression.  It is not
	 * treated as a statement or as series of statements.  This allows us to
	 * retrieve the resulting value of the evaluated expression.
	 */
	v = PyRun_String(Z_STRVAL_P(expr), Py_eval_input, d, d);
	if (v == NULL) {
		python_error(E_WARNING);
		PHP_PYTHON_THREAD_RELEASE();
		RETURN_NULL();
	}

	/*
	 * We convert the PyObject* value to a zval* so that we can return the
	 * result to PHP.  If the conversion fails, we'll still be left with a
	 * valid zval* equal to PHP's NULL.
	 *
	 * At this point, we're done with our PyObject* value, as well.  We can
	 * safely release our reference to it now.
	 */
	if (pip_pyobject_to_zval(v, return_value) == FAILURE)
		ZVAL_NULL(return_value);

	Py_DECREF(v);

	PHP_PYTHON_THREAD_RELEASE();
}

/**
 * Execute a string of code by passing it to the Python interpreter
 *
 * @param string $command
 */
PHP_METHOD(Phalcon_Py, exec){

	zval *command;
	PyObject *m, *d, *v;

	phalcon_fetch_params(0, 1, 0, &command);

	PHP_PYTHON_THREAD_ACQUIRE();

	/*
	 * The command will be evaluated in __main__'s context (for both
	 * globals and locals).  If __main__ doesn't already exist, it will be
	 * created.
	 */
	m = PyImport_AddModule("__main__");
	if (m == NULL) {
		PHP_PYTHON_THREAD_RELEASE();
		RETURN_FALSE;
	}
	d = PyModule_GetDict(m);

	/*
	 * The string is executed as a sequence of statements.  This means that
	 * can only detect the overall success or failure of the execution.  We
	 * cannot return any other kind of result value.
	 */
	v = PyRun_String(Z_STRVAL_P(command), Py_file_input, d, d);
	if (v == NULL) {
		python_error(E_WARNING);
		PHP_PYTHON_THREAD_RELEASE();
		RETURN_FALSE;
	}

	Py_DECREF(v);

	PHP_PYTHON_THREAD_RELEASE();

	RETURN_TRUE;
}

/**
 * PHP array to Python list
 *
 * @param array $val
 */
PHP_METHOD(Phalcon_Py, list){

	zval *val;
	PyObject *v;

	phalcon_fetch_params(0, 1, 0, &val);

	PHP_PYTHON_THREAD_ACQUIRE();

	v = pip_hash_to_list(val);
	pip_pyobject_to_zval(v, return_value);
	Py_DECREF(v);

	PHP_PYTHON_THREAD_RELEASE();
}

/**
 * PHP array to Python tuple
 *
 * @param array $val
 */
PHP_METHOD(Phalcon_Py, tuple){

	zval *val;
	PyObject *v;

	phalcon_fetch_params(0, 1, 0, &val);

	PHP_PYTHON_THREAD_ACQUIRE();

	v = pip_hash_to_tuple(val);
	pip_pyobject_to_zval(v, return_value);
	Py_DECREF(v);

	PHP_PYTHON_THREAD_RELEASE();
}

/**
 * PHP array to Python dict
 *
 * @param array $val
 */
PHP_METHOD(Phalcon_Py, dict){

	zval *val;
	PyObject *v;

	phalcon_fetch_params(0, 1, 0, &val);

	PHP_PYTHON_THREAD_ACQUIRE();

	v = pip_hash_to_dict(val);
	pip_pyobject_to_zval(v, return_value);
	Py_DECREF(v);

	PHP_PYTHON_THREAD_RELEASE();
}

/**
 * PHP val to Python val
 *
 * @param mixed $val
 */
PHP_METHOD(Phalcon_Py, val){

	zval *val;
	PyObject *v;

	phalcon_fetch_params(0, 1, 0, &val);

	PHP_PYTHON_THREAD_ACQUIRE();

	v = pip_zval_to_pyobject(val);
	pip_pyobject_to_zval(v, return_value);
	Py_DECREF(v);

	PHP_PYTHON_THREAD_RELEASE();
}

/**
 * Returns the Python interpreter's version as a string
 *
 */
PHP_METHOD(Phalcon_Py, version){

	const char *version;

	PHP_PYTHON_THREAD_ACQUIRE();
	version = Py_GetVersion();
	PHP_PYTHON_THREAD_RELEASE();

	RETURN_STRING((char *)version);
}
