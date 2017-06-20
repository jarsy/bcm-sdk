/*
 * $Id: ped.c,v 1.18.20.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    ped.c
 * Purpose: Caladan3 Packet Editor drivers
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/ped.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>

#define SOC_SBX_CALADAN3_PD_DEFAULT_LEN_POS                0
#define SOC_SBX_CALADAN3_PD_DEFAULT_IPV4_LEN_POS           4
#define SOC_SBX_CALADAN3_PD_DEFAULT_IPV4_LEN_SIZE          3
#define SOC_SBX_CALADAN3_PD_DEFAULT_IPV4_LEN_UNITS         2

/*
 *
 * Function:
 *     soc_sbx_caladan3_ped_default_param_init
 * Purpose:
 *     initialize default params drivers
 */
STATIC int soc_sbx_caladan3_ped_default_param_init(int unit) 
{
    soc_sbx_caladan3_pd_config_t *ped = &SOC_SBX_CFG_CALADAN3(unit)->ped_cfg;

    ped->debug                = FALSE;
    ped->route_hdr_len_update = TRUE;
    ped->ipv4_chksum_update   = TRUE;
    ped->ipv4_chksum_conditional_update = TRUE;
    ped->truncation_remove    = 4;  /* mac crc */
    ped->truncation_value     = 12+ 64; /* erh + min size packet */
    ped->route_header_len_update = TRUE;
    ped->flags = SOC_SBX_CALADAN3_PD_DQ_DROP | \
                 SOC_SBX_CALADAN3_PD_SQ_DROP | \
                 SOC_SBX_CALADAN3_PD_OVERLAP_DROP| \
                 SOC_SBX_CALADAN3_PD_LEN_DROP| \
                 SOC_SBX_CALADAN3_PD_OVERRUN_DROP;

    return SOC_E_NONE;
}
#if 0
/*
 *
 * Function:
 *     soc_sbx_caladan3_ped_debug_config
 * Purpose:
 *    debug interface
 */
STATIC int soc_sbx_caladan3_ped_debug_config(int unit) 
{
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_ped_interrupt_config
 * Purpose:
 *     initialize interrupts
 */
STATIC int soc_sbx_caladan3_ped_interrupt_config(int unit) 
{
    return SOC_E_NONE;
}
#endif

/*
 * Function:
 *     soc_sbx_caladan3_ipv4_hdr_config
 * Purpose:
 *     setup ipv4 header type
 */
int 
soc_sbx_caladan3_ipv4_hdr_config(int unit, int type, int cond_cksum, int enable)
{
    soc_sbx_caladan3_pd_config_t *ped = &SOC_SBX_CFG_CALADAN3(unit)->ped_cfg;
    uint32 regval = 0;
    if (type > 0) {
        ped->ipv4_chksum_conditional_update = cond_cksum;
        ped->ipv4_chksum_update = enable;

        READ_PD_IPV4_CONFIGr(unit, &regval);
        if (enable && soc_reg_field_get(unit, PD_IPV4_CONFIGr, regval, ENBf)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d IPv4 header overwrite detected, old type(%x) new type(%x)"),
                       unit, 
                       soc_reg_field_get(unit, PD_IPV4_CONFIGr, regval, HDR_TYPEf), 
                       type));
            return SOC_E_PARAM;
        }
        soc_reg_field_set(unit, PD_IPV4_CONFIGr, &regval, CONDITIONAL_UPDATEf,
                          cond_cksum);
        soc_reg_field_set(unit, PD_IPV4_CONFIGr, &regval, ENBf, enable);
        soc_reg_field_set(unit, PD_IPV4_CONFIGr, &regval, HDR_TYPEf, type);
        SOC_IF_ERROR_RETURN(WRITE_PD_IPV4_CONFIGr(unit, regval));

    }
    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_caladan3_varlen_hdr_config
 * Purpose:
 *     setup varlen header type
 */
int 
soc_sbx_caladan3_varlen_hdr_config(int unit, int type, int units, int size, int posn, int enable)
{
    uint32 regval = 0;

    READ_PD_HDR_CONFIGr(unit, type, &regval);
    soc_reg_field_set(unit, PD_HDR_CONFIGr, &regval, LEN_POSNf,
                              posn);

    soc_reg_field_set(unit, PD_HDR_CONFIGr, &regval, LEN_SIZEf,
                              size);

    soc_reg_field_set(unit, PD_HDR_CONFIGr, &regval, LEN_UNITSf,
                              units);
    soc_reg_field_set(unit, PD_HDR_CONFIGr, &regval, LEN_MODf, enable);
    SOC_IF_ERROR_RETURN(WRITE_PD_HDR_CONFIGr(unit, type, regval));
    
    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_caladan3_xferlen_hdr_config
 * Purpose:
 *     setup header type that updates transfer len to a field in the header 
 */
int 
soc_sbx_caladan3_xferlen_hdr_config(int unit, int type, int size, int offset, int enable)
{
    soc_sbx_caladan3_pd_config_t *ped = &SOC_SBX_CFG_CALADAN3(unit)->ped_cfg;
    uint32 regval = 0;
    if (type > 0) {
        if ((offset >= 16) || (size > 2)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Invalid parameter offset(0-15)=%d size(0-2)=%d"), offset, size));
            return SOC_E_PARAM;
        }

        READ_PD_LEN_UPDATE_CONFIGr(unit, &regval);
        if (enable && soc_reg_field_get(unit, PD_LEN_UPDATE_CONFIGr, regval, ENBf)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Xferlen header overwrite detected, old type(%x) new type(%x)"),
                       unit, 
                       soc_reg_field_get(unit, PD_LEN_UPDATE_CONFIGr, regval, HDR_TYPEf), 
                       type));
            return SOC_E_PARAM;
        }
        ped->xferlen_enable = enable;
        ped->xferlen_offset = offset;
        ped->xferlen_size = size;
        soc_reg_field_set(unit, PD_LEN_UPDATE_CONFIGr, &regval, ENBf, enable);
        soc_reg_field_set(unit, PD_LEN_UPDATE_CONFIGr, &regval, SIZEf, size); 
        soc_reg_field_set(unit, PD_LEN_UPDATE_CONFIGr, &regval, HDR_TYPEf, type);
        soc_reg_field_set(unit, PD_LEN_UPDATE_CONFIGr, &regval, OFFSETf, offset); 
        SOC_IF_ERROR_RETURN(WRITE_PD_LEN_UPDATE_CONFIGr(unit, regval));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_caladan3_elen_hdr_config
 * Purpose:
 *     setup elen header type
 */
int 
soc_sbx_caladan3_elen_hdr_config(int unit, int type, int enable)
{
    uint32 regval = 0;
    if (type > 0) {
        SOC_IF_ERROR_RETURN(READ_PD_EMB_LEN_CONFIGr(unit, &regval));
        if (enable && soc_reg_field_get(unit, PD_EMB_LEN_CONFIGr, regval, ENBf)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Elen header overwrite detected, old type(%x) new type(%x)"),
                       unit, 
                       soc_reg_field_get(unit, PD_EMB_LEN_CONFIGr, regval, HDR_TYPEf), 
                       type));
            return SOC_E_PARAM;
        }
        soc_reg_field_set(unit, PD_EMB_LEN_CONFIGr, &regval, ENBf, enable);
        soc_reg_field_set(unit, PD_EMB_LEN_CONFIGr, &regval, HDR_TYPEf, type);
        SOC_IF_ERROR_RETURN(WRITE_PD_EMB_LEN_CONFIGr(unit, regval));
    }
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_ped_hdr_init
 * Purpose:
 *     Initialize/Clear header attributes
 */
int
soc_sbx_caladan3_ped_hdr_init(int unit,
                              soc_sbx_caladan3_pd_hdr_info_t *info,
                              uint8 clear)
{
    soc_sbx_caladan3_pd_config_t *ped;

    ped = &SOC_SBX_CFG_CALADAN3(unit)->ped_cfg;

    if (!info) {
        return SOC_E_PARAM;
    }

    if (info->type >= SOC_SBX_CALADAN3_PD_LAST_HEADER_TYPE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "!!!! uint[%d] - Max header supported: %d "),
                   unit, SOC_SBX_CALADAN3_PD_LAST_HEADER_TYPE));
        return SOC_E_PARAM;
    }
    if ((info->elen_type + info->ipv4_type + info->xferlen_type) > 1) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "!!!! uint[%d] - header can be of only one type: %d "),
                   unit, info->type));
        return SOC_E_PARAM;

    }
    if (info->xferlen_type) {
        if (info->xferlen_offset > 16) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "!!!! uint[%d] - XFERLEN type offset invalid: %d"),
                       unit, info->xferlen_offset));
            return SOC_E_PARAM;
        }
        if (info->xferlen_size > 2) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "!!!! uint[%d] - XFERLEN type size invalid: %d "),
                       unit, info->xferlen_size));
            return SOC_E_PARAM;
        }
    }

    if (info->varlen_mod) {
        if (info->varlen_units > 3) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "!!!! uint[%d] - VARLEN MOD units invalid: %d"),
                       unit, info->varlen_units));
            return SOC_E_PARAM;
        }
        if (info->varlen_size > 7) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "!!!! uint[%d] - VARLEN MOD size invalid: %d"),
                       unit, info->varlen_size));
            return SOC_E_PARAM;
        }
        if (info->varlen_posn > 511) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "!!!! uint[%d] - VARLEN MOD posn invalid: %d"),
                       unit, info->varlen_posn));
            return SOC_E_PARAM;
        }
    }

    if (ped->hdr_attr[info->type].valid && !clear) {
        /* trying to overwrite header assignment */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "!!!! uint[%d] - Clear old header"
                              " info before overrideing"),unit));
        return SOC_E_PARAM;
    }

    if (clear) {
        ped->hdr_attr[info->type].valid = FALSE;
    } else {
        ped->hdr_attr[info->type].type = info->type;
        ped->hdr_attr[info->type].length = info->length;
        ped->hdr_attr[info->type].valid = TRUE;
        ped->hdr_attr[info->type].elen_type = info->elen_type ? 1 : 0;
        ped->hdr_attr[info->type].xferlen_type = info->xferlen_type ? 1 : 0;
        ped->hdr_attr[info->type].varlen_mod = info->varlen_mod ? 1 : 0;
        if (ped->hdr_attr[info->type].xferlen_type) {
            ped->hdr_attr[info->type].xferlen_offset = info->xferlen_offset;
            ped->hdr_attr[info->type].xferlen_size = info->xferlen_size;
        }
        if (ped->hdr_attr[info->type].varlen_mod) {
            ped->hdr_attr[info->type].varlen_units = info->varlen_units;
            ped->hdr_attr[info->type].varlen_size = info->varlen_size;
            ped->hdr_attr[info->type].varlen_posn = info->varlen_posn;
        } 
        ped->hdr_attr[info->type].ipv4_type = info->ipv4_type ? 1 : 0;
    }

    return (soc_sbx_caladan3_ped_hdr_config(unit, info->type, clear));

}



/*
 *
 * Function:
 *     soc_sbx_caladan3_ped_hdr_config
 * Purpose:
 *     initialize header parsing data
 */
int soc_sbx_caladan3_ped_hdr_config(int unit, int index, int clear) 
{
    soc_sbx_caladan3_pd_config_t *ped;
    uint32 regval;

    ped = &SOC_SBX_CFG_CALADAN3(unit)->ped_cfg;

    if (clear) {
        regval = 0;
        SOC_IF_ERROR_RETURN(WRITE_PD_HDR_CONFIGr(unit, index, regval));
        if (ped->hdr_attr[index].elen_type) {
            SOC_IF_ERROR_RETURN(soc_sbx_caladan3_elen_hdr_config(unit, index, FALSE));
        }
        if (ped->hdr_attr[index].ipv4_type) {
            SOC_IF_ERROR_RETURN(soc_sbx_caladan3_ipv4_hdr_config(unit, index, FALSE, FALSE));
        }
        if (ped->hdr_attr[index].xferlen_type) {
            SOC_IF_ERROR_RETURN(soc_sbx_caladan3_xferlen_hdr_config(unit, index, 
                                0, 0, FALSE));
        }
        if (ped->hdr_attr[index].varlen_mod) {
            SOC_IF_ERROR_RETURN(soc_sbx_caladan3_varlen_hdr_config(unit, index, 
                                                                   0, 0, 0, FALSE));
        }
    } else {
        regval = 0;
        if (ped->hdr_attr[index].valid) {
            if (ped->hdr_attr[index].varlen_mod == 0) {
                SOC_IF_ERROR_RETURN(soc_sbx_caladan3_varlen_hdr_config(unit, index, 
                                0, 
                                0,  
                                SOC_SBX_CALADAN3_PD_DEFAULT_LEN_POS,
                                FALSE));
            } else {
                SOC_IF_ERROR_RETURN(soc_sbx_caladan3_varlen_hdr_config(unit, index, 
                                ped->hdr_attr[index].varlen_units, 
                                ped->hdr_attr[index].varlen_size,  
                                ped->hdr_attr[index].varlen_posn,
                                TRUE));
            }

            READ_PD_HDR_CONFIGr(unit, index, &regval);
            soc_reg_field_set(unit, PD_HDR_CONFIGr, &regval, BASE_LENGTHf,
                              ped->hdr_attr[index].length);
            SOC_IF_ERROR_RETURN(WRITE_PD_HDR_CONFIGr(unit, index, regval));
        }
        if (ped->hdr_attr[index].elen_type) {
            SOC_IF_ERROR_RETURN(soc_sbx_caladan3_elen_hdr_config(unit, index, TRUE));
        } 
        if (ped->hdr_attr[index].ipv4_type) {
            SOC_IF_ERROR_RETURN(soc_sbx_caladan3_ipv4_hdr_config(unit, index, TRUE, TRUE));
        } 
        if (ped->hdr_attr[index].xferlen_type) {
            SOC_IF_ERROR_RETURN(soc_sbx_caladan3_xferlen_hdr_config(unit, index, 
                                ped->hdr_attr[index].xferlen_size, 
                                ped->hdr_attr[index].xferlen_offset,  TRUE));
        } 
    }

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_ped_driver_init
 * Purpose:
 *     Bring up ped drivers
 */
int soc_sbx_caladan3_ped_driver_init(int unit) 
{
    uint32 regval;
    int index, enabled;
    soc_sbx_caladan3_pd_config_t *ped = &SOC_SBX_CFG_CALADAN3(unit)->ped_cfg;

#ifdef BCM_WARM_BOOT_SUPPORT
    if(!SOC_WARM_BOOT(unit))
    {
#endif /* BCM_WARM_BOOT_SUPPORT */
        sal_memset(ped, 0, sizeof(soc_sbx_caladan3_pd_config_t));
        
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_ped_default_param_init(unit));
        
        regval = 0;
        soc_reg_field_set(unit, PD_CONFIG0r, &regval, SOFT_RESET_Nf, 1);
        SOC_IF_ERROR_RETURN(WRITE_PD_CONFIG0r(unit, regval));
        
        /* truncation config */
        SOC_IF_ERROR_RETURN(READ_PD_TRUNC_CONFIGr(unit, &regval));   
        soc_reg_field_set(unit, PD_TRUNC_CONFIGr, &regval, LENGTHf, ped->truncation_value);
        soc_reg_field_set(unit, PD_TRUNC_CONFIGr, &regval, REMOVEf, ped->truncation_remove);
        SOC_IF_ERROR_RETURN(WRITE_PD_TRUNC_CONFIGr(unit, regval));  
        
        
        /* validate squeue/dqueue on this system */
        for (index=0; index < SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES; index++) {
            if ((index % 32) == 0) {
                SOC_IF_ERROR_RETURN(
                                    READ_PD_VALID_SQUEUESr(unit, index/32, &regval));  
            }
            SOC_IF_ERROR_RETURN(
                                soc_sbx_caladan3_sws_get_queue_status(unit, index, &enabled));
            
            regval |= (enabled << (index % 32));
            
            if (((index+1) % 32) == 0) {
                SOC_IF_ERROR_RETURN(
                                    WRITE_PD_VALID_SQUEUESr(unit, index/32, regval));  
            }
        }
        
        for (index=0;index < SOC_SBX_CALADAN3_SWS_MAX_EGRESS_QUEUES; index++)
            {
                if ((index % 32) == 0) {
                    SOC_IF_ERROR_RETURN(
                                        READ_PD_VALID_DQUEUESr(unit, index/32, &regval));  
                }
                SOC_IF_ERROR_RETURN(
                                    soc_sbx_caladan3_sws_get_queue_status(unit,
                                                                          index + SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES,
                                                                          &enabled));
                
                regval |= (enabled << (index % 32));
                
                if (((index+1) % 32) == 0) {
                    SOC_IF_ERROR_RETURN(
                                        WRITE_PD_VALID_DQUEUESr(unit, index/32, regval));  
                }
            }
        
        /* check, control config */
        regval =  ped->flags;
        SOC_IF_ERROR_RETURN(WRITE_PD_CHECKSr(unit, regval));

#ifdef BCM_WARM_BOOT_SUPPORT
    }
#endif

    return SOC_E_NONE;
}




/*
 *
 * Function:
 *     soc_sbx_caladan3_ped_driver_uninit
 * Purpose:
 *     Clean up ped drivers
 */
int soc_sbx_caladan3_ped_driver_uninit(int unit)
{
    /* Currently no clean up is needed for the PED */
    return SOC_E_NONE;
}



/*
 * PED header dump 
 */
soc_sbx_caladan3_hdesc_format_t ped_hdesc_format[] = {
{  1, "cont ",         24,  8},  
{  1, "hdr0 ",         16,  6}, 
{  1, "hdr1 ",          8,  6}, 
{  1, "hdr2 ",          0,  6}, 
{  2, "hdr3 ",         24,  6}, 
{  2, "hdr4 ",         16,  6}, 
{  2, "hdr5 ",          8,  6}, 
{  2, "hdr6 ",          0,  6}, 
{  3, "hdr7 ",         24,  6}, 
{  3, "hdr8 ",         16,  6}, 
{  3, "hdr9 ",          8,  6}, 
{  3, "hdr10",          0,  6}, 
{  4, "hdr11",         24,  6}, 
{  4, "hdr12",         16,  6}, 
{  4, "hdr13",          8,  6}, 
{  4, "hdr14",          0,  6}, 
{  5, "loc0 ",         24,  8}, 
{  5, "loc1 ",         16,  8}, 
{  5, "loc2 ",          8,  8}, 
{  5, "loc3 ",          0,  8}, 
{  6, "loc4 ",         24,  8}, 
{  6, "loc5 ",         16,  8}, 
{  6, "loc6 ",          8,  8}, 
{  6, "loc7 ",          0,  8}, 
{  7, "loc8 ",         24,  8}, 
{  7, "loc9 ",         16,  8}, 
{  7, "loc10",          8,  8}, 
{  7, "loc11",          0,  8}, 
{  8, "loc12",         24,  8}, 
{  8, "loc13",         16,  8}, 
{  8, "loc14",          8,  8}, 
{  8, "loc15",          0,  8}, 
{  9, "ccde ",         30,  2}, 
{  9, "sbuf ",         16, 14}, 
{  9, "dque ",          8,  8}, 
{  9, "sque ",          0,  7}, 
{  9, "egress",         7,  1}, 
{ 10, "flen",          16, 14}, 
{ 10, "redir",         15,  1}, 
{ 10, "recir",         13,  2}, 
{ 10, "rep",           12,  1}, 
{ 10, "repcnt",         0, 12}, 
{ 10, "dmque",          0,  8}, 
{ 11, "str",           28,  4}, 
{ 11, "drp",           27,  1}, 
{ 11, "err",           26,  1}, 
{ 11, "cpy",           25,  1}, 
{ 11, "frg",           24,  1}, 
{ 11, "src",           22,  2}, 
{ 11, "ogts",          21,  1}, 
{ 11, "maccmd",        18,  3}, 
{ 11, "ccdp",          16,  2}, 
{ 11, "hdrlen",         8,  8}, 
{ 11, "otsloc",         0,  8}, 
{ 12, "trun  ",        30,  2}, 
{ 12, "xferlen",       16, 14}, 
{ 13, "repdata  ",      0, 32}, 
{ 14, "seqnum   ",     16, 16}, 
{ 14, "procreg  ",      0, 32}, 
{ 15, "aggrhash ",      0, 32}, 
{ 16, "timestamp",      0, 32}, 
{0}
};

int 
soc_sbx_caladan3_ped_hd_parsed(uint32 *rawdata, int max, char*desc)
{
    int i, j, s, data, maxdesc;
    uint32 m;
    soc_sbx_caladan3_hdesc_format_t *element;

    s = i = 0; maxdesc = COUNTOF(ped_hdesc_format);
    for (j = 0; j < maxdesc; ) {
        element = &ped_hdesc_format[j];
        if (!element->word) {
            break;
        }
        /*if ((s == 0) || (s >=50)) {*/
        if (s == i) {
            LOG_CLI((BSL_META("\n%sDesc> 0x%02x"), desc, i<<2));
            s++;
        }
        if (element->word-1 == i) {
            m = (1 << element->size) - 1;
            data = (rawdata[i] >> element->pos) & m;
            LOG_CLI((BSL_META(" %s:%0*x"), element->desc,
                     (element->size+3)/4, data));
            j++;
        } else {
            i++;
        }
    }
    for (i++; i < max; i++) {
        if (i%4==0) {
            LOG_CLI((BSL_META("\n%sData> 0x%02x"), desc, i<<2));
        }
        LOG_CLI((BSL_META(" %08x"), rawdata[i]));
    }
    LOG_CLI((BSL_META("\n")));
    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_ped_hd(int unit, int pedin, int pedout, int mode, uint32 *headerin, uint32 *headerout)
{
    int npedin, npedout;
    int npacket = 0, i, j;
    uint32 regval = 0;
    uint32 data = 0;
    uint32 *rp, rawdata[256];
    rp = &rawdata[0];
    READ_PD_COPY_BUF_LEVELr(unit, &regval);
    npedin = soc_reg_field_get(unit, PD_COPY_BUF_LEVELr,
                            regval, CI_LVLf);
    npedout = soc_reg_field_get(unit, PD_COPY_BUF_LEVELr,
                            regval, CO_LVLf);
    if ((npedin & 1) || (npedout & 1)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "PED Capture buffer out of syn, restart capture!\n")));
        regval = 0;
        soc_reg_field_set(unit, PD_COPY_BUF_CTRLr, &regval, POP_INf, 1);
        while(npedin-- > 0) {
           WRITE_PD_COPY_BUF_CTRLr(unit, regval);
        }
        regval = 0;
        soc_reg_field_set(unit, PD_COPY_BUF_CTRLr, &regval, POP_OUTf, 1);
        while(npedout-- > 0) {
           WRITE_PD_COPY_BUF_CTRLr(unit, regval);
        }
        return SOC_E_NONE;
    }
    do {
        if (pedin || pedout) {
            regval = 0; 
            if (headerin) {
                rp = headerin;
                headerin += 64;
            } else 
                rp = &rawdata[0];

            soc_reg_field_set(unit, PD_COPY_BUF_CTRLr, &regval, POP_INf, 1);
                
            if (npedin > 0) {
                if (!headerin && pedin)
                   LOG_CLI((BSL_META_U(unit,
                                       "\nPED IN Headerin Descriptor for packet %d %s"), npacket, mode ? "" : "(raw)"));
		/* 1st part of desc */
		WRITE_PD_COPY_BUF_CTRLr(unit, regval);
		for (j=0,i=31; i >=0; i--, j++) {
		    if ((!mode) && (j%4 == 0) && !headerin && pedin) {
			LOG_CLI((BSL_META_U(unit,
                                            "\n0x%02x"), j*4));
		    }
		    data = 0;
		    READ_PD_COPY_BUF_DATAr(unit, i, &data);
		    if (mode || headerin) {
			*rp++ = data;
		    } else {
			if (pedin) {
			    LOG_CLI((BSL_META_U(unit,
                                                " %08x"), data));
			}
		    }
		}
		/* 2nd part of desc */
		WRITE_PD_COPY_BUF_CTRLr(unit, regval);
		for (j=0, i=31; i >=0; i--, j++) {
		    if ((!mode) && (j%4 == 0) && !headerin && pedin) {
			LOG_CLI((BSL_META_U(unit,
                                            "\n0x%02x"), 128 + j*4));
		    }
		    data = 0;
		    READ_PD_COPY_BUF_DATAr(unit, i, &data);
		    if (mode || headerin) {
			*rp++ = data;
		    } else {
			if (pedin) {
			    LOG_CLI((BSL_META_U(unit,
                                                " %08x"), data));
			}
		    }
		}
		npedin-=2;
		if (mode && !headerin && pedin) {
		    soc_sbx_caladan3_ped_hd_parsed(&rawdata[0], 64, "PEDIn");
		}
            }
        }
        if (pedin || pedout) {
            regval = 0;
            if (headerout) {
                rp = headerout;
                headerout += 64;
            } else 
                rp = &rawdata[0];

            soc_reg_field_set(unit, PD_COPY_BUF_CTRLr, &regval, POP_OUTf, 1);
            
            if (npedout > 0) {
                if (!headerout && pedout)
                   LOG_CLI((BSL_META_U(unit,
                                       "\nPED OUT Header Descriptor for packet %d %s"), npacket, mode ? "" : "(raw)"));
		/* 1st part */
		WRITE_PD_COPY_BUF_CTRLr(unit, regval);
		for (j=0, i=31; i >=0; i--, j++) {
		    if ((!mode) && (j%4 == 0) && !headerout && pedout) {
			LOG_CLI((BSL_META_U(unit,
                                            "\n0x%02x"), j*4));
		    }
		    data = 0;
		    READ_PD_COPY_BUF_DATAr(unit, i, &data);
		    if (mode || headerout) {
			*rp++ = data;
		    } else {
			if (pedout) {
			    LOG_CLI((BSL_META_U(unit,
                                                " %08x"), data));
			}
		    }
		}
		/* 2nd part */
		WRITE_PD_COPY_BUF_CTRLr(unit, regval);
		for (j=0, i=31; i >=0; i--, j++) {
		    if ((!mode) && (j%4 == 0) && !headerout && pedout) {
			LOG_CLI((BSL_META_U(unit,
                                            "\n0x%02x"), 128 + j*4));
		    }
		    data = 0;
		    READ_PD_COPY_BUF_DATAr(unit, i, &data);
		    if (mode || headerout) {
			*rp++ = data;
		    } else {
			if (pedout) {
			    LOG_CLI((BSL_META_U(unit,
                                                " %08x"), data));
			}
		    }
		}
		npedout-=2;
		if (mode && !headerout && pedout) {
		    soc_sbx_caladan3_ped_hd_parsed(&rawdata[0], 64, "PEDout");
		}
            }
        }
        npacket++;
        if (!headerout)
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
    } while((npedin > 0) && (npedout > 0));
    return SOC_E_NONE;   
}


#endif
