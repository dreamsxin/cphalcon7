
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
int phalcon_require_ret(zval *return_value, const char *require_path)
{
	zval ret;
	zend_file_handle file_handle;
	zend_op_array *op_array;
	char realpath[MAXPATHLEN];

	if (!VCWD_REALPATH(require_path, realpath)) {
		return FAILURE;
	}

	file_handle.filename = require_path;
	file_handle.free_filename = FAILURE;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	op_array = zend_compile_file(&file_handle, ZEND_INCLUDE);

	if (op_array && file_handle.handle.stream.handle) {
		if (!file_handle.opened_path) {
			file_handle.opened_path = zend_string_init(require_path, strlen(require_path), 0);
		}

		zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path);
	}
	zend_destroy_file_handle(&file_handle);

	if (op_array) {
		ZVAL_UNDEF(&ret);
		zend_execute(op_array, &ret);
		zend_exception_restore();
		destroy_op_array(op_array);
		efree(op_array);

        if (EG(exception)) {
            zval_ptr_dtor(&ret);
			return FAILURE;
        }
		if (return_value) {
			ZVAL_COPY(return_value, &ret);
		}

	    return SUCCESS;
	}

	return FAILURE;
}
