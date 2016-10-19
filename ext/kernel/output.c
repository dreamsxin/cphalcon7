
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

#include "kernel/output.h"

#include <main/php_output.h>

#include "kernel/memory.h"

void phalcon_ob_start()
{
	php_output_start_default();
}

void phalcon_ob_get_contents(zval *result)
{
	php_output_get_contents(result);
}

int phalcon_ob_end_flush()
{
	if (phalcon_ob_get_level() < 1) {
		php_error_docref("ref.outcontrol", E_NOTICE, "failed to delete and flush buffer. No buffer to flush");
		return FAILURE;
	}

	return php_output_end();
}

int phalcon_ob_end_clean()
{
	if (phalcon_ob_get_level() < 1) {
		php_error_docref("ref.outcontrol", E_NOTICE, "failed to delete buffer. No buffer to delete");
		return FAILURE;
	}

	return php_output_discard();
}

int phalcon_ob_flush()
{
	if (phalcon_ob_get_level() < 1) {
		php_error_docref("ref.outcontrol", E_NOTICE, "failed to flush buffer. No buffer to flush");
		return FAILURE;
	}

	return php_output_flush();
}

int phalcon_ob_clean()
{
	if (phalcon_ob_get_level() < 1) {
		php_error_docref("ref.outcontrol", E_NOTICE, "failed to delete buffer. No buffer to delete");
		return FAILURE;
	}

	return php_output_clean();

}

int phalcon_ob_get_clean(zval *result)
{
	if (phalcon_ob_get_level() < 1) {
		php_error_docref("ref.outcontrol", E_NOTICE, "failed to delete buffer. No buffer to delete");
		return FAILURE;
	}

	if (php_output_get_contents(result) == FAILURE) {
		php_error_docref("ref.outcontrol", E_NOTICE, "failed to delete buffer. No buffer to delete");
		return FAILURE;
	}

	return php_output_discard();

}

int phalcon_ob_get_level()
{
	return php_output_get_level();
}
