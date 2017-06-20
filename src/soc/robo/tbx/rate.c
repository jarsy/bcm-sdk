/*
 * $Id: rate.c,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/mem.h>
#include "robo_tbx.h"

static int tbx_ingress_rate_init_flag[SOC_MAX_NUM_DEVICES] = {0};

/*
 *  Function : _drv_tbx_port_irc_set
 *
 *  Purpose :
 *      Set the burst size and rate limit value of the selected ingress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      limit       :  rate limit value (Kbits : kilobits (1000 bits) per second).
 *      burst_size  :  max burst size (Kbits : kilobits (1000 bits)).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0 (storm control use bucket1). 
 *
 */
static int
_drv_tbx_port_irc_set(int unit, uint32 port, uint32 limit,
    uint32 burst_size)
{
    irc_port_entry_t  irc_entry;
    uint32  index, field_val32;
    uint32  burst_kbyte = 0;
    soc_pbmp_t  pbmp;
    int  rv = SOC_E_NONE;

    sal_memset(&irc_entry, 0, sizeof(irc_entry));

    /*
      * COVERITY
      *
      * Comparing unsigned less than zero is never true.
      * It is kept intentionally as a defensive check.
      */
    /* coverity[unsigned_compare] */
    if ((limit > TB_RATE_METER_MAX(unit)) ||
        (limit < TB_RATE_METER_MIN(unit))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "_drv_tbx_port_irc_set : rate unsupported.\n")));
        return SOC_E_PARAM;
    }

    /*
      * COVERITY
      *
      * Comparing unsigned less than zero is never true.
      * It is kept intentionally as a defensive check.
      */
    /* coverity[unsigned_compare] */
    if ((burst_size > TB_RATE_BURST_MAX(unit)) ||
        (burst_size < TB_RATE_BURST_MIN(unit))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "_drv_tbx_port_irc_set : burst size unsupported.\n")));
        return SOC_E_PARAM;
    }

    /* 
     * Check maximum supported rate limit of FE and GE ports, 
     * respectively.
     */
    if (IS_GE_PORT(unit, port)) {
        if (IS_S_PORT(unit, port)) {
            if (limit > TB_RATE_MAX_LIMIT_KBPS_GE_S) {
                return SOC_E_PARAM;
            }
        } else {
            if (limit > TB_RATE_MAX_LIMIT_KBPS_GE) {
                return SOC_E_PARAM;
            }
        }
    } else {
        if (limit > TB_RATE_MAX_LIMIT_KBPS_FE) {
            return SOC_E_PARAM;
        }
    }

    /* Check if global ingress rate config is set */ 
    if (limit != 0) {
        if (!tbx_ingress_rate_init_flag[unit]) {
            pbmp = PBMP_ALL(unit);
            SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET
                (unit, pbmp, DRV_RATE_CONFIG_PKT_MASK, 
                TB_RATE_IRC_PKT_MASK(unit)));
            tbx_ingress_rate_init_flag[unit] = 1;
        }
    }

    /* Read Ingress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
    SOC_IF_ERROR_RETURN(rv);

    if (limit == 0) {  /* Disable ingress rate control */
        /* Disable ingress rate control 
          *    - set BKT0_IRC_ENf as 0, it will stop this port's bucket 0 
          *       ingress rate control.
          */
        field_val32 = 0;
        rv = soc_IRC_PORTm_field_set(unit, (uint32 *)&irc_entry, 
            BKT0_IRC_ENf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);
    } else {  /* Enable ingress rate control */
        /* Burst size */
        burst_kbyte = (burst_size / 8);
        field_val32 = ((burst_kbyte * 1000) / TB_RATE_BUCKET_UNIT_SIZE(unit));

        /* Check ingress rate control minimum available bucket size */
        if (field_val32 < TB_RATE_IRC_MIN_AVAIL_BUCKET_SIZE(unit)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Bucket size < IRC Minimum Available Bucket Size.\n")));
            return SOC_E_PARAM;
        }

        rv = soc_IRC_PORTm_field_set(unit, (uint32 *)&irc_entry, 
            BKT0_BKT_SIZEf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);

        field_val32 = (limit / TB_RATE_REF_COUNT_GRANULARITY(unit));
        rv = soc_IRC_PORTm_field_set(unit, (uint32 *)&irc_entry, 
            BKT0_REF_CNTf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);

        /* Enable ingress rate control */
        field_val32 = 1;
        rv = soc_IRC_PORTm_field_set(unit, (uint32 *)&irc_entry, 
            BKT0_IRC_ENf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);
    }

    /* Write to Ingress Rate Control Memory */
    rv = MEM_WRITE_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
    SOC_IF_ERROR_RETURN(rv);

    return rv;
}

/*
 *  Function : _drv_tbx_port_irc_get
 *
 *  Purpose :
 *      Get the burst size and rate limit value of the selected ingress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      limit       :  (OUT) rate limit value (Kbits : kilobits (1000 bits) per second).
 *      burst_size  :  (OUT) max burst size (Kbits : kilobits (1000 bits)).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0 (storm control use bucket1). 
 *
 */
static int
_drv_tbx_port_irc_get(uint32 unit, uint32 port, uint32 *limit, 
    uint32 *burst_size)
{
    irc_port_entry_t  irc_entry;
    uint32  index, field_val32;
    int  rv = SOC_E_NONE;

    sal_memset(&irc_entry, 0, sizeof(irc_entry));

    /* Check global ingress rate control setting */
    if (tbx_ingress_rate_init_flag[unit] != 0) {
        field_val32 = 0;
        SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
            (unit, port, DRV_RATE_CONFIG_PKT_MASK, &field_val32));

        /* If robo_ingress_rate_init_flag=0, only next ingress rate setting may 
         * set the properly PKT_MASK0 again currenly.
         */ 
        tbx_ingress_rate_init_flag[unit] = (field_val32 == 0) ? 0 : 1;
    }

    /* Check ingress rate control  
      *    - if BKT0_IRC_ENf is 0, it means that there is no limit rate on bucket 0,
      *       and return limit = 0 and burst_size = 0.
      *    - set the BKT0_REF_CNT to the MAX value means packets could 
      *       be forwarded by no limit rate. (set to 0 will block all this 
      *       port's traffic)
      */

    /* Read Ingress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
    SOC_IF_ERROR_RETURN(rv);

    rv = soc_IRC_PORTm_field_get(unit, (uint32 *)&irc_entry, 
        BKT0_IRC_ENf, &field_val32);
    SOC_IF_ERROR_RETURN(rv);

    if (field_val32 == 0) {
        *limit = 0;
        *burst_size = 0;
    } else {
        rv = soc_IRC_PORTm_field_get(unit, (uint32 *)&irc_entry, 
            BKT0_REF_CNTf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);
        *limit = (field_val32 * TB_RATE_REF_COUNT_GRANULARITY(unit));

        rv = soc_IRC_PORTm_field_get(unit, (uint32 *)&irc_entry, 
            BKT0_BKT_SIZEf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);
        *burst_size = (field_val32 * TB_RATE_BUCKET_UNIT_SIZE(unit) * 8) / 1000;
    }

    return rv;
}

/*
 *  Function : _drv_tbx_port_erc_check
 *
 *  Purpose :
 *      Software protection : Max rate of total bucket >= Min rate of all queues.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      ref_cnt     :  refresh count.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
static int
_drv_tbx_port_erc_check(uint32 unit, uint32 port, uint32 ref_cnt)
{
    erc_port_entry_t  erc_entry;
    uint32  index, field_val32 = 0, valid = 0;
    uint32  bucket_qn_ref_cnt_total = 0, bucket_t_ref_cnt_max = 0;
    uint32  bucket_qn_ref_cnt_min = 0, bucket_qn_erc_en = 0;
    int  i = 0;
    int  rv = SOC_E_NONE;

    sal_memset(&erc_entry, 0, sizeof(erc_entry));

    /* Read Egress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
    SOC_IF_ERROR_RETURN(rv);

    for (i = 0; i < NUM_COS(unit); i++) {
        switch (i) {
            case 0:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q0_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q0_ERC_Q_ENf);
                break;
            case 1:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q1_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q1_ERC_Q_ENf);
                break;
            case 2:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q2_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q2_ERC_Q_ENf);
                break;
            case 3:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q3_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q3_ERC_Q_ENf);
                break;
            case 4:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q4_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q4_ERC_Q_ENf);
                break;
            case 5:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q5_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q5_ERC_Q_ENf);
                break;
            case 6:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q6_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q6_ERC_Q_ENf);;
                break;
            case 7:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q7_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q7_ERC_Q_ENf);
                break;
            default:
                rv = SOC_E_PARAM;
        }
        rv = DRV_MEM_FIELD_GET(unit, INDEX(ERC_PORTm), 
            bucket_qn_erc_en, (uint32 *)&erc_entry, &valid);
        SOC_IF_ERROR_RETURN(rv);    

        if (valid != 0) {
            rv = DRV_MEM_FIELD_GET(unit, INDEX(ERC_PORTm), 
                bucket_qn_ref_cnt_min, (uint32 *)&erc_entry, &field_val32);
            SOC_IF_ERROR_RETURN(rv);    
        } else {
            field_val32 = 0;
        }
        bucket_qn_ref_cnt_total += field_val32;
    }

    bucket_t_ref_cnt_max = ref_cnt;

    if (bucket_t_ref_cnt_max < bucket_qn_ref_cnt_total) {
        rv = SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : _drv_tbx_port_erc_set
 *
 *  Purpose :
 *      Set the burst size and rate limit value of the selected egress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      limit       :  rate limit value (Kbits : kilobits (1000 bits) per second).
 *      burst_size  :  max burst size (Kbits : kilobits (1000 bits)).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
static int
_drv_tbx_port_erc_set(uint32 unit, uint32 port, uint32 limit, 
    uint32 burst_size)
{
    erc_port_entry_t  erc_entry;
    uint32  index, field_val32, ref_cnt;
    uint32  rate_type = 0, burst_kbyte = 0;
    int  rv = SOC_E_NONE;

    sal_memset(&erc_entry, 0, sizeof(erc_entry));

    /* Limit rate for Total */
    SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
        (unit, port, DRV_RATE_CONFIG_RATE_TYPE, &rate_type));
    if (rate_type == TB_RATE_TYPE_PPS) {
        /*
          * COVERITY
          *
          * Comparing unsigned less than zero is never true.
          * It is kept intentionally as a defensive check.
          */
        /* coverity[unsigned_compare] */    
        if ((limit > TB_RATE_METER_PKT_MAX(unit)) ||
            (limit < TB_RATE_METER_MIN(unit))) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("_drv_tbx_port_erc_set : rate unsupported.\n")));
            return  SOC_E_PARAM;
        }        

        /* 
         * Check maximum supported rate limit (pps) of FE and GE ports, 
         * respectively.
         */
        if (IS_GE_PORT(unit, port)) {
            if (IS_S_PORT(unit, port)) {
                if (limit > TB_RATE_MAX_LIMIT_PPS_GE_S) {
                    return SOC_E_PARAM;
                }
            } else {
                if (limit > TB_RATE_MAX_LIMIT_PPS_GE) {
                    return SOC_E_PARAM;
                }
            }
        } else {
            if (limit > TB_RATE_MAX_LIMIT_PPS_FE) {
                return SOC_E_PARAM;
            }
        }
    } else {
        /*
          * COVERITY
          *
          * Comparing unsigned less than zero is never true.
          * It is kept intentionally as a defensive check.
          */
        /* coverity[unsigned_compare] */    
        if ((limit > TB_RATE_METER_MAX(unit)) ||
            (limit < TB_RATE_METER_MIN(unit))) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("_drv_tbx_port_erc_set : rate unsupported.\n")));
            return  SOC_E_PARAM;
        }
        /* 
         * Check maximum supported rate limit (kbps) of FE and GE ports, 
         * respectively.
         */
        if (IS_GE_PORT(unit, port)) {
            if (IS_S_PORT(unit, port)) {
                if (limit > TB_RATE_MAX_LIMIT_KBPS_GE_S) {
                    return SOC_E_PARAM;
                }
            } else {
                if (limit > TB_RATE_MAX_LIMIT_KBPS_GE) {
                    return SOC_E_PARAM;
                }
            }
        } else {
            if (limit > TB_RATE_MAX_LIMIT_KBPS_FE) {
                return SOC_E_PARAM;
            }
        }
    }
    /*
      * COVERITY
      *
      * Comparing unsigned less than zero is never true.
      * It is kept intentionally as a defensive check.
      */
    /* coverity[unsigned_compare] */
    if ((burst_size > TB_RATE_BURST_MAX(unit)) ||
        (burst_size < TB_RATE_BURST_MIN(unit))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("_drv_tbx_port_erc_set : burst size unsupported.\n")));
        return SOC_E_PARAM;
    }

    /* Read Egress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
    SOC_IF_ERROR_RETURN(rv);

    if (limit == 0) {  /* Disable egress rate control for Total */
        field_val32 = 0;
        rv = soc_ERC_PORTm_field_set(unit, (uint32 *)&erc_entry, 
            BKT_T_ERC_T_ENf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);    
    } else {  /* Enable egress rate control for Total */
        if (rate_type == TB_RATE_TYPE_PPS) {
            ref_cnt = (limit / TB_RATE_REF_COUNT_PKT_GRANULARITY(unit));
        } else {
            ref_cnt = (limit / TB_RATE_REF_COUNT_GRANULARITY(unit));
        }

        /* Software protection : Max rate of total bucket > Min rate of all queues. */
        rv = _drv_tbx_port_erc_check(unit, port, ref_cnt);
        if (rv < 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON, \
                      (BSL_META("_drv_tbx_port_erc_set : \
                                Max rate of total < Min rate of all queues.\n")));
            return SOC_E_PARAM;
        }

        /* Burst size for Total */
        burst_kbyte = (burst_size / 8);
        field_val32 = ((burst_kbyte * 1000) / TB_RATE_BUCKET_UNIT_SIZE(unit));

        /* Check egress rate control minimum available bucket size */
        /*
          * COVERITY
          *
          * Comparing unsigned less than zero is never true.
          * It is kept intentionally as a defensive check.
          */
        /* coverity[unsigned_compare] */
        if (field_val32 < TB_RATE_ERC_MIN_AVAIL_BUCKET_SIZE(unit)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("Bucket size < ERC Minimum Available Bucket Size.\n")));
            return SOC_E_PARAM;
        }

        rv = soc_ERC_PORTm_field_set(unit, (uint32 *)&erc_entry, 
            BKT_T_BKT_SIZEf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);    

        rv = soc_ERC_PORTm_field_set(unit, (uint32 *)&erc_entry, 
            BKT_T_REF_CNT_MAXf, &ref_cnt);
        SOC_IF_ERROR_RETURN(rv);    

        /* Enable egress rate control for Total */
        field_val32 = 1;
        rv = soc_ERC_PORTm_field_set(unit, (uint32 *)&erc_entry, 
            BKT_T_ERC_T_ENf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);    
    }

    /* Write to Egress Rate Control memory table */
    rv = MEM_WRITE_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
    SOC_IF_ERROR_RETURN(rv);    

    return rv;
}

/*
 *  Function : _drv_tbx_port_erc_get
 *
 *  Purpose :
 *      Get the burst size and rate limit value of the selected egress port.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      limit       :  (OUT) rate limit value (Kbits : kilobits (1000 bits) per second).
 *      burst_size  :  (OUT) max burst size (Kbits : kilobits (1000 bits)).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
static int
_drv_tbx_port_erc_get(uint32 unit, uint32 port, uint32 *limit, 
    uint32 *burst_size)
{
    erc_port_entry_t  erc_entry;
    uint32  index, field_val32, rate_type = 0;
    int  rv = SOC_E_NONE;

    sal_memset(&erc_entry, 0, sizeof(erc_entry));

    /* Read Egress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
    SOC_IF_ERROR_RETURN(rv);

    rv = soc_ERC_PORTm_field_get(unit, (uint32 *)&erc_entry, 
        BKT_T_ERC_T_ENf, &field_val32);
    SOC_IF_ERROR_RETURN(rv);

    if (field_val32 == 0) {
        *limit = 0;
        *burst_size = 0;
    } else {
        rv = soc_ERC_PORTm_field_get(unit, (uint32 *)&erc_entry, 
            BKT_T_REF_CNT_MAXf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);

        SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
            (unit, port, DRV_RATE_CONFIG_RATE_TYPE, &rate_type));
        if (rate_type == TB_RATE_TYPE_KBPS) {
            /* kbps */
            *limit = (field_val32 * TB_RATE_REF_COUNT_GRANULARITY(unit));
        } else {
            /* pps */
            *limit = (field_val32 * TB_RATE_REF_COUNT_PKT_GRANULARITY(unit));
        }
        rv = soc_ERC_PORTm_field_get(unit, (uint32 *)&erc_entry, 
            BKT_T_BKT_SIZEf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);

        *burst_size = (field_val32 * TB_RATE_BUCKET_UNIT_SIZE(unit) * 8) / 1000; 
    }

    return rv;
}

/*
 *  Function : _drv_tbx_port_queue_erc_check
 *
 *  Purpose :
 *      Software protection : Min rate of all queues <= Max rate of total bucket.
 *
 *  Parameters :
 *      unit        :  unit id.
 *      port        :  port id.
 *      queue_n     :  COSQ id.
 *      ref_cnt     :  refresh count.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
static int
_drv_tbx_port_queue_erc_check(uint32 unit, uint32 port, uint8 queue_n,
    uint32 ref_cnt)
{
    erc_port_entry_t  erc_entry;
    uint32  index, field_val32 = 0, valid = 0;
    uint32  bucket_qn_ref_cnt_total = 0, bucket_t_ref_cnt_max = 0;
    uint32  bucket_qn_ref_cnt_min = 0, bucket_qn_erc_en = 0;
    int  i = 0;
    int  rv = SOC_E_NONE;

    sal_memset(&erc_entry, 0, sizeof(erc_entry));

    /* Read Egress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
    SOC_IF_ERROR_RETURN(rv);

    for (i = 0; i < NUM_COS(unit); i++) {
        switch (i) {
            case 0:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q0_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q0_ERC_Q_ENf);
                break;
            case 1:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q1_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q1_ERC_Q_ENf);;
                break;
            case 2:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q2_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q2_ERC_Q_ENf);;
                break;
            case 3:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q3_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q3_ERC_Q_ENf);;
                break;
            case 4:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q4_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q4_ERC_Q_ENf);;
                break;
            case 5:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q5_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q5_ERC_Q_ENf);;
                break;
            case 6:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q6_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q6_ERC_Q_ENf);;
                break;
            case 7:
                bucket_qn_ref_cnt_min = INDEX(BKT_Q7_REF_CNT_MINf);
                bucket_qn_erc_en = INDEX(BKT_Q7_ERC_Q_ENf);
                break;
            default:
                rv = SOC_E_PARAM;
        }
        rv = DRV_MEM_FIELD_GET(unit, INDEX(ERC_PORTm), bucket_qn_erc_en, 
            (uint32 *)&erc_entry, &valid);
        SOC_IF_ERROR_RETURN(rv);

        if (valid) {
            rv = DRV_MEM_FIELD_GET(unit, INDEX(ERC_PORTm), bucket_qn_ref_cnt_min, 
                (uint32 *)&erc_entry, &field_val32);
            SOC_IF_ERROR_RETURN(rv);
        } else {
            field_val32 = 0;
        }

        if (i == (int)queue_n) {
            field_val32 = ref_cnt;
        }
        bucket_qn_ref_cnt_total += field_val32;
    }

    rv = soc_ERC_PORTm_field_get(unit, (uint32 *)&erc_entry, 
        BKT_T_REF_CNT_MAXf, &bucket_t_ref_cnt_max);
    SOC_IF_ERROR_RETURN(rv);

    if (bucket_qn_ref_cnt_total > bucket_t_ref_cnt_max) {
        rv = SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : _drv_tbx_port_queue_erc_set
 *
 *  Purpose :
 *     Set the per-queue burst size and rate limit value of the selected egress port.
 *
 *  Parameters :
 *      unit          :  unit id.
 *      port          :  port id.
 *      queue_n       :  COSQ id.
 *      kbits_sec_min :  minimum rate, kbits/sec (Kbits : kilobits (1000 bits)).
 *      kbits_sec_max :  maximum rate, kbits/sec (Kbits : kilobits (1000 bits)).
 *      burst_size    :  max burst size (Kbits : kilobits (1000 bits)).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
static int
_drv_tbx_port_queue_erc_set(uint32 unit, uint32 port, uint8 queue_n, 
    uint32 kbits_sec_min, uint32 kbits_sec_max, uint32 burst_size)
{
    erc_port_entry_t  erc_entry;
    uint32  field_val32, ref_cnt_min;
    uint32  bucket_qn_ref_cnt_min = 0, bucket_qn_ref_cnt_max = 0;
    uint32  bucket_qn_bkt_size = 0, bucket_qn_erc_en = 0;
    uint32  burst_kbyte = 0, rate_type = 0;
    int  rv = SOC_E_NONE;

    sal_memset(&erc_entry, 0, sizeof(erc_entry));

    if (burst_size == kbits_sec_max) {
         burst_size = TB_RATE_BURST_MAX(unit);
    } else {
        return SOC_E_PARAM;
    }

    /* Limit rate for Total */
    SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
        (unit, port, DRV_RATE_CONFIG_RATE_TYPE, &rate_type));
    if (rate_type == TB_RATE_TYPE_PPS) {
        /*
          * COVERITY
          *
          * Comparing unsigned less than zero is never true.
          * It is kept intentionally as a defensive check.
          */
        /* coverity[unsigned_compare] */
        if ((kbits_sec_min > TB_RATE_METER_PKT_MAX(unit)) ||
            (kbits_sec_min < TB_RATE_METER_MIN(unit))) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("_drv_tbx_port_queue_erc_set : rate unsupported.\n")));
            return  SOC_E_PARAM;
        }
        /*
          * COVERITY
          *
          * Comparing unsigned less than zero is never true.
          * It is kept intentionally as a defensive check.
          */
        /* coverity[unsigned_compare] */
        if ((kbits_sec_max > TB_RATE_METER_PKT_MAX(unit)) ||
            (kbits_sec_max < TB_RATE_METER_MIN(unit))) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("_drv_tbx_port_queue_erc_set : rate unsupported.\n")));
            return  SOC_E_PARAM;
        }

        /* 
         * Check maximum supported rate limit (pps) of FE and GE ports, 
         * respectively.
         */
        if (IS_GE_PORT(unit, port)) {
            if (IS_S_PORT(unit, port)) {
                if (kbits_sec_min > TB_RATE_MAX_LIMIT_PPS_GE_S || 
                    kbits_sec_max > TB_RATE_MAX_LIMIT_PPS_GE_S) {
                    return SOC_E_PARAM;
                }
            } else {
                if (kbits_sec_min > TB_RATE_MAX_LIMIT_PPS_GE || 
                    kbits_sec_max > TB_RATE_MAX_LIMIT_PPS_GE) {
                    return SOC_E_PARAM;
                }
            }
        } else {
            if (kbits_sec_min > TB_RATE_MAX_LIMIT_PPS_FE || 
                kbits_sec_max > TB_RATE_MAX_LIMIT_PPS_FE) {
                return SOC_E_PARAM;
            }
        }
    } else {
        /*
          * COVERITY
          *
          * Comparing unsigned less than zero is never true.
          * It is kept intentionally as a defensive check.
          */
        /* coverity[unsigned_compare] */
        if ((kbits_sec_min > TB_RATE_METER_MAX(unit)) ||
            (kbits_sec_min < TB_RATE_METER_MIN(unit))) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("_drv_tbx_port_queue_erc_set : rate unsupported.\n")));
            return  SOC_E_PARAM;
        }
        /*
          * COVERITY
          *
          * Comparing unsigned less than zero is never true.
          * It is kept intentionally as a defensive check.
          */
        /* coverity[unsigned_compare] */
        if ((kbits_sec_max > TB_RATE_METER_MAX(unit)) ||
            (kbits_sec_max < TB_RATE_METER_MIN(unit))) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("_drv_tbx_port_queue_erc_set : rate unsupported.\n")));
            return  SOC_E_PARAM;
        }
        /* 
         * Check maximum supported rate limit (kbps) of FE and GE ports, 
         * respectively.
         */
        if (IS_GE_PORT(unit, port)) {
            if (IS_S_PORT(unit, port)) {
                if (kbits_sec_min > TB_RATE_MAX_LIMIT_KBPS_GE_S || 
                    kbits_sec_max > TB_RATE_MAX_LIMIT_KBPS_GE_S) {
                    return SOC_E_PARAM;
                }
            } else {
                if (kbits_sec_min > TB_RATE_MAX_LIMIT_KBPS_GE || 
                    kbits_sec_max > TB_RATE_MAX_LIMIT_KBPS_GE) {
                    return SOC_E_PARAM;
                }
            }
        } else {
            if (kbits_sec_min > TB_RATE_MAX_LIMIT_KBPS_FE || 
                kbits_sec_max > TB_RATE_MAX_LIMIT_KBPS_FE) {
                return SOC_E_PARAM;
            }
        }
    }

    /*
      * COVERITY
      *
      * Comparing unsigned less than zero is never true.
      * It is kept intentionally as a defensive check.
      */
    /* coverity[unsigned_compare] */
    if ((burst_size > TB_RATE_BURST_MAX(unit)) ||
        (burst_size < TB_RATE_BURST_MIN(unit))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("_drv_tbx_port_queue_erc_set : burst size unsupported.\n")));
        return SOC_E_PARAM;
    }

    switch (queue_n) {
        case 0:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q0_REF_CNT_MINf);
            bucket_qn_ref_cnt_max = INDEX(BKT_Q0_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q0_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q0_ERC_Q_ENf);
            break;
        case 1:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q1_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q1_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q1_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q1_ERC_Q_ENf);;
            break;
        case 2:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q2_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q2_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q2_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q2_ERC_Q_ENf);;
            break;
        case 3:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q3_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q3_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q3_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q3_ERC_Q_ENf);;
            break;
        case 4:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q4_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q4_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q4_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q4_ERC_Q_ENf);;
            break;
        case 5:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q5_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q5_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q5_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q5_ERC_Q_ENf);;
            break;
        case 6:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q6_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q6_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q6_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q6_ERC_Q_ENf);;
            break;
        case 7:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q7_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q7_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q7_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q7_ERC_Q_ENf);
            break;
        default:
            rv = SOC_E_PARAM;
    }

    /* Read Egress Rate Control memory table by index (port) */
    rv = MEM_READ_ERC_PORTm(unit, port, (uint32 *)&erc_entry);
    SOC_IF_ERROR_RETURN(rv);

    if (kbits_sec_max == 0) {  /* Disable egress rate control for Q-n */
        field_val32 = 0;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, INDEX(ERC_PORTm), 
            bucket_qn_erc_en, (uint32 *)&erc_entry, &field_val32));
    } else {  /* Enable egress rate control for Q-n */
        /* kbits_sec_min */
        if (rate_type == TB_RATE_TYPE_PPS) {
            /* pps */
            ref_cnt_min = 
                (kbits_sec_min / TB_RATE_REF_COUNT_PKT_GRANULARITY(unit));
        } else {
            /* kbps*/
            ref_cnt_min = 
                (kbits_sec_min / TB_RATE_REF_COUNT_GRANULARITY(unit));
        }

        rv = soc_ERC_PORTm_field_get(unit, (uint32 *)&erc_entry, 
            BKT_T_ERC_T_ENf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);

        /* Software protection : Min rate of all queues < Max rate of total bucket. */
        if (field_val32) {
            rv = _drv_tbx_port_queue_erc_check
                (unit, port, queue_n, ref_cnt_min);
            if (rv < 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON, \
                          (BSL_META("_drv_tbx_port_queue_erc_set : \
                                    Min rate of all queues > Max rate of total.\n")));
                return SOC_E_PARAM;
            }
        }

        /* Burst size */
        burst_kbyte = (burst_size / 8);
        field_val32 = ((burst_kbyte * 1000) / TB_RATE_BUCKET_UNIT_SIZE(unit));
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, INDEX(ERC_PORTm), bucket_qn_bkt_size, 
            (uint32 *)&erc_entry, &field_val32));

        /* kbits_sec_min */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, INDEX(ERC_PORTm), bucket_qn_ref_cnt_min, 
            (uint32 *)&erc_entry, &ref_cnt_min));

        /* kbits_sec_max */
        if (rate_type == TB_RATE_TYPE_PPS) {
            /* pps */
            field_val32 = 
                (kbits_sec_max / TB_RATE_REF_COUNT_PKT_GRANULARITY(unit));
        } else {
            /* kbps*/
            field_val32 = 
                (kbits_sec_max / TB_RATE_REF_COUNT_GRANULARITY(unit));
        }          
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, INDEX(ERC_PORTm), bucket_qn_ref_cnt_max, 
            (uint32 *)&erc_entry, &field_val32));

        /* Enable egress rate control for Q-n */
        field_val32 = 1;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, INDEX(ERC_PORTm), bucket_qn_erc_en, 
            (uint32 *)&erc_entry, &field_val32));

    }

    /* Write to Egress Rate Control memory table */
    rv = MEM_WRITE_ERC_PORTm(unit, port, (uint32 *)&erc_entry);
    SOC_IF_ERROR_RETURN(rv);

    return rv;
}

/*
 *  Function : _drv_tbx_port_queue_erc_get
 *
 *  Purpose :
 *     Get the per-queue burst size and rate limit value of the selected egress port.
 *
 *  Parameters :
 *      unit          :  unit id.
 *      port          :  port id.
 *      queue_n       :  COSQ id.
 *      kbits_sec_min :  (OUT) minimum rate, kbits/sec (Kbits : kilobits (1000 bits)).
 *      kbits_sec_max :  (OUT) maximum rate, kbits/sec (Kbits : kilobits (1000 bits)).
 *      burst_size    :  (OUT) max burst size (Kbits : kilobits (1000 bits)).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
static int
_drv_tbx_port_queue_erc_get(uint32 unit, uint32 port, uint8 queue_n,
    uint32 *kbits_sec_min, uint32 *kbits_sec_max, uint32 *burst_size)
{
    erc_port_entry_t  erc_entry;
    uint32  index, field_val32;
    uint32  bucket_qn_ref_cnt_min = 0, bucket_qn_ref_cnt_max = 0;
    uint32  bucket_qn_bkt_size = 0, bucket_qn_erc_en = 0;
    uint32  rate_type = 0;
    int  rv = SOC_E_NONE;

    sal_memset(&erc_entry, 0, sizeof(erc_entry));

    /* Read Egress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
    SOC_IF_ERROR_RETURN(rv);

    switch (queue_n) {
        case 0:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q0_REF_CNT_MINf);
            bucket_qn_ref_cnt_max = INDEX(BKT_Q0_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q0_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q0_ERC_Q_ENf);
            break;
        case 1:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q1_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q1_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q1_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q1_ERC_Q_ENf);;
            break;
        case 2:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q2_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q2_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q2_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q2_ERC_Q_ENf);;
            break;
        case 3:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q3_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q3_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q3_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q3_ERC_Q_ENf);;
            break;
        case 4:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q4_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q4_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q4_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q4_ERC_Q_ENf);;
            break;
        case 5:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q5_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q5_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q5_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q5_ERC_Q_ENf);;
            break;
        case 6:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q6_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q6_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q6_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q6_ERC_Q_ENf);;
            break;
        case 7:
            bucket_qn_ref_cnt_min = INDEX(BKT_Q7_REF_CNT_MINf);
            bucket_qn_ref_cnt_max =  INDEX(BKT_Q7_REF_CNT_MAXf);
            bucket_qn_bkt_size = INDEX(BKT_Q7_BKT_SIZEf);
            bucket_qn_erc_en = INDEX(BKT_Q7_ERC_Q_ENf);
            break;
        default:
            rv = SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
        (unit, port, DRV_RATE_CONFIG_RATE_TYPE, &rate_type));
    /* 
     * Get Egress Rate control Enable bit for Per-Queue.
     * If it is in disabled state, return disabled value of rate control.
     */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
        (unit, INDEX(ERC_PORTm), bucket_qn_erc_en, 
        (uint32 *)&erc_entry, &field_val32));
    if (field_val32 == 0) {
        *kbits_sec_min = 0;
        *kbits_sec_max = 0;
        *burst_size = 0;
    } else {
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
            (unit, INDEX(ERC_PORTm), bucket_qn_ref_cnt_min, 
            (uint32 *)&erc_entry, &field_val32));
        if (rate_type == TB_RATE_TYPE_PPS) {
            *kbits_sec_min = 
                (field_val32 * TB_RATE_REF_COUNT_PKT_GRANULARITY(unit));
        } else {
            *kbits_sec_min = 
                (field_val32 * TB_RATE_REF_COUNT_GRANULARITY(unit));
        }
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
            (unit, INDEX(ERC_PORTm), bucket_qn_ref_cnt_max, 
            (uint32 *)&erc_entry, &field_val32));

        if (rate_type == TB_RATE_TYPE_PPS) {
            *kbits_sec_max = 
                (field_val32 * TB_RATE_REF_COUNT_PKT_GRANULARITY(unit));
        } else {
            *kbits_sec_max = 
                (field_val32 * TB_RATE_REF_COUNT_GRANULARITY(unit));
        }
        /* 
         * If the value of rate limit meets maximum,
         * return *limit=0 adn *burst_size=0 to represent the rate control is disable.
         */
        if (*kbits_sec_max == TB_RATE_METER_MAX(unit)) {
            *kbits_sec_min = 0;
            *kbits_sec_max = 0;
            *burst_size = 0;
            return rv;
        }

        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
            (unit, INDEX(ERC_PORTm), bucket_qn_bkt_size, 
            (uint32 *)&erc_entry, &field_val32));

        *burst_size = (field_val32 * TB_RATE_BUCKET_UNIT_SIZE(unit) * 8) / 1000; 
    }

    return rv;
}

/*
 *  Function : drv_tbx_rate_config_set
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
drv_tbx_rate_config_set(int unit, soc_pbmp_t pbmp, uint32 config_type, 
    uint32 value)
{
    irc_port_entry_t  irc_entry;
    erc_port_entry_t  erc_entry;
    uint32  index, field_val32;
    soc_port_t  port;
    int  rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_tbx_rate_config_set: \
                         unit = %d, bmp = 0x%x, type = %d, value = %d\n"),
              unit, SOC_PBMP_WORD_GET(pbmp, 0), config_type, value));

    PBMP_ITER(pbmp, port) {
        index = port;        
        switch (config_type) {
            case DRV_RATE_CONFIG_RATE_TYPE: 
                /* Read Egress Rate Control memory table by index (port) */
                sal_memset(&erc_entry, 0, sizeof(erc_entry));
                rv = MEM_READ_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
                SOC_IF_ERROR_RETURN(rv);

                /* Set Rate type bit for per-port Egress Rate Control */
                /* Rate type :
                  * 0: DRV_RATE_REF_COUNT_GRANULARITY(unit)
                  * 1: DRV_RATE_REF_COUNT_PKT_GRANULARITY(unit)
                  */
                field_val32 = (value) ? TB_RATE_TYPE_PPS : TB_RATE_TYPE_KBPS;
                rv = soc_ERC_PORTm_field_set(unit, (uint32 *)&erc_entry, 
                    BKT_T_TYPEf, &field_val32);
                SOC_IF_ERROR_RETURN(rv);

                /* Write to Engress Rate Control memory table */
                rv = MEM_WRITE_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
                SOC_IF_ERROR_RETURN(rv);
                break;
            case DRV_RATE_CONFIG_RATE_BAC: 
                /* Set BAC bit for per-port Egress Rate Control */
                /* Read Egress Rate Control memory table by index (port) */
                sal_memset(&erc_entry, 0, sizeof(erc_entry));
                rv = MEM_READ_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
                SOC_IF_ERROR_RETURN(rv);

                /* BAC (Burst Accumulate Control) :
                  *  0: To indicate the value of the Burst Tolerance Size should be reset to zero 
                  *      when there is no packet in the queue waiting to be transmitted.
                  *  1: Enable accumulation
                  */
                field_val32 = (value) ? 1 : 0;
                rv = soc_ERC_PORTm_field_set(unit, (uint32 *)&erc_entry, 
                    BKT_BACf, &field_val32);
                SOC_IF_ERROR_RETURN(rv);

                /* Write to Engress Rate Control memory table */
                rv = MEM_WRITE_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
                SOC_IF_ERROR_RETURN(rv);                
                break;
            case DRV_RATE_CONFIG_DROP_ENABLE: 
                /* Read Igress Rate Control memory table by index (port) */
                sal_memset(&irc_entry, 0, sizeof(irc_entry));
                rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
                SOC_IF_ERROR_RETURN(rv);

                /* Set DROP_EN bit for per-port Ingress Rate Control */
                field_val32 = (value) ? 1 : 0;
                rv = soc_IRC_PORTm_field_set(unit, (uint32 *)&irc_entry, 
                    EN_DROPf, &field_val32);
                SOC_IF_ERROR_RETURN(rv);

                /* Write to Ingress Rate Control memory table */
                rv = MEM_WRITE_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
                SOC_IF_ERROR_RETURN(rv);
                break;
            case DRV_RATE_CONFIG_PKT_MASK: 
                /* Read Igress Rate Control memory table by index (port) */
                sal_memset(&irc_entry, 0, sizeof(irc_entry));
                rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
                SOC_IF_ERROR_RETURN(rv);

                /* Set PKT_MASK on bucket 0 for per-port Ingress Rate Control */
                field_val32 = value;
                rv = soc_IRC_PORTm_field_set(unit, (uint32 *)&irc_entry, 
                    BKT0_PKT_MASKf, &field_val32);
                SOC_IF_ERROR_RETURN(rv);

                /* Write to Ingress Rate Control memory table */
                rv = MEM_WRITE_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
                SOC_IF_ERROR_RETURN(rv);
                break;
			case DRV_RATE_CONFIG_INGRESS_IPG_INCLUDE:
			case DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_OFF:
			case DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_ON:
				return SOC_E_UNAVAIL;
            default:
                return SOC_E_PARAM;
        }
    }

    return rv;
}

/*
 *  Function : drv_tbx_rate_config_get
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
drv_tbx_rate_config_get(int unit, uint32 port, uint32 config_type, 
    uint32 *value)
{
    irc_port_entry_t  irc_entry;
    erc_port_entry_t  erc_entry;
    uint32  index, field_val32;
    int  rv = SOC_E_NONE;

    index = port;            
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE: 
            sal_memset(&erc_entry, 0, sizeof(erc_entry));
            /* Get Rate type bit for per-port Egress Rate Control */
            rv = MEM_READ_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
            SOC_IF_ERROR_RETURN(rv);
            rv = soc_ERC_PORTm_field_get(unit, (uint32 *)&erc_entry, 
                BKT_T_TYPEf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);

            /* Rate type :
              *  0 : bit/sec 
              *  1 : pkt/sec
              */
            *value = (field_val32) ? TB_RATE_TYPE_PPS : TB_RATE_TYPE_KBPS;
            break;
        case DRV_RATE_CONFIG_RATE_BAC: 
            /* Get BAC bit for per-port Egress Rate Control */
            sal_memset(&erc_entry, 0, sizeof(erc_entry));
            rv = MEM_READ_ERC_PORTm(unit, index, (uint32 *)&erc_entry);
            SOC_IF_ERROR_RETURN(rv);
            rv = soc_ERC_PORTm_field_get(unit, (uint32 *)&erc_entry, 
                BKT_BACf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);

            /* BAC (Burst Accumulate Control) :
              *  0: To indicate the value of the Burst Tolerance Size should be reset to zero 
              *      when there is no packet in the queue waiting to be transmitted.
              *  1: Enable accumulation
              */
            *value = (field_val32) ? 1 : 0;
            break;
        case DRV_RATE_CONFIG_DROP_ENABLE: 
            /* Read Igress Rate Control memory table by index (port) */
            sal_memset(&irc_entry, 0, sizeof(irc_entry));
            rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
            SOC_IF_ERROR_RETURN(rv);

            /* Get DROP_EN bit for per-port Ingress Rate Control */
            rv = soc_IRC_PORTm_field_get(unit, (uint32 *)&irc_entry, 
                EN_DROPf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);
            *value = (field_val32) ? 1 : 0;
            break;
        case DRV_RATE_CONFIG_PKT_MASK: 
            /* Read Igress Rate Control memory table by index (port) */
            sal_memset(&irc_entry, 0, sizeof(irc_entry));
            rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
            SOC_IF_ERROR_RETURN(rv);

            /* Get PKT_MASK on bucket 0 for per-port Ingress Rate Control */
            rv = soc_IRC_PORTm_field_get(unit, (uint32 *)&irc_entry, 
                BKT0_PKT_MASKf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);
            *value = field_val32;
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
                         "drv_rate_config_get: \
                         unit = %d, port = %d, type = %d, value = %d\n"),
              unit, port, config_type, *value));

    return rv;
}

/*
 *  Function : drv_tbx_rate_set
 *
 *  Purpose :
 *      Set the ingress/egress rate control to the selected ports.
 *
 *  Parameters :
 *      unit          :  unit id.
 *      bmp           :  port bitmap.
 *      queue_n       :  COSQ id.
 *      direction     :  direction of rate control (ingress/egress). 
 *      kbits_sec_min :  minimum rate, kbits/sec (Kbits : kilobits (1000 bits)).
 *      kbits_sec_max :  maximum rate, kbits/sec (Kbits : kilobits (1000 bits)).
 *      burst_size    :  max burst size (Kbits : kilobits (1000 bits)).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int 
drv_tbx_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, int direction, 
    uint32 flags, uint32 kbits_sec_min, 
    uint32 kbits_sec_max, uint32 burst_size)
{
    uint32  port;
    int  rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_tbx_rate_set: unit = %d, bmp = 0x%x, %s, flags = 0x%x, \
                         kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n"), 
              unit, SOC_PBMP_WORD_GET(bmp, 0), (direction - 1) ? "EGRESS" : "INGRESS", 
              flags, kbits_sec_min, kbits_sec_max, burst_size));

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_tbx_port_irc_set
                    (unit, port, kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_tbx_port_erc_set
                    (unit, port, kbits_sec_max, burst_size));
            }                
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_tbx_port_queue_erc_set
                    (unit, port, queue_n, 
                    kbits_sec_min, kbits_sec_max, burst_size));
            }                
            break;
        default:
            return SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : drv_tbx_rate_get
 *
 *  Purpose :
 *      Get the ingress/egress rate control to the selected ports.
 *
 *  Parameters :
 *      unit          :  unit id.
 *      port          :  port id.
 *      queue_n       :  COSQ id.
 *      direction     :  direction of rate control (ingress/egress). 
 *      kbits_sec_min :  (OUT) minimum rate, kbits/sec (Kbits : kilobits (1000 bits)).
 *      kbits_sec_max :  (OUT) maximum rate, kbits/sec (Kbits : kilobits (1000 bits)).
 *      burst_size    :  (OUT) max burst size (Kbits : kilobits (1000 bits)).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int 
drv_tbx_rate_get(int unit, uint32 port, uint8 queue_n, int direction, 
    uint32 *flags, uint32 *kbits_sec_min, 
    uint32 *kbits_sec_max, uint32 *burst_size)
{
    int  rv = SOC_E_NONE;
    uint32  min_rate = 0;  /* Dummy variable */

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            SOC_IF_ERROR_RETURN(_drv_tbx_port_irc_get
                (unit, port, kbits_sec_max, burst_size));
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            SOC_IF_ERROR_RETURN(_drv_tbx_port_erc_get
                (unit, port, kbits_sec_max, burst_size));
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            SOC_IF_ERROR_RETURN(_drv_tbx_port_queue_erc_get
                (unit, port, queue_n, 
                kbits_sec_min, kbits_sec_max, burst_size));
            min_rate = *kbits_sec_min;
            break;
        default:
            return SOC_E_PARAM;
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_tbx_rate_get: unit = %d, port = %d, %s, flags = 0x%x, \
                         kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n"), 
              unit, port, (direction - 1) ? "EGRESS" : "INGRESS", 
              *flags, min_rate, *kbits_sec_max, *burst_size));

    return rv;
}
