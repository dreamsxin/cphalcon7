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

#ifndef ASYNC_EVENT_H
#define ASYNC_EVENT_H

#define ASYNC_TICK_EVENT_FLAG_CLOSED 1
#define ASYNC_TICK_EVENT_FLAG_REFERENCED (1 << 1)

struct _async_tick_event {
	zend_object std;

	async_task_scheduler *scheduler;
	async_context *context;
	zend_fcall_info_cache fcc;

	zval callback;
	uint8_t flags;

	async_tick_event *prev;
	async_tick_event *next;
	async_tick_list *list;
};

#define ASYNC_POLL_EVENT_FLAG_READABLE UV_READABLE
#define ASYNC_POLL_EVENT_FLAG_WRITABLE UV_WRITABLE
#define ASYNC_POLL_EVENT_FLAG_DISCONNECT UV_DISCONNECT
#define ASYNC_POLL_EVENT_FLAG_CLOSED (1 << 6)
#define ASYNC_POLL_EVENT_FLAG_SCHEDULED (1 << 7)

#define ASYNC_POLL_EVENT_MASK (ASYNC_POLL_EVENT_FLAG_READABLE | ASYNC_POLL_EVENT_FLAG_WRITABLE)

typedef struct _async_poll_event {
	zend_object std;

	async_task_scheduler *scheduler;
	async_context *context;
	zend_fcall_info_cache fcc;

	zval callback;
	zval stream;
	uv_poll_t handle;
	uint8_t flags;

	async_cancel_cb shutdown;
} async_poll_event;

#define ASYNC_TIMER_EVENT_FLAG_PERIODIC 1
#define ASYNC_TIMER_EVENT_FLAG_CLOSED (1 << 1)
#define ASYNC_TIMER_EVENT_FLAG_SCHEDULED (1 << 2)

typedef struct _async_timer_event {
	zend_object std;

	async_task_scheduler *scheduler;
	async_context *context;
	zend_fcall_info_cache fcc;

	zval callback;
	uv_timer_t handle;
	uint64_t delay;
	uint8_t flags;

	async_cancel_cb shutdown;
} async_timer_event;

async_poll_event *async_poll_event_object_create(async_task_scheduler *scheduler, zval *callback, zval *val, php_socket_t fd);
async_tick_event *async_tick_event_object_create(async_task_scheduler *scheduler, zval *callback);
async_timer_event *async_timer_event_object_create(async_task_scheduler *scheduler, zval *callback);

#endif
