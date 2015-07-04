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
	return phalcon_call_func_aparams(NULL, SL("session_start"), 0, NULL);
}

int phalcon_session_destroy()
{
	return phalcon_call_func_aparams(NULL, SL("session_destroy"), 0, NULL);
}

int phalcon_get_session_id(zval *return_value)
{
	return phalcon_call_func_aparams(&return_value, SL("session_id"), 0, NULL);
}

int phalcon_set_session_id(zval *sid)
{
	zval *params[] = { sid };
	return phalcon_call_func_aparams(NULL, SL("session_id"), 1, params);
}

int phalcon_session_write_close()
{
	return phalcon_call_func_aparams(NULL, SL("session_write_close"), 0, NULL);
}
