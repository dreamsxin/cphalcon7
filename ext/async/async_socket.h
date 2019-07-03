/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Martin Schröder <m.schroeder2007@gmail.com>                 |
  +----------------------------------------------------------------------+
*/

#ifndef ASYNC_SOCKET_H
#define ASYNC_SOCKET_H

#include "async/async_stream.h"
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
	char buf[64];
		
	if (addr->sa_family == AF_INET) {
		uv_ip4_name((const struct sockaddr_in *) addr, buf, sizeof(buf));
		
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
		uv_ip6_name((const struct sockaddr_in6 *) addr, buf, sizeof(buf));
		
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

static zend_always_inline int async_socket_set_reuseaddr(php_socket_t sock, int yes)
{
#ifdef PHP_WIN32
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &yes, sizeof(yes))) {
    	return uv_translate_sys_error(php_socket_errno());
    }
#else
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
    	return uv_translate_sys_error(php_socket_errno());
    }
#endif

	return 0;
}

static zend_always_inline int async_socket_set_reuseport(php_socket_t sock, int yes)
{
#ifndef PHP_WIN32
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes))) {
    	return uv_translate_sys_error(php_socket_errno());
    }
#endif

	return 0;
}

static zend_always_inline int async_socket_is_alive(async_stream *stream)
{
	php_socket_t sock;
	struct timeval tv;
	
	char buf;
	int error;
	
#ifdef PHP_WIN32
	int code;
#else
	ssize_t code;
#endif

	if (UNEXPECTED(stream->flags & ASYNC_STREAM_CLOSED)) {
		return 0;
	}
	
	if (UNEXPECTED(stream->buffer.len > 0)) {
		return 1;
	}
	
#ifdef HAVE_ASYNC_SSL
	if (stream->ssl.ssl != NULL) {
		if (UNEXPECTED(stream->flags & ASYNC_STREAM_SSL_FATAL || SSL_get_shutdown(stream->ssl.ssl) & SSL_RECEIVED_SHUTDOWN)) {
			return 0;
		}
	}
#endif
	
	if (UNEXPECTED(0 != uv_fileno((const uv_handle_t *) stream->handle, (uv_os_fd_t *) &sock))) {
		return 0;
	}
	
	if (UNEXPECTED(sock == -1)) {
		return 0;
	}
	
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	if (php_pollfd_for(sock, PHP_POLLREADABLE | POLLPRI, &tv) < 1) {
		return 1;
	}
	
	code = recv(sock, &buf, sizeof(buf), MSG_PEEK);
	error = php_socket_errno();
	
	if (code > 0) {
		return 1;
	}
	
	return (code < 0 && (error == EWOULDBLOCK || error == EAGAIN || error == EMSGSIZE)) ? 1 : 0;
}

// Socket

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_get_address, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_get_port, 0, 0, IS_LONG, 1)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_set_option, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_LONG, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

// SocketStream

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_is_alive, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_get_remote_address, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_get_remote_port, 0, 0, IS_LONG, 1)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_stream_flush, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_socket_get_write_queue_size, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO();

// Server

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_server_accept, 0, 0, Phalcon\\Async\\Network\\SocketStream, 0)
ZEND_END_ARG_INFO();

#endif
