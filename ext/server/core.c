
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

#include "server/core.h"

#include <sys/select.h>

void phalcon_server_init_log(struct phalcon_server_context *ctx)
{
	ctx->pfd = dup(STDERR_FILENO);
	if (unlikely(PHALCON_GLOBAL(debug).enable_debug) && ctx->log_path && ctx->log_path->len > 0) {
		ctx->log_file = fopen(ctx->log_path->val, "a");
		if (!ctx->log_file) {
			perror("Open log file failed");
			phalcon_server_exit_cleanup(ctx);
		}

		printf("Open log file %s\n\n", ctx->log_path->val);

		dup2(fileno(ctx->log_file), STDOUT_FILENO);
		dup2(fileno(ctx->log_file), STDERR_FILENO);

		phalcon_server_log_printf(ctx, "Log starts\n");
	}
}

int phalcon_server_bind_process_cpu(int cpu)
{
	cpu_set_t cmask;
	size_t n;
	int ret;

	n = phalcon_server_get_cpu_num();

	if (cpu < 0 || cpu >= (int)n) {
		errno = EINVAL;
		return -1;
	}

	CPU_ZERO(&cmask);
	CPU_SET(cpu, &cmask);

	ret = sched_setaffinity(0, n, &cmask);

	CPU_ZERO(&cmask);

	return ret;
}

void phalcon_server_init_signal(struct phalcon_server_context *ctx)
{
	sigset_t siglist;

	if(sigemptyset(&siglist) == -1) {
		perror("Unable to initialize signal list");
		phalcon_server_exit_cleanup(ctx);
	}

	if(sigaddset(&siglist, SIGALRM) == -1) {
		perror("Unable to add SIGALRM signal to signal list");
		phalcon_server_exit_cleanup(ctx);
	}

	if(sigaddset(&siglist, SIGINT) == -1) {
		perror("Unable to add SIGINT signal to signal list");
		phalcon_server_exit_cleanup(ctx);
	}

	if(pthread_sigmask(SIG_BLOCK, &siglist, NULL) != 0) {
		perror("Unable to change signal mask");
		phalcon_server_exit_cleanup(ctx);
	}
}

void phalcon_server_init_timer(struct phalcon_server_context *ctx)
{
	struct itimerval interval;

	interval.it_interval.tv_sec = 1;
	interval.it_interval.tv_usec = 0;
	interval.it_value.tv_sec = 1;
	interval.it_value.tv_usec = 0;

	if(setitimer(ITIMER_REAL, &interval, NULL) != 0) {
		perror("Unable to set interval timer");
		phalcon_server_exit_cleanup(ctx);
	}
}

struct phalcon_server_context_pool *phalcon_server_init_pool(int size)
{
	struct phalcon_server_context_pool *ret;
	int i;

	assert(size > 0);

	ret = calloc(1, sizeof(struct phalcon_server_context_pool));
	assert(ret);

	ret->arr = calloc(1, sizeof(struct phalcon_server_conn_context) * size);
	assert(ret->arr);

	ret->total = size;
	ret->allocated = 0;
	ret->next_idx = 0;

	for (i = 0; i < size - 1; i++)
		ret->arr[i].next_idx = i + 1;

	ret->arr[size - 1].next_idx = -1;

	return ret;
}

void phalcon_server_free_pool(struct phalcon_server_context_pool *pool)
{
	if (pool) {
		if (pool->arr) {
			free(pool->arr);
		}
		free(pool);
	}
}

struct phalcon_server_conn_context *phalcon_server_alloc_context(struct phalcon_server_context_pool *pool)
{
	struct phalcon_server_conn_context *ret;

	assert(pool->allocated < pool->total);
	pool->allocated++; // __sync_fetch_and_add(pool->allocated, 1);

	ret = &pool->arr[pool->next_idx];
	pool->next_idx = pool->arr[pool->next_idx].next_idx;

	ret->fd = 0;
	ret->fd_added = 0;
	ret->next_idx = -1;

	ret->pool = pool;

	return ret;
}

void phalcon_server_free_context(struct phalcon_server_conn_context *context)
{
	struct phalcon_server_context_pool *pool = context->pool;

	assert(pool->allocated > 0);
	pool->allocated--; // __sync_fetch_and_sub(pool->allocated, 1);

	context->next_idx = pool->next_idx;
	pool->next_idx = context - pool->arr;
}

struct phalcon_server_conn_context *phalcon_server_get_context(struct phalcon_server_context_pool *pool, int fd)
{
	struct phalcon_server_conn_context *ret = NULL;
	int i;

	assert(pool->allocated > 0);
	for (i = 0; i < pool->allocated; i++) {
		ret = &pool->arr[i];
		if (ret->fd == fd) {
			break;
		}
	}

	return ret;
}

int phalcon_server_init_single_server(struct phalcon_server_context *ctx, struct in_addr ip, uint16_t port)
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int serverfd, flags, value;

	if((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Unable to open socket");
		phalcon_server_exit_cleanup(ctx);
	}

	flags = fcntl(serverfd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(serverfd, F_SETFL, flags);

	value = 1;
	if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) == -1) {
		perror("Unable to set socket reuseaddr option");
		phalcon_server_exit_cleanup(ctx);
	}

	memset(&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = ip;

	if(bind(serverfd, (struct sockaddr *)&addr, addrlen) == -1) {
		perror("Unable to bind socket");
		phalcon_server_exit_cleanup(ctx);
	}

	if(listen(serverfd, 8192) != 0) {
		perror("Cannot listen for client connections");
		phalcon_server_exit_cleanup(ctx);
	}

	return serverfd;
}

void phalcon_server_init_server(struct phalcon_server_context *ctx)
{
	int ret, i;
	struct rlimit limits;

	assert(ctx->num_workers >= 1 && ctx->num_workers <= phalcon_server_get_cpu_num());
	assert(ctx->start_cpu >= 0 && ctx->start_cpu < phalcon_server_get_cpu_num());
	assert(ctx->start_cpu <= phalcon_server_get_cpu_num() - ctx->start_cpu);

	for (i = 0; i < ctx->la_num; i++){
		struct in_addr ip;
		uint16_t port;

		ip = ctx->la[i].listenip;
		port = ctx->la[i].param_port;

		ctx->la[i].listen_fd = phalcon_server_init_single_server(ctx, ip, port);
	}

	limits.rlim_cur = RLIM_INFINITY;
	limits.rlim_max = RLIM_INFINITY;

	ret = setrlimit(RLIMIT_CORE, &limits);
	if (ret < 0) {
		perror("Set core limit failed");
		phalcon_server_exit_cleanup(ctx);
	}

	getrlimit(RLIMIT_CORE, &limits);
	phalcon_server_log_printf(ctx, "Core limit %ld %ld\n", limits.rlim_cur, limits.rlim_max);

	limits.rlim_cur = PHALCON_SERVER_MAX_CONNS_PER_WORKER;
	limits.rlim_max = PHALCON_SERVER_MAX_CONNS_PER_WORKER;

	ret = setrlimit(RLIMIT_NOFILE, &limits);
	if (ret < 0) {
		perror("Set open file limit failed");
		phalcon_server_exit_cleanup(ctx);
	}

	getrlimit(RLIMIT_NOFILE, &limits);
	phalcon_server_log_printf(ctx, "Open file limit %ld %ld\n", limits.rlim_cur, limits.rlim_max);
}

#if PHALCON_USE_THREADPOOL
/* Thread start */
static void phalcon_server_worker_thread_process(void *arg) {
	struct phalcon_server_worker_data *data = (struct phalcon_server_worker_data *)arg;

	while(likely(!data->shutdown)) {
		struct phalcon_server_worker_queue_op *op = phalcon_message_queue_read(&data->worker_queue);

		phalcon_server_log_printf(op->ctx, "Thread op %d, cpu %d\n", op->type, op->client_ctx->cpu_id);
		switch(op->type) {
		case OP_EXIT:
			phalcon_message_queue_message_free(&data->worker_queue, op);
			return;
		default:
			op->handler(op->ctx, op->client_ctx);
			break;
		}
		phalcon_message_queue_message_free(&data->worker_queue, op);
	}
}

static void phalcon_server_worker_threadpool_init(struct phalcon_server_context *ctx, struct phalcon_server_worker_data *data) {
	int i;
	data->main_thread = pthread_self();
	phalcon_message_queue_init(&data->worker_queue, sizeof(struct phalcon_server_worker_queue_op), 512);
	for(i=0;i<PHALCON_SERVER_MAX_WORKER_THREADS;++i) {
		phalcon_server_log_printf(ctx, "Thread create %d, cpu %d\n", i, data->cpu_id);
		pthread_create(&data->worker_threads[i], NULL, (void *)&phalcon_server_worker_thread_process, (void *)data);
	}
}

static void phalcon_server_worker_threadpool_destroy(struct phalcon_server_context *ctx, struct phalcon_server_worker_data *data) {
	int i;
	for(i=0;i<PHALCON_SERVER_MAX_WORKER_THREADS;++i) {
		phalcon_server_log_printf(ctx, "Thread destroy %d, cpu %d\n", i, data->cpu_id);
		struct phalcon_server_worker_queue_op *op = phalcon_message_queue_message_alloc_blocking(&data->worker_queue);
		phalcon_server_log_printf(ctx, "Thread destroy %d, cpu %d\n", i, data->cpu_id);
		op->type = OP_EXIT;
		phalcon_message_queue_write(&data->worker_queue, op);
		phalcon_server_log_printf(ctx, "Thread destroy %d, cpu %d\n", i, data->cpu_id);
	}

}
#endif

/* Thread end */

void phalcon_server_process_clients(struct phalcon_server_context *ctx, void *arg)
{
	struct phalcon_server_worker_data *mydata = (struct phalcon_server_worker_data *)arg;

	struct epoll_event evt;
	struct epoll_event evts[PHALCON_SERVER_EVENTS_PER_BATCH];
#if PHALCON_USE_THREADPOOL
	fd_set listen_fds;
#endif
	int ret;

	int cpu_id = mydata->cpu_id;;
	int ep_fd;
	int i;

	struct phalcon_server_conn_context *listen_ctx;

	ret = phalcon_server_bind_process_cpu(cpu_id);
	if (ret < 0) {
		perror("Unable to Bind worker on CPU");
		phalcon_server_exit_cleanup(ctx);
	}

	ctx->pool = phalcon_server_init_pool(PHALCON_SERVER_MAX_CONNS_PER_WORKER);

	if ((ep_fd = epoll_create(PHALCON_SERVER_MAX_CONNS_PER_WORKER)) < 0) {
		perror("Unable to create epoll FD");
		phalcon_server_exit_cleanup(ctx);
	}

	for (i = 0; i < ctx->la_num; i++) {
		listen_ctx = phalcon_server_alloc_context(ctx->pool);

		listen_ctx->fd = ctx->la[i].listen_fd;
		listen_ctx->handler = ctx->accept ? ctx->accept : phalcon_server_builtin_process_accept;
		listen_ctx->cpu_id = cpu_id;
		listen_ctx->ep_fd = ep_fd;

		evt.events = EPOLLIN | EPOLLHUP | EPOLLERR;
		evt.data.ptr = listen_ctx;

		if (epoll_ctl(listen_ctx->ep_fd, EPOLL_CTL_ADD, listen_ctx->fd, &evt) < 0) {
			perror("Unable to add Listen Socket to epoll");
			phalcon_server_exit_cleanup(ctx);
		}
#if PHALCON_USE_THREADPOOL
		FD_SET(listen_ctx->fd, &listen_fds);
#endif
	}
#if PHALCON_USE_THREADPOOL
	phalcon_server_worker_threadpool_init(ctx, mydata);
#endif
	mydata->polls_min = PHALCON_SERVER_EVENTS_PER_BATCH;

	while (likely(!mydata->shutdown)) {
		int num_events;
		int i;
		int events;

		num_events = epoll_wait(ep_fd, evts, PHALCON_SERVER_EVENTS_PER_BATCH, -1);
		if (num_events < 0) {
			if (errno == EINTR)
				continue;
			perror("epoll_wait() error");
		}
		if (!num_events)
			mydata->polls_mpt++;
		else if (num_events < mydata->polls_min)
			ctx->wdata[cpu_id].polls_min = num_events;
		if (num_events > mydata->polls_max)
			mydata->polls_max = num_events;

		mydata->polls_sum += num_events;
		mydata->polls_cnt++;
		mydata->polls_avg = ctx->wdata[cpu_id].polls_sum / ctx->wdata[cpu_id].polls_cnt;
		mydata->polls_lst = num_events;

		for (i = 0 ; i < num_events; i++) {
			int active_fd;

			events = evts[i].events;
			listen_ctx = evts[i].data.ptr;
			listen_ctx->events = events;

			active_fd = listen_ctx->fd;

			phalcon_server_log_printf(ctx, "%dth event[0x%x] at fd %d\n", i, events, active_fd);
#if PHALCON_USE_THREADPOOL
			if(events & EPOLLOUT || FD_ISSET(active_fd, &listen_fds)) {
				listen_ctx->handler(ctx, listen_ctx);
			} else {
				phalcon_server_log_printf(ctx, "Message queue write cpu %d\n", cpu_id);
				struct phalcon_server_worker_queue_op *op = phalcon_message_queue_message_alloc_blocking(&mydata->worker_queue);
				op->type = OP_READ;
				op->ctx = ctx;
				op->client_ctx = listen_ctx;
				op->handler = listen_ctx->handler;
				phalcon_message_queue_write(&mydata->worker_queue, op);
			}
#else
			listen_ctx->handler(ctx, listen_ctx);
#endif
		}
	}
#if PHALCON_USE_THREADPOOL
	phalcon_server_worker_threadpool_destroy(ctx, mydata);
#endif
}

void phalcon_server_init_workers(struct phalcon_server_context *ctx)
{
	int i, pid;

	ctx->wdata = mmap(NULL, ctx->num_workers * sizeof(struct phalcon_server_worker_data),
		     PROT_READ|PROT_WRITE,
		     MAP_ANON|MAP_SHARED,
		     -1, 0);

	memset(ctx->wdata, 0, ctx->num_workers * sizeof(struct phalcon_server_worker_data));

	if (ctx->wdata == NULL) {
		perror("Unable to mmap shared global wdata");
		phalcon_server_exit_cleanup(ctx);
	}

	for(i = 0; i < ctx->num_workers; i++) {
		ctx->wdata[i].trancnt = 0;
		ctx->wdata[i].cpu_id = i + ctx->start_cpu;

		if ( (pid = fork()) < 0) {
			perror("Unable to fork child process");
			phalcon_server_exit_cleanup(ctx);
		} else if( pid == 0) {
			ctx->wdata[i].process = getpid();
			ctx->cpu_id = ctx->wdata[i].cpu_id ;
			phalcon_server_process_clients(ctx, (void *)&(ctx->wdata[i]));
			exit(0);
		}
	}
}

void phalcon_server_client_close(struct phalcon_server_conn_context *ctx)
{
	int fd, ep_fd, ret;
	struct epoll_event evt;

	ep_fd = ctx->ep_fd;
	fd = ctx->fd;

	evt.events = EPOLLHUP | EPOLLERR;
	evt.data.ptr = ctx;


	if (ctx->fd_added) {
		ret = epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, &evt);
		if (ret < 0)
			perror("Unable to delete client socket from epoll");
	}
	close(fd);
}

void phalcon_server_builtin_process_accept(struct phalcon_server_context *ctx, struct phalcon_server_conn_context * listen_ctx)
{
	int client_fd, listen_fd;
	int events = listen_ctx->events;
	struct epoll_event evt;

	struct phalcon_server_conn_context *client_ctx;

	int cpu_id = listen_ctx->cpu_id;
	int ret = 0;
	int i;

	listen_fd = listen_ctx->fd;

	//TODO: What else should I do.
	if (events & (EPOLLHUP | EPOLLERR))
		return;

	for (i = 0; i < PHALCON_SERVER_ACCEPT_PER_LISTEN_EVENT; i++) {
		struct pahlcon_server_socket_address client_addr;
    	socklen_t client_addrlen = sizeof(client_addr);
#ifdef HAVE_ACCEPT4
        client_fd = accept4(listen_fd, (struct sockaddr *) &client_addr, &client_addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
		int flags;
        client_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addrlen);
#endif
		if (client_fd < 0) {
			ctx->wdata[cpu_id].accept_cnt++;
			goto back;
		}

#ifndef HAVE_ACCEPT4
		flags = fcntl(client_fd, F_GETFL, 0);
		flags |= O_NONBLOCK;
		fcntl(client_fd, F_SETFL, flags);
#endif

		phalcon_server_log_printf(ctx, "Accept socket %d from %d\n", client_fd, listen_fd);

		client_ctx = phalcon_server_alloc_context(listen_ctx->pool);
		assert(client_ctx);

		client_ctx->fd = client_fd;

		client_ctx->handler = ctx->read;

		client_ctx->cpu_id = listen_ctx->cpu_id;
		client_ctx->ep_fd = listen_ctx->ep_fd;

		evt.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLET;
		evt.data.ptr = client_ctx;

		ret = epoll_ctl(client_ctx->ep_fd, EPOLL_CTL_ADD, client_ctx->fd, &evt);
		if (ret < 0) {
			perror("Unable to add client socket read event to epoll");
			goto free_back;
		}

		client_ctx->fd_added = 1;
		ctx->wdata[cpu_id].acceptcnt++;
	}

	goto back;

free_back:
	phalcon_server_log_printf(ctx, "cpu[%d] close socket %d\n", cpu_id, client_ctx->fd);

	phalcon_server_client_close(client_ctx);
	phalcon_server_free_context(client_ctx);

back:
	return;
}

void phalcon_server_do_stats(struct phalcon_server_context *ctx)
{
	sigset_t siglist;
	int signum;
	int i;

	if(sigemptyset(&siglist) == -1) {
		perror("Unable to initalize stats signal list");
		phalcon_server_exit_cleanup(ctx);
	}

	if(sigaddset(&siglist, SIGALRM) == -1) {
		perror("Unable to add SIGALRM signal to stats signal list");
		phalcon_server_exit_cleanup(ctx);
	}


	if(sigaddset(&siglist, SIGINT) == -1) {
		perror("Unable to add SIGINT signal to stats signal list");
		phalcon_server_exit_cleanup(ctx);
	}

	FILE *p = fdopen(ctx->pfd, "w");

	while(1) {
		if(sigwait(&siglist, &signum) != 0) {
			perror("Error waiting for signal");
			phalcon_server_exit_cleanup(ctx);
		}

		if(signum == SIGALRM) {
			uint64_t acceptcnt = 0, trancnt = 0;

			for(i = 0; i < ctx->num_workers; i++)
			{
				acceptcnt += ctx->wdata[i].acceptcnt - ctx->wdata[i].acceptcnt_prev;
				trancnt += ctx->wdata[i].trancnt - ctx->wdata[i].trancnt_prev;
				if (unlikely(ctx->enable_verbose))
					fprintf(p, "%"PRIu64"[%"PRIu64"-%"PRIu64"-%"PRIu64"-%"PRIu64"-%"PRIu64"-%"PRIu64"-%"PRIu64"-%"PRIu64"]  ",
						ctx->wdata[i].trancnt - ctx->wdata[i].trancnt_prev, ctx->wdata[i].polls_mpt,
						ctx->wdata[i].polls_lst, ctx->wdata[i].polls_min, ctx->wdata[i].polls_max,
						ctx->wdata[i].polls_avg, ctx->wdata[i].accept_cnt, ctx->wdata[i].read_cnt,
						ctx->wdata[i].write_cnt);
				ctx->wdata[i].acceptcnt_prev = ctx->wdata[i].acceptcnt;
				ctx->wdata[i].trancnt_prev = ctx->wdata[i].trancnt;
			}

			fprintf(p, "\tRequest/s %8"PRIu64",%8"PRIu64"\n", acceptcnt, trancnt);

		} else if(signum == SIGINT) {
			phalcon_server_stop_workers(ctx);
			break;
		}
	}
}

void phalcon_server_exit_cleanup(struct phalcon_server_context *ctx)
{
	phalcon_server_stop_workers(ctx);
	exit(EXIT_FAILURE);
}

void phalcon_server_stop_workers(struct phalcon_server_context *ctx)
{
	int i;

	if (ctx->wdata) {
		for(i = 0; i < ctx->num_workers; i++) {
			ctx->wdata[i].shutdown = 1;
			if (ctx->wdata[i].process) {
				phalcon_server_log_printf(ctx, "kill process %d\n", ctx->wdata[i].process);
				kill(ctx->wdata[i].process, SIGTERM);
			}
		}
	}
}
