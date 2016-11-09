
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

#include "binary.h"
#include "di.h"
#include "filterinterface.h"

#include <ext/standard/php_array.h>
#include <ext/spl/spl_array.h>

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/exception.h"

#include "interned-strings.h"

/**
 * Phalcon\Binary
 *
 * Provides utilities to work with binary data
 */
zend_class_entry *phalcon_binary_ce;

/**
 * Phalcon\Binary initializer
 */
PHALCON_INIT_CLASS(Phalcon_Binary){

	PHALCON_REGISTER_CLASS(Phalcon, Binary, binary, NULL, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_class_constant_long(phalcon_binary_ce, SL("ENDIAN_MACHINE"), PHALCON_BINARY_ENDIAN_MACHINE);
	zend_declare_class_constant_long(phalcon_binary_ce, SL("ENDIAN_BIG"), PHALCON_BINARY_ENDIAN_BIG);
	zend_declare_class_constant_long(phalcon_binary_ce, SL("ENDIAN_LITTLE"), PHALCON_BINARY_ENDIAN_LITTLE);
	return SUCCESS;
}
