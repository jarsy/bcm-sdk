/*
 * $Id: dev.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h> 
#include "robo_bb.h"
#include <soc/drv.h> 

int
_drv_blackbird_station_move_drop_set(int unit, uint32 prop_val)
{
    int         rv = SOC_E_NONE;
    uint32      reg_value, enable;
    
    if ((rv = REG_READ_GARLCFGr(unit, &reg_value)) < 0) {
       return rv;
    }

    /* 
     * If the ARL control mode = 10 (BCM_L2_DISCARD_SRC), then
     * enable = 0 : Drop packet if SA match (Default, allow station movement) 
     * enable = 1 : Drop packet if SA match and port number not match 
     * (Not allow station movement)
     */
    if (prop_val) {
        enable = 1;
    } else {
        enable = 0;
    }
    soc_GARLCFGr_field_set(unit, &reg_value, 
        SA_MOVE_DROPf, &enable);

    if ((rv = REG_WRITE_GARLCFGr(unit, &reg_value)) < 0) {
       return rv;
    }        

    return rv;
}

/*
 *  Function : drv_blackbird_dev_prop_get
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
drv_blackbird_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    switch (prop_type) {
        case DRV_DEV_PROP_MCAST_NUM:
            *prop_val = DRV_BB_MCAST_GROUP_NUM;
            break;
        case DRV_DEV_PROP_AGE_TIMER_MAX_S:
            *prop_val = DRV_BB_AGE_TIMER_MAX;
            break;
        case DRV_DEV_PROP_TRUNK_NUM:
            *prop_val = DRV_BB_TRUNK_GROUP_NUM;
            break;
        case DRV_DEV_PROP_TRUNK_MAX_PORT_NUM:
            *prop_val = DRV_BB_TRUNK_MAX_PORT_NUM;
            break;
        case DRV_DEV_PROP_COSQ_NUM:
            *prop_val = NUM_COS(unit);
            break;
        case DRV_DEV_PROP_MSTP_NUM:
            *prop_val = DRV_BB_MSTP_GROUP_NUM;
            break;
        case DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT:
            *prop_val = DRV_BB_SEC_MAC_NUM_PER_PORT;
            break;
        case DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE:
            *prop_val = DRV_BB_COS_QUEUE_MAX_WEIGHT_VALUE;
            break;
        case DRV_DEV_PROP_AUTH_PBMP:
            *prop_val = DRV_BB_AUTH_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_RATE_CONTROL_PBMP:
            *prop_val = DRV_BB_RATE_CONTROL_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_VLAN_ENTRY_NUM:
            *prop_val = DRV_BB_VLAN_ENTRY_NUM;
            break;
        case DRV_DEV_PROP_BPDU_NUM:
            *prop_val = DRV_BB_BPDU_NUM;
            break;
        case DRV_DEV_PROP_AUTH_SEC_MODE:
            *prop_val = DRV_BB_AUTH_SEC_MODE;
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

/*
 *  Function : drv_blackbird_dev_prop_set
 *
 *  Purpose :
 *     Set the device property information
 *
 *  Parameters :
 *      unit        :   unit id
 *      prop_type   :   property type
 *      prop_val     :   property value of the property type
 *
 *  Return :
 *      SOC_E_UNAVAIL 
 *
 *  Note :
 *      This function is to set the device porperty information.
 *
 */
int 
drv_blackbird_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    int         rv = SOC_E_NONE;

    switch (prop_type) {
        case DRV_DEV_PROP_SA_STATION_MOVE_DROP:
            rv = _drv_blackbird_station_move_drop_set(unit, prop_val);
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}

