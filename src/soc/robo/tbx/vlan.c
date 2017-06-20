/*
 * $Id: vlan.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include "robo_tbx.h"

static int tbx_vt_enable[SOC_MAX_NUM_SWITCH_DEVICES];

/*
 *  Function : drv_port_vlan_pvid_set
 *
 *  Purpose :
 *      Set the default tag value of the selected port.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      vid     :   vlan value.
 *      prio    :   priority value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_port_vlan_pvid_set(int unit, uint32 port, uint32 outer_tag, uint32 inner_tag)
{
    uint64  reg_value;
    uint32 vid, pri;
    
    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "drv_port_vlan_pvid_set: unit = %d, port = %d, outer_tag = 0x%x, inner_tag = 0x%x\n"),
              unit, port, outer_tag, inner_tag));

    SOC_IF_ERROR_RETURN(REG_READ_DEF_PORT_QOS_CFGr(unit, port, (void *)&reg_value));

    /* Write to DEF_PORT_QOS_CFGr(0x28, 0x00~0xa8), SVID, SDEI and SPCP fields */
    vid = VLAN_CTRL_ID(outer_tag);
    pri = VLAN_CTRL_PRIO(outer_tag);
    soc_DEF_PORT_QOS_CFGr_field_set(unit, (void *)&reg_value, SVIDf, &vid);
    soc_DEF_PORT_QOS_CFGr_field_set(unit, (void *)&reg_value, SPCPf, &pri);


    /* Write to DEF_PORT_QOS_CFGr(0x28, 0x00~0xa8), CVID, CDEI and CPCP fields */
    vid = VLAN_CTRL_ID(inner_tag);
    pri = VLAN_CTRL_PRIO(inner_tag);

    soc_DEF_PORT_QOS_CFGr_field_set(unit, (void *)&reg_value, CVIDf, &vid);
    soc_DEF_PORT_QOS_CFGr_field_set(unit, (void *)&reg_value, CPCPf, &pri);

    SOC_IF_ERROR_RETURN(REG_WRITE_DEF_PORT_QOS_CFGr(unit,port, (void *)&reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_port_vlan_pvid_get
 *
 *  Purpose :
 *      Get the default tag value of the selected port.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      vid     :   vlan value.
 *      prio    :   priority value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_port_vlan_pvid_get(int unit, uint32 port, uint32 *outer_tag, uint32 *inner_tag)
{
    uint64  reg_value;
    uint32 vid = 0, cfi = 0, pri = 0;

    SOC_IF_ERROR_RETURN(REG_READ_DEF_PORT_QOS_CFGr(unit, port, (void *)&reg_value));

    /* Read SVID, SDEI and SPCP fields from DEF_PORT_QOS_CFGr(0x28, 0x00~0xa8) */
    soc_DEF_PORT_QOS_CFGr_field_get(unit, (void *)&reg_value, SVIDf, &vid);
    soc_DEF_PORT_QOS_CFGr_field_get(unit, (void *)&reg_value, SPCPf, &pri);
    *outer_tag = VLAN_CTRL(pri, cfi, vid);

    /* Read CVID, CDEI and CPCP fields from DEF_PORT_QOS_CFGr(0x28, 0x00~0xa8) */
    soc_DEF_PORT_QOS_CFGr_field_get(unit, (void *)&reg_value, CVIDf, &vid);
    soc_DEF_PORT_QOS_CFGr_field_get(unit, (void *)&reg_value, CPCPf, &pri);
    *inner_tag = VLAN_CTRL(pri, cfi, vid);
    
    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "drv_port_vlan_pvid_get: unit = %d, port = %d, outer_tag = 0x%x, inner_tag = 0x%x\n"),
              unit, port, *outer_tag, *inner_tag));
    return SOC_E_NONE;
}

/*
 *  Function : drv_port_vlan_set
 *
 *  Purpose :
 *      Set the group member ports of the selected port. (port-base)
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      bmp     :   group member port bitmap.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_port_vlan_set(int unit, uint32 port, soc_pbmp_t bmp)
{
    int rv = SOC_E_NONE;
    soc_pbmp_t block_bmp;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "drv_port_vlan_set: unit = %d, port = %d, bmp = %x\n"),
              unit, port, SOC_PBMP_WORD_GET(bmp, 0)));

    SOC_PBMP_CLEAR(block_bmp);
    SOC_PBMP_ASSIGN(block_bmp, PBMP_ALL(unit));
    SOC_PBMP_REMOVE(block_bmp, bmp);

    rv = DRV_PORT_BLOCK_SET(unit, port, DRV_BLOCK_ALL, block_bmp);
    return rv;
}

/*
 *  Function : drv_port_vlan_get
 *
 *  Purpose :
 *      Get the group member ports of the selected port. (port-base)
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      bmp     :   group member port bitmap.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_port_vlan_get(int unit, uint32 port, soc_pbmp_t *bmp)
{
    int rv = SOC_E_NONE;
    soc_pbmp_t fwd_bmp;
    soc_pbmp_t block_bmp;

    SOC_PBMP_CLEAR(fwd_bmp);
    SOC_PBMP_ASSIGN(fwd_bmp, PBMP_ALL(unit));

    rv = DRV_PORT_BLOCK_GET(unit, port, DRV_BLOCK_ALL, &block_bmp);
    SOC_IF_ERROR_RETURN(rv);
    
    SOC_PBMP_REMOVE(fwd_bmp, block_bmp);
    SOC_PBMP_ASSIGN(*bmp, fwd_bmp);

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "drv_port_vlan_get: unit = %d, port = %d, bmp = %x\n"),
              unit, port, SOC_PBMP_WORD_GET(*bmp, 0)));

    return rv;
}

/*
 *  Function : drv_vlan_mode_set
 *
 *  Purpose :
 *      Set the VLAN mode. (port-base/tag-base)
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      mode   :   vlan mode.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_vlan_mode_set(int unit, uint32 mode)
{
    uint32  reg_value, temp;
    soc_pbmp_t bmp;

    SOC_PBMP_CLEAR(bmp);
    SOC_PBMP_ASSIGN(bmp, PBMP_ALL(unit));

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "drv_vlan_mode_set: unit = %d, mode = %d\n"),
              unit, mode));
    switch (mode) 
    {
        case DRV_VLAN_MODE_TAG:
            /* 
             * Set 802.1Q VLAN Enable.
             * No need to enable/disable 1Q Vlan on TBX.
             */

            /* Drop frame if ingress vid violation(non member drop) */
            temp = 1;
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, bmp, DRV_PORT_PROP_INGRESS_VLAN_CHK, temp));
            /* Drop frame if ingress vid unregistered */
            SOC_IF_ERROR_RETURN(
                DRV_VLAN_PROP_SET(unit, DRV_VLAN_PROP_VTABLE_MISS_DROP, temp));

            /* enable GMRP/GVRP been sent to CPU :
             *  - GMRP/GVRP frame won't sent to CPU without set this bit.
             */
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_value));
            temp = 1;
            soc_VLAN_GLOBAL_CTLr_field_set(unit, &reg_value, EN_MGE_REV_GVRPf, &temp);
            soc_VLAN_GLOBAL_CTLr_field_set(unit, &reg_value, EN_MGE_REV_GMRPf, &temp);

            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_GLOBAL_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_MODE_PORT_BASE:
            return SOC_E_UNAVAIL;
        default :
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_vlan_mode_get
 *
 *  Purpose :
 *      Get the VLAN mode. (port-base/tag-base)
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      mode   :   vlan mode.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_vlan_mode_get(int unit, uint32 *mode)
{
    /* 
     * No need to enable/disable 1Q Vlan on TBX.
     * Always return 1Q Vlan enabled.
     */
    *mode = DRV_VLAN_MODE_TAG;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "drv_vlan_mode_get: unit = %d, mode = %d\n"),
              unit, *mode));
    return SOC_E_NONE;
}

/*
 *  Function : drv_vlan_prop_set
 *
 *  Purpose :
 *      Set the VLAN property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32 reg_value = 0;
    uint32 temp = 0;
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit = %d, property type = %d, value = %x\n"),
              FUNCTION_NAME(), unit, prop_type, prop_val));

    switch (prop_type) {
        case DRV_VLAN_PROP_VTABLE_MISS_DROP:
            SOC_IF_ERROR_RETURN(REG_READ_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_GLB_VLAN_ING_FILTER_CTLr_field_set(unit, &reg_value, EN_UNREGISTERED_DROPf, &temp);
            SOC_IF_ERROR_RETURN(REG_WRITE_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_TRANSLATE_MODE:
            rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_VLAN_TRANSLATE_MODE_NUM, &temp);
            SOC_IF_ERROR_RETURN(rv);
            if (prop_val >= temp) {
                /* Configure value exceeds max supported number */
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_value));
            temp = prop_val;
            soc_VLAN_GLOBAL_CTLr_field_set(unit, &reg_value, VT_MODEf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_GLOBAL_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_value));
            if (prop_val) {
                /* SVL */
               temp = 0;
            } else {
                /* IVL */
               temp = 1;
            }        
            soc_VLAN_GLOBAL_CTLr_field_set(unit, &reg_value, VID_MAC_CTRLf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_GLOBAL_CTLr(unit, &reg_value));
            break;            
        case DRV_VLAN_PROP_GVRP_TO_CPU:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_VLAN_GLOBAL_CTLr_field_set(unit, &reg_value, EN_MGE_REV_GVRPf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_GLOBAL_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_GMRP_TO_CPU:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_VLAN_GLOBAL_CTLr_field_set(unit, &reg_value, EN_MGE_REV_GMRPf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_GLOBAL_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_BYPASS_IGMP_MLD:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_VLAN_BYPASS_CTLr_field_set(unit, &reg_value, VLAN_BYPASS_IGMP_MLDf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_BYPASS_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_BYPASS_ARP_DHCP:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_VLAN_BYPASS_CTLr_field_set(unit, &reg_value, VLAN_CPUCOPY_ARP_DHCPf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_BYPASS_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_BYPASS_MIIM:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_VLAN_BYPASS_CTLr_field_set(unit, &reg_value, VLAN_BYPASS_MIIMf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_BYPASS_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_BYPASS_MCAST:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_VLAN_BYPASS_CTLr_field_set(unit, &reg_value, VLAN_BYPASS_MCSTf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_BYPASS_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_BYPASS_RSV_MCAST:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_VLAN_BYPASS_CTLr_field_set(unit, &reg_value, VLAN_BYPASS_RSV_MCASTf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_BYPASS_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_BYPASS_L2_USER_ADDR:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_VLAN_BYPASS_CTLr_field_set(unit, &reg_value, VLAN_BYPASS_L2_USER_ADDRf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_BYPASS_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_VTABLE_MISS_LEARN:
            SOC_IF_ERROR_RETURN(REG_READ_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 0; /* Enable learn */
            } else {
                temp = 1; /* Disable learn */
            }
            soc_GLB_VLAN_ING_FILTER_CTLr_field_set(unit, &reg_value, DIS_LRN_UNREGISTEREDf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_VTABLE_MISS_TO_CPU:
            SOC_IF_ERROR_RETURN(REG_READ_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_GLB_VLAN_ING_FILTER_CTLr_field_set(unit, &reg_value, EN_UNREGISTERED_CPUCOPYf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_MEMBER_MISS_LEARN:
            SOC_IF_ERROR_RETURN(REG_READ_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 0; /* Enable learn */
            } else {
                temp = 1; /* Disable learn */
            }
            soc_GLB_VLAN_ING_FILTER_CTLr_field_set(unit, &reg_value, DIS_LRN_NONMEMBERf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_MEMBER_MISS_TO_CPU:
            SOC_IF_ERROR_RETURN(REG_READ_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            if (prop_val) {
                temp = 1;
            }
            soc_GLB_VLAN_ING_FILTER_CTLr_field_set(unit, &reg_value, EN_NONMEMBER_CPUCOPYf, &temp);
            
            SOC_IF_ERROR_RETURN(REG_WRITE_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            break;
        case DRV_VLAN_PROP_V2V:
            if (prop_val) {
                tbx_vt_enable[unit] = 1;
            } else {
                tbx_vt_enable[unit] = 0;
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }

    return rv;
}

/*
 *  Function : drv_vlan_prop_get
 *
 *  Purpose :
 *      Get the VLAN property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_vlan_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32  reg_value, temp = 0;
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit = %d, property type = %d, value = %x\n"),
              FUNCTION_NAME(), unit, prop_type, *prop_val));

    switch (prop_type) {
        case DRV_VLAN_PROP_VTABLE_MISS_DROP:
            SOC_IF_ERROR_RETURN(REG_READ_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            soc_GLB_VLAN_ING_FILTER_CTLr_field_get(unit, &reg_value, EN_UNREGISTERED_DROPf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_TRANSLATE_MODE:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_value));
            soc_VLAN_GLOBAL_CTLr_field_get(unit, &reg_value, VT_MODEf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_value));
            soc_VLAN_GLOBAL_CTLr_field_get(unit, &reg_value, VID_MAC_CTRLf, &temp);
            if (temp) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;            
        case DRV_VLAN_PROP_GVRP_TO_CPU:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_value));
            soc_VLAN_GLOBAL_CTLr_field_get(unit, &reg_value, EN_MGE_REV_GVRPf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_GMRP_TO_CPU:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_value));
            soc_VLAN_GLOBAL_CTLr_field_get(unit, &reg_value, EN_MGE_REV_GMRPf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_BYPASS_IGMP_MLD:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            soc_VLAN_BYPASS_CTLr_field_get(unit, &reg_value, VLAN_BYPASS_IGMP_MLDf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_BYPASS_ARP_DHCP:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            soc_VLAN_BYPASS_CTLr_field_get(unit, &reg_value, VLAN_CPUCOPY_ARP_DHCPf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_BYPASS_MIIM:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            soc_VLAN_BYPASS_CTLr_field_get(unit, &reg_value, VLAN_BYPASS_MIIMf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_BYPASS_MCAST:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            soc_VLAN_BYPASS_CTLr_field_get(unit, &reg_value, VLAN_BYPASS_MCSTf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_BYPASS_RSV_MCAST:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            soc_VLAN_BYPASS_CTLr_field_get(unit, &reg_value, VLAN_BYPASS_RSV_MCASTf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_BYPASS_L2_USER_ADDR:
            SOC_IF_ERROR_RETURN(REG_READ_VLAN_BYPASS_CTLr(unit, &reg_value));
            soc_VLAN_BYPASS_CTLr_field_get(unit, &reg_value, VLAN_BYPASS_L2_USER_ADDRf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_VTABLE_MISS_LEARN:
            SOC_IF_ERROR_RETURN(REG_READ_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            soc_GLB_VLAN_ING_FILTER_CTLr_field_get(unit, &reg_value, DIS_LRN_UNREGISTEREDf, &temp);
            if (temp) {
                *prop_val = 0; /* Learning disabled */
            } else {
                *prop_val = 1; /* Learning enabled */
            }
            break;
        case DRV_VLAN_PROP_VTABLE_MISS_TO_CPU:
            SOC_IF_ERROR_RETURN(REG_READ_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            soc_GLB_VLAN_ING_FILTER_CTLr_field_get(unit, &reg_value, EN_UNREGISTERED_CPUCOPYf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_MEMBER_MISS_LEARN:
            SOC_IF_ERROR_RETURN(REG_READ_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            soc_GLB_VLAN_ING_FILTER_CTLr_field_get(unit, &reg_value, DIS_LRN_NONMEMBERf, &temp);
            if (temp) {
                *prop_val = 0; /* Learning disabled */
            } else {
                *prop_val = 1; /* Learning enabled */
            }
            break;
        case DRV_VLAN_PROP_MEMBER_MISS_TO_CPU:
            SOC_IF_ERROR_RETURN(REG_READ_GLB_VLAN_ING_FILTER_CTLr(unit, &reg_value));
            soc_GLB_VLAN_ING_FILTER_CTLr_field_get(unit, &reg_value, EN_NONMEMBER_CPUCOPYf, &temp);
            *prop_val = temp;            
            break;
        case DRV_VLAN_PROP_V2V:
            if (tbx_vt_enable[unit]) {
                *prop_val = 1;
            } else {
                *prop_val = 0;
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    
    return rv;
}

/*
 *  Function : drv_vlan_prop_port_enable_set
 *
 *  Purpose :
 *      Set the port enable status by different VLAN property.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_val    :   vlan property value.
 *      bmp         : port bitmap
 *      val         : value
 *
 *  Return :
 *      SOC_E_NONE
 *
 */
int 
drv_tbx_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val)
{
    uint32  reg_value;
    uint64  reg_value64;
    uint32 temp = 0, profile = 0;
    int rv = SOC_E_NONE;
    soc_pbmp_t set_bmp, temp_bmp;
    soc_port_t port;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit=%d, prop=%d, value=0x%x\n"), 
              FUNCTION_NAME(), unit, prop_type, val));

    if (SOC_PBMP_IS_NULL(bmp)){ /* no port to set */
        return SOC_E_NONE;
    }

    switch(prop_type){
        case    DRV_VLAN_PROP_UNTAGGED_DROP :
            rv = DRV_PORT_SET(unit, bmp, DRV_PORT_PROP_ENABLE_DROP_UNTAG, val);
            return rv;
            break;
        case    DRV_VLAN_PROP_PRI_TAGGED_DROP :
            rv = DRV_PORT_SET(unit, bmp, DRV_PORT_PROP_ENABLE_DROP_PRITAG, val);
            return rv;
            break;
        case    DRV_VLAN_PROP_INGRESS_VT_HIT_DROP :
            rv = DRV_PORT_SET(unit, bmp, DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_HIT, val);
            return rv;
            break;
        case    DRV_VLAN_PROP_INGRESS_VT_MISS_DROP :
            rv = DRV_PORT_SET(unit, bmp, DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_MISS, val);
            return rv;
            break;
        case    DRV_VLAN_PROP_MEMBER_MISS_DROP:
            rv = DRV_PORT_SET(unit, bmp, DRV_PORT_PROP_INGRESS_VLAN_CHK, val);
            return rv;
            break;
        case DRV_VLAN_PROP_EGRESS_VT_MISS_UNTAG:
            /* 
              * 1. Get the port's port profile id. 
              * 2. Configure the port profile according to the parameter.
              */
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(REG_READ_DEF_PORT_QOS_CFGr(unit, port, (void *)&reg_value64));
            
                /* Read port profile id from DEF_PORT_QOS_CFGr */
                soc_DEF_PORT_QOS_CFGr_field_get(unit, (void *)&reg_value64, PORT_PROFILEf, &profile);

                SOC_IF_ERROR_RETURN(REG_READ_PROFILE_CTLr(unit, &reg_value));
                soc_PROFILE_CTLr_field_get(unit, &reg_value, EVM_MISS_PROFILE_CTLf, &temp);
                if (val) {
                    /* Untag EVM Missed packet's outer tag. */
                    temp &= ~(1 << profile);
                } else {
                    /* Keep EVM Missed packet's outer tag. */
                    temp |= 1 << profile;
                }
                soc_PROFILE_CTLr_field_set(unit, &reg_value, EVM_MISS_PROFILE_CTLf, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_PROFILE_CTLr(unit, &reg_value));
            }
            return SOC_E_NONE;
            break;
        case    DRV_VLAN_PROP_TRUST_VLAN_PORT :
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_CVIDr(unit, &reg_value));
            soc_TRUST_CVIDr_field_get(unit, &reg_value, TRUST_CVIDf, &temp);
            break;
        case    DRV_VLAN_PROP_ISP_PORT :
            SOC_IF_ERROR_RETURN(REG_READ_ISP_SEL_PORTMAPr(unit, &reg_value));
            soc_ISP_SEL_PORTMAPr_field_get(unit, &reg_value, ISP_PORTMAPf, &temp);
            break;
        case DRV_VLAN_PROP_V2V_PORT:
            if (val) {
                tbx_vt_enable[unit] = 1;
            } else {
                tbx_vt_enable[unit] = 0;
            }
            return SOC_E_NONE;
        default :
            return SOC_E_UNAVAIL;
    }
    SOC_PBMP_CLEAR(temp_bmp);
    SOC_PBMP_WORD_SET(temp_bmp, 0, temp);
    
    /* check the action process */
    SOC_PBMP_CLEAR(set_bmp);
    SOC_PBMP_OR(set_bmp, temp_bmp);
    
    if (val == TRUE){       /* set for enable */
        SOC_PBMP_OR(set_bmp, bmp);
    }else {
        SOC_PBMP_REMOVE(set_bmp, bmp);
    }

    /* check if the set value is equal to current setting */
    if (SOC_PBMP_EQ(temp_bmp, set_bmp)){
        /* do nothing */
        return SOC_E_NONE;
    }

    /* write to register */
    temp = SOC_PBMP_WORD_GET(set_bmp, 0);    
    switch(prop_type){
        case    DRV_VLAN_PROP_TRUST_VLAN_PORT :
            soc_TRUST_CVIDr_field_set(unit, &reg_value, TRUST_CVIDf, &temp);
            SOC_IF_ERROR_RETURN(REG_WRITE_TRUST_CVIDr(unit, &reg_value));
            break;
        case    DRV_VLAN_PROP_ISP_PORT :
            soc_ISP_SEL_PORTMAPr_field_set(unit, &reg_value, ISP_PORTMAPf, &temp);
            SOC_IF_ERROR_RETURN(REG_WRITE_ISP_SEL_PORTMAPr(unit, &reg_value));
            break;
    }

    return rv;
}

/*
 *  Function : drv_vlan_prop_port_enable_get
 *
 *  Purpose :
 *      Get the port enable status by different VLAN property.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_val    :   vlan property value.
 *      port_n      : port number. 
 *      val         : (OUT) value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. if port_n = 0xffffffff , means get device basis value.
 *
 */
int 
drv_tbx_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 *val)
{
    uint32 reg_value;
    uint64 reg_value64;
    uint32 temp = 0, profile = 0;
    soc_pbmp_t pbmp;
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit=%d, prop=%d, port=%d\n"), 
              FUNCTION_NAME(), unit, prop_type, port_n));

    switch(prop_type){
        case    DRV_VLAN_PROP_UNTAGGED_DROP :
            rv = DRV_PORT_GET(unit, port_n, DRV_PORT_PROP_ENABLE_DROP_UNTAG, val);
            return rv;
            break;
        case    DRV_VLAN_PROP_PRI_TAGGED_DROP :
            rv = DRV_PORT_GET(unit, port_n, DRV_PORT_PROP_ENABLE_DROP_PRITAG, val);
            return rv;
            break;
        case    DRV_VLAN_PROP_INGRESS_VT_HIT_DROP :
            rv = DRV_PORT_GET(unit, port_n, DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_HIT, val);
            return rv;
            break;
        case    DRV_VLAN_PROP_INGRESS_VT_MISS_DROP :
            rv = DRV_PORT_GET(unit, port_n, DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_MISS, val);
            return rv;
            break;
        case    DRV_VLAN_PROP_MEMBER_MISS_DROP:
            rv = DRV_PORT_GET(unit, port_n, DRV_PORT_PROP_INGRESS_VLAN_CHK, val);
            return rv;
            break;
        case DRV_VLAN_PROP_EGRESS_VT_MISS_UNTAG:
            /* 
              * 1. Get the port's port profile id. 
              * 2. Configure the port profile according to the parameter.
              */
            SOC_IF_ERROR_RETURN(REG_READ_DEF_PORT_QOS_CFGr(unit, port_n, (void *)&reg_value64));
        
            /* Read port profile id from DEF_PORT_QOS_CFGr */
            soc_DEF_PORT_QOS_CFGr_field_get(unit, (void *)&reg_value64, PORT_PROFILEf, &profile);

            SOC_IF_ERROR_RETURN(REG_READ_PROFILE_CTLr(unit, &reg_value));
            soc_PROFILE_CTLr_field_get(unit, &reg_value, EVM_MISS_PROFILE_CTLf, &temp);
            if (temp & (1 << profile)) {
                /* 
                  * The profile configuration is "As Is" for outer tag of 
                  * EVM missed packets. 
                  */
                *val = FALSE;
            } else {
                /* 
                  * The profile configuration is "Remove" for outer tag of 
                  * EVM missed packets. 
                  */
                *val = TRUE;
            }
            return SOC_E_NONE;
            break;
        case    DRV_VLAN_PROP_TRUST_VLAN_PORT :
            SOC_IF_ERROR_RETURN(REG_READ_TRUST_CVIDr(unit, &reg_value));
            soc_TRUST_CVIDr_field_get(unit, &reg_value, TRUST_CVIDf, &temp);
            break;
        case    DRV_VLAN_PROP_ISP_PORT :
            SOC_IF_ERROR_RETURN(REG_READ_ISP_SEL_PORTMAPr(unit, &reg_value));
            soc_ISP_SEL_PORTMAPr_field_get(unit, &reg_value, ISP_PORTMAPf, &temp);
            break;
        case DRV_VLAN_PROP_V2V_PORT:
            if (tbx_vt_enable[unit]) {
                *val = 1;
            } else {
                *val = 0;
            }
            return SOC_E_NONE;
        default :
            return SOC_E_UNAVAIL;
    }

    /* check if the value get is port basis or device basis. */
    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, temp);

    if (port_n == 0xffffffff) {     /* device basis */
        int     i;

        for (i = 0; i < SOC_PBMP_WORD_MAX; i++){
            *(val + i) = SOC_PBMP_WORD_GET(pbmp, i);
        }
    } else {
        *val = (SOC_PBMP_MEMBER(pbmp, port_n)) ? TRUE : FALSE;
    }

    return rv;
}

