
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

#include "mvc/view/engineinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_mvc_view_engineinterface_ce;

static const zend_function_entry phalcon_mvc_view_engineinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_Mvc_View_EngineInterface, getContent, arginfo_empty)
	PHP_ABSTRACT_ME(Phalcon_Mvc_View_EngineInterface, startSection, arginfo_phalcon_mvc_view_engineinterface_startsection)
	PHP_ABSTRACT_ME(Phalcon_Mvc_View_EngineInterface, stopSection, arginfo_empty)
	PHP_ABSTRACT_ME(Phalcon_Mvc_View_EngineInterface, section, arginfo_phalcon_mvc_view_engineinterface_section)
	PHP_ABSTRACT_ME(Phalcon_Mvc_View_EngineInterface, partial, arginfo_phalcon_mvc_view_engineinterface_partial)
	PHP_ABSTRACT_ME(Phalcon_Mvc_View_EngineInterface, render, arginfo_phalcon_mvc_view_engineinterface_render)
	PHP_ABSTRACT_ME(Phalcon_Mvc_View_EngineInterface, addMethod, arginfo_phalcon_mvc_view_engineinterface_addmethod)
	PHP_ABSTRACT_ME(Phalcon_Mvc_View_EngineInterface, __call, arginfo_phalcon_mvc_view_engineinterface___call)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\View\EngineInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_View_EngineInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon\\Mvc\\View, EngineInterface, mvc_view_engineinterface, phalcon_mvc_view_engineinterface_method_entry);

	return SUCCESS;
}

/**
 * Returns cached ouput on another view stage
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_View_EngineInterface, getContent);

/**
 * Start a new section block
 *
 * @param string $name
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_View_EngineInterface, startSection);

/**
 * Stop the current section block
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_View_EngineInterface, stopSection);

/**
 * Returns the content for a section block
 *
 * @param string $name
 * @param string $default
 * @return string|null
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_View_EngineInterface, section);

/**
 * Renders a partial inside another view
 *
 * @param string $partialPath
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_View_EngineInterface, partial);

/**
 * Renders a view using the template engine
 *
 * @param string $path
 * @param array $params
 * @param boolean $mustClean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_View_EngineInterface, render);

/**
 * Adds a user-defined method
 *
 * @param string $name
 * @param callable $handler
 * @return Phalcon\Mvc\View\EngineInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_View_EngineInterface, addMethod);

/**
 * Handles method calls when a method is not implemented
 *
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_View_EngineInterface, __call);
