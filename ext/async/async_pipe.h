
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

#ifndef ASYNC_PIPE_H
#define ASYNC_PIPE_H

#define ASYNC_PIPE_FLAG_IPC 1
#define ASYNC_PIPE_FLAG_LAZY (1 << 1)

typedef struct {
	/* PHP object handle. */
	zend_object std;
	
	/* Task scheduler being used. */
	async_task_scheduler *scheduler;

	/* UV pipe handle. */
	uv_pipe_t handle;

	uint8_t flags;

	/* Hostname or IP address that was used to establish the connection. */
	zend_string *name;

	/* Number of pending connection attempts queued in the backlog. */
	zend_uchar pending;

	/* Error being used to close the server. */
	zval error;
	
	/* Number of referenced accept operations. */
	zend_uchar ref_count;

	/* Queue of tasks waiting to accept a socket connection. */
	async_op_list accepts;
	
	async_cancel_cb cancel;
} async_pipe_server;

typedef struct {
	/* PHP object handle. */
	zend_object std;

	/* Task scheduler being used. */
	async_task_scheduler *scheduler;

	/* UV pipe handle. */
	uv_pipe_t handle;
	
	uint8_t flags;
	
	async_cancel_cb cancel;

	/* Hostname or IP address that was used to establish the connection. */
	zend_string *name;

	/* Refers to the (local) server that accepted the pipe connection. */
	async_pipe_server *server;
	
	async_stream *stream;

	/* Error being used to close the read stream. */
	zval read_error;

	/* Error being used to close the write stream. */
	zval write_error;
} async_pipe;

async_pipe *async_pipe_init_ipc();

void async_pipe_import_stream(async_pipe *pipe, uv_stream_t *handle);
void async_pipe_export_stream(async_pipe *pipe, uv_stream_t *handle);

#endif
