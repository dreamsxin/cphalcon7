
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
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_SOCKET_H
#define PHALCON_SOCKET_H

#include "php_phalcon.h"

#include <main/php_network.h>

#ifdef PHALCON_USE_PHP_SOCKET
#include <ext/sockets/php_sockets.h>
#endif

#define PHALCON_SOCKET_AF_UNIX			AF_UNIX
#define PHALCON_SOCKET_AF_INET			AF_INET

#if HAVE_IPV6
#define PHALCON_SOCKET_AF_INET6			AF_INET6
#else
#define PHALCON_SOCKET_AF_INET6
#endif

#define PHALCON_SOCKET_SOCK_STREAM		SOCK_STREAM
#define PHALCON_SOCKET_SOCK_DGRAM		SOCK_DGRAM
#define PHALCON_SOCKET_SOCK_RAW			SOCK_RAW
#define PHALCON_SOCKET_SOCK_SEQPACKET	SOCK_SEQPACKET
#define PHALCON_SOCKET_SOCK_RDM			SOCK_RDM

#define PHALCON_SOCKET_SOL_TCP			IPPROTO_TCP
#define PHALCON_SOCKET_SOL_UDP			IPPROTO_UDP

extern zend_class_entry *phalcon_socket_ce;

PHALCON_INIT_CLASS(Phalcon_Socket);

#endif /* PHALCON_SOCKET_H */
