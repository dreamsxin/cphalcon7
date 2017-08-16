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
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  +------------------------------------------------------------------------+
*/

#include "kernel/session.h"

#ifdef PHALCON_USE_PHP_SESSION
#include <ext/session/php_session.h>
#endif

#include "kernel/main.h"
#include "kernel/fcall.h"

int phalcon_session_start()
{
	int ret = SUCCESS;
#ifdef PHALCON_USE_PHP_SESSION
# if PHP_SESSION_API >= 20161017
	ret = php_session_start();
# else
    php_session_start();
# endif
#else
	ret = phalcon_call_function_with_params(NULL, SL("session_start"), 0, NULL);
#endif
	return ret;
}

int phalcon_session_regenerate_id(zend_bool delete_old_session)
{
	zval *tmp = delete_old_session ? &PHALCON_GLOBAL(z_true) : &PHALCON_GLOBAL(z_false);
	zval *params[] = { tmp };

	return phalcon_call_function_with_params(NULL, SL("session_regenerate_id"), 1, params);
}

int phalcon_session_destroy()
{
	return phalcon_call_function_with_params(NULL, SL("session_destroy"), 0, NULL);
}

int phalcon_get_session_id(zval *retval)
{
#ifdef PHALCON_USE_PHP_SESSION
	if (PS(id)) {
		/* keep compatibility for "\0" characters ???
		 * see: ext/session/tests/session_id_error3.phpt */
		size_t len = strlen(ZSTR_VAL(PS(id)));
		if (UNEXPECTED(len != ZSTR_LEN(PS(id)))) {
			ZVAL_NEW_STR(retval, zend_string_init(ZSTR_VAL(PS(id)), len, 0));
		} else {
			ZVAL_STR_COPY(retval, PS(id));
		}
	} else {
		ZVAL_EMPTY_STRING(retval);
	}
	return SUCCESS;
#else
	return phalcon_call_function_with_params(retval, SL("session_id"), 0, NULL);
#endif
}

int phalcon_set_session_id(zval *sid)
{
#ifdef PHALCON_USE_PHP_SESSION
	if (PS(id)) {
		zend_string_release(PS(id));
	}
	PS(id) = zend_string_copy(Z_STR_P(sid));
	return SUCCESS;
#else
	zval *params[] = { sid };
	return phalcon_call_function_with_params(NULL, SL("session_id"), 1, params);
#endif
}

int phalcon_session_write_close()
{
//#ifdef PHALCON_USE_PHP_SESSION
//	if (PS(session_status) != php_session_active) {
//		return FAILURE;
//	}
//	php_session_flush(1);
//	return SUCCESS;
//#else
	return phalcon_call_function_with_params(NULL, SL("session_write_close"), 0, NULL);
//#endif
}

int phalcon_session_set(zval *name, zval *val)
{
#ifdef PHALCON_USE_PHP_SESSION
	if (php_set_session_var(Z_STR_P(name), val, NULL)) {
		if (Z_REFCOUNTED_P(val)) Z_ADDREF_P(val);
		return SUCCESS;
	}
	return FAILURE;
#else
	return FAILURE;
#endif
}

zval* phalcon_session_get(zval *name)
{
#ifdef PHALCON_USE_PHP_SESSION
	return php_get_session_var(Z_STR_P(name));
#endif
	return NULL;
}
