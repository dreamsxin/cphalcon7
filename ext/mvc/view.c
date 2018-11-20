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

#include "mvc/view.h"
#include "mvc/viewinterface.h"
#include "mvc/view/engine.h"
#include "mvc/view/engineinterface.h"
#include "mvc/view/engine/php.h"
#include "mvc/view/exception.h"
#include "mvc/view/modelinterface.h"
#include "cache/backendinterface.h"
#include "di/injectable.h"
#include "debug.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/fcall.h"
#include "kernel/output.h"
#include "kernel/operators.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/string.h"
#include "kernel/file.h"
#include "kernel/debug.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Mvc\View
 *
 * Phalcon\Mvc\View is a class for working with the "view" portion of the model-view-controller pattern.
 * That is, it exists to help keep the view script separate from the model and controller scripts.
 * It provides a system of helpers, output filters, and variable escaping.
 *
 * <code>
 * //Setting views directory
 * $view = new Phalcon\Mvc\View();
 * $view->setViewsDir('app/views/');
 *
 * $view->start();
 * //Shows recent posts view (app/views/posts/recent.phtml)
 * $view->render('posts', 'recent');
 * $view->finish();
 *
 * //Printing views output
 * echo $view->getContent();
 * </code>
 */
zend_class_entry *phalcon_mvc_view_ce;

PHP_METHOD(Phalcon_Mvc_View, __construct);
PHP_METHOD(Phalcon_Mvc_View, setViewsDir);
PHP_METHOD(Phalcon_Mvc_View, getViewsDir);
PHP_METHOD(Phalcon_Mvc_View, setLayoutsDir);
PHP_METHOD(Phalcon_Mvc_View, getLayoutsDir);
PHP_METHOD(Phalcon_Mvc_View, setPartialsDir);
PHP_METHOD(Phalcon_Mvc_View, getPartialsDir);
PHP_METHOD(Phalcon_Mvc_View, setBasePath);
PHP_METHOD(Phalcon_Mvc_View, getBasePath);
PHP_METHOD(Phalcon_Mvc_View, getCurrentRenderLevel);
PHP_METHOD(Phalcon_Mvc_View, getRenderLevel);
PHP_METHOD(Phalcon_Mvc_View, setRenderLevel);
PHP_METHOD(Phalcon_Mvc_View, disableLevel);
PHP_METHOD(Phalcon_Mvc_View, getDisabledLevels);
PHP_METHOD(Phalcon_Mvc_View, setMainView);
PHP_METHOD(Phalcon_Mvc_View, getMainView);
PHP_METHOD(Phalcon_Mvc_View, setLayout);
PHP_METHOD(Phalcon_Mvc_View, getLayout);
PHP_METHOD(Phalcon_Mvc_View, setTemplateBefore);
PHP_METHOD(Phalcon_Mvc_View, cleanTemplateBefore);
PHP_METHOD(Phalcon_Mvc_View, setTemplateAfter);
PHP_METHOD(Phalcon_Mvc_View, cleanTemplateAfter);
PHP_METHOD(Phalcon_Mvc_View, setParamToView);
PHP_METHOD(Phalcon_Mvc_View, getParamsToView);
PHP_METHOD(Phalcon_Mvc_View, setVars);
PHP_METHOD(Phalcon_Mvc_View, setVar);
PHP_METHOD(Phalcon_Mvc_View, getVar);
PHP_METHOD(Phalcon_Mvc_View, setControllerName);
PHP_METHOD(Phalcon_Mvc_View, getControllerName);
PHP_METHOD(Phalcon_Mvc_View, setActionName);
PHP_METHOD(Phalcon_Mvc_View, getActionName);
PHP_METHOD(Phalcon_Mvc_View, setParams);
PHP_METHOD(Phalcon_Mvc_View, getParams);
PHP_METHOD(Phalcon_Mvc_View, setNamespaceName);
PHP_METHOD(Phalcon_Mvc_View, getNamespaceName);
PHP_METHOD(Phalcon_Mvc_View, start);
PHP_METHOD(Phalcon_Mvc_View, _loadTemplateEngines);
PHP_METHOD(Phalcon_Mvc_View, _engineRender);
PHP_METHOD(Phalcon_Mvc_View, registerEngines);
PHP_METHOD(Phalcon_Mvc_View, getRegisteredEngines);
PHP_METHOD(Phalcon_Mvc_View, getEngines);
PHP_METHOD(Phalcon_Mvc_View, exists);
PHP_METHOD(Phalcon_Mvc_View, render);
PHP_METHOD(Phalcon_Mvc_View, pick);
PHP_METHOD(Phalcon_Mvc_View, partial);
PHP_METHOD(Phalcon_Mvc_View, getRender);
PHP_METHOD(Phalcon_Mvc_View, finish);
PHP_METHOD(Phalcon_Mvc_View, _createCache);
PHP_METHOD(Phalcon_Mvc_View, isCaching);
PHP_METHOD(Phalcon_Mvc_View, getCache);
PHP_METHOD(Phalcon_Mvc_View, cache);
PHP_METHOD(Phalcon_Mvc_View, setContent);
PHP_METHOD(Phalcon_Mvc_View, getContent);
PHP_METHOD(Phalcon_Mvc_View, startSection);
PHP_METHOD(Phalcon_Mvc_View, stopSection);
PHP_METHOD(Phalcon_Mvc_View, section);
PHP_METHOD(Phalcon_Mvc_View, getActiveRenderPath);
PHP_METHOD(Phalcon_Mvc_View, disable);
PHP_METHOD(Phalcon_Mvc_View, enable);
PHP_METHOD(Phalcon_Mvc_View, isDisabled);
PHP_METHOD(Phalcon_Mvc_View, enableNamespaceView);
PHP_METHOD(Phalcon_Mvc_View, disableNamespaceView);
PHP_METHOD(Phalcon_Mvc_View, enableMultiNamespaceView);
PHP_METHOD(Phalcon_Mvc_View, disableMultiNamespaceView);
PHP_METHOD(Phalcon_Mvc_View, enableLowerCase);
PHP_METHOD(Phalcon_Mvc_View, disableLowerCase);
PHP_METHOD(Phalcon_Mvc_View, setConverter);
PHP_METHOD(Phalcon_Mvc_View, getConverter);
PHP_METHOD(Phalcon_Mvc_View, reset);
PHP_METHOD(Phalcon_Mvc_View, __set);
PHP_METHOD(Phalcon_Mvc_View, __get);
PHP_METHOD(Phalcon_Mvc_View, __isset);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_view_method_entry[] = {
	PHP_ME(Phalcon_Mvc_View, __construct, arginfo_phalcon_mvc_view___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_View, setViewsDir, arginfo_phalcon_mvc_viewinterface_setviewsdir, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getViewsDir, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setLayoutsDir, arginfo_phalcon_mvc_viewinterface_setlayoutsdir, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getLayoutsDir, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setPartialsDir, arginfo_phalcon_mvc_viewinterface_setpartialsdir, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getPartialsDir, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setBasePath, arginfo_phalcon_mvc_viewinterface_setbasepath, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getBasePath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getCurrentRenderLevel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getRenderLevel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setRenderLevel, arginfo_phalcon_mvc_viewinterface_setrenderlevel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, disableLevel, arginfo_phalcon_mvc_viewinterface_disablelevel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getDisabledLevels, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setMainView, arginfo_phalcon_mvc_viewinterface_setmainview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getMainView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setLayout, arginfo_phalcon_mvc_viewinterface_setlayout, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getLayout, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setTemplateBefore, arginfo_phalcon_mvc_viewinterface_settemplatebefore, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, cleanTemplateBefore, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setTemplateAfter, arginfo_phalcon_mvc_viewinterface_settemplateafter, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, cleanTemplateAfter, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setParamToView, arginfo_phalcon_mvc_viewinterface_setparamtoview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getParamsToView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setVars, arginfo_phalcon_mvc_viewinterface_setvars, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setVar, arginfo_phalcon_mvc_viewinterface_setvar, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getVar, arginfo_phalcon_mvc_viewinterface_getvar, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setControllerName, arginfo_phalcon_mvc_viewinterface_setcontrollername, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getControllerName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setActionName, arginfo_phalcon_mvc_viewinterface_setactionname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getActionName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setParams, arginfo_phalcon_mvc_viewinterface_setparams, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setNamespaceName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getNamespaceName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, _loadTemplateEngines, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_View, _engineRender, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_View, registerEngines, arginfo_phalcon_mvc_viewinterface_registerengines, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getRegisteredEngines, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getEngines, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, exists, arginfo_phalcon_mvc_viewinterface_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, render, arginfo_phalcon_mvc_viewinterface_render, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, pick, arginfo_phalcon_mvc_viewinterface_pick, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, partial, arginfo_phalcon_mvc_viewinterface_partial, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getRender, arginfo_phalcon_mvc_viewinterface_getrender, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, finish, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, _createCache, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_View, isCaching, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getCache, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, cache, arginfo_phalcon_mvc_viewinterface_cache, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setContent, arginfo_phalcon_mvc_viewinterface_setcontent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getContent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, startSection, arginfo_phalcon_mvc_viewinterface_startsection, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, stopSection, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, section, arginfo_phalcon_mvc_viewinterface_section, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getActiveRenderPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, disable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, enable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, isDisabled, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, enableNamespaceView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, disableNamespaceView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, enableMultiNamespaceView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, disableMultiNamespaceView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, enableLowerCase, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, disableLowerCase, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setConverter, arginfo_phalcon_mvc_viewinterface_setconverter, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getConverter, arginfo_phalcon_mvc_viewinterface_getconverter, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, reset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, __set, arginfo___set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, __isset, arginfo___isset, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Mvc_View, insert, partial, arginfo_phalcon_mvc_viewinterface_partial, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\View initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_View){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, View, mvc_view, phalcon_di_injectable_ce, phalcon_mvc_view_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_view_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_mvc_view_ce, SL("_basePath"), "", ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_mvc_view_ce, SL("_content"), "", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_sections"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_view_ce, SL("_renderLevel"), 6, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_view_ce, SL("_currentRenderLevel"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_disabledLevels"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_viewParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_layout"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_mvc_view_ce, SL("_layoutsDir"), "", ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_mvc_view_ce, SL("_partialsDir"), "", ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_enableLayoutsAbsolutePath"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_enablePartialsAbsolutePath"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_viewsDir"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_enableNamespaceView"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_enableMultiNamespaceView"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_templatesBefore"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_templatesAfter"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_engines"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_registeredEngines"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_mvc_view_ce, SL("_mainView"), "index", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_controllerName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_namespaceName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_actionName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_params"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_pickView"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_cache"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_view_ce, SL("_cacheLevel"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_cacheMode"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_activeRenderPath"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_disabled"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_lowerCase"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_converters"), ZEND_ACC_PROTECTED);

	/**
	 * Render level
	 */
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_MAIN"), PHALCON_VIEW_LEVEL_MAIN);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_MAIN_LAYOUT"), PHALCON_VIEW_LEVEL_MAIN);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_AFTER_TEMPLATE"), PHALCON_VIEW_LEVEL_AFTER_TEMPLATE);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_NAMESPACE"), PHALCON_VIEW_LEVEL_NAMESPACE);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_CONTROLLER"), PHALCON_VIEW_LEVEL_CONTROLLER);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_LAYOUT"), PHALCON_VIEW_LEVEL_CONTROLLER);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_BEFORE_TEMPLATE"), PHALCON_VIEW_LEVEL_BEFORE_TEMPLATE);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_ACTION"), PHALCON_VIEW_LEVEL_ACTION);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_ACTION_VIEW"), PHALCON_VIEW_LEVEL_ACTION);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_NO_RENDER"), PHALCON_VIEW_LEVEL_NO_RENDER);

	zend_declare_class_constant_bool(phalcon_mvc_view_ce, SL("CACHE_MODE_NONE"), 0);
	zend_declare_class_constant_bool(phalcon_mvc_view_ce, SL("CACHE_MODE_INVERSE"), 1);

	zend_class_implements(phalcon_mvc_view_ce, 1, phalcon_mvc_viewinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\View constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_View, __construct){

	zval *options = NULL;

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		phalcon_update_property(getThis(), SL("_options"), options);
	}

	phalcon_update_property_empty_array(getThis(), SL("_sections"));
	phalcon_update_property_empty_array(getThis(), SL("_converters"));
}

/**
 * Sets views directory. Depending of your platform, always add a trailing slash or backslash
 *
 * @param string $viewsDir
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setViewsDir){

	zval *views_dir, path = {};

	phalcon_fetch_params(0, 1, 0, &views_dir);

	phalcon_add_trailing_slash(&path, views_dir);
	phalcon_update_property(getThis(), SL("_viewsDir"), &path);
	zval_ptr_dtor(&path);

	RETURN_THIS();
}

/**
 * Gets views directory
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getViewsDir){


	RETURN_MEMBER(getThis(), "_viewsDir");
}

/**
 * Sets the layouts sub-directory. Must be a directory under the views directory. Depending of your platform, always add a trailing slash or backslash
 *
 *<code>
 * $view->setLayoutsDir('../common/layouts/');
 *</code>
 *
 * @param string $layoutsDir
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setLayoutsDir){

	zval *layouts_dir, *absolute_path = NULL, path = {};
	int absolute = 0;

	phalcon_fetch_params(0, 1, 1, &layouts_dir, &absolute_path);
	phalcon_add_trailing_slash(&path, layouts_dir);
	phalcon_update_property(getThis(), SL("_layoutsDir"), &path);
	zval_ptr_dtor(&path);
	absolute = absolute_path ? zend_is_true(absolute_path) : 0;
	phalcon_update_property_bool(getThis(), SL("_enableLayoutsAbsolutePath"), absolute);
	RETURN_THIS();
}

/**
 * Gets the current layouts sub-directory
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getLayoutsDir){


	RETURN_MEMBER(getThis(), "_layoutsDir");
}

/**
 * Sets a partials sub-directory. Must be a directory under the views directory. Depending of your platform, always add a trailing slash or backslash
 *
 *<code>
 * $view->setPartialsDir('../common/partials/');
 *</code>
 *
 * @param string $partialsDir
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setPartialsDir){

	zval *partials_dir, *absolute_path = NULL, path = {};
	int absolute = 0;

	phalcon_fetch_params(0, 1, 1, &partials_dir, &absolute_path);
	phalcon_add_trailing_slash(&path, partials_dir);
	absolute = absolute_path ? zend_is_true(absolute_path) : 0;
	phalcon_update_property(getThis(), SL("_partialsDir"), &path);
	zval_ptr_dtor(&path);
	phalcon_update_property_bool(getThis(), SL("_enablePartialsAbsolutePath"), absolute);
	RETURN_THIS();
}

/**
 * Gets the current partials sub-directory
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getPartialsDir){


	RETURN_MEMBER(getThis(), "_partialsDir");
}

/**
 * Sets base path. Depending of your platform, always add a trailing slash or backslash
 *
 * <code>
 * 	$view->setBasePath(__DIR__ . '/');
 * </code>
 *
 * @param string|array $basePath
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setBasePath){

	zval *base_path, paths, *path;

	phalcon_fetch_params(0, 1, 0, &base_path);

	if (Z_TYPE_P(base_path) == IS_ARRAY) {
		array_init(&paths);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(base_path), path) {
			zval tmp = {};
			phalcon_add_trailing_slash(&tmp, path);
			zval_ptr_dtor(&tmp);
			phalcon_array_append(&paths, path, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_update_property(getThis(), SL("_basePath"), &paths);
		zval_ptr_dtor(&paths);
	} else {
		zval tmp = {};
		phalcon_add_trailing_slash(&tmp, base_path);
		zval_ptr_dtor(&tmp);
		phalcon_update_property(getThis(), SL("_basePath"), base_path);
	}

	RETURN_THIS();
}

/**
 * Gets base path
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getBasePath){

	RETURN_MEMBER(getThis(), "_basePath");
}

/**
 * Returns the render level for the view
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_View, getCurrentRenderLevel) {

	RETURN_MEMBER(getThis(), "_currentRenderLevel");
}

/**
 * Returns the render level for the view
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_View, getRenderLevel) {

	RETURN_MEMBER(getThis(), "_renderLevel");
}

/**
 * Sets the render level for the view
 *
 * <code>
 * 	//Render the view related to the controller only
 * 	$this->view->setRenderLevel(View::LEVEL_LAYOUT);
 * </code>
 *
 * @param string $level
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setRenderLevel){

	zval *level;

	phalcon_fetch_params(0, 1, 0, &level);

	phalcon_update_property(getThis(), SL("_renderLevel"), level);
	RETURN_THIS();
}

/**
 * Disables a specific level of rendering
 *
 *<code>
 * //Render all levels except ACTION level
 * $this->view->disableLevel(View::LEVEL_ACTION_VIEW);
 *</code>
 *
 * @param int|array $level
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, disableLevel){

	zval *level;

	phalcon_fetch_params(0, 1, 0, &level);

	if (Z_TYPE_P(level) == IS_ARRAY) {
		phalcon_update_property(getThis(), SL("_disabledLevels"), level);
	} else {
		phalcon_update_property_array(getThis(), SL("_disabledLevels"), level, &PHALCON_GLOBAL(z_true));
	}

	RETURN_THIS();
}

/**
 * Returns an array with disabled render levels
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View, getDisabledLevels) {

	RETURN_MEMBER(getThis(), "_disabledLevels");
}

/**
 * Sets default view name. Must be a file without extension in the views directory
 *
 * <code>
 * 	//Renders as main view views-dir/base.phtml
 * 	$this->view->setMainView('base');
 * </code>
 *
 * @param string $viewPath
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setMainView){

	zval *view_path;

	phalcon_fetch_params(0, 1, 0, &view_path);

	phalcon_update_property(getThis(), SL("_mainView"), view_path);
	RETURN_THIS();
}

/**
 * Returns the name of the main view
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getMainView){


	RETURN_MEMBER(getThis(), "_mainView");
}

/**
 * Change the layout to be used instead of using the name of the latest controller name
 *
 * <code>
 * 	$this->view->setLayout('main');
 * </code>
 *
 * @param string $layout
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setLayout){

	zval *layout;

	phalcon_fetch_params(0, 1, 0, &layout);

	phalcon_update_property(getThis(), SL("_layout"), layout);
	RETURN_THIS();
}

/**
 * Returns the name of the main view
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getLayout){


	RETURN_MEMBER(getThis(), "_layout");
}

/**
 * Appends template before controller layout
 *
 * @param string|array $templateBefore
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setTemplateBefore){

	zval *template_before, array_template = {};

	phalcon_fetch_params(0, 1, 0, &template_before);

	if (Z_TYPE_P(template_before) != IS_ARRAY) {
		array_init_size(&array_template, 1);
		phalcon_array_append(&array_template, template_before, PH_COPY);
		phalcon_update_property(getThis(), SL("_templatesBefore"), &array_template);
		zval_ptr_dtor(&array_template);
	} else {
		phalcon_update_property(getThis(), SL("_templatesBefore"), template_before);
	}

	RETURN_THIS();
}

/**
 * Resets any template before layouts
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, cleanTemplateBefore){


	phalcon_update_property_null(getThis(), SL("_templatesBefore"));
	RETURN_THIS();
}

/**
 * Appends template after controller layout
 *
 * @param string|array $templateAfter
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setTemplateAfter){

	zval *template_after, array_template = {};

	phalcon_fetch_params(0, 1, 0, &template_after);

	if (Z_TYPE_P(template_after) != IS_ARRAY) {
		array_init_size(&array_template, 1);
		phalcon_array_append(&array_template, template_after, PH_COPY);
		phalcon_update_property(getThis(), SL("_templatesAfter"), &array_template);
		zval_ptr_dtor(&array_template);
	} else {
		phalcon_update_property(getThis(), SL("_templatesAfter"), template_after);
	}

	RETURN_THIS();
}

/**
 * Resets any template after layouts
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, cleanTemplateAfter){


	phalcon_update_property_null(getThis(), SL("_templatesAfter"));
	RETURN_THIS();
}

/**
 * Adds parameters to views (alias of setVar)
 *
 *<code>
 *	$this->view->setParamToView('products', $products);
 *</code>
 *
 * @param string $key
 * @param mixed $value
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setParamToView){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(getThis(), SL("_viewParams"), key, value);
	RETURN_THIS();
}

/**
 * Returns parameters to views
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View, getParamsToView){


	RETURN_MEMBER(getThis(), "_viewParams");
}

/**
 * Set all the render params
 *
 *<code>
 *	$this->view->setVars(array('products' => $products));
 *</code>
 *
 * @param array $params
 * @param boolean $merge
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setVars){

	zval *params, *merge = NULL, view_params = {};

	phalcon_fetch_params(0, 1, 1, &params, &merge);
	PHALCON_SEPARATE_PARAM(params);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_true);
	}

	if (Z_TYPE_P(params) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "The render parameters must be an array");
		return;
	}
	if (zend_is_true(merge)) {
		phalcon_read_property(&view_params, getThis(), SL("_viewParams"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(view_params) == IS_ARRAY) {
			phalcon_array_replace(&view_params, params);
		} else {
			phalcon_update_property(getThis(), SL("_viewParams"), params);
		}
	} else {
		phalcon_update_property(getThis(), SL("_viewParams"), params);
	}

	RETURN_THIS();
}

/**
 * Set a single view parameter
 *
 *<code>
 *	$this->view->setVar('products', $products);
 *</code>
 *
 * @param string $key
 * @param mixed $value
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setVar){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(getThis(), SL("_viewParams"), key, value);
	RETURN_THIS();
}

/**
 * Returns a parameter previously set in the view
 *
 * @param string $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_View, getVar){

	zval *key, params = {}, value = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&params, getThis(), SL("_viewParams"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset_fetch(&value, &params, key, PH_READONLY)) {
		RETURN_CTOR(&value);
	}

	RETURN_NULL();
}

/**
 * Sets the controller name to be view
 *
 * @param string $controllerName
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setControllerName){

	zval *controller_name;

	phalcon_fetch_params(0, 1, 0, &controller_name);

	phalcon_update_property(getThis(), SL("_controllerName"), controller_name);
	RETURN_THIS();
}

/**
 * Gets the name of the controller rendered
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getControllerName){


	RETURN_MEMBER(getThis(), "_controllerName");
}

/**
 * Sets the action name to be view
 *
 * @param string $actionName
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setActionName){

	zval *action_name;

	phalcon_fetch_params(0, 1, 0, &action_name);

	phalcon_update_property(getThis(), SL("_actionName"), action_name);
	RETURN_THIS();
}

/**
 * Gets the name of the action rendered
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getActionName){


	RETURN_MEMBER(getThis(), "_actionName");
}

/**
 * Sets the extra parameters to be view
 *
 * @param array $params
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setParams){

	zval *params;

	phalcon_fetch_params(0, 1, 0, &params);

	phalcon_update_property(getThis(), SL("_params"), params);
	RETURN_THIS();
}

/**
 * Gets extra parameters of the action rendered
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View, getParams){


	RETURN_MEMBER(getThis(), "_params");
}

PHP_METHOD(Phalcon_Mvc_View, setNamespaceName){

	zval *namespace_name;

	phalcon_fetch_params(0, 1, 0, &namespace_name);

	phalcon_update_property(getThis(), SL("_namespaceName"), namespace_name);
	RETURN_THIS();
}

PHP_METHOD(Phalcon_Mvc_View, getNamespaceName){


	RETURN_MEMBER(getThis(), "_namespaceName");
}

/**
 * Starts rendering process enabling the output buffering
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, start){

	phalcon_update_property_null(getThis(), SL("_content"));
	phalcon_ob_start();
	RETURN_THIS();
}

/**
 * Loads registered template engines, if none is registered it will use Phalcon\Mvc\View\Engine\Php
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View, _loadTemplateEngines){

	zval dependency_injector = {}, registered_engines = {}, php_engine = {}, arguments = {}, *engine_service;
	zend_string *str_key;

	PHALCON_MM_INIT();
	phalcon_read_property(return_value, getThis(), SL("_engines"), PH_COPY);

	/**
	 * If the engines aren't initialized 'engines' is false
	 */
	if (PHALCON_IS_FALSE(return_value)) {
		PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);

		array_init(return_value);

		phalcon_read_property(&registered_engines, getThis(), SL("_registeredEngines"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(registered_engines) != IS_ARRAY) {
			/**
			 * We use Phalcon\Mvc\View\Engine\Php as default
			 */
			object_init_ex(&php_engine, phalcon_mvc_view_engine_php_ce);
			PHALCON_MM_ADD_ENTRY(&php_engine);
			PHALCON_MM_CALL_METHOD(NULL, &php_engine, "__construct", getThis(), &dependency_injector);

			phalcon_array_update_str(return_value, SL(".phtml"), &php_engine, PH_COPY);
		} else {
			if (Z_TYPE(dependency_injector) != IS_OBJECT) {
				PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "A dependency injector container is required to obtain the application services");
				return;
			}

			array_init_size(&arguments, 2);
			PHALCON_MM_ADD_ENTRY(&arguments);
			phalcon_array_append(&arguments, getThis(), PH_COPY);
			phalcon_array_append(&arguments, &dependency_injector, PH_COPY);

			ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(registered_engines), str_key, engine_service) {
				zval extension = {}, engine_object = {}, exception_message = {};
				if (str_key) {
					ZVAL_STR(&extension, str_key);
					if (Z_TYPE_P(engine_service) == IS_OBJECT) {

						/**
						 * Engine can be a closure
						 */
						if (instanceof_function(Z_OBJCE_P(engine_service), zend_ce_closure)) {
							PHALCON_MM_CALL_USER_FUNC_ARRAY(&engine_object, engine_service, &arguments);
						} else {
							ZVAL_COPY_VALUE(&engine_object, engine_service);
						}
					} else {
						/**
						 * Engine can be a string representing a service in the DI
						 */
						if (Z_TYPE_P(engine_service) == IS_STRING) {
							PHALCON_MM_CALL_METHOD(&engine_object, &dependency_injector, "getshared", engine_service, &arguments);
							PHALCON_MM_ADD_ENTRY(&engine_object);
							PHALCON_MM_VERIFY_INTERFACE(&engine_object, phalcon_mvc_view_engineinterface_ce);
						} else {
							PHALCON_CONCAT_SV(&exception_message, "Invalid template engine registration for extension: ", &extension);
							PHALCON_MM_ADD_ENTRY(&exception_message);
							PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_view_exception_ce, &exception_message);
							return;
						}
					}
					phalcon_array_update(return_value, &extension, &engine_object, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
		}
		phalcon_update_property(getThis(), SL("_engines"), return_value);
	}
	RETURN_MM();
}

/**
 * Checks whether view exists on registered extensions and render it
 *
 * @param array $engines
 * @param string $viewPath
 * @param boolean $silence
 * @param boolean $mustClean
 */
PHP_METHOD(Phalcon_Mvc_View, _engineRender){

	zval *engines, *view_path, *silence, *must_clean, *absolute_path = NULL, debug_message = {}, render_level = {}, cache_level = {};
	zval cache_mode = {}, cache = {}, not_exists = {}, views_dir_paths = {}, base_path = {}, views_dir = {}, *path;
	zval key = {}, lifetime = {}, view_options = {}, cache_options = {}, cached_view = {};
	zval view_params = {}, *engine, event_name = {}, status = {}, exception_message = {};
	zend_string *str_key;

	phalcon_fetch_params(1, 4, 1, &engines, &view_path, &silence, &must_clean, &absolute_path);

	if (absolute_path == NULL) {
		absolute_path = &PHALCON_GLOBAL(z_false);
	}

	/**
	 * Start the cache if there is a cache level enabled
	 */
	phalcon_read_property(&cache_level, getThis(), SL("_cacheLevel"), PH_NOISY|PH_READONLY);

	if (zend_is_true(&cache_level)) {
		phalcon_read_property(&render_level, getThis(), SL("_currentRenderLevel"), PH_NOISY|PH_READONLY);
		phalcon_read_property(&cache_mode, getThis(), SL("_cacheMode"), PH_NOISY|PH_READONLY);

		if (PHALCON_IS_TRUE(&cache_mode)) {
			if (PHALCON_LE(&render_level, &cache_level)) {
				PHALCON_MM_CALL_METHOD(&cache, getThis(), "getcache");
				PHALCON_MM_ADD_ENTRY(&cache);
			}
		} else {
			if (PHALCON_GE(&render_level, &cache_level)) {
				PHALCON_MM_CALL_METHOD(&cache, getThis(), "getcache");
				PHALCON_MM_ADD_ENTRY(&cache);
			}
		}
	}

	ZVAL_TRUE(&not_exists);
	array_init(&views_dir_paths);
	PHALCON_MM_ADD_ENTRY(&views_dir_paths);
	if (zend_is_true(absolute_path)) {
		phalcon_array_append(&views_dir_paths, view_path, PH_COPY);
	} else {
		phalcon_read_property(&base_path, getThis(), SL("_basePath"), PH_NOISY|PH_READONLY);
		phalcon_read_property(&views_dir, getThis(), SL("_viewsDir"), PH_NOISY|PH_READONLY);

		if (Z_TYPE(base_path) == IS_ARRAY) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(base_path), path) {
				zval *dir;
				if (Z_TYPE(views_dir) == IS_ARRAY) {
					ZEND_HASH_FOREACH_VAL(Z_ARRVAL(views_dir), dir) {
						zval views_dir_path = {};
						PHALCON_CONCAT_VVV(&views_dir_path, path, dir, view_path);
						phalcon_array_append(&views_dir_paths, &views_dir_path, 0);
					} ZEND_HASH_FOREACH_END();
				} else {
					zval views_dir_path = {};
					PHALCON_CONCAT_VVV(&views_dir_path, path, &views_dir, view_path);
					phalcon_array_append(&views_dir_paths, &views_dir_path, 0);
				}
			} ZEND_HASH_FOREACH_END();
		} else {
			if (Z_TYPE(views_dir) == IS_ARRAY) {
				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(views_dir), path) {
					zval views_dir_path = {};
					PHALCON_CONCAT_VVV(&views_dir_path, &base_path, path, view_path);
					phalcon_array_append(&views_dir_paths, &views_dir_path, 0);
				} ZEND_HASH_FOREACH_END();
			} else {
				zval views_dir_path = {};
				PHALCON_CONCAT_VVV(&views_dir_path, &base_path, &views_dir, view_path);
				phalcon_array_append(&views_dir_paths, &views_dir_path, 0);
			}
		}
	}

	if (Z_TYPE(cache) == IS_OBJECT) {
		/**
		 * Check if the cache is started, the first time a cache is started we start the
		 * cache
		 */
		phalcon_read_property(&view_options, getThis(), SL("_options"), PH_READONLY);

		/**
		 * Check if the user has defined a different options to the default
		 */
		ZVAL_NULL(&lifetime);
		if (Z_TYPE(view_options) == IS_ARRAY) {
			if (phalcon_array_isset_fetch_str(&cache_options, &view_options, SL("cache"), PH_READONLY)) {
				if (Z_TYPE(cache_options) == IS_ARRAY) {
					if (phalcon_array_isset_str(&cache_options, SL("key"))) {
						phalcon_array_fetch_str(&key, &cache_options, SL("key"), PH_NOISY|PH_READONLY);
					}
					if (phalcon_array_isset_str(&cache_options, SL("lifetime"))) {
						phalcon_array_fetch_str(&lifetime, &cache_options, SL("lifetime"), PH_NOISY|PH_READONLY);
					}
				}
			}
		}

		/**
		 * If a cache key is not set we create one using a md5
		 */
		if (Z_TYPE(key) <= IS_NULL) {
			phalcon_md5(&key, view_path);
			PHALCON_MM_ADD_ENTRY(&key);
		}

		/**
		 * We start the cache using the key set
		 */
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_CONCAT_SV(&debug_message, "--Get view cache: ", &key);
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
		}

		PHALCON_MM_CALL_METHOD(&cached_view, &cache, "start", &key, &lifetime, &PHALCON_GLOBAL(z_true));
		PHALCON_MM_ADD_ENTRY(&cached_view);
		if (Z_TYPE(cached_view) != IS_NULL) {
			phalcon_update_property(getThis(), SL("_content"), &cached_view);
			RETURN_MM_NULL();
		}
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "Render View: ", view_path);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	phalcon_read_property(&view_params, getThis(), SL("_viewParams"), PH_NOISY|PH_READONLY);

	/**
	 * Views are rendered in each engine
	 */
	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(engines), str_key, engine) {
		zval extension = {};
		if (!str_key) {
			continue;
		}
		ZVAL_STR(&extension, str_key);
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(views_dir_paths), path) {
			zval view_engine_path = {};
			PHALCON_CONCAT_VV(&view_engine_path, path, &extension);
			PHALCON_MM_ADD_ENTRY(&view_engine_path);
			if (phalcon_file_exists(&view_engine_path) != SUCCESS) {
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					PHALCON_CONCAT_SV(&debug_message, "--Not Found: ", &view_engine_path);
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}
				continue;
			}

			phalcon_update_property(getThis(), SL("_activeRenderPath"), &view_engine_path);
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

			PHALCON_MM_CALL_METHOD(NULL, engine, "render", &view_engine_path, &view_params, must_clean);

			/**
			 * Call afterRenderView if there is a events manager available
			 */
			ZVAL_FALSE(&not_exists);

			PHALCON_MM_ZVAL_STRING(&event_name, "view:afterRenderView");
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
			break;
		} ZEND_HASH_FOREACH_END();

		if (!zend_is_true(&not_exists)) {
			break;
		}
	} ZEND_HASH_FOREACH_END();

	if (PHALCON_IS_TRUE(&not_exists)) {
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			ZVAL_STRING(&debug_message, "--Not Found View");
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
		}

		zval contents = {};
		phalcon_ob_get_contents(&contents);
		php_output_clean();
		PHALCON_MM_ADD_ENTRY(&contents);
		if (Z_TYPE(contents) == IS_STRING) {
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setcontent", &contents, &PHALCON_GLOBAL(z_true));
		}

		/**
		 * Notify about not found views
		 */
		PHALCON_MM_ZVAL_STRING(&event_name, "view:notFoundView");
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

		if (!zend_is_true(silence)) {
			PHALCON_CONCAT_SVS(&exception_message, "View '", view_path, "' was not found in the views directory");
			PHALCON_MM_ADD_ENTRY(&exception_message);
			PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_view_exception_ce, &exception_message);
			return;
		}
	}

	/**
	 * Store the data in the cache
	 */
	if (Z_TYPE(cache) == IS_OBJECT) {
		zval contents = {};
		PHALCON_MM_CALL_METHOD(&contents, getThis(), "getcontent");
		PHALCON_MM_ADD_ENTRY(&contents);
		PHALCON_MM_CALL_METHOD(NULL, &cache, "save", &PHALCON_GLOBAL(z_null), &contents);

		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			ZVAL_STRING(&debug_message, "--Save view cache");
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
		}
	}
	RETURN_MM();
}

/**
 * Register templating engines
 *
 *<code>
 *$this->view->registerEngines(array(
 *  ".phtml" => "Phalcon\Mvc\View\Engine\Php",
 *  ".volt" => "Phalcon\Mvc\View\Engine\Volt",
 *  ".mhtml" => "MyCustomEngine"
 *));
 *</code>
 *
 * @param array $engines
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, registerEngines){

	zval *engines;

	phalcon_fetch_params(0, 1, 0, &engines);

	if (Z_TYPE_P(engines) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "Engines to register must be an array");
		return;
	}
	phalcon_update_property(getThis(), SL("_registeredEngines"), engines);

	RETURN_THIS();
}

/**
 * Returns the registered templating engines
 *
 * @brief array Phalcon\Mvc\View::getRegisteredEngines()
 */
PHP_METHOD(Phalcon_Mvc_View, getRegisteredEngines) {

	RETURN_MEMBER(getThis(), "_registeredEngines")
}

/**
 * Returns the registered templating engines, if none is registered it will use Phalcon\Mvc\View\Engine\Php
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View, getEngines) {

	PHALCON_RETURN_CALL_METHOD(getThis(), "_loadtemplateengines");
}

/**
 * Checks whether a view file exists
 *
 * @param string $view
 * @param boolean $absolutePath
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_View, exists) {

	zval *view, *absolute_path = NULL, base_dir = {}, view_dir = {}, engines = {}, path = {};
	zend_string *str_key;
	int exists = 0;

	phalcon_fetch_params(1, 1, 1, &view, &absolute_path);
	PHALCON_ENSURE_IS_STRING(view);

	if (absolute_path && zend_is_true(absolute_path)) {
		ZVAL_COPY_VALUE(&path, view);
	} else {
		phalcon_read_property(&base_dir, getThis(), SL("_basePath"), PH_NOISY|PH_READONLY);
		phalcon_read_property(&view_dir, getThis(), SL("_viewsDir"), PH_NOISY|PH_READONLY);
		PHALCON_CONCAT_VVV(&path, &base_dir, &view_dir, view);
		PHALCON_MM_ADD_ENTRY(&path);
	}

	PHALCON_MM_CALL_METHOD(&engines, getThis(), "getengines");
	PHALCON_MM_ADD_ENTRY(&engines);
	ZEND_HASH_FOREACH_STR_KEY(Z_ARRVAL(engines), str_key) {
		zval ext = {}, filepath = {};
		if (str_key) {
			ZVAL_STR(&ext, str_key);
			PHALCON_CONCAT_VV(&filepath, &path, &ext);
			PHALCON_MM_ADD_ENTRY(&filepath);
			if (SUCCESS == phalcon_file_exists(&filepath)) {
				exists = 1;
				break;
			}
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_MM_BOOL(exists);
}

/**
 * Executes render process from dispatching data
 *
 *<code>
 * //Shows recent posts view (app/views/posts/recent.phtml)
 * $view->start()->render('posts', 'recent')->finish();
 *</code>
 *
 * @param string $controllerName
 * @param string $actionName
 * @param array $params
 * @param string $namespace_name
 * @param Phalcon\Mvc\View\ModelInterface $viewModel
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, render){

	zval *controller, *action, *params = NULL, *namespace = NULL, *view_model = NULL, ds = {}, namespace_separator = {}, disabled = {};
	zval controller_name = {}, action_name = {}, namespace_name = {};
	zval contents = {}, converter_key = {}, converter = {}, parameters = {}, lower_case = {}, lower_controller_name = {}, lower_action_name = {};
	zval layouts_dir = {}, enable_namespace_view = {}, ds_lower_namespace_name = {}, layout_namespace = {}, debug_message = {};
	zval layout = {}, layout_name = {}, engines = {}, pick_view = {}, render_view = {}, pick_view_action = {}, event_name = {}, status = {}, silence = {}, disabled_levels = {};
	zval render_level = {}, enable_layouts_absolute_path = {}, templates_before = {}, *tpl, view_tpl_path = {}, templates_after = {}, main_view = {};
	char slash[2] = {DEFAULT_SLASH, 0};

	phalcon_fetch_params(1, 2, 3, &controller, &action, &params, &namespace, &view_model);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_MM_ZVAL_COPY(&controller_name, controller);
	if (action) {
		PHALCON_MM_ZVAL_COPY(&action_name, action);
	}

	if (namespace) {
		PHALCON_MM_ZVAL_COPY(&namespace_name, namespace);
	}

	PHALCON_MM_ZVAL_STRING(&ds, slash);
	PHALCON_MM_ZVAL_STRING(&namespace_separator, "\\");

	phalcon_update_property(getThis(), SL("_currentRenderLevel"), &PHALCON_GLOBAL(z_zero));

	/**
	 * If the view is disabled we simply update the buffer from any output produced in
	 * the controller
	 */
	phalcon_read_property(&disabled, getThis(), SL("_disabled"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_NOT_FALSE(&disabled)) {
		phalcon_ob_get_contents(&contents);
		php_output_clean();
		PHALCON_MM_ADD_ENTRY(&contents);
		if (Z_TYPE(contents) == IS_STRING) {
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setcontent", &contents, &PHALCON_GLOBAL(z_true));
		}
		RETURN_MM_FALSE;
	}

	phalcon_update_property(getThis(), SL("_controllerName"), &controller_name);
	phalcon_update_property(getThis(), SL("_actionName"), &action_name);
	phalcon_update_property(getThis(), SL("_params"), params);
	phalcon_update_property(getThis(), SL("_namespaceName"), &namespace_name);

	PHALCON_MM_ZVAL_STRING(&converter_key, "controller");
	PHALCON_MM_CALL_SELF(&converter, "getconverter", &converter_key);
	PHALCON_MM_ADD_ENTRY(&converter);

	if (phalcon_is_callable(&converter)) {
		zval tmp = {};
		array_init_size(&parameters, 1);
		PHALCON_MM_ADD_ENTRY(&parameters);
		phalcon_array_append(&parameters, &controller_name, PH_COPY);

		PHALCON_MM_CALL_USER_FUNC_ARRAY(&tmp, &converter, &parameters);
		PHALCON_MM_ADD_ENTRY(&tmp);
		ZVAL_COPY_VALUE(&controller_name, &tmp);
	}

	PHALCON_MM_ZVAL_STRING(&converter_key, "action");
	PHALCON_MM_CALL_SELF(&converter, "getconverter", &converter_key);
	PHALCON_MM_ADD_ENTRY(&converter);

	if (phalcon_is_callable(&converter)) {
		zval tmp = {};

		array_init_size(&parameters, 1);
		PHALCON_MM_ADD_ENTRY(&parameters);
		phalcon_array_append(&parameters, &action_name, PH_COPY);
		PHALCON_MM_CALL_USER_FUNC_ARRAY(&tmp, &converter, &parameters);
		PHALCON_MM_ADD_ENTRY(&tmp);
		ZVAL_COPY_VALUE(&action_name, &tmp);
	}

	PHALCON_MM_ZVAL_STRING(&converter_key, "namespace");
	PHALCON_MM_CALL_SELF(&converter, "getconverter", &converter_key);

	if (phalcon_is_callable(&converter)) {
		zval tmp = {};

		array_init_size(&parameters, 1);
		PHALCON_MM_ADD_ENTRY(&parameters);
		phalcon_array_append(&parameters, &namespace_name, PH_COPY);
		PHALCON_MM_CALL_USER_FUNC_ARRAY(&tmp, &converter, &parameters);
		PHALCON_MM_ADD_ENTRY(&tmp);
		ZVAL_COPY_VALUE(&namespace_name, &tmp);
	}

	phalcon_read_property(&lower_case, getThis(), SL("_lowerCase"), PH_NOISY|PH_READONLY);

	if (zend_is_true(&lower_case)) {
		phalcon_fast_strtolower(&lower_controller_name, &controller_name);
		phalcon_fast_strtolower(&lower_action_name, &action_name);
		PHALCON_MM_ADD_ENTRY(&lower_controller_name);
		PHALCON_MM_ADD_ENTRY(&lower_action_name);
	} else {
		ZVAL_COPY_VALUE(&lower_controller_name, &controller_name);
		ZVAL_COPY_VALUE(&lower_action_name, &action_name);
	}

	/**
	 * Check if there is a layouts directory set
	 */
	phalcon_read_property(&layouts_dir, getThis(), SL("_layoutsDir"), PH_COPY);
	if (!zend_is_true(&layouts_dir)) {
		PHALCON_MM_ZVAL_STRING(&layouts_dir, "layouts/");
	}

	phalcon_read_property(&enable_namespace_view, getThis(), SL("_enableNamespaceView"), PH_NOISY|PH_READONLY);

	if (zend_is_true(&enable_namespace_view)) {
		zval lower_namespace_name = {};
		if (zend_is_true(&lower_case)) {
			phalcon_fast_strtolower(&lower_namespace_name, &namespace_name);
			PHALCON_MM_ADD_ENTRY(&lower_namespace_name);
		} else {
			ZVAL_COPY_VALUE(&lower_namespace_name, &namespace_name);
		}

		PHALCON_STR_REPLACE(&ds_lower_namespace_name, &namespace_separator, &ds, &lower_namespace_name);
		PHALCON_MM_ADD_ENTRY(&ds_lower_namespace_name);
		PHALCON_CONCAT_SV(&layout_namespace, "namespace/", &ds_lower_namespace_name);
		PHALCON_MM_ADD_ENTRY(&layout_namespace);
	}

	/**
	 * Check if the user has defined a custom layout
	 */
	phalcon_read_property(&layout, getThis(), SL("_layout"), PH_NOISY|PH_READONLY);
	if (zend_is_true(&layout)) {
		ZVAL_COPY_VALUE(&layout_name, &layout);
	} else if (PHALCON_IS_NOT_EMPTY(&ds_lower_namespace_name)) {
		PHALCON_CONCAT_VSV(&layout_name, &ds_lower_namespace_name, "/", &lower_controller_name);
		PHALCON_MM_ADD_ENTRY(&layout_name);
	} else {
		ZVAL_COPY_VALUE(&layout_name, &lower_controller_name);
	}

	/**
	 * Load the template engines
	 */
	PHALCON_MM_CALL_METHOD(&engines, getThis(), "_loadtemplateengines");
	PHALCON_MM_ADD_ENTRY(&engines);

	/**
	 * Check if the user has picked a view diferent than the automatic
	 */
	phalcon_read_property(&pick_view, getThis(), SL("_pickView"), PH_READONLY);
	if (Z_TYPE(pick_view) == IS_NULL) {
		if (PHALCON_IS_NOT_EMPTY(&ds_lower_namespace_name)) {
			PHALCON_CONCAT_VSVSV(&render_view, &ds_lower_namespace_name, "/", &lower_controller_name, "/", &lower_action_name);
		} else {
			PHALCON_CONCAT_VSV(&render_view, &lower_controller_name, "/", &lower_action_name);
		}
		PHALCON_MM_ADD_ENTRY(&render_view);
	} else {
		/**
		 * The 'picked' view is an array, where the first element is controller and the
		 * second the action
		 */
		phalcon_array_fetch_long(&render_view, &pick_view, 0, PH_NOISY|PH_COPY);
		if (phalcon_array_isset_fetch_long(&pick_view_action, &pick_view, 1, PH_READONLY)) {
			ZVAL_COPY_VALUE(&layout_name, &pick_view_action);
		}
	}

	/**
	 * Call beforeRender if there is an events manager
	 */
	PHALCON_MM_ZVAL_STRING(&event_name, "view:beforeRender");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}
	zval_ptr_dtor(&status);

	/**
	 * Get the current content in the buffer maybe some output from the controller
	 */
	phalcon_ob_get_contents(&contents);
	php_output_clean();
	PHALCON_MM_ADD_ENTRY(&contents);
	if (Z_TYPE(contents) == IS_STRING) {
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setcontent", &contents, &PHALCON_GLOBAL(z_true));
	}
	ZVAL_TRUE(&silence);

	/**
	 * Disabled levels allow to avoid an specific level of rendering
	 */
	phalcon_read_property(&disabled_levels, getThis(), SL("_disabledLevels"), PH_NOISY|PH_READONLY);

	/**
	 * Render level will tell use when to stop
	 */
	phalcon_read_property(&render_level, getThis(), SL("_renderLevel"), PH_NOISY|PH_READONLY);
	if (zend_is_true(&render_level)) {
		phalcon_read_property(&enable_layouts_absolute_path, getThis(), SL("_enableLayoutsAbsolutePath"), PH_READONLY);

		if (view_model && Z_TYPE_P(view_model) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(view_model), phalcon_mvc_view_modelinterface_ce, 1)) {
			zval model_content = {};
			PHALCON_MM_CALL_METHOD(NULL, view_model, "setview", getThis());
			PHALCON_MM_CALL_METHOD(&model_content, view_model, "render");
			phalcon_update_property(getThis(), SL("_content"), &model_content);
			zval_ptr_dtor(&model_content);
		}

		if (PHALCON_GE_LONG(&render_level, PHALCON_VIEW_LEVEL_ACTION)) {
			/**
			 * Inserts view related to action
			 */
			if (!phalcon_array_isset_long(&disabled_levels, PHALCON_VIEW_LEVEL_ACTION)) {
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "Ready insert action view");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}
				phalcon_update_property_long(getThis(), SL("_currentRenderLevel"), PHALCON_VIEW_LEVEL_ACTION);
				PHALCON_MM_CALL_METHOD(NULL, getThis(), "_enginerender", &engines, &render_view, &silence, &PHALCON_GLOBAL(z_true));
			}
		}

		/**
		 * Inserts templates before layout
		 */
		if (PHALCON_GE_LONG(&render_level, PHALCON_VIEW_LEVEL_BEFORE_TEMPLATE)) {
			if (!phalcon_array_isset_long(&disabled_levels, PHALCON_VIEW_LEVEL_BEFORE_TEMPLATE)) {
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "Ready insert templates before layout");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}
				phalcon_update_property_long(getThis(), SL("_currentRenderLevel"), PHALCON_VIEW_LEVEL_BEFORE_TEMPLATE);

				phalcon_read_property(&templates_before, getThis(), SL("_templatesBefore"), PH_NOISY|PH_READONLY);

				/**
				 * Templates before must be an array
				 */
				if (PHALCON_IS_NOT_EMPTY_ARR(&templates_before)) {

					ZVAL_FALSE(&silence);

					ZEND_HASH_FOREACH_VAL(Z_ARRVAL(templates_before), tpl) {
						PHALCON_CONCAT_VV(&view_tpl_path, &layouts_dir, tpl);
						PHALCON_MM_ADD_ENTRY(&view_tpl_path);
						PHALCON_MM_CALL_METHOD(NULL, getThis(), "_enginerender", &engines, &view_tpl_path, &silence, &PHALCON_GLOBAL(z_true), &enable_layouts_absolute_path);
					} ZEND_HASH_FOREACH_END();

					ZVAL_TRUE(&silence);
				} else if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "--Not set");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}
			}
		}

		/**
		 * Inserts controller layout
		 */
		if (PHALCON_GE_LONG(&render_level, PHALCON_VIEW_LEVEL_CONTROLLER)) {
			if (!phalcon_array_isset_long(&disabled_levels, PHALCON_VIEW_LEVEL_CONTROLLER)) {
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "Ready insert controller layout");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}
				phalcon_update_property_long(getThis(), SL("_currentRenderLevel"), PHALCON_VIEW_LEVEL_CONTROLLER);

				PHALCON_CONCAT_VV(&view_tpl_path, &layouts_dir, &layout_name);
				PHALCON_MM_ADD_ENTRY(&view_tpl_path);
				PHALCON_MM_CALL_METHOD(NULL, getThis(), "_enginerender", &engines, &view_tpl_path, &silence, &PHALCON_GLOBAL(z_true), &enable_layouts_absolute_path);
			}
		}

		/**
		 * Inserts namespace layout
		 */
		if (PHALCON_GE_LONG(&render_level, PHALCON_VIEW_LEVEL_NAMESPACE) && PHALCON_IS_NOT_EMPTY(&layout_namespace)) {
			if (!phalcon_array_isset_long(&disabled_levels, PHALCON_VIEW_LEVEL_NAMESPACE)) {
				zval enable_multi_namespace_view = {}, pos = {};
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "Ready insert namespace layout");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}
				phalcon_update_property_long(getThis(), SL("_currentRenderLevel"), PHALCON_VIEW_LEVEL_NAMESPACE);

				PHALCON_CONCAT_VV(&view_tpl_path, &layouts_dir, &layout_namespace);
				PHALCON_MM_ADD_ENTRY(&view_tpl_path);
				PHALCON_MM_CALL_METHOD(NULL, getThis(), "_enginerender", &engines, &view_tpl_path, &silence, &PHALCON_GLOBAL(z_true), &enable_layouts_absolute_path);

				phalcon_read_property(&enable_multi_namespace_view, getThis(), SL("_enableMultiNamespaceView"), PH_NOISY|PH_READONLY);
				if (zend_is_true(&enable_multi_namespace_view)) {
					/**
					 * Top-level namespace
					 */
					while (phalcon_fast_strrpos(&pos, &layout_namespace, &ds)) {
						zval tmp = {};

						phalcon_substr(&tmp, &layout_namespace, 0, Z_LVAL(pos));
						PHALCON_MM_ADD_ENTRY(&tmp);
						ZVAL_COPY_VALUE(&layout_namespace, &tmp);

						PHALCON_CONCAT_VV(&view_tpl_path, &layouts_dir, &layout_namespace);
						PHALCON_MM_ADD_ENTRY(&view_tpl_path);
						PHALCON_CALL_METHOD(NULL, getThis(), "_enginerender", &engines, &view_tpl_path, &silence, &PHALCON_GLOBAL(z_true), &enable_layouts_absolute_path);
					}
				}
			}
		}

		/**
		 * Inserts templates after layout
		 */
		if (PHALCON_GE_LONG(&render_level, PHALCON_VIEW_LEVEL_AFTER_TEMPLATE)) {
			if (!phalcon_array_isset_long(&disabled_levels, PHALCON_VIEW_LEVEL_AFTER_TEMPLATE)) {
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "Ready inserts templates after layout");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}
				phalcon_update_property_long(getThis(), SL("_currentRenderLevel"), PHALCON_VIEW_LEVEL_AFTER_TEMPLATE);

				/**
				 * Templates after must be an array
				 */
				phalcon_read_property(&templates_after, getThis(), SL("_templatesAfter"), PH_NOISY|PH_READONLY);
				if (PHALCON_IS_NOT_EMPTY_ARR(&templates_after)) {
					ZVAL_FALSE(&silence);

					ZEND_HASH_FOREACH_VAL(Z_ARRVAL(templates_after), tpl) {
						PHALCON_CONCAT_VV(&view_tpl_path, &layouts_dir, tpl);
						PHALCON_MM_ADD_ENTRY(&view_tpl_path);
						PHALCON_MM_CALL_METHOD(NULL, getThis(), "_enginerender", &engines, &view_tpl_path, &silence, &PHALCON_GLOBAL(z_true), &enable_layouts_absolute_path);
					} ZEND_HASH_FOREACH_END();

					ZVAL_TRUE(&silence);
				} else if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "--Not set");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}
			}
		}

		/**
		 * Inserts main view
		 */
		if (PHALCON_GE_LONG(&render_level, PHALCON_VIEW_LEVEL_MAIN)) {
			if (!phalcon_array_isset_long(&disabled_levels, PHALCON_VIEW_LEVEL_MAIN)) {
				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					ZVAL_STRING(&debug_message, "Ready insert main view");
					PHALCON_DEBUG_LOG(&debug_message);
					zval_ptr_dtor(&debug_message);
				}
				phalcon_update_property_long(getThis(), SL("_currentRenderLevel"), PHALCON_VIEW_LEVEL_MAIN);

				phalcon_read_property(&main_view, getThis(), SL("_mainView"), PH_NOISY|PH_READONLY);
				PHALCON_MM_CALL_METHOD(NULL, getThis(), "_enginerender", &engines, &main_view, &silence, &PHALCON_GLOBAL(z_true));
			}
		}

		phalcon_update_property(getThis(), SL("_currentRenderLevel"), &PHALCON_GLOBAL(z_zero));
	}

	/**
	 * Call afterRender event
	 */
	PHALCON_MM_ZVAL_STRING(&event_name, "view:afterRender");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

	RETURN_MM_THIS();
}

/**
 * Choose a different view to render instead of last-controller/last-action
 *
 * <code>
 * class ProductsController extends Phalcon\Mvc\Controller
 * {
 *
 *    public function saveAction()
 *    {
 *
 *         //Do some save stuff...
 *
 *         //Then show the list view
 *         $this->view->pick("products/list");
 *    }
 * }
 * </code>
 *
 * @param string|array $renderView
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, pick){

	zval *render_view, pick_view = {}, parts = {}, layout = {};

	phalcon_fetch_params(1, 1, 0, &render_view);

	if (Z_TYPE_P(render_view) == IS_ARRAY) {
		ZVAL_COPY_VALUE(&pick_view, render_view);
	} else {
		array_init_size(&pick_view, 2);
		PHALCON_MM_ADD_ENTRY(&pick_view);
		phalcon_array_append(&pick_view, render_view, PH_COPY);

		if (phalcon_memnstr_str(render_view, SL("/"))) {
			phalcon_fast_explode_str(&parts, SL("/"), render_view);
			phalcon_array_fetch_long(&layout, &parts, 0, PH_NOISY|PH_READONLY);
			phalcon_array_append(&pick_view, &layout, PH_COPY);
			zval_ptr_dtor(&parts);
		}
	}
	phalcon_update_property(getThis(), SL("_pickView"), &pick_view);

	RETURN_MM_THIS();
}

/**
 * Renders a partial view
 *
 * <code>
 * 	//Show a partial inside another view
 * 	$this->partial('shared/footer');
 * </code>
 *
 * <code>
 * 	//Show a partial inside another view with parameters
 * 	$this->partial('shared/footer', array('content' => $html));
 * </code>
 *
 * @param string $partialPath
 * @param array $params
 * @param boolean $autorender
 */
PHP_METHOD(Phalcon_Mvc_View, partial){

	zval *partial_path, *params = NULL, *autorender = NULL, view_params = {}, new_params = {}, partials_dir = {}, enable_partials_absolute_path = {};
	zval real_path = {}, engines = {};

	phalcon_fetch_params(1, 1, 2, &partial_path, &params, &autorender);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	if (!autorender) {
		autorender = &PHALCON_GLOBAL(z_true);
	}

	/**
	 * If the developer pass an array of variables we create a new virtual symbol table
	 */
	if (Z_TYPE_P(params) == IS_ARRAY) {
		phalcon_read_property(&view_params, getThis(), SL("_viewParams"), PH_NOISY|PH_COPY);

		/**
		 * Merge or assign the new params as parameters
		 */
		if (Z_TYPE(view_params) == IS_ARRAY) {
			phalcon_fast_array_merge(&new_params, &view_params, params);
			PHALCON_MM_ADD_ENTRY(&new_params);
		} else {
			ZVAL_COPY_VALUE(&new_params, params);
		}

		/**
		 * Update the parameters with the new ones
		 */
		phalcon_update_property(getThis(), SL("_viewParams"), &new_params);
	}

	phalcon_read_property(&partials_dir, getThis(), SL("_partialsDir"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&enable_partials_absolute_path, getThis(), SL("_enablePartialsAbsolutePath"), PH_NOISY|PH_READONLY);

	/**
	 * Partials are looked up under the partials directory
	 */
	PHALCON_CONCAT_VV(&real_path, &partials_dir, partial_path);
	PHALCON_MM_ADD_ENTRY(&real_path);

	/**
	 * We need to check if the engines are loaded first, this method could be called
	 * outside of 'render'
	 */
	PHALCON_MM_CALL_METHOD(&engines, getThis(), "_loadtemplateengines");
	PHALCON_MM_ADD_ENTRY(&engines);

	/**
	 * Call engine render, this checks in every registered engine for the partial
	 */
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "_enginerender", &engines, &real_path, &PHALCON_GLOBAL(z_false), &PHALCON_GLOBAL(z_false), &enable_partials_absolute_path);

	/**
	 * Now we need to restore the original view parameters
	 */
	if (Z_TYPE_P(params) == IS_ARRAY) {
		/**
		 * Restore the original view params
		 */
		phalcon_update_property(getThis(), SL("_viewParams"), &view_params);
	}

	if (!PHALCON_IS_TRUE(autorender)) {
		phalcon_ob_get_contents(return_value);
		phalcon_ob_clean();
	}
	RETURN_MM();
}

/**
 * Perform the automatic rendering returning the output as a string
 *
 * <code>
 * 	$template = $this->view->getRender('products', 'show', array('products' => $products));
 * </code>
 *
 * @param string $controllerName
 * @param string $actionName
 * @param array $params
 * @param mixed $configCallback
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getRender){

	zval *controller_name, *action_name, *params = NULL, *config_callback = NULL, view = {}, params_tmp = {};

	phalcon_fetch_params(1, 2, 2, &controller_name, &action_name, &params, &config_callback);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	if (!config_callback) {
		config_callback = &PHALCON_GLOBAL(z_null);
	}

	/**
	 * We must to clone the current view to keep the old state
	 */
	if (phalcon_clone(&view, getThis()) == FAILURE) {
		RETURN_MM();
	}

	PHALCON_MM_ADD_ENTRY(&view);

	/**
	 * The component must be reset to its defaults
	 */
	PHALCON_MM_CALL_METHOD(NULL, &view, "reset");

	/**
	 * Set the render variables
	 */
	if (Z_TYPE_P(params) == IS_ARRAY) {
		PHALCON_MM_CALL_METHOD(NULL, &view, "setvars", params);
	}

	/**
	 * Perform extra configurations over the cloned object
	 */
	if (Z_TYPE_P(config_callback) == IS_OBJECT) {
		array_init_size(&params_tmp, 1);
		PHALCON_MM_ADD_ENTRY(&params_tmp);
		phalcon_array_append(&params_tmp, &view, PH_COPY);

		PHALCON_MM_CALL_USER_FUNC_ARRAY(NULL, config_callback, &params_tmp);
	}

	/**
	 * Start the output buffering
	 */
	PHALCON_MM_CALL_METHOD(NULL, &view, "start");

	/**
	 * Perform the render passing only the controller and action
	 */
	PHALCON_MM_CALL_METHOD(NULL, &view, "render", controller_name, action_name);

	/**
	 * Stop the output buffering
	 */
	phalcon_ob_end_clean();

	/**
	 * Get the processed content
	 */
	PHALCON_MM_RETURN_CALL_METHOD(&view, "getcontent");
	RETURN_MM();
}

/**
 * Finishes the render process by stopping the output buffering
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, finish){

	phalcon_ob_end_clean();
	RETURN_THIS();
}

/**
 * Create a Phalcon\Cache based on the internal cache options
 *
 * @return Phalcon\Cache\BackendInterface
 */
PHP_METHOD(Phalcon_Mvc_View, _createCache){

	zval dependency_injector = {}, view_options = {}, cache_options = {}, cache_service = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "A dependency injector container is required to obtain the view cache services");
		return;
	}

	PHALCON_MM_ZVAL_STRING(&cache_service, "viewCache");

	phalcon_read_property(&view_options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(view_options) == IS_ARRAY) {
		if (phalcon_array_isset_fetch_str(&cache_options, &view_options, SL("cache"), PH_READONLY)) {
			if (Z_TYPE(cache_options) == IS_ARRAY && phalcon_array_isset_str(&cache_options, SL("service"))) {
				phalcon_array_fetch_str(&cache_service, &cache_options, SL("service"), PH_NOISY|PH_READONLY);
			}
		}
	}

	/**
	 * The injected service must be an object
	 */
	PHALCON_MM_CALL_METHOD(return_value, &dependency_injector, "getshared", &cache_service);
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "The injected caching service is invalid");
		return;
	}

	PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_cache_backendinterface_ce);
	RETURN_MM();
}

/**
 * Check if the component is currently caching the output content
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_View, isCaching){

	zval cache_level = {};

	phalcon_read_property(&cache_level, getThis(), SL("_cacheLevel"), PH_NOISY|PH_READONLY);
	is_smaller_function(return_value, &PHALCON_GLOBAL(z_zero), &cache_level);
}

/**
 * Returns the cache instance used to cache
 *
 * @return Phalcon\Cache\BackendInterface
 */
PHP_METHOD(Phalcon_Mvc_View, getCache){

	zval cache = {};

	PHALCON_MM_INIT();
	phalcon_read_property(&cache, getThis(), SL("_cache"), PH_READONLY);
	if (zend_is_true(&cache)) {
		if (Z_TYPE(cache) != IS_OBJECT) {
			PHALCON_CALL_METHOD(&cache, getThis(), "_createcache");
			PHALCON_MM_ADD_ENTRY(&cache);
			phalcon_update_property(getThis(), SL("_cache"), &cache);

		}
	} else {
		PHALCON_MM_CALL_METHOD(&cache, getThis(), "_createcache");
		PHALCON_MM_ADD_ENTRY(&cache);
		phalcon_update_property(getThis(), SL("_cache"), &cache);
	}

	RETURN_MM_CTOR(&cache);
}

/**
 * Cache the actual view render to certain level
 *
 *<code>
 *  $this->view->cache(array('key' => 'my-key', 'lifetime' => 86400));
 *</code>
 *
 * @param boolean|array $options
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, cache){

	zval *options = NULL, view_options = {}, cache_options = {}, *value, cache_level = {}, cache_mode = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 0, 1, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_true);
	}

	if (Z_TYPE_P(options) == IS_ARRAY) {
		phalcon_read_property(&view_options, getThis(), SL("_options"), PH_READONLY);
		if (Z_TYPE(view_options) != IS_ARRAY) {
			array_init(&view_options);
			PHALCON_MM_ADD_ENTRY(&view_options);
		}

		/**
		 * Get the default cache options
		 */
		if (!phalcon_array_isset_fetch_str(&cache_options, &view_options, SL("cache"), PH_READONLY)) {
			array_init(&cache_options);
			PHALCON_MM_ADD_ENTRY(&cache_options);
		}

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(options), idx, str_key, value) {
			if (str_key) {
				phalcon_array_update_string(&cache_options, str_key, value, PH_COPY);
			} else {
				phalcon_array_update_long(&cache_options, idx, value, PH_COPY);
			}

		} ZEND_HASH_FOREACH_END();

		/**
		 * Check if the user has defined a default cache level or use 5 as default
		 */
		if (phalcon_array_isset_fetch_str(&cache_level, &cache_options, SL("level"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_cacheLevel"), &cache_level);
		} else {
			phalcon_update_property_long(getThis(), SL("_cacheLevel"), 5);
		}

		if (phalcon_array_isset_fetch_str(&cache_mode, &cache_options, SL("mode"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_cacheMode"), &cache_mode);
		} else {
			phalcon_update_property_bool(getThis(), SL("_cacheMode"), 0);
		}

		phalcon_array_update_str(&view_options, SL("cache"), &cache_options, PH_COPY);
		phalcon_update_property(getThis(), SL("_options"), &view_options);
	} else {
		/**
		 * If 'options' isn't an array we enable the cache with the default options
		 */
		if (zend_is_true(options)) {
			phalcon_update_property_long(getThis(), SL("_cacheLevel"), 5);
		} else {
			phalcon_update_property_long(getThis(), SL("_cacheLevel"), 0);
		}

		phalcon_update_property_bool(getThis(), SL("_cacheMode"), 0);
	}

	RETURN_MM_THIS();
}

/**
 * Externally sets the view content
 *
 *<code>
 *	$this->view->setContent("<h1>hello</h1>");
 *</code>
 *
 * @param string $content
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setContent){

	zval *content, *append = NULL;

	phalcon_fetch_params(0, 1, 1, &content, &append);

	if (Z_TYPE_P(content) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "Content must be a string");
		return;
	}

	if (append && Z_TYPE_P(append) == IS_TRUE) {
		zval old_content = {}, new_content = {};
		phalcon_read_property(&old_content, getThis(), SL("_content"), PH_NOISY|PH_READONLY);
		PHALCON_CONCAT_VV(&new_content, &old_content, content);
		phalcon_update_property(getThis(), SL("_content"), &new_content);
		zval_ptr_dtor(&new_content);
	} else {
		phalcon_update_property(getThis(), SL("_content"), content);
	}

	RETURN_THIS();
}

/**
 * Returns cached output from another view stage
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getContent){


	RETURN_MEMBER(getThis(), "_content");
}

/**
 * Start a new section block
 *
 * @param string $name
 */
PHP_METHOD(Phalcon_Mvc_View, startSection){

	zval *name;

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_update_property_array(getThis(), SL("_sections"), name, &PHALCON_GLOBAL(z_null));
	phalcon_ob_start();
	RETURN_THIS();
}

/**
 * Stop the current section block
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, stopSection){

	zval content = {}, sections = {}, name = {};
	HashTable *array;

	phalcon_ob_get_clean(&content);

	phalcon_read_property(&sections, getThis(), SL("_sections"), PH_NOISY|PH_READONLY);

	array = Z_ARRVAL(sections);

	zend_hash_internal_pointer_end(array);
	zend_hash_get_current_key_zval(array, &name);

	phalcon_update_property_array(getThis(), SL("_sections"), &name, &content);
	zval_ptr_dtor(&name);
	zval_ptr_dtor(&content);
	RETURN_THIS();
}

/**
 * Returns the content for a section block
 *
 * @param string $name
 * @param string $default
 * @return string|null
 */
PHP_METHOD(Phalcon_Mvc_View, section){

	zval *name, *default_value = NULL;

	phalcon_fetch_params(0, 1, 1, &name, &default_value);

	if (phalcon_isset_property_array(getThis(), SL("_sections"), name)) {
		phalcon_read_property_array(return_value, getThis(), SL("_sections"), name, PH_COPY);
	} else if (default_value) {
		RETURN_CTOR(default_value);
	} else {
		RETURN_NULL();
	}
}

/**
 * Returns the path of the view that is currently rendered
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getActiveRenderPath){


	RETURN_MEMBER(getThis(), "_activeRenderPath");
}

/**
 * Disables the auto-rendering process
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, disable){


	phalcon_update_property_bool(getThis(), SL("_disabled"), 1);
	RETURN_THIS();
}

/**
 * Enables the auto-rendering process
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, enable){


	phalcon_update_property_bool(getThis(), SL("_disabled"), 0);
	RETURN_THIS();
}

/**
 * Whether automatic rendering is enabled
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_View, isDisabled){

	RETURN_MEMBER(getThis(), "_disabled");
}

/**
 * Enables namespace view render
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, enableNamespaceView){

	phalcon_update_property_bool(getThis(), SL("_enableNamespaceView"), 1);
	RETURN_THIS();
}

/**
 * Disables namespace view render
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, disableNamespaceView){

	phalcon_update_property_bool(getThis(), SL("_enableNamespaceView"), 0);
	RETURN_THIS();
}

/**
 * Enables multi namespace view render
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, enableMultiNamespaceView){

	phalcon_update_property_bool(getThis(), SL("_enableMultiNamespaceView"), 1);
	RETURN_THIS();
}

/**
 * Disables multi namespace view render
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, disableMultiNamespaceView){

	phalcon_update_property_bool(getThis(), SL("_enableMultiNamespaceView"), 0);
	RETURN_THIS();
}

/**
 * Enables to lower case view path
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, enableLowerCase){

	phalcon_update_property_bool(getThis(), SL("_lowerCase"), 1);
	RETURN_THIS();
}

/**
 * Whether to lower case view path
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, disableLowerCase){

	phalcon_update_property_bool(getThis(), SL("_lowerCase"), 0);
	RETURN_THIS();
}

/**
 * Adds a converter
 *
 * @param string $name
 * @param callable $converter
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setConverter){

	zval *name, *converter;

	phalcon_fetch_params(0, 2, 0, &name, &converter);

	if (!phalcon_is_callable(converter)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "The paramter `converter` is not callable");
		return;
	}

	phalcon_update_property_array(getThis(), SL("_converters"), name, converter);
	RETURN_THIS();
}

/**
 * Returns the router converter
 *
 * @return callable|null
 */
PHP_METHOD(Phalcon_Mvc_View, getConverter) {

	zval *name, converters = {}, converter = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_property(&converters, getThis(), SL("_converters"), PH_NOISY|PH_READONLY);

	if (phalcon_array_isset_fetch(&converter, &converters, name, PH_READONLY)) {
		RETURN_CTOR(&converter);
	}

	RETURN_NULL();
}

/**
 * Resets the view component to its factory default values
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, reset){

	phalcon_update_property(getThis(), SL("_disabled"), &PHALCON_GLOBAL(z_false));
	phalcon_update_property(getThis(), SL("_engines"), &PHALCON_GLOBAL(z_false));
	phalcon_update_property(getThis(), SL("_cache"), &PHALCON_GLOBAL(z_null));
	phalcon_update_property_long(getThis(), SL("_renderLevel"), 5);
	phalcon_update_property(getThis(), SL("_cacheLevel"), &PHALCON_GLOBAL(z_zero));
	phalcon_update_property(getThis(), SL("_content"), &PHALCON_GLOBAL(z_null));
	phalcon_update_property_empty_array(getThis(), SL("_sections"));
	phalcon_update_property(getThis(), SL("_templatesBefore"), &PHALCON_GLOBAL(z_null));
	phalcon_update_property(getThis(), SL("_templatesAfter"), &PHALCON_GLOBAL(z_null));
	RETURN_THIS();
}

/**
 * Magic method to pass variables to the views
 *
 *<code>
 *	$this->view->products = $products;
 *</code>
 *
 * @param string $key
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_View, __set){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(getThis(), SL("_viewParams"), key, value);

}

/**
 * Magic method to retrieve a variable passed to the view
 *
 *<code>
 *	echo $this->view->products;
 *</code>
 *
 * @param string $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_View, __get){

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
 *<code>
 *	isset($this->view->products)
 *</code>
 *
 * @param string $key
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_View, __isset){

	zval *key, params = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&params, getThis(), SL("_viewParams"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset(&params, key)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
