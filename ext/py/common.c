
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

#include "py/common.h"
#include "py/object.h"

#include "kernel/fcall.h"

void python_error(int error_type)
{
	PyObject *ptype, *pvalue, *ptraceback;
	PyObject *type, *value;

	PHP_PYTHON_THREAD_ASSERT();

	/* Fetch the last error and store the type and value as strings. */
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	type = PyObject_Str(ptype);
	value = PyObject_Str(pvalue);

	Py_XDECREF(ptype);
	Py_XDECREF(pvalue);
	Py_XDECREF(ptraceback);

	if (type && value) {
		php_error(error_type, "Python: [%s] '%s'", PyString_AsString(type), PyString_AsString(value));
		Py_DECREF(type);
		Py_DECREF(value);
	}
}

/* Returns the number of arguments expected by the given callable object */
zend_ulong python_num_args(PyObject *callable)
{
	PyObject *func_code, *co_argcount;
	zend_ulong num_args = 0;

	PHP_PYTHON_THREAD_ACQUIRE();

	if ((func_code = PyObject_GetAttrString(callable, "func_code"))) {
		if ((co_argcount = PyObject_GetAttrString(func_code, "co_argcount"))) {
			num_args = PyInt_AsLong(co_argcount);
			Py_DECREF(co_argcount);
		}
		Py_DECREF(func_code);
	}

	PHP_PYTHON_THREAD_RELEASE();

	return num_args;
}

/* Fills out the arg_info array and returns the total number of arguments */
zend_ulong python_get_arg_info(PyObject *callable, zend_internal_arg_info **arg_info)
{
	PyObject *func_code, *co_varnames;
	zend_ulong num_args = 0;

	PHP_PYTHON_THREAD_ACQUIRE();

	/* Make sure that we've been passed a valid, callable object. */
	if (!callable || PyCallable_Check(callable) == 0)
	{
		PHP_PYTHON_THREAD_RELEASE();
		return 0;
	}

	/*
	 * The arguments are described by the object's func_code.co_varnames
	 * member.  They're represented as a Python tuple.
	 */
	if ((func_code = PyObject_GetAttrString(callable, "func_code"))) {
		if ((co_varnames = PyObject_GetAttrString(func_code, "co_varnames"))) {
			PyObject *arg;
			int i, num_vars, start = 0;

			/*
			 * Get the number of arguments defined by the co_varnames tuple
			 * and use that value to resize the arg_info array.
			 */
			num_vars = num_args = PyTuple_Size(co_varnames);

			/* If this is a method, skip the explicit "self" argument. */
			if (PyMethod_Check(callable)) {
				start = 1;
				num_args -= 1;
			}

			/* Resize the arg_info array based on the number of arguments. */
			*arg_info = ecalloc(num_args, sizeof(zend_internal_arg_info));

			/* Describe each of this method's arguments. */
			for (i = start; i < num_vars; ++i) {
				arg = PyTuple_GetItem(co_varnames, i);

				/* Fill out the zend_arg_info structure for this argument. */
				if (arg && PyString_Check(arg)) {
					arg_info[i-start]->name = estrdup(PyString_AS_STRING(arg));
					arg_info[i-start]->class_name = '\0';
					arg_info[i-start]->allow_null = 1;
					arg_info[i-start]->pass_by_reference = 0;
				}
			}

			Py_DECREF(co_varnames);
		}
		Py_DECREF(func_code);
	}

	PHP_PYTHON_THREAD_RELEASE();

	return num_args;
}

/* PHP to Python Conversion */

PyObject* pip_hash_to_list(zval *hash)
{
	PyObject *list;
	zval *entry;
	int pos = 0;

	PHP_PYTHON_THREAD_ASSERT();

	/* Make sure we were given a PHP hash. */
	if (Z_TYPE_P(hash) != IS_ARRAY) {
		return NULL;
	}

	/* Create a list with the same number of elements as the hash. */
	list = PyList_New(zend_hash_num_elements(Z_ARRVAL_P(hash)));

	/* Let's start at the very beginning, a very good place to start. */
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(hash));

	/*
	 * Iterate over of the hash's elements.  We ignore the keys and convert
	 * each value to its Python equivalent before inserting it into the list.
	 */
	while ((entry = zend_hash_get_current_data(Z_ARRVAL_P(hash))) != NULL) {
		PyObject *item = pip_zval_to_pyobject(entry);
		PyList_SetItem(list, pos++, item);
		zend_hash_move_forward(Z_ARRVAL_P(hash));
	}

	return list;
}

PyObject* pip_hash_to_tuple(zval *hash)
{
	PHP_PYTHON_THREAD_ASSERT();

	return PyList_AsTuple(pip_hash_to_list(hash));
}

PyObject* pip_hash_to_dict(zval *hash)
{
	PyObject *dict, *integer;
	zval *entry;
	zend_string *string_key;
	unsigned long num_key;

	PHP_PYTHON_THREAD_ASSERT();

	/* Make sure we were given a PHP hash. */
	if (Z_TYPE_P(hash) != IS_ARRAY) {
		return NULL;
	}

	/* Create a new empty dictionary. */
	dict = PyDict_New();

	/* Let's start at the very beginning, a very good place to start. */
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(hash));

	/* Iterate over the hash's elements. */
	while ((entry =zend_hash_get_current_data(Z_ARRVAL_P(hash))) != NULL) {

		/* Convert the PHP value to its Python equivalent (recursion). */
		PyObject *item = pip_zval_to_pyobject(entry);

		/* Assign the item with the appropriate key type (string or integer). */
		switch (zend_hash_get_current_key(Z_ARRVAL_P(hash), &string_key, &num_key)) {
			case HASH_KEY_IS_STRING:
				PyDict_SetItemString(dict, ZSTR_VAL(string_key), item);
				break;
			case HASH_KEY_IS_LONG:
				integer = PyInt_FromLong(num_key);
				PyDict_SetItem(dict, integer, item);
				Py_DECREF(integer);
				break;
		}

		/* Advance to the next entry. */
		zend_hash_move_forward(Z_ARRVAL_P(hash));
	}

	return dict;
}

PyObject* pip_zobject_to_pyobject(zval *obj)
{
	PyObject *dict, *str;
	zval *entry;
	zend_string *string_key;
	unsigned long num_key;

	PHP_PYTHON_THREAD_ASSERT();

	/*
	 * At this point, we represent a PHP object as a dictionary of
	 * its properties.  In the future, we may provide a true object
	 * conversion (which is entirely possible, but it's more work
	 * that I plan on doing right now).
	 */
	dict = PyDict_New();

	/* Start at the beginning of the object properties hash */
	zend_hash_internal_pointer_reset(Z_OBJPROP_P(obj));

	/* Iterate over the hash's elements */
	while ((entry =zend_hash_get_current_data(Z_OBJPROP_P(obj))) != NULL) {
		/* Convert the PHP value to its Python equivalent (recursion) */
		PyObject *item = pip_zval_to_pyobject(entry);

		switch (zend_hash_get_current_key(Z_OBJPROP_P(obj), &string_key, &num_key)) {
			case HASH_KEY_IS_STRING:
				PyDict_SetItemString(dict, ZSTR_VAL(string_key), item);
				break;
			case HASH_KEY_IS_LONG:
				str = PyString_FromFormat("%lu", num_key);
				PyObject_SetItem(dict, str, item);
				Py_DECREF(str);
				break;
			case HASH_KEY_NON_EXISTENT:
				php_error(E_ERROR, "Hash key is nonexistent");
				break;
		}

		/* Advance to the next entry */
		zend_hash_move_forward(Z_OBJPROP_P(obj));
	}

	return dict;
}

PyObject* pip_zval_to_pyobject(zval *val)
{
	PyObject *ret;

	PHP_PYTHON_THREAD_ASSERT();

	if (val == NULL) {
		return NULL;
	}

	if (Z_TYPE_P(val) == IS_INDIRECT) {
		val = Z_INDIRECT_P(val);
	}

	switch (Z_TYPE_P(val)) {
	case IS_TRUE:
		ret = PyBool_FromLong(1);
		break;
	case IS_FALSE:
		ret = PyBool_FromLong(0);
		break;
	case IS_LONG:
		ret = PyLong_FromLong(Z_LVAL_P(val));
		break;
	case IS_DOUBLE:
		ret = PyFloat_FromDouble(Z_DVAL_P(val));
		break;
	case IS_STRING:
		ret = PyString_FromStringAndSize(Z_STRVAL_P(val), Z_STRLEN_P(val));
		break;

	case IS_ARRAY:
		ret = pip_hash_to_dict(val);
		break;
	case IS_OBJECT:
		ret = pip_zobject_to_pyobject(val);
		break;
	case IS_NULL:
		Py_INCREF(Py_None);
		ret = Py_None;
		break;
	default:
		ret = NULL;
		break;
	}

	return ret;
}

/* Python to PHP Conversion */

int pip_sequence_to_hash(PyObject *o, HashTable *ht)
{
	PyObject *item;
	zval v;
	int i, size;

	PHP_PYTHON_THREAD_ASSERT();

	/* Make sure this object implements the sequence protocol. */
	if (!PySequence_Check(o))
		return FAILURE;

	size = PySequence_Size(o);
	for (i = 0; i < size; ++i) {
		/*
		 * Acquire a reference to the current item in the sequence.  If we
		 * fail to get the object, we return a hard failure.
		 */
		item = PySequence_ITEM(o, i);
		if (item == NULL)
			return FAILURE;

		/*
		 * Attempt to convert the item from a Python object to a PHP value.
		 * This involves allocating a new zval, testing the success of the
		 * conversion, and potentially cleaning up the zval upon failure.
		 */
		if (pip_pyobject_to_zval(item, &v) == FAILURE) {
			zval_dtor(&v);
			Py_DECREF(item);
			return FAILURE;
		}
		Py_DECREF(item);

		/*
		 * Append (i.e., insert into the next slot) the PHP value into our
		 * hashtable.  Failing to insert the value results in a hard
		 * failure.
		 */
		if (!zend_hash_next_index_insert(ht, &v))
			return FAILURE;
	}

	return SUCCESS;
}

int pip_sequence_to_array(PyObject *o, zval *zv)
{
	PHP_PYTHON_THREAD_ASSERT();

	/*
	 * Initialize our zval as an array.  The converted sequence will be
	 * stored in the array's hashtable.
	 */
	if (array_init(zv) == SUCCESS)
		return pip_sequence_to_hash(o, Z_ARRVAL_P(zv));

	return FAILURE;
}

int pip_mapping_to_hash(PyObject *o, HashTable *ht)
{
	PyObject *keys, *key, *str, *item;
	char *name;
	int i, size;
	Py_ssize_t name_len;
	int status = FAILURE;

	PHP_PYTHON_THREAD_ASSERT();

	/*
	 * We start by retrieving the list of keys for this mapping.  We will
	 * use this list below to address each item in the mapping.
	 */
	keys = PyMapping_Keys(o);
	if (keys) {
		/*
		 * Iterate over all of the mapping's keys.
		 */
		size = PySequence_Size(keys);
		for (i = 0; i < size; ++i) {
			zval v = {};
			/*
			 * Acquire a reference to the current item in the mapping.  If
			 * we fail to get the object, we return a hard failure.
			 */
			key = PySequence_ITEM(keys, i);
			if (key == NULL) {
				status = FAILURE;
				break;
			}

			/*
			 * We request the string representation of this key.  PHP
			 * hashtables only support string-based keys.
			 */
			str = PyObject_Str(key);
			if (!str || PyString_AsStringAndSize(str, &name, &name_len) == -1) {
				Py_DECREF(key);
				status = FAILURE;
				break;
			}

			/*
			 * Extract the item associated with the named key.
			 */
			item = PyMapping_GetItemString(o, name);
			if (item == NULL) {
				Py_DECREF(str);
				Py_DECREF(key);
				status = FAILURE;
				break;
			}

			/*
			 * Attempt to convert the item from a Python object to a PHP
			 * value.  This involves allocating a new zval.  If the
			 * conversion fails, we must remember to free the allocated zval
			 * below.
			 */
			status = pip_pyobject_to_zval(item, &v);

			/*
			 * If we've been successful up to this point, attempt to add the
			 * new item to our hastable.
			 */
			if (status == SUCCESS) {
				if (!zend_hash_str_update(ht, name, name_len + 1, &v)) {
					zval_dtor(&v);
				}
			}

			/*
			 * Release our reference to the Python objects that are still
			 * active in this scope.
			 */
			Py_DECREF(item);
			Py_DECREF(str);
			Py_DECREF(key);
		}

		Py_DECREF(keys);
	}

	return status;
}

int pip_mapping_to_array(PyObject *o, zval *zv)
{
	PHP_PYTHON_THREAD_ASSERT();

	if (array_init(zv) == SUCCESS)
		return pip_mapping_to_hash(o, Z_ARRVAL_P(zv));

	return FAILURE;
}

int pip_pyobject_to_zobject(PyObject *o, zval *zv)
{
	phalcon_py_object_object *pip;

	PHP_PYTHON_THREAD_ASSERT();

	/* Create a new instance of a PHP Python object. */
	if (object_init_ex(zv, phalcon_py_object_ce) != SUCCESS)
		return FAILURE;

	/*
	 * Fetch the php_python_object data for this object instance. Bump the
	 * reference count of our Python object and associate it with our PHP
	 * Python object instance.
	 */
	pip = phalcon_py_object_object_from_obj(Z_OBJ_P(zv));
	Py_INCREF(o);
	pip->obj = o;

	return SUCCESS;
}

int pip_pyobject_to_zval(PyObject *o, zval *zv)
{
	PHP_PYTHON_THREAD_ASSERT();

	/*
	 * The general approach taken below is to infer the Python object's type
	 * using a series of tests based on Python's type-specific _Check()
	 * functions.  If the test passes, then we proceed immediately with the
	 * fastest possible conversion (which implies using the versions of the
	 * conversion functions that don't perform any additional error
	 * checking).
	 *
	 * Complex conversions are farmed out to reusable helper functions.
	 *
	 * The order of the tests below is largel insignificant, aside from the
	 * initial test of (o == NULL).  They could be reordered for performance
	 * purposes in the future once we have a sense of the most commonly
	 * converted types.
	 */

	/*
	 * If our object is invalid or None, treat it like PHP's NULL value.
	 */
	if (o == NULL || o == Py_None) {
		ZVAL_NULL(zv);
		return SUCCESS;
	}

	/*
	 * Python integers and longs (and their subclasses) are treated as PHP
	 * longs.  We don't perform any kind of fancy type casting here; if the
	 * original object isn't already an integer type, we don't attempt to
	 * treat it like one.
	 */
	if (PyInt_Check(o)) {
		ZVAL_LONG(zv, PyInt_AS_LONG(o));
		return SUCCESS;
	}
	if (PyLong_Check(o)) {
		ZVAL_LONG(zv, PyLong_AsLong(o));
		return SUCCESS;
	}

	/*
	 * Python floating point objects are treated as PHP doubles.
	 */
	if (PyFloat_Check(o)) {
		ZVAL_DOUBLE(zv, PyFloat_AS_DOUBLE(o));
		return SUCCESS;
	}

	/*
	 * Python strings are converted directly to PHP strings.  The contents
	 * of the string are copied (i.e., duplicated) into the zval.
	 */
	if (PyString_Check(o)) {
		ZVAL_STRINGL(zv, PyString_AS_STRING(o), PyString_GET_SIZE(o));
		return SUCCESS;
	}

	/*
	 * Python Unicode strings are converted to UTF8 and stored as PHP
	 * strings.  It is possible for this encoding-based conversion to fail.
	 * The contents of the UTF9-encoded string are copied (i.e., duplicated)
	 * into the zval.
	 *
	 * TODO:
	 * - Support richer conversions if PHP's Unicode support is available.
	 * - Potentially break this conversion out into its own routine.
	 */
	if (PyUnicode_Check(o)) {
		PyObject *s = PyUnicode_AsUTF8String(o);
		if (s) {
			ZVAL_STRINGL(zv, PyString_AS_STRING(s), PyString_GET_SIZE(s));
			Py_DECREF(s);
			return SUCCESS;
		}

		return FAILURE;
	}

	/*
	 * If all of the other conversions failed, we attempt to convert the
	 * Python object to a PHP object.
	 */
	return pip_pyobject_to_zobject(o, zv);
}
/* }}} */

/* Argument Conversions */

PyObject* pip_args_to_tuple(int argc, int start)
{
	PyObject *arg, *args = NULL;
	zval *zargs;
	int i;

	PHP_PYTHON_THREAD_ASSERT();

	if (argc < start)
		return NULL;

	/*
	 * In order to convert our current arguments to a Python tuple object,
	 * we need to make a temporary copy of the argument array.  We use this
	 * array copy to create and populate a new tuple object containing the
	 * individual arguments.
	 *
	 * An alternative to the code below is zend_copy_parameters_array().
	 * That function populates a PHP array instance with the parameters.  It
	 * seems like the code below is probably going to be more efficient
	 * because it pre-allocates all of the memory for the array up front,
	 * but it would be worth testing the two approaches to know for sure.
	 */
	zargs = (zval *) emalloc(sizeof(zval) * argc);
	if (zargs) {
		if (zend_get_parameters_array_ex(argc, zargs) == SUCCESS) {
			args = PyTuple_New(argc - start);
			if (args) {
				for (i = start; i < argc; ++i) {
					arg = pip_zval_to_pyobject(&zargs[i]);
					PyTuple_SetItem(args, i - start, arg);
				}
			}
		}

		efree(zargs);
	}

	return args;
}

/* Object Representations */

int python_str(PyObject *o, char **buffer, Py_ssize_t *length)
{
	PyObject *str;
	int ret = -1;

	PHP_PYTHON_THREAD_ASSERT();

	/*
	 * Compute the string representation of the given object.  This is the
	 * equivalent of 'str(obj)' or passing the object to 'print'.
	 */
 	str = PyObject_Str(o);

	if (str) {
		/* XXX: length is a Py_ssize_t and could overflow an int. */
		ret = PyString_AsStringAndSize(str, buffer, length);
		Py_DECREF(str);

		/*
		 * If the conversion raised a TypeError, clear it and just return
		 * our simple failure code to the caller.
		 */
		if (ret == -1 && PyErr_ExceptionMatches(PyExc_TypeError))
			PyErr_Clear();
	}

	return ret;
}

/* python call php */

static PyObject *php_call(PyObject *self, PyObject *args)
{
	const char *name;
	int name_len, i, argc;
	zval *argv, lcname = {}, ret = {};
	PyObject *params = NULL;

	if (!PyArg_ParseTuple(args, "s#|O:call", &name, &name_len, &params))
		return NULL;

	if (params && !PySequence_Check(params)) {
		PyErr_Format(PyExc_ValueError, "Second argument must be a sequence");
		return NULL;
	}

	/* Name entries in the PHP function_table are always lowercased. */
	ZVAL_STRINGL(&lcname, (char *)name, name_len);
	zend_str_tolower(Z_STRVAL(lcname), name_len);

	/* If this isn't a valid PHP function name, we cannot proceed. */
	if (!zend_hash_exists(CG(function_table), Z_STR(lcname))) {
		PyErr_Format(PyExc_NameError, "Function does not exist: %s", Z_STRVAL(lcname));
		zval_ptr_dtor(&lcname);
		return NULL;
	}

	/* Convert the parameters into PHP values. */
	argc = params ? PySequence_Size(params) : 0;
	argv = emalloc(sizeof(zval) * argc);

	for (i = 0; i < argc; ++i) {
		PyObject *item = PySequence_GetItem(params, i);

		if (pip_pyobject_to_zval(item, &argv[i]) != SUCCESS) {
			PyErr_Format(PyExc_ValueError, "Bad argument at index %d", i);
			Py_DECREF(item);
			efree(argv);
			return NULL;
		}

		Py_DECREF(item);
	}

	/* Now we can call the PHP function. */
	if (phalcon_call_user_func_args(&ret, &lcname, argv, argc) != SUCCESS) {
		PyErr_Format(PyExc_Exception, "Failed to execute function: %s", Z_STRVAL(lcname));
		efree(argv);
		zval_ptr_dtor(&lcname);
		return NULL;
	}
	zval_ptr_dtor(&lcname);

	efree(argv);

	return pip_zval_to_pyobject(&ret);
}

static PyObject *php_var(PyObject *self, PyObject *args)
{
	char *name;
	int len;
	zval *v;

	if (!PyArg_ParseTuple(args, "s#", &name, &len))
		return NULL;

	if ((v = zend_hash_str_find(&EG(symbol_table), name, len)) == NULL) {
		PyErr_Format(PyExc_NameError, "Undefined variable: %s", name);
		return NULL;
	}

	return pip_zval_to_pyobject(v);
}

static PyObject *php_version(PyObject *self, PyObject *args)
{
	return Py_BuildValue("s", PHP_VERSION);
}

static PyMethodDef python_php_methods[] = {
	{"call",			php_call,			METH_VARARGS},
	{"var",				php_var,			METH_VARARGS},
	{"version",			php_version,		METH_NOARGS},
	{NULL, NULL, 0, NULL}
};

int python_php_init()
{
	if (Py_InitModule3("phalcon", python_php_methods, "Phalcon7 Module") == NULL)
		return FAILURE;

	return SUCCESS;
}


/* {{{ OutputStream
 */
typedef struct {
	PyObject_HEAD
} OutputStream;

static PyTypeObject OutputStream_Type;

/* {{{ OutputStream_close
 */
static PyObject *
OutputStream_close(OutputStream *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":close"))
		return NULL;

	PyErr_SetString(PyExc_RuntimeError, "Output stream cannot be closed");
	return NULL;
}
/* }}} */
/* {{{ OutputStream_flush
 */
static PyObject *
OutputStream_flush(OutputStream *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":flush"))
		return NULL;

	Py_INCREF(Py_None);
	return Py_None;
}
/* }}} */
/* {{{ OutputStream_write
 */
static PyObject *
OutputStream_write(OutputStream *self, PyObject *args)
{
	const char *str;
	int len;

	if (!PyArg_ParseTuple(args, "s#:write", &str, &len))
		return NULL;

	ZEND_WRITE(str, len);

	Py_INCREF(Py_None);
	return Py_None;
}
/* }}} */
/* {{{ OutputStream_writelines
 */
static PyObject *
OutputStream_writelines(OutputStream *self, PyObject *args)
{
	PyObject *sequence;
	PyObject *iterator;
	PyObject *item;
	char *str;
	Py_ssize_t len;

	if (!PyArg_ParseTuple(args, "O:writelines", &sequence))
        return NULL;

	iterator = PyObject_GetIter(sequence);
	if (iterator == NULL)
		return NULL;

	while ((item = PyIter_Next(iterator))) {
		if (PyString_AsStringAndSize(item, &str, &len) != -1) {
			ZEND_WRITE(str, len);
			Py_DECREF(item);
		} else {
			Py_DECREF(item);
			break;
		}
	}

	Py_DECREF(iterator);

	if (item && !str)
		return NULL;

	Py_INCREF(Py_None);
	return Py_None;
}
/* }}} */

/* {{{ OutputStream_closed
 */
static PyObject *
OutputStream_closed(OutputStream *self, void *closure)
{
	Py_INCREF(Py_False);
	return Py_False;
}
/* }}} */
/* {{{ OutputStream_isatty
 */
static PyObject *
OutputStream_isatty(OutputStream *self, void *closure)
{
	Py_INCREF(Py_False);
	return Py_False;
}
/* }}} */

/* {{{ OutputStream_methods
 */
static PyMethodDef OutputStream_methods[] = {
	{ "close",		(PyCFunction)OutputStream_close,		METH_VARARGS, 0 },
	{ "flush",		(PyCFunction)OutputStream_flush,		METH_VARARGS, 0 },
	{ "write",		(PyCFunction)OutputStream_write,		METH_VARARGS, 0 },
	{ "writelines",	(PyCFunction)OutputStream_writelines,	METH_VARARGS, 0 },
	{ NULL, NULL}
};
/* }}} */
/* {{{ OutputStream_getset
 */
static PyGetSetDef OutputStream_getset[] = {
	{ "closed",		(getter)OutputStream_closed,	NULL, 0 },
	{ "isatty",		(getter)OutputStream_isatty,	NULL, 0 },
	{ NULL },
};
/* }}} */
/* {{{ OutputStream_Type
 */
static PyTypeObject OutputStream_Type = {
	PyObject_HEAD_INIT(NULL)
	0,													/* ob_size */
	"php.OutputStream",									/* tp_name */
	sizeof(OutputStream),								/* tp_basicsize */
	0,													/* tp_itemsize */
	0,													/* tp_dealloc */
	0,													/* tp_print */
	0,													/* tp_getattr */
	0,													/* tp_setattr */
	0,													/* tp_compare */
	0,													/* tp_repr */
	0,													/* tp_as_number */
	0,													/* tp_as_sequence */
	0,													/* tp_as_mapping */
	0,													/* tp_hash */
	0,													/* tp_call */
	0,													/* tp_str */
	0,													/* tp_getattro */
	0,													/* tp_setattro */
	0,													/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,									/* tp_flags */
	"PHP OutputStream",									/* tp_doc */
	0,													/* tp_traverse */
	0,													/* tp_clear */
	0,													/* tp_richcompare */
	0,													/* tp_weaklistoffset */
	0,													/* tp_iter */
	0,													/* tp_iternext */
	OutputStream_methods,								/* tp_methods */
	0,													/* tp_members */
	OutputStream_getset,								/* tp_getset */
};
/* }}} */
/* }}} */
/* {{{ ErrorStream
 */
typedef struct {
	PyObject_HEAD
} ErrorStream;

static PyTypeObject ErrorStream_Type;

/* {{{ ErrorStream_close
 */
static PyObject *
ErrorStream_close(ErrorStream *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":close"))
		return NULL;

	PyErr_SetString(PyExc_RuntimeError, "Error stream cannot be closed");
	return NULL;
}
/* }}} */
/* {{{ ErrorStream_flush
 */
static PyObject *
ErrorStream_flush(ErrorStream *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":flush"))
		return NULL;

	Py_INCREF(Py_None);
	return Py_None;
}
/* }}} */
/* {{{ ErrorStream_write
 */
static PyObject *
ErrorStream_write(ErrorStream *self, PyObject *args)
{
	const char *str;

	if (!PyArg_ParseTuple(args, "s:write", &str))
		return NULL;

	php_error(E_NOTICE, "%s", str);

	Py_INCREF(Py_None);
	return Py_None;
}
/* }}} */
/* {{{ ErrorStream_writelines
 */
static PyObject *
ErrorStream_writelines(ErrorStream *self, PyObject *args)
{
	PyObject *sequence;
	PyObject *iterator;
	PyObject *item;
	const char *str;

	if (!PyArg_ParseTuple(args, "O:writelines", &sequence))
        return NULL;

	iterator = PyObject_GetIter(sequence);
	if (iterator == NULL)
		return NULL;

	while ((item = PyIter_Next(iterator))) {
		if ((str = PyString_AsString(item))) {
			php_error(E_NOTICE, "%s", str);
			Py_DECREF(item);
		} else {
			Py_DECREF(item);
			break;
		}
	}

	Py_DECREF(iterator);

	if (item && !str)
		return NULL;

	Py_INCREF(Py_None);
	return Py_None;
}
/* }}} */

/* {{{ ErrorStream_close
 */
static PyObject *
ErrorStream_closed(ErrorStream *self, void *closure)
{
	Py_INCREF(Py_False);
	return Py_False;
}
/* }}} */
/* {{{ ErrorStream_isatty
 */
static PyObject *
ErrorStream_isatty(ErrorStream *self, void *closure)
{
	Py_INCREF(Py_False);
	return Py_False;
}
/* }}} */

/* {{{ ErrorStream_methods
 */
static PyMethodDef ErrorStream_methods[] = {
	{ "close",		(PyCFunction)ErrorStream_close,			METH_VARARGS, 0 },
	{ "flush",		(PyCFunction)ErrorStream_flush,			METH_VARARGS, 0 },
	{ "write",		(PyCFunction)ErrorStream_write,			METH_VARARGS, 0 },
	{ "writelines",	(PyCFunction)ErrorStream_writelines,	METH_VARARGS, 0 },
	{ NULL, NULL}
};
/* }}} */
/* {{{ ErrorStream_getset
 */
static PyGetSetDef ErrorStream_getset[] = {
	{ "closed",		(getter)ErrorStream_closed,		NULL, 0 },
	{ "isatty",		(getter)ErrorStream_isatty,		NULL, 0 },
	{ NULL },
};
/* }}} */
/* {{{ ErrorStream_Type
 */
static PyTypeObject ErrorStream_Type = {
	PyObject_HEAD_INIT(NULL)
	0,													/* ob_size */
	"php.ErrorStream",									/* tp_name */
	sizeof(ErrorStream),								/* tp_basicsize */
	0,													/* tp_itemsize */
	0,													/* tp_dealloc */
	0,													/* tp_print */
	0,													/* tp_getattr */
	0,													/* tp_setattr */
	0,													/* tp_compare */
	0,													/* tp_repr */
	0,													/* tp_as_number */
	0,													/* tp_as_sequence */
	0,													/* tp_as_mapping */
	0,													/* tp_hash */
	0,													/* tp_call */
	0,													/* tp_str */
	0,													/* tp_getattro */
	0,													/* tp_setattro */
	0,													/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,									/* tp_flags */
	"PHP ErrorStream",									/* tp_doc */
	0,													/* tp_traverse */
	0,													/* tp_clear */
	0,													/* tp_richcompare */
	0,													/* tp_weaklistoffset */
	0,													/* tp_iter */
	0,													/* tp_iternext */
	ErrorStream_methods,								/* tp_methods */
	0,													/* tp_members */
	ErrorStream_getset,									/* tp_getset */
};
/* }}} */
/* }}} */

/* Initialize the Python streams interface. */
int python_streams_init()
{
	if (PyType_Ready(&OutputStream_Type) == -1)
		return FAILURE;

	if (PyType_Ready(&ErrorStream_Type) == -1)
		return FAILURE;

	return SUCCESS;
}

/* Redirect Python's streams to PHP equivalents. */
int python_streams_intercept()
{
	PyObject *stream;

	/* Redirect sys.stdout to an instance of our output stream type. */
	stream = (PyObject *)PyObject_New(OutputStream, &OutputStream_Type);
	PySys_SetObject("stdout", stream);
	Py_DECREF(stream);

	/* Redirect sys.stderr to an instance of our error stream type. */
	stream = (PyObject *)PyObject_New(ErrorStream, &ErrorStream_Type);
	PySys_SetObject("stderr", stream);
	Py_DECREF(stream);

	return SUCCESS;
}