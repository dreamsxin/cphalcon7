
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

#include "server/http.h"
#include "server/core.h"
#include "server/exception.h"
#include "server/utils.h"

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
 * Phalcon\Server\Http
 *
 *<code>
 *	class App extends Phalcon\Application {
 *		public function handle($data = NULL):Phalcon\Http\ResponseInterface{
 *			$response = new Phalcon\Http\Response('hello');
 *			$response->setHeader("Content-Type", "text/html");
 *			if ($data) {
 *				$response->setContent(json_encode($data));
 *			}
 *			return $response;
 *		}
 *	}
 *
 *	$server = new Phalcon\Server\Http('127.0.0.1', 8989);
 *  $server->start($application);
 *
 *</code>
 */
zend_class_entry *phalcon_server_http_ce;

PHP_METHOD(Phalcon_Server_Http, __construct);
PHP_METHOD(Phalcon_Server_Http, start);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server_http___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, config, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server_http_start, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, application, Phalcon\\Application, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_server_http_method_entry[] = {
	PHP_ME(Phalcon_Server_Http, __construct, arginfo_phalcon_server_http___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Server_Http, start, arginfo_phalcon_server_http_start, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_server_http_object_handlers;
zend_object* phalcon_server_http_object_create_handler(zend_class_entry *ce)
{
	phalcon_server_http_object *intern = ecalloc(1, sizeof(phalcon_server_http_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_server_http_object_handlers;

	memset(&intern->ctx, 0, sizeof(struct phalcon_server_context));
	intern->ctx.start_cpu = 0;
	intern->ctx.la_num = 1;

	return &intern->std;
}

void phalcon_server_http_object_free_handler(zend_object *object)
{
	phalcon_server_http_object *intern = phalcon_server_http_object_from_obj(object);

	if (intern->ctx.log_path) {
		zend_string_release(intern->ctx.log_path);
	}
	zend_object_std_dtor(object);
}

/**
 * Phalcon\Server\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Server_Http){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Server, Http, server_http, phalcon_server_http_method_entry, 0);

	zend_declare_property_null(phalcon_server_http_ce, SL("_config"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Http\Server constructor
 *
 * @param array $config
 * @throws \Phalcon\Server\Exception
 */
PHP_METHOD(Phalcon_Server_Http, __construct){

	zval *config, verbose = {}, worker = {}, log_path = {}, host = {}, port = {};
	phalcon_server_http_object *intern;
	int num_workers = 2;

	phalcon_fetch_params(0, 1, 0, &config);

	phalcon_update_property(getThis(), SL("_config"), config);

	intern = phalcon_server_http_object_from_obj(Z_OBJ_P(getThis()));

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

char *http_200="HTTP/1.0 200 OK\r\n"
	"Cache-Control: no-cache\r\n"
	"Connection: close\r\n"
	"Content-Type: text/html\r\n"
	"\r\n"
	"<html><body><h1>200 OK</h1>\nEverything is fine.\n</body></html>\n";

void phalcon_server_http_process_write(struct phalcon_server_context *ctx, struct phalcon_server_conn_context *client_ctx)
{
	int ep_fd, fd;
	int events = client_ctx->events;
	int cpu_id = client_ctx->cpu_id;
	int old_len, ret;
	struct epoll_event evt;
	phalcon_server_http_object *intern;

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
		ret = write(fd, http_200, strlen(http_200));
		if (ret < 0) {
			ctx->wdata[cpu_id].write_cnt++;
			perror("process_write() can't write client socket");
			goto free_back;
		}
	}

	phalcon_server_log_printf(ctx, "Write %d to socket %d\n", ret, fd);

	ctx->wdata[cpu_id].trancnt++;

	intern = phalcon_server_http_object_from_ctx(ctx);
	if (!intern->enable_keepalive)
		goto free_back;

	client_ctx->handler = ctx->read;

	evt.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLET;
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

extern struct http_parser_settings http_parser_request_settings;
static void phalcon_server_http_process_read(struct phalcon_server_context *ctx, struct phalcon_server_conn_context *client_ctx)
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
		phalcon_server_log_printf(ctx, "Socket %d is closed\n", fd);
		goto free_back;
	} else {
		phalcon_http_parser_data *parser_data;
		if (!client_ctx->user_data) {
			client_ctx->user_data = phalcon_http_parser_data_new(&http_parser_request_settings, HTTP_REQUEST);
		}
		parser_data = (phalcon_http_parser_data *)client_ctx->user_data;

		http_parser_execute(parser_data->parser, parser_data->settings, buf, ret);

		phalcon_server_log_printf(ctx, "Parser state %d, request from socket %d\n", parser_data->parser->state, fd);
        if (parser_data->parser->state >= HTTP_PARSER_STATE_END || ret < PHALCON_SERVER_MAX_BUFSIZE) {
			zval data, url = {}, response = {}, content = {};
			phalcon_server_http_object *intern;
			int flag = 0;

			array_init(&data);

			phalcon_array_update_str(&data, SL("header"), &parser_data->head, PH_COPY);

			ZVAL_STR(&url, parser_data->url.s);
			phalcon_array_update_str(&data, SL("url"), &url, PH_COPY);
			if (parser_data->body.s) {
				zval body = {};
				ZVAL_STR(&body, parser_data->body.s);
				phalcon_array_update_str(&data, SL("body"), &body, PH_COPY);
			}

			intern = phalcon_server_http_object_from_ctx(ctx);
			PHALCON_CALL_METHOD_FLAG(flag, &response, &intern->application, "handle", &data);
			phalcon_http_parser_data_free(parser_data);
			zval_ptr_dtor(&data);
			client_ctx->user_data = NULL;
			if (flag != FAILURE) {
				zval header = {};
				PHALCON_CALL_METHOD_FLAG(flag, &header, &response, "getheaders");
				if (flag != FAILURE) {
					zval ret = {};
					PHALCON_CALL_METHOD_FLAG(flag, &ret, &header, "tostring");
					if (flag != FAILURE) {
						client_ctx->response = phalcon_server_http_get_headers(Z_STRVAL(ret));
						zval_ptr_dtor(&ret);
					} else {
						client_ctx->response = phalcon_server_http_get_headers(NULL);
					}
					zval_ptr_dtor(&header);
				} else {
					client_ctx->response = phalcon_server_http_get_headers(NULL);
				}
			} else {
				client_ctx->response = phalcon_server_http_get_headers(NULL);
			}
			if (flag == FAILURE) {
				if (EG(exception)) {
					zval ex, msg;
					ZVAL_OBJ(&ex, EG(exception));
					phalcon_read_property(&msg, &ex, SL("message"), PH_NOISY|PH_READONLY);
					if (Z_TYPE(msg) == IS_STRING) {
						PHALCON_SERVER_STRING_APPEND(client_ctx->response, Z_STR(msg));
					}
					zend_clear_exception();
				}
			} else {
				PHALCON_CALL_METHOD_FLAG(flag, &content, &response, "getcontent");
				if (Z_TYPE(content) == IS_STRING) {
					PHALCON_SERVER_STRING_APPEND(client_ctx->response, Z_STR(content));
				}
				zval_ptr_dtor(&content);
				zval_ptr_dtor(&response);
			}
			client_ctx->handler = ctx->write;
			evt.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
			evt.data.ptr = client_ctx;
			ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &evt);
			if (ret < 0) {
				perror("Unable to add client socket write event to epoll");
				goto free_back;
			}
			goto back;
        }

		phalcon_server_log_printf(ctx, "Read %d from socket %d\n", ret, fd);
		client_ctx->handler = ctx->read;
		evt.events = EPOLLIN | EPOLLERR | EPOLLET;
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

/**
 * Run the Server
 *
 */
PHP_METHOD(Phalcon_Server_Http, start){

	zval *application;
	phalcon_server_http_object *intern;

	phalcon_fetch_params(0, 1, 0, &application);

	phalcon_update_property(getThis(), SL("_application"), application);

	intern = phalcon_server_http_object_from_obj(Z_OBJ_P(getThis()));
	PHALCON_SERVER_COPY_TO_STACK(&intern->application, application);
	intern->ctx.read = phalcon_server_http_process_read;
	intern->ctx.write = phalcon_server_http_process_write;
	printf("Listen address:\n\t%s:%d\n", intern->ctx.la[0].param_ip, intern->ctx.la[0].param_port);

	phalcon_server_init_log(&intern->ctx);
	phalcon_server_init_server(&intern->ctx);
	phalcon_server_init_signal(&intern->ctx);
	phalcon_server_init_workers(&intern->ctx);
	phalcon_server_init_timer(&intern->ctx);
	phalcon_server_do_stats(&intern->ctx);
}
