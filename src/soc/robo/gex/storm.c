/*
 * $Id: storm.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <gex/robo_gex.h>

/*
 *  Function : _drv_gex_storm_control_type_enable_set
 *
 *  Purpose :
 *      Set the status and types of storm control.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      port       :  port number.
 *      type       :  storm control types.
 *      enable     :  status of the storm control types.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
static int 
_drv_gex_storm_control_type_enable_set(int unit, uint32 port, 
    uint32 type, uint32 enable)
{
    uint32  temp;
    uint32  reg_value;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    uint32  specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }

    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) 
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_P7r
            (unit, &reg_value));
    } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }
    
    temp = enable;
    if (type & DRV_STORM_CONTROL_RSV_MCAST) {
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, EN_RSV_MLTCST_SUPf, &temp));
    }
    if (type & DRV_STORM_CONTROL_BCAST) {
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, EN_BCAST_SUPf, &temp));
    }
    if (type & DRV_STORM_CONTROL_MCAST) {
        /* Storm Mcast Suppression : Mcast lookup hit and failed */
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, EN_MCAST_SUPf, &temp));
    } 
    if (type & DRV_STORM_CONTROL_DLF) {
        /* Storm DLF Suppression : Unicast lookup hit and failed */
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, EN_DLF_SUPf, &temp));
    }

#ifdef BCM_LOTUS_SUPPORT
    if (SOC_IS_LOTUS(unit)) {
        if (type & DRV_STORM_CONTROL_VLAN_POLICING) {
            /* VLAN based policing */
            SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
                (unit, &reg_value, EN_VLAN_POLICINGf, &temp));
        }
    }
#endif /* BCM_LOTUS_SUPPORT */

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) 
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_P7r
            (unit, &reg_value));
    } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    return SOC_E_NONE;
}

/*
 *  Function : _drv_gex_storm_control_type_enable_get
 *
 *  Purpose :
 *      Get the status and types of storm control.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      port       :  port number.
 *      type       :  (OUT) storm control types.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
static int 
_drv_gex_storm_control_type_enable_get(int unit, uint32 port, 
    uint32 *type)
{
    uint32  temp = 0;
    uint32  reg_value;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    uint32  specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }

    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) 
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_P7r
            (unit, &reg_value));
    } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    *type = 0;
    SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
        (unit, &reg_value, EN_BCAST_SUPf, &temp));
    if (temp) {
        *type |= DRV_STORM_CONTROL_BCAST;
    }

    SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
        (unit, &reg_value, EN_MCAST_SUPf, &temp));
    if (temp) {
        *type |= DRV_STORM_CONTROL_MCAST;
    }

    SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
        (unit, &reg_value, EN_DLF_SUPf, &temp));   
    if (temp) {
        *type |= DRV_STORM_CONTROL_DLF;
    }

    SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
        (unit, &reg_value, EN_RSV_MLTCST_SUPf, &temp));
    if (temp) {
        *type |= DRV_STORM_CONTROL_RSV_MCAST;
    }

#ifdef BCM_LOTUS_SUPPORT
    if (SOC_IS_LOTUS(unit)) {
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
            (unit, &reg_value, EN_VLAN_POLICINGf, &temp));
        if (temp) {
            *type |= DRV_STORM_CONTROL_VLAN_POLICING;
        }
    }
#endif /* BCM_LOTUS_SUPPORT */

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_storm_control_enable_set
 *
 *  Purpose :
 *      Set the status for global storm control of selected port.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      port       :  port number.
 *      enable     :  status of the storm control (global).
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
int 
drv_gex_storm_control_enable_set(int unit, uint32 port, uint8 enable)
{
    uint32  temp;
    uint32  reg_value;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    uint32  specified_port_num;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_gex_storm_control_enable_set: \
                         unit = %d, port = %d, %sable\n"), unit, port, (enable) ? "en" : "dis"));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    /* Enable drop_enable bit of bucket 1 */
    SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
        (unit, &reg_value));
    temp = 1;
    SOC_IF_ERROR_RETURN(soc_COMM_IRC_CONr_field_set
        (unit, &reg_value, DROP_EN1f, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_COMM_IRC_CONr
        (unit, &reg_value));

    /* Enable Ingress rate control bit */
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) 
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_P7r
            (unit, &reg_value));
    } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    temp = enable;
    SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
        (unit, &reg_value, EN_BUCKET1f, &temp));

    /* Enable Suppression bit of Storm Control */
    SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
        (unit, &reg_value, EN_STORM_SUPf, &temp));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit))
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_P7r
            (unit, &reg_value));
    } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_storm_control_enable_get
 *
 *  Purpose :
 *      Get the status for global storm control of selected port.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      port       :  port number.
 *      enable     :  (OUT) status of the storm control (global).
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
int 
drv_gex_storm_control_enable_get(int unit, uint32 port, uint8 *enable)
{
    uint32  temp = 0;
    uint32  reg_value;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    uint32  specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }

    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) 
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_P7r
            (unit, &reg_value));
    } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
        (unit, &reg_value, EN_BUCKET1f, &temp));
    *enable = temp;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_gex_storm_control_enable_get: \
                         unit = %d, port = %d, %sable\n"), unit, port, (*enable) ? "en" : "dis"));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_storm_control_set
 *
 *  Purpose :
 *      Set the types and limit value for storm control of selected port.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      bmp        :  port bitmap.
 *      type       :  types of strom control.
 *      limit      :  limit value of storm control (Kbits).
 *      burst_size :  burst bucket size (Kbits).
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
int 
drv_gex_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size)
{
    uint32  reg_value, temp;
    uint32  port;
    uint32  disable_type = 0, burst_kbyte;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    uint32  specified_port_num;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_gex_storm_control_set: \
                         unit = %d, bmp = 0x%x, type = 0x%x, limit = %dK\n"), 
              unit, SOC_PBMP_WORD_GET(bmp, 0), type, limit));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    PBMP_ITER(bmp, port) {
        /* 
        * Check maximum supported rate limit of FE and GE ports, 
        * respectively.
        */
        if (!SOC_PBMP_MEMBER(PBMP_GE_ALL(unit), port)) {
            if (limit > 100000) {
                return SOC_E_PARAM;
            }
        }

        if (limit == 0) {  /* Disable storm suppression type */
            SOC_IF_ERROR_RETURN(_drv_gex_storm_control_type_enable_set
                (unit, port, type, FALSE));
            /* Need diable ingress rate control ? */
        } else {
            disable_type = GEX_STORM_CONTROL_PKT_MASK;

#ifdef BCM_LOTUS_SUPPORT
            if (SOC_IS_LOTUS(unit)) {
                disable_type |= DRV_STORM_CONTROL_VLAN_POLICING;
            }
#endif /* BCM_LOTUS_SUPPORT */

            /* Set storm suppression type */
            SOC_IF_ERROR_RETURN(_drv_gex_storm_control_type_enable_set
                (unit, port, type, TRUE));

            disable_type ^= type;
            if (disable_type != 0) {
                SOC_IF_ERROR_RETURN(_drv_gex_storm_control_type_enable_set
                    (unit, port, disable_type, FALSE));
            }

            /* Set bucket 1 refresh count */
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) 
                && (port == specified_port_num)) {
                SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_P7r
                    (unit, &reg_value));
            } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
            {
                SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
                    (unit, port, &reg_value));
	        }

            /* Burst size */
            if (burst_size) {
                if (burst_size > (500 * 8)) {  /* 500 KB */
                    return SOC_E_PARAM;
                }
                burst_kbyte = (burst_size / 8);

#if defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
                if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
                    if (burst_kbyte <= 16) {  /* 16KB */
                        temp = 0;
                    } else if (burst_kbyte <= 20) {  /* 20KB */
                        temp = 1;
                    } else if (burst_kbyte <= 28) {  /* 28KB */
                        temp = 2;
                    } else if (burst_kbyte <= 44) {  /* 44KB */
                        temp = 3;
                    } else if (burst_kbyte <= 76) {  /* 76KB */
                        temp = 4;
                    } else {  /* else burst_kbyte <= 500 for 500KB */
                        temp = 7;
                    }
                } else 
#endif  /* BCM_LOTUS_SUPPORT (STARFIGHTER/POLAR/NORTHSTAR) */
                {
                    if (burst_kbyte <= 16) {  /* 16KB */
                        temp = 0;
                    } else if (burst_kbyte <= 20) {  /* 20KB */
                        temp = 1;
                    } else if (burst_kbyte <= 28) {  /* 28KB */
                        temp = 2;
                    } else if (burst_kbyte <= 40) {  /* 40KB */
                        temp = 3;
                    } else if (burst_kbyte <= 76) {  /* 76KB */
                        temp = 4;
                    } else if (burst_kbyte <= 140) {  /* 140KB */
                        temp = 5;
                    } else if (burst_kbyte <= 268) {  /* 268KB */
                        temp = 6;
                    } else {  /* else burst_kbyte <= 500 for 500KB */
                        temp = 7;
                    }
                }

                SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
                    (unit, &reg_value, BUCKET1_SIZEf, &temp));
            }
            
            /* Refresh count  (fixed type) */
            if (limit <= 1792) {  /* 64KB ~ 1.792MB */
                temp = ((limit - 1) / 64) + 1;
            } else if (limit <= 100000) {  /* 2MB ~ 100MB */
                temp = (limit / 1000 ) + 27;
            } else if (limit <= 1000000) {  /* 104MB ~ 1000MB */
                temp = (limit / 8000) + 115;
            } else {
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
                (unit, &reg_value, BUCKET1_REF_CNTf, &temp));
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) 
                && (port == specified_port_num)) {
                SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_P7r
                    (unit, &reg_value));
            } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
            {
                SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_Pr
                    (unit, port, &reg_value));
	        }
        }
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_storm_control_get
 *
 *  Purpose :
 *      Get the types and limit value for storm control of selected port.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      port       :  port number.
 *      type       :  (OUT) types of strom control.
 *      limit      :  (OUT) limit value of storm control (Kbits).
 *      burst_size :  (OUT) burst bucket size (Kbits).
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
int 
drv_gex_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size)
{
    uint32  reg_value, temp = 0;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
    uint32  specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    SOC_IF_ERROR_RETURN(_drv_gex_storm_control_type_enable_get
        (unit, port, type));
    if (!(*type)) {
        *limit = 0;
    } else {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
        if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) 
            && (port == specified_port_num)) {
            SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_P7r
                (unit, &reg_value));
        } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
        {
            SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
                (unit, port, &reg_value));
	    }

        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
            (unit, &reg_value, BUCKET1_REF_CNTf, &temp));
        if (temp <= 28) {
            *limit = temp * 64;
        } else if (temp <= 127) {
            *limit = (temp - 27) * 1000;
        } else if (temp <= 240) {
            *limit = (temp - 115) * 1000 * 8;
        } else {
            return SOC_E_INTERNAL;
        }

        /* Burst size */
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
            (unit, &reg_value, BUCKET1_SIZEf, &temp));

#if defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
        if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
            switch (temp) {
                case 0:
                    *burst_size = 16 * 8;  /* 16KB */
                    break;
                case 1:
                    *burst_size = 20 * 8;  /* 20KB */
                    break;
                case 2:
                    *burst_size = 28 * 8;  /* 28KB */
                    break;
                case 3:
                    *burst_size = 44 * 8;  /* 44KB */
                    break;
                case 4:
                    *burst_size = 76 * 8;  /* 76KB */
                    break;
                case 6:
                case 7:
                    *burst_size = 500 * 8;  /* 500KB */
                    break;
                default:
                    return SOC_E_INTERNAL;
            }
        } else 
#endif  /* BCM_LOTUS_SUPPORT (STARFIGHTER/POLAR/NORTHSTAR) */
        {
            switch (temp) {
                case 0:
                    *burst_size = 16 * 8;  /* 16KB */
                    break;
                case 1:
                    *burst_size = 20 * 8;  /* 20KB */
                    break;
                case 2:
                    *burst_size = 28 * 8;  /* 28KB */
                    break;
                case 3:
                    *burst_size = 40 * 8;  /* 40KB */
                    break;
                case 4:
                    *burst_size = 76 * 8;  /* 76KB */
                    break;
                case 5:
                    *burst_size = 140 * 8;  /* 140KB */
                    break;
                case 6:
                    *burst_size = 268 * 8;  /* 268KB */
                    break;
                case 7:
                    *burst_size = 500 * 8;  /* 500KB */
                    break;
                default:
                    return SOC_E_INTERNAL;
            }
        }
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_gex_storm_control_get: \
                         unit = %d, port = %d, type = 0x%x, limit = %dK\n"), 
              unit, port, *type, *limit));

    return SOC_E_NONE;
}
