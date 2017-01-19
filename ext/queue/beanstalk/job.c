
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
  +------------------------------------------------------------------------+
*/

#include "queue/beanstalk/job.h"
#include "queue/beanstalk.h"
#include "queue/../exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/operators.h"

/**
 * Phalcon\Queue\Beanstalk\Job
 *
 * Represents a job in a beanstalk queue
 */
zend_class_entry *phalcon_queue_beanstalk_job_ce;

PHP_METHOD(Phalcon_Queue_Beanstalk_Job, __construct);
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, getId);
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, getBody);
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, delete);
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, release);
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, bury);
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, touch);
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, kick);
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, __wakeup);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_queue_beanstalk_job___construct, 0, 0, 3)
	ZEND_ARG_INFO(0, queue)
	ZEND_ARG_INFO(0, id)
	ZEND_ARG_INFO(0, body)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_queue_beanstalk_job_method_entry[] = {
	PHP_ME(Phalcon_Queue_Beanstalk_Job, __construct, arginfo_phalcon_queue_beanstalk_job___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Queue_Beanstalk_Job, getId, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk_Job, getBody, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk_Job, delete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk_Job, release, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk_Job, bury, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk_Job, touch, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk_Job, kick, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Queue_Beanstalk_Job, __wakeup, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Queue\Beanstalk\Job initializer
 */
PHALCON_INIT_CLASS(Phalcon_Queue_Beanstalk_Job){

	PHALCON_REGISTER_CLASS(Phalcon\\Queue\\Beanstalk, Job, queue_beanstalk_job, phalcon_queue_beanstalk_job_method_entry, 0);

	zend_declare_property_null(phalcon_queue_beanstalk_job_ce, SL("_queue"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_queue_beanstalk_job_ce, SL("_id"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_queue_beanstalk_job_ce, SL("_body"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Queue\Beanstalk\Job
 *
 * @param Phalcon\Queue\Beanstalk $queue
 * @param string $id
 * @param mixed $body
 */
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, __construct){

	zval *queue, *id, *body;

	phalcon_fetch_params(0, 3, 0, &queue, &id, &body);
	PHALCON_VERIFY_CLASS_EX(queue, phalcon_queue_beanstalk_ce, phalcon_exception_ce);
	PHALCON_ENSURE_IS_STRING(id);

	phalcon_update_property_zval(getThis(), SL("_queue"), queue);
	phalcon_update_property_zval(getThis(), SL("_id"),    id   );
	phalcon_update_property_zval(getThis(), SL("_body"),  body );
}

/**
 * Returns the job id
 *
 * @return string
 */
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, getId){


	RETURN_MEMBER(getThis(), "_id");
}

/**
 * Returns the job body
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, getBody){


	RETURN_MEMBER(getThis(), "_body");
}

/**
 * Removes a job from the server entirely
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, delete){

	zval id = {}, queue = {}, command = {}, response = {}, status = {};

	phalcon_read_property(&id, getThis(), SL("_id"), PH_NOISY);
	phalcon_read_property(&queue, getThis(), SL("_queue"), PH_NOISY);

	PHALCON_CONCAT_SV(&command, "delete ", &id);
	PHALCON_CALL_METHOD(NULL, &queue, "write", &command);

	PHALCON_CALL_METHOD(&response, &queue, "readstatus");

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY);
	if (PHALCON_IS_STRING(&status, "DELETED")) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * The release command puts a reserved job back into the ready queue (and marks
 * its state as "ready") to be run by any client. It is normally used when the job
 * fails because of a transitory error.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, release){

	zval *priority = NULL, *delay = NULL, z_priority = {}, id = {}, queue = {}, command = {}, response = {}, status = {};

	phalcon_fetch_params(0, 0, 2, &priority, &delay);

	if (!priority) {
		ZVAL_LONG(&z_priority, 100);
		priority = &z_priority;
	}

	if (!delay) {
		delay = &PHALCON_GLOBAL(z_zero);
	}

	phalcon_read_property(&id, getThis(), SL("_id"), PH_NOISY);
	phalcon_read_property(&queue, getThis(), SL("_queue"), PH_NOISY);

	PHALCON_CONCAT_SVSVSV(&command, "release ", &id, " ", priority, " ", delay);
	PHALCON_CALL_METHOD(NULL, &queue, "write", &command);

	PHALCON_CALL_METHOD(&response, &queue, "readstatus");

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY);
	if (PHALCON_IS_STRING(&status, "RELEASED")) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * The bury command puts a job into the "buried" state. Buried jobs are put into
 * a FIFO linked list and will not be touched by the server again until a client
 * kicks them with the "kick" command.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, bury){

	zval *priority = NULL, z_priority, id = {}, queue = {}, command = {}, response = {}, status = {};

	phalcon_fetch_params(0, 0, 1, &priority);

	if (!priority) {
		ZVAL_LONG(&z_priority, 100);
		priority = &z_priority;
	}

	phalcon_read_property(&id, getThis(), SL("_id"), PH_NOISY);
	phalcon_read_property(&queue, getThis(), SL("_queue"), PH_NOISY);

	PHALCON_CONCAT_SVSV(&command, "bury ", &id, " ", priority);
	PHALCON_CALL_METHOD(NULL, &queue, "write", &command);
	PHALCON_CALL_METHOD(&response, &queue, "readstatus");

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY);
	if (PHALCON_IS_STRING(&status, "BURIED")) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * The `touch` command allows a worker to request more time to work on a job.
 * This is useful for jobs that potentially take a long time, but you still
 * want the benefits of a TTR pulling a job away from an unresponsive worker.
 * A worker may periodically tell the server that it's still alive and processing
 * a job (e.g. it may do this on `DEADLINE_SOON`). The command postpones the auto
 * release of a reserved job until TTR seconds from when the command is issued.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, touch){

	zval id = {}, queue = {}, command = {}, response = {}, status = {};

	phalcon_read_property(&id, getThis(), SL("_id"), PH_NOISY);
	phalcon_read_property(&queue, getThis(), SL("_queue"), PH_NOISY);

	PHALCON_CONCAT_SV(&command, "touch ", &id);
	PHALCON_CALL_METHOD(NULL, &queue, "write", &command);
	PHALCON_CALL_METHOD(&response, &queue, "readstatus");

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY);
	if (PHALCON_IS_STRING(&status, "TOUCHED")) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Move the job to the ready queue if it is delayed or buried.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Queue_Beanstalk_Job, kick){

	zval id = {}, queue = {}, command = {}, response = {}, status = {};

	phalcon_read_property(&id, getThis(), SL("_id"), PH_NOISY);
	phalcon_read_property(&queue, getThis(), SL("_queue"), PH_NOISY);

	PHALCON_CONCAT_SV(&command, "kick-job ", &id);
	PHALCON_CALL_METHOD(NULL, &queue, "write", &command);
	PHALCON_CALL_METHOD(&response, &queue, "readstatus");

	phalcon_array_fetch_long(&status, &response, 0, PH_NOISY);
	if (PHALCON_IS_STRING(&status, "KICKED")) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHP_METHOD(Phalcon_Queue_Beanstalk_Job, __wakeup) {

	zval id = {}, queue = {};

	phalcon_read_property(&id, getThis(), SL("_id"), PH_NOISY);
	phalcon_read_property(&queue, getThis(), SL("_queue"), PH_NOISY);

	PHALCON_VERIFY_CLASS_EX(&queue, phalcon_queue_beanstalk_ce, phalcon_exception_ce);

	if (Z_TYPE(id) != IS_STRING) {
		zend_throw_exception_ex(phalcon_exception_ce, 0, "Unexpected inconsistency in %s - possible break-in attempt!", "Phalcon\\Queue\\Beanstalk\\Job::__wakeup()");
	}
}
