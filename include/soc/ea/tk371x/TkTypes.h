/*
 * $Id: TkTypes.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        name.h
 * Purpose:     Purpose of the file
 */

#ifndef _SOC_TKTYPES_H_
#define _SOC_TKTYPES_H_

#include <sal/types.h>

#ifndef Bool
#define Bool    unsigned char
#endif

#ifndef OK
#define OK      0
#endif

#ifndef ERROR
#define ERROR   (-1)
#endif

typedef enum eRc {
    RcOk,
    RcFail,
    RcFound,
    RcNotFound,
    RcEmptyFound,
    RcBadParam,
    RcTimedOut,
    RcDoubleAlloc,
    RcTooManyLlids,
    RcNoLlidToRegister,
    RcNotProvisioned,
    RcNoOamBw,
    RcWrongMode,
    RcOnuReturnedErr,
    RcMissingTlv,
    RcBadOnuResponse,
    RcNoResource,
    rcNoSupport,
    RcEnumForceS8 = 0x7F
    } eRc;

#if defined(UNIX) || defined(VXWORKS) || defined(LINUX)
    #define PACK __attribute__((__packed__))
#else
    #error "Must define OS: LINUX, VXWORKS or UNIX"
#endif

#endif	/* !_SOC_TKTYPES_H_ */
