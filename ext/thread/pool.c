
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

#include "thread/pool.h"
#include "thread/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/debug.h"

#include "kernel/thread/pool.h"
#include "kernel/thread/pool.h"

/**
 * Phalcon\Thread\Pool
 *
 *<code>
 *
 * $pool = new Phalcon\Thread\Pool(2);
 * $pool->add(function(){ echo 'Hello world!';});
 *
 *</code>
 */
zend_class_entry *phalcon_thread_pool_ce;

PHP_METHOD(Phalcon_Thread_Pool, __construct);
PHP_METHOD(Phalcon_Thread_Pool, getNumThreads);
PHP_METHOD(Phalcon_Thread_Pool, inc);
PHP_METHOD(Phalcon_Thread_Pool, dec);
PHP_METHOD(Phalcon_Thread_Pool, add);
PHP_METHOD(Phalcon_Thread_Pool, wait);
PHP_METHOD(Phalcon_Thread_Pool, destroy);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_thread_pool___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, num, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_thread_pool_inc, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, num, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_thread_pool_dec, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, num, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_thread_pool_add, 0, 0, 1)
	ZEND_ARG_CALLABLE_INFO(0, work, 0)
	ZEND_ARG_TYPE_INFO(0, args, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_thread_pool_method_entry[] = {
	PHP_ME(Phalcon_Thread_Pool, __construct, arginfo_phalcon_thread_pool___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Thread_Pool, getNumThreads, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Thread_Pool, inc, arginfo_phalcon_thread_pool_inc, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Thread_Pool, dec, arginfo_phalcon_thread_pool_dec, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Thread_Pool, add, arginfo_phalcon_thread_pool_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Thread_Pool, wait, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Thread_Pool, destroy, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_thread_pool_object_handlers;
zend_object* phalcon_thread_pool_object_create_handler(zend_class_entry *ce)
{
	phalcon_thread_pool_object *intern = ecalloc(1, sizeof(phalcon_thread_pool_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_thread_pool_object_handlers;

	return &intern->std;
}

void phalcon_thread_pool_object_free_handler(zend_object *object)
{
	phalcon_thread_pool_object *intern;

	intern = phalcon_thread_pool_object_from_obj(object);

	if (intern->pool) {
		phalcon_thread_pool_destroy(intern->pool, 1);
		intern->pool = NULL;
	}
	zend_object_std_dtor(object);
}

/**
 * Phalcon\Thread\Thread initializer
 */
PHALCON_INIT_CLASS(Phalcon_Thread_Pool){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Thread, Pool, thread_pool, phalcon_thread_pool_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Thread\Pool constructor
 *
 * @param int $num
 * @throws \Phalcon\Thread\Exception
 */
PHP_METHOD(Phalcon_Thread_Pool, __construct){

	zval *num = NULL;
	phalcon_thread_pool_object *intern;
	int num_worker_threads = 1;

	phalcon_fetch_params(0, 0, 1, &num);

	intern = phalcon_thread_pool_object_from_obj(Z_OBJ_P(getThis()));

	if (num && Z_TYPE_P(num) == IS_LONG) {
		num_worker_threads = Z_LVAL_P(num) ;
	}

	if (num_worker_threads <= 0) {
		num_worker_threads = 1;
	}

	if (num_worker_threads > 1024) {
		num_worker_threads = 1024;
	}

	intern->pool = phalcon_thread_pool_init(num_worker_threads);
	if (!intern->pool) {
		return;
	}
}

/**
 * 
 * @return int
 */
PHP_METHOD(Phalcon_Thread_Pool, getNumThreads){

	phalcon_thread_pool_object *intern;

	intern = phalcon_thread_pool_object_from_obj(Z_OBJ_P(getThis()));

	RETURN_LONG(intern->pool->num_threads);
}

/**
 * 
 * @param int $num
 * @return boolean
 */
PHP_METHOD(Phalcon_Thread_Pool, inc){

	zval *num = NULL;
	phalcon_thread_pool_object *intern;
	int num_worker_threads = 1;

	phalcon_fetch_params(0, 0, 1, &num);

	num_worker_threads = Z_LVAL_P(num) ;

	if (num_worker_threads <= 0) {
		num_worker_threads = 1;
	}

	intern = phalcon_thread_pool_object_from_obj(Z_OBJ_P(getThis()));

	if (!phalcon_thread_pool_inc_threads(intern->pool, num_worker_threads)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * 
 * @param int $num
 * @return boolean
 */
PHP_METHOD(Phalcon_Thread_Pool, dec){

	zval *num = NULL;
	phalcon_thread_pool_object *intern;
	int num_worker_threads = 1;

	phalcon_fetch_params(0, 0, 1, &num);

	num_worker_threads = Z_LVAL_P(num) ;

	if (num_worker_threads <= 0) {
		num_worker_threads = 1;
	}

	intern = phalcon_thread_pool_object_from_obj(Z_OBJ_P(getThis()));

	if (!phalcon_thread_pool_dec_threads(intern->pool, num_worker_threads)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * 
 *
 */
PHP_METHOD(Phalcon_Thread_Pool, add){

	zval *work, *args = NULL;
	phalcon_thread_pool_object *intern;

	phalcon_fetch_params(0, 1, 1, &work, &args);

	intern = phalcon_thread_pool_object_from_obj(Z_OBJ_P(getThis()));
	phalcon_thread_pool_add_work(intern->pool, work, args);
}

/**
 * 
 *
 */
PHP_METHOD(Phalcon_Thread_Pool, wait){

	phalcon_thread_pool_object *intern;

	intern = phalcon_thread_pool_object_from_obj(Z_OBJ_P(getThis()));

	phalcon_thread_pool_destroy(intern->pool, 1);
	intern->pool = NULL;
}

/**
 * 
 *
 */
PHP_METHOD(Phalcon_Thread_Pool, destroy){

	phalcon_thread_pool_object *intern;

	intern = phalcon_thread_pool_object_from_obj(Z_OBJ_P(getThis()));

	phalcon_thread_pool_destroy(intern->pool, 0);
	intern->pool = NULL;
}
