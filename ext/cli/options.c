
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

#ifndef _WIN32
#include <sys/ioctl.h>
#include <termios.h>
#else
#include <windows.h>
#endif

#include "cli/options.h"
#include "cli/options/exception.h"
#include "cli/color.h"
#include "di/injectable.h"

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
 * Phalcon\Cli\Options
 *
 *<code>
 *
 * $ops = new \Phalcon\Cli\Options('Phalcon CLI');
 *  $ops->add([
 * 	'type' => \Phalcon\Cli\Options::TYPE_INT,
 * 	'name' => 'min'
 * ]);
 *  $ops->add([
 * 	'type' => \Phalcon\Cli\Options::TYPE_INT,
 * 	'name' => 'max',
 * 	'shortName' => 'm',
 * 	'required' => false,
 * 	'desc' => "int",
 * 	'help' => "must be int",
 * 	'defaultValue' => 1
 * ]);
 * $ops->add(\Phalcon\Cli\Options::TYPE_STRING, 'name', 'n', true, "name", "must be string", "Phalcon");
 * $values = $ops->parse();
 *
 *</code>
 */
zend_class_entry *phalcon_cli_options_ce;

PHP_METHOD(Phalcon_Cli_Options, __construct);
PHP_METHOD(Phalcon_Cli_Options, add);
PHP_METHOD(Phalcon_Cli_Options, help);
PHP_METHOD(Phalcon_Cli_Options, parse);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_options___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, title, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, program, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, argString, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, desc, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
	ZEND_ARG_INFO(0, dependencyInjector)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_options_add, 0, 0, 1)
	ZEND_ARG_INFO(0, arg)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, shortname, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, required, _IS_BOOL, 1)
	ZEND_ARG_TYPE_INFO(0, desc, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, help, IS_STRING, 1)
	ZEND_ARG_INFO(0, defaultValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_options_parse, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cli_options_method_entry[] = {
	PHP_ME(Phalcon_Cli_Options, __construct, arginfo_phalcon_cli_options___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cli_Options, add, arginfo_phalcon_cli_options_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cli_Options, help, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cli_Options, parse, arginfo_phalcon_cli_options_parse, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Cli_Options, addOption, add, arginfo_phalcon_cli_options_add, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cli\Options initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cli_Options){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cli, Options, cli_options, phalcon_di_injectable_ce, phalcon_cli_options_method_entry, 0);

	zend_declare_property_string(phalcon_cli_options_ce, SL("_title"), "Usage", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_program"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_cli_options_ce, SL("_argString"), "[OPTIONS...]", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_desc"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_types"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_longopts"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_descs"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_helps"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_required"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_defaultValues"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_names"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_options_ce, SL("_shortNames"), ZEND_ACC_PROTECTED);

	/* constraints */
	zend_declare_class_constant_long(phalcon_cli_options_ce, SL("TYPE_ANY"),		PHALCON_CLI_OPTIONS_TYPE_ANY);
	zend_declare_class_constant_long(phalcon_cli_options_ce, SL("TYPE_INT"),		PHALCON_CLI_OPTIONS_TYPE_INT);
	zend_declare_class_constant_long(phalcon_cli_options_ce, SL("TYPE_FLOAT"),		PHALCON_CLI_OPTIONS_TYPE_FLOAT);
	zend_declare_class_constant_long(phalcon_cli_options_ce, SL("TYPE_BOOLEAN"),	PHALCON_CLI_OPTIONS_TYPE_BOOLEAN);
	zend_declare_class_constant_long(phalcon_cli_options_ce, SL("TYPE_STRING"),		PHALCON_CLI_OPTIONS_TYPE_STRING);
	zend_declare_class_constant_long(phalcon_cli_options_ce, SL("TYPE_ARRAY"),		PHALCON_CLI_OPTIONS_TYPE_ARRAY);

	return SUCCESS;
}

/**
 * Phalcon\Cli\Options constructor
 */
PHP_METHOD(Phalcon_Cli_Options, __construct){

	zval *title = NULL, *program = NULL, *arg_string = NULL, *desc = NULL, *options = NULL, *dependency_injector = NULL;
	zval name = {}, short_name = {};

	phalcon_fetch_params(0, 1, 5, &title, &program, &arg_string, &desc, &options, &dependency_injector);

	if (title && Z_TYPE_P(title) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_title"), title);
	}

	if (arg_string && Z_TYPE_P(arg_string) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_argString"), arg_string);
	}

	if (program && Z_TYPE_P(program) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_program"), program);
	}

	if (desc && Z_TYPE_P(desc) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_desc"), desc);
	}

	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHOD(NULL, getThis(), "setdi", dependency_injector);
	}

	ZVAL_STRING(&name, "help");
	ZVAL_STRING(&short_name, "h");

	phalcon_update_property_array_append(getThis(), SL("_longopts"), &name);
	phalcon_update_property_array_append(getThis(), SL("_options"), &short_name);

	phalcon_update_property_array(getThis(), SL("_names"), &short_name, &name);
	phalcon_update_property_array(getThis(), SL("_shortNames"), &name, &short_name);
	zval_ptr_dtor(&name);
	zval_ptr_dtor(&short_name);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		zval *option;
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(options), option) {
			PHALCON_CALL_METHOD(NULL, getThis(), "add", option);
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * Add option
 */
PHP_METHOD(Phalcon_Cli_Options, add){

	zval *arg, *_name = NULL, *_short_name = NULL, *_required = NULL, *_desc = NULL, *_help = NULL, *_default_value = NULL;
	zval type = {}, names = {}, short_names = {}, name = {}, short_name = {}, required = {}, desc = {}, help = {}, default_value = {};
	zval key = {};
	int t = 0;

	phalcon_fetch_params(0, 1, 6, &arg, &_name, &_short_name, &_required, &_desc, &_help, &_default_value);

	if (Z_TYPE_P(arg) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_str(&type, arg, SL("type"), PH_READONLY)) {
			ZVAL_NULL(&type);
		}
		if (!phalcon_array_isset_fetch_str(&name, arg, SL("name"), PH_READONLY)) {
			ZVAL_NULL(&name);
		}
		if (!phalcon_array_isset_fetch_str(&short_name, arg, SL("shortName"), PH_READONLY)) {
			ZVAL_NULL(&short_name);
		}
		if (!phalcon_array_isset_fetch_str(&required, arg, SL("required"), PH_READONLY)) {
			ZVAL_FALSE(&required);
		}
		if (!phalcon_array_isset_fetch_str(&desc, arg, SL("desc"), PH_READONLY)) {
			ZVAL_NULL(&desc);
		}
		if (!phalcon_array_isset_fetch_str(&help, arg, SL("help"), PH_READONLY)) {
			ZVAL_NULL(&help);
		}
		if (!phalcon_array_isset_fetch_str(&default_value, arg, SL("defaultValue"), PH_READONLY)) {
			ZVAL_NULL(&default_value);
		}
	} else if (Z_TYPE_P(arg) == IS_LONG) {
		ZVAL_COPY_VALUE(&type, arg);
		
		if (_name) {
			ZVAL_COPY_VALUE(&name, _name);
		}

		if (_short_name) {
			ZVAL_COPY_VALUE(&short_name, _short_name);
		}

		if (_required) {
			ZVAL_COPY_VALUE(&required, _required);
		}

		if (_desc) {
			ZVAL_COPY_VALUE(&desc, _desc);
		}

		if (_help) {
			ZVAL_COPY_VALUE(&help, _help);
		}

		if (_default_value) {
			ZVAL_COPY_VALUE(&default_value, _default_value);
		}
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "type must be int");
		return;
	}
		
	if (Z_TYPE(name) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "name must be string");
		return;
	}
	
	if (Z_STRLEN(name) > 32) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "name not exceed 32 characters long");
		return;
	}

	phalcon_update_property_array(getThis(), SL("_types"), &name, &type);

	t = Z_LVAL(type);

	if (t < PHALCON_CLI_OPTIONS_TYPE_INT || t > PHALCON_CLI_OPTIONS_TYPE_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "type is not valid");
		return;
	}

	switch (t)
	{
		case PHALCON_CLI_OPTIONS_TYPE_ANY:
			PHALCON_CONCAT_VS(&key, &name, ":");
			break;
		case PHALCON_CLI_OPTIONS_TYPE_BOOLEAN:
			ZVAL_COPY(&key, &name);
			break;
		default:
			PHALCON_CONCAT_VS(&key, &name, "::");
			break;
	}

	phalcon_update_property_array_append(getThis(), SL("_longopts"), &key);
	zval_ptr_dtor(&key);

	if (zend_is_true(&required)) {
		phalcon_update_property_array_append(getThis(), SL("_required"), &name);
	}
	
	phalcon_read_property(&names, getThis(), SL("_names"), PH_READONLY);
	if (Z_TYPE(short_name) == IS_STRING) {
		if (Z_STRLEN(short_name) != 1) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "short name not exceed 1 characters long");
			return;
		}
		if (phalcon_array_isset(&names, &short_name)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "duplicate short name exists");
			return;
		}
		phalcon_read_property(&short_names, getThis(), SL("_shortNames"), PH_READONLY);
		if (phalcon_array_isset(&short_names, &name)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "duplicate name exists");
			return;
		}

		phalcon_update_property_array(getThis(), SL("_names"), &short_name, &name);
		phalcon_update_property_array(getThis(), SL("_shortNames"), &name, &short_name);

		if (t == PHALCON_CLI_OPTIONS_TYPE_BOOLEAN) {
			ZVAL_COPY(&key, &short_name);
		} else if (zend_is_true(&required)) {
			PHALCON_CONCAT_VS(&key, &short_name, ":");
		} else {
			PHALCON_CONCAT_VS(&key, &short_name, "::");
		}
		phalcon_update_property_array_append(getThis(), SL("_options"), &key);
		zval_ptr_dtor(&key);
	} else {
		if (phalcon_array_isset(&names, &name)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "duplicate name exists");
			return;
		}
		phalcon_update_property_array(getThis(), SL("_names"), &name, &name);
	}

	if (Z_TYPE(desc) == IS_STRING) {
		if (Z_STRLEN(desc) > 120) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "desc must not exceed 120 characters long");
			return;
		}
		phalcon_update_property_array(getThis(), SL("_descs"), &name, &desc);
	}

	if (Z_TYPE(help) == IS_STRING) {
		if (Z_STRLEN(help) > 256) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "help must not exceed 256 characters long");
			return;
		}
		phalcon_update_property_array(getThis(), SL("_helps"), &name, &help);
	}

	if (Z_TYPE(default_value) != IS_NULL) {
		switch(t) {
			case PHALCON_CLI_OPTIONS_TYPE_INT:
				if (Z_TYPE(default_value) != IS_LONG) {
					PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "default value type must be int");
					return;
				}
				break;

			case PHALCON_CLI_OPTIONS_TYPE_FLOAT:
				if (Z_TYPE(default_value) != IS_DOUBLE) {
					PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "default value type must be float");
					return;
				}
				break;
			case PHALCON_CLI_OPTIONS_TYPE_BOOLEAN:
				if (!PHALCON_IS_BOOL(&default_value)) {
					PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "default value type must be boolean");
					return;
				}
				break;
			case PHALCON_CLI_OPTIONS_TYPE_STRING:
				if (Z_TYPE(default_value) != IS_STRING) {
					PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "default value type must be string");
					return;
				}
				break;
			case PHALCON_CLI_OPTIONS_TYPE_ARRAY:
				if (Z_TYPE(default_value) != IS_ARRAY) {
					PHALCON_THROW_EXCEPTION_STR(phalcon_cli_options_exception_ce, "default value type must be array");
					return;
				}
				break;
		}
		phalcon_update_property_array(getThis(), SL("_defaultValues"), &name, &default_value);
	}
}

static int get_terminal_width(void)
{
#ifndef _WIN32
    struct winsize max;
    if (ioctl(0, TIOCGWINSZ, &max) != -1) {
        return max.ws_col;
    } else {
        return 80;
    }
#else
    CONSOLE_SCREEN_BUFFER_INFO cbsi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi);
    return cbsi.srWindow.Right - cbsi.srWindow.Left;
#endif
}

/**
 * Print help
 */
PHP_METHOD(Phalcon_Cli_Options, help){

	zval title = {}, program = {}, arg_string = {}, desc = {}, names = {}, descs = {}, helps = {};
	zval default_values = {}, *name;
	zend_string *str_key;
	int line_max;

	phalcon_read_property(&title, getThis(), SL("_title"), PH_READONLY);
	phalcon_read_property(&program, getThis(), SL("_program"), PH_READONLY);
	phalcon_read_property(&arg_string, getThis(), SL("_argString"), PH_READONLY);
	phalcon_read_property(&desc, getThis(), SL("_desc"), PH_READONLY);

	phalcon_read_property(&names, getThis(), SL("_names"), PH_READONLY);
	phalcon_read_property(&descs, getThis(), SL("_descs"), PH_READONLY);
	phalcon_read_property(&helps, getThis(), SL("_helps"), PH_READONLY);
	phalcon_read_property(&default_values, getThis(), SL("_defaultValues"), PH_READONLY);

    fprintf(stderr, "%s:\n", Z_STRVAL(title));
    if (PHALCON_IS_NOT_EMPTY(&program)) {
		fprintf(stderr, "  %s %s\n\n", Z_STRVAL(program), Z_STRVAL(arg_string));
	}
    if (PHALCON_IS_NOT_EMPTY(&desc)) {
        fprintf(stderr, "%s", Z_STRVAL(desc));
        fprintf(stderr, "\n");
    }

	line_max = get_terminal_width() - 3;

#define _advance_margin(offset) \
    while(buf - helpbuf < offset || *buf) { \
        if (!*buf) { \
            *buf = ' '; \
        } \
        buf++; \
    }

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(names), str_key, name) {
		zval item_desc = {}, item_help = {}, value = {};
		char helpbuf[1024] = { 0 };
		char *buf = helpbuf;
		if (str_key->len == 1) {
			buf += sprintf(buf, " -%c ", str_key->val[0]);
			//_advance_margin(4)
		}

		buf += sprintf(buf, " --%s ", Z_STRVAL_P(name));
		_advance_margin(12);

		if (phalcon_array_isset_fetch(&item_desc, &descs, name, PH_READONLY)) {
			buf += sprintf(buf, " <%s> ", Z_STRVAL(item_desc));
		}
		_advance_margin(35);

		if (phalcon_array_isset_fetch(&item_help, &helps, name, PH_READONLY)) {
			unsigned initial_indent = buf - helpbuf + 1;
			int curpos = initial_indent;
			const char *help_p = Z_STRVAL(item_help);

			for (; *help_p; help_p++, curpos++, buf++) {

				if (curpos >= line_max) {
					unsigned ii;
					if (!isspace(*help_p) && !isspace(*(help_p-1))) {
						*buf = '-';
						buf++;
					}
					*buf = '\n';
					buf++;

					for (ii = 0; ii < initial_indent+1; ii++, buf++) {
						*buf = ' ';
					}

					curpos = initial_indent;
					if (isspace(*help_p)) {
						buf--;
						continue;
					}
				}
				*buf = *help_p;
			}
		}
		*buf = '\0';
		fprintf(stderr, "  %s", helpbuf);

		if (phalcon_array_isset_fetch(&value, &default_values, name, PH_READONLY) && Z_TYPE(value) != IS_NULL) {
            fprintf(stderr, " [Default=");
			switch (Z_TYPE(value)) {
				case IS_STRING:
					fprintf(stderr, "'%s'", Z_STRVAL(value));
					break;
				case IS_ARRAY: {
					break;
				}
				case IS_DOUBLE:
					fprintf(stderr, "%0.2f", Z_DVAL(value));
					break;
				case IS_LONG:
					fprintf(stderr, "%ld", Z_LVAL(value));
					break;
				case IS_TRUE:
				case IS_FALSE:
					fprintf(stderr, "%s", zend_is_true(&value) ? "TRUE" : "FALSE");
					break;
				default:
					fprintf(stderr, "Unknown option type '%d'", Z_TYPE(value));
					break;
            }
            fprintf(stderr, "]");
		}

        fprintf(stderr, "\n");
	} ZEND_HASH_FOREACH_END();
#undef _advance_margin
}

/**
 * Parse and return values
 */
PHP_METHOD(Phalcon_Cli_Options, parse){

	zval *_options = NULL, names = {}, short_names ={}, options = {}, longopts = {}, joined_opts = {}, values = {};
	zval default_values = {}, types = {}, required = {}, *name;
	zend_string *str_key;

	phalcon_fetch_params(0, 0, 1, &_options);

	phalcon_read_property(&names, getThis(), SL("_names"), PH_READONLY);
	phalcon_read_property(&short_names, getThis(), SL("_shortNames"), PH_READONLY);
	phalcon_read_property(&options, getThis(), SL("_options"), PH_READONLY);
	phalcon_read_property(&longopts, getThis(), SL("_longopts"), PH_READONLY);

	phalcon_fast_join_str(&joined_opts, SL(""), &options);

	PHALCON_CALL_FUNCTION(&values, "getopt", &joined_opts, &longopts);
	zval_ptr_dtor(&joined_opts);

	if (phalcon_array_isset_str(&values, SL("help")) || phalcon_array_isset_str(&values, SL("h"))) {
		zval_ptr_dtor(&values);
		PHALCON_CALL_METHOD(NULL, getThis(), "help");
		RETURN_FALSE;
	}

	phalcon_read_property(&default_values, getThis(), SL("_defaultValues"), PH_READONLY);
	phalcon_read_property(&types, getThis(), SL("_types"), PH_READONLY);
	phalcon_read_property(&required, getThis(), SL("_required"), PH_READONLY);

	if (Z_TYPE(required) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(required), name) {
			if (!phalcon_array_isset(&values, name)) {
				zval short_name = {};
				if (!phalcon_array_isset_fetch(&short_name, &short_names, name, PH_READONLY)
					|| !phalcon_array_isset(&values, &short_name)) {
					zval msg = {}, out = {};
					zval_ptr_dtor(&values);
					PHALCON_CONCAT_VS(&msg, name, " is required");
					PHALCON_CALL_CE_STATIC(&out, phalcon_cli_color_ce, "error", &msg);
					zend_print_zval(&out, 0);
					zval_ptr_dtor(&msg);
					zval_ptr_dtor(&out);
					PHALCON_CALL_METHOD(NULL, getThis(), "help");
					RETURN_FALSE;
				}
			}
		} ZEND_HASH_FOREACH_END();
	}
	array_init(return_value);
	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(names), str_key, name) {
		zval short_name = {}, value = {};
		ZVAL_STR(&short_name, str_key);

		if (phalcon_array_isset_fetch(&value, &values, name, PH_COPY) || phalcon_array_isset_fetch(&value, &values, &short_name, PH_COPY)) {
			zval type = {};
			int t;
			phalcon_array_fetch(&type, &types, name, PH_READONLY);
			t = Z_LVAL(type);
			switch(t) {
				case PHALCON_CLI_OPTIONS_TYPE_INT:
					if (Z_TYPE(value) != IS_LONG) convert_to_long_ex(&value);
					break;

				case PHALCON_CLI_OPTIONS_TYPE_FLOAT:
					if (Z_TYPE(value) != IS_DOUBLE) convert_to_double_ex(&value);
					break;
				case PHALCON_CLI_OPTIONS_TYPE_BOOLEAN:
					if (!PHALCON_IS_BOOL(&value)) convert_to_boolean_ex(&value);
					break;
				case PHALCON_CLI_OPTIONS_TYPE_STRING:
					if (Z_TYPE(value) != IS_STRING) convert_to_string_ex(&value);
					break;
				case PHALCON_CLI_OPTIONS_TYPE_ARRAY:
					if (Z_TYPE(value) != IS_ARRAY) convert_to_array_ex(&value);
					break;
			}
			phalcon_array_update(return_value, name, &value, 0);
		} else if (phalcon_array_isset_fetch(&value, &default_values, name, PH_COPY)) {
			phalcon_array_update(return_value, name, &value, 0);
		}
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(&values);
}
