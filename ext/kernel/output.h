
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

#ifndef PHALCON_KERNEL_OUTPUT_H
#define PHALCON_KERNEL_OUTPUT_H

#include "php_phalcon.h"

void phalcon_ob_start();
void phalcon_ob_get_contents(zval *result);
int phalcon_ob_end_flush();
int phalcon_ob_end_clean();
int phalcon_ob_flush();
int phalcon_ob_clean();
int phalcon_ob_get_clean(zval *result);
int phalcon_ob_get_level();

#endif /* PHALCON_KERNEL_OUTPUT_H */
