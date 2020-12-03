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

#ifndef PHALCON_ASYNC_TCP_H
#define PHALCON_ASYNC_TCP_H

#include "async/core.h"
#include "async/async_ssl.h"
#include "async/async_stream.h"

#if PHALCON_USE_ASYNC

#define ASYNC_TCP_SERVER_FLAG_LAZY 1

typedef enum _phalcon_async_server_event_type
{
    ASYNC_SERVER_EVENT_ONCONNECT,
    ASYNC_SERVER_EVENT_ONREAD,
    ASYNC_SERVER_EVENT_ONREQUEST,
    ASYNC_SERVER_EVENT_ONWRITE,
    ASYNC_SERVER_EVENT_ONCLOSE,
} phalcon_async_server_event_type;

typedef struct _async_tcp_server {
	/* PHP object handle. */
	zend_object std;

	/* Task scheduler being used. */
	async_task_scheduler *scheduler;

	/* UV TCP handle. */
	uv_tcp_t handle;
	
	uint8_t flags;

	/* Hostname or IP address that was used to establish the connection. */
	zend_string *name;

	zend_string *addr;
	uint16_t port;

	/* Number of pending connection attempts queued in the backlog. */
	zend_uchar pending;

	/* Error being used to close the server. */
	zval error;
	
	/* Number of referenced accept operations. */
	zend_uchar ref_count;

	/* Queue of tasks waiting to accept a socket connection. */
	async_op_list accepts;
	
	async_cancel_cb cancel;

#ifdef HAVE_ASYNC_SSL
	/* TLS server encryption settings. */
	async_tls_server_encryption *encryption;
	
	async_ssl_settings settings;

	/* Server SSL context (shared between all socket connections). */
	SSL_CTX *ctx;
#endif
	uv_mutex_t jobmutex;
	uv_mutex_t writemutex;
	QUEUE writeQueue;
	uv_async_t async;

	zend_fcall_info_cache server_callbacks[ASYNC_SERVER_EVENT_ONCLOSE + 1];
} async_tcp_server;

typedef struct _async_tcp_socket {
	/* PHP object handle. */
	zend_object std;

	/* Task scheduler being used. */
	async_task_scheduler *scheduler;

	/* UV TCP handle. */
	uv_tcp_t handle;
	
	async_cancel_cb cancel;

	/* Hostname or IP address that was used to establish the connection. */
	zend_string *name;
	
	zend_string *local_addr;
	uint16_t local_port;
	
	zend_string *remote_addr;
	uint16_t remote_port;

	/* Refers to the (local) server that accepted the TCP socket connection. */
	async_tcp_server *server;
	
	async_stream *stream;

	/* Error being used to close the read stream. */
	zval read_error;

	/* Error being used to close the write stream. */
	zval write_error;

#ifdef HAVE_ASYNC_SSL
	/* TLS client encryption settings. */
	async_tls_client_encryption *encryption;
#endif
} async_tcp_socket;

int setup_server_tls(async_tcp_server *server, zval *tls);
async_tcp_socket *async_tcp_socket_object_create();

#endif /* PHALCON_USE_ASYNC */

#endif
