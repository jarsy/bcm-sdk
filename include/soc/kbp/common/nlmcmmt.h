/*
 * $Id: nlmcmmt.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#ifndef _NLMCM_MT_
#define _NLMCM_MT_

#if defined NLM_MT_OLD || defined NLM_MT

#ifndef NLMPLATFORM_BCM
#include "nlmcmportable.h"
#include "nlmcmbasic.h"
#include "nlmerrorcodes.h"
#else
#include <soc/kbp/common/nlmcmportable.h>
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmerrorcodes.h>
#endif

#ifdef NLM_POSIX
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#endif

#ifdef NLM_NETOS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <mempool.h>

#endif

#ifdef NLMPLATFORM_BCM
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sal/core/thread.h>
#include <sal/core/sync.h>
#include <sal/core/spl.h>
#endif /* ifdef NLMPLATFORM_BCM */


#define NLMNS_MT_MAXNUM_THREADS (8)


#ifdef NLM_MT_DEBUG

#define NLMNS_MT_PRINT1(arg1)                      NlmCm__printf(arg1)
#define NLMNS_MT_PRINT2(arg1, arg2)                NlmCm__printf(arg1, arg2)
#define NLMNS_MT_PRINT3(arg1, arg2, arg3)          NlmCm__printf(arg1, arg2, arg3)
#define NLMNS_MT_PRINT4(arg1, arg2, arg3, arg4)    NlmCm__printf(arg1, arg2, arg3, arg4)

#else

#define NLMNS_MT_PRINT1(arg1)
#define NLMNS_MT_PRINT2(arg1, arg2)
#define NLMNS_MT_PRINT3(arg1, arg2, arg3)
#define NLMNS_MT_PRINT4(arg1, arg2, arg3, arg4)

#endif /* NLM_MT_DEBUG */


#ifdef NLM_POSIX
extern __thread nlm_u32 threadID;

#define   NlmCmMtThreadId       pthread_t
typedef pthread_mutex_t NlmCmMtMutex;
typedef sem_t NlmCmMtSem;
typedef pthread_spinlock_t NlmCmMtSpinlock;

#define NlmCmMtFlag PTHREAD_PROCESS_PRIVATE


#elif NLM_NETOS
typedef int NlmCmMtThreadId;
typedef sem_t* NlmCmMtMutex;
typedef sem_t* NlmCmMtSem;
typedef sem_t* NlmCmMtSpinlock;

#define CLONE_BUDDY_FLGS (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD )
#define NlmCmMtFlag (O_CREAT | O_EXCL )

extern nlm_u32 threadIdArray[];

#endif

#ifdef WIN32
typedef HANDLE NlmCmMtThreadId;
typedef HANDLE NlmCmMtSpinlock;
#endif


#ifdef NLMPLATFORM_BCM
extern __thread nlm_u32 threadID;
typedef sal_thread_t NlmCmMtThreadId;
typedef sal_mutex_t NlmCmMtMutex;
typedef sal_sem_t NlmCmMtSem;

typedef int NlmCmMtSpinlock; /* It is level in case of SAL*/

#define NlmCmMtFlag PTHREAD_PROCESS_PRIVATE

extern nlm_u32 threadIdArray[];

#endif /* #ifdef NLMPLATFORM_BCM */



NlmErrNum_t
NlmCmMt__RegisterThread(
        nlm_u32 thNum,
        nlm_u32 cpuId,
        NlmReasonCode* o_reason);


static inline nlm_32
NlmCmMt__GetThreadID(nlm_u32 cpuId)
{
#if defined NLM_NETOS || defined NLMPLATFORM_BCM
    nlm_32 i = 0;

    for(i = 0; i < NLMNS_MT_MAXNUM_THREADS + 1; i++)
        if(threadIdArray[i] == cpuId)
            return i;

    NlmCmAssert(0, "!!!!Could not found mapping for CPU Id = %d !!!!\n");
    return -1;
#else
    return threadID;
#endif

}


static inline NlmErrNum_t
NlmCmMt__CreateThread(
                    NlmCmMtThreadId* threadId,
                    void* attrs,
                    void* (*startRoutine)(void *) ,
                    void* args)
{
    NlmErrNum_t errNum = NLMERR_OK;

#ifndef NLMPLATFORM_BCM 
    nlm_32 retVal = 0;
#endif

#ifdef NLM_POSIX
    retVal = pthread_create(threadId, attrs, startRoutine, args);
    if(retVal != 0)
        errNum = NLMERR_FAIL;
#endif

#ifdef NLM_NETOS
    (void) retVal;
    *threadId  = clone(startRoutine, attrs, CLONE_BUDDY_FLGS, args);
    if(*threadId == -1)
        errNum = NLMERR_FAIL;
#endif

#ifdef NLMPLATFORM_BCM
#define KBP_SAL_THREAD_PRIO (50)

    *threadId = sal_thread_create((char*)attrs, /* "kbp_sal_thread", */
              SAL_THREAD_STKSZ, KBP_SAL_THREAD_PRIO, (void*)startRoutine, args);

    if (*threadId == SAL_THREAD_ERROR)
       errNum = NLMERR_FAIL;

#endif
    return errNum;
}

static inline NlmErrNum_t
NlmCmMt__JoinThread(
                    NlmCmMtThreadId* threadId,
                    void* param)
{
    NlmErrNum_t errNum = NLMERR_OK;

    (void) param;

#ifdef NLM_POSIX
    pthread_join(*threadId, NULL);
#endif

#ifdef NLM_NETOS
    (void)threadId;

#endif

#ifdef NLMPLATFORM_BCM
    (void)threadId;

#endif

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__SemInit(
                    NlmCmMtSem* sem,
                    const char* semName,
                    int flags,
                    int value)
{
    NlmErrNum_t errNum = NLMERR_OK;

#ifndef NLMPLATFORM_BCM
    nlm_32 retVal = -1;
#endif

#ifdef NLM_POSIX
    retVal = sem_init( sem, flags, value);
    if(retVal != 0)
        errNum = NLMERR_FAIL;

#endif

#ifdef NLM_NETOS
    (void)retVal;
    *sem = sem_open(semName, flags, 0, value);
    if(*sem == NULL)
        errNum = NLMERR_FAIL;
#endif

#ifdef NLMPLATFORM_BCM
    *sem = sal_sem_create((char *)semName, 
                sal_sem_COUNTING, value);

    if(*sem == NULL)
        errNum = NLMERR_FAIL;

#endif

    return errNum;
}

static inline NlmErrNum_t
NlmCmMt__SemPost(
                    NlmCmMtSem* sem)
{
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_32 retVal = -1;

#ifdef NLM_POSIX
    retVal = sem_post(sem);
#endif
#ifdef NLM_NETOS
    retVal = sem_post(*sem);
#endif

#ifdef NLMPLATFORM_BCM
    retVal = sal_sem_give(*sem);
#endif

    if(retVal != 0)
        errNum = NLMERR_FAIL;

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__SemWait(
                    NlmCmMtSem* sem)
{
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_32 retVal = -1;

#ifdef NLM_POSIX
    retVal = sem_wait(sem);
#endif
#ifdef NLM_NETOS
    retVal = sem_wait(*sem);
#endif

#ifdef NLMPLATFORM_BCM
  retVal = sal_sem_take(*sem, sal_sem_FOREVER);
#endif

    if(retVal != 0)
        errNum = NLMERR_FAIL;

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__SemDestroy(
                    NlmCmMtSem* sem,
                    const char* semName)
{
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_32 retVal = -1;

#ifdef NLM_POSIX
    retVal = sem_destroy( sem);
#endif

#ifdef NLM_NETOS
    retVal = sem_unlink(semName);
#endif

#ifdef NLMPLATFORM_BCM
      sal_sem_destroy(*sem);
    retVal = 0;
#endif

    if(retVal != 0)
        errNum = NLMERR_FAIL;

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__MutexInit(
                    NlmCmMtMutex* mutex,
                    const char* mutexName,
                    int flags)
{
    NlmErrNum_t errNum = NLMERR_OK;
    
#ifndef NLMPLATFORM_BCM 
    nlm_32 retVal = -1;
#endif


#ifdef NLM_POSIX
    retVal = pthread_mutex_init( mutex, NULL);
    if(retVal != 0)
        errNum = NLMERR_FAIL;

#endif

#ifdef NLM_NETOS
    (void)retVal;
    *mutex = sem_open(mutexName, flags, 0, 1);
    if(*mutex == NULL)
        errNum = NLMERR_FAIL;
#endif

#ifdef NLMPLATFORM_BCM
    *mutex = sal_mutex_create((char*)mutexName);
    if(*mutex == NULL)
        errNum = NLMERR_FAIL;

#endif

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__MutexLock(
                    NlmCmMtMutex* mutex)
{
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_32 retVal = -1;

#ifdef NLM_POSIX
    retVal = pthread_mutex_lock(mutex);
#endif

#ifdef NLM_NETOS
    retVal = sem_wait(*mutex);
#endif

#ifdef NLMPLATFORM_BCM
    retVal = sal_mutex_take(*mutex, sal_sem_FOREVER);
#endif

    if(retVal != 0)
        errNum = NLMERR_FAIL;

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__MutexUnlock(
                    NlmCmMtMutex* mutex)
{
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_32 retVal = -1;

#ifdef NLM_POSIX
    retVal = pthread_mutex_unlock(mutex);
#endif

#ifdef NLM_NETOS
    retVal = sem_post(*mutex);
#endif

#ifdef NLMPLATFORM_BCM
    retVal = sal_mutex_give(*mutex);
#endif

    if(retVal != 0)
        errNum = NLMERR_FAIL;

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__MutexDestroy(
                    NlmCmMtMutex* mutex,
                    const char* mutexName)
{
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_32 retVal = -1;

#ifdef NLM_POSIX
    retVal = pthread_mutex_destroy(mutex);
#endif

#ifdef NLM_NETOS
    retVal = sem_unlink(mutexName);
#endif

#ifdef NLMPLATFORM_BCM
    sal_mutex_destroy(*mutex);
    retVal = 0;
#endif


    if(retVal != 0)
        errNum = NLMERR_FAIL;

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__SpinInit(
                    NlmCmMtSpinlock* spinlock,
                    const char* mutexName,
                    int flags)
{
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_32 retVal = -1;

#ifdef NLM_POSIX
    retVal = pthread_spin_init( spinlock, flags);
    if(retVal != 0)
        errNum = NLMERR_FAIL;

#endif

#ifdef NLM_NETOS
    (void)retVal;
    *spinlock = sem_open(mutexName, flags, 0, 1);
    if(*spinlock == NULL)
        errNum = NLMERR_FAIL;
#endif


#ifdef WIN32
    *spinlock = CreateMutex(NULL,  // default security attributes
                        FALSE,                  // initially not owned
                        NULL);
    (void)retVal;
    if(*spinlock == NULL)
        errNum = NLMERR_FAIL;

#endif

#ifdef NLMPLATFORM_BCM
    /* We have only one spin lock at SAL level which is initialized at 
    the time of boot */
    retVal = 0; 
#endif

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__SpinLock(
                    NlmCmMtSpinlock* spinlock)
{
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_32 retVal = -1;

#ifdef NLM_POSIX
    retVal = pthread_spin_lock(spinlock);
#endif

#ifdef NLM_NETOS
    do
    {
        retVal = sem_trywait(*spinlock);
    }while(retVal != 0);
#endif

#ifdef WIN32
    retVal = WaitForSingleObject(*spinlock, INFINITE);
    retVal = 0; /* Check the return value of Wait For Single Object */
#endif

#ifdef NLMPLATFORM_BCM
    *spinlock = sal_splhi();
    retVal = 0;
#endif


    if(retVal != 0)
        errNum = NLMERR_FAIL;

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__SpinUnlock(
                    NlmCmMtSpinlock* spinlock)
{
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_32 retVal = -1;

#ifdef NLM_POSIX
    retVal = pthread_spin_unlock(spinlock);
#endif

#ifdef NLM_NETOS
    retVal = sem_post(*spinlock);
#endif

#ifdef WIN32
        retVal = ReleaseMutex(*spinlock);
        retVal = 0; /* Check the return value of Wait For Single Object */
#endif

#ifdef NLMPLATFORM_BCM
        sal_spl(*spinlock);
        retVal = 0;
#endif

    if(retVal != 0)
        errNum = NLMERR_FAIL;

    return errNum;
}


static inline NlmErrNum_t
NlmCmMt__SpinDestroy(
                    NlmCmMtSpinlock* spinlock,
                    const char* spinName)
{
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_32 retVal = -1;

#ifdef NLM_POSIX
    retVal = pthread_spin_destroy(spinlock);
#endif

#ifdef NLM_NETOS
    retVal = sem_unlink(spinName);
#endif

#ifdef NLMPLATFORM_BCM
    retVal = 0;
#endif

    if(retVal != 0)
        errNum = NLMERR_FAIL;

    return errNum;
}

#endif /* if defined NLM_MT_OLD || defined NLM_MT */

#endif /* _NLMCM_MT_ */

