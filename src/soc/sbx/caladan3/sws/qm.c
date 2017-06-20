/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: qm.c,v 1.3.16.8 Broadcom SDK $
 *
 * File:    sws.c
 * Purpose: Caladan3 SWS drivers
 * Requires:
 */



#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>

/*
 * #define _CALADAN3_SWS_LOCKUP_WAR
 */
/*
 * This workaround is implemented below. To enable the workaround, uncomment
 * the define above. NOTE: enable this workaround will prevent a very rare SWS lockup
 * issue in A0/A1/B0 (which was fixed in B1 hardware), but might have impact of not 
 * achieving linerate for some TDMs.
 */


/**
 * Function:
 *     soc_sbx_caladan3_sws_pb_init
 * Purpose:
 *     Initialize Packet Buffer
 */
int soc_sbx_caladan3_sws_pb_init(int unit)
{
    uint32 regval;

    SOC_IF_ERROR_RETURN(READ_PB_CONFIGr(unit, &regval));
    soc_reg_field_set(unit, PB_CONFIGr, &regval, PB_SOFT_RESET_Nf, 1);
    SOC_IF_ERROR_RETURN(WRITE_PB_CONFIGr(unit, regval));

    soc_reg_field_set(unit, PB_CONFIGr, &regval, PB_DO_INITf, 1);
    SOC_IF_ERROR_RETURN(WRITE_PB_CONFIGr(unit, regval));
    
    /* wait for init done - global config response */
    SOC_IF_ERROR_RETURN(
        soc_sbx_caladan3_reg32_expect_field_timeout(unit, PB_CONFIGr, -1,
                                                    0, 0, PB_INIT_DONEf, 1, 
                                                    MILLISECOND_USEC * 100));

    regval = 0x000001ff;
    SOC_IF_ERROR_RETURN(WRITE_PB_MEM_INIT_DONEr(unit, regval));

    regval = 0;
    SOC_IF_ERROR_RETURN(WRITE_PB_ECC_ERRORr(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_PB_ERROR0r(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_PB_PARITY_ERRORr(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_PB_TRACE_IF_STATUSr(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_PB_ECC_ERROR_MASKr(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_PB_PARITY_ERROR_MASKr(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_PB_TRACE_IF_STATUS_MASKr(unit, regval));

    SOC_IF_ERROR_RETURN(READ_PB_CONFIGr(unit, &regval));
    soc_reg_field_set(unit, PB_CONFIGr, &regval, PB_ENABLEf, 1);
    SOC_IF_ERROR_RETURN(WRITE_PB_CONFIGr(unit, regval));
    return SOC_E_NONE;
}


STATIC int _soc_sbx_caladan3_sws_qm_common_init(int unit) 
{
    uint32 resvd, regval, value;
    int idx, rv = SOC_E_NONE;
    qm_buffer_state_age_entry_t bufstate;
    qm_replication_ref_entry_t  repstate;

    /* bring out of reset */
    SOC_IF_ERROR_RETURN(READ_QM_CONFIGr(unit, &regval));
    soc_reg_field_set(unit, QM_CONFIGr, &regval, QM_SOFT_RESET_Nf, 1); 
    SOC_IF_ERROR_RETURN(WRITE_QM_CONFIGr(unit, regval));

    /* initialize memory */
    regval = 0x7fffffff;
    SOC_IF_ERROR_RETURN(WRITE_QM_MEM_INITr(unit, regval));
    /* wait for init done */
    SOC_IF_ERROR_RETURN(
        soc_sbx_caladan3_reg32_expect_field_timeout(unit, QM_MEM_INIT_STATUSr, 
                                                    -1, 0, 0, -1, 0x7ffffff, 
                                                    MILLISECOND_USEC * 100));
    
    resvd = soc_sbx_caladan3_sws_config_param_data_get(unit, 
                      spn_NUM_PAGES_RESERVED, -1, SOC_SBX_CALADAN3_SWS_BUBBLE_RESERVED_PAGES);
    resvd = SOC_SBX_CALADAN3_SWS_MAX_PAGES - resvd;

    soc_reg_field_set(unit, QM_FP_CONFIG0r, &regval, FP_INITf, 1);
    soc_reg_field_set(unit, QM_FP_CONFIG0r, &regval, FP_INIT_START_PAGEf, 0);
    soc_reg_field_set(unit, QM_FP_CONFIG0r, &regval, 
                      FP_SCOREBOARD_PRESERVE_ON_READf, 1);
    soc_reg_field_set(unit, QM_FP_CONFIG0r, &regval, FP_INIT_END_PAGEf, resvd);
    SOC_IF_ERROR_RETURN(WRITE_QM_FP_CONFIG0r(unit, regval));

    regval = 0;
    SOC_IF_ERROR_RETURN(WRITE_QM_ECC_ERROR0r(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_QM_ECC_ERROR1r(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_QM_ECC_ERROR2r(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_QM_ECC_ERROR3r(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_QM_ERRORr(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_QM_PARITY_ERRORr(unit, regval));
    SOC_IF_ERROR_RETURN(WRITE_QM_TRACE_IF_STATUS_MASKr(unit, regval));

    /* Pause interface Deltas config */
    regval = 0;
    soc_reg_field_set(unit, QM_HPTE_PAUSE_CONFIG0r, &regval,
                    QUEUE_MAX_PAGES_DELTAf,
                    SOC_SBX_CALADAN3_SWS_QM_PAUSE_MAX_PAGES_DELTA);
    soc_reg_field_set(unit, QM_HPTE_PAUSE_CONFIG0r, &regval,
                    MIN_PAGES_HEADER_DELTAf,
                    SOC_SBX_CALADAN3_SWS_QM_PAUSE_HDR_MIN_PAGES_DELTA);
    soc_reg_field_set(unit, QM_HPTE_PAUSE_CONFIG0r, &regval,
                    INTERFACE_MAX_PAGES_DELTAf,
                    SOC_SBX_CALADAN3_SWS_QM_PAUSE_INTF_MAX_PAGES_DELTA);
    soc_reg_field_set(unit, QM_HPTE_PAUSE_CONFIG0r, &regval,
                    ENABLEf, 1);
    SOC_IF_ERROR_RETURN(WRITE_QM_HPTE_PAUSE_CONFIG0r(unit, regval));
    regval = 0;
    soc_reg_field_set(unit, QM_HPTE_PAUSE_CONFIG1r, &regval,
                    TOTAL_RESERVED_PAGES_DELTAf,
                    SOC_SBX_CALADAN3_SWS_QM_PAUSE_TOTAL_RSVD_PAGES_DELTA);
    soc_reg_field_set(unit, QM_HPTE_PAUSE_CONFIG1r, &regval,
                      TOTAL_BUFF_MAX_PAGES_DELTAf,
                      SOC_SBX_CALADAN3_SWS_QM_PAUSE_TOTAL_BUFF_MAX_PAGES_DELTA);
    SOC_IF_ERROR_RETURN(WRITE_QM_HPTE_PAUSE_CONFIG1r(unit, regval));


    /* Reserved pages init */
    sal_memset(&bufstate, 0, sizeof(bufstate));
    value = SOC_SBX_CALADAN3_SWS_PAGE_RESERVED;
    soc_mem_field_set(unit, QM_BUFFER_STATE_AGEm, &bufstate.entry_data[0], 
                      AGEf, &value );

    sal_memset(&repstate, 0, sizeof(repstate));
    value = SOC_SBX_CALADAN3_SWS_PAGE_REFCNT_MAX;
    soc_mem_field_set(unit, QM_REPLICATION_REFm, &repstate.entry_data[0], 
                      REF_CNTf, &value );

    for (idx = resvd + 1;
             idx <= SOC_SBX_CALADAN3_SWS_MAX_PAGES; idx++) {
        rv = soc_mem_write(unit, QM_REPLICATION_REFm,
                           MEM_BLOCK_ANY, idx, &repstate);
        if (SOC_FAILURE(rv)) {
            return rv;
        }
        rv = soc_mem_write(unit, QM_BUFFER_STATE_AGEm, MEM_BLOCK_ANY, 
                           idx, &bufstate);
        if (SOC_FAILURE(rv)) {
            return rv;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function
 *    soc_sbx_caladan3_sws_qm_dest_queue_enable_all
 * Purpose
 *    Enable or Disable all dest queue
 */
int
soc_sbx_caladan3_sws_qm_dest_queue_enable_all(int unit, int enable)
{
    uint32 regidx;
    uint32 regval = 0;
    int dq_cfg_reg[] = {QM_DQUEUE_CONFIG0r, QM_DQUEUE_CONFIG1r, 
                        QM_DQUEUE_CONFIG2r, QM_DQUEUE_CONFIG3r};

    for (regidx = 0; 
             regidx < (sizeof(dq_cfg_reg)/sizeof(dq_cfg_reg[0]));
                regidx++) {
        regval =  (enable) ? 0xffffffff : 0;
        SOC_IF_ERROR_RETURN(
            soc_reg32_set(unit, dq_cfg_reg[regidx], REG_PORT_ANY, 0, regval));
    }
    return SOC_E_NONE;
}

/*
 * Function
 *    soc_sbx_caladan3_sws_qm_dest_queue_enable
 * Purpose
 *    Enable or Disable a given queue
 */
int
soc_sbx_caladan3_sws_qm_dest_queue_enable(int unit, int qid, int enable)
{
    uint32 regidx, qenable;
    uint32 regval = 0;
    int dq_cfg_reg[] = {QM_DQUEUE_CONFIG0r, QM_DQUEUE_CONFIG1r, 
                        QM_DQUEUE_CONFIG2r, QM_DQUEUE_CONFIG3r};

    if ((qid < SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES) ||
         (qid > SOC_SBX_CALADAN3_SWS_MAX_QUEUE_ID)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, qm_dest_queue_enable invalid queue id %d\n"), unit, qid));
        return SOC_E_PARAM;
    }

    regidx = (qid >> 5) - 4;
    qenable = (1 << (qid % 32));
    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, dq_cfg_reg[regidx], REG_PORT_ANY, 0, &regval));
    regval =  (enable) ? (regval | qenable) : (regval & (~qenable));
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, dq_cfg_reg[regidx], REG_PORT_ANY, 0, regval));

    return SOC_E_NONE;
   
}

/*
 * Function
 *    soc_sbx_caladan3_sws_qm_src_queue_enable_all
 * Purpose
 *    Enable or Disable all source queue
 */
int
soc_sbx_caladan3_sws_qm_src_queue_enable_all(int unit, int enable)
{
    int qid;
    uint32 val = 0;
    qm_source_queue_config_entry_t *entry;
    static qm_source_queue_config_entry_t *entries = NULL;
    uint32  alloc_size;
    soc_error_t status;

    val = (enable > 0) ? 1 : 0;

    if (entries == NULL) {
        alloc_size = soc_mem_index_count(unit,QM_SOURCE_QUEUE_CONFIGm) *
            sizeof(qm_source_queue_config_entry_t);
        entries = soc_cm_salloc( unit, alloc_size, "QM_SOURCE_QUEUE_CONFIGm");
        if (entries == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: %s: soc_cm_salloc for QM_SOURCE_QUEUE_CONFIGm failed\n"),
                       unit, FUNCTION_NAME()));
            return SOC_E_MEMORY;
        }
    }
    status = soc_mem_read_range(unit, QM_SOURCE_QUEUE_CONFIGm,
                                MEM_BLOCK_ANY,
                                SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE,
                                SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE +
                                    SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES-1,
                                entries);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d: %s: soc_mem_read_range failed: %s\n"),
                   unit, FUNCTION_NAME(), soc_errmsg(status)));
    }
    for (qid = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE;
         qid  < (SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE +
                 SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES);
         qid++) {
        entry = soc_mem_table_idx_to_pointer(unit,
                              QM_SOURCE_QUEUE_CONFIGm,
                              qm_source_queue_config_entry_t *,
                              entries, qid);
        soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry->entry_data[0], 
                          ENABLEf, &val);
    }
    status = soc_mem_write_range(unit, QM_SOURCE_QUEUE_CONFIGm,
                                MEM_BLOCK_ANY,
                                SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE,
                                SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE +
                                    SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES-1,
                                entries);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d: %s: soc_mem_write_range failed: %s\n"),
                   unit, FUNCTION_NAME(), soc_errmsg(status)));
    }

    return SOC_E_NONE;
}

/*
 * Function
 *    soc_sbx_caladan3_sws_qm_src_queue_enable
 * Purpose
 *    Enable or Disable a given source queue
 */
int
soc_sbx_caladan3_sws_qm_src_queue_enable(int unit, int qid, int enable)
{
    qm_source_queue_config_entry_t entry;
    uint32 val = 0 ;

    if ((qid < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) ||
         (qid >= SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, qm_src_queue_enable invalid src queue id %d\n"), 
                   unit, qid));
        return SOC_E_PARAM;
    }

    val = (enable > 0) ? 1 : 0;

    SOC_IF_ERROR_RETURN(
        READ_QM_SOURCE_QUEUE_CONFIGm(unit, MEM_BLOCK_ANY, qid, &entry));
    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0], 
                      ENABLEf, &val);
    SOC_IF_ERROR_RETURN(
        WRITE_QM_SOURCE_QUEUE_CONFIGm(unit, MEM_BLOCK_ANY, qid, &entry));

    return SOC_E_NONE;
}


/**
 * Function:
 *     soc_sbx_caladan3_sws_qm_source_queue_config
 * Purpose:
 *     configure a given source queue
 */
int 
soc_sbx_caladan3_sws_qm_source_queue_config(int unit, int qid, sws_qm_source_queue_cfg_t* queue_cfg, int en)
{
    qm_source_queue_config_entry_t entry;
    uint32  enabled = 0;
    uint32  hw_enabled;

    if (qid >= SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Qid %d not a source queue"), unit, qid));
        return SOC_E_INIT;
    }
    if (!queue_cfg) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Qid %d Source Queue Parameter not found\n"), unit, qid));
        return SOC_E_INIT;
    }
    enabled = (en > 0) ? 1: 0;

     LOG_VERBOSE(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d, Configuring Qid %d \n"), unit, qid));

    SOC_IF_ERROR_RETURN(
        READ_QM_SOURCE_QUEUE_CONFIGm(unit, MEM_BLOCK_ANY, qid, &entry));


    soc_mem_field_get(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0],
                      ENABLEf, &hw_enabled);
    if (SOC_HOTSWAP_TDM && en && hw_enabled) {
        return SOC_E_NONE;
    }

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0], 
                      DROP_THRESH_DE1f, &queue_cfg->drop_thres_de1);
 
    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0], 
                      DROP_THRESH_DE2f, &queue_cfg->drop_thres_de2); 

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0], 
                      ENABLEf, &enabled); 

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0],
                      FLOW_CTRL_THRESHf, &queue_cfg->flow_ctrl_thresh); 

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0],
                      MAX_PAGESf, &queue_cfg->max_pages); 

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0], 
                      MIN_PAGES_DATAf, &queue_cfg->min_pages_data); 
        
    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0], 
                      MIN_PAGES_HEADERf, &queue_cfg->min_pages_header);
   
    SOC_IF_ERROR_RETURN(
        WRITE_QM_SOURCE_QUEUE_CONFIGm(unit, MEM_BLOCK_ANY, qid, &entry));

    return SOC_E_NONE;
}

/*
 * Load parameters from the Config / Default
 */
int
soc_sbx_caladan3_buffer_param_config_get(int unit, sws_qm_buffer_cfg_t *buf_cfg)
{
    char *prop;
    if (!buf_cfg) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d buffer_config_get invalid parameter"), unit));
        return SOC_E_PARAM;
    }
    if (buf_cfg) {
        sal_memset(buf_cfg, 0, sizeof(sws_qm_buffer_cfg_t));
        buf_cfg->num_pages_reserved = soc_sbx_caladan3_sws_config_param_data_get(unit, 
                                          spn_NUM_PAGES_RESERVED,  -1,
                                          SOC_SBX_CALADAN3_SWS_BUBBLE_RESERVED_PAGES);
        if (buf_cfg->num_pages_reserved > SOC_SBX_CALADAN3_SWS_MAX_PAGES) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Total Reserved Pages too large\n"), unit));
            return SOC_E_PARAM;
        }
        buf_cfg->total_buff_max_pages = soc_sbx_caladan3_sws_config_param_data_get(unit,
                                             spn_TOTAL_BUFF_MAX_PAGES, -1,
                                            (SOC_SBX_CALADAN3_SWS_MAX_PAGES - buf_cfg->num_pages_reserved));
        if (buf_cfg->total_buff_max_pages > 
                (SOC_SBX_CALADAN3_SWS_MAX_PAGES - buf_cfg->num_pages_reserved)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Total Buf Max Pages does account for Reserved pages\n"), unit));
            return SOC_E_PARAM;
        }
        buf_cfg->total_buff_hysteresis_delta = soc_sbx_caladan3_sws_config_param_data_get(unit, 
                                                    spn_TOTAL_BUFF_HYSTERESIS_DELTA, -1, 16);
        prop = soc_sbx_caladan3_sws_config_param_str_get(unit, spn_TOTAL_BUFF_DROP_THRES_DE1, -1);
        if (prop) {
            if (prop[2]=='%') {
                buf_cfg->total_buff_drop_thres_de1 = _shr_ctoi(prop) * buf_cfg->total_buff_max_pages / 100;
            } else {
                buf_cfg->total_buff_drop_thres_de1 = _shr_ctoi(prop);
            }
        }
        if (buf_cfg->total_buff_drop_thres_de1 <= 0) {
            buf_cfg->total_buff_drop_thres_de1 = 
                SOC_SBX_CALADAN3_SWS_GLOBAL_DE1_TRESHOLD_PERCENT * buf_cfg->total_buff_max_pages / 100;
        }
        prop = soc_sbx_caladan3_sws_config_param_str_get(unit, spn_TOTAL_BUFF_DROP_THRES_DE2, -1);
        if (prop) {
            if (prop[2]=='%') {
                buf_cfg->total_buff_drop_thres_de2 = _shr_ctoi(prop) * buf_cfg->total_buff_max_pages / 100;
            } else {
                buf_cfg->total_buff_drop_thres_de2 = _shr_ctoi(prop);
            }
        }
        if (buf_cfg->total_buff_drop_thres_de1 <= 0) {
            buf_cfg->total_buff_drop_thres_de1 = 
                SOC_SBX_CALADAN3_SWS_GLOBAL_DE2_TRESHOLD_PERCENT * buf_cfg->total_buff_max_pages / 100;
        }
        buf_cfg->ingress_max_pages= soc_sbx_caladan3_sws_config_param_data_get(unit, 
                                        spn_INGRESS_MAX_PAGES, -1, 0);
        if (buf_cfg->ingress_max_pages >= SOC_SBX_CALADAN3_SWS_MAX_PAGES) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Total Ingress Max Pages too large\n"), unit));
            return SOC_E_PARAM;
        }
        buf_cfg->ingress_hysteresis_delta =
              soc_sbx_caladan3_sws_config_param_data_get(unit, spn_INGRESS_HYSTERESIS_DELTA, -1, 0);

        prop = soc_sbx_caladan3_sws_config_param_str_get(unit, spn_INGRESS_DROP_THRES_DE1, -1);
        if (prop) {
            if (prop[2]=='%') {
                buf_cfg->ingress_drop_thres_de1 = _shr_ctoi(prop) * buf_cfg->ingress_max_pages / 100;
            } else {
                buf_cfg->ingress_drop_thres_de1 = _shr_ctoi(prop);
            }
        }
        if (buf_cfg->total_buff_drop_thres_de1 <= 0) {
            buf_cfg->total_buff_drop_thres_de1 = 
                SOC_SBX_CALADAN3_SWS_DE1_TRESHOLD_PERCENT * buf_cfg->total_buff_max_pages / 100;
        }
        prop = soc_sbx_caladan3_sws_config_param_str_get(unit, spn_INGRESS_DROP_THRES_DE2, -1);
        if (prop) {
            if (prop[2]=='%') {
                buf_cfg->ingress_drop_thres_de2 = _shr_ctoi(prop) * buf_cfg->ingress_max_pages / 100;
            } else {
                buf_cfg->ingress_drop_thres_de2 = _shr_ctoi(prop);
            }
        }
        if (buf_cfg->total_buff_drop_thres_de1 <= 0) {
            buf_cfg->total_buff_drop_thres_de1 = 
                SOC_SBX_CALADAN3_SWS_DE2_TRESHOLD_PERCENT * buf_cfg->total_buff_max_pages / 100;
        }
        buf_cfg->egress_hysteresis_delta =
            soc_sbx_caladan3_sws_config_param_data_get(unit, spn_EGRESS_HYSTERESIS_DELTA, -1, 0);
      
        buf_cfg->egress_max_pages =
            soc_sbx_caladan3_sws_config_param_data_get(unit, spn_EGRESS_MAX_PAGES, -1, 0);
        if (buf_cfg->egress_max_pages >= SOC_SBX_CALADAN3_SWS_MAX_PAGES) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Egress Max Pages too large\n"), unit));
            return SOC_E_PARAM;
        }

        prop = soc_sbx_caladan3_sws_config_param_str_get(unit, spn_EGRESS_DROP_THRES_DE1, -1);
        if (prop) {
            if (prop[2]=='%') {
                buf_cfg->egress_drop_thres_de1 = _shr_ctoi(prop) * buf_cfg->egress_max_pages / 100;
            } else {
                buf_cfg->egress_drop_thres_de1 = _shr_ctoi(prop);
            }
        }
        if (buf_cfg->total_buff_drop_thres_de1 <= 0) {
            buf_cfg->total_buff_drop_thres_de1 = 
                SOC_SBX_CALADAN3_SWS_DE1_TRESHOLD_PERCENT * buf_cfg->total_buff_max_pages / 100;
        }

        prop = soc_sbx_caladan3_sws_config_param_str_get(unit, spn_EGRESS_DROP_THRES_DE2, -1);
        if (prop) {
            if (prop[2]=='%') {
                buf_cfg->egress_drop_thres_de2 = _shr_ctoi(prop) * buf_cfg->egress_max_pages / 100;
            } else {
                buf_cfg->egress_drop_thres_de2 = _shr_ctoi(prop);
            }
        }
      
        if (buf_cfg->total_buff_drop_thres_de1 <= 0) {
            buf_cfg->total_buff_drop_thres_de1 = 
                SOC_SBX_CALADAN3_SWS_DE2_TRESHOLD_PERCENT * buf_cfg->total_buff_max_pages / 100;
        }

        buf_cfg->fc_total_buffer_xoff_thresh
            = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_FC_TOTAL_BUFFER_XOFF_THRESH, -1, 0);
        buf_cfg->fc_ingress_xoff_thresh
            = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_FC_INGRESS_XOFF_THRESH, -1, 0);
        buf_cfg->fc_egress_xoff_thresh
            = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_FC_EGRESS_XOFF_THRESH, -1, 0);
        buf_cfg->per_queue_drop_hysteresis_delta 
            = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_PER_QUEUE_DROP_HYSTERESIS_DELTA, -1, 0);
    }
    return SOC_E_NONE;
}


/**
 * Function:
 *     soc_sbx_caladan3_sws_qm_config_init
 * Purpose:
 *     Plugin QM parameters & Initialize QM
 *
 */
int 
soc_sbx_caladan3_sws_qm_config_init(int unit, sws_qm_config_t *cfg)
{
    soc_sbx_caladan3_sws_queue_info_t *queue_info;
    soc_sbx_caladan3_port_map_t *port_map;
    uint32 regval;
    sws_qm_buffer_cfg_t       *buf_cfg   = NULL;
    sws_qm_source_queue_cfg_t *queue_cfg = NULL;
    int rv, qid, cos;
    int ing_qid0=0, ing_qid1=0, egr_qid0=0, egr_qid1=0;
    uint32 fc_enable = 1;

#ifdef _CALADAN3_SWS_LOCKUP_WAR
    uint16 dev_id;
    uint8  rev_id;
    uint32 reserved_pages, start_page, end_page, total_pages, active_pages;
#endif /* _CALADAN3_SWS_LOCKUP_WAR */

    if (!cfg) return SOC_E_PARAM;

    fc_enable = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_FC_ENABLE, -1, fc_enable);

    queue_info = soc_sbx_caladan3_sws_queue_info_get(unit);
    if (queue_info == NULL) {
        return SOC_E_INIT;
    }

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    if (port_map == NULL) {
        return SOC_E_INIT;
    }

    /* Flush all queues */
    soc_sbx_caladan3_sws_qm_dest_queue_enable_all(unit, FALSE);
    soc_sbx_caladan3_sws_qm_src_queue_enable_all(unit, FALSE);

     /* configure buffer thresholds */
    if (cfg) {
        buf_cfg = &cfg->buffer;
    } else {
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Buffer Config data not found\n"), unit));
        return SOC_E_PARAM;
    }

#ifdef _CALADAN3_SWS_LOCKUP_WAR

    /* 
     * SWS lockup workaround for A0,A1,B0
     *
     * Workaround description:
     *  I am assuming that we will just implement the work-around from the base line above. The pseudo code for the implementation:
     * (1) Let x be the total pages be available, x = QM_FP_CONFIG0, fp_init_end_page - fp_init_start_page + 1;
     * (2) Let y be the extra pages to be reserved for the work-around (10160 for 88038 and 7572 for 88034)
     * (3) Pages available for active traffic = x - y
     * (4) Update QM_BUFFER_CONFIG0.total_buff_max_pages = (x-y)
     * (5) Update QM_BUFFER_CONFIG0.total_buff_hysteresis_delta= 10% of (x-y)
     * (6) Verify the following field and update to (x-y) if they are greater, and adjust ingress_hysteresis_delta to 10% of (x-y)
     *           QM_BUFFER_CONFIG2 ingress_max_pages
     * (7) Verify the following field and update to (x-y) if they are greater, and adjust egress_hysteresis_delta to 10% of (x-y)
     *           QM_BUFFER_CONFIG4 egress_max_pages
     * (8) Verify the following fields and update to (x-y) if they are greater
     *           QM_BUFFER_CONFIG1.total_buff_drop_thresh_de1, total_buff_drop_thresh_de2
     *           QM_BUFFER_CONFIG3.ingress_drop_thresh_de1, ingress_drop_thresh_de2
     *           QM_BUFFER_CONFIG5.egress_drop_thresh_de1, egress_drop_thresh_de2
     *
     * Workaround implementation:
     *  We enforce the policy here based on what's configured before.
     *  The only input we take from existing config is the "x", (which is read
     *  from QM_FP_CONFIG0, note that during back-to-back init soc or warmboot, this config
     *  is driven by soc property and won't change.
     *  We derive everything else based on formula above and make sure the corresponding
     *  policy is applied, then we overwrite the corresponding "buf_cfg" software state
     *  to match hardware value.
     */
#define _CALADAN3_SWS_QM_WAR_RESERVED_PAGES_88034 (7572)
#define _CALADAN3_SWS_QM_WAR_RESERVED_PAGES_88038 (10160)
#define _CALADAN3_SWS_QM_WAR_RESERVED_PAGES_88039 (10160)
#define _CALADAN3_SWS_QM_WAR_HYSTERESIS_DELTA     (10)

    reserved_pages = 0;
    soc_cm_get_id(unit, &dev_id, &rev_id);
    if ((rev_id == BCM88030_A0_REV_ID) || (rev_id == BCM88030_A1_REV_ID) ||
        (rev_id == BCM88030_B0_REV_ID)) {

        /* this workaround only applies to B0 and earlier,
         * reserved pages based on device ID
         */
        if (dev_id == BCM88030_DEVICE_ID) {
            reserved_pages = _CALADAN3_SWS_QM_WAR_RESERVED_PAGES_88038;
        } else if (dev_id == BCM88034_DEVICE_ID) {
            reserved_pages = _CALADAN3_SWS_QM_WAR_RESERVED_PAGES_88034;            
        } else if (dev_id == BCM88039_DEVICE_ID) {
            reserved_pages = _CALADAN3_SWS_QM_WAR_RESERVED_PAGES_88039;
        }

        /* read total pages, calculate active pages */
        SOC_IF_ERROR_RETURN(READ_QM_FP_CONFIG0r(unit, &regval));
        start_page = soc_reg_field_get(unit, QM_FP_CONFIG0r, regval, FP_INIT_START_PAGEf);
        end_page = soc_reg_field_get(unit, QM_FP_CONFIG0r, regval, FP_INIT_END_PAGEf);

        if (end_page <= start_page) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Invalid QM buffer config. Start 0x%x, end 0x%x\n"),
                       unit, start_page, end_page));
            return SOC_E_PARAM;            
        }

        total_pages = end_page - start_page + 1;

        if (total_pages <= reserved_pages) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Invalid QM buffer config. Total 0x%x, reserved 0x%x\n"),
                       unit, total_pages, reserved_pages));
            return SOC_E_PARAM;            
        }

        active_pages = total_pages - reserved_pages;
        
        /* Apply the policy */
        buf_cfg->total_buff_max_pages = active_pages;
        buf_cfg->total_buff_hysteresis_delta = active_pages/_CALADAN3_SWS_QM_WAR_HYSTERESIS_DELTA;
        if (buf_cfg->total_buff_drop_thres_de1 > active_pages) {
            buf_cfg->total_buff_drop_thres_de1 = active_pages;
        }
        if (buf_cfg->total_buff_drop_thres_de2 > active_pages) {
            buf_cfg->total_buff_drop_thres_de2 = active_pages;
        }
        if (buf_cfg->ingress_max_pages > active_pages) {
            buf_cfg->ingress_max_pages = active_pages;
            buf_cfg->ingress_hysteresis_delta = active_pages/_CALADAN3_SWS_QM_WAR_HYSTERESIS_DELTA;
        }

        if (buf_cfg->egress_max_pages > active_pages) {
            buf_cfg->egress_max_pages = active_pages;
            buf_cfg->egress_hysteresis_delta = active_pages/_CALADAN3_SWS_QM_WAR_HYSTERESIS_DELTA;
        }
        if (buf_cfg->ingress_drop_thres_de1 > active_pages) {
            buf_cfg->ingress_drop_thres_de1 = active_pages;
        }
        if (buf_cfg->ingress_drop_thres_de2 > active_pages) {
            buf_cfg->ingress_drop_thres_de2 = active_pages;
        }
        if (buf_cfg->egress_drop_thres_de1 > active_pages) {
            buf_cfg->egress_drop_thres_de1 = active_pages;
        }
        if (buf_cfg->egress_drop_thres_de2 > active_pages) {
            buf_cfg->egress_drop_thres_de2 = active_pages;
        }
    }
#endif /* _CALADAN3_SWS_LOCKUP_WAR */
    
    regval = 0;
    soc_reg_field_set(unit, QM_BUFFER_CONFIG0r, &regval, 
                      TOTAL_BUFF_MAX_PAGESf,
                      buf_cfg->total_buff_max_pages);
    soc_reg_field_set(unit, QM_BUFFER_CONFIG0r, &regval, 
                      TOTAL_BUFF_HYSTERESIS_DELTAf, 
                      buf_cfg->total_buff_hysteresis_delta);
    SOC_IF_ERROR_RETURN(WRITE_QM_BUFFER_CONFIG0r(unit, regval));

    regval = 0;
    soc_reg_field_set(unit, QM_BUFFER_CONFIG1r, &regval, 
                      TOTAL_BUFF_DROP_THRESH_DE1f, 
                      buf_cfg->total_buff_drop_thres_de1);
    soc_reg_field_set(unit, QM_BUFFER_CONFIG1r, &regval, 
                      TOTAL_BUFF_DROP_THRESH_DE2f, 
                      buf_cfg->total_buff_drop_thres_de2);
    SOC_IF_ERROR_RETURN(WRITE_QM_BUFFER_CONFIG1r(unit, regval));


    regval = 0;
    soc_reg_field_set(unit, QM_BUFFER_CONFIG7r, &regval, NUM_PAGES_RESERVEDf, 
                      buf_cfg->num_pages_reserved);
    SOC_IF_ERROR_RETURN(WRITE_QM_BUFFER_CONFIG7r(unit, regval));

    /* Line/Fabric Interface Thresholds */
    regval = 0;
    soc_reg_field_set(unit, QM_BUFFER_CONFIG2r, &regval, 
                      INGRESS_MAX_PAGESf, buf_cfg->ingress_max_pages);
    soc_reg_field_set(unit, QM_BUFFER_CONFIG2r, &regval, 
                      INGRESS_HYSTERESIS_DELTAf, 
                      buf_cfg->ingress_hysteresis_delta);
    SOC_IF_ERROR_RETURN(WRITE_QM_BUFFER_CONFIG2r(unit, regval));
   
    regval = 0;
    soc_reg_field_set(unit, QM_BUFFER_CONFIG3r, &regval, 
                      INGRESS_DROP_THRESH_DE1f, 
                      buf_cfg->ingress_drop_thres_de1);
    soc_reg_field_set(unit, QM_BUFFER_CONFIG3r, &regval, 
                      INGRESS_DROP_THRESH_DE2f, 
                      buf_cfg->ingress_drop_thres_de2);
    SOC_IF_ERROR_RETURN(WRITE_QM_BUFFER_CONFIG3r(unit, regval));

    regval = 0;
    soc_reg_field_set(unit, QM_BUFFER_CONFIG4r, &regval, 
                      EGRESS_MAX_PAGESf, buf_cfg->egress_max_pages);
    soc_reg_field_set(unit, QM_BUFFER_CONFIG4r, &regval, 
                      EGRESS_HYSTERESIS_DELTAf, 
                      buf_cfg->egress_hysteresis_delta);
    SOC_IF_ERROR_RETURN(WRITE_QM_BUFFER_CONFIG4r(unit, regval));
   
    regval = 0;
    soc_reg_field_set(unit, QM_BUFFER_CONFIG5r, &regval, 
                      EGRESS_DROP_THRESH_DE1f, 
                      buf_cfg->egress_drop_thres_de1);
    soc_reg_field_set(unit, QM_BUFFER_CONFIG5r, &regval, 
                      EGRESS_DROP_THRESH_DE2f,
                      buf_cfg->egress_drop_thres_de2);
    SOC_IF_ERROR_RETURN(WRITE_QM_BUFFER_CONFIG5r(unit, regval));

    regval = 0;
    soc_reg_field_set(unit, QM_FC_CONFIG1r, &regval, 
                      FC_TOTAL_BUFFER_XOFF_THRESHf, 
                      buf_cfg->fc_total_buffer_xoff_thresh);
    SOC_IF_ERROR_RETURN(WRITE_QM_FC_CONFIG1r(unit, regval));

    regval = 0;
    soc_reg_field_set(unit, QM_FC_CONFIG2r, &regval, FC_INGRESS_XOFF_THRESHf, 
                      buf_cfg->fc_ingress_xoff_thresh);
    soc_reg_field_set(unit, QM_FC_CONFIG2r, &regval, FC_EGRESS_XOFF_THRESHf, 
                      buf_cfg->fc_egress_xoff_thresh);
    SOC_IF_ERROR_RETURN(WRITE_QM_FC_CONFIG2r(unit, regval));

    regval = 0;
    soc_reg_field_set(unit, QM_FC_CONFIG0r, &regval, FC_ENABLEf, fc_enable);
    SOC_IF_ERROR_RETURN(WRITE_QM_FC_CONFIG0r(unit, regval));


    regval = 0;
    soc_reg_field_set(unit, QM_BUFFER_CONFIG6r, &regval, 
                      PER_QUEUE_DROP_HYSTERESIS_DELTAf,
                      buf_cfg->per_queue_drop_hysteresis_delta);
    SOC_IF_ERROR_RETURN(WRITE_QM_BUFFER_CONFIG6r(unit, regval));    


    /* Configure and Enable Port queues */
    soc_sbx_caladan3_sws_source_queue_update(unit);

    /* Enable special queues */
    rv = soc_sbx_caladan3_sws_redirect_queues_get(unit, &ing_qid0, 
                                             &ing_qid1, &egr_qid0, &egr_qid1);
    if (SOC_SUCCESS(rv)) {
        if (ing_qid0 >= 0) {
            queue_cfg = queue_info[ing_qid0].squeue_config;
            soc_sbx_caladan3_sws_qm_source_queue_config(unit, ing_qid0, queue_cfg, 1);
        }
    
        if (ing_qid1 >= 0) {
            queue_cfg = queue_info[ing_qid1].squeue_config;
            soc_sbx_caladan3_sws_qm_source_queue_config(unit, ing_qid1, queue_cfg, 1);
        }
    
        if (egr_qid0 >= 0) {
            queue_cfg = queue_info[egr_qid0].squeue_config;
            soc_sbx_caladan3_sws_qm_source_queue_config(unit, egr_qid0, queue_cfg, 1);
        }
        if (egr_qid1 >= 0) {
            queue_cfg = queue_info[egr_qid1].squeue_config;
            soc_sbx_caladan3_sws_qm_source_queue_config(unit, egr_qid1, queue_cfg, 1);
        }
    }

    rv = soc_sbx_caladan3_sws_bubble_queues_get(unit, &ing_qid0, &egr_qid0);
    if (SOC_SUCCESS(rv)) {
        if (ing_qid0 >= 0) {
             queue_cfg = queue_info[ing_qid0].squeue_config;
             soc_sbx_caladan3_sws_qm_source_queue_config(unit, ing_qid0, queue_cfg, 1);
        }
        if (egr_qid0 >= 0) {
             queue_cfg = queue_info[egr_qid0].squeue_config;
             soc_sbx_caladan3_sws_qm_source_queue_config(unit, egr_qid0, queue_cfg, 1);
        }
    }

    rv = soc_sbx_caladan3_sws_app_queues_get(unit, &ing_qid0, &cos);
    if (SOC_SUCCESS(rv)) {
        for (qid = ing_qid0; qid < (ing_qid0+cos); qid++) {
             queue_cfg = queue_info[qid].squeue_config;
             soc_sbx_caladan3_sws_qm_source_queue_config(unit, qid, queue_cfg, 1);
        }
    }

    return SOC_E_NONE;
}

int soc_sbx_caladan3_sws_source_queue_update(int unit)
{
    soc_sbx_caladan3_port_map_t         *port_map;
    soc_sbx_caladan3_sws_queue_info_t   *queue_info;
    int                                 port;
    soc_sbx_caladan3_port_map_info_t    *port_info;
    int                                 qid;
    sws_qm_source_queue_cfg_t           *queue_cfg = NULL;
    soc_sbx_caladan3_queues_t           *qi = NULL;


    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    if (port_map == NULL) {
        return SOC_E_INIT;
    }

    queue_info = soc_sbx_caladan3_sws_queue_info_get(unit);
    if (queue_info == NULL) {
        return SOC_E_INIT;
    }

    for(port = 0; port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {

        /* Enable Fabric side */
        port_info = &port_map->fabric_port_info[port];
        if (!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        qi = &port_info->port_queues;
        SOC_SBX_C3_BMP_ITER_RANGE(qi->dqueue_bmp, SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE,
                                  qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {
            soc_sbx_caladan3_sws_qm_dest_queue_enable(unit, qid, TRUE);
        }

        SOC_SBX_C3_BMP_ITER_RANGE(qi->squeue_bmp, SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE,
                                  qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
            queue_cfg = queue_info[qid].squeue_config;
            soc_sbx_caladan3_sws_qm_source_queue_config(unit, qid, queue_cfg, 
                                                        queue_info[qid].enabled);
        }

        /* Enable Line side */
        port_info = &port_map->line_port_info[port];
        qi = &port_info->port_queues;
        SOC_SBX_C3_BMP_ITER_RANGE(qi->dqueue_bmp, SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE,
                                  qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {
            soc_sbx_caladan3_sws_qm_dest_queue_enable(unit, qid, TRUE);
        }

        SOC_SBX_C3_BMP_ITER_RANGE(qi->squeue_bmp, SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE,
                                  qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
            queue_cfg = queue_info[qid].squeue_config;
            soc_sbx_caladan3_sws_qm_source_queue_config(unit, qid, queue_cfg, 
                                                        queue_info[qid].enabled);
        }

    }

    return SOC_E_NONE;

}


/**
 * Function:
 *     soc_sbx_caladan3_sws_qm_init
 * Purpose:
 *     Initialize QM
 *
 */
int soc_sbx_caladan3_sws_qm_init(int unit)
{
    soc_sbx_caladan3_sws_queue_info_t *queue_info = NULL;
    uint32 regval = 0;
    sws_qm_config_t *pcfg;

    queue_info = soc_sbx_caladan3_sws_queue_info_get(unit);
    if (queue_info == NULL) {
        return SOC_E_INIT;
    }

    if (!SOC_RECONFIG_TDM) {
        SOC_IF_ERROR_RETURN(_soc_sbx_caladan3_sws_qm_common_init(unit)); 
    }

    pcfg = soc_sbx_caladan3_sws_qm_cfg_get(unit);
    if (pcfg != NULL) {
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_sws_qm_config_init(unit, pcfg));
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Cannot locate config \n"), unit));
        return SOC_E_INIT;
    }

    if (SOC_RECONFIG_TDM) {
        return SOC_E_NONE;
    }

    /* Enable the QM block */
    SOC_IF_ERROR_RETURN(READ_QM_CONFIGr(unit, &regval));
    soc_reg_field_set(unit, QM_CONFIGr, &regval, QM_ENABLEf, 1); 
    SOC_IF_ERROR_RETURN(WRITE_QM_CONFIGr(unit, regval));

    return SOC_E_NONE;
}



/**
 * Function:
 *     soc_sbx_caladan3_sws_qm_source_queue_get
 * Purpose:
 *     get the configure of a given source queue
 */
int
soc_sbx_caladan3_sws_qm_source_queue_get(int unit, int qid, sws_qm_source_queue_cfg_t* queue_cfg, int* en)
{
    soc_sbx_caladan3_sws_queue_info_t*  queue_info;
    sws_qm_source_queue_cfg_t*          queue_config = NULL;
    int                                 enable;

    if ((qid < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) || (qid >= SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE))
    {
        return SOC_E_PARAM;
    }

    queue_info = soc_sbx_caladan3_sws_queue_info_get(unit);
    if (queue_info == NULL) {
        return SOC_E_INIT;
    }
    queue_config = queue_info[qid].squeue_config;
    enable = queue_info[qid].enabled;

    if (queue_cfg != NULL)
    {
        sal_memcpy(queue_cfg, queue_config, sizeof(sws_qm_source_queue_cfg_t));
    }
    if (en != NULL)
    {
        *en = enable;
    }

    return SOC_E_NONE;
}


/**
 * Function:
 *     soc_sbx_caladan3_sws_qm_source_queue_set
 * Purpose:
 *     set the configure of a given source queue
 */
int
soc_sbx_caladan3_sws_qm_source_queue_set(int unit, int qid, sws_qm_source_queue_cfg_t* queue_cfg, int en)
{
    soc_sbx_caladan3_sws_queue_info_t*  queue_info;
    sws_qm_source_queue_cfg_t*          queue_config = NULL;
    uint32                              enabled = 0;
    qm_source_queue_config_entry_t      entry;

    if ((qid < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) || (qid >= SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE))
    {
        return SOC_E_PARAM;
    }
    if (queue_cfg == NULL)
    {
        return SOC_E_PARAM;
    }

    enabled = (en > 0) ? 1: 0;

    queue_info = soc_sbx_caladan3_sws_queue_info_get(unit);
    if (queue_info == NULL) {
        return SOC_E_INIT;
    }
    queue_config = queue_info[qid].squeue_config;

    sal_memcpy(queue_config, queue_cfg, sizeof(sws_qm_source_queue_cfg_t));
    queue_info[qid].enabled = enabled;

    SOC_IF_ERROR_RETURN(
        READ_QM_SOURCE_QUEUE_CONFIGm(unit, MEM_BLOCK_ANY, qid, &entry));

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0],
                      DROP_THRESH_DE1f, &queue_cfg->drop_thres_de1);

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0],
                      DROP_THRESH_DE2f, &queue_cfg->drop_thres_de2);

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0],
                      ENABLEf, &enabled);

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0],
                      FLOW_CTRL_THRESHf, &queue_cfg->flow_ctrl_thresh);

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0],
                      MAX_PAGESf, &queue_cfg->max_pages);

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0],
                      MIN_PAGES_DATAf, &queue_cfg->min_pages_data);

    soc_mem_field_set(unit, QM_SOURCE_QUEUE_CONFIGm, &entry.entry_data[0],
                      MIN_PAGES_HEADERf, &queue_cfg->min_pages_header);

    SOC_IF_ERROR_RETURN(
        WRITE_QM_SOURCE_QUEUE_CONFIGm(unit, MEM_BLOCK_ANY, qid, &entry));


    return SOC_E_NONE;
}



#endif
