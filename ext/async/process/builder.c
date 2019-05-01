
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

#include "async/core.h"

#if PHALCON_USE_UV

#include "async/async_process.h"
#include "kernel/backend.h"

#include <main/SAPI.h>

PHPAPI extern char *php_ini_opened_path;

ASYNC_API zend_class_entry *async_process_builder_ce;

static zend_object_handlers async_process_builder_handlers;

#define ASYNC_PROCESS_BUILDER_CONST(const_name, value) \
	zend_declare_class_constant_long(async_process_builder_ce, const_name, sizeof(const_name)-1, (zend_long)value);


static zend_object *async_process_builder_object_create(zend_class_entry *ce)
{
	async_process_builder *builder;
	
	char path[MAXPATHLEN];
	char *ret;

	builder = ecalloc(1, sizeof(async_process_builder));

	zend_object_std_init(&builder->std, ce);
	builder->std.handlers = &async_process_builder_handlers;

	ZVAL_UNDEF(&builder->env);

	builder->flags |= ASYNC_PROCESS_FLAG_INHERIT_ENV;
	
#if HAVE_GETCWD
	ret = VCWD_GETCWD(path, MAXPATHLEN);
#elif HAVE_GETWD
	ret = VCWD_GETWD(path);
#endif

	if (EXPECTED(ret)) {
		builder->cwd = zend_string_init(path, strlen(path), 0);
	}

	return &builder->std;
}

static async_process_builder *clone_builder(async_process_builder *builder)
{
	async_process_builder *clone;
	
	uint32_t i;
	
	clone = ecalloc(1, sizeof(async_process_builder));
	
	zend_object_std_init(&clone->std, builder->std.ce);
	clone->std.handlers = builder->std.handlers;
	
	clone->flags = builder->flags;
	clone->command = zend_string_copy(builder->command);
	clone->argc = builder->argc;
	
	if (builder->argc > 0) {
		clone->argv = ecalloc(builder->argc, sizeof(zval));
	
		for (i = 0; i < builder->argc; i++) {
			ZVAL_COPY(&clone->argv[i], &builder->argv[i]);
		}
	}
	
	if (builder->cwd) {
		clone->cwd = zend_string_copy(builder->cwd);
	}
	
	ZVAL_COPY(&clone->env, &builder->env);
	
	for (i = 0; i < 4; i++) {
		clone->stdio[i] = builder->stdio[i];
	}
	
	return clone;
}

static void async_process_builder_object_destroy(zend_object *object)
{
	async_process_builder *builder;
	uint32_t i;

	builder = (async_process_builder *) object;

	zend_string_release(builder->command);

	if (builder->cwd != NULL) {
		zend_string_release(builder->cwd);
	}

	if (builder->argc > 0) {
		for (i = 0; i < builder->argc; i++) {
			zval_ptr_dtor(&builder->argv[i]);
		}

		efree(builder->argv);
	}

	zval_ptr_dtor(&builder->env);

	zend_object_std_dtor(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, __construct)
{
	async_process_builder *builder;
	
	zend_string *cmd;
	zval *argv;

	uint32_t argc;
	uint32_t i;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, -1)
		Z_PARAM_STR(cmd)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', argv, argc)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());
	
	builder->command = zend_string_copy(cmd);
	builder->argc = argc;

	if (argc > 0) {
		builder->argv = ecalloc(argc, sizeof(zval));

		for (i = 0; i < argc; i++) {
			ZVAL_COPY(&builder->argv[i], &argv[i]);
		}
	}
}

static char **split_ini_settings(char *settings, int *count)
{
	char **ini;
	int c;
	
	size_t i;
	size_t j;
	size_t len;
	
	ini = NULL;
	c = 0;
	
	if (settings) {
		for (len = strlen(settings), j = 0, i = 0; i < len; i++) {
			if (UNEXPECTED(settings[i] == '\n')) {
				if (c++ == 0) {
					ini = emalloc(sizeof(char *));
				} else {
					ini = erealloc(ini, c * sizeof(char *));
				}
				
				ini[c - 1] = emalloc(i - j + 3);
				sprintf(ini[c - 1], "-d%.*s", (int) (i - j), settings + j);
				
				j = i + 1;
			}
		}
	}
	
	*count = c;
	
	return ini;
}

static ZEND_METHOD(ProcessBuilder, fork)
{
	async_process_builder *builder;
	
	zend_string *ini;
	char **settings;
	
	zval *file;
	
	uint32_t i;
	
	int count;
	int j;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(file)
	ZEND_PARSE_PARAMETERS_END();
	
	builder = (async_process_builder *) async_process_builder_object_create(async_process_builder_ce);
	
	builder->command = zend_string_init(PG(php_binary), strlen(PG(php_binary)), 0);
	
	settings = split_ini_settings(sapi_module.ini_entries, &count);
	i = 0;
	
	builder->argc = count + (php_ini_opened_path ? 3 : 2);
	builder->argv = ecalloc(builder->argc, sizeof(zval));
	
	// Inherit used INI file.
	if (php_ini_opened_path) {
		ASYNC_STRF(ini, "-c%s", php_ini_opened_path);
		ZVAL_STR(&builder->argv[i++], ini);
	}
	
	// Inherit all explicit INI settings.
	if (count > 0) {
		for (j = 0; j < count; j++) {
			ZVAL_STRING(&builder->argv[i++], settings[j]);
			
			efree(settings[j]);
		}
		
		efree(settings);
	}
	
	// Mark new process as forked worker and setup input file.
	ZVAL_STRING(&builder->argv[i++], "-dphalcon.async.forked=1");
	ZVAL_COPY(&builder->argv[i++], file);
	
	builder->flags |= ASYNC_PROCESS_FLAG_IPC;
	builder->flags |= ASYNC_PROCESS_FLAG_INHERIT_ENV;
	
	builder->stdio[ASYNC_PROCESS_STDIN].flags = UV_IGNORE;
	builder->stdio[ASYNC_PROCESS_STDOUT].flags = UV_INHERIT_FD;
	builder->stdio[ASYNC_PROCESS_STDOUT].data.fd = ASYNC_PROCESS_STDOUT;
	builder->stdio[ASYNC_PROCESS_STDERR].flags = UV_INHERIT_FD;
	builder->stdio[ASYNC_PROCESS_STDERR].data.fd = ASYNC_PROCESS_STDERR;
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, shell)
{
	async_process_builder *builder;
	
	zend_long interactive;
	
	interactive = 0;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(interactive)
	ZEND_PARSE_PARAMETERS_END();
	
	builder = (async_process_builder *) async_process_builder_object_create(async_process_builder_ce);
	
	ASYNC_CHECK_ERROR(FAILURE == async_process_setup_shell(builder, interactive ? 1 : 0), "Failed to setup shell");
	
	if (interactive) {
		builder->flags |= ASYNC_PROCESS_FLAG_INTERACTIVE_SHELL;
		
		builder->stdio[ASYNC_PROCESS_STDIN].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;
		builder->stdio[ASYNC_PROCESS_STDOUT].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
		builder->stdio[ASYNC_PROCESS_STDERR].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
	}
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withCwd)
{
	async_process_builder *builder;
	
	zend_string *cwd;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(cwd)
	ZEND_PARSE_PARAMETERS_END();
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	if (builder->cwd) {
		zend_string_release(builder->cwd);
	}
	
	builder->cwd = zend_string_copy(cwd);
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withEnv)
{
	async_process_builder *builder;
	
	zval *env;
	zend_ulong i;
	zend_string *k;
	zval *entry;
	zval tmp;
	
	zend_long inherit;
	
	inherit = 0;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_ARRAY_EX(env, 1, 0)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(inherit)
	ZEND_PARSE_PARAMETERS_END();
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	zval_ptr_dtor(&builder->env);
	
	array_init_size(&builder->env, zend_hash_num_elements(Z_ARRVAL_P(env)));
	
	ZEND_HASH_FOREACH_KEY_VAL_IND(Z_ARRVAL_P(env), i, k, entry) {
		if (UNEXPECTED(k == NULL)) {
			zend_throw_error(NULL, "Cannot use index %lu as env var name", i);
			break;
		}
		
		ZVAL_COPY(&tmp, entry);
	
		if (UNEXPECTED(Z_TYPE_P(entry) != IS_STRING)) {
			convert_to_string(&tmp);
			
			ASYNC_BREAK_ON_ERROR();
		}
		
		zend_hash_add(Z_ARRVAL_P(&builder->env), k, &tmp);
	} ZEND_HASH_FOREACH_END();
	
	if (UNEXPECTED(EG(exception))) {
		ASYNC_DELREF(&builder->std);
		return;
	}
	
	if (inherit) {
		builder->flags |= ASYNC_PROCESS_FLAG_INHERIT_ENV;
	} else {
		builder->flags &= ~ASYNC_PROCESS_FLAG_INHERIT_ENV;
	}
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withStdinPipe)
{
	async_process_builder *builder;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	builder->stdio[ASYNC_PROCESS_STDIN].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withStdinInherited)
{
	async_process_builder *builder;
	
	zend_long fd;
	
	fd = ASYNC_PROCESS_STDIN;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(fd)
	ZEND_PARSE_PARAMETERS_END();
	
	ASYNC_CHECK_ERROR(fd != ASYNC_PROCESS_STDIN, "Child process STDIN can only inherit from parent STDIN");
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	builder->stdio[ASYNC_PROCESS_STDIN].flags = UV_INHERIT_FD;
	builder->stdio[ASYNC_PROCESS_STDIN].data.fd = (int) fd;
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withoutStdin)
{
	async_process_builder *builder;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	builder->stdio[ASYNC_PROCESS_STDIN].flags = UV_IGNORE;
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withStdoutPipe)
{
	async_process_builder *builder;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	builder->stdio[ASYNC_PROCESS_STDOUT].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withStdoutInherited)
{
	async_process_builder *builder;
	
	zend_long fd;
	
	fd = ASYNC_PROCESS_STDOUT;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(fd)
	ZEND_PARSE_PARAMETERS_END();
	
	ASYNC_CHECK_ERROR(fd != ASYNC_PROCESS_STDOUT && fd != ASYNC_PROCESS_STDERR, "Child process STDOUT can only inherit from parent STDOUT or STDERR");
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	builder->stdio[ASYNC_PROCESS_STDOUT].flags = UV_INHERIT_FD;
	builder->stdio[ASYNC_PROCESS_STDOUT].data.fd = (int) fd;
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withoutStdout)
{
	async_process_builder *builder;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	builder->stdio[ASYNC_PROCESS_STDOUT].flags = UV_IGNORE;
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withStderrPipe)
{
	async_process_builder *builder;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	builder->stdio[ASYNC_PROCESS_STDERR].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withStderrInherited)
{
	async_process_builder *builder;
	
	zend_long fd;
	
	fd = ASYNC_PROCESS_STDERR;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(fd)
	ZEND_PARSE_PARAMETERS_END();
	
	ASYNC_CHECK_ERROR(fd != ASYNC_PROCESS_STDOUT && fd != ASYNC_PROCESS_STDERR, "Child process STDERR can only inherit from parent STDERR or STDOUT");
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	builder->stdio[ASYNC_PROCESS_STDERR].flags = UV_INHERIT_FD;
	builder->stdio[ASYNC_PROCESS_STDERR].data.fd = (int) fd;
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, withoutStderr)
{
	async_process_builder *builder;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	builder = clone_builder((async_process_builder *) Z_OBJ_P(getThis()));
	
	builder->stdio[ASYNC_PROCESS_STDERR].flags = UV_IGNORE;
	
	RETURN_OBJ(&builder->std);
}

static ZEND_METHOD(ProcessBuilder, execute)
{
	async_process_builder *builder;
	
	uint32_t argc;
	zval *argv;

	int code;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, -1)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', argv, argc)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());
	
	code = async_process_execute(builder, argc, argv);
	
	if (EXPECTED(code != FAILURE)) {
		RETURN_LONG(code);
	}
}

static ZEND_METHOD(ProcessBuilder, start)
{
	async_process_builder *builder;
	zend_object *proc;

	uint32_t argc;
	zval *argv;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, -1)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', argv, argc)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());
	
	proc = async_process_start(builder, argc, argv);
	
	if (EXPECTED(proc)) {
		RETURN_OBJ(proc);
	}
}

static ZEND_METHOD(ProcessBuilder, daemon)
{
	async_process_builder *builder;
	zend_object *proc;

	uint32_t argc;
	zval *argv;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, -1)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', argv, argc)
	ZEND_PARSE_PARAMETERS_END();

	builder = (async_process_builder *) Z_OBJ_P(getThis());
	builder->detached = 1;
	proc = async_process_start(builder, argc, argv);
	
	if (EXPECTED(proc)) {
		RETURN_OBJ(proc);
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_process_builder_ctor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, arguments, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_fork, 0, 1, Phalcon\\Async\\Process\\ProcessBuilder, 0)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_shell, 0, 0, Phalcon\\Async\\Process\\Builder, 0)
	ZEND_ARG_TYPE_INFO(0, interactive, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_with_cwd, 0, 1, Phalcon\\Async\\Process\\Builder, 0)
	ZEND_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_with_env, 0, 1, Phalcon\\Async\\Process\\Builder, 0)
	ZEND_ARG_TYPE_INFO(0, env, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, inherit, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_with_stdin_pipe, 0, 0, Phalcon\\Async\\Process\\Builder, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_with_stdin_inherited, 0, 0, Phalcon\\Async\\Process\\Builder, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_without_stdin, 0, 0, Phalcon\\Async\\Process\\Builder, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_with_stdout_pipe, 0, 0, Phalcon\\Async\\Process\\Builder, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_with_stdout_inherited, 0, 0, Phalcon\\Async\\Process\\Builder, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_without_stdout, 0, 0, Phalcon\\Async\\Process\\Builder, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_with_stderr_pipe, 0, 0, Phalcon\\Async\\Process\\Builder, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_with_stderr_inherited, 0, 0, Phalcon\\Async\\Process\\Builder, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_without_stderr, 0, 0, Phalcon\\Async\\Process\\Builder, 0)
ZEND_END_ARG_INFO()


#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_execute, 0, 0, IS_LONG, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, arguments, IS_STRING, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_process_builder_execute, 0, 0, IS_LONG, NULL, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, arguments, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_process_builder_start, 0, 0, Phalcon\\Async\\Process, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, arguments, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry async_process_builder_functions[] = {
	ZEND_ME(ProcessBuilder, __construct, arginfo_process_builder_ctor, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, fork, arginfo_process_builder_fork, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(ProcessBuilder, shell, arginfo_process_builder_shell, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(ProcessBuilder, withCwd, arginfo_process_builder_with_cwd, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, withEnv, arginfo_process_builder_with_env, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, withStdinPipe, arginfo_process_builder_with_stdin_pipe, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, withStdinInherited, arginfo_process_builder_with_stdin_inherited, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, withoutStdin, arginfo_process_builder_without_stdin, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, withStdoutPipe, arginfo_process_builder_with_stdout_pipe, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, withStdoutInherited, arginfo_process_builder_with_stdout_inherited, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, withoutStdout, arginfo_process_builder_without_stdout, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, withStderrPipe, arginfo_process_builder_with_stderr_pipe, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, withStderrInherited, arginfo_process_builder_with_stderr_inherited, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, withoutStderr, arginfo_process_builder_without_stderr, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, execute, arginfo_process_builder_execute, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, start, arginfo_process_builder_start, ZEND_ACC_PUBLIC)
	ZEND_ME(ProcessBuilder, daemon, arginfo_process_builder_start, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


void async_process_builder_ce_register()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Process\\Builder", async_process_builder_functions);
	async_process_builder_ce = zend_register_internal_class(&ce);
	async_process_builder_ce->ce_flags |= ZEND_ACC_FINAL;
	async_process_builder_ce->create_object = async_process_builder_object_create;
	async_process_builder_ce->serialize = zend_class_serialize_deny;
	async_process_builder_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_process_builder_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_process_builder_handlers.free_obj = async_process_builder_object_destroy;
	async_process_builder_handlers.clone_obj = NULL;

	ASYNC_PROCESS_BUILDER_CONST("STDIN", ASYNC_PROCESS_STDIN);
	ASYNC_PROCESS_BUILDER_CONST("STDOUT", ASYNC_PROCESS_STDOUT);
	ASYNC_PROCESS_BUILDER_CONST("STDERR", ASYNC_PROCESS_STDERR);
}

#endif /* PHALCON_USE_UV */
