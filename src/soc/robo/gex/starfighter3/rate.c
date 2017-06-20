/*
 * $Id: rate.c $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <gex/robo_gex.h>
#include "robo_sf3.h"

/*
 *  Function : _drv_sf3_port_irc_set
 *
 *  Purpose :
 *      Set the burst size and rate limit value of the selected ingress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      limit       :  rate limit value (Kbits).
 *      burst_size  :  max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0 (storm control use bucket1).
 *
 */
static int
_drv_sf3_port_irc_set(int unit, uint32 port,
    uint32 limit, uint32 burst_size)
{
    uint32  reg_value, temp = 0;
    uint32  burst_kbyte = 0;
    soc_pbmp_t  pbmp;

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_IMPr
            (unit, CMIC_PORT(unit), &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    if (limit == 0) {  /* Disable ingress rate control */
        /* Disable ingress rate control bucket 0 */
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, EN_BUCKET0f, &temp));
    } else {  /* Enable ingress rate control */
        /* Include all packet types */
        SOC_PBMP_PORT_SET(pbmp, port);
        SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET
            (unit, pbmp, DRV_RATE_CONFIG_PKT_MASK, GEX_IRC_PKT_MASK_IS_3F));

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
            (unit, &reg_value, BUCKET0_REF_CNTf, &temp));

        if (burst_size > (500 * 8)) {  /* 500 KB */
            return SOC_E_PARAM;
        }
        burst_kbyte = (burst_size / 8);
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
        } else if (burst_kbyte <= 500) {  /* 500KB */
            temp = 7;
        }
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, BUCKET0_SIZEf, &temp));

        /* Enable ingress rate control */
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, EN_BUCKET0f, &temp));
    }

    /* Write register */
    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_IMPr
            (unit, CMIC_PORT(unit), &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    return SOC_E_NONE;
}


/*
 *  Function : _drv_sf3_port_shaper_set
 *
 *  Purpose :
 *      Set the burst size and rate limit value of the selected ingress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      pause_on    :  pause on threshold(Kbits).
 *      pause_off   :  pause off threshold(Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0 (storm control use bucket1).
 *
 */
static int
_drv_sf3_port_shaper_set(int unit, uint32 port,
    uint32 pause_on, uint32 pause_off)
{
    uint32  reg_value, temp = 0;
    uint32  burst_kbyte = 0;

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_IMPr
            (unit, CMIC_PORT(unit), &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    if (pause_off == 0) {  /* Disable ingress rate control */
        /* Disable shaper mode */
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, BUCKET_MODE0f, &temp));
    } else {  /* Enable ingress rate control */

        /* Pause on is fixed at 12K bytes of Starfighter3 */
        if ((pause_on != (SF3_INGRESS_RATE_PAUSE_ON_THRESHOLD_KBYTES * 8)) &&
            (pause_on != 0x0)) {
            return SOC_E_PARAM;
        }

        if (pause_off > (40 * 8)) {  /* 40 KB */
            return SOC_E_PARAM;
        }
        burst_kbyte = (pause_off / 8);
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
            (unit, &reg_value, BUCKET0_SIZEf, &temp));
        if (burst_kbyte <= 16) {  /* 16KB */
            temp = 1;
        } else if (burst_kbyte <= 24) {  /* 24KB */
            temp = 2;
        } else if (burst_kbyte <= 40) {  /* 40KB */
            if (temp <= 2) {
                /* Configure to max bucket size */
                temp = 7;
            }
        }

        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, BUCKET0_SIZEf, &temp));

        /* Enable shaper mode */
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, BUCKET_MODE0f, &temp));
    }

    /* Write register */
    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_IMPr
            (unit, CMIC_PORT(unit), &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    return SOC_E_NONE;
}


/*
 *  Function : _drv_sf3_port_irc_get
 *
 *  Purpose :
 *      Get the burst size and rate limit value of the selected ingress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      limit       :  (OUT) rate limit value (Kbits).
 *      burst_size  :  (OUT) max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0 (storm control use bucket1).
 *
 */
static int
_drv_sf3_port_irc_get(uint32 unit, uint32 port,
    uint32 *limit, uint32 *burst_size)
{
    uint32  reg_value, temp;

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_IMPr
            (unit, CMIC_PORT(unit), &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    temp = 0;
    SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
        (unit, &reg_value, EN_BUCKET0f, &temp));

    if (temp == 0) {
        *limit = 0;
        *burst_size = 0;
    } else {
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
            (unit, &reg_value, BUCKET0_SIZEf, &temp));
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
            case 5:
            case 6:
            case 7:
                *burst_size = 500 * 8;  /* 500KB */
                break;
            default:
                return SOC_E_INTERNAL;
        }

        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
            (unit, &reg_value, BUCKET0_REF_CNTf, &temp));
        if (temp <= 28) {
            *limit = temp * 64;
        } else if (temp <= 127) {
            *limit = (temp - 27) * 1000;
        } else if (temp <= 240) {
            *limit = (temp -115) * 1000 * 8;
        } else {
            return SOC_E_INTERNAL;
        }
    }

    return SOC_E_NONE;
}

/*
 *  Function : _drv_sf3_port_shaper_get
 *
 *  Purpose :
 *      Get the burst size and rate limit value of the selected ingress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      pause_on    :  (OUT) rate limit value (Kbits).
 *      pause_off   :  (OUT) max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0 (storm control use bucket1).
 *
 */
static int
_drv_sf3_port_shaper_get(uint32 unit, uint32 port,
    uint32 *pause_on, uint32 *pause_off)
{
    uint32  reg_value, temp;

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_IMPr
            (unit, CMIC_PORT(unit), &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    temp = 0;
    SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
        (unit, &reg_value, BUCKET_MODE0f, &temp));

    if (temp == 1) {
        *pause_on = 0;
        *pause_off = 0;
    } else {
        /* Pause on is fixed at 12K bytes of Starfighter3 */
        *pause_on = SF3_INGRESS_RATE_PAUSE_ON_THRESHOLD_KBYTES * 8;

        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
            (unit, &reg_value, BUCKET0_SIZEf, &temp));
            /* Shaper */
            switch (temp) {
                case 1:
                    *pause_off = 16 * 8;  /* 16KB */
                    break;
                case 2:
                    *pause_off = 24 * 8;  /* 24KB */
                    break;
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                    *pause_off = 40 * 8;  /* 40KB */
                    break;
                default:
                    return SOC_E_INTERNAL;
            }
    }

    return SOC_E_NONE;
}


/*
 *  Function : _drv_sf3_port_erc_set
 *
 *  Purpose :
 *     Set the burst size and rate limit value of the selected egress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      limit       :  rate limit value (Kbits).
 *      burst_size  :  max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int
_drv_sf3_port_erc_set(uint32 unit, uint32 port, uint32 flags,
    uint32 limit, uint32 burst_size)
{
    uint32  reg_value, temp = 0;
    uint32  burst_kbyte = 0;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META("_drv_sf3_port_erc_set: \
                       unit = %d, port = %d, limit = %d, burst_size = %d\n"),
              unit, port, limit, burst_size));

    if (limit == 0) {
        /* Disable shaper */
        SOC_IF_ERROR_RETURN(REG_READ_PORT_SHAPER_ENABLEr
            (unit, &reg_value));
        soc_PORT_SHAPER_ENABLEr_field_get(unit, &reg_value,
            PORT_SHAPER_ENABLEf, &temp);
        temp &= ~(0x1 << port);
        soc_PORT_SHAPER_ENABLEr_field_set(unit, &reg_value,
            PORT_SHAPER_ENABLEf, &temp);
        SOC_IF_ERROR_RETURN(REG_WRITE_PORT_SHAPER_ENABLEr
            (unit, &reg_value));
    } else {
        /* Include IPG bytes */
        SOC_IF_ERROR_RETURN(
            REG_READ_IFG_BYTESr(unit, &reg_value));
        soc_IFG_BYTESr_field_get(unit, &reg_value, IFG_BYTESf, &temp);
        if (flags & DRV_RATE_CONTROL_FLAG_EGRESS_IPG_INCLUDE) {
            temp |= (0x1 << port);
        } else {
            temp &= ~(0x1 << port);
        }
        soc_IFG_BYTESr_field_set(unit, &reg_value, IFG_BYTESf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_IFG_BYTESr(unit, &reg_value));

        /* AVB shaping mode */
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_SHAPER_AVB_SHAPING_MODEr(unit, &reg_value));
        soc_PORT_SHAPER_AVB_SHAPING_MODEr_field_get(unit, &reg_value,
            PORT_SHAPER_AVB_SHAPING_MODEf, &temp);
        if (flags & DRV_RATE_CONTROL_FLAG_EGRESS_AVB_MODE) {
            temp |= (0x1 << port);
        } else {
            temp &= ~(0x1 << port);
        }
        soc_PORT_SHAPER_AVB_SHAPING_MODEr_field_set(unit, &reg_value,
            PORT_SHAPER_AVB_SHAPING_MODEf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_PORT_SHAPER_AVB_SHAPING_MODEr(unit, &reg_value));

        /* Packet-based or bytes-based */
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_SHAPER_BUCKET_COUNT_SELECTr(unit, &reg_value));
        soc_PORT_SHAPER_BUCKET_COUNT_SELECTr_field_get(unit, &reg_value,
            PORT_SHAPER_BUCKET_COUNT_SELECTf, &temp);
        if (flags & DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED) {
            temp |= (0x1 << port);
        } else {
            temp &= ~(0x1 << port);
        }
        soc_PORT_SHAPER_BUCKET_COUNT_SELECTr_field_set(unit, &reg_value,
            PORT_SHAPER_BUCKET_COUNT_SELECTf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_PORT_SHAPER_BUCKET_COUNT_SELECTr(unit, &reg_value));

        if (flags & DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED) {
            /* Packet-based */

            if ((limit > SF3_RATE_PACKET_BASED_MAX_REFRESH_VALUE) ||
                (burst_size > SF3_RATE_PACKET_BASED_MAX_BUCKET_VALUE)) {
                return SOC_E_PARAM;
            }

            /* Refresh unit */
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_PORT_SHAPER_PACKET_BASED_MAX_REFRESHr(
                    unit, port, &reg_value));
            temp = limit + SF3_RATE_PACKET_BASED_REFRESH_UNIT - 1;
            temp /= SF3_RATE_PACKET_BASED_REFRESH_UNIT;
            soc_PN_PORT_SHAPER_PACKET_BASED_MAX_REFRESHr_field_set(
                    unit, &reg_value, MAX_REFRESHf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_PN_PORT_SHAPER_PACKET_BASED_MAX_REFRESHr(
                    unit, port, &reg_value));

            /* Bucket size */
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_PORT_SHAPER_PACKET_BASED_MAX_THD_SELr(
                    unit, port, &reg_value));
            temp = burst_size + SF3_RATE_PACKET_BASED_BUCKET_UNIT - 1;
            temp /= SF3_RATE_PACKET_BASED_BUCKET_UNIT;
            soc_PN_PORT_SHAPER_PACKET_BASED_MAX_THD_SELr_field_set(
                    unit, &reg_value, MAX_THD_SELf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_PN_PORT_SHAPER_PACKET_BASED_MAX_THD_SELr(
                    unit, port, &reg_value));
        } else {
            /* Bytes-based */
            /* Bucket size */
            burst_kbyte = (burst_size / 8);
            if ((limit > SF3_RATE_MAX_REFRESH_RATE) ||
                ((burst_kbyte * 1000) > SF3_RATE_MAX_BUCKET_SIZE)) {
                return SOC_E_PARAM;
            }

            /* Refresh count */
            SOC_IF_ERROR_RETURN(REG_READ_PN_PORT_SHAPER_BYTE_BASED_MAX_REFRESHr(
                unit, port, &reg_value));
            temp = ((limit - 1) / SF3_RATE_REFRESH_GRANULARITY) + 1;
            soc_PN_PORT_SHAPER_BYTE_BASED_MAX_REFRESHr_field_set(unit, &reg_value,
                MAX_REFRESHf, &temp);
            SOC_IF_ERROR_RETURN(REG_WRITE_PN_PORT_SHAPER_BYTE_BASED_MAX_REFRESHr(
                unit, port, &reg_value));

            /* Bucket size */
            SOC_IF_ERROR_RETURN(REG_READ_PN_PORT_SHAPER_BYTE_BASED_MAX_THD_SELr(
                unit, port, &reg_value));
            temp = ((burst_kbyte * 1000) / SF3_RATE_BUCKET_UNIT_SIZE);
            soc_PN_PORT_SHAPER_BYTE_BASED_MAX_THD_SELr_field_set(unit, &reg_value,
                MAX_THD_SELf, &temp);
            SOC_IF_ERROR_RETURN(REG_WRITE_PN_PORT_SHAPER_BYTE_BASED_MAX_THD_SELr(
                unit, port, &reg_value));
        }
        /* Enable shaper */
        SOC_IF_ERROR_RETURN(REG_READ_PORT_SHAPER_ENABLEr
            (unit, &reg_value));
        soc_PORT_SHAPER_ENABLEr_field_get(unit, &reg_value,
            PORT_SHAPER_ENABLEf, &temp);
        temp |= (0x1 << port);
        soc_PORT_SHAPER_ENABLEr_field_set(unit, &reg_value,
            PORT_SHAPER_ENABLEf, &temp);
        SOC_IF_ERROR_RETURN(REG_WRITE_PORT_SHAPER_ENABLEr
            (unit, &reg_value));
    }
    return SOC_E_NONE;
}

/*
 *  Function : _drv_sf3_port_erc_per_queue_set
 *
 *  Purpose :
 *     Set the burst size and rate limit value of the selected egress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      queue_n     :  COSQ id.
 *      limit       :  rate limit value (Kbits per second).
 *      burst_size  :  max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int
_drv_sf3_port_erc_per_queue_set(uint32 unit, uint32 port,
    uint8 queue_n, uint32 flags,
    uint32 limit, uint32 burst_size)
{
    uint32  reg_value, temp = 0;
    uint32  burst_kbyte = 0;
    uint32  enable_addr, reg_addr;
    int     enable_len, reg_len;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META("_drv_sf3_port_erc_per_queue_set: \
                       unit = %d, port = %d, flags = 0x%x, limit = %d, burst_size = %d\n"),
              unit, port, flags, limit, burst_size));

    if (queue_n >= DRV_SF3_COS_QUEUE_NUM) {
        return SOC_E_PARAM;
    }

    enable_addr = DRV_REG_ADDR(unit,
        QUEUE0_SHAPER_ENABLEr_ROBO, port, 0);
    enable_addr += (queue_n << SOC_ROBO_PAGE_BP);
    enable_len = DRV_REG_LENGTH_GET(unit, QUEUE0_SHAPER_ENABLEr_ROBO);

    if (limit == 0) {
        /* Disable shaper */
        SOC_IF_ERROR_RETURN(
            DRV_REG_READ(unit, enable_addr, &reg_value, enable_len));
        soc_QUEUE0_SHAPER_ENABLEr_field_get(unit, &reg_value,
            QUEUE0_SHAPER_ENABLEf, &temp);
        temp &= ~(0x1 << port);
        soc_QUEUE0_SHAPER_ENABLEr_field_set(unit, &reg_value,
            QUEUE0_SHAPER_ENABLEf, &temp);
        SOC_IF_ERROR_RETURN(
            DRV_REG_WRITE(unit, enable_addr, &reg_value, enable_len));
    } else {
        /* AVB shaping mode */
        reg_addr = DRV_REG_ADDR(unit,
            QUEUE0_AVB_SHAPING_MODEr_ROBO, port, 0);
        reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
        reg_len = DRV_REG_LENGTH_GET(unit, QUEUE0_AVB_SHAPING_MODEr_ROBO);

        SOC_IF_ERROR_RETURN(
            DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));

        soc_QUEUE0_AVB_SHAPING_MODEr_field_get(unit, &reg_value,
            QUEUE0_AVB_SHAPING_MODEf, &temp);

        if (flags & DRV_RATE_CONTROL_FLAG_EGRESS_AVB_MODE) {
            temp |= (0x1 << port);
        } else {
            temp &= (0x1 << port);
        }
        soc_QUEUE0_AVB_SHAPING_MODEr_field_set(unit, &reg_value,
            QUEUE0_AVB_SHAPING_MODEf, &temp);
        SOC_IF_ERROR_RETURN(
            DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len));

        /* packet-based selection */
        reg_addr = DRV_REG_ADDR(unit,
            QUEUE0_SHAPER_BUCKET_COUNT_SELECTr_ROBO, port, 0);
        reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
        reg_len = DRV_REG_LENGTH_GET(unit,
            QUEUE0_SHAPER_BUCKET_COUNT_SELECTr_ROBO);
        SOC_IF_ERROR_RETURN(
            DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));

        soc_QUEUE0_SHAPER_BUCKET_COUNT_SELECTr_field_get(unit, &reg_value,
            QUEUE0_SHAPER_BUCKET_COUNT_SELECTf, &temp);

        if (flags & DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED) {
            temp |= (0x1 << port);
        } else {
            temp &= (0x1 << port);
        }
        soc_QUEUE0_SHAPER_BUCKET_COUNT_SELECTr_field_set(unit, &reg_value,
            QUEUE0_SHAPER_BUCKET_COUNT_SELECTf, &temp);
        SOC_IF_ERROR_RETURN(
            DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len));

        if (flags & DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED) {
            /* Packet-based */
            if ((limit > SF3_RATE_PACKET_BASED_MAX_REFRESH_VALUE) ||
                (burst_size > SF3_RATE_PACKET_BASED_MAX_BUCKET_VALUE)) {
                return SOC_E_PARAM;
            }

            /* Refresh count */
            reg_addr = DRV_REG_ADDR(unit,
                PN_QUEUE0_MAX_PACKET_REFRESHr_ROBO, port, 0);
            reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
            reg_len = DRV_REG_LENGTH_GET(unit,
                PN_QUEUE0_MAX_PACKET_REFRESHr_ROBO);

            SOC_IF_ERROR_RETURN(
                DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));
            temp = burst_size + SF3_RATE_PACKET_BASED_REFRESH_UNIT - 1;
            temp /= SF3_RATE_PACKET_BASED_REFRESH_UNIT;
            soc_PN_QUEUE0_MAX_PACKET_REFRESHr_field_set(unit, &reg_value,
                MAX_REFRESHf, &temp);
            SOC_IF_ERROR_RETURN(
                DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len));

            /* Bucket size */
            reg_addr = DRV_REG_ADDR(unit,
                PN_QUEUE0_MAX_PACKET_THD_SELr_ROBO, port, 0);
            reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
            reg_len = DRV_REG_LENGTH_GET(unit,
                PN_QUEUE0_MAX_PACKET_THD_SELr_ROBO);

            SOC_IF_ERROR_RETURN(
                DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));
            temp = limit + SF3_RATE_PACKET_BASED_BUCKET_UNIT - 1;
            temp /= SF3_RATE_PACKET_BASED_BUCKET_UNIT;
            soc_PN_QUEUE0_MAX_PACKET_THD_SELr_field_set(unit, &reg_value,
                MAX_THD_SELf, &temp);
            SOC_IF_ERROR_RETURN(
                DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len));
        } else {
            /* Bytes-based */

            burst_kbyte = (burst_size / 8);
            if ((limit > SF3_RATE_MAX_REFRESH_RATE) ||
                ((burst_kbyte * 1000) > SF3_RATE_MAX_BUCKET_SIZE)) {
                return SOC_E_PARAM;
            }

            /* Refresh count */
            reg_addr = DRV_REG_ADDR(unit,
                PN_QUEUE0_MAX_REFRESHr_ROBO, port, 0);
            reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
            reg_len = DRV_REG_LENGTH_GET(unit, PN_QUEUE0_MAX_REFRESHr_ROBO);

            SOC_IF_ERROR_RETURN(
                DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));
            temp = ((limit - 1) / SF3_RATE_REFRESH_GRANULARITY) + 1;
            soc_PN_QUEUE0_MAX_REFRESHr_field_set(unit, &reg_value,
                MAX_REFRESHf, &temp);
            SOC_IF_ERROR_RETURN(
                DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len));

            /* Bucket size */
            reg_addr = DRV_REG_ADDR(unit,
                PN_QUEUE0_MAX_THD_SELr_ROBO, port, 0);
            reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
            reg_len = DRV_REG_LENGTH_GET(unit, PN_QUEUE0_MAX_THD_SELr_ROBO);

            SOC_IF_ERROR_RETURN(
                DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));
            temp = (((burst_kbyte * 1000) + (SF3_RATE_BUCKET_UNIT_SIZE - 1))
                           / SF3_RATE_BUCKET_UNIT_SIZE);

            soc_PN_QUEUE0_MAX_THD_SELr_field_set(unit, &reg_value,
                MAX_THD_SELf, &temp);
            SOC_IF_ERROR_RETURN(
                DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len));
        }

        /* Enable shaper */
        SOC_IF_ERROR_RETURN(
            DRV_REG_READ(unit, enable_addr, &reg_value, enable_len));
        soc_QUEUE0_SHAPER_ENABLEr_field_get(unit, &reg_value,
            QUEUE0_SHAPER_ENABLEf, &temp);
        temp |= (0x1 << port);
        soc_QUEUE0_SHAPER_ENABLEr_field_set(unit, &reg_value,
            QUEUE0_SHAPER_ENABLEf, &temp);
        SOC_IF_ERROR_RETURN(
            DRV_REG_WRITE(unit, enable_addr, &reg_value, enable_len));
    }
    return SOC_E_NONE;
}

/*
 *  Function : _drv_sf3_port_erc_get
 *
 *  Purpose :
 *     Get the burst size and rate limit value of the selected egress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      limit       :  (OUT) rate limit value (Kbits).
 *      burst_size  :  (OUT) max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
static int
_drv_sf3_port_erc_get(uint32 unit, uint32 port, uint32 *flags,
    uint32 *limit, uint32 *burst_size)
{
    uint32  reg_value, temp = 0;

    *flags = 0;
    SOC_IF_ERROR_RETURN(REG_READ_PORT_SHAPER_ENABLEr
        (unit, &reg_value));
    soc_PORT_SHAPER_ENABLEr_field_get(unit, &reg_value,
        PORT_SHAPER_ENABLEf, &temp);
    /* Shaper disabled */
    if (!(temp & (0x1 << port))) {
        *limit = 0;
        *burst_size = 0;
    } else {

        /* Include IPG bytes */
        SOC_IF_ERROR_RETURN(
            REG_READ_IFG_BYTESr(unit, &reg_value));
        soc_IFG_BYTESr_field_get(unit, &reg_value, IFG_BYTESf, &temp);
        if (temp & (0x1 << port)) {
            *flags |= DRV_RATE_CONTROL_FLAG_EGRESS_IPG_INCLUDE;
        }

        /* AVB shaping mode */
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_SHAPER_AVB_SHAPING_MODEr(unit, &reg_value));
        soc_PORT_SHAPER_AVB_SHAPING_MODEr_field_get(unit, &reg_value,
            PORT_SHAPER_AVB_SHAPING_MODEf, &temp);
        if (temp & (0x1 << port)) {
            *flags |= DRV_RATE_CONTROL_FLAG_EGRESS_AVB_MODE;
        }

        /* Packet-based or bytes-based */
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_SHAPER_BUCKET_COUNT_SELECTr(unit, &reg_value));
        soc_PORT_SHAPER_BUCKET_COUNT_SELECTr_field_get(unit, &reg_value,
            PORT_SHAPER_BUCKET_COUNT_SELECTf, &temp);
        if (temp & (0x1 << port)) {
            *flags |= DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED;
        }

        if (*flags & DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED) {
            /* Refresh unit */
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_PORT_SHAPER_PACKET_BASED_MAX_REFRESHr(
                    unit, port, &reg_value));
            soc_PN_PORT_SHAPER_PACKET_BASED_MAX_REFRESHr_field_get(
                    unit, &reg_value, MAX_REFRESHf, &temp);
            *limit = temp * SF3_RATE_PACKET_BASED_REFRESH_UNIT;

            /* Bucket size */
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_PORT_SHAPER_PACKET_BASED_MAX_THD_SELr(
                    unit, port, &reg_value));
            soc_PN_PORT_SHAPER_PACKET_BASED_MAX_THD_SELr_field_get(
                    unit, &reg_value, MAX_THD_SELf, &temp);
            *burst_size = temp * SF3_RATE_PACKET_BASED_BUCKET_UNIT;
        } else {
            /* Refresh count */
            SOC_IF_ERROR_RETURN(REG_READ_PN_PORT_SHAPER_BYTE_BASED_MAX_REFRESHr(
                unit, port, &reg_value));
            soc_PN_PORT_SHAPER_BYTE_BASED_MAX_REFRESHr_field_get(
                unit, &reg_value, MAX_REFRESHf, &temp);
            *limit = temp * 64;
            /* Burst size */
            SOC_IF_ERROR_RETURN(REG_READ_PN_PORT_SHAPER_BYTE_BASED_MAX_THD_SELr(
                unit, port, &reg_value));
            soc_PN_PORT_SHAPER_BYTE_BASED_MAX_THD_SELr_field_get(unit,
                &reg_value, MAX_THD_SELf, &temp);
            *burst_size = ((temp * SF3_RATE_BUCKET_UNIT_SIZE * 8) / 1000);
        }
    }
    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META("_drv_sf3_port_erc_get: \
                       unit = %d, port = %d, limit = %dK, burst size = %dKB\n"),
              unit, port, *limit, *burst_size));

    return SOC_E_NONE;
}

/*
 *  Function : _drv_sf3_port_erc_per_queue_get
 *
 *  Purpose :
 *     Get the burst size and rate limit value of the selected egress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      queue_n     :  COSQ id.
 *      limit       :  (OUT) rate limit value (Kbits per second).
 *      burst_size  :  (OUT) max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int
_drv_sf3_port_erc_per_queue_get(uint32 unit, uint32 port,
    uint8 queue_n, uint32 *flags, uint32 *limit, uint32 *burst_size)
{
    uint32  reg_value, temp = 0;
    uint32  enable_addr, reg_addr;
    int     enable_len, reg_len;

    if (queue_n >= DRV_SF3_COS_QUEUE_NUM) {
        return SOC_E_PARAM;
    }

    *flags = 0;
    enable_addr = DRV_REG_ADDR(unit,
        QUEUE0_SHAPER_ENABLEr_ROBO, port, 0);
    enable_addr += (queue_n << SOC_ROBO_PAGE_BP);
    enable_len = DRV_REG_LENGTH_GET(unit, QUEUE0_SHAPER_ENABLEr_ROBO);

    /* Check enable bit */
    SOC_IF_ERROR_RETURN(
        DRV_REG_READ(unit, enable_addr, &reg_value, enable_len));
    soc_QUEUE0_SHAPER_ENABLEr_field_get(unit, &reg_value,
        QUEUE0_SHAPER_ENABLEf, &temp);
    if (!(temp & (0x1 << port))) {
        *limit = 0;
        *burst_size = 0;
    } else {

        /* AVB shaping mode */
        reg_addr = DRV_REG_ADDR(unit,
            QUEUE0_AVB_SHAPING_MODEr_ROBO, port, 0);
        reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
        reg_len = DRV_REG_LENGTH_GET(unit, QUEUE0_AVB_SHAPING_MODEr_ROBO);

        SOC_IF_ERROR_RETURN(
            DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));

        soc_QUEUE0_AVB_SHAPING_MODEr_field_get(unit, &reg_value,
            QUEUE0_AVB_SHAPING_MODEf, &temp);

        if (temp & (0x1 << port)) {
            *flags |= DRV_RATE_CONTROL_FLAG_EGRESS_AVB_MODE;
        }

        /* packet-based selection */
        reg_addr = DRV_REG_ADDR(unit,
            QUEUE0_SHAPER_BUCKET_COUNT_SELECTr_ROBO, port, 0);
        reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
        reg_len = DRV_REG_LENGTH_GET(unit,
            QUEUE0_SHAPER_BUCKET_COUNT_SELECTr_ROBO);
        SOC_IF_ERROR_RETURN(
            DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));

        soc_QUEUE0_SHAPER_BUCKET_COUNT_SELECTr_field_get(unit, &reg_value,
            QUEUE0_SHAPER_BUCKET_COUNT_SELECTf, &temp);

        if (temp & (0x1 << port)) {
            *flags |= DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED;
        }

        if (*flags & DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED) {
            /* Packet-based */
            /* Refresh count */
            reg_addr = DRV_REG_ADDR(unit,
                PN_QUEUE0_MAX_PACKET_REFRESHr_ROBO, port, 0);
            reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
            reg_len = DRV_REG_LENGTH_GET(unit,
                PN_QUEUE0_MAX_PACKET_REFRESHr_ROBO);

            SOC_IF_ERROR_RETURN(
                DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));
            soc_PN_QUEUE0_MAX_PACKET_REFRESHr_field_get(unit, &reg_value,
                MAX_REFRESHf, &temp);
            *limit = temp * SF3_RATE_PACKET_BASED_REFRESH_UNIT;

            /* Bucket size */
            reg_addr = DRV_REG_ADDR(unit,
                PN_QUEUE0_MAX_PACKET_THD_SELr_ROBO, port, 0);
            reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
            reg_len = DRV_REG_LENGTH_GET(unit,
                PN_QUEUE0_MAX_PACKET_THD_SELr_ROBO);

            SOC_IF_ERROR_RETURN(
                DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));
            soc_PN_QUEUE0_MAX_PACKET_THD_SELr_field_get(unit, &reg_value,
                MAX_THD_SELf, &temp);
            *burst_size = temp * SF3_RATE_PACKET_BASED_BUCKET_UNIT;
        } else {
            /* Bytes-based */
            /* Refresh count */
            reg_addr = DRV_REG_ADDR(unit,
                PN_QUEUE0_MAX_REFRESHr_ROBO, port, 0);
            reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
            reg_len = DRV_REG_LENGTH_GET(unit, PN_QUEUE0_MAX_REFRESHr_ROBO);

            SOC_IF_ERROR_RETURN(
                DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));
            soc_PN_QUEUE0_MAX_REFRESHr_field_get(unit, &reg_value,
                MAX_REFRESHf, &temp);
            *limit = temp * SF3_RATE_REFRESH_GRANULARITY;

            /* Bucket size */
            reg_addr = DRV_REG_ADDR(unit,
                PN_QUEUE0_MAX_THD_SELr_ROBO, port, 0);
            reg_addr += (queue_n << SOC_ROBO_PAGE_BP);
            reg_len = DRV_REG_LENGTH_GET(unit, PN_QUEUE0_MAX_THD_SELr_ROBO);

            SOC_IF_ERROR_RETURN(
                DRV_REG_READ(unit, reg_addr, &reg_value, reg_len));
            soc_PN_QUEUE0_MAX_THD_SELr_field_get(unit, &reg_value,
                MAX_THD_SELf, &temp);
            *burst_size = ((temp * SF3_RATE_BUCKET_UNIT_SIZE * 8) / 1000);
        }
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META("_drv_sf3_port_erc_per_queue_get: \
                       unit = %d, port = %d, limit = %dK, burst size = %dKB\n"),
              unit, port, *limit, *burst_size));

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_rate_config_set
 *
 *  Purpose :
 *      Set the rate control type value to the selected ports.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      pbmp        :  port bitmap.
 *      config_type :  rate control type.
 *      value       :  value of rate control type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int
drv_sf3_rate_config_set(int unit, soc_pbmp_t pbmp, uint32 config_type,
    uint32 value)
{
    uint32  reg_value, temp;
    uint32  pause_on, pause_off;
    int port;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_sf3_rate_config_set: \
                         unit = %d, bmp = 0x%x, type = 0x%x, value = 0x%x\n"),
              unit, SOC_PBMP_WORD_GET(pbmp, 0), config_type, value));

    /* Set bucket 0 */
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE:
            /* Per chip */
            if (SOC_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                    (unit, &reg_value));

                temp = value;
                soc_COMM_IRC_CONr_field_set(
                    unit, &reg_value, RATE_TYPE0f, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_COMM_IRC_CONr
                    (unit, &reg_value));
            }
            break;
        case DRV_RATE_CONFIG_DROP_ENABLE:
            /* Per port */
            PBMP_ITER(pbmp, port) {
                SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
                    (unit, port, &reg_value));
                temp = value;
                soc_BC_SUP_RATECTRL_Pr_field_set(unit, &reg_value,
                    BUCKET_MODE0f, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_Pr
                    (unit, port, &reg_value));
            }
            break;
        case DRV_RATE_CONFIG_PKT_MASK:
            /* Per port */
            PBMP_ITER(pbmp, port) {
                SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_1_Pr
                    (unit, port, &reg_value));
                temp = value & GEX_IRC_PKT_MASK_IS_3F;
                soc_BC_SUP_RATECTRL_1_Pr_field_set(unit, &reg_value,
                    PKT_MSK0f, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_1_Pr
                    (unit, port, &reg_value));
            }
            break;
        case DRV_RATE_CONFIG_INGRESS_IPG_INCLUDE:
            /* Per port */
            PBMP_ITER(pbmp, port) {
                SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_1_Pr
                    (unit, port, &reg_value));
                temp =  (value)? 1:0;
                soc_BC_SUP_RATECTRL_1_Pr_field_set(unit, &reg_value,
                    IFG_BYTES0f, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_1_Pr
                    (unit, port, &reg_value));
            }
            break;
        case DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_OFF:
            /* Per port */
            PBMP_ITER(pbmp, port) {
                SOC_IF_ERROR_RETURN(
                    _drv_sf3_port_shaper_get(unit, port,
                                &pause_on, &pause_off));
                pause_off = value;
                SOC_IF_ERROR_RETURN(
                    _drv_sf3_port_shaper_set(unit, port,
                                pause_on, pause_off));
            }
            break;
        case DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_ON:
            if ((value != SF3_INGRESS_RATE_PAUSE_ON_THRESHOLD_KBYTES * 8) &&
                (value != 0x0)) {
                return SOC_E_UNAVAIL;
            }
            break;
        default:
            return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_rate_config_get
 *
 *  Purpose :
 *      Get the rate control type value to the selected ports.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      pbmp        :  port bitmap.
 *      config_type :  rate control type.
 *      value       :  (OUT) value of rate control type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int
drv_sf3_rate_config_get(int unit, uint32 port, uint32 config_type,
     uint32 *value)
{
    uint32  reg_value, temp = 0, pause_on = 0;

    /* Get bucket 0*/
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE:
            /* Per chip */
            SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                (unit, &reg_value));
            soc_COMM_IRC_CONr_field_get
                (unit, &reg_value, RATE_TYPE0f, &temp);
            *value = temp;
            break;
        case DRV_RATE_CONFIG_DROP_ENABLE:
            /* Per port */
            SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
                (unit, port, &reg_value));
            soc_BC_SUP_RATECTRL_Pr_field_get(unit, &reg_value,
                    BUCKET_MODE0f, &temp);
            *value = temp;
            break;
        case DRV_RATE_CONFIG_PKT_MASK:
            /* Per port */
            SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_1_Pr
                (unit, port, &reg_value));
            soc_BC_SUP_RATECTRL_1_Pr_field_get(unit, &reg_value,
                PKT_MSK0f, &temp);
            *value = temp;
            break;
        case DRV_RATE_CONFIG_INGRESS_IPG_INCLUDE:
            /* Per port */
            SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_1_Pr
                (unit, port, &reg_value));
            soc_BC_SUP_RATECTRL_1_Pr_field_get(unit, &reg_value,
                IFG_BYTES0f, &temp);
            *value = temp;
            break;
        case DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_OFF:
            SOC_IF_ERROR_RETURN(
                _drv_sf3_port_shaper_get(unit, port,
                            &pause_on, &temp));
            *value = temp;
            break;
        case DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_ON:
            SOC_IF_ERROR_RETURN(
                _drv_sf3_port_shaper_get(unit, port,
                            &pause_on, &temp));
            *value = pause_on;
            break;
        default:
            return SOC_E_PARAM;
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_sf3_rate_config_get: \
                         unit = %d, port = %d, type = 0x%x, value = 0x%x\n"),
              unit, port, config_type, *value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_rate_set
 *
 *  Purpose :
 *      Set the ingress/egress rate control to the selected ports.
 *
 *  Parameters :
 *      unit          :  unit id.
 *      bmp           :  port bitmap.
 *      queue_n       :  COSQ id.
 *      direction     :  direction of rate control (ingress/egress).
 *      kbits_sec_min :  minimum bandwidth, kbits/sec.
 *      kbits_sec_max :  maximum bandwidth, kbits/sec.
 *      burst_size    :  max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int
drv_sf3_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, int direction,
    uint32 flags, uint32 kbits_sec_min,
    uint32 kbits_sec_max, uint32 burst_size)
{
    uint32  port;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_sf3_rate_set: unit = %d, bmp = 0x%x, %s, flags = 0x%x, \
                         kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), (direction - 1) ? "EGRESS" : "INGRESS",
              flags, kbits_sec_min, kbits_sec_max, burst_size));

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_sf3_port_irc_set
                    (unit, port, kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_sf3_port_erc_set
                    (unit, port, flags, kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_sf3_port_erc_per_queue_set
                    (unit, port, queue_n, flags, kbits_sec_min, burst_size));
            }
            break;
        default:
            return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_sf3_rate_get
 *
 *  Purpose :
 *      Get the ingress/egress rate control to the selected ports.
 *
 *  Parameters :
 *      unit          :  unit id.
 *      port          :  port id.
 *      queue_n       :  COSQ id.
 *      direction     :  direction of rate control (ingress/egress).
 *      kbits_sec_min :  (OUT) minimum bandwidth, kbits/sec.
 *      kbits_sec_max :  (OUT) maximum bandwidth, kbits/sec.
 *      burst_size    :  (OUT) max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 */
int
drv_sf3_rate_get(int unit, uint32 port, uint8 queue_n, int direction,
    uint32 *flags, uint32 *kbits_sec_min,
    uint32 *kbits_sec_max, uint32 *burst_size)
{
    uint32  min_rate = 0;  /* Dummy variable */

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            SOC_IF_ERROR_RETURN(_drv_sf3_port_irc_get
                (unit, port, kbits_sec_max, burst_size));
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            SOC_IF_ERROR_RETURN(_drv_sf3_port_erc_get
                (unit, port, flags, kbits_sec_max, burst_size));
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            SOC_IF_ERROR_RETURN(_drv_sf3_port_erc_per_queue_get
                (unit, port, queue_n, flags, kbits_sec_min, burst_size));
            *kbits_sec_max = *burst_size;
            break;
        default:
            return SOC_E_PARAM;
    }

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_sf3_rate_get: unit = %d, port = %d, %s, flags = 0x%x, \
                         kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n"),
              unit, port, (direction - 1) ? "EGRESS" : "INGRESS",
              *flags, min_rate, *kbits_sec_max, *burst_size));

    return SOC_E_NONE;
}
