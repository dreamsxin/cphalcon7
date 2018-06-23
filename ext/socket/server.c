
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

#include <sys/wait.h>

#include <Zend/zend_closures.h>

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
 *  $server = new Phalcon\Socket\Server('127.0.0.1', 8989);
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
 * class HttpServer extends Phalcon\Socket\Server {
 * 
 * 	  public function onTimeout() {
 * 	  }
 * 
 * 	  public function onError(Phalcon\Socket\Client $client) {
 * 	  }
 * 
 * 	  public function onConnection(Phalcon\Socket\Client $client) {
 * 	  }
 * 
 * 	  public function onRecv(Phalcon\Socket\Client $client) {
 * 	  }
 * 
 * 	  public function onSend(Phalcon\Socket\Client $client) {
 * 		  $client->write("HTTP/1.0 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nConnection: close\r\n\r\nHello World");
 * 		  return FALSE;
 * 	  }
 * 
 * 	  public function onClose(Phalcon\Socket\Client $client) {
 * 	  }
 * }
 * 
 * $server = new HttpServer('localhost', 6000);
 * $server->setOption(Phalcon\Socket::SOL_SOCKET, SO_REUSEADDR, 1);
 * $server->setOption(Phalcon\Socket::SOL_SOCKET, SO_REUSEPORT, 1);
 * $server->setOption(Phalcon\Socket::SOL_TCP, Phalcon\Socket::TCP_NODELAY, 1);
 * $server->setOption(Phalcon\Socket::SOL_TCP, Phalcon\Socket::TCP_QUICKACK, 1);
 * $server->run();
 *</code>
 */
zend_class_entry *phalcon_socket_server_ce;

PHP_METHOD(Phalcon_Socket_Server, __construct);
PHP_METHOD(Phalcon_Socket_Server, setTimeout);
PHP_METHOD(Phalcon_Socket_Server, setDaemon);
PHP_METHOD(Phalcon_Socket_Server, setMaxChildren);
PHP_METHOD(Phalcon_Socket_Server, setEvent);
PHP_METHOD(Phalcon_Socket_Server, getEvent);
PHP_METHOD(Phalcon_Socket_Server, listen);
PHP_METHOD(Phalcon_Socket_Server, accept);
PHP_METHOD(Phalcon_Socket_Server, getClients);
PHP_METHOD(Phalcon_Socket_Server, getClient);
PHP_METHOD(Phalcon_Socket_Server, removeClient);
PHP_METHOD(Phalcon_Socket_Server, disconnect);
PHP_METHOD(Phalcon_Socket_Server, run);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server___construct, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, domain, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, protocol, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_settimeout, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, sec, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, usec, IS_LONG, 1)
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
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_removeclient, 0, 0, 1)
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_disconnect, 0, 0, 1)
	ZEND_ARG_INFO(0, socket)
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
	PHP_ME(Phalcon_Socket_Server, setTimeout, arginfo_phalcon_socket_server_settimeout, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, setDaemon, arginfo_phalcon_socket_server_setdaemon, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, setMaxChildren, arginfo_phalcon_socket_server_setmaxchildren, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, setEvent, arginfo_phalcon_socket_server_setevent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, getEvent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, listen, arginfo_phalcon_socket_server_listen, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, accept, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, getClients, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, getClient, arginfo_phalcon_socket_server_getclient, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, removeClient, arginfo_phalcon_socket_server_removeclient, ZEND_ACC_PUBLIC)
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
	zend_declare_property_long(phalcon_socket_server_ce, SL("_tv_sec"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_socket_server_ce, SL("_tv_usec"), 0, ZEND_ACC_PROTECTED);

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
				if ( Z_LVAL_P(port) > 0) {
					ZVAL_LONG(&domain, PHALCON_SOCKET_AF_INET);
				} else {
					ZVAL_LONG(&domain, PHALCON_SOCKET_AF_UNIX);
				}
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
	zval_ptr_dtor(&socket);

	phalcon_update_property(getThis(), SL("_address"), address);
	phalcon_update_property(getThis(), SL("_port"), port);

	phalcon_update_property_empty_array(getThis(), SL("_clients"));
}

/**
 * Sets the timeout
 */
PHP_METHOD(Phalcon_Socket_Server, setTimeout){

	zval *sec, *usec = NULL;

	phalcon_fetch_params(0, 1, 1, &sec, &usec);

	phalcon_update_property(getThis(), SL("_tv_sec"), sec);
	if (usec && Z_TYPE_P(usec) == IS_LONG) {
		phalcon_update_property(getThis(), SL("_tv_usec"), usec);
	}

	RETURN_THIS();
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
 * @param resource $socket
 * @return Phalcon\Socket\Client
 */
PHP_METHOD(Phalcon_Socket_Server, accept){

	zval socket = {}, client_socket = {};

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_FUNCTION(&client_socket, "socket_accept", &socket);

	if (PHALCON_IS_FALSE(&client_socket)) {
		RETURN_FALSE;
	} else {
		object_init_ex(return_value, phalcon_socket_client_ce);
		PHALCON_CALL_METHOD(NULL, return_value, "__construct", &client_socket);

		phalcon_update_property_array_append(getThis(), SL("_clients"), return_value);
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
 * @param resource $socket
 * @return Phalcon\Socket\Client
 */
PHP_METHOD(Phalcon_Socket_Server, getClient){

	zval *socket, clients = {}, *client;

	phalcon_fetch_params(1, 1, 0, &socket);

	phalcon_read_property(&clients, getThis(), SL("_clients"), PH_READONLY);
	
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(clients), client) {
		zval client_socket = {};
		PHALCON_MM_CALL_METHOD(&client_socket, client, "getsocket");
		PHALCON_MM_ADD_ENTRY(&client_socket);
		if (phalcon_compare(&client_socket, socket) == 0) {
			RETURN_MM_CTOR(client);
		}
	} ZEND_HASH_FOREACH_END();
	RETURN_MM_NULL();
}

/**
 * Remove a connection
 *
 * @param resource $socket
 * @return boolean
 */
PHP_METHOD(Phalcon_Socket_Server, removeClient){

	zval *socket, clients = {}, *client;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 1, 0, &socket);

	phalcon_read_property(&clients, getThis(), SL("_clients"), PH_READONLY);
	
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(clients), idx, str_key, client) {
		zval tmp = {}, client_socket = {};
		if (str_key) {
			ZVAL_STR(&tmp, str_key);
		} else {
			ZVAL_LONG(&tmp, idx);
		}
		PHALCON_MM_CALL_METHOD(&client_socket, client, "getsocket");
		PHALCON_MM_ADD_ENTRY(&client_socket);
		if (phalcon_compare(&client_socket, socket) == 0) {
			phalcon_array_unset(&clients, &tmp, 0);
			RETURN_MM_TRUE;
		}
	} ZEND_HASH_FOREACH_END();
	RETURN_MM_FALSE;
}

/**
 * Close a client
 *
 * @param resource $socket
 * @return Phalcon\Socket\Server
 */
PHP_METHOD(Phalcon_Socket_Server, disconnect){

	zval *socket, clients = {}, *client;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 1, 0, &socket);

	phalcon_read_property(&clients, getThis(), SL("_clients"), PH_READONLY);
	
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(clients), idx, str_key, client) {
		zval tmp = {}, client_socket = {};
		if (str_key) {
			ZVAL_STR(&tmp, str_key);
		} else {
			ZVAL_LONG(&tmp, idx);
		}
		PHALCON_MM_CALL_METHOD(&client_socket, client, "getsocket");
		PHALCON_MM_ADD_ENTRY(&client_socket);
		if (phalcon_compare(&client_socket, socket) == 0) {
			PHALCON_CALL_METHOD(NULL, client, "close");
			phalcon_array_unset(&clients, &tmp, 0);
			break;
		}
	} ZEND_HASH_FOREACH_END();
	RETURN_MM_THIS();
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

struct _phalcon_socket_server {
	zval socket;
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

	//PHALCON_CALL_FUNCTION(NULL, "socket_close", &server->socket);
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

/**
 * Run the Server
 *
 */
PHP_METHOD(Phalcon_Socket_Server, run)
{
	zval *_onconnection = NULL, *_onrecv = NULL, *_onsend = NULL, *_onclose = NULL, *_onerror = NULL, *_ontimeout = NULL, *_timeout = NULL, *_usec = NULL;
	zval onconnection = {}, onrecv = {}, onsend = {}, onclose = {}, onerror = {}, ontimeout, timeout = {}, usec = {};
	zval listensocket = {}, maxlen = {}, event = {};
	zval daemon = {}, max_children = {}, *msg_dontwait;
	int flag = 0;

	phalcon_fetch_params(1, 0, 8, &_onconnection, &_onrecv, &_onsend, &_onclose, &_onerror, &_ontimeout, &timeout, &usec);

	if (_onconnection) {
		if (Z_TYPE_P(_onconnection) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onconnection), zend_ce_closure, 0)) {
			PHALCON_MM_CALL_CE_STATIC(&onconnection, zend_ce_closure, "bind", _onconnection, getThis());
			PHALCON_MM_ADD_ENTRY(&onconnection);
		} else {
			PHALCON_MM_ZVAL_COPY(&onconnection, _onconnection);
		}
	}

	if (_onrecv) {
		if (Z_TYPE_P(_onrecv) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onrecv), zend_ce_closure, 0)) {
			PHALCON_MM_CALL_CE_STATIC(&onrecv, zend_ce_closure, "bind", _onrecv, getThis());
			PHALCON_MM_ADD_ENTRY(&onrecv);
		} else {
			PHALCON_MM_ZVAL_COPY(&onrecv, _onrecv);
		}
	}

	if (_onsend) {
		if (Z_TYPE_P(_onsend) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onsend), zend_ce_closure, 0)) {
			PHALCON_MM_CALL_CE_STATIC(&onsend, zend_ce_closure, "bind", _onsend, getThis());
			PHALCON_MM_ADD_ENTRY(&onsend);
		} else {
			PHALCON_MM_ZVAL_COPY(&onsend, _onsend);
		}
	}

	if (_onclose) {
		if (Z_TYPE_P(_onclose) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onclose), zend_ce_closure, 0)) {
			PHALCON_MM_CALL_CE_STATIC(&onclose, zend_ce_closure, "bind", _onclose, getThis());
			PHALCON_MM_ADD_ENTRY(&onclose);
		} else {
			PHALCON_MM_ZVAL_COPY(&onclose, _onclose);
		}
	}

	if (_onerror) {
		if (Z_TYPE_P(_onerror) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onerror), zend_ce_closure, 0)) {
			PHALCON_MM_CALL_CE_STATIC(&onerror, zend_ce_closure, "bind", _onerror, getThis());
			PHALCON_MM_ADD_ENTRY(&onerror);
		} else {
			PHALCON_MM_ZVAL_COPY(&onerror, _onerror);
		}
	}

	if (_ontimeout) {
		if (Z_TYPE_P(_ontimeout) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_ontimeout), zend_ce_closure, 0)) {
			PHALCON_MM_CALL_CE_STATIC(&ontimeout, zend_ce_closure, "bind", _ontimeout, getThis());
			PHALCON_MM_ADD_ENTRY(&ontimeout);
		} else {
			PHALCON_MM_ZVAL_COPY(&ontimeout, _ontimeout);
		}
	}

	if (!_timeout || Z_TYPE_P(_timeout) != IS_LONG) {
		phalcon_read_property(&timeout, getThis(), SL("_tv_sec"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&timeout, _timeout);
	}

	if (!_usec || Z_TYPE_P(_usec) != IS_LONG) {
		phalcon_read_property(&usec, getThis(), SL("_tv_usec"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&usec, _usec);
	}

	if ((msg_dontwait = zend_get_constant_str(SL("MSG_DONTWAIT"))) == NULL) {
		phalcon_socket_server_destroy();
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_socket_exception_ce, "Can't get constant MSG_DONTWAIT");
		return;
	}

	phalcon_read_property(&listensocket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&maxlen, getThis(), SL("_maxlen"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&event, getThis(), SL("_event"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&daemon, getThis(), SL("_daemon"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&max_children, getThis(), SL("_maxChildren"), PH_NOISY|PH_READONLY);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "listen");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "setblocking", &PHALCON_GLOBAL(z_false));

	phalcon_socket_server_init();
	ZVAL_COPY_VALUE(&server->socket, &listensocket);

	if (zend_is_true(&daemon) && !phalcon_socket_server_start_daemon()) {
		phalcon_socket_server_destroy();
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_socket_exception_ce, "Failed to setup daemon");
		return;
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
		RETURN_MM_FALSE;
	}
worker:
	while(server->running) {
		zval r_array = {}, w_array = {}, e_array = {}, clients = {}, ret = {}, *client = NULL, *client_socket = NULL;
		
		array_init(&r_array);
		array_init(&w_array);
		array_init(&e_array);

		phalcon_read_property(&clients, getThis(), SL("_clients"), PH_NOISY|PH_READONLY);

		phalcon_array_append(&r_array, &listensocket, PH_COPY);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(clients), client) {
			zval client_socket = {};
			PHALCON_MM_CALL_METHOD(&client_socket, client, "getsocket");
			phalcon_array_append(&r_array, &client_socket, 0);
			//phalcon_array_append(&w_array, &client_socket, 0);
		} ZEND_HASH_FOREACH_END();

		ZVAL_MAKE_REF(&r_array);
		ZVAL_MAKE_REF(&w_array);
		ZVAL_MAKE_REF(&e_array);
		PHALCON_MM_CALL_FUNCTION(&ret, "socket_select", &r_array, &w_array, &e_array, &timeout, &usec);
		ZVAL_UNREF(&r_array);
		ZVAL_UNREF(&w_array);
		ZVAL_UNREF(&e_array);

		if (PHALCON_IS_FALSE(&ret)) {
			zval_ptr_dtor(&r_array);
			zval_ptr_dtor(&w_array);
			zval_ptr_dtor(&e_array);
			phalcon_socket_server_destroy();
			RETURN_MM();
		}
		if (PHALCON_IS_LONG_IDENTICAL(&ret, 0)) {
			if (phalcon_method_exists_ex(getThis(), SL("ontimeout")) == SUCCESS) {
				PHALCON_MM_CALL_METHOD(NULL, getThis(), "ontimeout");
			}
			if (Z_TYPE(ontimeout) > IS_NULL) {
				PHALCON_MM_CALL_USER_FUNC(NULL, &ontimeout);
			}
			zval_ptr_dtor(&r_array);
			zval_ptr_dtor(&w_array);
			zval_ptr_dtor(&e_array);
			continue;
		}
		zval_ptr_dtor(&ret);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(r_array), client_socket) {
			zval *args, ret2 = {};

			if (phalcon_compare(client_socket, &listensocket) == 0) {
				zval clientsocket = {};
				PHALCON_CALL_FUNCTION(&clientsocket, "socket_accept", &listensocket);
				if (Z_TYPE(clientsocket) == IS_RESOURCE) {
					zval tmp_client = {};
		
					object_init_ex(&tmp_client, phalcon_socket_client_ce);
					PHALCON_CALL_METHOD(NULL, &tmp_client, "__construct", &clientsocket);
					PHALCON_CALL_METHOD(NULL, &tmp_client, "setblocking", &PHALCON_GLOBAL(z_false));
					PHALCON_CALL_METHOD(NULL, &tmp_client, "keepalive");
					phalcon_array_append(&clients, &tmp_client, 0);

					if (Z_TYPE(onconnection) > IS_NULL) {
						args = (zval *)safe_emalloc(1, sizeof(zval), 0);
						ZVAL_COPY_VALUE(&args[0], &tmp_client);
						PHALCON_CALL_USER_FUNC_ARGS(NULL, &onconnection, args, 1);
						efree(args);
					} else if (phalcon_method_exists_ex(getThis(), SL("onconnection")) == SUCCESS) {
						PHALCON_CALL_METHOD(NULL, getThis(), "onconnection", &tmp_client);
					}
				}
				continue;
			} else {
				zval tmp_client = {};
				int status = 0;
				PHALCON_MM_CALL_METHOD(&tmp_client, getThis(), "getclient", client_socket);
				PHALCON_MM_ADD_ENTRY(&tmp_client);
				while(1) {
					zval bytes = {}, buf = {};
					
					ZVAL_MAKE_REF(&buf);
					PHALCON_MM_CALL_FUNCTION(&bytes, "socket_recv", client_socket, &buf, &maxlen, msg_dontwait);
					ZVAL_UNREF(&buf);

					if (PHALCON_LT_LONG(&bytes, 0)) {
						if(errno == EAGAIN || errno == EWOULDBLOCK) {
							status = 1;
							break;
						}
						if (Z_TYPE(onerror) > IS_NULL) {
							args = (zval *)safe_emalloc(1, sizeof(zval), 0);
							ZVAL_COPY_VALUE(&args[0], &tmp_client);
							PHALCON_CALL_USER_FUNC_ARGS_FLAG(flag, NULL, &onerror, args, 1);
							efree(args);
						} else if (phalcon_method_exists_ex(getThis(), SL("onerror")) == SUCCESS) {
							PHALCON_MM_CALL_METHOD(NULL, getThis(), "onerror", &tmp_client);
						}
						status = -1;
						break;
					} else if (PHALCON_LE_LONG(&bytes, 0)) {
						zval_ptr_dtor(&buf);
						break;
					} else {
						status = 1;
						if (Z_TYPE(onrecv) > IS_NULL) {
							args = (zval *)safe_emalloc(2, sizeof(zval), 0);
							ZVAL_COPY_VALUE(&args[0], &tmp_client);
							ZVAL_COPY_VALUE(&args[1], &buf);
							PHALCON_CALL_USER_FUNC_ARGS_FLAG(flag, &ret2, &onrecv, args, 2);
							efree(args);
						} else if (phalcon_method_exists_ex(getThis(), SL("onrecv")) == SUCCESS) {
							PHALCON_MM_CALL_METHOD(&ret2, getThis(), "onrecv", &tmp_client, &buf);
						}

						if (PHALCON_IS_FALSE(&ret2)) {
							status = -1;
							break;
						}
						zval_ptr_dtor(&ret2);
					}
					zval_ptr_dtor(&buf);
				}
				if (status <= 0) {
					if (Z_TYPE(onclose) > IS_NULL) {
						args = (zval *)safe_emalloc(1, sizeof(zval), 0);
						ZVAL_COPY_VALUE(&args[0], &tmp_client);
						PHALCON_CALL_USER_FUNC_ARGS_FLAG(flag, NULL, &onclose, args, 1);
						efree(args);
					} else if (phalcon_method_exists_ex(getThis(), SL("onclose")) == SUCCESS) {
						PHALCON_MM_CALL_METHOD(NULL, getThis(), "onclose", &tmp_client);
					}

					PHALCON_MM_CALL_METHOD(NULL, getThis(), "removeclient", client_socket);
					PHALCON_MM_CALL_METHOD(NULL, &tmp_client, "shutdown");
					PHALCON_MM_CALL_METHOD(NULL, &tmp_client, "close");
				} else {
					if (Z_TYPE(onsend) > IS_NULL) {
						args = (zval *)safe_emalloc(1, sizeof(zval), 0);
						ZVAL_COPY_VALUE(&args[0], &tmp_client);
						PHALCON_CALL_USER_FUNC_ARGS_FLAG(flag, &ret2, &onsend, args, 1);
						efree(args);
					} else if (phalcon_method_exists_ex(getThis(), SL("onsend")) == SUCCESS) {
						PHALCON_CALL_METHOD_FLAG(flag, &ret2, getThis(), "onsend", &tmp_client);
					}
					if (flag == FAILURE || PHALCON_IS_FALSE(&ret2)) {
						if (Z_TYPE(onclose) > IS_NULL) {
							args = (zval *)safe_emalloc(1, sizeof(zval), 0);
							ZVAL_COPY_VALUE(&args[0], &tmp_client);
							PHALCON_CALL_USER_FUNC_ARGS_FLAG(flag, NULL, &onclose, args, 1);
							efree(args);
						} else if (phalcon_method_exists_ex(getThis(), SL("onclose")) == SUCCESS) {
							PHALCON_CALL_METHOD_FLAG(flag, NULL, getThis(), "onclose", &tmp_client);
						}
						PHALCON_MM_CALL_METHOD(NULL, getThis(), "removeclient", client_socket);

						PHALCON_MM_CALL_METHOD(NULL, &tmp_client, "shutdown");
						PHALCON_MM_CALL_METHOD(NULL, &tmp_client, "close");
					} 
					zval_ptr_dtor(&ret2);
				}
			}
		} ZEND_HASH_FOREACH_END();
		zval_ptr_dtor(&r_array);
		zval_ptr_dtor(&w_array);
		zval_ptr_dtor(&e_array);
	}

	phalcon_socket_server_destroy();
	RETURN_MM();
}
