/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * OS Independent Layer
 * 
 * Copyright(c) 2001 Broadcom Corp.
 * $Id: osl.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#ifndef _osl_h_
#define _osl_h_

#ifdef V2_HAL
#include <v2hal_osl.h>
#elif defined(vxworks)
#include <vx_osl.h>
#elif defined(linux)
#include <linux_osl.h>
#elif PMON
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
#error "Unsupported OSL requested"
#endif

/* handy */
#define	SET_REG(r, mask, val)	W_REG((r), ((R_REG(r) & ~(mask)) | (val)))

#endif	/* _osl_h_ */
