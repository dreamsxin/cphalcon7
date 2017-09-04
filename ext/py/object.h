
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
#ifndef PHALCON_PY_OBJECT_H
#define PHALCON_PY_OBJECT_H

#include "php_phalcon.h"

#include "py.h"

#if PHALCON_USE_PYTHON

typedef struct {
	PyObject *obj;
	zend_object std;
} phalcon_py_object_object;

static inline phalcon_py_object_object *phalcon_py_object_object_from_obj(zend_object *obj) {
	return (phalcon_py_object_object*)((char*)(obj) - XtOffsetOf(phalcon_py_object_object, std));
}

#endif

extern zend_class_entry *phalcon_py_object_ce;

PHALCON_INIT_CLASS(Phalcon_Py_Object);

#endif	/* PHALCON_PY_OBJECT_H */
