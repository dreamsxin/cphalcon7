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

#ifndef ASYNC_STREAM_H
#define ASYNC_STREAM_H

#include "async/async_buffer.h"
#include "async/async_ssl.h"

#include "kernel/backend.h"

#define ASYNC_STREAM_EOF 1
#define ASYNC_STREAM_CLOSED (1 << 1)
#define ASYNC_STREAM_SHUT_RD (1 << 2)
#define ASYNC_STREAM_SHUT_WR (1 << 3)
#define ASYNC_STREAM_READING (1 << 4)
#define ASYNC_STREAM_SHUTDOWN (1 << 5)
#define ASYNC_STREAM_WANT_READ (1 << 6)
#define ASYNC_STREAM_DELAY_SHUTDOWN (1 << 7)
#define ASYNC_STREAM_WRITING (1 << 8)
#define ASYNC_STREAM_SSL_FATAL (1 << 9)
#define ASYNC_STREAM_IPC (1 << 10)

#define ASYNC_STREAM_SHUT_RDWR (ASYNC_STREAM_SHUT_RD | ASYNC_STREAM_SHUT_WR)

typedef struct _async_stream async_stream;
typedef struct _async_stream_import async_stream_import;

typedef void (* async_stream_write_cb)(void *arg);
typedef void (* async_stream_dispose_cb)(void *arg);

#define ASYNC_STREAM_READ_REQ_FLAG_IMPORT 1

typedef struct _async_stream_read_req {
	struct {
		size_t len;
		char *buffer;
		uv_stream_t *handle;
		uint64_t timeout;
		uint8_t flags;
	} in;
	struct {
		size_t len;
		zend_string *str;
		int error;
#ifdef HAVE_ASYNC_SSL
		int ssl_error;
#endif
	} out;
} async_stream_read_req;

#define ASYNC_STREAM_WRITE_REQ_FLAG_ASYNC 1
#define ASYNC_STREAM_WRITE_REQ_FLAG_EXPORT (1 << 1)

typedef struct _async_stream_write_req {
	struct {
		size_t len;
		char *buffer;
		uv_stream_t *handle;
		zend_string *str;
		zval *ref;
		uint8_t flags;
		async_context *context;
		void (* callback)(async_op *);
		void *arg;
	} in;
	struct {
		int error;
#ifdef HAVE_ASYNC_SSL
		int ssl_error;
#endif
	} out;
} async_stream_write_req;

typedef struct _async_stream_read_op {
	async_op base;
	async_stream_read_req *req;
} async_stream_read_op;

typedef struct _async_stream_shutdown_request {
	uv_shutdown_t req;
	zval ref;
} async_stream_shutdown_request;

struct _async_stream {
	uv_stream_t *handle;
	uv_timer_t timer;
	uint16_t flags;
	zend_uchar ref_count;
	async_ring_buffer buffer;
	async_ssl_engine ssl;
	async_stream_read_op read;
	async_stream_shutdown_request shutdown;
	async_op_list writes;
	async_op_list flush;
	zval read_error;
	zval write_error;
	zval ref;
	async_stream_dispose_cb dispose;
	void *arg;
};

#define ASYNC_STREAM_WRITE_OP_FLAG_NEEDS_FREE 1
#define ASYNC_STREAM_WRITE_OP_FLAG_ASYNC (1 << 1)
#define ASYNC_STREAM_WRITE_OP_FLAG_STARTED (1 << 2)
#define ASYNC_STREAM_WRITE_OP_FLAG_EXPORT (1 << 3)
#define ASYNC_STREAM_WRITE_OP_FLAG_CALLBACK (1 << 4)

typedef struct _async_stream_write_buf {
	size_t size;
	size_t offset;
	char *data;
} async_stream_write_buf;

typedef struct _async_stream_write_op {
	async_op base;
	async_stream *stream;
	async_context *context;
	uint8_t flags;
	int code;
#ifdef HAVE_ASYNC_SSL
	int ssl_error;
#endif
	uv_write_t req;
	uv_stream_t *handle;
	zend_string *str;
	zval ref;
	async_stream_write_buf in;
	async_stream_write_buf out;
} async_stream_write_op;

typedef struct _async_stream_reader {
	zend_object std;
	zend_object *ref;
	async_stream *stream;
	zval *error;
} async_stream_reader;

typedef struct _async_stream_writer {
	zend_object std;
	zend_object *ref;
	async_stream *stream;
	zval *error;
} async_stream_writer;

async_stream *async_stream_init(uv_stream_t *handle, size_t bufsize);
void async_stream_free(async_stream *stream);
void async_stream_close(async_stream *stream, zval *ref);
void async_stream_close_cb(async_stream *stream, async_stream_dispose_cb callback, void *data);
void async_stream_shutdown(async_stream *stream, int how, zval *ref);
void async_stream_flush(async_stream *stream);
int async_stream_read(async_stream *stream, async_stream_read_req *req);
int async_stream_write(async_stream *stream, async_stream_write_req *req);

#ifdef HAVE_ASYNC_SSL
int async_stream_ssl_handshake(async_stream *stream, async_ssl_handshake_data *data);
#endif

static zend_always_inline void set_stream_receive_buffer(async_stream *stream, int v)
{
	uv_recv_buffer_size((uv_handle_t *) stream->handle, &v);
}

static zend_always_inline void set_stream_send_buffer(async_stream *stream, int v)
{
	uv_send_buffer_size((uv_handle_t *) stream->handle, &v);
}

static zend_always_inline int async_stream_call_close(zval *stream)
{
	zval tmp;
	
	if (stream == NULL) {
		return 0;
	}
	
	ZVAL_UNDEF(&tmp);
#if PHP_VERSION_ID >= 80000
	zend_call_method_with_0_params(Z_OBJ_P(stream), Z_OBJCE_P(stream), NULL, "close", &tmp);
#else
	zend_call_method_with_0_params(stream, Z_OBJCE_P(stream), NULL, "close", &tmp);
#endif
	zval_ptr_dtor(&tmp);
	
	return 1;
}

static zend_always_inline int async_stream_call_close_obj(zend_object *object)
{
	zval obj;
	zval tmp;
	
	if (object == NULL) {
		return 0;
	}
	
	ZVAL_OBJ(&obj, object);
	ZVAL_UNDEF(&tmp);

#if PHP_VERSION_ID >= 80000
	zend_call_method_with_0_params(Z_OBJ(obj), Z_OBJCE_P(&obj), NULL, "close", &tmp);
#else
	zend_call_method_with_0_params(&obj, Z_OBJCE_P(&obj), NULL, "close", &tmp);
#endif
	zval_ptr_dtor(&tmp);
	
	return 1;
}

static zend_always_inline zend_bool is_socket_disconnect_error(uv_handle_t *handle, int error)
{
	if (handle->type != UV_TCP && handle->type != UV_NAMED_PIPE) {
		return 0;
	}

	switch (error) {
	case UV_ECONNABORTED:
	case UV_ECONNRESET:
	case UV_ENETDOWN:
	case UV_ENETUNREACH:
	case UV_ENOTCONN:
	case UV_ESHUTDOWN:
	case UV_ETIMEDOUT:
	case UV_EOF:
		return 1;
	}
	
	return 0;
}

static zend_always_inline void forward_stream_read_error(async_stream *stream, async_stream_read_req *req)
{
	ASYNC_RETURN_ON_ERROR();
	
#ifdef HAVE_ASYNC_SSL
	ASYNC_CHECK_EXCEPTION(req->out.ssl_error, async_stream_exception_ce, "Read operation failed: SSL %s", ERR_reason_error_string(req->out.ssl_error));
#endif

	ASYNC_CHECK_EXCEPTION(req->out.error == UV_EALREADY, async_pending_read_exception_ce, "Cannot read while another read is pending");
	
	if (is_socket_disconnect_error((uv_handle_t *) stream->handle, req->out.error)) {
		zend_throw_exception_ex(async_socket_disconnect_exception_ce, 0, "Read operation failed: %s", uv_strerror(req->out.error));
	} else {
		zend_throw_exception_ex(async_stream_exception_ce, 0, "Read operation failed: %s", uv_strerror(req->out.error));
	}
}

static zend_always_inline void forward_stream_write_error(async_stream *stream, async_stream_write_req *req)
{
	ASYNC_RETURN_ON_ERROR();
	
#ifdef HAVE_ASYNC_SSL
	ASYNC_CHECK_EXCEPTION(req->out.ssl_error, async_stream_exception_ce, "Write operation failed: SSL %s", ERR_reason_error_string(req->out.ssl_error));
#endif

	if (is_socket_disconnect_error((uv_handle_t *) stream->handle, req->out.error)) {
		zend_throw_exception_ex(async_socket_disconnect_exception_ce, 0, "Write operation failed: %s", uv_strerror(req->out.error));
	} else {	
		zend_throw_exception_ex(async_stream_exception_ce, 0, "Write operation failed: %s", uv_strerror(req->out.error));
	}
}

static zend_always_inline void async_stream_call_read(async_stream *stream, zval *error, INTERNAL_FUNCTION_PARAMETERS)
{
	async_stream_read_req read;

	zval *hint = NULL, *timeout = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(hint)
		Z_PARAM_ZVAL(timeout)
	ZEND_PARSE_PARAMETERS_END();

	read.in.buffer = NULL;
	read.in.handle = NULL;
	read.in.timeout = 0;
	read.in.flags = 0;

	if (hint == NULL || Z_TYPE_P(hint) == IS_NULL) {
		read.in.len = stream->buffer.size;
	} else if (Z_LVAL_P(hint) < 1) {
		zend_throw_exception_ex(async_stream_exception_ce, 0, "Invalid read length: %d", (int) Z_LVAL_P(hint));
		return;
	} else {
		read.in.len = (size_t) Z_LVAL_P(hint);
	}

	if (timeout != NULL && Z_TYPE_P(timeout) == IS_LONG && Z_LVAL_P(timeout) > 0) {
		read.in.timeout = Z_LVAL_P(timeout);
	}

	if (UNEXPECTED(Z_TYPE_P(error) != IS_UNDEF)) {
		zval tmp = {};
		ASYNC_PREPARE_EXCEPTION(&tmp, execute_data, async_stream_exception_ce, "Read operation failed");
		trace_error_info(&tmp, error);
		ASYNC_FORWARD_ERROR(&tmp);
		zval_ptr_dtor(&tmp);
		return;
	}

	if (EXPECTED(SUCCESS == async_stream_read(stream, &read))) {
		if (EXPECTED(read.out.len)) {
			RETURN_STR(read.out.str);
		}

		return;
	}

	forward_stream_read_error(stream, &read);
}

static zend_always_inline void async_stream_call_write(async_stream *stream, zval *error, INTERNAL_FUNCTION_PARAMETERS)
{
	async_stream_write_req write;

	zend_string *data;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(data)
	ZEND_PARSE_PARAMETERS_END();

	if (UNEXPECTED(Z_TYPE_P(error) != IS_UNDEF)) {
		zval tmp = {};
		ASYNC_PREPARE_EXCEPTION(&tmp, execute_data, async_stream_exception_ce, "Write operation failed");
		trace_error_info(&tmp, error);
		ASYNC_FORWARD_ERROR(&tmp);
		zval_ptr_dtor(&tmp);
		return;
	}

	write.in.len = ZSTR_LEN(data);
	write.in.buffer = ZSTR_VAL(data);
	write.in.handle = NULL;
	write.in.str = data;
	write.in.ref = getThis();
	write.in.flags = 0;
	write.in.callback = NULL;

	if (UNEXPECTED(FAILURE == async_stream_write(stream, &write))) {
		forward_stream_write_error(stream, &write);
	}
}

async_stream_reader *async_stream_reader_create(async_stream *stream, zend_object *ref, zval *error);
async_stream_writer *async_stream_writer_create(async_stream *stream, zend_object *ref, zval *error);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO();

// ReadableStream

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readable_stream_read, 0, 0, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
ZEND_END_ARG_INFO();

// WritableStream

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_writable_stream_write, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO();

// DuplexStream

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_duplex_stream_get_readable_stream, 0, 0, Phalcon\\Async\\Stream\\ReadableStream, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_duplex_stream_get_writable_stream, 0, 0, Phalcon\\Async\\Stream\\WritableStream, 0)
ZEND_END_ARG_INFO();

#endif
