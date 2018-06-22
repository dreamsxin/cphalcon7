
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

#include "server.h"
#include "server/core.h"
#include "server/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/exception.h"

/**
 * Phalcon\Server
 *
 * It‘s an implementation of the socket server
 *</code>
 */
zend_class_entry *phalcon_server_ce;

PHP_METHOD(Phalcon_Server, __construct);
PHP_METHOD(Phalcon_Server, start);
PHP_METHOD(Phalcon_Server, close);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, config, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server_onconnect, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server_onreceive, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server_onsend, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server_onclose, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server_close, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_server_method_entry[] = {
	PHP_ME(Phalcon_Server, __construct, arginfo_phalcon_server___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Server, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Server, close, arginfo_phalcon_server_close, ZEND_ACC_PUBLIC)
	PHP_ABSTRACT_ME(Phalcon_Server, onConnect, arginfo_phalcon_server_onconnect)
	PHP_ABSTRACT_ME(Phalcon_Server, onReceive, arginfo_phalcon_server_onreceive)
	PHP_ABSTRACT_ME(Phalcon_Server, onSend, arginfo_phalcon_server_onsend)
	PHP_ABSTRACT_ME(Phalcon_Server, onClose, arginfo_phalcon_server_onclose)
	PHP_FE_END
};

zend_object_handlers phalcon_server_object_handlers;
zend_object* phalcon_server_object_create_handler(zend_class_entry *ce)
{
	phalcon_server_object *intern = ecalloc(1, sizeof(phalcon_server_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_server_object_handlers;

	memset(&intern->ctx, 0, sizeof(struct phalcon_server_context));
	intern->ctx.start_cpu = 0;
	intern->ctx.la_num = 1;

	return &intern->std;
}

void phalcon_server_object_free_handler(zend_object *object)
{
	phalcon_server_object *intern = phalcon_server_object_from_obj(object);

	if (intern->ctx.log_path) {
		zend_string_release(intern->ctx.log_path);
	}
	zend_object_std_dtor(object);
}

/**
 * Phalcon\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Server){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon, Server, server, phalcon_server_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Server constructor
 *
 * @param array $config
 * @throws \Phalcon\Server\Exception
 */
PHP_METHOD(Phalcon_Server, __construct){

	zval *config, verbose = {}, worker = {}, log_path = {}, host = {}, port = {};
	phalcon_server_object *intern;
	int num_workers = 2;

	phalcon_fetch_params(0, 1, 0, &config);

	phalcon_update_property(getThis(), SL("_config"), config);

	intern = phalcon_server_object_from_obj(Z_OBJ_P(getThis()));

	if (phalcon_array_isset_fetch_str(&verbose, config, SL("verbose"), PH_READONLY)) {
		intern->ctx.enable_verbose = zend_is_true(&verbose);
	}

	if (phalcon_array_isset_fetch_str(&worker, config, SL("worker"), PH_READONLY) && Z_TYPE(worker) == IS_LONG) {
		num_workers = Z_LVAL(worker);
	}

	intern->ctx.num_workers = num_workers > phalcon_server_get_cpu_num() ? phalcon_server_get_cpu_num() : num_workers;

	if (phalcon_array_isset_fetch_str(&log_path, config, SL("log"), PH_READONLY) && Z_TYPE(log_path) == IS_STRING) {
		intern->ctx.log_path = zend_string_copy(Z_STR(log_path));
	}

	if (phalcon_array_isset_fetch_str(&host, config, SL("host"), PH_READONLY) && Z_TYPE(host) == IS_STRING) {
		strncpy(intern->ctx.la[0].param_ip, Z_STRVAL(host), 32);
	} else {
		strncpy(intern->ctx.la[0].param_ip, "0.0.0.0", 32);
	}
	inet_aton(intern->ctx.la[0].param_ip, &intern->ctx.la[0].listenip);

	if (phalcon_array_isset_fetch_str(&port, config, SL("port"), PH_READONLY) && Z_TYPE(port) == IS_LONG) {
		intern->ctx.la[0].param_port =Z_LVAL(port);
	} else {
		intern->ctx.la[0].param_port = 8383;
	}
}

/**
 * Process handle
 */
void phalcon_server_process_write(struct phalcon_server_context *ctx, struct phalcon_server_conn_context *client_ctx)
{
	zval zfd = {}, response = {};
	phalcon_server_object *intern;
	int ep_fd, fd, flag = 0;
	int events = client_ctx->events;
	int cpu_id = client_ctx->cpu_id;
	int old_len, ret;
	struct epoll_event evt;

	ep_fd = client_ctx->ep_fd;
	fd = client_ctx->fd;

	phalcon_server_log_printf(ctx, "Process write event[%02x]\n", events);

	if (events & (EPOLLHUP | EPOLLERR)) {
		printf("process_write() with events HUP or ERR\n");
		goto free_back;
	}

	old_len = ZSTR_LEN(client_ctx->response);
	if (ZSTR_LEN(client_ctx->response)) {
		ret = write(fd, ZSTR_VAL(client_ctx->response), old_len);
		if (ret < 0) {
			ctx->wdata[cpu_id].write_cnt++;
			perror("process_write() can't write client socket");
			goto free_back;
		}
		if (ret < old_len) {
			zend_string *new_response = zend_string_alloc(old_len - ret, 0);
			memcpy(ZSTR_VAL(new_response), ZSTR_VAL(client_ctx->response) + ret, old_len - ret);
			zend_string_release(client_ctx->response);
			client_ctx->response = new_response;
			evt.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
			evt.data.ptr = client_ctx;
			ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &evt);
			if (ret < 0) {
				perror("Unable to add client socket read event to epoll");
				goto free_back;
			}
			goto back;
		} else {
			zend_string_release(client_ctx->response);
			client_ctx->response = NULL;
		}
	} else {
		ret = write(fd, '\0', 2);
		if (ret < 0) {
			ctx->wdata[cpu_id].write_cnt++;
			perror("process_write() can't write client socket");
			goto free_back;
		}
	}

	phalcon_server_log_printf(ctx, "Write %d to socket %d\n", ret, fd);

	ctx->wdata[cpu_id].trancnt++;

	ZVAL_LONG(&zfd, fd);

	intern = phalcon_server_object_from_ctx(ctx);
	PHALCON_CALL_METHOD_FLAG(flag, &response, &intern->application, "onsend", &zfd);
	if (flag == FAILURE) {
		if (EG(exception)) {
			zval ex, msg;
			ZVAL_OBJ(&ex, EG(exception));
			phalcon_read_property(&msg, &ex, SL("message"), PH_NOISY|PH_READONLY);
			if (Z_TYPE(msg) == IS_STRING) {
				perror(Z_STRVAL(msg));
			} else {
				perror("Couldn't execute method Server::onReceive");
			}
			zend_clear_exception();
		}
		goto free_back;
	} else if (Z_TYPE(response) == IS_STRING) {
		client_ctx->response = zend_string_copy(Z_STR(response));

		client_ctx->handler = ctx->write;
		evt.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
		evt.data.ptr = client_ctx;
		ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &evt);
		if (ret < 0) {
			perror("Unable to add client socket write event to epoll");
			goto free_back;
		}
		goto back;
	} else if (Z_TYPE(response) == IS_FALSE) {
		goto free_back;
	}

	client_ctx->handler = ctx->read;

	evt.events = EPOLLIN | EPOLLERR | EPOLLET;
	evt.data.ptr = client_ctx;

	ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &evt);
	if (ret < 0) {
		perror("Unable to add client socket read event to epoll");
		goto free_back;
	}

	goto back;

free_back:
	// __sync_synchronize();
	phalcon_server_client_close(client_ctx);
	// __sync_synchronize();
	phalcon_server_free_context(client_ctx);

back:
	return;
}

static void phalcon_server_process_read(struct phalcon_server_context *ctx, struct phalcon_server_conn_context *client_ctx)
{
	int ep_fd, fd;
	int events = client_ctx->events;
	struct epoll_event evt;
	int ret;
	char *buf = client_ctx->buf;
	int cpu_id = client_ctx->cpu_id;

	ep_fd = client_ctx->ep_fd;
	fd = client_ctx->fd;

	//FIXME: What else should I do.
	if (events & EPOLLHUP) {
		phalcon_server_log_printf(ctx, "process_read() with events HUP\n");
		goto free_back;
	}
	if (events & EPOLLERR) {
		phalcon_server_log_printf(ctx, "process_read() with events ERR\n");
		goto free_back;
	}

	phalcon_server_log_printf(ctx, "Process read event[%02x] on socket %d\n", events, fd);

    ret = read(fd, buf, PHALCON_SERVER_MAX_BUFSIZE);
	client_ctx->data_len = ret;
	if (ret < 0) {
		ctx->wdata[cpu_id].read_cnt++;
		perror("process_read() can't read client socket");
		goto free_back;
	} else if (ret == 0) {
		zval zfd = {}, response = {};
		phalcon_server_object *intern;
		int flag = 0;

		phalcon_server_log_printf(ctx, "Socket %d is closed\n", fd);

		ZVAL_LONG(&zfd, fd);
		intern = phalcon_server_object_from_ctx(ctx);
		PHALCON_CALL_METHOD_FLAG(flag, &response, &intern->application, "onclose", &zfd);
		if (flag == FAILURE) {
			if (EG(exception)) {
				zval ex, msg;
				ZVAL_OBJ(&ex, EG(exception));
				phalcon_read_property(&msg, &ex, SL("message"), PH_NOISY|PH_READONLY);
				if (Z_TYPE(msg) == IS_STRING) {
					perror(Z_STRVAL(msg));
				} else {
					perror("Couldn't execute method Server::onReceive");
				}
				zend_clear_exception();
			}
		}
		goto free_back;
	} else {
		zval zfd = {}, data = {}, response = {};
		phalcon_server_object *intern;
		int flag = 0;

		ZVAL_LONG(&zfd, fd);
		ZVAL_STRINGL(&data, buf, ret);
		intern = phalcon_server_object_from_ctx(ctx);
		PHALCON_CALL_METHOD_FLAG(flag, &response, &intern->application, "onreceive", &zfd, &data);
		if (flag == FAILURE) {
			if (EG(exception)) {
				zval ex, msg;
				ZVAL_OBJ(&ex, EG(exception));
				phalcon_read_property(&msg, &ex, SL("message"), PH_NOISY|PH_READONLY);
				if (Z_TYPE(msg) == IS_STRING) {
					perror(Z_STRVAL(msg));
				} else {
					perror("Couldn't execute method Server::onReceive");
				}
				zend_clear_exception();
			}
			goto free_back;
		} else if (Z_TYPE(response) == IS_STRING) {
			client_ctx->response = zend_string_copy(Z_STR(response));

			client_ctx->handler = ctx->write;
			evt.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
			evt.data.ptr = client_ctx;
			ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &evt);
			if (ret < 0) {
				perror("Unable to add client socket write event to epoll");
				goto free_back;
			}
			goto back;
        } else if (Z_TYPE(response) == IS_FALSE) {
			goto free_back;
		}

		phalcon_server_log_printf(ctx, "Read %d from socket %d\n", ret, fd);
		client_ctx->handler = ctx->read;
		evt.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLET;
		evt.data.ptr = client_ctx;

		ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &evt);
		if (ret < 0) {
			perror("Unable to add client socket read event to epoll");
			goto free_back;
		}
	}

	goto back;

free_back:
	phalcon_server_log_printf(ctx, "cpu[%d] close socket %d\n", cpu_id, client_ctx->fd);
	//__sync_synchronize();
	phalcon_server_client_close(client_ctx);
	//__sync_synchronize();
	phalcon_server_free_context(client_ctx);

back:
	return;
}

void phalcon_server_process_accept(struct phalcon_server_context *ctx, struct phalcon_server_conn_context * listen_ctx)
{
	zval zfd = {}, response = {};
	phalcon_server_object *intern;
	int client_fd, listen_fd, flag = 0;
	int events = listen_ctx->events;
	struct epoll_event evt;
	struct pahlcon_server_socket_address client_addr;
	socklen_t client_addrlen = sizeof(client_addr);

	struct phalcon_server_conn_context *client_ctx;

	int cpu_id = listen_ctx->cpu_id;
	int ret = 0;

	listen_fd = listen_ctx->fd;

	//TODO: What else should I do.
	if (events & (EPOLLHUP | EPOLLERR))
		return;

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

	ZVAL_LONG(&zfd, client_fd);
	intern = phalcon_server_object_from_ctx(ctx);
	PHALCON_CALL_METHOD_FLAG(flag, &response, &intern->application, "onconnect", &zfd);
	if (flag == FAILURE) {
		if (EG(exception)) {
			zval ex, msg;
			ZVAL_OBJ(&ex, EG(exception));
			phalcon_read_property(&msg, &ex, SL("message"), PH_NOISY|PH_READONLY);
			if (Z_TYPE(msg) == IS_STRING) {
				perror(Z_STRVAL(msg));
			} else {
				perror("Couldn't execute method Server::onReceive");
			}
			zend_clear_exception();
		}
		goto free_back;
	} else if (Z_TYPE(response) == IS_FALSE) {
		goto free_back;
	}

	evt.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLET;
	evt.data.ptr = client_ctx;

	ret = epoll_ctl(client_ctx->ep_fd, EPOLL_CTL_ADD, client_ctx->fd, &evt);
	if (ret < 0) {
		perror("Unable to add client socket read event to epoll");
		goto free_back;
	}

	client_ctx->fd_added = 1;
	ctx->wdata[cpu_id].acceptcnt++;

	goto back;

free_back:
	phalcon_server_log_printf(ctx, "cpu[%d] close socket %d\n", cpu_id, client_ctx->fd);

	phalcon_server_client_close(client_ctx);
	phalcon_server_free_context(client_ctx);

back:
	return;
}

/**
 * Run the Server
 *
 */
PHP_METHOD(Phalcon_Server, start){

	zval this;
	phalcon_server_object *intern;

	intern = phalcon_server_object_from_obj(Z_OBJ_P(getThis()));
	ZVAL_OBJ(&this, Z_OBJ_P(getThis()));
	PHALCON_SERVER_COPY_TO_STACK(&intern->application, &this);
	intern->ctx.read = phalcon_server_process_accept;
	intern->ctx.read = phalcon_server_process_read;
	intern->ctx.write = phalcon_server_process_write;
	printf("Listen address:\n\t%s:%d\n", intern->ctx.la[0].param_ip, intern->ctx.la[0].param_port);

	phalcon_server_init_log(&intern->ctx);
	phalcon_server_init_server(&intern->ctx);
	phalcon_server_init_signal(&intern->ctx);
	phalcon_server_init_workers(&intern->ctx);
	phalcon_server_init_timer(&intern->ctx);
	phalcon_server_do_stats(&intern->ctx);
}

/**
 * Close the socket
 *
 * @param int $fd
 * @return boolean
 */
PHP_METHOD(Phalcon_Server, close){

	zval *zfd;
	phalcon_server_object *intern;
	struct phalcon_server_conn_context *client_ctx;
	int fd;

	phalcon_fetch_params(0, 1, 0, &zfd);

	fd = Z_LVAL_P(zfd);

	intern = phalcon_server_object_from_obj(Z_OBJ_P(getThis()));

	__sync_synchronize();
	client_ctx = phalcon_server_get_context(intern->ctx.pool, fd);

	if (client_ctx) {
		phalcon_server_log_printf(ctx, "cpu[%d] close socket %d\n", intern->ctx.cpu_id, client_ctx->fd);
		__sync_synchronize();
		phalcon_server_client_close(client_ctx);
		__sync_synchronize();
		phalcon_server_free_context(client_ctx);
	}
}

/**
 * Emitted when the socket has connected
 *
 * @param int $fd
 */
PHALCON_DOC_METHOD(Phalcon_Server, onConnect);

/**
 * Emitted when the socket has received
 *
 * @param int $fd
 * @param string $data
 */
PHALCON_DOC_METHOD(Phalcon_Server, onReceive);

/**
 * Emitted when the socket has send
 *
 * @param int $fd
 * @param string $data
 */
PHALCON_DOC_METHOD(Phalcon_Server, onSend);

/**
 * Emitted when the socket has closed
 *
 * @param int $fd
 */
PHALCON_DOC_METHOD(Phalcon_Server, onClose);

/**
 * Emitted when the socket can write content
 *
 * @param int $fd
 * @return string|boolean
 */
PHALCON_DOC_METHOD(Phalcon_Server, getContent);
