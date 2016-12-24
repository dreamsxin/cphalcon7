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

#ifndef PHALCON_KERNEL_SESSION_H
#define PHALCON_KERNEL_SESSION_H

#include "php_phalcon.h"

#define IF_SESSION_VARS() \
	if (Z_ISREF_P(&PS(http_session_vars)) && Z_TYPE_P(Z_REFVAL(PS(http_session_vars))) == IS_ARRAY)

#define SESSION_CHECK_ACTIVE_STATE	\
	if (PS(session_status) == php_session_active) {	\
		php_error_docref(NULL, E_WARNING, "A session is active. You cannot change the session module's ini settings at this time");	\
		return FAILURE;	\
	}

int phalcon_session_start() PHALCON_ATTR_WARN_UNUSED_RESULT;
int phalcon_session_destroy() PHALCON_ATTR_WARN_UNUSED_RESULT;
int phalcon_get_session_id(zval *return_value) PHALCON_ATTR_WARN_UNUSED_RESULT;
int phalcon_set_session_id(zval *sid) PHALCON_ATTR_WARN_UNUSED_RESULT;
int phalcon_session_write_close() PHALCON_ATTR_WARN_UNUSED_RESULT;
int phalcon_session_set(zval *name, zval *val);
zval* phalcon_session_get(zval *name);

#endif /* PHALCON_KERNEL_SESSION_H */
