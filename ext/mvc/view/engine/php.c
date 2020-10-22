
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

	zval *path, *params, *must_clean = NULL, contents = {}, view = {};
	int clean = 0;

	phalcon_fetch_params(0, 2, 2, &path, &params, &must_clean);

	PHALCON_ENSURE_IS_STRING(path);

	if (must_clean) {
		clean = PHALCON_IS_TRUE(must_clean);
	}

	if (clean && phalcon_ob_get_level() >= 1) {
		phalcon_ob_clean();
	}

	/**
	 * Require the file
	 */
	if (phalcon_exec_file(&contents, NULL, path, params) == FAILURE) {
		RETURN_FALSE;
	}

	if (clean) {
		int flag = 0;

		phalcon_read_property(&view, getThis(), SL("_view"), PH_NOISY|PH_READONLY);
		PHALCON_CALL_METHOD_FLAG(flag, NULL, &view, "setcontent", &contents, &PHALCON_GLOBAL(z_true));
		zval_ptr_dtor(&contents);
		if (flag == FAILURE) {
			RETURN_FALSE;
		}
	} else {
		zend_print_zval_r(&contents, 0);
		zval_ptr_dtor(&contents);
	}

	RETURN_TRUE;
}
