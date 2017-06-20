/*
 * $Id: vlan.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>

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
drv_vlan_mode_set(int unit, uint32 mode)
{
    uint32	reg_value, temp;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "drv_vlan_mode_set: unit = %d, mode = %d\n"),
              unit, mode));
    switch (mode) 
    {
        case DRV_VLAN_MODE_TAG:
            /* set 802.1Q VLAN Enable */
            SOC_IF_ERROR_RETURN(
                REG_READ_VLAN_CTRL0r(unit, &reg_value));
            temp = 1;
            soc_VLAN_CTRL0r_field_set(unit, &reg_value, 
                VLAN_ENf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_VLAN_CTRL0r(unit, &reg_value));              
            /* enable GMRP/GVRP been sent to CPU :
             *  - GMRP/GVRP frame won't sent to CPU without set this bit.
             */
            SOC_IF_ERROR_RETURN(
                REG_READ_VLAN_CTRL4r(unit, &reg_value));
            temp = 1; /* Drop frame if ingress vid violation */
            soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                INGR_VID_CHKf, &temp);

            temp = 1;
            soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                EN_MGE_REV_GVRPf, &temp);
            soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                EN_MGE_REV_GMRPf, &temp);
            
            SOC_IF_ERROR_RETURN(
                REG_WRITE_VLAN_CTRL4r(unit, &reg_value));
                
            break;
        case DRV_VLAN_MODE_PORT_BASE:
            /* set 802.1Q VLAN Disable */
            SOC_IF_ERROR_RETURN(
                REG_READ_VLAN_CTRL0r(unit, &reg_value));
            temp = 0;
            soc_VLAN_CTRL0r_field_set(unit, &reg_value, 
                VLAN_ENf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_VLAN_CTRL0r(unit, &reg_value));
            break;
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
drv_vlan_mode_get(int unit, uint32 *mode)
{
    uint32	reg_value, temp = 0;

    SOC_IF_ERROR_RETURN(
        REG_READ_VLAN_CTRL0r(unit, &reg_value));
    soc_VLAN_CTRL0r_field_get(unit, &reg_value, 
                VLAN_ENf, &temp);
    if (temp) {
        *mode = DRV_VLAN_MODE_TAG;
    } else {
        *mode = DRV_VLAN_MODE_PORT_BASE;
    }
    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "drv_vlan_mode_get: unit = %d, mode = %d\n"),
              unit, *mode));
    return SOC_E_NONE;
}

/* config port base vlan */
/*
 *  Function : drv_port_vlan_pvid_set
 *
 *  Purpose :
 *      Set the default tag value of the selected port.
 *
 *  Parameters :
 *      unit    :   RoboSwitch unit number.
 *      port    :   port number.
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
drv_port_vlan_pvid_set(int unit, uint32 port, uint32 outer_tag, uint32 inner_tag)
{
    uint32	reg_value;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32  specified_port_num;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ */

    LOG_INFO(BSL_LS_SOC_VLAN, \
             (BSL_META_U(unit, \
                         "drv_port_vlan_pvid_set: \
                         unit = %d, port = %d, outer_tag = 0x%x, inner_tag = 0x%x\n"),
              unit, port, outer_tag, inner_tag));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ */

    reg_value = outer_tag;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit)) 
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(
            REG_WRITE_DEFAULT_1Q_TAG_P7r(unit, &reg_value));
    } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ */
    {
        SOC_IF_ERROR_RETURN(
            REG_WRITE_DEFAULT_1Q_TAGr(unit, port, &reg_value));
    }

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
drv_port_vlan_pvid_get(int unit, uint32 port, uint32 *outer_tag, uint32 *inner_tag)
{
    uint32	reg_value;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32  specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }

    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit)) 
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_DEFAULT_1Q_TAG_P7r(unit, &reg_value));
    } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ */
    {
        SOC_IF_ERROR_RETURN(
            REG_READ_DEFAULT_1Q_TAGr(unit, port, &reg_value));
    }

    *outer_tag = reg_value;
    
    LOG_INFO(BSL_LS_SOC_VLAN, \
             (BSL_META_U(unit, \
                         "drv_port_vlan_pvid_get: \
                         unit = %d, port = %d, outer_tag = 0x%x, inner_tag = 0x%x\n"),
              unit, port, *outer_tag, *inner_tag));

    return SOC_E_NONE;
}

