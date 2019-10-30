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
#include "async/async_ssl.h"
#include "async/async_stream.h"

#include <Zend/zend_smart_str.h>
#include <Zend/zend_inheritance.h>

ASYNC_API zend_class_entry *async_duplex_stream_ce;
ASYNC_API zend_class_entry *async_pending_read_exception_ce;
ASYNC_API zend_class_entry *async_readable_memory_stream_ce;
ASYNC_API zend_class_entry *async_readable_stream_ce;
ASYNC_API zend_class_entry *async_stream_closed_exception_ce;
ASYNC_API zend_class_entry *async_stream_exception_ce;
ASYNC_API zend_class_entry *async_stream_reader_ce;
ASYNC_API zend_class_entry *async_stream_writer_ce;
ASYNC_API zend_class_entry *async_writable_memory_stream_ce;
ASYNC_API zend_class_entry *async_writable_stream_ce;

static zend_object_handlers async_readable_memory_stream_handlers;
static zend_object_handlers async_writable_memory_stream_handlers;

static zend_object_handlers async_stream_reader_handlers;
static zend_object_handlers async_stream_writer_handlers;

#define ASYNC_READABLE_MEMORY_STREAM_FLAG_CLOSED 1

typedef struct _async_readable_memory_stream {
	zend_object std;

	uint8_t flags;

	size_t offset;
	zend_string *data;
	zval error;
} async_readable_memory_stream;

#define ASYNC_WRITABLE_MEMORY_STREAM_FLAG_CLOSED 1

typedef struct _async_writable_memory_stream {
	zend_object std;

	uint8_t flags;

	smart_str data;
	zval error;
} async_writable_memory_stream;

ASYNC_CALLBACK write_cb(uv_write_t *req, int status);


static zend_always_inline int await_op(async_stream *stream, async_op *op)
{
	async_context *context;
	int code;
	
	context = async_context_get();
	
	if (!async_context_is_background(context) && ++stream->ref_count == 1) {
		uv_ref((uv_handle_t *) stream->handle);
	}
	
	code = async_await_op((async_op *) op);
	
	if (!async_context_is_background(context) && --stream->ref_count == 0) {
		uv_unref((uv_handle_t *) stream->handle);
	}
	
	return code;
}

static zend_always_inline int encode_buffer(async_stream *stream, async_stream_write_op *op)
{
	ZEND_ASSERT(op->in.offset < op->in.size);

	op->out.data = op->in.data;
	op->out.size = op->in.size;
	op->out.offset = op->in.offset;
	
	op->in.offset = op->in.size;
	
	return (int) op->out.size;
}

#ifdef HAVE_ASYNC_SSL

typedef struct _async_stream_ssl_sync {
	uv_write_t req;
	uv_buf_t buf;
} async_stream_ssl_sync;

ASYNC_CALLBACK ssl_sync_cb(uv_write_t *req, int status)
{
	async_stream_ssl_sync *sync;
	
	sync = (async_stream_ssl_sync *) req->data;
	
	ZEND_ASSERT(sync != NULL);

	efree(sync->buf.base);
	efree(sync);
}

static int ssl_encode_buffer(async_stream *stream, async_stream_write_op *op)
{
	int code;
	int error;
	
	size_t len;
	
	ZEND_ASSERT(op->in.offset < op->in.size);
	
	op->out.data = NULL;
	op->out.size = 0;
	op->out.offset = 0;
	
	op->flags &= ~ASYNC_STREAM_WRITE_OP_FLAG_NEEDS_FREE;
	
	ERR_clear_error();
	code = SSL_write(stream->ssl.ssl, op->in.data + op->in.offset, (int) (op->in.size - op->in.offset));
	
	if (UNEXPECTED(code < 0)) {
		error = SSL_get_error(stream->ssl.ssl, code);
		
		switch (error) {
		case SSL_ERROR_ZERO_RETURN:
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ:
			return 0;
		case SSL_ERROR_SYSCALL:
		case SSL_ERROR_SSL:
			stream->flags |= ASYNC_STREAM_SSL_FATAL;
			break;
		}
		
		op->code = UV_EPROTO;
		op->ssl_error = (int) ERR_get_error();
		
		return FAILURE;
	}
	
	op->in.offset += code;
	
	op->out.size = BIO_ctrl_pending(stream->ssl.wbio);
	op->out.data = emalloc(op->out.size);
	
	len = 0;
	
	while (len < op->out.size) {
		ERR_clear_error();
		code = BIO_read(stream->ssl.wbio, op->out.data + len, (int) (op->out.size - len));
		
		if (UNEXPECTED(code < 1)) {
			efree(op->out.data);
			
			op->out.data = NULL;
			op->out.size = 0;
			
			op->code = UV_EPROTO;
			op->ssl_error = (int) ERR_get_error();
			
			return FAILURE;
		}
		
		len += code;
	}
	
	op->flags |= ASYNC_STREAM_WRITE_OP_FLAG_NEEDS_FREE;
	
	return (int) len;
}

#define ASYNC_STREAM_ENCODE_BUFFER(stream, op) (((stream)->ssl.ssl == NULL) ? encode_buffer(stream, op) : ssl_encode_buffer(stream, op))

#else

#define ASYNC_STREAM_ENCODE_BUFFER(stream, op) encode_buffer(stream, op)

#endif

static zend_always_inline void init_buffer(async_stream *stream)
{
	stream->buffer.base = emalloc(stream->buffer.size);
	stream->buffer.rpos = stream->buffer.base;
	stream->buffer.wpos = stream->buffer.base;
}

async_stream *async_stream_init(uv_stream_t *handle, size_t bufsize)
{
	async_stream *stream;
	
	stream = ecalloc(1, sizeof(async_stream));
	
	stream->buffer.size = MAX(bufsize, 0x8000);

	stream->handle = handle;
	handle->data = stream;
	
	uv_timer_init(handle->loop, &stream->timer);
	
	stream->timer.data = stream;
	
	uv_unref((uv_handle_t *) handle);
	uv_unref((uv_handle_t *) &stream->timer);
	
	stream->shutdown.req.data = stream;
	
	return stream;
}

void async_stream_free(async_stream *stream)
{
	if (stream->buffer.base != NULL) {
		efree(stream->buffer.base);
		stream->buffer.base = NULL;
	}
	
	efree(stream);
}

ASYNC_CALLBACK close_stream_cb(uv_handle_t *handle)
{
	async_stream *stream;

	stream = handle->data;

	ZEND_ASSERT(stream != NULL);

	if (stream->dispose) {
		stream->dispose(stream->arg);
	}
}

#ifdef HAVE_ASYNC_SSL
static int shutdown_ssl(async_stream *stream)
{
	async_stream_ssl_sync *sync;
	
	int code;
	
	char *buf;
	size_t len;
	
	if (stream->ssl.ssl == NULL || stream->flags & ASYNC_STREAM_SSL_FATAL || SSL_get_shutdown(stream->ssl.ssl) & SSL_RECEIVED_SHUTDOWN) {
		return 1;
	}
	
	ERR_clear_error();
	code = SSL_shutdown(stream->ssl.ssl);
	
	if ((len = BIO_ctrl_pending(stream->ssl.wbio)) > 0) {
		buf = emalloc(len);

		ERR_clear_error();

		if ((code = BIO_read(stream->ssl.wbio, buf, (int) len)) > 0) {
			sync = emalloc(sizeof(async_stream_ssl_sync));
			
			sync->req.data = sync;
			sync->buf = uv_buf_init(buf, (uv_buf_size_t) len);
			
			if (0 == uv_write(&sync->req, stream->handle, &sync->buf, 1, ssl_sync_cb)) {
				return code;
			}
		}

		efree(buf);
	}
	
	return code;
}
#endif

#ifdef HAVE_ASYNC_SSL
ASYNC_CALLBACK dispose_read_cb(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
	async_stream *stream;

	int code;
	
	stream = (async_stream *) handle->data;
	
	ZEND_ASSERT(stream != NULL);

	if (stream->ssl.ssl != NULL && nread > 0 && !(stream->flags & ASYNC_STREAM_SSL_FATAL)) {
		ERR_clear_error();
		BIO_read(stream->ssl.rbio, buf->base, buf->len);

		code = shutdown_ssl(stream);
		
		if (code == 1) {
			nread = UV_EPROTO;
		} else if (UNEXPECTED(code < 0)) {
			code = SSL_get_error(stream->ssl.ssl, code);
			
			switch (code) {
			case SSL_ERROR_ZERO_RETURN:
			case SSL_ERROR_WANT_READ:
				break;
			default:
				nread = UV_EPROTO;
			}
		}
	}
	
	if (nread < 0) {
		ASYNC_UV_TRY_CLOSE(handle, close_stream_cb);
	}
	
	efree(buf->base);
}

ASYNC_CALLBACK dispose_alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->len = 512;
	buf->base = emalloc(buf->len);
}

ASYNC_CALLBACK dispose_timer_cb(uv_timer_t *handle)
{
	async_stream *stream;
	
	stream = (async_stream *) handle->data;
	
	ZEND_ASSERT(stream != NULL);
	
	ASYNC_UV_TRY_CLOSE(stream->handle, close_stream_cb);
}
#endif

void async_stream_close_cb(async_stream *stream, async_stream_dispose_cb callback, void *data)
{
	if (stream->flags & ASYNC_STREAM_CLOSED) {
		return;
	}
	
	stream->flags |= ASYNC_STREAM_CLOSED | ASYNC_STREAM_EOF | ASYNC_STREAM_SHUT_RDWR;
	
	stream->dispose = callback;
	stream->arg = data;

	ASYNC_UV_TRY_CLOSE(&stream->timer, NULL);

	if (stream->flags & ASYNC_STREAM_READING) {
		uv_read_stop(stream->handle);

		stream->flags &= ~ASYNC_STREAM_READING;
	}
	
	if (stream->read.base.status == ASYNC_STATUS_RUNNING) {
		stream->read.req->out.error = UV_ECANCELED;

		ASYNC_FINISH_OP(&stream->read);
	}

#ifdef HAVE_ASYNC_SSL
	if (stream->ssl.ssl && stream->writes.first == NULL) {
		if (1 != shutdown_ssl(stream)) {
			stream->ref_count++;
			
			uv_ref((uv_handle_t *) stream->handle);
			
			uv_read_start(stream->handle, dispose_alloc_cb, dispose_read_cb);
			uv_timer_start(&stream->timer, dispose_timer_cb, 5000, 0);
			
			return;
		}
	}
#endif
	
	if (uv_is_closing((uv_handle_t *) stream->handle)) {
		callback(data);
	} else {
		ASYNC_UV_CLOSE(stream->handle, close_stream_cb);
	}
}

ASYNC_CALLBACK close_cb(void *arg)
{
	async_stream *stream;
	
	stream = (async_stream *) arg;
	
	ZEND_ASSERT(stream != NULL);
	
	zval_ptr_dtor(&stream->ref);
	ZVAL_UNDEF(&stream->ref);
}

void async_stream_close(async_stream *stream, zval *ref)
{
	if (!(stream->flags & ASYNC_STREAM_CLOSED)) {
		if (ref != NULL) {
			ZVAL_COPY(&stream->ref, ref);
		}
		
		async_stream_close_cb(stream, close_cb, stream);
	}
}

ASYNC_CALLBACK shutdown_cb(uv_shutdown_t *req, int status)
{
	async_stream_shutdown_request *shut;
	async_stream *stream;

	shut = (async_stream_shutdown_request *) req;
	stream = req->data;
	
	ZEND_ASSERT(stream != NULL);
		
	stream->flags &= ~ASYNC_STREAM_SHUTDOWN;
	stream->flags &= ~ASYNC_STREAM_DELAY_SHUTDOWN;
	
	if (stream->flags & ASYNC_STREAM_SHUT_RD) {
		async_stream_close(stream, &shut->ref);
	}

	zval_ptr_dtor(&shut->ref);
}

static int trigger_shutdown_wr(async_stream *stream)
{
	int code;
	
#ifdef HAVE_ASYNC_SSL
	shutdown_ssl(stream);
#endif

	code = uv_shutdown(&stream->shutdown.req, stream->handle, shutdown_cb);
	
	if (code == 0) {
		stream->flags |= ASYNC_STREAM_SHUTDOWN;
	} else {
		zval_ptr_dtor(&stream->shutdown.ref);
	}
	
	return code;
}

void async_stream_shutdown(async_stream *stream, int how, zval *ref)
{
	int flags;
	int code;

	flags = stream->flags & ASYNC_STREAM_SHUT_RDWR;
	code = 0;

	if (how & ASYNC_STREAM_SHUT_RD) {
		stream->flags |= ASYNC_STREAM_EOF;
	}

	if (how & ASYNC_STREAM_SHUT_RD && !(flags & ASYNC_STREAM_SHUT_RD)) {
		stream->flags |= ASYNC_STREAM_SHUT_RD;
		
		if (stream->flags & ASYNC_STREAM_READING) {
			uv_read_stop(stream->handle);

			stream->flags &= ~ASYNC_STREAM_READING;
		}
		
#ifdef HAVE_ASYNC_SSL
		if (stream->ssl.ssl != NULL && !(SSL_get_shutdown(stream->ssl.ssl) & SSL_RECEIVED_SHUTDOWN)) {
			if (!(stream->flags & ASYNC_STREAM_SHUT_WR) && !(how & ASYNC_STREAM_SHUT_WR)) {
				stream->ref_count++;
				
				uv_ref((uv_handle_t *) stream->handle);				
				uv_read_start(stream->handle, dispose_alloc_cb, dispose_read_cb);
				
				ZVAL_COPY(&stream->ref, ref);
			}
		}
#endif
	}
	
	if (how & ASYNC_STREAM_SHUT_WR && !(flags & ASYNC_STREAM_SHUT_WR)) {
		stream->flags |= ASYNC_STREAM_SHUT_WR;
	}
	
	if (how & ASYNC_STREAM_SHUT_RD && !(flags & ASYNC_STREAM_SHUT_RD)) {
		if (stream->read.base.status == ASYNC_STATUS_RUNNING) {
			stream->read.req->out.error = UV_ECANCELED;
	
			ASYNC_FINISH_OP(&stream->read);
		}
	}
	
	if (how & ASYNC_STREAM_SHUT_WR && !(flags & ASYNC_STREAM_SHUT_WR)) {
		ZVAL_COPY(&stream->shutdown.ref, ref);
	
		if (stream->writes.first == NULL) {
			code = trigger_shutdown_wr(stream);
		} else {
			stream->flags |= ASYNC_STREAM_SHUTDOWN | ASYNC_STREAM_DELAY_SHUTDOWN;
		}
	}

	if (stream->flags & ASYNC_STREAM_SHUT_RD && stream->flags & ASYNC_STREAM_SHUT_WR) {
		if (code == 0 && !(stream->flags & ASYNC_STREAM_SHUTDOWN)) {
			async_stream_close(stream, ref);
		}
	}

	if (UNEXPECTED(code < 0)) {
		async_stream_close(stream, ref);
	
		switch (code) {
		case UV_ENOTCONN:
		case UV_EPIPE:
			// These are acceptable during shutdown.
			break;
		default:
			zend_throw_error(NULL, "Shutdown failed [%d]: %s", code, uv_strerror(code));
		}
	}
}

void async_stream_flush(async_stream *stream)
{
	async_op *op;
	
	if (stream->writes.first == NULL) {
		return;
	}
	
	ASYNC_ALLOC_OP(op);
	ASYNC_APPEND_OP(&stream->flush, op);
	
	if (UNEXPECTED(async_await_op(op)) == FAILURE) {
		ASYNC_FORWARD_OP_ERROR(op);
	}
	
	ASYNC_FREE_OP(op);
}

#ifdef HAVE_ASYNC_SSL

static int process_input_bytes(async_stream *stream, int count)
{
	async_stream_ssl_sync *sync;
	
	int len;
	int code;
	
	size_t wlen;
	
	if (stream->ssl.ssl == NULL) {
		async_ring_buffer_write_move(&stream->buffer, count);
	
		return SUCCESS;
	}

	async_ssl_bio_increment(stream->ssl.rbio, count);
	
	if (UNEXPECTED(!SSL_is_init_finished(stream->ssl.ssl))) {
		return SUCCESS;
	}
	
	code = SSL_ERROR_NONE;
	
	while (1) {
		do {
			ERR_clear_error();

			len = SSL_read(stream->ssl.ssl, stream->buffer.wpos, (int) async_ring_buffer_write_len(&stream->buffer));
			code = SSL_get_error(stream->ssl.ssl, len);
			
			if (UNEXPECTED(len < 1)) {
				switch (code) {
				case SSL_ERROR_ZERO_RETURN:
					stream->flags |= ASYNC_STREAM_EOF;					
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
					break;
				case SSL_ERROR_SYSCALL:
				case SSL_ERROR_SSL:
					stream->flags |= ASYNC_STREAM_SSL_FATAL;
				default:
					return (int) ERR_get_error();
				}
				
				break;
			}
			
			if (UNEXPECTED((wlen = BIO_ctrl_pending(stream->ssl.wbio)) > 0)) {
				sync = emalloc(sizeof(async_stream_ssl_sync));
								
				sync->buf = uv_buf_init(emalloc(wlen), (unsigned int) wlen);
				sync->req.data = sync;
				
				ERR_clear_error();				
				
				if (BIO_read(stream->ssl.wbio, sync->buf.base, sync->buf.len) > 0) {				
					uv_write(&sync->req, stream->handle, &sync->buf, 1, ssl_sync_cb);
				}
			}
			
			async_ring_buffer_write_move(&stream->buffer, len);
			
			stream->ssl.pending += len;
		} while (SSL_pending(stream->ssl.ssl));
		
		if (UNEXPECTED(code != SSL_ERROR_NONE)) {
			break;
		}

		stream->ssl.available += stream->ssl.pending;
		stream->ssl.pending = 0;
	}
	
	if (UNEXPECTED(stream->flags & ASYNC_STREAM_WANT_READ)) {
		stream->flags &= ~ASYNC_STREAM_WANT_READ;

		if (stream->writes.first != NULL) {
			write_cb(&((async_stream_write_op *) stream->writes.first)->req, 0);
		}
	}
	
	return (stream->ssl.available > 0 || stream->flags & ASYNC_STREAM_EOF) ? SUCCESS : FAILURE;
}

#define ASYNC_STREAM_BUFFER_LEN(stream) (((stream)->ssl.ssl == NULL) ? (stream)->buffer.len : (stream)->ssl.available)
#define ASYNC_STREAM_BUFFER_CONSUME(stream, length) do { \
	if ((stream)->ssl.ssl != NULL) { \
		(stream)->ssl.available -= length; \
	} \
} while (0)

#else

static zend_always_inline int process_input_bytes(async_stream *stream, int count)
{
	async_ring_buffer_write_move(&stream->buffer, count);
	
	return SUCCESS;
}

#define ASYNC_STREAM_BUFFER_LEN(stream) (stream)->buffer.len
#define ASYNC_STREAM_BUFFER_CONSUME(stream, length)

#endif

static zend_always_inline int should_start_read(async_stream *stream)
{
	if (stream->flags & ASYNC_STREAM_IPC) {
		return 0;
	}
	
	return ((stream->buffer.size - stream->buffer.len) >= 4096);
}

static zend_always_inline int should_continue_read(async_stream *stream)
{
	if (stream->flags & ASYNC_STREAM_IPC) {
		return (stream->read.base.status == ASYNC_STATUS_RUNNING);
	}

	return ((stream->buffer.size - stream->buffer.len) >= 4096);
}

ASYNC_CALLBACK read_cb(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
	async_stream *stream;
	
	size_t blen;
	
	if (UNEXPECTED(nread == 0)) {
		return;
	}
	
	stream = (async_stream *) handle->data;
	
	if (UNEXPECTED(nread == UV_ECONNRESET)) {
		nread = UV_EOF;
	}
	
	if (UNEXPECTED(nread < 0 && nread != UV_EOF)) {
		uv_read_stop(handle);
		
		stream->flags &= ~ASYNC_STREAM_READING;
		
		while (stream->read.base.status == ASYNC_STATUS_RUNNING) {
			stream->read.req->out.error = (int) nread;
			
			ASYNC_FINISH_OP(&stream->read);
		}
		
		return;
	}

	if (UNEXPECTED(nread == UV_EOF)) {
		stream->flags |= ASYNC_STREAM_EOF;
	} else {
		if (UNEXPECTED(stream->flags & ASYNC_STREAM_IPC && uv_pipe_pending_count((uv_pipe_t *) stream->handle))) {
			while (!(stream->read.req->in.flags & ASYNC_STREAM_READ_REQ_FLAG_IMPORT)) {
				stream->read.req->out.error = UV_ENOBUFS;
				
				ASYNC_FINISH_OP(&stream->read);
			}
		
			stream->read.req->out.error = uv_accept(stream->handle, stream->read.req->in.handle);
			
			nread--;
			
			ASYNC_FINISH_OP(&stream->read);
		}
	
#ifdef HAVE_ASYNC_SSL
		int code;

		if (UNEXPECTED(nread > 0 && SUCCESS != (code = process_input_bytes(stream, (int) nread)))) {
			if (code == FAILURE) {
				return;
			}
		
			uv_read_stop(handle);
			
			stream->flags &= ~ASYNC_STREAM_READING;
			
			while (stream->read.base.status == ASYNC_STATUS_RUNNING) {
				stream->read.req->out.error = UV_EPROTO;
				stream->read.req->out.ssl_error = code;
				
				ASYNC_FINISH_OP(&stream->read);
			}
			
			return;
		}
#else
		process_input_bytes(stream, (int) nread);
#endif
	}

	while (stream->read.base.status == ASYNC_STATUS_RUNNING && (blen = ASYNC_STREAM_BUFFER_LEN(stream)) > 0) {
		if (UNEXPECTED(stream->read.req->in.flags & ASYNC_STREAM_READ_REQ_FLAG_IMPORT)) {
			stream->read.req->out.error = UV_ENOBUFS;
		} else {	
			if (stream->read.req->in.buffer == NULL) {
				stream->read.req->out.len = async_ring_buffer_read_string(&stream->buffer, &stream->read.req->out.str, MIN(stream->read.req->in.len, blen));
			} else {
				stream->read.req->out.len = async_ring_buffer_read(&stream->buffer, stream->read.req->in.buffer, MIN(stream->read.req->in.len, blen));
			}
			
			ASYNC_STREAM_BUFFER_CONSUME(stream, stream->read.req->out.len);
		}
		
		ASYNC_FINISH_OP(&stream->read);
	}
	
	if (UNEXPECTED(nread == UV_EOF || stream->flags & ASYNC_STREAM_EOF)) {	
#ifdef HAVE_ASYNC_SSL
		if (stream->ssl.ssl && stream->writes.first == NULL) {
			if (1 != shutdown_ssl(stream)) {
				stream->ref_count++;
				
				uv_ref((uv_handle_t *) stream->handle);
				
				uv_read_start(stream->handle, dispose_alloc_cb, dispose_read_cb);
				uv_timer_start(&stream->timer, dispose_timer_cb, 5000, 0);
			}
		} else {
#endif
			uv_read_stop(handle);
#ifdef HAVE_ASYNC_SSL
		}
#endif
		
		stream->flags &= ~ASYNC_STREAM_READING;

		while (stream->read.base.status == ASYNC_STATUS_RUNNING) {
			ASYNC_FINISH_OP(&stream->read);
		}
	
		return;
	}
	
	if (!should_continue_read(stream)) {
		uv_read_stop(handle);
		
		stream->flags &= ~ASYNC_STREAM_READING;
	}
}

ASYNC_CALLBACK read_alloc_cb(uv_handle_t *handle, size_t suggested, uv_buf_t *buf)
{
	async_stream *stream;
	uv_buf_size_t len;
	
	stream = (async_stream *) handle->data;
	
	ZEND_ASSERT(stream != NULL);
	
	len = (uv_buf_size_t) async_ring_buffer_write_len(&stream->buffer);
	
#ifdef HAVE_ASYNC_SSL
	if (stream->ssl.ssl) {
		async_ssl_bio_expose_buffer(stream->ssl.rbio, &buf->base, &buf->len);
		
		if (buf->len > len) {
			buf->len = len;
		}
		
		return;
	}
#endif
	
	buf->base = stream->buffer.wpos;
	buf->len = len;
}

ASYNC_CALLBACK timeout_read(uv_timer_t *timer)
{
	async_stream *stream;
	
	stream = (async_stream *) timer->data;
	
	ZEND_ASSERT(stream != NULL);
	
	if (stream->read.base.status == ASYNC_STATUS_RUNNING) {
		stream->read.req->out.error = UV_ETIMEDOUT;
		
		ASYNC_FINISH_OP(&stream->read);
	}
}

int async_stream_read(async_stream *stream, async_stream_read_req *req)
{
	size_t blen;	
	int code;
	
	req->out.len = 0;
	req->out.str = NULL;
	req->out.error = 0;
	
#ifdef HAVE_ASYNC_SSL
	req->out.ssl_error = 0;
#endif
	
	if (UNEXPECTED(stream->flags & ASYNC_STREAM_SHUT_RD)) {
		req->out.error = UV_EOF;
		return FAILURE;
	}
	
	if (UNEXPECTED(stream->read.base.status == ASYNC_STATUS_RUNNING)) {
		req->out.error = UV_EALREADY;
		return FAILURE;
	}
	
	if (UNEXPECTED(stream->buffer.base == NULL)) {
		init_buffer(stream);
	}
	
	if ((blen = ASYNC_STREAM_BUFFER_LEN(stream)) > 0) {
		if (UNEXPECTED(req->in.flags & ASYNC_STREAM_READ_REQ_FLAG_IMPORT)) {
			req->out.error = UV_EALREADY;
			return FAILURE;
		}
	
		if (req->in.buffer == NULL) {
			req->out.len = async_ring_buffer_read_string(&stream->buffer, &req->out.str, MIN(req->in.len, blen));
		} else {
			req->out.len = async_ring_buffer_read(&stream->buffer, req->in.buffer, MIN(req->in.len, blen));
		}
		
		ASYNC_STREAM_BUFFER_CONSUME(stream, req->out.len);
		
		if (!(stream->flags && ASYNC_STREAM_EOF) && should_start_read(stream)) {
			if (!(stream->flags & ASYNC_STREAM_READING)) {
				uv_read_start(stream->handle, read_alloc_cb, read_cb);
				
				stream->flags |= ASYNC_STREAM_READING;
			}
		}
		
		return SUCCESS;
	}
	
	if (UNEXPECTED(req->in.flags & ASYNC_STREAM_READ_REQ_FLAG_IMPORT)) {
		if (uv_pipe_pending_count((uv_pipe_t *) stream->handle)) {
			req->out.error = uv_accept(stream->handle, req->in.handle);
			
			return SUCCESS;
		}
	}
	
	if (UNEXPECTED(stream->flags & ASYNC_STREAM_EOF)) {
		return SUCCESS;
	}
	
	if (EXPECTED(!(stream->flags & ASYNC_STREAM_READING))) {
		uv_read_start(stream->handle, read_alloc_cb, read_cb);
		
		stream->flags |= ASYNC_STREAM_READING;
	}
	
	stream->read.req = req;
	
	if (req->in.timeout > 0) {
		uv_timer_start(&stream->timer, timeout_read, req->in.timeout, 0);
	}
	
	code = await_op(stream, (async_op *) &stream->read);
	
	if (req->in.timeout > 0) {
		uv_timer_stop(&stream->timer);
	}
	
	if (UNEXPECTED(code == FAILURE)) {
		ASYNC_FORWARD_OP_ERROR(&stream->read);
		ASYNC_RESET_OP(&stream->read);
		
		return FAILURE;
	}
	
	ASYNC_RESET_OP(&stream->read);
	
	return (UNEXPECTED(req->out.error < 0)) ? FAILURE : SUCCESS;
}

static zend_always_inline void cleanup_write(async_stream_write_op *op)
{
	if (op->flags & ASYNC_STREAM_WRITE_OP_FLAG_NEEDS_FREE) {
		efree(op->out.data);
	}
	
	if (op->flags & ASYNC_STREAM_WRITE_OP_FLAG_CALLBACK) {
		op->base.callback((async_op *) op);
	}

	if (op->str != NULL) {
		zend_string_release(op->str);
	}
	
	zval_ptr_dtor(&op->ref);
	
	if (op->context != NULL) {
		ASYNC_DELREF(&op->context->std);
	}

	ASYNC_FREE_OP(op);
}

static zend_always_inline int process_write(async_stream_write_op *op)
{
	uv_buf_t bufs[1];
	int code;
	
	if (UNEXPECTED(op->flags & ASYNC_STREAM_WRITE_OP_FLAG_EXPORT)) {
		bufs[0] = uv_buf_init(".", 1);
		
		uv_write2(&op->req, op->stream->handle, bufs, 1, op->handle, write_cb);
		
		op->flags |= ASYNC_STREAM_WRITE_OP_FLAG_STARTED;
		
		return FAILURE;
	}
	
	code = ASYNC_STREAM_ENCODE_BUFFER(op->stream, op);
		
	if (UNEXPECTED(code == FAILURE)) {
		ASYNC_FINISH_OP(op);
		
		cleanup_write(op);
		
		return SUCCESS;
	}
	
	if (UNEXPECTED(code == 0)) {
		op->stream->flags |= ASYNC_STREAM_WANT_READ;
	} else {
		bufs[0] = uv_buf_init(op->out.data + op->out.offset, (unsigned int) (op->out.size - op->out.offset));
		
		uv_write(&op->req, op->stream->handle, bufs, 1, write_cb);
		
		op->flags |= ASYNC_STREAM_WRITE_OP_FLAG_STARTED;
	}
	
	return FAILURE;
}

ASYNC_CALLBACK write_cb(uv_write_t *req, int status)
{
	async_stream *stream;
	async_stream_write_op *op;
	async_stream_write_op *next;
	
	op = (async_stream_write_op *) req->data;
	
	ZEND_ASSERT(op != NULL);
	
	stream = op->stream;
	
	stream->flags |= ASYNC_STREAM_WRITING;
	
	op->code = status;
	
	if (status < 0 || op->in.offset == op->in.size) {
		ASYNC_FINISH_OP(op);
		
		cleanup_write(op);
	} else if (FAILURE == process_write(op)) {
		return;
	}
	
	do {
		next = (async_stream_write_op *) stream->writes.first;
		
		if (next && !(next->flags & ASYNC_STREAM_WRITE_OP_FLAG_STARTED)) {
			if (SUCCESS == process_write(next)) {
				continue;
			}
		}
	} while (0);
	
	if (stream->writes.first == NULL) {
		while (stream->flush.first != NULL) {
			ASYNC_FINISH_OP(stream->flush.first);
		}
	
		if (stream->flags & ASYNC_STREAM_DELAY_SHUTDOWN) {
			trigger_shutdown_wr(stream);
		}
	}
	
	stream->flags &= ~ASYNC_STREAM_WRITING;
}

static zend_always_inline void setup_async_write(async_stream_write_op *op, async_stream_write_req *req)
{
	op->flags |= ASYNC_STREAM_WRITE_OP_FLAG_ASYNC;
	op->context = req->in.context;

	if (op->context) {
		ASYNC_ADDREF(&op->context->std);
	} else {
		op->context = async_context_ref();
	}
	
	if (req->in.ref != NULL) {
		ZVAL_COPY(&op->ref, req->in.ref);
	}
	
	if (req->in.str != NULL) {
		op->str = zend_string_copy(req->in.str);
	}
}

int async_stream_write(async_stream *stream, async_stream_write_req *req)
{
	async_stream_write_op *op;
	
	uv_stream_t *handle;
	uv_buf_t bufs[1];
	int code;
	
	handle = NULL;

	req->out.error = 0;
	
#ifdef HAVE_ASYNC_SSL
	req->out.ssl_error = 0;
#endif
	
	if (UNEXPECTED(stream->flags & ASYNC_STREAM_SHUT_WR)) {
		req->out.error = UV_EOF;
		return FAILURE;
	}
	
	if (UNEXPECTED(req->in.flags & ASYNC_STREAM_WRITE_REQ_FLAG_EXPORT)) {
		if (!(stream->flags & ASYNC_STREAM_IPC)) {
			req->out.error = UV_EINVAL;
			return FAILURE;
		}
		
		handle = req->in.handle;
	}
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_stream_write_op));
	
	op->in.data = req->in.buffer;
	op->in.size = req->in.len;
	
	if (UNEXPECTED(req->in.flags & ASYNC_STREAM_WRITE_REQ_FLAG_ASYNC && req->in.callback)) {
		op->base.callback = req->in.callback;
		op->base.arg = req->in.arg;

		op->flags |= ASYNC_STREAM_WRITE_OP_FLAG_CALLBACK;
	}

	if (UNEXPECTED(req->in.flags & ASYNC_STREAM_WRITE_REQ_FLAG_EXPORT)) {
		op->flags |= ASYNC_STREAM_WRITE_OP_FLAG_EXPORT;
		op->handle = handle;
	}
	
	if (stream->writes.first == NULL && !(stream->flags & ASYNC_STREAM_WRITING)) {
		if (UNEXPECTED(op->flags & ASYNC_STREAM_WRITE_OP_FLAG_EXPORT)) {
			bufs[0] = uv_buf_init(".", 1);
			
			uv_write2(&op->req, stream->handle, bufs, 1, handle, write_cb);
		} else {
			code = ASYNC_STREAM_ENCODE_BUFFER(stream, op);
			
			if (UNEXPECTED(code == FAILURE)) {
				cleanup_write(op);
				
				return FAILURE;
			}
			
			if (UNEXPECTED(code == 0)) {
				stream->flags |= ASYNC_STREAM_WANT_READ;
			} else {
				do {					
					bufs[0] = uv_buf_init(op->out.data + op->out.offset, (unsigned int) (op->out.size - op->out.offset));
					
					code = uv_try_write(stream->handle, bufs, 1);
					
					if (code == UV_EAGAIN) {
						break;
					}
					
					if (UNEXPECTED(code < 0)) {
						cleanup_write(op);
					
						req->out.error = code;
						
						return FAILURE;
					}
					
					op->out.offset += code;
					
					if (op->out.offset == op->out.size) {
						cleanup_write(op);

						return SUCCESS;
					}
				} while (1);
			}
			
			bufs[0] = uv_buf_init(op->out.data + op->out.offset, (unsigned int) (op->out.size - op->out.offset));
				
			uv_write(&op->req, stream->handle, bufs, 1, write_cb);
		}
		
		op->flags |= ASYNC_STREAM_WRITE_OP_FLAG_STARTED;
	}
	
	ASYNC_APPEND_OP(&stream->writes, op);
	
	op->stream = stream;
	op->req.data = op;
	
	if (req->in.flags & ASYNC_STREAM_WRITE_REQ_FLAG_ASYNC) {
		setup_async_write(op, req);
		
		return SUCCESS;
	}
	
	if (UNEXPECTED(await_op(stream, (async_op *) op) == FAILURE)) {
		req->out.error = UV_ECANCELED;
	
		if (op->flags & ASYNC_STREAM_WRITE_OP_FLAG_STARTED) {
			ASYNC_FORWARD_OP_ERROR(op);
			
			ASYNC_RESET_OP(op);
			ASYNC_PREPEND_OP(&stream->writes, op);
		
			req->in.context = async_context_get();

			setup_async_write(op, req);
			
			return FAILURE;
		}
		
		ASYNC_FORWARD_OP_ERROR(op);
		
		cleanup_write(op);
		
		return FAILURE;
	}
	
	if (UNEXPECTED(op->code < 0)) {
		req->out.error = op->code;
		
		return FAILURE;
	}
	
	return SUCCESS;
}

#ifdef HAVE_ASYNC_SSL

ASYNC_CALLBACK receive_handshake_bytes_cb(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
	async_stream *stream;
	async_ssl_op *op;
	
	stream = (async_stream *) handle->data;
	
	ZEND_ASSERT(stream != NULL);
	ZEND_ASSERT(stream->ssl.handshake != NULL);
	
	if (nread == 0) {
		return;
	}
	
	uv_read_stop(handle);
	
	op = stream->ssl.handshake;
	
	if (UNEXPECTED(nread < 0)) {
		op->uv_error = (int) nread;
	} else {
		op->ssl_error = process_input_bytes(stream, (int) nread);
	}
	
	ASYNC_FINISH_OP(op);
}

static int receive_handshake_bytes(async_stream *stream, async_ssl_handshake_data *data)
{
	async_ssl_op *op;
	
	int code;
	
	code = uv_read_start(stream->handle, read_alloc_cb, receive_handshake_bytes_cb);
	
	if (UNEXPECTED(code < 0)) {
		data->uv_error = code;
		
		return FAILURE;
	}
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_ssl_op));
	
	stream->ssl.handshake = op;
	
	if (UNEXPECTED(await_op(stream, (async_op *) op) == FAILURE)) {
		ASYNC_FREE_OP(op);
		
		return FAILURE;
	}
	
	if (UNEXPECTED(op->uv_error < 0)) {
		ASYNC_FREE_OP(op);
		
		data->uv_error = op->uv_error;
		
		return FAILURE;
	}
	
	code = op->ssl_error;

	ASYNC_FREE_OP(op);
	
	if (UNEXPECTED(code != SSL_ERROR_NONE)) {
		data->ssl_error = code;
		
		return FAILURE;
	}

	return SUCCESS;
}

ASYNC_CALLBACK send_handshake_bytes_cb(uv_write_t *req, int status)
{
	async_uv_op *op;
	
	op = (async_uv_op *) req->data;
	
	ZEND_ASSERT(op != NULL);
	
	op->code = status;
	
	ASYNC_FINISH_OP(op);
}

static int send_handshake_bytes(async_stream *stream, async_ssl_handshake_data *data)
{
	async_uv_op *op;

	uv_write_t req;
	uv_buf_t bufs[1];

	char *base;
	size_t len;
	int code;

	while ((len = BIO_ctrl_pending(stream->ssl.wbio)) > 0) {
		ERR_clear_error();
		
		base = emalloc(len);
		len = BIO_read(stream->ssl.wbio, base, (int) len);
		
		if (len < 1) {
			data->ssl_error = (int) ERR_get_error();
			
			return FAILURE;
		}
		
		bufs[0] = uv_buf_init(base, (unsigned int) len);
		
		while (bufs[0].len > 0) {
			code = uv_try_write(stream->handle, bufs, 1);
			
			if (code == UV_EAGAIN) {
				break;
			}
			
			if (UNEXPECTED(code < 0)) {
				efree(base);
				data->uv_error = code;
		
				return FAILURE;
			}
			
			bufs[0].base += code;
			bufs[0].len -= code;
		}
		
		if (bufs[0].len == 0) {
			efree(base);

			continue;
		}
		
		code = uv_write(&req, stream->handle, bufs, 1, send_handshake_bytes_cb);
		
		if (UNEXPECTED(code < 0)) {
			efree(base);
			data->uv_error = code;

			return FAILURE;
		}

		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op));

		req.data = op;

		if (UNEXPECTED(await_op(stream, (async_op *) op) == FAILURE)) {
			efree(base);
			ASYNC_FREE_OP(op);
			
			return FAILURE;
		}
		
		code = op->code;

		efree(base);
		ASYNC_FREE_OP(op);

		if (UNEXPECTED(code < 0)) {
			data->uv_error = code;
		
			return FAILURE;
		}
	}
	
	return SUCCESS;
}

int async_stream_ssl_handshake(async_stream *stream, async_ssl_handshake_data *handshake)
{
	X509 *cert;

	long result;
	int code;
	int error;
	
	ZEND_ASSERT(stream->ssl.ssl != NULL);
	ZEND_ASSERT(handshake->settings != NULL);

	if (UNEXPECTED(stream->buffer.base == NULL)) {
		init_buffer(stream);
	}

	if (stream->flags & ASYNC_STREAM_READING) {
		uv_read_stop(stream->handle);

		stream->flags &= ~ASYNC_STREAM_READING;
	}
	
	if (handshake->settings->mode == ASYNC_SSL_MODE_SERVER) {
		SSL_set_accept_state(stream->ssl.ssl);
	} else {
		SSL_set_connect_state(stream->ssl.ssl);
		
#ifdef ASYNC_TLS_SNI
		if (!handshake->settings->disable_sni) {
			if (handshake->settings->peer_name != NULL) {
				SSL_set_tlsext_host_name(stream->ssl.ssl, ZSTR_VAL(handshake->settings->peer_name));
			} else if (handshake->host != NULL) {
				SSL_set_tlsext_host_name(stream->ssl.ssl, ZSTR_VAL(handshake->host));
			}
		}
#endif
	}
	
	do {
		ERR_clear_error();
		
		code = SSL_do_handshake(stream->ssl.ssl);
		
		if (code == 1) {
			break;
		}
		
		error = SSL_get_error(stream->ssl.ssl, code);
		
		switch (error) {
		case SSL_ERROR_NONE:
		case SSL_ERROR_ZERO_RETURN:
		case SSL_ERROR_WANT_READ:
			if (BIO_ctrl_pending(stream->ssl.wbio) > 0) {
				if (SUCCESS != send_handshake_bytes(stream, handshake)) {
					return FAILURE;
				}
			}
	
			if (UNEXPECTED(SUCCESS != receive_handshake_bytes(stream, handshake))) {
				return FAILURE;
			}
			break;
		default:
			handshake->ssl_error = ERR_get_error();
			
			return FAILURE;
		}
	} while (1);
	
	// Send remaining bytes that mark handshake completion as needed.
	if (BIO_ctrl_pending(stream->ssl.wbio) > 0) {
		if (SUCCESS != send_handshake_bytes(stream, handshake)) {
			return FAILURE;
		}
	}
	
	// Feed remaining buffered input bytes into SSL engine to decrypt them.
	if (UNEXPECTED(SUCCESS != (code = process_input_bytes(stream, 0)))) {
		if (code != FAILURE) {
			handshake->ssl_error = code;
	
			return FAILURE;
		}
	}
	
	if (handshake->settings->mode == ASYNC_SSL_MODE_CLIENT) {
		if (UNEXPECTED(NULL == (cert = SSL_get_peer_certificate(stream->ssl.ssl)))) {
			ASYNC_STR(handshake->error, "Failed to access server SSL certificate");
			
			return FAILURE;
		}

		X509_free(cert);

		result = SSL_get_verify_result(stream->ssl.ssl);
		
		if (X509_V_OK != result && !(handshake->settings->allow_self_signed && result == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT)) {
			ASYNC_STRF(handshake->error, "Failed to verify server SSL certificate [%ld]: %s", result, X509_verify_cert_error_string(result));
			
			return FAILURE;
		}
	}
	
	return SUCCESS;
}

#endif


static PHP_METHOD(ReadableStream, close) { }
static PHP_METHOD(ReadableStream, read) { }

static const zend_function_entry async_readable_stream_functions[] = {
	PHP_ME(ReadableStream, close, arginfo_stream_close, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(ReadableStream, read, arginfo_readable_stream_read, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_FE_END
};


static PHP_METHOD(WritableStream, close) { }
static PHP_METHOD(WritableStream, write) { }

static const zend_function_entry async_writable_stream_functions[] = {
	PHP_ME(WritableStream, close, arginfo_stream_close, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(WritableStream, write, arginfo_writable_stream_write, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_FE_END
};


static PHP_METHOD(DuplexStream, getReadableStream) { }
static PHP_METHOD(DuplexStream, getWritableStream) { }

static const zend_function_entry async_duplex_stream_functions[] = {
	PHP_ME(DuplexStream, getReadableStream, arginfo_duplex_stream_get_readable_stream, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(DuplexStream, getWritableStream, arginfo_duplex_stream_get_writable_stream, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_FE_END
};


static zend_object *async_readable_memory_stream_object_create(zend_class_entry *ce)
{
	async_readable_memory_stream *stream;

	stream = ecalloc(1, sizeof(async_readable_memory_stream));

	zend_object_std_init(&stream->std, ce);
	stream->std.handlers = &async_readable_memory_stream_handlers;

	return &stream->std;
}

static void async_readable_memory_stream_object_destroy(zend_object *object)
{
	async_readable_memory_stream *stream;

	stream = (async_readable_memory_stream *) object;

	zend_string_release(stream->data);
	zval_ptr_dtor(&stream->error);

	zend_object_std_dtor(&stream->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_readable_memory_stream_ctor, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(ReadableMemoryStream, __construct)
{
	async_readable_memory_stream *stream;

	zend_string *str;

	str = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_STR(str)
	ZEND_PARSE_PARAMETERS_END();

	stream = (async_readable_memory_stream *) Z_OBJ_P(getThis());

	if (str == NULL) {
		stream->data = ZSTR_EMPTY_ALLOC();
	} else {
		stream->data = zend_string_copy(str);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readable_memory_stream_is_closed, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(ReadableMemoryStream, isClosed)
{
	async_readable_memory_stream *stream;

	ZEND_PARSE_PARAMETERS_NONE();

	stream = (async_readable_memory_stream *) Z_OBJ_P(getThis());

	RETURN_BOOL(stream->flags & ASYNC_READABLE_MEMORY_STREAM_FLAG_CLOSED);
}

static PHP_METHOD(ReadableMemoryStream, close)
{
	async_readable_memory_stream *stream;

	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(val, zend_ce_throwable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	stream = (async_readable_memory_stream *) Z_OBJ_P(getThis());

	if (stream->flags & ASYNC_READABLE_MEMORY_STREAM_FLAG_CLOSED) {
		return;
	}

	stream->flags |= ASYNC_READABLE_MEMORY_STREAM_FLAG_CLOSED;

	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		ZVAL_COPY(&stream->error, val);
	}
}

static PHP_METHOD(ReadableMemoryStream, read)
{
	async_readable_memory_stream *stream;
	zend_string *chunk;

	zval *hint;
	size_t len;

	hint = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(hint)
	ZEND_PARSE_PARAMETERS_END();

	if (hint == NULL || Z_TYPE_P(hint) == IS_NULL) {
		len = 8192;
	} else if (Z_LVAL_P(hint) < 1) {
		zend_throw_exception_ex(async_stream_exception_ce, 0, "Invalid read length: %d", (int) Z_LVAL_P(hint));
		return;
	} else {
		len = (size_t) Z_LVAL_P(hint);
	}

	stream = (async_readable_memory_stream *) Z_OBJ_P(getThis());

	if (stream->flags & ASYNC_READABLE_MEMORY_STREAM_FLAG_CLOSED) {
		zend_throw_exception(async_stream_closed_exception_ce, "Cannot read from closed stream", 0);

		if (Z_TYPE(stream->error) != IS_UNDEF) {
			zend_exception_set_previous(EG(exception), Z_OBJ(stream->error));
			GC_ADDREF(Z_OBJ(stream->error));
		}

		return;
	}

	len = MIN(len, ZSTR_LEN(stream->data) - stream->offset);

	if (len > 0) {
		chunk = zend_string_init(ZSTR_VAL(stream->data) + stream->offset, len, 0);
		stream->offset += len;

		RETVAL_STR(chunk);
	}
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(ReadableMemoryStream, async_readable_memory_stream_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_readable_memory_stream_functions[] = {
	PHP_ME(ReadableMemoryStream, __construct, arginfo_readable_memory_stream_ctor, ZEND_ACC_PUBLIC)
	PHP_ME(ReadableMemoryStream, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(ReadableMemoryStream, isClosed, arginfo_readable_memory_stream_is_closed, ZEND_ACC_PUBLIC)
	PHP_ME(ReadableMemoryStream, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	PHP_ME(ReadableMemoryStream, read, arginfo_readable_stream_read, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


static zend_object *async_writable_memory_stream_object_create(zend_class_entry *ce)
{
	async_writable_memory_stream *stream;

	stream = ecalloc(1, sizeof(async_writable_memory_stream));

	zend_object_std_init(&stream->std, ce);
	stream->std.handlers = &async_writable_memory_stream_handlers;

	return &stream->std;
}

static void async_writable_memory_stream_object_destroy(zend_object *object)
{
	async_writable_memory_stream *stream;

	stream = (async_writable_memory_stream *) object;

	smart_str_free(&stream->data);
	zval_ptr_dtor(&stream->error);

	zend_object_std_dtor(&stream->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_writable_memory_stream_is_closed, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(WritableMemoryStream, isClosed)
{
	async_writable_memory_stream *stream;

	ZEND_PARSE_PARAMETERS_NONE();

	stream = (async_writable_memory_stream *) Z_OBJ_P(getThis());

	RETURN_BOOL(stream->flags & ASYNC_WRITABLE_MEMORY_STREAM_FLAG_CLOSED);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_writable_memory_stream_get_contents, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(WritableMemoryStream, getContents)
{
	async_writable_memory_stream *stream;

	ZEND_PARSE_PARAMETERS_NONE();

	stream = (async_writable_memory_stream *) Z_OBJ_P(getThis());

	if (stream->data.s) {
		RETURN_STR(zend_string_dup(stream->data.s, 0));
	} else {
		RETURN_STR(ZSTR_EMPTY_ALLOC());
	}
}

static PHP_METHOD(WritableMemoryStream, close)
{
	async_writable_memory_stream *stream;

	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(val, zend_ce_throwable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	stream = (async_writable_memory_stream *) Z_OBJ_P(getThis());

	if (stream->flags & ASYNC_WRITABLE_MEMORY_STREAM_FLAG_CLOSED) {
		return;
	}

	stream->flags |= ASYNC_WRITABLE_MEMORY_STREAM_FLAG_CLOSED;

	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		ZVAL_COPY(&stream->error, val);
	}
}

static PHP_METHOD(WritableMemoryStream, write)
{
	async_writable_memory_stream *stream;

	zend_string *data;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(data)
	ZEND_PARSE_PARAMETERS_END();

	stream = (async_writable_memory_stream *) Z_OBJ_P(getThis());

	if (stream->flags & ASYNC_WRITABLE_MEMORY_STREAM_FLAG_CLOSED) {
		zend_throw_exception(async_stream_closed_exception_ce, "Cannot write to closed stream", 0);

		if (Z_TYPE(stream->error) != IS_UNDEF) {
			zend_exception_set_previous(EG(exception), Z_OBJ(stream->error));
			GC_ADDREF(Z_OBJ(stream->error));
		}

		return;
	}

	smart_str_append(&stream->data, data);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(WritableMemoryStream, async_writable_memory_stream_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_writable_memory_stream_functions[] = {
	PHP_ME(WritableMemoryStream, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(WritableMemoryStream, isClosed, arginfo_writable_memory_stream_is_closed, ZEND_ACC_PUBLIC)
	PHP_ME(WritableMemoryStream, getContents, arginfo_writable_memory_stream_get_contents, ZEND_ACC_PUBLIC)
	PHP_ME(WritableMemoryStream, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	PHP_ME(WritableMemoryStream, write, arginfo_writable_stream_write, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


async_stream_reader *async_stream_reader_create(async_stream *stream, zend_object *ref, zval *error)
{
	async_stream_reader *reader;

	reader = ecalloc(1, sizeof(async_stream_reader));

	zend_object_std_init(&reader->std, async_stream_reader_ce);
	reader->std.handlers = &async_stream_reader_handlers;

	reader->stream = stream;
	reader->ref = ref;
	reader->error = error;

	ASYNC_ADDREF(ref);

	return reader;
}

static void async_stream_reader_object_destroy(zend_object *object)
{
	async_stream_reader *reader;

	reader = (async_stream_reader *) object;

	ASYNC_DELREF(reader->ref);

	zend_object_std_dtor(&reader->std);
}

static PHP_METHOD(StreamReader, close)
{
	async_stream_reader *reader;

	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(val, zend_ce_throwable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	reader = (async_stream_reader *) Z_OBJ_P(getThis());

	if (Z_TYPE_P(reader->error) != IS_UNDEF) {
		return;
	}

	ASYNC_PREPARE_EXCEPTION(reader->error, execute_data, async_stream_closed_exception_ce, "Stream has been closed");

	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(reader->error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}

	async_stream_shutdown(reader->stream, ASYNC_STREAM_SHUT_RD, getThis());
}

static PHP_METHOD(StreamReader, read)
{
	async_stream_reader *reader;

	reader = (async_stream_reader *) Z_OBJ_P(getThis());

	async_stream_call_read(reader->stream, reader->error, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(StreamReader, async_stream_reader_ce)
ASYNC_METHOD_NO_WAKEUP(StreamReader, async_stream_reader_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_stream_reader_functions[] = {
	PHP_ME(StreamReader, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(StreamReader, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(StreamReader, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	PHP_ME(StreamReader, read, arginfo_readable_stream_read, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


async_stream_writer *async_stream_writer_create(async_stream *stream, zend_object *ref, zval *error)
{
	async_stream_writer *writer;

	writer = ecalloc(1, sizeof(async_stream_writer));

	zend_object_std_init(&writer->std, async_stream_writer_ce);
	writer->std.handlers = &async_stream_writer_handlers;

	writer->stream = stream;
	writer->ref = ref;
	writer->error = error;

	ASYNC_ADDREF(ref);

	return writer;
}

static void async_stream_writer_object_destroy(zend_object *object)
{
	async_stream_writer *writer;

	writer = (async_stream_writer *) object;

	ASYNC_DELREF(writer->ref);

	zend_object_std_dtor(&writer->std);
}

static PHP_METHOD(StreamWriter, close)
{
	async_stream_writer *writer;

	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(val, zend_ce_throwable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	writer = (async_stream_writer *) Z_OBJ_P(getThis());

	if (Z_TYPE_P(writer->error) != IS_UNDEF) {
		return;
	}

	// If use execute_data, In task call close dtor/destroy will not be called
	ASYNC_PREPARE_EXCEPTION(writer->error, NULL, async_stream_closed_exception_ce, "Stream has been closed");

	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(writer->error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}

	async_stream_shutdown(writer->stream, ASYNC_STREAM_SHUT_WR, getThis());
}

static PHP_METHOD(StreamWriter, write)
{
	async_stream_writer *writer;

	writer = (async_stream_writer *) Z_OBJ_P(getThis());

	async_stream_call_write(writer->stream, writer->error, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(StreamWriter, async_stream_writer_ce)
ASYNC_METHOD_NO_WAKEUP(StreamWriter, async_stream_writer_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_stream_writer_functions[] = {
	PHP_ME(StreamWriter, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(StreamWriter, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(StreamWriter, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	PHP_ME(StreamWriter, write, arginfo_writable_stream_write, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


static const zend_function_entry empty_funcs[] = {
	PHP_FE_END
};

void async_stream_ce_register()
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream", "ReadableStream", async_readable_stream_functions);
	async_readable_stream_ce = zend_register_internal_interface(&ce);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream", "WritableStream", async_writable_stream_functions);
	async_writable_stream_ce = zend_register_internal_interface(&ce);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream", "DuplexStream", async_duplex_stream_functions);
	async_duplex_stream_ce = zend_register_internal_interface(&ce);

	zend_class_implements(async_duplex_stream_ce, 2, async_readable_stream_ce, async_writable_stream_ce);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream", "StreamException", empty_funcs);
	async_stream_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_stream_exception_ce, zend_ce_exception);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream", "StreamClosedException", empty_funcs);
	async_stream_closed_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_stream_closed_exception_ce, async_stream_exception_ce);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream", "PendingReadException", empty_funcs);
	async_pending_read_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_pending_read_exception_ce, async_stream_exception_ce);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream", "ReadableMemoryStream", async_readable_memory_stream_functions);
	async_readable_memory_stream_ce = zend_register_internal_class(&ce);
	async_readable_memory_stream_ce->ce_flags |= ZEND_ACC_FINAL;
	async_readable_memory_stream_ce->create_object = async_readable_memory_stream_object_create;
	async_readable_memory_stream_ce->serialize = zend_class_serialize_deny;
	async_readable_memory_stream_ce->unserialize = zend_class_unserialize_deny;

	zend_class_implements(async_readable_memory_stream_ce, 1, async_readable_stream_ce);

	memcpy(&async_readable_memory_stream_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_readable_memory_stream_handlers.free_obj = async_readable_memory_stream_object_destroy;
	async_readable_memory_stream_handlers.clone_obj = NULL;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream", "WritableMemoryStream", async_writable_memory_stream_functions);
	async_writable_memory_stream_ce = zend_register_internal_class(&ce);
	async_writable_memory_stream_ce->ce_flags |= ZEND_ACC_FINAL;
	async_writable_memory_stream_ce->create_object = async_writable_memory_stream_object_create;
	async_writable_memory_stream_ce->serialize = zend_class_serialize_deny;
	async_writable_memory_stream_ce->unserialize = zend_class_unserialize_deny;

	zend_class_implements(async_writable_memory_stream_ce, 1, async_writable_stream_ce);

	memcpy(&async_writable_memory_stream_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_writable_memory_stream_handlers.free_obj = async_writable_memory_stream_object_destroy;
	async_writable_memory_stream_handlers.clone_obj = NULL;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream", "StreamReader", async_stream_reader_functions);
	async_stream_reader_ce = zend_register_internal_class(&ce);
	async_stream_reader_ce->ce_flags |= ZEND_ACC_FINAL;
	async_stream_reader_ce->serialize = zend_class_serialize_deny;
	async_stream_reader_ce->unserialize = zend_class_unserialize_deny;

	zend_class_implements(async_stream_reader_ce, 1, async_readable_stream_ce);

	memcpy(&async_stream_reader_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_stream_reader_handlers.free_obj = async_stream_reader_object_destroy;
	async_stream_reader_handlers.clone_obj = NULL;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Stream", "StreamWriter", async_stream_writer_functions);
	async_stream_writer_ce = zend_register_internal_class(&ce);
	async_stream_writer_ce->ce_flags |= ZEND_ACC_FINAL;
	async_stream_writer_ce->serialize = zend_class_serialize_deny;
	async_stream_writer_ce->unserialize = zend_class_unserialize_deny;

	zend_class_implements(async_stream_writer_ce, 1, async_writable_stream_ce);

	memcpy(&async_stream_writer_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_stream_writer_handlers.free_obj = async_stream_writer_object_destroy;
	async_stream_writer_handlers.clone_obj = NULL;
}
