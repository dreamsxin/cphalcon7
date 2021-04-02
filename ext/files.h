
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

#ifndef PHALCON_FILES_H
#define PHALCON_FILES_H

#include "php_phalcon.h"

#ifndef LOCK_EX
#	define LOCK_SH 1
#	define LOCK_EX 2
#	define LOCK_NB 4
#	define LOCK_UN 8
#endif

extern zend_class_entry *phalcon_files_ce;

PHALCON_INIT_CLASS(Phalcon_Files);

#endif /* PHALCON_FILES_H */
