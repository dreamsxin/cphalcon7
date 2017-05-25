
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

#include "cli/color.h"

#include <main/SAPI.h>

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/debug.h"

/**
 * Phalcon\Cli\Color
 *
 *<code>
 *
 *	echo Phalcon\Cli\Color::head("head");
 *	echo Phalcon\Cli\Color::error("error");
 *	echo Phalcon\Cli\Color::success("success");
 *	echo Phalcon\Cli\Color::info("info");
 *</code>
 */
zend_class_entry *phalcon_cli_color_ce;

PHP_METHOD(Phalcon_Cli_Color, isSupportedShell);
PHP_METHOD(Phalcon_Cli_Color, colorize);
PHP_METHOD(Phalcon_Cli_Color, head);
PHP_METHOD(Phalcon_Cli_Color, error);
PHP_METHOD(Phalcon_Cli_Color, success);
PHP_METHOD(Phalcon_Cli_Color, info);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_color_colorize, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, fg, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, at, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, bg, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_color_head, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_color_error, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_color_success, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_color_info, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cli_color_method_entry[] = {
	PHP_ME(Phalcon_Cli_Color, isSupportedShell, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Cli_Color, colorize, arginfo_phalcon_cli_color_colorize, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Cli_Color, head, arginfo_phalcon_cli_color_head, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Cli_Color, error, arginfo_phalcon_cli_color_error, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Cli_Color, success, arginfo_phalcon_cli_color_success, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Cli_Color, info, arginfo_phalcon_cli_color_info, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Cli\Color initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cli_Color){

	PHALCON_REGISTER_CLASS(Phalcon\\Cli, Color, cli_color, phalcon_cli_color_method_entry, 0);

	zend_declare_property_null(phalcon_cli_color_ce, SL("_fg"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_cli_color_ce, SL("_bg"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_cli_color_ce, SL("_at"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	/* constraints */
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_BLACK"),			PHALCON_CLI_COLOR_FG_BLACK);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_DARK_GRAY"),		PHALCON_CLI_COLOR_FG_DARK_GRAY);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_RED"),			PHALCON_CLI_COLOR_FG_RED);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_LIGHT_RED"),		PHALCON_CLI_COLOR_FG_LIGHT_RED);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_GREEN"),			PHALCON_CLI_COLOR_FG_GREEN);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_LIGHT_GREEN"),	PHALCON_CLI_COLOR_FG_LIGHT_GREEN);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_BROWN"),			PHALCON_CLI_COLOR_FG_BROWN);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_YELLOW"),			PHALCON_CLI_COLOR_FG_YELLOW);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_BLUE"),			PHALCON_CLI_COLOR_FG_BLUE);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_LIGHT_BLUE"),		PHALCON_CLI_COLOR_FG_LIGHT_BLUE);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_PURPLE"),			PHALCON_CLI_COLOR_FG_PURPLE);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_LIGHT_PURPLE"),	PHALCON_CLI_COLOR_FG_LIGHT_PURPLE);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_CYAN"),			PHALCON_CLI_COLOR_FG_CYAN);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_LIGHT_CYAN"),		PHALCON_CLI_COLOR_FG_LIGHT_CYAN);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_LIGHT_GRAY"),		PHALCON_CLI_COLOR_FG_LIGHT_GRAY);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("FG_WHITE"),			PHALCON_CLI_COLOR_FG_WHITE);

	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("BG_BLACK"),			PHALCON_CLI_COLOR_BG_BLACK);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("BG_RED"),			PHALCON_CLI_COLOR_BG_RED);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("BG_GREEN"),			PHALCON_CLI_COLOR_BG_GREEN);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("BG_YELLOW"),			PHALCON_CLI_COLOR_BG_YELLOW);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("BG_BLUE"),			PHALCON_CLI_COLOR_BG_BLUE);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("BG_MAGENTA"),		PHALCON_CLI_COLOR_BG_MAGENTA);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("BG_CYAN"),			PHALCON_CLI_COLOR_BG_CYAN);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("BG_LIGHT_GRAY"),		PHALCON_CLI_COLOR_BG_LIGHT_GRAY);

	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("AT_NORMAL"),			PHALCON_CLI_COLOR_AT_NORMAL);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("AT_BOLD"),			PHALCON_CLI_COLOR_AT_BOLD);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("AT_ITALIC"),			PHALCON_CLI_COLOR_AT_ITALIC);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("AT_UNDERLINE"),		PHALCON_CLI_COLOR_AT_UNDERLINE);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("AT_BLINK"),			PHALCON_CLI_COLOR_AT_BLINK);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("AT_OUTLINE"),		PHALCON_CLI_COLOR_AT_OUTLINE);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("AT_REVERSE"),		PHALCON_CLI_COLOR_AT_REVERSE);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("AT_NONDISP"),		PHALCON_CLI_COLOR_AT_NONDISP);
	zend_declare_class_constant_long(phalcon_cli_color_ce, SL("AT_STRIKE"),			PHALCON_CLI_COLOR_AT_STRIKE);

	return SUCCESS;
}

/**
 * Identify if console supports colors
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Cli_Color, isSupportedShell){

	zval ret = {};
#ifdef PHP_WIN32
	zval arg = {};

	ZVAL_STRING(&arg, "ANSICON");

	PHALCON_CALL_FUNCTION(&ret, "getenv", &arg);
	zval_ptr_dtor(&arg);

	if (!PHALCON_IS_FALSE(&ret)) {
		RETURN_TRUE;
	}
	zval_ptr_dtor(&ret);

	ZVAL_STRING(&arg, "ConEmuANSI");

	PHALCON_CALL_FUNCTION(&ret, "getenv", &arg);
	zval_ptr_dtor(&arg);

	if (PHALCON_IS_STRING(&ret, "ON")) {
		zval_ptr_dtor(&ret);
		RETURN_TRUE;
	}
	zval_ptr_dtor(&ret);

	ZVAL_STRING(&arg, "TERM");

	PHALCON_CALL_FUNCTION(&ret, "getenv", &arg);
	zval_ptr_dtor(&arg);

	if (PHALCON_IS_STRING(&ret, "xterm")) {
		zval_ptr_dtor(&ret);
		RETURN_TRUE;
	}
	zval_ptr_dtor(&ret);
#endif

	if (phalcon_has_constant(SL("STDOUT"))) {
		if (phalcon_function_exists_ex(SL("posix_isatty")) == SUCCESS) {
			zval arg = {};
			ZVAL_STRING(&arg, "STDOUT");
			PHALCON_CALL_FUNCTION(&ret, "posix_isatty", &arg);
			if (zend_is_true(&ret)) {
				zval_ptr_dtor(&arg);
				RETURN_TRUE;
			}
			zval_ptr_dtor(&arg);
		}
	}

	RETURN_FALSE;
}

static char* fg_map[] = {"0;30","1;30","0;31","1;31","0;32","1;32","0;33","1;33","0;34","1;34","0;35","1;35","0;36","1;36","0;37","1;37"};
static char* at_map[] = {"0","1","3","4","5","6","7","8","9"};
static char* bg_map[] = {"40","41","42","43","44","45","46","47"};

/**
 * Colorizes the string using provided colors.
 *
 * @param string $str
 * @param null|integer $fg
 * @param null|integer $at
 * @param null|integer $bg
 * @return string
 */
PHP_METHOD(Phalcon_Cli_Color, colorize){

	zval *str, *fg = NULL, *at = NULL, *bg = NULL;
	zval ret = {}, colored = {};
	int i;

	phalcon_fetch_params(0, 1, 3, &str, &fg, &at, &bg);

	PHALCON_CALL_STATIC(&ret, "issupportedshell");

	if (!zend_is_true(&ret)) {
		RETURN_CTOR(str);
	}

	if (fg && Z_TYPE_P(fg) == IS_LONG) {
		i = Z_LVAL_P(fg);
		if (i >= PHALCON_CLI_COLOR_FG_BLACK && i <= PHALCON_CLI_COLOR_FG_WHITE) {
			phalcon_concat_sss(&colored, SL("\033["), fg_map[i], strlen(fg_map[i]), "m", 1, 1);
		}
	}

	if (bg && Z_TYPE_P(bg) == IS_LONG) {
		i = Z_LVAL_P(bg);
		if (i >= PHALCON_CLI_COLOR_BG_BLACK && i <= PHALCON_CLI_COLOR_BG_LIGHT_GRAY) {
			phalcon_concat_sss(&colored, SL("\033["), bg_map[i], strlen(bg_map[i]), "m", 1, 1);
		}
	}

	if (at && Z_TYPE_P(at) == IS_LONG) {
		i = Z_LVAL_P(at);
		if (i >= PHALCON_CLI_COLOR_AT_NORMAL && i <= PHALCON_CLI_COLOR_AT_STRIKE) {
			phalcon_concat_sss(&colored, SL("\033["), at_map[i], strlen(at_map[i]), "m", 1, 1);
		}
	}

	PHALCON_SCONCAT_VS(&colored, str, "\033[0m");

    RETURN_ZVAL(&colored, 0, 0);
}

/**
 * Color style for head messages.
 *
 * @param string $str
 * @return string
 */
PHP_METHOD(Phalcon_Cli_Color, head){

	zval *str, fg = {};

	phalcon_fetch_params(0, 1, 0, &str);

	ZVAL_LONG(&fg, PHALCON_CLI_COLOR_FG_BROWN);

	PHALCON_CALL_STATIC(return_value, "colorize", str, &fg);
	PHALCON_SCONCAT_STR(return_value, PHP_EOL);
}

/**
 * Color style for error messages.
 *
 * @param string $str
 * @return string
 */
PHP_METHOD(Phalcon_Cli_Color, error){

	zval *str, fg = {}, at= {}, bg= {}, msg = {}, tmp = {};
	zval input = {}, out = {}, line1 = {}, line2 = {};
	int space;

	phalcon_fetch_params(0, 1, 0, &str);

	ZVAL_LONG(&fg, PHALCON_CLI_COLOR_FG_WHITE);
	ZVAL_LONG(&at, PHALCON_CLI_COLOR_AT_BOLD);
	ZVAL_LONG(&bg, PHALCON_CLI_COLOR_BG_RED);

	PHALCON_CONCAT_SV(&tmp, "Error: ", str);

	space = Z_STRLEN(tmp) + 4;

	PHALCON_CONCAT_SVS(&msg, "  ", &tmp, "  ");
	zval_ptr_dtor(&tmp);

	ZVAL_STRING(&input, " ");
	phalcon_pad_str(&out, &input, space, " ", PHALCON_PDA_RIGHT);
	zval_ptr_dtor(&input);

	PHALCON_CALL_STATIC(&line1, "colorize", &out, &fg, &at, &bg);
	PHALCON_CALL_STATIC(&line2, "colorize", &msg, &fg, &at, &bg);
	zval_ptr_dtor(&out);
	zval_ptr_dtor(&msg);

	PHALCON_CONCAT_VSVSVS(return_value, &line1, PHP_EOL, &line2, PHP_EOL, &line1, PHP_EOL);
	zval_ptr_dtor(&line1);
	zval_ptr_dtor(&line2);
}

/**
 * Color style for success messages.
 *
 * @param string $str
 * @return string
 */
PHP_METHOD(Phalcon_Cli_Color, success){

	zval *str, fg = {}, at= {}, bg= {}, msg = {}, tmp = {};
	zval input = {}, out = {}, line1 = {}, line2 = {};
	int space;

	phalcon_fetch_params(0, 1, 0, &str);

	ZVAL_LONG(&fg, PHALCON_CLI_COLOR_FG_WHITE);
	ZVAL_LONG(&at, PHALCON_CLI_COLOR_AT_BOLD);
	ZVAL_LONG(&bg, PHALCON_CLI_COLOR_BG_GREEN);

	PHALCON_CONCAT_SV(&tmp, "Success: ", str);

	space = Z_STRLEN(tmp) + 4;

	PHALCON_CONCAT_SVS(&msg, "  ", &tmp, "  ");
	zval_ptr_dtor(&tmp);

	ZVAL_STRING(&input, " ");
	phalcon_pad_str(&out, &input, space, " ", PHALCON_PDA_RIGHT);
	zval_ptr_dtor(&input);

	PHALCON_CALL_STATIC(&line1, "colorize", &out, &fg, &at, &bg);
	PHALCON_CALL_STATIC(&line2, "colorize", &msg, &fg, &at, &bg);
	zval_ptr_dtor(&out);
	zval_ptr_dtor(&msg);

	PHALCON_CONCAT_VSVSVS(return_value, &line1, PHP_EOL, &line2, PHP_EOL, &line1, PHP_EOL);
	zval_ptr_dtor(&line1);
	zval_ptr_dtor(&line2);
}

/**
 * Color style for info messages.
 *
 * @param string $str
 * @return string
 */
PHP_METHOD(Phalcon_Cli_Color, info){

	zval *str, fg = {}, at= {}, bg= {}, msg = {}, tmp = {};
	zval input = {}, out = {}, line1 = {}, line2 = {};
	int space;

	phalcon_fetch_params(0, 1, 0, &str);

	ZVAL_LONG(&fg, PHALCON_CLI_COLOR_FG_WHITE);
	ZVAL_LONG(&at, PHALCON_CLI_COLOR_AT_BOLD);
	ZVAL_LONG(&bg, PHALCON_CLI_COLOR_BG_BLUE);

	PHALCON_CONCAT_SV(&tmp, "Info: ", str);

	space = Z_STRLEN(tmp) + 4;

	PHALCON_CONCAT_SVS(&msg, "  ", &tmp, "  ");
	zval_ptr_dtor(&tmp);

	ZVAL_STRING(&input, " ");
	phalcon_pad_str(&out, &input, space, " ", PHALCON_PDA_RIGHT);
	zval_ptr_dtor(&input);

	PHALCON_CALL_STATIC(&line1, "colorize", &out, &fg, &at, &bg);
	PHALCON_CALL_STATIC(&line2, "colorize", &msg, &fg, &at, &bg);
	zval_ptr_dtor(&out);
	zval_ptr_dtor(&msg);

	PHALCON_CONCAT_VSVSVS(return_value, &line1, PHP_EOL, &line2, PHP_EOL, &line1, PHP_EOL);
	zval_ptr_dtor(&line1);
	zval_ptr_dtor(&line2);
}
