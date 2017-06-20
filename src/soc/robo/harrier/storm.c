/*
 * $Id: storm.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/mem.h>
#include <harrier/robo_harrier.h>

/*
 *  Function : _drv_harrier_storm_control_type_enable_set
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
_drv_harrier_storm_control_type_enable_set(int unit, uint32 port, 
    uint32 type, uint32 enable)
{
    uint32  zero_value, reg_value, acc_ctrl = 0;
    uint32  retry, temp;
    int  rv = SOC_E_NONE;

    /* Process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    /* Clear Rate Control Memory Data access registers */
    zero_value = 0;

    /* Use RCM_DATA1 register set bucket x */
    /*
     * Bucket 0 is default used as per port rate control.
     * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
     * If no bucket number assigned, use bucket 1 as default bucket.
     */
    if (type & DRV_STORM_CONTROL_BUCKET_2) {
        if ((rv = REG_WRITE_RCM_DATA2r(unit, &zero_value)) < 0) {
            goto storm_type_enable_set_exit;
        }
    } else {
        if ((rv = REG_WRITE_RCM_DATA1r(unit, &zero_value)) < 0) {
            goto storm_type_enable_set_exit;
        }
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_set_exit;
    }

    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_set_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);

    /* 1'b0 : Ingress Rate Control Memory access */
    temp = 0;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);

    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_set_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_type_enable_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_type_enable_set_exit;
    }

    /* Read Rate Control Memory DATA x register */
    if (type & DRV_STORM_CONTROL_BUCKET_2) {
        if ((rv = REG_READ_RCM_DATA2r(unit, &reg_value)) < 0) {
            goto storm_type_enable_set_exit;
        }
        soc_RCM_DATA2r_field_get(unit, &reg_value, 
            IRC_PKT_MASK_B2f, &temp);
    } else {
        if ((rv = REG_READ_RCM_DATA1r(unit, &reg_value)) < 0) {
            goto storm_type_enable_set_exit;
        }
        soc_RCM_DATA1r_field_get(unit, &reg_value, 
            IRC_PKT_MASK_B1f, &temp);
    }

    if (type & DRV_STORM_CONTROL_BCAST) {
        if (enable) {
            temp |= HARRIER_STORM_SUPPRESSION_BROADCAST_MASK;
        } else {
            temp &= ~HARRIER_STORM_SUPPRESSION_BROADCAST_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_MCAST) {
        if (enable) {
            temp |= (HARRIER_STORM_SUPPRESSION_MULTICAST_MASK |
                     HARRIER_STORM_SUPPRESSION_MLF_MASK);
        } else {
            temp &= ~(HARRIER_STORM_SUPPRESSION_MULTICAST_MASK |
                      HARRIER_STORM_SUPPRESSION_MLF_MASK);
        }
    } 
    if (type & DRV_STORM_CONTROL_DLF) {
        if (enable) {
            temp |= HARRIER_STORM_SUPPRESSION_DLF_MASK;
        } else {
            temp &= ~HARRIER_STORM_SUPPRESSION_DLF_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_SALF) {
        if (enable) {
            temp |= HARRIER_STORM_SUPPRESSION_UNKNOW_SA_MASK;
        } else {
            temp &= ~HARRIER_STORM_SUPPRESSION_UNKNOW_SA_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_RSV_MCAST) {
        if (enable) {
            temp |= HARRIER_STORM_SUPPRESSION_BPDU_MASK;
        } else {
            temp &= ~HARRIER_STORM_SUPPRESSION_BPDU_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_UCAST) {
        if (enable) {
            temp |= HARRIER_STORM_SUPPRESSION_UNICAST_MASK;
        } else {
            temp &= ~HARRIER_STORM_SUPPRESSION_UNICAST_MASK;
        }
    }

    if (type & DRV_STORM_CONTROL_BUCKET_2) {
        soc_RCM_DATA2r_field_set(unit, &reg_value, 
            IRC_PKT_MASK_B2f, &temp);
        if ((rv = REG_WRITE_RCM_DATA2r(unit, &reg_value)) < 0) {
            goto storm_type_enable_set_exit;
        }
    } else {
        soc_RCM_DATA1r_field_set(unit, &reg_value, 
            IRC_PKT_MASK_B1f, &temp);
        if ((rv = REG_WRITE_RCM_DATA1r(unit, &reg_value)) < 0) {
            goto storm_type_enable_set_exit;
        }
    }

    /* Write to Rate Control Memory */
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_set_exit;
    }

    temp = MEM_TABLE_WRITE;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);

    /* 1'b0 : Ingress Rate Control Memory access */
    temp = 0;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);

    /* Start Write Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_set_exit;
    }

    /* Wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_type_enable_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_type_enable_set_exit;
    }

 storm_type_enable_set_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));

    return rv;
}

/*
 *  Function : _drv_harrier_storm_control_type_enable_get
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
_drv_harrier_storm_control_type_enable_get(int unit, uint32 port, 
    uint32 *type)
{
    uint32  zero_value, reg_value, acc_ctrl = 0;
    uint32  retry, temp;
    int  rv = SOC_E_NONE;

    /* Process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));

    /* Clear Rate Control Memory Data access registers */
    zero_value = 0;

    /* Use RCM_DATA1 register set bucket x */
    /*
     * Bucket 0 is default used as per port rate control.
     * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
     * If no bucket number assigned, use bucket 1 as default bucket.
     */
    if (*type & DRV_STORM_CONTROL_BUCKET_2) {
        if ((rv = REG_WRITE_RCM_DATA2r(unit, &zero_value)) < 0) {
            goto storm_type_enable_get_exit;
        }
    } else {
        if ((rv = REG_WRITE_RCM_DATA1r(unit, &zero_value)) < 0) {
            goto storm_type_enable_get_exit;
        }
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_get_exit;
    }

    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_get_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);

    /* 1'b0 : Ingress Rate Control Memory access */
    temp = 0;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);

    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_get_exit;
    }

    /* Wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_type_enable_get_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_type_enable_get_exit;
    }

    /* Read Rate Control Memory DATA x register */
    if (*type & DRV_STORM_CONTROL_BUCKET_2) {
        if ((rv = REG_READ_RCM_DATA2r(unit, &reg_value)) < 0) {
            goto storm_type_enable_get_exit;
        }
        soc_RCM_DATA2r_field_get(unit, &reg_value, 
            IRC_PKT_MASK_B2f, &temp);
    } else {
        if ((rv = REG_READ_RCM_DATA1r(unit, &reg_value)) < 0) {
            goto storm_type_enable_get_exit;
        }
        soc_RCM_DATA1r_field_get(unit, &reg_value, 
            IRC_PKT_MASK_B1f, &temp);
    }

    if (temp & HARRIER_STORM_SUPPRESSION_BROADCAST_MASK) {
        *type |= DRV_STORM_CONTROL_BCAST;
    }
    
    if (temp & HARRIER_STORM_SUPPRESSION_MULTICAST_MASK) {
        *type |= DRV_STORM_CONTROL_MCAST;
    }
    
    if (temp & HARRIER_STORM_SUPPRESSION_DLF_MASK) {
        *type |= DRV_STORM_CONTROL_DLF;
    }

    if (temp & HARRIER_STORM_SUPPRESSION_UNKNOW_SA_MASK) {
        *type |= DRV_STORM_CONTROL_SALF;
    }

    if (temp & HARRIER_STORM_SUPPRESSION_BPDU_MASK) {
        *type |= DRV_STORM_CONTROL_RSV_MCAST;
    }

    if (temp & HARRIER_STORM_SUPPRESSION_UNICAST_MASK) {
        *type |= DRV_STORM_CONTROL_UCAST;
    }

 storm_type_enable_get_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));

    return rv;
}

/*
 *  Function : drv_harrier_storm_control_enable_set
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
drv_harrier_storm_control_enable_set(int unit, uint32 port, uint8 enable)
{
    uint32  retry, temp;
    uint32  zero_value, reg_value, acc_ctrl = 0;
    int  rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_harrier_storm_control_enable_set: \
                         unit = %d, port = %d, %sable\n"), unit, port, (enable) ? "en" : "dis"));

    /* Process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));

    /* Clear Rate Control Memory Data access registers */
    /* Use RCM_DATA0 register set bucket 0 */
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto storm_enable_set_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_set_exit;
    }

    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_set_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);

    /* 1'b0 : Ingress Rate Control Memory access */
    temp = 0;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);

    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_set_exit;
    }

    /* Wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_enable_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_enable_set_exit;
    }

    /* Read Rate Control Memory DATA 0 register */
    if ((rv = REG_READ_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto storm_enable_set_exit;
    }

    /* Enable drop_enable bit */
    temp = 1;
    soc_RCM_DATA0r_field_set(unit, &reg_value,
        IRC_DROP_ENf, &temp);

    /* Enable Ingress rate control bit */
    soc_RCM_DATA0r_field_get(unit, &reg_value,
        IE_RC_ENf, &temp);
    if (!temp) {
        temp = 1;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            IE_RC_ENf, &temp);
    }

    /* 
     * When global ingress rate control bit enabled.
     * The rate limit and bucket size can't be 0.
     * Set to maxmimum value if no value in the fields.
     */
    soc_RCM_DATA0r_field_get(unit, &reg_value,
        REF_UNITf, &temp);
    if (!temp) {
        temp = HARRIER_RATE_REF_UNIT_1M;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            REF_UNITf, &temp);
    }

    soc_RCM_DATA0r_field_get(unit, &reg_value,
        REF_CNTSf, &temp);
    if (!temp) {
        temp = HARRIER_RATE_MAX_REF_CNTS;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);
    }

    soc_RCM_DATA0r_field_get(unit, &reg_value,
        BUCKET_SIZEf, &temp);
    if (!temp) {
        temp = HARRIER_RATE_MAX_BUCKET_SIZE;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            BUCKET_SIZEf, &temp);
    }

    if ((rv = REG_WRITE_RCM_DATA0r(unit, &reg_value)) < 0) {
        goto storm_enable_set_exit;
    }

    /* Write to Rate Control Memory */
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_set_exit;
    }

    temp = MEM_TABLE_WRITE;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);

    /* 1'b0 : Ingress Rate Control Memory access */
    temp = 0;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);

    /* Start Write Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_set_exit;
    }

    /* Wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_enable_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_enable_set_exit;
    }

storm_enable_set_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));

    return rv;
}

/*
 *  Function : drv_harrier_storm_control_enable_get
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
drv_harrier_storm_control_enable_get(int unit, uint32 port, uint8 *enable)
{
    uint32  retry, temp;
    uint32  zero_value, reg_value, acc_ctrl = 0;
    int  rv = SOC_E_NONE;

    /* Process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));

    /* Clear Rate Control Memory Data access registers */
    /* Use RCM_DATA0 register set bucket 0 */
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto storm_enable_get_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_get_exit;
    }

    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_get_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);

    /* 1'b0 : Ingress Rate Control Memory access */
    temp = 0;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_get_exit;
    }

    /* Wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_enable_get_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_enable_get_exit;
    }

    /* Read Rate Control Memory DATA 0 register */
    if ((rv = REG_READ_RCM_DATA0r(unit, &reg_value)) < 0) {
        goto storm_enable_get_exit;
    }

    soc_RCM_DATA0r_field_get(unit, &reg_value,
        IE_RC_ENf, &temp);
    *enable = temp;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_harrier_storm_control_enable_get: \
                         unit = %d, port = %d, %sable\n"), unit, port, (*enable) ? "en" : "dis"));
    
 storm_enable_get_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));

    return rv;
}

/*
 *  Function : drv_harrier_storm_control_set
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
drv_harrier_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size)
{
    uint32  port;
    uint32  retry, temp;
    uint32  zero_value, reg_value, acc_ctrl = 0;
    uint32  disable_type = 0, burst_kbyte = 0;
    uint32  ref_u;
    uint32  quotient_64k, remainder_64k, quotient_1m, remainder_1m;
    int  rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_harrier_storm_control_set: \
                         unit = %d, bmp = 0x%x, type = 0x%x, limit = %dK\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), type, limit));

    if (((int)limit > HARRIER_RATE_METER_MAX) ||
        ((int)limit < HARRIER_RATE_METER_MIN)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "drv_harrier_storm_control_set : rate unsupported.\n")));
        return SOC_E_PARAM;
    }

    if (((int)burst_size > HARRIER_RATE_BURST_MAX) ||
        ((int)burst_size < HARRIER_RATE_BURST_MIN)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "drv_harrier_storm_control_set : burst size unsupported.\n")));
        return SOC_E_PARAM;
    }

    PBMP_ITER(bmp, port) {
        /* 
         * Check maximum supported rate limit of FE and GE ports, 
         * respectively.
         */
        if (SOC_PBMP_MEMBER(PBMP_GE_ALL(unit), port)) {
            if (limit > HARRIER_RATE_MAX_LIMIT_KBPS_GE) {
                return SOC_E_PARAM;
            }
        } else {
            if (limit > HARRIER_RATE_MAX_LIMIT_KBPS_FE) {
                return SOC_E_PARAM;
            }
        }

        if (limit == 0) {  /* Disable storm suppression type */
            SOC_IF_ERROR_RETURN(_drv_harrier_storm_control_type_enable_set
                (unit, port, type, FALSE));
            /* need diable ingress rate control ? */
        } else {
            /* Set storm suppression type */
            SOC_IF_ERROR_RETURN(_drv_harrier_storm_control_type_enable_set
                (unit, port, type, TRUE));

            disable_type = HARRIER_STORM_CONTROL_PKT_MASK;
            disable_type ^= type;
            if (disable_type) {
                SOC_IF_ERROR_RETURN(_drv_harrier_storm_control_type_enable_set
                    (unit, port, disable_type, FALSE));
            }

            /* Set bucket 1 refresh count */
            /* Process write action */
            MEM_LOCK(unit, INDEX(GEN_MEMORYm));
            
            /* Clear Rate Control Memory Data access registers */
            zero_value = 0;

            /* Use RCM_DATA1 register set bucket x */
            /*
             * Bucket 0 is default used as per port rate control.
             * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
             * If no bucket number assigned, use bucket 1 as default bucket.
             */
            if (type & DRV_STORM_CONTROL_BUCKET_2) {
                if ((rv = REG_WRITE_RCM_DATA2r(unit, &zero_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            } else {
                if ((rv = REG_WRITE_RCM_DATA1r(unit, &zero_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            }

            /* Set Rate Control Memory Port register */
            soc_RCM_PORTr_field_set(unit, &acc_ctrl,
                RCM_PORTf, &port);
            if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
                goto storm_contol_set_exit;
            }

            /* Read Rate Control Memory register */
            if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                goto storm_contol_set_exit;
            }

            temp = MEM_TABLE_READ;
            soc_RCM_CTLr_field_set(unit, &acc_ctrl,
                RCM_RWf, &temp);
                
            /* 1'b0 : Ingress Rate Control Memory access */
            temp = 0;
            soc_RCM_CTLr_field_set(unit, &acc_ctrl,
                IE_INDf, &temp);

            /* Start Read Process */
            temp = 1;
            soc_RCM_CTLr_field_set(unit, &acc_ctrl,
                RCM_RW_STRTDNf, &temp);
            if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                goto storm_contol_set_exit;
            }

            /* Wait for complete */
            for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
                if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                    goto storm_contol_set_exit;
                }
                soc_RCM_CTLr_field_get(unit, &acc_ctrl,
                    RCM_RW_STRTDNf, &temp);
                if (!temp) {
                    break;
                }
            }
            if (retry >= SOC_TIMEOUT_VAL) {
                rv = SOC_E_TIMEOUT;
                goto storm_contol_set_exit;
            }

            /* Read Rate Control Memory DATA x register */
            if (type & DRV_STORM_CONTROL_BUCKET_2) {
                if ((rv = REG_READ_RCM_DATA2r(unit, &reg_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            } else {
                if ((rv = REG_READ_RCM_DATA1r(unit, &reg_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            }

            /* Burst size : set maximum burst size for storm contol at bucket x */
            /* Need to set burst size if open ingress pkt mask for each bucket (0 ~2) */
            if (burst_size) {
                /* Burst size */
                burst_kbyte = (burst_size / 8);
                temp = (burst_kbyte / 8);
            } else {
                temp = HARRIER_RATE_MAX_BUCKET_SIZE;
            }
            soc_RCM_DATA1r_field_set(unit, &reg_value,
                BUCKET_SIZEf, &temp);
            
            /* Refresh count by refresh unit */
            /* Can be divided by 1000K with no remainder? */
            quotient_1m = (limit / 1000);
            remainder_1m = limit - (quotient_1m * 1000);
            
            /* Can be divided by 62.5K with no remainder? */
            quotient_64k = ((limit * 10) / 625);
            remainder_64k = (limit * 10) - (quotient_64k * 625);
            if (remainder_1m == 0) {
                ref_u = HARRIER_RATE_REF_UNIT_1M;
                temp = quotient_1m;
            } else if (remainder_64k == 0) {
                ref_u = HARRIER_RATE_REF_UNIT_64K;
                temp = quotient_64k;
            } else {
                /* Others */
                if (limit <= (1024000)) {  /* (2^14 * 62.5) */
                    /* Use 62.5K as unit */
                    ref_u = HARRIER_RATE_REF_UNIT_64K;
                    temp = quotient_64k;
                } else {
                    /* 62.5K unit can't represent, so use 1M as unit */
                    ref_u = HARRIER_RATE_REF_UNIT_1M;
                    temp = quotient_1m;
                }
            }
            soc_RCM_DATA1r_field_set(unit, &reg_value,
                REF_UNITf, &ref_u);
            soc_RCM_DATA1r_field_set(unit, &reg_value,
                REF_CNTSf, &temp);

            if (type & DRV_STORM_CONTROL_BUCKET_2) {
                if ((rv = REG_WRITE_RCM_DATA2r(unit, &reg_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            } else {
                if ((rv = REG_WRITE_RCM_DATA1r(unit, &reg_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            }

            /* Write to Rate Control Memory */
            /* Read Rate Control Memory register */
            if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                goto storm_contol_set_exit;
            }

            temp = MEM_TABLE_WRITE;
            soc_RCM_CTLr_field_set(unit, &acc_ctrl,
                RCM_RWf, &temp);

            /* 1'b0 : Ingress Rate Control Memory access */
            temp = 0;
            soc_RCM_CTLr_field_set(unit, &acc_ctrl,
                IE_INDf, &temp);

            /* Start Write Process */
            temp = 1;
            soc_RCM_CTLr_field_set(unit, &acc_ctrl,
                RCM_RW_STRTDNf, &temp);
            if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                goto storm_contol_set_exit;
            }

            /* Wait for complete */
            for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
                if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                    goto storm_contol_set_exit;
                }
                soc_RCM_CTLr_field_get(unit, &acc_ctrl,
                    RCM_RW_STRTDNf, &temp);
                if (!temp) {
                    break;
                }
            }
            if (retry >= SOC_TIMEOUT_VAL) {
                rv = SOC_E_TIMEOUT;
                goto storm_contol_set_exit;
            }

 storm_contol_set_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
        }
    }

    return rv;
}

/*
 *  Function : drv_harrier_storm_control_get
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
drv_harrier_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size)
{
    uint32  retry, temp;
    uint32  zero_value, reg_value, acc_ctrl = 0;
    uint32  ref_u = 0;
    uint32  loc_type = 0;
    int  rv = SOC_E_NONE;

    SOC_IF_ERROR_RETURN(_drv_harrier_storm_control_type_enable_get
        (unit, port, type));

    loc_type = (*type & HARRIER_STORM_CONTROL_PKT_MASK);
    if (!loc_type) {
        *limit = 0;
    } else {
        /* Process write action */
        MEM_LOCK(unit, INDEX(GEN_MEMORYm));

        /* Clear Rate Control Memory Data access registers */
        zero_value = 0;

        /* Use RCM_DATA1 register set bucket x */
        /*
         * Bucket 0 is default used as per port rate control.
         * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
         * If no bucket number assigned, use bucket 1 as default bucket.
         */
        if (*type & DRV_STORM_CONTROL_BUCKET_2) {
            if ((rv = REG_WRITE_RCM_DATA2r(unit, &zero_value)) < 0) {
                goto storm_contol_get_exit;
            }
        } else {
            if ((rv = REG_WRITE_RCM_DATA1r(unit, &zero_value)) < 0) {
                goto storm_contol_get_exit;
            }
        }

        /* Set Rate Control Memory Port register */
        soc_RCM_PORTr_field_set(unit, &acc_ctrl,
            RCM_PORTf, &port);
        if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
            goto storm_contol_get_exit;
        }

        /* Read Rate Control Memory register */
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_contol_get_exit;
        }

        temp = MEM_TABLE_READ;
        soc_RCM_CTLr_field_set(unit, &acc_ctrl,
            RCM_RWf, &temp);

        /* 1'b0 : Ingress Rate Control Memory access */
        temp = 0;
        soc_RCM_CTLr_field_set(unit, &acc_ctrl,
            IE_INDf, &temp);
        
        /* Start Read Process */
        temp = 1;
        soc_RCM_CTLr_field_set(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_contol_get_exit;
        }

        /* Wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                goto storm_contol_get_exit;
            }
            soc_RCM_CTLr_field_get(unit, &acc_ctrl,
                RCM_RW_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto storm_contol_get_exit;
        }

        /* Read Rate Control Memory DATA x register */
        if (*type & DRV_STORM_CONTROL_BUCKET_2) {
            if ((rv = REG_READ_RCM_DATA2r(unit, &reg_value)) < 0) {
                goto storm_contol_get_exit;
            }
        } else {
            if ((rv = REG_READ_RCM_DATA1r(unit, &reg_value)) < 0) {
                goto storm_contol_get_exit;
            }
        }

        soc_RCM_DATA1r_field_get(unit, &reg_value,
            REF_UNITf, &ref_u);
        soc_RCM_DATA1r_field_get(unit, &reg_value,
            REF_CNTSf, &temp);
        if (ref_u == HARRIER_RATE_REF_UNIT_64K) {
            temp *= 625;
            *limit = (temp / 10);
        } else {
            *limit = (temp * 1000);
        }

        soc_RCM_DATA1r_field_get(unit, &reg_value,
            BUCKET_SIZEf, &temp);
        *burst_size = (temp * 8 * 8);
        
 storm_contol_get_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_harrier_storm_control_get: \
                         unit = %d, port = %d, type = 0x%x, limit = %dK\n"),
              unit, port, *type, *limit));

    return rv;
}
