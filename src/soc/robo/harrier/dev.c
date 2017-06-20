/*
 * $Id: dev.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h> 
#include "robo_harrier.h"
#include <soc/drv.h> 

/*
 *  Function : drv_harrier_dev_prop_get
 *
 *  Purpose :
 *      Get the device property information
 *
 *  Parameters :
 *      unit        :   unit id
 *      prop_type   :   property type
 *      prop_val     :   property value of the property type
 *
 *  Return :
 *      SOC_E_NONE      :   success
 *      SOC_E_PARAM    :   parameter error
 *
 *  Note :
 *      This function is to get the device porperty information.
 *
 */
int 
drv_harrier_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    switch (prop_type) {
        case DRV_DEV_PROP_MCAST_NUM:
            *prop_val = DRV_HARRIER_MCAST_GROUP_NUM;
            break;
        case DRV_DEV_PROP_AGE_TIMER_MAX_S:
            *prop_val = DRV_HARRIER_AGE_TIMER_MAX;
            break;
        case DRV_DEV_PROP_TRUNK_NUM:
            *prop_val = DRV_HARRIER_TRUNK_GROUP_NUM;
            break;
        case DRV_DEV_PROP_TRUNK_MAX_PORT_NUM:
            *prop_val = DRV_HARRIER_TRUNK_MAX_PORT_NUM;
            break;
        case DRV_DEV_PROP_COSQ_NUM:
            *prop_val = NUM_COS(unit);
            break;
        case DRV_DEV_PROP_MSTP_NUM:
            *prop_val = DRV_HARRIER_MSTP_GROUP_NUM;
            break;
        case DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT:
            *prop_val = DRV_HARRIER_SEC_MAC_NUM_PER_PORT;
            break;
        case DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE:
            *prop_val = DRV_HARRIER_COS_QUEUE_MAX_WEIGHT_VALUE;
            break;
        case DRV_DEV_PROP_AUTH_PBMP:
            if (SOC_IS_ROBO53242(unit)) {
                *prop_val = DRV_HARRIER_AUTH_SUPPORT_PBMP_BCM53242;
            } else {
                *prop_val = DRV_HARRIER_AUTH_SUPPORT_PBMP_BCM53262;
            }
            break;
        case DRV_DEV_PROP_RATE_CONTROL_PBMP:
            if (SOC_IS_ROBO53242(unit)) {
                *prop_val = DRV_HARRIER_RATE_CONTROL_SUPPORT_PBMP_BCM53242;
            } else {
                *prop_val = DRV_HARRIER_RATE_CONTROL_SUPPORT_PBMP_BCM53262;
            }
            break;
        case DRV_DEV_PROP_VLAN_ENTRY_NUM:
            *prop_val = DRV_HARRIER_VLAN_ENTRY_NUM;
            break;
        case DRV_DEV_PROP_BPDU_NUM:
            *prop_val = DRV_HARRIER_BPDU_NUM;
            break;
        case DRV_DEV_PROP_CFP_TCAM_SIZE:
            *prop_val = DRV_HARRIER_CFP_TCAM_SIZE;
            break;
        case DRV_DEV_PROP_CFP_UDFS_NUM:
            *prop_val = DRV_HARRIER_CFP_UDFS_NUM;
            break;
        case DRV_DEV_PROP_CFP_RNG_NUM:
            *prop_val = DRV_HARRIER_CFP_RNG_NUM;
            break;
        case DRV_DEV_PROP_CFP_VID_RNG_NUM:
            *prop_val = DRV_HARRIER_CFP_VID_RNG_NUM;
            break;
        case DRV_DEV_PROP_CFP_UDFS_OFFSET_MAX:
            *prop_val = DRV_HARRIER_CFP_UDFS_OFFSET_MAX;
            break;
        case DRV_DEV_PROP_AUTH_SEC_MODE:
            *prop_val = DRV_HARRIER_AUTH_SEC_MODE;
            break;            
        case DRV_DEV_PROP_AGE_HIT_VALUE:
            *prop_val = 0x1;
            break;
        case DRV_DEV_PROP_MAX_INGRESS_SFLOW_VALUE:
            /* Max value at INGRESS_RMONr.INGRESS_CFGf */
            *prop_val = 0xF;
            break;
        case DRV_DEV_PROP_MAX_EGRESS_SFLOW_VALUE:
            /* Max value at EGRESS_RMONr.EGRESS_CFGf */
            *prop_val = 0xF;
            break;
        case DRV_DEV_PROP_SUPPORTED_LED_FUNCTIONS:
            *prop_val = DRV_LED_FUNC_ALL_MASK & 
                    ~(DRV_LED_FUNC_PHYLED4 | DRV_LED_FUNC_PHYLED3 |
                    DRV_LED_FUNC_EAV_LINK | DRV_LED_FUNC_SP_100_200 | 
                    DRV_LED_FUNC_100_200_ACT | DRV_LED_FUNC_LNK_ACT_SP);
            break;
        default:
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}


