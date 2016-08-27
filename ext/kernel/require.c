
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
int phalcon_require_ret(zval *return_value_ptr, const char *require_path)
{
	zend_file_handle file_handle;
	int ret;
	zend_op_array *new_op_array;
	zval dummy, local_retval;

	ZVAL_UNDEF(&local_retval);

	file_handle.filename = require_path;
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	new_op_array = zend_compile_file(&file_handle, ZEND_REQUIRE);
	if (new_op_array) {

		if (file_handle.handle.stream.handle) {
			ZVAL_NULL(&dummy);
			if (!file_handle.opened_path) {
				file_handle.opened_path = zend_string_init(require_path, strlen(require_path), 0);
			}

			zend_hash_add(&EG(included_files), file_handle.opened_path, &dummy);
			zend_destroy_file_handle(&file_handle);
		}

#if PHP_VERSION_ID >= 70100
		new_op_array->scope = EG(fake_scope);
#else
		new_op_array->scope = EG(scope);
#endif
		zend_execute(new_op_array, &local_retval);

		if (return_value_ptr) {
			zval_ptr_dtor(return_value_ptr);
			ZVAL_COPY_VALUE(return_value_ptr, &local_retval);
		} else {
			zval_ptr_dtor(&local_retval);
		}

		destroy_op_array(new_op_array);
		efree_size(new_op_array, sizeof(zend_op_array));

		if (EG(exception)) {
			ret = FAILURE;
		} else {
			ret = SUCCESS;
		}

		return ret;
	} else {
		zend_destroy_file_handle(&file_handle);
	}

	return FAILURE;
}
