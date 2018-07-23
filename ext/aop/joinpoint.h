
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
  |          Julien Salleyron <julien.salleyron@gmail.com>                 |
  |          <pangudashu@gmail.com>                                        |
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_AOP_JOINPOINT_H
#define PHALCON_AOP_JOINPOINT_H

#include "php_phalcon.h"

#include "aop.h"

typedef struct {
    phalcon_aop_pointcut *current_pointcut;
    zend_array *advice;
    HashPosition pos;
    int kind_of_advice;
    zend_execute_data *ex;
    int is_ex_executed;
    zend_object *exception;
    
    zval *args;
    zval *return_value;
    int return_value_changed;

    zval *object;
    zval *member;
    int type;
    void **cache_slot;
    zval *rv;
    zval property_value;
	zend_object std;
} phalcon_aop_joinpoint_object;

static inline phalcon_aop_joinpoint_object *phalcon_aop_joinpoint_object_from_obj(zend_object *obj) {
	return (phalcon_aop_joinpoint_object*)((char*)(obj) - XtOffsetOf(phalcon_aop_joinpoint_object, std));
}

extern zend_class_entry *phalcon_aop_joinpoint_ce;

PHALCON_INIT_CLASS(Phalcon_Aop_Joinpoint);

#endif /* PHALCON_AOP_JOINPOINT_H */
