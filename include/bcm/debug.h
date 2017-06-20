/*
 * $Id: debug.h,v 1.48 Broadcom SDK $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __BCM_DEBUG_H__
#define __BCM_DEBUG_H__

#include <shared/bsl.h>

#include <bcm/types.h>

#ifndef BCM_HIDE_DISPATCHABLE

/* _bcm_debug_api */
extern void _bcm_debug_api(
    int log_src,
    char *api, 
    int nargs, 
    int ninargs, 
    int arg1, 
    int arg2, 
    int arg3, 
    int rv);

#endif /* BCM_HIDE_DISP  ATCHABLE */

#if defined(BROADCOM_DEBUG)
#define BCM_API(_1, _2, _3, _4, _5, _6, _7, _8)  \
    do { \
        if (LOG_CHECK(_1 | BSL_VERBOSE)) \
           _bcm_debug_api(_1,_2,_3,_4,_5,_6,_7,_8); \
    } while (0) 
#else
#define BCM_API(_1, _2, _3, _4, _5, _6, _7, _8)
#endif

#endif /* __BCM_DEBUG_H__ */
