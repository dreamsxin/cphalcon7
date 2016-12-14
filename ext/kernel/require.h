
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

#ifndef PHALCON_KERNEL_REQUIRE_H
#define PHALCON_KERNEL_REQUIRE_H

#include "php_phalcon.h"

zend_array *phalcon_build_symtable(zval *vars);
static inline void phalcon_destroy_symtable(zend_array *symbol_table) {
	zend_array_destroy(symbol_table);
}

int _phalcon_exec(zval* ret, zval *object, zend_op_array *op_array, zend_array *symbol_table);
int phalcon_exec_file(zval *ret, zval *object, zval *file, zval *vars);
int phalcon_exec_code(zval *ret, zval *object, zval *code, zval * vars);

int phalcon_require_ret(zval *return_value_ptr, const char *require_path) PHALCON_ATTR_NONNULL1(2);

PHALCON_ATTR_NONNULL static inline int phalcon_require(const char *require_path)
{
	return phalcon_require_ret(NULL, require_path);
}

PHALCON_ATTR_NONNULL static inline int phalcon_require_zval(const zval *require_path)
{
    return phalcon_require_ret(NULL, Z_TYPE_P(require_path) == IS_STRING ? Z_STRVAL_P(require_path) : "");
}

PHALCON_ATTR_NONNULL static inline int phalcon_require_zval_ret(zval *return_value, const zval *require_path)
{
    return phalcon_require_ret(return_value, Z_TYPE_P(require_path) == IS_STRING ? Z_STRVAL_P(require_path) : "");
}
#endif /* PHALCON_KERNEL_REQUIRE_H */
