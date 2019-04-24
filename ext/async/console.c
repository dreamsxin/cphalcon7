
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

ASYNC_API zend_class_entry *async_readable_pipe_ce;
ASYNC_API zend_class_entry *async_writable_pipe_ce;

static zend_object_handlers async_readable_pipe_handlers;
static zend_object_handlers async_writable_pipe_handlers;

#define ASYNC_CONSOLE_FLAG_TTY 1
#define ASYNC_CONSOLE_FLAG_PIPE (1 << 1)
#define ASYNC_CONSOLE_FLAG_FILE (1 << 2)

#define ASYNC_CONSOLE_FLAG_EOF (1 << 5)

typedef struct {
	zend_object std;	
	uint16_t flags;
	
	union {
		struct {
			uv_tty_t handle;
			async_stream *stream;
		} tty;
		struct {
			uv_pipe_t handle;
			async_stream *stream;
		} pipe;
		struct {
			uv_file file;
			async_op op;
			size_t size;
			char *buffer;
			size_t len;
			char *offset;
		} file;
	} handle;
	
	async_task_scheduler *scheduler;
	async_cancel_cb shutdown;
	zval error;
} async_readable_pipe;

typedef struct {
	zend_object std;	
	uint16_t flags;
	
	union {
		struct {
			uv_tty_t handle;
			async_stream *stream;
		} tty;
		struct {
			uv_pipe_t handle;
			async_stream *stream;
		} pipe;
		struct {
			uv_file file;
			async_op_list writes;
		} file;
	} handle;
	
	async_task_scheduler *scheduler;
	async_cancel_cb shutdown;
	zval error;
} async_writable_pipe;

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pipe_is_terminal, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pipe_is_terminal, 0, 0, _IS_BOOL, NULL, 0)
ZEND_END_ARG_INFO()
#endif

ASYNC_CALLBACK read_close_file_cb(uv_fs_t* req)
{
	async_readable_pipe *pipe;

	pipe = (async_readable_pipe *) req->data;

	ZEND_ASSERT(pipe != NULL);

	ASYNC_DELREF(&pipe->std);

	uv_fs_req_cleanup(req);
	efree(req);
}

ASYNC_CALLBACK readable_pipe_shutdown(void *object, zval *error)
{
	async_readable_pipe *pipe;

	uv_fs_t *req;
	zval obj;

	pipe = (async_readable_pipe *) object;

	pipe->shutdown.func = NULL;

	if (Z_TYPE_P(&pipe->error) == IS_UNDEF) {
		if (error == NULL) {
			ASYNC_PREPARE_EXCEPTION(&pipe->error, async_stream_closed_exception_ce, "Console stream has been closed");
		} else {
			ZVAL_COPY(&pipe->error, error);
		}
	}

	if (pipe->flags & ASYNC_CONSOLE_FLAG_FILE) {
		ASYNC_ADDREF(&pipe->std);

		if (pipe->handle.file.op.status == ASYNC_STATUS_RUNNING) {
			ASYNC_FAIL_OP(&pipe->handle.file.op, &pipe->error);
		}

		req = emalloc(sizeof(uv_fs_t));
		req->data = pipe;

		uv_fs_close(&pipe->scheduler->loop, req, pipe->handle.file.file, read_close_file_cb);
	} else if (pipe->flags & ASYNC_CONSOLE_FLAG_TTY) {
		ZVAL_OBJ(&obj, &pipe->std);

		async_stream_close(pipe->handle.tty.stream, &obj);
	} else {
		ZVAL_OBJ(&obj, &pipe->std);

		async_stream_close(pipe->handle.pipe.stream, &obj);
	}
}

static async_readable_pipe* async_readable_pipe_object_create(uv_file file, uv_handle_type type)
{
	async_readable_pipe *pipe;
	int code;
	pipe = ecalloc(1, sizeof(async_readable_pipe));
	
	pipe->scheduler = async_task_scheduler_get();
	
	if (type == UV_TTY) {
		pipe->flags |= ASYNC_CONSOLE_FLAG_TTY;
	
		uv_tty_init(&pipe->scheduler->loop, &pipe->handle.tty.handle, file, 1);
		
		pipe->handle.tty.stream = async_stream_init((uv_stream_t *) &pipe->handle.tty.handle, 0);
	} else if (type == UV_NAMED_PIPE) {
		pipe->flags |= ASYNC_CONSOLE_FLAG_PIPE;
	
		code = uv_pipe_init(&pipe->scheduler->loop, &pipe->handle.pipe.handle, 0);
		
		if (UNEXPECTED(code < 0)) {
			efree(pipe);
			zend_throw_error(NULL, "Failed to init pipe: %s", uv_strerror(code));
			return NULL;
		}
		
		code = uv_pipe_open(&pipe->handle.pipe.handle, file);

		if (UNEXPECTED(code < 0)) {
			efree(pipe);
			zend_throw_error(NULL, "Failed to open pipe [%d]: %s", (int) file, uv_strerror(code));
			return NULL;
		}
		
		pipe->handle.pipe.stream = async_stream_init((uv_stream_t *) &pipe->handle.pipe.handle, 0);
	} else if (type == UV_FILE) {
		pipe->flags |= ASYNC_CONSOLE_FLAG_FILE;
		
		pipe->handle.file.file = file;
		pipe->handle.file.size = 0x8000;
		pipe->handle.file.buffer = emalloc(pipe->handle.file.size);
		pipe->handle.file.offset = pipe->handle.file.buffer;
	} else {
		efree(pipe);
		
		return NULL;
	}

	ASYNC_ADDREF(&pipe->scheduler->std);
	
	zend_object_std_init(&pipe->std, async_readable_pipe_ce);
	pipe->std.handlers = &async_readable_pipe_handlers;
	
	pipe->shutdown.object = pipe;
	pipe->shutdown.func = readable_pipe_shutdown;

	ASYNC_LIST_APPEND(&pipe->scheduler->shutdown, &pipe->shutdown);

	return pipe;
}

static void async_readable_pipe_object_dtor(zend_object *object)
{
	async_readable_pipe *pipe;
	
	pipe = (async_readable_pipe *) object;
	
	if (pipe->shutdown.func != NULL) {
		ASYNC_LIST_REMOVE(&pipe->scheduler->shutdown, &pipe->shutdown);

		pipe->shutdown.func(pipe, NULL);
	}
}

static void async_readable_pipe_object_destroy(zend_object *object)
{
	async_readable_pipe *pipe;

	pipe = (async_readable_pipe *) object;
	
	if (pipe->flags & ASYNC_CONSOLE_FLAG_FILE) {
		efree(pipe->handle.file.buffer);
	} else if (pipe->flags & ASYNC_CONSOLE_FLAG_TTY) {
		async_stream_free(pipe->handle.tty.stream);
	} else {
		async_stream_free(pipe->handle.pipe.stream);
	}
	
	zval_ptr_dtor(&pipe->error);
	
	async_task_scheduler_unref(pipe->scheduler);
	
	zend_object_std_dtor(&pipe->std);
}

static ZEND_METHOD(ReadablePipe, getStdin)
{
	async_readable_pipe *pipe;
	
	uv_file file;
	uv_handle_type type;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	ASYNC_CHECK_ERROR(!ASYNC_G(cli), "Cannot access STDIN when not running in cli mode");

	file = (uv_file) STDIN_FILENO;
	type = uv_guess_handle(file);
	
	pipe = async_readable_pipe_object_create(file, type);
	
	if (UNEXPECTED(pipe == NULL)) {
		ASYNC_RETURN_ON_ERROR();
		zend_throw_error(NULL, "STDIN cannot be opened, it is detected as %s", uv_handle_type_name(type));
		return;
	}

	RETURN_OBJ(&pipe->std);
}

static ZEND_METHOD(ReadablePipe, isTerminal)
{
	async_readable_pipe *pipe;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	pipe = (async_readable_pipe *) Z_OBJ_P(getThis());
	
	RETURN_BOOL(pipe->flags & ASYNC_CONSOLE_FLAG_TTY);
}

static ZEND_METHOD(ReadablePipe, close)
{
	async_readable_pipe *pipe;
	
	zval *val;
	zval error;

	val = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	pipe = (async_readable_pipe *) Z_OBJ_P(getThis());
	
	if (pipe->shutdown.func == NULL) {
		return;
	}
	
	ASYNC_PREPARE_EXCEPTION(&error, async_stream_closed_exception_ce, "Console stream has been closed");

	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}
	
	ASYNC_LIST_REMOVE(&pipe->scheduler->shutdown, &pipe->shutdown);

	pipe->shutdown.func(pipe, &error);

	zval_ptr_dtor(&error);
}

ASYNC_CALLBACK read_fs_cb(uv_fs_t *req)
{
	async_op *op;
	
	op = (async_op *) req->data;
	
	ZEND_ASSERT(op != NULL);
	
	ASYNC_FINISH_OP(op);
}

static ZEND_METHOD(ReadablePipe, read)
{
	async_readable_pipe *pipe;
	async_stream *stream;
	async_stream_read_req read;
	
	uv_buf_t bufs[1];
	uv_fs_t req;
	
	zval *hint;
	zend_string *str;
	
	size_t len;
	
	hint = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(hint)
	ZEND_PARSE_PARAMETERS_END();
	
	pipe = (async_readable_pipe *) Z_OBJ_P(getThis());
	
	if (hint == NULL || Z_TYPE_P(hint) == IS_NULL) {
		if (pipe->flags & ASYNC_CONSOLE_FLAG_FILE) {
			len = pipe->handle.file.size;
		} else if (pipe->flags & ASYNC_CONSOLE_FLAG_TTY) {
			len = pipe->handle.tty.stream->buffer.size;
		} else {
			len = pipe->handle.pipe.stream->buffer.size;
		}
	} else if (Z_LVAL_P(hint) < 1) {
		zend_throw_exception_ex(async_stream_exception_ce, 0, "Invalid read length: %d", (int) Z_LVAL_P(hint));
		return;
	} else {
		len = (size_t) Z_LVAL_P(hint);
	}
	
	if (UNEXPECTED(Z_TYPE_P(&pipe->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&pipe->error);
		return;
	}
	
	if (pipe->flags & ASYNC_CONSOLE_FLAG_FILE) {
		if (UNEXPECTED(pipe->flags & ASYNC_CONSOLE_FLAG_EOF)) {
			return;
		}
		
		ASYNC_CHECK_EXCEPTION(pipe->handle.file.op.status == ASYNC_STATUS_RUNNING, async_pending_read_exception_ce, "Cannot read while another read is pending");
	
		if (pipe->handle.file.len == 0) {
			bufs[0] = uv_buf_init(pipe->handle.file.buffer, (unsigned int) pipe->handle.file.size);
		
			req.data = &pipe->handle.file.op;

			uv_fs_read(&pipe->scheduler->loop, &req, pipe->handle.file.file, bufs, 1, -1, read_fs_cb);
			
			if (UNEXPECTED(async_await_op(&pipe->handle.file.op) == FAILURE)) {
				ASYNC_FORWARD_OP_ERROR(&pipe->handle.file.op);
				ASYNC_RESET_OP(&pipe->handle.file.op);				
				uv_fs_req_cleanup(&req);
				
				return;
			}
			
			ASYNC_RESET_OP(&pipe->handle.file.op);
			uv_fs_req_cleanup(&req);
			
			ASYNC_CHECK_EXCEPTION(req.result < 0, async_stream_exception_ce, "Failed to read from input file: %s", uv_strerror((int) req.result));
			
			if (UNEXPECTED(req.result == 0)) {
				pipe->flags |= ASYNC_CONSOLE_FLAG_EOF;
				return;
			}
			
			pipe->handle.file.len = (size_t) req.result;
			pipe->handle.file.offset = pipe->handle.file.buffer;
		}
		
		str = zend_string_init(pipe->handle.file.offset, MIN(len, pipe->handle.file.len), 0);
		
		pipe->handle.file.len -= ZSTR_LEN(str);
		pipe->handle.file.offset += ZSTR_LEN(str);
		
		RETURN_STR(str);
	}
	
	if (pipe->flags & ASYNC_CONSOLE_FLAG_TTY) {
		stream = pipe->handle.tty.stream;
	} else {
		stream = pipe->handle.pipe.stream;
	}
	
	read.in.len = len;
	read.in.buffer = NULL;
	read.in.handle = NULL;
	read.in.timeout = 0;
	read.in.flags = 0;
	
	if (EXPECTED(SUCCESS == async_stream_read(stream, &read))) {
		if (EXPECTED(read.out.len)) {
			RETURN_STR(read.out.str);
		}
		
		return;
	}

	forward_stream_read_error(&read);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_readable_console_stream_get_stdin, 0, 0, Phalcon\\Async\\Stream\\ReadablePipe, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry async_readable_pipe_functions[] = {
	ZEND_ME(ReadablePipe, getStdin, arginfo_readable_console_stream_get_stdin, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(ReadablePipe, isTerminal, arginfo_pipe_is_terminal, ZEND_ACC_PUBLIC)
	ZEND_ME(ReadablePipe, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	ZEND_ME(ReadablePipe, read, arginfo_readable_stream_read, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


ASYNC_CALLBACK write_close_file_cb(uv_fs_t* req)
{
	async_writable_pipe *pipe;
	
	pipe = (async_writable_pipe *) req->data;
	
	ZEND_ASSERT(pipe != NULL);
	
	ASYNC_DELREF(&pipe->std);
	
	uv_fs_req_cleanup(req);
	efree(req);
}

ASYNC_CALLBACK writable_pipe_shutdown(void *object, zval *error)
{
	async_writable_pipe *pipe;
	async_op *op;

	uv_fs_t *req;
	zval obj;

	pipe = (async_writable_pipe *) object;

	pipe->shutdown.func = NULL;

	if (Z_TYPE_P(&pipe->error) == IS_UNDEF) {
		if (error == NULL) {
			ASYNC_PREPARE_EXCEPTION(&pipe->error, async_stream_closed_exception_ce, "Console stream has been closed");
		} else {
			ZVAL_COPY(&pipe->error, error);
		}
	}

	if (pipe->flags & ASYNC_CONSOLE_FLAG_FILE) {
		ASYNC_ADDREF(&pipe->std);

		while (pipe->handle.file.writes.first != NULL) {
			ASYNC_NEXT_OP(&pipe->handle.file.writes, op);
			ASYNC_FAIL_OP(op, &pipe->error);
		}

		req = emalloc(sizeof(uv_fs_t));
		req->data = pipe;

		uv_fs_close(&pipe->scheduler->loop, req, pipe->handle.file.file, write_close_file_cb);
	} else if (pipe->flags & ASYNC_CONSOLE_FLAG_TTY) {
		ZVAL_OBJ(&obj, &pipe->std);

		async_stream_close(pipe->handle.tty.stream, &obj);
	} else {
		ZVAL_OBJ(&obj, &pipe->std);
		
		async_stream_close(pipe->handle.pipe.stream, &obj);
	}
}

static async_writable_pipe* async_writable_pipe_object_create(uv_file file, uv_handle_type type)
{
	async_writable_pipe *pipe;
	
	pipe = ecalloc(1, sizeof(async_writable_pipe));
	
	pipe->scheduler = async_task_scheduler_ref();
	
	if (type == UV_TTY) {
		pipe->flags |= ASYNC_CONSOLE_FLAG_TTY;
	
		uv_tty_init(&pipe->scheduler->loop, &pipe->handle.tty.handle, file, 0);
		
		pipe->handle.tty.stream = async_stream_init((uv_stream_t *) &pipe->handle.tty.handle, 0);
	} else if (type == UV_NAMED_PIPE) {
		pipe->flags |= ASYNC_CONSOLE_FLAG_PIPE;
	
		uv_pipe_init(&pipe->scheduler->loop, &pipe->handle.pipe.handle, 0);
		uv_pipe_open(&pipe->handle.pipe.handle, file);
		
		pipe->handle.pipe.stream = async_stream_init((uv_stream_t *) &pipe->handle.pipe.handle, 0);
	} else if (type == UV_FILE) {
		pipe->flags |= ASYNC_CONSOLE_FLAG_FILE;
		
		pipe->handle.file.file = file;
	} else {
		async_task_scheduler_unref(pipe->scheduler);
		
		efree(pipe);
		
		return NULL;
	}
	
	zend_object_std_init(&pipe->std, async_writable_pipe_ce);
	pipe->std.handlers = &async_writable_pipe_handlers;
	
	pipe->shutdown.object = pipe;
	pipe->shutdown.func = writable_pipe_shutdown;

	ASYNC_LIST_APPEND(&pipe->scheduler->shutdown, &pipe->shutdown);

	return pipe;
}

static void async_writable_pipe_object_dtor(zend_object *object)
{
	async_writable_pipe *pipe;

	pipe = (async_writable_pipe *) object;

	if (pipe->shutdown.func != NULL) {
		ASYNC_LIST_REMOVE(&pipe->scheduler->shutdown, &pipe->shutdown);

		pipe->shutdown.func(pipe, NULL);
	}
}

static void async_writable_pipe_object_destroy(zend_object *object)
{
	async_writable_pipe *pipe;

	pipe = (async_writable_pipe *) object;
	
	if (pipe->flags & ASYNC_CONSOLE_FLAG_TTY) {
		async_stream_free(pipe->handle.tty.stream);
	} else if (pipe->flags & ASYNC_CONSOLE_FLAG_PIPE) {
		async_stream_free(pipe->handle.pipe.stream);
	}
	
	zval_ptr_dtor(&pipe->error);
	
	async_task_scheduler_unref(pipe->scheduler);
	
	zend_object_std_dtor(&pipe->std);
}

static ZEND_METHOD(WritablePipe, getStdout)
{
	async_writable_pipe *pipe;
	
	uv_file file;
	uv_handle_type type;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	ASYNC_CHECK_ERROR(!ASYNC_G(cli), "Cannot access STDOUT when not running in cli mode");

	file = (uv_file) STDOUT_FILENO;
	type = uv_guess_handle(file);
	
	pipe = async_writable_pipe_object_create(file, type);
	
	if (UNEXPECTED(pipe == NULL)) {
		zend_throw_error(NULL, "STDOUT cannot be opened, it is detected as %s", uv_handle_type_name(type));
		return;
	}

	RETURN_OBJ(&pipe->std);
}

static ZEND_METHOD(WritablePipe, getStderr)
{
	async_writable_pipe *pipe;
	
	uv_file file;
	uv_handle_type type;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	ASYNC_CHECK_ERROR(!ASYNC_G(cli), "Cannot access STDERR when not running in cli mode");

	file = (uv_file) STDERR_FILENO;
	type = uv_guess_handle(file);
	
	pipe = async_writable_pipe_object_create(file, type);
	
	if (UNEXPECTED(pipe == NULL)) {
		zend_throw_error(NULL, "STDERR cannot be opened, it is detected as %s", uv_handle_type_name(type));
		return;
	}

	RETURN_OBJ(&pipe->std);
}

static ZEND_METHOD(WritablePipe, isTerminal)
{
	async_writable_pipe *pipe;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	pipe = (async_writable_pipe *) Z_OBJ_P(getThis());
	
	RETURN_BOOL(pipe->flags & ASYNC_CONSOLE_FLAG_TTY);
}

static ZEND_METHOD(WritablePipe, close)
{
	async_writable_pipe *pipe;
	
	zval *val;
	zval error;

	val = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	pipe = (async_writable_pipe *) Z_OBJ_P(getThis());
	
	if (pipe->shutdown.func == NULL) {
		return;
	}
	
	ASYNC_PREPARE_EXCEPTION(&error, async_stream_closed_exception_ce, "Console stream has been closed");

	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}
	
	ASYNC_LIST_REMOVE(&pipe->scheduler->shutdown, &pipe->shutdown);

	pipe->shutdown.func(pipe, &error);

	zval_ptr_dtor(&error);
}

ASYNC_CALLBACK write_fs_cb(uv_fs_t *req)
{
	async_op *op;
	
	op = (async_op *) req->data;
	
	ZEND_ASSERT(op != NULL);
	
	ASYNC_FINISH_OP(op);
}

static ZEND_METHOD(WritablePipe, write)
{
	async_writable_pipe *pipe;
	async_stream *stream;
	async_stream_write_req write;
	async_op *op;
	
	uv_buf_t bufs[1];
	uv_fs_t req;
	
	zend_string *data;
	int code;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(data)
	ZEND_PARSE_PARAMETERS_END();
	
	pipe = (async_writable_pipe *) Z_OBJ_P(getThis());
	
	if (UNEXPECTED(Z_TYPE_P(&pipe->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&pipe->error);
		return;
	}
	
	if (pipe->flags & ASYNC_CONSOLE_FLAG_FILE) {
		bufs[0] = uv_buf_init(ZSTR_VAL(data), (unsigned int) ZSTR_LEN(data));
		
		ASYNC_ALLOC_OP(op);
		
		req.data = op;

		code = uv_fs_write(&pipe->scheduler->loop, &req, pipe->handle.file.file, bufs, 1, -1, write_fs_cb);
		
		if (UNEXPECTED(code < 0)) {
			ASYNC_FREE_OP(op);
			zend_throw_exception_ex(async_stream_exception_ce, 0, "Failed to write data to file: %s", uv_strerror(code));
			return;
		}
		
		if (UNEXPECTED(async_await_op((async_op *) op) == FAILURE)) {
			ASYNC_FORWARD_OP_ERROR(op);
			ASYNC_FREE_OP(op);
			
			uv_fs_req_cleanup(&req);
			return;
		}
		
		ASYNC_FREE_OP(op);
		uv_fs_req_cleanup(&req);
		
		ASYNC_CHECK_EXCEPTION(req.result < 0, async_stream_exception_ce, "Failed to write data to file: %s", uv_strerror((int) req.result));
		
		return;
	}
	
	if (pipe->flags & ASYNC_CONSOLE_FLAG_TTY) {
		stream = pipe->handle.tty.stream;
	} else {
		stream = pipe->handle.pipe.stream;
	}
	
	write.in.len = ZSTR_LEN(data);
	write.in.buffer = ZSTR_VAL(data);
	write.in.handle = NULL;
	write.in.str = data;
	write.in.ref = getThis();
	write.in.flags = 0;
	
	if (UNEXPECTED(FAILURE == async_stream_write(stream, &write))) {
		forward_stream_write_error(&write);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_writable_console_stream_get_stdout, 0, 0, Phalcon\\Async\\Stream\\WritablePipe, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_writable_console_stream_get_stderr, 0, 0, Phalcon\\Async\\Stream\\WritablePipe, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry async_writable_pipe_functions[] = {
	ZEND_ME(WritablePipe, getStdout, arginfo_writable_console_stream_get_stdout, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(WritablePipe, getStderr, arginfo_writable_console_stream_get_stderr, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(WritablePipe, isTerminal, arginfo_pipe_is_terminal, ZEND_ACC_PUBLIC)
	ZEND_ME(WritablePipe, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	ZEND_ME(WritablePipe, write, arginfo_writable_stream_write, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


void async_console_ce_register()
{
	zend_class_entry ce;
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream\\ReadablePipe", async_readable_pipe_functions);
	async_readable_pipe_ce = zend_register_internal_class(&ce);
	async_readable_pipe_ce->ce_flags |= ZEND_ACC_FINAL;
	async_readable_pipe_ce->serialize = zend_class_serialize_deny;
	async_readable_pipe_ce->unserialize = zend_class_unserialize_deny;
	
	zend_class_implements(async_readable_pipe_ce, 1, async_readable_stream_ce);
	
	memcpy(&async_readable_pipe_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_readable_pipe_handlers.dtor_obj = async_readable_pipe_object_dtor;
	async_readable_pipe_handlers.free_obj = async_readable_pipe_object_destroy;
	async_readable_pipe_handlers.clone_obj = NULL;
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream\\WritablePipe", async_writable_pipe_functions);
	async_writable_pipe_ce = zend_register_internal_class(&ce);
	async_writable_pipe_ce->ce_flags |= ZEND_ACC_FINAL;
	async_writable_pipe_ce->serialize = zend_class_serialize_deny;
	async_writable_pipe_ce->unserialize = zend_class_unserialize_deny;
	
	zend_class_implements(async_writable_pipe_ce, 1, async_writable_stream_ce);
	
	memcpy(&async_writable_pipe_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_writable_pipe_handlers.dtor_obj = async_writable_pipe_object_dtor;
	async_writable_pipe_handlers.free_obj = async_writable_pipe_object_destroy;
	async_writable_pipe_handlers.clone_obj = NULL;
}

#endif
