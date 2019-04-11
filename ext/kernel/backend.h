
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

#ifndef PHALCON_KERNEL_BACKEND_H
#define PHALCON_KERNEL_BACKEND_H

#include <Zend/zend_types.h>

#ifndef IS_VOID
#define IS_VOID 0
#endif

#ifndef ZEND_ARG_VARIADIC_TYPE_INFO
#define ZEND_ARG_VARIADIC_TYPE_INFO(pass_by_ref, name, type_hint, allow_null) { #name, NULL, type_hint, pass_by_ref, allow_null, 1 },
#endif

#ifndef ZEND_ARG_VARIADIC_OBJ_INFO
# define ZEND_ARG_VARIADIC_OBJ_INFO(pass_by_ref, name, classname, allow_null)  { #name, #classname, IS_OBJECT, pass_by_ref, allow_null, 1 },
#endif

#ifndef ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX
# define ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(name, return_reference, required_num_args, class_name, allow_null) \
	static const zend_internal_arg_info name[] = { \
	   	{ (const char*)(zend_uintptr_t)(required_num_args), #class_name, IS_OBJECT, return_reference, allow_null, 0 },
#endif

#ifndef ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO
#define ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO(name, class_name, allow_null) \
	ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(name, 0, -1, class_name, allow_null)
#endif

#endif