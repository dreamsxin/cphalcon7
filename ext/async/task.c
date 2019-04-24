
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

#include "kernel/main.h"
#include "kernel/fcall.h"

#if PHALCON_USE_UV

#include "kernel/backend.h"
#include "async/async_fiber.h"

ASYNC_API zend_class_entry *async_task_ce;
ASYNC_API zend_class_entry *async_task_scheduler_ce;

static zend_object_handlers async_task_handlers;
static zend_object_handlers async_task_scheduler_handlers;

static zend_op_array task_run_func;
static zend_try_catch_element task_terminate_try_catch_array = { 0, 1, 0, 0 };
static zend_op task_run_op[2];

static void await_val(async_task *task, zval *val, zval *return_value, zend_execute_data *execute_data);

static zend_always_inline void async_task_scheduler_enqueue(async_task *task);
static zend_always_inline void async_task_scheduler_run_loop(async_task_scheduler *scheduler);

#define ASYNC_TASK_STATUS_INIT 0
#define ASYNC_TASK_STATUS_SUSPENDED 1
#define ASYNC_TASK_STATUS_RUNNING 2
#define ASYNC_TASK_STATUS_FINISHED ASYNC_OP_RESOLVED
#define ASYNC_TASK_STATUS_FAILED ASYNC_OP_FAILED

#define ASYNC_OP_CHECK_ERROR(op, expr, message, ...) do { \
    if (UNEXPECTED(expr)) { \
    	(op)->status = ASYNC_STATUS_FAILED; \
    	ASYNC_PREPARE_ERROR(&(op)->result, message ASYNC_VA_ARGS(__VA_ARGS__)); \
    	return FAILURE; \
    } \
} while (0)

#define ASYNC_TASK_DELEGATE_RESULT(status, result) do { \
	if (status == ASYNC_OP_RESOLVED) { \
		RETURN_ZVAL(result, 1, 0); \
	} else if (status == ASYNC_OP_FAILED) { \
		ASYNC_FORWARD_ERROR(result); \
		return; \
	} \
} while (0)

#define ASYNC_TASK_DELEGATE_OP_RESULT(op) do { \
	if ((op)->status == ASYNC_STATUS_RESOLVED) { \
		RETURN_ZVAL(&(op)->result, 1, 1); \
	} \
	if (Z_TYPE_P(&(op)->result) != IS_UNDEF) { \
		EG(current_execute_data)->opline--; \
		zend_throw_exception_internal(&(op)->result); \
		EG(current_execute_data)->opline++; \
		return; \
	} \
	zval_ptr_dtor(&(op)->result); \
	if (UNEXPECTED(EG(exception) == NULL)) { \
		zend_throw_error(NULL, "Awaitable has not been resolved"); \
	} \
	return; \
} while (0)


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
	
	task = (async_task *) arg;
	
	ZEND_ASSERT(task != NULL);

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
	zend_vm_init_call_frame(exec, ZEND_CALL_TOP_FUNCTION, (zend_function *) &task_run_func, 0, NULL, NULL);
	exec->opline = task_run_op;
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
		ASYNC_G(exit) = 1;
		
		if (task->scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ACTIVE) {
			uv_stop(&task->scheduler->loop);
		}
	} zend_end_try()

	zend_vm_stack_destroy();
	
	async_fiber_suspend(task->scheduler);
}

static int task_run_opcode_handler(zend_execute_data *exec)
{
	async_task *task;

	zval tmp;

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
			
			await_val(task, &tmp, &task->result, exec);
			
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
	
	trigger_ops(task);
	
	return ZEND_USER_OPCODE_RETURN;
}

static zval *async_task_get_debug_info(async_task *task, zval *retval)
{
	array_init(retval);
	
	add_assoc_string(retval, "status", async_status_label(task->status));
	add_assoc_bool(retval, "suspended", task->status == ASYNC_TASK_STATUS_SUSPENDED);
	add_assoc_str(retval, "file", zend_string_copy(task->file));
	add_assoc_long(retval, "line", task->line);

	if (Z_TYPE_P(&task->result) != IS_UNDEF) {
		Z_TRY_ADDREF_P(&task->result);
		
		add_assoc_zval(retval, "result", &task->result);
	}
	
	return retval;
}

static zend_always_inline void async_task_dispose(async_task *task)
{
	if (task->flags & ASYNC_TASK_FLAG_DISPOSED) {
		return;
	}
	
	task->flags |= ASYNC_TASK_FLAG_DISPOSED;

	if (task->status == ASYNC_TASK_STATUS_SUSPENDED) {
		task->status = ASYNC_TASK_STATUS_RUNNING;

		async_fiber_switch(task->scheduler, task->fiber, ASYNC_FIBER_SUSPEND_PREPEND);
	}

	trigger_ops(task);
	
	//ASYNC_DELREF(&task->std);
}

static void async_task_execute_inline(async_task *task, async_context *context)
{
	zval tmp;

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
	
	zend_call_function(&task->fci, &task->fcc);
	zend_fcall_info_args_clear(&task->fci, 1);
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(task->scheduler);
	}
	
	if (UNEXPECTED(EG(exception) == NULL && Z_TYPE_P(&task->result) == IS_OBJECT)) {
		if (instanceof_function(Z_OBJCE_P(&task->result), async_awaitable_ce) != 0) {
			ZVAL_COPY(&tmp, &task->result);
			zval_ptr_dtor(&task->result);
			
			await_val(task, &tmp, &task->result, EG(current_execute_data));

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
	
	async_task_dispose(task);
}

/* Continue fiber-based task execution. */
ASYNC_CALLBACK continue_op_task(async_op *op)
{
	async_task *task;
	zend_bool flag;
	
	task = (async_task *) op->arg;
	
	if (op->flags & ASYNC_OP_FLAG_CANCELLED) {
		return;
	}

	if (UNEXPECTED(op->flags & ASYNC_OP_FLAG_DEFER)) {
		async_task_scheduler_enqueue(task);
	} else {
		flag = (task->scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_NOWAIT) ? 1 : 0;

		if (UNEXPECTED(flag)) {
			task->scheduler->flags &= ~ASYNC_TASK_SCHEDULER_FLAG_NOWAIT;
		}

		task->status = ASYNC_TASK_STATUS_RUNNING;

		async_fiber_switch(task->scheduler, task->fiber, ASYNC_FIBER_SUSPEND_PREPEND);
		
		if (UNEXPECTED(flag)) {
			task->scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_NOWAIT;
		}

		if (task->status == ASYNC_OP_RESOLVED || task->status == ASYNC_OP_FAILED) {
			async_task_dispose(task);
		}
	}
}

/* Continue fiber-based task due to cancellation of an async operation. */
ASYNC_CALLBACK cancel_op(void *obj, zval *error)
{
	async_op *op;
	async_task *task;
	
	op = (async_op *) obj;
	task = (async_task *) op->arg;
	
	if (op->status != ASYNC_STATUS_RUNNING) {
		return;
	}
	
	ZEND_ASSERT(task != NULL);
	
	ZVAL_COPY(&op->result, error);
	
	op->status = ASYNC_STATUS_FAILED;
	op->flags |= ASYNC_OP_FLAG_CANCELLED;
	op->cancel.object = NULL;
	op->cancel.func = NULL;

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

/* Continue root task execution. */
ASYNC_CALLBACK continue_op_root(async_op *op)
{
	async_task_scheduler *scheduler;

	scheduler = (async_task_scheduler *) op->arg;
	
	ZEND_ASSERT(scheduler != NULL);
	
	if (op->flags & ASYNC_OP_FLAG_CANCELLED) {
		return;
	}
	
	if (UNEXPECTED(ASYNC_G(exit))) {
		if (scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ACTIVE) {
			uv_stop(&scheduler->loop);
		}
		
		async_fiber_suspend(scheduler);
	} else if (UNEXPECTED(op->flags & ASYNC_OP_FLAG_DEFER)) {
		async_fiber_switch(scheduler, scheduler->caller, ASYNC_FIBER_SUSPEND_APPEND);
	} else {
		async_fiber_switch(scheduler, scheduler->caller, ASYNC_FIBER_SUSPEND_PREPEND);
	}
}

/* Continue root task due to cancellation of an async operation. */
ASYNC_CALLBACK cancel_op_root(void *obj, zval *error)
{
	async_op *op;
	async_task_scheduler *scheduler;
	
	op = (async_op *) obj;
	scheduler = (async_task_scheduler *) op->arg;
	
	if (UNEXPECTED(op->status != ASYNC_STATUS_RUNNING)) {
		return;
	}
	
	ZEND_ASSERT(scheduler != NULL);
	
	ZVAL_COPY(&op->result, error);
	
	op->status = ASYNC_STATUS_FAILED;
	op->flags |= ASYNC_OP_FLAG_CANCELLED;
	op->cancel.object = NULL;
	op->cancel.func = NULL;

	if (UNEXPECTED(ASYNC_G(exit))) {
		if (scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ACTIVE) {
			uv_stop(&scheduler->loop);
		}
		
		async_fiber_suspend(scheduler);
	} else if (UNEXPECTED(op->flags & ASYNC_OP_FLAG_DEFER)) {
		async_fiber_switch(scheduler, scheduler->caller, ASYNC_FIBER_SUSPEND_APPEND);
	} else {
		async_fiber_switch(scheduler, scheduler->caller, ASYNC_FIBER_SUSPEND_PREPEND);
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
		ASYNC_OP_CHECK_ERROR(op, task->status != ASYNC_TASK_STATUS_RUNNING, "Cannot await in a task that is not running");
		ASYNC_OP_CHECK_ERROR(op, task->flags & ASYNC_TASK_FLAG_DISPOSED, "Task has been destroyed");
		ASYNC_OP_CHECK_ERROR(op, task->scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Cannot await after the task scheduler has been disposed");
		
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
			ASYNC_APPEND_OP(&task->scheduler->operations, op);
		}
		
		task->status = ASYNC_TASK_STATUS_SUSPENDED;
	
		async_fiber_suspend(task->scheduler);
	}
	
	if (UNEXPECTED(cancellable)) {
		ASYNC_DELREF(&context->std);
		
		if (EXPECTED(op->cancel.object != NULL)) {
			ASYNC_LIST_REMOVE(&context->cancel->callbacks, &op->cancel);
			
			op->cancel.object = NULL;
			op->cancel.func = NULL;
		}
	}
	
	if (UNEXPECTED(ASYNC_G(exit))) {
		ASYNC_RESET_OP(op);
		
		zend_bailout();
	}
	
	if (UNEXPECTED(op->status == ASYNC_STATUS_PENDING)) {
		ASYNC_RESET_OP(op);
				
		ASYNC_PREPARE_ERROR(&op->result, "Awaitable has not been resolved");
		
		return FAILURE;
	}
	
	return (op->status == ASYNC_STATUS_RESOLVED) ? SUCCESS : FAILURE;
}

static void await_val(async_task *task, zval *val, zval *return_value, zend_execute_data *execute_data)
{
	zend_class_entry *ce;
	async_task *inner;
	async_task_scheduler *scheduler;
	async_deferred_state *state;
	async_context *context;
	
	context = async_context_get();
	ce = (Z_TYPE_P(val) == IS_OBJECT) ? Z_OBJCE_P(val) : NULL;

	// Check for root level await.
	if (UNEXPECTED(task == NULL)) {
		scheduler = async_task_scheduler_get();

		ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Cannot await after the task scheduler has been disposed");
		ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_RUNNING, "Cannot await in the fiber that is running the task scheduler loop");
		ASYNC_CHECK_ERROR(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_NOWAIT, "Cannot await within the current execution");

		if (ce == async_deferred_awaitable_ce) {
			state = ((async_deferred_awaitable *) Z_OBJ_P(val))->state;

			ASYNC_TASK_DELEGATE_RESULT(state->status, &state->result);
			
			scheduler->op.status = ASYNC_STATUS_RUNNING;
			scheduler->op.flags = 0;
			scheduler->op.callback = continue_op_root;
			scheduler->op.arg = scheduler;
			
			ASYNC_APPEND_OP(&state->operations, &scheduler->op);
		} else {
			inner = (async_task *) Z_OBJ_P(val);
			
			ASYNC_CHECK_ERROR(inner->scheduler != scheduler, "Cannot await a task that runs on a different task scheduler");
			
			// Inline task if it has not been started yet.
			if (UNEXPECTED(inner->fiber == NULL)) {
				async_task_execute_inline(inner, context);
			}
			
			ASYNC_TASK_DELEGATE_RESULT(inner->status, &inner->result);
			
			scheduler->op.status = ASYNC_STATUS_RUNNING;
			scheduler->op.flags = 0;
			scheduler->op.callback = continue_op_root;
			scheduler->op.arg = scheduler;
			
			ASYNC_APPEND_OP(&inner->operations, &scheduler->op);
		}
		
		if (!async_context_is_background(context)) {
			ASYNC_BUSY_ENTER(scheduler);
		}
		
		async_task_scheduler_run_loop(scheduler);
		
		if (!async_context_is_background(context)) {
			ASYNC_BUSY_EXIT(scheduler);
		}
		
		if (UNEXPECTED(ASYNC_G(exit))) {
			ASYNC_RESET_OP(&scheduler->op);
			
			zend_bailout();
		}
		
		ASYNC_TASK_DELEGATE_OP_RESULT(&scheduler->op);
	}

	ASYNC_CHECK_ERROR(task->status != ASYNC_TASK_STATUS_RUNNING, "Cannot await in a task that is not running");
	ASYNC_CHECK_ERROR(task->flags & ASYNC_TASK_FLAG_DISPOSED, "Task has been destroyed");

	ASYNC_CHECK_ERROR(task->scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, "Cannot await after the task scheduler has been disposed");
	ASYNC_CHECK_ERROR(task->flags & ASYNC_TASK_FLAG_NOWAIT, "Cannot await within the current execution");

	if (ce == async_task_ce) {
		inner = (async_task *) Z_OBJ_P(val);

		ASYNC_CHECK_ERROR(inner->scheduler != task->scheduler, "Cannot await a task that runs on a different task scheduler");
		
		// Perform task-inlining optimization where applicable.
		if (UNEXPECTED(inner->fiber == NULL && inner->stack_size <= task->stack_size)) {
			async_task_execute_inline(inner, context);
		}
		
		ASYNC_TASK_DELEGATE_RESULT(inner->status, &inner->result);
		
		task->op.status = ASYNC_STATUS_RUNNING;
		task->op.flags = 0;
		task->op.callback = continue_op_task;
		task->op.arg = task;
		
		ASYNC_APPEND_OP(&inner->operations, &task->op);
	} else {
		state = ((async_deferred_awaitable *) Z_OBJ_P(val))->state;
		
		ASYNC_TASK_DELEGATE_RESULT(state->status, &state->result);
							
		task->op.status = ASYNC_STATUS_RUNNING;
		task->op.flags = 0;
		task->op.callback = continue_op_task;
		task->op.arg = task;

		ASYNC_APPEND_OP(&state->operations, &task->op);
	}

	task->status = ASYNC_TASK_STATUS_SUSPENDED;
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_ENTER(task->scheduler);
	}
	
	async_fiber_suspend(task->scheduler);
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(task->scheduler);
	}
	
	if (UNEXPECTED(ASYNC_G(exit))) {
		ASYNC_RESET_OP(&task->op);
	
		zend_bailout();
	}
	
	ASYNC_TASK_DELEGATE_OP_RESULT(&task->op);
}

static async_task *async_task_object_create(zend_execute_data *call, async_task_scheduler *scheduler, async_context *context)
{
	async_task *task;
	zend_long stack_size;

	ZEND_ASSERT(scheduler != NULL);
	ZEND_ASSERT(context != NULL);

	task = ecalloc(1, sizeof(async_task));

	task->status = ASYNC_TASK_STATUS_INIT;

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

	if (call != NULL && call->func && ZEND_USER_CODE(call->func->common.type)) {
		if (call->func->op_array.filename != NULL) {
			task->file = zend_string_copy(call->func->op_array.filename);
		}

		task->line = call->opline->lineno;
	}
	
	async_task_scheduler_enqueue(task);

	return task;
}

static void async_task_object_destroy(zend_object *object)
{
	async_task *task;

	task = (async_task *) object;
	
	if (task->fiber != NULL) {
		if (task->fiber->flags & ASYNC_FIBER_FLAG_QUEUED) {
			ASYNC_LIST_REMOVE(&task->scheduler->fibers, task->fiber);
		}
	
		async_fiber_destroy(task->fiber);
		task->fiber = NULL;
	}

	if (task->file != NULL) {
		zend_string_release(task->file);
	}
	
	zval_ptr_dtor(&task->result);

	ASYNC_DELREF_CB(task->fci);
	ASYNC_DELREF(&task->context->std);
	
	ASYNC_DELREF(&task->std);
}

static zval *read_task_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
	async_task *task;
	
	char *key;
	
	task = (async_task *) Z_OBJ_P(object);	
	key = Z_STRVAL_P(member);
	
	if (strcmp(key, "status") == 0) {
		ZVAL_STRING(rv, async_status_label(task->status));
	} else if (strcmp(key, "file") == 0) {
		ZVAL_STR(rv, zend_string_copy(task->file));
	} else if (strcmp(key, "line") == 0) {
		ZVAL_LONG(rv, task->line);
	} else {
		rv = &EG(uninitialized_zval);
	}
	
	return rv;
}

static int has_task_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
	zval rv;
	zval *val;

    val = read_task_property(object, member, 0, cache_slot, &rv);

    if (val == &EG(uninitialized_zval)) {
    	return 0;
    }

    switch (has_set_exists) {
    	case ZEND_PROPERTY_EXISTS:
    	case ZEND_PROPERTY_ISSET:
    		zval_ptr_dtor(val);
    		return 1;
    }
 
    convert_to_boolean(val);

    return (Z_TYPE_P(val) == IS_TRUE) ? 1 : 0;
}

static ZEND_METHOD(Task, __construct)
{
	ZEND_PARSE_PARAMETERS_NONE();

	zend_throw_error(NULL, "Tasks must not be constructed by userland code");
}

static ZEND_METHOD(Task, __debugInfo)
{
	ZEND_PARSE_PARAMETERS_NONE();
	
	if (USED_RET()) {
		async_task_get_debug_info((async_task *) Z_OBJ_P(getThis()), return_value);
	}
}

static ZEND_METHOD(Task, async)
{
	async_task *task;

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

	fci.no_separation = 1;

	if (count == 0) {
		fci.param_count = 0;
	} else {
		zend_fcall_info_argp(&fci, count, params);
	}

	task = async_task_object_create(EX(prev_execute_data), async_task_scheduler_get(), async_context_get());
	task->fci = fci;
	task->fcc = fcc;
	
	ASYNC_ADDREF_CB(task->fci);

	RETURN_OBJ(&task->std);
}

static ZEND_METHOD(Task, asyncWithContext)
{
	async_task *task;

	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	uint32_t count;
	uint32_t i;

	zval *ctx;
	zval *params;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, -1)
		Z_PARAM_ZVAL(ctx)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', params, count)
	ZEND_PARSE_PARAMETERS_END();

	for (i = 1; i <= count; i++) {
		ASYNC_CHECK_ERROR(ARG_SHOULD_BE_SENT_BY_REF(fcc.function_handler, i), "Cannot pass async call argument %d by reference", (int) i);
	}

	fci.no_separation = 1;

	if (count == 0) {
		fci.param_count = 0;
	} else {
		zend_fcall_info_argp(&fci, count, params);
	}

	task = async_task_object_create(EX(prev_execute_data), async_task_scheduler_get(), (async_context *) Z_OBJ_P(ctx));
	task->fci = fci;
	task->fcc = fcc;
	
	ASYNC_ADDREF_CB(task->fci);

	RETURN_OBJ(&task->std);
}


static ZEND_METHOD(Task, await)
{
	zval *val;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	await_val(ASYNC_G(task), val, return_value, execute_data);
}

static ZEND_METHOD(Task, __wakeup)
{
	ZEND_PARSE_PARAMETERS_NONE();

	zend_throw_error(NULL, "Unserialization of a task is not allowed");
}

ZEND_BEGIN_ARG_INFO(arginfo_task_ctor, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_task_debug_info, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_task_async, 0, 1, Phalcon\\Async\\Task, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
	ZEND_ARG_VARIADIC_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_task_async_with_context, 0, 2, Phalcon\\Async\\Task, 0)
	ZEND_ARG_OBJ_INFO(0, context, Phalcon\\Async\\Context, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
	ZEND_ARG_VARIADIC_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_task_await, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, awaitable, Phalcon\\Async\\Awaitable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_task_wakeup, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry task_functions[] = {
	ZEND_ME(Task, __construct, arginfo_task_ctor, ZEND_ACC_PRIVATE)
	ZEND_ME(Task, __debugInfo, arginfo_task_debug_info, ZEND_ACC_PUBLIC)
	ZEND_ME(Task, async, arginfo_task_async, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Task, asyncWithContext, arginfo_task_async_with_context, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Task, await, arginfo_task_await, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Task, __wakeup, arginfo_task_wakeup, ZEND_ACC_PUBLIC)
	ZEND_FE_END
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
	
	if (EXPECTED(!uv_is_closing(handle))) {
		uv_close(handle, NULL);
	}
}

static void async_task_scheduler_dispose(async_task_scheduler *scheduler)
{
	async_cancel_cb *cancel;
	async_op *op;
	zval error = {};

	if (UNEXPECTED(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED || ASYNC_G(exit))) {
		return;
	}
	
	if (EXPECTED(!ASYNC_G(exit))) {
		async_task_scheduler_run_loop(scheduler);
	}

	scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_DISPOSED;
	//ASYNC_PREPARE_ERROR(&error, "Task scheduler has been disposed");

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
	
				cancel->func(cancel->object, &error);
			} while (scheduler->shutdown.first != NULL);
		}
		
		async_task_scheduler_run_loop(scheduler);
		
		uv_walk(&scheduler->loop, dispose_walk_cb, scheduler);
		
		async_task_scheduler_run_loop(scheduler);
	} while (uv_loop_alive(&scheduler->loop));

	zval_ptr_dtor(&error);
}

ASYNC_CALLBACK busy_timer(uv_timer_t *handle)
{
	// Dummy timer being used to keep the loop busy...
}

ASYNC_API int async_call_nowait(zend_fcall_info *fci, zend_fcall_info_cache *fcc)
{
	async_task_scheduler *scheduler;
	async_task *task;
	
	int result;
	int flag;
	
	task = ASYNC_G(task);
	
	if (UNEXPECTED(task == NULL)) {
		scheduler = async_task_scheduler_get();
	
		flag = (scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_NOWAIT);
		scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_NOWAIT;
	
		result = zend_call_function(fci, fcc);
	
		if (EXPECTED(!flag)) {
			scheduler->flags &= ~ASYNC_TASK_SCHEDULER_FLAG_NOWAIT;
		}
	} else {
		flag = (task->flags & ASYNC_TASK_FLAG_NOWAIT);
		task->flags |= ASYNC_TASK_FLAG_NOWAIT;
	
		result = zend_call_function(fci, fcc);
	
		if (EXPECTED(!flag)) {
			task->flags &= ~ASYNC_TASK_FLAG_NOWAIT;
		}
	}
	
	return result;
}

static zend_always_inline void async_task_scheduler_enqueue(async_task *task)
{
	async_task_scheduler *scheduler;

	scheduler = task->scheduler;

	ZEND_ASSERT(scheduler != NULL);
	
	if (UNEXPECTED(ASYNC_G(exit))) {
		switch (task->status) {
		case ASYNC_TASK_STATUS_INIT:
			async_task_dispose(task);
			break;
		case ASYNC_TASK_STATUS_SUSPENDED:
			async_task_dispose(task);
			break;
		}
		
		return;
	}
	
	switch (task->status) {
	case ASYNC_TASK_STATUS_INIT:
		ASYNC_ADDREF(&task->std);
	case ASYNC_TASK_STATUS_SUSPENDED:
		break;
	default:
		return;
	}

	if (scheduler->ready.first == NULL) {
		uv_idle_start(&scheduler->idle, dispatch_tasks);
	}

	ASYNC_LIST_APPEND(&scheduler->ready, task);
}

static zend_always_inline void async_task_scheduler_run_loop(async_task_scheduler *scheduler)
{
	if (UNEXPECTED(ASYNC_G(exit))) {
		return;
	}

	ASYNC_CHECK_FATAL(scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_RUNNING, "Duplicate scheduler loop run detected");
	
	scheduler->caller = ASYNC_G(fiber);
	scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_RUNNING;
	
	async_fiber_suspend(scheduler);
	
	scheduler->flags &= ~ASYNC_TASK_SCHEDULER_FLAG_RUNNING;
	scheduler->caller = NULL;
}

ASYNC_FIBER_CALLBACK run_scheduler_fiber(void *arg)
{
	async_task_scheduler *scheduler;
	
	scheduler = (async_task_scheduler *) arg;
	
	ZEND_ASSERT(scheduler != NULL);
	
	ASYNC_G(scheduler) = scheduler;
	ASYNC_G(context) = ASYNC_G(foreground);
	ASYNC_G(task) = NULL;
	
	async_fiber_restore_og(ASYNC_G(context));

	while (1) {
		scheduler->flags |= ASYNC_TASK_SCHEDULER_FLAG_ACTIVE;
	
		uv_run(&scheduler->loop, UV_RUN_DEFAULT);
		
		scheduler->flags &= ~ASYNC_TASK_SCHEDULER_FLAG_ACTIVE;
		
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
	
	return scheduler;
}

ASYNC_CALLBACK walk_loop_cb(uv_handle_t *handle, void *arg)
{
	int *count;
	
	count = (int *) arg;
	
	(*count)++;
	
	printf(">> UV HANDLE LEAKED: [%d] %s -> %d\n", handle->type, uv_handle_type_name(handle->type), uv_is_closing(handle));
	
	if (!uv_is_closing(handle)) {
		uv_close(handle, NULL);
	}
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

	int code;

	scheduler = (async_task_scheduler *)object;

	async_task_scheduler_dispose(scheduler);
	
	uv_close((uv_handle_t *) &scheduler->busy, NULL);
	uv_close((uv_handle_t *) &scheduler->idle, NULL);
	
	// Run loop again to cleanup idle watcher.
	uv_run(&scheduler->loop, UV_RUN_DEFAULT);

	ZEND_ASSERT(!uv_loop_alive(&scheduler->loop));
	ZEND_ASSERT(debug_handles(&scheduler->loop) == 0);
	
	code = uv_loop_close(&scheduler->loop);
	ZEND_ASSERT(code == 0);
	
	ZEND_ASSERT(scheduler->ready.first == NULL);
	ZEND_ASSERT(scheduler->fibers.first == NULL);
	
	zend_object_std_dtor(object);
	
	if (scheduler->runner != NULL) {
		if (scheduler->runner->flags & ASYNC_FIBER_FLAG_QUEUED) {
			ASYNC_LIST_REMOVE(&scheduler->fibers, scheduler->runner);
		}

		async_fiber_destroy(scheduler->runner);
		scheduler->runner = NULL;
	}
}

static ZEND_METHOD(TaskScheduler, __construct)
{
	ZEND_PARSE_PARAMETERS_NONE();

	zend_throw_error(NULL, "Task scheduler must not be constructed by userland code");
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
		zend_hash_index_update(Z_ARRVAL_P(&args[0]), i, async_task_get_debug_info(task, &obj));

		task = task->next;
		i++;
	}

	fci->param_count = 1;
	fci->params = args;
	fci->retval = &retval;
	fci->no_separation = 1;

	zend_call_function(fci, fcc);

	zval_ptr_dtor(&args[0]);
	zval_ptr_dtor(&retval);

	ASYNC_CHECK_FATAL(EG(exception), "Must not throw an error from scheduler inspector");
}

static ZEND_METHOD(TaskScheduler, run)
{
	async_task_scheduler *scheduler;
	async_task_scheduler *prev;
	
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	
	zend_fcall_info fci2;
	zend_fcall_info_cache fcc2;
	
	zval retval;
	zval error;
	
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
	
	fci.retval = &retval;
	fci.param_count = 0;
	
	zend_call_function(&fci, &fcc);
	
	if (UNEXPECTED(EG(exception))) {
		ZVAL_OBJ(&error, EG(exception));
		Z_ADDREF_P(&error);
		
		zend_clear_exception();
	} else {
		ZVAL_UNDEF(&error);
		
		if (ZEND_NUM_ARGS() > 1) {
			debug_pending_tasks(scheduler, &fci2, &fcc2);
		}
	}
	
	async_task_scheduler_dispose(scheduler);
	
	ASYNC_G(scheduler) = prev;
	
	async_task_scheduler_unref(scheduler);
	
	if (UNEXPECTED(ASYNC_G(exit))) {
		zend_bailout();
	}
	
	if (Z_TYPE_P(&error) != IS_UNDEF) {
		execute_data->opline--;
		zend_throw_exception_internal(&error);
		execute_data->opline++;
		return;
	}
	
	RETURN_ZVAL(&retval, 1, 1);
}

static ZEND_METHOD(TaskScheduler, runWithContext)
{
	async_task_scheduler *scheduler;
	async_task_scheduler *prev;
	async_context *context;
	async_context *scope;
	
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	
	zend_fcall_info fci2;
	zend_fcall_info_cache fcc2;
	
	zval *ctx;
	zval retval;
	zval error;
	
	fci2 = empty_fcall_info;
	fcc2 = empty_fcall_info_cache;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 3)
		Z_PARAM_ZVAL(ctx)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC_EX(fci2, fcc2, 1, 0)
	ZEND_PARSE_PARAMETERS_END();
	
	scope = (async_context *) Z_OBJ_P(ctx);
	
	scheduler = async_task_scheduler_object_create();
	
	prev = ASYNC_G(scheduler);
	ASYNC_G(scheduler) = scheduler;
	
	context = ASYNC_G(context);
	ASYNC_G(context) = scope;
	
	async_fiber_capture_og(context);
	async_fiber_restore_og(scope);
	
	fci.retval = &retval;
	fci.param_count = 0;
	
	zend_call_function(&fci, &fcc);
	
	ASYNC_G(context) = context;
	
	async_fiber_capture_og(scope);
	async_fiber_restore_og(context);
	
	if (UNEXPECTED(EG(exception))) {
		ZVAL_OBJ(&error, EG(exception));
		Z_ADDREF_P(&error);
		
		zend_clear_exception();
	} else {
		ZVAL_UNDEF(&error);
		
		if (ZEND_NUM_ARGS() > 2) {
			debug_pending_tasks(scheduler, &fci2, &fcc2);
		}
	}
	
	async_task_scheduler_dispose(scheduler);
	async_task_scheduler_unref(scheduler);
	
	ASYNC_G(scheduler) = prev;
	
	if (UNEXPECTED(ASYNC_G(exit))) {
		zend_bailout();
	}
	
	if (Z_TYPE_P(&error) != IS_UNDEF) {
		execute_data->opline--;
		zend_throw_exception_internal(&error);
		execute_data->opline++;
		return;
	}
	
	RETURN_ZVAL(&retval, 1, 1);
}

static ZEND_METHOD(TaskScheduler, __wakeup)
{
	ZEND_PARSE_PARAMETERS_NONE();

	zend_throw_error(NULL, "Unserialization of a task scheduler is not allowed");
}

ZEND_BEGIN_ARG_INFO(arginfo_task_scheduler_ctor, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_task_scheduler_run, 0, 0, 1)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
	ZEND_ARG_CALLABLE_INFO(0, finalizer, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_task_scheduler_run_with_context, 0, 0, 2)
ZEND_ARG_OBJ_INFO(0, context, Phalcon\\Async\\Context, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
	ZEND_ARG_CALLABLE_INFO(0, finalizer, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_task_scheduler_wakeup, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry task_scheduler_functions[] = {
	ZEND_ME(TaskScheduler, __construct, arginfo_task_scheduler_ctor, ZEND_ACC_PRIVATE)
	ZEND_ME(TaskScheduler, run, arginfo_task_scheduler_run, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(TaskScheduler, runWithContext, arginfo_task_scheduler_run_with_context, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(TaskScheduler, __wakeup, arginfo_task_scheduler_wakeup, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


void async_task_ce_register()
{
	zend_class_entry ce;
	zend_uchar opcode;

	opcode = ZEND_VM_LAST_OPCODE + 1;
	
	while (1) {
		if (opcode == 255) {
			return;
		} else if (zend_get_user_opcode_handler(opcode) == NULL) {
			break;
		}
		opcode++;
	}

	zend_set_user_opcode_handler(opcode, task_run_opcode_handler);
	
	memset(task_run_op, 0, sizeof(task_run_op));
	task_run_op[0].opcode = opcode;
	zend_vm_set_opcode_handler_ex(task_run_op, 0, 0, 0);
	task_run_op[1].opcode = opcode;
	zend_vm_set_opcode_handler_ex(task_run_op + 1, 0, 0, 0);

	memset(&task_run_func, 0, sizeof(task_run_func));
	ASYNC_STRP(task_run_func.function_name, "ext-async");	
	task_run_func.type = ZEND_USER_FUNCTION;
	task_run_func.filename = ZSTR_EMPTY_ALLOC();
	task_run_func.opcodes = task_run_op;
	task_run_func.last_try_catch = 1;
	task_run_func.try_catch_array = &task_terminate_try_catch_array;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Task", task_functions);
	async_task_ce = zend_register_internal_class(&ce);
	async_task_ce->ce_flags |= ZEND_ACC_FINAL;
	async_task_ce->serialize = zend_class_serialize_deny;
	async_task_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_task_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_task_handlers.free_obj = async_task_object_destroy;
	async_task_handlers.clone_obj = NULL;
	async_task_handlers.has_property = has_task_property;
	async_task_handlers.read_property = read_task_property;

	zend_class_implements(async_task_ce, 1, async_awaitable_ce);
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\TaskScheduler", task_scheduler_functions);
	async_task_scheduler_ce = zend_register_internal_class(&ce);
	async_task_scheduler_ce->ce_flags |= ZEND_ACC_FINAL;
	async_task_scheduler_ce->serialize = zend_class_serialize_deny;
	async_task_scheduler_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_task_scheduler_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_task_scheduler_handlers.free_obj = async_task_scheduler_object_destroy;
	async_task_scheduler_handlers.clone_obj = NULL;
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
}

void async_task_scheduler_run()
{
	async_task_scheduler *scheduler;

	scheduler = ASYNC_G(executor);

	if (scheduler != NULL) {
		async_task_scheduler_dispose(scheduler);
	}
}

void async_task_scheduler_shutdown()
{
	async_task_scheduler *scheduler;

	scheduler = ASYNC_G(executor);
	
	if (scheduler != NULL && !ASYNC_G(exit)) {
		ASYNC_G(executor) = NULL;

		async_task_scheduler_dispose(scheduler);
		async_task_scheduler_unref(scheduler);
	}

	async_fiber_destroy(ASYNC_G(root));
}

void async_task_ce_unregister()
{
	zend_string_free(task_run_func.function_name);
	task_run_func.function_name = NULL;
}

#endif
