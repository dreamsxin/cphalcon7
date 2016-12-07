
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

#include "process/proc.h"
#include "process/procinterface.h"
#include "process/../date.h"

#include "kernel/main.h"
#include "kernel/object.h"

/**
 * Phalcon\Process\Pro
 *
 * Pro for Phalcon\Process procs
 */
zend_class_entry *phalcon_process_proc_ce;

PHP_METHOD(Phalcon_Process_Pro, __construct);
PHP_METHOD(Phalcon_Process_Pro, __destruct);
PHP_METHOD(Phalcon_Process_Pro, setMode);
PHP_METHOD(Phalcon_Process_Pro, getDescriptors);
PHP_METHOD(Phalcon_Process_Pro, isPtySupported);
PHP_METHOD(Phalcon_Process_Pro, start);
PHP_METHOD(Phalcon_Process_Pro, stop);
PHP_METHOD(Phalcon_Process_Pro, reStart);
PHP_METHOD(Phalcon_Process_Pro, isRunning);
PHP_METHOD(Phalcon_Process_Pro, update);
PHP_METHOD(Phalcon_Process_Pro, getCommandForPid);
PHP_METHOD(Phalcon_Process_Pro, getStarttimeForPid);
PHP_METHOD(Phalcon_Process_Pro, getStatForPid);
PHP_METHOD(Phalcon_Process_Pro, checkTimeout);
PHP_METHOD(Phalcon_Process_Pro, getPid);
PHP_METHOD(Phalcon_Process_Pro, wait);
PHP_METHOD(Phalcon_Process_Pro, read);
PHP_METHOD(Phalcon_Process_Pro, write);
PHP_METHOD(Phalcon_Process_Pro, hasSystemCallBeenInterrupted);
PHP_METHOD(Phalcon_Process_Pro, setBlocking);
PHP_METHOD(Phalcon_Process_Pro, close);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, command)
	ZEND_ARG_TYPE_INFO(0, cwd, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, env, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_setmode, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_stop, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, signal, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_update, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, blocking, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_getcommandforpid, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, pid, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_getstarttimeforpid, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, pid, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_getstatforpid, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, pid, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_read, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, blocking, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_write, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_setblocking, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, flag, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_process_proc_method_entry[] = {
	PHP_ME(Phalcon_Process_Pro, __construct, arginfo_phalcon_process_proc___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, __destruct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, setMode, arginfo_phalcon_process_proc_setmode, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, getDescriptors, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, isPtySupported, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, stop, arginfo_phalcon_process_proc_stop, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, reStart, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, isRunning, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, update, arginfo_phalcon_process_proc_update, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, getCommandForPid, arginfo_phalcon_process_proc_getcommandforpid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, getStarttimeForPid, arginfo_phalcon_process_proc_getstarttimeforpid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, getStatForPid, arginfo_phalcon_process_proc_getstatforpid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, checkTimeout, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, getPid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, wait, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, read, arginfo_phalcon_process_proc_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, write, arginfo_phalcon_process_proc_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, hasSystemCallBeenInterrupted, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Process_Pro, setBlocking, arginfo_phalcon_process_proc_setblocking, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Pro, close, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Process\Pro initializer
 */
PHALCON_INIT_CLASS(Phalcon_Process_Pro){

	PHALCON_REGISTER_CLASS(Phalcon\\Process, Pro, process_proc, phalcon_process_proc_method_entry, 0);

	zend_declare_property_null(phalcon_process_proc_ce, SL("_command"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_cwd"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_env"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_timeout"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_process_proc_ce, SL("_mode"), PHALCON_PROCESS_MODE_NONE, ZEND_ACC_PROTECTED);

	zend_declare_property_null(phalcon_process_proc_ce, SL("_process"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_pid"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_process_proc_ce, SL("_status"), PHALCON_PROCESS_STATUS_READY, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_information"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_running"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_process_proc_ce, SL("_signaled"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("termsig"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_latestSignal"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_startTime"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_exitCode"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_running"), ZEND_ACC_PROTECTED);

	zend_declare_property_null(phalcon_process_proc_ce, SL("_pipes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_exitMessage"), ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("MODE_NONE"), PHALCON_PROCESS_MODE_NONE);
	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("MODE_TTY"), PHALCON_PROCESS_MODE_TTY);
	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("MODE_PTY"), PHALCON_PROCESS_MODE_PTY);

	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("STATUS_READY"), PHALCON_PROCESS_STATUS_READY);
	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("STATUS_STARTED"), PHALCON_PROCESS_STATUS_STARTED);
	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("STATUS_STOP"), PHALCON_PROCESS_STATUS_STOP);
	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("STATUS_TERMINATED"), PHALCON_PROCESS_STATUS_TERMINATED);

	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("STDIN"), PHALCON_PROCESS_STDIN);
	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("STDOUT"), PHALCON_PROCESS_STDOUT);
	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("STDERR"), PHALCON_PROCESS_STDERR);

	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("SIGKILL"), 9);
	zend_declare_class_constant_long(phalcon_process_proc_ce, SL("SIGTERM"), 15);

	return SUCCESS;
}

/**
 * Phalcon\Process\Pro constructor
 *
 * @param string $command     The command line to run
 * @param int $pid            The process identifier
 * @param string $cwd         The working directory or null to use the working dir of the current PHP process
 * @param array $env          The environment variables or null to use the same environment as the current PHP process
 * @param float $timeout      The timeout in seconds or null to disable
 * @param array $options
 */
PHP_METHOD(Phalcon_Process_Pro, __construct){

	zval *command, *cwd = NULL, *env = NULL, *timeout = NULL, *options = NULL, real_command = {}, merge_options = {};

	phalcon_fetch_params(0, 1, 4, &command, &cwd, &env, &timeout, &options);

	if (Z_TYPE_P(command) == IS_LONG) {
		phalcon_update_property_zval(getThis(), SL("_pid"), command);
		PHALCON_CALL_METHODW(&real_command, getThis(), "getcommandforpid", command);
		if (zend_is_true(&real_command)) {
			phalcon_update_property_zval(getThis(), SL("_command"), &real_command);
			phalcon_update_property_long(getThis(), SL("_status"), PHALCON_PROCESS_STATUS_STARTED);
		} else {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_process_exception_ce, "Can't find process pid");
			return;
		}
	} else if (Z_TYPE_P(command) == IS_STRING) {
		phalcon_update_property_zval(getThis(), SL("_command"), command);
	} else {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_process_exception_ce, "Command error");
		return;
	}

	if (cwd) {
		phalcon_update_property_zval(getThis(), SL("_cwd"), cwd);
	}

	if (env) {
		phalcon_update_property_zval(getThis(), SL("_env"), env);
	}

	if (timeout) {
		phalcon_update_property_zval(getThis(), SL("_timeout"), timeout);
	}

	array_init(&merge_options);
	phalcon_array_update_str(&merge_options, SL("suppress_errors"), &PHALCON_GLOBAL(z_true), 0);
	phalcon_array_update_str(&merge_options, SL("binary_pipes"), &PHALCON_GLOBAL(z_true), 0);

	if (options) {
		phalcon_array_replace(&merge_options, options);
	}
	phalcon_update_property_zval(getThis(), SL("_options"), &merge_options);
}

PHP_METHOD(Phalcon_Process_Pro, __destruct){

	PHALCON_CALL_METHODW(NULL, getThis(), "stop", &PHALCON_GLOBAL(z_zero));
}

/**
 * Sets the mode.
 */
PHP_METHOD(Phalcon_Process_Pro, setMode){

	zval *mode;

	phalcon_fetch_params(0, 1, 0, &mode);

	switch (Z_TYPE_P(mode)) {
		case PHALCON_PROCESS_MODE_NONE:
			phalcon_update_property_zval(getThis(), SL("_mode"), mode);
			break;
		case PHALCON_PROCESS_MODE_TTY:
			phalcon_update_property_zval(getThis(), SL("_mode"), mode);
			break;
		case PHALCON_PROCESS_MODE_PTY:
			phalcon_update_property_zval(getThis(), SL("_mode"), mode);
			break;
		default:
			PHALCON_THROW_EXCEPTION_STRW(phalcon_process_exception_ce, "Process mode error");
			return;
	}
}

/**
 * Creates the descriptors needed by the proc_open.
 *
 * @return array
 */
PHP_METHOD(Phalcon_Process_Pro, getDescriptors){

	zval mode = {}, stdin = {}, stdout = {}, stderr = {};

	phalcon_read_property(&mode, getThis(), SL("_mode"), PH_NOISY);

	switch (Z_TYPE_P(mode)) {
		case PHALCON_PROCESS_MODE_TTY:
		{
			array_init(&stdin, 3);
			phalcon_array_append(&stdin, SL("file"), PH_COPY);
			phalcon_array_append(&stdin, SL("/dev/tty"), PH_COPY);
			phalcon_array_append(&stdin, SL("r"), PH_COPY);
			array_init(&stdout, 3);
			phalcon_array_append(&stdout, SL("file"), PH_COPY);
			phalcon_array_append(&stdout, SL("/dev/tty"), PH_COPY);
			phalcon_array_append(&stdout, SL("w"), PH_COPY);
			array_init(&stderr, 3);
			phalcon_array_append(&stderr, SL("file"), PH_COPY);
			phalcon_array_append(&stderr, SL("/dev/tty"), PH_COPY);
			phalcon_array_append(&stderr, SL("w"), PH_COPY);

			array_init(return_value, 3);
			phalcon_array_append(return_value, &stdin, PH_COPY);
			phalcon_array_append(return_value, &stdout, PH_COPY);
			phalcon_array_append(return_value, &stderr, PH_COPY);
			break;
		}
		case PHALCON_PROCESS_MODE_PTY:
		{
			array_init(&stdin, 1);
			phalcon_array_append(&stdin, SL("pty"), PH_COPY);
			array_init(&stdout, 1);
			phalcon_array_append(&stdout, SL("pty"), PH_COPY);
			array_init(&stderr, 1);
			phalcon_array_append(&stderr, SL("pty"), PH_COPY);

			array_init(return_value, 3);
			phalcon_array_append(return_value, &stdin, PH_COPY);
			phalcon_array_append(return_value, &stdout, PH_COPY);
			phalcon_array_append(return_value, &stderr, PH_COPY);
			break;
		}
		default:
		{
			array_init(&stdin, 2);
			phalcon_array_append(&stdin, SL("pipe"), PH_COPY);
			phalcon_array_append(&stdin, SL("r"), PH_COPY);
			array_init(&stdout, 2);
			phalcon_array_append(&stdout, SL("pipe"), PH_COPY);
			phalcon_array_append(&stdout, SL("w"), PH_COPY);
			array_init(&stderr, 2);
			phalcon_array_append(&stderr, SL("pipe"), PH_COPY);
			phalcon_array_append(&stderr, SL("w"), PH_COPY);

			array_init(return_value, 3);
			phalcon_array_append(return_value, &stdin, PH_COPY);
			phalcon_array_append(return_value, &stdout, PH_COPY);
			phalcon_array_append(return_value, &stderr, PH_COPY);
			break;
		}
	}
}

/**
 * Returns whether PTY is supported on the current operating system.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Pro, isPtySupported){

	zval command = {}, stdin = {}, stdout = {}, stderr = {}, descriptors = {}, pipes = {}, process = {};

	ZVAL_STRING(&command, "echo 1");

	array_init(&stdin, 2);
	phalcon_array_append(&stdin, SL("pipe"), PH_COPY);
	phalcon_array_append(&stdin, SL("r"), PH_COPY);
	array_init(&stdout, 2);
	phalcon_array_append(&stdout, SL("pipe"), PH_COPY);
	phalcon_array_append(&stdout, SL("w"), PH_COPY);
	array_init(&stderr, 2);
	phalcon_array_append(&stderr, SL("pipe"), PH_COPY);
	phalcon_array_append(&stderr, SL("w"), PH_COPY);

	array_init(&descriptors, 3);
	phalcon_array_append(&descriptors, &stdin, PH_COPY);
	phalcon_array_append(&descriptors, &stdout, PH_COPY);
	phalcon_array_append(&descriptors, &stderr, PH_COPY);

	PHALCON_CALL_FUNCTIONW(&process, "proc_open", &command, &descriptors, &pipes);
	if (Z_TYPE(process) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

/**
  * Starts the process and returns after writing the input to STDIN.
  *
  * This method blocks until all STDIN data is sent to the process then it
  * returns while the process runs in the background.
  *
  * The termination of the process can be awaited with wait().
  *
  * The callback receives the type of output (out or err) and some bytes from
  * the output in real-time while writing the standard input to the process.
  * It allows to have feedback from the independent process during execution.
  *
  *
  * @throws Phalcon\Process\Exception When process can't be launched
  * @throws Phalcon\Process\Exception When process is already running
  */
PHP_METHOD(Phalcon_Process_Pro, start){

	zval status = {}, starttime = {}, descriptors = {}, process = {};

	phalcon_read_property(&status, getThis(), SL("_status"), PH_NOISY);
	if (PHALCON_IS_LONG(&status, PHALCON_PROCESS_STATUS_STARTED)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_process_exception_ce, "Process is already running");
		return;
	}

	phalcon_time(&starttime);

	PHALCON_CALL_METHODW(&descriptors, getThis(), "getdescriptors");

	phalcon_read_property(&command, getThis(), SL("_command"), PH_NOISY);
	phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY);
	phalcon_read_property(&cwd, getThis(), SL("_cwd"), PH_NOISY);
	phalcon_read_property(&env, getThis(), SL("_env"), PH_NOISY);
	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY);

	PHALCON_CALL_FUNCTIONW(&process, "proc_open", &command, &descriptors, &pipes, &cwd, &env, &options);
    if (Z_TYPE(process) != IS_RESOURCE) {
		RETURN_FALSE;
    }
	phalcon_update_property_zval(getThis(), SL("_startTime"), &starttime);
	phalcon_update_property_long(getThis(), SL("_status"), PHALCON_PROCESS_STATUS_STARTED);
	PHALCON_CALL_METHODW(NULL, getThis(), "update");

	RETURN_TRUE;
}

/**
 * Stops the process.
 *
 * @param int|float $timeout The timeout in seconds
 * @param int       $signal  A POSIX signal to send in case the process has not stop at timeout, default is SIGKILL (9)
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Pro, stop){

	zval *timeout = NULL, *_signal = NULL, signal = {}, status = {}, isrunning = {}, process = {}, pid = {}, command = {}, pipes = {}, *s;
	double mtime, curtime;

	phalcon_fetch_params(0, 0, 2, &timeout, &_signal);

	phalcon_read_property(&status, getThis(), SL("_status"), PH_NOISY);
	if (!PHALCON_IS_LONG(&status, PHALCON_PROCESS_STATUS_STARTED)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_process_exception_ce, "Process not running");
		return;
	}

	if (!timeout){
		mtime = 10;
	} else {
		mtime = Z_DVAL_P(timeout);
	}

	if (!_signal){
		ZVAL_LONG(&signal, 15);
	} else {
		PHALCON_CPY_WRT(&signal, _signal);
	}

	PHALCON_CALL_METHODW(&isrunning, getThis(), "isrunning");
	if (zend_is_true(&isrunning)) {
		phalcon_read_property(&process, getThis(), SL("_process"), PH_NOISY);
		phalcon_read_property(&pid, getThis(), SL("_pid"), PH_NOISY);
		if (zend_is_true(&process)) {
			curtime = phalcon_get_time();
			mtime = mtime + curtime;
			do {
				PHALCON_CALL_FUNCTIONW(NULL, "proc_terminate", &process, &signal);
				PHALCON_CALL_METHODW(&isrunning, getThis(), "isrunning");
				if (zend_is_true(&isrunning)) {
					usleep(1000);
				}
		    } while (zend_is_true(&isrunning) && phalcon_get_time() < mtime);

			if (zend_is_true(&isrunning)) {
				PHALCON_CONCAT_SV(&command, "kill -9 ", &pid);
				PHALCON_CALL_FUNCTIONW(NULL, "exec", &command);
			}

			phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY);
			if (Z_TYPE(pipes) == IS_ARRAY && phalcon_fast_count_ev(&pipes)) {
				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(pipes), s) {
					PHALCON_CALL_FUNCTIONW(NULL, "fclose", s);
				} ZEND_HASH_FOREACH_END();
			}
			phalcon_update_property_null(getThis(), SL("_pipes"));
			PHALCON_CALL_METHODW(NULL, getThis(), "update");
		} else {
			PHALCON_CONCAT_SV(&command, "kill -9 ", &pid);
			PHALCON_CALL_FUNCTIONW(NULL, "exec", &command);
		}
	}

	phalcon_update_property_long(getThis(), SL("_status"), PHALCON_PROCESS_STATUS_TERMINATED);
	RETURN_TRUE;
}

/**
 * Restarts the process
 */
PHP_METHOD(Phalcon_Process_Pro, reStart){

	PHALCON_CALL_METHODW(&isrunning, getThis(), "stop");
	PHALCON_CALL_METHODW(&isrunning, getThis(), "start");
}

/**
 * Checks if the process is currently running.
 *
 * @return boolean true if the process is currently running, false otherwise
 */
PHP_METHOD(Phalcon_Process_Pro, isRunning){

	zval status = {}, information = {};

	phalcon_read_property(&status, getThis(), SL("_status"), PH_NOISY);
	if (!PHALCON_IS_LONG(&status, PHALCON_PROCESS_STATUS_STARTED)) {
		RETURN_FALSE;
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "update");
	phalcon_read_property(&information, getThis(), SL("_information"), PH_NOISY);
	if (z_type(information) != IS_ARRAY || !phalcon_array_isset_fetch_str(return_value, &information, SL("running"))) {
		RETURN_FALSE;
	}
}

/**
 * Updates the status of the process, reads pipes.
 *
 * @param boolean $blocking Whether to use a blocking read call
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Pro, update){

	zval *blocking = NULL, status = {}, pid = {}, process = {}, information = {}, running = {}, exitcode = {}, exitmessage = {};
	zval signaled = {}, termsig = {}, stopped = {}, stopsig = {}, stdtype = {};
	zval command = {}, descriptors = {}, stdin = {}, stdout = {}, stderr = {}, pipes = {}, ret = {};


	if (!blocking) {
		blocking = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&status, getThis(), SL("_status"), PH_NOISY);
	if (!PHALCON_IS_LONG(&status, PHALCON_PROCESS_STATUS_STARTED)) {
		RETURN_FALSE;
	}
	phalcon_read_property(&pid, getThis(), SL("_pid"), PH_NOISY);
	phalcon_read_property(&process, getThis(), SL("_process"), PH_NOISY);
	if (Z_TYPE(process) != IS_NULL) {
		PHALCON_CALL_FUNCTIONW(&information, "proc_get_status", &process);
		if (Z_TYPE(information) == IS_ARRAY) {
			phalcon_array_fetch_str(&pid, &information, SL("pid"));
			phalcon_update_property_zval(getThis(), SL("_pid"), &pid);

			phalcon_array_fetch_str(&running, &information, SL("running"));
			phalcon_update_property_zval(getThis(), SL("_running"), &running);

			if (!zend_is_true(&running)) {
				if (phalcon_array_isset_fetch_str(&exitcode, &information, SL("exitcode"))) {
					if (Z_LVAL(exitcode) != -1) {
						phalcon_update_property_zval(getThis(), SL("_exitCode"), &exitcode);
					}
				}
				phalcon_array_fetch_str(&signaled, &information, SL("signaled"));
				if (zend_is_true(&signaled)) {
					if (phalcon_array_isset_fetch_str(&termsig, &information, SL("termsig")) && Z_LVAL(termsig) > 0) {
						phalcon_update_property_long(getThis(), SL("_exitCode"), 128 + Z_LVAL(termsig));
						phalcon_update_property_zval(getThis(), SL("_exitCode"), &termsig);
					}
					phalcon_update_property_bool(getThis(), SL("_signaled"), 1);
				}

				if (!zend_is_true(&signaled)) {
					ZVAL_LONG(&stdtype, PHALCON_PROCESS_STDERR);
					PHALCON_CALL_METHODW(&exitmessage, getThis(), "read", &stdtype);
					if (Z_TYPE(exitmessage) != IS_NULL) {
						phalcon_update_property_zval(getThis(), SL("_exitMessage"), &exitmessage);
					}
				}
			} else {
				phalcon_array_fetch_str(&stopped, &information, SL("stopped"));
				if (zend_is_true(&stopped)) {
					if (phalcon_array_isset_fetch_str(&stopsig, &information, SL("stopsig"))) {
						phalcon_update_property_long(getThis(), SL("_stopCode"), Z_LVAL(stopsig));
					}
					phalcon_update_property_long(getThis(), SL("_status"), PHALCON_PROCESS_STATUS_STOP);
				}
			}
		}
	} else {
		PHALCON_CALL_METHODW(&stat, getThis(), "getstatforpid", &pid);
		if (zend_is_true(&stat)) {
			phalcon_update_property_bool(getThis(), SL("_running"), 1);
		} else {
			phalcon_update_property_bool(getThis(), SL("_running"), 0);
			phalcon_update_property_long(getThis(), SL("_status"), PHALCON_PROCESS_STATUS_TERMINATED);
		}
	}

	RETURN_TRUE;
}

PHP_METHOD(Phalcon_Process_Pro, getCommandForPid){

	zval *pid, stdin = {}, stdout = {}, stderr = {}, descriptors = {}, pipes = {}, process ={}, information ={};
	zval running = {}, pipe = {};

	phalcon_fetch_params(0, 1, 0, &pid);

	// ps eh -o args -p pid
	PHALCON_CONCAT_SV(&command, "ps -o args -p ", pid);
	array_init(&stdin, 2);
	phalcon_array_append(&stdin, SL("pipe"), PH_COPY);
	phalcon_array_append(&stdin, SL("r"), PH_COPY);
	array_init(&stdout, 2);
	phalcon_array_append(&stdout, SL("pipe"), PH_COPY);
	phalcon_array_append(&stdout, SL("w"), PH_COPY);
	array_init(&stderr, 2);
	phalcon_array_append(&stderr, SL("pipe"), PH_COPY);
	phalcon_array_append(&stderr, SL("w"), PH_COPY);

	array_init(&descriptors, 3);
	phalcon_array_append(&descriptors, &stdin, PH_COPY);
	phalcon_array_append(&descriptors, &stdout, PH_COPY);
	phalcon_array_append(&descriptors, &stderr, PH_COPY);

	PHALCON_CALL_FUNCTIONW(&process, "proc_open", &command, &descriptors, &pipes);
	if (Z_TYPE(process) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	PHALCON_CALL_FUNCTIONW(&information, "proc_get_status", &process);
	if (Z_TYPE(information) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_str(&running, &information, SL("running")) || !zend_is_true(&running)) {
			RETURN_FALSE;
		}
		if (phalcon_array_isset_fetch_long(&pipe, &pipes, 2)) {
			PHALCON_CALL_FUNCTIONW(&ret, "fgets", &pipe);
			if (!PHALCON_IS_FALSE(&ret)) {
				RETURN_FALSE;
			}
		}
		if (!phalcon_array_isset_fetch_long(&pipe, &pipes, 1)) {
			RETURN_FALSE;
		}
		PHALCON_CALL_FUNCTIONW(NULL, "fgets", &pipe);
		PHALCON_CALL_FUNCTIONW(return_value, "fgets", &pipe);
	}
}

PHP_METHOD(Phalcon_Process_Pro, getStarttimeForPid){

	zval *pid;
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 1, 0, &pid);

	ZVAL_LONG(return_value, phalcon_get_proc_starttime(Z_LVAL_P(pid)));
}

PHP_METHOD(Phalcon_Process_Pro, getStatForPid){

	zval *pid, stdin = {}, stdout = {}, stderr = {}, descriptors = {}, pipes = {}, process ={}, information ={};
	zval running = {}, pipe = {}, ret = {};

	phalcon_fetch_params(0, 1, 0, &pid);

	// ps eh -o args -p pid
	PHALCON_CONCAT_SV(&command, "ps h -o stat -p ", pid);
	array_init(&stdin, 2);
	phalcon_array_append(&stdin, SL("pipe"), PH_COPY);
	phalcon_array_append(&stdin, SL("r"), PH_COPY);
	array_init(&stdout, 2);
	phalcon_array_append(&stdout, SL("pipe"), PH_COPY);
	phalcon_array_append(&stdout, SL("w"), PH_COPY);
	array_init(&stderr, 2);
	phalcon_array_append(&stderr, SL("pipe"), PH_COPY);
	phalcon_array_append(&stderr, SL("w"), PH_COPY);

	array_init(&descriptors, 3);
	phalcon_array_append(&descriptors, &stdin, PH_COPY);
	phalcon_array_append(&descriptors, &stdout, PH_COPY);
	phalcon_array_append(&descriptors, &stderr, PH_COPY);

	PHALCON_CALL_FUNCTIONW(&process, "proc_open", &command, &descriptors, &pipes);
	if (Z_TYPE(process) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	PHALCON_CALL_FUNCTIONW(&information, "proc_get_status", &process);
	if (Z_TYPE(information) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_str(&running, &information, SL("running")) || !zend_is_true(&running)) {
			RETURN_FALSE;
		}
		if (phalcon_array_isset_fetch_long(&pipe, &pipes, 2)) {
			PHALCON_CALL_FUNCTIONW(&ret, "fgets", &pipe);
			if (!PHALCON_IS_FALSE(&ret)) {
				RETURN_FALSE;
			}
		}
		if (!phalcon_array_isset_fetch_long(&pipe, &pipes, 1)) {
			RETURN_FALSE;
		}
		PHALCON_CALL_FUNCTIONW(NULL, "fgets", &pipe);
		PHALCON_CALL_FUNCTIONW(return_value, "fgets", &pipe);
	}
}

/**
 * Performs a check between the timeout definition and the time the process started
 *
 * In case you run a background process (with the start method), you should
 * trigger this method regularly to ensure the process timeout
 *
 * @throws Phalcon\Process\Exception In case the timeout was reached
 */
PHP_METHOD(Phalcon_Process_Pro, checkTimeout){

	zval status = {}, timeout = {}, start_time = {}, microtime = {};

	phalcon_read_property(&status, getThis(), SL("_status"), PH_NOISY);
	if (!PHALCON_IS_LONG(&status, PHALCON_PROCESS_STATUS_STARTED)) {
		RETURN_FALSE;
	}

	phalcon_read_property(&timeout, getThis(), SL("_timeout"), PH_NOISY);
	if (Z_TYPE(timeout) == IS_NULL) {
		RETURN_TRUE;
	}

	phalcon_read_property(&start_time, getThis(), SL("_startTime"), PH_NOISY);
	phalcon_microtime(&microtime, &PHALCON_GLOBAL(z_true));
	if (PHALCON_LE_LONG(&timeout, Z_DVAL(microtime) - Z_DVAL(start_time))) {
		PHALCON_CALL_METHODW(NULL, getThis(), "stop", &PHALCON_GLOBAL(z_zero));
		PHALCON_THROW_EXCEPTION_STRW(phalcon_process_exception_ce, "Process time-out");
		return;
	}
	RETURN_TRUE;
}

/**
 * Returns the Pid (process identifier), if applicable.
 *
 * @return int|null The process id if running, null otherwise
 */
PHP_METHOD(Phalcon_Process_Pro, getPid){

	RETURN_MEMBER(getThis(), "_pid");
}

/**
 * Waits for the process to terminate
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Pro, wait){

	zval status = {}, isrunning = {};

	phalcon_read_property(&status, getThis(), SL("_status"), PH_NOISY);
	if (!PHALCON_IS_LONG(&status, PHALCON_PROCESS_STATUS_STARTED)) {
		RETURN_FALSE;
	}
	PHALCON_CALL_METHODW(&isrunning, getThis(), "isrunning");
	while (zend_is_true(&isrunning)) {
		usleep(1000);
		PHALCON_CALL_METHODW(NULL, getThis(), "checktimeout");
		PHALCON_CALL_METHODW(&isrunning, getThis(), "isrunning");
	}
	RETURN_TRUE;
}

/**
 * Reads data in file handles and pipes
 *
 * @param int $type
 * @param boolean $blocking
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Pro, read){

	zval *type, *timeout = NULL, pipes = {}, pipe = {}, r_array = {}, w_array = {}, e_array = {}, ret = {};

	phalcon_fetch_params(0, 1, 1, &type, &timeout);

	if (!timeout) {
		timeout = &PHALCON_GLOBAL(z_zero);
	}

	phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY);
	if (Z_TYPE(pipes) != IS_ARRAY || !phalcon_array_isset_fetch_zval(&pipe, &pipes, type)) {
		RETURN_FALSE;
	}

	array_init(&r_array);
	array_init(&w_array);
	array_init(&e_array);

	phalcon_array_append(&r_array, &pipe, PH_COPY);

	ZVAL_MAKE_REF(&r_array);
	ZVAL_MAKE_REF(&w_array);
	ZVAL_MAKE_REF(&e_array);
	PHALCON_CALL_FUNCTIONW(&ret, "stream_select", &r_array, &w_array, &e_array, timeout, &PHALCON_GLOBAL(z_zero));
	ZVAL_UNREF(&r_array);
	ZVAL_UNREF(&w_array);
	ZVAL_UNREF(&e_array);

	if (PHALCON_IS_FALSE(&ret)) {
		RETURN_EMPTY_STRING();
	}

	if (!PHALCON_GT_LONG(&ret, 0)) {
		RETURN_EMPTY_STRING();
	}

	ZVAL_LONG(&len, 1024);
	PHALCON_CALL_FUNCTIONW(return_value, "fgets", &pipe, &len);
}

/**
 * Writes data to stdin
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Pro, write){

	zval *data, pipes = {}, stdin = {}, r = {}, w = {}, e = {}, ret = {}, *s, writebuf = {};
	int len;

	phalcon_fetch_params(0, 1, 0, &data);

	phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY);
	if (Z_TYPE(pipes) != IS_ARRAY || !phalcon_array_isset_fetch_long(&stdin, &pipes, 0)) {
		RETURN_FALSE;
	}

	array_init(&r);
	array_init(&w);
	array_init(&e);
	phalcon_array_append(&w, &stdin, PH_COPY);

	ZVAL_MAKE_REF(&r);
	ZVAL_MAKE_REF(&w);
	ZVAL_MAKE_REF(&e);
	PHALCON_CALL_FUNCTIONW(&ret, "stream_select", &r, &w, &e, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
	ZVAL_UNREF(&r);
	ZVAL_UNREF(&w);
	ZVAL_UNREF(&e);

	if (Z_TYPE(ret) != IS_LONG || PHALCON_LE_LONG(&ret, 0)) {
		RETURN_FALSE;
	}

	len = Z_STRLEN_P(data);
	ZVAL_LONG(&ret, 0);
	PHALCON_CPY_WRT_CTOR(&writebuf, data);
	while(1) {
		zval writelen = {};
		ZVAL_LONG(&writelen, len);
		PHALCON_CALL_FUNCTIONW(&ret, "fwrite", &stdin, &writebuf, &writelen);

		if (Z_TYPE(ret) == IS_LONG && Z_LVAL(ret) < len) {
			len -= Z_LVAL(ret);
			phalcon_substr(&writebuf, &writebuf, Z_LVAL(ret), len);
		} else {
			break;
		}
    }

	if (PHALCON_IS_FALSE(&ret)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

/**
 * Returns true if a system call has been interrupted
 *
 * @return bool
 */
PHP_METHOD(Phalcon_Process_Pro, hasSystemCallBeenInterrupted){

	zval last_error = {}, message = {};

	PHALCON_CALL_FUNCTIONW(&last_error, "error_get_last");
	if (Z_TYPE(last_error) == IS_ARRAY && phalcon_array_isset_fetch_str(&message, &last_error, SL("message"))) {
		phalcon_fast_stripos_str(return_value, &message, SL("interrupted system call"));
	} else {
		RETURN_FALSE;
	}
}

/**
 * Set streams to blocking / non blocking
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Pro, setBlocking){

	zval *flag, pipes = {}, blocked = {};

	phalcon_fetch_params(0, 1, 0, &flag);

	phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY);
	if (Z_TYPE(pipes) != IS_ARRAY && !phalcon_fast_count_ev(&pipes)) {
		RETURN_FALSE;
	}

	phalcon_read_property(&blocked, getThis(), SL("_blocking"), PH_NOISY);

	if (zend_is_true(&blocked) == zend_is_true(flag)) {
		RETURN_TRUE;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(pipes), s) {
		PHALCON_CALL_FUNCTIONW(NULL, "stream_set_blocking", s, flag);
	} ZEND_HASH_FOREACH_END();

	phalcon_update_property_zval(getThis(), SL("_blocking"), flag);
}
