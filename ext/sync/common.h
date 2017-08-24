
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

#ifndef PHALCON_SYNC_COMMON_H
#define PHALCON_SYNC_COMMON_H

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

#ifdef __APPLE__
#	include <mach/clock.h>
#	include <mach/mach.h>
#	ifndef SHM_NAME_MAX
#		define SHM_NAME_MAX 31
#	endif
#else
#	ifndef SHM_NAME_MAX
#		define SHM_NAME_MAX 255
#	endif
#endif

#ifndef INFINITE
# define INFINITE   0xFFFFFFFF
#endif

/* Some platforms are broken even for unnamed semaphores (e.g. Mac OSX). */
/* This allows for implementing all semaphores directly, bypassing POSIX semaphores. */
typedef struct _phalcon_semaphore_wrapper {
	pthread_mutex_t *MxMutex;
	volatile uint32_t *MxCount;
	volatile uint32_t *MxMax;
	pthread_cond_t *MxCond;
} phalcon_semaphore_wrapper;

/* Implements a more efficient (and portable) event object interface than trying to use semaphores. */
typedef struct _phalcon_event_wrapper {
	pthread_mutex_t *MxMutex;
	volatile char *MxManual;
	volatile char *MxSignaled;
	volatile uint32_t *MxWaiting;
	pthread_cond_t *MxCond;
} phalcon_event_wrapper;


/* POSIX pthreads. */
static inline pthread_t phalcon_getcurrent_threadid()
{
	return pthread_self();
}

static inline uint64_t phalcon_getmicrosecondtime()
{
	struct timeval TempTime;

	if (gettimeofday(&TempTime, NULL))  return 0;

	return (uint64_t)((uint64_t)TempTime.tv_sec * (uint64_t)1000000 + (uint64_t)TempTime.tv_usec);
}

static inline int phalcon_clock_gettime(struct timespec *ts)
{
#ifdef __APPLE__
	clock_serv_t cclock;
	mach_timespec_t mts;

	if (host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock) != KERN_SUCCESS)  return -1;
	if (clock_get_time(cclock, &mts) != KERN_SUCCESS)  return -1;
	if (mach_port_deallocate(mach_task_self(), cclock) != KERN_SUCCESS)  return -1;

	ts->tv_sec = mts.tv_sec;
	ts->tv_nsec = mts.tv_nsec;

	return 0;
#else
	return clock_gettime(CLOCK_REALTIME, ts);
#endif
}

static inline size_t phalcon_getsystemalignmentsize()
{
	struct {
		int MxInt;
	} x;

	struct {
		int MxInt;
		char MxChar;
	} y;

	return sizeof(y) - sizeof(x);
}

static inline size_t phalcon_getalignsize(size_t Size)
{
	size_t AlignSize = phalcon_getsystemalignmentsize();

	if (Size % AlignSize) {
		Size += AlignSize - (Size % AlignSize);
	}

	return Size;
}

static inline int phalcon_namedmem_init(char **ResultMem, size_t *StartPos, const char *Prefix, const char *Name, size_t Size)
{
	int Result = -1;
	*ResultMem = NULL;
	*StartPos = (Name != NULL ? phalcon_getalignsize(1) + phalcon_getalignsize(sizeof(pthread_mutex_t)) + phalcon_getalignsize(sizeof(uint32_t)) : 0);

	/* First byte indicates initialization status (0 = completely uninitialized, 1 = first mutex initialized, 2 = ready). */
	/* Next few bytes are a shared mutex object. */
	/* Size bytes follow for whatever. */
	Size += *StartPos;
	Size = phalcon_getalignsize(Size);

	if (Name == NULL) {
		*ResultMem = (char *)ecalloc(1, Size);

		Result = 0;
	} else {
		/* Deal with really small name limits with a pseudo-hash. */
		char Name2[SHM_NAME_MAX], Nums[50];
		size_t x, x2 = 0, y = strlen(Prefix), z = 0;

		memset(Name2, 0, sizeof(Name2));

		for (x = 0; x < y; x++) {
			Name2[x2] = (char)(((unsigned int)(unsigned char)Name2[x2]) * 37 + ((unsigned int)(unsigned char)Prefix[x]));
			x2++;

			if (x2 == sizeof(Name2) - 1)
			{
				x2 = 1;
				z++;
			}
		}

		sprintf(Nums, "-%u-%u-", (unsigned int)phalcon_getsystemalignmentsize(), (unsigned int)Size);

		y = strlen(Nums);
		for (x = 0; x < y; x++) {
			Name2[x2] = (char)(((unsigned int)(unsigned char)Name2[x2]) * 37 + ((unsigned int)(unsigned char)Nums[x]));
			x2++;

			if (x2 == sizeof(Name2) - 1) {
				x2 = 1;
				z++;
			}
		}

		y = strlen(Name);
		for (x = 0; x < y; x++) {
			Name2[x2] = (char)(((unsigned int)(unsigned char)Name2[x2]) * 37 + ((unsigned int)(unsigned char)Name[x]));
			x2++;

			if (x2 == sizeof(Name2) - 1) {
				x2 = 1;
				z++;
			}
		}

		/* Normalize the alphabet if it looped. */
		if (z) {
			unsigned char TempChr;
			y = (z > 1 ? sizeof(Name2) - 1 : x2);
			for (x = 1; x < y; x++)
			{
				TempChr = ((unsigned char)Name2[x]) & 0x3F;

				if (TempChr < 10)  TempChr += '0';
				else if (TempChr < 36)  TempChr = TempChr - 10 + 'A';
				else if (TempChr < 62)  TempChr = TempChr - 36 + 'a';
				else if (TempChr == 62)  TempChr = '_';
				else  TempChr = '-';

				Name2[x] = (char)TempChr;
			}
		}

		for (x = 1; x < sizeof(Name2) && Name2[x]; x++) {
			if (Name2[x] == '\\' || Name2[x] == '/')  Name2[x] = '_';
		}

		pthread_mutex_t *MutexPtr;
		uint32_t *RefCountPtr;

		/* Attempt to create the named shared memory object. */
		mode_t PrevMask = umask(0);
		int fp = shm_open(Name2, O_RDWR | O_CREAT | O_EXCL, 0666);
		if (fp > -1) {
			/* Ignore platform errors (for now). */
			while (ftruncate(fp, Size) < 0 && errno == EINTR) {
			}

			*ResultMem = (char *)mmap(NULL, Size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
			if ((*ResultMem) == MAP_FAILED) {
				*ResultMem = NULL;
			} else {
				pthread_mutexattr_t MutexAttr;

				pthread_mutexattr_init(&MutexAttr);
				pthread_mutexattr_setpshared(&MutexAttr, PTHREAD_PROCESS_SHARED);

				MutexPtr = (pthread_mutex_t *)((*ResultMem) + phalcon_getalignsize(1));
				RefCountPtr = (uint32_t *)((*ResultMem) + phalcon_getalignsize(1) + phalcon_getalignsize(sizeof(pthread_mutex_t)));

				pthread_mutex_init(MutexPtr, &MutexAttr);
				pthread_mutex_lock(MutexPtr);

				(*ResultMem)[0] = '\x01';
				RefCountPtr[0] = 1;

				Result = 0;
			}

			close(fp);
		} else {
			/* Attempt to open the named shared memory object. */
			fp = shm_open(Name2, O_RDWR, 0666);
			if (fp > -1) {
				/* Ignore platform errors (for now). */
				while (ftruncate(fp, Size) < 0 && errno == EINTR) {
				}

				*ResultMem = (char *)mmap(NULL, Size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
				if (*ResultMem == MAP_FAILED) {
					ResultMem = NULL;
				} else {
					/* Wait until the space is fully initialized. */
					if ((*ResultMem)[0] == '\x00') {
						while ((*ResultMem)[0] == '\x00') {
							usleep(2000);
						}
					}

					char *MemPtr = (*ResultMem) + phalcon_getalignsize(1);
					MutexPtr = (pthread_mutex_t *)(MemPtr);
					MemPtr += phalcon_getalignsize(sizeof(pthread_mutex_t));

					RefCountPtr = (uint32_t *)(MemPtr);
					MemPtr += phalcon_getalignsize(sizeof(uint32_t));

					pthread_mutex_lock(MutexPtr);

					if (RefCountPtr[0]) {
						Result = 1;
					} else {
						/* If this is the first reference, reset the RAM to 0's for platform consistency to force a rebuild of the object. */
						memset(MemPtr, 0, Size);

						Result = 0;
					}

					RefCountPtr[0]++;

					pthread_mutex_unlock(MutexPtr);
				}

				close(fp);
			}
		}

		umask(PrevMask);
	}

	return Result;
}

static inline void phalcon_namedmem_ready(char *MemPtr)
{
	pthread_mutex_unlock((pthread_mutex_t *)(MemPtr + phalcon_getalignsize(1)));
}

static inline void phalcon_namedmem_unmap(char *MemPtr, size_t Size)
{
	pthread_mutex_t *MutexPtr;
	uint32_t *RefCountPtr;

	char *MemPtr2 = MemPtr + phalcon_getalignsize(1);
	MutexPtr = (pthread_mutex_t *)(MemPtr2);
	MemPtr2 += phalcon_getalignsize(sizeof(pthread_mutex_t));

	RefCountPtr = (uint32_t *)(MemPtr2);

	pthread_mutex_lock(MutexPtr);
	if (RefCountPtr[0])  RefCountPtr[0]--;
	pthread_mutex_unlock(MutexPtr);

	munmap(MemPtr, phalcon_getalignsize(1) + phalcon_getalignsize(sizeof(pthread_mutex_t)) + phalcon_getalignsize(sizeof(uint32_t)) + Size);
}

/* Basic *NIX Semaphore functions. */
static inline size_t phalcon_semaphore_getsize()
{
	return phalcon_getalignsize(sizeof(pthread_mutex_t)) + phalcon_getalignsize(sizeof(uint32_t)) + phalcon_getalignsize(sizeof(uint32_t)) + phalcon_getalignsize(sizeof(pthread_cond_t));
}

static inline void phalcon_semaphore_get(phalcon_semaphore_wrapper *Result, char *Mem)
{
	Result->MxMutex = (pthread_mutex_t *)(Mem);
	Mem += phalcon_getalignsize(sizeof(pthread_mutex_t));

	Result->MxCount = (uint32_t *)(Mem);
	Mem += phalcon_getalignsize(sizeof(uint32_t));

	Result->MxMax = (uint32_t *)(Mem);
	Mem += phalcon_getalignsize(sizeof(uint32_t));

	Result->MxCond = (pthread_cond_t *)(Mem);
}

static inline void phalcon_semaphore_init(phalcon_semaphore_wrapper *UnixSemaphore, int Shared, uint32_t Start, uint32_t Max)
{
	pthread_mutexattr_t MutexAttr;
	pthread_condattr_t CondAttr;

	pthread_mutexattr_init(&MutexAttr);
	pthread_condattr_init(&CondAttr);

	if (Shared) {
		pthread_mutexattr_setpshared(&MutexAttr, PTHREAD_PROCESS_SHARED);
		pthread_condattr_setpshared(&CondAttr, PTHREAD_PROCESS_SHARED);
	}

	pthread_mutex_init(UnixSemaphore->MxMutex, &MutexAttr);
	if (Start > Max) {
		Start = Max;
	}
	UnixSemaphore->MxCount[0] = Start;
	UnixSemaphore->MxMax[0] = Max;
	pthread_cond_init(UnixSemaphore->MxCond, &CondAttr);

	pthread_condattr_destroy(&CondAttr);
	pthread_mutexattr_destroy(&MutexAttr);
}

static inline int phalcon_semaphore_wait(phalcon_semaphore_wrapper *UnixSemaphore, uint32_t Wait)
{
	if (Wait == 0) {
		/* Avoid the scenario of deadlock on the semaphore itself for 0 wait. */
		if (pthread_mutex_trylock(UnixSemaphore->MxMutex) != 0)  return 0;
	}
	else {
		if (pthread_mutex_lock(UnixSemaphore->MxMutex) != 0)  return 0;
	}

	int Result = 0;

	if (UnixSemaphore->MxCount[0]) {
		UnixSemaphore->MxCount[0]--;

		Result = 1;
	} else if (Wait == INFINITE) {
		int Result2;
		do {
			Result2 = pthread_cond_wait(UnixSemaphore->MxCond, UnixSemaphore->MxMutex);
			if (Result2 != 0)  break;
		} while (!UnixSemaphore->MxCount[0]);

		if (Result2 == 0) {
			UnixSemaphore->MxCount[0]--;
			Result = 1;
		}
	} else if (Wait == 0) {
		/* Failed to obtain lock.  Nothing to do. */
	} else {
		struct timespec TempTime;

		if (phalcon_clock_gettime(&TempTime) == -1)  return 0;
		TempTime.tv_sec += Wait / 1000;
		TempTime.tv_nsec += (Wait % 1000) * 1000000;
		TempTime.tv_sec += TempTime.tv_nsec / 1000000000;
		TempTime.tv_nsec = TempTime.tv_nsec % 1000000000;

		int Result2;
		do {
			/* Some platforms have pthread_cond_timedwait() but not pthread_mutex_timedlock() or sem_timedwait() (e.g. Mac OSX). */
			Result2 = pthread_cond_timedwait(UnixSemaphore->MxCond, UnixSemaphore->MxMutex, &TempTime);
			if (Result2 != 0)  break;
		} while (!UnixSemaphore->MxCount[0]);

		if (Result2 == 0) {
			UnixSemaphore->MxCount[0]--;

			Result = 1;
		}
	}

	pthread_mutex_unlock(UnixSemaphore->MxMutex);

	return Result;
}

static inline int phalcon_semaphore_release(phalcon_semaphore_wrapper *UnixSemaphore, uint32_t *PrevVal)
{
	if (pthread_mutex_lock(UnixSemaphore->MxMutex) != 0)  return 0;

	if (PrevVal != NULL)  *PrevVal = UnixSemaphore->MxCount[0];
	UnixSemaphore->MxCount[0]++;
	if (UnixSemaphore->MxCount[0] > UnixSemaphore->MxMax[0])  UnixSemaphore->MxCount[0] = UnixSemaphore->MxMax[0];

	/* Let a waiting thread have at it. */
	pthread_cond_signal(UnixSemaphore->MxCond);

	pthread_mutex_unlock(UnixSemaphore->MxMutex);

	return 1;
}

static inline void phalcon_semaphore_free(phalcon_semaphore_wrapper *UnixSemaphore)
{
	pthread_mutex_destroy(UnixSemaphore->MxMutex);
	pthread_cond_destroy(UnixSemaphore->MxCond);
}

/* Basic *NIX Event functions. */
static inline size_t phalcon_event_getsize()
{
	return phalcon_getalignsize(sizeof(pthread_mutex_t)) + phalcon_getalignsize(2) + phalcon_getalignsize(sizeof(uint32_t)) + phalcon_getalignsize(sizeof(pthread_cond_t));
}

static inline void phalcon_event_get(phalcon_event_wrapper *Result, char *Mem)
{
	Result->MxMutex = (pthread_mutex_t *)(Mem);
	Mem += phalcon_getalignsize(sizeof(pthread_mutex_t));

	Result->MxManual = Mem;
	Result->MxSignaled = Mem + 1;
	Mem += phalcon_getalignsize(2);

	Result->MxWaiting = (uint32_t *)(Mem);
	Mem += phalcon_getalignsize(sizeof(uint32_t));

	Result->MxCond = (pthread_cond_t *)(Mem);
}

static inline void phalcon_event_init(phalcon_event_wrapper *UnixEvent, int Shared, int Manual, int Signaled)
{
	pthread_mutexattr_t MutexAttr;
	pthread_condattr_t CondAttr;

	pthread_mutexattr_init(&MutexAttr);
	pthread_condattr_init(&CondAttr);

	if (Shared) {
		pthread_mutexattr_setpshared(&MutexAttr, PTHREAD_PROCESS_SHARED);
		pthread_condattr_setpshared(&CondAttr, PTHREAD_PROCESS_SHARED);
	}

	pthread_mutex_init(UnixEvent->MxMutex, &MutexAttr);
	UnixEvent->MxManual[0] = (Manual ? '\x01' : '\x00');
	UnixEvent->MxSignaled[0] = (Signaled ? '\x01' : '\x00');
	UnixEvent->MxWaiting[0] = 0;
	pthread_cond_init(UnixEvent->MxCond, &CondAttr);

	pthread_condattr_destroy(&CondAttr);
	pthread_mutexattr_destroy(&MutexAttr);
}

static inline int phalcon_event_wait(phalcon_event_wrapper *UnixEvent, uint32_t Wait)
{
	if (Wait == 0) {
		/* Avoid the scenario of deadlock on the semaphore itself for 0 wait. */
		if (pthread_mutex_trylock(UnixEvent->MxMutex) != 0)  return 0;
	} else {
		if (pthread_mutex_lock(UnixEvent->MxMutex) != 0)  return 0;
	}

	int Result = 0;

	/* Avoid a potential starvation issue by only allowing signaled manual events OR if there are no other waiting threads. */
	if (UnixEvent->MxSignaled[0] != '\x00' && (UnixEvent->MxManual[0] != '\x00' || !UnixEvent->MxWaiting[0])) {
		/* Reset auto events. */
		if (UnixEvent->MxManual[0] == '\x00')  UnixEvent->MxSignaled[0] = '\x00';

		Result = 1;
	} else if (Wait == INFINITE) {
		UnixEvent->MxWaiting[0]++;

		int Result2;
		do {
			Result2 = pthread_cond_wait(UnixEvent->MxCond, UnixEvent->MxMutex);
			if (Result2 != 0)  break;
		} while (UnixEvent->MxSignaled[0] == '\x00');

		UnixEvent->MxWaiting[0]--;

		if (Result2 == 0) {
			/* Reset auto events. */
			if (UnixEvent->MxManual[0] == '\x00')  UnixEvent->MxSignaled[0] = '\x00';

			Result = 1;
		}
	} else if (Wait == 0) {
		/* Failed to obtain lock.  Nothing to do. */
	} else {
		struct timespec TempTime;

		if (phalcon_clock_gettime(&TempTime) == -1) {
			pthread_mutex_unlock(UnixEvent->MxMutex);
			return 0;
		}

		TempTime.tv_sec += Wait / 1000;
		TempTime.tv_nsec += (Wait % 1000) * 1000000;
		TempTime.tv_sec += TempTime.tv_nsec / 1000000000;
		TempTime.tv_nsec = TempTime.tv_nsec % 1000000000;

		UnixEvent->MxWaiting[0]++;

		int Result2;
		do {
			/* Some platforms have pthread_cond_timedwait() but not pthread_mutex_timedlock() or sem_timedwait() (e.g. Mac OSX). */
			Result2 = pthread_cond_timedwait(UnixEvent->MxCond, UnixEvent->MxMutex, &TempTime);
			if (Result2 != 0)  break;
		} while (UnixEvent->MxSignaled[0] == '\x00');

		UnixEvent->MxWaiting[0]--;

		if (Result2 == 0) {
			/* Reset auto events. */
			if (UnixEvent->MxManual[0] == '\x00')  UnixEvent->MxSignaled[0] = '\x00';

			Result = 1;
		}
	}

	pthread_mutex_unlock(UnixEvent->MxMutex);

	return Result;
}

static inline int phalcon_event_fire(phalcon_event_wrapper *UnixEvent)
{
	if (pthread_mutex_lock(UnixEvent->MxMutex) != 0)  return 0;

	UnixEvent->MxSignaled[0] = '\x01';

	/* Let all waiting threads through for manual events, otherwise just one waiting thread (if any). */
	if (UnixEvent->MxManual[0] != '\x00')  pthread_cond_broadcast(UnixEvent->MxCond);
	else  pthread_cond_signal(UnixEvent->MxCond);

	pthread_mutex_unlock(UnixEvent->MxMutex);

	return 1;
}

/* Only call for manual events. */
static inline int phalcon_event_reset(phalcon_event_wrapper *UnixEvent)
{
	if (UnixEvent->MxManual[0] == '\x00')  return 0;
	if (pthread_mutex_lock(UnixEvent->MxMutex) != 0)  return 0;

	UnixEvent->MxSignaled[0] = '\x00';

	pthread_mutex_unlock(UnixEvent->MxMutex);

	return 1;
}

static inline void phalcon_event_free(phalcon_event_wrapper *UnixEvent)
{
	pthread_mutex_destroy(UnixEvent->MxMutex);
	pthread_cond_destroy(UnixEvent->MxCond);
}

#endif /* PHALCON_SYNC_COMMON_H */
