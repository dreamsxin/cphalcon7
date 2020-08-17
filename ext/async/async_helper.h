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

#ifndef ASYNC_HELPER_H
#define ASYNC_HELPER_H

#include <main/php_network.h>

void async_channel_ce_register();
void async_console_ce_register();
void async_context_ce_register();
void async_deferred_ce_register();
void async_dns_ce_register();
void async_event_ce_register();
void async_monitor_ce_register();
void async_pipe_ce_register();
void async_poll_ce_register();
void async_process_ce_register();
void async_signal_ce_register();
void async_socket_ce_register();
void async_ssl_ce_register();
void async_stream_ce_register();
void async_sync_ce_register();
void async_task_ce_register();
void async_tcp_ce_register();
void async_thread_ce_register();
void async_timer_ce_register();
void async_udp_socket_ce_register();

void async_channel_ce_unregister();
void async_deferred_ce_unregister();
void async_dns_ce_unregister();
void async_monitor_ce_unregister();
void async_ssl_ce_unregister();
void async_task_ce_unregister();
void async_tcp_ce_unregister();
void async_thread_ce_unregister();
void async_udp_socket_ce_unregister();

void async_context_init();
void async_dns_init();
void async_filesystem_init();
void async_helper_init();
void async_tcp_socket_init();
void async_task_scheduler_init();
void async_timer_init();
void async_udp_socket_init();
void async_unix_socket_init();

void async_context_shutdown();
void async_dns_shutdown();
void async_filesystem_shutdown();
void async_tcp_socket_shutdown();
void async_task_scheduler_shutdown();
void async_timer_shutdown();
void async_udp_socket_shutdown();
void async_unix_socket_shutdown();

char *async_status_label(zend_uchar status);

int async_get_poll_fd(zval *val, php_socket_t *sock, zend_string **error);

#if PHP_VERSION_ID < 80000
#define ASYNC_DEBUG_INFO_HANDLER(name) HashTable *name(zval *obj_, int *temp)
#define ASYNC_DEBUG_INFO_OBJ() Z_OBJ_P(obj_)
#else
#define ASYNC_DEBUG_INFO_HANDLER(name) HashTable *name(zend_object *object, int *temp)
#define ASYNC_DEBUG_INFO_OBJ() object
#endif

#if PHP_VERSION_ID >= 80000
zval *async_prop_write_handler_readonly(zend_object *object, zend_string *member, zval *value, void **cache_slot);
#elif PHP_VERSION_ID < 70400
void async_prop_write_handler_readonly(zval *object, zval *member, zval *value, void **cache_slot);
#else
zval *async_prop_write_handler_readonly(zval *object, zval *member, zval *value, void **cache_slot);
#endif

#endif
