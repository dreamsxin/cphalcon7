/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Martin Schröder <m.schroeder2007@gmail.com>                 |
  +----------------------------------------------------------------------+
*/

#include "async/core.h"

#include "async/async_helper.h"
#include "async/async_fiber.h"
#include "async/async_event.h"

#include <Zend/zend_builtin_functions.h>
#include "kernel/backend.h"

ASYNC_API zend_class_entry *async_awaitable_ce;
ASYNC_API zend_class_entry *async_awaitable_impl_ce;

ASYNC_API zend_class_entry *async_component_ce;

ASYNC_API zend_class_entry *async_task_ce;
ASYNC_API zend_class_entry *async_task_scheduler_ce;

static zend_object_handlers async_awaitable_impl_handlers;

static zend_object_handlers async_task_handlers;
static zend_object_handlers async_task_scheduler_handlers;

static zend_uchar task_opcode;
static zend_try_catch_element task_terminate_try_catch_array = { 0, 1, 0, 0 };

static HashTable async_interceptors;
static HashTable await_handlers;

static async_await_handler task_await_handler;
static async_await_handler awaitable_impl_await_handler;

static void await_val(async_task *task, zval *val, INTERNAL_FUNCTION_PARAMETERS);
static void async_task_execute_inline(async_task *task, async_context *context);

static zend_always_inline void async_task_scheduler_enqueue(async_task *task);
static zend_always_inline void async_task_scheduler_run_loop(async_task_scheduler *scheduler);

static void dispose_ticks(async_task_scheduler *scheduler);

#define ASYNC_TASK_STATUS_INIT 0
#define ASYNC_TASK_STATUS_SUSPENDED 1
#define ASYNC_TASK_STATUS_RUNNING 2
#define ASYNC_TASK_STATUS_FINISHED ASYNC_OP_RESOLVED
#define ASYNC_TASK_STATUS_FAILED ASYNC_OP_FAILED

static zend_string *str_main;
static zend_string *str_status;
static zend_string *str_file;
static zend_string *str_line;

#define ASYNC_OP_CHECK_ERROR(op, expr, message, ...) do { \
    if (UNEXPECTED(expr)) { \
    	(op)->status = ASYNC_STATUS_FAILED; \
    	ASYNC_PREPARE_ERROR(&(op)->result, EG(current_execute_data), message ASYNC_VA_ARGS(__VA_ARGS__)); \
    	return FAILURE; \
    } \
} while (0)

typedef struct _async_component_factory {
	zval callable;
	zend_class_entry *ce;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} async_component_factory;


static zend_always_inline async_task *async_task_obj(zend_object *object)
{
	return (async_task *)((char *) object - XtOffsetOf(async_task, std));
}

static zend_always_inline uint32_t async_task_prop_offset(zend_string *name)
{
	return zend_get_property_info(async_task_ce, name, 1)->offset;
}

static zend_always_inline async_awaitable_impl *async_awaitable_impl_obj(zend_object *object)
{
	return (async_awaitable_impl *)((char *) object - XtOffsetOf(async_awaitable_impl, std));
}

static zend_always_inline uint32_t async_awaitable_impl_prop_offset(zend_string *name)
{
	return zend_get_property_info(async_awaitable_impl_ce, name, 1)->offset;
}

ASYNC_API void async_register_interceptor(zend_function *func, async_interceptor interceptor)
{
	ASYNC_CHECK_FATAL(func->type != ZEND_INTERNAL_FUNCTION, "Function %s cannot be intercepted because it is not internal", ZSTR_VAL(func->common.function_name));

	zend_hash_index_add_ptr(&async_interceptors, (zend_ulong) func, interceptor);
}

ASYNC_API void async_register_awaitable(zend_class_entry *ce, async_await_handler *handler)
{
	zend_hash_add_ptr(&await_handlers, ce->name, handler);
	zend_class_implements(ce, 1, async_awaitable_ce);
}

ASYNC_API async_await_handler *async_get_await_handler(zend_class_entry *ce)
{
	async_await_handler *handler;

	handler = (async_await_handler *) zend_hash_find_ptr(&await_handlers, ce->name);

	return handler;
}

static int implements_awaitable(zend_class_entry *ce, zend_class_entry *impl)
{
	if (zend_hash_find_ptr(&await_handlers, impl->name)) {
		return SUCCESS;
	}

	zend_error_noreturn(E_CORE_ERROR, "Interface %s cannot be implemented by class %s", ZSTR_VAL(ce->name), ZSTR_VAL(impl->name));

	return FAILURE;
}

static int task_await_delegate(zend_object *obj, zval *result, async_task *caller, async_context *context, int flags)
{
	async_task *task;

	task = async_task_obj(obj);

	if (task->scheduler != (caller ? caller->scheduler : async_task_scheduler_get())) {
		ASYNC_PREPARE_ERROR(result, EG(current_execute_data), "Cannot await a task that is running on a different scheduler");

		return ASYNC_OP_FAILED;
	}

	if (UNEXPECTED(flags & ASYNC_AWAIT_HANDLER_INLINE && task->fiber == NULL)) {
		if (caller == NULL || task->stack_size <= caller->stack_size) {
			async_task_execute_inline(task, context);
		}
	}

	switch (task->status) {
	case ASYNC_OP_RESOLVED:
	case ASYNC_OP_FAILED:
		ZVAL_COPY(result, &task->result);

		return (int) task->status;
	}

	return 0;
}

static void task_await_schedule(zend_object *obj, async_op *op, async_task *caller, async_context *context, int flags)
{
	async_task *task;

	task = async_task_obj(obj);

	ASYNC_APPEND_OP(&task->operations, op);
}

ASYNC_API async_awaitable_impl *async_create_awaitable(zend_execute_data *call, void *arg)
{
	async_awaitable_impl *awaitable;

	awaitable = ecalloc(1, sizeof(async_awaitable_impl) + zend_object_properties_size(async_awaitable_impl_ce));

	zend_object_std_init(&awaitable->std, async_awaitable_impl_ce);
	awaitable->std.handlers = &async_awaitable_impl_handlers;

	object_properties_init(&awaitable->std, async_awaitable_impl_ce);

	awaitable->arg = arg;

	ZVAL_STRING(OBJ_PROP(&awaitable->std, async_awaitable_impl_prop_offset(str_status)), async_status_label(awaitable->status));

	if (call != NULL && call->func && ZEND_USER_CODE(call->func->common.type)) {
		if (call->func->op_array.filename != NULL) {
			ZVAL_STR_COPY(OBJ_PROP(&awaitable->std, async_awaitable_impl_prop_offset(str_file)), call->func->op_array.filename);
		}

		ZVAL_LONG(OBJ_PROP(&awaitable->std, async_awaitable_impl_prop_offset(str_line)), call->opline->lineno);
	}

	return awaitable;
}

static void async_awaitable_impl_object_destroy(zend_object *object)
{
	async_awaitable_impl *awaitable;

	awaitable = async_awaitable_impl_obj(object);

	zval_ptr_dtor(&awaitable->result);

	zend_object_std_dtor(&awaitable->std);
}

ASYNC_API async_awaitable_impl *async_create_resolved_awaitable(zend_execute_data *call, zval *result)
{
	async_awaitable_impl *awaitable;

	zval *status;

	awaitable = async_create_awaitable(call, NULL);

	awaitable->status = ASYNC_OP_RESOLVED;

	if (result) {
		ZVAL_COPY(&awaitable->result, result);
	} else {
		ZVAL_NULL(&awaitable->result);
	}

	status = OBJ_PROP(&awaitable->std, async_awaitable_impl_prop_offset(str_status));
	zval_ptr_dtor(status);
	ZVAL_STRING(status, async_status_label(awaitable->status));

	return awaitable;
}

ASYNC_API async_awaitable_impl *async_create_failed_awaitable(zend_execute_data *call, zval *error)
{
	async_awaitable_impl *awaitable;

	zval *status;

	awaitable = async_create_awaitable(call, NULL);

	awaitable->status = ASYNC_OP_FAILED;

	ZVAL_COPY(&awaitable->result, error);

	status = OBJ_PROP(&awaitable->std, async_awaitable_impl_prop_offset(str_status));
	zval_ptr_dtor(status);
	ZVAL_STRING(status, async_status_label(awaitable->status));

	return awaitable;
}

ASYNC_API void async_awaitable_resolve(async_awaitable_impl *awaitable, zval *result)
{
	zval *status;

	if (awaitable->status == ASYNC_OP_PENDING) {
		awaitable->status = ASYNC_OP_RESOLVED;

		if (result) {
			ZVAL_COPY(&awaitable->result, result);
		} else {
			ZVAL_NULL(&awaitable->result);
		}

		status = OBJ_PROP(&awaitable->std, async_awaitable_impl_prop_offset(str_status));
		zval_ptr_dtor(status);
		ZVAL_STRING(status, async_status_label(awaitable->status));

		while (awaitable->operations.first) {
			ASYNC_RESOLVE_OP(awaitable->operations.first, &awaitable->result);
		}
	}
}

ASYNC_API void async_awaitable_fail(async_awaitable_impl *awaitable, zval *error)
{
	zval *status;

	if (awaitable->status == ASYNC_OP_PENDING) {
		awaitable->status = ASYNC_OP_FAILED;

		ZVAL_COPY(&awaitable->result, error);

		status = OBJ_PROP(&awaitable->std, async_awaitable_impl_prop_offset(str_status));
		zval_ptr_dtor(status);
		ZVAL_STRING(status, async_status_label(awaitable->status));

		while (awaitable->operations.first) {
			ASYNC_FAIL_OP(awaitable->operations.first, &awaitable->result);
		}
	}
}

static int awaitable_impl_delegate(zend_object *obj, zval *result, async_task *caller, async_context *context, int flags)
{
	async_awaitable_impl *awaitable;

	awaitable = async_awaitable_impl_obj(obj);

	switch (awaitable->status) {
	case ASYNC_OP_RESOLVED:
	case ASYNC_OP_FAILED:
		ZVAL_COPY(result, &awaitable->result);

		return (int) awaitable->status;
	}

	return 0;
}

static void awaitable_impl_schedule(zend_object *obj, async_op *op, async_task *caller, async_context *context, int flags)
{
	async_awaitable_impl *awaitable;

	awaitable = async_awaitable_impl_obj(obj);

	ASYNC_APPEND_OP(&awaitable->operations, op);
}

static zend_always_inline void populate_error_info(zval *error)
{
	zend_class_entry *base;

	zval tmp;

	base = instanceof_function(Z_OBJCE_P(error), zend_ce_exception) ? zend_ce_exception : zend_ce_error;
#if PHP_VERSION_ID >= 80000
	if (0 != zval_get_long(zend_read_property_ex(base, Z_OBJ_P(error), ZSTR_KNOWN(ZEND_STR_LINE), 1, &tmp))) {
		return;
	}

	ZVAL_STRING(&tmp, zend_get_executed_filename());
	zend_update_property_ex(base, Z_OBJ_P(error), ZSTR_KNOWN(ZEND_STR_FILE), &tmp);
	zval_ptr_dtor(&tmp);

	ZVAL_LONG(&tmp, zend_get_executed_lineno());
	zend_update_property_ex(base, Z_OBJ_P(error), ZSTR_KNOWN(ZEND_STR_LINE), &tmp);

	zend_fetch_debug_backtrace(&tmp, 0, 0, 0);
	Z_SET_REFCOUNT(tmp, 0);

	zend_update_property_ex(base, Z_OBJ_P(error), ZSTR_KNOWN(ZEND_STR_TRACE), &tmp);
#elif PHP_VERSION_ID < 70200
	if (0 != zval_get_long(zend_read_property(base, error, SL("line"), 1, &tmp))) {
		return;
	}

	ZVAL_STRING(&tmp, zend_get_executed_filename());
	zend_update_property(base, error, SL("file"), &tmp);
	zval_ptr_dtor(&tmp);

	ZVAL_LONG(&tmp, zend_get_executed_lineno());
	zend_update_property(base, error, SL("line"), &tmp);

	zend_fetch_debug_backtrace(&tmp, 0, 0, 0);
	Z_SET_REFCOUNT(tmp, 0);

	zend_update_property(base, error, SL("trace"), &tmp);
#else
	if (0 != zval_get_long(zend_read_property_ex(base, error, ZSTR_KNOWN(ZEND_STR_LINE), 1, &tmp))) {
		return;
	}

	ZVAL_STRING(&tmp, zend_get_executed_filename());
	zend_update_property_ex(base, error, ZSTR_KNOWN(ZEND_STR_FILE), &tmp);
	zval_ptr_dtor(&tmp);

	ZVAL_LONG(&tmp, zend_get_executed_lineno());
	zend_update_property_ex(base, error, ZSTR_KNOWN(ZEND_STR_LINE), &tmp);

	zend_fetch_debug_backtrace(&tmp, 0, 0, 0);
	Z_SET_REFCOUNT(tmp, 0);

	zend_update_property_ex(base, error, ZSTR_KNOWN(ZEND_STR_TRACE), &tmp);
#endif
}

static zend_always_inline void trigger_ops(async_task *task)
{
	async_op *op;
	
	while (task->operations.first != NULL) {
		ASYNC_NEXT_OP(&task->operations, op);
		
		if (task->status == ASYNC_OP_RESOLVED) {
			ASYNC_RESOLVE_OP(op, &task->result);
		} else {
			ASYNC_FAIL_OP(op, &task->result);
		}
	}
}

ASYNC_FIBER_CALLBACK run_task_fiber(void *arg)
{
	async_task *task;
	
	zend_execute_data *exec;
	zend_vm_stack stack;

	zend_op_array func;
	zend_op run[2];

	zval *file;
	uint32_t line;
	
	task = (async_task *) arg;
	
	ZEND_ASSERT(task != NULL);

	file = OBJ_PROP(&task->std, async_task_prop_offset(str_file));
	line = (uint32_t) Z_LVAL_P(OBJ_PROP(&task->std, async_task_prop_offset(str_line)));

	memset(run, 0, sizeof(run));

	run[0].opcode = task_opcode;
	run[0].lineno = line;
	zend_vm_set_opcode_handler_ex(run, 0, 0, 0);

	run[1].opcode = task_opcode;
	run[1].lineno = line;
	zend_vm_set_opcode_handler_ex(run + 1, 0, 0, 0);

	memset(&func, 0, sizeof(func));

	func.function_name = zend_string_copy(str_main);
	func.type = ZEND_USER_FUNCTION;
	func.opcodes = run;
	func.last_try_catch = 1;
	func.try_catch_array = &task_terminate_try_catch_array;
	func.filename = (Z_TYPE_P(file) == IS_NULL) ? ZSTR_EMPTY_ALLOC() : zend_string_copy(Z_STR_P(file));

	stack = (zend_vm_stack) emalloc(ASYNC_FIBER_VM_STACK_SIZE);
	stack->top = ZEND_VM_STACK_ELEMENTS(stack) + 1;
	stack->end = (zval *) ((char *) stack + ASYNC_FIBER_VM_STACK_SIZE);
	stack->prev = NULL;

	EG(vm_stack) = stack;
	EG(vm_stack_top) = stack->top;
	EG(vm_stack_end) = stack->end;
#if PHP_VERSION_ID >= 70300
	EG(vm_stack_page_size) = ASYNC_FIBER_VM_STACK_SIZE;
#endif
	exec = (zend_execute_data *) EG(vm_stack_top);
	EG(vm_stack_top) = (zval *) exec + ZEND_CALL_FRAME_SLOT;

#if PHP_VERSION_ID < 70400
	zend_vm_init_call_frame(exec, ZEND_CALL_TOP_FUNCTION, (zend_function *) &func, 0, NULL, NULL);
#else
	zend_vm_init_call_frame(exec, ZEND_CALL_TOP_FUNCTION, (zend_function *) &func, 0, NULL);
#endif

	exec->opline = run;
	exec->call = NULL;
	exec->return_value = NULL;
	exec->prev_execute_data = NULL;

	EG(current_execute_data) = exec;
	
	ASYNC_G(task) = task;
	ASYNC_G(context) = task->context;
	
	async_fiber_restore_og(task->context);

	zend_first_try {
		execute_ex(exec);
	} zend_catch {
		async_task_scheduler_handle_exit(task->scheduler);
	} zend_end_try()

	zend_string_release(func.function_name);
	zend_string_release(func.filename);

	zend_vm_stack_destroy();

	async_fiber_suspend(task->scheduler);
}

static int task_run_opcode_handler(zend_execute_data *exec)
{
	async_task *task;

	zval tmp;
	zval *status;

	task = ASYNC_G(task);
	ZEND_ASSERT(task != NULL);

	task->status = ASYNC_TASK_STATUS_RUNNING;
	task->fci.retval = &task->result;

	zend_call_function(&task->fci, &task->fcc);
	zend_fcall_info_args_clear(&task->fci, 1);
	
	// Have the task await a returned awaitable.
	if (UNEXPECTED(EG(exception) == NULL && Z_TYPE_P(&task->result) == IS_OBJECT)) {
		if (instanceof_function(Z_OBJCE_P(&task->result), async_awaitable_ce) != 0) {
			ZVAL_COPY(&tmp, &task->result);
			zval_ptr_dtor(&task->result);
			
			await_val(task, &tmp, exec, &task->result);
			
			zval_ptr_dtor(&tmp);
		}
	}
	
	if (UNEXPECTED(EG(exception))) {
		task->status = ASYNC_TASK_STATUS_FAILED;
		
		zval_ptr_dtor(&task->result);

		ZVAL_OBJ(&task->result, EG(exception));
		Z_ADDREF(task->result);
		
		zend_clear_exception();
	} else {
		task->status = ASYNC_TASK_STATUS_FINISHED;
	}
	
	status = OBJ_PROP(&task->std, async_task_prop_offset(str_status));
	zval_ptr_dtor(status);
	ZVAL_STRING(status, async_status_label(task->status));

	trigger_ops(task);
	
	return ZEND_USER_OPCODE_RETURN;
}

static zend_always_inline void async_task_dispose(async_task *task)
{
	if (task->flags & ASYNC_TASK_FLAG_DISPOSED) {
		return;
	}
	
	ZEND_ASSERT(task->status != ASYNC_TASK_STATUS_SUSPENDED);

	task->flags |= ASYNC_TASK_FLAG_DISPOSED;

	trigger_ops(task);
	
	ASYNC_DELREF(&task->std);
}

static void async_task_execute_inline(async_task *task, async_context *context)
{
	zval tmp;
	zval *status;

	ASYNC_LIST_REMOVE(&task->scheduler->ready, task);

	// Mark task fiber as suspended to avoid duplicate inlining attempts.
	task->status = ASYNC_TASK_STATUS_RUNNING;
	
	ASYNC_G(context) = task->context;
	
	async_fiber_capture_og(context);
	async_fiber_restore_og(task->context);

	task->fci.retval = &task->result;
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_ENTER(task->scheduler);
	}
	
	zend_try {
		zend_call_function(&task->fci, &task->fcc);
	} zend_catch {
		async_task_scheduler_handle_exit(task->scheduler);
	} zend_end_try();

	zend_fcall_info_args_clear(&task->fci, 1);
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(task->scheduler);
	}
	
	if (UNEXPECTED(EG(exception) == NULL && Z_TYPE_P(&task->result) == IS_OBJECT)) {
		if (instanceof_function(Z_OBJCE_P(&task->result), async_awaitable_ce) != 0) {
			ZVAL_COPY(&tmp, &task->result);
			zval_ptr_dtor(&task->result);
			
			await_val(task, &tmp, EG(current_execute_data), &task->result);

			zval_ptr_dtor(&tmp);
		}
	}
	
	ASYNC_G(context) = context;
	
	async_fiber_capture_og(task->context);
	async_fiber_restore_og(context);
	
	if (UNEXPECTED(EG(exception))) {
		task->status = ASYNC_TASK_STATUS_FAILED;

		ZVAL_OBJ(&task->result, EG(exception));
		Z_ADDREF(task->result);
		
		zend_clear_exception();
	} else {
		task->status = ASYNC_TASK_STATUS_FINISHED;
	}
	
	status = OBJ_PROP(&task->std, async_task_prop_offset(str_status));
	zval_ptr_dtor(status);
	ZVAL_STRING(status, async_status_label(task->status));

	async_task_dispose(task);

	ASYNC_FORWARD_EXIT();
}

static zend_always_inline void reschedule_task(async_op *op, async_task *task)
{
	ZEND_ASSERT(task != NULL);

	if (UNEXPECTED(op->flags & ASYNC_OP_FLAG_DEFER)) {
		async_task_scheduler_enqueue(task);
	} else {
		task->status = ASYNC_TASK_STATUS_RUNNING;

		async_fiber_switch(task->scheduler, task->fiber, ASYNC_FIBER_SUSPEND_PREPEND);

		if (task->status == ASYNC_OP_RESOLVED || task->status == ASYNC_OP_FAILED) {
			async_task_dispose(task);
		}
	}
}

/* Continue fiber-based task execution. */
ASYNC_CALLBACK continue_op_task(async_op *op)
{
	if (EXPECTED(!(op->flags & ASYNC_OP_FLAG_CANCELLED))) {
		reschedule_task(op, (async_task *) op->arg);
	}
}

/* Continue fiber-based task due to cancellation of an async operation. */
ASYNC_CALLBACK cancel_op(void *obj, zval *error)
{
	async_op *op;
	
	op = (async_op *) obj;

	if (EXPECTED(op->status == ASYNC_STATUS_RUNNING)) {
		ZVAL_COPY(&op->result, error);

		op->status = ASYNC_STATUS_FAILED;
		op->flags |= ASYNC_OP_FLAG_CANCELLED;
		op->cancel.object = NULL;
		op->cancel.func = NULL;

		reschedule_task(op, (async_task *) op->arg);
	}
}

static zend_always_inline void reschedule_root(async_op *op, async_task_scheduler *scheduler)
{
	ZEND_ASSERT(scheduler != NULL);

	if (UNEXPECTED(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR)) {
		async_fiber_suspend(scheduler);
	} else if (UNEXPECTED(op->flags & ASYNC_OP_FLAG_DEFER)) {
		async_task_scheduler_enqueue((async_task *) &scheduler->root);
	} else {
		async_fiber_switch(scheduler, scheduler->caller, ASYNC_FIBER_SUSPEND_PREPEND);
	}

	EG(current_execute_data) = scheduler->caller->current_execute_data;
}

/* Continue root task execution. */
ASYNC_CALLBACK continue_op_root(async_op *op)
{
	if (EXPECTED(!(op->flags & ASYNC_OP_FLAG_CANCELLED))) {
		reschedule_root(op, (async_task_scheduler *) op->arg);
	}
}

/* Continue root task due to cancellation of an async operation. */
ASYNC_CALLBACK cancel_op_root(void *obj, zval *error)
{
	async_op *op;
	
	op = (async_op *) obj;
	
	if (EXPECTED(op->status == ASYNC_STATUS_RUNNING)) {
		ZVAL_COPY(&op->result, error);

		op->status = ASYNC_STATUS_FAILED;
		op->flags |= ASYNC_OP_FLAG_CANCELLED;
		op->cancel.object = NULL;
		op->cancel.func = NULL;
	
		reschedule_root(op, (async_task_scheduler *) op->arg);
	}
}

/* Suspend the current execution until the async operation has been resolved or cancelled. */
ASYNC_API int async_await_op(async_op *op)
{
	async_task *task;
	async_task_scheduler *scheduler;
	async_context *context;
	
	zend_bool cancellable;

	ZEND_ASSERT(op->status == ASYNC_OP_PENDING);

	task = ASYNC_G(task);
	context = ASYNC_G(context);
	
	ZVAL_NULL(&op->result);
	
	cancellable = !(op->flags & ASYNC_OP_FLAG_ATOMIC) && context->cancel != NULL;
	
	if (UNEXPECTED(cancellable && context->cancel->flags & ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED)) {
		op->status = ASYNC_STATUS_FAILED;
		
		ZVAL_COPY(&op->result, &context->cancel->error);
		
		return FAILURE;
	}
	
	if (UNEXPECTED(task == NULL)) {
		scheduler = async_task_scheduler_get();

		ASYNC_OP_CHECK_ERROR(op, scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Cannot await after the task scheduler has been disposed");
		ASYNC_OP_CHECK_ERROR(op, scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR, "Cannot await after the task scheduler was stopped due to an error");
		ASYNC_OP_CHECK_ERROR(op, scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_RUNNING, "Cannot await in the fiber that is running the task scheduler loop");
		
		if (UNEXPECTED(cancellable)) {			
			op->cancel.object = op;
			op->cancel.func = cancel_op_root;
		
			ASYNC_LIST_APPEND(&context->cancel->callbacks, &op->cancel);
			ASYNC_ADDREF(&context->std);
		}
		
		op->status = ASYNC_STATUS_RUNNING;
		op->callback = continue_op_root;
		op->arg = scheduler;
		
		if (UNEXPECTED(op->list == NULL)) {
			ASYNC_APPEND_OP(&scheduler->operations, op);
		}
		
		async_task_scheduler_run_loop(scheduler);
	} else {
		scheduler = task->scheduler;

		ASYNC_OP_CHECK_ERROR(op, task->status != ASYNC_TASK_STATUS_RUNNING, "Cannot await in a task that is not running");
		ASYNC_OP_CHECK_ERROR(op, task->flags & ASYNC_TASK_FLAG_DISPOSED, "Task has been destroyed");
		ASYNC_OP_CHECK_ERROR(op, scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Cannot await after the task scheduler has been disposed");
		ASYNC_OP_CHECK_ERROR(op, scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR, "Cannot await after the task scheduler has been stopped due to an error");
		
		if (UNEXPECTED(cancellable)) {			
			op->cancel.object = op;
			op->cancel.func = cancel_op;
		
			ASYNC_LIST_APPEND(&context->cancel->callbacks, &op->cancel);
			ASYNC_ADDREF(&context->std);
		}
		
		op->status = ASYNC_STATUS_RUNNING;
		op->callback = continue_op_task;
		op->arg = task;
		
		if (UNEXPECTED(op->list == NULL)) {
			ASYNC_APPEND_OP(&scheduler->operations, op);
		}
		
		task->status = ASYNC_TASK_STATUS_SUSPENDED;
	
		async_fiber_suspend(scheduler);
	}
	
	if (UNEXPECTED(cancellable)) {
		ASYNC_DELREF(&context->std);
		
		if (EXPECTED(op->cancel.object != NULL)) {
			ASYNC_LIST_REMOVE(&context->cancel->callbacks, &op->cancel);
			
			op->cancel.object = NULL;
			op->cancel.func = NULL;
		}
	}
	
	if (UNEXPECTED(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR)) {
		ASYNC_RESET_OP(op);
		
		zend_bailout();
	}
	
	if (op->status == ASYNC_STATUS_FAILED) {
		populate_error_info(&op->result);
	}

	return (op->status == ASYNC_STATUS_RESOLVED) ? SUCCESS : FAILURE;
}

static void await_val(async_task *task, zval *val, INTERNAL_FUNCTION_PARAMETERS)
{
	async_task_scheduler *scheduler;
	async_await_handler *handler;
	async_context *context;
	async_op *op;

	zval result;

	handler = async_get_await_handler(Z_OBJCE_P(val));
	context = async_context_get();

	ZEND_ASSERT(handler != NULL);

	if (task) {
		scheduler = task->scheduler;

		ASYNC_CHECK_ERROR(task->status != ASYNC_TASK_STATUS_RUNNING, "Cannot await in a task that is not running");
		ASYNC_CHECK_ERROR(task->flags & ASYNC_TASK_FLAG_DISPOSED, "Task has been destroyed");

		ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Cannot await after the task scheduler has been disposed");
		ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR, "Cannot await after the task scheduler has been stopped due to an error");
		ASYNC_CHECK_ERROR(task->flags & ASYNC_TASK_FLAG_NOWAIT, "Cannot await within the current execution");

		op = &task->op;
	} else {
		scheduler = async_task_scheduler_get();

		ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Cannot await after the task scheduler has been disposed");
		ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR, "Cannot await after the task scheduler has been stopped due to an error");
		ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_RUNNING, "Cannot await in the fiber that is running the task scheduler loop");
		ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_NOWAIT, "Cannot await within the current execution");

		op = &scheduler->op;
	}

	switch (handler->delegate(Z_OBJ_P(val), &result, task, context, ASYNC_AWAIT_HANDLER_INLINE)) {
	case ASYNC_OP_RESOLVED:
		RETURN_ZVAL(&result, 1, 1);
	case ASYNC_OP_FAILED:
		populate_error_info(&result);
		ASYNC_FORWARD_ERROR(&result);
		zval_ptr_dtor(&result);
		return;
	}

	op->status = ASYNC_STATUS_RUNNING;
	op->flags = 0;

	handler->schedule(Z_OBJ_P(val), op, task, context, 0);

	if (!async_context_is_background(context)) {
		ASYNC_BUSY_ENTER(scheduler);
	}

	if (task) {
		op->callback = continue_op_task;
		op->arg = task;

		async_fiber_suspend(scheduler);
	} else {
		op->callback = continue_op_root;
		op->arg = scheduler;

		async_task_scheduler_run_loop(scheduler);
	}

	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(scheduler);
	}

	if (UNEXPECTED(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR)) {
		ASYNC_RESET_OP(op);

		zend_bailout();
	}

	if (op->status == ASYNC_STATUS_RESOLVED) {
		RETURN_ZVAL(&op->result, 1, 1);
	}

	populate_error_info(&op->result);

	execute_data->opline--;
#if PHP_VERSION_ID >= 80000
	zend_throw_exception_internal(Z_OBJ(op->result));
#else
	zend_throw_exception_internal(&op->result);
#endif
	execute_data->opline++;
}

static int intercept_async_call(async_interceptor interceptor, async_context *context, zval *params, uint32_t count, zend_fcall_info_cache *fcc, INTERNAL_FUNCTION_PARAMETERS)
{
	zend_execute_data *exec;
	zend_execute_data *call;

	zend_bool ret;
	zval error;
	uint32_t i;

#if PHP_VERSION_ID < 70400
	call = zend_vm_stack_push_call_frame(ZEND_CALL_TOP_FUNCTION, fcc->function_handler, count, NULL, NULL);
#else
	call = zend_vm_stack_push_call_frame(ZEND_CALL_TOP_FUNCTION, fcc->function_handler, count, NULL);
#endif

	for (i = 0; i < count; i++) {
		ZVAL_COPY(ZEND_CALL_ARG(call, i + 1), &params[i]);
	}

	ret = USED_RET();
	exec = EG(current_execute_data);

	call->prev_execute_data = exec;
	EG(current_execute_data) = call;

	interceptor(context, fcc->object, call, ret ? return_value : NULL);

	EG(current_execute_data) = exec;

	if (UNEXPECTED(EG(exception))) {
		ZVAL_OBJ(&error, EG(exception));
		RETVAL_OBJ(&async_create_failed_awaitable(EX(prev_execute_data), &error)->std);

		zend_clear_exception();
	} else if (!ret) {
		RETVAL_OBJ(ASYNC_G(awaitable));
		Z_ADDREF_P(return_value);
	}

	zend_vm_stack_free_args(call);
	zend_vm_stack_free_call_frame(call);

	return SUCCESS;
}

static async_task *async_task_object_create(zend_execute_data *call, async_task_scheduler *scheduler, async_context *context)
{
	async_task *task;
	zend_long stack_size;

	ZEND_ASSERT(scheduler != NULL);
	ZEND_ASSERT(context != NULL);

	task = ecalloc(1, sizeof(async_task) + zend_object_properties_size(async_task_ce));
	task->status = ASYNC_TASK_STATUS_INIT;
	task->id = ++ASYNC_G(nc);
	stack_size = ASYNC_G(stack_size);

	if (stack_size == 0) {
		stack_size = 4096 * (((sizeof(void *)) < 8) ? 16 : 128);
	}

	task->stack_size = stack_size;

	ZVAL_NULL(&task->result);

	task->scheduler = scheduler;
	task->context = context;
	
	ASYNC_ADDREF(&context->std);

	zend_object_std_init(&task->std, async_task_ce);
	task->std.handlers = &async_task_handlers;

	object_properties_init(&task->std, async_task_ce);

	ZVAL_STRING(OBJ_PROP(&task->std, async_task_prop_offset(str_status)), async_status_label(task->status));

	if (call != NULL && call->func && ZEND_USER_CODE(call->func->common.type)) {
		if (call->func->op_array.filename != NULL) {
			ZVAL_STR_COPY(OBJ_PROP(&task->std, async_task_prop_offset(str_file)), call->func->op_array.filename);
		}

		ZVAL_LONG(OBJ_PROP(&task->std, async_task_prop_offset(str_line)), call->opline->lineno);
	}
	
	async_task_scheduler_enqueue(task);

	return task;
}

static void async_task_object_destroy(zend_object *object)
{
	async_task *task;

	task = async_task_obj(object);
	
	if (task->fiber != NULL) {
		ZEND_ASSERT(!(task->fiber->flags & ASYNC_FIBER_FLAG_QUEUED));
	
		async_fiber_destroy(task->fiber);
	}

	zval_ptr_dtor(&task->di);
	zval_ptr_dtor(&task->result);

	ASYNC_DELREF_CB(task->fci);
	ASYNC_DELREF(&task->context->std);
	
	zend_object_std_dtor(&task->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_task_async, 0, 1, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
	ZEND_ARG_VARIADIC_INFO(0, arguments)
ZEND_END_ARG_INFO();

static PHP_METHOD(Task, async)
{
	async_task *task;
	async_context *context;
	async_interceptor interceptor;

	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	uint32_t count;
	uint32_t i;

	zval *params;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, -1)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', params, count)
	ZEND_PARSE_PARAMETERS_END();

	for (i = 1; i <= count; i++) {
		ASYNC_CHECK_ERROR(ARG_SHOULD_BE_SENT_BY_REF(fcc.function_handler, i), "Cannot pass async call argument %d by reference", (int) i);
	}

	context = async_context_get();

	if (fcc.function_handler && fcc.function_handler->type == ZEND_INTERNAL_FUNCTION) {
		if (NULL != (interceptor = (async_interceptor) zend_hash_index_find_ptr(&async_interceptors, (zend_ulong) fcc.function_handler))) {
			if (SUCCESS == intercept_async_call(interceptor, context, params, count, &fcc, INTERNAL_FUNCTION_PARAM_PASSTHRU)) {
				return;
			}
		}
	}

#if PHP_VERSION_ID < 80000
	fci.no_separation = 1;
#endif

	if (count == 0) {
		fci.param_count = 0;
	} else {
		zend_fcall_info_argp(&fci, count, params);
	}

	task = async_task_object_create(EX(prev_execute_data), async_task_scheduler_get(), context);
	task->fci = fci;
	task->fcc = fcc;
	
	ASYNC_ADDREF_CB(task->fci);

	RETURN_OBJ(&task->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_task_async_with_context, 0, 2, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_OBJ_INFO(0, context, Phalcon\\Async\\Context, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
	ZEND_ARG_VARIADIC_INFO(0, arguments)
ZEND_END_ARG_INFO();

static PHP_METHOD(Task, asyncWithContext)
{
	async_task *task;
	async_context *context;
	async_interceptor interceptor;

	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	uint32_t count;
	uint32_t i;

	zval *ctx;
	zval *params;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, -1)
		Z_PARAM_OBJECT_OF_CLASS(ctx, async_context_ce)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', params, count)
	ZEND_PARSE_PARAMETERS_END();

	for (i = 1; i <= count; i++) {
		ASYNC_CHECK_ERROR(ARG_SHOULD_BE_SENT_BY_REF(fcc.function_handler, i), "Cannot pass async call argument %d by reference", (int) i);
	}

	context = (async_context *) Z_OBJ_P(ctx);

	if (fcc.function_handler && fcc.function_handler->type == ZEND_INTERNAL_FUNCTION) {
		if (NULL != (interceptor = (async_interceptor) zend_hash_index_find_ptr(&async_interceptors, (zend_ulong) fcc.function_handler))) {
			if (SUCCESS == intercept_async_call(interceptor, context, params, count, &fcc, INTERNAL_FUNCTION_PARAM_PASSTHRU)) {
				return;
			}
		}
	}

#if PHP_VERSION_ID < 80000
	fci.no_separation = 1;
#endif

	if (count == 0) {
		fci.param_count = 0;
	} else {
		zend_fcall_info_argp(&fci, count, params);
	}

	task = async_task_object_create(EX(prev_execute_data), async_task_scheduler_get(), context);
	task->fci = fci;
	task->fcc = fcc;
	
	ASYNC_ADDREF_CB(task->fci);

	RETURN_OBJ(&task->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_task_await, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, awaitable, Phalcon\\Async\\Awaitable, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Task, await)
{
	zval *val;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(val, async_awaitable_ce)
	ZEND_PARSE_PARAMETERS_END();
	
	await_val(ASYNC_G(task), val, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_task_get_trace, 0, 0, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, limit, IS_LONG, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(Task, getTrace)
{
	async_task *task;
	zend_execute_data *prev;

	zend_long options;
	zend_long limit;

	options = DEBUG_BACKTRACE_PROVIDE_OBJECT;
	limit = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(options)
		Z_PARAM_LONG(limit)
	ZEND_PARSE_PARAMETERS_END();

	task = async_task_obj(Z_OBJ_P(getThis()));

	if (task->status == ASYNC_TASK_STATUS_SUSPENDED) {
		prev = EG(current_execute_data);
		EG(current_execute_data) = task->fiber->current_execute_data;

		zend_fetch_debug_backtrace(return_value, 0, (int) options, (int) limit);

		EG(current_execute_data) = prev;
	} else {
		array_init(return_value);
	}
}

static PHP_METHOD(Task, getId)
{

	RETURN_LONG(Z_OBJ_HANDLE_P(getThis()));
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(Task, async_task_ce)
ASYNC_METHOD_NO_WAKEUP(Task, async_task_ce)
//LCOV_EXCL_STOP

static const zend_function_entry task_functions[] = {
	PHP_ME(Task, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(Task, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(Task, async, arginfo_task_async, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Task, asyncWithContext, arginfo_task_async_with_context, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Task, await, arginfo_task_await, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Task, getTrace, arginfo_task_get_trace, ZEND_ACC_PUBLIC)
	PHP_ME(Task, getId, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


ASYNC_CALLBACK dispatch_tasks(uv_idle_t *idle)
{
	async_task_scheduler *scheduler;
	async_task *task;

	scheduler = (async_task_scheduler *) idle->data;

	ZEND_ASSERT(scheduler != NULL);

	while (scheduler->ready.first != NULL) {
		ASYNC_LIST_EXTRACT_FIRST(&scheduler->ready, task);

		if (EXPECTED(task->fiber == NULL)) {
			if (UNEXPECTED(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR)) {
				async_task_dispose(task);

				continue;
			}

			task->fiber = async_fiber_create();
		
			ASYNC_CHECK_FATAL(task->fiber == NULL, "Failed to create native fiber context");
			ASYNC_CHECK_FATAL(!async_fiber_init(task->fiber, task->context, run_task_fiber, task, task->stack_size), "Failed to create native fiber");
		
			async_fiber_switch(task->scheduler, task->fiber, ASYNC_FIBER_SUSPEND_PREPEND);
		} else {
			task->status = ASYNC_TASK_STATUS_RUNNING;

			async_fiber_switch(task->scheduler, task->fiber, ASYNC_FIBER_SUSPEND_PREPEND);
		}

		if (UNEXPECTED(task->status == ASYNC_OP_RESOLVED || task->status == ASYNC_OP_FAILED)) {
			async_task_dispose(task);
		}
	}
	
	uv_idle_stop(idle);
}

ASYNC_CALLBACK dispose_walk_cb(uv_handle_t *handle, void *arg)
{
	async_task_scheduler *scheduler;
	
	scheduler = (async_task_scheduler *) arg;
	
	ZEND_ASSERT(scheduler != NULL);
	
	if (UNEXPECTED((void *) handle == (void *) &scheduler->idle)) {
		return;
	}
	
	if (UNEXPECTED((void *) handle == (void *) &scheduler->busy)) {
		return;
	}

	ASYNC_UV_TRY_CLOSE(handle, NULL);
}

static void async_task_scheduler_dispose(async_task_scheduler *scheduler)
{
	async_cancel_cb *cancel;
	async_op *op;
	
	zval *component;
	zval error;
	
	int flag;
	
	ASYNC_CHECK_FATAL(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Scheduler has already been disposed");

	if (EXPECTED(Z_TYPE(scheduler->error) == IS_UNDEF)) {
		async_task_scheduler_run_loop(scheduler);
	}

	scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_DISPOSED;

	dispose_ticks(scheduler);

	flag = (scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_NOWAIT);
	scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_NOWAIT;

	ZEND_HASH_FOREACH_VAL(&scheduler->components, component) {
		if (instanceof_function(Z_OBJCE_P(component), async_component_ce)) {
			zend_try {

#if PHP_VERSION_ID >= 80000
				zend_call_method_with_0_params(Z_OBJ_P(component), Z_OBJCE_P(component), NULL, "shutdown", &error);
#else
				zend_call_method_with_0_params(component, Z_OBJCE_P(component), NULL, "shutdown", &error);
#endif
			} zend_catch {
				async_task_scheduler_handle_exit(scheduler);
			} zend_end_try();

			if (UNEXPECTED(EG(exception))) {
				async_task_scheduler_handle_error(scheduler, EG(exception));
				zend_clear_exception();
			}

			ASYNC_FORWARD_EXIT();
		}
	} ZEND_HASH_FOREACH_END();

	if (EXPECTED(!flag)) {
		scheduler->flags &= ~ASYNC_TASK_SCHEDULER_FLAG_NOWAIT;
	}

	ASYNC_PREPARE_SCHEDULER_ERROR(&error, "Task scheduler has been disposed");
	
	do {
		if (scheduler->operations.first != NULL) {
			do {
				ASYNC_NEXT_OP(&scheduler->operations, op);
				ASYNC_FAIL_OP(op, &error);
			} while (scheduler->operations.first != NULL);
		}
	
		if (scheduler->shutdown.first != NULL) {
			do {
				ASYNC_LIST_EXTRACT_FIRST(&scheduler->shutdown, cancel);
				if (cancel->func != NULL) {
					cancel->func(cancel->object, &error);
				}
			} while (scheduler->shutdown.first != NULL);
		}
		
		async_task_scheduler_run_loop(scheduler);
		
		uv_walk(&scheduler->loop, dispose_walk_cb, scheduler);
		
		async_task_scheduler_run_loop(scheduler);
	} while (uv_loop_alive(&scheduler->loop));
	
	zval_ptr_dtor(&error);
}

//LCOV_EXCL_START
ASYNC_CALLBACK busy_timer(uv_timer_t *handle)
{
	// Dummy timer being used to keep the loop busy...
}
//LCOV_EXCL_STOP

ASYNC_API int async_call_nowait(zend_execute_data *exec, zend_fcall_info *fci, zend_fcall_info_cache *fcc)
{
	async_task_scheduler *scheduler;
	async_task *task;
	
	zend_execute_data *prev;

	int result;
	int flag;
	
	task = ASYNC_G(task);
	result = FAILURE;
	
	if (UNEXPECTED(exec == NULL)) {
		exec = ASYNC_G(exec);
	}

	prev = EG(current_execute_data);
	EG(current_execute_data) = exec;

	if (UNEXPECTED(task == NULL)) {
		scheduler = async_task_scheduler_get();
	
		flag = (scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_NOWAIT);
		scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_NOWAIT;
	
		zend_try {
			result = zend_call_function(fci, fcc);
		} zend_catch {
			async_task_scheduler_handle_exit(scheduler);
		} zend_end_try();
	
		if (EXPECTED(!flag)) {
			scheduler->flags &= ~ASYNC_TASK_SCHEDULER_FLAG_NOWAIT;
		}
	} else {
		flag = (task->flags & ASYNC_TASK_FLAG_NOWAIT);
		task->flags |= ASYNC_TASK_FLAG_NOWAIT;
	
		zend_try {
			result = zend_call_function(fci, fcc);
		} zend_catch {
			async_task_scheduler_handle_exit(task->scheduler);
		} zend_end_try();
	
		if (EXPECTED(!flag)) {
			task->flags &= ~ASYNC_TASK_FLAG_NOWAIT;
		}
	}
	
	EG(current_execute_data) = prev;

	ASYNC_FORWARD_EXIT();

	if (UNEXPECTED(EG(exception))) {
		return FAILURE;
	}

	return result;
}

static zend_always_inline void async_task_scheduler_enqueue(async_task *task)
{
	async_task_scheduler *scheduler;

	scheduler = task->scheduler;

	ZEND_ASSERT(scheduler != NULL);
	
	if (UNEXPECTED(task->flags & ASYNC_TASK_FLAG_ROOT)) {
		task->fiber = scheduler->caller;
	} else {
		switch (task->status) {
		case ASYNC_TASK_STATUS_INIT:
			ASYNC_ADDREF(&task->std);
		case ASYNC_TASK_STATUS_SUSPENDED:
			break;
		default:
			return;
		}
	}

	if (scheduler->ready.first == NULL) {
		uv_idle_start(&scheduler->idle, dispatch_tasks);
	}

	ASYNC_LIST_APPEND(&scheduler->ready, task);
}

static zend_always_inline void async_task_scheduler_run_loop(async_task_scheduler *scheduler)
{
	ASYNC_CHECK_FATAL(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_RUNNING, "Duplicate scheduler loop run detected");

	scheduler->caller = ASYNC_G(fiber);
	scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_RUNNING;
	
	async_fiber_suspend(scheduler);
	
	scheduler->flags &= ~ASYNC_TASK_SCHEDULER_FLAG_RUNNING;
	scheduler->caller = NULL;
}

ASYNC_API void async_task_scheduler_handle_exit(async_task_scheduler *scheduler)
{
	ASYNC_G(exit) = 1;

	scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_ERROR;

	if (scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ACTIVE) {
		uv_stop(&scheduler->loop);
	}

	dispose_ticks(scheduler);
}

ASYNC_API void async_task_scheduler_handle_error(async_task_scheduler *scheduler, zend_object *error)
{
	scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_ERROR;

	if (Z_TYPE(scheduler->error) == IS_UNDEF) {
		ZVAL_OBJ(&scheduler->error, error);
		Z_ADDREF(scheduler->error);
	}

	if (scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ACTIVE) {
		uv_stop(&scheduler->loop);
	}

	dispose_ticks(scheduler);
}

static void dispose_ticks(async_task_scheduler *scheduler)
{
	async_tick_event *event;

	while (scheduler->running.first) {
		ASYNC_LIST_EXTRACT_FIRST(&scheduler->running, event);
		event->list = NULL;

		if (EXPECTED(event->flags & ASYNC_TICK_EVENT_FLAG_REFERENCED)) {
			scheduler->refticks--;
		}

		ASYNC_DELREF(&event->std);
	}

	while (scheduler->ticks.first) {
		ASYNC_LIST_EXTRACT_FIRST(&scheduler->ticks, event);
		event->list = NULL;

		if (EXPECTED(event->flags & ASYNC_TICK_EVENT_FLAG_REFERENCED)) {
			scheduler->refticks--;
		}

		ASYNC_DELREF(&event->std);
	}
}

static void run_ticks(async_task_scheduler *scheduler)
{
	async_tick_event *event;

	zend_fcall_info fci;

	zval args[1];
	zval retval;

	int code;

	ZEND_ASSERT(scheduler->ticks.first != NULL);

	scheduler->running = scheduler->ticks;
	event = scheduler->running.first;

	scheduler->ticks.first = NULL;
	scheduler->ticks.last = NULL;

	do {
		event->list = &scheduler->running;
		event = event->next;
	} while (event);

	while (scheduler->running.first) {
		ASYNC_LIST_EXTRACT_FIRST(&scheduler->running, event);

		event->list = NULL;

		if (EXPECTED(event->flags & ASYNC_TICK_EVENT_FLAG_REFERENCED)) {
			scheduler->refticks--;
		}

		fci = empty_fcall_info;

		ZVAL_OBJ(&args[0], &event->std);

		fci.size = sizeof(zend_fcall_info);
		fci.object = event->fcc.object;

#if PHP_VERSION_ID < 80000
		fci.no_separation = 1;
#endif
		ZVAL_COPY_VALUE(&fci.function_name, &event->callback);

		fci.params = args;
		fci.param_count = 1;
		fci.retval = &retval;

		zend_try {
			code = async_call_nowait(ASYNC_G(exec), &fci, &event->fcc);
		} zend_catch {
			async_task_scheduler_handle_exit(scheduler);
			ASYNC_DELREF(&event->std);
			return;
		} zend_end_try();

		if (UNEXPECTED(code == FAILURE)) {
			ASYNC_ENSURE_ERROR("Failed to invoke tick callback");
			ASYNC_DELREF(&event->std);
			return;
		}

		zval_ptr_dtor(&retval);

		ASYNC_DELREF(&event->std);
	}
}

ASYNC_FIBER_CALLBACK run_scheduler_fiber(void *arg)
{
	async_task_scheduler *scheduler;

	int again;

	scheduler = (async_task_scheduler *) arg;
	
	ZEND_ASSERT(scheduler != NULL);
	
	ASYNC_G(scheduler) = scheduler;
	ASYNC_G(context) = ASYNC_G(foreground);
	ASYNC_G(task) = NULL;
	
	async_fiber_restore_og(ASYNC_G(context));

	EG(current_execute_data) = NULL;

	while (1) {
		do {
			if (scheduler->ticks.first) {
				run_ticks(scheduler);

				if (UNEXPECTED(EG(exception))) {
					async_task_scheduler_handle_error(scheduler, EG(exception));
					zend_clear_exception();
				}
			}

			scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_ACTIVE;

			again = uv_run(&scheduler->loop, scheduler->ticks.first ? UV_RUN_NOWAIT : UV_RUN_DEFAULT);

			scheduler->flags &= ~ASYNC_TASK_SCHEDULER_FLAG_ACTIVE;

			if (UNEXPECTED(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR)) {
				break;
			}
		} while (again || scheduler->refticks > 0);
		
		async_fiber_switch(scheduler, scheduler->caller, ASYNC_FIBER_SUSPEND_NONE);
	}
}

static async_task_scheduler *async_task_scheduler_object_create()
{
	async_task_scheduler *scheduler;

	scheduler = ecalloc(1, sizeof(async_task_scheduler));

	zend_object_std_init(&scheduler->std, async_task_scheduler_ce);

	scheduler->std.handlers = &async_task_scheduler_handlers;
	
	uv_loop_init(&scheduler->loop);
	uv_idle_init(&scheduler->loop, &scheduler->idle);
	
	uv_timer_init(&scheduler->loop, &scheduler->busy);
	uv_timer_start(&scheduler->busy, busy_timer, 3600 * 1000, 3600 * 1000);	
	uv_unref((uv_handle_t *) &scheduler->busy);

	scheduler->idle.data = scheduler;
	
	scheduler->runner = async_fiber_create();
	async_fiber_init(scheduler->runner, ASYNC_G(foreground), run_scheduler_fiber, scheduler, 1024 * 1024 * 128);
	
	zend_hash_init(&scheduler->components, 0, NULL, ZVAL_PTR_DTOR, 0);

	scheduler->root.scheduler = scheduler;
	scheduler->root.flags = ASYNC_TASK_FLAG_ROOT;
	scheduler->root.status = ASYNC_TASK_STATUS_SUSPENDED;

	return scheduler;
}

ASYNC_CALLBACK walk_loop_cb(uv_handle_t *handle, void *arg)
{
	int *count;
	
	count = (int *) arg;
	
	(*count)++;
	
	printf(">> UV HANDLE LEAKED: [%d] %s -> %d\n", handle->type, uv_handle_type_name(handle->type), uv_is_closing(handle));
	
	ASYNC_UV_TRY_CLOSE(handle, NULL);
}

static zend_always_inline int debug_handles(uv_loop_t *loop)
{
    int pending;
    
    pending = 0;

	uv_walk(loop, walk_loop_cb, &pending);
	
	return pending;
}

static void async_task_scheduler_object_destroy(zend_object *object)
{
	async_task_scheduler *scheduler;

#if ZEND_DEBUG
	int code;
#endif

	scheduler = (async_task_scheduler *)object;
	
	zend_hash_destroy(&scheduler->components);

	ASYNC_UV_CLOSE((uv_handle_t *) &scheduler->busy, NULL);
	ASYNC_UV_CLOSE((uv_handle_t *) &scheduler->idle, NULL);
	
	// Run loop again to cleanup idle watcher.
	uv_run(&scheduler->loop, UV_RUN_DEFAULT);

#if ZEND_DEBUG
	ZEND_ASSERT(!uv_loop_alive(&scheduler->loop));
	ZEND_ASSERT(debug_handles(&scheduler->loop) == 0);

	code = uv_loop_close(&scheduler->loop);
#else
	uv_loop_close(&scheduler->loop);
#endif
	
	if (scheduler->runner != NULL) {
		ZEND_ASSERT(!(scheduler->runner->flags & ASYNC_FIBER_FLAG_QUEUED));
		
		async_fiber_destroy(scheduler->runner);
	}
	
#if ZEND_DEBUG
	ZEND_ASSERT(code == 0);
	ZEND_ASSERT(scheduler->ready.first == NULL);
	ZEND_ASSERT(scheduler->fibers.first == NULL);
#endif

	zval_ptr_dtor(&scheduler->error);

	zend_object_std_dtor(object);
}

ASYNC_CALLBACK debug_pending_tasks(async_task_scheduler *scheduler, zend_fcall_info *fci, zend_fcall_info_cache *fcc)
{
	async_task *task;

	zend_ulong i;

	zval args[1];
	zval retval;
	zval obj;
	
	array_init(&args[0]);

	task = scheduler->ready.first;
	i = 0;

	while (task != NULL) {
		ZVAL_OBJ(&obj, &task->std);
		ASYNC_ADDREF(&task->std);

		zend_hash_index_update(Z_ARRVAL_P(&args[0]), i, &obj);

		task = task->next;
		i++;
	}

	fci->param_count = 1;
	fci->params = args;
	fci->retval = &retval;

#if PHP_VERSION_ID < 80000
	fci->no_separation = 1;
#endif
	zend_call_function(fci, fcc);

	zval_ptr_dtor(&args[0]);
	zval_ptr_dtor(&retval);

	ASYNC_CHECK_FATAL(EG(exception), "Must not throw an error from scheduler inspector");
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_task_scheduler_run, 0, 0, 1)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
	ZEND_ARG_CALLABLE_INFO(0, finalizer, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(TaskScheduler, run)
{
	async_task_scheduler *scheduler;
	async_task_scheduler *prev;
	zend_execute_data *exec;
	
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	
	zend_fcall_info fci2;
	zend_fcall_info_cache fcc2;
	
	zval retval;
	
	fci2 = empty_fcall_info;
	fcc2 = empty_fcall_info_cache;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC_EX(fci2, fcc2, 1, 0)
	ZEND_PARSE_PARAMETERS_END();
	
	scheduler = async_task_scheduler_object_create();
	
	prev = ASYNC_G(scheduler);
	ASYNC_G(scheduler) = scheduler;
	
	exec = ASYNC_G(exec);
	ASYNC_G(exec) = execute_data;

	fci.retval = &retval;
	fci.param_count = 0;

	zend_try {
		zend_call_function(&fci, &fcc);
	} zend_catch {
		async_task_scheduler_handle_exit(scheduler);
		async_task_scheduler_unref(scheduler);
	} zend_end_try();
	
	if (UNEXPECTED(ASYNC_G(exit))) {
		ASYNC_G(exec) = exec;
		ASYNC_G(scheduler) = prev;

		zend_bailout();
	}

	if (UNEXPECTED(EG(exception))) {
		async_task_scheduler_handle_error(scheduler, EG(exception));
		zend_clear_exception();
	} else {
		if (ZEND_NUM_ARGS() > 1) {
			debug_pending_tasks(scheduler, &fci2, &fcc2);
		}
	}
	
	async_task_scheduler_dispose(scheduler);
	
	ASYNC_G(exec) = exec;
	ASYNC_G(scheduler) = prev;
	
	if (Z_TYPE(scheduler->error) != IS_UNDEF) {
		ASYNC_FORWARD_ERROR(&scheduler->error);
	} else {
		RETVAL_ZVAL(&retval, 1, 1);
	}

	async_task_scheduler_unref(scheduler);
	
	ASYNC_FORWARD_EXIT();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_task_scheduler_run_with_context, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, context, Phalcon\\Async\\Context, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
	ZEND_ARG_CALLABLE_INFO(0, finalizer, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(TaskScheduler, runWithContext)
{
	async_task_scheduler *scheduler;
	async_task_scheduler *prev;
	zend_execute_data *exec;
	async_context *context;
	async_context *scope;
	
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	
	zend_fcall_info fci2;
	zend_fcall_info_cache fcc2;
	
	zval *ctx;
	zval retval;
	
	fci2 = empty_fcall_info;
	fcc2 = empty_fcall_info_cache;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 3)
		Z_PARAM_OBJECT_OF_CLASS(ctx, async_context_ce)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC_EX(fci2, fcc2, 1, 0)
	ZEND_PARSE_PARAMETERS_END();
	
	scope = (async_context *) Z_OBJ_P(ctx);
	
	scheduler = async_task_scheduler_object_create();
	
	prev = ASYNC_G(scheduler);
	ASYNC_G(scheduler) = scheduler;
	
	exec = ASYNC_G(exec);
	ASYNC_G(exec) = execute_data;

	context = ASYNC_G(context);
	ASYNC_G(context) = scope;
	
	async_fiber_capture_og(context);
	async_fiber_restore_og(scope);
	
	fci.retval = &retval;
	fci.param_count = 0;
	
	zend_try {
		zend_call_function(&fci, &fcc);
	} zend_catch {
		async_task_scheduler_handle_exit(scheduler);
		async_task_scheduler_unref(scheduler);
	} zend_end_try();
	
	ASYNC_G(context) = context;
	
	async_fiber_capture_og(scope);
	async_fiber_restore_og(context);
	
	if (UNEXPECTED(ASYNC_G(exit))) {
		ASYNC_G(exec) = exec;
		ASYNC_G(scheduler) = prev;

		zend_bailout();
	}

	if (UNEXPECTED(EG(exception))) {
		async_task_scheduler_handle_error(scheduler, EG(exception));
		zend_clear_exception();
	} else {
		if (ZEND_NUM_ARGS() > 2) {
			debug_pending_tasks(scheduler, &fci2, &fcc2);
		}
	}
	
	async_task_scheduler_dispose(scheduler);
	
	ASYNC_G(exec) = exec;
	ASYNC_G(scheduler) = prev;
	
	if (Z_TYPE(scheduler->error) != IS_UNDEF) {
		ASYNC_FORWARD_ERROR(&scheduler->error);
	} else {
		RETVAL_ZVAL(&retval, 1, 1);
	}

	async_task_scheduler_unref(scheduler);

	ASYNC_FORWARD_EXIT();
}

ASYNC_API zval *async_get_component(async_task_scheduler *scheduler, zend_string *type, zend_execute_data *exec)
{
	async_component_factory *factory;
	async_context *context;

	zval component;
	zval args[1];
	zval *val;

	int code;

	val = zend_hash_find(&scheduler->components, type);

	if (val) {
		return val;
	}

	factory = (async_component_factory *) zend_hash_find_ptr(ASYNC_G(factories), type);

	if (UNEXPECTED(!factory)) {
		zend_throw_error(NULL, "No factory registered for type %s", ZSTR_VAL(type));
		return NULL;
	}

	ZVAL_OBJ(&args[0], &scheduler->std);

	factory->fci.retval = &component;
	factory->fci.param_count = 1;
	factory->fci.params = args;

	context = async_context_get();
	ASYNC_G(context) = ASYNC_G(foreground);

	code = FAILURE;

	zend_try {
		code = async_call_nowait(exec, &factory->fci, &factory->fcc);
	} zend_catch {
		async_task_scheduler_handle_exit(scheduler);
	} zend_end_try();

	ASYNC_G(context) = context;

	ASYNC_FORWARD_EXIT();

	if (UNEXPECTED(code != SUCCESS)) {
		ASYNC_ENSURE_ERROR("Failed to call factory for type %s", ZSTR_VAL(type));
		return NULL;
	}

	if (UNEXPECTED(Z_TYPE_P(&component) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(&component), factory->ce))) {
		zend_throw_error(NULL, "Factory must return an instance of %s", ZSTR_VAL(factory->ce->name));
		zval_ptr_dtor(&component);
		return NULL;
	}

	return zend_hash_add(&scheduler->components, type, &component);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_task_scheduler_get, 0, 1, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TaskScheduler, get)
{
	zend_string *type;
	zval *val;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(type)
	ZEND_PARSE_PARAMETERS_END();

	val = async_get_component(async_task_scheduler_get(), type, execute_data);

	if (EXPECTED(val)) {
		RETURN_ZVAL(val, 1, 0);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_task_scheduler_register, 0, 2, IS_CALLABLE, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
	ZEND_ARG_CALLABLE_INFO(0, factory, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(TaskScheduler, register)
{
	async_component_factory *factory;

	zend_string *type;
	zend_class_entry *ce;

	zval *callable;
	zval prev;

	char *error;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_STR(type)
		Z_PARAM_ZVAL(callable)
	ZEND_PARSE_PARAMETERS_END();

	ce = zend_lookup_class(type);

	ASYNC_CHECK_ERROR(!ce, "Type not found: %s", ZSTR_VAL(type));

	factory = (async_component_factory *) zend_hash_find_ptr(ASYNC_G(factories), type);

	if (factory) {
		ZVAL_COPY(&prev, &factory->callable);
	} else {
		ZVAL_NULL(&prev);
	}

	if (Z_TYPE_P(callable) == IS_NULL) {
		if (factory) {
			zend_hash_del(ASYNC_G(factories), type);
		}

		RETURN_ZVAL(&prev, 1, 1);
	}

	factory = emalloc(sizeof(async_component_factory));
	factory->ce = ce;
	factory->fci = empty_fcall_info;
	factory->fcc = empty_fcall_info_cache;

	if (FAILURE == zend_fcall_info_init(callable, 0, &factory->fci, &factory->fcc, NULL, &error)) {
		zend_throw_error(NULL, "Failed to register factory: %s", error);

		zval_ptr_dtor(&prev);
		efree(factory);
		efree(error);
		return;
	}

	ZVAL_COPY(&factory->callable, callable);

	ASYNC_ADDREF_CB(factory->fci);

	zend_hash_update_ptr(ASYNC_G(factories), type, factory);

	RETURN_ZVAL(&prev, 1, 1);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_task_scheduler_tick, 0, 1, Phalcon\\Async\\TickEvent, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TaskScheduler, tick)
{
	async_task_scheduler *scheduler;
	async_tick_event *event;

	zval *callback;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

	scheduler = (async_task_scheduler *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Task scheduler has been disposed");
	ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR, "Task scheduler was stopped due to an error");

	event = async_tick_event_object_create(scheduler, callback);

	if (EXPECTED(event)) {
		RETURN_OBJ(&event->std);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_task_scheduler_timer, 0, 1, Phalcon\\Async\\TimerEvent, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TaskScheduler, timer)
{
	async_task_scheduler *scheduler;
	async_timer_event *event;

	zval *callback;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

	scheduler = (async_task_scheduler *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Task scheduler has been disposed");
	ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR, "Task scheduler was stopped due to an error");

	event = async_timer_event_object_create(scheduler, callback);

	if (EXPECTED(event)) {
		RETURN_OBJ(&event->std);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_task_scheduler_poll, 0, 2, Phalcon\\Async\\PollEvent, 0)
	ZEND_ARG_TYPE_INFO(0, stream, IS_RESOURCE, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TaskScheduler, poll)
{
	async_task_scheduler *scheduler;
	async_poll_event *event;

	php_socket_t fd;
	zend_string *error;

	zval *val;
	zval *callback;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_RESOURCE(val)
		Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

	scheduler = (async_task_scheduler *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Task scheduler has been disposed");
	ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR, "Task scheduler was stopped due to an error");

	if (UNEXPECTED(FAILURE == async_get_poll_fd(val, &fd, &error))) {
		zend_throw_error(NULL, "%s", ZSTR_VAL(error));
		zend_string_release(error);
		return;
	}

	event = async_poll_event_object_create(scheduler, callback, val, fd);

	if (EXPECTED(event)) {
		RETURN_OBJ(&event->std);
	}
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(TaskScheduler, async_task_scheduler_ce)
ASYNC_METHOD_NO_WAKEUP(TaskScheduler, async_task_scheduler_ce)
//LCOV_EXCL_STOP

static const zend_function_entry task_scheduler_functions[] = {
	PHP_ME(TaskScheduler, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(TaskScheduler, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(TaskScheduler, run, arginfo_task_scheduler_run, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(TaskScheduler, runWithContext, arginfo_task_scheduler_run_with_context, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(TaskScheduler, get, arginfo_task_scheduler_get, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(TaskScheduler, register, arginfo_task_scheduler_register, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(TaskScheduler, tick, arginfo_task_scheduler_tick, ZEND_ACC_PUBLIC)
	PHP_ME(TaskScheduler, timer, arginfo_task_scheduler_timer, ZEND_ACC_PUBLIC)
	PHP_ME(TaskScheduler, poll, arginfo_task_scheduler_poll, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_component_shutdown, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Component, shutdown) { }

static const zend_function_entry async_component_functions[] = {
	PHP_ME(Component, shutdown, arginfo_component_shutdown, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_FE_END
};


static const zend_function_entry empty_funcs[] = {
	PHP_FE_END
};

void async_task_ce_register()
{
	zend_class_entry ce;

#if PHP_VERSION_ID >= 70400
	zval tmp;
#endif

	str_main = zend_new_interned_string(zend_string_init(ZEND_STRL("main"), 1));
	str_status = zend_new_interned_string(zend_string_init(ZEND_STRL("status"), 1));
	str_file = zend_new_interned_string(zend_string_init(ZEND_STRL("file"), 1));
	str_line = zend_new_interned_string(zend_string_init(ZEND_STRL("line"), 1));

	task_opcode = ZEND_VM_LAST_OPCODE + 1;
	
	//LCOV_EXCL_START
	while (1) {
		if (task_opcode == 255) {
			return;
		} else if (zend_get_user_opcode_handler(task_opcode) == NULL) {
			break;
		}
		task_opcode++;
	}
	//LCOV_EXCL_STOP

	zend_set_user_opcode_handler(task_opcode, task_run_opcode_handler);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "Awaitable", empty_funcs);
	async_awaitable_ce = zend_register_internal_interface(&ce);
	async_awaitable_ce->interface_gets_implemented = implements_awaitable;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "Component", async_component_functions);
	async_component_ce = zend_register_internal_interface(&ce);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "AwaitableImpl", empty_funcs);
	async_awaitable_impl_ce = zend_register_internal_class(&ce);
	async_awaitable_impl_ce->ce_flags |= ZEND_ACC_FINAL;
	async_awaitable_impl_ce->serialize = zend_class_serialize_deny;
	async_awaitable_impl_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_awaitable_impl_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_awaitable_impl_handlers.offset = XtOffsetOf(async_awaitable_impl, std);
	async_awaitable_impl_handlers.free_obj = async_awaitable_impl_object_destroy;
	async_awaitable_impl_handlers.clone_obj = NULL;
	async_awaitable_impl_handlers.write_property = async_prop_write_handler_readonly;

#if PHP_VERSION_ID >= 80000
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_awaitable_impl_ce, str_status, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 0, 0));
	zval_ptr_dtor(&tmp);

	ZVAL_NULL(&tmp);
	zend_declare_typed_property(async_awaitable_impl_ce, str_file, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 1, 0));
	zend_declare_typed_property(async_awaitable_impl_ce, str_line, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_LONG, 0, 0));
#elif PHP_VERSION_ID < 70400
	zend_declare_property_null(async_awaitable_impl_ce, ZEND_STRL("status"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_awaitable_impl_ce, ZEND_STRL("file"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_awaitable_impl_ce, ZEND_STRL("line"), ZEND_ACC_PUBLIC);
#else
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_awaitable_impl_ce, str_status, &tmp, ZEND_ACC_PUBLIC, NULL, IS_STRING);
	zval_ptr_dtor(&tmp);

	ZVAL_NULL(&tmp);
	zend_declare_typed_property(async_awaitable_impl_ce, str_file, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_STRING, 1));
	zend_declare_typed_property(async_awaitable_impl_ce, str_line, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_LONG, 1));
#endif

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "Task", task_functions);
	async_task_ce = zend_register_internal_class(&ce);
	async_task_ce->ce_flags |= ZEND_ACC_FINAL;
	async_task_ce->serialize = zend_class_serialize_deny;
	async_task_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_task_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_task_handlers.offset = XtOffsetOf(async_task, std);
	async_task_handlers.free_obj = async_task_object_destroy;
	async_task_handlers.clone_obj = NULL;
	async_task_handlers.write_property = async_prop_write_handler_readonly;

#if PHP_VERSION_ID >= 80000
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_task_ce, str_status, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 0, 0));
	zval_ptr_dtor(&tmp);

	ZVAL_NULL(&tmp);
	zend_declare_typed_property(async_task_ce, str_file, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 1, 0));
	zend_declare_typed_property(async_task_ce, str_line, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_LONG, 0, 0));
#elif PHP_VERSION_ID < 70400
	zend_declare_property_null(async_task_ce, ZEND_STRL("status"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_task_ce, ZEND_STRL("file"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_task_ce, ZEND_STRL("line"), ZEND_ACC_PUBLIC);
#else
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_task_ce, str_status, &tmp, ZEND_ACC_PUBLIC, NULL, IS_STRING);
	zval_ptr_dtor(&tmp);

	ZVAL_NULL(&tmp);
	zend_declare_typed_property(async_task_ce, str_file, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_STRING, 1));
	zend_declare_typed_property(async_task_ce, str_line, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_LONG, 1));
#endif

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "TaskScheduler", task_scheduler_functions);
	async_task_scheduler_ce = zend_register_internal_class(&ce);
	async_task_scheduler_ce->ce_flags |= ZEND_ACC_FINAL;
	async_task_scheduler_ce->serialize = zend_class_serialize_deny;
	async_task_scheduler_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_task_scheduler_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_task_scheduler_handlers.free_obj = async_task_scheduler_object_destroy;
	async_task_scheduler_handlers.clone_obj = NULL;

	zend_hash_init(&async_interceptors, 0, NULL, NULL, 1);
	zend_hash_init(&await_handlers, 0, NULL, NULL, 1);

	task_await_handler.delegate = task_await_delegate;
	task_await_handler.schedule = task_await_schedule;

	async_register_awaitable(async_task_ce, &task_await_handler);

	awaitable_impl_await_handler.delegate = awaitable_impl_delegate;
	awaitable_impl_await_handler.schedule = awaitable_impl_schedule;

	async_register_awaitable(async_awaitable_impl_ce, &awaitable_impl_await_handler);
}

static void factory_dtor(zval *arg)
{
	async_component_factory *factory;

	factory = (async_component_factory *) Z_PTR_P(arg);

	ASYNC_DELREF_CB(factory->fci);
	zval_ptr_dtor(&factory->callable);

	efree(factory);
}

void async_task_scheduler_init()
{
	async_task_scheduler *scheduler;
	async_fiber *fiber;
	
	scheduler = async_task_scheduler_object_create();
	
	ASYNC_G(executor) = scheduler;
	ASYNC_G(scheduler) = scheduler;
	
	fiber = async_fiber_create_root();
	
	ASYNC_G(root) = fiber;
	ASYNC_G(fiber) = fiber;

	ASYNC_G(awaitable) = &async_create_resolved_awaitable(NULL, NULL)->std;

	ALLOC_HASHTABLE(ASYNC_G(factories));
	zend_hash_init(ASYNC_G(factories), 0, NULL, factory_dtor, 0);
}

ASYNC_API void async_task_scheduler_run(async_task_scheduler *scheduler, zend_execute_data *execute_data)
{
	zend_execute_data *prev;

	prev = ASYNC_G(exec);
	ASYNC_G(exec) = execute_data->prev_execute_data;

	async_task_scheduler_dispose(scheduler);

	ASYNC_G(exec) = prev;

	if (Z_TYPE(scheduler->error) != IS_UNDEF) {
		ASYNC_FORWARD_ERROR(&scheduler->error);
	}
}

void async_task_scheduler_shutdown()
{
	async_task_scheduler *scheduler;

	scheduler = ASYNC_G(executor);
	
	if (scheduler != NULL) {
		ASYNC_G(executor) = NULL;

		async_task_scheduler_unref(scheduler);
	}
	
	zend_hash_destroy(ASYNC_G(factories));
	FREE_HASHTABLE(ASYNC_G(factories));

	async_fiber_destroy(ASYNC_G(root));

	ASYNC_DELREF(ASYNC_G(awaitable));
}

void async_task_ce_unregister()
{
	zend_hash_destroy(&async_interceptors);
	zend_hash_destroy(&await_handlers);

	zend_string_release(str_main);
	zend_string_release(str_status);
	zend_string_release(str_file);
	zend_string_release(str_line);
}
