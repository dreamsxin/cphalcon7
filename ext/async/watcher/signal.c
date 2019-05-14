
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
  |          Martin Schröder <m.schroeder2007@gmail.com>                   |
  +------------------------------------------------------------------------+
*/

#include "async/core.h"
#include "kernel/backend.h"

#if PHALCON_USE_UV

ASYNC_API zend_class_entry *async_signal_ce;

static zend_object_handlers async_signal_handlers;

#define ASYNC_SIGNAL_CONST(const_name, value) \
	zend_declare_class_constant_long(async_signal_ce, const_name, sizeof(const_name)-1, (zend_long)value);


typedef struct {
	/* PHP object handle. */
	zend_object std;

	/* Close error (undef by default). */
	zval error;

	int signum;

	uv_signal_t handle;

	async_op_list observers;

	zend_uchar ref_count;

	async_task_scheduler *scheduler;
	
	async_cancel_cb cancel;
} async_signal;

ASYNC_CALLBACK trigger_signal(uv_signal_t *handle, int signum)
{
	async_signal *signal;
	async_op *op;
	async_op *last;
	zend_bool cont;

	signal = (async_signal *) handle->data;

	ZEND_ASSERT(signal != NULL);

	if (EXPECTED(Z_TYPE_P(&signal->error) == IS_UNDEF)) {
		last = signal->observers.last;
		cont = 1;
	
		while (cont && signal->observers.first != NULL) {
			ASYNC_NEXT_OP(&signal->observers, op);
			
			cont = (op != last);
			
			ASYNC_FINISH_OP(op);
		}
	} else {
		while (signal->observers.first != NULL) {
			ASYNC_NEXT_OP(&signal->observers, op);
			ASYNC_FAIL_OP(op, &signal->error);
		}
	}
	
	if (signal->observers.first == NULL) {
		if (!uv_is_closing((uv_handle_t *) handle)) {
			uv_signal_stop(handle);
		}
	}
}

ASYNC_CALLBACK close_signal(uv_handle_t *handle)
{
	async_signal *signal;

	signal = (async_signal *) handle->data;

	ZEND_ASSERT(signal != NULL);

	ASYNC_DELREF(&signal->std);
}

ASYNC_CALLBACK shutdown_signal(void *obj, zval *error)
{
	async_signal *signal;
	async_op *op;

	signal = (async_signal *) obj;

	ZEND_ASSERT(signal != NULL);
	
	signal->cancel.func = NULL;
	
	if (error != NULL && Z_TYPE_P(&signal->error) == IS_UNDEF) {
		ZVAL_COPY(&signal->error, error);
	}

	if (!uv_is_closing((uv_handle_t *) &signal->handle)) {
		ASYNC_ADDREF(&signal->std);

		uv_close((uv_handle_t *) &signal->handle, close_signal);
	}
	
	if (error != NULL) {
		while (signal->observers.first != NULL) {
			ASYNC_NEXT_OP(&signal->observers, op);
			ASYNC_FAIL_OP(op, &signal->error);
		}
	}
}


static zend_object *async_signal_object_create(zend_class_entry *ce)
{
	async_signal *signal;

	signal = ecalloc(1, sizeof(async_signal));

	zend_object_std_init(&signal->std, ce);
	signal->std.handlers = &async_signal_handlers;

	ZVAL_UNDEF(&signal->error);
	
	signal->scheduler = async_task_scheduler_ref();
	
	signal->cancel.object = signal;
	signal->cancel.func = shutdown_signal;

	return &signal->std;
}

static void async_signal_object_dtor(zend_object *object)
{
	async_signal *signal;

	signal = (async_signal *) object;
	
	if (signal->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&signal->scheduler->shutdown, &signal->cancel);
		
		signal->cancel.func(signal, NULL);
	}
}

static void async_signal_object_destroy(zend_object *object)
{
	async_signal *signal;

	signal = (async_signal *) object;

	zval_ptr_dtor(&signal->error);
	
	async_task_scheduler_unref(signal->scheduler);

	zend_object_std_dtor(&signal->std);
}

ZEND_METHOD(Signal, __construct)
{
	async_signal *signal;
	zend_long signum;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
	    Z_PARAM_LONG(signum)
	ZEND_PARSE_PARAMETERS_END();

	ASYNC_CHECK_ERROR(!ASYNC_G(cli), "Signal support is only available in CLI mode");

	signal = (async_signal *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(signum < 1, "Invalid signal number: %d", (int) signum);

	signal->signum = (int) signum;

	uv_signal_init(&signal->scheduler->loop, &signal->handle);
	uv_unref((uv_handle_t *) &signal->handle);

	signal->handle.data = signal;
	
	ASYNC_LIST_APPEND(&signal->scheduler->shutdown, &signal->cancel);
}

ZEND_METHOD(Signal, close)
{
	async_signal *signal;

	zval error;
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	signal = (async_signal *) Z_OBJ_P(getThis());

	if (signal->cancel.func == NULL) {
		return;
	}
	
	ASYNC_PREPARE_ERROR(&error, "Signal has been closed");
	
	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}
	
	ASYNC_LIST_REMOVE(&signal->scheduler->shutdown, &signal->cancel);
	
	signal->cancel.func(signal, &error);
	
	zval_ptr_dtor(&error);
}

ZEND_METHOD(Signal, awaitSignal)
{
	async_signal *signal;
	async_op *op;
	async_context *context;

	ZEND_PARSE_PARAMETERS_NONE();

	signal = (async_signal *) Z_OBJ_P(getThis());

	if (UNEXPECTED(Z_TYPE_P(&signal->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&signal->error);
		return;
	}
	
	if (signal->observers.first == NULL && !uv_is_active((uv_handle_t *) &signal->handle)) {
		uv_signal_start(&signal->handle, trigger_signal, signal->signum);
	}

	context = async_context_get();
	
	ASYNC_ALLOC_OP(op);
	ASYNC_APPEND_OP(&signal->observers, op);

	ASYNC_UNREF_ENTER(context, signal);

	if (UNEXPECTED(async_await_op(op) == FAILURE)) {
		ASYNC_FORWARD_OP_ERROR(op);
	}

	ASYNC_UNREF_EXIT(context, signal);
	ASYNC_FREE_OP(op);
}

static int is_signal_supported(int signum)
{
	if (EXPECTED(signum > 0 && ASYNC_G(cli))) {
		switch (signum) {
		case ASYNC_SIGNAL_SIGHUP:
		case ASYNC_SIGNAL_SIGINT:
		case ASYNC_SIGNAL_SIGQUIT:
		case ASYNC_SIGNAL_SIGKILL:
		case ASYNC_SIGNAL_SIGTERM:
		case ASYNC_SIGNAL_SIGUSR1:
		case ASYNC_SIGNAL_SIGUSR2:
			return 1;
		}
	}
	
	return 0;
}

ZEND_METHOD(Signal, isSupported)
{
	zend_long tmp;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(tmp)
	ZEND_PARSE_PARAMETERS_END();

	if (is_signal_supported((int) tmp)) {
		RETURN_TRUE;
	}
	
	RETURN_FALSE;
}

ZEND_METHOD(Signal, raise)
{
	zend_long tmp;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(tmp)
	ZEND_PARSE_PARAMETERS_END();
	
#ifdef PHP_WIN32
	zend_throw_error(NULL, "Cannot raise a signal when running on Windows");
#else
	int code;
	
	code = raise((int) tmp);
	
	ASYNC_CHECK_ERROR(code != 0, "Failed to raise signal %d: %s", (int) tmp, uv_strerror(uv_translate_sys_error(code)));
#endif
}

ZEND_METHOD(Signal, signal)
{
	zend_long pid;
	zend_long sig;
	
	int signum;
	int code;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(pid)
		Z_PARAM_LONG(sig)
	ZEND_PARSE_PARAMETERS_END();
	
	signum = (int) sig;
	
	ASYNC_CHECK_ERROR(is_signal_supported(signum), "Signal %d is not supported", signum);
	
	code = uv_kill((int) pid, signum);
	
	if (UNEXPECTED(code < 0)) {
		zend_throw_error(NULL, "Failed to signal process %d: %s", (int) pid, uv_strerror(code));
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_signal_ctor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_signal_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_signal_await_signal, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_signal_is_supported, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_signal_raise, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_signal_signal, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, pid, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_signal_close, 0, 0, IS_VOID, NULL, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_signal_await_signal, 0, 0, IS_VOID, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_signal_is_supported, 0, 1, _IS_BOOL, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_signal_raise, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_signal_signal, 0, 2, IS_VOID, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, pid, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

static const zend_function_entry async_signal_functions[] = {
	ZEND_ME(Signal, __construct, arginfo_signal_ctor, ZEND_ACC_PUBLIC)
	ZEND_ME(Signal, close, arginfo_signal_close, ZEND_ACC_PUBLIC)
	ZEND_ME(Signal, awaitSignal, arginfo_signal_await_signal, ZEND_ACC_PUBLIC)
	ZEND_ME(Signal, isSupported, arginfo_signal_is_supported, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Signal, raise, arginfo_signal_raise, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Signal, signal, arginfo_signal_signal, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_FE_END
};


void async_signal_ce_register()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Signal", async_signal_functions);
	async_signal_ce = zend_register_internal_class(&ce);
	async_signal_ce->ce_flags |= ZEND_ACC_FINAL;
	async_signal_ce->create_object = async_signal_object_create;
	async_signal_ce->serialize = zend_class_serialize_deny;
	async_signal_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_signal_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_signal_handlers.free_obj = async_signal_object_destroy;
	async_signal_handlers.dtor_obj = async_signal_object_dtor;
	async_signal_handlers.clone_obj = NULL;

	ASYNC_SIGNAL_CONST("SIGHUP", ASYNC_SIGNAL_SIGHUP);
	ASYNC_SIGNAL_CONST("SIGINT", ASYNC_SIGNAL_SIGINT);
	ASYNC_SIGNAL_CONST("SIGQUIT", ASYNC_SIGNAL_SIGQUIT);
	ASYNC_SIGNAL_CONST("SIGKILL", ASYNC_SIGNAL_SIGKILL);
	ASYNC_SIGNAL_CONST("SIGTERM", ASYNC_SIGNAL_SIGTERM);
	ASYNC_SIGNAL_CONST("SIGUSR1", ASYNC_SIGNAL_SIGUSR1);
	ASYNC_SIGNAL_CONST("SIGUSR2", ASYNC_SIGNAL_SIGUSR2);
}

#endif
