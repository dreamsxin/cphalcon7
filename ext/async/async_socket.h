
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

#ifndef ASYNC_SOCKET_H
#define ASYNC_SOCKET_H

#include "kernel/backend.h"

static zend_always_inline int async_socket_addr_size(const struct sockaddr *addr)
{
#ifdef HAVE_IPV6
	if (addr->sa_family == AF_INET6) {
		return sizeof(struct sockaddr_in6);
	}
#endif
	
	return sizeof(struct sockaddr_in);
}

static zend_always_inline int async_socket_set_port(struct sockaddr *addr, zend_long port)
{
	if (addr->sa_family == AF_INET) {
		((struct sockaddr_in *) addr)->sin_port = htons((uint16_t) port);
		
		return SUCCESS;
	}
	
#ifdef HAVE_IPV6
	if (addr->sa_family == AF_INET) {
		((struct sockaddr_in6 *) addr)->sin6_port = htons((uint16_t) port);
		
		return SUCCESS;
	}
#endif

	return FAILURE;
}

static zend_always_inline int async_socket_get_peer(const struct sockaddr *addr, zend_string **ip, uint16_t *port)
{
	char buf[256];
		
	if (addr->sa_family == AF_INET) {
		uv_ip4_name((const struct sockaddr_in *) addr, buf, 256);
		
		if (EXPECTED(ip != NULL)) {
			*ip = zend_string_init(buf, strlen(buf), 0);
		}
		
		if (EXPECTED(port != NULL)) {
			*port = (uint16_t) ntohs(((struct sockaddr_in *) addr)->sin_port);
		}
		
		return SUCCESS;
	}
	
#ifdef HAVE_IPV6
	if (addr->sa_family == AF_INET6) {
		uv_ip6_name((const struct sockaddr_in6 *) addr, buf, 256);
		
		if (EXPECTED(ip != NULL)) {
			*ip = zend_string_init(buf, strlen(buf), 0);
		}
		
		if (EXPECTED(port != NULL)) {
			*port = (uint16_t) ntohs(((struct sockaddr_in6 *) addr)->sin6_port);
		}
		
		return SUCCESS;
	}
#endif

	if (EXPECTED(ip != NULL)) {
		*ip = NULL;
	}
	
	if (EXPECTED(port != NULL)) {
		*port = 0;
	}

	return FAILURE;
}

static zend_always_inline int async_socket_get_local_peer(php_socket_t sock, zend_string **ip, uint16_t *port)
{
	struct sockaddr *addr;
	socklen_t len;
	int code;
	
	addr = NULL;
	
	if (EXPECTED(0 == php_network_get_sock_name(sock, NULL, &addr, &len))) {
		code = async_socket_get_peer((const struct sockaddr *) addr, ip, port);			
		efree(addr);
		
		return code;
	}
	
	if (EXPECTED(ip != NULL)) {
		*ip = NULL;
	}
	
	if (EXPECTED(port != NULL)) {
		*port = 0;
	}
	
	return FAILURE;
}

static zend_always_inline int async_socket_get_remote_peer(php_socket_t sock, zend_string **ip, uint16_t *port)
{
	struct sockaddr *addr;
	socklen_t len;
	int code;
	
	addr = NULL;
	
	if (EXPECTED(0 == php_network_get_peer_name(sock, NULL, &addr, &len))) {
		code = async_socket_get_peer((const struct sockaddr *) addr, ip, port);			
		efree(addr);
		
		return code;
	}
	
	if (EXPECTED(ip != NULL)) {
		*ip = NULL;
	}
	
	if (EXPECTED(port != NULL)) {
		*port = 0;
	}
	
	return FAILURE;
}

static zend_always_inline int async_socket_parse_ip(const char *address, uint16_t port, php_sockaddr_storage *dest)
{
	if (0 == uv_ip4_addr(address, port, (struct sockaddr_in *) dest)) {
		return SUCCESS;
	}

#ifdef HAVE_IPV6
	if (0 == uv_ip6_addr(address, port, (struct sockaddr_in6 *) dest)) {
		return SUCCESS;
	}
#endif

	return FAILURE;
}

static zend_always_inline int async_socket_parse_ipv4(const char *address, uint16_t port, struct sockaddr_in *dest)
{
	return (0 == uv_ip4_addr(address, port, (struct sockaddr_in *) dest)) ? SUCCESS : FAILURE;
}

#ifdef HAVE_IPV6
static zend_always_inline int async_socket_parse_ipv6(const char *address, uint16_t port, struct sockaddr_in6 *dest)
{
	return (0 == uv_ip6_addr(address, port, (struct sockaddr_in6 *) dest)) ? SUCCESS : FAILURE;
}
#endif

// Socket

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_get_address, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_get_port, 0, 0, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_set_option, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_LONG, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

// SocketStream

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_get_remote_address, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_get_remote_port, 0, 0, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_flush, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_write_async, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_get_write_queue_size, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

// Server

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_server_accept, 0, 0, Phalcon\\Async\\Network\\SocketStream, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_get_address, 0, 0, IS_STRING, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_get_port, 0, 0, IS_LONG, NULL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_set_option, 0, 2, _IS_BOOL, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_LONG, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

// SocketStream

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_get_remote_address, 0, 0, IS_STRING, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_get_remote_port, 0, 0, IS_LONG, NULL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_flush, 0, 0, IS_VOID, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_write_async, 0, 1, IS_LONG, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_get_write_queue_size, 0, 0, IS_LONG, NULL, 0)
ZEND_END_ARG_INFO()

// Server

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_server_accept, 0, 0, IS_OBJECT, "Phalcon\\Async\\Network\\SocketStream", 0)
ZEND_END_ARG_INFO()
#endif

#endif
