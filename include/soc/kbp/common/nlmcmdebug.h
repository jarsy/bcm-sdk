/*
 * $Id: nlmcmdebug.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#ifndef INCLUDED_NLMCMDEBUG_H
#define INCLUDED_NLMCMDEBUG_H

#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmcmexterncstart.h>
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif

/*----------------------------------------------------------------*/


typedef enum NlmCmDebug__ErrorBoxMode_t {
    NLMCM_DBG_EBM_DISABLE = -1 ,
    NLMCM_DBG_EBM_VIA_ENV =  0 ,
    NLMCM_DBG_EBM_ENABLE     =  1
} NlmCmDebug__ErrorBoxMode ;

/*
 * __GNUC__ is always defined while using gcc
 * The flag will enable us to exclude __attribute__
 * from compilers who do not recognize it.
 *
 * no_instrument_function makes sure the -finstrument-function
 * compiler flag does not effect these particular functions
 * to avoid potential problems (ie, infinite loops/recursion)
 *
 * g++ does not support the no_instrument_function attribute
 */

#if defined(__GNUC__) && !defined(__cplusplus)
#define NO_INSTRUMENT_FUNCTION __attribute__((no_instrument_function))
#else
#define NO_INSTRUMENT_FUNCTION
#endif /*__GNUC__*/

/*----------------------------------------------------------------*/

/* Summary
   Enable/control optional debugging features.

   Description
   Calls NlmCmDebug__SetErrorBox and NlmCmDebug__AllocBreak;
   see their documentation for further details.

   See Also
   NlmCmDebug__SetErrorBox
   NlmCmDebug__AllocBreak
 */
extern void NlmCmDebug__Setup(nlm_32 alloc_id, NlmCmDebug__ErrorBoxMode eb_mode) ;

/* Summary
   Enable or disable the debugging popup dialog box on supported platforms.

   Description
   Enable or disable the debugging popup dialog box on supported platforms.
   No-op on platforms that do not support it.
   Supported platforms currently include Microsoft Visual C++.

   See Also
   NlmCmDebug__Setup
 */
extern void NlmCmDebug__SetErrorBox(NlmCmDebug__ErrorBoxMode eb_mode) ;


/* Summary
   Enable memory allocation debugging and leak checking on supported platforms.

   Description
   Enable support for debugging memory allocation and leak checking on
   platforms that support it. No-op on platforms that do not support
   it. Supported platforms currently include Microsoft Visual C++.

   To use, pass the id of the memory allocation at which to break. Alternatively,
   pass -1 for the id to just enable leak checking.

   Better yet, use NlmCmDebug__Setup to enable memory allocation
   debugging and to optionally disable debugging popup dialog boxes.

   See Also
   NlmCmDebug__Setup
   NlmCmDebug__IsMemLeak
 */
extern void NlmCmDebug__AllocBreak(nlm_32 alloc_id) ;

extern void NlmCmDebug__AllocVerify(void* ptr) ;

/*----------------------------------------------------------------*/

/* Trigger a breakpoint when run in a debugger on some platforms */
extern void NlmCmDebug__Break(void) NO_INSTRUMENT_FUNCTION;

/*----------------------------------------------------------------*/

/* Summary
   Detect memory leaks on supported platforms

   Description
   Returns a boolean value indicating whether memory leaks were detected. Only
   works on certain platforms. Microsoft Visual C++ on Windows is the only
   supported platform.

   Return
   * NlmFALSE if there were no memory leaks or the current platform can not
     automatically detect memory leaks.
   * NlmTRUE if there were memory leaks.

   See Also
   NlmCmDebug__AllocBreak

 */
extern NlmBool NlmCmDebug__IsMemLeak(void);

/*----------------------------------------------------------------*/


void NlmCmDebug__cyg_profile_func_enter(void*, void*) NO_INSTRUMENT_FUNCTION ; 
void NlmCmDebug__cyg_profile_func_exit(void*, void*) NO_INSTRUMENT_FUNCTION ;
extern void NlmCmDebug__ResetStackMarkers(void) NO_INSTRUMENT_FUNCTION ;
extern int NlmCmDebug__StackBreak(int) NO_INSTRUMENT_FUNCTION ;


/*----------------------------------------------------------------*/
#ifndef NLMPLATFORM_BCM
#include <nlmcmexterncend.h>
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif

#endif
/*[]*/
