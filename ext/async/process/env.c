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

#include "async/core.h"
#include "async/async_process.h"

#ifdef ZEND_WIN32

char **async_process_create_env(HashTable *add, int inherit)
{
	char **env;
	zend_string *key;
	zval *v;

	LPTCH envstr;
	char t;

	size_t len;
	int count;
	int i;
	char *j;

	count = 0;

	if (inherit) {
		envstr = GetEnvironmentStrings();

		for (i = 0;; i++) {
			if (envstr[i] == '\0') {
				if (t == '\0') {
					break;
				}

				count++;
			}

			t = envstr[i];
		}

		count -= 3;
	}

	env = ecalloc(zend_hash_num_elements(add) + count + 1, sizeof(char *));
	i = 0;

	if (inherit) {
		j = envstr;

		while (*j != '\0') {
			len = strlen(j) + 1;

			if (*j != '=') {
				env[i] = emalloc(sizeof(char) * len);
				memcpy(env[i++], j, len);
			}

			j += len;
		}

		FreeEnvironmentStrings(envstr);
	}

	ZEND_HASH_FOREACH_STR_KEY_VAL(add, key, v) {
		env[i] = emalloc(sizeof(char) * key->len + Z_STRLEN_P(v) + 2);

		slprintf(env[i++], key->len + Z_STRLEN_P(v) + 2, "%s=%s", key->val, Z_STRVAL_P(v));
	} ZEND_HASH_FOREACH_END();

	env[i] = NULL;

	return env;
}

int async_process_setup_shell(async_process_builder *builder, int interactive)
{
	wchar_t dummybuf;
	wchar_t *key;
	wchar_t *val;
	
	char *str;
	DWORD size;

	key = php_win32_cp_conv_any_to_w("COMSPEC", sizeof("COMSPEC")-1, PHP_WIN32_CP_IGNORE_LEN_P);
	
	if (!key) {
		return FAILURE;
	}
	
	SetLastError(0);
	
	size = GetEnvironmentVariableW(key, &dummybuf, 0);

	if (GetLastError() == ERROR_ENVVAR_NOT_FOUND || size == 0) {
		ASYNC_STR(builder->command, "cmd.exe");
	} else {
		val = emalloc((size + 1) * sizeof(wchar_t));
		size = GetEnvironmentVariableW(key, val, size);
		
		if (size == 0) {
			ASYNC_STR(builder->command, "cmd.exe");
		} else {
			str = php_win32_cp_w_to_any(val);
			builder->command = zend_string_init(str, size, 0);
			
			free(str);
		}
		
		efree(val);
	}
	
	free(key);
	
	if (interactive) {
		builder->argc = 1;
		builder->argv = ecalloc(builder->argc, sizeof(zval));
		
		ZVAL_STRING(&builder->argv[0], "/q");
	} else {
		builder->argc = 3;
		builder->argv = ecalloc(builder->argc, sizeof(zval));
		
		ZVAL_STRING(&builder->argv[0], "/d");
		ZVAL_STRING(&builder->argv[1], "/s");
		ZVAL_STRING(&builder->argv[2], "/c");
	}
	
	return SUCCESS;
}

#else

char **async_process_create_env(HashTable *add, int inherit)
{
	char **env;
	zend_string *key;
	zval *v;

	size_t len;
	int count;
	int i;

	i = 0;
	count = 0;

	if (inherit) {
		char **tmp;
		
		for (tmp = environ; tmp != NULL && *tmp != NULL; tmp++) {
			count++;
		}
	}

	env = ecalloc(zend_hash_num_elements(add) + count + 1, sizeof(char *));

	if (inherit) {
		for (; i < count; i++) {
			len = strlen(environ[i]) + 1;

			env[i] = emalloc(len);
			memcpy(env[i], environ[i], len);
		}
	}

	ZEND_HASH_FOREACH_STR_KEY_VAL(add, key, v) {
		env[i] = emalloc(sizeof(char) * key->len + Z_STRLEN_P(v) + 2);

		slprintf(env[i++], key->len + Z_STRLEN_P(v) + 2, "%s=%s", key->val, Z_STRVAL_P(v));
	} ZEND_HASH_FOREACH_END();

	env[i] = NULL;

	return env;
}

int async_process_setup_shell(async_process_builder *builder, int interactive)
{
	ASYNC_STR(builder->command, "/bin/sh");
	
	if (interactive) {
		return SUCCESS;
	}
	
	builder->argc = 1;
	builder->argv = ecalloc(builder->argc, sizeof(zval));
	
	ZVAL_STRING(&builder->argv[0], "-c");
	
	return SUCCESS;
}

#endif
