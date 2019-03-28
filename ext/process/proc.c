
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
#include "process/exception.h"
#include "process/../date.h"
#include "process/system.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/exception.h"
#include "kernel/time.h"

/**
 * Phalcon\Process\Proc
 *
 * Pro for Phalcon\Process procs
 */
zend_class_entry *phalcon_process_proc_ce;

PHP_METHOD(Phalcon_Process_Proc, __construct);
PHP_METHOD(Phalcon_Process_Proc, __destruct);
PHP_METHOD(Phalcon_Process_Proc, setMode);
PHP_METHOD(Phalcon_Process_Proc, getDescriptors);
PHP_METHOD(Phalcon_Process_Proc, isPtySupported);
PHP_METHOD(Phalcon_Process_Proc, start);
PHP_METHOD(Phalcon_Process_Proc, stop);
PHP_METHOD(Phalcon_Process_Proc, close);
PHP_METHOD(Phalcon_Process_Proc, reStart);
PHP_METHOD(Phalcon_Process_Proc, isRunning);
PHP_METHOD(Phalcon_Process_Proc, update);
PHP_METHOD(Phalcon_Process_Proc, getCommandForPid);
PHP_METHOD(Phalcon_Process_Proc, getStarttimeForPid);
PHP_METHOD(Phalcon_Process_Proc, getStatForPid);
PHP_METHOD(Phalcon_Process_Proc, checkTimeout);
PHP_METHOD(Phalcon_Process_Proc, getPid);
PHP_METHOD(Phalcon_Process_Proc, wait);
PHP_METHOD(Phalcon_Process_Proc, read);
PHP_METHOD(Phalcon_Process_Proc, write);
PHP_METHOD(Phalcon_Process_Proc, handle);
PHP_METHOD(Phalcon_Process_Proc, hasSystemCallBeenInterrupted);
PHP_METHOD(Phalcon_Process_Proc, setBlocking);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, pidFile, IS_STRING, 1)
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_handle, 0, 0, 0)
	ZEND_ARG_CALLABLE_INFO(0, onrecv, 1)
	ZEND_ARG_CALLABLE_INFO(0, onsend, 1)
	ZEND_ARG_CALLABLE_INFO(0, onerror, 1)
	ZEND_ARG_CALLABLE_INFO(0, ontimeout, 1)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_proc_setblocking, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, flag, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_process_proc_method_entry[] = {
	PHP_ME(Phalcon_Process_Proc, __construct, arginfo_phalcon_process_proc___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, __destruct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, setMode, arginfo_phalcon_process_proc_setmode, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, getDescriptors, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, isPtySupported, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, stop, arginfo_phalcon_process_proc_stop, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, reStart, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, isRunning, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, update, arginfo_phalcon_process_proc_update, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, getCommandForPid, arginfo_phalcon_process_proc_getcommandforpid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, getStarttimeForPid, arginfo_phalcon_process_proc_getstarttimeforpid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, getStatForPid, arginfo_phalcon_process_proc_getstatforpid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, checkTimeout, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, getPid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, wait, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, read, arginfo_phalcon_process_proc_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, write, arginfo_phalcon_process_proc_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, handle, arginfo_phalcon_process_proc_handle, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Proc, hasSystemCallBeenInterrupted, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Process_Proc, setBlocking, arginfo_phalcon_process_proc_setblocking, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Process\Proc initializer
 */
PHALCON_INIT_CLASS(Phalcon_Process_Proc){

	PHALCON_REGISTER_CLASS(Phalcon\\Process, Proc, process_proc, phalcon_process_proc_method_entry, 0);

	zend_declare_property_null(phalcon_process_proc_ce, SL("_command"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_cwd"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_env"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_timeout"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_process_proc_ce, SL("_pidFile"), ZEND_ACC_PROTECTED);
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
	zend_declare_property_bool(phalcon_process_proc_ce, SL("_blocking"), 1, ZEND_ACC_PROTECTED);

	zend_declare_property_null(phalcon_process_proc_ce, SL("_pipes"), ZEND_ACC_PROTECTED);

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
 * Phalcon\Process\Proc constructor
 *
 * @param string $command     The command line to run
 * @param int $pid            The process identifier
 * @param string $cwd         The working directory or null to use the working dir of the current PHP process
 * @param array $env          The environment variables or null to use the same environment as the current PHP process
 * @param float $timeout      The timeout in seconds or null to disable
 * @param array $options
 */
PHP_METHOD(Phalcon_Process_Proc, __construct){

	zval *command, *pid_file = NULL, *cwd = NULL, *env = NULL, *timeout = NULL, *options = NULL;

	phalcon_fetch_params(0, 1, 5, &command, &pid_file, &cwd, &env, &timeout, &options);

	phalcon_update_property(getThis(), SL("_command"), command);

	if (pid_file && PHALCON_IS_NOT_EMPTY(pid_file)) {
		phalcon_update_property(getThis(), SL("_pidFile"), pid_file);
		PHALCON_CALL_METHOD(NULL, getThis(), "isrunning");
	}

	if (cwd) {
		phalcon_update_property(getThis(), SL("_cwd"), cwd);
	}

	if (env) {
		phalcon_update_property(getThis(), SL("_env"), env);
	}

	if (timeout) {
		phalcon_update_property(getThis(), SL("_timeout"), timeout);
	}

	if (options) {
		phalcon_update_property(getThis(), SL("_options"), options);
	}
}

PHP_METHOD(Phalcon_Process_Proc, __destruct){

	PHALCON_CALL_METHOD(NULL, getThis(), "close");
}

/**
 * Sets the mode.
 */
PHP_METHOD(Phalcon_Process_Proc, setMode){

	zval *mode;

	phalcon_fetch_params(0, 1, 0, &mode);

	switch (Z_TYPE_P(mode)) {
		case PHALCON_PROCESS_MODE_NONE:
			phalcon_update_property(getThis(), SL("_mode"), mode);
			break;
		case PHALCON_PROCESS_MODE_TTY:
			phalcon_update_property(getThis(), SL("_mode"), mode);
			break;
		case PHALCON_PROCESS_MODE_PTY:
			phalcon_update_property(getThis(), SL("_mode"), mode);
			break;
		default:
			PHALCON_THROW_EXCEPTION_STR(phalcon_process_exception_ce, "Process mode error");
			return;
	}
}

/**
 * Creates the descriptors needed by the proc_open.
 *
 * @return array
 */
PHP_METHOD(Phalcon_Process_Proc, getDescriptors){

	zval mode = {}, stdin = {}, stdout = {}, stderr = {};

	phalcon_read_property(&mode, getThis(), SL("_mode"), PH_NOISY|PH_READONLY);

	switch (Z_TYPE(mode)) {
		case PHALCON_PROCESS_MODE_TTY:
		{
			array_init_size(&stdin, 3);
			phalcon_array_append_str(&stdin, SL("file"), 0);
			phalcon_array_append_str(&stdin, SL("/dev/tty"), 0);
			phalcon_array_append_str(&stdin, SL("r"), 0);
			array_init_size(&stdout, 3);
			phalcon_array_append_str(&stdout, SL("file"), 0);
			phalcon_array_append_str(&stdout, SL("/dev/tty"), 0);
			phalcon_array_append_str(&stdout, SL("w"), 0);
			array_init_size(&stderr, 3);
			phalcon_array_append_str(&stderr, SL("file"), 0);
			phalcon_array_append_str(&stderr, SL("/dev/tty"), 0);
			phalcon_array_append_str(&stderr, SL("w"), 0);

			array_init_size(return_value, 3);
			phalcon_array_append(return_value, &stdin, 0);
			phalcon_array_append(return_value, &stdout, 0);
			phalcon_array_append(return_value, &stderr, 0);
			break;
		}
		case PHALCON_PROCESS_MODE_PTY:
		{
			array_init_size(&stdin, 1);
			phalcon_array_append_str(&stdin, SL("pty"), 0);
			array_init_size(&stdout, 1);
			phalcon_array_append_str(&stdout, SL("pty"), 0);
			array_init_size(&stderr, 1);
			phalcon_array_append_str(&stderr, SL("pty"), 0);

			array_init_size(return_value, 3);
			phalcon_array_append(return_value, &stdin, 0);
			phalcon_array_append(return_value, &stdout, 0);
			phalcon_array_append(return_value, &stderr, 0);
			break;
		}
		default:
		{
			array_init_size(&stdin, 2);
			phalcon_array_append_str(&stdin, SL("pipe"), 0);
			phalcon_array_append_str(&stdin, SL("r"), 0);
			array_init_size(&stdout, 2);
			phalcon_array_append_str(&stdout, SL("pipe"), 0);
			phalcon_array_append_str(&stdout, SL("w"), 0);
			array_init_size(&stderr, 2);
			phalcon_array_append_str(&stderr, SL("pipe"), 0);
			phalcon_array_append_str(&stderr, SL("w"), 0);

			array_init_size(return_value, 3);
			phalcon_array_append(return_value, &stdin, 0);
			phalcon_array_append(return_value, &stdout, 0);
			phalcon_array_append(return_value, &stderr, 0);
			break;
		}
	}
}

/**
 * Returns whether PTY is supported on the current operating system.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Proc, isPtySupported){

	zval command = {}, stdin = {}, stdout = {}, stderr = {}, descriptors = {}, pipes = {}, process = {};

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&command, "echo 1");

	array_init_size(&stdin, 2);
	phalcon_array_append_str(&stdin, SL("pipe"), 0);
	phalcon_array_append_str(&stdin, SL("r"), 0);
	array_init_size(&stdout, 2);
	phalcon_array_append_str(&stdout, SL("pipe"), 0);
	phalcon_array_append_str(&stdout, SL("w"), 0);
	array_init_size(&stderr, 2);
	phalcon_array_append_str(&stderr, SL("pipe"), 0);
	phalcon_array_append_str(&stderr, SL("w"), 0);

	array_init_size(&descriptors, 3);
	phalcon_array_append(&descriptors, &stdin, 0);
	phalcon_array_append(&descriptors, &stdout, 0);
	phalcon_array_append(&descriptors, &stderr, 0);

	PHALCON_MM_ADD_ENTRY(&descriptors);

	PHALCON_MM_CALL_FUNCTION(&process, "proc_open", &command, &descriptors, &pipes);
	PHALCON_MM_ADD_ENTRY(&process);

	if (Z_TYPE(process) != IS_RESOURCE) {
		RETURN_MM_FALSE;
	}

	RETURN_MM_TRUE;
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
  * @param string $pidFile
  * @throws Phalcon\Process\Exception When process can't be launched
  * @throws Phalcon\Process\Exception When process is already running
  */
PHP_METHOD(Phalcon_Process_Proc, start){

	zval status = {}, descriptors = {}, command = {}, pipes = {}, cwd = {}, env = {}, options = {}, starttime ={}, process = {};

	phalcon_read_property(&status, getThis(), SL("_status"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_LONG(&status, PHALCON_PROCESS_STATUS_STARTED)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_process_exception_ce, "Process is already running");
		return;
	}

	PHALCON_MM_INIT();

	PHALCON_MM_CALL_METHOD(&descriptors, getThis(), "getdescriptors");
	PHALCON_MM_ADD_ENTRY(&descriptors);

	phalcon_read_property(&command, getThis(), SL("_command"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&cwd, getThis(), SL("_cwd"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&env, getThis(), SL("_env"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);

	phalcon_time(&starttime);
	PHALCON_MM_ADD_ENTRY(&starttime);

	ZVAL_MAKE_REF(&pipes);
	PHALCON_MM_CALL_FUNCTION(&process, "proc_open", &command, &descriptors, &pipes, &cwd, &env, &options);
	ZVAL_UNREF(&pipes);
	PHALCON_MM_ADD_ENTRY(&process);

    if (Z_TYPE(process) != IS_RESOURCE) {
		RETURN_MM_FALSE;
    }
	phalcon_update_property(getThis(), SL("_process"), &process);
	phalcon_update_property(getThis(), SL("_pipes"), &pipes);
	phalcon_update_property(getThis(), SL("_startTime"), &starttime);
	phalcon_update_property_long(getThis(), SL("_status"), PHALCON_PROCESS_STATUS_STARTED);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "update");

	RETURN_MM_TRUE;
}

/**
 * Stops the process.
 *
 * @param int|float $timeout The timeout in seconds
 * @param int $signal  A POSIX signal to send in case the process has not stop at timeout, default is SIGKILL (9)
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Proc, stop){

	zval *timeout = NULL, *_signal = NULL, signal = {}, process = {}, ppid = {}, command = {};
	zval ret = {}, pattern = {}, pids = {}, *pid, status = {}, isrunning = {};
	double mtime, curtime;

	phalcon_fetch_params(1, 0, 2, &timeout, &_signal);

	phalcon_read_property(&status, getThis(), SL("_status"), PH_NOISY|PH_READONLY);
	if (!PHALCON_IS_LONG(&status, PHALCON_PROCESS_STATUS_STARTED)) {
		RETURN_MM_TRUE;
	}

	if (!timeout){
		mtime = 10;
	} else {
		mtime = Z_DVAL_P(timeout);
	}

	if (!_signal){
		ZVAL_LONG(&signal, 15);
	} else {
		ZVAL_COPY(&signal, _signal);
	}

	phalcon_read_property(&process, getThis(), SL("_process"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&ppid, getThis(), SL("_pid"), PH_NOISY|PH_READONLY);

	if (zend_is_true(&ppid)) {
		PHALCON_CONCAT_SV(&command, "ps -o pid --no-heading --ppid ", &ppid);
		PHALCON_MM_ADD_ENTRY(&command);
		PHALCON_MM_CALL_FUNCTION(&ret, "shell_exec", &command);
		PHALCON_MM_ADD_ENTRY(&ret);

		if (zend_is_true(&ret)) {
			PHALCON_MM_ZVAL_STRING(&pattern, "/\\s+/");
			PHALCON_MM_CALL_FUNCTION(&pids, "preg_split", &pattern, &ret);
			PHALCON_MM_ADD_ENTRY(&pids);
			if (Z_TYPE(pids) == IS_ARRAY) {
				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(pids), pid) {
					PHALCON_CONCAT_SV(&command, "kill -9 ", pid);
					PHALCON_MM_ADD_ENTRY(&command);
					PHALCON_MM_CALL_FUNCTION(NULL, "exec", &command);
				} ZEND_HASH_FOREACH_END();
			}
		}
	}

	if (zend_is_true(&process)) {
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "close");

		curtime = phalcon_get_time();
		mtime = mtime + curtime;
		do {
			PHALCON_MM_CALL_FUNCTION(NULL, "proc_terminate", &process, &signal);
			PHALCON_MM_CALL_METHOD(&isrunning, getThis(), "isrunning");
			if (zend_is_true(&isrunning)) {
				usleep(1000);
			}
		} while (zend_is_true(&isrunning) && phalcon_get_time() < mtime);
		PHALCON_MM_CALL_FUNCTION(NULL, "proc_close", &process);
	} else {
		PHALCON_CONCAT_SVSV(&command, "kill -", &signal, " ", &ppid);
		PHALCON_MM_ADD_ENTRY(&command);
		curtime = phalcon_get_time();
		mtime = mtime + curtime;
		do {
			PHALCON_MM_CALL_FUNCTION(NULL, "exec", &command);
			PHALCON_MM_CALL_METHOD(&isrunning, getThis(), "isrunning");
			if (zend_is_true(&isrunning)) {
				usleep(1000);
			}
		} while (zend_is_true(&isrunning) && phalcon_get_time() < mtime);

		if (zend_is_true(&isrunning)) {
			PHALCON_CONCAT_SV(&command, "kill -9 ", &ppid);
			PHALCON_MM_ADD_ENTRY(&command);
			PHALCON_MM_CALL_FUNCTION(NULL, "exec", &command);
		}
	}

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "update");

	RETURN_MM_TRUE;
}

PHP_METHOD(Phalcon_Process_Proc, close){

	zval pipes = {}, *s;

	phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(pipes) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(pipes), s) {
			PHALCON_CALL_FUNCTION(NULL, "pclose", s);
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_update_property_null(getThis(), SL("_pipes"));
}

/**
 * Restarts the process
 */
PHP_METHOD(Phalcon_Process_Proc, reStart){

	PHALCON_CALL_METHOD(NULL, getThis(), "stop");
	PHALCON_CALL_METHOD(return_value, getThis(), "start");
}

/**
 * Checks if the process is currently running.
 *
 * @return boolean true if the process is currently running, false otherwise
 */
PHP_METHOD(Phalcon_Process_Proc, isRunning){

	PHALCON_CALL_METHOD(NULL, getThis(), "update");
	RETURN_MEMBER(getThis(), "_running");
}

/**
 * Updates the status of the process, reads pipes.
 *
 * @param boolean $blocking Whether to use a blocking read call
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Proc, update){

	zval *blocking = NULL, pid = {}, pid_file = {}, process = {}, information = {}, running = {}, exitcode = {};
	zval signaled = {}, termsig = {}, stopped = {}, stopsig = {}, ret = {};

	PHALCON_MM_INIT();

	if (!blocking) {
		blocking = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&process, getThis(), SL("_process"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(process) != IS_NULL) {
		PHALCON_CALL_FUNCTION(&information, "proc_get_status", &process);
		PHALCON_MM_ADD_ENTRY(&information);
		if (Z_TYPE(information) == IS_ARRAY) {
			phalcon_array_fetch_str(&pid, &information, SL("pid"), PH_NOISY|PH_READONLY);
			phalcon_update_property(getThis(), SL("_pid"), &pid);

			phalcon_read_property(&pid_file, getThis(), SL("_pidFile"), PH_NOISY|PH_READONLY);
			if (zend_is_true(&pid) && zend_is_true(&pid_file)) {
				PHALCON_MM_CALL_FUNCTION(NULL, "file_put_contents", &pid_file, &pid);
			}

			phalcon_array_fetch_str(&running, &information, SL("running"), PH_NOISY|PH_READONLY);
			phalcon_update_property(getThis(), SL("_running"), &running);

			if (!zend_is_true(&running)) {
				if (phalcon_array_isset_fetch_str(&exitcode, &information, SL("exitcode"), PH_READONLY)) {
					if (Z_LVAL(exitcode) != -1) {
						phalcon_update_property(getThis(), SL("_exitCode"), &exitcode);
					}
				}
				phalcon_array_fetch_str(&signaled, &information, SL("signaled"), PH_NOISY|PH_READONLY);
				if (zend_is_true(&signaled)) {
					if (phalcon_array_isset_fetch_str(&termsig, &information, SL("termsig"), PH_READONLY) && Z_LVAL(termsig) > 0) {
						phalcon_update_property_long(getThis(), SL("_exitCode"), 128 + Z_LVAL(termsig));
						phalcon_update_property(getThis(), SL("_exitCode"), &termsig);
					}
					phalcon_update_property_bool(getThis(), SL("_signaled"), 1);
				}

				phalcon_update_property_long(getThis(), SL("_status"), PHALCON_PROCESS_STATUS_TERMINATED);
			} else {
				phalcon_array_fetch_str(&stopped, &information, SL("stopped"), PH_NOISY|PH_READONLY);
				if (zend_is_true(&stopped)) {
					if (phalcon_array_isset_fetch_str(&stopsig, &information, SL("stopsig"), PH_READONLY)) {
						phalcon_update_property_long(getThis(), SL("_stopCode"), Z_LVAL(stopsig));
					}
					phalcon_update_property_long(getThis(), SL("_status"), PHALCON_PROCESS_STATUS_STOP);
				}
			}
		}
	} else {
		phalcon_read_property(&pid, getThis(), SL("_pid"), PH_NOISY|PH_READONLY);
		phalcon_read_property(&pid_file, getThis(), SL("_pidFile"), PH_NOISY|PH_READONLY);
		if (!zend_is_true(&pid) && zend_is_true(&pid_file)) {
			PHALCON_MM_CALL_FUNCTION(&pid, "file_get_contents", &pid_file);
			PHALCON_MM_ADD_ENTRY(&pid);
			phalcon_update_property(getThis(), SL("_pid"), &pid);
		}

		PHALCON_MM_CALL_METHOD(&ret, getThis(), "getstatforpid", &pid);
		PHALCON_MM_ADD_ENTRY(&ret);
		if (zend_is_true(&ret)) {
			phalcon_update_property_long(getThis(), SL("_status"), PHALCON_PROCESS_STATUS_STARTED);
			phalcon_update_property_bool(getThis(), SL("_running"), 1);
		} else {
			phalcon_update_property_bool(getThis(), SL("_running"), 0);
			phalcon_update_property_long(getThis(), SL("_status"), PHALCON_PROCESS_STATUS_TERMINATED);
		}
	}

	RETURN_MM_TRUE;
}

PHP_METHOD(Phalcon_Process_Proc, getCommandForPid){

	zval *pid, command = {}, stdin = {}, stdout = {}, stderr = {}, descriptors = {}, pipes = {}, process ={}, information ={};
	zval running = {}, pipe = {}, ret = {};

	phalcon_fetch_params(1, 1, 0, &pid);

	// ps eh -o args -p pid
	PHALCON_CONCAT_SV(&command, "ps -o args -p ", pid);
	PHALCON_MM_ADD_ENTRY(&command);

	array_init_size(&stdin, 2);
	phalcon_array_append_str(&stdin, SL("pipe"), 0);
	phalcon_array_append_str(&stdin, SL("r"), 0);
	PHALCON_MM_ADD_ENTRY(&stdin);

	array_init_size(&stdout, 2);
	phalcon_array_append_str(&stdout, SL("pipe"), 0);
	phalcon_array_append_str(&stdout, SL("w"), 0);
	PHALCON_MM_ADD_ENTRY(&stdout);

	array_init_size(&stderr, 2);
	phalcon_array_append_str(&stderr, SL("pipe"), 0);
	phalcon_array_append_str(&stderr, SL("w"), 0);
	PHALCON_MM_ADD_ENTRY(&stderr);

	array_init_size(&descriptors, 3);
	phalcon_array_append(&descriptors, &stdin, 0);
	phalcon_array_append(&descriptors, &stdout, 0);
	phalcon_array_append(&descriptors, &stderr, 0);
	PHALCON_MM_ADD_ENTRY(&descriptors);

	ZVAL_MAKE_REF(&pipes);
	PHALCON_MM_CALL_FUNCTION(&process, "proc_open", &command, &descriptors, &pipes);
	ZVAL_UNREF(&pipes);

	PHALCON_MM_ADD_ENTRY(&process);

	if (Z_TYPE(process) != IS_RESOURCE) {
		RETURN_MM_FALSE;
	}
	PHALCON_MM_CALL_FUNCTION(&information, "proc_get_status", &process);
	PHALCON_MM_ADD_ENTRY(&information);
	if (Z_TYPE(information) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_str(&running, &information, SL("running"), PH_READONLY) || !zend_is_true(&running)) {
			RETURN_MM_FALSE;
		}
		if (phalcon_array_isset_fetch_long(&pipe, &pipes, 2, PH_READONLY)) {
			PHALCON_MM_CALL_FUNCTION(&ret, "fgets", &pipe);
			PHALCON_MM_ADD_ENTRY(&ret);
			if (!PHALCON_IS_FALSE(&ret)) {
				RETURN_MM_FALSE;
			}
		}
		if (!phalcon_array_isset_fetch_long(&pipe, &pipes, 1, PH_READONLY)) {
			RETURN_MM_FALSE;
		}
		PHALCON_MM_CALL_FUNCTION(NULL, "fgets", &pipe);
		PHALCON_MM_CALL_FUNCTION(return_value, "fgets", &pipe);
	}
	RETURN_MM();
}

PHP_METHOD(Phalcon_Process_Proc, getStarttimeForPid){

	zval *pid;

	phalcon_fetch_params(0, 1, 0, &pid);

	ZVAL_LONG(return_value, phalcon_get_proc_starttime(Z_LVAL_P(pid)));
}

PHP_METHOD(Phalcon_Process_Proc, getStatForPid){

	zval *pid, command = {}, stdin = {}, stdout = {}, stderr = {}, descriptors = {}, pipes = {}, process ={}, information ={};
	zval running = {}, pipe = {}, ret = {};

	phalcon_fetch_params(1, 1, 0, &pid);

	PHALCON_CONCAT_SV(&command, "ps h -o stat -p ", pid);
	PHALCON_MM_ADD_ENTRY(&command);

	array_init_size(&stdin, 2);
	phalcon_array_append_str(&stdin, SL("pipe"), 0);
	phalcon_array_append_str(&stdin, SL("r"), 0);
	PHALCON_MM_ADD_ENTRY(&stdin);

	array_init_size(&stdout, 2);
	phalcon_array_append_str(&stdout, SL("pipe"), 0);
	phalcon_array_append_str(&stdout, SL("w"), 0);
	PHALCON_MM_ADD_ENTRY(&stdout);

	array_init_size(&stderr, 2);
	phalcon_array_append_str(&stderr, SL("pipe"), 0);
	phalcon_array_append_str(&stderr, SL("w"), 0);
	PHALCON_MM_ADD_ENTRY(&stderr);

	array_init_size(&descriptors, 3);
	phalcon_array_append(&descriptors, &stdin, 0);
	phalcon_array_append(&descriptors, &stdout, 0);
	phalcon_array_append(&descriptors, &stderr, 0);
	PHALCON_MM_ADD_ENTRY(&descriptors);

	ZVAL_MAKE_REF(&pipes);
	PHALCON_CALL_FUNCTION(&process, "proc_open", &command, &descriptors, &pipes);
	ZVAL_UNREF(&pipes);
	PHALCON_MM_ADD_ENTRY(&process);

	if (Z_TYPE(process) != IS_RESOURCE) {
		RETURN_MM_FALSE;
	}
	PHALCON_MM_CALL_FUNCTION(&information, "proc_get_status", &process);
	if (Z_TYPE(information) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_str(&running, &information, SL("running"), PH_READONLY) || !zend_is_true(&running)) {
			RETURN_MM_FALSE;
		}
		if (phalcon_array_isset_fetch_long(&pipe, &pipes, 2, PH_READONLY)) {
			PHALCON_MM_CALL_FUNCTION(&ret, "fgets", &pipe);
			PHALCON_MM_ADD_ENTRY(&ret);
			if (!PHALCON_IS_FALSE(&ret)) {
				RETURN_MM_FALSE;
			}
		}
		if (!phalcon_array_isset_fetch_long(&pipe, &pipes, 1, PH_READONLY)) {
			RETURN_MM_FALSE;
		}
		PHALCON_MM_CALL_FUNCTION(&ret, "fgets", &pipe);
		PHALCON_MM_ADD_ENTRY(&ret);
		if (zend_is_true(&ret)) {
			RETURN_MM_TRUE;
		}
	}
	RETURN_MM_FALSE;
}

/**
 * Performs a check between the timeout definition and the time the process started
 *
 * In case you run a background process (with the start method), you should
 * trigger this method regularly to ensure the process timeout
 *
 * @throws Phalcon\Process\Exception In case the timeout was reached
 */
PHP_METHOD(Phalcon_Process_Proc, checkTimeout){

	zval status = {}, timeout = {}, start_time = {}, microtime = {};

	phalcon_read_property(&status, getThis(), SL("_status"), PH_NOISY|PH_READONLY);
	if (!PHALCON_IS_LONG(&status, PHALCON_PROCESS_STATUS_STARTED)) {
		RETURN_FALSE;
	}

	phalcon_read_property(&timeout, getThis(), SL("_timeout"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(timeout) == IS_NULL) {
		RETURN_TRUE;
	}

	phalcon_read_property(&start_time, getThis(), SL("_startTime"), PH_NOISY|PH_READONLY);
	phalcon_microtime(&microtime, &PHALCON_GLOBAL(z_true));
	if (PHALCON_LE_LONG(&timeout, Z_DVAL(microtime) - Z_DVAL(start_time))) {
		PHALCON_CALL_METHOD(NULL, getThis(), "stop", &PHALCON_GLOBAL(z_zero));
		PHALCON_THROW_EXCEPTION_STR(phalcon_process_exception_ce, "Process time-out");
		return;
	}
	RETURN_TRUE;
}

/**
 * Returns the Pid (process identifier), if applicable.
 *
 * @return int|null The process id if running, null otherwise
 */
PHP_METHOD(Phalcon_Process_Proc, getPid){

	RETURN_MEMBER(getThis(), "_pid");
}

/**
 * Waits for the process to terminate
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Proc, wait){

	zval status = {}, isrunning = {};

	phalcon_read_property(&status, getThis(), SL("_status"), PH_NOISY|PH_READONLY);
	if (!PHALCON_IS_LONG(&status, PHALCON_PROCESS_STATUS_STARTED)) {
		RETURN_FALSE;
	}
	PHALCON_CALL_METHOD(&isrunning, getThis(), "isrunning");
	while (zend_is_true(&isrunning)) {
		usleep(1000);
		PHALCON_CALL_METHOD(NULL, getThis(), "checktimeout");
		PHALCON_CALL_METHOD(&isrunning, getThis(), "isrunning");
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
PHP_METHOD(Phalcon_Process_Proc, read){

	zval *type, *timeout = NULL, pipes = {}, pipe = {};

	phalcon_fetch_params(0, 1, 1, &type, &timeout);

	if (!timeout) {
		timeout = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(pipes) != IS_ARRAY || !phalcon_array_isset_fetch(&pipe, &pipes, type, 0)) {
		RETURN_FALSE;
	}

	PHALCON_CALL_FUNCTION(return_value, "fgets", &pipe);
}

/**
 * Writes data to stdin
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Proc, write){

	zval *data, pipes = {}, stdin = {}, r = {}, w = {}, e = {}, ret = {}, writebuf = {};
	int len;

	phalcon_fetch_params(0, 1, 0, &data);

	phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(pipes) != IS_ARRAY || !phalcon_array_isset_fetch_long(&stdin, &pipes, 0, PH_READONLY)) {
		RETURN_FALSE;
	}

	array_init(&r);
	array_init(&w);
	array_init(&e);
	phalcon_array_append(&w, &stdin, PH_COPY);

	ZVAL_MAKE_REF(&r);
	ZVAL_MAKE_REF(&w);
	ZVAL_MAKE_REF(&e);
	PHALCON_CALL_FUNCTION(&ret, "stream_select", &r, &w, &e, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
	ZVAL_UNREF(&r);
	ZVAL_UNREF(&w);
	ZVAL_UNREF(&e);

	if (Z_TYPE(ret) != IS_LONG || PHALCON_LE_LONG(&ret, 0)) {
		RETURN_FALSE;
	}

	len = Z_STRLEN_P(data);
	ZVAL_LONG(&ret, 0);
	ZVAL_DUP(&writebuf, data);
	while(1) {
		zval writelen = {};
		ZVAL_LONG(&writelen, len);
		PHALCON_CALL_FUNCTION(&ret, "fwrite", &stdin, &writebuf, &writelen);

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
 * Handle the process
 *
 * @param callable $onrecv
 * @param callable $onsend
 * @param callable $onerror
 * @param callable $ontimeout
 * @param int $timeout
 */
PHP_METHOD(Phalcon_Process_Proc, handle){

	zval *_onrecv, *_onsend = NULL, *_onerror = NULL, *_ontimeout = NULL, *timeout = NULL;
	zval isrunning = {}, onrecv = {}, onsend = {}, onerror = {}, ontimeout = {}, pipes = {}, maxlen = {};
	int flag;

	phalcon_fetch_params(1, 0, 5, &_onrecv, &_onsend, &_onerror, &_ontimeout, &timeout);

	phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(pipes) != IS_ARRAY || !phalcon_fast_count_ev(&pipes)) {
		RETURN_MM_FALSE;
	}

	if (_onrecv) {
		if (Z_TYPE_P(_onrecv) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onrecv), zend_ce_closure, 0)) {
			PHALCON_MM_CALL_CE_STATIC(&onrecv, zend_ce_closure, "bind", _onrecv, getThis());
		} else {
			ZVAL_COPY(&onrecv, _onrecv);
		}
		PHALCON_MM_ADD_ENTRY(&onrecv);
	}

	if (_onsend) {
		if (Z_TYPE_P(_onsend) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onsend), zend_ce_closure, 0)) {
			PHALCON_MM_CALL_CE_STATIC(&onsend, zend_ce_closure, "bind", _onsend, getThis());
		} else {
			ZVAL_COPY(&onsend, _onsend);
		}
		PHALCON_MM_ADD_ENTRY(&onsend);
	}

	if (_onerror) {
		if (Z_TYPE_P(_onerror) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_onerror), zend_ce_closure, 0)) {
			PHALCON_MM_CALL_CE_STATIC(&onerror, zend_ce_closure, "bind", _onerror, getThis());
		} else {
			ZVAL_COPY(&onerror, _onerror);
		}
		PHALCON_MM_ADD_ENTRY(&onerror);
	}

	if (_ontimeout) {
		if (Z_TYPE_P(_ontimeout) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(_ontimeout), zend_ce_closure, 0)) {
			PHALCON_MM_CALL_CE_STATIC(&ontimeout, zend_ce_closure, "bind", _ontimeout, getThis());
		} else {
			ZVAL_COPY(&ontimeout, _ontimeout);
		}
		PHALCON_MM_ADD_ENTRY(&ontimeout);
	}

	if (!timeout) {
		timeout = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_LONG(&maxlen, 1024);
	while(1) {
		zval r_array = {}, w_array = {}, e_array = {}, ret = {}, stdin = {}, stdout = {}, stderr = {}, *s;

		PHALCON_MM_CALL_METHOD(&isrunning, getThis(), "isrunning");
		if (!zend_is_true(&isrunning)) {
			break;
		}

		array_init(&r_array);
		array_init(&w_array);
		array_init(&e_array);

		if (phalcon_array_isset_fetch_long(&stdout, &pipes, PHALCON_PROCESS_STDOUT, PH_READONLY)) {
			phalcon_array_append(&r_array, &stdout, PH_COPY);
		}

		if (phalcon_array_isset_fetch_long(&stdin, &pipes, PHALCON_PROCESS_STDIN, PH_READONLY)) {
			phalcon_array_append(&w_array, &stdin, PH_COPY);
		}

		if (phalcon_array_isset_fetch_long(&stderr, &pipes, PHALCON_PROCESS_STDERR, PH_READONLY)) {
			phalcon_array_append(&e_array, &stderr, PH_COPY);
		}

		ZVAL_MAKE_REF(&r_array);
		ZVAL_MAKE_REF(&w_array);
		ZVAL_MAKE_REF(&e_array);
		PHALCON_MM_CALL_FUNCTION(&ret, "stream_select", &r_array, &w_array, &e_array, timeout, &PHALCON_GLOBAL(z_zero));
		ZVAL_UNREF(&r_array);
		ZVAL_UNREF(&w_array);
		ZVAL_UNREF(&e_array);
		if (PHALCON_IS_FALSE(&ret)) {
			goto error;
		}
		if (PHALCON_IS_LONG_IDENTICAL(&ret, 0)) {
			if (phalcon_method_exists_ex(getThis(), SL("ontimeout")) == SUCCESS) {
				PHALCON_CALL_METHOD_FLAG(flag, NULL, getThis(), "ontimeout");
			}
			if (Z_TYPE(ontimeout) > IS_NULL) {
				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &ontimeout);
			}
			goto next;
		}
		if (Z_TYPE(r_array) == IS_ARRAY) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(r_array), s) {
				zval data = {};
				do {
					PHALCON_CALL_FUNCTION_FLAG(flag, &data, "fgets", s);
					if (flag == FAILURE || !zend_is_true(&data)) {
						goto error;
						break;
					}
					if (phalcon_method_exists_ex(getThis(), SL("onrecv")) == SUCCESS) {
						PHALCON_CALL_METHOD_FLAG(flag, NULL, getThis(), "onrecv", s, &data);
					}
					if (Z_TYPE(onrecv) > IS_NULL) {
						zval args[2] = {};
						ZVAL_COPY_VALUE(&args[0], s);
						ZVAL_COPY_VALUE(&args[1], &data);
						PHALCON_CALL_USER_FUNC_ARGS_FLAG(flag, NULL, &onrecv, args, 2);
					}
				} while (zend_is_true(&data));
			} ZEND_HASH_FOREACH_END();
		}
		if (Z_TYPE(w_array) == IS_ARRAY) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(w_array), s) {
				if (phalcon_method_exists_ex(getThis(), SL("onsend")) == SUCCESS) {
					PHALCON_CALL_METHOD_FLAG(flag, NULL, getThis(), "onsend", s);
				}
				if (Z_TYPE(onsend) > IS_NULL) {
					zval args[1] = {};
					ZVAL_COPY_VALUE(&args[0], s);
					PHALCON_CALL_USER_FUNC_ARGS_FLAG(flag, NULL, &onsend, args, 1);
				}
			} ZEND_HASH_FOREACH_END();
		}
		if (Z_TYPE(e_array) == IS_ARRAY) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(e_array), s) {
				zval data = {};
				do {
					PHALCON_CALL_FUNCTION_FLAG(flag, &data, "fgets", s);
					if (!zend_is_true(&data)) {
						goto error;
						break;
					}
					if (phalcon_method_exists_ex(getThis(), SL("onerror")) == SUCCESS) {
						PHALCON_CALL_METHOD(NULL, getThis(), "onerror", s, &data);
					}
					if (Z_TYPE(onerror) > IS_NULL) {
						zval args[2] = {};
						ZVAL_COPY_VALUE(&args[0], s);
						ZVAL_COPY_VALUE(&args[1], &data);
						PHALCON_CALL_USER_FUNC_ARGS_FLAG(flag, NULL, &onerror, args, 2);
					}
				} while (zend_is_true(&data));
			} ZEND_HASH_FOREACH_END();
		}
next:
		zval_ptr_dtor(&r_array);
		zval_ptr_dtor(&w_array);
		zval_ptr_dtor(&e_array);
		continue;
error:
		zval_ptr_dtor(&r_array);
		zval_ptr_dtor(&w_array);
		zval_ptr_dtor(&e_array);
		break;
	}
	RETURN_MM_TRUE;
}

/**
 * Returns true if a system call has been interrupted
 *
 * @return bool
 */
PHP_METHOD(Phalcon_Process_Proc, hasSystemCallBeenInterrupted){

	zval last_error = {}, message = {};

	PHALCON_CALL_FUNCTION(&last_error, "error_get_last");
	if (Z_TYPE(last_error) == IS_ARRAY && phalcon_array_isset_fetch_str(&message, &last_error, SL("message"), PH_READONLY)) {
		phalcon_fast_stripos_str(return_value, &message, SL("interrupted system call"));
	} else {
		RETVAL_FALSE;
	}
	zval_ptr_dtor(&last_error);
}

/**
 * Set streams to blocking / non blocking
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Process_Proc, setBlocking){

	zval *flag, pipes = {}, blocked = {}, *s;

	phalcon_fetch_params(0, 1, 0, &flag);

	phalcon_read_property(&pipes, getThis(), SL("_pipes"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(pipes) != IS_ARRAY && !phalcon_fast_count_ev(&pipes)) {
		RETURN_FALSE;
	}

	phalcon_read_property(&blocked, getThis(), SL("_blocking"), PH_NOISY|PH_READONLY);

	if (zend_is_true(&blocked) == zend_is_true(flag)) {
		RETURN_TRUE;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(pipes), s) {
		PHALCON_CALL_FUNCTION(NULL, "stream_set_blocking", s, flag);
	} ZEND_HASH_FOREACH_END();

	phalcon_update_property(getThis(), SL("_blocking"), flag);
}
