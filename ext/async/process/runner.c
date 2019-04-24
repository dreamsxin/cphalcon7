
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

#if PHALCON_USE_UV

#include "async/async_process.h"
#include "async/async_stream.h"
#include "async/async_pipe.h"

#ifdef ZEND_WIN32
#include <Zend/zend_smart_str.h>
#endif

ASYNC_API zend_class_entry *async_process_ce;
ASYNC_API zend_class_entry *async_readable_process_pipe_ce;
ASYNC_API zend_class_entry *async_writable_process_pipe_ce;

static zend_object_handlers async_process_handlers;
static zend_object_handlers async_readable_process_pipe_handlers;
static zend_object_handlers async_writable_process_pipe_handlers;

typedef struct _async_process async_process;

typedef struct {
	async_process *process;

	uv_pipe_t handle;
	
	uint8_t flags;

	async_stream *stream;

	zval error;
} async_writable_process_pipe_state;

typedef struct {
	async_process *process;

	uv_pipe_t handle;
	
	uint8_t flags;

	async_stream *stream;

	zval error;
} async_readable_process_pipe_state;

struct _async_process {
	/* Fiber PHP object handle. */
	zend_object std;

	uint8_t flags;

	/* Task scheduler providing the event loop. */
	async_task_scheduler *scheduler;

	/* Process handle providing access to the running process instance. */
	uv_process_t handle;

	/* Process configuration, provided by process builder. */
	uv_process_options_t options;

	/* Process ID, will be 0 if the process has finished execution. */
	int pid;

	/* Exit code returned by the process, will be -1 if the process has not terminated yet. */
	int status;

	async_writable_process_pipe_state stdin_state;
	async_readable_process_pipe_state stdout_state;
	async_readable_process_pipe_state stderr_state;
	
	async_pipe *ipc;

	zend_uchar pipes;

	/* Inlined cancel callback being used to dispose of the process. */
	async_cancel_cb cancel;

	/* Exit code / process termination observers. */
	async_op_list observers;

	/* Use UV_PROCESS_DETACHED */
	zend_bool detached;
};

typedef struct {
	/* Fiber PHP object handle. */
	zend_object std;

	async_readable_process_pipe_state *state;
} async_readable_process_pipe;

typedef struct {
	/* Fiber PHP object handle. */
	zend_object std;

	async_writable_process_pipe_state *state;
} async_writable_process_pipe;

static async_process *async_process_object_create(unsigned int detached);
static async_readable_process_pipe *async_readable_process_pipe_object_create(async_process *process, async_readable_process_pipe_state *state);
static async_writable_process_pipe *async_writable_process_pipe_object_create(async_process *process, async_writable_process_pipe_state *state);


static zend_always_inline int get_pipe_index(async_process *proc, int pipe)
{
#ifdef ZEND_WIN32
	if (proc->flags & ASYNC_PROCESS_FLAG_INTERACTIVE_SHELL && pipe == ASYNC_PROCESS_STDOUT) {
		return ASYNC_PROCESS_IPC;
	}
#endif

	return pipe;
}

ASYNC_CALLBACK dispose_process(uv_handle_t *handle)
{
	async_process *proc;
	async_op *op;

	proc = (async_process *) handle->data;

	while (proc->observers.first != NULL) {
		ASYNC_NEXT_OP(&proc->observers, op);
		ASYNC_FINISH_OP(op);
	}

	ASYNC_DELREF(&proc->std);
}

ASYNC_CALLBACK shutdown_process(void *obj, zval *error)
{
	async_process *proc;

	proc = (async_process *) obj;

	proc->cancel.func = NULL;

	if (!proc->detached) {
#ifdef PHP_WIN32
		uv_process_kill(&proc->handle, ASYNC_SIGNAL_SIGINT);
#else
		uv_process_kill(&proc->handle, ASYNC_SIGNAL_SIGKILL);
#endif
	}
	ASYNC_ADDREF(&proc->std);

	uv_close((uv_handle_t *) &proc->handle, dispose_process);
}

static zend_always_inline void create_readable_state(async_process *process, async_readable_process_pipe_state *state, int i)
{
	uv_pipe_init(&process->scheduler->loop, &state->handle, 0);

	state->handle.data = state;
	state->process = process;
	
	process->options.stdio[i].data.stream = (uv_stream_t *) &state->handle;
	process->pipes++;

	state->stream = async_stream_init((uv_stream_t *) &state->handle, 0);
}

static zend_always_inline void dispose_read_state(uv_handle_t *handle)
{
	async_readable_process_pipe_state *state;

	state = (async_readable_process_pipe_state *) handle->data;
	state->process->pipes--;

	async_stream_free(state->stream);

	ASYNC_DELREF(&state->process->std);
}

static zend_always_inline void create_writable_state(async_process *process, async_writable_process_pipe_state *state, int i)
{
	uv_pipe_init(&process->scheduler->loop, &state->handle, 0);

	state->handle.data = state;
	state->process = process;

	process->options.stdio[i].data.stream = (uv_stream_t *) &state->handle;
	process->pipes++;

	state->stream = async_stream_init((uv_stream_t *) &state->handle, 0);
}

static zend_always_inline void dispose_write_state(uv_handle_t *handle)
{
	async_writable_process_pipe_state *state;

	state = (async_writable_process_pipe_state *) handle->data;
	state->process->pipes--;

	async_stream_free(state->stream);

	ASYNC_DELREF(&state->process->std);
}

ASYNC_CALLBACK exit_process(uv_process_t *handle, int64_t status, int signal)
{
	async_process *proc;

	proc = (async_process *) handle->data;

	proc->pid = 0;
	proc->status = (int) status;

	if (proc->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&proc->scheduler->shutdown, &proc->cancel);

		proc->cancel.func(proc, NULL);
	}
}

static void prepare_process(async_process_builder *builder, async_process *proc, zval *params, uint32_t count)
{
	char **args;
	
	uint32_t i;
	uint32_t j;
	
	args = ecalloc(builder->argc + count + 2, sizeof(char *));
	args[0] = ZSTR_VAL(builder->command);
	
	for (i = 1, j = 0; j < builder->argc; j++) {
		args[i++] = Z_STRVAL_P(&builder->argv[j]);
	}
	
	for (j = 0; j < count; j++) {
		args[i++] = Z_STRVAL_P(&params[j]);
	}

	args[i++] = NULL;

	proc->flags = builder->flags;
		
	proc->options.file = ZSTR_VAL(builder->command);
	proc->options.cwd = ZSTR_VAL(builder->cwd);
	proc->options.args = args;
	proc->options.stdio_count = 3;
	proc->options.stdio = builder->stdio;
	proc->options.exit_cb = exit_process;

#ifdef ZEND_WIN32
	proc->options.flags = UV_PROCESS_WINDOWS_HIDE | UV_PROCESS_WINDOWS_HIDE_CONSOLE | UV_PROCESS_WINDOWS_HIDE_GUI;
	
	if (proc->flags & ASYNC_PROCESS_FLAG_INTERACTIVE_SHELL) {
		proc->options.stdio_count = 4;
		
		proc->options.stdio[ASYNC_PROCESS_IPC] = proc->options.stdio[ASYNC_PROCESS_STDOUT];
		proc->options.stdio[ASYNC_PROCESS_STDOUT].flags = UV_IGNORE;
	}
#endif
	if (proc->detached) {
		proc->options.flags |= UV_PROCESS_DETACHED;
	}

	if (Z_TYPE_P(&builder->env) != IS_UNDEF && zend_hash_num_elements(Z_ARRVAL_P(&builder->env)) > 0) {
		proc->options.env = async_process_create_env(Z_ARRVAL_P(&builder->env), builder->flags & ASYNC_PROCESS_FLAG_INHERIT_ENV);
	} else if (!(builder->flags & ASYNC_PROCESS_FLAG_INHERIT_ENV)) {
		proc->options.env = emalloc(sizeof(char *));
		proc->options.env[0] = NULL;
	}
}

static async_process *async_process_object_create(unsigned int detached)
{
	async_process *proc;

	proc = ecalloc(1, sizeof(async_process));

	zend_object_std_init(&proc->std, async_process_ce);
	proc->std.handlers = &async_process_handlers;

	proc->handle.data = proc;

	proc->scheduler = async_task_scheduler_ref();

	proc->cancel.object = proc;
	proc->cancel.func = shutdown_process;

	proc->status = -1;
	proc->detached = detached;

	ZVAL_UNDEF(&proc->stdin_state.error);
	ZVAL_UNDEF(&proc->stdout_state.error);
	ZVAL_UNDEF(&proc->stderr_state.error);

	return proc;
}

static void async_process_object_dtor(zend_object *object)
{
	async_process *proc;

	proc = (async_process *) object;

	if (proc->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&proc->scheduler->shutdown, &proc->cancel);

		proc->cancel.func(proc, NULL);
	}
}

static void async_process_object_destroy(zend_object *object)
{
	async_process *proc;

	proc = (async_process *) object;
	
	if (proc->flags & ASYNC_PROCESS_FLAG_IPC) {
		ASYNC_DELREF(&proc->ipc->std);
	}

	zval_ptr_dtor(&proc->stdin_state.error);
	zval_ptr_dtor(&proc->stdout_state.error);
	zval_ptr_dtor(&proc->stderr_state.error);

	async_task_scheduler_unref(proc->scheduler);

	zend_object_std_dtor(&proc->std);
}

int async_process_execute(async_process_builder *builder, uint32_t argc, zval *argv)
{
	async_process *proc;
	async_context *context;
	async_op *op;

	uint32_t i;
	int result;
	int code;
	int x;

	for (i = 0; i < 4; i++) {
		if (builder->stdio[i].flags & UV_CREATE_PIPE) {
			zend_throw_error(NULL, "Cannot use pipes in execute(), use start() instead");
			return FAILURE;
		}
	}

	proc = async_process_object_create(builder->detached);

	prepare_process(builder, proc, argv, argc);

	code = uv_spawn(&proc->scheduler->loop, &proc->handle, &proc->options);

	efree(proc->options.args);

	if (proc->options.env != NULL) {
		x = 0;

		while (proc->options.env[x] != NULL) {
			efree(proc->options.env[x++]);
		}

		efree(proc->options.env);
	}

	if (UNEXPECTED(code != 0)) {
		zend_throw_error(NULL, "Failed to launch process \"%s\": %s", ZSTR_VAL(builder->command), uv_strerror(code));
		ASYNC_DELREF(&proc->std);		
		return FAILURE;
	}

	context = async_context_get();

	if (async_context_is_background(context)) {
		uv_unref((uv_handle_t *) &proc->handle);
	}

	ASYNC_ALLOC_OP(op);
	ASYNC_APPEND_OP(&proc->observers, op);
	
	result = FAILURE;

	if (UNEXPECTED(async_await_op(op) == FAILURE)) {
		ASYNC_FORWARD_OP_ERROR(op);
	}

	if (op->flags & ASYNC_OP_FLAG_CANCELLED) {
#ifdef PHP_WIN32
		uv_process_kill(&proc->handle, ASYNC_SIGNAL_SIGINT);
#else
		uv_process_kill(&proc->handle, ASYNC_SIGNAL_SIGKILL);
#endif
	}

	ASYNC_FREE_OP(op);

	if (EXPECTED(!EG(exception))) {
		result = proc->status;
	}

	ASYNC_DELREF(&proc->std);
	
	return result;
}

zend_object *async_process_start(async_process_builder *builder, uint32_t argc, zval *argv)
{
	async_process *proc;

	int code;
	int x;
	
	proc = async_process_object_create(builder->detached);

	prepare_process(builder, proc, argv, argc);

	if (proc->options.stdio[ASYNC_PROCESS_STDIN].flags & UV_CREATE_PIPE) {
		create_writable_state(proc, &proc->stdin_state, ASYNC_PROCESS_STDIN);
	}

	if (proc->options.stdio[ASYNC_PROCESS_STDOUT].flags & UV_CREATE_PIPE) {
		create_readable_state(proc, &proc->stdout_state, ASYNC_PROCESS_STDOUT);
	}

	if (proc->options.stdio[ASYNC_PROCESS_STDERR].flags & UV_CREATE_PIPE) {
		create_readable_state(proc, &proc->stderr_state, ASYNC_PROCESS_STDERR);
	}
	
	if (proc->flags & ASYNC_PROCESS_FLAG_INTERACTIVE_SHELL && proc->options.stdio[ASYNC_PROCESS_IPC].flags & UV_CREATE_PIPE) {
		create_readable_state(proc, &proc->stdout_state, ASYNC_PROCESS_IPC);
	}
	
	if (proc->flags & ASYNC_PROCESS_FLAG_IPC) {
		proc->ipc = async_pipe_init_ipc();
	
		proc->options.stdio_count = 4;
		
		proc->options.stdio[ASYNC_PROCESS_IPC].flags = UV_CREATE_PIPE | UV_READABLE_PIPE | UV_WRITABLE_PIPE;
		proc->options.stdio[ASYNC_PROCESS_IPC].data.stream = (uv_stream_t *) &proc->ipc->handle;
	}
	
	code = uv_spawn(&proc->scheduler->loop, &proc->handle, &proc->options);

	efree(proc->options.args);

	if (proc->options.env != NULL) {
		x = 0;

		while (proc->options.env[x] != NULL) {
			efree(proc->options.env[x++]);
		}

		efree(proc->options.env);
	}

	if (UNEXPECTED(code != 0)) {
		zend_throw_error(NULL, "Failed to launch process \"%s\": %s", ZSTR_VAL(builder->command), uv_strerror(code));
		ASYNC_DELREF(&proc->std);
		return NULL;
	}

	uv_unref((uv_handle_t *) &proc->handle);

	ASYNC_LIST_APPEND(&proc->scheduler->shutdown, &proc->cancel);

	proc->pid = proc->handle.pid;

	return &proc->std;
}

static ZEND_METHOD(Process, isForked)
{
	ZEND_PARSE_PARAMETERS_NONE();

	RETURN_BOOL(ASYNC_G(forked));
}

static ZEND_METHOD(Process, forked)
{
	async_pipe *pipe;
	
	int code;

	ZEND_PARSE_PARAMETERS_NONE();
	
	ASYNC_CHECK_ERROR(!ASYNC_G(forked), "Access to IPC pipe requires using fork() with a process builder");
	
	pipe = async_pipe_init_ipc();
	
	code = uv_pipe_open(&pipe->handle, ASYNC_PROCESS_IPC);
	
	ASYNC_CHECK_ERROR(code != 0, "Failed to open process IPC pipe: %s", uv_strerror(code));

	RETURN_OBJ(&pipe->std);
}

static ZEND_METHOD(Process, __debugInfo)
{
	async_process *proc;

	ZEND_PARSE_PARAMETERS_NONE();

	if (USED_RET()) {
		proc = (async_process *) Z_OBJ_P(getThis());
		
		array_init(return_value);
		
		add_assoc_long(return_value, "pid", proc->pid);
		add_assoc_long(return_value, "exit_code", proc->status);
		add_assoc_bool(return_value, "running", proc->status < 0);
	}
}

static ZEND_METHOD(Process, isRunning)
{
	async_process *proc;

	ZEND_PARSE_PARAMETERS_NONE();

	proc = (async_process *) Z_OBJ_P(getThis());

	RETURN_BOOL(proc->status < 0);
}

static ZEND_METHOD(Process, getPid)
{
	async_process *proc;

	ZEND_PARSE_PARAMETERS_NONE();

	proc = (async_process *) Z_OBJ_P(getThis());

	RETURN_LONG(proc->pid);
}

static ZEND_METHOD(Process, getStdin)
{
	async_process *proc;
	async_writable_process_pipe *pipe;

	ZEND_PARSE_PARAMETERS_NONE();

	proc = (async_process *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(!(proc->options.stdio[ASYNC_PROCESS_STDIN].flags & UV_CREATE_PIPE), "Cannot access STDIN because it is not configured to be a pipe");

	pipe = async_writable_process_pipe_object_create(proc, &proc->stdin_state);

	RETURN_OBJ(&pipe->std);
}

static ZEND_METHOD(Process, getStdout)
{
	async_process *proc;
	async_readable_process_pipe *pipe;

	ZEND_PARSE_PARAMETERS_NONE();

	proc = (async_process *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(!(proc->options.stdio[get_pipe_index(proc, ASYNC_PROCESS_STDOUT)].flags & UV_CREATE_PIPE), "Cannot access STDOUT because it is not configured to be a pipe");

	pipe = async_readable_process_pipe_object_create(proc, &proc->stdout_state);

	RETURN_OBJ(&pipe->std);
}

static ZEND_METHOD(Process, getStderr)
{
	async_process *proc;
	async_readable_process_pipe *pipe;

	ZEND_PARSE_PARAMETERS_NONE();

	proc = (async_process *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(!(proc->options.stdio[ASYNC_PROCESS_STDERR].flags & UV_CREATE_PIPE), "Cannot access STDERR because it is not configured to be a pipe");

	pipe = async_readable_process_pipe_object_create(proc, &proc->stderr_state);

	RETURN_OBJ(&pipe->std);
}

static ZEND_METHOD(Process, getIpc)
{
	async_process *proc;
	
	ZEND_PARSE_PARAMETERS_NONE();

	proc = (async_process *) Z_OBJ_P(getThis());
	
	ASYNC_CHECK_ERROR(!(proc->flags & ASYNC_PROCESS_FLAG_IPC), "Access to IPC pipe requires using fork() with a process builder");
	
	ASYNC_ADDREF(&proc->ipc->std);
	
	RETURN_OBJ(&proc->ipc->std);
}

static ZEND_METHOD(Process, signal)
{
	async_process *proc;

	zend_long signum;
	int code;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(signum)
	ZEND_PARSE_PARAMETERS_END();

	proc = (async_process *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(proc->status >= 0, "Cannot signal a process that has alredy been terminated");

	code = uv_process_kill(&proc->handle, (int) signum);

	ASYNC_CHECK_ERROR(code != 0, "Failed to signal process: %s", uv_strerror(code));
}

static ZEND_METHOD(Process, join)
{
	async_process *proc;
	async_context *context;
	async_op *op;

	ZEND_PARSE_PARAMETERS_NONE();

	proc = (async_process *) Z_OBJ_P(getThis());

	if (proc->status >= 0) {
		RETURN_LONG(proc->status);
	}
	
	context = async_context_get();

	if (!async_context_is_background(context)) {
		uv_ref((uv_handle_t *) &proc->handle);
	}

	ASYNC_ALLOC_OP(op);
	ASYNC_APPEND_OP(&proc->observers, op);

	if (UNEXPECTED(async_await_op(op) == FAILURE)) {
		ASYNC_FORWARD_OP_ERROR(op);
	}

	if (!async_context_is_background(context)) {
		uv_unref((uv_handle_t *) &proc->handle);
	}

	ASYNC_FREE_OP(op);

	if (EXPECTED(!EG(exception))) {
		RETURN_LONG(proc->status);
	}
}

ZEND_BEGIN_ARG_INFO(arginfo_process_debug_info, 0)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_is_forked, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_is_forked, 0, 0, _IS_BOOL, NULL, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_forked, 0, 0, Phalcon\\Async\\Network\\Pipe, 0)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_is_running, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_get_pid, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_is_running, 0, 0, _IS_BOOL, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_get_pid, 0, 0, IS_LONG, NULL, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_get_stdin, 0, 0, Phalcon\\Async\\Stream\\WritableStream, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_get_stdout, 0, 0, Phalcon\\Async\\Stream\\ReadableStream, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_get_stderr, 0, 0, Phalcon\\Async\\Stream\\ReadableStream, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_get_ipc, 0, 0, Phalcon\\Async\\Network\\Pipe, 0)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_signal, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_join, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_signal, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_join, 0, 0, IS_LONG, NULL, 0)
ZEND_END_ARG_INFO()
#endif

static const zend_function_entry async_process_functions[] = {
	ZEND_ME(Process, __debugInfo, arginfo_process_debug_info, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, isForked, arginfo_process_is_forked, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Process, forked, arginfo_process_forked, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Process, isRunning, arginfo_process_is_running, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, getPid, arginfo_process_get_pid, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, getStdin, arginfo_process_get_stdin, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, getStdout, arginfo_process_get_stdout, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, getStderr, arginfo_process_get_stderr, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, getIpc, arginfo_process_get_ipc, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, signal, arginfo_process_signal, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, join, arginfo_process_join, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static async_readable_process_pipe *async_readable_process_pipe_object_create(async_process *process, async_readable_process_pipe_state *state)
{
	async_readable_process_pipe *pipe;

	pipe = ecalloc(1, sizeof(async_readable_process_pipe));

	zend_object_std_init(&pipe->std, async_readable_process_pipe_ce);
	pipe->std.handlers = &async_readable_process_pipe_handlers;

	pipe->state = state;

	ASYNC_ADDREF(&pipe->state->process->std);

	return pipe;
}

static void async_readable_process_pipe_object_destroy(zend_object *object)
{
	async_readable_process_pipe *pipe;

	pipe = (async_readable_process_pipe *) object;

	pipe->state->process->pipes--;

	async_stream_free(pipe->state->stream);

	ASYNC_DELREF(&pipe->state->process->std);
	ASYNC_DELREF(&pipe->state->process->std);

	zend_object_std_dtor(&pipe->std);
}

static ZEND_METHOD(ReadablePipe, close)
{
	async_readable_process_pipe *pipe;

	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	pipe = (async_readable_process_pipe *) Z_OBJ_P(getThis());

	if (Z_TYPE_P(&pipe->state->error) != IS_UNDEF) {
		return;
	}
	
	ASYNC_PREPARE_EXCEPTION(&pipe->state->error, async_stream_closed_exception_ce, "Pipe has been closed");
	
	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&pipe->state->error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}

	if (!(pipe->state->stream->flags & ASYNC_STREAM_CLOSED)) {
		ASYNC_ADDREF(&pipe->state->process->std);

		async_stream_close(pipe->state->stream, getThis());
	}
}

static ZEND_METHOD(ReadablePipe, read)
{
	async_readable_process_pipe *pipe;
	async_stream_read_req read;

	zval *hint;
	size_t len;

	hint = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(hint)
	ZEND_PARSE_PARAMETERS_END();

	pipe = (async_readable_process_pipe *) Z_OBJ_P(getThis());

	if (hint == NULL || Z_TYPE_P(hint) == IS_NULL) {
		len = pipe->state->stream->buffer.size;
	} else if (Z_LVAL_P(hint) < 1) {
		zend_throw_error(NULL, "Invalid read length: %d", (int) Z_LVAL_P(hint));
		return;
	} else {
		len = (size_t) Z_LVAL_P(hint);
	}

	if (UNEXPECTED(Z_TYPE_P(&pipe->state->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&pipe->state->error);
		return;
	}
	
	read.in.len = len;
	read.in.buffer = NULL;
	read.in.handle = NULL;
	read.in.timeout = 0;
	read.in.flags = 0;
	
	if (EXPECTED(SUCCESS == async_stream_read(pipe->state->stream, &read))) {
		if (EXPECTED(read.out.len)) {
			RETURN_STR(read.out.str);
		}
		
		return;
	}

	forward_stream_read_error(&read);
}

static const zend_function_entry async_readable_process_pipe_functions[] = {
	ZEND_ME(ReadablePipe, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	ZEND_ME(ReadablePipe, read, arginfo_readable_stream_read, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static async_writable_process_pipe *async_writable_process_pipe_object_create(async_process *process, async_writable_process_pipe_state *state)
{
	async_writable_process_pipe *pipe;

	pipe = ecalloc(1, sizeof(async_writable_process_pipe));

	zend_object_std_init(&pipe->std, async_writable_process_pipe_ce);
	pipe->std.handlers = &async_writable_process_pipe_handlers;

	pipe->state = state;

	ASYNC_ADDREF(&process->std);

	return pipe;
}

static void async_writable_process_pipe_object_destroy(zend_object *object)
{
	async_writable_process_pipe *pipe;

	pipe = (async_writable_process_pipe *) object;

	pipe->state->process->pipes--;

	async_stream_free(pipe->state->stream);

	ASYNC_DELREF(&pipe->state->process->std);
	ASYNC_DELREF(&pipe->state->process->std);

	zend_object_std_dtor(&pipe->std);
}

static ZEND_METHOD(WritablePipe, close)
{
	async_writable_process_pipe *pipe;

	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	pipe = (async_writable_process_pipe *) Z_OBJ_P(getThis());

	if (UNEXPECTED(Z_TYPE_P(&pipe->state->error) != IS_UNDEF)) {
		return;
	}

	ASYNC_PREPARE_EXCEPTION(&pipe->state->error, async_stream_closed_exception_ce, "Pipe has been closed");

	if (!(pipe->state->stream->flags & ASYNC_STREAM_CLOSED)) {
		ASYNC_ADDREF(&pipe->state->process->std);

		async_stream_close(pipe->state->stream, getThis());
	}
}

static ZEND_METHOD(WritablePipe, write)
{
	async_writable_process_pipe *pipe;
	async_stream_write_req write;

	zend_string *data;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(data)
	ZEND_PARSE_PARAMETERS_END();

	pipe = (async_writable_process_pipe *) Z_OBJ_P(getThis());

	if (UNEXPECTED(Z_TYPE_P(&pipe->state->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&pipe->state->error);
		return;
	}
	
#ifdef ZEND_WIN32
	if (pipe->state->process->flags & ASYNC_PROCESS_FLAG_INTERACTIVE_SHELL) {
		smart_str str = {0};
		char *in;
		
		int i;
		int j;
		int len;
		
		in = ZSTR_VAL(data);
		
		for (len = (int) ZSTR_LEN(data), j = 0, i = 0; i < len; i++) {
			if (in[i] == '\r' && i < (len - 1) && in[i + 1] == '\n') {
				if (i == j) {
					i++;
					j = i + 1;
					
					continue;
				}
			
				smart_str_appendl(&str, in + j, i - j);
				smart_str_appends(&str, " 1>&3 \n");
				
				j = i + 2;
				i++;
			} else if (in[i] == '\n') {
				if (i == j) {
					j = i + 1;
					
					continue;
				}
			
				smart_str_appendl(&str, in + j, i - j);
				smart_str_appends(&str, " 1>&3 \n");
				
				j = i + 1;
			} else {
				switch (in[i]) {
					case ' ':
					case '\t':
						if (i == j) {
							j++;
						}
				}
			}
		}
		
		data = smart_str_extract(&str);
	}
#endif
	
	write.in.len = ZSTR_LEN(data);
	write.in.buffer = ZSTR_VAL(data);
	write.in.handle = NULL;
	write.in.str = data;
	write.in.ref = getThis();
	write.in.flags = 0;

	if (UNEXPECTED(FAILURE == async_stream_write(pipe->state->stream, &write))) {
		forward_stream_write_error(&write);
	}
	
#ifdef ZEND_WIN32
	if (pipe->state->process->flags & ASYNC_PROCESS_FLAG_INTERACTIVE_SHELL) {
		zend_string_release(data);
	}
#endif
}

static const zend_function_entry async_writable_process_pipe_functions[] = {
	ZEND_ME(WritablePipe, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	ZEND_ME(WritablePipe, write, arginfo_writable_stream_write, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


void async_process_ce_register()
{
	zend_class_entry ce;
	
	async_process_builder_ce_register();

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Process", async_process_functions);
	async_process_ce = zend_register_internal_class(&ce);
	async_process_ce->ce_flags |= ZEND_ACC_FINAL;
	async_process_ce->serialize = zend_class_serialize_deny;
	async_process_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_process_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_process_handlers.dtor_obj = async_process_object_dtor;
	async_process_handlers.free_obj = async_process_object_destroy;
	async_process_handlers.clone_obj = NULL;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Process\\ReadablePipe", async_readable_process_pipe_functions);
	async_readable_process_pipe_ce = zend_register_internal_class(&ce);
	async_readable_process_pipe_ce->ce_flags |= ZEND_ACC_FINAL;
	async_readable_process_pipe_ce->serialize = zend_class_serialize_deny;
	async_readable_process_pipe_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_readable_process_pipe_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_readable_process_pipe_handlers.free_obj = async_readable_process_pipe_object_destroy;
	async_readable_process_pipe_handlers.clone_obj = NULL;

	zend_class_implements(async_readable_process_pipe_ce, 1, async_readable_stream_ce);

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Process\\WritablePipe", async_writable_process_pipe_functions);
	async_writable_process_pipe_ce = zend_register_internal_class(&ce);
	async_writable_process_pipe_ce->ce_flags |= ZEND_ACC_FINAL;
	async_writable_process_pipe_ce->serialize = zend_class_serialize_deny;
	async_writable_process_pipe_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_writable_process_pipe_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_writable_process_pipe_handlers.free_obj = async_writable_process_pipe_object_destroy;
	async_writable_process_pipe_handlers.clone_obj = NULL;

	zend_class_implements(async_writable_process_pipe_ce, 1, async_writable_stream_ce);
}

#endif /* PHALCON_USE_UV */
