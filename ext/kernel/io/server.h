
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

#ifndef PHALCON_KERNEL_IO_SERVER_H
#define PHALCON_KERNEL_IO_SERVER_H

#include "kernel/io/client.h"
#include "kernel/io/tasks.h"

int phalcon_io_init_servers();
void *phalcon_io_create_server(void* parent, char* address, int port, char* networks, void (*callback)(phalcon_io_client_info* ci,int op), int worker_threads);
int phalcon_io_start_server(void* tpi);
void *phalcon_io_stop_server(void* tpi);
int phalcon_io_server_is_alive(void* tpi);
void phalcon_io_set_defaults(void* tpi, int use_write_events, int read_buffer_size, int write_buffer_size, int read_buffer_limit, int write_buffer_limit);

int phalcon_io_enqueue_message(phalcon_io_client_info* ci, int operation);
int phalcon_io_write_message(phalcon_io_client_info* ci, char *message);
int phalcon_io_write_internal_buffer(phalcon_io_client_info* ci);
int phalcon_io_write_external_buffer(phalcon_io_client_info* ci, char* buffer, int size);

#endif /* PHALCON_IO_SERVER_H */
