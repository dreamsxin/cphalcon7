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

#include "py/object.h"
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
 * Phalcon\Py\Object
 *
 */
zend_class_entry *phalcon_py_object_ce;

PHP_METHOD(Phalcon_Py_Object, __construct);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_py_object___construct, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, moduleName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, className, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_py_object_method_entry[] = {
	PHP_ME(Phalcon_Py_Object, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_FE_END
};

zend_object_handlers phalcon_py_object_object_handlers;
zend_object* phalcon_py_object_object_create_handler(zend_class_entry *ce)
{
	phalcon_py_object_object *intern = ecalloc(1, sizeof(phalcon_py_object_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_py_object_object_handlers;

	return &intern->std;
}

void phalcon_py_object_object_free_handler(zend_object *object)
{
	phalcon_py_object_object *intern;

	intern = phalcon_py_object_object_from_obj(object);

	PHP_PYTHON_THREAD_ACQUIRE();
	if (intern->obj) {
		Py_XDECREF(intern->obj);
	}
	PHP_PYTHON_THREAD_RELEASE();
	zend_object_std_dtor(object);
}

static void efree_function(zend_internal_function *func)
{
	zend_ulong i;

	zend_string_release(func->function_name);

	/* Free the argument information. */
	for (i = 0; i < func->num_args; ++i) {
		if (func->arg_info[i].name)
			efree((void *)func->arg_info[i].name);

		if (func->arg_info[i].class_name)
			efree((void *)func->arg_info[i].class_name);
	}
	efree(func->arg_info);

	/* Free the function structure itself. */
	efree(func);
}

static int merge_class_dict(PyObject *o, HashTable *ht)
{
	PyObject *d;
	PyObject *bases;

	PHP_PYTHON_THREAD_ASSERT();

	/* We assume that the Python object is a class type. */
	assert(PyClass_Check(o));

	/*
	 * Start by attempting to merge the contents of the class type's __dict__.
	 * It's alright if the class type doesn't have a __dict__ attribute.
	 */
	d = PyObject_GetAttrString(o, "__dict__");
	if (d == NULL)
		PyErr_Clear();
	else {
		int result = pip_mapping_to_hash(d, ht);
		Py_DECREF(d);
		if (result != SUCCESS)
			return FAILURE;
	}

	/*
	 * We now attempt to recursively merge in the base types' __dicts__s.
	 */
	bases = PyObject_GetAttrString(o, "__bases__");
	if (bases == NULL)
		PyErr_Clear();
	else {
		Py_ssize_t i, n;
		n = PySequence_Size(bases);
		if (n < 0)
			PyErr_Clear();
		else {
			for (i = 0; i < n; i++) {
				int status;
				PyObject *base = PySequence_GetItem(bases, i);
				if (base == NULL) {
					Py_DECREF(bases);
					return FAILURE;
				}

				/* Recurse through this base class. */
				status = merge_class_dict(base, ht);
				Py_DECREF(base);
				if (status != SUCCESS) {
					Py_DECREF(bases);
					return FAILURE;
				}
			}

		}

		Py_DECREF(bases);
	}

	return SUCCESS;
}

static int get_properties(PyObject *o, HashTable *ht)
{
	PyObject *attr;
	int status;

	PHP_PYTHON_THREAD_ASSERT();

	/*
	 * If the object supports the sequence or mapping protocol, just copy
	 * the container's contents into the output hashtable.
	 *
	 * XXX: This isn't strictly correct; it's a side-effect of our approach
	 * of treating all Python objects as PHP objects.  We'd really like PHP
	 * to attempt to attempt to "cast" our Python object to an array-like
	 * object first and then use the dimension APIs, in the spirit of
	 * Python's "duck typing", but those casting operations don't currently
	 * exist.  We now have the potential for false-positive conversions
	 * below, where we return the sequence or mapping contents of a Python
	 * object that also has a legitimate set of additional properties.
	 */
	if (PySequence_Check(o))
		return pip_sequence_to_hash(o, ht);

	if (PyMapping_Check(o))
		return pip_mapping_to_hash(o, ht);

	/*
	 * Attempt to append the contents of this object's __dict__ attribute to
	 * our hashtable.  If this object has no __dict__, we have no properties
	 * and return failure.
	 */
	attr = PyObject_GetAttrString(o, "__dict__");
	if (attr == NULL) {
		PyErr_Clear();
		return FAILURE;
	}

	status = pip_mapping_to_hash(attr, ht);
	Py_DECREF(attr);

	/*
	 * We also attempt to merge any properties inherited from our base
	 * class(es) into the final result.
	 */
	if (status == SUCCESS) {
		attr = PyObject_GetAttrString(o, "__class__");
		if (attr) {
			status = merge_class_dict(attr, ht);
			Py_DECREF(attr);
		}
	}

    return status;
}

static zval *python_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));

	zval *return_value = NULL;
	PyObject *attr;

	PHP_PYTHON_THREAD_ACQUIRE();

	convert_to_string_ex(member);

	attr = PyObject_GetAttrString(pip->obj, Z_STRVAL_P(member));
	if (attr) {
		pip_pyobject_to_zval(attr, rv);
		Py_DECREF(attr);
		return_value = rv;
	} else {
		return_value = &EG(uninitialized_zval);
		PyErr_Clear();
	}

	/* TODO: Do something with the 'type' parameter? */

	PHP_PYTHON_THREAD_RELEASE();

	return return_value;
}

static void python_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));
	PyObject *val;

	PHP_PYTHON_THREAD_ACQUIRE();
	
	val = pip_zval_to_pyobject(value);
	if (val) {
		convert_to_string_ex(member);
		if (PyObject_SetAttrString(pip->obj, Z_STRVAL_P(member), val) == -1) {
			PyErr_Clear();
			php_error(E_ERROR, "Python: Failed to set attribute %s", Z_STRVAL_P(member));
		}
	}

	PHP_PYTHON_THREAD_RELEASE();
}

static zval *python_read_dimension(zval *object, zval *offset, int type, zval *rv)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));
	zval *return_value = NULL;
	PyObject *item = NULL;

	PHP_PYTHON_THREAD_ACQUIRE();

	/*
	 * If we've been given a numeric value, start by attempting to use the
	 * sequence protocol.  The value may be a valid index.
	 */
	if (Z_TYPE_P(offset) == IS_LONG)
		item = PySequence_GetItem(pip->obj, Z_LVAL_P(offset));

	/*
	 * Otherwise, if this object provides the mapping protocol, our offset's
	 * string representation may be a key value.
	 */
	if (!item && PyMapping_Check(pip->obj)) {
		convert_to_string_ex(offset);
		item = PyMapping_GetItemString(pip->obj, Z_STRVAL_P(offset));
	}

	/* If we successfully fetched an item, return its PHP representation. */
	if (item) {
		pip_pyobject_to_zval(item, rv);
		Py_DECREF(item);
		return_value = rv;
	} else
		PyErr_Clear();
		return_value = &EG(uninitialized_zval);

	/* TODO: Do something with the 'type' parameter? */

	PHP_PYTHON_THREAD_RELEASE();

	return return_value;
}

static void python_write_dimension(zval *object, zval *offset, zval *value)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));
	PyObject *val;

	PHP_PYTHON_THREAD_ACQUIRE();

	val = pip_zval_to_pyobject(value);

	/*
	 * If this offset is a numeric value, we'll start by attempting to use
	 * the sequence protocol to set this item.
	 */
	if (Z_TYPE_P(offset) == IS_LONG && PySequence_Check(pip->obj)) {
		if (PySequence_SetItem(pip->obj, Z_LVAL_P(offset), val) == -1) {
			PyErr_Clear();
			php_error(E_ERROR, "Python: Failed to set sequence item %ld", Z_LVAL_P(offset));
		}
	}

	/*
	 * Otherwise, if this object supports the mapping protocol, use the string
	 * representation of the offset as the key value.
	 */
	else if (PyMapping_Check(pip->obj)) {
		convert_to_string_ex(offset);
		if (PyMapping_SetItemString(pip->obj, Z_STRVAL_P(offset), val) == -1) {
			PyErr_Clear();
			php_error(E_ERROR, "Python: Failed to set mapping item '%s'", Z_STRVAL_P(offset));
		}
	}

	Py_XDECREF(val);

	PHP_PYTHON_THREAD_RELEASE();
}

static int python_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));
	PyObject *attr;
	int exists = 0;

	/* We're only concerned with the string representation of this value. */
	convert_to_string_ex(member);

	PHP_PYTHON_THREAD_ACQUIRE();

	attr = PyObject_GetAttrString(pip->obj, Z_STRVAL_P(member));
	if (!attr) {
		PHP_PYTHON_THREAD_RELEASE();
		return exists;
	}

	switch (has_set_exists) {
	case 0:
		exists = (PyObject_IsTrue(attr) == 1);
		break;
	default:
		exists = (attr != Py_None);
		break;
	case 2:
		exists = 1;
		break;
	}

	Py_DECREF(attr);

	PHP_PYTHON_THREAD_RELEASE();

	return exists;
}

static int python_has_dimension(zval *object, zval *member, int check_empty)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));
	PyObject *item = NULL;
	int ret = 0;

	PHP_PYTHON_THREAD_ACQUIRE();

	/*
	 * If we've been handed an integer value, check if this is a valid item
	 * index.  PySequence_GetItem() will perform a PySequence_Check() test.
	 */
	if (Z_TYPE_P(member) == IS_LONG)
		item = PySequence_GetItem(pip->obj, Z_LVAL_P(member));

	/*
	 * Otherwise, check if this object provides the mapping protocol.  If it
	 * does, check if the string representation of our value is a valid key.
	 */
	if (!item && PyMapping_Check(pip->obj)) {
		convert_to_string_ex(member);
		item = PyMapping_GetItemString(pip->obj, Z_STRVAL_P(member));
	}

	/*
	 * If we have a valid item at this point, we have a chance at success.  The
	 * last thing we need to consider is whether or not the item's value is
	 * considered "true" if check_empty has been specified.  We use Python's
	 * notion of truth here for consistency, although it may be more correct to
	 * use PHP's notion of truth (as determined by zend_is_true()) should we
	 * encountered problems with this in the future.
	 */
	if (item) {
		ret = (check_empty) ? (PyObject_IsTrue(item) == 1) : 1;
		Py_DECREF(item);
	} else
		PyErr_Clear();

	PHP_PYTHON_THREAD_RELEASE();

	return ret;
}

static void python_unset_property(zval *object, zval *member, void **cache_slot)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));

	PHP_PYTHON_THREAD_ACQUIRE();

	convert_to_string_ex(member);

	if (pip->std.properties) {
		zend_hash_del(pip->std.properties, Z_STR_P(member));
	}

	if (PyObject_DelAttrString(pip->obj, Z_STRVAL_P(member)) == -1) {
		PyErr_Clear();
		php_error(E_ERROR, "Python: Failed to delete attribute '%s'", Z_STRVAL_P(member));
	}

	PHP_PYTHON_THREAD_RELEASE();
}

static void python_unset_dimension(zval *object, zval *offset)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));
	int deleted = 0;

	PHP_PYTHON_THREAD_ACQUIRE();

	/*
	 * If we've been given a numeric offset and this object provides the
	 * sequence protocol, attempt to delete the item using a sequence index.
	 */
	if (Z_TYPE_P(offset) == IS_LONG && PySequence_Check(pip->obj)) {
		deleted = PySequence_DelItem(pip->obj, Z_LVAL_P(offset)) != -1;

		if (pip->std.properties) {
			zend_hash_index_del(pip->std.properties, Z_LVAL_P(offset));
		}
	}

	/*
	 * If we failed to delete the item using the sequence protocol, use the
	 * offset's string representation and try the mapping protocol.
	 */
	if (PyMapping_Check(pip->obj)) {
		convert_to_string_ex(offset);
		deleted = PyMapping_DelItemString(pip->obj,Z_STRVAL_P(offset)) != -1;

		if (pip->std.properties) {
			zend_hash_del(pip->std.properties, Z_STR_P(offset));
		}
	}

	/* If we still haven't deleted the requested item, trigger an error. */
	if (!deleted) {
		PyErr_Clear();
		php_error(E_ERROR, "Python: Failed to delete item");
	}

	PHP_PYTHON_THREAD_RELEASE();
}

static HashTable *python_get_properties(zval *object)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));

	PHP_PYTHON_THREAD_ACQUIRE();

	if (!pip->std.properties) {
		rebuild_object_properties(&pip->std);
	} else {
		zend_hash_clean(pip->std.properties);
	}

	//if (zend_hash_num_elements(pip->std.properties) == 0)
	get_properties(pip->obj, pip->std.properties);

	PHP_PYTHON_THREAD_RELEASE();

    return pip->std.properties;
}

static union _zend_function *python_get_method(zend_object **object, zend_string *method, const zval *key)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(*object);
	zend_internal_function *f;
	PyObject *func;

	PHP_PYTHON_THREAD_ACQUIRE();

	/* Quickly check if this object has a method with the requested name. */
	if (PyObject_HasAttrString(pip->obj, ZSTR_VAL(method)) == 0) {
		PHP_PYTHON_THREAD_RELEASE();
		return NULL;
	}

	/* Attempt to fetch the requested method and verify that it's callable. */
	func = PyObject_GetAttrString(pip->obj, ZSTR_VAL(method));
	if (!func || PyMethod_Check(func) == 0 || PyCallable_Check(func) == 0) {
		Py_XDECREF(func);
		PHP_PYTHON_THREAD_RELEASE();
		return NULL;
	}

	/*
	 * Set up the function call structure for this method invokation.  We
	 * allocate a bit of memory here which will be released later on in
	 * python_call_method().
	 */
	f = emalloc(sizeof(zend_internal_function));
	memset(f, 0, sizeof(zend_internal_function));
	f->type = ZEND_OVERLOADED_FUNCTION_TEMPORARY;
	f->function_name = zend_string_dup(method, 0);
	f->scope = pip->std.ce;
	f->fn_flags = 0;
	f->prototype = NULL;
	f->num_args = python_get_arg_info(func, &(f->arg_info));
	zend_set_function_arg_flags((zend_function*)f);

	Py_DECREF(func);

	PHP_PYTHON_THREAD_RELEASE();

	return (union _zend_function *)f;
}

static int python_call_method(zend_string *method_name, zend_object *object, INTERNAL_FUNCTION_PARAMETERS)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(object);
	PyObject *method;
	int ret = FAILURE;

	PHP_PYTHON_THREAD_ACQUIRE();

	/* Get a pointer to the requested method from this object. */
	method = PyObject_GetAttrString(pip->obj, ZSTR_VAL(method_name));

	/* If the method exists and is callable ... */
	if (method && PyMethod_Check(method) && PyCallable_Check(method)) {
		PyObject *args, *result;

		/* Convert all of our PHP arguments into a Python-digestable tuple. */
		args = pip_args_to_tuple(ZEND_NUM_ARGS(), 0);

		/*
		 * Invoke the requested method and store the result.  If we have a
		 * tuple of arguments, remember to free (decref) it.
		 */
		result = PyObject_CallObject(method, args); 
		Py_DECREF(method);
		Py_XDECREF(args);

		if (result) {
			ret = pip_pyobject_to_zval(result, return_value);
			Py_DECREF(result);
		}
	}

	PHP_PYTHON_THREAD_RELEASE();

	/* Release the memory that we allocated for this function in method_get. */
	efree_function((zend_internal_function *)EG(current_execute_data)->func);

	return ret;
}
/*
static union _zend_function *python_constructor_get(zval *object)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));
	return pip->std.ce->constructor;
}
*/
static zend_string* python_get_class_name(const zend_object *object)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj((zend_object*)object);
	const char * const key = "__class__";
	PyObject *attr, *str;
	zend_string *retval;
	char *class_name = NULL;
	zend_ulong class_name_len = 0;

	PHP_PYTHON_THREAD_ACQUIRE();

	/*
	 * Attempt to use the Python object instance's special read-only attributes
	 * to determine object's class name.  We use __class__ unless we've been
	 * asked for the name of our parent, in which case we use __module__.  We
	 * prefix the class name with "Python " to avoid confusion with native PHP
	 * classes.
	 */
	if ((attr = PyObject_GetAttrString(pip->obj, key))) {
		if ((str = PyObject_Str(attr))) {
			class_name_len = sizeof("Python ") + PyString_GET_SIZE(str);
			class_name = (char *)emalloc(sizeof(char) * class_name_len);
			zend_sprintf(class_name, "Python %s", PyString_AS_STRING(str));
			Py_DECREF(str);
		}
		Py_DECREF(attr);
	}

	/* If we still don't have a string, use the PHP class entry's name. */
	if (class_name_len == 0) {
		class_name = estrndup(ZSTR_VAL(pip->std.ce->name), ZSTR_LEN(pip->std.ce->name));
		class_name_len = ZSTR_LEN(pip->std.ce->name);
	}

	PHP_PYTHON_THREAD_RELEASE();

	retval = zend_string_init(class_name, class_name_len, 0);
	efree(class_name);
	return retval;
}

static int python_compare(zval *object1, zval *object2)
{
	phalcon_py_object_object *a = phalcon_py_object_object_from_obj(Z_OBJ_P(object1));
	phalcon_py_object_object *b = phalcon_py_object_object_from_obj(Z_OBJ_P(object2));
	int cmp;

	PHP_PYTHON_THREAD_ACQUIRE();
	cmp = PyObject_Compare(a->obj, b->obj);
	PHP_PYTHON_THREAD_RELEASE();

	return cmp;
}

static int python_cast(zval *readobj, zval *writeobj, int type)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(readobj));
	PyObject *val = NULL;
	int ret = FAILURE;

	PHP_PYTHON_THREAD_ACQUIRE();

	switch (type) {
		case IS_STRING:
			val = PyObject_Str(pip->obj);
			break;
		default:
			return FAILURE;
	}

	if (val) {
		ret = pip_pyobject_to_zval(val, writeobj);
		Py_DECREF(val);
	}

	PHP_PYTHON_THREAD_RELEASE();

	return ret;
}

static int python_count_elements(zval *object, long *count)
{
	phalcon_py_object_object *pip = phalcon_py_object_object_from_obj(Z_OBJ_P(object));
	int len, result = FAILURE;

	PHP_PYTHON_THREAD_ACQUIRE();

	len = PyObject_Length(pip->obj);
	if (len != -1) {
		*count = len;
		result = SUCCESS;
	}

	PHP_PYTHON_THREAD_RELEASE();

	return result;
}

/**
 * Phalcon\Py\Object initializer
 */
PHALCON_INIT_CLASS(Phalcon_Py_Object){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Py, Object, py_object, phalcon_py_object_method_entry, 0);

	phalcon_py_object_object_handlers.read_property = python_read_property;
	phalcon_py_object_object_handlers.write_property = python_write_property;
	phalcon_py_object_object_handlers.read_dimension = python_read_dimension;
	phalcon_py_object_object_handlers.write_dimension = python_write_dimension;
	phalcon_py_object_object_handlers.has_property = python_has_property;
	phalcon_py_object_object_handlers.unset_property = python_unset_property;
	phalcon_py_object_object_handlers.has_dimension = python_has_dimension;
	phalcon_py_object_object_handlers.unset_dimension = python_unset_dimension;
	phalcon_py_object_object_handlers.get_properties = python_get_properties;
	phalcon_py_object_object_handlers.get_method = python_get_method;
	phalcon_py_object_object_handlers.call_method = python_call_method;
	//phalcon_py_object_object_handlers.get_constructor = python_constructor_get;
	phalcon_py_object_object_handlers.get_class_name = python_get_class_name;
	phalcon_py_object_object_handlers.compare_objects = python_compare;
	phalcon_py_object_object_handlers.cast_object = python_cast;
	phalcon_py_object_object_handlers.count_elements = python_count_elements;

	return SUCCESS;
}

/**
 * Phalcon\Py\Object constructor
 *
 */
PHP_METHOD(Phalcon_Py_Object, __construct)
{
	zval *module_name, *class_name;
	phalcon_py_object_object *object_intern;
	PyObject *module;

	phalcon_fetch_params(0, 2, 0, &module_name, &class_name);

	object_intern = phalcon_py_object_object_from_obj(Z_OBJ_P(getThis()));

	PHP_PYTHON_THREAD_ACQUIRE();

	module = PyImport_ImportModule(Z_STRVAL_P(module_name));
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
