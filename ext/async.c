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

#include "async.h"
#include "exception.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/debug.h"

/**
 * Phalcon\Async
 */
zend_class_entry *phalcon_async_ce;

PHP_METHOD(Phalcon_Async, call);
PHP_METHOD(Phalcon_Async, recv);
PHP_METHOD(Phalcon_Async, recvAll);
PHP_METHOD(Phalcon_Async, count);
PHP_METHOD(Phalcon_Async, clear);
PHP_METHOD(Phalcon_Async, setFilename);
PHP_METHOD(Phalcon_Async, getFilename);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_async_call, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, closure, Closure, 0)
	ZEND_ARG_TYPE_INFO(0, arguments, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_async_recv, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, pid, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flag, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_async_recvall, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, flag, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_async_setfilename, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_async_method_entry[] = {
	PHP_ME(Phalcon_Async, call, arginfo_phalcon_async_call, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Async, recv, arginfo_phalcon_async_recv, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Async, recvAll, arginfo_phalcon_async_recvall, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Async, count, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Async, clear, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Async, setFilename, arginfo_phalcon_async_setfilename, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Async, getFilename, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Async initializer
 */
PHALCON_INIT_CLASS(Phalcon_Async){

	PHALCON_REGISTER_CLASS(Phalcon, Async, async, phalcon_async_method_entry, 0);

	zend_declare_property_long(phalcon_async_ce, SL("_num"), 0, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_async_ce, SL("_filename"), __FILE__, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_async_ce, SL("_proj"), "a", ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	zend_declare_class_constant_long(phalcon_async_ce, SL("NOWAIT"),		PHALCON_ASYNC_NOWAIT);
	zend_declare_class_constant_long(phalcon_async_ce, SL("MSG_NOERROR"),	PHALCON_ASYNC_MSG_NOERROR);
	zend_declare_class_constant_long(phalcon_async_ce, SL("MSG_EXCEPT"),	PHALCON_ASYNC_MSG_EXCEPT);

	return SUCCESS;
}

/**
 * Called asynchronous
 *
 *<code>
 *	$async = Phalcon\Async::call(function () {
 *		return 'one';
 *	 });
 *</code>
 *
 * @param closure $callable
 * @return int
 */
PHP_METHOD(Phalcon_Async, call){

	zval *callable, *_arguments = NULL, arguments = {}, pid = {}, filename = {}, proj = {}, key = {}, seg = {}, result = {}, *sig;

	phalcon_fetch_params(1, 1, 1, &callable, &_arguments);

	if (Z_TYPE_P(callable) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(callable), zend_ce_closure)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_exception_ce, "Callable must be an closure object");
		return;
	}

	if (!_arguments || Z_TYPE_P(_arguments) != IS_ARRAY) {
		array_init(&arguments);
	} else {
		ZVAL_COPY(&arguments, _arguments);
	}
	PHALCON_MM_ADD_ENTRY(&arguments);

	PHALCON_MM_CALL_FUNCTION(&pid, "pcntl_fork");
	PHALCON_MM_ADD_ENTRY(&pid);

	if (PHALCON_LT_LONG(&pid, 0)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_exception_ce, "Callable must be an closure object");
		return;
	}

	if (PHALCON_GT_LONG(&pid, 0)) {
		phalcon_static_property_incr_ce(phalcon_async_ce, SL("_num"));
		RETURN_MM_CTOR(&pid);
	}

	phalcon_read_static_property_ce(&filename, phalcon_async_ce, SL("_filename"), PH_READONLY);
	phalcon_read_static_property_ce(&proj, phalcon_async_ce, SL("_proj"), PH_READONLY);

	PHALCON_MM_CALL_FUNCTION(&key, "ftok", &filename, &proj);
	PHALCON_MM_ADD_ENTRY(&key);
	PHALCON_MM_CALL_FUNCTION(&seg, "msg_get_queue", &key);
	PHALCON_MM_ADD_ENTRY(&seg);
	PHALCON_MM_CALL_USER_FUNC_ARRAY(&result, callable, &arguments);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_FUNCTION(&pid, "posix_getpid");
	PHALCON_MM_ADD_ENTRY(&pid);
	PHALCON_MM_CALL_FUNCTION(NULL, "msg_send", &seg, &pid, &result);

	if ((sig = zend_get_constant_str(SL("SIGKILL"))) != NULL ) {
		PHALCON_MM_CALL_FUNCTION(NULL, "posix_kill", &pid, sig);
	}
	RETURN_MM();
}

/**
 * Gets asynchronous result
 *
 *<code>
 *	$id = Phalcon\Async::call(function () {
 *		return 'one';
 *	});
 *	$data = Phalcon\Async::recv($id);
 *</code>
 *
 * @param int $pid
 * @param int $flag
 * @return mixed
 */
PHP_METHOD(Phalcon_Async, recv){

	zval *pid = NULL, *_flag = NULL, flag = {}, filename = {}, proj = {}, key = {}, seg = {}, type = {}, size = {}, message = {}, result = {};

	phalcon_fetch_params(1, 1, 1, &pid, &_flag);

	if (!_flag || Z_TYPE_P(_flag) == IS_NULL) {
		ZVAL_LONG(&flag, 0);
	}

	phalcon_read_static_property_ce(&filename, phalcon_async_ce, SL("_filename"), PH_READONLY);
	phalcon_read_static_property_ce(&proj, phalcon_async_ce, SL("_proj"), PH_READONLY);

	PHALCON_MM_CALL_FUNCTION(&key, "ftok", &filename, &proj);
	PHALCON_MM_ADD_ENTRY(&key);
	PHALCON_MM_CALL_FUNCTION(&seg, "msg_get_queue", &key);
	PHALCON_MM_ADD_ENTRY(&seg);

	ZVAL_LONG(&size, 1024);

	ZVAL_NULL(&type);
	ZVAL_NULL(&message);

	ZVAL_MAKE_REF(&type);
	ZVAL_MAKE_REF(&message);
	PHALCON_MM_CALL_FUNCTION(&result, "msg_receive", &seg, pid, &type, &size, &message, &PHALCON_GLOBAL(z_true), &flag);
	ZVAL_UNREF(&message);
	ZVAL_UNREF(&type);
	PHALCON_MM_ADD_ENTRY(&result);
	PHALCON_MM_ADD_ENTRY(&message);
	PHALCON_MM_ADD_ENTRY(&type);

	if (zend_is_true(&result)) {
		phalcon_static_property_decr_ce(phalcon_async_ce, SL("_num"));
	}

	RETURN_MM_CTOR(&message);
}

/**
 * Gets all asynchronous result
 *
 *<code>
 *	$id = Phalcon\Async::call(function () {
 *		return 'one';
 *	});
 *	$data = Phalcon\Async::recvAll();
 *</code>
 *
 * @return array
 */
PHP_METHOD(Phalcon_Async, recvAll){

	zval *_flag = NULL, flag = {}, num = {}, filename = {}, proj = {}, key = {}, seg = {}, pid = {}, size = {};
	int i = 0;

	phalcon_fetch_params(1, 0, 1, &_flag);

	if (!_flag || Z_TYPE_P(_flag) == IS_NULL) {
		ZVAL_LONG(&flag, 0);
	}

	phalcon_read_static_property_ce(&num, phalcon_async_ce, SL("_num"), PH_READONLY);
	phalcon_read_static_property_ce(&filename, phalcon_async_ce, SL("_filename"), PH_READONLY);
	phalcon_read_static_property_ce(&proj, phalcon_async_ce, SL("_proj"), PH_READONLY);

	i = phalcon_get_intval(&num);

	PHALCON_MM_CALL_FUNCTION(&key, "ftok", &filename, &proj);
	PHALCON_MM_ADD_ENTRY(&key);
	PHALCON_MM_CALL_FUNCTION(&seg, "msg_get_queue", &key);
	PHALCON_MM_ADD_ENTRY(&seg);

	ZVAL_LONG(&pid, 0);
	ZVAL_LONG(&size, 1024);

	array_init(return_value);

	while(i--) {
		zval type = {}, message = {}, result = {};
		ZVAL_NULL(&type);
		ZVAL_NULL(&message);

		ZVAL_MAKE_REF(&type);
		ZVAL_MAKE_REF(&message);

		PHALCON_MM_CALL_FUNCTION(&result, "msg_receive", &seg, &pid, &type, &size, &message, &PHALCON_GLOBAL(z_true), &flag);
		ZVAL_UNREF(&message);
		ZVAL_UNREF(&type);
		PHALCON_MM_ADD_ENTRY(&result);
		PHALCON_MM_ADD_ENTRY(&message);
		PHALCON_MM_ADD_ENTRY(&type);

		if (zend_is_true(&result)) {
			phalcon_static_property_decr_ce(phalcon_async_ce, SL("_num"));
		}

		phalcon_array_update(return_value, &type, &message, PH_COPY);
	}
	RETURN_MM();
}

/**
 * Gets result count
 *
 *<code>
 *	Phalcon\Async::count();
 *</code>
 *
 * @return int
 */
PHP_METHOD(Phalcon_Async, count){

	zval filename = {}, proj = {}, key = {}, seg = {}, result = {}, num = {};

	PHALCON_MM_INIT();

	phalcon_read_static_property_ce(&filename, phalcon_async_ce, SL("_filename"), PH_READONLY);
	phalcon_read_static_property_ce(&proj, phalcon_async_ce, SL("_proj"), PH_READONLY);

	PHALCON_MM_CALL_FUNCTION(&key, "ftok", &filename, &proj);
	PHALCON_MM_ADD_ENTRY(&key);
	PHALCON_MM_CALL_FUNCTION(&seg, "msg_get_queue", &key);
	PHALCON_MM_ADD_ENTRY(&seg);

	PHALCON_MM_CALL_FUNCTION(&result, "msg_stat_queue", &seg);
	PHALCON_MM_ADD_ENTRY(&result);

	if (Z_TYPE(result) == IS_ARRAY && phalcon_array_isset_fetch_str(&num, &result, SL("msg_qnum"), PH_READONLY)) {
		RETURN_MM_CTOR(&num);
	}
	RETURN_MM_FALSE;
}

/**
 * Destroy asynchronous
 *
 *<code>
 *	Phalcon\Async::clear();
 *</code>
 *
 * @return bool
 */
PHP_METHOD(Phalcon_Async, clear){

	zval filename = {}, proj = {}, key = {}, seg = {};

	PHALCON_MM_INIT();

	phalcon_read_static_property_ce(&filename, phalcon_async_ce, SL("_filename"), PH_READONLY);
	phalcon_read_static_property_ce(&proj, phalcon_async_ce, SL("_proj"), PH_READONLY);

	PHALCON_MM_CALL_FUNCTION(&key, "ftok", &filename, &proj);
	PHALCON_MM_ADD_ENTRY(&key);
	PHALCON_MM_CALL_FUNCTION(&seg, "msg_get_queue", &key);
	PHALCON_MM_ADD_ENTRY(&seg);

	PHALCON_MM_CALL_FUNCTION(return_value, "msg_remove_queue", &seg);
	RETURN_MM();
}

/**
 * Sets the filename to a System V IPC key
 *
 *<code>
 * Phalcon\Async::setFilename('/tmp');
 *</code>
 *
 * @param string $filename
 */
PHP_METHOD(Phalcon_Async, setFilename){

	zval *filename;

	phalcon_fetch_params(0, 1, 0, &filename);

	phalcon_update_static_property_ce(phalcon_async_ce, SL("_filename"), filename);
}

/**
 * Gets the filename
 *
 *<code>
 * Phalcon\Async::getFilename();
 *</code>
 *
 * @return string
 */
PHP_METHOD(Phalcon_Async, getFilename){

	phalcon_read_static_property_ce(return_value, phalcon_async_ce, SL("_filename"), PH_COPY);
}
