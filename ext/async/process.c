
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

#include "kernel/backend.h"
#include "async/async_stream.h"

ASYNC_API zend_class_entry *async_process_builder_ce;
ASYNC_API zend_class_entry *async_process_ce;
ASYNC_API zend_class_entry *async_readable_process_pipe_ce;
ASYNC_API zend_class_entry *async_writable_process_pipe_ce;

static zend_object_handlers async_process_builder_handlers;
static zend_object_handlers async_process_handlers;
static zend_object_handlers async_readable_process_pipe_handlers;
static zend_object_handlers async_writable_process_pipe_handlers;

#define ASYNC_PROCESS_STDIN 0
#define ASYNC_PROCESS_STDOUT 1
#define ASYNC_PROCESS_STDERR 2

#define ASYNC_PROCESS_STDIO_IGNORE 16
#define ASYNC_PROCESS_STDIO_INHERIT 17
#define ASYNC_PROCESS_STDIO_PIPE 18

#define ASYNC_PROCESS_BUILDER_CONST(const_name, value) \
	zend_declare_class_constant_long(async_process_builder_ce, const_name, sizeof(const_name)-1, (zend_long)value);

typedef struct _async_process async_process;
	
typedef struct {
	/* Fiber PHP object handle. */
	zend_object std;

	/* Command to be executed (without arguments). */
	zend_string *command;

	/* Number of additional arguments. */
	uint32_t argc;

	/* Additional args passed to the base command. */
	zval *argv;

	/* Current working directory for the process. */
	zend_string *cwd;

	/* Environment vars to be passed to the created process. */
	zval env;

	/* Set to inherit env vars from parent. */
	zend_bool inherit_env;

	/* STDIO pipe definitions for STDIN, STDOUT and STDERR. */
	uv_stdio_container_t stdio[3];

} async_process_builder;

typedef struct {
	async_process *process;

	uv_pipe_t handle;

	async_stream *stream;

	zval error;
} async_writable_process_pipe_state;

typedef struct {
	async_process *process;

	uv_pipe_t handle;

	async_stream *stream;

	zval error;
} async_readable_process_pipe_state;

struct _async_process {
	/* Fiber PHP object handle. */
	zend_object std;

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

static async_process *async_process_object_create();
static async_readable_process_pipe *async_readable_process_pipe_object_create(async_process *process, async_readable_process_pipe_state *state);
static async_writable_process_pipe *async_writable_process_pipe_object_create(async_process *process, async_writable_process_pipe_state *state);

#ifdef ZEND_WIN32

static char **populate_env(HashTable *add, zend_bool inherit)
{
	char **env;
	zend_string *key;
	zval *v;

	LPTCH envstr;
	char t;

	size_t len;
	int count;
	int i;
	char *j;

	count = 0;

	if (inherit) {
		envstr = GetEnvironmentStrings();

		for (i = 0;; i++) {
			if (envstr[i] == '\0') {
				if (t == '\0') {
					break;
				}

				count++;
			}

			t = envstr[i];
		}

		count -= 3;
	}

	env = ecalloc(zend_hash_num_elements(add) + count + 1, sizeof(char *));
	i = 0;

	if (inherit) {
		j = envstr;

		while (*j != '\0') {
			len = strlen(j) + 1;

			if (*j != '=') {
				env[i] = emalloc(sizeof(char) * len);
				memcpy(env[i++], j, len);
			}

			j += len;
		}

		FreeEnvironmentStrings(envstr);
	}

	ZEND_HASH_FOREACH_STR_KEY_VAL(add, key, v) {
		env[i] = emalloc(sizeof(char) * key->len + Z_STRLEN_P(v) + 2);

		slprintf(env[i++], key->len + Z_STRLEN_P(v) + 2, "%s=%s", key->val, Z_STRVAL_P(v));
	} ZEND_HASH_FOREACH_END();

	env[i] = NULL;

	return env;
}

#else

static char **populate_env(HashTable *add, zend_bool inherit)
{
	char **env;
	zend_string *key;
	zval *v;

	size_t len;
	int count;
	int i;

	i = 0;
	count = 0;

	if (inherit) {
		char **tmp;
		
		for (tmp = environ; tmp != NULL && *tmp != NULL; tmp++) {
			count++;
		}
	}

	env = ecalloc(zend_hash_num_elements(add) + count + 1, sizeof(char *));

	if (inherit) {
		for (; i < count; i++) {
			len = strlen(environ[i]) + 1;

			env[i] = emalloc(len);
			memcpy(env[i], environ[i], len);
		}
	}

	ZEND_HASH_FOREACH_STR_KEY_VAL(add, key, v) {
		env[i] = emalloc(sizeof(char) * key->len + Z_STRLEN_P(v) + 2);

		slprintf(env[i++], key->len + Z_STRLEN_P(v) + 2, "%s=%s", key->val, Z_STRVAL_P(v));
	} ZEND_HASH_FOREACH_END();

	env[i] = NULL;

	return env;
}

#endif

static void configure_stdio(async_process_builder *builder, int i, zend_long mode, zend_long fd, zend_execute_data *execute_data)
{
	if (mode == ASYNC_PROCESS_STDIO_IGNORE) {
		builder->stdio[i].flags = UV_IGNORE;
	} else if (mode == ASYNC_PROCESS_STDIO_INHERIT) {
		if (fd < 0 || fd > 2) {
			zend_throw_error(NULL, "Unsupported file descriptor, only STDIN, STDOUT and STDERR are supported");
			return;
		}

		if (i) {
			if (fd == 0) {
				zend_throw_error(NULL, "STDIN cannot be used as process output pipe");
				return;
			}
		} else {
			if (fd != 0) {
				zend_throw_error(NULL, "Only STDIN is supported as process input pipe");
				return;
			}
		}

		builder->stdio[i].flags = UV_INHERIT_FD;
		builder->stdio[i].data.fd = (int) fd;
	} else if (mode == ASYNC_PROCESS_STDIO_PIPE) {
		builder->stdio[i].flags = UV_CREATE_PIPE | UV_OVERLAPPED_PIPE;
		builder->stdio[i].flags |= i ? UV_WRITABLE_PIPE : UV_READABLE_PIPE;
	} else {
		zend_throw_error(NULL, "Unsupported process STDIO mode");
		return;
	}
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

static void prepare_process(async_process_builder *builder, async_process *proc, zval *params, uint32_t count, zval *return_value, zend_execute_data *execute_data)
{
	char **args;
	uint32_t i;

	count += builder->argc;

	args = ecalloc(sizeof(char *), count + 2);
	args[0] = ZSTR_VAL(builder->command);

	for (i = 1; i <= builder->argc; i++) {
		args[i] = Z_STRVAL_P(&builder->argv[i - 1]);
	}

	for (; i <= count; i++) {
		args[i] = Z_STRVAL_P(&params[i - 1]);
	}

	args[count + 1] = NULL;

	proc->options.file = ZSTR_VAL(builder->command);
	proc->options.args = args;
	proc->options.stdio_count = 3;
	proc->options.stdio = builder->stdio;
	proc->options.exit_cb = exit_process;
	if (proc->detached) {
		proc->options.flags = UV_PROCESS_WINDOWS_HIDE|UV_PROCESS_DETACHED;
	} else {
		proc->options.flags = UV_PROCESS_WINDOWS_HIDE;
	}

	if (builder->cwd != NULL) {
		proc->options.cwd = ZSTR_VAL(builder->cwd);
	}

	if (Z_TYPE_P(&builder->env) != IS_UNDEF) {
		proc->options.env = populate_env(Z_ARRVAL_P(&builder->env), builder->inherit_env);
	} else if (builder->inherit_env == 0) {
		proc->options.env = emalloc(sizeof(char *));
		proc->options.env[0] = NULL;
	}
}


static zend_object *async_process_builder_object_create(zend_class_entry *ce)
{
	async_process_builder *builder;

	builder = ecalloc(1, sizeof(async_process_builder));

	zend_object_std_init(&builder->std, ce);
	builder->std.handlers = &async_process_builder_handlers;

	ZVAL_UNDEF(&builder->env);

	builder->inherit_env = 1;

	return &builder->std;
}

static void async_process_builder_object_destroy(zend_object *object)
{
	async_process_builder *builder;
	uint32_t i;

	builder = (async_process_builder *) object;

	zend_string_release(builder->command);

	if (builder->cwd != NULL) {
		zend_string_release(builder->cwd);
	}

	if (builder->argc > 0) {
		for (i = 0; i < builder->argc; i++) {
			zval_ptr_dtor(&builder->argv[i]);
		}

		efree(builder->argv);
	}

	zval_ptr_dtor(&builder->env);

	zend_object_std_dtor(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, __construct)
{
	async_process_builder *builder;
	uint32_t i;

	zval *params;

	builder = (async_process_builder *) Z_OBJ_P(getThis());

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, -1)
		Z_PARAM_STR(builder->command)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', params, builder->argc)
	ZEND_PARSE_PARAMETERS_END();

	if (builder->argc > 0) {
		builder->argv = ecalloc(builder->argc, sizeof(zval));

		for (i = 0; i < builder->argc; i++) {
			ZVAL_COPY(&builder->argv[i], &params[i]);
		}
	}
}

static ZEND_METHOD(ProcessBuilder, setDirectory)
{
	async_process_builder *builder;

	builder = (async_process_builder *) Z_OBJ_P(getThis());

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(builder->cwd)
	ZEND_PARSE_PARAMETERS_END();
}

static ZEND_METHOD(ProcessBuilder, inheritEnv)
{
	async_process_builder *builder;

	zend_long flag;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(flag)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());

	builder->inherit_env = flag ? 1 : 0;
}

static ZEND_METHOD(ProcessBuilder, setEnv)
{
	async_process_builder *builder;

	zval *env;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ARRAY_EX(env, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());

	if (Z_TYPE_P(&builder->env) != IS_UNDEF) {
		zval_ptr_dtor(&builder->env);
	}

	ZVAL_COPY(&builder->env, env);
}

static ZEND_METHOD(ProcessBuilder, configureStdin)
{
	async_process_builder *builder;

	zend_long mode;
	zend_long fd;

	fd = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_LONG(mode)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(fd)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());

	configure_stdio(builder, 0, mode, fd, execute_data);
}

static ZEND_METHOD(ProcessBuilder, configureStdout)
{
	async_process_builder *builder;

	zend_long mode;
	zend_long fd;

	fd = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_LONG(mode)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(fd)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());

	configure_stdio(builder, 1, mode, fd, execute_data);
}

static ZEND_METHOD(ProcessBuilder, configureStderr)
{
	async_process_builder *builder;

	zend_long mode;
	zend_long fd;

	fd = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_LONG(mode)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(fd)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());

	configure_stdio(builder, 2, mode, fd, execute_data);
}

static ZEND_METHOD(ProcessBuilder, execute)
{
	async_process_builder *builder;
	async_process *proc;
	async_context *context;
	async_op *op;

	uint32_t i;
	uint32_t count;
	zval *params;

	int code;
	int x;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, -1)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', params, count)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());

	for (i = 0; i < 3; i++) {
		if (builder->stdio[i].flags & UV_CREATE_PIPE) {
			zend_throw_error(NULL, "Cannot use STDIO pipe in execute(), use start() instead");
			return;
		}
	}

	proc = async_process_object_create(0);

	prepare_process(builder, proc, params, count, return_value, execute_data);

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
		
		return;
	}

	context = async_context_get();

	if (async_context_is_background(context)) {
		uv_unref((uv_handle_t *) &proc->handle);
	}

	ASYNC_ALLOC_OP(op);
	ASYNC_APPEND_OP(&proc->observers, op);

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

	if (EXPECTED(!EG(exception) && USED_RET())) {
		ZVAL_LONG(return_value, proc->status);
	}

	ASYNC_DELREF(&proc->std);
}

static ZEND_METHOD(ProcessBuilder, start)
{
	async_process_builder *builder;
	async_process *proc;

	uint32_t count;
	zval *params;
	zval obj;

	int code;
	int x;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, -1)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', params, count)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());
	proc = async_process_object_create(0);

	prepare_process(builder, proc, params, count, return_value, execute_data);

	if (proc->options.stdio[0].flags & UV_CREATE_PIPE) {
		create_writable_state(proc, &proc->stdin_state, 0);
	}

	if (proc->options.stdio[1].flags & UV_CREATE_PIPE) {
		create_readable_state(proc, &proc->stdout_state, 1);
	}

	if (proc->options.stdio[2].flags & UV_CREATE_PIPE) {
		create_readable_state(proc, &proc->stderr_state, 2);
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
		
		return;
	}

	uv_unref((uv_handle_t *) &proc->handle);

	ASYNC_LIST_APPEND(&proc->scheduler->shutdown, &proc->cancel);

	proc->pid = proc->handle.pid;

	ZVAL_OBJ(&obj, &proc->std);

	RETURN_ZVAL(&obj, 1, 1);
}

static ZEND_METHOD(ProcessBuilder, daemon)
{
	async_process_builder *builder;
	async_process *proc;

	uint32_t count;
	zval *params;
	zval obj;

	int code;
	int x;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, -1)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', params, count)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());
	proc = async_process_object_create(1);

	prepare_process(builder, proc, params, count, return_value, execute_data);

	if (proc->options.stdio[0].flags & UV_CREATE_PIPE) {
		create_writable_state(proc, &proc->stdin_state, 0);
	}

	if (proc->options.stdio[1].flags & UV_CREATE_PIPE) {
		create_readable_state(proc, &proc->stdout_state, 1);
	}

	if (proc->options.stdio[2].flags & UV_CREATE_PIPE) {
		create_readable_state(proc, &proc->stderr_state, 2);
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
		
		return;
	}

	uv_unref((uv_handle_t *) &proc->handle);

	ASYNC_LIST_APPEND(&proc->scheduler->shutdown, &proc->cancel);

	proc->pid = proc->handle.pid;

	ZVAL_OBJ(&obj, &proc->std);

	RETURN_ZVAL(&obj, 1, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_process_builder_ctor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, arguments, IS_STRING, 0)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_set_directory, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, dir, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_inherit_env, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, inherit, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_set_env, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, env, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_configure_stdin, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_configure_stdout, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_configure_stderr, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_execute, 0, 0, IS_LONG, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, arguments, IS_STRING, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_set_directory, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, dir, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_inherit_env, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, inherit, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_set_env, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, env, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_configure_stdin, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_configure_stdout, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_configure_stderr, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_execute, 0, 0, IS_LONG, NULL, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, arguments, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_start, 0, 0, Phalcon\\Async\\Process\\Process, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, arguments, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_daemon, 0, 0, Phalcon\\Async\\Process\\Process, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, arguments, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry async_process_builder_functions[] = {
	ZEND_ME(ProcessBuilder, __construct, arginfo_process_builder_ctor, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, setDirectory, arginfo_process_builder_set_directory, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, inheritEnv, arginfo_process_builder_inherit_env, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, setEnv, arginfo_process_builder_set_env, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, configureStdin, arginfo_process_builder_configure_stdin, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, configureStdout, arginfo_process_builder_configure_stdout, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, configureStderr, arginfo_process_builder_configure_stderr, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, execute, arginfo_process_builder_execute, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, start, arginfo_process_builder_start, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, daemon, arginfo_process_builder_daemon, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

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

	ZVAL_UNDEF(&proc->stdin_state.error);
	ZVAL_UNDEF(&proc->stdout_state.error);
	ZVAL_UNDEF(&proc->stderr_state.error);

	proc->detached = detached;

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

	zval_ptr_dtor(&proc->stdin_state.error);
	zval_ptr_dtor(&proc->stdout_state.error);
	zval_ptr_dtor(&proc->stderr_state.error);

	async_task_scheduler_unref(proc->scheduler);

	zend_object_std_dtor(&proc->std);
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

	zval obj;

	ZEND_PARSE_PARAMETERS_NONE();

	proc = (async_process *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(!(proc->options.stdio[0].flags & UV_CREATE_PIPE), "Cannot access STDIN because it is not configured to be a pipe");

	pipe = async_writable_process_pipe_object_create(proc, &proc->stdin_state);

	ZVAL_OBJ(&obj, &pipe->std);

	RETURN_ZVAL(&obj, 1, 1);
}

static ZEND_METHOD(Process, getStdout)
{
	async_process *proc;
	async_readable_process_pipe *pipe;

	zval obj;

	ZEND_PARSE_PARAMETERS_NONE();

	proc = (async_process *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(!(proc->options.stdio[1].flags & UV_CREATE_PIPE), "Cannot access STDOUT because it is not configured to be a pipe");

	pipe = async_readable_process_pipe_object_create(proc, &proc->stdout_state);

	ZVAL_OBJ(&obj, &pipe->std);

	RETURN_ZVAL(&obj, 1, 1);
}

static ZEND_METHOD(Process, getStderr)
{
	async_process *proc;
	async_readable_process_pipe *pipe;

	zval obj;

	ZEND_PARSE_PARAMETERS_NONE();

	proc = (async_process *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(!(proc->options.stdio[2].flags & UV_CREATE_PIPE), "Cannot access STDERR because it is not configured to be a pipe");

	pipe = async_readable_process_pipe_object_create(proc, &proc->stderr_state);

	ZVAL_OBJ(&obj, &pipe->std);

	RETURN_ZVAL(&obj, 1, 1);
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
	ZEND_ME(Process, isRunning, arginfo_process_is_running, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, getPid, arginfo_process_get_pid, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, getStdin, arginfo_process_get_stdin, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, getStdout, arginfo_process_get_stdout, ZEND_ACC_PUBLIC)
	ZEND_ME(Process, getStderr, arginfo_process_get_stderr, ZEND_ACC_PUBLIC)
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
	read.in.timeout = 0;
	
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
	
	write.in.len = ZSTR_LEN(data);
	write.in.buffer = ZSTR_VAL(data);
	write.in.str = data;
	write.in.ref = getThis();
	write.in.flags = 0;

	if (UNEXPECTED(FAILURE == async_stream_write(pipe->state->stream, &write))) {
		forward_stream_write_error(&write);
	}
}

static const zend_function_entry async_writable_process_pipe_functions[] = {
	ZEND_ME(WritablePipe, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	ZEND_ME(WritablePipe, write, arginfo_writable_stream_write, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


void async_process_ce_register()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Process\\Builder", async_process_builder_functions);
	async_process_builder_ce = zend_register_internal_class(&ce);
	async_process_builder_ce->ce_flags |= ZEND_ACC_FINAL;
	async_process_builder_ce->create_object = async_process_builder_object_create;
	async_process_builder_ce->serialize = zend_class_serialize_deny;
	async_process_builder_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_process_builder_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_process_builder_handlers.free_obj = async_process_builder_object_destroy;
	async_process_builder_handlers.clone_obj = NULL;

	ASYNC_PROCESS_BUILDER_CONST("STDIN", ASYNC_PROCESS_STDIN);
	ASYNC_PROCESS_BUILDER_CONST("STDOUT", ASYNC_PROCESS_STDOUT);
	ASYNC_PROCESS_BUILDER_CONST("STDERR", ASYNC_PROCESS_STDERR);

	ASYNC_PROCESS_BUILDER_CONST("STDIO_IGNORE", ASYNC_PROCESS_STDIO_IGNORE);
	ASYNC_PROCESS_BUILDER_CONST("STDIO_INHERIT", ASYNC_PROCESS_STDIO_INHERIT);
	ASYNC_PROCESS_BUILDER_CONST("STDIO_PIPE", ASYNC_PROCESS_STDIO_PIPE);

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Process\\Process", async_process_functions);
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

#endif
