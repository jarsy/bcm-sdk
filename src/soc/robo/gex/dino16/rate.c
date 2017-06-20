/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <gex/robo_gex.h>

static int dino16_ingress_rate_init_flag[SOC_MAX_NUM_DEVICES] = {0};

/*
 *  Function : _drv_dino16_port_irc_set
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
_drv_dino16_port_irc_set(int unit, uint32 port, uint32 limit, 
    uint32 burst_size)
{
    uint32      reg_value, temp = 0;
    uint32      burst_kbyte = 0;
    soc_pbmp_t  pbmp;

    SOC_IF_ERROR_RETURN(REG_READ_PORT_IRC_CONr
        (unit, port, &reg_value));

    if (limit == 0) {
        /* Disable ingress rate control 
          *    - set the REF_CNT to the MAX value means packets could 
          *       be forwarded by no limit rate. (set to 0 will block all this 
          *       port's traffic)
          */
        temp = 254;
        soc_PORT_IRC_CONr_field_set(unit, &reg_value, 
            REF_CNT0f, &temp);

    } else {  /* Enable ingress rate control */
        /* Check if global ingress rate config is set */
        if (!dino16_ingress_rate_init_flag[unit]) {
            pbmp = PBMP_ALL(unit);

            SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET
                (unit, pbmp, DRV_RATE_CONFIG_PKT_MASK, 
                GEX_IRC_PKT_MASK_IS_3F));

            dino16_ingress_rate_init_flag[unit] = 1;
        }

        /* Set Burst size and Refresh count for bucket 0 */
        /* Burst size */
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
        } else if (burst_kbyte <= 40) {  /* 40KB */
            temp = 3;
        } else if (burst_kbyte <= 76) {  /* 76KB */
            temp = 4;
        } else if (burst_kbyte <= 140) {  /* 140KB */
            temp = 5;
        } else if (burst_kbyte <= 268) {  /* 268KB */
            temp = 6;
        } else if (burst_kbyte <= 500) {  /* 500KB */
            temp = 7;
        } 

        soc_PORT_IRC_CONr_field_set(unit, &reg_value, 
            BUCKET_SIZE0f, &temp);

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

        soc_PORT_IRC_CONr_field_set(unit, &reg_value, 
            REF_CNT0f, &temp);

        /* Enable ingress rate control for bucket 0 */
        temp = 1;
        soc_PORT_IRC_CONr_field_set(unit, &reg_value, 
            ING_RC_ENf, &temp);
    }

    /* Write register */
    SOC_IF_ERROR_RETURN(REG_WRITE_PORT_IRC_CONr
        (unit, port, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : _drv_dino16_port_irc_get
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
_drv_dino16_port_irc_get(uint32 unit, uint32 port, uint32 *limit, 
    uint32 *burst_size)
{
    uint32  reg_value, temp;

    SOC_IF_ERROR_RETURN(REG_READ_PORT_IRC_CONr
        (unit, port, &reg_value));

    /* Check global ingress rate control setting */
    if (dino16_ingress_rate_init_flag[unit] != 0) {
        temp = 0;
        SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
            (unit, SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0), 
            DRV_RATE_CONFIG_PKT_MASK, &temp));

        /* If dino16_ingress_rate_init_flag[unit] = 0, only next ingress rate setting 
         * may set the properly PKT_MASK0 again currenly.
         */ 
        dino16_ingress_rate_init_flag[unit] = (temp == 0) ? 0 : 1;
    }

    temp = 0;
    soc_PORT_IRC_CONr_field_get(unit, &reg_value, 
        REF_CNT0f, &temp);

    if (temp == 254) {
        *limit = 0;
        *burst_size = 0;
    } else {
        /* Get Burst size and Refresh count for bucket 0 */
        /* Burst size */
        soc_PORT_IRC_CONr_field_get(unit, &reg_value, 
            BUCKET_SIZE0f, &temp);

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
            REF_CNT0f, &temp);

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

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_rate_config_set
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
drv_dino16_rate_config_set(int unit, soc_pbmp_t pbmp, uint32 config_type, 
    uint32 value)
{
    uint32  reg_value, temp;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_dino16_rate_config_set: \
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
                soc_COMM_IRC_CONr_field_set(unit, &reg_value, 
                    RATE_TYPE0f, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_COMM_IRC_CONr
                    (unit, &reg_value));
            }
            break;
        case DRV_RATE_CONFIG_DROP_ENABLE:
            /* Per chip */
            if (SOC_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                    (unit, &reg_value));

                temp = value;
                soc_COMM_IRC_CONr_field_set(unit, &reg_value, 
                    DROP_EN0f, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_COMM_IRC_CONr
                    (unit, &reg_value));
            }
            break;
        case DRV_RATE_CONFIG_PKT_MASK: 
            /* Per chip */
            if (SOC_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                    (unit, &reg_value));

                temp = value & GEX_IRC_PKT_MASK_IS_3F;
                soc_COMM_IRC_CONr_field_set(unit, &reg_value, 
                    PKT_MSK0f, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_COMM_IRC_CONr
                    (unit, &reg_value));
            }
            break;
		case DRV_RATE_CONFIG_INGRESS_IPG_INCLUDE:
		case DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_OFF:
		case DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_ON:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_rate_config_get
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
drv_dino16_rate_config_get(int unit, uint32 port, uint32 config_type, 
     uint32 *value)
{
    uint32  reg_value, temp = 0;

    /* Get bucket 0*/
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE: 
            /* Per chip */
            SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                (unit, &reg_value));
            soc_COMM_IRC_CONr_field_get(unit, &reg_value, 
                RATE_TYPE0f, &temp);
            *value = temp;
            break;
        case DRV_RATE_CONFIG_DROP_ENABLE:
            /* Per chip */
            SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                (unit, &reg_value));
            soc_COMM_IRC_CONr_field_get(unit, &reg_value, 
                DROP_EN0f, &temp);
            *value = temp;
            break;
        case DRV_RATE_CONFIG_PKT_MASK:
            /* Per chip */
            SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                (unit, &reg_value));
            soc_COMM_IRC_CONr_field_get(unit, &reg_value, 
                PKT_MSK0f, &temp);
            *value = temp;
            break;
		case DRV_RATE_CONFIG_INGRESS_IPG_INCLUDE:
		case DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_OFF:
		case DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_ON:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_dino16_rate_config_get: \
                         unit = %d, port = %d, type = 0x%x, value = 0x%x\n"),
              unit, port, config_type, *value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_rate_set
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
drv_dino16_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, int direction, 
    uint32 flags, uint32 kbits_sec_min, 
    uint32 kbits_sec_max, uint32 burst_size)
{
    uint32  port;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_dino16_rate_set: unit = %d, bmp = 0x%x, %s, flasg = 0x%x, \
                         kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n"), 
              unit, SOC_PBMP_WORD_GET(bmp, 0), (direction - 1) ? "EGRESS" : "INGRESS", 
              flags, kbits_sec_min, kbits_sec_max, burst_size));

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_dino16_port_irc_set
                    (unit, port, kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_rate_get
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
 *
 *  Note :
 */
int 
drv_dino16_rate_get(int unit, uint32 port, uint8 queue_n, int direction, 
    uint32 *flags, uint32 *kbits_sec_min, 
    uint32 *kbits_sec_max, uint32 *burst_size)
{
    uint32  min_rate = 0;  /* Dummy variable */

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            SOC_IF_ERROR_RETURN(_drv_dino16_port_irc_get
                (unit, port, kbits_sec_max, burst_size));
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_dino16_rate_get: unit = %d, port = %d, %s, flags = 0x%x, \
                         kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n"),
              unit, port, (direction - 1) ? "EGRESS" : "INGRESS", 
              *flags, min_rate, *kbits_sec_max, *burst_size));

    return SOC_E_NONE;
}

