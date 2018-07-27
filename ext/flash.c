
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

#include "flash.h"
#include "flashinterface.h"
#include "flash/exception.h"
#include "di/injectable.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/concat.h"

/**
 * Phalcon\Flash
 *
 * Shows HTML notifications related to different circumstances. Classes can be stylized using CSS
 *
 *<code>
 * $flash->success("The record was successfully deleted");
 * $flash->error("Cannot open the file");
 *</code>
 */
zend_class_entry *phalcon_flash_ce;

PHP_METHOD(Phalcon_Flash, __construct);
PHP_METHOD(Phalcon_Flash, setImplicitFlush);
PHP_METHOD(Phalcon_Flash, setAutomaticHtml);
PHP_METHOD(Phalcon_Flash, setCssClasses);
PHP_METHOD(Phalcon_Flash, error);
PHP_METHOD(Phalcon_Flash, notice);
PHP_METHOD(Phalcon_Flash, success);
PHP_METHOD(Phalcon_Flash, warning);
PHP_METHOD(Phalcon_Flash, outputMessage);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_flash___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, cssClasses, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_flash_setimplicitflush, 0, 0, 1)
	ZEND_ARG_INFO(0, implicitFlush)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_flash_setautomatichtml, 0, 0, 1)
	ZEND_ARG_INFO(0, automaticHtml)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_flash_setcssclasses, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, cssClasses, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_flash_method_entry[] = {
	PHP_ME(Phalcon_Flash, __construct, arginfo_phalcon_flash___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Flash, setImplicitFlush, arginfo_phalcon_flash_setimplicitflush, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Flash, setAutomaticHtml, arginfo_phalcon_flash_setautomatichtml, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Flash, setCssClasses, arginfo_phalcon_flash_setcssclasses, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Flash, error, arginfo_phalcon_flashinterface_error, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Flash, notice, arginfo_phalcon_flashinterface_notice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Flash, success, arginfo_phalcon_flashinterface_success, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Flash, warning, arginfo_phalcon_flashinterface_warning, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Flash, outputMessage, arginfo_phalcon_flashinterface_outputmessage, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Flash initializer
 */
PHALCON_INIT_CLASS(Phalcon_Flash){

	PHALCON_REGISTER_CLASS_EX(Phalcon, Flash, flash, phalcon_di_injectable_ce, phalcon_flash_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_flash_ce, SL("_cssClasses"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_flash_ce, SL("_implicitFlush"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_flash_ce, SL("_automaticHtml"), 1, ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Flash constructor
 *
 * @param array $cssClasses
 */
PHP_METHOD(Phalcon_Flash, __construct){

	zval *_css_classes = NULL, css_classes = {};

	phalcon_fetch_params(0, 0, 1, &_css_classes);

	if (!_css_classes || Z_TYPE_P(_css_classes) == IS_NULL) {
		array_init_size(&css_classes, 4);
		phalcon_array_update_str_str(&css_classes, SL("error"), SL("errorMessage"), 0);
		phalcon_array_update_str_str(&css_classes, SL("notice"), SL("noticeMessage"), 0);
		phalcon_array_update_str_str(&css_classes, SL("success"), SL("successMessage"), 0);
		phalcon_array_update_str_str(&css_classes, SL("warning"), SL("warningMessage"), 0);
	} else {
		ZVAL_COPY_VALUE(&css_classes, _css_classes);
	}

	phalcon_update_property(getThis(), SL("_cssClasses"), &css_classes);
}

/**
 * Set whether the output must be implictly flushed to the output or returned as string
 *
 * @param boolean $implicitFlush
 * @return Phalcon\FlashInterface
 */
PHP_METHOD(Phalcon_Flash, setImplicitFlush){

	zval *implicit_flush;

	phalcon_fetch_params(0, 1, 0, &implicit_flush);

	phalcon_update_property(getThis(), SL("_implicitFlush"), implicit_flush);
	RETURN_THIS();
}

/**
 * Set if the output must be implictily formatted with HTML
 *
 * @param boolean $automaticHtml
 * @return Phalcon\FlashInterface
 */
PHP_METHOD(Phalcon_Flash, setAutomaticHtml){

	zval *automatic_html;

	phalcon_fetch_params(0, 1, 0, &automatic_html);

	phalcon_update_property(getThis(), SL("_automaticHtml"), automatic_html);
	RETURN_THIS();
}

/**
 * Set an array with CSS classes to format the messages
 *
 * @param array $cssClasses
 * @return Phalcon\FlashInterface
 */
PHP_METHOD(Phalcon_Flash, setCssClasses){

	zval *css_classes;

	phalcon_fetch_params(0, 1, 0, &css_classes);

	if (Z_TYPE_P(css_classes) == IS_ARRAY) {
		phalcon_update_property(getThis(), SL("_cssClasses"), css_classes);
		RETURN_THIS();
	}
	PHALCON_THROW_EXCEPTION_STR(phalcon_flash_exception_ce, "CSS classes must be an Array");
	return;
}

static void phalcon_flash_message_helper(INTERNAL_FUNCTION_PARAMETERS, const char *stype)
{
	zval *msg, type = {};

	phalcon_fetch_params(0, 1, 0, &msg);

	ZVAL_STRING(&type, stype);

	PHALCON_RETURN_CALL_METHOD(getThis(), "message", &type, msg);
}

/**
 * Shows a HTML error message
 *
 *<code>
 * $flash->error('This is an error');
 *</code>
 *
 * @param string $message
 * @return string
 */
PHP_METHOD(Phalcon_Flash, error)
{
	phalcon_flash_message_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, "error");
}

/**
 * Shows a HTML notice/information message
 *
 *<code>
 * $flash->notice('This is an information');
 *</code>
 *
 * @param string $message
 * @return string
 */
PHP_METHOD(Phalcon_Flash, notice)
{
	phalcon_flash_message_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, "notice");
}

/**
 * Shows a HTML success message
 *
 *<code>
 * $flash->success('The process was finished successfully');
 *</code>
 *
 * @param string $message
 * @return string
 */
PHP_METHOD(Phalcon_Flash, success)
{
	phalcon_flash_message_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, "success");
}

/**
 * Shows a HTML warning message
 *
 *<code>
 * $flash->warning('Hey, this is important');
 *</code>
 *
 * @param string $message
 * @return string
 */
PHP_METHOD(Phalcon_Flash, warning)
{
	phalcon_flash_message_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, "warning");
}

/**
 * Outputs a message formatting it with HTML
 *
 *<code>
 * $flash->outputMessage('error', $message);
 *</code>
 *
 * @param string $type
 * @param string $message
 */
PHP_METHOD(Phalcon_Flash, outputMessage){

	zval *type, *message, automatic_html = {}, classes = {}, type_classes = {}, joined_classes = {}, css_classes = {}, implicit_flush = {}, content = {}, *msg, html_message = {};
	int flag_automatic_html;
	int flag_implicit_flush;

	phalcon_fetch_params(1, 2, 0, &type, &message);

	phalcon_read_property(&automatic_html, getThis(), SL("_automaticHtml"), PH_READONLY);
	flag_automatic_html = zend_is_true(&automatic_html);
	if (flag_automatic_html) {
		phalcon_read_property(&classes, getThis(), SL("_cssClasses"), PH_READONLY);

		if (phalcon_array_isset_fetch(&type_classes, &classes, type, PH_READONLY)) {
			if (Z_TYPE(type_classes) == IS_ARRAY) {
				phalcon_fast_join_str(&joined_classes, SL(" "), &type_classes);

				PHALCON_CONCAT_SVS(&css_classes, " class=\"", &joined_classes, "\"");
				zval_ptr_dtor(&joined_classes);
			} else {
				PHALCON_CONCAT_SVS(&css_classes, " class=\"", &type_classes, "\"");
			}
		} else {
			ZVAL_EMPTY_STRING(&css_classes);
		}
	}
	PHALCON_MM_ADD_ENTRY(&css_classes);

	phalcon_read_property(&implicit_flush, getThis(), SL("_implicitFlush"), PH_READONLY);
	flag_implicit_flush = zend_is_true(&implicit_flush);
	if (Z_TYPE_P(message) == IS_ARRAY) {
		/**
		 * We create the message with implicit flush or other
		 */
		if (!flag_implicit_flush) {
			PHALCON_MM_ZVAL_EMPTY_STRING(&content);
		}

		/**
		 * We create the message with implicit flush or other
		 */
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(message), msg) {
			zval html_message0 = {};
			/**
			 * We create the applying formatting or not
			 */
			if (flag_automatic_html) {
				PHALCON_CONCAT_SVSVS(&html_message0, "<div", &css_classes, ">", msg, "</div>" PHP_EOL);
				PHALCON_MM_ADD_ENTRY(&html_message0);
			} else {
				ZVAL_COPY_VALUE(&html_message0, msg);
			}

			if (flag_implicit_flush) {
				zend_print_zval(&html_message0, 0);
			} else {
				phalcon_concat_self(&content, &html_message0);
				PHALCON_MM_ADD_ENTRY(&content);
			}
		} ZEND_HASH_FOREACH_END();

		/**
		 * We return the message as string if the implicit_flush is turned off
		 */
		if (!flag_implicit_flush) {
			RETURN_MM_CTOR(&content);
		}
	} else {
		/**
		 * We create the applying formatting or not
		 */
		if (flag_automatic_html) {
			PHALCON_CONCAT_SVSVS(&html_message, "<div", &css_classes, ">", message, "</div>" PHP_EOL);
			PHALCON_MM_ADD_ENTRY(&html_message);
		} else {
			ZVAL_COPY_VALUE(&html_message, message);
		}

		/**
		 * We return the message as string if the implicit_flush is turned off
		 */
		if (flag_implicit_flush) {
			zend_print_zval(&html_message, 0);
			RETURN_MM();
		}
		RETURN_MM_CTOR(&html_message);

	}
	RETURN_MM();
}
