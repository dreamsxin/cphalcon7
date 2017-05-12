
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
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
  |          Didier Bertrand <diblibre@gmail.com>                          |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_KERNEL_IO_CLIENT_H
#define PHALCON_KERNEL_IO_CLIENT_H

#include "kernel/io/definitions.h"

enum // socket operations for IOCP and tasks
{
	PHALCON_IO_OP_NONE,
	PHALCON_IO_OP_ACCEPT,
	PHALCON_IO_OP_READ,
	PHALCON_IO_OP_WRITE
};

enum // callback operation
{
	PHALCON_IO_CLIENT_CREATE,
	PHALCON_IO_CLIENT_DELETE,
	PHALCON_IO_CLIENT_READ,
	PHALCON_IO_CLIENT_WRITE,
	PHALCON_IO_CLIENT_DEFFERED_READ,
	PHALCON_IO_CLIENT_DEFFERED_WRITE
};

#define PHALCON_IO_CI_GET_DATA_HEAD(cb)       (cb->buffer + cb->head)
#define PHALCON_IO_CI_GET_DATA_MARK(cb)       (cb->buffer + cb->mark)
#define PHALCON_IO_CI_GET_DATA_TAIL(cb)       (cb->buffer + cb->tail)
#define PHALCON_IO_CI_GET_DATA_SIZE(cb)       (cb->tail - cb->head)
#define PHALCON_IO_CI_GET_FREE_SPACE(cb)      (cb->allocated - cb->tail)
#define PHALCON_IO_CI_GET_INCREASED_SIZE(cb)  (cb->tail - cb->mark)
#define PHALCON_IO_CI_GET_DECREASED_SIZE(cb)  (cb->head - cb->mark)
#define PHALCON_IO_CI_SLIDE_HEAD(cb,n)        (cb->head += n)
#define PHALCON_IO_CI_SLIDE_TAIL(cb,n)        (cb->tail += n)
#define PHALCON_IO_CI_SET_DATA_TAIL(cb,n)     (cb->tail = n)
#define PHALCON_IO_CI_CLEAR_BUFFER(cb)        (cb->head = cb->mark = cb->tail = 0)
#define PHALCON_IO_CI_SET_MARK_AT_HEAD(cb)    (cb->mark = cb->head)
#define PHALCON_IO_CI_SET_MARK_AT_TAIL(cb)    (cb->mark = cb->tail)

typedef struct _phalcon_io_client_info   phalcon_io_client_info;
typedef struct _phalcon_io_client_buffer phalcon_io_client_buffer;
typedef struct _phalcon_io_client_config phalcon_io_client_config;

struct _phalcon_io_client_buffer {
	char *buffer;	// 1 byte more than allocated
	int allocated;
	int limit;
	int head;
	int mark;		// previous tail after a write
	int tail;
};

// can be adjusted after
struct _phalcon_io_client_config {
	int use_write_events;		// PHALCON_IO_TRUE if checking write events. faster if we don't check write events (on kqueue)
	int read_buffer_size;		// default read buffer size
	int write_buffer_size;	// default write buffer size
	int read_buffer_limit;	// maximum read buffer size
	int write_buffer_limit;	// maximum write buffer size
};

// the use_ variables can be set during creation callback by application
struct _phalcon_io_client_info {
	void* tpi;					// pointer to the server
	phalcon_io_client_config *cc;		// reference to common clients config (tpi->cc)
	int use_write_events;		// if PHALCON_IO_FALSE, may be changed to PHALCON_IO_TRUE if write would block
	phalcon_io_socket_t socket;		// client socket
	struct in_addr remote_addr;	// client address
	int remote_port;			// client remote port
	struct in_addr local_addr;	// server address
	int local_port;				// server port
	int thread_id;				// reference to the thread number for debugging purpose
	phalcon_io_client_buffer* rb;		// read buffer
	phalcon_io_client_buffer* wb;		// write buffer
	phalcon_io_client_buffer* eb;		// external write buffer
	int use_eb;				// used internally by phalcon_io_do_write and phalcon_io_start_writing/phalcon_io_write_completed
	int read_end;				// PHALCON_IO_TRUE if no more data
	int error;				// PHALCON_IO_TRUE if I/O error
	int operation;			// IOCP and tasks operation
	void *overlapped;			// IOCP overlapped buffer
	int callback_has_written;	// I/O management. indication that data was written during callback
	int can_write;			// I/O management. if PHALCON_IO_FALSE, indicates a write started and waiting for readiness signal
	int write_pending;		// I/O management. indicates that a buffer is ready to be sent on write readiness
	phalcon_io_client_info* previous;
	phalcon_io_client_info* next;
	int step;					// set by user to organize read/write sequences
	void *context;				// user defined data
};

void phalcon_io_preset_client_config(void *tpi);
phalcon_io_client_info *phalcon_io_create_client(void *tpi, phalcon_io_socket_t socket);
phalcon_io_client_info *phalcon_io_delete_client(phalcon_io_client_info *ci, int want_callback);
phalcon_io_client_buffer *phalcon_io_create_client_buffer(int size, int limit);
phalcon_io_client_buffer *phalcon_io_delete_client_buffer();
int phalcon_io_trim_buffer(phalcon_io_client_buffer *cb);
char *phalcon_io_realloc_buffer(phalcon_io_client_buffer *cb);
char *phalcon_io_realloc_buffer_for(phalcon_io_client_buffer* cb, int size);
void phalcon_io_append_buffer(phalcon_io_client_buffer* cb, char* string);
char *phalcon_io_get_buffer_data(phalcon_io_client_buffer *cb);

void phalcon_io_init_clients(void *tpi);
void phalcon_io_clean_clients(void *tpi);

#endif /* PHALCON_KERNEL_IO_CLIENT_H */
