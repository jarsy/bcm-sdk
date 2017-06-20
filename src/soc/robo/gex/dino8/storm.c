/*
 * $Id: storm.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <gex/dino8/robo_dino8.h>

/*
 *  Function : _drv_dino8_storm_control_type_enable_set
 *
 *  Purpose :
 *      Set the status and types of storm control.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      type       :  storm control types.
 *      enable     :  status of the storm control types.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
static int 
_drv_dino8_storm_control_type_enable_set(int unit, uint32 type, 
    uint32 enable)
{
    uint32  temp = 0;
    uint32  reg_value;

    /* Set storm suppressed packet types of bucket 1 (Global) */
    SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
        (unit, &reg_value));
    soc_COMM_IRC_CONr_field_get(unit, &reg_value, 
        PKT_MSK1f, &temp);

    if (type & DRV_STORM_CONTROL_BCAST) {
        if (enable) {
            temp |= DINO8_STORM_SUPPRESSION_BROADCAST_MASK;
        } else {
            temp &= ~DINO8_STORM_SUPPRESSION_BROADCAST_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_MCAST) {
        if (enable) {
            temp |= DINO8_STORM_SUPPRESSION_MULTICAST_MASK;
        } else {
            temp &= ~DINO8_STORM_SUPPRESSION_MULTICAST_MASK;
        }
    } 
    if (type & DRV_STORM_CONTROL_DLF) {
        if (enable) {
            temp |= DINO8_STORM_SUPPRESSION_DLF_MASK;
        } else {
            temp &= ~DINO8_STORM_SUPPRESSION_DLF_MASK;
        }
    }

    soc_COMM_IRC_CONr_field_set(unit, &reg_value, 
        PKT_MSK1f, &temp);
    SOC_IF_ERROR_RETURN(REG_WRITE_COMM_IRC_CONr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : _drv_dino8_storm_control_type_enable_get
 *
 *  Purpose :
 *      Get the status and types of storm control.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      type       :  (OUT) storm control types.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
static int 
_drv_dino8_storm_control_type_enable_get(int unit, uint32 *type)
{
    uint32  temp = 0;
    uint32  reg_value;

    /* Get the storm suppressed packet types of bucket 1 (Global) */
    SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
        (unit, &reg_value));
    soc_COMM_IRC_CONr_field_get(unit, &reg_value, 
        PKT_MSK1f, &temp);

    if (temp & DINO8_STORM_SUPPRESSION_BROADCAST_MASK) {
        *type |= DRV_STORM_CONTROL_BCAST;
    }

    if (temp & DINO8_STORM_SUPPRESSION_MULTICAST_MASK) {
        *type |= DRV_STORM_CONTROL_MCAST;
    }

    if (temp & DINO8_STORM_SUPPRESSION_DLF_MASK) {
        *type |= DRV_STORM_CONTROL_DLF;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino8_storm_control_enable_set
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
drv_dino8_storm_control_enable_set(int unit, uint32 port, uint8 enable)
{
    uint32  temp;
    uint32  reg_value;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_dino8_storm_control_enable_set: \
                         unit = %d, port = %d, %sable\n"), unit, port, (enable) ? "en" : "dis"));

    /* Enable/disable drop_enable bit of bucket 1 */
    SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
        (unit, &reg_value));

    if (enable) {
        temp = 1;
    } else {
        temp = 0;
    }
    soc_COMM_IRC_CONr_field_set(unit, &reg_value, 
        DROP_EN1f, &temp);
    SOC_IF_ERROR_RETURN(REG_WRITE_COMM_IRC_CONr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino8_storm_control_enable_get
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
drv_dino8_storm_control_enable_get(int unit, uint32 port, uint8 *enable)
{
    uint32  temp = 0;
    uint32  reg_value;

    SOC_IF_ERROR_RETURN(REG_READ_PORT_IRC_CONr
        (unit, port, &reg_value));

    soc_PORT_IRC_CONr_field_get(unit, &reg_value, 
        ING_RC_EN1f, &temp);

    *enable = temp;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_dino8_storm_control_enable_get: \
                         unit = %d, port = %d, %sable\n"), unit, port, (*enable) ? "en" : "dis"));

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino8_storm_control_set
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
drv_dino8_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size)
{
    uint32  reg_value, temp;
    uint32  port;
    uint32  disable_type = 0, burst_kbyte;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_dino8_storm_control_set: \
                         unit = %d, bmp = 0x%x, type = 0x%x, limit = %dK\n"), 
              unit, SOC_PBMP_WORD_GET(bmp, 0), type, limit));

    /* 
    * Check maximum supported rate limit and burst size of GE ports.
    */
    if (limit > 1000000) {
        return SOC_E_PARAM;
    }

    if (burst_size > (500 * 8)) {  /* 500 KB */
        return SOC_E_PARAM;
    }

    PBMP_ITER(bmp, port) {
        if (limit == 0) {
            /* Disable per-port Ingress rate control bit of bucket 1 */
            SOC_IF_ERROR_RETURN(REG_READ_PORT_IRC_CONr
                (unit, port, &reg_value));
        
            temp = 0;
            soc_PORT_IRC_CONr_field_set(unit, &reg_value, 
                ING_RC_EN1f, &temp);
        
            SOC_IF_ERROR_RETURN(REG_WRITE_PORT_IRC_CONr
                (unit, port, &reg_value));
        } else {
            disable_type = DINO8_STORM_CONTROL_PKT_MASK;

            /* Set storm suppression type */
            SOC_IF_ERROR_RETURN(_drv_dino8_storm_control_type_enable_set
                (unit, type, TRUE));

            disable_type ^= type;
            if (disable_type != 0) {
                SOC_IF_ERROR_RETURN(_drv_dino8_storm_control_type_enable_set
                    (unit, disable_type, FALSE));
            }

            /* Set Burst size and Refresh count for bucket 1 */
            SOC_IF_ERROR_RETURN(REG_READ_PORT_IRC_CONr
                (unit, port, &reg_value));

            /* Burst size */
            if (burst_size) {
                burst_kbyte = (burst_size / 8);

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
                soc_PORT_IRC_CONr_field_set(unit, &reg_value, 
                    BUCKET_SIZE1f, &temp);
            }
            
            /* Refresh count  (fixed type) */
            if (limit <= 1792) {  /* 64KB ~ 1.792MB */
                temp = ((limit - 1) / 64) + 1;
            } else if (limit <= 100000) {  /* 2MB ~ 100MB */
                temp = (limit / 1000 ) + 27;
            } else if (limit <= 1000000) {  /* 104MB ~ 1000MB */
                temp = (limit / 8000) + 115;
            }
            soc_PORT_IRC_CONr_field_set(unit, &reg_value, 
                REF_CNT1f, &temp);

            /* Enable per-port Ingress rate control bit of bucket 1 */
            temp = 1;
            soc_PORT_IRC_CONr_field_set(unit, &reg_value, 
                ING_RC_EN1f, &temp);
            SOC_IF_ERROR_RETURN(REG_WRITE_PORT_IRC_CONr
                (unit, port, &reg_value));
        }
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino8_storm_control_get
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
drv_dino8_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size)
{
    uint32  reg_value, temp = 0;

    SOC_IF_ERROR_RETURN(_drv_dino8_storm_control_type_enable_get
        (unit, type));

    /* Get per-port Ingress rate control bit for bucket 1 */
    SOC_IF_ERROR_RETURN(REG_READ_PORT_IRC_CONr
        (unit, port, &reg_value));

    soc_PORT_IRC_CONr_field_get(unit, &reg_value, 
        ING_RC_EN1f, &temp);

    if (temp == 0) {
        *limit = 0;
    } else {
        /* Burst size */
        soc_PORT_IRC_CONr_field_get(unit, &reg_value, 
            BUCKET_SIZE1f, &temp);

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

        /* Refresh count  (fixed type) */
        soc_PORT_IRC_CONr_field_get(unit, &reg_value, 
            REF_CNT1f, &temp);

        if (temp <= 28) {
            *limit = temp * 64;
        } else if (temp <= 127) {
            *limit = (temp - 27) * 1000;
        } else if (temp <= 240) {
            *limit = (temp - 115) * 1000 * 8;
        } else {
            return SOC_E_INTERNAL;
        }
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_dino8_storm_control_get: \
                         unit = %d, port = %d, type = 0x%x, limit = %dK\n"), 
              unit, port, *type, *limit));

    return SOC_E_NONE;
}
