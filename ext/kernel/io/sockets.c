
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

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>

#include "kernel/io/definitions.h"
#include "kernel/io/support.h"
#include "kernel/io/client.h"
#include "kernel/io/sockets.h"
#include "kernel/io/threads.h"
#include "kernel/io/epoll.h"
#include "kernel/io/kqueue.h"

int phalcon_io_init_sockets (void)
{
	return 0;
}

phalcon_io_socket_t phalcon_io_create_server_socket(char* address, int port) {
	phalcon_io_socket_t server_socket;
	if ((server_socket = phalcon_io_create_socket()) == PHALCON_IO_INVALID_SOCKET)
		return PHALCON_IO_ERROR;
	if (phalcon_io_bind_socket(server_socket, address, port) < 0) {
		server_socket = phalcon_io_close_socket(server_socket);
		return PHALCON_IO_ERROR;
	}
	if (phalcon_io_start_listening(server_socket) < 0) {
		server_socket = phalcon_io_close_socket(server_socket);
		return PHALCON_IO_ERROR;
	}
	return server_socket;
}

phalcon_io_socket_t phalcon_io_create_socket()
{
	phalcon_io_socket_t a_socket;
	int yes=1;

	if ((a_socket = socket(PF_INET, SOCK_STREAM, 0/*IPPROTO_TCP*/)) == PHALCON_IO_INVALID_SOCKET)
		return phalcon_io_error_message ("Error creating socket: %s\n", strerror (errno));

	// reuse local address. not required for iocp?
	if (setsockopt(a_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) < 0)
		phalcon_io_error_message ("Error setting SO_REUSEADDR for socket: %s\n", strerror (errno));
	return a_socket;
}

phalcon_io_socket_t phalcon_io_close_socket(phalcon_io_socket_t a_socket)
{
	// phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "phalcon_io_close_socket(%d)\n", a_socket);
	if (a_socket >= 0)
		close (a_socket);
	return PHALCON_IO_ERROR;
}

void phalcon_io_block_socket (phalcon_io_socket_t a_socket, int block)
{
	int flag = fcntl(a_socket, F_GETFL, 0);
	if (block)
		fcntl(a_socket, F_SETFL, flag & ~O_NONBLOCK);
	else
		fcntl(a_socket, F_SETFL, flag | O_NONBLOCK);
}

int phalcon_io_bind_socket (phalcon_io_socket_t server_socket, char* address, int port)
{
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof sin);
	sin.sin_family = AF_INET;
	if (address==NULL || *address=='\0')
		sin.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		sin.sin_addr.s_addr = inet_addr(address);
	sin.sin_port = htons(port);

	if (bind(server_socket, (struct sockaddr *) &sin, sizeof sin) == PHALCON_IO_SOCKET_ERROR)
		return phalcon_io_error_message ("Error binding socket: %s\n", strerror (errno));
	return 0;
}

int phalcon_io_start_listening (phalcon_io_socket_t server_socket)
{
	if (listen(server_socket, PHALCON_IO_LISTEN_QUEUE_SIZE) < 0)
		return phalcon_io_error_message ("Error listening to socket: %s\n", strerror (errno));
	return 0;
}


phalcon_io_client_info *phalcon_io_do_accept (void *tpi, phalcon_io_socket_t server_socket)
{
	phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "phalcon_io_do_accept\n");
	phalcon_io_threads_info *ti = (phalcon_io_threads_info *) tpi;
	struct sockaddr client_addr;
	phalcon_io_socklen_t client_addr_len = sizeof client_addr;

	// accept connection
	phalcon_io_socket_t client_socket = accept (server_socket, &client_addr, &client_addr_len);
	if (client_socket < 0) {
		if (!ti->stop)
			phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "Error accepting connection: %s\n", strerror (errno));
		return NULL;
	}

	if (ti->networks_info != NULL) {
		char address_string[30];
		if (!phalcon_io_validate_network (tpi, &client_addr, address_string)) {
			phalcon_io_error_message ("Forbidden user: %s\n", address_string);
			client_socket = phalcon_io_close_socket ( client_socket );
			return NULL;
		}
	}

	// set non blocking
	if (ti->clients_are_non_blocking)
		phalcon_io_block_socket (client_socket, PHALCON_IO_FALSE);

	// allocate client
	phalcon_io_client_info *ci = phalcon_io_create_client(tpi, client_socket);

	// keep adress and port port for reference (debug printing)
	struct sockaddr_in *client_addr_in;
	client_addr_in = (struct sockaddr_in *) &client_addr;
	memcpy (&ci->remote_addr, &client_addr_in->sin_addr, sizeof(struct in_addr));	// keep client address for reference
	ci->remote_port = ntohs (client_addr_in->sin_port);								// and remote port
	getsockname (client_socket, &client_addr, &client_addr_len);
	memcpy (&ci->local_addr, &client_addr_in->sin_addr, sizeof(struct in_addr));	// keep server address for reference
	ci->local_port = ntohs (client_addr_in->sin_port);								// and local port

	phalcon_io_do_callback (ci, PHALCON_IO_CLIENT_CREATE);

	phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "accept:: socket=%d, ci=%6x\n", client_socket, ci);
#if HAVE_EPOLL || HAVE_KQUEUE
	if (ti->clients_are_non_blocking)
		phalcon_io_add_client_to_poll(ci);
#endif
	return ci;
}

int phalcon_io_do_read (phalcon_io_client_info *ci)
{
	phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "phalcon_io_do_read\n");
	phalcon_io_threads_info *ti = (phalcon_io_threads_info *)ci->tpi;
	int size;

	phalcon_io_trim_buffer (ci->rb);
	PHALCON_IO_CI_SET_MARK_AT_TAIL (ci->rb);

	do {
		size = recv (ci->socket, PHALCON_IO_CI_GET_DATA_TAIL(ci->rb), PHALCON_IO_CI_GET_FREE_SPACE(ci->rb), 0);
		phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "recv::<< socket=%d, ci=%6x, n=%d, errno=%d\n", ci->socket, ci, size, errno);
		if (size < 0) {
			if (errno == EAGAIN) {
				phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "recv: EAGAIN\n");
				break;
			}
			if (!((phalcon_io_threads_info *)ci->tpi)->stop)
				phalcon_io_error_message("recv: %s\n", strerror (errno));
			ci->error = PHALCON_IO_TRUE;
			return 0;
		}
		else if (size == 0) {
			ci->read_end = PHALCON_IO_TRUE;
			return 0;
		}
		PHALCON_IO_CI_SLIDE_TAIL (ci->rb, size);
		*PHALCON_IO_CI_GET_DATA_TAIL(ci->rb) = '\0';		// recv don't terminate buffer with '\0'
		phalcon_io_realloc_buffer (ci->rb);
		phalcon_io_debug_message(PHALCON_IO_DEBUG_DETAIL, "recv::<< \"%s\"\n", PHALCON_IO_CI_GET_DATA_MARK(ci->rb));
	} while (ti->clients_are_non_blocking);

	phalcon_io_adjust_stats();

	return PHALCON_IO_CI_GET_DATA_SIZE(ci->rb);
}

int phalcon_io_do_write(phalcon_io_client_info *ci)
{
	phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "phalcon_io_do_write. can_write==%d\n", ci->can_write);
	if (!ci->can_write && !ci->write_pending) {
		errno = EAGAIN;
		return PHALCON_IO_ERROR;
	}

	ci->callback_has_written = PHALCON_IO_TRUE;
	int written = 0;
	phalcon_io_client_buffer *cb = ci->use_eb? ci->eb: ci->wb;
	PHALCON_IO_CI_SET_MARK_AT_HEAD (cb);

	while (PHALCON_IO_CI_GET_DATA_SIZE(cb) > 0) {
		phalcon_io_threads_info *ti = (phalcon_io_threads_info *)ci->tpi;
		if (ti->clients_are_non_blocking && !ci->use_write_events)
			phalcon_io_block_socket (ci->socket, PHALCON_IO_TRUE);				// tweak when output don't use events
		int size = send (ci->socket, PHALCON_IO_CI_GET_DATA_HEAD(cb), PHALCON_IO_CI_GET_DATA_SIZE(cb), 0);
		if (ti->clients_are_non_blocking && !ci->use_write_events)
			phalcon_io_block_socket (ci->socket, PHALCON_IO_FALSE);				// tweak when output don't use events
		phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "send::>> socket=%d, ci=%6x, n=%d, errno=%d\n", ci->socket, ci, size, errno);
		if (size < 0) {
			if (errno == EAGAIN) {
				phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "send: EAGAIN\n");
				if (ci->use_write_events) {
					phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "send: write pending\n");
					ci->can_write = PHALCON_IO_FALSE;
					ci->write_pending = PHALCON_IO_TRUE;
				}
				break;
			}
			if (!((phalcon_io_threads_info *)ci->tpi)->stop)
				phalcon_io_error_message("send: %d - %s\n", errno, strerror (errno));
			ci->error = PHALCON_IO_TRUE;
			return written;
		}
		PHALCON_IO_CI_SLIDE_HEAD (cb, size);
		written += size;
		phalcon_io_debug_message(PHALCON_IO_DEBUG_DETAIL, "send::>> \"%s\"\n", PHALCON_IO_CI_GET_DATA_MARK(cb));
		if (PHALCON_IO_CI_GET_DATA_SIZE(cb) > 0)
			phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "send: written=%d: head=%d, mark=%d, tail=%d\n",
					written, cb->head, cb->mark, cb->tail);
	}
	if (phalcon_io_trim_buffer(cb) == 0)
		ci->use_eb = PHALCON_IO_FALSE;
	if (PHALCON_IO_CI_GET_DATA_SIZE(cb) == 0)
		ci->write_pending = PHALCON_IO_FALSE;
	return written;
}

int phalcon_io_query_write (phalcon_io_client_info *ci)
{
	phalcon_io_debug_message(PHALCON_IO_DEBUG_IO, "phalcon_io_query_write. can_write==%d\n", ci->can_write);
	if (ci->can_write) {
			return phalcon_io_do_write(ci);
	}
	errno = EAGAIN;
	return PHALCON_IO_ERROR;
}

void phalcon_io_do_callback (phalcon_io_client_info *ci, int op)
{
	phalcon_io_threads_info *ti = (phalcon_io_threads_info *) ci->tpi;
	if (ci->step < 0)
		ci->step = 0;
	if (ti->callback) {
		ci->callback_has_written = PHALCON_IO_FALSE;
		(*ti->callback) (ci, op);
	}
	if (ci->callback_has_written)
		PHALCON_IO_CI_CLEAR_BUFFER (ci->rb);
}

void phalcon_io_get_hostname( char *hostname, int namesize )
{
	struct in_addr in;
	struct hostent *h;

	gethostname(hostname,namesize);
	h = gethostbyname(hostname);
	memcpy(&in,h->h_addr,4);
	h = gethostbyaddr((char *)&in,4,PF_INET);
	if (h!=NULL)
		strcpy(hostname,h->h_name);
}
