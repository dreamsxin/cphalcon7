
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

#include "socket/server.h"
#include "socket.h"
#include "socket/client.h"
#include "socket/exception.h"

#include <Zend/zend_closures.h>

#if HAVE_EPOLL
# include <sys/epoll.h>
# include <unistd.h>      /* for fork & setsid */
# include <sys/stat.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <arpa/inet.h>
# include <signal.h>
# define EPOLL_EVENT_SIZE 1024
#endif

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
 * Phalcon\Socket\Server
 *
 *<code>
 *
 *	$server = new Phalcon\Socket\Server('127.0.0.1', 8989);
 *  $server->run(
 *      function(Phalcon\Socket\Client $client){
 *          // Connect
 *      },
 *      function(Phalcon\Socket\Client $client, $mssage){
 *          // Read
 *          echo $mssage.PHP_EOL;
 *      },
 *      function(Phalcon\Socket\Client $client){
 *          // Send
 *          $client->write("Welcome!");
 *      },
 *      function(Phalcon\Socket\Client $client){
 *          // Close
 *      },
 *      function(Phalcon\Socket\Client $client){
 *          // Error
 *      },
 *      function(){
 *          // Timeout
 *      }
 *  );
 *
 *</code>
 */
zend_class_entry *phalcon_socket_server_ce;

PHP_METHOD(Phalcon_Socket_Server, __construct);
PHP_METHOD(Phalcon_Socket_Server, setDaemon);
PHP_METHOD(Phalcon_Socket_Server, setMaxChildren);
PHP_METHOD(Phalcon_Socket_Server, setEvent);
PHP_METHOD(Phalcon_Socket_Server, getEvent);
PHP_METHOD(Phalcon_Socket_Server, listen);
PHP_METHOD(Phalcon_Socket_Server, accept);
PHP_METHOD(Phalcon_Socket_Server, getClients);
PHP_METHOD(Phalcon_Socket_Server, getClient);
PHP_METHOD(Phalcon_Socket_Server, disconnect);
PHP_METHOD(Phalcon_Socket_Server, run);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server___construct, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, domain, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, protocol, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_setdaemon, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, daemon, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_setmaxchildren, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, maxChildren, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_setevent, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_bind, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_listen, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, backlog, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_getclient, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, socketId, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_disconnect, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, socketId, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_run, 0, 0, 0)
	ZEND_ARG_CALLABLE_INFO(0, onconnection, 1)
	ZEND_ARG_CALLABLE_INFO(0, onrecv, 1)
	ZEND_ARG_CALLABLE_INFO(0, onsend, 1)
	ZEND_ARG_CALLABLE_INFO(0, onclose, 1)
	ZEND_ARG_CALLABLE_INFO(0, onerror, 1)
	ZEND_ARG_CALLABLE_INFO(0, ontimeout, 1)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, usec, IS_LONG, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_socket_server_method_entry[] = {
	PHP_ME(Phalcon_Socket_Server, __construct, arginfo_phalcon_socket_server___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Socket_Server, setDaemon, arginfo_phalcon_socket_server_setdaemon, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, setMaxChildren, arginfo_phalcon_socket_server_setmaxchildren, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, setEvent, arginfo_phalcon_socket_server_setevent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, getEvent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, listen, arginfo_phalcon_socket_server_listen, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, accept, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, getClients, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, getClient, arginfo_phalcon_socket_server_getclient, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, disconnect, arginfo_phalcon_socket_server_disconnect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, run, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Socket\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Socket_Server){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Socket, Server, socket_server, phalcon_socket_ce,  phalcon_socket_server_method_entry, 0);

	zend_declare_property_null(phalcon_socket_server_ce, SL("_address"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_socket_server_ce, SL("_port"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_socket_server_ce, SL("_daemon"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_socket_server_ce, SL("_maxChildren"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_socket_server_ce, SL("_clients"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_socket_server_ce, SL("_event"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_socket_server_ce, SL("_backlog"), 0, ZEND_ACC_PROTECTED);
#if HAVE_EPOLL
	zend_declare_property_long(phalcon_socket_server_ce, SL("_epollSize"), 1024, ZEND_ACC_PROTECTED);
#endif

	zend_declare_class_constant_long(phalcon_socket_server_ce, SL("USE_SELECT"),	0);
#if HAVE_EPOLL
	zend_declare_class_constant_long(phalcon_socket_server_ce, SL("USE_EPOLL"),		1);
#endif
	return SUCCESS;
}

/**
 * Phalcon\Socket\Server constructor
 *
 * @param string $address
 * @param int $port
 * @param int $domain
 * @param int $type
 * @param int $protocol
 * @throws \Phalcon\Socket\Exception
 */
PHP_METHOD(Phalcon_Socket_Server, __construct){

	zval *address, *port, *_domain = NULL, *_type = NULL, *_protocol = NULL, domain = {}, type = {}, protocol = {}, socket = {};
	zval filter_type = {}, filter_option = {}, filtered = {};

	phalcon_fetch_params(0, 2, 3, &address, &port, &_domain, &_type, &_protocol);

	if (!_domain || Z_TYPE_P(_domain) == IS_NULL) {
		ZVAL_LONG(&filter_type, 275); // FILTER_VALIDATE_IP
		ZVAL_LONG(&filter_option, 1048576); // FILTER_FLAG_IPV4
		PHALCON_CALL_FUNCTION(&filtered, "filter_var", address, &filter_type, &filter_option);
		if (zend_is_true(&filtered)) {
			ZVAL_LONG(&domain, PHALCON_SOCKET_AF_INET);
		} else {
			ZVAL_LONG(&filter_option, 2097152); // FILTER_FLAG_IPV6
			PHALCON_CALL_FUNCTION(&filtered, "filter_var", address, &filter_type, &filter_option);
			if (zend_is_true(&filtered)) {
				ZVAL_LONG(&domain, PHALCON_SOCKET_AF_INET6);
			} else {
				ZVAL_LONG(&domain, PHALCON_SOCKET_AF_UNIX);
			}
		}
	} else {
		ZVAL_COPY_VALUE(&domain, _domain);
	}

	if (!_type || Z_TYPE_P(_type) == IS_NULL) {
		phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&type, _type);
	}

	if (!_protocol || Z_TYPE_P(_protocol) == IS_NULL) {
		phalcon_read_property(&protocol, getThis(), SL("_protocol"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&protocol, _protocol);
	}

	PHALCON_CALL_FUNCTION(&socket, "socket_create", &domain, &type, &protocol);
	if (Z_TYPE(socket) != IS_RESOURCE) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_throwsocketexception");
		return;
	}
	phalcon_update_property(getThis(), SL("_socket"), &socket);

	phalcon_update_property(getThis(), SL("_address"), address);
	phalcon_update_property(getThis(), SL("_port"), port);

	phalcon_update_property_empty_array(getThis(), SL("_clients"));
}

/**
 * Sets the run mode
 */
PHP_METHOD(Phalcon_Socket_Server, setDaemon){

	zval *daemon;

	phalcon_fetch_params(0, 1, 0, &daemon);

	phalcon_update_property(getThis(), SL("_daemon"), daemon);

	RETURN_THIS();
}

/**
 * Sets the run mode
 */
PHP_METHOD(Phalcon_Socket_Server, setMaxChildren){

	zval *max_children;

	phalcon_fetch_params(0, 1, 0, &max_children);

	phalcon_update_property(getThis(), SL("_maxChildren"), max_children);

	RETURN_THIS();
}

/**
 * Sets the event
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Socket_Server, setEvent){

	zval *event;

	phalcon_fetch_params(0, 1, 0, &event);

	switch(Z_LVAL_P(event)) {
		case 0:
			phalcon_update_property(getThis(), SL("_event"), event);
			break;
#if HAVE_EPOLL
		case 1:
			phalcon_update_property(getThis(), SL("_event"), event);
			break;
#endif
		default:
			RETURN_FALSE;
	}

	RETURN_TRUE;
}

/**
 * Gets the event
 *
 * @return int
 */
PHP_METHOD(Phalcon_Socket_Server, getEvent){

	RETURN_MEMBER(getThis(), "_event");
}

/**
 * Listens for a connection on a socket
 *
 * @param int $backlog
 * @return boolean
 */
PHP_METHOD(Phalcon_Socket_Server, listen){

	zval *_backlog = NULL, backlog = {}, socket = {}, address = {}, port = {};

	phalcon_fetch_params(0, 0, 1, &_backlog);

	if (!_backlog) {
		phalcon_read_property(&backlog, getThis(), SL("_backlog"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&backlog, _backlog);
	}

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&address, getThis(), SL("_address"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&port, getThis(), SL("_port"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_FUNCTION(return_value, "socket_bind", &socket, &address, &port);
	if (!PHALCON_IS_TRUE(return_value)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_throwsocketexception");
	}

	PHALCON_CALL_FUNCTION(return_value, "socket_listen", &socket, &backlog);
	if (!PHALCON_IS_TRUE(return_value)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_throwsocketexception");
	}
}

/**
 * Accept a connection
 *
 * @return Phalcon\Socket\Client
 */
PHP_METHOD(Phalcon_Socket_Server, accept){

	zval socket = {}, client_socket = {}, socket_id = {};

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_FUNCTION(&client_socket, "socket_accept", &socket);

	if (PHALCON_IS_FALSE(&client_socket)) {
		RETURN_FALSE;
	} else {
		object_init_ex(return_value, phalcon_socket_client_ce);
		PHALCON_CALL_METHOD(NULL, return_value, "__construct", &client_socket);
		PHALCON_CALL_METHOD(&socket_id, return_value, "getsocketid");

		phalcon_update_property_array(getThis(), SL("_clients"), &socket_id, return_value);
	}
}

/**
 * Gets all connections
 *
 * @return Phalcon\Socket\Client
 */
PHP_METHOD(Phalcon_Socket_Server, getClients){

	RETURN_MEMBER(getThis(), "_clients")
}

/**
 * Gets a connection
 *
 * @return Phalcon\Socket\Client
 */
PHP_METHOD(Phalcon_Socket_Server, getClient){

	zval *socket_id;

	phalcon_fetch_params(0, 1, 0, &socket_id);

	phalcon_read_property_array(return_value, getThis(), SL("_clients"), socket_id, PH_COPY);
}

/**
 * Close a client
 *
 * @return Phalcon\Socket\Server
 */
PHP_METHOD(Phalcon_Socket_Server, disconnect){

	zval *socket_id, client = {};

	phalcon_fetch_params(0, 1, 0, &socket_id);

	phalcon_read_property_array(&client, getThis(), SL("_clients"), socket_id, PH_READONLY);
	if (Z_TYPE(client) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, &client, "close");
	}
	phalcon_unset_property_array(getThis(), SL("_clients"), socket_id);
}

void setkeepalive(int fd) {
	int keepAlive = 1;
	int keepIdle = 30;
	int keepInterval = 5;
	int keepCount = 3;

	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *) &keepAlive, sizeof (keepAlive));
	setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*) &keepIdle, sizeof (keepIdle));
	setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *) &keepInterval, sizeof (keepInterval));
	setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *) &keepCount, sizeof (keepCount));
}

#ifdef PHALCON_USE_PHP_SOCKET
struct _phalcon_socket_server {
	int fd;
	int ppid;
	int running_children;
	int running;
	int timeout;
} *server;

typedef struct _phalcon_socket_server phalcon_socket_server;

int phalcon_socket_server_init() {
	phalcon_socket_server *instance;
	if (server) {
		return 0;
	}

	instance = calloc(1, sizeof(phalcon_socket_server));
	instance->timeout = 3;
	server = instance;

	return 1;
}

void phalcon_socket_server_destroy() {
	if (server->fd) {
		close(server->fd);
	}
	free(server);
	server = NULL;
}

static void phalcon_socket_sig_handler(int signo) {
	(void)signo;

	server->running = 0;
	return;
}

static void phalcon_socket_sig_reg() {
	struct sigaction act;

	act.sa_handler = phalcon_socket_sig_handler;
	sigemptyset(&act.sa_mask);

#ifdef SA_INTERRUPT
	act.sa_flags = SA_INTERRUPT;
#else
	act.sa_flags = 0;
#endif

	signal(SIGPIPE, SIG_IGN);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);

	server->ppid = getpid();
}

static int phalcon_socket_server_start_daemon(void) {
	pid_t pid;
	int i, fd;

	pid = fork();
	switch (pid) {
		case -1:
			return 0;
		case 0:
			setsid();
			break;
		default:
			exit(0);
			break;
	}

	umask(0);

	if (chdir("/") <0 ) {
		return 0;
	}

	for (i=0; i < 3; i++) {
		close(i);
	}

	fd = open("/dev/null", O_RDWR);
	if (dup2(fd, 0) < 0 || dup2(fd, 1) < 0 || dup2(fd, 2) < 0) {
		close(fd);
		return 0;
	}

	close(fd);
	return 1;
}

static int phalcon_socket_server_startup_workers(zval *object, int max_childs) {
	int pid = 0;
	if (!max_childs) {
		phalcon_socket_sig_reg();
		return 1;
	} else {
		while (max_childs-- && (pid = fork())) {
			server->running_children++;
		}
		if (pid) { // parent
			phalcon_socket_sig_reg();
			return 0;
		} else { // child
			return 1;
		}
	}
}
#endif

/**
 * Run the Server
 *
 */
PHP_METHOD(Phalcon_Socket_Server, run){
#ifdef PHALCON_USE_PHP_SOCKET
	zval *_onconnection = NULL, *_onrecv = NULL, *_onsend = NULL, *_onclose = NULL, *_onerror = NULL, *_ontimeout = NULL, *timeout = NULL, *usec = NULL;
	zval onconnection = {}, onrecv = {}, onsend = {}, onclose = {}, onerror = {}, ontimeout, socket = {}, maxlen = {}, event = {};
	zval daemon = {}, max_children = {};
	php_socket *listen_php_sock;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof (client_addr);
	int listenfd;

	phalcon_fetch_params(0, 0, 8, &_onconnection, &_onrecv, &_onsend, &_onclose, &_onerror, &_ontimeout, &timeout, &usec);

	if (_onconnection) {
		if (Z_TYPE_P(_onconnection) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onconnection), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATIC(&onconnection, zend_ce_closure, "bind", _onconnection, getThis());
		} else {
			ZVAL_COPY_VALUE(&onconnection, _onconnection);
		}
	}

	if (_onrecv) {
		if (Z_TYPE_P(_onrecv) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onrecv), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATIC(&onrecv, zend_ce_closure, "bind", _onrecv, getThis());
		} else {
			ZVAL_COPY_VALUE(&onrecv, _onrecv);
		}
	}

	if (_onsend) {
		if (Z_TYPE_P(_onsend) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onsend), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATIC(&onsend, zend_ce_closure, "bind", _onsend, getThis());
		} else {
			ZVAL_COPY_VALUE(&onsend, _onsend);
		}
	}

	if (_onclose) {
		if (Z_TYPE_P(_onclose) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onclose), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATIC(&onclose, zend_ce_closure, "bind", _onclose, getThis());
		} else {
			ZVAL_COPY_VALUE(&onclose, _onclose);
		}
	}

	if (_onerror) {
		if (Z_TYPE_P(_onerror) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onerror), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATIC(&onerror, zend_ce_closure, "bind", _onerror, getThis());
		} else {
			ZVAL_COPY_VALUE(&onerror, _onerror);
		}
	}

	if (_ontimeout) {
		if (Z_TYPE_P(_ontimeout) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_ontimeout), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATIC(&ontimeout, zend_ce_closure, "bind", _ontimeout, getThis());
		} else {
			ZVAL_COPY_VALUE(&ontimeout, _ontimeout);
		}
	}

	if (!timeout) {
		timeout = &PHALCON_GLOBAL(z_null);
	}

	if (!usec) {
		usec = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&maxlen, getThis(), SL("_maxlen"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&event, getThis(), SL("_event"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&daemon, getThis(), SL("_daemon"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&max_children, getThis(), SL("_maxChildren"), PH_NOISY|PH_READONLY);

	if ((listen_php_sock = (php_socket *)zend_fetch_resource_ex(&socket, php_sockets_le_socket_name, php_sockets_le_socket())) == NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_socket_exception_ce, "epoll: can't fetch master socket");
		RETURN_FALSE;
	}

	PHALCON_CALL_METHOD(NULL, getThis(), "listen");
	PHALCON_CALL_METHOD(NULL, getThis(), "setblocking", &PHALCON_GLOBAL(z_false));

	phalcon_socket_server_init();
	server->fd = listenfd = listen_php_sock->bsd_socket;

	if (zend_is_true(&daemon) && !phalcon_socket_server_start_daemon()) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_socket_exception_ce, "Failed to setup daemon");
		phalcon_socket_server_destroy();
		RETURN_FALSE;
	}
	server->running = 1;
	if (!phalcon_socket_server_startup_workers(getThis(), Z_LVAL(max_children))) {
		/* master */
		pid_t cid;
		int stat;
		while (server->running) {
			if ((cid = waitpid(-1, &stat, 0)) > 0) {
				if (!(cid = fork())) {
					goto worker;
				}
			}
		}
		signal(SIGQUIT, SIG_IGN);
		kill(-(server->ppid), SIGQUIT);

		while(server->running_children) {
			while ((cid = waitpid(-1, &stat, 0)) > 0) {
				server->running_children--;
			}
		}
		phalcon_socket_server_destroy();
		return;
	}
worker:
#if HAVE_EPOLL
	if (Z_LVAL(event) == 0) {
#endif
		while(server->running) {
			zval r_array = {}, w_array = {}, e_array = {}, ret = {}, clients = {}, *client = NULL, *client_socket = NULL;

			array_init(&r_array);
			array_init(&w_array);
			array_init(&e_array);

			phalcon_array_append(&r_array, &socket, PH_COPY);

			phalcon_read_property(&clients, getThis(), SL("_clients"), PH_NOISY|PH_READONLY);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(clients), client) {
				zval client_socket = {};
				PHALCON_CALL_METHOD(&client_socket, client, "getsocket");
				phalcon_array_append(&r_array, &client_socket, PH_COPY);
			} ZEND_HASH_FOREACH_END();

			ZVAL_MAKE_REF(&r_array);
			ZVAL_MAKE_REF(&w_array);
			ZVAL_MAKE_REF(&e_array);
			PHALCON_CALL_FUNCTION(&ret, "socket_select", &r_array, &w_array, &e_array, timeout, usec);
			ZVAL_UNREF(&r_array);
			ZVAL_UNREF(&w_array);
			ZVAL_UNREF(&e_array);

			if (PHALCON_IS_FALSE(&ret)) {
				PHALCON_CALL_METHOD(NULL, getThis(), "_throwsocketexception");
				return;
			}
			if (PHALCON_IS_LONG_IDENTICAL(&ret, 0)) {
				if (phalcon_method_exists_ex(getThis(), SL("ontimeout")) == SUCCESS) {
					PHALCON_CALL_METHOD(NULL, getThis(), "ontimeout");
				}
				if (Z_TYPE(ontimeout) > IS_NULL) {
					PHALCON_CALL_USER_FUNC(NULL, &ontimeout);
				}
				continue;
			}

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(r_array), client_socket) {
				zval client_socket_id = {}, client = {}, *args, data = {};
				php_socket *php_sock;
				int client_fd, status = 0;
				if ((php_sock = (php_socket *)zend_fetch_resource_ex(client_socket, php_sockets_le_socket_name, php_sockets_le_socket())) == NULL) {
					continue;
				}
				client_fd = php_sock->bsd_socket;
				ZVAL_LONG(&client_socket_id, client_fd);
				if (php_sock->bsd_socket == listenfd) {
					if ((client_fd = accept(listenfd, (struct sockaddr *) &client_addr, &client_addr_len)) >= 0) {
						zval tmp_client_socket = {}, tmp_client = {}, tmp_client_socket_id = {};
						php_socket *tmp_client_sock;
						tmp_client_sock = php_create_socket();
						tmp_client_sock->bsd_socket = client_fd;
						tmp_client_sock->error = 0;
						tmp_client_sock->blocking = 1;
						tmp_client_sock->type = ((struct sockaddr *) &client_addr)->sa_family;

						ZVAL_RES(&tmp_client_socket, zend_register_resource(tmp_client_sock, php_sockets_le_socket()));

						object_init_ex(&tmp_client, phalcon_socket_client_ce);
						PHALCON_CALL_METHOD(NULL, &tmp_client, "__construct", &tmp_client_socket);

						ZVAL_LONG(&tmp_client_socket_id, client_fd);
						phalcon_update_property_array(getThis(), SL("_clients"), &tmp_client_socket_id, &tmp_client);

						PHALCON_CALL_METHOD(NULL, &tmp_client, "setblocking", &PHALCON_GLOBAL(z_false));

						if (Z_TYPE(onconnection) > IS_NULL) {
							args = (zval *)safe_emalloc(1, sizeof(zval), 0);
							ZVAL_COPY(&args[0], &tmp_client);
							PHALCON_CALL_USER_FUNC_ARGS(NULL, &onconnection, args, 1);
						} else if (phalcon_method_exists_ex(getThis(), SL("onconnection")) == SUCCESS) {
							PHALCON_CALL_METHOD(NULL, getThis(), "onconnection", &tmp_client);
						}
						setkeepalive(client_fd);
					}
					continue;
				} else {
					PHALCON_CALL_METHOD(&client, getThis(), "getclient", &client_socket_id);
					while(1) {
						zend_string	*tmpbuf;
						int retval;

						tmpbuf = zend_string_alloc(Z_LVAL(maxlen), 0);

						retval = recv(client_fd, ZSTR_VAL(tmpbuf), Z_LVAL(maxlen), MSG_DONTWAIT);

						if (retval < 0) {
							zend_string_free(tmpbuf);
							if(errno == EAGAIN || errno == EWOULDBLOCK) {
								status = 1;
								break;
							}
							if (Z_TYPE(onerror) > IS_NULL) {
								args = (zval *)safe_emalloc(1, sizeof(zval), 0);
								ZVAL_COPY(&args[0], &client);
								PHALCON_CALL_USER_FUNC_ARGS(NULL, &onerror, args, 1);
							} else if (phalcon_method_exists_ex(getThis(), SL("onerror")) == SUCCESS) {
								PHALCON_CALL_METHOD(NULL, getThis(), "onerror", &client);
							}
							status = -1;
							break;
						} else if (retval == 0) {
							zend_string_free(tmpbuf);
							status = -1;
							break;
						} else {
							status = 1;
							tmpbuf = zend_string_truncate(tmpbuf, retval, 0);
							ZSTR_LEN(tmpbuf) = retval;
							ZSTR_VAL(tmpbuf)[ZSTR_LEN(tmpbuf)] = '\0' ;

							ZVAL_NEW_STR(&data, tmpbuf);
							if (Z_TYPE(onrecv) > IS_NULL) {
								args = (zval *)safe_emalloc(2, sizeof(zval), 0);
								ZVAL_COPY(&args[0], &client);
								ZVAL_COPY_VALUE(&args[1], &data);
								PHALCON_CALL_USER_FUNC_ARGS(&ret, &onrecv, args, 2);
							} else if (phalcon_method_exists_ex(getThis(), SL("onrecv")) == SUCCESS) {
								PHALCON_CALL_METHOD(&ret, getThis(), "onrecv", &client, &data);
							}

							if (PHALCON_IS_FALSE(&ret)) {
								status = -1;
								break;
							}
						}
					}
					if (status <= 0) {
						if (Z_TYPE(onclose) > IS_NULL) {
							args = (zval *)safe_emalloc(1, sizeof(zval), 0);
							ZVAL_COPY(&args[0], &client);
							PHALCON_CALL_USER_FUNC_ARGS(NULL, &onclose, args, 1);
						} else if (phalcon_method_exists_ex(getThis(), SL("onclose")) == SUCCESS) {
							PHALCON_CALL_METHOD(NULL, getThis(), "onclose", &client);
						}
						shutdown(client_fd, SHUT_RDWR);
						close(client_fd);
						phalcon_unset_property_array(getThis(), SL("_clients"), &client_socket_id);
					} else {
						if (Z_TYPE(onsend) > IS_NULL) {
							args = (zval *)safe_emalloc(1, sizeof(zval), 0);
							ZVAL_COPY(&args[0], &client);
							PHALCON_CALL_USER_FUNC_ARGS(&ret, &onsend, args, 1);
						} else if (phalcon_method_exists_ex(getThis(), SL("onsend")) == SUCCESS) {
							PHALCON_CALL_METHOD(&ret, getThis(), "onsend", &client, &data);
						}
						if (PHALCON_IS_FALSE(&ret)) {
							if (Z_TYPE(onclose) > IS_NULL) {
								args = (zval *)safe_emalloc(1, sizeof(zval), 0);
								ZVAL_COPY(&args[0], &client);
								PHALCON_CALL_USER_FUNC_ARGS(NULL, &onclose, args, 1);
							} else if (phalcon_method_exists_ex(getThis(), SL("onclose")) == SUCCESS) {
								PHALCON_CALL_METHOD(NULL, getThis(), "onclose", &client);
							}

							shutdown(client_fd, SHUT_RDWR);
							close(client_fd);
							phalcon_unset_property_array(getThis(), SL("_clients"), &client_socket_id);
						}
					}
				}
			} ZEND_HASH_FOREACH_END();
		}
#if HAVE_EPOLL
	} else if (Z_LVAL(event) == 1) {
		struct epoll_event events[EPOLL_EVENT_SIZE];
		struct epoll_event ev = {0};
		int epollfd, sec = -1, epoll_ctl_ret;

		if (Z_TYPE_P(timeout) == IS_LONG) {
			sec = Z_LVAL_P(timeout) * 1000;
		}

		epollfd = epoll_create(EPOLL_EVENT_SIZE+1);
		if (epollfd < 0) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_socket_exception_ce, "epoll: unable to initialize");
			phalcon_socket_server_destroy();
			RETURN_FALSE;
		}

		ev.data.fd = listenfd;
		ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;

		if ((epoll_ctl_ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev)) < 0) {
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_socket_exception_ce, "epoll: unable to add fd %d", listenfd);
			phalcon_socket_server_destroy();
			RETURN_FALSE;
		}

		while (server->running) {
			int ret, i;
			ret = epoll_wait(epollfd, events, EPOLL_EVENT_SIZE/100, sec);
			if (ret < 0) {
				if (errno != EINTR && errno != EWOULDBLOCK) {
					PHALCON_THROW_EXCEPTION_FORMAT(phalcon_socket_exception_ce, "epoll_wait() returns %d", errno);
					phalcon_socket_server_destroy();
					RETURN_FALSE;
				}
				continue;
			} else if (ret == 0) {
				if (phalcon_method_exists_ex(getThis(), SL("ontimeout")) == SUCCESS) {
					PHALCON_CALL_METHOD(NULL, getThis(), "ontimeout");
				}
				if (Z_TYPE(ontimeout) > IS_NULL) {
					PHALCON_CALL_USER_FUNC(NULL, &ontimeout);
				}
				continue;
			}
			for (i = 0; i < ret; i++) {
				zval client = {}, client_socket_id = {}, data = {}, *args, ret = {};
				int event_fd, client_fd, status = 0;

				event_fd = events[i].data.fd;
				if (event_fd <= 0) {
					continue;
				}
				ZVAL_LONG(&client_socket_id, event_fd);
				if (event_fd == listenfd) {
					if (events[i].events & (EPOLLHUP | EPOLLERR)) {
						continue;
					}
					client_fd = accept(listenfd, (struct sockaddr *) &client_addr, &client_addr_len);
					if (client_fd < 0) {
						continue;
					}
					zval tmp_client_socket = {}, tmp_client = {}, tmp_client_socket_id = {};
					php_socket *tmp_client_sock;
					tmp_client_sock = php_create_socket();
					tmp_client_sock->bsd_socket = client_fd;
					tmp_client_sock->error = 0;
					tmp_client_sock->blocking = 1;
					tmp_client_sock->type = ((struct sockaddr *) &client_addr)->sa_family;

					ZVAL_RES(&tmp_client_socket, zend_register_resource(tmp_client_sock, php_sockets_le_socket()));

					object_init_ex(&tmp_client, phalcon_socket_client_ce);
					PHALCON_CALL_METHOD(NULL, &tmp_client, "__construct", &tmp_client_socket);

					ZVAL_LONG(&tmp_client_socket_id, client_fd);
					phalcon_update_property_array(getThis(), SL("_clients"), &tmp_client_socket_id, &tmp_client);

					PHALCON_CALL_METHOD(NULL, &tmp_client, "setblocking", &PHALCON_GLOBAL(z_false));

					if (Z_TYPE(onconnection) > IS_NULL) {
						args = (zval *)safe_emalloc(1, sizeof(zval), 0);
						ZVAL_COPY(&args[0], &tmp_client);
						PHALCON_CALL_USER_FUNC_ARGS(NULL, &onconnection, args, 1);
					} else if (phalcon_method_exists_ex(getThis(), SL("onconnection")) == SUCCESS) {
						PHALCON_CALL_METHOD(NULL, getThis(), "onconnection", &tmp_client);
					}
					setkeepalive(client_fd);
					ev.data.fd = client_fd;
					ev.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLET;
					epoll_ctl_ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &ev);
					if (epoll_ctl_ret < 0) {
						close(client_fd);
					}
					continue;
				}
				if (events[i].events & (EPOLLHUP | EPOLLERR)) {
						continue;
				}
				if (events[i].events & EPOLLIN) {
					PHALCON_CALL_METHOD(&client, getThis(), "getclient", &client_socket_id);

					zend_string	*tmpbuf;
					int retval;

					tmpbuf = zend_string_alloc(Z_LVAL(maxlen), 0);

					retval = recv(event_fd, ZSTR_VAL(tmpbuf), Z_LVAL(maxlen), MSG_DONTWAIT);

					if (retval < 0) {
						zend_string_free(tmpbuf);
						if (Z_TYPE(onerror) > IS_NULL) {
							args = (zval *)safe_emalloc(1, sizeof(zval), 0);
							ZVAL_COPY(&args[0], &client);
							PHALCON_CALL_USER_FUNC_ARGS(NULL, &onerror, args, 1);
						} else if (phalcon_method_exists_ex(getThis(), SL("onerror")) == SUCCESS) {
							PHALCON_CALL_METHOD(NULL, getThis(), "onerror", &client);
						}
						status = -1;
					} else if (retval == 0) {
						zend_string_free(tmpbuf);
						status = -1;
					} else {
						status = 1;
						tmpbuf = zend_string_truncate(tmpbuf, retval, 0);
						ZSTR_LEN(tmpbuf) = retval;
						ZSTR_VAL(tmpbuf)[ZSTR_LEN(tmpbuf)] = '\0' ;

						ZVAL_NEW_STR(&data, tmpbuf);
						if (Z_TYPE(onrecv) > IS_NULL) {
							args = (zval *)safe_emalloc(2, sizeof(zval), 0);
							ZVAL_COPY(&args[0], &client);
							ZVAL_COPY_VALUE(&args[1], &data);
							PHALCON_CALL_USER_FUNC_ARGS(&ret, &onrecv, args, 2);
						} else if (phalcon_method_exists_ex(getThis(), SL("onrecv")) == SUCCESS) {
							PHALCON_CALL_METHOD(&ret, getThis(), "onrecv", &client, &data);
						}

						if (PHALCON_IS_FALSE(&ret)) {
							status = -1;
						}
					}

					ev.data.fd = event_fd;
					if (status > 0) {
						if (retval == Z_LVAL(maxlen)) {
							ev.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLET;
						} else {
							ev.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
						}
						epoll_ctl_ret = epoll_ctl(epollfd, EPOLL_CTL_MOD, event_fd, &ev);
						if (epoll_ctl_ret < 0) {
							status = -1;
						}
					}
					if  (status < 0) {
						if (Z_TYPE(onclose) > IS_NULL) {
							args = (zval *)safe_emalloc(1, sizeof(zval), 0);
							ZVAL_COPY(&args[0], &client);
							PHALCON_CALL_USER_FUNC_ARGS(NULL, &onclose, args, 1);
						} else if (phalcon_method_exists_ex(getThis(), SL("onclose")) == SUCCESS) {
							PHALCON_CALL_METHOD(NULL, getThis(), "onclose", &client);
						}
						ev.events = EPOLLOUT | EPOLLHUP;
						epoll_ctl(epollfd, EPOLL_CTL_DEL, event_fd, &ev);
						close(event_fd);
						phalcon_unset_property_array(getThis(), SL("_clients"), &client_socket_id);
					}
				} else if (events[i].events & EPOLLOUT) {
					PHALCON_CALL_METHOD(&client, getThis(), "getclient", &client_socket_id);
					if (Z_TYPE(onsend) > IS_NULL) {
						args = (zval *)safe_emalloc(1, sizeof(zval), 0);
						ZVAL_COPY(&args[0], &client);
						PHALCON_CALL_USER_FUNC_ARGS(&ret, &onsend, args, 1);
					} else if (phalcon_method_exists_ex(getThis(), SL("onsend")) == SUCCESS) {
						PHALCON_CALL_METHOD(&ret, getThis(), "onsend", &client, &data);
					}
					ev.data.fd = event_fd;
					if (PHALCON_IS_FALSE(&ret)) {
						if (Z_TYPE(onclose) > IS_NULL) {
							args = (zval *)safe_emalloc(1, sizeof(zval), 0);
							ZVAL_COPY(&args[0], &client);
							PHALCON_CALL_USER_FUNC_ARGS(NULL, &onclose, args, 1);
						} else if (phalcon_method_exists_ex(getThis(), SL("onclose")) == SUCCESS) {
							PHALCON_CALL_METHOD(NULL, getThis(), "onclose", &client);
						}
						ev.events = EPOLLOUT | EPOLLHUP;
						epoll_ctl(epollfd, EPOLL_CTL_DEL, event_fd, &ev);
						close(event_fd);
						phalcon_unset_property_array(getThis(), SL("_clients"), &client_socket_id);
					} else {
						ev.data.fd = event_fd;
						ev.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLET;
						epoll_ctl_ret = epoll_ctl(epollfd, EPOLL_CTL_MOD, event_fd, &ev);
						if (epoll_ctl_ret < 0) {
							close(client_fd);
						}
					}
				}
			}
		}
	}
#endif

	phalcon_socket_server_destroy();
#else
	PHALCON_THROW_EXCEPTION_STR(phalcon_socket_exception_ce, "Don't support, please reinstall.");
#endif
}
