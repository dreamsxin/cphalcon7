
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

#ifndef PHALCON_CLI_OPTIONS_H
#define PHALCON_CLI_OPTIONS_H

#include "php_phalcon.h"

/* constraints */
#define PHALCON_CLI_OPTIONS_TYPE_ANY		0
#define PHALCON_CLI_OPTIONS_TYPE_INT		1
#define PHALCON_CLI_OPTIONS_TYPE_FLOAT		2
#define PHALCON_CLI_OPTIONS_TYPE_BOOLEAN	3
#define PHALCON_CLI_OPTIONS_TYPE_STRING		4
#define PHALCON_CLI_OPTIONS_TYPE_ARRAY		5


extern zend_class_entry *phalcon_cli_options_ce;

PHALCON_INIT_CLASS(Phalcon_Cli_Options);

#endif /* PHALCON_CLI_OPTIONS_H */
