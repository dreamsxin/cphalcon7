
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
int phalcon_require_ret(zval *result, const char *require_path)
{
	zval retval, dummy;
	zend_file_handle file_handle;
	zend_op_array *new_op_array;
	int ret;

	result = result ? result : &retval;
	ret = php_stream_open_for_zend_ex(require_path, &file_handle, USE_PATH|STREAM_OPEN_FOR_INCLUDE);

	if (ret == SUCCESS) {
		zend_string *opened_path;
		if (!file_handle.opened_path) {
			file_handle.opened_path = zend_string_init(require_path, strlen(require_path), 0);
		}
		opened_path = zend_string_copy(file_handle.opened_path);
		ZVAL_NULL(&dummy);
		if (zend_hash_add(&EG(included_files), opened_path, &dummy)) {
			new_op_array = zend_compile_file(&file_handle, ZEND_REQUIRE);
			zend_destroy_file_handle(&file_handle);
		} else {
			new_op_array = NULL;
			zend_file_handle_dtor(&file_handle);
		}
		zend_string_release(opened_path);
		if (new_op_array) {
			ZVAL_UNDEF(result);
			zend_execute(new_op_array, result);

			destroy_op_array(new_op_array);
			efree(new_op_array);
			if (!EG(exception)) {
				//zval_ptr_dtor(result);
			}

			return SUCCESS;
		}
	}

	return FAILURE;
}
