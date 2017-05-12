
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

#ifndef PHALCON_KERNEL_IO_EPOLL_H
#define PHALCON_KERNEL_IO_EPOLL_H

#include "kernel/io/definitions.h"
#include "kernel/io/networks.h"
#include "kernel/io/client.h"

#if HAVE_EPOLL

#include <sys/epoll.h>

typedef struct epoll_event phalcon_io_poll_event;

typedef struct {
	void* parent;
	int info_type;
	int allow_tasks;
	int clients_are_non_blocking;
	phalcon_io_client_config cc;
	phalcon_io_socket_t server_socket;
	int actual_connections;
	phalcon_io_network_info *networks_info;
	int networks_size;
	void (*callback)(phalcon_io_client_info *ci, int can_defer);
	int stop;
	phalcon_io_client_info *first_client;
	phalcon_io_client_info *last_client;
	phalcon_io_mutex_t lock;
	// specific
	int poll_handler;
	phalcon_io_thread_t poll_thread;
	phalcon_io_client_info *accept_ci;
} phalcon_io_poller_info;

#define PHALCON_IO_OP(pev)				(0)
#define PHALCON_IO_EV(pev)				(pev->events)
#define PHALCON_IO_DATA(pev)			(pev->data.ptr)
#define PHALCON_IO_READ_EV(pev)			(pev->events & EPOLLIN)
#define PHALCON_IO_WRITE_EV(pev)		(pev->events & EPOLLOUT)
#define PHALCON_IO_IGNORE_EV(pev)		(0)
#define PHALCON_IO_TRUE_EOF_EV(pev)		(1)
#define PHALCON_IO_FALSE_EOF_EV(pev)	(0)

// common to epoll, kqueue and iocp
phalcon_io_poller_info* phalcon_io_create_poller(void* parent, char* address, int port, char *networks, void (*callback)(phalcon_io_client_info *ci,int op));
int phalcon_io_start_poller(phalcon_io_poller_info *pi);
void *phalcon_io_stop_poller(phalcon_io_poller_info *pi);
int phalcon_io_poller_is_alive(phalcon_io_poller_info *pi);
phalcon_io_callback_t phalcon_io_event_loop (void *tpi);

int  phalcon_io_create_poll();
int  phalcon_io_wait_event(phalcon_io_poller_info *pi, phalcon_io_poll_event *events_queue, int events_queue_size);
void phalcon_io_add_server_to_poll(phalcon_io_client_info *ci);
void phalcon_io_add_client_to_poll(phalcon_io_client_info *ci);
void phalcon_io_remove_client_from_poll(phalcon_io_client_info *ci);
void phalcon_io_change_poll(phalcon_io_client_info *ci, int operation, int events);
const char *phalcon_io_name_event(int event);
const char *phalcon_io_name_operation(int operation);

#endif /* HAVE_EPOLL */

#endif /* PHALCON_KERNEL_IO_EPOLL_H */
