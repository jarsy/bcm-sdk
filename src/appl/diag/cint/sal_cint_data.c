/*
 * $Id: sal_cint_data.c,v 1.14 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * sal_cint_data.c
 *
 * Hand-coded support for a few SAL core routines. 
 */

int sal_core_cint_data_not_empty; 
#include <sdk_config.h>

#if defined(INCLUDE_LIB_CINT)

#include <cint_config.h>
#include <cint_types.h>
#include <cint_porting.h>
#include <sal/core/libc.h>
#include <sal/core/alloc.h>
#include <sal/core/thread.h>
#include <sal/appl/config.h>

#include <sal/appl/sal.h>



CINT_FWRAPPER_CREATE_RP2(void*, void, 1, 0, 
                         sal_alloc, 
                         int,int,size,0,0,
                         void*,void,name,1,0); 

CINT_FWRAPPER_CREATE_VP1(sal_free,
                         void*,void,p,1,0); 

/*
 * Explicit wrappers as these may be macros instead of functions
 */
static void* __memset(void* p, int c, int sz)
{
    return sal_memset(p, c, sz); 
}
static void* __memcpy(void* dst, void* src, int sz)
{
    return sal_memcpy(dst, src, sz); 
}
static int __strcmp(char* p, char* c)
{
    return sal_strcmp(p, c);
}
static char* __strcpy(char* dst, char* src)
{
    return sal_strcpy(dst, src);
}
static char* __strstr(char* p, char* c)
{
    return sal_strstr(p, c);
}
static int __strlen(char* c)
{
    return sal_strlen(c);
}
static int __rand(void)
{
    return sal_rand();
}
static int __srand(int p)
{
    sal_srand(p);
    return p;
}
static double __sal_time(void)
{
    return sal_time();
}
static double __sal_time_usecs(void)
{
    return sal_time_usecs();
}
static void __sleep(int sec)
{
    sal_sleep(sec);
}
static void __usleep(unsigned int usec)
{
    sal_usleep(usec);
}
static void __udelay(unsigned int usec)
{
    sal_udelay(usec);
}
CINT_FWRAPPER_CREATE_RP3(void*,void,1,0,
                         __memset,
                         void*,void,p,1,0,
                         int,int,c,0,0,
                         int,int,sz,0,0); 

CINT_FWRAPPER_CREATE_RP3(void*,void,1,0,
                         __memcpy,
                         void*,void,dst,1,0,
                         void*,void,src,1,0,
                         int,int,sz,0,0); 
CINT_FWRAPPER_CREATE_RP2(int,int,1,0,
                         __strcmp,
                         char*,char,p,1,0,
                         char*,char,c,1,0);

CINT_FWRAPPER_CREATE_RP2(char*,char,1,0,
                         __strcpy,
                         char*,char,dst,1,0,
                         char*,char,src,1,0);
CINT_FWRAPPER_CREATE_RP2(char*,char,1,0,
                         __strstr,
                         char*,char,p,1,0,
                         char*,char,c,1,0);
CINT_FWRAPPER_CREATE_RP1(int,int,0,0,
                         __strlen,
                         char*,char,c,1,0);
CINT_FWRAPPER_CREATE_RP0(int,int,0,0,
                         __rand);
CINT_FWRAPPER_CREATE_RP0(int,int,0,0,
                         __sal_time);
CINT_FWRAPPER_CREATE_RP0(int,int,0,0,
                         __sal_time_usecs);
CINT_FWRAPPER_CREATE_RP1(int,int,0,0,
                         __srand,
                         int,int,p,0,0);
CINT_FWRAPPER_CREATE_VP1(__sleep,
                         int,int,sec,0,0);
CINT_FWRAPPER_CREATE_VP1(__usleep,
                         unsigned int,unsigned int,usec,0,0);
CINT_FWRAPPER_CREATE_VP1(__udelay,
                         unsigned int,unsigned int,usec,0,0);
CINT_FWRAPPER_CREATE_RP2(int,int,1,0,
                         sal_config_set,
                         char*,char,name,1,0,
                         char*,char,value,1,0);  

/*
 * COMPILER_64 macros
 */
static void __COMPILER_64_TO_32_LO(int* dst, uint64 src) { COMPILER_64_TO_32_LO(*dst, src); }
CINT_FWRAPPER_CREATE_VP2(__COMPILER_64_TO_32_LO, int*,int,dst,1,0,uint64,uint64,src,0,0); 

static void __COMPILER_64_TO_32_HI(int* dst, uint64 src) { COMPILER_64_TO_32_HI(*dst, src); }
CINT_FWRAPPER_CREATE_VP2(__COMPILER_64_TO_32_HI, int*,int,dst,1,0,uint64,uint64,src,0,0); 

static int __COMPILER_64_HI(uint64* src) { return COMPILER_64_HI(*src); }
CINT_FWRAPPER_CREATE_RP1(int,int,0,0,__COMPILER_64_HI,uint64*,uint64,src,1,0); 

static unsigned int __COMPILER_64_LO(uint64* src) { return COMPILER_64_LO(*src); }
CINT_FWRAPPER_CREATE_RP1(unsigned int,unsigned int,0,0,__COMPILER_64_LO,uint64*,uint64,src,1,0); 

static void __COMPILER_64_ZERO(uint64* dst) { COMPILER_64_ZERO(*dst); }
CINT_FWRAPPER_CREATE_VP1(__COMPILER_64_ZERO, uint64*, uint64, dst, 1, 0); 

static int __COMPILER_64_IS_ZERO(uint64* src) { return COMPILER_64_IS_ZERO(*src); }
CINT_FWRAPPER_CREATE_RP1(int,int,0,0,__COMPILER_64_IS_ZERO,uint64*,uint64,src,1,0); 

static void __COMPILER_64_SET(uint64* dst, uint32 hi, uint32 lo) { COMPILER_64_SET(*dst, hi, lo); }
CINT_FWRAPPER_CREATE_VP3(__COMPILER_64_SET, 
                         uint64*, uint64, dst, 1, 0, 
                         uint32, uint32, hi, 0, 0, 
                         uint32, uint32, lo, 0, 0); 


static cint_function_t __cint_sal_functions[] = 
    {
        CINT_FWRAPPER_ENTRY(sal_alloc),
        CINT_FWRAPPER_ENTRY(sal_free),
        CINT_FWRAPPER_NENTRY("sal_memset", __memset), 
        CINT_FWRAPPER_NENTRY("sal_memcpy", __memcpy), 
        CINT_FWRAPPER_NENTRY("sal_strcpy", __strcpy),
        CINT_FWRAPPER_NENTRY("sal_strcmp", __strcmp),
        CINT_FWRAPPER_NENTRY("sal_strstr", __strstr),
        CINT_FWRAPPER_NENTRY("sal_strlen", __strlen),
        CINT_FWRAPPER_NENTRY("sal_rand", __rand),
        CINT_FWRAPPER_NENTRY("sal_time", __sal_time),
        CINT_FWRAPPER_NENTRY("sal_time_usecs", __sal_time_usecs),
        CINT_FWRAPPER_NENTRY("sal_srand", __srand),
        CINT_FWRAPPER_NENTRY("sal_sleep", __sleep),
        CINT_FWRAPPER_NENTRY("sal_usleep", __usleep),
        CINT_FWRAPPER_NENTRY("sal_udelay", __udelay),
        CINT_FWRAPPER_NENTRY("COMPILER_64_TO_32_LO", __COMPILER_64_TO_32_LO), 
        CINT_FWRAPPER_NENTRY("COMPILER_64_TO_32_HI", __COMPILER_64_TO_32_HI), 
        CINT_FWRAPPER_NENTRY("COMPILER_64_HI", __COMPILER_64_HI), 
        CINT_FWRAPPER_NENTRY("COMPILER_64_LO", __COMPILER_64_LO), 
        CINT_FWRAPPER_NENTRY("COMPILER_64_ZERO", __COMPILER_64_ZERO), 
        CINT_FWRAPPER_NENTRY("COMPILER_64_IS_ZERO", __COMPILER_64_IS_ZERO), 
        CINT_FWRAPPER_NENTRY("COMPILER_64_SET", __COMPILER_64_SET), 
        CINT_FWRAPPER_ENTRY(sal_config_set),

        CINT_ENTRY_LAST
    }; 


cint_data_t sal_cint_data = 
    {
        NULL,
        __cint_sal_functions,
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL
    }; 

/*
 * This is the print function used by CINT. 
 *
 * The purpose of this function is to set the SAL main thread to avoid background thread prefixes during output. 
 */

int cint_printk(const char* fmt, ...)
{
    int rv; 
    va_list args; 
    sal_thread_t t; 

    t = sal_thread_main_get(); 
    sal_thread_main_set(sal_thread_self()); 

    va_start(args, fmt);     
    rv = sal_vprintf(fmt, args); 
    va_end(args); 

    sal_thread_main_set(t); 
    return rv; 
}

#endif /* INCLUDE_LIB_CINT */

    

