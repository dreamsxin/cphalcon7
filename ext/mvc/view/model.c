
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

#include "mvc/view/model.h"
#include "mvc/view/exception.h"
#include "mvc/viewinterface.h"
#include "mvc/view/modelinterface.h"
#include "di.h"
#include "di/injectable.h"
#include "debug.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/file.h"
#include "kernel/string.h"
#include "kernel/output.h"
#include "kernel/debug.h"

#include "internal/arginfo.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\View\Model
 *
 * This component allows to render views without hicherquical levels
 *
 *<code>
 * $view = new Phalcon\Mvc\View\Model();
 * echo $view->render('templates/my-view', array('content' => $html));
 *</code>
 *
 */
zend_class_entry *phalcon_mvc_view_model_ce;

PHP_METHOD(Phalcon_Mvc_View_Model, __construct);
PHP_METHOD(Phalcon_Mvc_View_Model, setTemplate);
PHP_METHOD(Phalcon_Mvc_View_Model, getTemplate);
PHP_METHOD(Phalcon_Mvc_View_Model, setVars);
PHP_METHOD(Phalcon_Mvc_View_Model, getVars);
PHP_METHOD(Phalcon_Mvc_View_Model, setVar);
PHP_METHOD(Phalcon_Mvc_View_Model, getVar);
PHP_METHOD(Phalcon_Mvc_View_Model, addChild);
PHP_METHOD(Phalcon_Mvc_View_Model, appendChild);
PHP_METHOD(Phalcon_Mvc_View_Model, getChild);
PHP_METHOD(Phalcon_Mvc_View_Model, hasChild);
PHP_METHOD(Phalcon_Mvc_View_Model, setCaptureTo);
PHP_METHOD(Phalcon_Mvc_View_Model, getCaptureTo);
PHP_METHOD(Phalcon_Mvc_View_Model, setTerminal);
PHP_METHOD(Phalcon_Mvc_View_Model, getTerminal);
PHP_METHOD(Phalcon_Mvc_View_Model, setAppend);
PHP_METHOD(Phalcon_Mvc_View_Model, isAppend);
PHP_METHOD(Phalcon_Mvc_View_Model, setView);
PHP_METHOD(Phalcon_Mvc_View_Model, getView);
PHP_METHOD(Phalcon_Mvc_View_Model, render);
PHP_METHOD(Phalcon_Mvc_View_Model, __set);
PHP_METHOD(Phalcon_Mvc_View_Model, __get);
PHP_METHOD(Phalcon_Mvc_View_Model, __isset);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_model___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, vars)
	ZEND_ARG_INFO(0, template)
	ZEND_ARG_INFO(0, capture)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_view_model_method_entry[] = {
	PHP_ME(Phalcon_Mvc_View_Model, __construct, arginfo_phalcon_mvc_view_model___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_View_Model, setTemplate, arginfo_phalcon_mvc_view_modelinterface_settemplate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, getTemplate, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, setVars, arginfo_phalcon_mvc_view_modelinterface_setvars, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, getVars, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, setVar, arginfo_phalcon_mvc_view_modelinterface_setvar, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, getVar, arginfo_phalcon_mvc_view_modelinterface_getvar, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, addChild, arginfo_phalcon_mvc_view_modelinterface_addchild, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, appendChild, arginfo_phalcon_mvc_view_modelinterface_appendchild, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, getChild, arginfo_phalcon_mvc_view_modelinterface_getchild, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, hasChild, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, setCaptureTo, arginfo_phalcon_mvc_view_modelinterface_setcaptureto, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, getCaptureTo, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, setTerminal, arginfo_phalcon_mvc_view_modelinterface_setterminal, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, getTerminal, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, setAppend, arginfo_phalcon_mvc_view_modelinterface_setappend, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, isAppend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, setView, arginfo_phalcon_mvc_view_modelinterface_setview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, getView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, render, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, __set, arginfo___set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Model, __isset, arginfo___isset, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Mvc_View_Model, __toString, render, arginfo___tostring, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\View\Model initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_View_Model){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\View, Model, mvc_view_model, phalcon_di_injectable_ce, phalcon_mvc_view_model_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_view_model_ce, SL("_viewParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_mvc_view_model_ce, SL("_captureTo"), "content", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_model_ce, SL("_template"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_model_ce, SL("_childs"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_model_ce, SL("_terminal"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_model_ce, SL("_append"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_model_ce, SL("_view"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_view_model_ce, 1, phalcon_mvc_view_modelinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\View\Model constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_View_Model, __construct){

	zval *variables = NULL, *template = NULL, *capture = NULL;

	phalcon_fetch_params(0, 0, 3, &variables, &template, &capture);

	if (variables && Z_TYPE_P(variables) != IS_NULL) {
		PHALCON_CALL_SELF(NULL, "setvars", variables);
	}

	if (template && Z_TYPE_P(template) != IS_NULL) {
		PHALCON_CALL_SELF(NULL, "settemplate", template);
	}

	if (capture && Z_TYPE_P(capture) != IS_NULL) {
		PHALCON_CALL_SELF(NULL, "setcaptureto", capture);
	}
}

/**
 * Set the template to be used by this model
 *
 * @param  string $template
 * @return Phalcon\Mvc\View\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Model, setTemplate){

	zval *template;

	phalcon_fetch_params(0, 1, 0, &template);

	phalcon_update_property(getThis(), SL("_template"), template);

	RETURN_THIS();
}

/**
 * Get the template to be used by this model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View_Model, getTemplate){

	RETURN_MEMBER(getThis(), "_template");
}

/**
 * Set all the render params
 *
 * @param array $params
 * @param boolean $merge
 * @return Phalcon\Mvc\View\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Model, setVars){

	zval *params, *merge = NULL, view_params = {}, merged_params = {};

	phalcon_fetch_params(1, 1, 1, &params, &merge);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_true);
	}

	if (Z_TYPE_P(params) != IS_ARRAY) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "The render parameters must be an array");
		return;
	}

	if (zend_is_true(merge)) {
		phalcon_read_property(&view_params, getThis(), SL("_viewParams"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(view_params) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_params, &view_params, params);
			PHALCON_MM_ADD_ENTRY(&merged_params);
		} else {
			ZVAL_COPY_VALUE(&merged_params, params);
		}

		phalcon_update_property(getThis(), SL("_viewParams"), &merged_params);
	} else {
		phalcon_update_property(getThis(), SL("_viewParams"), params);
	}

	RETURN_MM_THIS();
}

/**
 * Get the vars
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View_Model, getVars){

	RETURN_MEMBER(getThis(), "_viewParams");
}

/**
 * Set a single view parameter
 *
 * @param string $key
 * @param mixed $value
 * @return Phalcon\Mvc\View\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Model, setVar){

	zval *key, *value, *isappend = NULL, view_params = {}, var = {}, var_append = {};

	phalcon_fetch_params(0, 2, 1, &key, &value, &isappend);

	if (isappend && zend_is_true(isappend)) {
		phalcon_read_property(&view_params, getThis(), SL("_viewParams"), PH_NOISY|PH_READONLY);

		if (Z_TYPE(view_params) != IS_ARRAY || !phalcon_array_isset_fetch(&var, &view_params, key, PH_READONLY)) {
			ZVAL_COPY_VALUE(&var_append, value);
		}
	} else {
		ZVAL_COPY_VALUE(&var_append, value);
	}

	phalcon_update_property_array(getThis(), SL("_viewParams"), key, &var_append);

	RETURN_THIS();
}

/**
 * Get the vars
 *
 * @param string $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_View_Model, getVar){

	zval *key, *default_value = NULL, view_params = {}, var = {};

	phalcon_fetch_params(0, 1, 1, &key, &default_value);

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&view_params, getThis(), SL("_viewParams"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(view_params) != IS_ARRAY || !phalcon_array_isset_fetch(&var, &view_params, key, PH_READONLY)) {
		ZVAL_COPY_VALUE(&var, default_value);
	}

	RETURN_CTOR(&var);
}

/**
 * Add a child model
 *
 * @param  Phalcon\Mvc\View\ModelInterface $child
 * @param  null|string $captureTo Optional; if specified, the "capture to" value to set on the child
 * @param  null|bool $append Optional; if specified, append to child  with the same capture
 * @return Phalcon\Mvc\View\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Model, addChild){

	zval *child, *capture_to = NULL, *append = NULL;

	phalcon_fetch_params(0, 1, 2, &child, &capture_to, &append);

	if (capture_to && !PHALCON_IS_EMPTY(capture_to)) {
		PHALCON_CALL_METHOD(NULL, child, "setcaptureto", capture_to);
	}

	if (append) {
		PHALCON_CALL_METHOD(NULL, child, "setappend", append);
	}

	phalcon_update_property_array_append(getThis(), SL("_childs"), child);

	RETURN_THIS();
}

/**
 * Append a child model
 *
 * @param  Phalcon\Mvc\View\ModelInterface $child
 * @param  null|string $captureTo Optional; if specified, the "capture to" value to set on the child
 * @return Phalcon\Mvc\View\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Model, appendChild){

	zval *child, *capture_to = NULL;

	phalcon_fetch_params(0, 1, 1, &child, &capture_to);

	if (!capture_to) {
		capture_to = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_SELF(NULL, "addchild", child, capture_to, &PHALCON_GLOBAL(z_true));

	RETURN_THIS();
}

/**
 * Return a child model or all child model
 *
 * @param null|string $captureTo
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View_Model, getChild){

	zval *capture_to = NULL, childs = {}, *child;

	phalcon_fetch_params(1, 0, 1, &capture_to);

	if (capture_to) {
		array_init(return_value);

		phalcon_read_property(&childs, getThis(), SL("_childs"), PH_NOISY|PH_READONLY);

		if (Z_TYPE(childs) != IS_ARRAY) {
			RETURN_MM_EMPTY_ARRAY();
		}

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(childs), child) {
			zval child_capture_to = {};

			PHALCON_MM_CALL_METHOD(&child_capture_to, child, "getcaptureto");
			PHALCON_MM_ADD_ENTRY(&child_capture_to);
			if (phalcon_memnstr(capture_to, &child_capture_to)) {
				phalcon_array_append(return_value, child, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
		RETURN_MM();
	}
	RETURN_MM_MEMBER(getThis(), "_childs");
}

/**
 * Does the model have any children?
 *
 * @param null|string $captureTo
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_View_Model, hasChild){

	zval *capture_to = NULL, childs = {}, *child;

	phalcon_fetch_params(1, 0, 1, &capture_to);

	phalcon_read_property(&childs, getThis(), SL("_childs"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(childs) != IS_ARRAY) {
		RETURN_MM_FALSE;
	}

	if (!phalcon_fast_count_ev(&childs)) {
		RETURN_MM_FALSE;
	}

	if (capture_to) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(childs), child) {
			zval child_capture_to = {};

			PHALCON_MM_CALL_METHOD(&child_capture_to, child, "getcaptureto");
			PHALCON_MM_ADD_ENTRY(&child_capture_to);
			if (phalcon_memnstr(capture_to, &child_capture_to)) {
				RETURN_TRUE;
			}
		} ZEND_HASH_FOREACH_END();

		RETURN_MM_FALSE;
	}

	RETURN_MM_TRUE;
}

/**
 * Set the name of the variable to capture this model to, if it is a child model
 *
 * @param string $capture
 * @return Phalcon\Mvc\View\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Model, setCaptureTo){

	zval *capture;

	phalcon_fetch_params(0, 1, 0, &capture);

	phalcon_update_property(getThis(), SL("_captureTo"), capture);

	RETURN_THIS();
}

/**
 * Get the name of the variable to which to capture this model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View_Model, getCaptureTo){

	RETURN_MEMBER(getThis(), "_captureTo");
}

/**
 * Set flag indicating whether or not this is considered a terminal or standalone model
 *
 * @param boolean $terminate
 * @return Phalcon\Mvc\View\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Model, setTerminal){

	zval *terminate;

	phalcon_fetch_params(0, 1, 0, &terminate);

	phalcon_update_property(getThis(), SL("_terminate"), terminate);

	RETURN_THIS();
}

/**
 * Is this considered a terminal or standalone model?
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_View_Model, getTerminal){

	RETURN_MEMBER(getThis(), "_terminate");
}

/**
 * Set flag indicating whether or not append to child  with the same capture
 *
 * @param boolean $append
 * @return Phalcon\Mvc\View\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Model, setAppend){

	zval *append;

	phalcon_fetch_params(0, 1, 0, &append);

	phalcon_update_property(getThis(), SL("_append"), append);

	RETURN_THIS();
}

/**
 * Is this append to child  with the same capture?
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_View_Model, isAppend){

	zval append = {};

	phalcon_read_property(&append, getThis(), SL("_append"), PH_NOISY|PH_READONLY);

	if (zend_is_true(&append)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Set the view
 *
 * @param Phalcon\Mvc\ViewInterface $view
 * @return Phalcon\Mvc\View\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Model, setView){

	zval *view;

	phalcon_fetch_params(0, 1, 0, &view);

	phalcon_update_property(getThis(), SL("_view"), view);

	RETURN_THIS();
}

/**
 * Get the view
 *
 * @return Phalcon\Mvc\ViewInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Model, getView){


	RETURN_MEMBER(getThis(), "_view");
}

/**
 * Renders the view
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View_Model, render){

	zval child_contents = {}, debug_message = {}, childs = {}, *child, view = {}, dependency_injector = {}, service = {}, event_name = {};
	zval status = {}, not_exists = {}, base_path = {}, paths = {}, views_dir = {}, vars = {}, new_vars = {}, tpl = {};
	zval views_dir_path = {}, engines = {}, *engine, *path, exception_message = {}, contents = {};
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_INIT();
	array_init(&child_contents);
	PHALCON_MM_ADD_ENTRY(&child_contents);

	PHALCON_MM_CALL_METHOD(&childs, getThis(), "getchild");

	if (Z_TYPE(childs) == IS_ARRAY && phalcon_fast_count_ev(&childs)) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(childs), child) {
			zval isappend = {}, capture = {}, content = {}, child_content = {};

			PHALCON_MM_CALL_METHOD(&isappend, child, "isappend");
			PHALCON_MM_ADD_ENTRY(&isappend);
			PHALCON_MM_CALL_METHOD(&capture, child, "getcaptureto");
			PHALCON_MM_ADD_ENTRY(&capture);
			PHALCON_MM_CALL_METHOD(&content, child, "render");
			PHALCON_MM_ADD_ENTRY(&content);

			if (zend_is_true(&isappend)) {
				if (Z_TYPE(child_contents) == IS_ARRAY && phalcon_array_isset_fetch(&child_content, &child_contents, &capture, PH_READONLY)) {
					zval content_append = {};
					PHALCON_CONCAT_VV(&content_append, &child_content, &content);
					phalcon_array_update(&child_contents, &capture, &content_append, 0);
				} else {
					phalcon_array_update(&child_contents, &capture, &content, PH_COPY);
				}
			} else {
				phalcon_array_update(&child_contents, &capture, &content, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_ob_start();

	phalcon_read_property(&view, getThis(), SL("_view"), PH_COPY);

	if (Z_TYPE(view) != IS_OBJECT) {
		PHALCON_MM_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);

		ZVAL_STR(&service, IS(view));
		PHALCON_MM_CALL_METHOD(&view, &dependency_injector, "getshared", &service);
		PHALCON_MM_ADD_ENTRY(&view);
	}

	PHALCON_MM_VERIFY_INTERFACE(&view, phalcon_mvc_viewinterface_ce);

	/**
	 * Call beforeRender if there is an events manager
	 */
	PHALCON_MM_ZVAL_STRING(&event_name, "view:beforeRender");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_NULL();
	}
	zval_ptr_dtor(&status);

	ZVAL_TRUE(&not_exists);

	PHALCON_MM_CALL_METHOD(&base_path, &view, "getbasepath");
	PHALCON_MM_ADD_ENTRY(&base_path);
	if (Z_TYPE(base_path) != IS_ARRAY) {
		array_init(&paths);
		PHALCON_MM_ADD_ENTRY(&paths);
		phalcon_array_append(&paths, &base_path, PH_COPY);
	} else {
		ZVAL_COPY_VALUE(&paths, &base_path);
	}

	PHALCON_MM_CALL_METHOD(&views_dir, &view, "getviewsdir");
	PHALCON_MM_ADD_ENTRY(&views_dir);
	PHALCON_MM_CALL_METHOD(&vars, getThis(), "getvars");
	PHALCON_MM_ADD_ENTRY(&vars);
	PHALCON_MM_CALL_METHOD(&tpl, getThis(), "gettemplate");
	PHALCON_MM_ADD_ENTRY(&tpl);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "Render Model View: ", &tpl);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	if (Z_TYPE(vars) == IS_ARRAY) {
		phalcon_fast_array_merge(&new_vars, &vars, &child_contents);
		PHALCON_MM_ADD_ENTRY(&new_vars);
	} else {
		ZVAL_COPY_VALUE(&new_vars, &child_contents);
	}

	PHALCON_CONCAT_VV(&views_dir_path, &views_dir, &tpl);
	PHALCON_MM_ADD_ENTRY(&views_dir_path);

	PHALCON_MM_CALL_METHOD(&engines, &view, "getEngines");
	PHALCON_MM_ADD_ENTRY(&engines);

	/**
	 * Views are rendered in each engine
	 */
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(engines), idx, str_key, engine) {
		zval extension = {};
		if (str_key) {
			ZVAL_STR(&extension, str_key);
		} else {
			ZVAL_LONG(&extension, idx);
		}

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(paths), path) {
			zval view_engine_path = {};

			PHALCON_CONCAT_VVV(&view_engine_path, path, &views_dir_path, &extension);
			PHALCON_MM_ADD_ENTRY(&view_engine_path);
			if (phalcon_file_exists(&view_engine_path) == SUCCESS) {

				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					PHALCON_CONCAT_SV(&debug_message, "--Found: ", &view_engine_path);
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}

				/**
				 * Call beforeRenderView if there is a events manager available
				 */
				PHALCON_MM_ZVAL_STRING(&event_name, "view:beforeRenderView");
				PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &view_engine_path);
				if (PHALCON_IS_FALSE(&status)) {
					continue;
				}
				zval_ptr_dtor(&status);

				PHALCON_MM_CALL_METHOD(NULL, engine, "render", &view_engine_path, &new_vars, &PHALCON_GLOBAL(z_false));

				/**
				 * Call afterRenderView if there is a events manager available
				 */
				ZVAL_FALSE(&not_exists);

				PHALCON_MM_ZVAL_STRING(&event_name, "view:afterRenderView");
				PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireeventcancel", &event_name);
				break;
			} else if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				PHALCON_CONCAT_SV(&debug_message, "--Not Found: ", &view_engine_path);
				PHALCON_DEBUG_LOG(&debug_message);
				zval_ptr_dtor(&debug_message);
			}
		} ZEND_HASH_FOREACH_END();
	} ZEND_HASH_FOREACH_END();

	/**
	 * Always throw an exception if the view does not exist
	 */
	if (PHALCON_IS_TRUE(&not_exists)) {
		PHALCON_CONCAT_SVS(&exception_message, "View '", &views_dir_path, "' was not found in the views directory");
		PHALCON_MM_ADD_ENTRY(&exception_message);
		PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_view_exception_ce, &exception_message);
		return;
	}

	/**
	 * Call afterRender event
	 */
	PHALCON_MM_ZVAL_STRING(&event_name, "view:afterRender");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

	phalcon_ob_get_contents(&contents);
	phalcon_ob_end_clean();

	RETURN_MM_NCTOR(&contents);
}

/**
 * Magic method to pass variables to the views
 *
 * @param string $key
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_View_Model, __set){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(getThis(), SL("_viewParams"), key, value);

}

/**
 * Magic method to retrieve a variable passed to the view
 *
 * @param string $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_View_Model, __get){

	zval *key, params = {}, value = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&params, getThis(), SL("_viewParams"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset_fetch(&value, &params, key, PH_READONLY)) {
		RETURN_CTOR(&value);
	}

	RETURN_NULL();
}

/**
 * Magic method to inaccessible a variable passed to the view
 *
 * @param string $key
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_View_Model, __isset){

	zval *key, params = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&params, getThis(), SL("_viewParams"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset(&params, key)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
