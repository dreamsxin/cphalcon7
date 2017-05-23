
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

#ifndef PHALCON_CLI_COLOR_H
#define PHALCON_CLI_COLOR_H

#include "php_phalcon.h"

enum phalcon_cli_color_fgs {
	PHALCON_CLI_COLOR_FG_BLACK,
	PHALCON_CLI_COLOR_FG_DARK_GRAY,
	PHALCON_CLI_COLOR_FG_RED,
	PHALCON_CLI_COLOR_FG_LIGHT_RED,
	PHALCON_CLI_COLOR_FG_GREEN,
	PHALCON_CLI_COLOR_FG_LIGHT_GREEN,
	PHALCON_CLI_COLOR_FG_BROWN,
	PHALCON_CLI_COLOR_FG_YELLOW,
	PHALCON_CLI_COLOR_FG_BLUE,
	PHALCON_CLI_COLOR_FG_LIGHT_BLUE,
	PHALCON_CLI_COLOR_FG_PURPLE,
	PHALCON_CLI_COLOR_FG_LIGHT_PURPLE,
	PHALCON_CLI_COLOR_FG_CYAN,
	PHALCON_CLI_COLOR_FG_LIGHT_CYAN,
	PHALCON_CLI_COLOR_FG_LIGHT_GRAY,
	PHALCON_CLI_COLOR_FG_WHITE
};

enum phalcon_cli_color_bgs {
	PHALCON_CLI_COLOR_BG_BLACK,
	PHALCON_CLI_COLOR_BG_RED,
	PHALCON_CLI_COLOR_BG_GREEN,
	PHALCON_CLI_COLOR_BG_YELLOW,
	PHALCON_CLI_COLOR_BG_BLUE,
	PHALCON_CLI_COLOR_BG_MAGENTA,
	PHALCON_CLI_COLOR_BG_CYAN,
	PHALCON_CLI_COLOR_BG_LIGHT_GRAY
};

enum phalcon_cli_color_ats {
	PHALCON_CLI_COLOR_AT_NORMAL,
	PHALCON_CLI_COLOR_AT_BOLD,
	PHALCON_CLI_COLOR_AT_ITALIC,
	PHALCON_CLI_COLOR_AT_UNDERLINE,
	PHALCON_CLI_COLOR_AT_BLINK,
	PHALCON_CLI_COLOR_AT_OUTLINE,
	PHALCON_CLI_COLOR_AT_REVERSE,
	PHALCON_CLI_COLOR_AT_NONDISP,
	PHALCON_CLI_COLOR_AT_STRIKE
};


extern zend_class_entry *phalcon_cli_color_ce;

PHALCON_INIT_CLASS(Phalcon_Cli_Color);

#endif /* PHALCON_CLI_COLOR_H */
