/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Martin Schröder <m.schroeder2007@gmail.com>                 |
  +----------------------------------------------------------------------+
*/

#ifndef ASYNC_FIBER_STACK_H
#define ASYNC_FIBER_STACK_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

typedef struct _async_fiber_stack {
	void *pointer;
	size_t size;

#ifdef HAVE_VALGRIND
	int valgrind;
#endif
} async_fiber_stack;

zend_bool async_fiber_stack_allocate(async_fiber_stack *stack, unsigned int size);
void async_fiber_stack_free(async_fiber_stack *stack);

#if _POSIX_MAPPED_FILES
#define HAVE_MMAP 1

#include <sys/mman.h>
#include <limits.h>

#ifndef MAP_ANONYMOUS
#ifdef MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#else
#undef HAVE_MMAP
#endif
#endif

#endif

#if _POSIX_MEMORY_PROTECTION
#define ASYNC_FIBER_GUARDPAGES 4
#endif

#ifndef ASYNC_FIBER_GUARDPAGES
#define ASYNC_FIBER_GUARDPAGES 0
#endif

#ifdef HAVE_MMAP
#define ASYNC_STACK_PAGESIZE sysconf(_SC_PAGESIZE)
#else
#define ASYNC_STACK_PAGESIZE 4096
#endif

#endif
