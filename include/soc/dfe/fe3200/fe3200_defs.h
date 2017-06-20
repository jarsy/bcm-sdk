/*
 * $Id: fe3200_defs.h,v 1.1.2.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * FE3200 DEFS H
 */
 
#ifndef _SOC_FE3200_DEFS_H_
#define _SOC_FE3200_DEFS_H_

#include <soc/dfe/cmn/dfe_defs.h>
#include <soc/dcmn/error.h>
#include <soc/dfe/fe3200/fe3200_intr.h>


#define SOC_FE3200_PORT_SPEED_MAX               (25781)
#define SOC_FE3200_MAX_NUM_OF_PRIORITIES        (4)
    
#define SOC_FE3200_NOF_BLK                      128

#define SOC_FE3200_ONLY(unit)         assert(SOC_IS_FE3200(unit))

#endif

