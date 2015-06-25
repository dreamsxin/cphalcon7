
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

#include "kernel/require.h"
#include "kernel/memory.h"
#include "kernel/backtrace.h"

#include <main/php_main.h>
#include <Zend/zend_hash.h>

/**
 * Do an internal require to a plain php file taking care of the value returned by the file
 */
int phalcon_require_ret(zval *return_value, const char *require_path TSRMLS_DC)
{
	zend_file_handle file_handle;
	zend_op_array *op_array;
	char realpath[MAXPATHLEN];

	if (!VCWD_REALPATH(require_path, realpath)) {
		return 0;
	}

	file_handle.filename = require_path;
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	op_array = zend_compile_file(&file_handle, ZEND_INCLUDE);

	if (op_array && file_handle.handle.stream.handle) {
	    zval dummy;
	    ZVAL_NULL(&dummy);

		if (!file_handle.opened_path) {
			file_handle.opened_path = zend_string_init(require_path, strlen(require_path)+1, 0);
		}

		zend_hash_add(&EG(included_files), file_handle.opened_path, &dummy);
	}
	zend_destroy_file_handle(&file_handle);

	if (op_array) {
        ZVAL_UNDEF(return_value);
		zend_execute(op_array, return_value);

		destroy_op_array(op_array);
		efree(op_array);
        if (!EG(exception)) {
            zval_ptr_dtor(return_value);
        }

	    return SUCCESS;
	}

	return FAILURE;
}
