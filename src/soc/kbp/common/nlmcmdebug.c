/*
 * $Id: nlmcmdebug.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include <nlmcmdebug.h>
#include <nlmcmstring.h>
#include <nlmcmportable.h>

void 
NlmCmDebug__Setup(nlm_32 alloc_id, NlmCmDebug__ErrorBoxMode eb_mode)
{
    NlmCmDebug__AllocBreak(alloc_id) ;
    NlmCmDebug__SetErrorBox(eb_mode) ;
}


void 
NlmCmDebug__AllocBreak(nlm_32 id)
{
    #if !defined(NDEBUG) && defined(_MSC_VER)
        _CrtSetDbgFlag(_CrtSetDbgFlag(0) | _CRTDBG_LEAK_CHECK_DF) ;
        _CrtSetBreakAlloc(id) ;

    #elif defined(DMALLOC_OPTIONS)
    _dmalloc_address = id ;

    #else   
        (void) id ;

    #endif
}


void 
NlmCmDebug__AllocVerify(void* ptr)
{
    #if !defined(NDEBUG) && defined(_MSC_VER)
        (void) ptr ;

    #elif defined(DMALLOC_OPTIONS)
        dmalloc_verify(ptr) ;

    #else
        (void) ptr ;

    #endif
}


void 
NlmCmDebug__SetErrorBox(NlmCmDebug__ErrorBoxMode eb_mode)
{
    const char *name_in_env = "NLMCM_DEBUG_ERRORBOX" ;
    const char *value_in_env ;
    
    (void) name_in_env ;
    (void) value_in_env ;

    switch (eb_mode) {

    case NLMCM_DBG_EBM_DISABLE:
    NlmCmDebug__EnableErrorBox(0) ;
    break ;

    case NLMCM_DBG_EBM_VIA_ENV:
    value_in_env = '\0';
/* Check whether NlmCmBasic__getenv is defined before calling it */
#ifdef NlmCmBasic__getenv
    value_in_env = NlmCmBasic__getenv(name_in_env) ;
#endif
    if (value_in_env) {
        /* printf("DEBUG: value_in_env=%s\n", value_in_env) ; */
        if      (NlmCm__strcmp(value_in_env, "0") == 0) {
        NlmCmBasic__Note("disable system error_box due to environment variable") ;
        NlmCmDebug__EnableErrorBox(0) ;
        }
        else if (NlmCm__strcmp(value_in_env, "1") == 0) {
        NlmCmBasic__Note("enable system error_box due to environment variable") ;
        NlmCmDebug__EnableErrorBox(1) ;
        }
        else {
        /* printf("DEBUG: skipped value_in_env=%s\n", value_in_env) ; */
        NlmCmBasic__Note("unexpected NLMCM_DEBUG_ERRORBOX environment variable value: not 0 or 1 -- ignored") ;
        }
    }
    break ;

    case NLMCM_DBG_EBM_ENABLE:
    NlmCmDebug__EnableErrorBox(1) ;
    break ;

    default:
    NlmCmDie("unexpected eb_mode") ;
    }
}


void
NlmCmDebug__Break(void)
{
    #if defined(_MSC_VER) && defined(_DEBUG)
    _CrtDbgBreak();         /* break to the debugger */
    #else
    static int count = 0;
    count++;                /* something to break on */
    #endif
}


NlmBool 
NlmCmDebug__IsMemLeak(void)
{
    #if !defined(NDEBUG) && defined(_MSC_VER)
    _CrtMemState msNow;

    _CrtMemCheckpoint(&msNow);

    if (msNow.lCounts[_CLIENT_BLOCK] != 0 || msNow.lCounts[_NORMAL_BLOCK] != 0 ||
    (_crtDbgFlag & _CRTDBG_CHECK_CRT_DF && msNow.lCounts[_CRT_BLOCK] != 0))
    {
    return NlmTRUE;         /* mem leak detected */
    }
    #endif

    return NlmFALSE;   /* no detected leaked objects */
}


/** See documentation for -finstrument-functions in the gcc man page */
static void *   NlmCmDebug__StackLoMark          = (void*)(NLM_VALUE_MAX);
static void *   NlmCmDebug__StackHiMark          = 0 ;
static int      NlmCmDebug__StackBreakPt     = 2048 ;   /* Default limit of stack space */
static NlmBool  NlmCmDebug__ReportStackBreakError = NlmTRUE ;


void
NlmCmDebug__cyg_profile_func_enter (        /* Not thread safe */
    void *this_fn,
    void *call_site)
{
    int stkChanged = 0 ;

    (void)call_site ;
    (void)this_fn ;
    (void)stkChanged ;

#ifdef NLM_ENABLE_STRICT_CHECKS

    if ( NlmCmDebug__StackLoMark > (void*)&stkChanged ) {
    NlmCmDebug__StackLoMark = &stkChanged ;
    stkChanged = 1 ;
    }
    if ( NlmCmDebug__StackHiMark < (void*)&stkChanged ) {
    NlmCmDebug__StackHiMark = &stkChanged ;
    stkChanged = 1 ;
    }
    if (stkChanged) {
    unsigned int diff = (unsigned int)NlmCmDebug__StackHiMark-(unsigned int)NlmCmDebug__StackLoMark ;

    /* N.B., This naked call to printf is legal! Our NlmCmFile
     * implementation is instrumented.  It is not reentrant. Thus we CAN
     * NOT call in to it from here. This code is only enabled in special
     * strict configurations.
     */
    NlmCm__printf("Note: Cur-Stack low = 0x%08x, high = 0x%08x span = %8d\n",
           (unsigned int)NlmCmDebug__StackLoMark, (unsigned int)NlmCmDebug__StackHiMark, diff) ;
    if (diff >= (unsigned int)NlmCmDebug__StackBreakPt) {
        if (NlmCmDebug__ReportStackBreakError) {

        /* N.B., This naked call to printf is legal! See note above. */
        NlmCm__printf("Error: Stack size %d exceeded limit %d\n", diff, NlmCmDebug__StackBreakPt) ;
        NlmCmDebug__ReportStackBreakError = NlmFALSE ;
        }
        NlmCmDebug__Break() ;
    }
    }
#endif /* NLM_ENABLE_STRICT_CHECKS */
}
    
void
NlmCmDebug__cyg_profile_func_exit  (
    void *this_fn,
    void *call_site)
{
    (void)this_fn ;
    (void)call_site ;
}

void
NlmCmDebug__ResetStackMarkers(void)
{
    int stkLoc ;

    (void) stkLoc ;
    
#if NLM_CMN_DC
    NlmCmDebug__StackLoMark = (void *)0xffffffff ;
    NlmCmDebug__StackHiMark = 0 ;
#endif

    NlmCmDebug__StackLoMark = (void*)&stkLoc ;
    NlmCmDebug__StackHiMark = (void*)&stkLoc ;

    NlmCmDebug__ReportStackBreakError = NlmTRUE ;
}

int
NlmCmDebug__StackBreak(
    int stack_depth)
{
    int ret = NlmCmDebug__StackBreakPt ;
    NlmCmDebug__StackBreakPt = stack_depth;
    return ret ;
}

/*[]*/
