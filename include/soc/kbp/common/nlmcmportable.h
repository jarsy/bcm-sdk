/*
 * $Id: nlmcmportable.h,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#ifndef INCLUDED_NLMCMPORTABLE_H
#define INCLUDED_NLMCMPORTABLE_H

/* #define NLMPLATFORM_BCM */

#ifdef NLMPLATFORM_BCM
#include "sal/core/alloc.h"
#include "sal/core/libc.h"
#include "sal/appl/sal.h"
#include "sal/appl/io.h"

#endif


#include <soc/kbp/common/nlmcmexterncstart.h>
#include <stdio.h>
#include <stdlib.h>

#define INT8_MAX             (127)
#define INT8_MIN            (-128)
#define INT16_MAX          (32767)
#define INT32_MAX     (2147483647)

#define UINT8_MAX          (255U)
#define UINT16_MAX       (65535U)
#define UINT32_MAX  (4294967295U)

#define INT16_MIN         (-32768)
#define INT32_MIN ((-INT32_MAX)-1)

typedef          char       nlm_8 ;
typedef unsigned char       nlm_u8 ;
typedef   signed short      nlm_16 ;
typedef unsigned short      nlm_u16 ;
typedef   signed int        nlm_32 ;
typedef unsigned int        nlm_u32 ;
typedef unsigned long long  nlm_u64;

typedef void*               nlm_ptr;
typedef   float             nlm_float32 ;
typedef   double            nlm_float64 ;





#ifdef NLMPLATFORM_BSD
#define NLMPLATFORM_UNIX
#endif

#ifdef _MSC_VER
#define NLMPLATFORM_WIN32
#endif /* _MSC_VER */

typedef int NlmBool ;
enum 
{
    NlmFALSE = 0,  /* logical false for NlmBool */
    NlmTRUE  = 1,  /* logical true for NlmBool */
    NLMFALSE = 0,  /* logical false for NlmBool */
    NLMTRUE  = 1,  /* logical true for NlmBool */
    NlmFalse = 0,  /* logical false for NlmBool */
    NlmTrue  = 1   /* logical true for NlmBool */
};

typedef struct NlmCmClock__timeval_t {
    long        tv_sec;     /* Seconds */
    long        tv_ns;      /* Nanoseconds */
} NlmCmClock__timeval;

/* FREE BSD  PLATFORM SPECIFIC DEFINITIONS */
/* ------------------------------------------------------------------------- */
#ifdef NLMPLATFORM_BSD
#define __va_copy(dst,src) memcpy(&dst, &src, sizeof(va_list))
#endif

/* WINDOWS PLATFORM SPECIFIC DEFINITIONS */
/* ------------------------------------------------------------------------- */
#ifndef NLMPLATFORM_BCM

#ifdef NLMPLATFORM_WIN32

#define NLM_HAVE_FILE_IO
#define NLM_BUILD_EXTENDED_SOURCE

#pragma warning (disable : 4115)
#include <io.h>
#include <windows.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#define va_copy(dst,src) ((dst) = (src))
#include <fcntl.h>
#if !defined(NDEBUG)
#include <crtdbg.h>
#define NlmCm__sysmalloc(sz) _malloc_dbg((sz), _NORMAL_BLOCK, __FILE__, __LINE__ )
#else
#if defined(DMALLOC_OPTIONS)
#include <dmalloc.h>
#endif /* DMALLOC_OPTIONS */
#endif /* NDEBUG */


#ifndef NlmCm__sysmalloc
#define NlmCm__sysmalloc        malloc
#endif

#define NlmCm__sysfree      free

#define NLM_HAVE_FILE_IO
#define NLMCM_CREATE_MODE   (_S_IREAD | _S_IWRITE)
#define NLMCM_O_WRONLY      _O_WRONLY
#define NLMCM_O_RDONLY      _O_RDONLY
#define NLMCM_O_CREAT       _O_CREAT
#define NLMCM_O_TRUNC       _O_TRUNC
#define NLMCM_O_APPEND      _O_APPEND
#define mode_t          unsigned int

#define NlmCm__sysopen      open
#define NlmCm__sysclose     close

#define NLM_HAVE_CONSOLE
#define NlmCm__syswrite     write
#define NlmCm__sysread      read

#define NlmCm__stdin        stdin
#define NlmCm__stdout       stdout
#define NlmCm__stderr       stderr

#define NlmCm__memset       memset
#define NlmCm__memcpy       memcpy
#define NlmCm__memmove      memmove
#define NlmCm__memcmp       memcmp
/* out stream function definitions */
#define NlmCm__sprintf      sprintf
#define NlmCm__snprintf     _snprintf
#define NlmCm__printf       printf
#define NlmCmFile           FILE
#define NlmCmFile__printf   NlmCm__printf
#define NlmCmFile__fopen    fopen
#define NlmCmFile__fprintf  fprintf
#define NlmCmFile__fscanf   fscanf
#define NlmCmFile__fputc    fputc
#define NlmCmFile__fputs    fputs
#define NlmCmFile__fwrite   fwrite
#define NlmCmFile__fread    fread
#define NlmCmFile__fgetc    fgetc
#define NlmCmFile__fgets    fgets
#define NlmCmFile__fclose   fclose
#define NlmCmFile__EOF      (-1)

#define NlmCm_va_list       va_list
#define NmlCm_va_start  va_start
#define NmlCm_vfprintf  vfprintf
#define NmlCm_vprintf       vprintf


#define NlmCm__gettimeofday NlmCm__win32_gettimeofday
#define NlmCm__gettimerval  NlmCm__win32_gettimerval

#define NlmCm__pvt_abort        NlmCm__win32_abort
#define NlmCm__pvt_exit     exit

/* NB: this is an *undocumented* interface
   -- we only require it on Windows to suppress the error box */
#define NlmCmBasic__getenv  getenv

#define NlmCmConcatIdentity(a)  a
#define NlmCmConcat2_(a,b)  NlmCmConcatIdentity(a)NlmCmConcatIdentity(b)
#define NlmCmConcat3_(a,b,c)    NlmCmConcatIdentity(a)NlmCmConcatIdentity(b)NlmCmConcatIdentity(c)



extern void NlmCm__win32_abort(void) ;


/* NlmCm__gettimerval is used to retrieve a timer value -- preferably a
 * real-time-clock.
 *
 *  int NlmCm__gettimerval(NlmCmClock__timeval* time) ;
 *
 * - time is the structure to be filled.
 *
 *  If successful, the tv_sec member of the 'time' parameter should contain
 *  the number of seconds since the timer was started, and the tv_ns member
 *  should contain the fraction of the current seconds in nanoseconds.
 *
 *  Return Values:
 *  -  0 on success
 *  - -1if the operation failed
 *  - -2 if it is not fully implemented
 *  
 *  Cynapse includes a default implementation in cycmportable_template.c that
 *  simply returns -2.
 */
extern int
NlmCm__win32_gettimeofday(
    NlmCmClock__timeval* tv
    ) ;

/* NlmCm__gettimerval is used to retrieve a timer value -- preferably a
 * real-time-clock.
 *
 *  int NlmCm__gettimerval(NlmCmClock__timeval* time) ;
 *
 * - time is the structure to be filled.
 *
 *  If successful, the tv_sec member of the 'time' parameter should contain
 *  the number of seconds since the timer was started, and the tv_ns member
 *  should contain the fraction of the current seconds in nanoseconds.
 *
 *  Return Values:
 *  -  0 on success
 *  - -1if the operation failed
 *  - -2 if it is not fully implemented
 *  
 *  Cynapse includes a default implementation in cycmportable_template.c that
 *  simply returns -2.
 */
extern int
NlmCm__win32_gettimerval(
    NlmCmClock__timeval* tv
    ) ;


extern void NlmCmDebug__EnableErrorBox(
    NlmBool enable) ;


#ifdef NLMMT
typedef struct NLMCM_MY_SEMAPHORE_INFO 
{
    HANDLE      m_semaphore;
    nlm_u32     m_value;
} NLMCM_MY_SEMAPHORE_INFO ;


typedef DWORD NlmCmThreadId_t;
typedef struct NLMCM_MY_THREAD_INFO 
{
    HANDLE  m_thread_handle ;   /* Handle to the thread */
    DWORD   m_thread_id ;       /* ID of the thread */
    nlm_u32 m_ensure_non_empty_struct;

} NLMCM_MY_THREAD_INFO ;
#endif  /* NLMMT */


/* For expat, only defined when compiled from some version of MS Visual Studio */
#define NLMCMEXPAT_COMPILED_FROM_DSP   1

#endif /* CYPLATFORM_WIN32 */


/* LINUX/SOLARIS PLATFORM SPECIFIC DEFINITIONS */
/* ------------------------------------------------------------------------- */
#ifdef NLMPLATFORM_UNIX
#define NDEBUG
/* The -ansi switch on the compiler will prevent nanosleep() from being properly
   defined, so we force it to defined by temporarily defining
   _POSIX_C_SOURCE. */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
/*#define _POSIX_C_SOURCE 199506L*/
#define NLM_DEFINED_CUSTOM_POSIX_C_SOURCE
#endif

#include <time.h>

#ifdef NLM_DEFINED_CUSTOM_POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#undef NLM_DEFINED_CUSTOM_POSIX_C_SOURCE
#endif

#define __STDC_LIMIT_MACROS
/*#include <stdint.h>*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
/* -ansi => C89 => va_copy (a C99 feature) is not available --
   so we define it using the available internal name */
/*
 * Redefinition warning is coming. Do a check whether we really need to redefine?
 */
#ifndef va_copy
#define va_copy __va_copy
#endif

#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>

#if defined(CYMT) /* Multi-threaded. */

/* Include headers for threads, semaphores, etc. here. It is suggested
 * that the headers needed for multi-threaded functionality only be included
 * when that functionality is called for.
 */
#include <semaphore.h>
#include <pthread.h>

#endif /* defined (CYMT) */

#define NLM_HAVE_FILE_IO
#define NLMCM_CREATE_MODE        S_IRWXU
#define NLMCM_O_WRONLY           O_WRONLY
#define NLMCM_O_RDONLY           O_RDONLY
#define NLMCM_O_CREAT            O_CREAT
#define NLMCM_O_TRUNC            O_TRUNC
#define NLMCM_O_APPEND           O_APPEND
#define NlmCm__sysopen           open
#define NlmCm__sysclose          close

#define NLM_HAVE_CONSOLE
#define NlmCm__syswrite     write
#define NlmCm__sysread      read

#define NlmCm__stdin        stdin
#define NlmCm__stdout       stdout
#define NlmCm__stderr       stderr

#define NlmCm__memset       memset
#define NlmCm__memcpy       memcpy
#define NlmCm__memmove      memmove
#define NlmCm__memcmp       memcmp

#define NlmCm__sysmalloc        malloc
#define NlmCm__sysfree      free

#define NlmCm__gettimeofday NlmCm__posix_gettimeofday
#define NlmCm__gettimerval  NlmCm__posix_gettimerval
#define NlmCm__pvt_abort        abort
#define NlmCm__pvt_exit     exit

/* Used to concatenate values using the preprocessor */
#define NlmCmConcat2_(a,b)  a##b
#define NlmCmConcat3_(a,b,c)    a##b##c
#include "stdio.h"
/* out stream function definitions */
#define NlmCm__sprintf      sprintf
#define NlmCm__snprintf     snprintf
#define NlmCm__printf       printf
#define NlmCmFile           FILE
#define NlmCmFile__printf   NlmCm__printf
#define NlmCmFile__fopen    fopen
#define NlmCmFile__fprintf  fprintf
#define NlmCmFile__fscanf   fscanf
#define NlmCmFile__fputc    fputc
#define NlmCmFile__fputs    fputs
#define NlmCmFile__fwrite   fwrite
#define NlmCmFile__fread    fread
#define NlmCmFile__fgetc    fgetc
#define NlmCmFile__fgets    fgets
#define NlmCmFile__fclose   fclose
#define NlmCmFile__EOF      (-1)

#define NlmCm_va_list       va_list
#define NmlCm_va_start  va_start
#define NmlCm_vfprintf  vfprintf
#define NmlCm_vprintf       vprintf


extern void 
NlmCmDebug__EnableErrorBox(
    NlmBool enable) ;

/* NlmCm__gettimeofday is used to get the time since the UNIX Epoch (00:00:00
 * UTC on January 1, 1970). Unlike the ANSI C gettimeofday routine, Nlmnapse
 * has no notion of time zones. Whether time is reported as UTC or not is up
 * to the implementor.
 *
 *  int NlmCm__gettimeofday(NlmCmClock__timeval* time) ;
 *
 * - time is the structure to be filled.
 *
 * If successful, the tv_sec member of the 'time' parameter should contain the
 * number of seconds since the Epoch and the tv_ns member should contain the
 * fraction of the current second in nanoseconds.
 *
 * Return Values:
 *  -  0 on success
 *  - -1 if the operation failed
 *  - -2 if it is not fully implemented
 * 
 * Nlmnapse provides a default implementation in nlmcmportable_template.c that
 * returns -2. 
 */
extern int
NlmCm__posix_gettimeofday(
    NlmCmClock__timeval* tv
    ) ;

/* NlmCm__gettimerval is used to retrieve a timer value -- preferably a
 * real-time-clock.
 *
 *  int NlmCm__gettimerval(NlmCmClock__timeval* time) ;
 *
 * - time is the structure to be filled.
 *
 *  If successful, the tv_sec member of the 'time' parameter should contain
 *  the number of seconds since the timer was started, and the tv_ns member
 *  should contain the fraction of the current seconds in nanoseconds.
 *
 *  Return Values:
 *  -  0 on success
 *  - -1if the operation failed
 *  - -2 if it is not fully implemented
 *  
 *  Nlmnapse includes a default implementation in nlmcmportable_template.c that
 *  simply returns -2.
 */
extern int
NlmCm__posix_gettimerval(
    NlmCmClock__timeval* tv
    ) ;


#ifdef NLMMT
typedef struct NLMCM_MY_SEMAPHORE_INFO
{
    sem_t*  m_semaphore;
    sem_t   m_semaphore_body;
} NLMCM_MY_SEMAPHORE_INFO ;

typedef pthread_t NlmCmThreadId_t;
typedef struct NLMCM_MY_THREAD_INFO 
{
    NlmCmThreadId_t     m_tid;          /* Thread id/handle */
} NLMCM_MY_THREAD_INFO ;



#endif /* NLMMT */
#endif /* CYPLATFORM_LINUX */



/* VXWORKS PLATFORM SPECIFIC DEFINITIONS */
#ifdef NLMPLATFORM_VXWORKS

#define NDEBUG

#include <vxWorks.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef va_copy
#define va_copy __va_copy
#endif

#include <string.h>
#include <sys/times.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stat.h>



#define NLM_HAVE_FILE_IO
#define NLMCM_CREATE_MODE        S_IRWXU
#define NLMCM_O_WRONLY           O_WRONLY
#define NLMCM_O_RDONLY           O_RDONLY
#define NLMCM_O_CREAT            O_CREAT
#define NLMCM_O_TRUNC            O_TRUNC
#define NLMCM_O_APPEND           O_APPEND
#define NlmCm__sysopen           open
#define NlmCm__sysclose          close

#define NLM_HAVE_CONSOLE
#define NlmCm__syswrite     write
#define NlmCm__sysread      read

#define NlmCm__stdin        stdin
#define NlmCm__stdout       stdout
#define NlmCm__stderr       stderr

#define NlmCm__memset       memset
#define NlmCm__memcpy       memcpy
#define NlmCm__memmove      memmove
#define NlmCm__memcmp       memcmp

#define NlmCm__sysmalloc        malloc
#define NlmCm__sysfree      free

#define NlmCm__gettimeofday NlmCm__vxworks_gettimeofday
#define NlmCm__gettimerval  NlmCm__vxworks_gettimerval
#define NlmCm__pvt_abort        abort
#define NlmCm__pvt_exit     exit

/* Used to concatenate values using the preprocessor */
#define NlmCmConcat2_(a,b)  a##b
#define NlmCmConcat3_(a,b,c)    a##b##c
#include "stdio.h"
/* out stream function definitions */
#define NlmCm__sprintf      sprintf
#define NlmCm__snprintf     snprintf
#define NlmCm__printf       printf
#define NlmCmFile           FILE
#define NlmCmFile__printf   NlmCm__printf
#define NlmCmFile__fopen    fopen
#define NlmCmFile__fprintf  fprintf
#define NlmCmFile__fscanf   fscanf
#define NlmCmFile__fputc    fputc
#define NlmCmFile__fputs    fputs
#define NlmCmFile__fwrite   fwrite
#define NlmCmFile__fread    fread
#define NlmCmFile__fgetc    fgetc
#define NlmCmFile__fgets    fgets
#define NlmCmFile__fclose   fclose
#define NlmCmFile__EOF      (-1)

#define NlmCm_va_list       va_list
#define NmlCm_va_start  va_start
#define NmlCm_vfprintf  vfprintf
#define NmlCm_vprintf       vprintf


extern void 
NlmCmDebug__EnableErrorBox(
    NlmBool enable) ;

int gettimeofday(struct timeval *tv, struct timezone *tz);


#endif /* NLMPLATFORM_VXWORKS */

#else   /* #ifndef NLMPLATFORM_BCM */

#include<assert.h>

/* Hopefully should work as we are passing this in fprintf 
and we are ignoring fp in that function */
#define NlmCm__stdout       (0)   
#define NlmCm__stderr       (0)



#define NlmCm__memset       sal_memset
#define NlmCm__memcpy       sal_memcpy


#define NlmCm__memcmp       sal_memcmp
#define NlmCm__sysmalloc(x) sal_alloc((x), __FILE__)

#define NlmCm__sysfree      sal_free

#if NLM_CMN_DC
#define NlmCm__gettimeofday sal_time     /*NlmCm__posix_gettimeofday--> Need to implement */
#define NlmCm__gettimerval  NlmCm__posix_gettimerval   /* Not present in SAL, Not Used also */
#endif

/* Instead of aborting which is triggered from assert, we are calling 
assert() from BCM SDK which is again mapped to sal_assert() or nulled out.*/
#define NlmCm__pvt_abort()  assert(0)      

/* Used to concatenate values using the preprocessor */
#define NlmCmConcat2_(a,b)  a##b
#define NlmCmConcat3_(a,b,c)    a##b##c

/* out stream function definitions */
#define NlmCm__sprintf      sal_sprintf
#define NlmCm__snprintf     sal_snprintf
#define NlmCm__printf       sal_printf
#define NlmCmFile           FILE
#define NlmCmFile__printf   NlmCm__printf

/* fopen() is not supported from kbp sdk. Application can use sal directory to open file */
/*
#define NlmCmFile__fopen(x,y)   sal_fopen((char*)x,(char*)y)  
#define NlmCmFile__fclose   sal_fclose
*/


#define NlmCmFile__fputs(str, fp)   printk_file("%s", str)


#define NlmCmFile__EOF      (EOF)

#define NlmCm_va_list       va_list
#define NmlCm_va_start      va_start
#define NmlCm_vprintf       sal_vprintf


/* Implementation for BCM as SAL is not having fprintf which accepts "fp" */
extern nlm_32
NlmCmFile__fprintf(NlmCmFile* fp,
                       const char *format, ...);


/* Implementation of memmove() for  BCM as SAL is not having this function */
void *
NlmCm__memmove(void *dstP, 
        const void *srcP, nlm_u32 numOfBytes);




extern void 
NlmCmDebug__EnableErrorBox(
    NlmBool enable) ;

#if NLM_CMN_DC
#ifdef NLMMT
typedef struct NLMCM_MY_SEMAPHORE_INFO
{
    sem_t*  m_semaphore;
    sem_t   m_semaphore_body;
} NLMCM_MY_SEMAPHORE_INFO ;

typedef pthread_t NlmCmThreadId_t;
typedef struct NLMCM_MY_THREAD_INFO 
{
    NlmCmThreadId_t     m_tid;          /* Thread id/handle */
} NLMCM_MY_THREAD_INFO ;


#endif /* NLMMT */
#endif

#endif   /* #ifndef NLMPLATFORM_BCM */

#ifdef NLM_BUILD64
#define NLM_PLATFORM_64
#endif /* NLM_BUILD64 */

#ifdef NLM_PLATFORM_64
#ifdef NLMPLATFORM_WIN32
typedef unsigned long long      nlm_value;
#else
typedef unsigned  long      nlm_value;
#endif /* NLMPLATFORM_WIN32 */

#define NLM_VALUE_MAX           0xffffffffffffffff
#else
typedef unsigned int            nlm_value;
#define NLM_VALUE_MAX           0xffffffff
#endif /* NLM_PLATFORM_64 */

typedef nlm_value   uintvs_t ;


#ifndef NLMPLATFORM_BCM

#define NLM_CAST_PTR_TO_NLM_U32(x) ((nlm_u32)(nlm_value)x)
#define NLM_CAST_NLM_U32_TO_PTR(x) (void*)((nlm_value)x)

#else  /* #ifndef NLMPLATFORM_BCM */

#define NLM_CAST_PTR_TO_NLM_U32(x)  PTR_TO_INT(x)
#define NLM_CAST_NLM_U32_TO_PTR(x)  INT_TO_PTR(x)   

#endif  /* #ifndef NLMPLATFORM_BCM */


#include <soc/kbp/common/nlmcmexterncend.h>
#endif /* INCLUDED_NLMCMPORTABLE_H */

/*[]*/
