
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

#ifndef PHALCON_KERNEL_IO_NETWORKS_H
#define PHALCON_KERNEL_IO_NETWORKS_H

#if !defined(__linux__) && !defined(__APPLE__)
# include <sys/socket.h>	// for generic platform
#endif

#include "kernel/io/definitions.h"

// network structure
typedef struct {
	struct in_addr mask;				// validity mask
	struct in_addr addr;
} phalcon_io_network_info;

void phalcon_io_assign_networks   ( void *tpi, char *networks_list );
int  phalcon_io_validate_network  ( void *tpi, struct sockaddr *client_address, char *address_string );
void phalcon_io_release_networks  ( void *tpi );

#endif /* PHALCON_KERNEL_IO_NETWORKS_H */
