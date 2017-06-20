/*
 * $Id: nlmcmportable.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include <nlmcmbasic.h>
#include <nlmcmportable.h>


#ifndef NLMPLATFORM_BCM

/* WINDOWS PLATFORM */
/* ------------------------------------------------------------------------- */
#ifdef NLMPLATFORM_WIN32  

void NlmCm__win32_abort(void)
{
    abort();
}

int NlmCm__win32_gettimeofday(
    NlmCmClock__timeval* tv)
{
    static const int cvt_100ns_to_sec = 10 * 1000 * 1000;
    static const int cvt_100ns_to_nano = 100 ;
    ULARGE_INTEGER timeDiff ;

    ULARGE_INTEGER epoch ;
    ULARGE_INTEGER cur ;

    FILETIME curFileTime ;
    FILETIME epochFileTime ;

    SYSTEMTIME curTime ;
    /* Unix Epoch as a Win32 SYSTEMTIME structure (UTC) */
    static const SYSTEMTIME epochTime =
    {
        1970,   /* year */
        1,      /* month (1 = January) */
        4,      /* day of week (0 = Sunday, epoch was a Thursday) */
        1,      /* day of month (1 = 1st of month) */
        0,      /* hour */
        0,      /* minute */
        0,      /* second */
        0       /* millisecond */
    } ;

    /* The current time (UTC) */
    GetSystemTime(&curTime) ;

    /* Convert the times to FILETIME structures */
    SystemTimeToFileTime(&epochTime, &epochFileTime) ;
    SystemTimeToFileTime(&curTime, &curFileTime) ;

    /* Convert the FILETIME structures to ULARGE_INTEGER unions */
    epoch.LowPart = epochFileTime.dwLowDateTime ;
    epoch.HighPart = epochFileTime.dwHighDateTime ;

    cur.LowPart = curFileTime.dwLowDateTime ;
    cur.HighPart = curFileTime.dwHighDateTime ;

    /* Access the 64bit integer portion of the union, and calculate 
     * difference between current time and the Epoch. 
     */
    timeDiff.QuadPart = cur.QuadPart - epoch.QuadPart ;

    /* strip out >= 1 sec portion */
    tv->tv_sec = (long)(timeDiff.QuadPart / cvt_100ns_to_sec) ;

    /* strip out < 1 sec portion and convert to usec */
    tv->tv_ns = (long)((timeDiff.QuadPart % cvt_100ns_to_sec) * cvt_100ns_to_nano) ;

    return 0 ;
}

int NlmCm__win32_gettimerval(
    NlmCmClock__timeval* tv)
{
    LARGE_INTEGER   freq ;  /* Counts per second */
    LARGE_INTEGER   now ;
    int         ret ;
    const int       nsPerSec = 1000*1000*1000 ;

    /* Ideally, we only need to call this once */
    if (QueryPerformanceFrequency(&freq) == 0)      /* No HW clock? */
    return NlmCm__win32_gettimeofday(tv) ;

    ret = QueryPerformanceCounter(&now) ;
    assert(ret != 0) ;

    if (freq.QuadPart == 0) {
    assert(0) ; /* Why did this fail if QueryPerformanceFrequency() succeeded */
    return -1 ;
    }

    /* Get the #seconds first or else we can overflow when we multiply by nsPerSec */
    tv->tv_sec = (nlm_u32)(now.QuadPart / freq.QuadPart) ;

    now.QuadPart -= ((LONGLONG)tv->tv_sec * freq.QuadPart) ;
    now.QuadPart *= nsPerSec ;
    now.QuadPart /= freq.QuadPart ;
    tv->tv_ns     = (nlm_u32)(now.QuadPart % nsPerSec) ;

    return 0 ;
}


void NlmCmDebug__EnableErrorBox(
    NlmBool enable)
{
    if (enable) 
    {
    SetErrorMode(0) ;
    _set_error_mode(_OUT_TO_DEFAULT) ;
    }
    else 
    {
    SetErrorMode(SEM_NOGPFAULTERRORBOX |
             SEM_FAILCRITICALERRORS |
             SEM_NOOPENFILEERRORBOX) ;
    _set_error_mode(_OUT_TO_STDERR) ;

#ifndef NDEBUG
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE ) ;
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR ) ;
#endif 
/* NDEBUG */
    }
}


/* Sleep for the given number of milliseconds */
extern void
NlmCmSleep__MilliSec(
    nlm_u32 ms)
{
    Sleep(ms);
}

/* sleep for the given number of microseconds */
extern void
NlmCmSleep__MicroSec(
    nlm_u32 us)
{
    Sleep((us +999) / 1000);
}

#ifdef NLMMT
#include <nlmcmsemaphore.h>

/* 
   NB: Under Windows, there doesn't seem to be a native mechanism by which we
   can obtain the value of the semaphore, so we shadow the semaphore value.

   Note that merely incrementing/decrementing the value using language
   mechanisms is not thread-safe -- we use the Win32 "Interlocked" routines
   increment and decrement to guarantee atomicity where it matters.
*/


/* Initialize a new semaphore. Return NLMCMERR_OK on success, or
 * NLMCMERR_SEM_FAIL on failure. 
 */
NlmCmErrNum_t
NlmCmSemaphore__init_body(
    NlmCmSemaphore* self,
    nlm_32        value)
{
    /* Your NLMCM_MY_SEMAPHORE_INFO structure is stored in the m_semaphoreInfo
     * member of the self parameter.
     */
    NLMCM_MY_SEMAPHORE_INFO* si     = &self->m_semaphoreInfo;
    SECURITY_ATTRIBUTES     secAtts = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    NlmCmRequire(value >= 0);
    si->m_value = value;

    si->m_semaphore = CreateSemaphore(&secAtts, value, INT32_MAX, NULL);
    if( !si->m_semaphore ) return NLMCMERR_SEM_FAIL;

    return NLMCMERR_OK;
}

/* Clean up after a semaphore that is no longer required. Return NLMCMERR_OK on
 * success and NLMCMERR_SEM_FAIL on failure. 
 */
NlmCmErrNum_t
NlmCmSemaphore__deinit_body(
    NlmCmSemaphore* self)
{
    NlmBool         success = NlmTrue;
    NLMCM_MY_SEMAPHORE_INFO* si     = &self->m_semaphoreInfo;

    (void) si;

    success  = CloseHandle(si->m_semaphore);

    return success ? NLMCMERR_OK : NLMCMERR_SEM_FAIL;
}

/* Block waiting on a semaphore. Return NLMCMERR_OK on success, or
 * NLMCMERR_SEM_FAIL on failure.
 */
NlmCmErrNum_t
NlmCmSemaphore__wait_body(
    NlmCmSemaphore* self)
{
    NLMCM_MY_SEMAPHORE_INFO* si     = &self->m_semaphoreInfo;
    DWORD           status  = WaitForSingleObject(si->m_semaphore, INFINITE);

    NlmCmAssert(WAIT_OBJECT_0 == status, 
        "Expecting only WAIT_OBJECT_0 from WaitForSingleObject");

    if( WAIT_OBJECT_0 == status ) 
    {
        InterlockedDecrement((LPLONG) &si->m_value);
    } else 
    {
        return NLMCMERR_SEM_FAIL;
    }

    return NLMCMERR_OK;
}

/* Wait on a semaphore but do not block. Return NLMCMERR_OK if the caller got
 * a lock, and NLMCMERR_SEM_AGAIN if the semaphore was already maxed out.
 */
NlmCmErrNum_t
NlmCmSemaphore__trywait_body(
    NlmCmSemaphore* self)
{
    NlmCmErrNum_t       rc      = NLMCMERR_OK;
    NLMCM_MY_SEMAPHORE_INFO* si     = &self->m_semaphoreInfo;
    DWORD           status  = WaitForSingleObject(si->m_semaphore, 0);

    NlmCmAssert(WAIT_OBJECT_0 == status || WAIT_TIMEOUT == status, 
        "Expecting only signaled/non-signaled semaphore state");
    
    if( WAIT_OBJECT_0 == status )   
    {
        InterlockedDecrement((LPLONG) &si->m_value);
    rc = NLMCMERR_OK;
    } else 
    {   
    rc = NLMCMERR_SEM_AGAIN;
    }

    return rc;
}

/* Release an already held lock. Return NLMCMERR_OK on success and
 * NLMCMERR_SEM_FAIL on failure.
 */
NlmCmErrNum_t
NlmCmSemaphore__post_body(
    NlmCmSemaphore* self)
{
    NLMCM_MY_SEMAPHORE_INFO* si = &self->m_semaphoreInfo;

    /* NB: In order for this to work, the security attributes of the semaphore
       must include SEMAPHORE_MODIFY_STATE.  If ReleaseSemaphore is failing,
       check semaphore creation -- security problems like this are only likely
       to occur when using OpenSemaphore (which we don't actually use). */

    if( !ReleaseSemaphore(si->m_semaphore, 1, NULL) )
    {
        NlmCmAssert(NlmFalse, "Failed to release semaphore");
        return NLMCMERR_SEM_FAIL;
    } else 
    {
        InterlockedIncrement((LPLONG) &si->m_value);
    }

    return NLMCMERR_OK;
}

/* Get the current value of the semaphore. Return NLMCMERR_OK on success. */
NlmCmErrNum_t
NlmCmSemaphore__getvalue_body(
    NlmCmSemaphore* self, 
    nlm_32* o_value)
{
    NLMCM_MY_SEMAPHORE_INFO* si = &self->m_semaphoreInfo;

    /* simple reads and writes to properly-aligned 32-bit variables are atomic. */
    *o_value = si->m_value;
    return NLMCMERR_OK;

}

/* Routine used to start threads. Return NlmTrue if the thread was successfully
 * launched. Otherwise return NlmFalse. Every created thread should be passed
 * its NlmCmThread pointer (the self argument).
 */
NlmBool
NlmCmThread__StartThread(
    NlmCmThread*        self,
    NlmCmThreadFunction thread_function)
{
    /* Only one active thread can be associated with a single thread object */
    if (NlmCmThread__GetActive(self))
    {
    return NlmFalse ;
    }

    /* We have to flag ourself as active to ensure we don't have two threads
     * using the same NlmCmThread object.
     */
    NlmCmThread__SetActive(self, NlmTrue);

    /* The m_threadInfo member is a pointer to our NLMCM_MY_THREAD_INFO
     * struture. We can do any required initialization now.
     */
    self->m_threadInfo.m_thread_handle = 0 ;
    self->m_threadInfo.m_thread_id = 0 ;

    self->m_threadInfo.m_thread_handle = 
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_function, self, 0,
    &(self->m_threadInfo.m_thread_id)) ;

    /* Deactivate ourself if there was a failure starting the thread. */
    if (self->m_threadInfo.m_thread_handle == 0) 
    {
        NlmCmThread__SetActive(self, NlmFalse);
        return NlmFalse ;
    }

    return NlmTrue;
}

NlmCmThreadId_t
NlmCmThread__GetMyId(void)
{
    return GetCurrentThreadId();
}

NlmBool
NlmCmThread__IsIdEqual(
    NlmCmThreadId_t one, 
    NlmCmThreadId_t two)
{
    return one == two;
}

#endif /* NLMMT */
#endif /* NLMPLATFORM_WIN32 */





/* LINUX PLATFORM */
/* ------------------------------------------------------------------------- */
#ifdef NLMPLATFORM_UNIX

void NlmCmDebug__EnableErrorBox(
    NlmBool enable)
{
    (void) enable ;
}

#include <time.h>
/*#include <nlmcmclock.h>*/
int
NlmCm__posix_gettimeofday(
    NlmCmClock__timeval* tv)
{
    /* We use our own timeval struct to avoid casting, in the off chance that
     * NlmCmClock__timeval should change in a manner that makes it incompatible
     * with struct timeval.
     */
    struct timeval t ;
    int retval ;

    retval = gettimeofday(&t, 0) ;

    tv->tv_sec = t.tv_sec ;
    tv->tv_ns = t.tv_usec*1000 ;

    return retval ;
}

int
NlmCm__posix_gettimerval(
    NlmCmClock__timeval* tv)
{
    /* We use our own timeval struct to avoid casting, in the off chance that
     * NlmCmClock__timeval should change in a manner that makes it incompatible
     * with struct timeval.
     */
    struct timeval t ;
    int retval ;
    
    retval = gettimeofday(&t, 0) ;

    tv->tv_sec = t.tv_sec ;
    tv->tv_ns = t.tv_usec*1000 ;

    return retval ;
}

#include <time.h>           /* for nanosleep() */
#include <errno.h>

/* nanosleep() is not available on NetOS, so 
add a while loop there. */

#ifndef NLM_HAL_NETOS
static void
my_nanosleep(
    nlm_u32 sec,
    nlm_u32 ns
    )
{
    struct timespec requested, *rqtp = &requested;
    struct timespec remaining, *rmtp = &remaining;

    rqtp->tv_sec  = sec;
    rqtp->tv_nsec = ns;
    while (nanosleep(rqtp, rmtp) < 0) {
        if (EINTR == errno) {
            /* nanosleep was interrupted by a signal, so we must repeat the
               sleep (but do so only for the remaining interval) */
            struct timespec* t = rqtp;
            rqtp = rmtp;
            rmtp = t;
        }
        else {
            NlmCmDie("NlmCmSleepMS: nanosleep failure");
        }
    }
}
#else

/*CPU is 1.2 GHz so 1 cycle = 1 ns. */

static void
my_nanosleep(
    nlm_u32 sec,
    nlm_u32 ns
    )
{
    nlm_u32 i = 0, j = 0;
    nlm_u64 total_ns = sec*1000*1000*1000 + ns;

    for(i = 0; i < total_ns; i++)
        j++;   
    return;
}

#endif

/* Sleep for the given number of milliseconds */
void
NlmCmSleep__MilliSec(
    nlm_u32 ms) 
{
    my_nanosleep(ms / 1000, (ms % 1000) * 1000000);
}

/* Sleep for the given number of microseconds */
void
NlmCmSleep__MicroSec(
    nlm_u32 us) 
{
    my_nanosleep(us / 1000000, (us % 1000000) * 1000);
}

#ifdef NLMMT

#include <nlmcmsemaphore.h>

/* Initialize a new semaphore. Return NLMCMERR_OK on success, or
 * NLMCMERR_SEM_FAIL on failure. 
 */
NlmCmErrNum_t
NlmCmSemaphore__init_body(
    NlmCmSemaphore* self,
    nlm_32        value)
{
    NLMCM_MY_SEMAPHORE_INFO* si = &self->m_semaphoreInfo;

    NlmCmRequire(value >= 0);

    si->m_semaphore = &si->m_semaphore_body;
    
    if( sem_init(si->m_semaphore, 0, value) < 0 )
    {
    return NLMCMERR_SEM_FAIL;
    }

    return NLMCMERR_OK;
}

/* Clean up after a semaphore that is no longer required. Return NLMCMERR_OK on
 * success and NLMCMERR_SEM_FAIL on failure. 
 */
NlmCmErrNum_t
NlmCmSemaphore__deinit_body(
    NlmCmSemaphore* self)
{
    NLMCM_MY_SEMAPHORE_INFO* si = &self->m_semaphoreInfo;
    
    if( sem_destroy(si->m_semaphore) < 0 )
    {
        return NLMCMERR_SEM_FAIL;
    }

    return NLMCMERR_OK;
}

/* Block waiting on a semaphore. Return NLMCMERR_OK on success, or
 * NLMCMERR_SEM_FAIL on failure.
 */
NlmCmErrNum_t
NlmCmSemaphore__wait_body(
    NlmCmSemaphore* self)
{
    NLMCM_MY_SEMAPHORE_INFO*    si = &self->m_semaphoreInfo;

    if( sem_wait(si->m_semaphore) < 0 )
    {
        return NLMCMERR_SEM_FAIL;
    }

    return NLMCMERR_OK;
}

/* Wait on a semaphore but do not block. Return NLMCMERR_OK if the caller got
 * a lock, and NLMCMERR_SEM_AGAIN if the semaphore was already maxed out.
 */
NlmCmErrNum_t
NlmCmSemaphore__trywait_body(
    NlmCmSemaphore* self)
{
    NLMCM_MY_SEMAPHORE_INFO*    si = &self->m_semaphoreInfo;

    if( sem_trywait(si->m_semaphore) < 0 ) 
    {
    assert(EAGAIN == errno);
    return NLMCMERR_SEM_AGAIN;
    }

    return NLMCMERR_OK;
}

/* Release an already held lock. Return NLMCMERR_OK on success and
 * NLMCMERR_SEM_FAIL on failure.
 */
NlmCmErrNum_t
NlmCmSemaphore__post_body(
    NlmCmSemaphore* self)
{
    NLMCM_MY_SEMAPHORE_INFO* si = &self->m_semaphoreInfo;

    if( sem_post(si->m_semaphore) < 0 )
    {
        return NLMCMERR_SEM_FAIL;
    }

    return NLMCMERR_OK;
}

/* Get the current value of the semaphore. Return NLMCMERR_OK on success. */
NlmCmErrNum_t
NlmCmSemaphore__getvalue_body(
    NlmCmSemaphore*  self, 
    nlm_32*     o_value)
{
    NLMCM_MY_SEMAPHORE_INFO* si = &self->m_semaphoreInfo;

    if( sem_getvalue(si->m_semaphore, o_value) < 0 )
    {
        return NLMCMERR_SEM_FAIL;
    }

    return NLMCMERR_OK;
}


#include <nlmcmthread.h>

/* Type used by this POSIX threads implementation so it can cast to the
 * appropriate function type (required for clean C++ compilation).
 */
#include <nlmcmexterncstart.h>
typedef void *(*POSIX_START_FUNCTION)(void*);
#include <nlmcmexterncend.h>

/* Routine used to start threads. Return NlmTrue if the thread was successfully
 * launched. Otherwise return NlmFalse. Every created thread should be passed
 * its NlmCmThread pointer (the self argument).
 */
NlmBool
NlmCmThread__StartThread(
    NlmCmThread*        self,
    NlmCmThreadFunction     thread_function)
{
    int         err;
    pthread_attr_t  attr;

    /* Only one active thread can be associated with a single thread object */
    if (NlmCmThread__GetActive(self))
    {
    return NlmFalse ;
    }

    /* We have to flag ourself as active to ensure we don't have two threads
     * using the same NlmCmThread object.
     */
    NlmCmThread__SetActive(self,NlmTrue);

    /* The m_threadInfo member is a pointer to our NLMCM_MY_THREAD_INFO
     * struture. We can do any required initialization now.
     */
    self->m_threadInfo.m_tid = 0 ;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); 
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
    err = pthread_create(&(self->m_threadInfo.m_tid), &attr,
             (POSIX_START_FUNCTION)thread_function, self);
    pthread_attr_destroy(&attr) ;

    /* Deactivate ourself if there was a failure starting the thread. */
    if (err) 
    {
      NlmCmThread__SetActive(self, NlmFalse);
      return NlmFalse;
    }

    return NlmTrue;
}

/* Return the id of the current thread */
NlmCmThreadId_t
NlmCmThread__GetMyId(void)
{
    return pthread_self() ;
}

/* Compare two thread ids for equality. */
NlmBool
NlmCmThread__IsIdEqual(
    NlmCmThreadId_t one,
    NlmCmThreadId_t two)
{
    return pthread_equal(one, two) ;
}

#endif /* NLMMT */
#endif /* NLMPLATFORM_LINUX */

#ifdef NLMPLATFORM_VXWORKS

#include <time.h> 

void NlmCmDebug__EnableErrorBox(
    NlmBool enable)
{
    (void) enable ;
}




int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    int retVal =  0;
    struct timespec ts;
    
    (void) tz;

    if  ( (retVal=clock_gettime(CLOCK_REALTIME, &ts))==0)
    {
        tv->tv_sec  = ts.tv_sec;
        tv->tv_usec = (ts.tv_nsec + 500) / 1000;
    }
     return retVal;
}




#endif /*NLMPLATFORM_VXWORKS*/

#else   /* #ifndef NLMPLATFORM_BCM */

void NlmCmDebug__EnableErrorBox(
    NlmBool enable)
{
    (void) enable ;
}



/* Implementation for BCM as SAL is not having fprintf which accepts "fp" */
nlm_32
NlmCmFile__fprintf(NlmCmFile* fp,
                       const char *format, ...)

{  
    return 0;
}


void *
NlmCm__memmove(void *dstP, const void *srcP, nlm_u32 numOfBytes)
{
    nlm_u8 *a = dstP;
    const nlm_u8 *b = srcP;

    /* Check for opverlapping area and if overlapping then copy backward otherwise
    copy forward */
    if (a <= b || a >= (b + numOfBytes))            
    {
        /* No overlap so copy forward */    
        while (numOfBytes--)
            *a++ = *b++;
    }
    else 
    {
        /* Overlap detected so copy backward  */
        a = a + numOfBytes - 1;
        b = b + numOfBytes - 1;

        while (numOfBytes--)
            *a-- = *b--;
    }

    return dstP;
}





#endif     /* #ifndef NLMPLATFORM_BCM */


/**/
