
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

#ifndef PHALCON_PROCESS_PROC_H
#define PHALCON_PROCESS_PROC_H

#include "php_phalcon.h"

#define PHALCON_PROCESS_MODE_NONE	0
#define PHALCON_PROCESS_MODE_TTY	1
#define PHALCON_PROCESS_MODE_PTY   	2

#define PHALCON_PROCESS_STATUS_READY		0
#define PHALCON_PROCESS_STATUS_STARTED		1
#define PHALCON_PROCESS_STATUS_STOP         2
#define PHALCON_PROCESS_STATUS_TERMINATED	3

#define PHALCON_PROCESS_STDIN	0
#define PHALCON_PROCESS_STDOUT	1
#define PHALCON_PROCESS_STDERR	2

extern zend_class_entry *phalcon_process_proc_ce;

PHALCON_INIT_CLASS(Phalcon_Process_Proc);

#endif /* PHALCON_PROCESS_PROC_H */
