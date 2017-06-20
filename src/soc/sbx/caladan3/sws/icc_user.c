/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: pr.c,v 1.9.16.14 Broadcom SDK $
 *
 * File:    icc_user.c
 * Purpose:  Routines that setup ICC entries and customer application helpers
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



#define VLAN_PCP_MAX_VAL 8
#define VLAN_TCI_PCP_SHIFT 5
#define VLAN_FRAME_PCP_BYTE_LOC  14  /* zero-based */
#define VLAN_FRAME_TPID_BYTE_LOC  12  /* zero-based */
#define PR_ICC_NUM_LOOKUP_STAGES 3

static int arad_header_programmed = 0;
static int sirius_header_programmed = 0;

int sws_pr_icc_table_entry_alloc(
    int unit, int pr_instance, int spread, int *entry_start_idx);

int sws_pr_icc_table_entry_free(
    int unit, int pr_instance, int base_entry_id, int spread);



static int pr_icc_is_programmed[SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE][MAX_LINE_FABRIC_ID];

int sws_pr_icc_is_programmed(int unit, int pr_instance, int side_is_fabric)
{
    pr_instance &= 1;
    assert(side_is_fabric <= 1);
    return pr_icc_is_programmed[pr_instance][side_is_fabric];
}


void sws_pr_icc_set_is_programmed(int unit, int pr_instance, int side_is_fabric, int programmed)
{
    pr_instance &= 1;
    assert(side_is_fabric <= 1);
    pr_icc_is_programmed[pr_instance][side_is_fabric] = programmed;
}


/*
 * Function:
 *     soc_sbx_calandan3_sws_icc_state_key_unpack
 * Purpose:
 *     unpack state value to state info 
 *      input: state;   output: state_ino
 */
int soc_sbx_calandan3_sws_icc_state_key_unpack(soc_sbx_calandan3_icc_state_info_t* state_info, uint8* state)
{
    int     rc = SOC_E_PARAM;

    if ((state_info != NULL) && (state != NULL))
    {
        rc = SOC_E_NONE;
        state_info->port      = state[0];
        state_info->profile   = 0;        /* now is not used */
        state_info->stage     = state[2] & 0xF;
        state_info->flag      = (state[2] & 0xF0) >> 4;
    }

    return rc;
}

/*
 * Function:
 *     soc_sbx_calandan3_sws_icc_state_key_pack
 * Purpose:
 *     pack state info to state value 
 *      input: state_ino;   output: state
 */
int soc_sbx_calandan3_sws_icc_state_key_pack(soc_sbx_calandan3_icc_state_info_t* state_info, uint8* state)
{
    int     rc = SOC_E_PARAM;

    if ((state_info != NULL) && (state != NULL))
    {
        rc = SOC_E_NONE;
        state[0]    = state_info->port;
        state[1]    = 0;
        state[2]    = ((state_info->flag & 0xF) << 4) | (state_info->stage & 0xF);
    }

    return rc;
}


/*
 * Function:
 *     soc_sbx_calandan3_sws_icc_state_result_get
 * Purpose:
 *     generage swc icc tcam result field state value
 * XXXTBC  WARNING: This function will no longer work correctly if B0
 * PR_ICC functionality is turned on since bits 23:19 are now defined as 
 * capture and hold bits.
 * State bits need to be redefined for B0 compatibility. "flag" bits are currently
 * used for L2 TPID programming on A0.
 *
 * Also note this function is coupled with functions: 
 * soc_sbx_calandan3_sws_icc_state_key_pack
 * soc_sbx_calandan3_sws_icc_state_key_unpack
 * soc_sbx_caladan3_sws_pr_icc_tcam_dump
 * cmd_sbx_caladan3_sws_icc_set
 */
uint32 soc_sbx_calandan3_sws_icc_state_result_get(uint32 flag, uint32 stage, uint32 port)
{
    uint32  rc = 0;

    rc = ((flag & 0xF) << 20) | ((stage & 0xF) << 16) | port;
    return rc;
}


int
sws_pr_icc_port_match_rule_set(int unit, int pr, int entry_valid,
                               int base_port_with_channel, 
                               int squeue,
                               int index_spread,
                               int pr_icc_table_index)
{
    int stage;
    int max;
    uint32 regval = 0;
    uint8 icckey[26] = {0}, iccmsk[26] = {0};
    uint8 state[PR_ICC_NUM_LOOKUP_STAGES] = {0}, smask[PR_ICC_NUM_LOOKUP_STAGES] = {0};
    soc_sbx_caladan3_pr_icc_lookup_data_t data;

    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_ICC_CONFIG0r, 
                            pr, 0, &regval));
    max = soc_reg_field_get(unit, PR_ICC_CONFIG0r, regval, LOOKUPS_REQUIREDf);

    assert(max == PR_ICC_NUM_LOOKUP_STAGES);
    assert(index_spread == PR_ICC_NUM_LOOKUP_STAGES);

    for (stage = 0; stage < index_spread; stage++, pr_icc_table_index++) {

        sal_memset(&data, 0, sizeof(data));
        sal_memset(&icckey, 0, sizeof(icckey));
        sal_memset(&iccmsk, 0, sizeof(iccmsk));
        state[0] = state[1] = state[2] = 0;
        smask[0] = smask[1] = smask[2] = 0;

        if (entry_valid == SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID) {
            state[0] = base_port_with_channel;
            state[2] = stage;
            smask[0] = 0x3F;
            smask[2] = 0xF;
            
            if (stage == index_spread - 1) {
                data.last = 1;
                data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP;
                data.queue = squeue;
            } else {
                data.last = 0;
                data.state = ((stage + 1) << 16) | (base_port_with_channel);
            }
        }

        soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, 
                                                 pr_icc_table_index, 
                                                 entry_valid,
                                                 icckey, iccmsk, 
                                                 state, smask, 
                                                 &data);

    }
    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_caladan3_sws_pr_tcam_match_all
 * Purpose:
 *     Initialize PR TCAM match all default entry
 */
void
soc_sbx_caladan3_sws_pr_tcam_match_all(int unit, int pr, int idx, int valid,
                                  soc_sbx_caladan3_pr_icc_lookup_data_t *data)
{
    uint8 iccKey[26], iccMsk[26];
    uint8 state[PR_ICC_NUM_LOOKUP_STAGES], smask[PR_ICC_NUM_LOOKUP_STAGES];
    sal_memset(iccKey, 0, sizeof(iccKey));
    sal_memset(iccMsk, 0, sizeof(iccMsk));
    sal_memset(state, 0, sizeof(state));
    sal_memset(smask, 0, sizeof(smask));
    soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, idx, valid,
                                             iccKey, iccMsk,
                                             state, smask, data);
}




void
soc_sbx_caladan3_sws_pr_icc_program_arad_header(int unit, int force)
{
    int need_entries = 0, need_matchall = 0, port = 0, cos = 0;
    soc_sbx_caladan3_pr_icc_lookup_data_t data;
    uint8 icckey[26] = {0}, iccmsk[26] = {0};
    soc_sbx_caladan3_queues_t qtemp, *queues = NULL;
    uint8 state[PR_ICC_NUM_LOOKUP_STAGES] = {0}, smask[PR_ICC_NUM_LOOKUP_STAGES] = {0};
    int qid, rv = SOC_E_NONE;
    int num_stages = 1;
    int icc_table_index = 0;
    int total_q_count;
    int index_spread = 0;
    int current_index = 0;
    int last_index_used = 0;

    PBMP_ITER(PBMP_HG_ALL(unit), port) {
        if (!soc_sbx_caladan3_is_line_port(unit, port)) {
            /* Found a HG port on Fabric side, TCAM entries already present,
             * do not waste entries 
             */
            if ((force) && (sws_pr_icc_is_programmed(unit, PR1_INSTANCE, 1) > 0)) {
                /* Debug scenario */
                return;
            }
            need_entries = 1;
            break;
        }
    }

    if (need_entries || force) {
        SOC_PBMP_ITER(PBMP_PORT_ALL(unit), port) {
            cos = 0;
            total_q_count = 1;  /*start by assuming non COS LPORT; i.e. single Q per lport*/

            icc_table_index = current_index = index_spread = 0;

            if (!soc_sbx_caladan3_is_line_port(unit, port)) {
                /* Skip fabric ports */
                continue;
            }
            rv = soc_sbx_caladan3_port_queues_get(unit, port, 1, &queues);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed getting queue info for port %d\n"),
                           unit, port));
                return;
            }
            
            /* Total COS count is required for allocation of table indices 
             * since the Qs are sequentially consecutive (by convention) for an LPORT.
             */            
            if (queues->num_squeue != 0) {
                /* num_squeue  will be zero if we are processing a single Q
                 * for this port. If this is a COS-based lport it will be non-zero.
                 */
                total_q_count = queues->num_squeue;
            }
        
            /* If this line_lport is already programmed in the PR_ICC then skip it.
             * We don't want to leak ICC memory by orphaning existing ones.
             * Note: An alternative allocation strategy could be to 
             * delete and re-alloc.
             */
            if (queues->squeue_base_pr_icc_table_index_valid) {
                continue;
            }
            num_stages = 1;  /* Historically only used one stage but Should be using 3 like Sirius side*/
            index_spread = num_stages * total_q_count;
            
            if (sws_pr_icc_table_entry_alloc(unit, PR1_INSTANCE, 
                                             index_spread,
                                             &icc_table_index) != SOC_E_NONE) {
                assert(1);
            } else {
                queues->squeue_base_pr_icc_table_index = icc_table_index;
                queues->squeue_base_pr_icc_table_spread = index_spread;
                queues->squeue_base_pr_icc_table_index_valid = 1;
            }
            current_index = icc_table_index;
            last_index_used = current_index;
            SOC_SBX_C3_BMP_CLEAR(qtemp.squeue_bmp, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES);


            SOC_SBX_C3_BMP_ITER_RANGE(queues->squeue_bmp, 
                                      SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE,
                                      qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {

                if (SOC_SBX_C3_BMP_MEMBER_IN_RANGE(qtemp.squeue_bmp, 
                                                   SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE, 
                                                   qid)) {
                   continue;
                }
                SOC_SBX_C3_BMP_ADD(qtemp.squeue_bmp, qid % SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);

                sal_memset(&icckey, 0, sizeof(icckey));
                sal_memset(&iccmsk, 0, sizeof(iccmsk));
                sal_memset(&data, 0, sizeof(data));

                
                icckey[2] = port;
                iccmsk[2] = 0xff;
                icckey[14] = cos << 3; 
                iccmsk[14] = cos > 0 ? 0x38 : 0;

                data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP;
                data.last = 1;
                data.queue = qid;
                soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, PR1_INSTANCE,
                                                         current_index,
                                                         SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID,
                                                         icckey, iccmsk, 
                                                         state, smask, 
                                                         &data);
                cos++;
            }
            
            need_matchall = 1;
            arad_header_programmed = 1;
        }
    }

    if (need_matchall) {
        if (last_index_used < 255) {
            /* MATCH ALL entry:
             * Redirect packets to CMIC in future, allow override using soc parameter
             * Currently set to Drop.
             */
            port = CMIC_PORT(unit);
            rv = soc_sbx_caladan3_port_queues_get(unit, port, 1, &queues);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed getting queue info for port %d\n"),
                           unit, port));
                return;
            }
            
            data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP;
            data.drop  = 1;
            data.last = 1;
            data.queue = queues->squeue_base;
            soc_sbx_caladan3_sws_pr_tcam_match_all(unit, PR1_INSTANCE,
                                                   255, 0x3, &data);
            /* Set ICC state */
            soc_sbx_caladan3_pr_set_icc_state(unit, 
                                              PR1_INSTANCE, SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE);

            sws_pr_icc_set_is_programmed(unit, PR1_INSTANCE, FABRIC_SIDE, 1);
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "WARNING: Unit %d: Cannot add MATCH ALL entry"
                                 " in PR1 TCAM, potentially fatal\n"), unit));
        }
    }
}

#ifdef NOT_CURRENTLY_CALLED_ANYWHERE
void
soc_sbx_caladan3_sws_pr_icc_program_loopback_header(int unit, int force)
{
    int need_entries = 0, need_matchall = 0, idx = 0, port = 0, cos = 0;
    soc_sbx_caladan3_pr_icc_lookup_data_t data;
    uint8 icckey[26] = {0}, iccmsk[26] = {0};
    soc_sbx_caladan3_queues_t qtemp, *queues = NULL;
    uint8 state[PR_ICC_NUM_LOOKUP_STAGES] = {0}, smask[PR_ICC_NUM_LOOKUP_STAGES] = {0};
    int qid, rv = 0;


    PBMP_ITER(PBMP_HG_ALL(unit), port) {
        if (!soc_sbx_caladan3_is_line_port(unit, port)) {
            /* Found a HG port on Fabric side, TCAM entries already present,
             * do not waste entries 
             */
            if ((force) && (sws_pr_icc_reserved_rules_get(unit, PR1_INSTANCE) > 0)) {
                /* Debug scenario */
                return;
            }
            need_entries = 1;
            break;
        }
    }

    idx=0;
    if (need_entries || force) {
        SOC_PBMP_ITER(PBMP_PORT_ALL(unit), port) {
            cos = 0;
            if (!soc_sbx_caladan3_is_line_port(unit, port)) {
                /* Skip fabric ports */
                continue;
            }
            rv = soc_sbx_caladan3_port_queues_get(unit, port, 1, &queues);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed getting queue info for port %d\n"),
                           unit, port));
                return;
            }
            idx = sws_pr_icc_reserved_rules_get(unit, PR1_INSTANCE);
            SOC_SBX_C3_BMP_CLEAR(qtemp.squeue_bmp, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES);
            SOC_SBX_C3_BMP_ITER_RANGE(queues->squeue_bmp, 
                                      SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE,
                                      qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
                if (SOC_SBX_C3_BMP_MEMBER_IN_RANGE(qtemp.squeue_bmp, 
                                                   SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE, 
                                                   qid)) {
                   continue;
                }
                SOC_SBX_C3_BMP_ADD(qtemp.squeue_bmp, qid % SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);
                sal_memset(&data, 0, sizeof(data));
                sal_memset(&icckey, 0, sizeof(icckey));
                sal_memset(&iccmsk, 0, sizeof(iccmsk));

                icckey[2] = (port << 3) & 0xff;
                iccmsk[2] = 0xf8;
                icckey[3] = (port >> 5);
                iccmsk[3] = 0x1;
                icckey[10] = cos << 3; 
                iccmsk[10] = cos > 0 ? 0x38 : 0;
                data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP;
                data.last = 1;
                data.queue = qid;
                need_matchall = 1;
                soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, PR1_INSTANCE,
                                                         idx,
                                                         SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID,
                                                         icckey, iccmsk, 
                                                         state, smask, 
                                                         &data);
                idx++;
                cos++;
            }
            sws_pr_icc_reserved_rules_set(unit, PR1_INSTANCE, idx);
        }
    }
    if (need_matchall) {
        if (idx < 255) {
            /* MATCH ALL entry:
             * Redirect packets to CMIC in future, allow override using soc parameter
             * Currently set to Drop.
             */
            port = CMIC_PORT(unit);
            rv = soc_sbx_caladan3_port_queues_get(unit, port, 1, &queues);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed getting queue info for port %d\n"),
                           unit, port));
                return;
            }
            data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP;
            data.drop  = 1;
            data.last = 1;
            data.queue = queues->squeue_base;
            soc_sbx_caladan3_sws_pr_tcam_match_all(unit, PR1_INSTANCE, 
                                                   255, 0x3, &data);
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "WARNING: Unit %d: Cannot add"
                                 " MATCH ALL entry in PR 1 TCAM, potentially fatal\n"), unit));
        }
        /* Set ICC state */
        soc_sbx_caladan3_pr_set_icc_state(unit, 
            PR1_INSTANCE, SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE);
    }
}
#endif


/*
 * In terms of behavior of this function with respect to removing entries from the
 * PR_ICC table, the following is relevant.
 * The caller needs to remove by Q id since COS Qing may be in effect. 
 * Q information is required to know how many table entries are in use 
 * for this particular lport/Q combination.
 * In terms of other parts of software implementation Q are always removed 
 * BEFORE LPORTs. Even though the key to this table is LPORT, due to COS Qing
 * we find entry of interest via Q iteration.
 */
void
soc_sbx_caladan3_sws_pr_icc_program_sirius_header(int unit, int force, 
                                                  int remove_entry, int q_to_remove,
                                                  int dont_add_resv_ports)
{
    int need_entries = 0, need_matchall = 0, hg_lport = 0, line_lport = 0,cos = 0;
    soc_sbx_caladan3_pr_icc_lookup_data_t data;
    uint8 icckey[26] = {0}, iccmsk[26] = {0};
    soc_sbx_caladan3_queues_t qtemp, *queues = NULL;
    uint8 state[PR_ICC_NUM_LOOKUP_STAGES] = {0}, smask[PR_ICC_NUM_LOOKUP_STAGES] = {0};
    int qid, rv = 0;
    uint32 regval = 0;
    int num_stages = 3;
    int port_index = 0;
    soc_sbx_caladan3_port_map_info_t *fab_port_info = NULL;
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    int stage;
    int icc_table_index = 0;
    int entry_removed = 0;
    int entry_valid = SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID;
    int add_entries = 1;
    int total_q_count;
    int index_spread = 0;
    int current_index = 0;
    int line_port_index = 0;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    add_entries = !remove_entry;  /*Within this function these are mutually exclusive per invocation!*/

    PBMP_ITER(PBMP_HG_ALL(unit), hg_lport) {
        if (!soc_sbx_caladan3_is_line_port(unit, hg_lport)) {

            if (add_entries) {
                need_entries = 1;
            }
            if ((rv = soc_sbx_caladan3_sws_pbmp_to_fab_port(unit, hg_lport, &port_index)) != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "*** Error: Failed getting port index info\n")));
                return;
            }
            fab_port_info = &port_map->fabric_port_info[port_index];
            break;
        }
    }

    if (soc_reg32_get(unit, PR_ICC_CONFIG0r, PR1_INSTANCE, 0, &regval) != 0) {

                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "*** Error: Failed getting PR_ICC info\n")));
    }

    num_stages = soc_reg_field_get(unit, PR_ICC_CONFIG0r, regval, LOOKUPS_REQUIREDf);
    /* There are numerous places that assume we are always performing a 3 stage lookup.
     * Even HW expects this will be the case even though some old docs may 
     * suggest otherwise (e.g. via a global config register). 
     * ALWAYS use 3 or beware.
     */
    assert(num_stages == PR_ICC_NUM_LOOKUP_STAGES);

    /*
     * It is entirely possible a single Q can be bound to multiple lports (e.g. XLPORTs).
     * Therefore, if we are removing a Q we iterate over all lports to find multiple
     * instances of this Q being referenced.
     */
    if (need_entries || force || remove_entry) {

        SOC_PBMP_ITER(PBMP_PORT_ALL(unit), line_lport) {

            cos = 0;
            total_q_count = 1;  /*start by assuming non COS LPORT; i.e. single Q per lport*/

            icc_table_index = current_index = index_spread = 0;

            if (!soc_sbx_caladan3_is_line_port(unit, line_lport)) {
                /* Skip fabric ports */
                continue;
            }

            rv = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, line_lport, &line_port_index);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d: "
                                      "*** Error: Failed to get port queue info for port %d\n"),
                           unit, line_lport));
                return;
            }

            /* There are cases where we require line ports to be programmed 
             * but not the "reserved" line ports.
             */
            if (add_entries && dont_add_resv_ports && 
                line_port_index > port_map->first_reserved_port) {
                continue;
            }

            /*Get the queues for the fabric side of this line side lport*/
            rv = soc_sbx_caladan3_port_queues_get(unit, line_lport, FABRIC_SIDE, &queues);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                "*** Error: Failed getting queue info for lport %d\n"),
                                line_lport));
                return;
            }

            if (remove_entry && queues->squeue_base != q_to_remove) {
                /*The Q to remove does not exist on this lport.  Move on to next lport.*/
                continue;
            }
 
            /* Total COS count is required for allocation of table indices 
             * since the Qs are sequentially consecutive (by convention) for an LPORT.
             */            
            if (queues->num_squeue != 0) {
                /* num_squeue  will be zero if we are processing a single Q
                 * for this port. If this is a COS-based lport it will be non-zero.
                 */
                total_q_count = queues->num_squeue;
            }
        
            if (add_entries) {

                /* If this line_lport is already programmed in the PR_ICC then skip it.
                 * We don't want to leak ICC memory by orphaning existing ones.
                 * Note: An alternative allocation strategy could be to 
                 * delete and re-alloc.
                 */
                if (queues->squeue_base_pr_icc_table_index_valid) {
                    continue;
                }

#if PRR_ICC_DEBUG
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d: "
                                      "*** %s ADD  line_lport: %d   queue: %d  add: %d\n"),
                           unit, __func__, line_lport, queues->squeue_base, 
                           add_entries ));
#endif    
                index_spread = num_stages * total_q_count;

                if (sws_pr_icc_table_entry_alloc(unit, PR1_INSTANCE, 
                                                 index_spread,
                                                 &icc_table_index) != SOC_E_NONE) {
                    assert(1);
                } else {
                    queues->squeue_base_pr_icc_table_index = icc_table_index;
                    queues->squeue_base_pr_icc_table_spread = index_spread;
                    queues->squeue_base_pr_icc_table_index_valid = 1;
                }
            }

            /* 
             * Lports with multiple Qs. In this case ONLY the base Q must be passed
             * to this function.  
             */
            if (remove_entry) {

#if PRR_ICC_DEBUG
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d: "
                                      "*** %s  REM line_lport: %d   queue: %d  add: %d\n"),
                           unit, __func__, line_lport, queues->squeue_base, 
                           add_entries ));
#endif
                entry_valid = SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_INVALID;

                icc_table_index = queues->squeue_base_pr_icc_table_index;
                index_spread = queues->squeue_base_pr_icc_table_spread;

                if(sws_pr_icc_table_entry_free(unit, PR1_INSTANCE, 
                                               queues->squeue_base_pr_icc_table_index,
                                               queues->squeue_base_pr_icc_table_spread) != SOC_E_NONE) {
                    assert(1);
                } else {
                    queues->squeue_base_pr_icc_table_index = 0;
                    queues->squeue_base_pr_icc_table_spread = 0;
                    queues->squeue_base_pr_icc_table_index_valid = 0;
                }
            }

            current_index = icc_table_index;

            SOC_SBX_C3_BMP_CLEAR(qtemp.squeue_bmp, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES);
            
            SOC_SBX_C3_BMP_ITER_RANGE(queues->squeue_bmp, 
                                      SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE,
                                      qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {

                if (SOC_SBX_C3_BMP_MEMBER_IN_RANGE(qtemp.squeue_bmp, 
                                                   SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE, 
                                                   qid)) {
                    continue;
                }
                SOC_SBX_C3_BMP_ADD(qtemp.squeue_bmp, qid % SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);

                /*
                 * Originally, only a single entry was programmed for the fabric side.
                 * The configuration of PR_ICC in HW global register specified 3 entries for
                 * a complete lookup.  Thus, a single entry would be recycled and provide 
                 * the required lookup (Q as return value).  This was a shortcut method,
                 * that appeared to work on A0 but does NOT work on B0.
                 *
                 * Borrowing warp cores from PR1 for line side ports (line side ports 
                 * traditionally only ever existed on PR0), on B0 and later chipsets,
                 * has proven this. So now 3 individual entries are programmed for
                 * all chip revs.
                 *
                 * Note this is currently ONLY instrumented for SIRIUS header and NOT for
                 * ARAD header since the TDMs requiring this functionality are ONLY
                 * on SIRIUS-based fabric.
                 */
                if (fab_port_info) {

                    for (stage = 0; stage < num_stages; stage++, current_index++) {

                        sal_memset(&data, 0, sizeof(data));
                        sal_memset(&icckey, 0, sizeof(icckey));
                        sal_memset(&iccmsk, 0, sizeof(iccmsk));
                        state[0] = state[1] = state[2] = 0;
                        smask[0] = smask[1] = smask[2] = 0;

                        if (add_entries) {

                            assert(icc_table_index != 0xFF);
                            state[0] = fab_port_info->base_port;  /*The HG fabric port*/
                            state[2] = stage;
                            /* For fabric side ILKN TDMs, if the port field is zero
                             * then having any non-zero mask
                             * for port will result in no match; resulting in 
                             * incorrect lookup behavior. 
                             */
                            if (fab_port_info->base_port != 0) {
                                smask[0] = 0x3F;
                            } else {
                                smask[0] = 0x0;
                            }
                            smask[2] = 0xF;

                            /* icckey is the "packet" field as referred to in
                             * HW spec and in swsiccget command
                             */
                            icckey[4] = line_lport;  /* Egress port as found in ERH header inserted by uCode*/
                            iccmsk[4] = 0xff;
                            icckey[8] = cos << 3; 
                            iccmsk[8] = cos > 0 ? (0x7 << 3) : 0;

                            if (stage == num_stages - 1) {
                                data.last = 1;
                                data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP;
                                data.queue = qid;
                            } else {
                                data.last = 0;
                                data.state = ((stage + 1) << 16) | (fab_port_info->base_port);
                            }

                        } else {
                            entry_removed = 1;
                        }

                        soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, PR1_INSTANCE, 
                                                                 current_index,
                                                                 entry_valid,
                                                                 icckey, iccmsk, 
                                                                 state, smask, 
                                                                 &data);                        
                    }

                } else {
                    /* 
                     * This case is for the "forced" option, whereby the fabric entries 
                     * are required for setting such as
                     * "g3p1GlobalSet higig_loop_enable 1" for debug.  In this case,
                     * this setting may be present but no HG fabric is actually present.
                     * The TDM could indicate ARAD header for support of ILKN fabric
                     * but the "higig_loop_enable" would essentially override.
                     * Also note that the current implementation is to use uCode
                     * (g3p1a vs. g3p1) to infer whether ARAD header or SIRIUS header is 
                     * to be programmed.
                     * 
                     * Below logic assumes we are NOT using ILKN on fabric side while 
                     * borrowing warp cores from PR1 AND using the "higig_loop_enable"
                     * debug enabled.  Borrowing warp cores as per above means
                     * we require all 3 entries per port to be able to differentiate.
                     *
                     * Having an ILKN based TDM but setting "g3p1GlobalSet higig_loop_enable 1"
                     * is a debug only scenario.
                     */
                    for (stage = 0; stage < num_stages; stage++, current_index++) {

                        sal_memset(&data, 0, sizeof(data));
                        sal_memset(&icckey, 0, sizeof(icckey));
                        sal_memset(&iccmsk, 0, sizeof(iccmsk));
                        state[0] = state[1] = state[2] = 0;
                        smask[0] = smask[1] = smask[2] = 0;

                        if (add_entries) {
                            state[0] = 0;
                            state[2] = stage;

                            smask[0] = 0x0;
                            smask[2] = 0xF;

                            /*icckey is the "packet" field as referred to in HW spec and in swsiccget command*/
                            icckey[4] = line_lport;  /*The egress port as found in ERH header inserted by uCode*/
                            iccmsk[4] = 0xff;
                            icckey[8] = cos << 3; 
                            iccmsk[8] = cos > 0 ? (0x7 << 3) : 0;

                            if (stage == num_stages - 1) {
                                data.last = 1;
                                data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP;
                                data.queue = qid;
                            } else {
                                data.last = 0;
                                data.state = ((stage + 1) << 16);
                            }
                        } else {
                            entry_removed = 1;
                        }

                        soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, PR1_INSTANCE, 
                                                                 current_index,
                                                                 entry_valid,
                                                                 icckey, iccmsk, 
                                                                 state, smask, 
                                                                 &data);
                    }
                }

                cos++;
                sirius_header_programmed = 1;
                if (add_entries) {
                    need_matchall = 1;
                }
            }

        }
    }

    assert(current_index == icc_table_index + index_spread);

    if (need_matchall && add_entries) {

            /* MATCH ALL entry:
             * Redirect packets to CMIC in future, allow override using soc parameter
             * Currently set to Drop.
             */
            line_lport = CMIC_PORT(unit);
            rv = soc_sbx_caladan3_port_queues_get(unit, line_lport, 1, &queues);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                "*** Error: Failed getting queue info for lport %d\n"),
                                line_lport));
                return;
            }
            sal_memset(&data, 0, sizeof(data));
            data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP;
            data.drop  = 1;
            data.last = 1;
            data.queue = queues->squeue_base;
            soc_sbx_caladan3_sws_pr_tcam_match_all(unit, PR1_INSTANCE, 
                                                   255, 0x3, &data);
            /* Set ICC state */
            soc_sbx_caladan3_pr_set_icc_state(unit, 
                                              PR1_INSTANCE, SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE);

            sws_pr_icc_set_is_programmed(unit, PR1_INSTANCE, FABRIC_SIDE, 1);
    }

    if (remove_entry && !entry_removed) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d: "
                              "ERROR: %s Failed to remove Q %d\n"), unit, __func__, q_to_remove));
    }
}



int soc_sbx_caladan3_remove_pr_icc_entries_for_queue(int unit, int q_to_rem)
{
    int remove_entry = 1;

    if (q_to_rem >= SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE &&
        q_to_rem < SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE) {

        if (soc_sbx_caladan3_sws_check_icc_state(unit, PR1_INSTANCE) !=
            SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS) {
            int force = 0;

            
            assert(!arad_header_programmed);

            assert(sirius_header_programmed);

            soc_sbx_caladan3_sws_pr_icc_program_sirius_header(
                unit, force, remove_entry, q_to_rem, 1);

        }
    }

    if (q_to_rem >= SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE &&
        q_to_rem < SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE) {

        if (soc_sbx_caladan3_sws_check_icc_state(unit, PR0_INSTANCE) !=
            SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS) {

            soc_sbx_caladan3_sws_pr_icc_program_port_match_entries(unit, 
                                                                   remove_entry, 
                                                                   q_to_rem, 0);
        }
    }
    return SOC_E_NONE;
}



#if 0
#define SWS_ICC_STATE_FLAG_TAG      0x1
#define SWS_ICC_STATE_FLAG_UNTAG    0x2
static int _soc_sws_pr_icc_tpid_tcam_entry_set(int unit, uint32* tpid, int tpid_num)
{
    int     idx, ii, pr;
    uint8   icckey[26] = {0}, iccmsk[26] = {0};
    uint8   state[4] = {0}, smask[4] = {0};
    uint32  regval = 0, max, stage, flag;
    soc_sbx_caladan3_pr_icc_lookup_data_t   data;
    soc_sbx_calandan3_icc_state_info_t      state_info, state_mask;


    pr = PR0_INSTANCE;
    idx = sws_pr_icc_reserved_rules_get(unit, pr);
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_ICC_CONFIG0r, 
                            pr, 0, &regval));
    max = soc_reg_field_get(unit, PR_ICC_CONFIG0r, regval, LOOKUPS_REQUIREDf);

    /* add tag tcam rule entry */
    for (ii = 0; ii < tpid_num + 1; ii++, idx++)
    {
        sal_memset(&data, 0, sizeof(data));
        sal_memset(&icckey, 0, sizeof(icckey));
        sal_memset(&iccmsk, 0, sizeof(iccmsk));
        sal_memset(&state_info, 0, sizeof(state_info));
        sal_memset(&state_mask, 0, sizeof(state_mask));

        flag = SWS_ICC_STATE_FLAG_UNTAG;
        if (ii < tpid_num)
        {
            flag = SWS_ICC_STATE_FLAG_TAG;
            icckey[VLAN_FRAME_TPID_BYTE_LOC] = (uint8)((tpid[ii] & 0xFF00) >> 8);
            icckey[VLAN_FRAME_TPID_BYTE_LOC + 1] = (uint8)(tpid[ii] & 0xFF);
            iccmsk[VLAN_FRAME_TPID_BYTE_LOC] = 0xFF;
            iccmsk[VLAN_FRAME_TPID_BYTE_LOC + 1] = 0xFF;
        }

        state_info.stage = 0;
        state_mask.stage = 0xF;
        soc_sbx_calandan3_sws_icc_state_key_pack(&state_info, state);
        soc_sbx_calandan3_sws_icc_state_key_pack(&state_mask, smask);

        data.last = 0;
        data.state = soc_sbx_calandan3_sws_icc_state_result_get(flag, 1, 0);

        soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, idx, 
                      SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID,
                      icckey, iccmsk, 
                      state, smask, 
                      &data);
    }

    /* add middle stage rules */

    /* Note this for loop is unnecessary since there are ALWAYS 
     * 3 stages used in ICC lookup pipeline. This means this "for" block 
     * is executed only ONCE.  More than 3 stages could be 
     * performed but this would reduce line rate due to HW limitations.
     * The "for" loop is left in place however for future support of more than
     * 3 lookups.
     */
    for (stage = 1; stage < max-1; stage++)
    {
        sal_memset(&data, 0, sizeof(data));
        sal_memset(&icckey, 0, sizeof(icckey));
        sal_memset(&iccmsk, 0, sizeof(iccmsk));
        sal_memset(&state_info, 0, sizeof(state_info));
        sal_memset(&state_mask, 0, sizeof(state_mask));

        state_info.stage = stage;
        state_mask.stage = 0xF;
        state_info.flag = SWS_ICC_STATE_FLAG_TAG;
        state_mask.flag = 0xF;
        soc_sbx_calandan3_sws_icc_state_key_pack(&state_info, state);
        soc_sbx_calandan3_sws_icc_state_key_pack(&state_mask, smask);
        data.state = soc_sbx_calandan3_sws_icc_state_result_get(SWS_ICC_STATE_FLAG_TAG, 2, 0);

        soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, idx, 
                      SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID,
                      icckey, iccmsk, 
                      state, smask, 
                      &data);
        idx++;

        state_info.flag = SWS_ICC_STATE_FLAG_UNTAG;
        state_mask.flag = 0xF;
        soc_sbx_calandan3_sws_icc_state_key_pack(&state_info, state);
        soc_sbx_calandan3_sws_icc_state_key_pack(&state_mask, smask);
        data.state = soc_sbx_calandan3_sws_icc_state_result_get(SWS_ICC_STATE_FLAG_UNTAG, 2, 0);

        soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, idx, 
                      SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID,
                      icckey, iccmsk, 
                      state, smask, 
                      &data);
        idx++;
        
    }

    sws_pr_icc_reserved_rules_set(unit, pr, idx);

    return SOC_E_NONE;
}



/*
 * This function is inherently coupled to the 
 * _soc_sws_pr_icc_tpid_tcam_entry_set() function.
 */
static int _soc_sws_pr_icc_dot1p_queue_map_tcam_entry_set(int unit, uint8* dot1p_queue_map)
{
    int     idx, vlan_pcp, pr;
    uint8   icckey[26] = {0}, iccmsk[26] = {0};
    uint8   state[4] = {0}, smask[4] = {0};
    uint32  regval = 0, max;
    soc_sbx_caladan3_pr_icc_lookup_data_t   data;
    soc_sbx_calandan3_icc_state_info_t      state_info, state_mask;

    pr = PR0_INSTANCE;
    idx = sws_pr_icc_reserved_rules_get(unit, pr);
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_ICC_CONFIG0r, 
                            pr, 0, &regval));
    max = soc_reg_field_get(unit, PR_ICC_CONFIG0r, regval, LOOKUPS_REQUIREDf);
    /* add tag tcam rule entry */
    for (vlan_pcp = 0; vlan_pcp < VLAN_PCP_MAX_VAL; vlan_pcp++, idx++)
    {
        sal_memset(&data, 0, sizeof(data));
        sal_memset(&icckey, 0, sizeof(icckey));
        sal_memset(&iccmsk, 0, sizeof(iccmsk));
        sal_memset(&state_info, 0, sizeof(state_info));
        sal_memset(&state_mask, 0, sizeof(state_mask));

        state_info.stage    = max - 1;
        state_info.flag     = SWS_ICC_STATE_FLAG_TAG;
        state_mask.stage    = 0xF;
        state_mask.flag     = 0xF;
        soc_sbx_calandan3_sws_icc_state_key_pack(&state_info, state);
        soc_sbx_calandan3_sws_icc_state_key_pack(&state_mask, smask);

        icckey[VLAN_FRAME_PCP_BYTE_LOC] = (uint8)(vlan_pcp << VLAN_TCI_PCP_SHIFT);
        iccmsk[VLAN_FRAME_PCP_BYTE_LOC] = 0xE0;

        data.last = 1;
        data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_INDEXED;
        data.queue = dot1p_queue_map[vlan_pcp];
        data.state = soc_sbx_calandan3_sws_icc_state_result_get(SWS_ICC_STATE_FLAG_TAG, 1, 0);

        soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, idx, 
                      SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID,
                      icckey, iccmsk, 
                      state, smask, 
                      &data);
    }

    /* add untag tcam rule entry */
    sal_memset(&data, 0, sizeof(data));
    sal_memset(&icckey, 0, sizeof(icckey));
    sal_memset(&iccmsk, 0, sizeof(iccmsk));
    sal_memset(&state_info, 0, sizeof(state_info));
    sal_memset(&state_mask, 0, sizeof(state_mask));
    state_info.stage    = max - 1;
    state_info.flag     = SWS_ICC_STATE_FLAG_UNTAG;
    state_mask.stage    = 0xF;
    state_mask.flag     = 0xF;
    soc_sbx_calandan3_sws_icc_state_key_pack(&state_info, state);
    soc_sbx_calandan3_sws_icc_state_key_pack(&state_mask, smask);

    data.last = 1;
    data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_INDEXED;
    data.queue = 0;     /* untg default map to queue0 */

    soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, idx, 
                      SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID,
                      icckey, iccmsk, 
                      state, smask, 
                      &data);
    idx++;
    sws_pr_icc_reserved_rules_set(unit, pr, idx);

    return SOC_E_NONE;
}



/*
 * This function is inherently coupled to the 
 * _soc_sws_pr_icc_tpid_tcam_entry_set() function.
 */
static int _soc_sws_pr_icc_dot1p_de_map_tcam_entry_set(int unit, uint8* dot1p_de_map)
{
    int     idx, vlan_pcp, pr;
    uint8   icckey[26] = {0}, iccmsk[26] = {0};
    uint8   state[4] = {0}, smask[4] = {0};
    uint32  regval = 0, max;
    soc_sbx_caladan3_pr_icc_lookup_data_t   data;
    soc_sbx_calandan3_icc_state_info_t      state_info, state_mask;

    pr = PR0_INSTANCE;
    idx = sws_pr_icc_reserved_rules_get(unit, pr);
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_ICC_CONFIG0r, 
                            pr, 0, &regval));
    max = soc_reg_field_get(unit, PR_ICC_CONFIG0r, regval, LOOKUPS_REQUIREDf);
    /* add tag tcam rule entry */
    for (vlan_pcp = 0; vlan_pcp < VLAN_PCP_MAX_VAL; vlan_pcp++, idx++)
    {
        sal_memset(&data, 0, sizeof(data));
        sal_memset(&icckey, 0, sizeof(icckey));
        sal_memset(&iccmsk, 0, sizeof(iccmsk));
        sal_memset(&state_info, 0, sizeof(state_info));
        sal_memset(&state_mask, 0, sizeof(state_mask));

        state_info.stage    = max - 1;
        state_info.flag     = SWS_ICC_STATE_FLAG_TAG;
        state_mask.stage    = 0xF;
        state_mask.flag     = 0xF;
        soc_sbx_calandan3_sws_icc_state_key_pack(&state_info, state);
        soc_sbx_calandan3_sws_icc_state_key_pack(&state_mask, smask);

        icckey[VLAN_FRAME_PCP_BYTE_LOC] = (uint8)(vlan_pcp << VLAN_TCI_PCP_SHIFT);
        iccmsk[VLAN_FRAME_PCP_BYTE_LOC] = 0xE0;

        data.last = 1;
        data.select_de = 0;     /* ??????????????? */
        data.default_de = dot1p_de_map[vlan_pcp];
        data.state = soc_sbx_calandan3_sws_icc_state_result_get(SWS_ICC_STATE_FLAG_TAG, 1, 0);
        soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, idx, 
                      SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID,
                      icckey, iccmsk, 
                      state, smask, 
                      &data);
    }

    /* add untag tcam rule entry */
    sal_memset(&data, 0, sizeof(data));
    sal_memset(&icckey, 0, sizeof(icckey));
    sal_memset(&iccmsk, 0, sizeof(iccmsk));
    sal_memset(&state_info, 0, sizeof(state_info));
    sal_memset(&state_mask, 0, sizeof(state_mask));
    state_info.stage    = max - 1;
    state_info.flag     = SWS_ICC_STATE_FLAG_UNTAG;
    state_mask.stage    = 0xF;
    state_mask.flag     = 0xF;
    soc_sbx_calandan3_sws_icc_state_key_pack(&state_info, state);
    soc_sbx_calandan3_sws_icc_state_key_pack(&state_mask, smask);

    data.last = 1;
    data.select_de = 0;     /* ?????????????? */
    data.default_de = 1;     /* untg default map to queue0 */
    soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, idx, 
                      SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID,
                      icckey, iccmsk, 
                      state, smask, 
                      &data);
    idx++;
    sws_pr_icc_reserved_rules_set(unit, pr, idx);

    return SOC_E_NONE;
}


int sws_pr_icc_multi_queue_mode_set(int unit)
{
    /* These 2 arrays are correlated and used in tandom in the
     * below functions.
     */
    uint32  tpid_conf[] = {0x8100, 0x9100, 0x9200, 0x88a8};
    uint8   dot1p_queue_map[] = {0, 0, 1, 1, 2, 2, 3, 3};
    int     idx;

    idx = 0;
    sws_pr_icc_reserved_rules_set(unit, PR0_INSTANCE, idx);
    _soc_sws_pr_icc_tpid_tcam_entry_set(unit, tpid_conf, COUNTOF(tpid_conf));
    _soc_sws_pr_icc_dot1p_queue_map_tcam_entry_set(unit, dot1p_queue_map);
    idx = sws_pr_icc_reserved_rules_get(unit, PR0_INSTANCE);
    LOG_CLI((BSL_META_U(unit,
                        "    sws_pr_icc_multi_queue_mode_set: total tcam idx=%d\n\r"), idx));
    return SOC_E_NONE;
}

int sws_pr_icc_multi_de_mode_set(int unit)
{
    /* These 2 arrays are correlated and used in tandom in the
     * below functions.
     */
    uint32  tpid_conf[] = {0x8100, 0x9100, 0x9200, 0x88a8};
    uint8   dot1p_de_map[] = {3, 3, 2, 2, 1, 1, 0, 0};
    int     idx;

    /* Setting the global index to zero here is questionable. It appears wrong
     * but not being the original author of this code segment nor knowing ALL 
     * the use cases specifically handled, no "correction" can be made without more 
     * exhaustive analysis. Setting the global index to 0 here could possibly 
     * overwrite other entries in the ICC table.  For example, see the
     * sws_pr_icc_multi_queue_mode_set() function.  It does the same thing.
     * Obviously at least these 2 functions MUST be mutually exclusive!
     */
    idx = 0;
    sws_pr_icc_reserved_rules_set(unit, PR0_INSTANCE, idx);
    _soc_sws_pr_icc_tpid_tcam_entry_set(unit, tpid_conf, COUNTOF(tpid_conf));
    _soc_sws_pr_icc_dot1p_de_map_tcam_entry_set(unit, dot1p_de_map);
    idx = sws_pr_icc_reserved_rules_get(unit, PR0_INSTANCE);
    LOG_CLI((BSL_META_U(unit,
                        "    sws_pr_icc_multi_de_mode_set: total tcam idx=%d\n\r"), idx));
    return SOC_E_NONE;
}



int sws_pr_icc_disable_all_entry(int unit)
{
    int     idx, pr;
    uint8   icckey[26] = {0}, iccmsk[26] = {0};
    uint8   state[4] = {0}, smask[4] = {0};
    soc_sbx_caladan3_pr_icc_lookup_data_t   data;

    pr = PR0_INSTANCE;
    sal_memset(&data, 0, sizeof(data));
    sal_memset(&icckey, 0, sizeof(icckey));
    sal_memset(&iccmsk, 0, sizeof(iccmsk));

    for (idx=0; idx<255; idx++)
    {
        soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, idx, 
                      SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_INVALID,
                      icckey, iccmsk, 
                      state, smask, 
                      &data);
    }
    sws_pr_icc_reserved_rules_set(unit, PR0_INSTANCE, 0);


    pr = PR1_INSTANCE;
    sal_memset(&data, 0, sizeof(data));
    sal_memset(&icckey, 0, sizeof(icckey));
    sal_memset(&iccmsk, 0, sizeof(iccmsk));

    for (idx=0; idx<255; idx++)
    {
        soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, idx, 
                      SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_INVALID,
                      icckey, iccmsk, 
                      state, smask, 
                      &data);
    }
    sws_pr_icc_reserved_rules_set(unit, PR1_INSTANCE, 0);


    LOG_CLI((BSL_META_U(unit,
                        "    sws_pr_icc_disable_all_entry:\n\r")));

    return SOC_E_NONE;
}


#endif


int
soc_sbx_caladan3_sws_tdm_has_channelized_ports(int unit)
{
    int channelized = FALSE;
    int dummy = 0;
    int rv = 0, port;

    PBMP_ALL_ITER(unit, port) {
        rv =  soc_sbx_caladan3_port_is_channelized_subport(unit, port, &channelized, &dummy);
        if (SOC_SUCCESS(rv) && (channelized == TRUE)) {
            return TRUE;
        }
    }
    return FALSE;
}

void
soc_sbx_caladan3_sws_pr_icc_enable(int unit)
{
    soc_sbx_caladan3_pr_icc_lookup_data_t data;
    sal_memset(&data, 0, sizeof(data));
    data.queue_action = SOC_SBX_CALADAN3_PR_QUEUE_ACTION_DEFAULT;
    data.last = 1;
    soc_sbx_caladan3_sws_pr_tcam_match_all(unit, PR0_INSTANCE, 255, 3, &data);
    soc_sbx_caladan3_sws_pr_tcam_match_all(unit, PR1_INSTANCE, 255, 3, &data);

    /* Set ICC state */
    soc_sbx_caladan3_pr_set_icc_state(unit, 
            PR0_INSTANCE, SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE);
    soc_sbx_caladan3_pr_set_icc_state(unit, 
            PR1_INSTANCE, SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE);
}

void
soc_sbx_caladan3_sws_pr_icc_program_port_match_entries(int unit, 
                                                       int remove_entry, 
                                                       int q_to_remove,
                                                       int dont_add_resv_ports)
{
    int first = 1, line_lport = 0, line_port_index = 0;
    soc_sbx_caladan3_port_map_info_t *port_info = NULL;
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    int channel = 0, entry_required_for_this_lport;
    int rv;

    int icc_table_index = 0;
    int entry_removed = 0;
    int entry_valid = SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID;
    int add_entries = 1;
    int index_spread = 0;

    soc_sbx_caladan3_queues_t *queues = NULL;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    /* What is the caller indicating?
     * Within this function these are mutually exclusive per invocation!
     */
    add_entries = !remove_entry; 

    /* Note: due to the nature of hotswap we always need to have line-side port
     * entries programmed into the PR_ICC. If we are moving to a TDM that 
     * requires them, something we can NOT know ahead of time since this is
     * a run-time decision by the user, then they will be required.  However, if
     * we try to program all entries (we only ever programm ALL entries required
     * by a given a TDM and not just those for the new lports) then the HW 
     * will get wedged since you can not program entries in the PR_ICC 
     * while traffic is running. You don't know if the customer is running
     * traffic during hotswap on the common ports but it's a safe bet they are! 
     */
    PBMP_ALL_ITER(unit, line_lport) {

        /* Only certain types of line_lport require ICC entries. Set below... */
        entry_required_for_this_lport = FALSE; 

        icc_table_index = index_spread = 0;

        if (!soc_sbx_caladan3_is_line_port(unit, line_lport)) {
            /* Skip fabric ports */
            continue;
        }

        rv = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, line_lport, &line_port_index);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: "
                                  "*** Error: Failed to get port queue info for port %d\n"),
                       unit, line_lport));
            return;
        }
        port_info = &port_map->line_port_info[line_port_index];
         
        /*Get the queues for the line side of this line side lport*/
        rv = soc_sbx_caladan3_port_queues_get(unit, line_lport, LINE_SIDE, &queues);
        if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                "*** Error: Failed getting queue info for lport %d\n"),
                                line_lport));
            return;
        }

        if (add_entries) {
            if (IS_IL_PORT(unit, line_lport)) {
                channel = queues->squeue_base;
                entry_required_for_this_lport = TRUE;
            }
            if (IS_E_PORT(unit, line_lport) ||
                port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CMIC) {
                entry_required_for_this_lport = TRUE;
            }
            if (IS_XL_PORT(unit, line_lport) && (SOC_PORT_BINDEX(unit, line_lport) & 1)) {
                entry_required_for_this_lport = FALSE;
            }
                
            if (entry_required_for_this_lport == FALSE) {
                continue; 
            }
        }

        if (add_entries) {
            /* 
             * ======================================================
             * There are a number of conditions which qualify whether 
             * a specific entry is required.
             * ======================================================
             */

            /* There are cases where we require line ports to be programmed 
             * but not the "reserved" line ports.
             */
            if (dont_add_resv_ports && 
                line_port_index > port_map->first_reserved_port) {
                continue;
            }
            /* If this line_lport is already programmed in the PR_ICC then skip it.
             * We don't want to leak ICC memory by orphaning existing ones.
             * Note: An alternative allocation strategy could be to 
             * delete and re-alloc.
             */
            if (queues->squeue_base_pr_icc_table_index_valid) {
                continue;
            }

#ifdef PR_ICC_DEBUG
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: "
                                  "*** %s  ADD line_lport: %d   queue: %d  add: %d\n"),
                       unit, __func__, line_lport, queues->squeue_base, 
                       add_entries & entry_required_for_this_lport));
#endif
            /*
             * Done with qualifying checks for this port
             * =============================================================
             */
            entry_valid = SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID;
            index_spread = PR_ICC_NUM_LOOKUP_STAGES;

            if (sws_pr_icc_table_entry_alloc(unit, PR_INSTANCE(port_info->physical_intf), 
                                             index_spread,
                                             &icc_table_index) != SOC_E_NONE) {
                assert(1);
            } else {
                queues->squeue_base_pr_icc_table_index = icc_table_index;
                queues->squeue_base_pr_icc_table_spread = index_spread;
                queues->squeue_base_pr_icc_table_index_valid = 1;
            }

            sws_pr_icc_port_match_rule_set(unit, PR_INSTANCE(port_info->physical_intf), 
                                           entry_valid,
                                           port_info->base_port + channel,
                                           port_info->port_queues.squeue_base,
                                           index_spread,
                                           icc_table_index);

            /* At least one entry is programmed; set appropriate global reg. state. */
            soc_sbx_caladan3_pr_set_icc_state(unit, 
                                              PR_INSTANCE(port_info->physical_intf),
                                              SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE);
                
        } else if (remove_entry) {

            if (queues->squeue_base != q_to_remove) {
                /*The Q to remove does not exist on this lport.  Move on to next lport.*/
                continue;
            }
#ifdef PR_ICC_DEBUG
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: "
                                  "*** %s  REM line_lport: %d   queue: %d  add: %d\n"),
                       unit, __func__, line_lport, queues->squeue_base, 
                       add_entries & entry_required_for_this_lport));
#endif
            entry_valid = SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_INVALID;

            icc_table_index = queues->squeue_base_pr_icc_table_index;
            index_spread = queues->squeue_base_pr_icc_table_spread;

            if(sws_pr_icc_table_entry_free(unit, PR_INSTANCE(port_info->physical_intf),
                                           queues->squeue_base_pr_icc_table_index,
                                           queues->squeue_base_pr_icc_table_spread) != SOC_E_NONE) {
                assert(1);
            } else {
                queues->squeue_base_pr_icc_table_index = 0;
                queues->squeue_base_pr_icc_table_spread = 0;
                queues->squeue_base_pr_icc_table_index_valid = 0;
            }

            sws_pr_icc_port_match_rule_set(unit, PR_INSTANCE(port_info->physical_intf),
                                           entry_valid,
                                           0,
                                           0,
                                           index_spread,
                                           icc_table_index);
            entry_removed = 1;

        } else {
            /* coverity [dead_error_line] */
            if (!first && SOC_SBX_IS_HG_INTF_TYPE(port_info->intftype)) {
                first = 0;
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "INFO: Unit %d: "
                                        "HG Interface requires PR ICC Tcam for corrrect operation\n"), unit));
            }
        }
    }

    if (remove_entry && !entry_removed) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d: "
                              "ERROR: %s Failed to remove Q %d\n"), unit, __func__, q_to_remove));
    }

}


#endif


