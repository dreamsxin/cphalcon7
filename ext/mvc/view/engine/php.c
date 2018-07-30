
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

#include "mvc/view/engine/php.h"
#include "mvc/view/engine.h"
#include "mvc/view/engineinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/output.h"
#include "kernel/hash.h"
#include "kernel/require.h"
#include "kernel/object.h"
#include "kernel/debug.h"

/**
 * Phalcon\Mvc\View\Engine\Php
 *
 * Adapter to use PHP itself as templating engine
 */
zend_class_entry *phalcon_mvc_view_engine_php_ce;

PHP_METHOD(Phalcon_Mvc_View_Engine_Php, render);

static const zend_function_entry phalcon_mvc_view_engine_php_method_entry[] = {
	PHP_ME(Phalcon_Mvc_View_Engine_Php, render, arginfo_phalcon_mvc_view_engineinterface_render, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\View\Engine\Php initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_View_Engine_Php){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\View\\Engine, Php, mvc_view_engine_php, phalcon_mvc_view_engine_ce, phalcon_mvc_view_engine_php_method_entry, 0);

	zend_class_implements(phalcon_mvc_view_engine_php_ce, 1, phalcon_mvc_view_engineinterface_ce);

	return SUCCESS;
}

/**
 * Renders a view using the template engine
 *
 * @param string $path
 * @param array $params
 * @param boolean $mustClean
 */
PHP_METHOD(Phalcon_Mvc_View_Engine_Php, render){

	zval *path, *params, *must_clean = NULL, *partial = NULL, contents = {}, view = {}, *value = NULL;
	zend_string *str_key;
	zend_array *symbol_table;
	ulong idx;
	int clean = 0;

	phalcon_fetch_params(0, 2, 2, &path, &params, &must_clean, &partial);

	PHALCON_ENSURE_IS_STRING(path);

	if (must_clean) {
		clean = PHALCON_IS_TRUE(must_clean);
	}

	if (!partial) {
		partial = &PHALCON_GLOBAL(z_false);
	}

	if (clean) {
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			phalcon_ob_flush();
		}

		phalcon_ob_clean();
	}

	// phalcon_exec_file(NULL, getThis(), path, params);
	symbol_table = zend_rebuild_symbol_table();

	/**
	 * Create the variables in local symbol table
	 */
	if (Z_TYPE_P(params) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY_VAL_IND(Z_ARRVAL_P(params), idx, str_key, value) {
			zval key = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}
			phalcon_set_symbol(symbol_table, &key, value);
		} ZEND_HASH_FOREACH_END();
	}

	/**
	 * Require the file
	 */
	if (phalcon_require(Z_STRVAL_P(path)) == FAILURE) {
		ZVAL_FALSE(return_value);
		goto end;
	}

	if (clean) {
		int flag = 0;
		phalcon_ob_get_contents(&contents);
		phalcon_ob_clean();

		phalcon_read_property(&view, getThis(), SL("_view"), PH_NOISY|PH_READONLY);
		PHALCON_CALL_METHOD_FLAG(flag, NULL, &view, "setcontent", &contents);
		zval_ptr_dtor(&contents);
		if (flag == FAILURE) {
			ZVAL_FALSE(return_value);
			goto end;
		}
	}

	ZVAL_TRUE(return_value);

end:
	if (Z_TYPE_P(params) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(params), idx, str_key) {
			zval key = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}
			phalcon_del_symbol(symbol_table, &key);
		} ZEND_HASH_FOREACH_END();
	}
}
