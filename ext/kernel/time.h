
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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_KERNEL_TIME_H
#define PHALCON_KERNEL_TIME_H

#include <php.h>
#include <Zend/zend.h>

#include <sys/time.h>
#include <sys/resource.h>

#if __APPLE__
# include <mach/mach_init.h>
# include <mach/mach_time.h>
#endif

#define MICRO_IN_SEC 1000000.00

void phalcon_time(zval *return_value);
int phalcon_get_time();

#ifdef HAVE_GETTIMEOFDAY
void phalcon_microtime(zval *return_value, zval *get_as_float);
double phalcon_get_microtime();
#endif

#define PHALCON_CLOCK_CGT 0
#define PHALCON_CLOCK_GTOD 1
#define PHALCON_CLOCK_TSC 2
#define PHALCON_CLOCK_MACH 3
#define PHALCON_CLOCK_QPC 4
#define PHALCON_CLOCK_NONE 255

static zend_always_inline uint64 phalcon_current_timestamp() {
    struct timeval tv;

    if (gettimeofday(&tv, NULL)) {
        php_error(E_ERROR, "tracer: Cannot acquire gettimeofday");
        return 0;
    }

    return 1000 * (uint64) tv.tv_sec + (uint64) tv.tv_usec / 1000;
}

/**
 * Get the current wallclock timer
 *
 * @return 64 bit unsigned integer
 */
static zend_always_inline uint64 phalcon_time_milliseconds(int source, double timebase_factor) {
#ifdef __APPLE__
    return mach_absolute_time() / timebase_factor;
#else
    uint32 a, d;
    uint64 val;

    switch (source) {
# if HAVE_CLOCK_GETTIME
        case PHALCON_CLOCK_CGT:
		{
			struct timespec s;
            if (clock_gettime(CLOCK_MONOTONIC, &s) == 0) {
                return s.tv_sec * 1000000 + s.tv_nsec / 1000;
            } else {
                struct timeval now;
                if (gettimeofday(&now, NULL) == 0) {
                    return now.tv_sec * 1000000 + now.tv_usec;
                }
            }
            return 0;
		}
# elif HAVE_GETTIMEOFDAY
        case PHALCON_CLOCK_GTOD:
		{
            struct timeval now;
            if (gettimeofday(&now, NULL) == 0) {
                return now.tv_sec * 1000000 + now.tv_usec;
            }
            return 0;
		}
# endif
        case PHALCON_CLOCK_TSC:
#if defined(__i386__)
            int64_t ret;
            __asm__ volatile("rdtsc" : "=A"(ret));
            return ret;
#elif defined(__x86_64__) || defined(__amd64__)
            asm volatile("rdtsc" : "=a" (a), "=d" (d));
            (val) = ((uint64)a) | (((uint64)d)<<32);
#elif defined(__powerpc__) || defined(__ppc__)
            asm volatile ("mftb %0" : "=r" (val));
#else
#error You need to define CycleTimer for your OS and CPU
#endif
            return val / timebase_factor;

        default:
			zend_error(E_WARNING, "Unsupported wallclock timer");
            return 0;
    }
#endif
}

/**
 * Get time delta in microseconds.
 */
static zend_always_inline long phalcon_get_us_interval(struct timeval *start, struct timeval *end)
{
    return (((end->tv_sec - start->tv_sec) * 1000000) + (end->tv_usec - start->tv_usec));
}

/**
 * Get the timebase factor necessary to divide by in time_milliseconds()
 */
static zend_always_inline double phalcon_get_timebase_factor(int source)
{
#ifdef __APPLE__
    mach_timebase_info_data_t sTimebaseInfo;
    (void) mach_timebase_info(&sTimebaseInfo);

    return (sTimebaseInfo.numer / sTimebaseInfo.denom) * 1000;
#else
    struct timeval start;
    struct timeval end;
    uint64 tsc_start;
    uint64 tsc_end;
    volatile int i;

    switch (source) {
        case PHALCON_CLOCK_TSC:

            if (gettimeofday(&start, 0)) {
                perror("gettimeofday");
                return 0.0;
            }

            tsc_start  = phalcon_time_milliseconds(source, 1.0);
            /* Busy loop for 5 miliseconds. */
            do {
                for (i = 0; i < 1000000; i++);
                    if (gettimeofday(&end, 0)) {
                        perror("gettimeofday");
                        return 0.0;
                    }
                tsc_end = phalcon_time_milliseconds(source, 1.0);
            } while (phalcon_get_us_interval(&start, &end) < 5000);

            return (tsc_end - tsc_start) * 1.0 / (phalcon_get_us_interval(&start, &end));
        default:
            return 1.0;
    }
#endif
}

/**
 * Get the current real CPU clock timer
 */
static zend_always_inline uint64 phalcon_cpu_timer() {
    struct rusage ru;
#if defined(CLOCK_PROCESS_CPUTIME_ID)
    struct timespec s;

    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &s) == 0) {
        return s.tv_sec * 1000000 + s.tv_nsec / 1000;
    }
#endif

    if (getrusage(RUSAGE_SELF, &ru) == 0) {
        return ru.ru_utime.tv_sec * 1000000 + ru.ru_utime.tv_usec +
            ru.ru_stime.tv_sec * 1000000 + ru.ru_stime.tv_usec;
    }

    return 0;
}

#endif /* PHALCON_KERNEL_TIME_H */
