
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
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
  |          Didier Bertrand <diblibre@gmail.com>                          |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_KERNEL_IO_SUPPORT_H
#define PHALCON_KERNEL_IO_SUPPORT_H

#include <stdio.h>

#define PHALCON_IO_DEBUG_NONE     0
#define PHALCON_IO_DEBUG_IO       1
#define PHALCON_IO_DEBUG_CLIENT   2
#define PHALCON_IO_DEBUG_DETAIL   3
#define PHALCON_IO_DEBUG_ALWAYS   4

void phalcon_io_set_debug_level( int _visualize, int _debugLevel, FILE *_fdlog );
int  phalcon_io_error_message( const char *fmt, ... );
int  phalcon_io_debug_message( int level, const char *fmt, ... );

void phalcon_io_reset_stats();
void phalcon_io_adjust_stats();
void phalcon_io_compute_stats();

int phalcon_io_get_processors_count();

#if defined(__ARM__) || defined(__X86__)
unsigned int  add_uint  (unsigned volatile int  *var, int val);
unsigned long add_ulong (unsigned volatile long *var, int val);
unsigned long and_ulong (unsigned volatile long *var, int val);
#endif

#endif /* PHALCON_KERNEL_IO_SUPPORT_H */
