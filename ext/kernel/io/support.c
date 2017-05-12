
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

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>

#include "kernel/io/definitions.h"
#include "kernel/io/support.h"

static int visualize  = 0;
static int debugLevel = 0;
static FILE *fdlog = NULL;

void phalcon_io_set_debug_level (int _visualize, int _debugLevel, FILE *_fdlog)
{
	visualize  = _visualize;
	debugLevel = _debugLevel;
	fdlog = _fdlog;
}

int phalcon_io_error_message (const char *format, ...)
{
	va_list args;

	va_start (args, format);
	vfprintf (stderr, format, args);
	va_end (args);
	return PHALCON_IO_ERROR;
}


int phalcon_io_debug_message(int level, const char *format, ...)
{
	va_list args;
	va_start (args, format);
	if (level<=visualize) {
		vprintf (format, args);
		fflush (stdout);
	}
	va_end (args);
	va_start (args, format);
	if (level<=debugLevel && fdlog != NULL) {
		vfprintf (fdlog, format, args);
		fflush (fdlog);
	}
	va_end (args);
	return 0;
}

static volatile unsigned long packet_count = 0;
static struct timeval present_time, previous_time;

void phalcon_io_reset_stats () {
	packet_count = 0;
	gettimeofday(&previous_time, NULL);
}

void phalcon_io_adjust_stats () {
	atomic_add_ulong(&packet_count, 1);
}

void phalcon_io_compute_stats () {
	unsigned long count = atomic_add_ulong(&packet_count, 0);
	if (count > 10000) {
	    count = atomic_and_ulong(&packet_count, 0);
		gettimeofday(&present_time, NULL);

		long long present_time_ms  = ((long long)present_time.tv_sec)*1000  + present_time.tv_usec/1000;
		long long previous_time_ms = ((long long)previous_time.tv_sec)*1000 + previous_time.tv_usec/1000;
		long long delta_time_ms = present_time_ms - previous_time_ms;
		fprintf(stderr, "%ld reqs per sec\n", (long)(count*1000/delta_time_ms));
		previous_time = present_time;
	}
}

int phalcon_io_get_processors_count ()
{
	int numCPU = sysconf (_SC_NPROCESSORS_CONF);
	return numCPU;
}


#if defined(__ARM__) || defined(__X86__)
// implementation of __sync_fetch_and_add_4 for ARM processors
unsigned int add_uint (unsigned volatile int *var, int val)
{
	unsigned volatile int v = *var;
	*var = val;
	return v;
}

unsigned long add_ulong (unsigned volatile long *var, int val)
{
	unsigned volatile long v = *var;
	*var = val;
	return v;
}

// implementation of __sync_fetch_and_add_4 for ARM processors
unsigned long and_ulong (unsigned volatile long *var, int val)
{
	unsigned volatile long v = *var;
	*var &= val;
	return v;
}
#endif	// __ARM__
