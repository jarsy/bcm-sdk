/*
 * $Id: dev.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h> 
#include "robo_vulcan.h"
#include <soc/drv.h> 

/*
 *  Function : drv_vulcan_dev_prop_get
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
drv_vulcan_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    switch (prop_type) {
        case DRV_DEV_PROP_MCAST_NUM:
            *prop_val = DRV_VULCAN_MCAST_GROUP_NUM;
            break;
        case DRV_DEV_PROP_AGE_TIMER_MAX_S:
            *prop_val = DRV_VULCAN_AGE_TIMER_MAX;
            break;
        case DRV_DEV_PROP_TRUNK_NUM:
            *prop_val = DRV_VULCAN_TRUNK_GROUP_NUM;
            break;
        case DRV_DEV_PROP_TRUNK_MAX_PORT_NUM:
            *prop_val = DRV_VULCAN_TRUNK_MAX_PORT_NUM;
            break;
        case DRV_DEV_PROP_COSQ_NUM:
            *prop_val = NUM_COS(unit);
            break;
        case DRV_DEV_PROP_MSTP_NUM:
            *prop_val = DRV_VULCAN_MSTP_GROUP_NUM;
            break;
        case DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT:
            *prop_val = DRV_VULCAN_SEC_MAC_NUM_PER_PORT;
            break;
        case DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE:
            *prop_val = DRV_VULCAN_COS_QUEUE_MAX_WEIGHT_VALUE;
            break;
        case DRV_DEV_PROP_AUTH_PBMP:
            *prop_val = DRV_VULCAN_AUTH_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_RATE_CONTROL_PBMP:
            *prop_val = DRV_VULCAN_RATE_CONTROL_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_VLAN_ENTRY_NUM:
            *prop_val = DRV_VULCAN_VLAN_ENTRY_NUM;
            break;
        case DRV_DEV_PROP_BPDU_NUM:
            *prop_val = DRV_VULCAN_BPDU_NUM;
            break;
        case DRV_DEV_PROP_CFP_TCAM_SIZE:
            *prop_val = DRV_VULCAN_CFP_TCAM_SIZE;
            break;
        case DRV_DEV_PROP_CFP_UDFS_NUM:
            *prop_val = DRV_VULCAN_CFP_UDFS_NUM;
            break;
        case DRV_DEV_PROP_CFP_UDFS_OFFSET_MAX:
            *prop_val = DRV_VULCAN_CFP_UDFS_OFFSET_MAX;
            break;
        case DRV_DEV_PROP_AUTH_SEC_MODE:
            *prop_val = DRV_VULCAN_AUTH_SEC_MODE;
            break;            
        case DRV_DEV_PROP_AGE_HIT_VALUE:
            *prop_val = 0x1;
            break;
        case DRV_DEV_PROP_SUPPORTED_LED_FUNCTIONS:
            *prop_val = DRV_LED_FUNC_ALL_MASK & 
                    ~(DRV_LED_FUNC_SP_100_200 |  DRV_LED_FUNC_100_200_ACT | 
                    DRV_LED_FUNC_LNK_ACT_SP);
            break;
        default:
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

