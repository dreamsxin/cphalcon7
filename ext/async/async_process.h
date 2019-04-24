
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
  |          Martin Schröder <m.schroeder2007@gmail.com>                   |
  +------------------------------------------------------------------------+
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

typedef struct {
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
