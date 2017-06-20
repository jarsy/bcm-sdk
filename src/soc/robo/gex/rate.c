/*
 * $Id: rate.c,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <gex/robo_gex.h>

static int gex_rate_index_value[GEX_RATE_INDEX_VALUE_SIZE] = {
    384, 512, 639, 768, 1024, 1280, 1536, 1791,
    2048, 2303, 2559, 2815, 3328, 3840, 4352, 4863,
    5376, 5887, 6400, 6911, 7936, 8960, 9984, 11008,
    12030, 13054, 14076, 15105, 17146, 19201, 21240, 23299,
    25354, 27382, 29446, 31486, 35561, 39682, 42589, 56818,
    71023, 85324, 99602, 113636, 127551, 142045, 213675, 284091, 
    357143, 423729, 500000, 568182, 641026, 714286, 781250, 862069, 
    925926, 1000000, 1086957, 1136364, 1190476, 1250000, 1315789, 1388889
};

static int gex_ingress_rate_init_flag[SOC_MAX_NUM_DEVICES] = {0};

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
/*
 *  Function : _drv_polar_port_irc_set
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
_drv_polar_port_irc_set(int unit, uint32 port, uint32 limit, 
    uint32 burst_size)
{
    uint32  reg_value, temp = 0, specified_port_num;
    uint32  burst_kbyte = 0;
    soc_pbmp_t  pbmp;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_IMPr
            (unit, CMIC_PORT(unit), &reg_value));
    } else if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit)) && 
        (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_P7r
            (unit, &reg_value));
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
        /* Check if global ingress rate config is set */
        if (!gex_ingress_rate_init_flag[unit]) {
            pbmp = PBMP_ALL(unit);
            SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET
                (unit, pbmp, DRV_RATE_CONFIG_PKT_MASK, GEX_IRC_PKT_MASK_IS_3F));
            gex_ingress_rate_init_flag[unit] = 1;
        }

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

        /* Enable ingress rate control */
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, EN_BUCKET0f, &temp));
    }

    /* Write register */
    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_IMPr
            (unit, CMIC_PORT(unit), &reg_value));
    } else if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit)) && 
        (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_P7r
            (unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    return SOC_E_NONE;
}

/*
 *  Function : _drv_polar_port_irc_get
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
_drv_polar_port_irc_get(uint32 unit, uint32 port, uint32 *limit, 
    uint32 *burst_size)
{
    uint32  reg_value, temp, specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_IMPr
            (unit, CMIC_PORT(unit), &reg_value));
    } else if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit)) && 
        (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_P7r
            (unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
            (unit, port, &reg_value));
    }

    /* Check global ingress rate control setting */
    if (gex_ingress_rate_init_flag[unit] != 0) {
        temp = 0;
        SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
            (unit, SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0), 
            DRV_RATE_CONFIG_PKT_MASK, &temp));
                            
        /* If gex_ingress_rate_init_flag[unit] = 0, only next ingress rate setting 
         * may set the properly PKT_MASK0 again currenly.
         */ 
        gex_ingress_rate_init_flag[unit] = (temp == 0) ? 0 : 1;
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
 *  Function : _drv_polar_port_erc_set
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
_drv_polar_port_erc_set(uint32 unit, uint32 port, uint32 limit, 
    uint32 burst_size)
{
    uint32  reg_value, temp, imp1_port_num;
    int  i;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META("_drv_polar_port_erc_set: \
                       unit = %d, port = %d, limit = %d, burst_size = %d\n"),
              unit, port, limit, burst_size));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_IMP1_PORT_NUM, &imp1_port_num));

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_EGRESS_RATE_CTRL_CFG_REGr
            (unit, &reg_value));

        if (limit > gex_rate_index_value[GEX_RATE_INDEX_VALUE_SIZE - 1]) {
            return SOC_E_PARAM;
        }

        /* Rate for IMP0 in terms of Packets Per Second (PPS) */
        for(i = 0; i < GEX_RATE_INDEX_VALUE_SIZE; i++) {
            if (limit <= gex_rate_index_value[i]) {
                temp = i;
                break;
            }
        }
        SOC_IF_ERROR_RETURN(soc_IMP_EGRESS_RATE_CTRL_CFG_REGr_field_set
            (unit, &reg_value, RATE_INDEXf, &temp));
        /* Write register */
        SOC_IF_ERROR_RETURN(REG_WRITE_IMP_EGRESS_RATE_CTRL_CFG_REGr
            (unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, FRM_MNGPf, &temp));
 
        /* If enable Dual-IMP port (IMP0 and IMP1) */
        /* Support Dual-IMP : Polar */
        if ((temp == GEX_ENABLE_DUAL_IMP_PORT) && (port == imp1_port_num)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP1_EGRESS_RATE_CTRL_CFG_REGr
                (unit, &reg_value));

            if (limit > 
                gex_rate_index_value[GEX_RATE_INDEX_VALUE_SIZE - 1]) {
                return SOC_E_PARAM;
            }

            /* Rate for IMP1 in terms of Packets Per Second (PPS) */
            for(i = 0; i < GEX_RATE_INDEX_VALUE_SIZE; i++) {
                if (limit <= gex_rate_index_value[i]) {
                    temp = i;
                    break;
                }
            }
            SOC_IF_ERROR_RETURN(soc_IMP1_EGRESS_RATE_CTRL_CFG_REGr_field_set
                (unit, &reg_value, RATE_INDEXf, &temp));
            /* Write register */            
            SOC_IF_ERROR_RETURN(REG_WRITE_IMP1_EGRESS_RATE_CTRL_CFG_REGr
                (unit, &reg_value));
        } else {
            return SOC_E_UNAVAIL;
        }
    }

    return SOC_E_NONE;
}

/*
 *  Function : _drv_polar_port_erc_per_queue_set
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
 *      Q5 -----------------Vitual Q2 of SCH2---> -------
 *      Q4 -----------------Vitual Q1 of SCH2---> |SCH2  |
 *      Q3 -->  -------                           |      |
 *      Q2 -->  |SCH1 |-----Vitual Q0 of SCH2---> -------- 
 *      Q1 -->  |     |
 *      Q0 -->  -------  *
 *
 *     PS1. There are 4 COS queues for SCH1. They are real COS0 ~ COSQ3.
 *            There are 3 COS queues for SCH2.
 *             - Q0 of SCH2 represents the COSQ where output of SCH1
 *             - Q1 of SCH2 represents the real COSQ4
 *             - Q2 of SCH2 represents the real COSQ5
 *     PS2. The SCH1 COS queues don't support leaky bucket shaper 
 *
 */
static int
_drv_polar_port_erc_per_queue_set(uint32 unit, uint32 port, 
    uint8 queue_n, uint32 limit, uint32 burst_size)
{
    uint32  reg_value, temp;
    uint32  specified_port_num, imp1_port_num, sch2_output_cosq;
    uint32  burst_kbyte = 0;
    uint8  numq;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META("_drv_polar_port_erc_per_queue_set: \
                       unit = %d, port = %d, limit = %d, burst_size = %d\n"),
              unit, port, limit, burst_size));

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_IMP1_PORT_NUM, &imp1_port_num));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_SCH2_OUTPUT_COSQ, &sch2_output_cosq));

    SOC_IF_ERROR_RETURN(DRV_QUEUE_COUNT_GET
        (unit, 0, &numq));

    if (IS_CPU_PORT(unit, port)) {
        return SOC_E_UNAVAIL;
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, FRM_MNGPf, &temp));
 
        /* If enable Dual-IMP port (IMP0 and IMP1) */
        /* Support Dual-IMP : Polar */
        if ((temp == GEX_ENABLE_DUAL_IMP_PORT) && (port == imp1_port_num)) {
            return SOC_E_UNAVAIL;
        } else {
            /* Leaky Bucket Shaper */
            if(queue_n < numq) {
                /* Polar doesn't support Leaky Bucket Shaper for Q0-Q3 */
                return SOC_E_UNAVAIL;
            }

            if(queue_n > sch2_output_cosq) {
                /* Polar only support Leaky Bucket Shaper for Q4-Q6 */
                return SOC_E_PARAM;
            }

            burst_kbyte = (burst_size / 8);
            if ((limit > POLAR_RATE_MAX_REFRESH_RATE) ||
                ((burst_kbyte * 1000) > POLAR_RATE_MAX_BUCKET_SIZE)) {
                return SOC_E_PARAM;
            }

            if (queue_n == 0x6) {
                SOC_IF_ERROR_RETURN(REG_READ_LOW_QUEUE_SHAPER_ENABLEr
                    (unit, &reg_value));
            } else if (queue_n == 0x4) {
                SOC_IF_ERROR_RETURN(REG_READ_QUEUE4_SHAPER_ENABLEr
                    (unit, &reg_value));
            } else if (queue_n == 0x5) {
                SOC_IF_ERROR_RETURN(REG_READ_QUEUE5_SHAPER_ENABLEr
                    (unit, &reg_value));                
            } 

            if (limit == 0x0) {
                /* Disable Shaper */
                reg_value = reg_value & ~(1 << port);
            } else {
                reg_value = reg_value | (1 << port);
            }

            if (queue_n == 0x6) {
                SOC_IF_ERROR_RETURN(REG_WRITE_LOW_QUEUE_SHAPER_ENABLEr
                    (unit, &reg_value));
            } else if (queue_n == 0x4) {
                SOC_IF_ERROR_RETURN(REG_WRITE_QUEUE4_SHAPER_ENABLEr
                    (unit, &reg_value));
            } else if (queue_n == 0x5) {
                SOC_IF_ERROR_RETURN(REG_WRITE_QUEUE5_SHAPER_ENABLEr
                    (unit, &reg_value));                
            } 

            if(limit == 0x0) {
                return SOC_E_NONE;
            }

            /* Program refresh rate */
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit)) && 
                (port == specified_port_num)) {
                if (queue_n == 0x6) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_LOW_QUEUE_MAX_REFRESHr
                        (unit, &reg_value));
                } else if (queue_n == 0x4) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_QUEUE4_MAX_REFRESHr
                        (unit, &reg_value));
                } else if (queue_n == 0x5) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_QUEUE5_MAX_REFRESHr
                        (unit, &reg_value));
                }
            } else {
                if (queue_n == 0x6) {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_LOW_QUEUE_MAX_REFRESHr
                        (unit, port, &reg_value));
                } else if (queue_n == 0x4) {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_QUEUE4_MAX_REFRESHr
                        (unit, port, &reg_value));
                } else if (queue_n == 0x5) {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_QUEUE5_MAX_REFRESHr
                        (unit, port, &reg_value));                
                }
            }

            temp = ((limit - 1) / POLAR_RATE_REFRESH_GRANULARITY) + 1;
            SOC_IF_ERROR_RETURN(soc_PN_LOW_QUEUE_MAX_REFRESHr_field_set
                (unit, &reg_value, MAX_REFRESHf, &temp));
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit)) && 
                (port == specified_port_num)) {
                if (queue_n == 0x6) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_LOW_QUEUE_MAX_REFRESHr
                        (unit, &reg_value));
                } else if (queue_n == 0x4) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_QUEUE4_MAX_REFRESHr
                        (unit, &reg_value));
                } else if (queue_n == 0x5) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_QUEUE5_MAX_REFRESHr
                        (unit, &reg_value));
                }
            } else {
                if (queue_n == 0x6) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_PN_LOW_QUEUE_MAX_REFRESHr
                        (unit, port, &reg_value));
                } else if (queue_n == 0x4) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_PN_QUEUE4_MAX_REFRESHr
                        (unit, port, &reg_value));
                } else if (queue_n == 0x5) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_PN_QUEUE5_MAX_REFRESHr
                        (unit, port, &reg_value));                
                }
            }

            /* Program max threshold for burst size */
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) && 
                (port == specified_port_num)) {
                if (queue_n == 0x6) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_LOW_QUEUE_MAX_THD_SELr
                        (unit, &reg_value));
                } else if (queue_n == 0x4) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_QUEUE4_MAX_THD_SELr
                        (unit, &reg_value));
                } else if (queue_n == 0x5) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_QUEUE5_MAX_THD_SELr
                        (unit, &reg_value));
                }
            } else {
                if (queue_n == 0x6) {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_LOW_QUEUE_MAX_THD_SELr
                        (unit, port, &reg_value));
                } else if (queue_n == 0x4) {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_QUEUE4_MAX_THD_SELr
                        (unit, port, &reg_value));
                } else if (queue_n == 0x5) {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_QUEUE5_MAX_THD_SELr
                        (unit, port, &reg_value));                
                }
            }

            /* Burst size for Total */
            temp = ((burst_kbyte * 1000) / POLAR_RATE_BUCKET_UNIT_SIZE);
            SOC_IF_ERROR_RETURN(soc_PN_LOW_QUEUE_MAX_THD_SELr_field_set
                (unit, &reg_value, MAX_THD_SELf, &temp));

            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit)) && 
                (port == specified_port_num)) {
                if (queue_n == 0x6) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_LOW_QUEUE_MAX_THD_SELr
                        (unit, &reg_value));
                } else if (queue_n == 0x4) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_QUEUE4_MAX_THD_SELr
                        (unit, &reg_value));
                } else if (queue_n == 0x5) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_P7_QUEUE5_MAX_THD_SELr
                        (unit, &reg_value));
                }
            } else {
                if (queue_n == 0x6) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_PN_LOW_QUEUE_MAX_THD_SELr
                        (unit, port, &reg_value));
                } else if (queue_n == 0x4) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_PN_QUEUE4_MAX_THD_SELr
                        (unit, port, &reg_value));
                } else if (queue_n == 0x5) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_PN_QUEUE5_MAX_THD_SELr
                        (unit, port, &reg_value));                
                }
            }
        }
    }

    return SOC_E_NONE;
}

/*
 *  Function : _drv_polar_port_erc_get
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
_drv_polar_port_erc_get(uint32 unit, uint32 port, uint32 *limit, 
    uint32 *burst_size)
{
    uint32  reg_value, temp, imp1_port_num;

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_IMP1_PORT_NUM, &imp1_port_num));

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_EGRESS_RATE_CTRL_CFG_REGr
            (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_IMP_EGRESS_RATE_CTRL_CFG_REGr_field_get
            (unit, &reg_value, RATE_INDEXf, &temp));

        /* Rate for IMP0 in terms of Packets Per Second (PPS) */
        *limit = gex_rate_index_value[temp];
        *burst_size = 0;
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, FRM_MNGPf, &temp));
        /* If enable Dual-IMP port (IMP0 and IMP1) */
        /* Support Dual-IMP : POLAR */
        if ((temp == GEX_ENABLE_DUAL_IMP_PORT) && (port == imp1_port_num)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP1_EGRESS_RATE_CTRL_CFG_REGr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_IMP1_EGRESS_RATE_CTRL_CFG_REGr_field_get
                (unit, &reg_value, RATE_INDEXf, &temp));

            /* Rate for IMP1 in terms of Packets Per Second (PPS) */
            *limit = gex_rate_index_value[temp];
            *burst_size = 0;
        } else {
            return SOC_E_UNAVAIL;
        }
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META("_drv_polar_port_erc_get: \
                       unit = %d, port = %d, limit = %dK, burst size = %dKB\n"), 
              unit, port, *limit, *burst_size));

    return SOC_E_NONE;
}

/*
 *  Function : _drv_polar_port_erc_per_queue_get
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
_drv_polar_port_erc_per_queue_get(uint32 unit, uint32 port, 
    uint8 queue_n, uint32 *limit, uint32 *burst_size)
{
    uint32  reg_value, temp;
	uint32  specified_port_num, imp1_port_num, sch2_output_cosq;
    uint8  numq;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_IMP1_PORT_NUM, &imp1_port_num));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_SCH2_OUTPUT_COSQ, &sch2_output_cosq));

    SOC_IF_ERROR_RETURN(DRV_QUEUE_COUNT_GET
        (unit, 0, &numq));

    if (IS_CPU_PORT(unit, port)) {
        return SOC_E_UNAVAIL; 
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, FRM_MNGPf, &temp));
        /* If enable Dual-IMP port (IMP0 and IMP1) */
        /* Support Dual-IMP : POLAR */
        if ((temp == GEX_ENABLE_DUAL_IMP_PORT) && (port == imp1_port_num)) {
            return SOC_E_UNAVAIL; 
        } else {
            /* Leaky Bucket Shaper */
            if(queue_n < numq) {
                /* Polar doesn't support Leaky Bucket Shaper for Q0-Q3 */
                return SOC_E_UNAVAIL;
            }

            if(queue_n > sch2_output_cosq) {
                /* Polar only support Leaky Bucket Shaper for Q4-Q6 */
                return SOC_E_PARAM;
            }

            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit)) && 
                (port == specified_port_num)) {
                if (queue_n == 0x6) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_LOW_QUEUE_MAX_REFRESHr
                        (unit, &reg_value));
                } else if (queue_n == 0x4) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_QUEUE4_MAX_REFRESHr
                        (unit, &reg_value));
                } else if (queue_n == 0x5) {
                    SOC_IF_ERROR_RETURN(REG_READ_P7_QUEUE5_MAX_REFRESHr
                        (unit, &reg_value));
                }
            } else {
                if (queue_n == 0x6) {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_LOW_QUEUE_MAX_REFRESHr
                        (unit, port, &reg_value));
                } else if (queue_n == 0x4) {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_QUEUE4_MAX_REFRESHr
                        (unit, port, &reg_value));
                } else if (queue_n == 0x5) {
                    SOC_IF_ERROR_RETURN(REG_READ_PN_QUEUE5_MAX_REFRESHr
                        (unit, port, &reg_value));                
                }
            }

            SOC_IF_ERROR_RETURN(soc_PN_LOW_QUEUE_MAX_REFRESHr_field_get
                (unit, &reg_value, MAX_REFRESHf, &temp));
            if (temp == 0) {
                *limit = 0;
                *burst_size = 0;
            } else {
                *limit = (temp * 64);
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_NORTHSTARPLUS(unit)) && 
                    (port == specified_port_num)) {
                    if (queue_n == 0x6) {
                        SOC_IF_ERROR_RETURN(REG_READ_P7_LOW_QUEUE_MAX_THD_SELr
                            (unit, &reg_value));
                    } else if (queue_n == 0x4) {
                        SOC_IF_ERROR_RETURN(REG_READ_P7_QUEUE4_MAX_THD_SELr
                            (unit, &reg_value));
                    } else if (queue_n == 0x5) {
                        SOC_IF_ERROR_RETURN(REG_READ_P7_QUEUE5_MAX_THD_SELr
                            (unit, &reg_value));
                    }
                } else {
                    if (queue_n == 0x6) {
                        SOC_IF_ERROR_RETURN(REG_READ_PN_LOW_QUEUE_MAX_THD_SELr
                            (unit, port, &reg_value));
                    } else if (queue_n == 0x4) {
                        SOC_IF_ERROR_RETURN(REG_READ_PN_QUEUE4_MAX_THD_SELr
                            (unit, port, &reg_value));
                    } else if (queue_n == 0x5) {
                        SOC_IF_ERROR_RETURN(REG_READ_PN_QUEUE5_MAX_THD_SELr
                            (unit, port, &reg_value));                
                    }
                }
                SOC_IF_ERROR_RETURN(soc_PN_LOW_QUEUE_MAX_THD_SELr_field_get
                    (unit, &reg_value, MAX_THD_SELf, &temp));

                *burst_size = ((temp * POLAR_RATE_BUCKET_UNIT_SIZE * 8) / 1000);
            }
        }
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META("_drv_polar_port_erc_per_queue_get: \
                       unit = %d, port = %d, limit = %dK, burst size = %dKB\n"), 
              unit, port, *limit, *burst_size));

    return SOC_E_NONE;
}
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

/*
 *  Function : _drv_gex_port_irc_set
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
_drv_gex_port_irc_set(int unit, uint32 port, uint32 limit, 
    uint32 burst_size)
{
    uint32  reg_value, temp = 0;
    uint32  burst_kbyte = 0;
    soc_pbmp_t  pbmp;

    SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
        (unit, port, &reg_value));

    if (limit == 0) {  /* Disable ingress rate control */
        /* Disable ingress rate control bucket 0 */
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, EN_BUCKET0f, &temp));
    } else {  /* Enable ingress rate control */
        /* Check if global ingress rate config is set */
        if (!gex_ingress_rate_init_flag[unit]) {
            pbmp = PBMP_ALL(unit);

            if (SOC_IS_LOTUS(unit)) {
#ifdef BCM_LOTUS_SUPPORT
                SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
                    (unit, &reg_value, EN_VLAN_POLICINGf, &temp));
                if (!temp) {
                    SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET
                        (unit, pbmp, DRV_RATE_CONFIG_PKT_MASK, 
                        GEX_IRC_PKT_MASK_IS_3F));
                }
#endif  /* BCM_LOTUS_SUPPORT */
            } else {
                SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET
                    (unit, pbmp, DRV_RATE_CONFIG_PKT_MASK, 
                    GEX_IRC_PKT_MASK_IS_7F));
            }
            gex_ingress_rate_init_flag[unit] = 1;
        }

        if (burst_size > (500 * 8)) {  /* 500 KB */
            return SOC_E_PARAM;
        }
        burst_kbyte = (burst_size / 8);

        if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) || 
            SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT)
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
#endif  /* BCM_LOTUS_SUPPORT (STARFIGHTER/BLACKBIRD2) */
        } else {
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
        }
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, BUCKET0_SIZEf, &temp));

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

        /* Enable ingress rate control */
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_set
            (unit, &reg_value, EN_BUCKET0f, &temp));
    }

    /* Write register */
    SOC_IF_ERROR_RETURN(REG_WRITE_BC_SUP_RATECTRL_Pr
        (unit, port, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : _drv_gex_port_irc_get
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
_drv_gex_port_irc_get(uint32 unit, uint32 port, uint32 *limit, 
    uint32 *burst_size)
{
    uint32  reg_value, temp;

    SOC_IF_ERROR_RETURN(REG_READ_BC_SUP_RATECTRL_Pr
        (unit, port, &reg_value));

    /* Check global ingress rate control setting */
    if (gex_ingress_rate_init_flag[unit] != 0) {
        temp = 0;
#ifdef BCM_LOTUS_SUPPORT
        if (SOC_IS_LOTUS(unit)) {
            SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
                (unit, &reg_value, EN_VLAN_POLICINGf, &temp));
            if (temp != 0) {
                *limit = 0;
                *burst_size = 0;
                return SOC_E_NONE; 
            }
        }
#endif  /* BCM_LOTUS_SUPPORT */
        SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
            (unit, SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0), 
            DRV_RATE_CONFIG_PKT_MASK, &temp));

        /* If gex_ingress_rate_init_flag[unit] = 0, only next ingress rate setting 
         * may set the properly PKT_MASK0 again currenly.
         */ 
        gex_ingress_rate_init_flag[unit] = (temp == 0) ? 0 : 1;
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

        if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) || 
            SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT)
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
#endif  /* BCM_LOTUS_SUPPORT (STARFIGHTER/BLACKBIRD2) */
        } else {
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

        SOC_IF_ERROR_RETURN(soc_BC_SUP_RATECTRL_Pr_field_get
            (unit, &reg_value, BUCKET0_REF_CNTf, &temp));
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
 *  Function : _drv_gex_port_erc_set
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
 */
static int
_drv_gex_port_erc_set(uint32 unit, uint32 port, uint32 limit, 
    uint32 burst_size)
{
    uint32  reg_value, temp = 0;
    uint32  burst_kbyte = 0;
    int  i;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META("_drv_gex_port_erc_set: \
                       unit = %d, port = %d, limit = %d, burst_size = %d\n"), 
              unit, port, limit, burst_size));

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_EGRESS_RATE_CTRL_CFG_REGr
            (unit, &reg_value));

        if (limit > gex_rate_index_value[GEX_RATE_INDEX_VALUE_SIZE - 1]) {
            return SOC_E_PARAM;
        }

        /* Rate for IMP0 in terms of Packets Per Second (PPS) */
        for(i = 0; i < GEX_RATE_INDEX_VALUE_SIZE; i++) {
            if (limit <= gex_rate_index_value[i]) {
                temp = i;
                break;
            }
        }
        SOC_IF_ERROR_RETURN(soc_IMP_EGRESS_RATE_CTRL_CFG_REGr_field_set
            (unit, &reg_value, RATE_INDEXf, &temp));
        /* Write register */
        SOC_IF_ERROR_RETURN(REG_WRITE_IMP_EGRESS_RATE_CTRL_CFG_REGr
            (unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, FRM_MNGPf, &temp));
 
        /* If enable Dual-IMP port (IMP0 and IMP1) */
        /* Support Dual-IMP : Vulcan/Starfighter */
        if ((SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit)) &&
            (temp == GEX_ENABLE_DUAL_IMP_PORT) && (port == 5)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP1_EGRESS_RATE_CTRL_CFG_REGr
                (unit, &reg_value));

            if (limit > gex_rate_index_value[GEX_RATE_INDEX_VALUE_SIZE - 1]) {
                return SOC_E_PARAM;
            }

            /* Rate for IMP1 in terms of Packets Per Second (PPS) */
            for(i = 0 ; i < GEX_RATE_INDEX_VALUE_SIZE ; i++) {
                if (limit <= gex_rate_index_value[i]) {
                    temp = i;
                    break;
                }
            }
            SOC_IF_ERROR_RETURN(soc_IMP1_EGRESS_RATE_CTRL_CFG_REGr_field_set
                (unit, &reg_value, RATE_INDEXf, &temp));
            /* Write register */
            SOC_IF_ERROR_RETURN(REG_WRITE_IMP1_EGRESS_RATE_CTRL_CFG_REGr
                (unit, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_PORT_ERC_CONr
                (unit, port, &reg_value));

            if (limit == 0) {  /* Disable ingress rate control */
                temp = 0;
                SOC_IF_ERROR_RETURN(soc_PORT_ERC_CONr_field_set
                    (unit, &reg_value, EGRESS_RC_ENf, &temp));
            } else {  /* Enable ingress rate control */
                if (burst_size > (500 * 8)) {  /* 500 KB */
                    return SOC_E_PARAM;
                }

                /* Burst size */
                burst_kbyte = (burst_size / 8);
                if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT)
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
#endif  /* BCM_LOTUS_SUPPORT (STARFIGHTER/BLACKBIRD2) */
                } else {
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
                }
                SOC_IF_ERROR_RETURN(soc_PORT_ERC_CONr_field_set
                    (unit, &reg_value, BUCKET_SIZEf, &temp));

                /* Refresh count  (fixed type) */
                if (limit <= 1792) {  /* 64KB ~ 1.792MB */
                    temp = ((limit - 1) / 64) + 1;
                } else if (limit <= 102400) {  /* 2MB ~ 100MB */
                    temp = (limit / 1000 ) + 27;
                } else if (limit <= 1000000) {  /* 104MB ~ 1000MB */
                    temp = (limit / 8000) + 115;
                } else {
                    return SOC_E_PARAM;
                }

                if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT)
                    SOC_IF_ERROR_RETURN(soc_PORT_ERC_CONr_field_set
                        (unit, &reg_value, REF_CNTf, &temp));
#endif  /* BCM_LOTUS_SUPPORT (STARFIGHTER/BLACKBIRD2) */
                } else {
                    SOC_IF_ERROR_RETURN(soc_PORT_ERC_CONr_field_set
                        (unit, &reg_value, REF_CNT_Rf, &temp));
                }

                /* Enable ingress rate control */
                temp = 1;
                SOC_IF_ERROR_RETURN(soc_PORT_ERC_CONr_field_set
                    (unit, &reg_value, EGRESS_RC_ENf, &temp));
            }

            /* Write register */
            SOC_IF_ERROR_RETURN(REG_WRITE_PORT_ERC_CONr
                (unit, port, &reg_value));
        }
    }

    return SOC_E_NONE;
}

/*
 *  Function : _drv_gex_port_erc_get
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
_drv_gex_port_erc_get(uint32 unit, uint32 port, uint32 *limit, 
    uint32 *burst_size)
{
    uint32  reg_value, temp = 0;

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_IMP_EGRESS_RATE_CTRL_CFG_REGr
            (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_IMP_EGRESS_RATE_CTRL_CFG_REGr_field_get
            (unit, &reg_value, RATE_INDEXf, &temp));

        /* Rate for IMP0 in terms of Packets Per Second (PPS) */
        *limit = gex_rate_index_value[temp];
        *burst_size = 0;
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, FRM_MNGPf, &temp));

        /* If enable Dual-IMP port (IMP0 and IMP1) */
        /* Support Dual-IMP : Vulcan */
        if ((SOC_IS_VULCAN(unit)) &&
            (temp == GEX_ENABLE_DUAL_IMP_PORT) && (port == 5)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP1_EGRESS_RATE_CTRL_CFG_REGr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_IMP1_EGRESS_RATE_CTRL_CFG_REGr_field_get
                (unit, &reg_value, RATE_INDEXf, &temp));

            /* Rate for IMP1 in terms of Packets Per Second (PPS) */
            *limit = gex_rate_index_value[temp];
            *burst_size = 0;
        } else if ((SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit)) &&
            (port == 5) && (temp == GEX_ENABLE_DUAL_IMP_PORT)) {
            SOC_IF_ERROR_RETURN(REG_READ_IMP_EGRESS_RATE_CTRL_CFG_REGr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_IMP_EGRESS_RATE_CTRL_CFG_REGr_field_get
                (unit, &reg_value, RATE_INDEXf, &temp));
            
            /* Rate for IMP0 in terms of Packets Per Second (PPS) */
            *limit = gex_rate_index_value[temp];
            *burst_size = 0;
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_PORT_ERC_CONr
                (unit, port, &reg_value));
            SOC_IF_ERROR_RETURN(soc_PORT_ERC_CONr_field_get
                (unit, &reg_value, EGRESS_RC_ENf, &temp));

            if (temp == 0) {
                *limit = 0;
                *burst_size = 0;
            } else {
                SOC_IF_ERROR_RETURN(soc_PORT_ERC_CONr_field_get
                    (unit, &reg_value, BUCKET_SIZEf, &temp));

                if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT)
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
#endif  /* BCM_LOTUS_SUPPORT (STARFIGHTER/BLACKBIRD2) */
                } else {
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

                if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_BLACKBIRD2(unit)) {
                    SOC_IF_ERROR_RETURN(soc_PORT_ERC_CONr_field_get
                        (unit, &reg_value, REF_CNTf, &temp));
                } else {
                    SOC_IF_ERROR_RETURN(soc_PORT_ERC_CONr_field_get
                        (unit, &reg_value, REF_CNT_Rf, &temp));
                }

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
        }
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META("_drv_gex_port_erc_get: \
                       unit = %d, port = %d, limit = %dK, burst size = %dKB\n"), 
              unit, port, *limit, *burst_size));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_rate_config_set
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
drv_gex_rate_config_set(int unit, soc_pbmp_t pbmp, uint32 config_type, 
    uint32 value)
{
    uint32  reg_value, temp;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_gex_rate_config_set: \
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
                SOC_IF_ERROR_RETURN(soc_COMM_IRC_CONr_field_set
                    (unit, &reg_value, RATE_TYPE0f, &temp));
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
                SOC_IF_ERROR_RETURN(soc_COMM_IRC_CONr_field_set
                    (unit, &reg_value, DROP_EN0f, &temp));
                SOC_IF_ERROR_RETURN(REG_WRITE_COMM_IRC_CONr
                    (unit, &reg_value));
            }
            break;
        case DRV_RATE_CONFIG_PKT_MASK: 
            /* Per chip */
            if (SOC_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                    (unit, &reg_value));

                if (SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) ||
                    SOC_IS_NORTHSTAR(unit)) {
                    temp = value & GEX_IRC_PKT_MASK_IS_3F;
                } else {
                    temp = value & GEX_IRC_PKT_MASK_IS_7F;
                }
                SOC_IF_ERROR_RETURN(soc_COMM_IRC_CONr_field_set
                    (unit, &reg_value, PKT_MSK0f, &temp));
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
 *  Function : drv_gex_rate_config_get
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
drv_gex_rate_config_get(int unit, uint32 port, uint32 config_type, 
     uint32 *value)
{
    uint32  reg_value, temp = 0;

    /* Get bucket 0*/
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE: 
            /* Per chip */
            SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_COMM_IRC_CONr_field_get
                (unit, &reg_value, RATE_TYPE0f, &temp));
            *value = temp;
            break;
        case DRV_RATE_CONFIG_DROP_ENABLE:
            /* Per chip */
            SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_COMM_IRC_CONr_field_get
                (unit, &reg_value, DROP_EN0f, &temp));
            *value = temp;
            break;
        case DRV_RATE_CONFIG_PKT_MASK:
            /* Per chip */
            SOC_IF_ERROR_RETURN(REG_READ_COMM_IRC_CONr
                (unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_COMM_IRC_CONr_field_get
                (unit, &reg_value, PKT_MSK0f, &temp));
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
                         "drv_gex_rate_config_get: \
                         unit = %d, port = %d, type = 0x%x, value = 0x%x\n"),
              unit, port, config_type, *value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_rate_set
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
drv_gex_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, int direction, 
    uint32 flags, uint32 kbits_sec_min, 
    uint32 kbits_sec_max, uint32 burst_size)
{
    uint32  port;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_gex_rate_set: unit = %d, bmp = 0x%x, %s, flags = 0x%x, \
                         kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n"), 
              unit, SOC_PBMP_WORD_GET(bmp, 0), (direction - 1) ? "EGRESS" : "INGRESS", 
              flags, kbits_sec_min, kbits_sec_max, burst_size));

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
                PBMP_ITER(bmp, port) {
                    SOC_IF_ERROR_RETURN(_drv_polar_port_irc_set
                        (unit, port, kbits_sec_max, burst_size));
                }
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
            {
                PBMP_ITER(bmp, port) {
                    SOC_IF_ERROR_RETURN(_drv_gex_port_irc_set
                        (unit, port, kbits_sec_max, burst_size));
                }
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
                PBMP_ITER(bmp, port) {
                    SOC_IF_ERROR_RETURN(_drv_polar_port_erc_set
                        (unit, port, kbits_sec_max, burst_size));
                }
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
            {
                PBMP_ITER(bmp, port) {
                    SOC_IF_ERROR_RETURN(_drv_gex_port_erc_set
                        (unit, port, kbits_sec_max, burst_size));
                }
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
                PBMP_ITER(bmp, port) {
                    SOC_IF_ERROR_RETURN(_drv_polar_port_erc_per_queue_set
                        (unit, port, queue_n, kbits_sec_max, burst_size));
                }
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
            {
                return SOC_E_UNAVAIL;
            }
            break;
        default:
            return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_rate_get
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
 *      For Polar QOS scheduling:
 *      Q5 ----------------------Vitual Q2 of SCH2---> -------
 *      Q4 ----------------------Vitual Q1 of SCH2---> |SCH2  |
 *      Q3 --->  -------                               |      |
 *      Q2 --->  |SCH1 |---Q6----Vitual Q0 of SCH2---> -------- 
 *      Q1 --->  |     |
 *      Q0 --->  ------- 
 *
 *      PS. SCH1 use LEVEL1_QOS_PRI_CTL register and SCH2 use LEVEL2_QOS_PRI_CTL 
 *           There are 4 COS queues for SCH1. They are COSQ0 ~ COSQ3.
 *           There are 3 COS queues for SCH2. They are COSQ4, COSQ5 
 *                             and COSQ6 (the output of SCH1). 
 */
int 
drv_gex_rate_get(int unit, uint32 port, uint8 queue_n, int direction, 
    uint32 *flags, uint32 *kbits_sec_min, uint32 *kbits_sec_max, uint32 *burst_size)
{
    uint32  min_rate = 0;  /* Dummy variable */

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
                SOC_IF_ERROR_RETURN(_drv_polar_port_irc_get
                    (unit, port, kbits_sec_max, burst_size));
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
            {
                SOC_IF_ERROR_RETURN(_drv_gex_port_irc_get
                    (unit, port, kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
                SOC_IF_ERROR_RETURN(_drv_polar_port_erc_get
                    (unit, port, kbits_sec_max, burst_size));
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
            {
                SOC_IF_ERROR_RETURN(_drv_gex_port_erc_get
                    (unit, port, kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
                SOC_IF_ERROR_RETURN(_drv_polar_port_erc_per_queue_get
                    (unit, port, queue_n, kbits_sec_max, burst_size));
                *kbits_sec_min = 0;
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */
            {
                return SOC_E_UNAVAIL;
            }
            break;
        default:
            return SOC_E_PARAM;
    }

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_gex_rate_get: unit = %d, port = %d, %s, flags = 0x%x, \
                         kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n"),
              unit, port, (direction - 1) ? "EGRESS" : "INGRESS", 
              *flags, min_rate, *kbits_sec_max, *burst_size));

    return SOC_E_NONE;
}

