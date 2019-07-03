/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Martin Schröder <m.schroeder2007@gmail.com>                 |
  +----------------------------------------------------------------------+
*/

#ifndef ASYNC_PROCESS_H
#define ASYNC_PROCESS_H

#define ASYNC_PROCESS_STDIN 0
#define ASYNC_PROCESS_STDOUT 1
#define ASYNC_PROCESS_STDERR 2
#define ASYNC_PROCESS_IPC 3

#define ASYNC_PROCESS_FLAG_IPC 1
#define ASYNC_PROCESS_FLAG_INHERIT_ENV (1 << 1)
#define ASYNC_PROCESS_FLAG_INTERACTIVE_SHELL (1 << 2)

typedef struct _async_process_builder {
	/* Fiber PHP object handle. */
	zend_object std;

	uint8_t flags;

	/* Command to be executed (without arguments). */
	zend_string *command;

	/* Number of additional arguments. */
	uint32_t argc;

	/* Additional args passed to the base command. */
	zval *argv;

	/* Current working directory for the process. */
	zend_string *cwd;

	/* Environment vars to be passed to the created process. */
	zval env;

	/* STDIO pipe definitions for STDIN, STDOUT and STDERR. */
	uv_stdio_container_t stdio[4];

	/* Use UV_PROCESS_DETACHED */
	zend_bool detached;
} async_process_builder;

char **async_process_create_env(HashTable *add, int inherit);
int async_process_setup_shell(async_process_builder *builder, int interactive);

int async_process_execute(async_process_builder *builder, uint32_t argc, zval *argv);
zend_object *async_process_start(async_process_builder *builder, uint32_t argc, zval *argv);

void async_process_builder_ce_register();

#endif
