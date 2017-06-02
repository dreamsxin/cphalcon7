
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

#ifndef PHALCON_SERVER_CORE_H
#define PHALCON_SERVER_CORE_H

#include "php_phalcon.h"

#include <Zend/zend_smart_str.h>

#include "kernel/message/queue.h"

#include <stdlib.h>
#include <stdio.h>

#include <inttypes.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sched.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <pthread.h>

#if HAVE_EPOLL
#include <sys/epoll.h>
#endif

#ifndef PHALCON_USE_THREADPOOL
# define PHALCON_USE_THREADPOOL	1
#endif

#define PHALCON_SERVER_MAX_CONNS_PER_WORKER		8192
#define PHALCON_SERVER_MAX_BUFSIZE				2048

#define PHALCON_SERVER_EVENTS_PER_BATCH			64
#define PHALCON_SERVER_ACCEPT_PER_LISTEN_EVENT	1
#define PHALCON_SERVER_MAX_WORKER_THREADS		4

typedef struct phalcon_server_conn_context phalcon_server_conn_context_t;
typedef struct phalcon_server_context phalcon_server_context_t;
typedef struct phalcon_server_context_pool phalcon_server_context_pool_t;

struct pahlcon_server_socket_address {
    union {
        struct sockaddr_in inet_v4;
        struct sockaddr_in6 inet_v6;
        struct sockaddr_un un;
    } addr;
    socklen_t len;
};

struct phalcon_server_conn_context {
	int fd;
	int fd_added;
	int flags;
	int ep_fd;
	int cpu_id;
	void (*handler)(phalcon_server_context_t *, phalcon_server_conn_context_t *);
	int events;
	int data_len;
	int next_idx;
	char buf[PHALCON_SERVER_MAX_BUFSIZE];
	void *user_data;
	zend_string *response;
	phalcon_server_context_pool_t *pool;
} *arr;

struct phalcon_server_context_pool {
	int total;
	int allocated;
	int next_idx;
	struct phalcon_server_conn_context *arr;
};

struct phalcon_server_listen_addr {
	int param_port;
	struct in_addr listenip;
	char param_ip[32];
	int listen_fd;
};

#if PHALCON_USE_THREADPOOL
struct phalcon_server_worker_queue_op {
	enum {OP_BEGIN, OP_READ, OP_WRITE, OP_EXIT} type;
	phalcon_server_context_t *ctx;
	phalcon_server_conn_context_t *client_ctx;
	void (*handler)(phalcon_server_context_t *, phalcon_server_conn_context_t *);
};
#endif

struct phalcon_server_worker_data {
	pid_t process;
	uint64_t trancnt;
	uint64_t trancnt_prev;
	uint64_t acceptcnt;
	uint64_t acceptcnt_prev;
	int cpu_id;
	uint64_t polls_max;
	uint64_t polls_min;
	uint64_t polls_avg;
	uint64_t polls_cnt;
	uint64_t polls_sum;
	uint64_t polls_mpt;
	uint64_t polls_lst;
	uint64_t accept_cnt;
	uint64_t read_cnt;
	uint64_t write_cnt;
	int shutdown;
#if PHALCON_USE_THREADPOOL
	pthread_t main_thread;
	pthread_t worker_threads[PHALCON_SERVER_MAX_WORKER_THREADS];
	struct phalcon_message_queue worker_queue;
#endif
};

struct phalcon_server_context {
	int enable_verbose;
	int num_workers;
	int start_cpu;
	int la_num;
	int pfd;
	FILE *log_file;
	zend_string *log_path;
	int cpu_id;
	phalcon_server_context_pool_t *pool;
	struct phalcon_server_worker_data *wdata;
	struct phalcon_server_listen_addr la[32];
	void (*accept)(phalcon_server_context_t *, phalcon_server_conn_context_t *);
	void (*read)(phalcon_server_context_t *, phalcon_server_conn_context_t *);
	void (*write)(phalcon_server_context_t *, phalcon_server_conn_context_t *);
};

void phalcon_server_init_log(struct phalcon_server_context *ctx);
void phalcon_server_init_server(struct phalcon_server_context *ctx);
void phalcon_server_init_signal(struct phalcon_server_context *ctx);
void phalcon_server_init_timer(struct phalcon_server_context *ctx);
void phalcon_server_init_workers(struct phalcon_server_context *ctx);
void phalcon_server_do_stats(struct phalcon_server_context *ctx);
void phalcon_server_exit_cleanup(struct phalcon_server_context *ctx);
void phalcon_server_stop_workers(struct phalcon_server_context *ctx);

void phalcon_server_client_close(struct phalcon_server_conn_context *client_ctx);
struct phalcon_server_conn_context *phalcon_server_alloc_context(struct phalcon_server_context_pool *pool);
void phalcon_server_free_context(struct phalcon_server_conn_context *client_ctx);
struct phalcon_server_conn_context *phalcon_server_get_context(struct phalcon_server_context_pool *pool, int fd);

void phalcon_server_builtin_process_accept(struct phalcon_server_context *ctx, struct phalcon_server_conn_context * listen_ctx);

static inline int phalcon_server_get_cpu_num(){
	return sysconf(_SC_NPROCESSORS_ONLN);
}

#define phalcon_server_log_printf(ctx, fmt, args...) ({\
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) \
			printf("Worker[%lu] %s:%d\t" fmt, syscall(__NR_gettid),__FUNCTION__ , __LINE__, ## args); \
	})

#define PHALCON_SERVER_COPY_TO_STACK(a, b) \
	{ \
    	memcpy(a, b, sizeof(zval)); \
	}

#define PHALCON_SERVER_STRING_APPEND(dest, str) \
	{ \
		int old_len, len; \
		old_len = ZSTR_LEN(dest); len = ZSTR_LEN(str); \
		dest = zend_string_extend(dest, old_len + len + 1, 0); \
		memcpy(&(ZSTR_VAL(dest)[old_len]), ZSTR_VAL(str), len); \
		ZSTR_VAL(dest)[old_len + len] = '\0'; \
	}

#endif /* PHALCON_SERVER_CORE_H */
