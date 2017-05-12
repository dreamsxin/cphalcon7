
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

#ifndef PHALCON_KERNEL_IO_DEFINITIONS_H
#define PHALCON_KERNEL_IO_DEFINITIONS_H

#include <arpa/inet.h>
#include <pthread.h>

#include "php_phalcon.h"

// constants
#define PHALCON_IO_LISTEN_QUEUE_SIZE 128		// limit on Darwin and Linux = 128
#define PHALCON_IO_EVENTS_QUEUE_SIZE 256		// initial size. will stay 128 on linux. Can grow on Darwin

#define PHALCON_IO_TP_THREADS 1
#define PHALCON_IO_TP_POLLER  2

#define PHALCON_IO_FALSE    0
#define PHALCON_IO_TRUE     1
#define PHALCON_IO_ERROR   -1
#define PHALCON_IO_DEFAULT -1

typedef socklen_t phalcon_io_socklen_t;
typedef int phalcon_io_socket_t;
typedef pthread_t phalcon_io_thread_t;
typedef void *phalcon_io_callback_t;

#define phalcon_io_mutex_t				pthread_mutex_t
#define phalcon_io_mutex_init(lock)		pthread_mutex_init(lock,NULL)
#define phalcon_io_mutex_destroy(lock)	pthread_mutex_destroy(lock)
#define phalcon_io_mutex_lock(lock)		pthread_mutex_lock (lock)
#define phalcon_io_mutex_unlock(lock)	pthread_mutex_unlock (lock)

#define PHALCON_IO_INVALID_SOCKET -1
#define PHALCON_IO_SOCKET_ERROR   -1

#if defined(__ARM__) || defined(__X86__)
# define atomic_add_uint  add_uint
# define atomic_add_ulong add_ulong
# define atomic_and_ulong and_ulong
#else
# define atomic_add_uint  __sync_fetch_and_add
# define atomic_add_ulong __sync_fetch_and_add
# define atomic_and_ulong __sync_fetch_and_and
#endif

#endif /* PHALCON_KERNEL_IO_DEFINITIONS_H */
