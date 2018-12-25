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

#include "queue/beanstalk.h"
#include "queue/beanstalk/job.h"
#include "queue/../exception.h"

#include <ext/standard/file.h>
#include <main/php_streams.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/variables.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/operators.h"

/**
 * Phalcon\Queue\Beanstalk
 *
 * Class to access the beanstalk queue service.
 * Partially implements the protocol version 1.2
 *
 * @see http://www.igvita.com/2010/05/20/scalable-work-queues-with-beanstalk/
 */
zend_class_entry *phalcon_queue_beanstalk_ce;

PHP_METHOD(Phalcon_Queue_Beanstalk, __construct);
PHP_METHOD(Phalcon_Queue_Beanstalk, connect);
PHP_METHOD(Phalcon_Queue_Beanstalk, put);
PHP_METHOD(Phalcon_Queue_Beanstalk, reserve);
PHP_METHOD(Phalcon_Queue_Beanstalk, choose);
PHP_METHOD(Phalcon_Queue_Beanstalk, watch);
PHP_METHOD(Phalcon_Queue_Beanstalk, stats);
PHP_METHOD(Phalcon_Queue_Beanstalk, statsTube);
PHP_METHOD(Phalcon_Queue_Beanstalk, peekReady);
PHP_METHOD(Phalcon_Queue_Beanstalk, peekDelayed);
PHP_METHOD(Phalcon_Queue_Beanstalk, peekBuried);
PHP_METHOD(Phalcon_Queue_Beanstalk, jobPeek);
PHP_METHOD(Phalcon_Queue_Beanstalk, readStatus);
PHP_METHOD(Phalcon_Queue_Beanstalk, readYaml);
PHP_METHOD(Phalcon_Queue_Beanstalk, read);
PHP_METHOD(Phalcon_Queue_Beanstalk, write);
PHP_METHOD(Phalcon_Queue_Beanstalk, disconnect);
PHP_METHOD(Phalcon_Queue_Beanstalk, quit);
PHP_METHOD(Phalcon_Queue_Beanstalk, listTubes);
PHP_METHOD(Phalcon_Queue_Beanstalk, listTubeUsed);
PHP_METHOD(Phalcon_Queue_Beanstalk, listTubesWatched);
PHP_METHOD(Phalcon_Queue_Beanstalk, ignore);
PHP_METHOD(Phalcon_Queue_Beanstalk, __sleep);
PHP_METHOD(Phalcon_Queue_Beanstalk, __wakeup);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk_put, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk_reserve, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk_choose, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, tube, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk_watch, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, tube, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk_statstube, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, tube, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk_read, 0, 0, 0)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk_write, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk_ignore, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, tube, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk_jobpeek, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, id, IS_LONG, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_queue_beanstalk_method_entry[] = {
	PHP_ME(Phalcon_Queue_Beanstalk, __construct, arginfo_phalcon_queue_beanstalk___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Queue_Beanstalk, connect, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, put, arginfo_phalcon_queue_beanstalk_put, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, reserve, arginfo_phalcon_queue_beanstalk_reserve, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, choose, arginfo_phalcon_queue_beanstalk_choose, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, watch, arginfo_phalcon_queue_beanstalk_watch, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, stats, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, statsTube, arginfo_phalcon_queue_beanstalk_statstube, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, peekReady, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, peekDelayed, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, peekBuried, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, jobPeek, arginfo_phalcon_queue_beanstalk_jobpeek, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, readStatus, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, readYaml, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, read, arginfo_phalcon_queue_beanstalk_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, write, arginfo_phalcon_queue_beanstalk_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, disconnect, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, quit, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, listTubes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, listTubeUsed, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, listTubesWatched, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, ignore, arginfo_phalcon_queue_beanstalk_ignore, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, __sleep, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk, __wakeup, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Queue\Beanstalk initializer
 */
PHALCON_INIT_CLASS(Phalcon_Queue_Beanstalk){

	PHALCON_REGISTER_CLASS(Phalcon\\Queue, Beanstalk, queue_beanstalk, phalcon_queue_beanstalk_method_entry, 0);

	zend_declare_property_null(phalcon_queue_beanstalk_ce, SL("_connection"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_queue_beanstalk_ce, SL("_parameters"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Queue\Beanstalk
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, __construct){

	zval *options = NULL, parameters = {};

	phalcon_fetch_params(1, 0, 1, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(options) != IS_ARRAY) {
		array_init_size(&parameters, 2);
	} else {
		ZVAL_DUP(&parameters, options);
	}
	PHALCON_MM_ADD_ENTRY(&parameters);

	if (!phalcon_array_isset_str(&parameters, SL("host"))) {
		phalcon_array_update_str_str(&parameters, SL("host"), SL("127.0.0.1"), 0);
	}

	if (!phalcon_array_isset_str(&parameters, SL("port"))) {
		phalcon_array_update_str_long(&parameters, SL("port"), 11300, 0);
	}

	phalcon_update_property(getThis(), SL("_parameters"), &parameters);
	RETURN_MM();
}

PHP_METHOD(Phalcon_Queue_Beanstalk, connect){

	zval connection = {}, parameters = {}, host = {}, port = {}, new_connection = {};

	PHALCON_MM_INIT();

	phalcon_read_property(&connection, getThis(), SL("_connection"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(connection) == IS_RESOURCE) {
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "disconnect");
	}

	phalcon_read_property(&parameters, getThis(), SL("_parameters"), PH_NOISY|PH_READONLY);

	if (!phalcon_array_isset_fetch_str(&host, &parameters, SL("host"), PH_READONLY)
		|| !phalcon_array_isset_fetch_str(&port, &parameters, SL("port"), PH_READONLY)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_exception_ce, "Unexpected inconsistency in options");
		return;
	}

	convert_to_string(&host);
	convert_to_long(&port);
	ZVAL_NULL(&new_connection);
	{
		ulong timeout = (ulong)(FG(default_socket_timeout) * 1000000.0);
		char *hostname;
		long int hostname_len = spprintf(&hostname, 0, "%s:%ld", Z_STRVAL(host), Z_LVAL(port));
		struct timeval tv;
		php_stream *stream;
		int err;
		zend_string *errstr = NULL;

		tv.tv_sec  = timeout / 1000000;
		tv.tv_usec = timeout % 1000000;

		stream = php_stream_xport_create(hostname, hostname_len, REPORT_ERRORS, STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, NULL, &tv, NULL, &errstr, &err);
		efree(hostname);

		if (!stream) {
			zend_throw_exception_ex(phalcon_exception_ce, err, "Unable to connect to Beanstalk server at %s:%ld (%s)", Z_STRVAL(host), Z_LVAL(port), (errstr == NULL ? "Unknown error" : errstr->val));
		}

		if (errstr) {
			zend_string_release(errstr);
		}

		if (!stream) {
			RETURN_MM_NULL();
		}

		tv.tv_sec  = -1;
		tv.tv_usec = 0;
		php_stream_set_option(stream, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &tv);

		php_stream_to_zval(stream, &new_connection);
		phalcon_update_property(getThis(), SL("_connection"), &new_connection);
	}
	RETURN_MM_NCTOR(&new_connection);
}

/**
 * Inserts jobs into the queue
 *
 * @param string $data
 * @param array $options
 * @return string|boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, put){

	zval *data, *options = NULL, priority = {}, delay = {}, ttr = {}, serialized = {}, serialized_length = {}, command = {}, response = {}, status = {}, job_id = {};

	phalcon_fetch_params(1, 1, 1, &data, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	/**
	 * Priority is 100 by default
	 */
	if (!phalcon_array_isset_fetch_str(&priority, options, SL("priority"), PH_READONLY)) {
		PHALCON_MM_ZVAL_STRING(&priority, "100");
	}

	if (!phalcon_array_isset_fetch_str(&delay, options, SL("delay"), PH_READONLY)) {
		PHALCON_MM_ZVAL_STRING(&delay, "0");
	}

	if (!phalcon_array_isset_fetch_str(&ttr, options, SL("ttr"), PH_READONLY)) {
		PHALCON_MM_ZVAL_STRING(&ttr, "86400");
	}

	/**
	 * Data is automatically serialized before be sent to the server
	 */
	phalcon_serialize(&serialized, data);
	PHALCON_MM_ADD_ENTRY(&serialized);
	if (Z_TYPE(serialized) == IS_STRING) {
		ZVAL_LONG(&serialized_length, Z_STRLEN(serialized));
	} else {
		RETURN_MM_FALSE;
	}

	/**
	 * Create the command
	 */
	PHALCON_CONCAT_SVSV(&command, "put ", &priority, " ", &delay);
	PHALCON_MM_ADD_ENTRY(&command);
	PHALCON_SCONCAT_SVSV(&command, " ", &ttr, " ", &serialized_length);
	PHALCON_MM_ADD_ENTRY(&command);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &serialized);

	PHALCON_MM_CALL_METHOD(&response, getThis(), "readstatus");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "INSERTED")) {
		phalcon_array_fetch_long(&job_id, &response, 1, PH_NOISY|PH_READONLY);
		RETURN_MM_CTOR(&job_id);
	} else if (PHALCON_IS_STRING(&status, "BURIED")) {
		phalcon_array_fetch_long(&job_id, &response, 1, PH_NOISY|PH_READONLY);
		RETURN_MM_CTOR(&job_id);
	}
	RETURN_MM_FALSE;;
}

/**
 * Reserves a job in the queue
 *
 * @return boolean|\Phalcon\Queue\Beanstalk\Job
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, reserve){

	zval *timeout = NULL, command = {}, response = {}, status = {}, job_id = {}, length = {}, serialized_body = {}, body = {};

	phalcon_fetch_params(1, 0, 1, &timeout);

	if (!timeout) {
		timeout = &PHALCON_GLOBAL(z_null);
	}

	if (zend_is_true(timeout)) {
		PHALCON_CONCAT_SV(&command, "reserve-with-timeout ", timeout);
	} else {
		ZVAL_STRING(&command, "reserve");
	}
	PHALCON_MM_ADD_ENTRY(&command);
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readstatus");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "RESERVED")) {
		/**
		 * The job is in the first position
		 */
		phalcon_array_fetch_long(&job_id, &response, 1, PH_NOISY|PH_READONLY);

		/**
		 * Next is the job length
		 */
		phalcon_array_fetch_long(&length, &response, 2, PH_NOISY|PH_READONLY);

		/**
		 * The body is serialized
		 */
		PHALCON_MM_CALL_METHOD(&serialized_body, getThis(), "read", &length);
		PHALCON_MM_ADD_ENTRY(&serialized_body);
		phalcon_unserialize(&body, &serialized_body);
		PHALCON_MM_ADD_ENTRY(&body);

		/**
		 * Create a beanstalk job abstraction
		 */
		object_init_ex(return_value, phalcon_queue_beanstalk_job_ce);
		PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", getThis(), &job_id, &body);
		RETURN_MM();
	}
	RETURN_MM_FALSE;
}

/**
 * Change the active tube. By default the tube is 'default'
 *
 * @param string $tube
 * @return string|boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, choose){

	zval *tube, command = {}, response = {}, status = {}, using_tube = {};

	phalcon_fetch_params(1, 1, 0, &tube);

	PHALCON_CONCAT_SV(&command, "use ", tube);
	PHALCON_MM_ADD_ENTRY(&command);
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readstatus");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "USING")) {
		phalcon_array_fetch_long(&using_tube, &response, 1, PH_NOISY|PH_READONLY);
		RETURN_MM_CTOR(&using_tube);
	}
	RETURN_MM_FALSE;
}

/**
 * Change the active tube. By default the tube is 'default'
 *
 * @param string $tube
 * @return string|boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, watch){

	zval *tube, command = {}, response = {}, status = {}, watching_tube = {};

	phalcon_fetch_params(1, 1, 0, &tube);

	PHALCON_CONCAT_SV(&command, "watch ", tube);
	PHALCON_MM_ADD_ENTRY(&command);
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readstatus");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "WATCHING")) {
		phalcon_array_fetch_long(&watching_tube, &response, 1, PH_NOISY|PH_READONLY);
		RETURN_MM_CTOR(&watching_tube);
	}
	RETURN_MM_FALSE;
}

/**
 * Get stats of the Beanstalk server.
 *
 * @return boolean|array
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, stats){

	zval command = {}, response = {}, status = {}, stats = {};

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&command, "stats");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readyaml");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "OK")) {
		phalcon_array_fetch_long(&stats, &response, 2, PH_NOISY|PH_READONLY);
		RETURN_MM_CTOR(&stats);
	}
	RETURN_MM_FALSE;
}

/**
 * Get stats of a tube
 *
 * @param string $tube
 * @return boolean|array
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, statsTube){

	zval *tube, command = {}, response = {}, status = {}, stats_tube = {};

	phalcon_fetch_params(1, 1, 0, &tube);

	PHALCON_CONCAT_SV(&command, "stats-tube ", tube);
	PHALCON_MM_ADD_ENTRY(&command);
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readyaml");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "OK")) {
		phalcon_array_fetch_long(&stats_tube, &response, 2, PH_NOISY|PH_READONLY);
		RETURN_MM_CTOR(&stats_tube);
	}
	RETURN_MM_FALSE;
}

static inline void phalcon_queue_beanstalk_peek_common(zval *return_value, zval *this_ptr, zval *response)
{
	zval job_id = {}, length = {}, serialized = {}, body = {};

	if (!phalcon_array_isset_fetch_long(&job_id, response, 1, PH_READONLY)) {
		ZVAL_NULL(&job_id);
	}

	if (!phalcon_array_isset_fetch_long(&length, response, 2, PH_READONLY)) {
		ZVAL_NULL(&length);
	}

	PHALCON_CALL_METHOD(&serialized, this_ptr, "read", &length);

	phalcon_unserialize(&body, &serialized);
	zval_ptr_dtor(&serialized);

	object_init_ex(return_value, phalcon_queue_beanstalk_job_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", this_ptr, &job_id, &body);
	zval_ptr_dtor(&body);
}

/**
 * Inspect the next ready job.
 *
 * @return boolean|\Phalcon\Queue\Beanstalk\Job
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, peekReady){

	zval command = {}, response = {}, status = {};

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&command, "peek-ready");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readstatus");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "FOUND")) {
		phalcon_queue_beanstalk_peek_common(return_value, getThis(), &response);
		RETURN_MM();
	}
	RETURN_MM_FALSE;
}

/**
 * Return the delayed job with the shortest delay left
 *
 * @return boolean|Phalcon\Queue\Beanstalk\Job
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, peekDelayed){

	zval command = {}, response = {}, status = {};

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&command, "peek-delayed");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readstatus");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "FOUND")) {
		phalcon_queue_beanstalk_peek_common(return_value, getThis(), &response);
		RETURN_MM();
	}
	RETURN_MM_FALSE;
}

/**
 * Return the next job in the list of buried jobs
 *
 * @return boolean|Phalcon\Queue\Beanstalk\Job
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, peekBuried){

	zval command = {}, response = {}, status = {};

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&command, "peek-buried");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readstatus");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "FOUND")) {
		phalcon_queue_beanstalk_peek_common(return_value, getThis(), &response);
		RETURN_MM();
	}
	RETURN_MM_FALSE;
}

/**
 * The peek commands let the client inspect a job in the system.
 *
 * @return boolean|\Phalcon\Queue\Beanstalk\Job
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, jobPeek){

	zval *id, command = {}, response = {}, status = {};

	phalcon_fetch_params(1, 1, 0, &id);

	PHALCON_CONCAT_SV(&command, "peek ", id);
	PHALCON_MM_ADD_ENTRY(&command);
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readstatus");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "FOUND")) {
		phalcon_queue_beanstalk_peek_common(return_value, getThis(), &response);
		RETURN_MM();
	}
	RETURN_MM_FALSE;
}

/**
 * Reads the latest status from the Beanstalkd server
 *
 * @return array
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, readStatus){

	zval response = {};

	PHALCON_CALL_METHOD(&response, getThis(), "read");
	phalcon_fast_explode_str(return_value, SL(" "), &response);
	zval_ptr_dtor(&response);
}

/**
 * Fetch a YAML payload from the Beanstalkd server
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, readYaml){

	zval response = {}, status = {}, num_bytes = {}, yaml_data = {}, data = {};

	PHALCON_MM_INIT();

	PHALCON_MM_CALL_METHOD(&response, getThis(), "readstatus");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);

	if (phalcon_array_isset_fetch_long(&num_bytes, &response, 1, PH_READONLY)) {
		PHALCON_MM_CALL_METHOD(&yaml_data, getThis(), "read", &num_bytes);
		PHALCON_MM_ADD_ENTRY(&yaml_data);
		PHALCON_MM_CALL_FUNCTION(&data, "yaml_parse", &yaml_data);
	} else {
		ZVAL_LONG(&num_bytes, 0);
		array_init(&data);
	}
	PHALCON_MM_ADD_ENTRY(&data);

	array_init_size(return_value, 3);
	phalcon_array_append(return_value, &status, PH_COPY);
	phalcon_array_append(return_value, &num_bytes, PH_COPY);
	phalcon_array_append(return_value, &data, PH_COPY);

	RETURN_MM();
}

/**
 * Reads a packet from the socket. Prior to reading from the socket will
 * check for availability of the connection.
 *
 * @param int $length Number of bytes to read.
 * @return string|boolean Data or `false` on error.
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, read){

	zval *length = NULL, connection = {}, meta = {}, timed_out = {};
	zend_bool timeout = 0;
	php_stream *stream;
	long int total_length;
	long int len;
	char *buf;

	phalcon_fetch_params(1, 0, 1, &length);

	if (!length) {
		length = &PHALCON_GLOBAL(z_zero);
	} else {
		PHALCON_ENSURE_IS_LONG(length);
	}

	phalcon_read_property(&connection, getThis(), SL("_connection"), PH_READONLY);
	if (Z_TYPE(connection) != IS_RESOURCE) {
		PHALCON_MM_CALL_METHOD(&connection, getThis(), "connect");
		PHALCON_MM_ADD_ENTRY(&connection);
		if (Z_TYPE(connection) != IS_RESOURCE) {
			RETURN_MM_FALSE;
		}
	}

	php_stream_from_zval_no_verify(stream, &connection);
	if (!stream) {
		RETURN_MM_FALSE;
	}

	if (zend_is_true(length)) {
		if (php_stream_eof(stream)) {
			RETURN_MM_FALSE;
		}

		total_length = Z_LVAL_P(length) + 2;

		buf = ecalloc(1, total_length + 1);
		len = php_stream_read(stream, buf, total_length);

		ZVAL_STRINGL(return_value, buf, len);
		efree(buf);

		array_init_size(&meta, 4);
		PHALCON_MM_ADD_ENTRY(&meta);
		if (php_stream_populate_meta_data(stream, &meta)) {
			if (phalcon_array_isset_fetch_str(&timed_out, &meta, SL("timed_out"), PH_READONLY)) {
				timeout = zend_is_true(&timed_out);
			}
		}

		if (timeout) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_exception_ce, "Connection timed out");
			return;
		}
	} else {
		size_t line_len = 0;
		long int len = 16384;
		char *buf = ecalloc(1, len+1);

		if (php_stream_get_line(stream, buf, len, &line_len) != NULL) {
			if (line_len < 512) {
				buf = erealloc(buf, line_len + 1);
			}

			ZVAL_STRINGL(return_value, buf, line_len);
			efree(buf);
		} else {
			efree(buf);
			ZVAL_FALSE(return_value);
		}
	}

	if (Z_TYPE_P(return_value) == IS_STRING && Z_STRLEN_P(return_value) >= 2) {
		char *s      = Z_STRVAL_P(return_value);
		long int len = Z_STRLEN_P(return_value);

		if (s[len-1] == '\n' && s[len-2] == '\r') {
			s[len-2] = '\0';
			Z_STRLEN_P(return_value) -= 2;
		}
	}
	RETURN_MM();
}

/**
 * Writes data to the socket. Performs a connection if none is available
 *
 * @param string $data
 * @return integer|boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, write){

	zval *data, connection = {}, packet = {};
	php_stream *stream;

	phalcon_fetch_params(0, 1, 0, &data);

	phalcon_read_property(&connection, getThis(), SL("_connection"), PH_READONLY);
	if (Z_TYPE(connection) != IS_RESOURCE) {
		PHALCON_CALL_METHOD(&connection, getThis(), "connect");
		if (Z_TYPE(connection) != IS_RESOURCE) {
			RETURN_FALSE;
		}
	}

	php_stream_from_zval_no_verify(stream, &connection);
	if (!stream) {
		RETURN_FALSE;
	}

	PHALCON_CONCAT_VS(&packet, data, "\r\n");

	php_stream_write(stream, Z_STRVAL(packet), Z_STRLEN(packet));
	zval_ptr_dtor(&packet);
}

/**
 * Closes the connection to the beanstalk server.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, disconnect){

	zval connection = {};
	php_stream *stream;

	phalcon_read_property(&connection, getThis(), SL("_connection"), PH_READONLY);
	if (Z_TYPE(connection) != IS_RESOURCE) {
		RETURN_FALSE;
	}

	php_stream_from_zval_no_verify(stream, &connection);
	if (!stream) {
		RETURN_FALSE;
	}

	if ((stream->flags & PHP_STREAM_FLAG_NO_FCLOSE) == 0) {
		if (!stream->is_persistent) {
			zend_list_delete(stream->res);
		}
		else {
			php_stream_pclose(stream);
		}
	}

	RETURN_TRUE;
}

/**
 * Simply closes the connection.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, quit){

	zval command = {};

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&command, "quit");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(return_value, getThis(), "disconnect");

	RETURN_MM();
}

/**
 * Returns a list of all existing tubes.
 *
 * @return array
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, listTubes){

	zval command = {}, response = {}, status = {};

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&command, "list-tubes");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readyaml");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "OK")) {
		phalcon_array_fetch_long(return_value, &response, 2, PH_NOISY|PH_COPY);
		RETURN_MM();
	}
	RETURN_MM_FALSE;
}

/**
 * Returns the tube currently being used by the client.
 *
 * @return array
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, listTubeUsed){

	zval command = {}, response = {}, status = {};

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&command, "list-tube-used");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readyaml");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "USING")) {
		phalcon_array_fetch_long(return_value, &response, 2, PH_NOISY|PH_COPY);
		RETURN_MM();
	}
	RETURN_MM_FALSE;
}

/**
 * Returns a list tubes currently being watched by the client.
 *
 * @return array
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, listTubesWatched){

	zval command = {}, response = {}, status = {};

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&command, "list-tube-watched");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readyaml");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "OK")) {
		phalcon_array_fetch_long(return_value, &response, 2, PH_NOISY|PH_COPY);
		RETURN_MM();
	}
	RETURN_MM_FALSE;
}

/**
 * It removes the named tube from the watch list for the current connection.
 *
 * @return boolean|int
 */
PHP_METHOD(Phalcon_Queue_Beanstalk, ignore){

	zval *tube, command = {}, response = {}, status = {};

	phalcon_fetch_params(1, 1, 0, &tube);

	PHALCON_CONCAT_SV(&command, "ignore ", tube);
	PHALCON_MM_ADD_ENTRY(&command);
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &command);
	PHALCON_MM_CALL_METHOD(&response, getThis(), "readstatus");
	PHALCON_MM_ADD_ENTRY(&response);

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY|PH_READONLY);
	if (PHALCON_IS_STRING(&status, "WATCHING")) {
		phalcon_array_fetch_long(return_value, &response, 1, PH_NOISY|PH_COPY);
		RETURN_MM();
	}
	RETURN_MM_FALSE;
}

PHP_METHOD(Phalcon_Queue_Beanstalk, __sleep){

	array_init_size(return_value, 1);
	add_next_index_string(return_value, "_parameters");
}

PHP_METHOD(Phalcon_Queue_Beanstalk, __wakeup){

	zval params = {}, host = {}, port = {};
	int fail;

	zend_update_property_null(phalcon_queue_beanstalk_ce, getThis(), SL("_connection"));

	phalcon_read_property(&params, getThis(), SL("_parameters"), PH_NOISY|PH_READONLY);
	if (
			Z_TYPE(params) != IS_ARRAY
		 || !phalcon_array_isset_fetch_str(&host, &params, SL("host"), PH_READONLY)
		 || !phalcon_array_isset_fetch_str(&port, &params, SL("port"), PH_READONLY)
	) {
		fail = 1;
	} else if (Z_TYPE(host) != IS_STRING || Z_TYPE(port) != IS_LONG) {
		fail = 1;
	} else {
		fail = 0;
	}

	if (fail) {
		zend_throw_exception_ex(phalcon_exception_ce, 0, "Invalid serialization data");
	}
}
