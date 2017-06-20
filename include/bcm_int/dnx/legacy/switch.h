/*
 * $Id: switch.h,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Switch Internal header
 */

#ifndef _BCM_INT_DNX_SWITCH_H_
#define _BCM_INT_DNX_SWITCH_H_

#include <bcm_int/dnx_dispatch.h>
#include <sal/types.h>
#include <bcm/types.h>
#include <bcm_int/dnx/legacy/utils.h>

#if (defined(BCM_DNX_SUPPORT) && defined(BCM_WARM_BOOT_SUPPORT))
extern int    dnx_warmboot_test_mode[BCM_MAX_NUM_UNITS];
extern int    dnx_override_wb_test[BCM_MAX_NUM_UNITS];
extern int    dnx_disable_once_wb_test[BCM_MAX_NUM_UNITS];
#endif 

typedef struct {
    int interrupts_event_storm_nominal;
} bcm_dnx_switch_info_t;

int
_bcm_dnx_switch_init(int unit);
int
_bcm_dnx_switch_detach(int unit);

/** 
 *  Defines for bcm_dnx_switch_l3_protocol_group_*
 */

int 
_bcm_dnx_switch_control_get(int unit, bcm_switch_control_t bcm_type, int *arg);

/* Defines for header compensation per packet. */
#define BCM_SWITCH_HDR_COMP_PER_PACKET_MAX_INDEX        (31)
#define BCM_SWITCH_HDR_COMP_PER_PACKET_MAX_ABS_VALUE    ((1 << PPC_API_EG_ENCAP_PER_PKT_HDR_COMP_NOF_VALUE_LSBS) - 1)

#endif /* _BCM_INT_DNX_SWITCH_H_ */


