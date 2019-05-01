
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

#include "async/async_stream.h"
#include "async/async_socket.h"
#include "async/async_pipe.h"
#include "kernel/backend.h"

#ifdef ZEND_WIN32
#include <Zend/zend_smart_str.h>
#endif

ASYNC_API zend_class_entry *async_pipe_ce;
ASYNC_API zend_class_entry *async_pipe_server_ce;

static zend_object_handlers async_pipe_handlers;
static zend_object_handlers async_pipe_server_handlers;


ASYNC_CALLBACK pipe_disposed(uv_handle_t *handle)
{
	async_pipe *pipe;

	pipe = (async_pipe *) handle->data;
	
	ZEND_ASSERT(pipe != NULL);
	
	ASYNC_DELREF(&pipe->std);
}

ASYNC_CALLBACK shutdown_pipe(void *arg, zval *error)
{
	async_pipe *pipe;
	
	zval obj;

	pipe = (async_pipe *) arg;
	
	ZEND_ASSERT(pipe != NULL);
	
	pipe->cancel.func = NULL;

	if (error != NULL) {
		if (Z_TYPE_P(&pipe->read_error) == IS_UNDEF) {
			ZVAL_COPY(&pipe->read_error, error);
		}
		
		if (Z_TYPE_P(&pipe->write_error) == IS_UNDEF) {
			ZVAL_COPY(&pipe->write_error, error);
		}
	}
	
	if (pipe->stream == NULL) {
		if (!uv_is_closing((uv_handle_t *) &pipe->handle)) {
			ASYNC_ADDREF(&pipe->std);
			
			pipe->handle.data = pipe;

			uv_close((uv_handle_t *) &pipe->handle, pipe_disposed);
		}
	} else {
		ZVAL_OBJ(&obj, &pipe->std);
		
		async_stream_close(pipe->stream, &obj);
	}
}

static async_pipe *async_pipe_object_create(int ipc)
{
	async_pipe *pipe;

	pipe = ecalloc(1, sizeof(async_pipe));

	zend_object_std_init(&pipe->std, async_pipe_ce);
	pipe->std.handlers = &async_pipe_handlers;
	
	pipe->scheduler = async_task_scheduler_ref();
	
	pipe->cancel.object = pipe;
	pipe->cancel.func = shutdown_pipe;
	
	ASYNC_LIST_APPEND(&pipe->scheduler->shutdown, &pipe->cancel);

	uv_pipe_init(&pipe->scheduler->loop, &pipe->handle, ipc);
	
	pipe->stream = async_stream_init((uv_stream_t *) &pipe->handle, ipc);

	if (ipc) {
		pipe->flags |= ASYNC_PIPE_FLAG_IPC;
		pipe->stream->flags |= ASYNC_STREAM_IPC;
	}

	return pipe;
}

static void async_pipe_object_dtor(zend_object *object)
{
	async_pipe *pipe;

	pipe = (async_pipe *) object;
	
	if (pipe->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&pipe->scheduler->shutdown, &pipe->cancel);
		
		pipe->cancel.func(pipe, NULL);
	}
}

static void async_pipe_object_destroy(zend_object *object)
{
	async_pipe *pipe;
	
	pipe = (async_pipe *) object;

	if (pipe->stream != NULL) {
		async_stream_free(pipe->stream);
	}

	if (pipe->server != NULL) {
		ASYNC_DELREF(&pipe->server->std);
	}
	
	zval_ptr_dtor(&pipe->read_error);
	zval_ptr_dtor(&pipe->write_error);

	if (pipe->name != NULL) {
		zend_string_release(pipe->name);
	}
	
	async_task_scheduler_unref(pipe->scheduler);

	zend_object_std_dtor(&pipe->std);
}

async_pipe *async_pipe_init_ipc()
{
	async_pipe *pipe;
	
	pipe = async_pipe_object_create(1);
	
	ASYNC_STR(pipe->name, "ipc");
	
	return pipe;
}

ASYNC_CALLBACK connect_cb(uv_connect_t *req, int status)
{
	async_uv_op *op;

	op = (async_uv_op *) req->data;

	ZEND_ASSERT(op != NULL);
	
	op->code = status;
	
	ASYNC_FINISH_OP(op);
}

static ZEND_METHOD(Pipe, connect)
{
	async_pipe *pipe;
	async_context *context;
	async_uv_op *op;
	
	zend_string *name;
	zend_string *host;
	zend_long ipc;
	
	uv_connect_t req;
	int code;
	
	host = NULL;
	ipc = 0;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 3)
		Z_PARAM_STR(name)
		Z_PARAM_OPTIONAL
		Z_PARAM_STR(host)
		Z_PARAM_LONG(ipc)
	ZEND_PARSE_PARAMETERS_END();
	
#ifdef ZEND_WIN32
	smart_str tmp = {0};
	
	if (strncasecmp(ZSTR_VAL(name), "\\\\", sizeof("\\\\") - 1) == 0) {
		smart_str_append(&tmp, name);
	} else {	
		smart_str_appends(&tmp, "\\\\");
		
		if (host) {
			smart_str_append(&tmp, host);
		} else {
			smart_str_appends(&tmp, ".");
		}
		
		smart_str_appends(&tmp, "\\pipe\\");
		smart_str_append(&tmp, name);
	}
	
	name = smart_str_extract(&tmp);
#else
	ASYNC_CHECK_ERROR(host != NULL, "Cannot connect a pipe to a host");
#endif
	
	pipe = async_pipe_object_create(ipc ? 1 : 0);
	
#ifdef ZEND_WIN32
	pipe->name = name;
#else
	pipe->name = zend_string_copy(name);
#endif
	
	uv_pipe_connect(&req, &pipe->handle, ZSTR_VAL(name), connect_cb);
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op));
	
	req.data = op;
	
	context = async_context_get();
	
	if (!async_context_is_background(context) && 1 == ++pipe->stream->ref_count) {
		uv_ref((uv_handle_t *) &pipe->handle);
	}
	
	code = async_await_op((async_op *) op);
	
	if (!async_context_is_background(context) && 0 == --pipe->stream->ref_count) {
		uv_unref((uv_handle_t *) &pipe->handle);
	}
	
	if (UNEXPECTED(code == FAILURE)) {
		ASYNC_DELREF(&pipe->std);
		ASYNC_FORWARD_OP_ERROR(op);
		ASYNC_FREE_OP(op);
		
		return;
	}
	
	code = op->code;
	
	ASYNC_FREE_OP(op);
	
	if (UNEXPECTED(code < 0)) {
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to connect pipe: %s", uv_strerror(code));
		ASYNC_DELREF(&pipe->std);		
		return;
	}
	
	RETURN_OBJ(&pipe->std);
}

static ZEND_METHOD(Pipe, import)
{
	async_pipe *pipe;
	
	zval *conn;
	zend_long ipc;
	
	char buf[128];
	size_t size;
	
	ipc = 0;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_ZVAL(conn)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(ipc)
	ZEND_PARSE_PARAMETERS_END();
	
	pipe = async_pipe_object_create(ipc ? 1 : 0);
	
	async_pipe_import_stream((async_pipe *) Z_OBJ_P(conn), (uv_stream_t *) &pipe->handle);
	
	if (UNEXPECTED(EG(exception))) {
		ASYNC_DELREF(&pipe->std);
		return;
	}
	
	size = 128;
	
	if (EXPECTED(0 == uv_pipe_getsockname(&pipe->handle, buf, &size))) {
		pipe->name = zend_string_init(buf, size, 0);
	} else {
		pipe->name = zend_string_init("unknown", sizeof("unknown")-1, 0);
	}
	
	RETURN_OBJ(&pipe->std);
}

static ZEND_METHOD(Pipe, export)
{
	async_pipe *pipe;
	
	zval *ipc;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(ipc)
	ZEND_PARSE_PARAMETERS_END();
	
	pipe = (async_pipe *) Z_OBJ_P(getThis());

	async_pipe_export_stream((async_pipe *) Z_OBJ_P(ipc), (uv_stream_t *) &pipe->handle);
}

#ifdef ZEND_WIN32

typedef struct {
	async_op base;
	int code;
	int pending;
} pipe_pair_op;

ASYNC_CALLBACK listen_pipe_pair(uv_stream_t *handle, int status)
{
	pipe_pair_op *op;
	
	op = (pipe_pair_op *) handle->data;
	op->code = status;
	
	if (0 == --op->pending) {
		ASYNC_FINISH_OP(op);
		ASYNC_FREE_OP(op);
	}
}

ASYNC_CALLBACK connect_pipe_pair(uv_connect_t *req, int status)
{
	pipe_pair_op *op;
	
	op = (pipe_pair_op *) req->data;
	op->code = status;
	
	if (0 == --op->pending) {
		ASYNC_FINISH_OP(op);
		ASYNC_FREE_OP(op);
	}
	
	efree(req);
}

#endif

static ZEND_METHOD(Pipe, pair)
{
	async_pipe *pipe;
	
	zval val;
	zend_long ipc;
	
	int code;
	int i;

	ipc = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(ipc)
	ZEND_PARSE_PARAMETERS_END();

	if (UNEXPECTED(!USED_RET())) {
		return;
	}

#ifdef ZEND_WIN32
	async_pipe *server;
	async_pipe *client;
	
	pipe_pair_op *op;
	
	uv_connect_t *req;
	char name[128];
	
	server = async_pipe_object_create(0);
	
	for (i = 0; i < 5; i++) {
		sprintf(name, "\\\\.\\pipe\\php\\%p-%lu.sock", (void *)(((char *) server) + i), GetCurrentProcessId());
		
		code = uv_pipe_bind(&server->handle, name);
		
		if (EXPECTED(code != UV_EADDRINUSE)) {
			break;
		}
	}
	
	if (UNEXPECTED(code != 0)) {
		ASYNC_DELREF(&server->std);
		
		zend_throw_exception_ex(async_stream_exception_ce, 0, "Failed to bind pipe: %s", uv_strerror(code));
		return;
	}
	
	uv_pipe_pending_instances(&server->handle, 1);
	
	pipe = async_pipe_object_create(ipc ? 1 : 0);
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(pipe_pair_op));
	
	op->pending = 2;
	
	server->handle.data = op;
		
	code = uv_listen((uv_stream_t *) &server->handle, 0, listen_pipe_pair);
	
	if (UNEXPECTED(code != 0)) {
		ASYNC_DELREF(&server->std);
		ASYNC_DELREF(&pipe->std);
		
		zend_throw_exception_ex(async_stream_exception_ce, 0, "Failed to listen: %s", uv_strerror(code));
		return;
	}
	
	req = emalloc(sizeof(uv_connect_t));
	req->data = op;
	
	uv_pipe_connect(req, &pipe->handle, name, connect_pipe_pair);
	uv_ref((uv_handle_t *) &server->handle);
	
	if (UNEXPECTED(async_await_op((async_op *) op) == FAILURE)) {
		ASYNC_FORWARD_OP_ERROR(op);
		
		ASYNC_DELREF(&server->std);
		ASYNC_DELREF(&pipe->std);
		
		return;
	}
	
	client = async_pipe_object_create(ipc ? 1 : 0);
	
	code = uv_accept((uv_stream_t *) &server->handle, (uv_stream_t *) &client->handle);
	
	ASYNC_DELREF(&server->std);
	
	if (UNEXPECTED(code != 0)) {
		ASYNC_DELREF(&pipe->std);
		ASYNC_DELREF(&client->std);
		
		zend_throw_exception_ex(async_stream_exception_ce, 0, "Failed to accept pipe: %s", uv_strerror(code));
		return;
	}
	
	array_init_size(return_value, 2);
	
	ZVAL_OBJ(&val, &pipe->std);
	ASYNC_STR(pipe->name, "pair");
	
	zend_hash_index_update(Z_ARRVAL_P(return_value), 0, &val);
	
	ZVAL_OBJ(&val, &client->std);
	ASYNC_STR(pipe->name, "pair");
	
	zend_hash_index_update(Z_ARRVAL_P(return_value), 1, &val);
#else
	uv_file tmp[2];
	
	code = socketpair(AF_UNIX, SOCK_STREAM, 0, tmp);
	
	ASYNC_CHECK_EXCEPTION(code != 0, async_stream_exception_ce, "Failed to create pipe pair: %s", uv_strerror(uv_translate_sys_error(php_socket_errno())));
	
	array_init_size(return_value, 2);
	
	for (i = 0; i < 2; i++) {
		pipe = async_pipe_object_create(ipc ? 1 : 0);
		
		code = uv_pipe_open(&pipe->handle, tmp[i]);
		
		if (UNEXPECTED(code != 0)) {
			ASYNC_DELREF(&pipe->std);
			zval_ptr_dtor(return_value);
			
			zend_throw_exception_ex(async_stream_exception_ce, 0, "Failed to open pipe: %s", uv_strerror(code));
			return;
		}
		
		ZVAL_OBJ(&val, &pipe->std);
		ASYNC_STR(pipe->name, "pair");
		
		zend_hash_index_update(Z_ARRVAL_P(return_value), i, &val);
	}
#endif
}

static ZEND_METHOD(Pipe, close)
{
	async_pipe *pipe;

	zval error;
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	pipe = (async_pipe *) Z_OBJ_P(getThis());
	
	if (pipe->cancel.func == NULL) {
		return;
	}

	ASYNC_PREPARE_EXCEPTION(&error, async_stream_closed_exception_ce, "Socket has been closed");

	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}

	ASYNC_LIST_REMOVE(&pipe->scheduler->shutdown, &pipe->cancel);
	
	pipe->cancel.func(pipe, &error);

	zval_ptr_dtor(&error);
}

static ZEND_METHOD(Pipe, flush)
{
	async_pipe *pipe;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	pipe = (async_pipe *) Z_OBJ_P(getThis());
	
	async_stream_flush(pipe->stream);
}

static ZEND_METHOD(Pipe, getAddress)
{
	async_pipe *pipe;

	ZEND_PARSE_PARAMETERS_NONE();

	pipe = (async_pipe *) Z_OBJ_P(getThis());

	RETURN_STR_COPY(pipe->name);
}

static ZEND_METHOD(Pipe, getPort)
{
	ZEND_PARSE_PARAMETERS_NONE();
}

static ZEND_METHOD(Pipe, setOption)
{
	zend_long option;
	zval *val;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(option)
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	RETURN_FALSE;
}

static ZEND_METHOD(Pipe, getRemoteAddress)
{
	async_pipe *pipe;

	ZEND_PARSE_PARAMETERS_NONE();

	pipe = (async_pipe *) Z_OBJ_P(getThis());

	RETURN_STR_COPY(pipe->name);
}

static ZEND_METHOD(Pipe, getRemotePort)
{
	ZEND_PARSE_PARAMETERS_NONE();
}

static ZEND_METHOD(Pipe, read)
{
	async_pipe *pipe;

	pipe = (async_pipe *) Z_OBJ_P(getThis());

	async_stream_call_read(pipe->stream, &pipe->read_error, return_value, execute_data);
}

static ZEND_METHOD(Pipe, getReadableStream)
{
	async_pipe *pipe;
	async_stream_reader *reader;

	ZEND_PARSE_PARAMETERS_NONE();

	pipe = (async_pipe *) Z_OBJ_P(getThis());
	reader = async_stream_reader_create(pipe->stream, &pipe->std, &pipe->read_error);
	
	RETURN_OBJ(&reader->std);
}

static ZEND_METHOD(Pipe, write)
{
	async_pipe *pipe;

	pipe = (async_pipe *) Z_OBJ_P(getThis());

	async_stream_call_write(pipe->stream, &pipe->write_error, return_value, execute_data);
}

static ZEND_METHOD(Pipe, writeAsync)
{
	async_pipe *pipe;
	async_stream_write_req write;
	
	zend_string *data;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(data)
	ZEND_PARSE_PARAMETERS_END();
	
	pipe = (async_pipe *) Z_OBJ_P(getThis());

	if (UNEXPECTED(Z_TYPE_P(&pipe->write_error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&pipe->write_error);
		return;
	}
	
	write.in.len = ZSTR_LEN(data);
	write.in.buffer = ZSTR_VAL(data);
	write.in.handle = NULL;
	write.in.str = data;
	write.in.ref = getThis();
	write.in.flags = ASYNC_STREAM_WRITE_REQ_FLAG_ASYNC;
	
	if (UNEXPECTED(FAILURE == async_stream_write(pipe->stream, &write))) {
		forward_stream_write_error(&write);
	} else {
		RETURN_LONG(pipe->handle.write_queue_size);
	}
}

static ZEND_METHOD(Pipe, getWriteQueueSize)
{
	async_pipe *pipe;
	
	ZEND_PARSE_PARAMETERS_NONE();

	pipe = (async_pipe *) Z_OBJ_P(getThis());
	
	RETURN_LONG((Z_TYPE_P(&pipe->write_error) == IS_UNDEF) ? pipe->handle.write_queue_size : 0);
}

static ZEND_METHOD(Pipe, getWritableStream)
{
	async_pipe *pipe;
	async_stream_writer *writer;

	ZEND_PARSE_PARAMETERS_NONE();

	pipe = (async_pipe *) Z_OBJ_P(getThis());
	writer = async_stream_writer_create(pipe->stream, &pipe->std, &pipe->write_error);
	
	RETURN_OBJ(&writer->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_pipe_connect, 0, 1, Phalcon\\Async\\Network\\Pipe, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, ipc, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_pipe_import, 0, 1, Phalcon\\Async\\Network\\Pipe, 0)
	ZEND_ARG_OBJ_INFO(0, pipe, Phalcon\\Async\\Network\\Pipe, 0)
	ZEND_ARG_TYPE_INFO(0, ipc, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pipe_pair, 0, 0, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, ipc, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pipe_export, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, pipe, Phalcon\\Async\\Network\\Pipe, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pipe_pair, 0, 0, IS_ARRAY, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, ipc, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pipe_export, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_OBJ_INFO(0, pipe, Phalcon\\Async\\Network\\Pipe, 0)
ZEND_END_ARG_INFO()
#endif

static const zend_function_entry async_pipe_functions[] = {
	ZEND_ME(Pipe, connect, arginfo_pipe_connect, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Pipe, import, arginfo_pipe_import, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Pipe, pair, arginfo_pipe_pair, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Pipe, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, flush, arginfo_socket_stream_flush, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, getAddress, arginfo_socket_get_address, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, getPort, arginfo_socket_get_port, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, setOption, arginfo_socket_set_option, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, getRemoteAddress, arginfo_socket_stream_get_remote_address, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, getRemotePort, arginfo_socket_stream_get_remote_port, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, read, arginfo_readable_stream_read, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, getReadableStream, arginfo_duplex_stream_get_readable_stream, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, write, arginfo_writable_stream_write, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, writeAsync, arginfo_socket_stream_write_async, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, getWriteQueueSize, arginfo_socket_get_write_queue_size, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, getWritableStream, arginfo_duplex_stream_get_writable_stream, ZEND_ACC_PUBLIC)
	ZEND_ME(Pipe, export, arginfo_pipe_export, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


ASYNC_CALLBACK server_disposed(uv_handle_t *handle)
{
	async_pipe_server *server;

	server = (async_pipe_server *) handle->data;
	
	ZEND_ASSERT(server != NULL);

	ASYNC_DELREF(&server->std);
}

ASYNC_CALLBACK dispose_pending_cb(uv_handle_t *handle)
{
	async_task_scheduler *scheduler;
	
	scheduler = (async_task_scheduler *) handle->data;

	ZEND_ASSERT(scheduler != NULL);

	efree(handle);
	
	ASYNC_DELREF(&scheduler->std);
}

ASYNC_CALLBACK shutdown_server(void *arg, zval *error)
{
	async_pipe_server *server;
	async_uv_op *op;
	
	uv_tcp_t *stream;
	
	server = (async_pipe_server *) arg;
	
	ZEND_ASSERT(server != NULL);
	
	server->cancel.func = NULL;
	
	if (error != NULL && Z_TYPE_P(&server->error) == IS_UNDEF) {
		ZVAL_COPY(&server->error, error);
	}
	
	while (server->pending > 0) {
		server->pending--;
		
		stream = emalloc(sizeof(uv_stream_t));
		
		uv_tcp_init(&server->scheduler->loop, stream);
		uv_accept((uv_stream_t *) &server->handle, (uv_stream_t *) stream);
		
		stream->data = server->scheduler;
		ASYNC_ADDREF(&server->scheduler->std);
		
		uv_close((uv_handle_t *) stream, dispose_pending_cb);
	}
	
	while (server->accepts.first != NULL) {
		ASYNC_NEXT_CUSTOM_OP(&server->accepts, op, async_uv_op);
		
		if (Z_TYPE_P(&server->error) != IS_UNDEF) {
			ASYNC_FAIL_OP(op, &server->error);
		} else {
			op->code = UV_ECANCELED;
		
			ASYNC_FINISH_OP(op);
		}
	}
	
	if (!uv_is_closing((uv_handle_t *) &server->handle)) {
		ASYNC_ADDREF(&server->std);
		
		uv_close((uv_handle_t *) &server->handle, server_disposed);
	}
}

static async_pipe_server *async_piper_server_object_create()
{
	async_pipe_server *server;
	
	server = ecalloc(1, sizeof(async_pipe_server));
	
	zend_object_std_init(&server->std, async_pipe_server_ce);
	server->std.handlers = &async_pipe_server_handlers;
	
	server->scheduler = async_task_scheduler_ref();
	
	server->cancel.object = server;
	server->cancel.func = shutdown_server;
	
	ASYNC_LIST_APPEND(&server->scheduler->shutdown, &server->cancel);
	
	uv_pipe_init(&server->scheduler->loop, &server->handle, 0);
	
	server->handle.data = server;
	
	return server;
}

static void async_pipe_server_object_dtor(zend_object *object)
{
	async_pipe_server *server;
	
	server = (async_pipe_server *) object;
	
	if (server->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&server->scheduler->shutdown, &server->cancel);
		
		server->cancel.func(server, NULL);
	}
}

static void async_pipe_server_object_destroy(zend_object *object)
{
	async_pipe_server *server;
	
	server = (async_pipe_server *) object;
	
	if (server->name != NULL) {
		zend_string_release(server->name);
	}
	
	zval_ptr_dtor(&server->error);
	
	async_task_scheduler_unref(server->scheduler);
	
	zend_object_std_dtor(&server->std);
}

ASYNC_CALLBACK listen_cb(uv_stream_t *stream, int status)
{
	async_pipe_server *server;
	async_uv_op *op;
	
	server = (async_pipe_server *) stream->data;
	
	ZEND_ASSERT(server != NULL);
	
	if (server->accepts.first == NULL) {
		server->pending++;
	} else {
		ASYNC_NEXT_CUSTOM_OP(&server->accepts, op, async_uv_op);
		
		op->code = status;
				
		ASYNC_FINISH_OP(op);
	}
}

static void create_server(async_pipe_server **result, zend_execute_data *execute_data, zval *return_value)
{
	async_pipe_server *server;
	
	zend_string *name;
	zend_long ipc;
	
	int code;
	
	*result = NULL;
	
	ipc = 0;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_STR(name)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(ipc)
	ZEND_PARSE_PARAMETERS_END();
	
#ifdef ZEND_WIN32
	smart_str tmp = {0};
	
	smart_str_appends(&tmp, "\\\\.\\pipe\\");
	smart_str_append(&tmp, name);
	
	name = smart_str_extract(&tmp);
#endif
	
	server = async_piper_server_object_create();
	
	if (ipc) {
		server->flags |= ASYNC_PIPE_FLAG_IPC;
	}
	
	code = uv_pipe_bind(&server->handle, ZSTR_VAL(name));
	
	if (UNEXPECTED(code) != 0) {
#ifdef ZEND_WIN32
		zend_string_release(name);
#endif

		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to bind server: %s", uv_strerror(code));
		ASYNC_DELREF(&server->std);
		return;
	}

#ifdef ZEND_WIN32
	server->name = name;
#else
	server->name = zend_string_copy(name);
#endif
	
	uv_unref((uv_handle_t *) &server->handle);
	
	*result = server;
}

static ZEND_METHOD(PipeServer, bind)
{
	async_pipe_server *server;
	
	create_server(&server, execute_data, return_value);
	
	if (EXPECTED(server)) {
		server->flags = ASYNC_PIPE_FLAG_LAZY;
		
		RETURN_OBJ(&server->std);
	}
}

static ZEND_METHOD(PipeServer, listen)
{
	async_pipe_server *server;
	
	int code;
	
	create_server(&server, execute_data, return_value);
	
	if (EXPECTED(server)) {
		code = uv_listen((uv_stream_t *) &server->handle, 128, listen_cb);
	
		if (UNEXPECTED(code != 0)) {
			zend_throw_exception_ex(async_socket_exception_ce, 0, "Server failed to listen: %s", uv_strerror(code));
			ASYNC_DELREF(&server->std);
			return;
		}
		
		RETURN_OBJ(&server->std);
	}
}

static ZEND_METHOD(PipeServer, import)
{
	async_pipe_server *server;
	
	zval *conn;
	zend_long ipc;
	
	char buf[128];
	size_t size;
	
	ipc = 0;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_ZVAL(conn)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(ipc)
	ZEND_PARSE_PARAMETERS_END();
	
	server = async_piper_server_object_create();
	server->flags = ASYNC_PIPE_FLAG_LAZY;
	
	if (ipc) {
		server->flags |= ASYNC_PIPE_FLAG_IPC;
	}
	
	async_pipe_import_stream((async_pipe *) Z_OBJ_P(conn), (uv_stream_t *) &server->handle);
	
	if (UNEXPECTED(EG(exception))) {
		ASYNC_DELREF(&server->std);
		return;
	}
	
	size = 128;
	
	if (EXPECTED(0 == uv_pipe_getsockname(&server->handle, buf, &size))) {
		server->name = zend_string_init(buf, size, 0);
	} else {
		server->name = zend_string_init("unknown", sizeof("unknown")-1, 0);
	}
	
	RETURN_OBJ(&server->std);
}

static ZEND_METHOD(PipeServer, export)
{
	async_pipe_server *server;
	
	zval *conn;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(conn)
	ZEND_PARSE_PARAMETERS_END();
	
	server = (async_pipe_server *) Z_OBJ_P(getThis());

	async_pipe_export_stream((async_pipe *) Z_OBJ_P(conn), (uv_stream_t *) &server->handle);
}

static ZEND_METHOD(PipeServer, close)
{
	async_pipe_server *server;

	zval error;
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	server = (async_pipe_server *) Z_OBJ_P(getThis());

	if (server->cancel.func == NULL) {
		return;
	}

	ASYNC_PREPARE_EXCEPTION(&error, async_socket_exception_ce, "Server has been closed");

	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}
	
	ASYNC_LIST_REMOVE(&server->scheduler->shutdown, &server->cancel);
	
	server->cancel.func(server, &error);

	zval_ptr_dtor(&error);
}

static ZEND_METHOD(PipeServer, getAddress)
{
	async_pipe_server *server;

	ZEND_PARSE_PARAMETERS_NONE();

	server = (async_pipe_server *) Z_OBJ_P(getThis());

	RETURN_STR_COPY(server->name);
}

static ZEND_METHOD(PipeServer, getPort)
{
	ZEND_PARSE_PARAMETERS_NONE();
}

static ZEND_METHOD(PipeServer, setOption)
{
	zend_long option;
	zval *val;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(option)
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	RETURN_FALSE;
}

static ZEND_METHOD(PipeServer, accept)
{
	async_pipe_server *server;
	async_pipe *pipe;
	async_context *context;	
	async_uv_op *op;
	
	int code;

	ZEND_PARSE_PARAMETERS_NONE();

	server = (async_pipe_server *) Z_OBJ_P(getThis());
	
	if (server->flags & ASYNC_PIPE_FLAG_LAZY) {
		code = uv_listen((uv_stream_t *) &server->handle, 128, listen_cb);
	
		ASYNC_CHECK_EXCEPTION(code != 0, async_socket_exception_ce, "Server failed to listen: %s", uv_strerror(code));
		
		server->flags ^= ASYNC_PIPE_FLAG_LAZY;
	}
	
	if (server->pending == 0) {
		if (UNEXPECTED(Z_TYPE_P(&server->error) != IS_UNDEF)) {
			ASYNC_FORWARD_ERROR(&server->error);
			return;
		}

		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op));
		ASYNC_APPEND_OP(&server->accepts, op);
		
		context = async_context_get();
		
		ASYNC_UNREF_ENTER(context, server);
		code = async_await_op((async_op *) op);
		ASYNC_UNREF_EXIT(context, server);
		
		if (UNEXPECTED(code == FAILURE)) {
			ASYNC_FORWARD_OP_ERROR(op);
			ASYNC_FREE_OP(op);
			
			return;
		}
		
		code = op->code;
		
		ASYNC_FREE_OP(op);
		
		ASYNC_CHECK_EXCEPTION(code < 0, async_socket_exception_ce, "Failed to await pipe connection: %s", uv_strerror(code));
	} else {
		server->pending--;
	}

	pipe = async_pipe_object_create((server->flags & ASYNC_PIPE_FLAG_IPC) ? 1 : 0);

	code = uv_accept((uv_stream_t *) &server->handle, (uv_stream_t *) &pipe->handle);

	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to accept pipe connection: %s", uv_strerror(code));
		ASYNC_DELREF(&pipe->std);
		
		return;
	}

	pipe->server = server;
	pipe->name = zend_string_copy(server->name);

	ASYNC_ADDREF(&server->std);

	RETURN_OBJ(&pipe->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_pipe_server_bind, 0, 1, Phalcon\\Async\\Network\\Pipe\\Server, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, ipc, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_pipe_server_listen, 0, 1, Phalcon\\Async\\Network\\Pipe\\Server, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, ipc, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_pipe_server_import, 0, 1, Phalcon\\Async\\Network\\Pipe\\Server, 0)
	ZEND_ARG_OBJ_INFO(0, pipe, Phalcon\\Async\\Network\\Pipe\\Server, 0)
	ZEND_ARG_TYPE_INFO(0, ipc, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pipe_server_export, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, pipe, Phalcon\\Async\\Network\\Pipe, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pipe_server_export, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_OBJ_INFO(0, pipe, Phalcon\\Async\\Network\\Pipe, 0)
ZEND_END_ARG_INFO()
#endif

static const zend_function_entry async_pipe_server_functions[] = {
	ZEND_ME(PipeServer, bind, arginfo_pipe_server_bind, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(PipeServer, listen, arginfo_pipe_server_listen, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(PipeServer, import, arginfo_pipe_server_import, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(PipeServer, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	ZEND_ME(PipeServer, getAddress, arginfo_socket_get_address, ZEND_ACC_PUBLIC)
	ZEND_ME(PipeServer, getPort, arginfo_socket_get_port, ZEND_ACC_PUBLIC)
	ZEND_ME(PipeServer, setOption, arginfo_socket_set_option, ZEND_ACC_PUBLIC)
	ZEND_ME(PipeServer, accept, arginfo_server_accept, ZEND_ACC_PUBLIC)
	ZEND_ME(PipeServer, export, arginfo_pipe_server_export, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


ASYNC_CALLBACK close_import_cb(uv_handle_t *handle)
{
	efree(handle->data);
}

int async_pipe_import(uv_stream_t *handle, zend_object **object)
{
	async_pipe *pipe;
	async_pipe_server *server;
	
	int code;
	
	pipe = ecalloc(1, MAX(sizeof(async_pipe), sizeof(async_pipe_server)));
	
	pipe->scheduler = async_task_scheduler_get();

	uv_pipe_init(&pipe->scheduler->loop, &pipe->handle, 0);
	
	code = uv_accept(handle, (uv_stream_t *) &pipe->handle);
	
	if (UNEXPECTED(code != 0)) {
		*object = NULL;
		
		pipe->handle.data = pipe;
	
		uv_close((uv_handle_t *) &pipe->handle, close_import_cb);
	
		return code;
	}
	
	if (uv_is_writable((uv_stream_t *) &pipe->handle)) {	
		zend_object_std_init(&pipe->std, async_pipe_ce);
		pipe->std.handlers = &async_pipe_handlers;
		
		pipe->cancel.object = pipe;
		pipe->cancel.func = shutdown_pipe;
		
		ASYNC_LIST_APPEND(&pipe->scheduler->shutdown, &pipe->cancel);
		
		pipe->stream = async_stream_init((uv_stream_t *) &pipe->handle, 0);
	} else {
		zend_object_std_init(&pipe->std, async_pipe_server_ce);
		pipe->std.handlers = &async_pipe_server_handlers;
		
		server = (async_pipe_server *) pipe;
		
		server->cancel.object = server;
		server->cancel.func = shutdown_server;
		
		ASYNC_LIST_APPEND(&server->scheduler->shutdown, &server->cancel);
		
		server->handle.data = server;
		
		uv_listen((uv_stream_t *) &server->handle, 128, listen_cb);
		uv_unref((uv_handle_t *) &server->handle);
	}
	
	ASYNC_ADDREF(&pipe->scheduler->std);
	
	*object = &pipe->std;
		
	return SUCCESS;
}

int async_pipe_export(zend_object *object, uv_stream_t **handle)
{
	if (object->ce == async_pipe_ce) {
		*handle = (uv_stream_t *) &((async_pipe *) object)->handle;
	
		return SUCCESS;
	}
	
	if (object->ce == async_pipe_server_ce) {
		*handle = (uv_stream_t *) &((async_pipe_server *) object)->handle;
	
		return SUCCESS;
	}

	return FAILURE;
}


void async_pipe_import_stream(async_pipe *pipe, uv_stream_t *handle)
{
	async_stream_read_req read;
	
	ASYNC_CHECK_EXCEPTION(!(pipe->flags & ASYNC_PIPE_FLAG_IPC), async_stream_exception_ce, "Reading a handle requires pipe to be opened with IPC support");

	read.in.len = 0;
	read.in.buffer = NULL;
	read.in.handle = handle;
	read.in.timeout = 0;
	read.in.flags = ASYNC_STREAM_READ_REQ_FLAG_IMPORT;
	
	if (EXPECTED(SUCCESS == async_stream_read(pipe->stream, &read))) {
		return;
	}
	
	forward_stream_read_error(&read);
}

void async_pipe_export_stream(async_pipe *pipe, uv_stream_t *handle)
{
	async_stream_write_req write;
	
	ASYNC_CHECK_EXCEPTION(!(pipe->flags & ASYNC_PIPE_FLAG_IPC), async_stream_exception_ce, "Writing a handle requires pipe to be opened with IPC support");
	
	write.in.len = 0;
	write.in.buffer = NULL;
	write.in.handle = handle;
	write.in.str = NULL;
	write.in.ref = NULL;
	write.in.flags = ASYNC_STREAM_WRITE_REQ_FLAG_EXPORT;
	
	if (UNEXPECTED(FAILURE == async_stream_write(pipe->stream, &write))) {
		forward_stream_write_error(&write);
}
}


void async_pipe_ce_register()
{
	zend_class_entry ce;
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Network\\Pipe", async_pipe_functions);
	async_pipe_ce = zend_register_internal_class(&ce);
	async_pipe_ce->ce_flags |= ZEND_ACC_FINAL;
	async_pipe_ce->serialize = zend_class_serialize_deny;
	async_pipe_ce->unserialize = zend_class_unserialize_deny;

	zend_class_implements(async_pipe_ce, 1, async_socket_stream_ce);

	memcpy(&async_pipe_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_pipe_handlers.dtor_obj = async_pipe_object_dtor;
	async_pipe_handlers.free_obj = async_pipe_object_destroy;
	async_pipe_handlers.clone_obj = NULL;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Network\\Pipe\\Server", async_pipe_server_functions);
	async_pipe_server_ce = zend_register_internal_class(&ce);
	async_pipe_server_ce->ce_flags |= ZEND_ACC_FINAL;
	async_pipe_server_ce->serialize = zend_class_serialize_deny;
	async_pipe_server_ce->unserialize = zend_class_unserialize_deny;
	
	zend_class_implements(async_pipe_server_ce, 1, async_server_ce);

	memcpy(&async_pipe_server_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_pipe_server_handlers.dtor_obj = async_pipe_server_object_dtor;
	async_pipe_server_handlers.free_obj = async_pipe_server_object_destroy;
	async_pipe_server_handlers.clone_obj = NULL;
}

#endif /* PHALCON_USE_UV */
