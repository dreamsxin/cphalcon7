
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

#ifndef PHALCON_KERNEL_IO_SOCKET_H
#define PHALCON_KERNEL_IO_SOCKET_H

#include "kernel/io/definitions.h"
#include "kernel/io/client.h"

int phalcon_io_init_sockets(void);
phalcon_io_socket_t phalcon_io_create_server_socket(char* address, int port);
phalcon_io_socket_t phalcon_io_create_socket();
phalcon_io_socket_t phalcon_io_close_socket(phalcon_io_socket_t a_socket);
void phalcon_io_block_socket(phalcon_io_socket_t a_socket, int block);
int phalcon_io_bind_socket(phalcon_io_socket_t server_socket, char* address, int port);
int phalcon_io_start_listening(phalcon_io_socket_t server_socket);
phalcon_io_client_info *phalcon_io_do_accept(void *tpi, phalcon_io_socket_t server_socket);
int phalcon_io_do_read(phalcon_io_client_info *ci);
int phalcon_io_do_write(phalcon_io_client_info *ci);
int phalcon_io_query_write(phalcon_io_client_info *ci);
void phalcon_io_do_callback(phalcon_io_client_info *ci, int op);
void phalcon_io_get_hostname(char *hostname, int namesize);

#endif /* PHALCON_KERNEL_IO_SOCKET_H */
