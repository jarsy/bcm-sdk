/*
 * $Id: mstp.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>

int gex_mst_field[] = { 
            INDEX(SPT_STA0f), INDEX(SPT_STA1f), INDEX(SPT_STA2f), 
            INDEX(SPT_STA3f), INDEX(SPT_STA4f), INDEX(SPT_STA5f), 
            INDEX(SPT_STA6f), INDEX(SPT_STA7f)};
/*
 *  Function : drv_gex_mstp_port_set
 *
 *  Purpose :
 *      Set the port state of a selected stp id.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mstp_gid    :   multiple spanning tree id.
 *      port        :   port number.
 *      port_state  :   state of the port.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_gex_mstp_port_set(int unit, uint32 mstp_gid, 
                           uint32 port, uint32 port_state)
{
    uint32  temp;
    uint32  reg_len, reg_addr;
    uint32  reg32, reg_index = 0;
    uint32  max_gid;
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
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS
        */
    
    LOG_INFO(BSL_LS_SOC_STP, \
             (BSL_META_U(unit, \
                         "drv_mstp_port_set : \
                         unit %d, STP id = %d, port = %d, port_state = %d \n"),
              unit, mstp_gid, port, port_state));
    if (port == CMIC_PORT(unit)) {
        return SOC_E_NONE;
    }
    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_MSTP_NUM, &max_gid));
    if (!soc_feature(unit, soc_feature_mstp)) {
           /* error checking */
        if (mstp_gid != STG_ID_DEFAULT) {
            return SOC_E_PARAM;
        }
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
        if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit)) 
            && (port == specified_port_num)) {
            SOC_IF_ERROR_RETURN(REG_READ_P7_CTLr(unit, &reg32));
        } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
        {
            SOC_IF_ERROR_RETURN(REG_READ_G_PCTLr(unit, port, &reg32));
        }
        switch (port_state)
        {
            case DRV_PORTST_DISABLE:
                temp = 1;
                break;
            case DRV_PORTST_BLOCK:
                temp = 2;
                break;
            case DRV_PORTST_LISTEN:
#ifdef BCM_DINO8_SUPPORT
                /* Listen and Learn are identical states */
                if (SOC_IS_DINO8(unit)) {
                    temp = 4;
                } else
#endif /* BCM_DINO8_SUPPORT */
                {
                    temp = 3;
                }
                break;
            case DRV_PORTST_LEARN:
                temp = 4;
                break;
            case DRV_PORTST_FORWARD:
                temp = 5;
                break;
            default:
                return SOC_E_PARAM;
        }

        soc_G_PCTLr_field_set(unit, &reg32, G_MISTP_STATEf, &temp);  

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
        if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit)) 
            && (port == specified_port_num)) {
            SOC_IF_ERROR_RETURN(REG_WRITE_P7_CTLr(unit, &reg32));
        } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT ||
        * BCM_NORTHSTARPLUS_SUPPORT
        */
        {
            SOC_IF_ERROR_RETURN(REG_WRITE_G_PCTLr(unit, port, &reg32));
        }
    } else {

        /* error checking */
        if ((mstp_gid > max_gid) || (mstp_gid < STG_ID_DEFAULT)) {
            return SOC_E_PARAM;
        }
        mstp_gid = mstp_gid % max_gid;
        if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
            reg_index = INDEX(MST_TBLr);
        } else {
            reg_index = INDEX(MST_TABr);
        }
        reg_addr = DRV_REG_ADDR(unit, reg_index, 0, mstp_gid);

        reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
        SOC_IF_ERROR_RETURN(DRV_REG_READ(unit, reg_addr, &reg32, reg_len));
    
        switch (port_state)
        {
            case DRV_PORTST_DISABLE:
                temp = 1;
                break;
            case DRV_PORTST_BLOCK:
                temp = 2;
                break;
            case DRV_PORTST_LISTEN:
                temp = 3;
                break;
            case DRV_PORTST_LEARN:
                temp = 4;
                break;
            case DRV_PORTST_FORWARD:
                temp = 5;
                break;
            default:
                return SOC_E_PARAM;
        }
    
        SOC_IF_ERROR_RETURN(DRV_REG_FIELD_SET
                (unit, reg_index, &reg32, gex_mst_field[port], &temp));  

        SOC_IF_ERROR_RETURN(DRV_REG_WRITE(unit, reg_addr, &reg32, reg_len));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_mstp_port_get
 *
 *  Purpose :
 *      Get the port state of a selected stp id.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mstp_gid    :   multiple spanning tree id.
 *      port        :   port number.
 *      port_state  :   state of the port.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_gex_mstp_port_get(int unit, uint32 mstp_gid, 
                                uint32 port, uint32 *port_state)
{
    uint32  portstate = 0;
    uint32  reg_len, reg_addr, reg32, reg_index = 0;
    uint32  max_gid;
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
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT ||
        * BCM_NORTHSTARPLUS_SUPPORT
        */

    if (port == CMIC_PORT(unit)) {
        *port_state = DRV_PORTST_DISABLE;
        return SOC_E_NONE;
    }
    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_MSTP_NUM, &max_gid));
    if (!soc_feature(unit, soc_feature_mstp)) {
        /* error checking */
        if (mstp_gid != STG_ID_DEFAULT) {
            return SOC_E_PARAM;
        }
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
        if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit)) 
            && (port == specified_port_num)) {
            SOC_IF_ERROR_RETURN(REG_READ_P7_CTLr(unit, &reg32));
        } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
        {
            SOC_IF_ERROR_RETURN(REG_READ_G_PCTLr(unit, port, &reg32));
        }
        soc_G_PCTLr_field_get(unit, &reg32, G_MISTP_STATEf, &portstate);
    
        switch (portstate)
        {
            case 1:
                *port_state = DRV_PORTST_DISABLE;
                break;
            case 2:
                *port_state = DRV_PORTST_BLOCK;
                break;
            case 3:
#ifdef BCM_DINO8_SUPPORT
                /* Listen and Learn are identical states */
                if (SOC_IS_DINO8(unit)) {
                    return SOC_E_INTERNAL;
                } else
#endif /* BCM_DINO8_SUPPORT */
                {
                    *port_state = DRV_PORTST_LISTEN;
                }
                break;
            case 4:
                *port_state = DRV_PORTST_LEARN;
                break;
            case 5:
                *port_state = DRV_PORTST_FORWARD;
                break;
            default:
                return SOC_E_INTERNAL;
        }
    } else {
        /* error checking */
        if ((mstp_gid > max_gid) || (mstp_gid < STG_ID_DEFAULT)) {
            return SOC_E_PARAM;
        }
        mstp_gid = mstp_gid % max_gid;
        if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
            reg_index = INDEX(MST_TBLr);
        } else {
            reg_index = INDEX(MST_TABr);
        }
        reg_addr = DRV_REG_ADDR(unit, reg_index, 0, mstp_gid);
        reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
        SOC_IF_ERROR_RETURN(DRV_REG_READ(unit, reg_addr, &reg32, reg_len));
        SOC_IF_ERROR_RETURN(DRV_REG_FIELD_GET
                (unit, reg_index, (uint32 *)&reg32, 
                gex_mst_field[port], &portstate)); 
   
        switch (portstate)
        {
            case 1:
                *port_state = DRV_PORTST_DISABLE;
                break;
            case 2:
                *port_state = DRV_PORTST_BLOCK;
                break;
            case 3:
                *port_state = DRV_PORTST_LISTEN;
                break;
            case 4:
                *port_state = DRV_PORTST_LEARN;
                break;
            case 5:
                *port_state = DRV_PORTST_FORWARD;
                break;
            default:
                return SOC_E_INTERNAL;
        }
    }

    LOG_INFO(BSL_LS_SOC_STP, \
             (BSL_META_U(unit, \
                         "drv_mstp_port_get : \
                         unit %d, STP id = %d, port = %d, port_state = %d \n"),
              unit, mstp_gid, port, *port_state));

    return SOC_E_NONE;
}
