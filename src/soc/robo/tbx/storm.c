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
#include "robo_tbx.h"

/*
 *  Function : _drv_tbx_storm_control_type_enable_set
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
_drv_tbx_storm_control_type_enable_set(int unit, uint32 port, 
    uint32 type, uint32 enable)
{
    irc_port_entry_t  irc_entry;
    uint32  index, field_val32;
    int  rv = SOC_E_NONE;

    sal_memset(&irc_entry, 0, sizeof(irc_entry));

    /* Read Ingress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
    SOC_IF_ERROR_RETURN(rv);

    /*
     * Bucket 0 is default used as per port rate control.
     * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
     * If no bucket number assigned, use bucket 1 as default bucket.
     */
    if (type & DRV_STORM_CONTROL_BUCKET_2) {
        rv = DRV_MEM_FIELD_GET(unit, INDEX(IRC_PORTm), 
            INDEX(BKT2_PKT_MASKf), (uint32 *)&irc_entry, &field_val32);
    } else {
        rv = DRV_MEM_FIELD_GET(unit, INDEX(IRC_PORTm), 
            INDEX(BKT1_PKT_MASKf), (uint32 *)&irc_entry, &field_val32);
    }
    SOC_IF_ERROR_RETURN(rv);

    if (type & DRV_STORM_CONTROL_BCAST) {
        if (enable) {
            field_val32 |= TBX_STORM_SUPPRESSION_BROADCAST_MASK;
        } else {
            field_val32 &= ~TBX_STORM_SUPPRESSION_BROADCAST_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_MCAST) {
        if (enable) {
            field_val32 |= (TBX_STORM_SUPPRESSION_MULTICAST_MASK |
                           TBX_STORM_SUPPRESSION_MLF_MASK);
        } else {
            field_val32 &= ~(TBX_STORM_SUPPRESSION_MULTICAST_MASK |
                             TBX_STORM_SUPPRESSION_MLF_MASK);
        }
    } 
    if (type & DRV_STORM_CONTROL_DLF) {
        if (enable) {
            field_val32 |= TBX_STORM_SUPPRESSION_DLF_MASK;
        } else {
            field_val32 &= ~TBX_STORM_SUPPRESSION_DLF_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_RSV_MCAST) {
        if (enable) {
            field_val32 |= TBX_STORM_SUPPRESSION_BPDU_MASK;
        } else {
            field_val32 &= ~TBX_STORM_SUPPRESSION_BPDU_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_UCAST) {
        if (enable) {
            field_val32 |= TBX_STORM_SUPPRESSION_UNICAST_MASK;
        } else {
            field_val32 &= ~TBX_STORM_SUPPRESSION_UNICAST_MASK;
        }
    }

    if (type & DRV_STORM_CONTROL_BUCKET_2) {
        rv = DRV_MEM_FIELD_SET(unit, INDEX(IRC_PORTm), 
            INDEX(BKT2_PKT_MASKf), (uint32 *)&irc_entry, &field_val32);
    } else {
        rv = DRV_MEM_FIELD_SET(unit, INDEX(IRC_PORTm), 
            INDEX(BKT1_PKT_MASKf), (uint32 *)&irc_entry, &field_val32);
    }
    SOC_IF_ERROR_RETURN(rv);

    /* Write to Ingress Rate Control Memory table */
    rv = MEM_WRITE_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
    SOC_IF_ERROR_RETURN(rv);

    return rv;
}

/*
 *  Function : _drv_tbx_storm_control_type_enable_get
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
_drv_tbx_storm_control_type_enable_get(int unit, uint32 port, 
    uint32 *type)
{
    irc_port_entry_t  irc_entry;
    uint32  index, field_val32;
    int  rv = SOC_E_NONE;

    sal_memset(&irc_entry, 0, sizeof(irc_entry));

    /* Read Ingress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
    SOC_IF_ERROR_RETURN(rv);

    /*
     * Bucket 0 is default used as per port rate control.
     * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
     * If no bucket number assigned, use bucket 1 as default bucket.
     */
    if (*type & DRV_STORM_CONTROL_BUCKET_2) {
        rv = DRV_MEM_FIELD_GET(unit, INDEX(IRC_PORTm), 
            INDEX(BKT2_PKT_MASKf), (uint32 *)&irc_entry, &field_val32);
    } else {
        rv = DRV_MEM_FIELD_GET(unit, INDEX(IRC_PORTm), 
            INDEX(BKT1_PKT_MASKf), (uint32 *)&irc_entry, &field_val32);
    }
    SOC_IF_ERROR_RETURN(rv);

    if (field_val32 & TBX_STORM_SUPPRESSION_BROADCAST_MASK) {
        *type |= DRV_STORM_CONTROL_BCAST;
    }
    if (field_val32 & TBX_STORM_SUPPRESSION_MULTICAST_MASK ||
        field_val32 & TBX_STORM_SUPPRESSION_MLF_MASK) {
        *type |= DRV_STORM_CONTROL_MCAST;
    }
    if (field_val32 & TBX_STORM_SUPPRESSION_DLF_MASK) {
        *type |= DRV_STORM_CONTROL_DLF;
    }
    if (field_val32 & TBX_STORM_SUPPRESSION_BPDU_MASK) {
        *type |= DRV_STORM_CONTROL_RSV_MCAST;
    }
    if (field_val32 & TBX_STORM_SUPPRESSION_UNICAST_MASK) {
        *type |= DRV_STORM_CONTROL_UCAST;
    }

    return rv;
}

/*
 *  Function : drv_tbx_storm_control_enable_set
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
drv_tbx_storm_control_enable_set(int unit, uint32 port, uint8 enable)
{
    irc_port_entry_t  irc_entry;
    uint32  index, field_val32;
    int  rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_tbx_storm_control_enable_set: \
                         unit = %d, port = %d, %sable\n"), unit, port, (enable) ? "en" : "dis"));

    sal_memset(&irc_entry, 0, sizeof(irc_entry));

    /* Read Ingress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
    SOC_IF_ERROR_RETURN(rv);

    /* Enable/disable IRC_EN bit (per-bucket) on bucket 1 for storm control */
    field_val32 = (enable) ? 1 : 0;
    rv = soc_IRC_PORTm_field_set(unit, (uint32 *)&irc_entry, 
        BKT1_IRC_ENf, &field_val32);
    SOC_IF_ERROR_RETURN(rv);

    /* 
     * When IRC_EN bit is enabled on bucket 1 for storm control,
     * on going to set the rate limit and bucket size.
     * The rate limit and bucket size can't be 0.
     * Set to maxmimum value if no value in the fields.
     */
    if (enable != 0) {
        rv = soc_IRC_PORTm_field_get(unit, (uint32 *)&irc_entry, 
            BKT1_REF_CNTf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);

        if (!field_val32) {
            field_val32 = TB_RATE_MAX_REF_CNTS(unit);
            rv = soc_IRC_PORTm_field_set(unit, (uint32 *)&irc_entry, 
                BKT1_REF_CNTf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);
        }

        rv = soc_IRC_PORTm_field_get(unit, (uint32 *)&irc_entry, 
            BKT1_BKT_SIZEf, &field_val32);
        SOC_IF_ERROR_RETURN(rv);

        if (!field_val32) {
            field_val32 = TB_RATE_MAX_BUCKET_UNIT(unit);
            rv = soc_IRC_PORTm_field_set(unit, (uint32 *)&irc_entry, 
                BKT1_BKT_SIZEf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);

        }
    }

    /* Write to Ingress Rate Control Memory table */
    rv = MEM_WRITE_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
    SOC_IF_ERROR_RETURN(rv);

    return rv;
}

/*
 *  Function : drv_tbx_storm_control_enable_get
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
drv_tbx_storm_control_enable_get(int unit, uint32 port, uint8 *enable)
{
    irc_port_entry_t  irc_entry;
    uint32  index, field_val32;
    int  rv = SOC_E_NONE;

    sal_memset(&irc_entry, 0, sizeof(irc_entry));

    /* Read Ingress Rate Control memory table by index (port) */
    index = port;
    rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
    SOC_IF_ERROR_RETURN(rv);

    /* Get IRC_EN bit (per-bucket) on bucket 1 for storm control */
    rv = soc_IRC_PORTm_field_get(unit, (uint32 *)&irc_entry, 
        BKT1_IRC_ENf, &field_val32);
    SOC_IF_ERROR_RETURN(rv);
    *enable = field_val32;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_tbx_storm_control_enable_get: \
                         unit = %d, port = %d, %sable\n"), unit, port, (*enable) ? "en" : "dis"));

    return rv;
}

/*
 *  Function : drv_tbx_storm_control_set
 *
 *  Purpose :
 *      Set the types and limit value for storm control of selected port.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      bmp        :  port bitmap.
 *      type       :  types of strom control.
 *      limit      :  limit value of storm control (Kbits : kilobits (1000 bits)).
 *      burst_size :  burst bucket size (Kbits : kilobits (1000 bits)).
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
int 
drv_tbx_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size)
{
    irc_port_entry_t  irc_entry;
    uint32  index, field_val32, port;
    uint32  disable_type = 0, burst_kbyte = 0;
    uint32  bucket_field_bkt_size = 0, bucket_field_ref_cnt = 0;
    int  rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_tbx_storm_control_set: \
                         unit = %d, bmp = 0x%x, type = 0x%x, limit = %dK, burst = %dK\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), type, limit, burst_size));

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
                              "drv_tbx_storm_control_set : rate unsupported.\n")));
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
                              "drv_tbx_storm_control_set : burst size unsupported.\n")));
        return SOC_E_PARAM;
    }

    PBMP_ITER(bmp, port) {
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

        if (limit == 0) {  /* Disable storm suppression type */
            SOC_IF_ERROR_RETURN(_drv_tbx_storm_control_type_enable_set
                (unit, port, type, FALSE));
        } else {
            /* Set storm suppression type */
            SOC_IF_ERROR_RETURN(_drv_tbx_storm_control_type_enable_set
                (unit, port, type, TRUE));

            disable_type = TBX_STORM_CONTROL_PKT_MASK;
            disable_type ^= type;
            if (disable_type != 0) {
                SOC_IF_ERROR_RETURN(_drv_tbx_storm_control_type_enable_set
                    (unit, port, disable_type, FALSE));
            }

            /*
             * Bucket 0 is default used as per port rate control.
             * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
             * If no bucket number assigned, use bucket 1 as default bucket.
             */
            if (type & DRV_STORM_CONTROL_BUCKET_2) {
                bucket_field_ref_cnt = INDEX(BKT2_REF_CNTf);
                bucket_field_bkt_size = INDEX(BKT2_BKT_SIZEf);
            } else {
                bucket_field_ref_cnt = INDEX(BKT1_REF_CNTf);
                bucket_field_bkt_size = INDEX(BKT1_BKT_SIZEf);
            }

            sal_memset(&irc_entry, 0, sizeof(irc_entry));

            /* Read Ingress Rate Control memory table by index (port) */
            index = port;
            rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
            SOC_IF_ERROR_RETURN(rv);

            if (type & DRV_STORM_CONTROL_BUCKET_2) {
               /* Get IRC_EN bit (per-bucket) on bucket 2 for storm control */
                rv = soc_IRC_PORTm_field_get(unit, (uint32 *)&irc_entry,
                    BKT2_IRC_ENf,(uint32 *)&field_val32);
                SOC_IF_ERROR_RETURN(rv);

                if (field_val32 == 0) {
                    /* Enable IRC_EN bit (per-bucket) on bucket 2 for storm control */
                    rv = soc_IRC_PORTm_field_set(unit, (uint32 *)&irc_entry,
                        BKT2_IRC_ENf,(uint32 *)&field_val32);
                    SOC_IF_ERROR_RETURN(rv);
                }
            }

            /* Burst size : set maximum burst size for storm contol at bucket x */
            /* Need to set burst size if open ingress pkt mask for each bucket (0 ~2) */
            if (burst_size) {
                burst_kbyte = (burst_size / 8);
                field_val32 = 
                    ((burst_kbyte * 1000) / TB_RATE_BUCKET_UNIT_SIZE(unit));
            } else {
                field_val32 = TB_RATE_MAX_BUCKET_UNIT(unit);
            }
            rv = DRV_MEM_FIELD_SET(unit, INDEX(IRC_PORTm), 
                bucket_field_bkt_size, (uint32 *)&irc_entry, &field_val32);
            SOC_IF_ERROR_RETURN(rv);

            field_val32 = (limit / TB_RATE_REF_COUNT_GRANULARITY(unit));
            rv = DRV_MEM_FIELD_SET(unit, INDEX(IRC_PORTm), 
                bucket_field_ref_cnt, (uint32 *)&irc_entry, &field_val32);
            SOC_IF_ERROR_RETURN(rv);

            /* Write to Ingress Rate Control Memory table */
            rv = MEM_WRITE_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
            SOC_IF_ERROR_RETURN(rv);
        }
    }

    return rv;
}

/*
 *  Function : drv_tbx_storm_control_get
 *
 *  Purpose :
 *      Get the types and limit value for storm control of selected port.
 *
 *  Parameters :
 *      unit       :  RoboSwitch unit number.
 *      port       :  port number.
 *      type       :  (OUT) types of strom control.
 *      limit      :  (OUT) limit value of storm control (Kbits : kilobits (1000 bits)).
 *      burst_size :  (OUT) burst bucket size (Kbits : kilobits (1000 bits)).
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
int 
drv_tbx_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size)
{
    irc_port_entry_t  irc_entry;
    uint32  index, field_val32;
    uint32  loc_type = 0;
    uint32  bucket_field_bkt_size = 0, bucket_field_ref_cnt = 0;
    int  rv = SOC_E_NONE;

     SOC_IF_ERROR_RETURN(_drv_tbx_storm_control_type_enable_get
         (unit, port, type));

    loc_type = (*type & TBX_STORM_CONTROL_PKT_MASK);
    if (!loc_type) {
        *limit = 0;
    } else {
        /*
         * Bucket 0 is default used as per port rate control.
         * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
         * If no bucket number assigned, use bucket 1 as default bucket.
         */
        if (*type & DRV_STORM_CONTROL_BUCKET_2) {
            bucket_field_ref_cnt = INDEX(BKT2_REF_CNTf);
            bucket_field_bkt_size = INDEX(BKT2_BKT_SIZEf);
        } else {
            bucket_field_ref_cnt = INDEX(BKT1_REF_CNTf);
            bucket_field_bkt_size = INDEX(BKT1_BKT_SIZEf);
        }

        sal_memset(&irc_entry, 0, sizeof(irc_entry));

        /* Read Ingress Rate Control memory table by index (port) */
        index = port;
        rv = MEM_READ_IRC_PORTm(unit, index, (uint32 *)&irc_entry);
        SOC_IF_ERROR_RETURN(rv);

        rv = DRV_MEM_FIELD_GET(unit, INDEX(IRC_PORTm), 
            bucket_field_ref_cnt, (uint32 *)&irc_entry, &field_val32);
        SOC_IF_ERROR_RETURN(rv);
        *limit = (field_val32 * TB_RATE_REF_COUNT_GRANULARITY(unit));

        rv = DRV_MEM_FIELD_GET(unit, INDEX(IRC_PORTm), 
            bucket_field_bkt_size, (uint32 *)&irc_entry, &field_val32);
        SOC_IF_ERROR_RETURN(rv);
        *burst_size = (field_val32 * TB_RATE_BUCKET_UNIT_SIZE(unit) * 8) / 1000;
    
    }
    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_tbx_storm_control_get: \
                         unit = %d, port = %d, type = 0x%x, limit = %dK, burst = %dK\n"),
              unit, port, *type, *limit, *burst_size));

    return rv;
}
