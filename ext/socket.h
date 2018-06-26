
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

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>		  /* for fcntl */

static inline int phalcon_socket_set_non_blocking(int fd) {
	int flags = fcntl(fd, F_GETFL);
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		return 0;
	}
	return 1;
}

#define PHALCON_SOCKET_AF_UNIX			AF_UNIX
#define PHALCON_SOCKET_AF_INET			AF_INET
#define PHALCON_SOCKET_AF_UNSPEC		AF_UNSPEC

#if HAVE_IPV6
#define PHALCON_SOCKET_AF_INET6			AF_INET6
#else
#define PHALCON_SOCKET_AF_INET6			10
#endif

#define PHALCON_SOCKET_SOCK_STREAM		SOCK_STREAM
#define PHALCON_SOCKET_SOCK_DGRAM		SOCK_DGRAM
#define PHALCON_SOCKET_SOCK_RAW			SOCK_RAW
#define PHALCON_SOCKET_SOCK_SEQPACKET	SOCK_SEQPACKET
#define PHALCON_SOCKET_SOCK_RDM			SOCK_RDM

#define PHALCON_SOCKET_SOL_SOCKET		SOL_SOCKET

#define PHALCON_SOCKET_IPPROTO_IP		IPPROTO_IP
#if HAVE_IPV6
#define PHALCON_SOCKET_IPPROTO_IPV6		IPPROTO_IPV6
#else
#define PHALCON_SOCKET_IPPROTO_IPV6		41
#endif

#define PHALCON_SOCKET_SOL_TCP			IPPROTO_TCP
#define PHALCON_SOCKET_SOL_UDP			IPPROTO_UDP

#define PHALCON_SOCKET_SO_DEBUG			SO_DEBUG
#define PHALCON_SOCKET_SO_REUSEADDR		SO_REUSEADDR
#ifdef SO_REUSEPORT
# define PHALCON_SOCKET_SO_REUSEPORT	SO_REUSEPORT
#else
# define PHALCON_SOCKET_SO_REUSEPORT	15
#endif

#define PHALCON_SOCKET_TCP_NODELAY		TCP_NODELAY

#ifdef TCP_QUICKACK
# define PHALCON_SOCKET_TCP_QUICKACK	TCP_QUICKACK
#endif
extern zend_class_entry *phalcon_socket_ce;

PHALCON_INIT_CLASS(Phalcon_Socket);

#endif /* PHALCON_SOCKET_H */
