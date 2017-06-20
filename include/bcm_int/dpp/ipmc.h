/*
 * $Id: ipmc.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * IPMC Internal header
 */

#ifndef _BCM_INT_DPP_IPMC_H_
#define _BCM_INT_DPP_IPMC_H_

#include <sal/types.h>
#include <bcm/types.h>

typedef struct bcm_dpp_ipmc_info_s {
    int         init;       /* TRUE if IPMC module has been initialized */    
} bcm_dpp_ipmc_info_t;

#define _BCM_DPP_IPMC_NOF_RPS(__unit)    (SOC_DPP_CONFIG(__unit)->l3.nof_rps)

#define _BCM_DPP_IPMC_BIDIR_SUPPORTED(__unit)    (_BCM_DPP_IPMC_NOF_RPS(__unit) > 0)

#define BCM_IPMC_SSM_DISABLE  0
#define BCM_IPMC_SSM_TCAM_LPM 1
#define BCM_IPMC_SSM_KAPS_LPM 2

/* L3 module flags for soc layer */
#define _BCM_IPMC_FLAGS_RAW_ENTRY                   (1 << 0)   /* insert entry with raw payload */

#endif /* _BCM_INT_DPP_IPMC_H_ */

