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
  |          Nikolaos Dimopoulos <nikos@phalconphp.com>                    |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "tag.h"
#include "tag/exception.h"
#include "tag/select.h"
#include "di.h"
#include "diinterface.h"
#include "escaperinterface.h"
#include "mvc/urlinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"

#include "interned-strings.h"

/**
 * Phalcon\Tag
 *
 * Phalcon\Tag is designed to simplify building of HTML tags.
 * It provides a set of helpers to generate HTML in a dynamic way.
 * This component is an abstract class that you can extend to add more helpers.
 */
zend_class_entry *phalcon_tag_ce;

PHP_METHOD(Phalcon_Tag, setDI);
PHP_METHOD(Phalcon_Tag, getDI);
PHP_METHOD(Phalcon_Tag, getUrlService);
PHP_METHOD(Phalcon_Tag, getEscaperService);
PHP_METHOD(Phalcon_Tag, getAutoescape);
PHP_METHOD(Phalcon_Tag, setAutoescape);
PHP_METHOD(Phalcon_Tag, setDefault);
PHP_METHOD(Phalcon_Tag, setDefaults);
PHP_METHOD(Phalcon_Tag, hasValue);
PHP_METHOD(Phalcon_Tag, getValue);
PHP_METHOD(Phalcon_Tag, resetInput);
PHP_METHOD(Phalcon_Tag, linkTo);
PHP_METHOD(Phalcon_Tag, _inputField);
PHP_METHOD(Phalcon_Tag, _inputFieldChecked);
PHP_METHOD(Phalcon_Tag, colorField);
PHP_METHOD(Phalcon_Tag, textField);
PHP_METHOD(Phalcon_Tag, numericField);
PHP_METHOD(Phalcon_Tag, rangeField);
PHP_METHOD(Phalcon_Tag, emailField);
PHP_METHOD(Phalcon_Tag, dateField);
PHP_METHOD(Phalcon_Tag, dateTimeField);
PHP_METHOD(Phalcon_Tag, dateTimeLocalField);
PHP_METHOD(Phalcon_Tag, monthField);
PHP_METHOD(Phalcon_Tag, timeField);
PHP_METHOD(Phalcon_Tag, weekField);
PHP_METHOD(Phalcon_Tag, passwordField);
PHP_METHOD(Phalcon_Tag, hiddenField);
PHP_METHOD(Phalcon_Tag, fileField);
PHP_METHOD(Phalcon_Tag, checkField);
PHP_METHOD(Phalcon_Tag, radioField);
PHP_METHOD(Phalcon_Tag, imageInput);
PHP_METHOD(Phalcon_Tag, searchField);
PHP_METHOD(Phalcon_Tag, telField);
PHP_METHOD(Phalcon_Tag, urlField);
PHP_METHOD(Phalcon_Tag, submitButton);
PHP_METHOD(Phalcon_Tag, selectStatic);
PHP_METHOD(Phalcon_Tag, select);
PHP_METHOD(Phalcon_Tag, textArea);
PHP_METHOD(Phalcon_Tag, form);
PHP_METHOD(Phalcon_Tag, endForm);
PHP_METHOD(Phalcon_Tag, setTitle);
PHP_METHOD(Phalcon_Tag, setTitleSeparator);
PHP_METHOD(Phalcon_Tag, appendTitle);
PHP_METHOD(Phalcon_Tag, prependTitle);
PHP_METHOD(Phalcon_Tag, getTitleSeparator);
PHP_METHOD(Phalcon_Tag, getTitle);
PHP_METHOD(Phalcon_Tag, stylesheetLink);
PHP_METHOD(Phalcon_Tag, javascriptInclude);
PHP_METHOD(Phalcon_Tag, friendlyTitle);
PHP_METHOD(Phalcon_Tag, setDocType);
PHP_METHOD(Phalcon_Tag, getDocType);
PHP_METHOD(Phalcon_Tag, tagHtml);
PHP_METHOD(Phalcon_Tag, tagHtmlClose);
PHP_METHOD(Phalcon_Tag, getDefault);
PHP_METHOD(Phalcon_Tag, getDefaults);
PHP_METHOD(Phalcon_Tag, setDefaultParams);
PHP_METHOD(Phalcon_Tag, getDefaultParams);
PHP_METHOD(Phalcon_Tag, setDefaultFormParams);
PHP_METHOD(Phalcon_Tag, getDefaultFormParams);
PHP_METHOD(Phalcon_Tag, choice);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_setdi, 0, 0, 1)
	ZEND_ARG_INFO(0, dependencyInjector)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_setautoescape, 0, 0, 1)
	ZEND_ARG_INFO(0, autoescape)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_setdefault, 0, 0, 2)
	ZEND_ARG_INFO(0, id)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_setdefaults, 0, 0, 1)
	ZEND_ARG_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_hasvalue, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_getvalue, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_linkto, 0, 0, 1)
	ZEND_ARG_INFO(0, parameters)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_generic_field, 0, 0, 1)
	ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_selectstatic, 0, 0, 1)
	ZEND_ARG_INFO(0, parameters)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_select, 0, 0, 1)
	ZEND_ARG_INFO(0, parameters)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_textarea, 0, 0, 1)
	ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_form, 0, 0, 0)
	ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_settitle, 0, 0, 1)
	ZEND_ARG_INFO(0, title)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_settitleseparator, 0, 0, 1)
	ZEND_ARG_INFO(0, separator)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_appendtitle, 0, 0, 1)
	ZEND_ARG_INFO(0, title)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_prependtitle, 0, 0, 1)
	ZEND_ARG_INFO(0, title)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_gettitle, 0, 0, 0)
	ZEND_ARG_INFO(0, tags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_stylesheetlink, 0, 0, 0)
	ZEND_ARG_INFO(0, parameters)
	ZEND_ARG_INFO(0, local)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_javascriptinclude, 0, 0, 0)
	ZEND_ARG_INFO(0, parameters)
	ZEND_ARG_INFO(0, local)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_imageinput, 0, 0, 0)
	ZEND_ARG_INFO(0, parameters)
	ZEND_ARG_INFO(0, local)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_friendlytitle, 0, 0, 1)
	ZEND_ARG_INFO(0, text)
	ZEND_ARG_INFO(0, separator)
	ZEND_ARG_INFO(0, lowercase)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_setdoctype, 0, 0, 1)
	ZEND_ARG_INFO(0, doctype)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_taghtml, 0, 0, 1)
	ZEND_ARG_INFO(0, tagName)
	ZEND_ARG_INFO(0, parameters)
	ZEND_ARG_INFO(0, selfClose)
	ZEND_ARG_INFO(0, onlyStart)
	ZEND_ARG_INFO(0, useEol)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_taghtmlclose, 0, 0, 1)
	ZEND_ARG_INFO(0, tagName)
	ZEND_ARG_INFO(0, useEol)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_getdefault, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_setdefaultparams, 0, 0, 1)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_setdefaultformparams, 0, 0, 1)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_tag_choice, 0, 0, 2)
	ZEND_ARG_INFO(0, expression)
	ZEND_ARG_INFO(0, value1)
	ZEND_ARG_INFO(0, value2)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_tag_method_entry[] = {
	PHP_ME(Phalcon_Tag, setDI, arginfo_phalcon_tag_setdi, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getDI, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getUrlService, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getEscaperService, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getAutoescape, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, setAutoescape, arginfo_phalcon_tag_setautoescape, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, setDefault, arginfo_phalcon_tag_setdefault, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, setDefaults, arginfo_phalcon_tag_setdefaults, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_MALIAS(Phalcon_Tag, displayTo, setDefault, arginfo_phalcon_tag_setdefault, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, hasValue, arginfo_phalcon_tag_hasvalue, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getValue, arginfo_phalcon_tag_getvalue, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, resetInput, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, linkTo, arginfo_phalcon_tag_linkto, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, _inputField, NULL, ZEND_ACC_STATIC|ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Tag, _inputFieldChecked, NULL, ZEND_ACC_STATIC|ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Tag, colorField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, textField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, numericField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, rangeField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, emailField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, dateField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, dateTimeField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, dateTimeLocalField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, monthField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, timeField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, weekField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, passwordField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, hiddenField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, searchField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, telField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, urlField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, fileField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, checkField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, radioField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, imageInput, arginfo_phalcon_tag_imageinput, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, submitButton, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, selectStatic, arginfo_phalcon_tag_selectstatic, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, select, arginfo_phalcon_tag_select, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, textArea, arginfo_phalcon_tag_textarea, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Tag, form, arginfo_phalcon_tag_form, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, endForm, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, setTitle, arginfo_phalcon_tag_settitle, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, setTitleSeparator, arginfo_phalcon_tag_settitleseparator, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, appendTitle, arginfo_phalcon_tag_appendtitle, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, prependTitle, arginfo_phalcon_tag_prependtitle, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getTitle, arginfo_phalcon_tag_gettitle, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getTitleSeparator, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, stylesheetLink, arginfo_phalcon_tag_stylesheetlink, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, javascriptInclude, arginfo_phalcon_tag_javascriptinclude, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, friendlyTitle, arginfo_phalcon_tag_friendlytitle, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, setDocType, arginfo_phalcon_tag_setdoctype, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getDocType, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, tagHtml, arginfo_phalcon_tag_taghtml, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, tagHtmlClose, arginfo_phalcon_tag_taghtmlclose, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getDefault, arginfo_phalcon_tag_getdefault, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getDefaults, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, setDefaultParams, arginfo_phalcon_tag_setdefaultparams, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getDefaultParams, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, setDefaultFormParams, arginfo_phalcon_tag_setdefaultformparams, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, getDefaultFormParams, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Tag, choice, arginfo_phalcon_tag_choice, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, color, colorField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, text, textField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, number, numericField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, range, rangeField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, email, emailField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, date, dateField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, dateTime, dateTimeField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, dateTimeLocal, dateTimeLocalField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, month, monthField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, time, timeField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, week, weekField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, password, passwordField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, hidden, hiddenField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, search, searchField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, tel, telField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, url, urlField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, file, fileField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, check, checkField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, radio, radioField, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, image, imageInput, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Tag, submit, submitButton, arginfo_phalcon_tag_generic_field, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Tag initializer
 */
PHALCON_INIT_CLASS(Phalcon_Tag){

	PHALCON_REGISTER_CLASS(Phalcon, Tag, tag, phalcon_tag_method_entry, 0);

	zend_declare_property_null(phalcon_tag_ce, SL("_displayValues"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_tag_ce, SL("_documentTitle"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_tag_ce, SL("_documentTitleSeparator"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_long(phalcon_tag_ce, SL("_documentType"), 11, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_tag_ce, SL("_dependencyInjector"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_tag_ce, SL("_dispatcherService"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_tag_ce, SL("_escaperService"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_bool(phalcon_tag_ce, SL("_autoEscape"), 1, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_tag_ce, SL("_defaultParams"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_tag_ce, SL("_defaultFormParams"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	zend_declare_class_constant_long(phalcon_tag_ce, SL("HTML32"), 1);
	zend_declare_class_constant_long(phalcon_tag_ce, SL("HTML401_STRICT"), 2);
	zend_declare_class_constant_long(phalcon_tag_ce, SL("HTML401_TRANSITIONAL"), 3);
	zend_declare_class_constant_long(phalcon_tag_ce, SL("HTML401_FRAMESET"), 4);
	zend_declare_class_constant_long(phalcon_tag_ce, SL("HTML5"), 5);
	zend_declare_class_constant_long(phalcon_tag_ce, SL("XHTML10_STRICT"), 6);
	zend_declare_class_constant_long(phalcon_tag_ce, SL("XHTML10_TRANSITIONAL"), 7);
	zend_declare_class_constant_long(phalcon_tag_ce, SL("XHTML10_FRAMESET"), 8);
	zend_declare_class_constant_long(phalcon_tag_ce, SL("XHTML11"), 9);
	zend_declare_class_constant_long(phalcon_tag_ce, SL("XHTML20"), 10);
	zend_declare_class_constant_long(phalcon_tag_ce, SL("XHTML5"), 11);

	return SUCCESS;
}

static void phalcon_tag_get_escaper(zval *escaper, zval *params)
{
	zval autoescape = {};

	if (!phalcon_array_isset_fetch_str(&autoescape, params, SL("escape"), PH_READONLY)) {
		phalcon_read_static_property_ce(&autoescape, phalcon_tag_ce, SL("_autoEscape"), PH_READONLY);
	}

	if (zend_is_true(&autoescape)) {
		if (FAILURE == phalcon_call_method_with_params(escaper, NULL, phalcon_tag_ce, phalcon_fcall_ce, SL("getescaperservice"), 0, NULL)) {
			return;
		}
	}
}

PHALCON_STATIC void phalcon_tag_render_attributes(zval *code, zval *attributes)
{
	zval escaper = {}, attrs = {}, v = {}, *value;
	zend_string *key;
	uint i;

	struct str_size_t {
		const char *str;
		uint size;
	};

	static const struct str_size_t order[14] = {
		{ SL("type") },
		{ SL("for") },
		{ SL("src") },
		{ SL("href") },
		{ SL("action") },
		{ SL("id") },
		{ SL("name") },
		{ SL("value") },
		{ SL("class") },
		{ SL("style") },
		{ SL("method") },
		{ SL("enctype") },
		{ SL("placeholder") },
		{ SL("multiple") },
	};

	assert(Z_TYPE_P(attributes) == IS_ARRAY);

	phalcon_tag_get_escaper(&escaper, attributes);
	if (Z_TYPE(escaper) <= IS_NULL) {
		Z_TRY_ADDREF_P(code);
		return;
	}
	PHALCON_MM_INIT();
	PHALCON_MM_ADD_ENTRY(&escaper);
	array_init(&attrs);
	PHALCON_MM_ADD_ENTRY(&attrs);
	for (i=0; i<sizeof(order)/sizeof(order[0]); ++i) {
		if (phalcon_array_isset_fetch_str(&v, attributes, order[i].str, order[i].size, PH_READONLY)) {
			phalcon_array_update_str(&attrs, order[i].str, order[i].size, &v, PH_COPY);
		}
	}

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(attrs), key, value) {
		zval tmp = {}, escaped = {};
		if (key && Z_TYPE_P(value) > IS_NULL) {
			ZVAL_STR(&tmp, key);
			if (Z_TYPE(escaper) == IS_OBJECT) {
				PHALCON_MM_CALL_METHOD(&escaped, &escaper, "escapehtmlattr", value);
				PHALCON_SCONCAT_SVSVS(code, " ", &tmp, "=\"", &escaped, "\"");
				zval_ptr_dtor(&escaped);
			} else {
				PHALCON_SCONCAT_SVSVS(code, " ", &tmp, "=\"", value, "\"");
			}
			PHALCON_MM_ADD_ENTRY(code);
		}
	} ZEND_HASH_FOREACH_END();
	Z_TRY_ADDREF_P(code);
	RETURN_MM();
}

/**
 * Sets the dependency injector container.
 *
 * @param Phalcon\DiInterface $dependencyInjector
 */
PHP_METHOD(Phalcon_Tag, setDI){

	zval *dependency_injector;

	phalcon_fetch_params(0, 1, 0, &dependency_injector);
	PHALCON_VERIFY_INTERFACE_EX(dependency_injector, phalcon_diinterface_ce, phalcon_tag_exception_ce);
	phalcon_update_static_property_ce(phalcon_tag_ce, SL("_dependencyInjector"), dependency_injector);
}

/**
 * Internally gets the dependency injector
 *
 * @return Phalcon\DiInterface
 */
PHP_METHOD(Phalcon_Tag, getDI){


	phalcon_read_static_property_ce(return_value, phalcon_tag_ce, SL("_dependencyInjector"), PH_COPY);
}

/**
 * Return a URL service from the default DI
 *
 * @return Phalcon\Mvc\UrlInterface
 */
PHP_METHOD(Phalcon_Tag, getUrlService){

	zval url = {}, dependency_injector = {}, service = {};

	PHALCON_MM_INIT();

	phalcon_read_static_property_ce(&dependency_injector, phalcon_tag_ce, SL("_dependencyInjector"), PH_READONLY);
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_MM_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);
		PHALCON_MM_VERIFY_INTERFACE_EX(&dependency_injector, phalcon_diinterface_ce, phalcon_tag_exception_ce);
		phalcon_update_static_property_ce(phalcon_tag_ce, SL("_dependencyInjector"), &dependency_injector);
	} else {
		PHALCON_MM_VERIFY_INTERFACE_EX(&dependency_injector, phalcon_diinterface_ce, phalcon_tag_exception_ce);
	}

	ZVAL_STR(&service, IS(url));

	PHALCON_MM_CALL_METHOD(&url, &dependency_injector, "getshared", &service);
	PHALCON_MM_ADD_ENTRY(&url);
	PHALCON_MM_VERIFY_INTERFACE(&url, phalcon_mvc_urlinterface_ce);

	RETURN_MM_CTOR(&url);
}

/**
 * Returns an Escaper service from the default DI
 *
 * @return Phalcon\EscaperInterface
 */
PHP_METHOD(Phalcon_Tag, getEscaperService){

	zval escaper = {}, dependency_injector = {}, service = {};

	PHALCON_MM_INIT();

	phalcon_read_static_property_ce(&escaper, phalcon_tag_ce, SL("_escaperService"), PH_READONLY);
	if (Z_TYPE(escaper) != IS_OBJECT) {
		phalcon_read_static_property_ce(&dependency_injector, phalcon_tag_ce, SL("_dependencyInjector"), PH_READONLY);
		if (Z_TYPE(dependency_injector) != IS_OBJECT) {
			PHALCON_MM_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
			PHALCON_MM_ADD_ENTRY(&dependency_injector);
		}

		if (Z_TYPE(dependency_injector) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_tag_exception_ce, "A dependency injector container is required to obtain the \"escaper\" service");
			return;
		}

		PHALCON_MM_VERIFY_INTERFACE(&dependency_injector, phalcon_diinterface_ce);

		ZVAL_STR(&service, IS(escaper));

		PHALCON_MM_CALL_METHOD(&escaper, &dependency_injector, "getshared", &service);
		PHALCON_MM_ADD_ENTRY(&escaper);

		PHALCON_MM_VERIFY_INTERFACE(&escaper, phalcon_escaperinterface_ce);
		phalcon_update_static_property_ce(phalcon_tag_ce, SL("_escaperService"), &escaper);
	}

	RETURN_MM_CTOR(&escaper);
}

/**
 * Get current autoescape mode
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Tag, getAutoescape) {


	phalcon_read_static_property_ce(return_value, phalcon_tag_ce, SL("_autoEscape"), PH_COPY);
}

/**
 * Set autoescape mode in generated html
 *
 * @param boolean $autoescape
 */
PHP_METHOD(Phalcon_Tag, setAutoescape){

	zval *autoescape;

	phalcon_fetch_params(0, 1, 0, &autoescape);

	phalcon_update_static_property_ce(phalcon_tag_ce, SL("_autoEscape"), autoescape);
}

/**
 * Assigns default values to generated tags by helpers
 *
 * <code>
 * //Assigning "peter" to "name" component
 * Phalcon\Tag::setDefault("name", "peter");
 *
 * //Later in the view
 * echo Phalcon\Tag::textField("name"); //Will have the value "peter" by default
 * </code>
 *
 * @param string $id
 * @param string $value
 */
PHP_METHOD(Phalcon_Tag, setDefault){

	zval *id, *value;

	phalcon_fetch_params(0, 2, 0, &id, &value);

	if (Z_TYPE_P(value) != IS_NULL) {
		if (Z_TYPE_P(value) == IS_ARRAY || Z_TYPE_P(value) == IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_tag_exception_ce, "Only scalar values can be assigned to UI components");
			return;
		}
	}

	phalcon_update_static_property_array_multi_ce(phalcon_tag_ce, SL("_displayValues"), value, SL("z"), 1, id);
}

/**
 * Assigns default values to generated tags by helpers
 *
 * <code>
 * //Assigning "peter" to "name" component
 * Phalcon\Tag::setDefaults(array("name" => "peter"));
 *
 * //Later in the view
 * echo Phalcon\Tag::textField("name"); //Will have the value "peter" by default
 * </code>
 *
 * @param array $values
 * @param boolean $merge
 */
PHP_METHOD(Phalcon_Tag, setDefaults){

	zval *values, *merge = NULL, display_values = {}, merged_values = {};

	phalcon_fetch_params(0, 1, 1, &values, &merge);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(values) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_tag_exception_ce, "An array is required as default values");
		return;
	}

	if (zend_is_true(merge)) {
		phalcon_read_static_property_ce(&display_values, phalcon_tag_ce, SL("_displayValues"), PH_READONLY);
		if (Z_TYPE(display_values) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_values, &display_values, values);
			phalcon_update_static_property_ce(phalcon_tag_ce, SL("_displayValues"), &merged_values);
			zval_ptr_dtor(&merged_values);
		} else {
			phalcon_update_static_property_ce(phalcon_tag_ce, SL("_displayValues"), values);
		}
	} else {
		phalcon_update_static_property_ce(phalcon_tag_ce, SL("_displayValues"), values);
	}
}

/**
 * Alias of Phalcon\Tag::setDefault
 *
 * @param string $id
 * @param string $value
 */
PHALCON_DOC_METHOD(Phalcon_Tag, displayTo)

/**
 * Check if a helper has a default value set using Phalcon\Tag::setDefault or value from $_POST
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Tag, hasValue){

	zval *name, display_values = {}, *_POST;

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_static_property_ce(&display_values, phalcon_tag_ce, SL("_displayValues"), PH_READONLY);

	/**
	 * Check if there is a predefined value for it
	 */
	if (phalcon_array_isset(&display_values, name)) {
		RETURN_TRUE;
	}

	/**
	 * Check if there is a post value for the item
	 */
	_POST = phalcon_get_global_str(SL("_POST"));
	RETURN_BOOL(phalcon_array_isset(_POST, name));
}

/**
 * Every helper calls this function to check whether a component has a predefined
 * value using Phalcon\Tag::setDefault or value from $_POST
 *
 * @param string $name
 * @param array $params
 * @return mixed
 */
PHP_METHOD(Phalcon_Tag, getValue){

	zval *name, *params = NULL, display_values = {}, value = {}, *_POST;

	phalcon_fetch_params(0, 1, 1, &name, &params);

	if (!params || !phalcon_array_isset_fetch_str(&value, params, SL("value"), PH_READONLY) || Z_TYPE(value) == IS_NULL) {
		phalcon_read_static_property_ce(&display_values, phalcon_tag_ce, SL("_displayValues"), PH_READONLY);

		/**
		 * Check if there is a predefined value for it
		 */
		if (!phalcon_array_isset_fetch(&value, &display_values, name, PH_READONLY)) {

			/* Check if there is a post value for the item */
			_POST = phalcon_get_global_str(SL("_POST"));
			if (!phalcon_array_isset_fetch(&value, _POST, name, PH_READONLY)) {
				RETURN_NULL();
			}
		}
	}

	RETURN_CTOR(&value);
}

/**
 * Resets the request and internal values to avoid those fields will have any default value
 */
PHP_METHOD(Phalcon_Tag, resetInput){

	phalcon_update_static_property_empty_array_ce(phalcon_tag_ce, SL("_displayValues"));
}

/**
 * Builds a HTML A tag using framework conventions
 *
 *<code>
 *	echo Phalcon\Tag::linkTo('signup/register', 'Register Here!');
 *	echo Phalcon\Tag::linkTo(array('signup/register', 'Register Here!'));
 *	echo Phalcon\Tag::linkTo(array('signup/register', 'Register Here!', 'class' => 'btn-primary'));
 *	echo Phalcon\Tag::linkTo('http://phalconphp.com/', 'Google', FALSE);
 *	echo Phalcon\Tag::linkTo(array('http://phalconphp.com/', 'Phalcon Home', FALSE));
 *	echo Phalcon\Tag::linkTo(array('http://phalconphp.com/', 'Phalcon Home', 'local' =>FALSE));
 *</code>
 *
 * @param array|string $parameters
 * @param string $text
 * @param boolean $local
 * @return string
 */
PHP_METHOD(Phalcon_Tag, linkTo){

	zval *parameters, *text = NULL, *local = NULL,  params = {}, default_params = {}, action = {}, link_text = {}, z_local = {}, query = {}, url = {}, internal_url = {}, code = {};

	phalcon_fetch_params(1, 1, 2, &parameters, &text, &local);

	if (!text) {
		text = &PHALCON_GLOBAL(z_null);
	}

	if (!local) {
		local = &PHALCON_GLOBAL(z_true);
	}

	if (Z_TYPE_P(parameters) != IS_ARRAY) {
		array_init_size(&params, 3);
		PHALCON_MM_ADD_ENTRY(&params);
		phalcon_array_append(&params, parameters, PH_COPY);
		phalcon_array_append(&params, text, PH_COPY);
		phalcon_array_append(&params, local, PH_COPY);
	} else {
		PHALCON_MM_ZVAL_DUP(&params, parameters);
	}

	phalcon_read_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"), PH_READONLY);
	if (Z_TYPE(default_params) == IS_ARRAY) {
		phalcon_array_merge_recursive_n2(&params, &default_params, PH_COPY);
	}

	if (!phalcon_array_isset_fetch_long(&action, &params, 0, PH_COPY)) {
		if (phalcon_array_isset_fetch_str(&action, &params, SL("action"), PH_COPY)) {
			PHALCON_MM_ADD_ENTRY(&action);
			phalcon_array_unset_str(&params, SL("action"), 0);
		} else {
			PHALCON_MM_ZVAL_EMPTY_STRING(&action);
		}
	} else {
		PHALCON_MM_ADD_ENTRY(&action);
		phalcon_array_unset_long(&params, 0, 0);
	}

	if (!phalcon_array_isset_fetch_long(&link_text, &params, 1, PH_COPY)) {
		if (phalcon_array_isset_fetch_str(&link_text, &params, SL("text"), PH_COPY)) {
			PHALCON_MM_ADD_ENTRY(&link_text);
			phalcon_array_unset_str(&params, SL("text"), 0);
		} else {
			PHALCON_MM_ZVAL_EMPTY_STRING(&link_text);
		}
	} else {
		PHALCON_MM_ADD_ENTRY(&link_text);
		phalcon_array_unset_long(&params, 1, 0);
	}

	if (!phalcon_array_isset_fetch_long(&z_local, &params, 2, PH_COPY)) {
		if (phalcon_array_isset_fetch_str(&z_local, &params, SL("local"), PH_COPY)) {
			PHALCON_MM_ADD_ENTRY(&z_local);
			phalcon_array_unset_str(&params, SL("local"), 0);
		} else {
			ZVAL_NULL(&z_local);
		}
	} else {
		PHALCON_MM_ADD_ENTRY(&z_local);
		phalcon_array_unset_long(&params, 2, 0);
	}

	if (phalcon_array_isset_fetch_str(&query, &params, SL("query"), PH_COPY)) {
		PHALCON_MM_ADD_ENTRY(&query);
		phalcon_array_unset_str(&params, SL("query"), 0);
	} else {
		ZVAL_NULL(&query);
	}

	PHALCON_MM_CALL_SELF(&url, "geturlservice");
	PHALCON_MM_ADD_ENTRY(&url);
	PHALCON_MM_CALL_METHOD(&internal_url, &url, "get", &action, &query, &z_local);
	phalcon_array_update_str(&params, SL("href"), &internal_url, 0);

	ZVAL_STRING(&code, "<a");

	phalcon_tag_render_attributes(&code, &params);
	if (EG(exception)) {
		zval_ptr_dtor(&code);
		RETURN_MM();;
	}

	PHALCON_CONCAT_VSVS(return_value, &code, ">", &link_text, "</a>");
	zval_ptr_dtor(&code);
	RETURN_MM();
}

/**
 * Builds generic INPUT tags
 *
 * @param string $type
 * @param array $parameters
 * @param boolean $asValue
 * @return string
 */
PHP_METHOD(Phalcon_Tag, _inputField){

	zval *type, *parameters, *as_value = NULL, params = {}, default_params = {}, value = {}, id = {}, name = {}, code = {}, doctype = {};
	zval label = {};

	phalcon_fetch_params(1, 2, 1, &type, &parameters, &as_value);

	if (Z_TYPE_P(parameters) != IS_ARRAY) {
		array_init_size(&params, 1);
		PHALCON_MM_ADD_ENTRY(&params);
		phalcon_array_append(&params, parameters, PH_COPY);
	} else {
		PHALCON_MM_ZVAL_DUP(&params, parameters);
	}

	if (!as_value) {
		as_value = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"), PH_READONLY);
	if (Z_TYPE(default_params) == IS_ARRAY) {
		phalcon_array_merge_recursive_n2(&params, &default_params, PH_COPY);
	}

	if (PHALCON_IS_FALSE(as_value)) {
		if (!phalcon_array_isset_fetch_str(&id, &params, SL("id"), PH_READONLY) || !zend_is_true(&id)) {
			if (!phalcon_array_isset_fetch_long(&id, &params, 0, PH_READONLY)) {
				ZVAL_NULL(&id);
			} else if (!phalcon_array_isset_str(&params, SL("id")) && Z_TYPE(id) == IS_STRING) {
				/**
				 * Automatically assign the id if the name is not an array
				 */
				if (!phalcon_memnstr_str(&id, SL("["))) {
					phalcon_array_update_str(&params, SL("id"), &id, PH_COPY);
				}
			}
		}

		if (!phalcon_array_isset_fetch_str(&name, &params, SL("name"), PH_READONLY)) {
			ZVAL_NULL(&name);
		}

		if (!zend_is_true(&name) && !zend_is_true(&id)) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_tag_exception_ce, "id or name must be specified");
			return;
		} else if (!zend_is_true(&name)) {
			ZVAL_COPY_VALUE(&name, &id);
			phalcon_array_update_string(&params, IS(name), &name, PH_COPY);
		}

		if (zend_is_true(&id)) {
			PHALCON_MM_CALL_SELF(&value, "getvalue", &id, &params);
		} else {
			PHALCON_MM_CALL_SELF(&value, "getvalue", &name, &params);
		}
		phalcon_array_update_string(&params, IS(value), &value, 0);

		if (phalcon_array_isset_fetch_str(&label, &params, SL("label"), PH_READONLY) && zend_is_true(&label)) {
			zval label_text = {};
			if (PHALCON_IS_NOT_EMPTY_STRING(&label)) {
				ZVAL_COPY_VALUE(&label_text, &label);
			} else {
				ZVAL_COPY_VALUE(&label_text, &name);
			}
			if (Z_TYPE(label) == IS_ARRAY) {
				PHALCON_SCONCAT_STR(&code, "<label");
				PHALCON_MM_ADD_ENTRY(&code);
				phalcon_tag_render_attributes(&code, &label);
				PHALCON_MM_ADD_ENTRY(&code);
				PHALCON_SCONCAT_SVS(&code, ">", &label_text, "</label>");
				PHALCON_MM_ADD_ENTRY(&code);
			} else {
				if (zend_is_true(&id)) {
					PHALCON_CONCAT_SVSVS(&code, "<label for=\"", &id, "\">", &label_text, "</label>");
				} else {
					PHALCON_CONCAT_SVS(&code, "<label>", &label_text, "</label>");
				}
				PHALCON_MM_ADD_ENTRY(&code);
			}
		}
	} else {
		/**
		 * Use the 'id' as value if the user hadn't set it
		 */
		if (!phalcon_array_isset_str(&params, SL("value"))) {
			if (phalcon_array_isset_fetch_long(&value, &params, 0, PH_READONLY)) {
				phalcon_array_update_string(&params, IS(value), &value, PH_COPY);
			}
		}
	}

	phalcon_array_update_string(&params, IS(type), type, PH_COPY);

	PHALCON_SCONCAT_STR(&code, "<input");
	PHALCON_MM_ADD_ENTRY(&code);

	phalcon_tag_render_attributes(&code, &params);
	PHALCON_MM_ADD_ENTRY(&code);

	phalcon_read_static_property_ce(&doctype, phalcon_tag_ce, SL("_documentType"), PH_READONLY);

	/**
	 * Check if Doctype is XHTML
	 */
	if (PHALCON_GT_LONG(&doctype, 5)) {
		PHALCON_CONCAT_VS(return_value, &code, " />");
	} else {
		PHALCON_CONCAT_VS(return_value, &code, ">");
	}

	RETURN_MM();
}

/**
 * Builds INPUT tags that implements the checked attribute
 *
 * @param   string $type
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, _inputFieldChecked){

	zval *type, *parameters, params = {}, default_params = {}, value = {}, id = {}, name = {}, current_value = {}, code = {}, doctype = {};

	phalcon_fetch_params(0, 2, 0, &type, &parameters);

	if (Z_TYPE_P(parameters) != IS_ARRAY) {
		array_init_size(&params, 1);
		phalcon_array_append(&params, parameters, PH_COPY);
	} else {
		ZVAL_DUP(&params, parameters);
	}

	phalcon_read_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"), PH_READONLY);
	if (Z_TYPE(default_params) == IS_ARRAY) {
		phalcon_array_merge_recursive_n2(&params, &default_params, PH_COPY);
	}

	if (!phalcon_array_isset_fetch_long(&id, &params, 0, PH_READONLY)) {
		phalcon_array_fetch_str(&id, &params, SL("id"), PH_NOISY|PH_READONLY);
		phalcon_array_update_long(&params, 0, &id, PH_COPY);
	}

	if (!phalcon_array_isset_fetch_str(&name, &params, SL("name"), PH_READONLY)) {
		phalcon_array_update_string(&params, IS(name), &id, PH_COPY);
	} else {
		if (!zend_is_true(&name)) {
			phalcon_array_update_string(&params, IS(name), &id, PH_COPY);
		}
	}

	/**
	 * Automatically assign the id if the name is not an array
	 */
	if (!phalcon_memnstr_str(&id, SL("["))) {
		if (!phalcon_array_isset_str(&params, SL("id"))) {
			phalcon_array_update_str(&params, SL("id"), &id, PH_COPY);
		}
	}

	/**
	 * Automatically check inputs
	 */
	if (phalcon_array_isset_fetch_str(&current_value, &params, SL("value"), PH_COPY)) {
		phalcon_array_unset_str(&params, SL("value"), 0);

		PHALCON_CALL_SELF(&value, "getvalue", &id, &params);

		if (Z_TYPE(value) != IS_NULL && PHALCON_IS_EQUAL(&current_value, &value)) {
			phalcon_array_update_str_str(&params, SL("checked"), SL("checked"), 0);
		}
		zval_ptr_dtor(&value);

		phalcon_array_update_string(&params, IS(value), &current_value, 0);
	} else {
		PHALCON_CALL_SELF(&value, "getvalue", &id, &params);

		/**
		 * Evaluate the value in POST
		 */
		if (zend_is_true(&value)) {
			phalcon_array_update_str_str(&params, SL("checked"), SL("checked"), 0);
		}

		phalcon_array_update_string(&params, IS(value), &value, 0);
	}

	phalcon_array_update_string(&params, IS(type), type, PH_COPY);

	ZVAL_STRING(&code, "<input");

	phalcon_tag_render_attributes(&code, &params);
	zval_ptr_dtor(&params);

	phalcon_read_static_property_ce(&doctype, phalcon_tag_ce, SL("_documentType"), PH_READONLY);

	/**
	 * Check if Doctype is XHTML
	 */
	if (PHALCON_GT_LONG(&doctype, 5)) {
		PHALCON_CONCAT_VS(return_value, &code, " />");
	} else {
		PHALCON_CONCAT_VS(return_value, &code, ">");
	}
	zval_ptr_dtor(&code);
}

static void phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAMETERS, const char* type, int as_value)
{
	zval *parameters, field_type = {};

	phalcon_fetch_params(1, 1, 0, &parameters);

	PHALCON_MM_ZVAL_STRING(&field_type, type);
	if (as_value) {
		PHALCON_MM_RETURN_CALL_SELF("_inputfield", &field_type, parameters, &PHALCON_GLOBAL(z_true));
	} else {
		PHALCON_MM_RETURN_CALL_SELF("_inputfield", &field_type, parameters);
	}
	RETURN_MM();
}

static void phalcon_tag_generic_field_checked(INTERNAL_FUNCTION_PARAMETERS, const char* type)
{
	zval *parameters, field_type = {};

	phalcon_fetch_params(1, 1, 0, &parameters);

	PHALCON_MM_ZVAL_STRING(&field_type, type);
	PHALCON_MM_RETURN_CALL_SELF("_inputfieldchecked", &field_type, parameters);
	RETURN_MM();
}

/**
 * Builds a HTML input[type="color"] tag
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, colorField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "color", 0);
}

/**
 * Builds a HTML input[type="text"] tag
 *
 * <code>
 *	echo Phalcon\Tag::textField(array("name", "size" => 30));
 * </code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, textField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "text", 0);
}

/**
 * Builds a HTML input[type="number"] tag
 *
 * <code>
 *	echo Phalcon\Tag::numericField(array("price", "min" => "1", "max" => "5"));
 * </code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, numericField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "number", 0);
}

/**
 * Builds a HTML input[type="range"] tag
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, rangeField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "range", 0);
}

/**
 * Builds a HTML input[type="email"] tag
 *
 * <code>
 *	echo Phalcon\Tag::emailField("email");
 * </code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, emailField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "email", 0);
}

/**
 * Builds a HTML input[type="date"] tag
 *
 * <code>
 *	echo Phalcon\Tag::dateField(array("born", "value" => "14-12-1980"))
 * </code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, dateField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "date", 0);
}

/**
 * Builds a HTML input[type="datetime"] tag
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, dateTimeField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "datetime", 0);
}

/**
 * Builds a HTML input[type="datetime-local"] tag
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, dateTimeLocalField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "datetime-local", 0);
}

/**
 * Builds a HTML input[type="month"] tag
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, monthField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "month", 0);
}

/**
 * Builds a HTML input[type="time"] tag
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, timeField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "time", 0);
}

/**
 * Builds a HTML input[type="week"] tag
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, weekField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "week", 0);
}

/**
 * Builds a HTML input[type="password"] tag
 *
 *<code>
 * echo Phalcon\Tag::passwordField(array("name", "size" => 30));
 *</code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, passwordField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "password", 0);
}

/**
 * Builds a HTML input[type="hidden"] tag
 *
 *<code>
 * echo Phalcon\Tag::hiddenField(array("name", "value" => "mike"));
 *</code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, hiddenField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "hidden", 0);
}

/**
 * Builds a HTML input[type="file"] tag
 *
 *<code>
 * echo Phalcon\Tag::fileField("file");
 *</code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, fileField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "file", 0);
}

/**
 * Builds a HTML input[type="search"] tag
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, searchField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "search", 0);
}

/**
 * Builds a HTML input[type="tel"] tag
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, telField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "tel", 0);
}

/**
 * Builds a HTML input[type="url"] tag
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, urlField){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "url", 0);
}

/**
 * Builds a HTML input[type="check"] tag
 *
 *<code>
 * echo Phalcon\Tag::checkField(array("terms", "value" => "Y"));
 *</code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, checkField){

	phalcon_tag_generic_field_checked(INTERNAL_FUNCTION_PARAM_PASSTHRU, "checkbox");
}

/**
 * Builds a HTML input[type="radio"] tag
 *
 *<code>
 * echo Phalcon\Tag::radioField(array("wheather", "value" => "hot"))
 *</code>
 *
 * Volt syntax:
 *<code>
 * {{ radio_field('Save') }}
 *</code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, radioField){

	phalcon_tag_generic_field_checked(INTERNAL_FUNCTION_PARAM_PASSTHRU, "radio");
}

/**
 * Builds a HTML input[type="submit"] tag
 *
 *<code>
 * echo Phalcon\Tag::submitButton("Save")
 *</code>
 *
 * Volt syntax:
 *<code>
 * {{ submit_button('Save') }}
 *</code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, submitButton){

	phalcon_tag_generic_field(INTERNAL_FUNCTION_PARAM_PASSTHRU, "submit", 1);
}

/**
 * Builds a HTML SELECT tag using a PHP array for options
 *
 *<code>
 *	echo Phalcon\Tag::selectStatic("status", array("A" => "Active", "I" => "Inactive"))
 *</code>
 *
 * @param array $parameters
 * @param   array $data
 * @return string
 */
PHP_METHOD(Phalcon_Tag, selectStatic){

	zval *parameters, *data = NULL;

	phalcon_fetch_params(0, 1, 1, &parameters, &data);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_CE_STATIC(phalcon_tag_select_ce, "selectfield", parameters, data);
}

/**
 * Builds a HTML SELECT tag using a Phalcon\Mvc\Model resultset as options
 *
 *<code>
 *	echo Phalcon\Tag::select(array(
 *		"robotId",
 *		Robots::find("type = 'mechanical'"),
 *		"using" => array("id", "name")
 * 	));
 *</code>
 *
 * Volt syntax:
 *<code>
 * {{ select("robotId", robots, "using": ["id", "name"]) }}
 *</code>
 *
 * @param array $parameters
 * @param   array $data
 * @return string
 */
PHP_METHOD(Phalcon_Tag, select){

	zval *parameters, *data = NULL;

	phalcon_fetch_params(0, 1, 1, &parameters, &data);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_CE_STATIC(phalcon_tag_select_ce, "selectfield", parameters, data);
}

/**
 * Builds a HTML TEXTAREA tag
 *
 *<code>
 * echo Phalcon\Tag::textArea(array("comments", "cols" => 10, "rows" => 4))
 *</code>
 *
 * Volt syntax:
 *<code>
 * {{ text_area("comments", "cols": 10, "rows": 4) }}
 *</code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, textArea){

	zval *parameters, params = {}, default_params = {}, id = {}, name = {}, content = {}, code = {}, escaper = {}, escaped = {};

	phalcon_fetch_params(0, 1, 0, &parameters);

	if (Z_TYPE_P(parameters) != IS_ARRAY) {
		array_init_size(&params, 1);
		phalcon_array_append(&params, parameters, PH_COPY);
	} else {
		ZVAL_DUP(&params, parameters);
	}

	phalcon_read_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"), PH_READONLY);
	if (Z_TYPE(default_params) == IS_ARRAY) {
		phalcon_array_merge_recursive_n2(&params, &default_params, PH_COPY);
	}

	if (!phalcon_array_isset_fetch_long(&id, &params, 0, PH_READONLY)) {
		if (phalcon_array_isset_fetch_str(&id, &params, SL("id"), PH_READONLY)) {
			phalcon_array_update_long(&params, 0, &id, PH_COPY);
		}
	}

	if (!phalcon_array_isset_fetch_str(&name, &params, SL("name"), PH_READONLY)) {
		phalcon_array_update_string(&params, IS(name), &id, PH_COPY);
	} else {
		if (!zend_is_true(&name)) {
			phalcon_array_update_string(&params, IS(name), &id, PH_COPY);
		}
	}

	if (!phalcon_array_isset_str(&params, SL("id"))) {
		phalcon_array_update_str(&params, SL("id"), &id, PH_COPY);
	}

	PHALCON_CALL_SELF(&content, "getvalue", &id, &params);

	phalcon_tag_get_escaper(&escaper, &params);

	if (Z_TYPE(escaper) == IS_OBJECT) {
		PHALCON_CALL_METHOD(&escaped, &escaper, "escapehtml", &content);
	} else {
		ZVAL_COPY(&escaped, &content);
	}
	zval_ptr_dtor(&escaper);
	zval_ptr_dtor(&content);

	ZVAL_STRING(&code, "<textarea");

	phalcon_tag_render_attributes(&code, &params);

	PHALCON_CONCAT_VSVS(return_value, &code, ">", &escaped, "</textarea>");
	zval_ptr_dtor(&escaped);
	zval_ptr_dtor(&code);
}

/**
 * Builds a HTML FORM tag
 *
 * <code>
 * echo Phalcon\Tag::form("posts/save");
 * echo Phalcon\Tag::form(array("posts/save", "method" => "post"));
 * </code>
 *
 * Volt syntax:
 * <code>
 * {{ form("posts/save") }}
 * {{ form("posts/save", "method": "post") }}
 * </code>
 *
 * @param array $parameters
 * @return string
 */
PHP_METHOD(Phalcon_Tag, form){

	zval *args = NULL, params = {}, default_params = {}, params_action = {}, action = {}, parameters = {}, url = {}, code = {};

	phalcon_fetch_params(0, 0, 1, &args);

	if (args) {
		if (Z_TYPE_P(args) != IS_ARRAY) {
			array_init_size(&params, 1);
			phalcon_array_append(&params, args, PH_COPY);
		} else {
			ZVAL_DUP(&params, args);
		}
	} else {
		array_init(&params);
	}

	phalcon_read_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"), PH_READONLY);
	if (Z_TYPE(default_params) == IS_ARRAY) {
		phalcon_array_merge_recursive_n2(&params, &default_params, PH_COPY);
	}

	if (!phalcon_array_isset_fetch_long(&params_action, &params, 0, PH_READONLY)) {
		if (!phalcon_array_isset_fetch_str(&params_action, &params, SL("action"), PH_READONLY)) {
			ZVAL_NULL(&params_action);
		}
	}

	/**
	 * By default the method is POST
	 */
	if (!phalcon_array_isset_str(&params, SL("method"))) {
		phalcon_array_update_str_str(&params, SL("method"), SL("post"), 0);
	}

	if (Z_TYPE(params_action) != IS_NULL) {
		PHALCON_CALL_SELF(&url, "geturlservice");
		PHALCON_CALL_METHOD(&action, &url, "get", &params_action);
		zval_ptr_dtor(&url);
	}

	/**
	 * Check for extra parameters
	 */
	if (phalcon_array_isset_fetch_str(&parameters, &params, SL("parameters"), PH_READONLY)) {
		PHALCON_SCONCAT_SV(&action, "?", &parameters);
	}

	if (Z_TYPE(action) != IS_NULL) {
		phalcon_array_update_str(&params, SL("action"), &action, PH_COPY);
		zval_ptr_dtor(&action);
	}

	ZVAL_STRING(&code, "<form");

	phalcon_tag_render_attributes(&code, &params);
	zval_ptr_dtor(&params);

	PHALCON_CONCAT_VS(return_value, &code, ">");
	zval_ptr_dtor(&code);
}

/**
 * Builds a HTML close FORM tag
 *
 * @return string
 */
PHP_METHOD(Phalcon_Tag, endForm){


	RETURN_STRING("</form>");
}

/**
 * Set the title of view content
 *
 *<code>
 * Phalcon\Tag::setTitle('Welcome to my Page');
 *</code>
 *
 * @param string $title
 */
PHP_METHOD(Phalcon_Tag, setTitle){

	zval *title;

	phalcon_fetch_params(0, 1, 0, &title);

	phalcon_update_static_property_ce(phalcon_tag_ce, SL("_documentTitle"), title);

}

/**
 * Set the title separator of view content
 *
 *<code>
 * Phalcon\Tag::setTitleSeparator('-');
 *</code>
 *
 * @param string $titleSeparator
 */
PHP_METHOD(Phalcon_Tag, setTitleSeparator){

	zval *title_separator;

	phalcon_fetch_params(0, 1, 0, &title_separator);

	phalcon_update_static_property_ce(phalcon_tag_ce, SL("_documentTitleSeparator"), title_separator);

}

/**
 * Appends a text to current document title
 *
 * @param string $title
 */
PHP_METHOD(Phalcon_Tag, appendTitle){

	zval *title, document_title = {}, document_title_separator = {}, s = {};

	phalcon_fetch_params(0, 1, 0, &title);

	phalcon_read_static_property_ce(&document_title, phalcon_tag_ce, SL("_documentTitle"), PH_READONLY);
	phalcon_read_static_property_ce(&document_title_separator, phalcon_tag_ce, SL("_documentTitleSeparator"), PH_READONLY);

	PHALCON_CONCAT_VVV(&s, &document_title, &document_title_separator, title);
	phalcon_update_static_property_ce(phalcon_tag_ce, SL("_documentTitle"), &s);
	zval_ptr_dtor(&s);
}

/**
 * Prepends a text to current document title
 *
 * @param string $title
 */
PHP_METHOD(Phalcon_Tag, prependTitle){

	zval *title, document_title = {}, document_title_separator = {}, s = {};

	phalcon_fetch_params(0, 1, 0, &title);

	phalcon_read_static_property_ce(&document_title, phalcon_tag_ce, SL("_documentTitle"), PH_READONLY);
	phalcon_read_static_property_ce(&document_title_separator, phalcon_tag_ce, SL("_documentTitleSeparator"), PH_READONLY);

	PHALCON_CONCAT_VVV(&s, title, &document_title_separator, &document_title);
	phalcon_update_static_property_ce(phalcon_tag_ce, SL("_documentTitle"), &s);
	zval_ptr_dtor(&s);
}

/**
 * Gets the current document title
 *
 * <code>
 * 	echo Phalcon\Tag::getTitle();
 * </code>
 *
 * <code>
 * 	{{ get_title() }}
 * </code>
 *
 * @return string
 */
PHP_METHOD(Phalcon_Tag, getTitle){

	zval *tags = NULL, document_title = {};

	phalcon_fetch_params(0, 0, 1, &tags);

	phalcon_read_static_property_ce(&document_title, phalcon_tag_ce, SL("_documentTitle"), PH_READONLY);
	if (!tags || zend_is_true(tags)) {
		PHALCON_CONCAT_SVS(return_value, "<title>", &document_title, "</title>" PHP_EOL);
	} else {
		RETURN_CTOR(&document_title);
	}
}

/**
 * Gets the current document title separator
 *
 * <code>
 * 	echo Phalcon\Tag::getTitleSeparator();
 * </code>
 *
 * <code>
 * 	{{ get_title_separator() }}
 * </code>
 *
 * @return string
 */
PHP_METHOD(Phalcon_Tag, getTitleSeparator)
{


	phalcon_read_static_property_ce(return_value, phalcon_tag_ce, SL("_documentTitleSeparator"), PH_COPY);
}

/**
 * Builds a LINK[rel="stylesheet"] tag
 *
 * <code>
 * 	echo Phalcon\Tag::stylesheetLink("http://fonts.googleapis.com/css?family=Rosario", false);
 * 	echo Phalcon\Tag::stylesheetLink("css/style.css");
 * </code>
 *
 * Volt Syntax:
 *<code>
 * 	{{ stylesheet_link("http://fonts.googleapis.com/css?family=Rosario", false) }}
 * 	{{ stylesheet_link("css/style.css") }}
 *</code>
 *
 * @param array $parameters
 * @param boolean $local
 * @param array $args
 * @return string
 */
PHP_METHOD(Phalcon_Tag, stylesheetLink){

	zval *parameters = NULL, *local = NULL, *args = NULL, params = {}, default_params = {};
	zval first_param = {}, z_local = {}, z_args = {}, url = {}, url_href = {}, href = {}, code = {}, doctype = {}, rel = {};

	phalcon_fetch_params(0, 0, 3, &parameters, &local, &args);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (!local) {
		local = &PHALCON_GLOBAL(z_true);
	}

	if (!args) {
		args = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(parameters) != IS_ARRAY) {
		array_init_size(&params, 3);
		phalcon_array_append(&params, parameters, PH_COPY);
		phalcon_array_append(&params, local, PH_COPY);
		phalcon_array_append(&params, args, PH_COPY);
	} else {
		ZVAL_DUP(&params, parameters);
	}

	phalcon_read_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"), PH_READONLY);
	if (Z_TYPE(default_params) == IS_ARRAY) {
		phalcon_array_merge_recursive_n2(&params, &default_params, PH_COPY);
	}

	if (!phalcon_array_isset_str(&params, SL("href"))) {
		if (phalcon_array_isset_fetch_long(&first_param, &params, 0, PH_READONLY)) {
			phalcon_array_update_str(&params, SL("href"), &first_param, PH_COPY);
		} else {
			phalcon_array_update_str_str(&params, SL("href"), SL(""), 0);
		}
	}

	if (!phalcon_array_isset_fetch_long(&z_local, &params, 1, PH_COPY)) {
		if (phalcon_array_isset_fetch_str(&z_local, &params, SL("local"), PH_COPY)) {
			phalcon_array_unset_str(&params, SL("local"), 0);
		} else {
			ZVAL_TRUE(&z_local);
		}
	}

	if (!phalcon_array_isset_fetch_long(&z_args, &params, 2, PH_COPY)) {
		if (phalcon_array_isset_fetch_str(&z_args, &params, SL("args"), PH_COPY)) {
			phalcon_array_unset_str(&params, SL("args"), 0);
		} else {
			ZVAL_TRUE(&z_args);
		}
	}

	if (!phalcon_array_isset_str(&params, SL("type"))) {
		phalcon_array_update_string_str(&params, IS(type), SL("text/css"), 0);
	}

	/**
	 * URLs are generated through the 'url' service
	 */
	if (zend_is_true(&z_local)) {
		PHALCON_CALL_SELF(&url, "geturlservice");

		phalcon_array_fetch_str(&url_href, &params, SL("href"), PH_NOISY|PH_READONLY);

		PHALCON_CALL_METHOD(&href, &url, "getstatic", &url_href, &z_args);
		zval_ptr_dtor(&url);
		phalcon_array_update_str(&params, SL("href"), &href, 0);
	}
	zval_ptr_dtor(&z_local);
	zval_ptr_dtor(&z_args);

	if (phalcon_array_isset_fetch_str(&rel, &params, SL("rel"), PH_READONLY)) {
		phalcon_array_unset_str(&params, SL("rel"), 0);
		PHALCON_CONCAT_SVS(&code, "<link rel=\"", &rel, "\"");
	} else {
		ZVAL_STRING(&code, "<link rel=\"stylesheet\"");
	}

	phalcon_tag_render_attributes(&code, &params);
	zval_ptr_dtor(&params);

	phalcon_read_static_property_ce(&doctype, phalcon_tag_ce, SL("_documentType"), PH_READONLY);

	/**
	 * Check if Doctype is XHTML
	 */
	if (PHALCON_GT_LONG(&doctype, 5)) {
		PHALCON_CONCAT_VS(return_value, &code, " />" PHP_EOL);
	} else {
		PHALCON_CONCAT_VS(return_value, &code, ">" PHP_EOL);
	}
	zval_ptr_dtor(&code);
}

/**
 * Builds a SCRIPT[type="javascript"] tag
 *
 * <code>
 * 	echo Phalcon\Tag::javascriptInclude("http://ajax.googleapis.com/ajax/libs/jquery/1.7.1/jquery.min.js", false);
 * 	echo Phalcon\Tag::javascriptInclude("javascript/jquery.js");
 * </code>
 *
 * Volt syntax:
 * <code>
 * {{ javascript_include("http://ajax.googleapis.com/ajax/libs/jquery/1.7.1/jquery.min.js", false) }}
 * {{ javascript_include("javascript/jquery.js") }}
 * </code>
 *
 * @param array $parameters
 * @param boolean $local
 * @param array $args
 * @return string
 */
PHP_METHOD(Phalcon_Tag, javascriptInclude){

	zval *parameters = NULL, *local = NULL, *args = NULL, params = {}, default_params = {};
	zval first_param = {}, z_local = {}, z_args = {}, params_src = {}, url = {}, src = {}, code = {};

	phalcon_fetch_params(1, 0, 3, &parameters, &local, &args);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (!local) {
		local = &PHALCON_GLOBAL(z_true);
	}

	if (!args) {
		args = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(parameters) != IS_ARRAY) {
		array_init_size(&params, 3);
		phalcon_array_append(&params, parameters, PH_COPY);
		phalcon_array_append(&params, local, PH_COPY);
		phalcon_array_append(&params, args, PH_COPY);
		PHALCON_MM_ADD_ENTRY(&params);
	} else {
		PHALCON_MM_ZVAL_DUP(&params, parameters);
	}

	phalcon_read_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"), PH_READONLY);
	if (Z_TYPE(default_params) == IS_ARRAY) {
		phalcon_array_merge_recursive_n2(&params, &default_params, PH_COPY);
	}

	if (!phalcon_array_isset_str(&params, SL("src"))) {
		if (phalcon_array_isset_fetch_long(&first_param, &params, 0, PH_READONLY)) {
			phalcon_array_update_str(&params, SL("src"), &first_param, PH_COPY);
		} else {
			phalcon_array_update_str_str(&params, SL("src"), SL(""), 0);
		}
	}

	if (!phalcon_array_isset_fetch_long(&z_local, &params, 1, PH_READONLY)) {
		if (phalcon_array_isset_fetch_str(&z_local, &params, SL("local"), PH_COPY)) {
			PHALCON_MM_ADD_ENTRY(&z_local);
			phalcon_array_unset_str(&params, SL("local"), 0);
		} else {
			ZVAL_TRUE(&z_local);
		}
	}

	if (!phalcon_array_isset_fetch_long(&z_args, &params, 2, PH_READONLY)) {
		if (phalcon_array_isset_fetch_str(&z_args, &params, SL("args"), PH_COPY)) {
			PHALCON_MM_ADD_ENTRY(&z_args);
			phalcon_array_unset_str(&params, SL("args"), 0);
		} else {
			ZVAL_TRUE(&z_args);
		}
	}

	if (!phalcon_array_isset_str(&params, SL("type"))) {
		phalcon_array_update_string_str(&params, IS(type), SL("text/javascript"), 0);
	}

	/**
	 * URLs are generated through the 'url' service
	 */
	if (zend_is_true(&z_local)) {
		PHALCON_MM_CALL_SELF(&url, "geturlservice");
		PHALCON_MM_ADD_ENTRY(&url);

		phalcon_array_fetch_str(&params_src, &params, SL("src"), PH_NOISY|PH_READONLY);

		PHALCON_MM_CALL_METHOD(&src, &url, "getstatic", &params_src, &z_args);

		phalcon_array_update_str(&params, SL("src"), &src, 0);
	}

	ZVAL_STRING(&code, "<script");

	phalcon_tag_render_attributes(&code, &params);

	PHALCON_CONCAT_VS(return_value, &code, "></script>" PHP_EOL);
	PHALCON_MM_ADD_ENTRY(&code);
	RETURN_MM();
}

/**
 * Builds HTML IMG tags
 *
 * <code>
 * 	echo Phalcon\Tag::image("img/bg.png");
 * 	echo Phalcon\Tag::image(array("img/photo.jpg", "alt" => "Some Photo"));
 * </code>
 *
 * Volt Syntax:
 * <code>
 * 	{{ image("img/bg.png") }}
 * 	{{ image("img/photo.jpg", "alt": "Some Photo") }}
 * 	{{ image("http://static.mywebsite.com/img/bg.png", false) }}
 * </code>
 *
 * @param  array $parameters
 * @param  boolean $local
 * @return string
 */
PHP_METHOD(Phalcon_Tag, imageInput){

	zval *parameters = NULL, *local = NULL, params = {}, default_params = {}, first_param = {}, second_param = {};
	zval url = {}, url_src = {}, src = {}, code = {}, doctype = {};

	phalcon_fetch_params(0, 0, 2, &parameters, &local);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (!local) {
		local = &PHALCON_GLOBAL(z_true);
		if (Z_TYPE_P(parameters) == IS_ARRAY && phalcon_array_isset_fetch_long(&second_param, parameters, 1, PH_READONLY)) {
			if (!zend_is_true(&second_param)) {
				local = &PHALCON_GLOBAL(z_false);
			}
		}
	}

	if (Z_TYPE_P(parameters) != IS_ARRAY) {
		array_init_size(&params, 1);
		phalcon_array_append(&params, parameters, PH_COPY);
	} else {
		ZVAL_DUP(&params, parameters);
	}

	phalcon_read_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"), PH_READONLY);
	if (Z_TYPE(default_params) == IS_ARRAY) {
		phalcon_array_merge_recursive_n2(&params, &default_params, PH_COPY);
	}

	if (!phalcon_array_isset_str(&params, SL("src"))) {
		if (phalcon_array_isset_fetch_long(&first_param, &params, 0, PH_READONLY)) {
			phalcon_array_update_str(&params, SL("src"), &first_param, PH_COPY);
		} else {
			phalcon_array_update_str_str(&params, SL("src"), SL(""), 0);
		}
	}

	/**
	 * Use the 'url' service if the URI is local
	 */
	if (zend_is_true(local)) {
		PHALCON_CALL_SELF(&url, "geturlservice");

		phalcon_array_fetch_str(&url_src, &params, SL("src"), PH_NOISY|PH_READONLY);

		PHALCON_CALL_METHOD(&src, &url, "getstatic", &url_src);
		zval_ptr_dtor(&url);
		phalcon_array_update_str(&params, SL("src"), &src, 0);
	}

	ZVAL_STRING(&code, "<img");

	phalcon_tag_render_attributes(&code, &params);
	zval_ptr_dtor(&params);

	phalcon_read_static_property_ce(&doctype, phalcon_tag_ce, SL("_documentType"), PH_READONLY);

	/**
	 * Check if Doctype is XHTML
	 */
	if (PHALCON_GT_LONG(&doctype, 5)) {
		PHALCON_CONCAT_VS(return_value, &code, " />");
	} else {
		PHALCON_CONCAT_VS(return_value, &code, ">");
	}
	zval_ptr_dtor(&code);
}

/**
 * Converts texts into URL-friendly titles
 *
 *<code>
 * echo Phalcon\Tag::friendlyTitle('These are big important news', '-')
 *</code>
 *
 * @param string $text
 * @param string $separator
 * @param boolean $lowercase
 * @return text
 */
PHP_METHOD(Phalcon_Tag, friendlyTitle){

	zval *text, *separator = NULL, *lowercase = NULL, sep = {}, pattern = {}, friendly = {};

	phalcon_fetch_params(1, 1, 2, &text, &separator, &lowercase);

	if (!separator) {
		PHALCON_MM_ZVAL_STRING(&sep, "-");
	} else {
		ZVAL_COPY_VALUE(&sep, separator);
	}

	if (!lowercase) {
		lowercase = &PHALCON_GLOBAL(z_true);
	}

	PHALCON_MM_ZVAL_STRING(&pattern, "~[^a-z0-9A-Z]+~");

	phalcon_fast_preg_replace(&friendly, &pattern, &sep, text);
	PHALCON_MM_ADD_ENTRY(&friendly);
	if (zend_is_true(lowercase)) {
		phalcon_fast_strtolower(return_value, &friendly);
		RETURN_MM();
	}
	RETURN_MM_CTOR(&friendly);
}

/**
 * Set the document type of content
 *
 * @param string $doctype
 */
PHP_METHOD(Phalcon_Tag, setDocType){

	zval *doctype;

	phalcon_fetch_params(0, 1, 0, &doctype);

	phalcon_update_static_property_ce(phalcon_tag_ce, SL("_documentType"), doctype);

}

/**
 * Get the document type declaration of content
 *
 * @return string
 */
PHP_METHOD(Phalcon_Tag, getDocType){

	zval doctype = {};

	phalcon_read_static_property_ce(&doctype, phalcon_tag_ce, SL("_documentType"), PH_READONLY);

	switch (phalcon_get_intval(&doctype)) {
		case 1:  RETURN_STRING("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">" PHP_EOL);
		/* no break */
		case 2:  RETURN_STRING("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\"" PHP_EOL "\t\"http://www.w3.org/TR/html4/strict.dtd\">" PHP_EOL);
		/* no break */
		case 3:  RETURN_STRING("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"" PHP_EOL "\t\"http://www.w3.org/TR/html4/loose.dtd\">" PHP_EOL);
		/* no break */
		case 4:  RETURN_STRING("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Frameset//EN\"" PHP_EOL"\t\"http://www.w3.org/TR/html4/frameset.dtd\">" PHP_EOL);
		/* no break */
		case 5:  RETURN_STRING("<!DOCTYPE html>" PHP_EOL);
		/* no break */
		case 6:  RETURN_STRING("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"" PHP_EOL "\t\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" PHP_EOL);
		/* no break */
		case 7:  RETURN_STRING("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"" PHP_EOL "\t\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" PHP_EOL);
		/* no break */
		case 8:  RETURN_STRING("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Frameset//EN\"" PHP_EOL "\t\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd\">" PHP_EOL);
		/* no break */
		case 9:  RETURN_STRING("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"" PHP_EOL"\t\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">" PHP_EOL);
		/* no break */
		case 10: RETURN_STRING("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 2.0//EN\"" PHP_EOL "\t\"http://www.w3.org/MarkUp/DTD/xhtml2.dtd\">" PHP_EOL);
		/* no break */
		case 11: RETURN_STRING("<!DOCTYPE html>" PHP_EOL);
		/* no break */
		default: RETURN_EMPTY_STRING();
	}
}

/**
 * Builds a HTML tag
 *
 *<code>
 *	echo Phalcon\Tag::tagHtml($name, $parameters, $selfClose, $onlyStart, $eol);
 *</code>
 *
 * @param string $tagName
 * @param array $parameters
 * @param boolean $selfClose
 * @param boolean $onlyStart
 * @param boolean $useEol
 * @return string
 */
PHP_METHOD(Phalcon_Tag, tagHtml){

	zval *tag_name, *parameters = NULL, *self_close = NULL, *only_start = NULL;
	zval *use_eol = NULL, params = {}, default_params = {}, local_code = {}, doctype = {};

	phalcon_fetch_params(1, 1, 4, &tag_name, &parameters, &self_close, &only_start, &use_eol);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (!self_close) {
		self_close = &PHALCON_GLOBAL(z_false);
	}

	if (!only_start) {
		only_start = &PHALCON_GLOBAL(z_false);
	}

	if (!use_eol) {
		use_eol = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(parameters) != IS_ARRAY) {
		array_init(&params);
		PHALCON_MM_ADD_ENTRY(&params);
		phalcon_array_append(&params, parameters, PH_COPY);
	} else {
		PHALCON_MM_ZVAL_DUP(&params, parameters);
	}

	phalcon_read_static_property_ce(&default_params, phalcon_tag_ce, SL("_defaultParams"), PH_READONLY);
	if (Z_TYPE(default_params) == IS_ARRAY) {
		phalcon_array_merge_recursive_n2(&params, &default_params, PH_COPY);
	}

	PHALCON_CONCAT_SV(&local_code, "<", tag_name);
	PHALCON_MM_ADD_ENTRY(&local_code);
	phalcon_tag_render_attributes(&local_code, &params);
	PHALCON_MM_ADD_ENTRY(&local_code);
	phalcon_read_static_property_ce(&doctype, phalcon_tag_ce, SL("_documentType"), PH_READONLY);

	/**
	 * Check if Doctype is XHTML
	 */
	if (PHALCON_GT_LONG(&doctype, 5)) {
		if (zend_is_true(self_close)) {
			phalcon_concat_self_str(&local_code, SL(" />"));
		} else {
			phalcon_concat_self_str(&local_code, SL(">"));
		}
		PHALCON_MM_ADD_ENTRY(&local_code);
	} else {
		if (zend_is_true(only_start)) {
			phalcon_concat_self_str(&local_code, SL(">"));
		} else {
			PHALCON_SCONCAT_SVS(&local_code, "></", tag_name, ">");
		}
		PHALCON_MM_ADD_ENTRY(&local_code);
	}

	if (zend_is_true(use_eol)) {
		phalcon_concat_self_str(&local_code, SL(PHP_EOL));
		PHALCON_MM_ADD_ENTRY(&local_code);
	}

	RETURN_MM_CTOR(&local_code);
}

/**
 * Builds a HTML tag closing tag
 *
 *<code>
 *	echo Phalcon\Tag::tagHtmlClose('script', true)
 *</code>
 *
 * @param string $tagName
 * @param boolean $useEol
 * @return string
 */
PHP_METHOD(Phalcon_Tag, tagHtmlClose){

	zval *tag_name, *use_eol = NULL, local_code = {};

	phalcon_fetch_params(1, 1, 1, &tag_name, &use_eol);

	if (!use_eol) {
		use_eol = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_CONCAT_SVS(&local_code, "</", tag_name, ">");
	PHALCON_MM_ADD_ENTRY(&local_code);
	if (zend_is_true(use_eol)) {
		phalcon_concat_self_str(&local_code, SL(PHP_EOL));
		PHALCON_MM_ADD_ENTRY(&local_code);
	}

	RETURN_MM_CTOR(&local_code);
}
/**
 * Return default value
 *
 * @param string $name
 * @return mixed
 */
PHP_METHOD(Phalcon_Tag, getDefault){

	zval *name, display_values = {}, value = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_static_property_ce(&display_values, phalcon_tag_ce, SL("_displayValues"), PH_READONLY);

	if (!phalcon_array_isset_fetch(&value, &display_values, name, PH_READONLY)) {
		RETURN_NULL();
	}

	RETURN_CTOR(&value);
}

/**
 * Return default values
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Tag, getDefaults){


	phalcon_read_static_property_ce(return_value, phalcon_tag_ce, SL("_displayValues"), PH_COPY);
}

/**
 * Set default parameters
 *
 * @param array $params
 */
PHP_METHOD(Phalcon_Tag, setDefaultParams){

	zval *params;

	phalcon_fetch_params(0, 1, 0, &params);

	if (Z_TYPE_P(params) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_tag_exception_ce, "Default parameters must be an array");
		return;
	}

	phalcon_update_static_property_ce(phalcon_tag_ce, SL("_defaultParams"), params);
}

/**
 * Returns default params
 *
 * @return array
 */
PHP_METHOD(Phalcon_Tag, getDefaultParams)
{

	phalcon_read_static_property_ce(return_value, phalcon_tag_ce, SL("_defaultParams"), PH_COPY);
}

/**
 * Set default form parameters
 *
 * @param array $params
 */
PHP_METHOD(Phalcon_Tag, setDefaultFormParams){

	zval *params;

	phalcon_fetch_params(0, 1, 0, &params);

	if (Z_TYPE_P(params) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_tag_exception_ce, "Default parameters must be an array");
		return;
	}

	phalcon_update_static_property_ce(phalcon_tag_ce, SL("_defaultFormParams"), params);
}

/**
 * Returns default params
 *
 * @return array
 */
PHP_METHOD(Phalcon_Tag, getDefaultFormParams)
{

	phalcon_read_static_property_ce(return_value, phalcon_tag_ce, SL("_defaultFormParams"), PH_COPY);
}

PHP_METHOD(Phalcon_Tag, choice){

	zval *expression, *value1, *value2 = NULL;

	phalcon_fetch_params(0, 2, 1, &expression, &value1, &value2);

	if (!value2) {
		value2 = &PHALCON_GLOBAL(z_null);
	}

	if (zend_is_true(expression)) {
		RETURN_CTOR(value1);
	}

	RETURN_CTOR(value2);
}
