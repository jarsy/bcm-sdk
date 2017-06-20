/*
 * $Id: osl.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * OS Independent Layer
 */

#ifndef _osl_h_
#define _osl_h_

#ifdef V2_HAL
#include <v2hal_osl.h>
#elif defined(vxworks)
#include <vx_osl.h>
#elif defined(linux) && defined(__KERNEL__)
#include <soc/brcm_osl.h>
#elif defined(PMON)
#include <pmon_osl.h>
#elif defined(NDIS)
#include <ndis_osl.h>
#elif defined(_CFE_)
#include <cfe_osl.h>
#elif defined (MACOS9)
#include <macos9_osl.h>
#elif defined(MACOSX)
#include <macosx_osl.h>
#else
#include <soc/brcm_osl.h>
#endif

#endif	/* _osl_h_ */
