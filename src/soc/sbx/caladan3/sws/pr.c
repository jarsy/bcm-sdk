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


#define PACK_DATA(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))
#if 1
#define UN_PACK_DATA(value, a, b, c, d)\
    do{\
        a = (value & 0xFF000000) >> 24; \
        b = (value & 0xFF0000) >> 16; \
        c = (value & 0xFF00) >> 8; \
        d = (value & 0xFF); \
    }while(0)
#endif


int
soc_sbx_caladan3_sws_pr_icc_queue_map_set(int unit, int blk, int base_queue);

/*
 * Profile Based PR buffer allocation
 * - This feature allows user to pick a profile for pr buffer level allocation
 *   this is used in conjunction with Hotswap/Fastreset to setup
 *   universal pr buffer scheme. The profile id selects the scheme to apply
 */
typedef struct pr_profile_buffer_s {
    int profile_id;
    int pr_instance;
    sws_pr_port_buffer_t pr_buffer;
} pr_profile_buffer_t;


pr_profile_buffer_t 
profile_buffer[SOC_SBX_CALADAN3_PR_BUFFER_PROFILE_MAX] =
{
    {
       SOC_SBX_CALADAN3_PR_BUFFER_PROFILE_ID_CL_XT,  /* Profile Id */
       0,                                            /* Applicable to Line side */
       {
          51,
          {
            {0,0,28},
            {1,28,16},
            {2,540,16},
            {3,556,56},
            {4,268,56},
            {5,324,56},
            {6,380,56},
            {7,436,56},
            {8,44,56},
            {9,100,56},
            {10,156,56},
            {11,212,56},
            {12,556,14},
            {13,570,14},
            {14,584,14},
            {15,598,14},
            {16,268,14},
            {17,282,14},
            {18,296,14},
            {19,310,14},
            {20,324,14},
            {21,338,14},
            {22,352,14},
            {23,366,14},
            {24,380,14},
            {25,394,14},
            {26,408,14},
            {27,422,14},
            {28,436,14},
            {29,450,14},
            {30,464,14},
            {31,478,14},
            {32,44,14},
            {33,58,14},
            {34,72,14},
            {35,86,14},
            {36,100,14},
            {37,114,14},
            {38,128,14},
            {39,142,14},
            {40,156,14},
            {41,170,14},
            {42,184,14},
            {43,198,14},
            {44,212,14},
            {45,226,14},
            {46,240,14},
            {47,254,14},
            {48,492,16},
            {49,508,16},
            {50,524,16},
          },
       },
    },
    {
       SOC_SBX_CALADAN3_PR_BUFFER_PROFILE_ID_CL_ONLY,     /* Profile Id */
       0,                                                 /* Applicable to Line side */
       {
          51,
          {
            {0,0,24},
            {1,24,24},
            {2,48,24},
            {3,72,24},
            {4,96,32},
            {5,128,32},
            {6,160,32},
            {7,192,32},
            {8,224,48},
            {9,272,48},
            {10,320,48},
            {11,368,48},
            {12,0,0},
            {13,0,0},
            {14,0,0},
            {15,0,0},
            {16,0,0},
            {17,0,0},
            {18,0,0},
            {19,0,0},
            {20,0,0},
            {21,0,0},
            {22,0,0},
            {23,0,0},
            {24,0,0},
            {25,0,0},
            {26,0,0},
            {27,0,0},
            {28,0,0},
            {29,0,0},
            {30,0,0},
            {31,0,0},
            {32,0,0},
            {33,0,0},
            {34,0,0},
            {35,0,0},
            {36,0,0},
            {37,0,0},
            {38,0,0},
            {39,0,0},
            {40,0,0},
            {41,0,0},
            {42,0,0},
            {43,0,0},
            {44,0,0},
            {45,0,0},
            {46,0,0},
            {47,0,0},
            {48,416,64},
            {49,480,64},
            {50,544,64},
           },
       },
    }
};

/*
 * Function:
 *    soc_sbx_caladan3_pr_port_buffer_set
 * Purpose
 *    allocate port buffers
 */
int
soc_sbx_caladan3_pr_port_buffer_set(int unit, int instance, 
                                    sws_pr_port_buffer_t *pr_buf)
{
    int port;
    uint32 regval;
    pr_port_buffer_entry_t *port_buf;

    for (port=0; port < pr_buf->num_elements; port++) {
        port_buf = &pr_buf->entry[port];
        regval = 0;
        soc_reg_field_set(unit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr, 
                          &regval, DEPTHf, port_buf->size);
        soc_reg_field_set(unit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr, 
                          &regval, START_ADDRESSf, port_buf->start);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
                                PR_IPRE_RX_PORT_PAGE_BUFFER_CFGr,
                                PR_INSTANCE(instance), port_buf->port, regval));
    }
    if (pr_buf->num_elements > 0) {
        regval = 0;
        soc_reg_field_set(unit, PR_IPRE_RX_PORT_PAGE_BUFFER_CFG_COMPLETEr, 
                          &regval, CONFIGURATION_COMPLETEf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
                            PR_IPRE_RX_PORT_PAGE_BUFFER_CFG_COMPLETEr,
                            PR_INSTANCE(instance), 0, regval));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_pr_port_buffer_profile_alloc
 * Purpose
 *    allocate port buffers based on a profile
 */
int
soc_sbx_caladan3_pr_port_buffer_profile_alloc(int unit, int instance, int prof_id,
                                              sws_pr_port_buffer_t **pr_buf)
{
    int i;

    for (i=0; i < SOC_SBX_CALADAN3_PR_BUFFER_PROFILE_MAX; i++) {
        if ((prof_id == profile_buffer[i].profile_id) && 
              (instance == profile_buffer[i].pr_instance)) {
            *pr_buf = &profile_buffer[i].pr_buffer;
            break;
        }
    }
    if (i >= SOC_SBX_CALADAN3_PR_BUFFER_PROFILE_MAX) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Invalid profile id %d for PR%d\n"), 
                   unit, prof_id, instance));
        return SOC_E_PARAM;
    }
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
             (BSL_META_U(unit,
                         "Unit %d, Buffer Profile %d  Selected for PR%d\n"), 
              unit, prof_id, instance));
   
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_pr_port_buffer_alloc
 * Purpose
 *    allocate port buffers
 */
int 
soc_sbx_caladan3_pr_port_buffer_alloc(int unit, int instance,
                                      sws_pr_port_buffer_t *pr_buf)
{
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_port_map_info_t *port_info;
    int port, bw=0, idx=0;
    uint32 pages=0, offset=0, max_pages=0;
    pr_port_buffer_entry_t *port_buf;
    uint32 lsport0 = -1, lsport1 = -1;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    bw = ((instance == 0) ? port_map->max_line_bw: port_map->max_fabric_bw);
    pages=0; offset=0; 
    max_pages=SOC_SBX_CALADAN3_SWS_PR_RX_PORT_PAGE_BUFFER_MAX;

    sal_memset(pr_buf, 0, sizeof(sws_pr_port_buffer_t));

    /* if line side, reserve pages for cmic,xl ports in advance */
    if (instance == 0) {
        if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
            max_pages -= (port_map->num_1g_ports - 2 )* 
                          SOC_SBX_SWS_PR_RX_PORT_PAGE_GE_PORT;
        } else {
            max_pages -= port_map->num_1g_ports * 
                          SOC_SBX_SWS_PR_RX_PORT_PAGE_GE_PORT;
        }
        bw -= (port_map->num_1g_ports - SOC_SBX_CALADAN3_NUM_RESV_PORTS);
    }
    for (idx=0, port=0; port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {
        port_info = &port_map->line_port_info[port];
        if (!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }

        if (port_info->flags & SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT) {
            continue;
        }

        if (port_info->physical_intf == instance) {

            if ((port_info->base_port >=0) &&
                (lsport0 != port_info->base_port)) {

                pages = (port_map->intf_attr[port_info->intftype].speed * 
                         max_pages) / bw;

                if ((port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_XLPORT) &&
                       (port_info->bindex & 1)) {
                    continue;
                }

                if (port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE ||
                      port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_XTPORT ||
                        port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_XLPORT ||
                          port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CMIC) {
                    pages = SOC_SBX_SWS_PR_RX_PORT_PAGE_GE_PORT;
                }
                port_buf = &pr_buf->entry[idx++];
                port_buf->start = offset;
                port_buf->size  = pages;
                port_buf->port = port_info->base_port;
                offset += pages;
                lsport0 = port_info->base_port;
            }
        }
 
        port_info = &port_map->fabric_port_info[port];
        if (port_info->physical_intf == instance) {
            if ((port_info->base_port >=0) &&
                (lsport1 != port_info->base_port)) {
                pages = (port_map->intf_attr[port_info->intftype].speed * 
                         max_pages) / bw;

                port_buf = &pr_buf->entry[idx++];
                port_buf->start = offset;
                port_buf->size  = pages;
                port_buf->port = port_info->base_port;
                offset += pages;
                lsport1 = port_info->base_port;
            }
        }
    }
    pr_buf->num_elements = idx;
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_pr_set_icc_state(int unit, int pr, int flag)
{
    uint32 regval = 0;
    if ((flag != SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE) &&
             (flag != SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS)) {
        return SOC_E_PARAM;
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_ICC_CONFIG0r, 
                        pr, 0, &regval));
    soc_reg_field_set(unit, PR_ICC_CONFIG0r, &regval, 
                      OPERATION_TYPEf, (uint32)flag);
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_ICC_CONFIG0r, 
                        pr, 0, regval));

    soc_sbx_caladan3_sws_set_icc_state(unit, pr, flag);

    return SOC_E_NONE;
}



/* 
 * Function: 
 *     soc_sbx_caladan3_sws_pr_icc_tcam_entry_get 
 * Purpose: 
 *     Query the TCAM entry 
 */ 
void
soc_sbx_caladan3_sws_pr_icc_tcam_entry_get(
                                int unit, int pr, int idx, int* valid,
                                uint8 *key, uint8 *mask,
                                uint8 *state, uint8 *state_mask,
                                soc_sbx_caladan3_pr_icc_lookup_data_t *data)
{
    uint32  iccKey[8] = {0}, iccMsk[8] = {0}, val;
    int     ii;
    uint32  pkt_key[8], pkt_mask[8];
    uint8   l_state[4], l_state_mask[4];
    int     rv;
    pr_icc_lookup_core_tcam_table_entry_t           iccTblEntry;
    pr_icc_lookup_core_lookup_results_table_entry_t iccResEntry;

    pr = (pr == PR1_INSTANCE) ? PR1_BLOCK(unit) : PR0_BLOCK(unit);
    if (!soc_sbx_caladan3_sws_qm_cfg_get(unit))
    {
        /* Not intialized return early */
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d, PR Tcam cannot be get, not initialized\n"), unit));
        return;
    }
    if ((idx > 255) || (idx < 0) || (valid == NULL))
    {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "PR Tcam get param ERR\n")));
        return;
    }

    sal_memset(&iccTblEntry, 0, sizeof(pr_icc_lookup_core_tcam_table_entry_t));
    rv = READ_PR_ICC_LOOKUP_CORE_TCAM_TABLEm(unit, pr, idx, &iccTblEntry);
    if (rv != SOC_E_NONE)
    {
        LOG_CLI((BSL_META_U(unit,
                            "soc_sbx_caladan3_sws_pr_icc_tcam_get read tcam table error!!\n\r")));
        return;
    }

    /* tcam entry is not enable */
    soc_PR_ICC_LOOKUP_CORE_TCAM_TABLEm_field_get(unit, &iccTblEntry, VALIDf, &val);
    *valid = val;
    if (*valid != SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID)
    {
        return;
    }

    soc_PR_ICC_LOOKUP_CORE_TCAM_TABLEm_field_get(unit, &iccTblEntry, KEYf, &iccKey[0]);
    soc_PR_ICC_LOOKUP_CORE_TCAM_TABLEm_field_get(unit, &iccTblEntry, MSKf, &iccMsk[0]);

    /* iccKey[0]/iccMsk[0] is state(only 3 byte is valid), output: state&state_mask */
    UN_PACK_DATA(iccKey[0], l_state[3], l_state[2], l_state[1], l_state[0]);
    UN_PACK_DATA(iccMsk[0], l_state_mask[3], l_state_mask[2], l_state_mask[1], l_state_mask[0]);
    if ((state != NULL) && (state_mask != NULL))
    {
        sal_memcpy(state, l_state, 3);
        sal_memcpy(state_mask, l_state_mask, 3);
    }

    /* packet key/mask parse, output: key&mask */
    pkt_key[0]  = iccKey[7] >> 2;
    pkt_mask[0] = iccMsk[7] >> 2;
    for (ii=1; ii<=6; ii++)
    {
        pkt_key[ii]  = (iccKey[7-ii] >> 2) | ((iccKey[7-ii+1] & 0x3) << 30);
        pkt_mask[ii] = (iccMsk[7-ii] >> 2) | ((iccMsk[7-ii+1] & 0x3) << 30);
    }
    if ((key != NULL) && (mask != NULL))
    {
        key[0] = pkt_key[0];
        mask[0] = pkt_mask[0];
        for (ii=0; ii<6; ii++)
        {
            UN_PACK_DATA(pkt_key[ii+1], key[ii*4+1], key[ii*4+2], key[ii*4+3], key[ii*4+4]);
            UN_PACK_DATA(pkt_mask[ii+1], mask[ii*4+1], mask[ii*4+2], mask[ii*4+3], mask[ii*4+4]);
        }
    }

    /* tcam result parse */
    sal_memset(&iccResEntry, 0, sizeof(iccResEntry));
    rv = READ_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm(unit, pr, idx, &iccResEntry);
    if ((rv == SOC_E_NONE) && (data != NULL))
    {
        uint32  last, reg_state, queue_action, queue, drop, default_de, select_de, cos, dp, shift;
        /* Only unused bits of state will be preserved in last case */
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_get(unit, &iccResEntry, LASTf, &last);
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_get(unit, &iccResEntry, QUEUE_ACTIONf, &queue_action);
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_get(unit, &iccResEntry, QUEUEf, &queue);
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_get(unit, &iccResEntry, DROPf, &drop);
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_get(unit, &iccResEntry, DEFAULT_DEf, &default_de);
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_get(unit, &iccResEntry, SELECT_DEf, &select_de);
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_get(unit, &iccResEntry, COSf, &cos);
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_get(unit, &iccResEntry, DPf, &dp);
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_get(unit, &iccResEntry, STATEf, &reg_state);
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_get(unit, &iccResEntry, SHIFTf, &shift);

        data->last = last;
        data->queue_action = queue_action;
        data->queue = queue;
        data->drop = drop;
        data->select_de = select_de;
        data->default_de = default_de;
        data->cos = cos;
        data->dp = dp;
        data->state = reg_state;
        data->shift = shift;
    }

    return;
}
   


/*
 * Function:
 *     soc_sbx_caladan3_sws_pr_icc_tcam_program
 * Purpose:
 *     Initialize PR TCAM 
 */
void
soc_sbx_caladan3_sws_pr_icc_tcam_program(
                                int unit, int pr, int idx, int valid, 
                                uint8 *key, uint8 *mask, 
                                uint8 *state, uint8 *state_mask, 
                                soc_sbx_caladan3_pr_icc_lookup_data_t *data)
{
    uint32 val = 0, iccKey[8] = {0}, iccMsk[8] = {0};
    uint32 tmpKey = 0, prevKey = 0;
    uint32 tmpMsk = 0, prevMsk = 0;
    pr_icc_lookup_core_tcam_table_entry_t iccTblEntry;	
    pr_icc_lookup_core_lookup_results_table_entry_t iccResEntry;
    int last, i, j;
    int rv;
    uint16 dev_id;
    uint8  rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    pr = (pr == PR1_INSTANCE) ? PR1_BLOCK(unit) : PR0_BLOCK(unit);

    if (!soc_sbx_caladan3_sws_qm_cfg_get(unit)) {
        /* Not intialized return early */
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d, PR Tcam "
                                " cannot be programmed, not initialized\n"), unit));
        return;
    }
    if ((valid != 0) && (valid != 3)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d, Invalid parameter, valid must be either 0 or 3\n"), unit));
        return;
    }

    last = sizeof(iccKey)/sizeof(uint32)-1;
    j = 0;
    iccKey[last] = PACK_DATA(0, 0, 0, key[j]);
    iccMsk[last] = PACK_DATA(0, 0, 0, mask[j]);

    for (j+=1, i=last-1; ((i > 0) && (j < 25)); i--, j+=4) {
        iccKey[i] = PACK_DATA(key[j], key[j+1], key[j+2], key[j+3]);
        iccMsk[i] = PACK_DATA(mask[j], mask[j+1], mask[j+2], mask[j+3]);
    }

    /* format the key and mask */
    for (i=last; i > 0; i--) {
        prevKey = iccKey[i-1] >> 30;
        prevMsk = iccMsk[i-1] >> 30;
        tmpKey = (prevKey)  | (iccKey[i] << 2);
        tmpMsk = (prevMsk)  | (iccMsk[i] << 2);
        iccKey[i] = tmpKey;
        iccMsk[i] = tmpMsk;
    }
    iccKey[i] = PACK_DATA(0, state[2], state[1], state[0]);
    iccMsk[i] = PACK_DATA(0, state_mask[2], state_mask[1], state_mask[0]);

    sal_memset(&iccTblEntry, 0, sizeof(pr_icc_lookup_core_tcam_table_entry_t));
    rv = READ_PR_ICC_LOOKUP_CORE_TCAM_TABLEm(unit, pr, idx, &iccTblEntry);
    if (rv != SOC_E_NONE) {
        return;
    }

    soc_PR_ICC_LOOKUP_CORE_TCAM_TABLEm_field_set(unit, &iccTblEntry, KEYf, 
                                                 &iccKey[0]);
    soc_PR_ICC_LOOKUP_CORE_TCAM_TABLEm_field_set(unit, &iccTblEntry, MSKf, 
                                                 &iccMsk[0]);
    val = valid;
    soc_PR_ICC_LOOKUP_CORE_TCAM_TABLEm_field_set(unit, &iccTblEntry, 
                                                 VALIDf, &val);
    rv = WRITE_PR_ICC_LOOKUP_CORE_TCAM_TABLEm(unit, pr, idx, &iccTblEntry);
    if (SOC_FAILURE(rv)) {
        return;
    }

    sal_memset(&iccResEntry, 0, sizeof(iccResEntry));
    rv = READ_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm(unit, pr, 
                                                       idx, &iccResEntry);
    if (rv != SOC_E_NONE) {
        return;
    }
    soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
        &iccResEntry, LASTf, &data->last);
    if (data->last) {
        /* Only unused bits of state will be preserved in last case */
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
            &iccResEntry, STATEf, &data->state);  
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
            &iccResEntry, QUEUE_ACTIONf, &data->queue_action);
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
            &iccResEntry, QUEUEf, &data->queue);  
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
            &iccResEntry, DROPf, &data->drop);  
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
            &iccResEntry, DEFAULT_DEf, &data->default_de);  
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
            &iccResEntry, SELECT_DEf, &data->select_de);  
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
            &iccResEntry, COSf, &data->cos);  
        soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
            &iccResEntry, DPf, &data->dp);  
    } else {
        if (rev_id == BCM88030_B0_REV_ID ||  rev_id == BCM88030_B1_REV_ID) {

            if(data->queue_action == SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP ||
               data->queue_action == SOC_SBX_CALADAN3_PR_QUEUE_ACTION_INDEXED) {
                data->state |= 0x800000 | (data->queue_action << 8) | (data->queue & 0xff);
            }
            if(data->drop != 0 ) {
                data->state |= 0x400000 | (data->drop<<10);
            }
            if(data->cos != 0 ) {
                data->state |= 0x200000 | (data->cos << 11);
            }
            if(data->dp != 0 ) {
                data->state |= 0x100000 | (data->dp << 17);
            }
            if(data->default_de != 0 || data->select_de !=0 ) {
                data->state |= 0x080000 | (data->select_de << 14) | (data->default_de << 15);
            }
            soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
                &iccResEntry, STATEf, &data->state);  

            soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
                &iccResEntry, SHIFTf, &data->shift);  

        } else {

            soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
                &iccResEntry, STATEf, &data->state);  
            soc_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm_field_set(unit, 
                &iccResEntry, SHIFTf, &data->shift);  
        }
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "INFO:state 0x%06x queue_action %d, queue %d default_de %d select_de %d\n"), data->state, data->queue_action, data->queue, data->default_de, data->select_de ));

    rv = WRITE_PR_ICC_LOOKUP_CORE_LOOKUP_RESULTS_TABLEm(unit, pr, 
                                                        idx, &iccResEntry);
    if (SOC_FAILURE(rv)) {
        return;
    }

}

/*
 * Pr IDP Policer
 *    There are 127 entries, resources are allocated based on user give profile
 *    There is no way to reconfig currently, free and then re-allocate.
 */

int
soc_sbx_caladan3_sws_pr_idp_policer_free(int unit, int instance, int polid)
{
    soc_sbx_caladan3_sws_pr_policer_cfg_t *pr_pol_cfg;

    if ((polid < 0) || (polid > 127)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Invalid policer id : %d"), unit, polid));
        return SOC_E_PARAM;
    }

    pr_pol_cfg = soc_sbx_caladan3_sws_pr_policer_cfg_get(unit, instance);
    if (!pr_pol_cfg) {
        return SOC_E_PARAM;
    }
    pr_pol_cfg->refcnt[polid]--;
    if (!pr_pol_cfg->refcnt[polid]) {
        sal_memset(&pr_pol_cfg->policer[polid], 0, sizeof(soc_sbx_caladan3_pr_policer_t));
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_pr_idp_policer_allocate(int unit, int instance, soc_sbx_caladan3_pr_policer_t *policer, int *id)
{
    soc_sbx_caladan3_sws_pr_policer_cfg_t *pr_pol_cfg;
    int i, found  = 0;

    pr_pol_cfg = soc_sbx_caladan3_sws_pr_policer_cfg_get(unit, instance);
    if (!pr_pol_cfg || !id) {
        return SOC_E_PARAM;
    }

    /* Try to locate already existing policer */
    for (i=0; i < SOC_SBX_CALADAN3_SWS_MAX_PR_POLICER; i++) {
        if (pr_pol_cfg->refcnt[i]) {
            if (0 == sal_memcmp(&pr_pol_cfg->policer[i], 
                         policer, sizeof(soc_sbx_caladan3_pr_policer_t))) {
                found = 1;
                break;
            }
        }
    }

    if (!found) {
        for (i=0; i < SOC_SBX_CALADAN3_SWS_MAX_PR_POLICER; i++) {
            if (!pr_pol_cfg->refcnt[i]) {
                found = 1;
                break;
            }
        }
    } 

    if (found) {
        pr_pol_cfg->refcnt[i]++;
        *id = i;
        return SOC_E_NONE;
    }

    return SOC_E_MEMORY;
}


#define SET_UNSET_FLAG(d, c, s, u, e, f)                         \
    if ((d) & (c)) {                                             \
        soc_mem_field32_set(unit, PR_IDP_POLICER_METER_CONFIGm,    \
                          (e), (f), (s));                        \
    } else {                                                     \
        soc_mem_field32_set(unit, PR_IDP_POLICER_METER_CONFIGm,    \
                          (e), (f), (u));                        \
    }



int
soc_sbx_caladan3_sws_pr_idp_policer_config(int unit, int instance, int id, soc_sbx_caladan3_pr_policer_t *polcfg)
{
 
    pr_idp_policer_meter_config_entry_t entry;
    soc_sbx_caladan3_sws_pr_policer_cfg_t *pr_pol_cfg;
    int blk = 0;
    uint32 set = 1;
    uint32 unset = 0;
    uint32 regval = 0;


    pr_pol_cfg = soc_sbx_caladan3_sws_pr_policer_cfg_get(unit, instance);
    if (!pr_pol_cfg) {
       return SOC_E_INIT;
    }
    if ((id < 0) || (id > 127)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Invalid policer id : %d"), unit, id));
        return SOC_E_PARAM;
    }

    sal_memset(&entry, 0, sizeof(entry));

    blk = soc_sbx_block_find(unit, SOC_BLK_PR, instance);

    SOC_IF_ERROR_RETURN(soc_mem_read(unit, 
                           PR_IDP_POLICER_METER_CONFIGm, blk, id, &entry));
    SET_UNSET_FLAG(polcfg->flags, SOC_SBX_CALADAN3_PR_POLICER_PKT_MODE, 
                   set, unset, &entry, PKT_MODEf);
    SET_UNSET_FLAG(polcfg->flags, SOC_SBX_CALADAN3_PR_POLICER_BKT_E_LOCKED, 
                   set, unset, &entry, BKTE_NODECf);
    SET_UNSET_FLAG(polcfg->flags, SOC_SBX_CALADAN3_PR_POLICER_BKT_C_LOCKED, 
                   set, unset, &entry, BKTC_NODECf);
    SET_UNSET_FLAG(polcfg->flags, SOC_SBX_CALADAN3_PR_POLICER_BKT_E_STRICT, 
                   set, unset, &entry, BKTE_STRICTf);
    SET_UNSET_FLAG(polcfg->flags, SOC_SBX_CALADAN3_PR_POLICER_BKT_C_STRICT, 
                   set, unset, &entry, BKTC_STRICTf);
    SET_UNSET_FLAG(polcfg->flags, SOC_SBX_CALADAN3_PR_POLICER_RFC2698, 
                   set, unset, &entry, RFC2698f);
    SET_UNSET_FLAG(polcfg->flags, SOC_SBX_CALADAN3_PR_POLICER_OVERFLOW, 
                   set, unset, &entry, CP_FLAGf);
    SET_UNSET_FLAG(polcfg->flags, SOC_SBX_CALADAN3_PR_POLICER_DROP_ON_RED, 
                   set, unset, &entry, DROP_ON_REDf);
    SET_UNSET_FLAG(polcfg->flags, SOC_SBX_CALADAN3_PR_POLICER_COLOR_BLIND, 
                   set, unset, &entry, BLINDf);

    soc_mem_field32_set(unit, PR_IDP_POLICER_METER_CONFIGm, &entry, EBSf, polcfg->ebs);
    soc_mem_field32_set(unit, PR_IDP_POLICER_METER_CONFIGm, &entry, EIRf, polcfg->eir);
    soc_mem_field32_set(unit, PR_IDP_POLICER_METER_CONFIGm, &entry, CBSf, polcfg->cbs);
    soc_mem_field32_set(unit, PR_IDP_POLICER_METER_CONFIGm, &entry, CIRf, polcfg->cir);
    
    SOC_IF_ERROR_RETURN(soc_mem_write(unit, 
                            PR_IDP_POLICER_METER_CONFIGm, 
                            blk, id, &entry)); 

    if (!pr_pol_cfg->enabled) {
        soc_reg_field_set(unit, PR_IDP_POLICER_DISABLEr, &regval, 
                          DISABLE_POLICERf, 0);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IDP_POLICER_DISABLEr,
                            PR_INSTANCE(instance), 0, regval));
        pr_pol_cfg->enabled = 1;
    }

    sal_memcpy(&pr_pol_cfg->policer[id], polcfg, sizeof(*polcfg));
    
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_pr_tcam_default_entry(int unit, int pr, int idx,
                                           uint32 def_profile, 
                                           uint32 policer_base, 
                                           uint32 def_queue)
{
    pr_icc_lookup_core_port_defaults_table_entry_t iccDefault;
    int rv = SOC_E_NONE;

    pr = (pr == PR1_INSTANCE) ? PR1_BLOCK(unit) : PR0_BLOCK(unit);

    /* set up default ICC port to queue mapping table */
    SOC_IF_ERROR_RETURN(
        READ_PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm(unit,
            pr, idx, 
            &iccDefault));
    soc_PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm_field_set(unit,
        &iccDefault, DEFAULT_QUEUEf, &def_queue);
    soc_PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm_field_set(unit, 
        &iccDefault, POLICER_BASEf, &policer_base);  
    soc_PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm_field_set(unit, 
        &iccDefault, PROFILEf, &def_profile);
    rv = WRITE_PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm(unit, 
            pr, idx, &iccDefault);
    if (SOC_FAILURE(rv)) {
        return rv;
    }
    return SOC_E_NONE;
}


int
soc_sbx_caladan3_sws_pr_client_enable(int unit, int instance, int port, int enable)
{
    soc_sbx_caladan3_port_map_info_t *port_info;
    uint32 regval=0, regval1=0, fcdata=0;
    int base_port;
    int reg=0, fc_enable=1, idx=0, fc_interface_mapping = 0;
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    soc_sbx_caladan3_queues_t *queueinfo = NULL;

    static int il_fab_fc_complete = 0;
    static int il_line_fc_complete = 0;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    fc_enable = soc_sbx_caladan3_sws_config_param_data_get(unit,
                     spn_FC_ENABLE, port, fc_enable);

    if (PR_INSTANCE(instance) == PR1_INSTANCE) {
        port_info = &port_map->fabric_port_info[port];

        queueinfo = &port_info->port_queues;

        soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata,
                          FC_LOGICAL_OR_MAPPINGf,
                          (SOC_SBX_CALADAN3_SWS_PR_FC_ENQR_FIFO_XOFF |
                           SOC_SBX_CALADAN3_SWS_PR_FC_FABRIC_LLFC_XOFF |
                           SOC_SBX_CALADAN3_SWS_PR_FC_SQUEUE_XOFF));

        idx = queueinfo->squeue_base - SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE;

    } else {
        port_info = &port_map->line_port_info[port];
        queueinfo = &port_info->port_queues;

        if (port_info->intftype != SOC_SBX_CALADAN3_PORT_INTF_CMIC) {
            soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata,
                              FC_LOGICAL_OR_MAPPINGf,
                              (SOC_SBX_CALADAN3_SWS_PR_FC_ENQR_FIFO_XOFF |
                               SOC_SBX_CALADAN3_SWS_PR_FC_LINE_LLFC_XOFF |
                               SOC_SBX_CALADAN3_SWS_PR_FC_SQUEUE_XOFF));
        }

        idx = queueinfo->squeue_base - SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE;
    }

    switch (queueinfo->squeue_base>>5) {

    case 0:
        reg = PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_031_000r;
        break;
    case 1:
        reg = PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_063_032r;
        break;
    case 2:
        reg = PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_095_064r;
        break;
    case 3:
        reg = PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_127_096r;
        break;
    default:
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, invalid squeue(%d) on port(%d)\n"), unit,
                   queueinfo->squeue_base, port_info->port));
        return SOC_E_PARAM;
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, PR_INSTANCE(instance), 0, &regval));
    regval1 = soc_reg_field_get(unit, reg, regval, ENABLE_SQUEUESf);
    regval1 |= (fc_enable << (queueinfo->squeue_base%32));
    soc_reg_field_set(unit, reg, &regval, ENABLE_SQUEUESf, regval1);
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, reg, PR_INSTANCE(instance), 0, regval));

    regval = 0;
    regval1 = 0;
    base_port = port_info->base_port - port_info->bindex;

    switch(port_info->intftype) {

    case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
    case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
    case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
    case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
    case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
    case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
    case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
    case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
        fc_interface_mapping = 1;

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG0r, 
                            PR_INSTANCE(instance), 0, &regval));
        if (enable != 
                soc_reg_field_get(unit, PR_IPRE_CI_CONFIG0r, regval, CL_CLIENT_IFf))
        {
            soc_reg_field_set(unit, PR_IPRE_CI_CONFIG0r, &regval, CL_CLIENT_IFf, 1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG0r, 
                                PR_INSTANCE(instance), 0, regval));
        }
        break;

    case SOC_SBX_CALADAN3_PORT_INTF_XTPORT:
        fc_interface_mapping = 1;

        if (port_info->instance == 0) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG0r, 
                                PR_INSTANCE(instance), 0, &regval));
            if (enable != soc_reg_field_get(unit, PR_IPRE_CI_CONFIG0r,
                                            regval, XT0_CLIENT_IFf)) {

                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG1r, 
                                    PR_INSTANCE(instance), 0, &regval1));
                soc_reg_field_set(unit, PR_IPRE_CI_CONFIG0r, 
                                  &regval, XT0_CLIENT_IFf, 1);
                soc_reg_field_set(unit, PR_IPRE_CI_CONFIG1r, &regval1, 
                                  XT0_BASE_PORTf, base_port);
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG1r, 
                                    PR_INSTANCE(instance), 0, regval1));
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG0r, 
                                    PR_INSTANCE(instance), 0, regval));
            }
        } else if (port_info->instance == 1) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG0r, 
                                PR_INSTANCE(instance), 0, &regval));
            if (enable != soc_reg_field_get(unit, PR_IPRE_CI_CONFIG0r,
                                       regval, XT1_CLIENT_IFf)) {
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG1r, 
                                    PR_INSTANCE(instance), 0, &regval1));
                soc_reg_field_set(unit, PR_IPRE_CI_CONFIG0r, 
                                  &regval, XT1_CLIENT_IFf, 1);
                soc_reg_field_set(unit, PR_IPRE_CI_CONFIG1r, &regval1, 
                                  XT1_BASE_PORTf, base_port);
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG1r, 
                                    PR_INSTANCE(instance), 0, regval1));
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG0r, 
                                    PR_INSTANCE(instance), 0, regval));
            }
        } else {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG0r, 
                                PR_INSTANCE(instance), 0, &regval));
            if (enable != soc_reg_field_get(unit, PR_IPRE_CI_CONFIG0r,
                                       regval, XT2_CLIENT_IFf)) {
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG2r, 
                                    PR_INSTANCE(instance), 0, &regval1));
                soc_reg_field_set(unit, PR_IPRE_CI_CONFIG0r, &regval, 
                                  XT2_CLIENT_IFf, 1);
                soc_reg_field_set(unit, PR_IPRE_CI_CONFIG2r, &regval1, 
                                  XT2_BASE_PORTf, base_port);
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG2r, 
                                    PR_INSTANCE(instance), 0, regval1));
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG0r, 
                                    PR_INSTANCE(instance), 0, regval));
            }
        }
        break;

    case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
    case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
    case SOC_SBX_CALADAN3_PORT_INTF_IL100:
        fc_interface_mapping = 0;

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG0r, 
                            PR_INSTANCE(instance), 0, &regval));
        if (enable != soc_reg_field_get(unit, PR_IPRE_CI_CONFIG0r,
                                        regval, IL_CLIENT_IFf)) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG1r, 
                                PR_INSTANCE(instance), 0, &regval1));
            soc_reg_field_set(unit, PR_IPRE_CI_CONFIG0r, &regval, 
                              IL_CLIENT_IFf, 1);
            soc_reg_field_set(unit, PR_IPRE_CI_CONFIG1r, &regval1, 
                              IL_BASE_PORTf, port_info->base_port);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG1r, 
                                PR_INSTANCE(instance), 0, regval1));
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG0r, 
                                PR_INSTANCE(instance), 0, regval));
        }
        break;

    case SOC_SBX_CALADAN3_PORT_INTF_CMIC:
        if (PR_INSTANCE(instance) == PR0_INSTANCE) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG0r, 
                                PR_INSTANCE(instance), 0, &regval));
            if (enable != soc_reg_field_get(unit, PR_IPRE_CI_CONFIG0r,
                                       regval, CMIC_CLIENT_IFf)) {
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG2r, 
                                    PR_INSTANCE(instance), 0, &regval1));
                soc_reg_field_set(unit, PR_IPRE_CI_CONFIG0r, &regval, 
                                  CMIC_CLIENT_IFf, 1);
                soc_reg_field_set(unit, PR_IPRE_CI_CONFIG2r, &regval1, 
                                  CMIC_BASE_PORTf, base_port);
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG2r, 
                                  PR_INSTANCE(instance), 0, regval1));
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG0r, 
                                  PR_INSTANCE(instance), 0, regval));
            }
        }
        break;

    case SOC_SBX_CALADAN3_PORT_INTF_XLPORT:
        fc_interface_mapping = 1;

        if (PR_INSTANCE(instance) == PR0_INSTANCE) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG0r, 
                                PR_INSTANCE(instance), 0, &regval));
            if (enable != soc_reg_field_get(unit, PR_IPRE_CI_CONFIG0r,
                                            regval, XL_CLIENT_IFf)) {
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_CI_CONFIG2r, 
                                    PR_INSTANCE(instance), 0, &regval1));
                soc_reg_field_set(unit, PR_IPRE_CI_CONFIG0r, &regval,
                                  XL_CLIENT_IFf, 1);
                soc_reg_field_set(unit, PR_IPRE_CI_CONFIG2r, &regval1,
                                  XL_BASE_PORTf, base_port);
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG2r, 
                                    PR_INSTANCE(instance), 0, regval1));
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_CI_CONFIG0r, 
                                  PR_INSTANCE(instance), 0, regval));
            }
        }
        break;
    default:
        break;
    }

    /* No FC for CMIC and FC for ILKN done differently due to it needing to be done for
     * ALL channels irrespective if whether a channel is enabled or not.
     */
    if (port_info->intftype != SOC_SBX_CALADAN3_PORT_INTF_CMIC) {

        if ((port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_IL50n ||
             port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_IL50w ||
             port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_IL100) ) {
            int i;
            /* fabric side = 11 - Port/Channel XOFF state = Enq Req FIFO XOFF
             * state | Fabric LLFC XOFF state | Squeue XOFF state
             */
            if (PR_INSTANCE(instance) == PR1_INSTANCE &&
                il_fab_fc_complete == 0) {

                for (i = 0; i < 64; i++) {
                    fcdata = 0;
                    soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata, SQUEUEf, 64+i);
                    soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata, FC_ENABLEf, 1 );
                    soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata, FC_INTERFACE_MAPPINGf, 0);

                    soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata,
                                      FC_LOGICAL_OR_MAPPINGf,
                                      (SOC_SBX_CALADAN3_SWS_PR_FC_ENQR_FIFO_XOFF |
                                       SOC_SBX_CALADAN3_SWS_PR_FC_FABRIC_LLFC_XOFF |
                                       SOC_SBX_CALADAN3_SWS_PR_FC_SQUEUE_XOFF));

                    soc_reg32_set(unit, PR_IPRE_FC_CONFIGr, PR_INSTANCE(instance), i, fcdata);
                }
                il_fab_fc_complete = 1;

            } else if (PR_INSTANCE(instance) == PR0_INSTANCE) {
                /* line side = 13 - Port/Channel XOFF state = Enq Req FIFO
                 * XOFF state | Line LLFC XOFF state | Squeue XOFF state
                 */
                if (il_line_fc_complete == 0) {
                    for (i = 0; i < 64; i++) {
                        fcdata = 0;
                        soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata, SQUEUEf, i);
                        soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata, FC_ENABLEf, 1);
                        soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata, FC_INTERFACE_MAPPINGf, 0);

                        soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata,
                                          FC_LOGICAL_OR_MAPPINGf,
                                          (SOC_SBX_CALADAN3_SWS_PR_FC_ENQR_FIFO_XOFF |
                                           SOC_SBX_CALADAN3_SWS_PR_FC_LINE_LLFC_XOFF |
                                           SOC_SBX_CALADAN3_SWS_PR_FC_SQUEUE_XOFF));

                        soc_reg32_set(unit, PR_IPRE_FC_CONFIGr, PR_INSTANCE(instance), i, fcdata);
                    }
                    il_line_fc_complete = 1;
                }
            }
        } else {
            if (port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_XLPORT) {
                /* Flow control channels for XL Ports occupy reg idx 64 and 65. Adjust 
                   as necessary to avoid stepping on ILKN channel space 0-63 */

                if (queueinfo->squeue_base == SOC_SBX_CALADAN3_SWS_XL_PORT_BASE) {
                    idx = 64;
                } else {
                    idx = 65;
                }
            }
            soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata, FC_ENABLEf, fc_enable);
            
            soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata, SQUEUEf,
                              queueinfo->squeue_base);
            
            soc_reg_field_set(unit, PR_IPRE_FC_CONFIGr, &fcdata, 
                              FC_INTERFACE_MAPPINGf, fc_interface_mapping);
            
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IPRE_FC_CONFIGr,
                                              PR_INSTANCE(instance), idx, fcdata));
        }
    }

    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_pr_port_enable(int unit, int port, int enable)
{
    int status, instance;
    int portidx = 0;

    status = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, port, &portidx);
    if (status == SOC_E_NOT_FOUND) {
        return SOC_E_NONE;
    }
    for (instance=0; 
             instance < SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE; 
                 instance++) {
        status = soc_sbx_caladan3_sws_pr_client_enable(unit, instance, 
                                                       portidx, enable);
        if (SOC_FAILURE(status)) {
            return status;
        }
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_pr_port_enable_all(int unit)
{
    soc_port_t port;
    int status;

    SOC_PBMP_ITER(PBMP_ALL(unit), port) {
       status = soc_sbx_caladan3_sws_pr_port_enable(unit, port, TRUE);
       if (SOC_FAILURE(status)) {
           return status;
       }
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_pr_icc_queue_map_set(int unit, int blk, int base_queue)
{
    uint32 regval = 0;
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_ICC_CONFIG1r, 
                            blk, 0, &regval));
    soc_reg_field_set(unit, PR_ICC_CONFIG1r, &regval, QUEUE_BASEf, base_queue);
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_ICC_CONFIG1r, 
                            blk, 0, regval));

    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_pr_enable(int unit, int blk, int enable)
{
    uint32 regval = 0;

    if (enable) {
        soc_reg_field_set(unit, PR_IDP_CONFIG0r, &regval, IDP_ENABLEf, 1);
    } else {
        soc_reg_field_set(unit, PR_IDP_CONFIG0r, &regval, IDP_ENABLEf, 0);
    }
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IDP_CONFIG0r, blk, 0, regval));

    regval = 0;
    if (enable) {
        soc_reg_field_set(unit, PR_HDP_CONFIGr, &regval, HDP_ENABLEf, 1);
    } else {
        soc_reg_field_set(unit, PR_HDP_CONFIGr, &regval, HDP_ENABLEf, 0);
    }
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_HDP_CONFIGr, blk, 0, regval));

    return SOC_E_NONE;
}


int
soc_sbx_caladan3_sws_cmic_hpp_loopback_dir_set(int unit, int ingress, 
                                               int enable)
{
    int queue_base = 0;
    uint32 regval = 0;
    if (enable) {
        queue_base = (ingress > 0) ? SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE :
            (SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + 
             SOC_SBX_CALADAN3_SWS_INGRESS_CMIC_SQUEUE -
             SOC_SBX_CALADAN3_SWS_CMIC_PORT_BASE);

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_ICC_CONFIG0r, 
                                          PR0_INSTANCE, 0, &regval));
        if (ingress <= 0)  {
            soc_reg_field_set(unit, PR_ICC_CONFIG0r, &regval, 
                     OPERATION_TYPEf, SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS);
        } else {

            if (soc_sbx_caladan3_sws_check_icc_state(unit, PR0_INSTANCE) !=
                SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS) {
                soc_reg_field_set(unit, PR_ICC_CONFIG0r, &regval, 
                                  OPERATION_TYPEf, SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE);
            }
        }
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_ICC_CONFIG0r, 
                                          PR0_INSTANCE, 0, regval));
    } else {
        queue_base = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE;

        if (soc_sbx_caladan3_sws_check_icc_state(unit, PR0_INSTANCE) !=
                SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_ICC_CONFIG0r, 
                     PR0_INSTANCE, 0, &regval));
            soc_reg_field_set(unit, PR_ICC_CONFIG0r, &regval, 
                     OPERATION_TYPEf, SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_ICC_CONFIG0r, 
                     PR0_INSTANCE, 0, regval));
        }
    }
    return (soc_sbx_caladan3_sws_pr_icc_queue_map_set(unit, PR0_INSTANCE, 
                                                       queue_base));
}

int
soc_sbx_caladan3_sws_pr_fc_init(int unit, int pr)
{
    uint32 data;
    sws_pr_idp_thresholds_config_t *pr_idp_cfg = NULL;

    data = 0xffffffff;
    soc_reg32_set(unit, PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_031_000r, PR_INSTANCE(pr), 0, data);
    soc_reg32_set(unit, PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_063_032r, PR_INSTANCE(pr), 0, data);
    soc_reg32_set(unit, PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_095_064r, PR_INSTANCE(pr), 0, data);
    soc_reg32_set(unit, PR_IPRE_FC_TX_STATE_ENABLE_SQUEUES_127_096r, PR_INSTANCE(pr), 0, data);

    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IDP_ENQ_REQ_FIFO_NFULL_THRESHOLDr,
                        PR_INSTANCE(pr), 0, &data));
    pr_idp_cfg = soc_sbx_caladan3_sws_pr_idp_cfg_get(unit, pr);
    if ((pr_idp_cfg != NULL) && (pr_idp_cfg->nfull_threshold == 0)) {
        if (pr) {
            soc_reg_field_set(unit, PR_IDP_ENQ_REQ_FIFO_NFULL_THRESHOLDr, &data,
                              NFULL_THRESHOLDf, 228);
        } else {
            soc_reg_field_set(unit, PR_IDP_ENQ_REQ_FIFO_NFULL_THRESHOLDr, &data,
                              NFULL_THRESHOLDf, 22);
        }
    }
    soc_reg32_set(unit, PR_IDP_ENQ_REQ_FIFO_NFULL_THRESHOLDr, PR_INSTANCE(pr), 0, data);

    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_FC_TX_STATE_CONFIG2r, PR_INSTANCE(pr), 0, &data ));
    soc_reg_field_set(unit, PR_IPRE_FC_TX_STATE_CONFIG2r, &data, ENABLE_ENQ_REQ_FIFO_XOFFf, 1);
    if (pr) {
        soc_reg_field_set(unit, PR_IPRE_FC_TX_STATE_CONFIG2r, &data, ENABLE_LINE_LLFCf, 0);
        soc_reg_field_set(unit, PR_IPRE_FC_TX_STATE_CONFIG2r, &data, ENABLE_FABRIC_LLFCf, 1);
    } else {
        soc_reg_field_set(unit, PR_IPRE_FC_TX_STATE_CONFIG2r, &data, ENABLE_LINE_LLFCf, 1);
        soc_reg_field_set(unit, PR_IPRE_FC_TX_STATE_CONFIG2r, &data, ENABLE_FABRIC_LLFCf, 0);
    }
    soc_reg32_set(unit, PR_IPRE_FC_TX_STATE_CONFIG2r, PR_INSTANCE(pr), 0, data );

    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_IPRE_IL_TX_LLFCr, PR_INSTANCE(pr), 0, &data));
    if (pr) {
        /* fabric side = 10 - Port/Channel XOFF state = Enq Req FIFO XOFF state | Fabric LLFC XOFF state */
        soc_reg_field_set(unit, PR_IPRE_IL_TX_LLFCr, &data, FC_LOGICAL_OR_MAPPINGf, 10);
    } else {
        /* line side = 12 - Port/Channel XOFF state = Enq Req FIFO XOFF state | line LLFC XOFF state */
        soc_reg_field_set(unit, PR_IPRE_IL_TX_LLFCr, &data, FC_LOGICAL_OR_MAPPINGf, 12);
    }
    soc_reg_field_set(unit, PR_IPRE_IL_TX_LLFCr, &data, SOFTWARE_CONTROLLED_XOFF_XONf, 0);
    soc_reg32_set(unit, PR_IPRE_IL_TX_LLFCr, PR_INSTANCE(pr), 0, data);

    return SOC_E_NONE;
}

/**
 * Function:
 *     soc_sbx_caladan3_sws_pr_init
 * Purpose:
 *     Initialize Packet Receiver
 */
int soc_sbx_caladan3_sws_pr_init(int unit)
{
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    sws_pr_idp_thresholds_config_t *pr_idp_cfg = NULL;
    sws_pr_port_buffer_t *pr_rx_buf_cfg_ptr = NULL;
    sws_pr_port_buffer_t pr_l_rx_buf_cfg, pr_f_rx_buf_cfg;
    soc_sbx_caladan3_port_map_info_t *port_info = NULL;
    soc_sbx_caladan3_sws_pr_policer_cfg_t *pr_pol_cfg;
    uint32 regval, field;
    int profid, port, instance;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    /* Precompute the pr buffer allocation, use only if needed */
    sal_memset(&pr_l_rx_buf_cfg, 0, sizeof(pr_l_rx_buf_cfg));
    sal_memset(&pr_f_rx_buf_cfg, 0, sizeof(pr_f_rx_buf_cfg));
    soc_sbx_caladan3_pr_port_buffer_alloc(unit, 0, &pr_l_rx_buf_cfg);
    soc_sbx_caladan3_pr_port_buffer_alloc(unit, 1, &pr_f_rx_buf_cfg);

    /* Initialize the port buffer allocations */
    for (instance=0; 
             instance < SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE;
                 instance++) {
        /* PR Profile based buffer allocation */
        if (instance == 0) {
            profid = soc_sbx_caladan3_sws_config_param_data_get(unit, 
                        spn_LINE_PR_BUFFER_PROFILE, -1, 0);
        } else {
            /* Note: No profiles defined for fabric side, 
             * get property approved when profile is defined
             */
            profid = soc_sbx_caladan3_sws_config_param_data_get(unit, 
                        "fabric_pr_buffer_profile", -1, 0);
        }
        if ((!SOC_RECONFIG_TDM)  || (profid <= 0) ||
                 !(soc_sbx_caladan3_sws_is_tdm_reusable(unit))) {
            /* Note: soc_sbx_caladan3_sws_is_tdm_reusable() has to be used with the view
             *       of the individual block reset setting. If the block is reset, 
             *       using the function for selective config does not make sense.
             */
            if (profid > 0) {
                soc_sbx_caladan3_pr_port_buffer_profile_alloc(unit, instance, profid,
                                                              &pr_rx_buf_cfg_ptr);
            } else if (soc_sbx_caladan3_sws_pr_port_buffer_cfg_is_valid(unit, instance)) {
                pr_rx_buf_cfg_ptr = 
                    soc_sbx_caladan3_sws_pr_port_buffer_cfg_get(unit, instance);
                if (pr_rx_buf_cfg_ptr == NULL) {
                    return SOC_E_PARAM;
                }
            } else {
                /* Use precomputed buffer sizing data */
                pr_rx_buf_cfg_ptr = (instance == 0) ? &pr_l_rx_buf_cfg : &pr_f_rx_buf_cfg;
            }
            soc_sbx_caladan3_pr_port_buffer_set(unit, instance, pr_rx_buf_cfg_ptr);
        }
    }

    /* soft reset should be done on soc init */
    if (!SOC_RECONFIG_TDM) {
        for (instance=0; 
                 instance < SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE ; instance++) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PR_GLOBAL_CONFIGr, 
                                PR_INSTANCE(instance), 0, &regval));
            
            soc_reg_field_set(unit, PR_GLOBAL_CONFIGr, &regval, SOFT_RESET_Nf, 1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_GLOBAL_CONFIGr,
                                PR_INSTANCE(instance), 0, regval));

            soc_reg_field_set(unit, PR_GLOBAL_CONFIGr, &regval, INITf, 1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_GLOBAL_CONFIGr,
                                PR_INSTANCE(instance), 0, regval));

            /* wait for init done - global config response */
            SOC_IF_ERROR_RETURN(soc_sbx_caladan3_reg32_expect_field_timeout(unit,
                                PR_GLOBAL_CONFIG_RESPONSEr, instance, 0, 0, 
                                INIT_DONEf, 1,  MILLISECOND_USEC * 200));
        }
    }

    for (instance=0; 
             instance < SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE;
                 instance++) {
	/* clear error registers */
	regval = 0;
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_HDP_ERROR_0r, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit,
            PR_HDP_ERROR_1r, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_ICC_ERROR_0r, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit,
            PR_IDP_ERROR_0r, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_IDP_ERROR_1r, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_IDP_POLICER_ERRORr, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_IPRE_ERROR_0r, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_IPRE_ERROR_1r, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_IPRE_ERROR_2r, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_TRACE_IF_STATUSr, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_HDP_ERROR_0_MASKr, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_HDP_ERROR_1_MASKr, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_ICC_ERROR_0_MASKr, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_IDP_ERROR_1_MASKr, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_IDP_POLICER_ERROR_MASKr, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_IPRE_ERROR_0_MASKr, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_IPRE_ERROR_1_MASKr, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_IPRE_ERROR_2_MASKr, PR_INSTANCE(instance), 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
            PR_TRACE_IF_STATUS_MASKr, PR_INSTANCE(instance), 0, regval));

        /* enable tcam ser recovery, correct single bit error, 
         * invalidate multi-bits error
         */
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, 
                            PR_ICC_LOOKUP_CORE_TCAM_ECC_DEBUG0r, 
                            PR_INSTANCE(instance), 0, &regval));
        soc_reg_field_set(unit, PR_ICC_LOOKUP_CORE_TCAM_ECC_DEBUG0r, 
                          &regval, TCAM_SCRUB_DEC_INVALIDf, 1);
        soc_reg_field_set(unit, PR_ICC_LOOKUP_CORE_TCAM_ECC_DEBUG0r,
                          &regval, TCAM_SCRUB_SEC_INVALIDf, 0);
        soc_reg_field_set(unit, PR_ICC_LOOKUP_CORE_TCAM_ECC_DEBUG0r, 
                          &regval, TCAM_SCRUB_SEC_CORRECTf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
                            PR_ICC_LOOKUP_CORE_TCAM_ECC_DEBUG0r,
                            PR_INSTANCE(instance), 0, regval));

	
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, 
                            PR_ICC_LOOKUP_CORE_TCAM_SCRUB_REQ_PERIODr, 
                            PR_INSTANCE(instance), 0, &regval));
        soc_reg_field_set(unit, 
            PR_ICC_LOOKUP_CORE_TCAM_SCRUB_REQ_PERIODr, &regval, 
            PERIODf, 0x7FFFFFFF);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
                            PR_ICC_LOOKUP_CORE_TCAM_SCRUB_REQ_PERIODr,
                            PR_INSTANCE(instance), 0, regval));	
        if (SOC_RECONFIG_TDM) {
            break;
        }
    }

    /* Configure Default ICC mapping */
    for (instance=0; 
         instance < SOC_SBX_CALADAN3_MAX_SYSTEM_INTF; 
         instance++) {

        int base_port, blk;
        pr_icc_lookup_core_port_defaults_table_entry_t entry;


        for (port = 0; port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {
            port_info = (instance == 0) ? &port_map->line_port_info[port]: \
                                          &port_map->fabric_port_info[port];

            if (!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                continue;
            }

            base_port = port_info->base_port;
            if (base_port < 0) {
                continue;
            }

            if (port_map->line_port_info[port].flags & SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT) {
                continue;
            }


            /* set up default ICC port to queue mapping table */
            blk = soc_sbx_block_find(unit, SOC_BLK_PR, instance);
            SOC_IF_ERROR_RETURN(soc_mem_read(unit, 
                                   PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm, 
                                   blk, base_port, &entry));
            field = port_info->port_queues.squeue_base;
            soc_mem_field_set(unit, PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm, 
                              &entry.entry_data[0], DEFAULT_QUEUEf, &field);
            SOC_IF_ERROR_RETURN(soc_mem_write(unit, 
                                    PR_ICC_LOOKUP_CORE_PORT_DEFAULTS_TABLEm, 
                                    blk, base_port, &entry)); 
        }

        
        /* ICC Policer Operation */
        regval = 0;
        soc_reg_field_set(unit, PR_IDP_POLICER_DISABLEr, &regval, 
                          DISABLE_POLICERf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PR_IDP_POLICER_DISABLEr,
                            PR_INSTANCE(instance), 0, regval));
        pr_pol_cfg = soc_sbx_caladan3_sws_pr_policer_cfg_get(unit, instance);
        if (pr_pol_cfg) {
            sal_memset(pr_pol_cfg, 0, sizeof(soc_sbx_caladan3_sws_pr_policer_cfg_t));
        }

        /* Set ICC state */
        soc_sbx_caladan3_pr_set_icc_state(unit, 
            PR0_INSTANCE, SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS);
        if (!SOC_RECONFIG_TDM) {
            soc_sbx_caladan3_pr_set_icc_state(unit, 
                PR1_INSTANCE, SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS);
        }

        /* DE Thresholds */
        pr_idp_cfg = soc_sbx_caladan3_sws_pr_idp_cfg_get(unit, instance);
        if (pr_idp_cfg == NULL) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Unit %d Failed to locate PR%d IDP thresholds config\n"), 
                      unit, instance));
            return SOC_E_PARAM;
        }
        regval = 0;
        soc_reg_field_set(unit, PR_IDP_CONFIGr, &regval, 
                          DE0_THRESHOLDf, pr_idp_cfg->de0_threshold);
        soc_reg_field_set(unit, PR_IDP_CONFIGr, &regval, 
                          DE1_THRESHOLDf, pr_idp_cfg->de1_threshold);
        soc_reg_field_set(unit, PR_IDP_CONFIGr, &regval, 
                          DE2_THRESHOLDf, pr_idp_cfg->de2_threshold);
        soc_reg_field_set(unit, PR_IDP_CONFIGr, &regval, 
                          DE3_THRESHOLDf, pr_idp_cfg->de3_threshold);
        /* PR 0/1 configuration */
        soc_reg32_set(unit, PR_IDP_CONFIGr, PR_INSTANCE(instance), 0, regval);

        regval = 0;
        soc_reg_field_set(unit, PR_IDP_ENQ_REQ_FIFO_NFULL_THRESHOLDr, 
                          &regval, NFULL_THRESHOLDf,
                          pr_idp_cfg->nfull_threshold);
        soc_reg32_set(unit, PR_IDP_ENQ_REQ_FIFO_NFULL_THRESHOLDr,
                      PR_INSTANCE(instance), 0, regval);

        if (SOC_RECONFIG_TDM) {
            break;
        }
    }

    /* set base queue for pr0/pr1 */
    soc_sbx_caladan3_sws_pr_icc_queue_map_set(unit, PR0_INSTANCE, 
                                              SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE);
    soc_sbx_caladan3_sws_pr_icc_queue_map_set(unit, PR1_INSTANCE, 
                                                   SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE);

    /* FC enable */
    for (instance=0; 
             instance < SOC_SBX_CALADAN3_MAX_SYSTEM_INTF; 
                 instance++) {
        soc_sbx_caladan3_sws_pr_fc_init(unit, instance);
    }

    soc_sbx_caladan3_sws_pr_icc_program_port_match_entries(unit, 0, 0, 0);

    /* Enable All ports */
    soc_sbx_caladan3_sws_pr_port_enable_all(unit);

    /* enable PRs */
    if (!SOC_RECONFIG_TDM) {
        soc_sbx_caladan3_sws_pr_enable(unit, PR1_INSTANCE, 1);
        soc_sbx_caladan3_sws_pr_enable(unit, PR0_INSTANCE, 1);
    }

    return SOC_E_NONE;
}


#endif


