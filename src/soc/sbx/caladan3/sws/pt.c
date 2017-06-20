/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: pt.c,v 1.8.16.18 Broadcom SDK $
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

#define PT1_INSTANCE  SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(1)
#define PT0_INSTANCE  SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(0)
#define PT_INSTANCE(inst) SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE((inst))
#define MAX_PT        (SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE)
#define MAX_CALENDAR  (2)




typedef struct pt_ipte_grp_config {
    int grp0_base;
    int grp0_range;
    int grp1_base;
    int grp1_range;
} pt_ipte_grp_config_t;



/* One for PT0 and one for PT1 */
static pt_ipte_grp_config_t pt_ipte_grp[SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE] = {{0}};

static int pt_ipte_init = 0;
#define MAX_PT_QS 64


int
soc_sbx_caladan3_sws_pt_wdrr_scheduler_config(int unit, int pt,
                                              int base_port, int enable);
int
soc_sbx_caladan3_sws_pt_strict_priority_config(int unit, int pt, int base_port,
                                               int spr0_qid, int spr1_qid);


#define CHECK_AND_UPDATE_SQ_INFO(q1, cur, q2, cnt)             \
            /* This assumes consecutive numbering*/            \
            if ((cur)) {                                       \
                if ((q1) == (q2)) {                            \
                    (cnt) = (cur);                             \
                } else if ((q1) < (q2)) {                      \
                    (cnt) = (((q1)+(cur)) > ((q2)+(cnt))) ?    \
                             (cur) : ((q2) + (cnt) - (q1));    \
                    (q2) = (q1);                               \
                } else if ((q1) > (q2)) {                      \
                    (cnt) = (((q1)+(cur)) > ((q2)+(cnt))) ?    \
                             ((q1) + (cur) - (q2)) : (cnt);    \
                }                                              \
            }

/*
 * PT Control Block
 */
typedef struct _pt_cb_s {

    /* Memory that is allocated for DMA block access */
    ipte_port_cal_entry_t    *port_entries[MAX_CALENDAR];
    ipte_client_cal_entry_t  *client_entries[MAX_CALENDAR];

    /* Memories */
    soc_mem_t client_cal[MAX_CALENDAR][MAX_PT];
    soc_mem_t port_cal[MAX_CALENDAR][MAX_PT];

    /* Current active calendar 0/1 */
    uint8 active_client_cal[MAX_PT];
    uint8 active_port_cal[MAX_PT];
    uint8 in_use;
    uint8 max_calendar;
    uint8 remap_en;

} pt_cb_t;

/* Per unit CB */
pt_cb_t  _pt_control[SOC_MAX_NUM_DEVICES];

#define PT_CB_INUSE(unit)   (_pt_control[(unit)].in_use)
#define PT_CB_MAX_CAL(unit) (_pt_control[(unit)].max_calendar)
#define PT_REMAP_ENABLED(unit) (_pt_control[(unit)].remap_en)

#define INACTIVE_CAL_ID(unit, cal) (((cal)+1)%(PT_CB_MAX_CAL(unit)))

/* Switch the Active/inactive calendar */
#define ACTIVE_CLIENT_CAL_ID(unit, inst)        (_pt_control[(unit)].active_client_cal[(inst)])
#define ACTIVE_PORT_CAL_ID(unit, inst)          (_pt_control[(unit)].active_port_cal[(inst)])
#define INACTIVE_CLIENT_CAL_ID(unit, inst) INACTIVE_CAL_ID((unit),_pt_control[(unit)].active_client_cal[(inst)])
#define INACTIVE_PORT_CAL_ID(unit, inst)   INACTIVE_CAL_ID((unit),_pt_control[(unit)].active_port_cal[(inst)])

/* Get the Active/inactive calendar */
#define INACTIVE_PORT_CAL(unit, inst)   \
           (_pt_control[(unit)].port_cal[INACTIVE_CAL_ID((unit),ACTIVE_PORT_CAL_ID((unit),(inst)))][(inst)])
#define INACTIVE_CLIENT_CAL(unit, inst) \
           (_pt_control[(unit)].client_cal[INACTIVE_CAL_ID((unit),ACTIVE_CLIENT_CAL_ID((unit),(inst)))][(inst)])
#define ACTIVE_CLIENT_CAL(unit, inst)   \
           (_pt_control[(unit)].client_cal[ACTIVE_CLIENT_CAL_ID((unit),(inst))][(inst)])
#define ACTIVE_PORT_CAL(unit, inst)     \
           (_pt_control[(unit)].port_cal[ACTIVE_PORT_CAL_ID((unit),(inst))][(inst)])


/* Get the Active/inactive entries */
#define CLIENT_ENTRIES(unit, inst)  \
            (_pt_control[(unit)].client_entries[INACTIVE_CAL_ID((unit),ACTIVE_CLIENT_CAL_ID((unit),(inst)))])
#define PORT_ENTRIES(unit, inst)    \
            (_pt_control[(unit)].port_entries[INACTIVE_CAL_ID((unit),ACTIVE_PORT_CAL_ID((unit),(inst)))])


/*
 * Function: soc_sbx_caladan3_sws_pt_remap_enabled
 * Purpose: Return True if Caladan3 PT Remap Enabled
 */
int
soc_sbx_caladan3_sws_pt_remap_enabled(int unit) {
    return (PT_REMAP_ENABLED(unit) ? 1 : 0);
}

/*
 * Function:
 *    soc_sbx_caladan3_pt_port_update
 * Purpose
 *    Update port fifo and/or credits
 *    update_* must be set to trigger update
 *    page_threshold, size and base required for fifo update
 *    credit value is required for credit update
 *    Note:
 *       Setting Arbitrary values for credit can break the system
 *       Preferably, Credits should be issued by the mac.
 */
int
soc_sbx_caladan3_pt_port_update(int unit, int instance, int port,
                                int page_threshold, int size, int base,
                                int update_fifo,
                                int credits, int update_credits)
{
    uint32 regval = 0;

    if (!SOC_IS_CALADAN3_REVB(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Cannot update fifo/credits in rev A devices\n"), unit));
        return SOC_E_PARAM;
    }
    update_fifo = (update_fifo > 0) ? 1 : 0;
    update_credits = (update_credits > 0) ? 1: 0;
    if (update_fifo) {
        soc_reg_field_set(unit, PT_IPTE_PORT_FIFO_CFGr, &regval,
                          PKT_SERVICE_PAGES_THRESHOLDf,
                          page_threshold);
        soc_reg_field_set(unit, PT_IPTE_PORT_FIFO_CFGr, &regval,
                          SIZEf, size);
        soc_reg_field_set(unit, PT_IPTE_PORT_FIFO_CFGr, &regval,
                          BASE_ADDRESSf, base);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit,  PT_IPTE_PORT_FIFO_CFGr,
                                PT_INSTANCE(instance), port, regval));
    }
    if ((update_fifo) || (update_credits)) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit,  PT_IPTE_PORT_RECFGr,
                                PT_INSTANCE(instance), port, &regval));
        soc_reg_field_set(unit, PT_IPTE_PORT_RECFGr, &regval,
                          FIFO_UPDATE_PORTf, port);
        soc_reg_field_set(unit, PT_IPTE_PORT_RECFGr, &regval,
                          FIFO_UPDATEf, update_fifo);
        soc_reg_field_set(unit, PT_IPTE_PORT_RECFGr, &regval,
                          CREDIT_UPDATE_PORTf, port);
        soc_reg_field_set(unit, PT_IPTE_PORT_RECFGr, &regval,
                          CREDIT_UPDATE_VALUEf, credits);
        soc_reg_field_set(unit, PT_IPTE_PORT_RECFGr, &regval,
                          CREDIT_UPDATEf, update_credits);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit,  PT_IPTE_PORT_RECFGr,
                                PT_INSTANCE(instance), port, regval));
    }

    return SOC_E_NONE;
}



/*
 * Function:
 *    soc_sbx_caladan3_pt_port_fifo_set
 * Purpose
 *    program port fifos
 */
int
soc_sbx_caladan3_pt_port_fifo_set(int unit, int instance,
                                  sws_pt_port_fifo_t *pt_fifo)
{
    int status = SOC_E_NONE;
    int idx;
    uint32 regval;
    pt_port_fifo_entry_t *fifo_entry;

    for (idx = 0; idx < pt_fifo->num_elements; idx++) {
        fifo_entry = &pt_fifo->entry[idx];
        regval = 0;
        soc_reg_field_set(unit, PT_IPTE_PORT_FIFO_CFGr, &regval,
                          PKT_SERVICE_PAGES_THRESHOLDf, 
                          fifo_entry->page_threshold);
        if (fifo_entry->size & 1) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Fifo entry %d size is odd sized\n"),
                       unit, idx));
            return SOC_E_PARAM;
        }
        if (fifo_entry->size > 0) {
            soc_reg_field_set(unit, PT_IPTE_PORT_FIFO_CFGr, &regval,
                              SIZEf, fifo_entry->size);
            soc_reg_field_set(unit, PT_IPTE_PORT_FIFO_CFGr, &regval, 
                              BASE_ADDRESSf, fifo_entry->base);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit,  PT_IPTE_PORT_FIFO_CFGr,
                                PT_INSTANCE(instance), fifo_entry->port, regval));
        }
    }
    if (pt_fifo->num_elements > 0) {
        regval = 0;
        soc_reg_field_set(unit, PT_IPTE_PORT_FIFO_INITr, &regval, 
                          PT_PORT_FIFO_INITf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit,  PT_IPTE_PORT_FIFO_INITr,
                            PT_INSTANCE(instance), 0, regval));
    }
    return status;
}

/*
 * Function:
 *    soc_sbx_caladan3_pt_port_fifo_set_hotswap
 * Purpose
 *    Program port fifos only for ports that are new.
 */
int
soc_sbx_caladan3_pt_port_fifo_set_hotswap(int unit, int instance,
                                          sws_pt_port_fifo_t *pt_fifo)
{
    int                     status = SOC_E_NONE;
    int                     idx;
    uint32                  regval;
    pt_port_fifo_entry_t    *fifo_entry;
    uint32                  hw_size;
    uint32                  hw_page_threshold;
    uint32                  hw_base;

    for (idx = 0; idx < pt_fifo->num_elements; idx++) {
        fifo_entry = &pt_fifo->entry[idx];

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit,  PT_IPTE_PORT_FIFO_CFGr,
                            PT_INSTANCE(instance), fifo_entry->port, &regval));
        hw_size = soc_reg_field_get(unit, PT_IPTE_PORT_FIFO_CFGr,
                                    regval, SIZEf);
        hw_page_threshold = soc_reg_field_get(unit, PT_IPTE_PORT_FIFO_CFGr,
                                    regval, PKT_SERVICE_PAGES_THRESHOLDf);
        hw_base = soc_reg_field_get(unit, PT_IPTE_PORT_FIFO_CFGr,
                                    regval, BASE_ADDRESSf);
        if (hw_size == fifo_entry->size &&
            hw_page_threshold == fifo_entry->page_threshold &&
            hw_base == fifo_entry->base) {
            continue;
        }

        if (fifo_entry->size & 1) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Fifo entry %d size is odd sized\n"),
                       unit, idx));
            return SOC_E_PARAM;
        }
        if (fifo_entry->size > 0) {
            soc_sbx_caladan3_pt_port_update(unit, instance,
                                            fifo_entry->port,
                                            fifo_entry->page_threshold,
                                            fifo_entry->size,
                                            fifo_entry->base,
                                            TRUE, 0, FALSE);
        }


    }
    return status;
}

/*
 * Function:
 *    soc_sbx_caladan3_pt_port_credit_set_hotswap
 * Purpose
 *    Program credits for new ports as part of hotswap.
 */
int
soc_sbx_caladan3_pt_port_credit_set_hotswap(int unit)
{
    int                                 instance = 0;
    soc_sbx_caladan3_port_map_t         *port_map;
    soc_sbx_caladan3_port_map_info_t    *port_info;
    int                                 port;
    int                                 credits;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    for (port=0; port < SOC_SBX_CALADAN3_MAX_LINE_PORT; port++) {
        port_info = &port_map->line_port_info[port];
        if ((!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) ||
            SOC_PBMP_MEMBER(SOC_CONTROL(unit)->mac_phy_skip_pbm, port)) {
            continue;
        }

        switch (port_info->intftype) {
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE:
            case SOC_SBX_CALADAN3_PORT_INTF_IL100:
                credits = 16;
                break;
            default:
                credits = 4;
                break;
        }

        soc_sbx_caladan3_pt_port_update(unit, instance,
                                        port,
                                        0, 0, 0, FALSE,
                                        credits, TRUE);

    }

    return SOC_E_NONE;
}



/*
 * Function:
 *    soc_sbx_caladan3_pt_port_fifo_alloc
 * Purpose
 *    allocate port fifos
 */

#define SET_FIFO_ENTRY(f, p, off, np)          \
            (f)->port = (p);                   \
            (f)->base = (off);                 \
            (f)->size = (np);                  \
            (f)->page_threshold =              \
                (f->size % 4) ? ((f->size+4) >> 2) : (f->size >> 2);

int
soc_sbx_caladan3_pt_port_fifo_alloc(int unit, int instance,
                                    sws_pt_port_fifo_t *pt_fifo)
{
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_port_map_info_t *port_info;
    int idx, port, npages = 0, offset = 0;
    pt_port_fifo_entry_t *fifo_entry;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    for (idx=0, port=0; port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {
        port_info = &port_map->line_port_info[port];
        if (!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        if ((port_info->physical_intf == instance) &&
             (port_info->base_port >=0)) {
            switch (port_info->intftype) {

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                if ((port_info->bindex != 0) &&
                        (port_info->bindex != 4) &&
                            (port_info->bindex != 8)) {
                    continue;
                }
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
            case SOC_SBX_CALADAN3_PORT_INTF_IL100:
                if (port_info->flags & SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT) {continue;} 
                if (port_info->bindex != 0) { continue; }
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_XLPORT:
                if (port_info->bindex & 1) { continue; }
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
                if (port_info->flags & SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT) {continue;} 
                break;
            default:
                ;
            }

            npages = port_map->intf_attr[port_info->intftype].pt_fifo_level;
            npages = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_TX_FIFO_SIZE,
                                                                port_info->port, npages);
            fifo_entry = &pt_fifo->entry[idx];
            SET_FIFO_ENTRY(fifo_entry, 
                           port_info->base_port,
                           offset, npages);
            offset += npages * 8;
            idx++;
        }
        port_info = &port_map->fabric_port_info[port];
        if ((port_info->physical_intf == instance) &&
                 (port_info->base_port >=0)) {
            switch (port_info->intftype) {
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                if ((port_info->bindex != 0) &&
                        (port_info->bindex != 4) &&
                            (port_info->bindex != 8)) {
                    continue;
                }
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
            case SOC_SBX_CALADAN3_PORT_INTF_IL100:
                if (port_info->bindex != 0) { continue; }
                break;
            default:
                ;
            }
            npages = port_map->intf_attr[port_info->intftype].pt_fifo_level;
            npages = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_TX_FIFO_SIZE,
                                                                port_info->port, npages);
            fifo_entry = &pt_fifo->entry[idx];
            SET_FIFO_ENTRY(fifo_entry, 
                           port_info->base_port,
                           offset, npages);
            offset += npages * 8;
            idx++;
        }
    }
    if (offset > SOC_SBX_CALADAN3_SWS_PT_PORT_FIFO_DEPTH_32B) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d port_fifo_alloc page allocation(%d) exceeded system limits(%d)\n"), 
                   unit, offset, SOC_SBX_CALADAN3_SWS_PT_PORT_FIFO_DEPTH_PAGES));
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Please check config\n"), unit));
        return SOC_E_PARAM;
    }
    pt_fifo->num_elements = idx;
    return SOC_E_NONE;
}


/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_property_get_csv  
 * Purpose
 *    Get override parameters from the Config.bcm
 *    This is generic low level function can be used for any property
 */
int
soc_sbx_caladan3_sws_pt_property_get_csv(int unit, char *string, int num, int *array)
{
    int cnt = 0;
    char buffer[128];
    int id;
 
    /* Per TDM property if found is used, if not, global property is looked up */
    id = soc_sbx_caladan3_sws_tdm_id_current(unit);
    sal_sprintf(buffer, "%s.%d", string, id);
    cnt = soc_property_get_csv(unit, buffer, num, array);
    if (cnt <= 0) {
        cnt = soc_property_get_csv(unit, string, num, array);
    }
    return cnt;
}

/*
 * Function:
 *    sws_pt_port_calendar_enable  
 * Purpose
 *    Set up the port calendar limits and enable it
 *    Internal routine
 */
int
sws_pt_port_calendar_enable(int unit, int instance, int num_elements)
{
    uint32 regval = 0;
    int revb;
    int pt_blk = soc_sbx_block_find(unit, SOC_BLK_PT, instance);

    if (1 <= num_elements) {
        revb = SOC_IS_CALADAN3_REVB(unit);
        if (revb && INACTIVE_PORT_CAL_ID(unit, instance)) {
            soc_reg_field_set(unit, PT_IPTE_DS_PORT_CAL1_CFGr, &regval,
                              PT_IPTE_PORT_CAL1_ENf, 1);
            soc_reg_field_set(unit, PT_IPTE_DS_PORT_CAL1_CFGr, &regval,
                              PT_IPTE_PORT_CAL1_END_ADDRf, num_elements-1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_DS_PORT_CAL1_CFGr,
                                PT_INSTANCE(instance), 0, regval));
        } else {
            soc_reg_field_set(unit, PT_IPTE_DS_PORT_CAL_CFGr, &regval,
                              PT_IPTE_PORT_CAL_ENf, 1);
            soc_reg_field_set(unit, PT_IPTE_DS_PORT_CAL_CFGr, &regval,
                              PT_IPTE_PORT_CAL_END_ADDRf, num_elements-1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_DS_PORT_CAL_CFGr,
                                PT_INSTANCE(instance), 0, regval));
        }
        if (revb) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG4r,
                                   PT_INSTANCE(instance), 0, &regval));
            soc_reg_field_set(unit, PT_IPTE_CONFIG4r, &regval,
                              PT_IPTE_PORT_CAL_SELf,
                              INACTIVE_PORT_CAL_ID(unit, instance));
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG4r,
                                PT_INSTANCE(instance), 0, regval));
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG4r,
                                PT_INSTANCE(instance), 0, &regval));
            ACTIVE_PORT_CAL_ID(unit, instance) =
               soc_reg_field_get(unit, PT_IPTE_CONFIG4r, regval,
                                 PT_IPTE_ACTIVE_PORT_CALf);
            SOC_IF_ERROR_RETURN(soc_mem_clear(unit,
                                INACTIVE_PORT_CAL(unit, instance), pt_blk, 0));
        }

    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_calendar_config
 * Purpose
 *    configure the port calendar entries and activate the calendar
 */
int
soc_sbx_caladan3_sws_pt_port_calendar_config(int unit, int instance, 
                                             sws_pt_port_cal_t *cal)
{
    int idx, enable = 0;
    int pt_blk = soc_sbx_block_find(unit, SOC_BLK_PT, instance);
    int cnt = 0, port_cal[256] = {0};
    ipte_port_cal_entry_t *entry;
    soc_error_t status;

    sal_memset(PORT_ENTRIES(unit, instance), 0,
                   sizeof(ipte_port_cal_entry_t) *
                       soc_mem_index_count(unit,IPTE_PORT_CALm));
    sal_memset(&port_cal[0], 0, sizeof(port_cal));
    if (instance) {
        cnt = soc_sbx_caladan3_sws_pt_property_get_csv(unit,
                  spn_FABRIC_PORT_CALENDAR, 256, &port_cal[0]);
    } else {
        cnt = soc_sbx_caladan3_sws_pt_property_get_csv(unit,
                  spn_LINE_PORT_CALENDAR, 256, &port_cal[0]);
    }
    if (cnt > 0) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d Loaded Port Calendar on pt%d, %d entries\n"),
                     unit, instance, cnt));
        for (idx=0; idx < cnt; idx++) {
            enable = (port_cal[idx] != SOC_SBX_CALADAN3_SWS_PT_PORTX);
            entry = soc_mem_table_idx_to_pointer(unit,
                                                INACTIVE_PORT_CAL(unit, instance),
                                                ipte_port_cal_entry_t *,
                                                PORT_ENTRIES(unit, instance), idx);
            if (enable) {
                soc_mem_field_set(unit, INACTIVE_PORT_CAL(unit, instance),
                                  &entry->entry_data[0],
                                  PORTf, (uint32*)&port_cal[idx]);
            }
            soc_mem_field_set(unit, INACTIVE_PORT_CAL(unit, instance),
                              &entry->entry_data[0],
                              ACTIVEf, (uint32*)&enable);
        }
        /*    coverity[negative_returns : FALSE]    */
        status = soc_mem_write_range(unit, INACTIVE_PORT_CAL(unit, instance),
                                     pt_blk, 0, cnt-1,
                                     PORT_ENTRIES(unit, instance));
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: %s: soc_mem_write_range failed: %s\n"),
                       unit, FUNCTION_NAME(), soc_errmsg(status)));
            return status;
        }
        sws_pt_port_calendar_enable(unit, instance, cnt);
        return SOC_E_NONE;
    }

    for (idx=0; idx < cal->num_elements; idx++) {
        enable = (cal->port_id[idx] != SOC_SBX_CALADAN3_SWS_PT_PORTX);
        entry = soc_mem_table_idx_to_pointer(unit,
                              INACTIVE_PORT_CAL(unit, instance),
                              ipte_port_cal_entry_t *,
                              PORT_ENTRIES(unit, instance), idx);
        if (enable) {
            soc_mem_field_set(unit, INACTIVE_PORT_CAL(unit, instance),
                              &entry->entry_data[0],
                              PORTf, (uint32*)&cal->port_id[idx]);
        }
        soc_mem_field_set(unit, INACTIVE_PORT_CAL(unit, instance),
                          &entry->entry_data[0],
                          ACTIVEf, (uint32*)&enable);
    }
    if (cal->num_elements > 0) {
        /*    coverity[negative_returns : FALSE]    */
        status = soc_mem_write_range(unit, INACTIVE_PORT_CAL(unit, instance),
                                     pt_blk, 0, cal->num_elements-1,
                                     PORT_ENTRIES(unit, instance));
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: %s: soc_mem_write_range failed: %s\n"),
                       unit, FUNCTION_NAME(), soc_errmsg(status)));
                return status;
        }
        sws_pt_port_calendar_enable(unit, instance, cal->num_elements);
    } else {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d PT %d Port Calendar not setup\n"),
                  unit, instance));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_calendar_clear_internal
 * Purpose
 *    Clear a given port calendar enty slot
 */
int
soc_sbx_caladan3_sws_pt_port_calendar_clear_internal(int unit, int instance,
                                                     int portidx,
                                                     ipte_port_cal_entry_t *entries)
{
    int port, idx;
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    soc_sbx_caladan3_port_map_info_t *port_info = NULL;
    ipte_port_cal_entry_t *entry;
    int max;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    if (portidx >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
        return SOC_E_NONE;
    }
    if (instance) {
        port_info = &port_map->fabric_port_info[portidx];
    } else {
        port_info = &port_map->line_port_info[portidx];
    }
    if (!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
        return SOC_E_PARAM;
    }

    max = soc_mem_index_count(unit, IPTE_PORT_CALm);
    for (idx=0; idx < max; idx++) {
        entry = soc_mem_table_idx_to_pointer(unit,
                              IPTE_PORT_CALm,
                              ipte_port_cal_entry_t *,
                              entries, idx);
        port = soc_mem_field32_get(unit, IPTE_PORT_CALm, 
                                 &entry->entry_data[0],
                                 PORTf);
        if (port == port_info->port) {
            soc_mem_field32_set(unit, IPTE_PORT_CALm, &entry->entry_data[0], 
                              PORTf, 0);
            soc_mem_field32_set(unit, IPTE_PORT_CALm, &entry->entry_data[0], 
                              ACTIVEf, 0);
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    sws_pt_client_calendar_entry_set
 * Purpose
 *    Set up the given client calendar entry
 *    Internal routine
 */
int
sws_pt_client_calendar_entry_set(int unit, int instance, int idx,
                                 uint32 client0, uint32 enable0, 
                                 uint32 client1, uint32 enable1)
{
    ipte_client_cal_entry_t entry;
    int blk = soc_sbx_block_find(unit, SOC_BLK_PT, instance);

    sal_memset(&entry, 0, sizeof(ipte_client_cal_entry_t));

    soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance),
                      &entry.entry_data[0],
                      CLIENT0f, &client0);
    soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance),
                      &entry.entry_data[0],
                      ACTIVE0f, &enable0);
    soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance),
                      &entry.entry_data[0],
                      CLIENT1f, &client1);
    soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance),
                      &entry.entry_data[0],
                      ACTIVE1f, &enable1);
    SOC_IF_ERROR_RETURN(soc_mem_write(unit,
                        INACTIVE_CLIENT_CAL(unit, instance),
                        blk, idx, &entry));

    return SOC_E_NONE;
}

/*
 * Function:
 *    sws_pt_client_calendar_entry_enable
 * Purpose
 *    Set up the client calendar limits and enable
 *    Internal routine
 */
int
sws_pt_client_calendar_enable(int unit, int instance,
                                               int num_elements)
{
    int revb;
    uint32 regval= 0;
    int pt_blk = soc_sbx_block_find(unit, SOC_BLK_PT, instance);

    if (1 < num_elements) {
        revb = SOC_IS_CALADAN3_REVB(unit);
        if (revb && INACTIVE_CLIENT_CAL_ID(unit, instance)) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CLIENT_CAL1_CFGr,
                                PT_INSTANCE(instance), 0, &regval));
            soc_reg_field_set(unit, PT_IPTE_CLIENT_CAL1_CFGr, &regval,
                              CAL1_END_ADDRf, num_elements - 1);
            soc_reg_field_set(unit, PT_IPTE_CLIENT_CAL1_CFGr,
                              &regval, CAL1_ENf, 1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CLIENT_CAL1_CFGr,
                                PT_INSTANCE(instance), 0, regval));
        } else {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CLIENT_CAL_CFGr,
                                PT_INSTANCE(instance), 0, &regval));
            soc_reg_field_set(unit, PT_IPTE_CLIENT_CAL_CFGr, &regval,
                              CAL_END_ADDRf, num_elements - 1);
            soc_reg_field_set(unit, PT_IPTE_CLIENT_CAL_CFGr,
                              &regval, CAL_ENf, 1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CLIENT_CAL_CFGr,
                                PT_INSTANCE(instance), 0, regval));
        }
        if (revb) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG4r,
                                    PT_INSTANCE(instance), 0, &regval));
            soc_reg_field_set(unit, PT_IPTE_CONFIG4r, &regval,
                              PT_IPTE_CLIENT_CAL_SELf,
                              INACTIVE_CLIENT_CAL_ID(unit, instance));
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG4r,
                                    PT_INSTANCE(instance), 0, regval));
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG4r,
                                   PT_INSTANCE(instance), 0, &regval));
            ACTIVE_CLIENT_CAL_ID(unit, instance) =
                 soc_reg_field_get(unit, PT_IPTE_CONFIG4r, regval,
                                   PT_IPTE_ACTIVE_CLIENT_CALf);
            SOC_IF_ERROR_RETURN(soc_mem_clear(unit,
                                INACTIVE_CLIENT_CAL(unit, instance), pt_blk, 0));
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *   soc_sbx_caladan3_sws_pt_client_calendar_config 
 * Purpose
 *    Set up the client calendar limits and enable
 */
int
soc_sbx_caladan3_sws_pt_client_calendar_config(int unit, int instance,
                                          sws_pt_client_cal_t *cal)
{
    int calidx, idx, en0 = 0, en1 = 0;
    int cnt = 0, client_cal[256] = {0};
    int client0, client1;
    int pt_blk = soc_sbx_block_find(unit, SOC_BLK_PT, instance);
    ipte_client_cal_entry_t *entry;
    int status = SOC_E_NONE;
    
    sal_memset(CLIENT_ENTRIES(unit, instance), 0,
                   sizeof(ipte_client_cal_entry_t) *
                       soc_mem_index_count(unit,IPTE_CLIENT_CALm));

    if (instance) {
        cnt = soc_sbx_caladan3_sws_pt_property_get_csv(unit,
                  spn_FABRIC_CLIENT_CALENDAR, 256, &client_cal[0]);
    } else {
        cnt = soc_sbx_caladan3_sws_pt_property_get_csv(unit,
                  spn_LINE_CLIENT_CALENDAR, 256, &client_cal[0]);
    }
    if (cnt > 0) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d Loaded client calendar on pt%d, %d entries\n"), 
                     unit, instance, cnt));
        if ((cnt & 1) && 
             (cnt < SOC_SBX_CALADAN3_SWS_PT_MAX_CLIENT_CALENDAR)) {
            client_cal[cnt++] = CLIENTX;
        }
        for (idx=0, calidx=0; idx < cnt; idx+=2, calidx++) {
            en0 = (client_cal[idx] != CLIENTX);
            en1 = (client_cal[idx+1] != CLIENTX);
            client0 = (en0) ? client_cal[idx] : 0;
            client1 = (en1) ? client_cal[idx+1] : 0;
            entry = soc_mem_table_idx_to_pointer(unit,
                                                INACTIVE_CLIENT_CAL(unit, instance),
                                                ipte_client_cal_entry_t *,
                                                CLIENT_ENTRIES(unit, instance), calidx);
            soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance), &entry->entry_data[0],
                              CLIENT0f, (uint32*)&client0);
            soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance), &entry->entry_data[0], 
                              ACTIVE0f, (uint32*)&en0);
            soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance), &entry->entry_data[0],
                              CLIENT1f, (uint32*)&client1);
            soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance), &entry->entry_data[0], 
                              ACTIVE1f, (uint32*)&en1);
        }
        /*    coverity[negative_returns : FALSE]    */
        status = soc_mem_write_range(unit, INACTIVE_CLIENT_CAL(unit, instance), pt_blk,
                                     0, cnt-1, CLIENT_ENTRIES(unit, instance));
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: %s: soc_mem_write_range failed: %s\n"),
                       unit, FUNCTION_NAME(), soc_errmsg(status)));
            return status;
        }
        sws_pt_client_calendar_enable(unit, instance, cnt);
        return SOC_E_NONE;
    }

    if ((cal->num_elements & 1) && 
         (cal->num_elements < SOC_SBX_CALADAN3_SWS_PT_MAX_CLIENT_CALENDAR)) {
        cal->client_id[cal->num_elements++] = CLIENTX;
    }
    for (idx=0, calidx=0; idx < cal->num_elements; idx+=2, calidx++) {
        en0 = (cal->client_id[idx] != CLIENTX);
        en1 = (cal->client_id[idx+1] != CLIENTX);
        client0 = (en0) ? cal->client_id[idx] : 0;
        client1 = (en1) ? cal->client_id[idx+1] : 0;
        entry = soc_mem_table_idx_to_pointer(unit,
                                            INACTIVE_CLIENT_CAL(unit, instance),
                                            ipte_client_cal_entry_t *,
                                            CLIENT_ENTRIES(unit, instance), calidx);
        soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance), &entry->entry_data[0],
                          CLIENT0f, (uint32*)&client0);
        soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance), &entry->entry_data[0], 
                          ACTIVE0f, (uint32*)&en0);
        soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance), &entry->entry_data[0],
                          CLIENT1f, (uint32*)&client1);
        soc_mem_field_set(unit, INACTIVE_CLIENT_CAL(unit, instance), &entry->entry_data[0], 
                          ACTIVE1f, (uint32*)&en1);
    }
    if (cal->num_elements > 0) {
        /*    coverity[negative_returns : FALSE]    */
        status = soc_mem_write_range(unit, INACTIVE_CLIENT_CAL(unit, instance), pt_blk,
                                 0, cal->num_elements-1, CLIENT_ENTRIES(unit, instance));
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: %s: soc_mem_write_range failed: %s\n"),
                       unit, FUNCTION_NAME(), soc_errmsg(status)));
            return status;
        }
        sws_pt_client_calendar_enable(unit, instance, cal->num_elements);
    } else {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d PT %d Client Calendar not setup\n"),
                  unit, instance));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_config_clear_internal
 * Purpose
 *    Clear the port configuration, on B0/B1 this updates Q2C table
 *    On A0/A1 this updates the port queue base and range
 *    Internal routine
 */
int
soc_sbx_caladan3_sws_pt_port_config_clear_internal(int unit, int instance, int port)
{
    uint32 config = 0;
    int base_port = 0, cur_port, index;
    soc_sbx_caladan3_port_map_info_t *pi;
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    ipte_q2c_map_entry_t entry;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    if (port >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
        return SOC_E_NONE;
    }
    if (instance) {
        pi = &port_map->fabric_port_info[port];
    } else {
        pi = &port_map->line_port_info[port];
    }
    if (!(pi->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
        return SOC_E_PARAM;
    }
    base_port = pi->base_port;
    instance = pi->physical_intf;

    if ((pi->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE) ||
        (pi->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25) ||
        (pi->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42)) {
        if (PT_REMAP_ENABLED(unit)) {
            base_port = base_port >> 2;
        }
    }

    if (SOC_IS_CALADAN3_REVB(unit)) {
        if (base_port < SOC_SBX_CALADAN3_SWS_XT_PORT_BASE) {
            /* loop through IPTE_Q2C_MAP, clear all related entries by set port to 0xF */
            for (index = 0; index < soc_mem_index_count(unit, IPTE_Q2C_MAPm); index++) {
                
                SOC_IF_ERROR_RETURN(soc_mem_read(unit, IPTE_Q2C_MAPm,
                                                 soc_sbx_block_find(unit, SOC_BLK_PT, instance),
                                                 index, &entry));
                
                cur_port = soc_mem_field32_get(unit, IPTE_Q2C_MAPm, &entry, PORTf);
                
                if ((cur_port >= 0) && (cur_port <= 2)) {
                    soc_mem_field32_set(unit, IPTE_Q2C_MAPm, &entry, FC_INDEXf, 0);
                    soc_mem_field32_set(unit, IPTE_Q2C_MAPm, &entry, PORTf, 0xF);
                    SOC_IF_ERROR_RETURN(soc_mem_write(unit, IPTE_Q2C_MAPm,
                                                      soc_sbx_block_find(unit, SOC_BLK_PT, instance),
                                                      index, &entry));
                }
            }
        } else if (base_port < SOC_SBX_CALADAN3_SWS_XL_PORT_BASE) {
            /* clear 12-47, only clear it when the base port match
             * this implies that the base_port should be the last one
             * cleared.
             */
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG4r,
                                              PT_INSTANCE(instance), 0, &config));
            cur_port = soc_reg_field_get(unit, PT_IPTE_CONFIG4r, config,
                                         PT_IPTE_PORT12_PORT47_BASE_PORTf);
            if (base_port == cur_port) {
                soc_reg_field_set(unit, PT_IPTE_CONFIG4r, &config,
                                  PT_IPTE_PORT12_PORT47_BASE_QUEUEf, 0);
                soc_reg_field_set(unit, PT_IPTE_CONFIG4r, &config,
                                  PT_IPTE_PORT12_PORT47_BASE_PORTf, 0);
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG4r,
                                                  PT_INSTANCE(instance), 0, config));
            }
        } else if (base_port == SOC_SBX_CALADAN3_SWS_XL_PORT_BASE) {
            if (instance == 0) {
                /* clear 48-50 */
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG2r,
                                                  PT_INSTANCE(instance), 0, &config));
                soc_reg_field_set(unit, PT_IPTE_CONFIG2r, &config,
                                  PT_IPTE_PORT48_PORT50_BASE_QUEUEf, 0);
                soc_reg_field_set(unit, PT_IPTE_CONFIG2r, &config,
                                  PT_IPTE_PORT48_PORT50_BASE_PORTf, 0);
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG2r,
                                                  PT_INSTANCE(instance), 0, config));
            }
        }
    } else { /* !revB */

        switch(base_port) {
        case 0:
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG0r,
                                PT_INSTANCE(instance), 0, &config));
            soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                              PT_IPTE_PORT0_BASE_QUEUEf, 0);
            soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                              PT_IPTE_PORT0_QUEUE_RANGEf, 0);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG0r,
                                PT_INSTANCE(instance), 0, config));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG0r,
                                PT_INSTANCE(instance), 0, &config));
            soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                              PT_IPTE_PORT1_BASE_QUEUEf, 0);
            soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                              PT_IPTE_PORT1_QUEUE_RANGEf, 0);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG0r,
                                PT_INSTANCE(instance), 0, config));
            break;
        case 2:
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG1r,
                                PT_INSTANCE(instance), 0, &config));
            soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                              PT_IPTE_PORT2_BASE_QUEUEf, 0);
            soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                              PT_IPTE_PORT2_QUEUE_RANGEf, 0);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG1r,
                                PT_INSTANCE(instance), 0, config));
            break;
        case 3:
        case 4:
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG1r,
                                PT_INSTANCE(instance), 0, &config));
            soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                              PT_IPTE_PORT3_PORT47_BASE_QUEUEf, 0);
            soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                              PT_IPTE_PORT3_PORT47_BASE_PORTf, 0);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG1r,
                                PT_INSTANCE(instance), 0, config));
            break;

        case SOC_SBX_CALADAN3_SWS_XL_PORT_BASE:
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG2r,
                                PT_INSTANCE(instance), 0, &config));
            soc_reg_field_set(unit, PT_IPTE_CONFIG2r, &config,
                              PT_IPTE_PORT48_PORT50_BASE_QUEUEf, 0);
            soc_reg_field_set(unit, PT_IPTE_CONFIG2r, &config,
                              PT_IPTE_PORT48_PORT50_BASE_PORTf, 0);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG2r,
                                PT_INSTANCE(instance), 0, config));
            break;
        default:
            /* Do nothing */
                  ;
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_config_clear
 * Purpose
 *    Clear the port configuration, on B0/B1 this updates Q2C table
 *    On A0/A1 this updates the port queue base and range
 *    Calls the internal routine to accomplish this
 */
int
soc_sbx_caladan3_sws_pt_port_config_clear(int unit)
{
    int status = 0, port, portidx = 0;
    int instance = 0;

    
    SOC_PBMP_ITER(PBMP_ALL(unit), port) {
        status = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, port, &portidx);
        instance = 0;
        if (status == SOC_E_NOT_FOUND) {
            status = soc_sbx_caladan3_sws_pbmp_to_fab_port(unit, port, &portidx);
            instance = 1;
        }
        if (status == SOC_E_NOT_FOUND) {
            return SOC_E_NONE;
        }
        if (SOC_SUCCESS(status)) {
            soc_sbx_caladan3_sws_pt_port_config_clear_internal(unit, instance, portidx);
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_config_set
 * Purpose
 *    Set the port configuration for the given port, This routine is A0/A1 specific
 *    On A0/A1 this updates the port queue base and range
 */
int
soc_sbx_caladan3_sws_pt_port_config_set(int unit, int port)
{
    int ports_pt_instance, ilkn, clport, count, base_port;
    uint32 config = 0;
    int dqueue, dqueue_base, dq, qr;
    soc_sbx_caladan3_port_map_info_t *pi;
    soc_sbx_caladan3_queues_t *qi;
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    int portidx, status;
    int fab_line_side;
    int pt_split = 0;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    for (fab_line_side = LINE_SIDE; 
         fab_line_side < MAX_LINE_FABRIC_ID;
         fab_line_side++) {

        status = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, port, &portidx);
        if (SOC_SUCCESS(status)) {
            if (port >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
                return SOC_E_NONE;
            }
            clport = soc_sbx_block_find(unit, SOC_BLK_CLPORT, fab_line_side);
            ilkn = soc_sbx_block_find(unit, SOC_BLK_IL, fab_line_side);

            /* Fabric is side 1 by HW convention */
            if (fab_line_side) {
                pi = &port_map->fabric_port_info[port];
            } else {
                pi = &port_map->line_port_info[port];
            }
            if (!(pi->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                return SOC_E_PARAM;
            }
            qi = &pi->port_queues;
            assert(pt_ipte_init);
            ports_pt_instance = pi->physical_intf;
            base_port = pi->base_port;

            SOC_SBX_C3_BMP_ITER(qi->dqueue_bmp, dqueue_base, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {

                dqueue = dqueue_base % 64;

                /* See if their are two groups defined for this PT instance. 
                 * If so we need to account for this.
                 */
                if (pt_ipte_grp[ports_pt_instance].grp1_base && 
                    qi->dqueue_base == pt_ipte_grp[ports_pt_instance].grp1_base) {

                    /* The base queue for the GRP1 range starts consecutively 
                     * after the range of GRP0 queues.
                     */
                    dqueue = pt_ipte_grp[ports_pt_instance].grp0_range;
                    pt_split = 1;
                }

                if (!SOC_IS_CALADAN3_REVB(unit)) {

                    if ((pi->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE) ||
                       (pi->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25) ||
                           (pi->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42)) {

                        if (PT_REMAP_ENABLED(unit)) {
                            base_port = base_port >> 2;
                        }
                    }

                    if (qi->num_dqueue > 1) {
                        if (ilkn == pi->blk) {
                            count = qi->num_dqueue;
                            switch(base_port) {
                            case 0:
                                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG0r,
                                                    PT_INSTANCE(ports_pt_instance), 0, &config));
                                dq = soc_reg_field_get(unit, PT_IPTE_CONFIG0r, config,
                                                       PT_IPTE_PORT0_BASE_QUEUEf);
                                qr = soc_reg_field_get(unit, PT_IPTE_CONFIG0r, config,
                                                       PT_IPTE_PORT0_QUEUE_RANGEf);
                                CHECK_AND_UPDATE_SQ_INFO(dq, qr, dqueue, count);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                                                  PT_IPTE_PORT0_BASE_QUEUEf, dqueue);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                                                  PT_IPTE_PORT0_QUEUE_RANGEf,
                                                  count);
                                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG0r,
                                                    PT_INSTANCE(ports_pt_instance), 0, config));
                                break;
                            default:
                                    LOG_WARN(BSL_LS_SOC_COMMON,
                                             (BSL_META_U(unit,
                                                         "Unit %d, Baseport 0 on non IL Port(%d)\n"),
                                              unit, port));
                                    return SOC_E_NONE;
                            }
                            if (qi->spri_set) {
                                SOC_IF_ERROR_RETURN(
                                    soc_sbx_caladan3_sws_pt_strict_priority_config(unit,
                                        PT_INSTANCE(ports_pt_instance), 0, qi->spri0_qid,
                                        qi->spri1_qid));
                            } else {
                                SOC_IF_ERROR_RETURN(
                                    soc_sbx_caladan3_sws_pt_wdrr_scheduler_config(unit,
                                        PT_INSTANCE(ports_pt_instance), 0, TRUE));
                            }
                        } else if (clport == pi->blk) {
                            /* Currently we don't support splitting PTs on the channellized ports */
                            assert(!pt_split);
                            
                            count = qi->num_dqueue;
                            switch(base_port) {
                            case 0:
                                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG0r,
                                                    PT_INSTANCE(ports_pt_instance), 0, &config));
                                dq = soc_reg_field_get(unit, PT_IPTE_CONFIG0r, config,
                                                       PT_IPTE_PORT0_BASE_QUEUEf);
                                qr = soc_reg_field_get(unit, PT_IPTE_CONFIG0r, config,
                                                       PT_IPTE_PORT0_QUEUE_RANGEf);
                                CHECK_AND_UPDATE_SQ_INFO(dq, qr, dqueue, count);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                                                  PT_IPTE_PORT0_BASE_QUEUEf, dqueue);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                                                  PT_IPTE_PORT0_QUEUE_RANGEf,
                                                  count);
                                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG0r,
                                                    PT_INSTANCE(ports_pt_instance), 0, config));
                                break;
                            case 1:
                                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG0r,
                                                    PT_INSTANCE(ports_pt_instance), 0, &config));
                                dq = soc_reg_field_get(unit, PT_IPTE_CONFIG0r, config,
                                                       PT_IPTE_PORT1_BASE_QUEUEf);
                                qr = soc_reg_field_get(unit, PT_IPTE_CONFIG0r, config,
                                                       PT_IPTE_PORT1_QUEUE_RANGEf);
                                CHECK_AND_UPDATE_SQ_INFO(dq, qr, dqueue, count);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                                                  PT_IPTE_PORT1_BASE_QUEUEf, dqueue);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                                                  PT_IPTE_PORT1_QUEUE_RANGEf,
                                                  count);
                                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG0r,
                                                    PT_INSTANCE(ports_pt_instance), 0, config));
                                break;
                            case 2:
                                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG1r,
                                                    PT_INSTANCE(ports_pt_instance), 0, &config));
                                dq = soc_reg_field_get(unit, PT_IPTE_CONFIG1r, config,
                                                       PT_IPTE_PORT2_BASE_QUEUEf);
                                qr = soc_reg_field_get(unit, PT_IPTE_CONFIG1r, config,
                                                       PT_IPTE_PORT2_QUEUE_RANGEf);
                                CHECK_AND_UPDATE_SQ_INFO(dq, qr, dqueue, count);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                                                  PT_IPTE_PORT2_BASE_QUEUEf, dqueue);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                                                  PT_IPTE_PORT2_QUEUE_RANGEf,
                                                  count);
                                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG1r,
                                                    PT_INSTANCE(ports_pt_instance), 0, config));
                                break;
                            case 3:
                                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG1r,
                                            PT_INSTANCE(ports_pt_instance), 0, &config));
                                soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                                                  PT_IPTE_PORT3_PORT47_BASE_QUEUEf, dqueue);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                                                  PT_IPTE_PORT3_PORT47_BASE_PORTf,
                                                  base_port);
                                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG1r,
                                                    PT_INSTANCE(ports_pt_instance), 0, config));
                                break;
                            }

                            if (qi->spri_set) {
                                SOC_IF_ERROR_RETURN(
                                    soc_sbx_caladan3_sws_pt_strict_priority_config(unit,
                                        PT_INSTANCE(ports_pt_instance), base_port,
                                        qi->spri0_qid, qi->spri1_qid));
                            } else {
                                SOC_IF_ERROR_RETURN(
                                    soc_sbx_caladan3_sws_pt_wdrr_scheduler_config(unit,
                                        PT_INSTANCE(ports_pt_instance), base_port, TRUE));
                            }
                        } else {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "Unit %d Channellized port not on clport \n"), unit));
                            return SOC_E_PARAM;
                        }
                    } else {
                        count = 1;
                        if ((base_port == 0) &&
                             (pi->blk != clport) && (pi->blk != ilkn)) {
                            LOG_WARN(BSL_LS_SOC_COMMON,
                                     (BSL_META_U(unit,
                                                 "Unit %d, Baseport 0 on non CL/IL Port(%d)\n"), unit, port));
                            return SOC_E_NONE;
                        }
                        switch(base_port) {
                        case 0:
                            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG0r,
                                                PT_INSTANCE(ports_pt_instance), 0, &config));
                            dq = soc_reg_field_get(unit, PT_IPTE_CONFIG0r, config,
                                                   PT_IPTE_PORT0_BASE_QUEUEf);
                            qr = soc_reg_field_get(unit, PT_IPTE_CONFIG0r, config,
                                                   PT_IPTE_PORT0_QUEUE_RANGEf);
                            CHECK_AND_UPDATE_SQ_INFO(dq, qr, dqueue, count);
                            soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                                              PT_IPTE_PORT0_BASE_QUEUEf, dqueue);
                            soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                                              PT_IPTE_PORT0_QUEUE_RANGEf,
                                              count);
                            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG0r,
                                                PT_INSTANCE(ports_pt_instance), 0, config));
                            break;
                        case 1:
                            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG0r,
                                                PT_INSTANCE(ports_pt_instance), 0, &config));
                            dq = soc_reg_field_get(unit, PT_IPTE_CONFIG0r, config,
                                                   PT_IPTE_PORT1_BASE_QUEUEf);
                            qr = soc_reg_field_get(unit, PT_IPTE_CONFIG0r, config,
                                                   PT_IPTE_PORT1_QUEUE_RANGEf);
                            CHECK_AND_UPDATE_SQ_INFO(dq, qr, dqueue, count);
                            soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                                              PT_IPTE_PORT1_BASE_QUEUEf, dqueue);
                            soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &config,
                                              PT_IPTE_PORT1_QUEUE_RANGEf,
                                              count);
                            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG0r,
                                                PT_INSTANCE(ports_pt_instance), 0, config));
                            break;
                        case 2:
                            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG1r,
                                                PT_INSTANCE(ports_pt_instance), 0, &config));
                            dq = soc_reg_field_get(unit, PT_IPTE_CONFIG1r, config,
                                                   PT_IPTE_PORT2_BASE_QUEUEf);
                            qr = soc_reg_field_get(unit, PT_IPTE_CONFIG1r, config,
                                                   PT_IPTE_PORT2_QUEUE_RANGEf);
                            CHECK_AND_UPDATE_SQ_INFO(dq, qr, dqueue, count);
                            soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                                              PT_IPTE_PORT2_BASE_QUEUEf, dqueue);
                            soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                                              PT_IPTE_PORT2_QUEUE_RANGEf,
                                              count);
                            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG1r,
                                                PT_INSTANCE(ports_pt_instance), 0, config));
                            break;
                        case 3:
                            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG1r,
                                                PT_INSTANCE(ports_pt_instance), 0, &config));
                            soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                                              PT_IPTE_PORT3_PORT47_BASE_QUEUEf, dqueue);
                            soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                                              PT_IPTE_PORT3_PORT47_BASE_PORTf,
                                              base_port);
                            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG1r,
                                                PT_INSTANCE(ports_pt_instance), 0, config));
                            break;
             
                        case 4:
                            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG1r,
                                                PT_INSTANCE(ports_pt_instance), 0, &config));
                            if (0 == soc_reg_field_get(unit, PT_IPTE_CONFIG1r, config,
                                              PT_IPTE_PORT3_PORT47_BASE_QUEUEf)) {
                                soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                                              PT_IPTE_PORT3_PORT47_BASE_QUEUEf, dqueue);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG1r, &config,
                                                  PT_IPTE_PORT3_PORT47_BASE_PORTf,
                                                  base_port);
                                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG1r,
                                                    PT_INSTANCE(ports_pt_instance), 0, config));
                            }
                            break;
             
                        case SOC_SBX_CALADAN3_SWS_XL_PORT_BASE:
                            if (ports_pt_instance == 0) {
                                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG2r,
                                               PT_INSTANCE(ports_pt_instance), 0, &config));
                                soc_reg_field_set(unit, PT_IPTE_CONFIG2r, &config,
                                               PT_IPTE_PORT48_PORT50_BASE_QUEUEf, dqueue);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG2r, &config,
                                                  PT_IPTE_PORT48_PORT50_BASE_PORTf,
                                                  base_port);
                                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG2r,
                                                    PT_INSTANCE(ports_pt_instance), 0, config));
                                /* Clear default value on PT1 */
                                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG2r,
                                               PT_INSTANCE(1), 0, &config));
                                soc_reg_field_set(unit, PT_IPTE_CONFIG2r, &config,
                                               PT_IPTE_PORT48_PORT50_BASE_QUEUEf, 0);
                                soc_reg_field_set(unit, PT_IPTE_CONFIG2r, &config,
                                                  PT_IPTE_PORT48_PORT50_BASE_PORTf, 0);
                                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG2r,
                                                    PT_INSTANCE(1), 0, config));
                            }
                            break;
                        default:
                            /* Do nothing */
                                  ;
                        }
             
                        if (base_port <= 2) {
                            if (qi->spri_set) {
                                SOC_IF_ERROR_RETURN(
                                    soc_sbx_caladan3_sws_pt_strict_priority_config(unit,
                                        PT_INSTANCE(ports_pt_instance), base_port,
                                        qi->spri0_qid, qi->spri1_qid));
                            } else {
                                SOC_IF_ERROR_RETURN(
                                    soc_sbx_caladan3_sws_pt_wdrr_scheduler_config(unit,
                                        PT_INSTANCE(ports_pt_instance), base_port, TRUE));
                            }
                        }
                    }
                } /* Rev A check */
            } /* Dqueue Iter */
        }
    } /* for loop */
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_config_set_rev_b
 * Purpose
 *    Set the port configuration for the given port, This routine is B0/B1 specific
 *    On B0/B1 this updates the Q2C map for the given port
 */
int
soc_sbx_caladan3_sws_pt_port_config_set_rev_b(int unit, int port)
{
    int ilkn, clport,  base_port, cur_port;
    uint32 config = 0;
    int dqueue, cur_queue;
    soc_sbx_caladan3_port_map_info_t *pi;
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    soc_sbx_caladan3_queues_t *qi = NULL;
    ipte_q2c_map_entry_t entry;
    int fab_line_side, status, portidx;
    int ports_pt_instance;    
    int pt_split = 0;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    
    for (fab_line_side = 0; 
         fab_line_side < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE; 
         fab_line_side++) {

        status = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, port, &portidx);

        if (SOC_SUCCESS(status)) {
            if (port >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
                return SOC_E_NONE;
            }
            
            clport = soc_sbx_block_find(unit, SOC_BLK_CLPORT, fab_line_side);
            ilkn = soc_sbx_block_find(unit, SOC_BLK_IL, fab_line_side);

            if (fab_line_side) {
                pi = &port_map->fabric_port_info[port];
            } else {
                pi = &port_map->line_port_info[port];
            }
            if (!(pi->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                return SOC_E_PARAM;
            }

            qi = &pi->port_queues;
            dqueue = qi->dqueue_base % 64;
            base_port = pi->base_port;

            assert(pt_ipte_init);
            ports_pt_instance = pi->physical_intf;
            
            /* See if their are two groups defined for this PT instance. 
             * If so we need to account for this.
             */
            if (pt_ipte_grp[ports_pt_instance].grp1_base && 
                qi->dqueue_base >= pt_ipte_grp[ports_pt_instance].grp1_base &&
                qi->dqueue_base < 
                (pt_ipte_grp[ports_pt_instance].grp1_base + pt_ipte_grp[ports_pt_instance].grp1_range)) {
                
                /* The base queue for the GRP1 range starts consecutively 
                 * after the range of GRP0 queues.
                 */
                dqueue = pt_ipte_grp[ports_pt_instance].grp0_range + 
                    (qi->dqueue_base - pt_ipte_grp[ports_pt_instance].grp1_base);

                pt_split = 1;
            }

            if ((pi->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE) ||
                (pi->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25) ||
                (pi->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42)) {
                if (PT_REMAP_ENABLED(unit)) {
                    base_port = base_port >> 2;
                }
            }
                        
            /* B0+ */        
            if (qi->num_dqueue > 1) {
                if (ilkn == pi->blk) {
                    switch(base_port) {
                        case 0:
                            /* mark all queues to use the base_port */
                            SOC_SBX_C3_BMP_ITER(qi->dqueue_bmp, cur_queue, 
                                                SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {

                                SOC_IF_ERROR_RETURN(
                                    soc_mem_read(unit, IPTE_Q2C_MAPm,
                                                 soc_sbx_block_find(unit, SOC_BLK_PT, ports_pt_instance),
                                                 cur_queue, &entry));
                                
                                soc_mem_field32_set(unit, IPTE_Q2C_MAPm, &entry, PORTf, base_port);
                                soc_mem_field32_set(unit, IPTE_Q2C_MAPm, &entry, FC_INDEXf, cur_queue);

                                SOC_IF_ERROR_RETURN(
                                    soc_mem_write(unit, IPTE_Q2C_MAPm,
                                                  soc_sbx_block_find(unit, SOC_BLK_PT, ports_pt_instance),
                                                  cur_queue, &entry));
                            }
                            break;
                    default:
                        LOG_WARN(BSL_LS_SOC_COMMON,
                                 (BSL_META_U(unit,
                                             "Unit %d, Baseport 0 on non IL Port(%d)\n"),
                                  unit, port));
                        return SOC_E_NONE;
                    }
                    if (qi->spri_set) {
                        SOC_IF_ERROR_RETURN(
                            soc_sbx_caladan3_sws_pt_strict_priority_config(unit,
                                                                           PT_INSTANCE(ports_pt_instance),
                                                                           0, qi->spri0_qid,
                                                                           qi->spri1_qid));
                    } else {
                        SOC_IF_ERROR_RETURN(
                            soc_sbx_caladan3_sws_pt_wdrr_scheduler_config(unit,
                                                                          PT_INSTANCE(ports_pt_instance),
                                                                          0, TRUE));
                    }
                } else if (clport == pi->blk) {
                    /* Currently we don't support splitting PTs on the channellized ports */
                    assert(!pt_split);

                    /* channelized ports */
                    /* mark all queues to use the base_port  */
                    SOC_SBX_C3_BMP_ITER(qi->dqueue_bmp, cur_queue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {

                        SOC_IF_ERROR_RETURN(
                            soc_mem_read(unit, IPTE_Q2C_MAPm,
                                         soc_sbx_block_find(unit, SOC_BLK_PT, ports_pt_instance),
                                         cur_queue, &entry));
                        
                        soc_mem_field32_set(unit, IPTE_Q2C_MAPm, &entry, PORTf, base_port);
                        soc_mem_field32_set(unit, IPTE_Q2C_MAPm, &entry, FC_INDEXf, cur_queue);

                        SOC_IF_ERROR_RETURN(
                            soc_mem_write(unit, IPTE_Q2C_MAPm,
                                          soc_sbx_block_find(unit, SOC_BLK_PT, ports_pt_instance),
                                          cur_queue, &entry));
                    }
                    if (qi->spri_set) {
                        SOC_IF_ERROR_RETURN(
                            soc_sbx_caladan3_sws_pt_strict_priority_config(unit,
                                                                           PT_INSTANCE(ports_pt_instance),
                                                                           base_port,
                                                                           qi->spri0_qid,
                                                                           qi->spri1_qid));

                    } else {
                            SOC_IF_ERROR_RETURN(
                                soc_sbx_caladan3_sws_pt_wdrr_scheduler_config(unit,
                                                                              PT_INSTANCE(ports_pt_instance), 
                                                                              base_port, TRUE));
                    }
                } else {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d Channellized port not on clport \n"), unit));
                    return SOC_E_PARAM;
                }
            } else {
                if ((base_port == 0) &&
                    (pi->blk != clport) && (pi->blk != ilkn)) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Unit %d, Baseport 0 on non CL/IL Port(%d)\n"), unit, port));
                    return SOC_E_NONE;
                }
                
                if (base_port < SOC_SBX_CALADAN3_SWS_XT_PORT_BASE) {
                    cur_queue = dqueue;
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit, IPTE_Q2C_MAPm,
                                                     soc_sbx_block_find(unit, SOC_BLK_PT, ports_pt_instance),
                                                     cur_queue, &entry));
                        
                    soc_mem_field32_set(unit, IPTE_Q2C_MAPm, &entry, PORTf, base_port);
                    soc_mem_field32_set(unit, IPTE_Q2C_MAPm, &entry, FC_INDEXf, cur_queue);

                    SOC_IF_ERROR_RETURN(soc_mem_write(unit, IPTE_Q2C_MAPm,
                                                      soc_sbx_block_find(unit, SOC_BLK_PT, ports_pt_instance),
                                                      cur_queue, &entry));
                } else if (base_port < SOC_SBX_CALADAN3_SWS_XL_PORT_BASE) {
                    /* non channelized ports */
                    /* we need to make sure port 12-47 is consistent if there is
                     * existing configuration
                     */
                    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG4r,
                                                      PT_INSTANCE(ports_pt_instance), 0, &config));

                    cur_port = soc_reg_field_get(unit, PT_IPTE_CONFIG4r, config,
                                                 PT_IPTE_PORT12_PORT47_BASE_PORTf);
                    cur_queue = soc_reg_field_get(unit, PT_IPTE_CONFIG4r, config,
                                                  PT_IPTE_PORT12_PORT47_BASE_QUEUEf);
                   if (cur_port != 0) {
                        /* has existing config */
                        if (cur_port >= cur_queue) {
                            if ((base_port - dqueue) != (cur_port - cur_queue)) {
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "Unit %d port %d queue %d not consistent " \
                                                      "with existing port %d queue %d on xtport \n"),
                                           unit, base_port, dqueue, cur_port, cur_queue));
                                return SOC_E_PARAM;
                            }
                        } else {
                            if ((dqueue - base_port) != (cur_queue - cur_port)) {
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "Unit %d port %d queue %d not consistent " \
                                                      "with existing port %d queue %d on xtport \n"),
                                           unit, base_port, dqueue, cur_port, cur_queue));
                                return SOC_E_PARAM;
                            }
                        }
                    }
        
                    if ((cur_port == 0) || (base_port < cur_port)) {
                        /* if not configed yet or configed with a higher base_port
                         * replace with current base_port, base_queue
                         */ 
                        soc_reg_field_set(unit, PT_IPTE_CONFIG4r, &config,
                                          PT_IPTE_PORT12_PORT47_BASE_QUEUEf, dqueue);
                        soc_reg_field_set(unit, PT_IPTE_CONFIG4r, &config,
                                          PT_IPTE_PORT12_PORT47_BASE_PORTf, base_port);
                        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG4r,
                                                          PT_INSTANCE(ports_pt_instance), 0, config));                
                    }
                } else if (base_port == SOC_SBX_CALADAN3_SWS_XL_PORT_BASE) {
                    /* service ports */
                    if (ports_pt_instance == 0) {
                        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG2r,
                                                          PT_INSTANCE(ports_pt_instance), 0, &config));
                        soc_reg_field_set(unit, PT_IPTE_CONFIG2r, &config,
                                          PT_IPTE_PORT48_PORT50_BASE_QUEUEf, dqueue);
                        soc_reg_field_set(unit, PT_IPTE_CONFIG2r, &config,
                                          PT_IPTE_PORT48_PORT50_BASE_PORTf,
                                          base_port);
                        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG2r,
                                                          PT_INSTANCE(ports_pt_instance), 0, config));
                    }
                }
                
                /* B0 support 12 channelized ports */
                if (base_port < SOC_SBX_CALADAN3_SWS_PT_MAX_CHANNELIZED_PORT_REVB) {
                    if (qi->spri_set) {
                        SOC_IF_ERROR_RETURN(
                            soc_sbx_caladan3_sws_pt_strict_priority_config(unit,
                                                                           PT_INSTANCE(ports_pt_instance), base_port,
                                                                           qi->spri0_qid, qi->spri1_qid));
                    } else {
                        SOC_IF_ERROR_RETURN(
                            soc_sbx_caladan3_sws_pt_wdrr_scheduler_config(unit,
                                                                          PT_INSTANCE(ports_pt_instance), base_port, TRUE));
                    }
                }
            }
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_config_validate
 * Purpose
 *    Validate the port configuration
 *    This verifies that all pbmp ports are sitting on unique physical ports
 *    Also validates that  channelized ports are correctly setup
 */
int
soc_sbx_caladan3_sws_pt_port_config_validate(int unit)
{
    int port, base_port, idx;
    int port_set[2][SOC_SBX_CALADAN3_SWS_PT_MAX_PORTS] = {{0}};
    soc_sbx_caladan3_port_map_info_t *port_info;
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    soc_sbx_caladan3_queues_t *qi = NULL;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    SOC_PBMP_ITER(PBMP_PORT_ALL(unit), port) {
        if (SOC_E_NONE == 
                soc_sbx_caladan3_sws_pbmp_to_line_port(unit, port, &idx)) {
            port_info = &port_map->line_port_info[idx];
        } else if (SOC_E_NONE == 
                soc_sbx_caladan3_sws_pbmp_to_fab_port(unit, port, &idx)) {
            port_info = &port_map->fabric_port_info[idx];
        } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Cannot find info on port %d\n"), unit, port));
                return SOC_E_PARAM;
        }
        qi = &port_info->port_queues;
        if (qi->num_dqueue > 1) {
            base_port = port_info->base_port;
            if ((port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE) ||
                   (port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25) ||
                       (port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42)) {
                if (PT_REMAP_ENABLED(unit)) {
                    base_port = base_port >> 2;
                }
            }
            if (SOC_IS_CALADAN3_REVB(unit)) {
                if ((base_port < 0) || (base_port > SOC_SBX_CALADAN3_SWS_PT_MAX_CHANNELIZED_PORT_REVB)) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Unit %d *** Warning: Cannot Channelize port %d\n"), unit, port));
                }
            } else {
                if ((base_port < 0) || (base_port > SOC_SBX_CALADAN3_SWS_PT_MAX_CHANNELIZED_PORT)) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Unit %d *** Warning: Cannot Channelize port %d\n"), unit, port));
                }
            }
        }
        if ((port_set[port_info->physical_intf][port_info->base_port]) &&
            !(port_info->blktype == SOC_BLK_XLPORT) &&
            !(port_info->flags & SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT)) {

            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Multiple ports(including port%d) with base port %d\n"), 
                       unit, port, port_info->base_port));

            return SOC_E_PARAM;

        } else {
            port_set[port_info->physical_intf][port_info->base_port] = 1;
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_hpte_map_setup
 * Purpose
 *    Set up a Squeue to Dqueue map. Updates the HPTE_DQUEUE_LOOKUP Table
 */
int
soc_sbx_caladan3_sws_hpte_map_setup(int unit, uint32 squeue, uint32 dqueue)
{
    hpte_dqueue_lookup_entry_t entry;

    sal_memset(&entry, 0, sizeof(entry));
    dqueue &= 0xff;
    squeue &= 0x7f;
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Unit %d, Setting up HPTE_DQUEUE_TABLE SQ(%x %d)--> DQ(%x %d)\n"),
                 unit, squeue, squeue, dqueue, dqueue));

    soc_mem_field_set(unit, HPTE_DQUEUE_LOOKUPm, &entry.entry_data[0], 
                      DQUEUEf, &dqueue); 
    SOC_IF_ERROR_RETURN(soc_mem_write(unit, HPTE_DQUEUE_LOOKUPm,
                                MEM_BLOCK_ALL,
                                squeue, &entry));
/*
    SOC_IF_ERROR_RETURN(soc_mem_write(unit, HPTE_DQUEUE_LOOKUPm,
                                soc_sbx_block_find(unit, SOC_BLK_PT, 1),
                                squeue, &entry));
 */
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_ipte_wdrr_setup
 * Purpose
 *    Updates the IPTE_WDRR_CREDIT_CFG Table
 */
int
soc_sbx_caladan3_sws_ipte_wdrr_setup(int unit, int dqueue, int instance, uint32 credits)
{
    ipte_wdrr_credit_cfg_entry_t entry;
    char buffer[128];

    sal_memset(&entry, 0, sizeof(entry));
    sal_snprintf(buffer, 128, "%s_%d", spn_WDRR_WEIGHT_QUEUE, dqueue);
    credits = soc_sbx_caladan3_sws_config_param_data_get(unit, buffer, -1, credits);
    if (dqueue >= SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE) {
        dqueue -= SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE;
    } else {
        dqueue -= SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE;
    }
    soc_mem_field_set(unit, IPTE_WDRR_CREDIT_CFGm, &entry.entry_data[0], 
                      CREDITf, &credits); 
    SOC_IF_ERROR_RETURN(soc_mem_write(unit, IPTE_WDRR_CREDIT_CFGm,
                                      soc_sbx_block_find(unit, SOC_BLK_PT, instance),
                                      dqueue, &entry));
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_hpte_wdrr_setup
 * Purpose
 *    Updates the HPTE_WDRR_CREDIT_CFG Table
 */
int
soc_sbx_caladan3_sws_hpte_wdrr_setup(int unit, int squeue, uint32 credits)
{
    hpte_wdrr_credit_cfg_entry_t entry;
    char buffer[128];

    sal_memset(&entry, 0, sizeof(entry));
    sal_snprintf(buffer, 128, "%s_%d", spn_WDRR_WEIGHT_QUEUE, squeue);
    credits = soc_sbx_caladan3_sws_config_param_data_get(unit, buffer, -1, credits);
    soc_mem_field_set(unit, HPTE_WDRR_CREDIT_CFGm, &entry.entry_data[0], 
                      CREDITf, &credits); 
    SOC_IF_ERROR_RETURN(soc_mem_write(unit, HPTE_WDRR_CREDIT_CFGm,
                                      MEM_BLOCK_ALL,
                                      squeue, &entry));
/*
    SOC_IF_ERROR_RETURN(soc_mem_write(unit, HPTE_WDRR_CREDIT_CFGm,
                                      soc_sbx_block_find(unit, SOC_BLK_PT, 1),
                                      squeue, &entry));
*/
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_client_enable
 * Purpose
 *    For a given port enable the corresponding client
 */
int
soc_sbx_caladan3_sws_pt_client_enable(int unit, int port, int enable)
{
    soc_sbx_caladan3_port_map_info_t *port_info;
    uint32 regval=0;
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    int status, portidx, instance;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    for (instance=0; 
             instance < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE; 
                 instance++) {
        status = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, port, &portidx);
        if (SOC_SUCCESS(status)) {
            if (portidx >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
                return SOC_E_NONE;
            }
         
            if (instance) {
                port_info = &port_map->fabric_port_info[portidx];
            } else {
                port_info = &port_map->line_port_info[portidx];
            }
            if (!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                return SOC_E_PARAM;
            }
         
            switch(port_info->intftype) {
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CLIENT_CFG0r,
                                                  PT_INSTANCE(instance), 0, &regval));
                if ((enable != soc_reg_field_get(unit,  PT_IPTE_CLIENT_CFG0r, 
                                                 regval, CLIENT_ENf)) ||
                    (1 != soc_reg_field_get(unit,  PT_IPTE_CLIENT_CFG0r, 
                                            regval, CLIENT_IDf))) 
                {
                    soc_reg_field_set(unit,  PT_IPTE_CLIENT_CFG0r, &regval, 
                                      CLIENT_IDf, 1);
                    soc_reg_field_set(unit,  PT_IPTE_CLIENT_CFG0r, &regval, 
                                      CLIENT_ENf, enable);
                    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CLIENT_CFG0r,
                                                      PT_INSTANCE(instance), 0, regval));
                }
                break;
         
            case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
            case SOC_SBX_CALADAN3_PORT_INTF_IL100:
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CLIENT_CFG0r,
                                                  PT_INSTANCE(instance), 0, &regval));
                if ((enable != soc_reg_field_get(unit,  PT_IPTE_CLIENT_CFG0r, 
                                                 regval, CLIENT_ENf)) ||
                    (0 != soc_reg_field_get(unit,  PT_IPTE_CLIENT_CFG0r, 
                                            regval, CLIENT_IDf))) 
                {
                    soc_reg_field_set(unit,  PT_IPTE_CLIENT_CFG0r, &regval, 
                                      CLIENT_IDf, 0);
                    soc_reg_field_set(unit,  PT_IPTE_CLIENT_CFG0r, &regval, 
                                      CLIENT_ENf, enable);
                    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CLIENT_CFG0r,
                                            PT_INSTANCE(instance), 0, regval));
                }
                break;
         
            case SOC_SBX_CALADAN3_PORT_INTF_XTPORT:
                if (port_info->instance == 0) {
                    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CLIENT_CFG1r,
                                                      PT_INSTANCE(instance), 
                                                      0, &regval));
                    if (enable != soc_reg_field_get(unit,  PT_IPTE_CLIENT_CFG1r, 
                                                    regval, CLIENT_ENf)) {
                        soc_reg_field_set(unit, PT_IPTE_CLIENT_CFG1r, &regval, 
                                          CLIENT_ENf, enable);
                        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CLIENT_CFG1r,
                                                          PT_INSTANCE(instance), 
                                                          0, regval));
                    }
                } else if (port_info->instance == 1) {
                    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CLIENT_CFG2r,
                                                      PT_INSTANCE(instance), 
                                                      0, &regval));
                    if (enable != soc_reg_field_get(unit,  PT_IPTE_CLIENT_CFG2r, 
                                                    regval, CLIENT_ENf)) {
                        soc_reg_field_set(unit, PT_IPTE_CLIENT_CFG2r, &regval, 
                                          CLIENT_ENf, enable);
                        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CLIENT_CFG2r,
                                                          PT_INSTANCE(instance), 
                                                          0, regval));
                    }
                } else if (port_info->instance == 2) {
                    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CLIENT_CFG3r,
                                                      PT_INSTANCE(instance), 
                                                      0, &regval));
                    if (enable != soc_reg_field_get(unit,  PT_IPTE_CLIENT_CFG3r,
                                                    regval, CLIENT_ENf)) {
                        soc_reg_field_set(unit, PT_IPTE_CLIENT_CFG3r, &regval, 
                                          CLIENT_ENf, 1);
                        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CLIENT_CFG3r,
                                                          PT_INSTANCE(instance), 
                                                          0, regval));
                    }
                }
                break;
         
            case SOC_SBX_CALADAN3_PORT_INTF_XLPORT:
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CLIENT_CFG4r,
                                                  PT_INSTANCE(instance), 
                                                  0, &regval));
                if (enable != soc_reg_field_get(unit,  PT_IPTE_CLIENT_CFG4r,
                                                regval, CLIENT_ENf)) {
                    soc_reg_field_set(unit, PT_IPTE_CLIENT_CFG4r, &regval, 
                                      CLIENT_ENf, enable);
                    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CLIENT_CFG4r,
                                                      PT_INSTANCE(instance), 
                                                      0, regval));
                }
                break; 
         
            case SOC_SBX_CALADAN3_PORT_INTF_CMIC:
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CLIENT_CFG5r,
                                                  PT_INSTANCE(instance), 
                                                  0, &regval));
                if (enable != soc_reg_field_get(unit,  PT_IPTE_CLIENT_CFG5r,
                                                regval, CLIENT_ENf)) {
                    soc_reg_field_set(unit, PT_IPTE_CLIENT_CFG5r, &regval, 
                                      CLIENT_ENf, enable);
                    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CLIENT_CFG5r,
                                                      PT_INSTANCE(instance), 
                                                      0, regval));
                }
                break; 
         
            default:
                break;
            }
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *   _soc_sbx_caladan3_pt_port_strict_prio_config 
 * Purpose
 *    Setup up IPTE Strict priority
 *    Internal routine
 */
static int
_soc_sbx_caladan3_pt_port_strict_prio_config(int unit, int pt, soc_reg_t reg,
                                             int spr0_qid, int spr1_qid)
{
    uint32 regval = 0;
    int old_spr0 = 0, old_spr1 = 0;
    int set=0;

    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, pt, 0, &regval));

    old_spr0 = soc_reg_field_get(unit, reg, regval, PT_IPTE_SPR0_IDf);
    if ((spr0_qid > 0) && (old_spr0 != spr0_qid)) {
        soc_reg_field_set(unit, reg, &regval, PT_IPTE_SPR0_IDf, spr0_qid);
        soc_reg_field_set(unit, reg, &regval, PT_IPTE_SPR0_ENf, 1);
        set=1;
    }
    old_spr1 = soc_reg_field_get(unit, reg, regval, PT_IPTE_SPR1_IDf);
    if ((spr1_qid > 0) && (old_spr1 != spr1_qid)) {
        soc_reg_field_set(unit, reg, &regval, PT_IPTE_SPR1_IDf, spr1_qid);
        soc_reg_field_set(unit, reg, &regval, PT_IPTE_SPR1_ENf, 1);
        set=1;
    }
    if (!soc_reg_field_get(unit, reg, regval, PT_IPTE_WDRR_ENf)) {
        soc_reg_field_set(unit, reg, &regval, PT_IPTE_WDRR_ENf, 1);
        set=1;
    }
    if (set) {
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, reg, pt, 0, regval));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *   soc_sbx_caladan3_pt_port_strict_priority_config 
 * Purpose
 *    Setup up IPTE Strict priority
 *    Calls the corresponding internal routine to accomplish work
 */
int
soc_sbx_caladan3_sws_pt_strict_priority_config(int unit, int pt, int base_port,
                                               int spr0_qid, int spr1_qid)
{
    int rv = SOC_E_NONE;
    spr1_qid &= 0x3f;
    spr0_qid &= 0x3f;

    if (SOC_IS_CALADAN3_REVB(unit) && (base_port > 2)) {
        return rv;
    }

    switch (base_port) {
    case 0:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT0_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 1:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT1_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 2:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT2_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 3:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT3_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 4:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT4_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 5:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT5_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 6:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT6_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 7:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT7_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 8:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT8_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 9:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT9_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 10:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT10_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    case 11:
            rv = _soc_sbx_caladan3_pt_port_strict_prio_config(unit, pt,
                     PT_IPTE_PORT11_WDRR_CFGr, spr0_qid, spr1_qid);
            break;
    default:
          ;
    }

    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_hpte_hipri_queue_set_revb
 * Purpose
 *    Setup up HPTE Hi priority queue config
 *    This is B0 specific routine
 */
int
soc_sbx_caladan3_sws_hpte_hipri_queue_set_revb(int unit, int pt, int num,
                                               int *qid, int *cap)
{
    uint32 regval = 0;
    int i;

    if (PT_INSTANCE(0) == pt) {
        if (num > 3) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, hpte_hipri_queue_set ignoring excess ingress queues\n"), unit));
            num=3;
        }
        for (i=0; i<num; i++) {
            if ((cap[i] < 0) || (cap[i] > 511)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d, hpte_hipri_queue_set invalid cap(%d) for qid(%d)\n"),
                           unit, cap[i], qid[i]));
                return SOC_E_PARAM;
            }
            if ((qid[i] < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) &&
                 (qid[i] >= SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE)) {
                return SOC_E_PARAM;
            }
        }
        for (i=0; i<num; i++) {
            switch (i) {
            case 0:
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_INGRESS_CONFIGr,
                                                 pt, 0, &regval));
                soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                       PT_HPTE_INGRESS_PORT0_HIPRI_QUEUE_IDf, qid[i]);
                soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                       PT_HPTE_INGRESS_PORT0_HIPRI_QUEUE_ENf, 1);
                soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                       PT_HPTE_INGRESS_PORT0_HIPRI_QUEUE_CAPf, cap[i]);
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_INGRESS_CONFIGr,
                                                 pt, 0, regval));
                break;
            case 1:
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_INGRESS_CONFIG1r,
                                                 pt, 0, &regval));
                soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                   PT_HPTE_INGRESS_PORT1_HIPRI_QUEUE_IDf, qid[i]);
                soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                   PT_HPTE_INGRESS_PORT1_HIPRI_QUEUE_ENf, 1);
                soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                   PT_HPTE_INGRESS_PORT1_HIPRI_QUEUE_CAPf, cap[i]);
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_INGRESS_CONFIG1r,
                                                 pt, 0, regval));
                break;
            case 2:
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_INGRESS_CONFIG1r,
                                                  pt, 0, &regval));
                soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                    PT_HPTE_INGRESS_PORT2_HIPRI_QUEUE_IDf, qid[i]);
                soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                    PT_HPTE_INGRESS_PORT2_HIPRI_QUEUE_ENf, 1);
                soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                    PT_HPTE_INGRESS_PORT2_HIPRI_QUEUE_CAPf, cap[i]);
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_INGRESS_CONFIG1r,
                                                  pt, 0, regval));
                break;
            }
        }
    } else {
        soc_field_t fields_cap[] = {
            PT_HPTE_EGRESS_SPRI0_QUEUE_CAPf,
            PT_HPTE_EGRESS_SPRI1_QUEUE_CAPf,
            PT_HPTE_EGRESS_SPRI2_QUEUE_CAPf,
            PT_HPTE_EGRESS_SPRI3_QUEUE_CAPf,
            PT_HPTE_EGRESS_SPRI4_QUEUE_CAPf,
            PT_HPTE_EGRESS_SPRI5_QUEUE_CAPf,
        };
        soc_field_t fields_qid[] = {
            PT_HPTE_EGRESS_SPRI0_QUEUE_IDf,
            PT_HPTE_EGRESS_SPRI1_QUEUE_IDf,
            PT_HPTE_EGRESS_SPRI2_QUEUE_IDf,
            PT_HPTE_EGRESS_SPRI3_QUEUE_IDf,
            PT_HPTE_EGRESS_SPRI4_QUEUE_IDf,
            PT_HPTE_EGRESS_SPRI5_QUEUE_IDf,
        };
        soc_field_t fields_en[] = {
            PT_HPTE_EGRESS_SPRI0_QUEUE_ENf,
            PT_HPTE_EGRESS_SPRI1_QUEUE_ENf,
            PT_HPTE_EGRESS_SPRI2_QUEUE_ENf,
            PT_HPTE_EGRESS_SPRI3_QUEUE_ENf,
            PT_HPTE_EGRESS_SPRI4_QUEUE_ENf,
            PT_HPTE_EGRESS_SPRI5_QUEUE_ENf,
        };
        soc_reg_t regs[] = {
            PT_HPTE_EGRESS_CONFIG2r,
            PT_HPTE_EGRESS_CONFIG3r,
            PT_HPTE_EGRESS_CONFIG4r,
            PT_HPTE_EGRESS_CONFIG5r,
        };

        if (num > 6) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, hpte_hipri_queue_set ignoring excess egress queues\n"),
                       unit));
            num=6;
        }
        for (i=0; i<num; i++) {
            if ((cap[i] < 0) || (cap[i] > 511)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d, hpte_hipri_queue_set invalid cap(%d) for qid(%d)\n"),
                           unit, cap[i], qid[i]));
            }
            if ((qid[i] < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) &&
                 (qid[i] >= SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE)) {
                return SOC_E_PARAM;
            }
        }
        for (i=0; i<num; i++) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, regs[i>>1], pt, 0, &regval));
            soc_reg_field_set(unit, regs[i>>1], &regval, fields_qid[i], qid[i]);
            soc_reg_field_set(unit, regs[i>>1], &regval, fields_en[i], 1);
            soc_reg_field_set(unit, regs[i>>1], &regval, fields_cap[i], cap[i]);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, regs[i>>1], pt, 0, regval));
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_hpte_hipri_queue_set
 * Purpose
 *    Setup up HPTE Hi priority queue config
 *    This is A0/A1 specific routine
 */
int
soc_sbx_caladan3_sws_hpte_hipri_queue_set(int unit, int pt, 
                                          int qid0, int cap0, 
                                          int qid1, int cap1, 
                                          int qid2, int cap2)
{
    uint32 regval = 0;

    if ((cap0 < 0) || (cap0 > 511) ||
            (cap1 < 0) || (cap1 > 511) ||
                 (cap2 < 0) || (cap2 > 511)) {
        return SOC_E_PARAM;
    }
    
    if (PT_INSTANCE(0) == pt) {
        if (qid0 >= 0) {
            if ((qid0 < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) && 
                 (qid0 >= SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE)) {
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_INGRESS_CONFIGr,
                                              pt, 0, &regval));
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                    PT_HPTE_INGRESS_PORT0_HIPRI_QUEUE_IDf, qid0);
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                    PT_HPTE_INGRESS_PORT0_HIPRI_QUEUE_ENf, 1);
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                    PT_HPTE_INGRESS_PORT0_HIPRI_QUEUE_CAPf, cap0);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_INGRESS_CONFIGr,
                                              pt, 0, regval));
        }
        if (qid1 >= 0) {
            if ((qid1 < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) && 
                 (qid1 >= SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE)) {
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_INGRESS_CONFIG1r,
                                              pt, 0, &regval));
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                PT_HPTE_INGRESS_PORT1_HIPRI_QUEUE_IDf, qid1);
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                PT_HPTE_INGRESS_PORT1_HIPRI_QUEUE_ENf, 1);
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                PT_HPTE_INGRESS_PORT1_HIPRI_QUEUE_CAPf, cap1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_INGRESS_CONFIG1r,
                                              pt, 0, regval));
        }
        if (qid2 >= 0) {
            if ((qid2 < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) && 
                 (qid2 >= SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE)) {
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_INGRESS_CONFIG1r,
                                              pt, 0, &regval));
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                PT_HPTE_INGRESS_PORT2_HIPRI_QUEUE_IDf, qid2);
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                PT_HPTE_INGRESS_PORT2_HIPRI_QUEUE_ENf, 1);
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIG1r, &regval,
                PT_HPTE_INGRESS_PORT2_HIPRI_QUEUE_CAPf, cap2);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_INGRESS_CONFIG1r,
                                              pt, 0, regval));
        }
    } else {
        if (qid0 >= 0) {
            if ((qid0 < SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE) && 
                 (qid0 >= SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE)) {
		/* coverity[dead_error_line] */
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_EGRESS_CONFIGr,
                                              pt, 0, &regval));
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIGr, &regval,
                    PT_HPTE_EGRESS_PORT0_HIPRI_QUEUE_IDf, qid0);
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIGr, &regval,
                    PT_HPTE_EGRESS_PORT0_HIPRI_QUEUE_ENf, 1);
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIGr, &regval,
                    PT_HPTE_EGRESS_PORT0_HIPRI_QUEUE_CAPf, cap0);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_EGRESS_CONFIGr,
                                              pt, 0, regval));
        }
        if (qid1 >= 0) {
            if ((qid1 < SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE) && 
                 (qid1 >= SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE)) {
		/* coverity[dead_error_line] */
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_EGRESS_CONFIG1r,
                                              pt, 0, &regval));
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIG1r, &regval,
                PT_HPTE_EGRESS_PORT1_HIPRI_QUEUE_IDf, qid1);
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIG1r, &regval,
                PT_HPTE_EGRESS_PORT1_HIPRI_QUEUE_ENf, 1);
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIG1r, &regval,
                PT_HPTE_EGRESS_PORT1_HIPRI_QUEUE_CAPf, cap1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_EGRESS_CONFIG1r,
                                              pt, 0, regval));
        }
        if (qid2 >= 0) {
            if ((qid2 < SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE) && 
                 (qid2 >= SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE)) {
		/* coverity[dead_error_line] */
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_EGRESS_CONFIG1r,
                                              pt, 0, &regval));
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIG1r, &regval,
                PT_HPTE_EGRESS_PORT2_HIPRI_QUEUE_IDf, qid2);
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIG1r, &regval,
                PT_HPTE_EGRESS_PORT2_HIPRI_QUEUE_ENf, 1);
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIG1r, &regval,
                PT_HPTE_EGRESS_PORT2_HIPRI_QUEUE_CAPf, cap2);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_EGRESS_CONFIG1r,
                                              pt, 0, regval));
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_hpte_hipri_queues_config
 * Purpose
 *    Setup up HPTE Hi priority queue config
 *    This is generic routine that uses A0/B0 specific versions to setup the 
 *    Hi Priority queue config
 */
int
soc_sbx_caladan3_hpte_hipri_queues_config(int unit) 
{
    int rv = SOC_E_NONE;
    int max = 0, port = 0, i = 0;
    int ing_queues[3] = {0}; 
    int egr_queues[6] = {0};
    int ing_cap[3] = {0}; 
    int egr_cap[6] = {0};
    char buffer[128];
    soc_sbx_caladan3_port_map_t *port_map;
    soc_info_t *si __attribute__((unused));


    si = &SOC_INFO(unit);
    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    for (i=0; i < 3; i++) {
        sal_sprintf(buffer, "%s%d", "ingress_hipri_qid", i);
        ing_queues[i] = soc_sbx_caladan3_sws_config_param_data_get(unit, buffer, -1, -1);
        if (ing_queues[i] < 0) {
            sal_sprintf(buffer, "%s%d", "ingress_hipri_port", i);
            port = soc_sbx_caladan3_sws_config_param_data_get(unit, buffer, -1, -1);
            if (port >= 0) {
                rv = soc_sbx_caladan3_get_squeue_from_port(unit, port, 0, 0,
                                                           &ing_queues[i]);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d Hi Priority port (%d) not found\n"),
                               unit, port));
                    ing_queues[i] = 0;
                }
            } else {
                ing_queues[i] = port_map->default_hpte_hiprio_ingress_squeues[i];
            }
        }
        sal_sprintf(buffer, "%s%d", "ingress_hipri_cap", i);
        ing_cap[i] = soc_sbx_caladan3_sws_config_param_data_get(unit, buffer, -1, 0);
        if ((ing_cap[i] <0) || (ing_cap[i] > 150000000)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Ingress Hi Priority Cap illegal (%d)\n"),
                       unit, ing_cap[i]));
            ing_cap[i] = 0;
        }
        if (ing_cap[i] > 511) {
            ing_cap[i] = ((SOC_SBX_CALADAN3_SWS_CLOCK_RATE)/(ing_cap[i]*4/1000000))-1;
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Unit %d Ingress Hi Priority Cap%d set (%d)\n"),
                         unit, i, ing_cap[i]));
        }
    }
    if (SOC_IS_CALADAN3_REVB(unit)) {
        rv = soc_sbx_caladan3_sws_hpte_hipri_queue_set_revb(unit, 0, 3,
                    ing_queues, ing_cap);
    } else {
         rv = soc_sbx_caladan3_sws_hpte_hipri_queue_set(unit, 0,
                    ing_queues[0], ing_cap[0], ing_queues[1],
                    ing_cap[1], ing_queues[2], ing_cap[2]);
    }
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Ingress hipri_queue_set failed (%d)"), unit, rv));
        return rv;
    }

    max = (SOC_IS_CALADAN3_REVB(unit)) ? 6 : 3;
    for (i=0; i < max; i++) {
        sal_sprintf(buffer, "%s%d", "egress_hipri_qid", i);
        egr_queues[i] =
           soc_sbx_caladan3_sws_config_param_data_get(unit, buffer, -1, -1);
        if (egr_queues[i] < 0) {
            /* Not a mistake, we look for ingress here */
            sal_sprintf(buffer, "%s%d", "ingress_hipri_port", i);
            port = soc_sbx_caladan3_sws_config_param_data_get(unit, buffer, -1, -1);
            if (port >= 0) {
                rv = soc_sbx_caladan3_get_squeue_from_port(unit, port, 1, 0,
                                                           &egr_queues[i]);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d Hi Priority port (%d) not found\n"),
                               unit, port));
                    egr_queues[i] = 0;
                }
            } else {
                if (i >=3) {
                  LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d invalid egress sq block (%d)\n"),
                               unit, i));
                  continue; 
                }
                egr_queues[i] = port_map->default_hpte_hiprio_egress_squeues[i];
            }
        }
        sal_sprintf(buffer, "%s%d", "ingress_hipri_cap", i);
        egr_cap[i] =
            soc_sbx_caladan3_sws_config_param_data_get(unit, buffer, -1, 0);
        if ((egr_cap[i] <0) || (egr_cap[i] > 150000000)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d Egress Hi Priority Cap illegal (%d)\n"),
                           unit, egr_cap[i]));
            egr_cap[i] = 0;
        }
        if (egr_cap[i] > 511) {
            egr_cap[i] =
               ((SOC_SBX_CALADAN3_SWS_CLOCK_RATE)/(egr_cap[i]*4/1000000))-1;
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Unit %d Egress Hi Priority Cap%d set (%d)\n"),
                         unit, i, egr_cap[i]));
        }
    }
    if (SOC_IS_CALADAN3_REVB(unit)) {
        rv = soc_sbx_caladan3_sws_hpte_hipri_queue_set_revb(unit, 1,
                 max, egr_queues, egr_cap);
    } else {
        rv = soc_sbx_caladan3_sws_hpte_hipri_queue_set(unit, 1,
                 egr_queues[0], egr_cap[0], egr_queues[1],
                 egr_cap[1], egr_queues[2], egr_cap[2]);
    }
                                          
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Egress hipri_queue_set failed (%d)"), unit, rv));
    }
    
    return rv;
}


/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_wdrr_scheduler_config
 * Purpose
 *    Setup up IPTE_WDRR  for given port
 */
int
soc_sbx_caladan3_sws_pt_wdrr_scheduler_config(int unit, int pt,
                                              int base_port, int enable)
{
    uint32 regval = 0;
    soc_reg_t regs[] = {
         PT_IPTE_PORT0_WDRR_CFGr,
         PT_IPTE_PORT1_WDRR_CFGr,
         PT_IPTE_PORT2_WDRR_CFGr,
         PT_IPTE_PORT3_WDRR_CFGr,
         PT_IPTE_PORT4_WDRR_CFGr,
         PT_IPTE_PORT5_WDRR_CFGr,
         PT_IPTE_PORT6_WDRR_CFGr,
         PT_IPTE_PORT7_WDRR_CFGr,
         PT_IPTE_PORT8_WDRR_CFGr,
         PT_IPTE_PORT9_WDRR_CFGr,
         PT_IPTE_PORT10_WDRR_CFGr,
         PT_IPTE_PORT11_WDRR_CFGr,
    };

    if (!SOC_IS_CALADAN3_REVB(unit) && (base_port > 2)) {
        return SOC_E_NONE;
    }

    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, regs[base_port], pt, 0, &regval));
    if (enable != soc_reg_field_get(unit, regs[base_port], regval, PT_IPTE_WDRR_ENf)) {
        soc_reg_field_set(unit, regs[base_port], &regval, PT_IPTE_WDRR_ENf, enable);
        SOC_IF_ERROR_RETURN( soc_reg32_set(unit, regs[base_port], pt, 0, regval));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_os_scheduler_config
 * Purpose
 *    Oversubscription configuration
 */
int
soc_sbx_caladan3_sws_pt_os_scheduler_config(int unit)
{
    int port, phy_port;
    uint32 regval = 0;
    uint32 grpal = 0, grpbl = 0, grpcl = 0;
    uint32 grpaf = 0, grpbf = 0, grpcf = 0;
    soc_sbx_caladan3_port_map_t *port_map;
    soc_info_t *si;
    int is_tdm_oversubscribed = 0;

    si = &SOC_INFO(unit);
    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    is_tdm_oversubscribed = 
        soc_sbx_caladan3_sws_tdm_is_oversubscribed(unit, soc_sbx_caladan3_sws_tdm_id_current(unit));

    SOC_PBMP_ITER(PBMP_CL_ALL(unit), port) {
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy_port = si->port_l2p_mapping[port];
        } else {
            phy_port = port;
        }
        if (phy_port < 0) {
            break;
        }
 
        if (SOC_PORT_BLOCK(unit, phy_port) == soc_sbx_block_find(unit, SOC_BLK_CLPORT, 0)) {
            grpcl = 0x7;
            if ((si->port_speed_max[port] >= 25 * SOC_SBX_CALADAN3_GIG_SPEED) ||
                 (si->port_speed_max[port] >= 42 * SOC_SBX_CALADAN3_GIG_SPEED)) {
                grpal |= 1 << SOC_PORT_BINDEX(unit, phy_port);
            } else {
                grpbl |= 1 << SOC_PORT_BINDEX(unit, phy_port);
            }
        } else if (SOC_PORT_BLOCK(unit, phy_port) == soc_sbx_block_find(unit, SOC_BLK_CLPORT, 1)) {
            grpcf = 0;
            if ((si->port_speed_max[port] >= 25 * SOC_SBX_CALADAN3_GIG_SPEED) ||
                 (si->port_speed_max[port] >= 42 * SOC_SBX_CALADAN3_GIG_SPEED)) {
                grpaf |= 1 << SOC_PORT_BINDEX(unit, phy_port);
            } else {
                grpbf |= 1 << SOC_PORT_BINDEX(unit, phy_port);
            }
        }
    }

    if ((grpal & grpbl) != 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Overlapping map for Line Oversubscription GRPA(%x) GRPB(%x)\n"), 
                   unit, grpal, grpbl));
        return SOC_E_PARAM;
    }
    if ((grpaf & grpbf) != 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Overlapping map for Fab Oversubscription GRPA(%x) GRPB(%x)\n"), 
                   unit, grpaf, grpbf));
        return SOC_E_PARAM;
    }
    if (port_map->max_line_bw >= 100 || is_tdm_oversubscribed) {
        if ((grpal == 0) && (grpbl != 0)) {
            grpal = grpbl;
            grpbl = 0;
        }
    } else {
        grpal = 0;
        grpbl = 0;
        grpcl = 0;
    }

    if (port_map->max_fabric_bw >= 100) {
        if ((grpaf == 0) && (grpbf != 0)) {
            grpaf = grpbf;
            grpbf = 0;
        }
    } else {
        grpaf = 0;
        grpbf = 0;
        grpcf = 0;
    }

    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, PT_IPTE_CONFIG3r,
                      PT_INSTANCE(0), 0, &regval));
    soc_reg_field_set(unit, PT_IPTE_CONFIG3r, &regval,
        PT_IPTE_OVERSUBSCRIBED_GRP_A_PORT_ENf, grpal);
    soc_reg_field_set(unit, PT_IPTE_CONFIG3r, &regval,
        PT_IPTE_OVERSUBSCRIBED_GRP_B_PORT_ENf, grpbl);
    soc_reg_field_set(unit, PT_IPTE_CONFIG3r, &regval,
        PT_IPTE_OVERSUBSCRIBED_GRP_C_PORT_ENf, grpcl);
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, PT_IPTE_CONFIG3r, 
                      PT_INSTANCE(0), 0, regval));

    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, PT_IPTE_CONFIG3r,
                      PT_INSTANCE(1), 0, &regval));
    soc_reg_field_set(unit, PT_IPTE_CONFIG3r, &regval,
        PT_IPTE_OVERSUBSCRIBED_GRP_A_PORT_ENf, grpaf);
    soc_reg_field_set(unit, PT_IPTE_CONFIG3r, &regval,
        PT_IPTE_OVERSUBSCRIBED_GRP_B_PORT_ENf, grpbf);
    soc_reg_field_set(unit, PT_IPTE_CONFIG3r, &regval,
        PT_IPTE_OVERSUBSCRIBED_GRP_C_PORT_ENf, grpcf);
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, PT_IPTE_CONFIG3r, 
                      PT_INSTANCE(1), 0, regval));

    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_queues_map
 * Purpose
 *    Setup the HPTE_DEQUEUE_LOOKUP Table
 */
int
soc_sbx_caladan3_sws_pt_port_queues_map(int unit, int port)
{
    int status;
    int squeue, dqueue;
    int portidx = 0;
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_queues_t *lqinfo = NULL, *fqinfo = NULL, qtemp;
    soc_sbx_caladan3_port_map_info_t *lpinfo = NULL, *fpinfo = NULL;

    status = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, port, &portidx);
    if (SOC_SUCCESS(status)) {
        port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
        if (!port_map) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Queue info not found for port (%d)\n"),
                       unit, port));
            return SOC_E_PARAM;
        }
        lpinfo = &port_map->line_port_info[portidx];
        fpinfo = &port_map->fabric_port_info[portidx];
        lqinfo = &lpinfo->port_queues;
        fqinfo = &fpinfo->port_queues;

        SOC_SBX_C3_BMP_CLEAR(qtemp.dqueue_bmp, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES);

        /* Map one Ingress Squeue to 1 Dqueue, if this is a non-channelized port there is only one to map */
        SOC_SBX_C3_BMP_ITER_RANGE(lqinfo->squeue_bmp, SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE,
                                  squeue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {

            SOC_SBX_C3_BMP_ITER_RANGE(fqinfo->dqueue_bmp, SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE,
                                  dqueue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {

                if (SOC_SBX_C3_BMP_MEMBER_IN_RANGE(qtemp.dqueue_bmp,
                                                   SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE, dqueue)) {
                    continue;
                }

                SOC_SBX_C3_BMP_ADD(qtemp.dqueue_bmp, dqueue % SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);
                soc_sbx_caladan3_sws_hpte_wdrr_setup(unit, squeue, 1);
                soc_sbx_caladan3_sws_hpte_map_setup(unit, squeue, dqueue);
                soc_sbx_caladan3_sws_ipte_wdrr_setup(unit, dqueue, fpinfo->physical_intf, 1);
            break;
            }
        }
        SOC_SBX_C3_BMP_CLEAR(qtemp.dqueue_bmp, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES);
        /* Map one Egress Squeue to 1 Dqueue, if this is a non-channelized port there is only one to map */
        SOC_SBX_C3_BMP_ITER_RANGE(fqinfo->squeue_bmp, SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE,
                                  squeue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
            SOC_SBX_C3_BMP_ITER_RANGE(lqinfo->dqueue_bmp, SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE,
                                  dqueue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {
                if (SOC_SBX_C3_BMP_MEMBER_IN_RANGE(qtemp.dqueue_bmp, SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE, dqueue)) {
                    continue;
                }
                SOC_SBX_C3_BMP_ADD(qtemp.dqueue_bmp, dqueue % SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);
                soc_sbx_caladan3_sws_hpte_wdrr_setup(unit, squeue, 1);
                soc_sbx_caladan3_sws_hpte_map_setup(unit, squeue, dqueue);
                soc_sbx_caladan3_sws_ipte_wdrr_setup(unit, dqueue, lpinfo->physical_intf, 1);
                break;
            }
        }
    }
    return status;
}



/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_enable
 * Purpose
 *    Enable the port and validate if required, a0/A1 versions
 */
int
soc_sbx_caladan3_sws_pt_port_enable(int unit, int port, int enable, int validate)
{
    int status = SOC_E_NONE;
    if (enable) {
        /* 
         * Need to validate for every enable, but let caller decide 
         */
        if (validate) {
            status = soc_sbx_caladan3_sws_pt_port_config_validate(unit);
            if (SOC_FAILURE(status)) {
                return status;
            }
        }
        if (SOC_SUCCESS(status)) {
            status = soc_sbx_caladan3_sws_pt_port_config_set(unit, port);
        }
        if (SOC_SUCCESS(status)) {
            status = soc_sbx_caladan3_sws_pt_client_enable(unit, port, enable);
        }
        if (SOC_SUCCESS(status)) {
            status = soc_sbx_caladan3_sws_pt_port_queues_map(unit, port);
        }
        
    } else {
        status = soc_sbx_caladan3_sws_pt_client_enable(unit, port, enable);
    }
    return status;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_enable_rev_b
 * Purpose
 *    Enable the port and validate if required, B0 specific version
 */
int
soc_sbx_caladan3_sws_pt_port_enable_rev_b(int unit, int port, int enable, int validate)
{
    int status = SOC_E_NONE;
    if (enable) {
        /* 
         * Need to validate for every enable, but let caller decide 
         */
        if (validate) {
            status = soc_sbx_caladan3_sws_pt_port_config_validate(unit);
            if (SOC_FAILURE(status)) {
                return status;
            }
        }
        if (SOC_SUCCESS(status)) {

            status = soc_sbx_caladan3_sws_pt_port_config_set_rev_b(unit, port);

            if (SOC_SUCCESS(status)) {
                status = soc_sbx_caladan3_sws_pt_client_enable(unit, port, enable);
            }

            if (SOC_SUCCESS(status)) {
                status = soc_sbx_caladan3_sws_pt_port_queues_map(unit, port);
            }
        }
    } else {
        status = soc_sbx_caladan3_sws_pt_client_enable(unit, port, enable);
    }
    return status;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_enable_all
 * Purpose
 *    Enable all the port and validate if required, A0/a1 version
 */
int
soc_sbx_caladan3_sws_pt_port_enable_all(int unit)
{
    soc_port_t port;
    int status;

    if (SOC_RECONFIG_TDM && !SOC_HOTSWAP_TDM) {
        soc_sbx_caladan3_sws_pt_port_config_clear(unit);
    }
    status = soc_sbx_caladan3_sws_pt_port_config_validate(unit);
    if (SOC_FAILURE(status)) {
        return status;
    }
    SOC_PBMP_ITER(PBMP_ALL(unit), port) {
       status = soc_sbx_caladan3_sws_pt_port_enable(unit, port, TRUE, FALSE);
       if (SOC_FAILURE(status)) {
           return status;
       }
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_port_enable_all_rev_b
 * Purpose
 *    Enable all the port and validate if required, B0/B1 version
 */
int
soc_sbx_caladan3_sws_pt_port_enable_all_rev_b(int unit)
{
    int portidx = 0;
    int status;
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    soc_sbx_caladan3_port_map_info_t *port_info = NULL;

    if (SOC_RECONFIG_TDM && !SOC_HOTSWAP_TDM) {
        soc_sbx_caladan3_sws_pt_port_config_clear(unit);
    }
    status = soc_sbx_caladan3_sws_pt_port_config_validate(unit);
    if (SOC_FAILURE(status)) {
        return status;
    }

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    for (portidx = 0; portidx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; portidx++) {

        port_info = &port_map->line_port_info[portidx];

        if (!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }

        status = soc_sbx_caladan3_sws_pt_port_enable_rev_b(unit, portidx, TRUE, FALSE);
        if (SOC_FAILURE(status)) {
            return status;
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_caladan3_sws_hpte_redir_setup
 * Purpose:
 *     Setup redirection queues
 */
int
soc_sbx_caladan3_sws_hpte_redir_setup(int unit, int pt, int ingress, 
                                      int qid0, int qid1)
{
    uint32 regval = 0;
    
    if (ingress) {
        if ((qid0 >= SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) && 
             (qid0 < SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE)) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_INGRESS_CONFIGr,
                                              pt, 0, &regval));
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                              PT_HPTE_INGRESS_REDIRECTION_QUEUE_ID0f,
                              qid0);
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                              PT_HPTE_INGRESS_REDIRECTION_QUEUE_EN0f,
                              1);
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                              PT_HPTE_INGRESS_HI_PRI_QUEUE_EN0f,
                              1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_INGRESS_CONFIGr,
                                              pt, 0, regval));
        }
        if ((qid0 != qid1) &&
            (qid1 > SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) && 
            (qid1 < SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE)) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_INGRESS_CONFIGr,
                                              pt, 0, &regval));
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                              PT_HPTE_INGRESS_REDIRECTION_QUEUE_ID1f,
                              qid1);
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                              PT_HPTE_INGRESS_REDIRECTION_QUEUE_EN1f,
                              1);
            soc_reg_field_set(unit, PT_HPTE_INGRESS_CONFIGr, &regval,
                              PT_HPTE_INGRESS_HI_PRI_QUEUE_EN1f,
                              1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_INGRESS_CONFIGr,
                                              pt, 0, regval));
        }
    } else {
        if ((qid0 >= SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE) && 
             (qid0 < SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE))
        {
            uint32 qid = qid0 % SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION;
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_EGRESS_CONFIGr,
                                              pt, 0, &regval));
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIGr, &regval,
                              PT_HPTE_EGRESS_REDIRECTION_QUEUE_ID0f,
                              qid);
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIGr, &regval,
                              PT_HPTE_EGRESS_REDIRECTION_QUEUE_EN0f,
                              1);
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIGr, &regval,
                              PT_HPTE_EGRESS_HI_PRI_QUEUE_EN0f,
                              1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_EGRESS_CONFIGr,
                                              pt, 0, regval));
        }
        if ((qid0 != qid1) && 
            (qid1 >= SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE) && 
            (qid1 < SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE))
        {
            uint32 qid = qid1 % SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION;
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_EGRESS_CONFIGr,
                                              pt, 0, &regval));
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIGr, &regval,
                              PT_HPTE_EGRESS_REDIRECTION_QUEUE_ID1f,
                              qid);
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIGr, &regval,
                              PT_HPTE_EGRESS_REDIRECTION_QUEUE_EN1f,
                              1);
            soc_reg_field_set(unit, PT_HPTE_EGRESS_CONFIGr, &regval,
                              PT_HPTE_EGRESS_HI_PRI_QUEUE_EN1f,
                              1);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_EGRESS_CONFIGr,
                                              pt, 0, regval));
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_sws_pt_fc_init
 * Purpose
 *    Set up FC
 */
int
soc_sbx_caladan3_sws_pt_fc_init(int unit)
{
    int i;

    for (i=0; i < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE; i++) {
        soc_sbx_caladan3_sws_pt_instance_fc_init(unit, i);
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *   soc_sbx_caladan3_sws_pt_instance_fc_init 
 * Purpose
 *   Setup FC on a given instance
 */
int
soc_sbx_caladan3_sws_pt_instance_fc_init(int unit, int instance)
{
    uint32 uRegisterValue = 0;
    uint32 fc_calendar_length;
    uint32 uEnable0 = 0;
    uint32 uEnable1 = 0;
 
    if (instance == 0) {
        fc_calendar_length = soc_property_get(unit, "fc_calendar_length_il_line", 64);
    } else {
        fc_calendar_length = soc_property_get(unit, "fc_calendar_length_il_fabric", 0);
    }
        
    switch (fc_calendar_length) 
        {
        case 16:
            uEnable0 = 0xffff;
            uEnable1 = 0x0;
            break;
        case 32:
            uEnable0 = 0xffffffff;
            uEnable1 = 0x0;
            break;
        case 64:
        case 128:
        case 256:
            uEnable0 = 0xffffffff;
            uEnable1 = 0xffffffff;
            break;
        case 0:
                uEnable0 = 0;
                uEnable1 = 0;
                break;
        default:
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Invalid ilkn fc_calendar_length(%d) enabling 64 IPTE(%d) fc queues\n"),
                      fc_calendar_length, instance));
            uEnable0 = 0xffffffff;
            uEnable1 = 0xffffffff;
            break;
        }
    
    soc_reg_field_set(unit, PT_IPTE_FC_CHAN_CFG0r, &uRegisterValue, ENABLEf, uEnable0);
    soc_reg32_set(unit, PT_IPTE_FC_CHAN_CFG0r, PT_INSTANCE(instance), 0, uRegisterValue);
    soc_reg_field_set(unit, PT_IPTE_FC_CHAN_CFG1r, &uRegisterValue, ENABLEf, uEnable1);
    soc_reg32_set(unit, PT_IPTE_FC_CHAN_CFG1r, PT_INSTANCE(instance), 0, uRegisterValue );

    return SOC_E_NONE;
}

#define PT_INVALID_Q_VAL 0xFF

/**
 * Function:
 *     soc_sbx_caladan3_sws_ipte_init
 * Purpose:
 *     Initialize IPTE
 * XXXTBD: Believe this function needs to still account for channelized 
 * ports; i.e. dqueue_base + num_dqueue 
 * Math in algorithm may need to be adjusted or is above math sufficient??... XXXTBC
 */
int 
soc_sbx_caladan3_sws_ipte_init(int unit)
{
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_port_map_info_t *port_info;
    soc_sbx_caladan3_queues_t *qi;
    uint32 regval = 0;

    uint32 pt0_grp0_base = 0;
    uint32 pt0_grp0_max = 0;
    uint32 pt0_grp1_base = 0;
    uint32 pt0_grp1_max = 0;

    uint32 pt1_grp0_base = 0;
    uint32 pt1_grp0_max = 0;
    uint32 pt1_grp1_base = 0;
    uint32 pt1_grp1_max = 0;

    uint8 pt0_queues[MAX_PT_QS] = {PT_INVALID_Q_VAL};
    uint8 pt1_queues[MAX_PT_QS] = {PT_INVALID_Q_VAL};
    uint8 temp_pt0_q, temp_pt1_q;
    uint8 pt0_q_cnt = 0;
    uint8 pt1_q_cnt = 0;
    int side;
    int pt0_grp0_min_idx = 0, pt0_grp0_max_idx = 0;
    int pt0_grp1_min_idx = 0, pt0_grp1_max_idx = 0;
    int pt1_grp0_min_idx = 0, pt1_grp0_max_idx = 0;
    int pt1_grp1_min_idx = 0, pt1_grp1_max_idx = 0;

    int p, idx, pt_instance;

    int pt0_idx, pt1_idx;
    int no_overlap = 0; 

    pt0_grp0_min_idx = 0;
    pt1_grp0_min_idx = 0;
    pt0_grp0_max_idx = 0;
    pt1_grp0_max_idx = 0;


    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    for (p = 0; p < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; p++) {

        /* Just use the line side to see if this lport is valid. If so then process
         * BOTH line and fabric side pair for this valid logical port. 
         */
        port_info = &port_map->line_port_info[p];
        if (!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }

        /* Populate PT Qs for both the line side and fabric side */
        for (side = 0; side < MAX_LINE_FABRIC_ID; side++) {

            /* Line Side port_info loaded above (for side = 0)*/
            if (side == 1) {                
                port_info = &port_map->fabric_port_info[p];
            }
            qi = &port_info->port_queues;

            if (port_info->physical_intf == 0) {
                /* PT0 */
                /* Inserted sorted, ascending */
                if (pt0_q_cnt < MAX_PT_QS) {
                    if (pt0_q_cnt != 0) { 
                        if (qi->dqueue_base < pt0_queues[pt0_q_cnt - 1]) {
                            for (idx = 0; 
                                 idx < pt0_q_cnt && qi->dqueue_base < pt0_queues[pt0_q_cnt - 1 - idx]; idx++) {

                                temp_pt0_q = pt0_queues[pt0_q_cnt - 1 - idx];
                                pt0_queues[pt0_q_cnt - 1 - idx] = qi->dqueue_base;
                                pt0_queues[pt0_q_cnt - idx] = temp_pt0_q;
                            }
                        } else {
                            pt0_queues[pt0_q_cnt] = qi->dqueue_base;
                        }
                    } else {
                        pt0_queues[pt0_q_cnt] = qi->dqueue_base;
                    }

                    pt0_q_cnt++;

                } else {
                    assert(0);
                }

            } else {
                /* PT1 */
                if (pt1_q_cnt < MAX_PT_QS) {
                    if (pt1_q_cnt != 0) {
                        if (qi->dqueue_base < pt1_queues[pt1_q_cnt - 1]) {
                            for (idx = 0; 
                                 idx < pt1_q_cnt && qi->dqueue_base < pt1_queues[pt1_q_cnt - 1 - idx]; idx++) {
                                
                                temp_pt1_q = pt1_queues[pt1_q_cnt - 1 - idx];
                                pt1_queues[pt1_q_cnt - 1 - idx] = qi->dqueue_base;
                                pt1_queues[pt1_q_cnt - idx] = temp_pt1_q;
                            }
                        } else {
                            pt1_queues[pt1_q_cnt] = qi->dqueue_base;
                        }
                    } else {
                        pt1_queues[pt1_q_cnt] = qi->dqueue_base;
                    } 
                    pt1_q_cnt++;
                } else {
                    assert(0);
                }
            }
        }
    }

    /* Compare the PT Qs of each and determine which Q group has lowest Q number.
     * From above the Qs are now sorted ascending. Therefore element 0 has 
     * lowest number Q in each group.
     */
    if (pt0_queues[0] < pt1_queues[0] && pt0_q_cnt && pt1_q_cnt) {

        /* Determine if overlap in Q ranges are present. 
         * Compare MIN of larger Q value range to MAX of smaller Q value range.
         */
        if (pt1_queues[0] < pt0_queues[pt0_q_cnt - 1]) {

            while ((pt0_grp0_max_idx < pt0_q_cnt - 1) && pt0_queues[pt0_grp0_max_idx] < pt1_queues[0]) {
                pt0_grp0_max_idx++;
            }
            pt0_grp0_max_idx--; 

            if (pt0_grp0_max_idx < pt0_q_cnt - 1) {

                pt0_grp1_min_idx = pt0_grp0_max_idx + 1;
                pt0_grp1_max_idx = pt0_q_cnt - 1;
                
                while ((pt1_grp0_max_idx < pt1_q_cnt - 1) && 
                       pt1_queues[pt1_grp0_max_idx] < pt0_queues[pt0_grp1_min_idx]) {

                    pt1_grp0_max_idx++;
                }
                pt1_grp0_max_idx--;

                if (pt1_grp0_max_idx < pt1_q_cnt - 1) {
                    pt1_grp1_min_idx = pt1_grp0_max_idx + 1;
                    pt1_grp1_max_idx = pt1_q_cnt - 1;
                }
                /* Sanity check to ensure no more than two groups*/
                for (pt0_idx = pt0_grp1_min_idx; pt0_idx < pt0_grp1_max_idx; pt0_idx++) {

                    for (pt1_idx = pt1_grp1_min_idx; pt1_idx < pt1_grp1_max_idx; pt1_idx++) {
                        if (pt0_queues[pt0_idx] > pt1_queues[pt1_idx]) {
                            assert(0);
                        }
                    }
                }
            }
        } else {
            no_overlap = 1;
        }

    } else if (pt1_queues[0] < pt0_queues[0] && pt0_q_cnt && pt1_q_cnt) {

        /* Determine if overlap in Q ranges are present. 
         * Compare MIN of larger Q value range to MAX of smaller Q value range.
         */
        if (pt0_queues[0] < pt1_queues[pt1_q_cnt - 1]) {

            while ((pt1_grp0_max_idx < pt1_q_cnt - 1) && pt1_queues[pt1_grp0_max_idx] < pt0_queues[0]) {
                pt1_grp0_max_idx++;
            }
            pt1_grp0_max_idx--; 

            if (pt1_grp0_max_idx < pt1_q_cnt - 1) {

                pt1_grp1_min_idx = pt1_grp0_max_idx + 1;
                pt1_grp1_max_idx = pt1_q_cnt - 1;
                
                while ((pt0_grp0_max_idx < pt0_q_cnt - 1) && 
                       pt0_queues[pt0_grp0_max_idx] < pt1_queues[pt1_grp1_min_idx]) {

                    pt0_grp0_max_idx++;
                }
                pt0_grp0_max_idx--;

                if (pt0_grp0_max_idx < pt0_q_cnt - 1) {
                    pt0_grp1_min_idx = pt0_grp0_max_idx + 1;
                    pt0_grp1_max_idx = pt0_q_cnt - 1;
                }
                /* Sanity check to ensure no more than two groups*/
                for (pt1_idx = pt1_grp1_min_idx; pt1_idx < pt1_grp1_max_idx; pt1_idx++) {

                    for (pt0_idx = pt0_grp1_min_idx; pt0_idx < pt0_grp1_max_idx; pt0_idx++) {
                        if (pt1_queues[pt1_idx] > pt0_queues[pt0_idx]) {
                            assert(0);
                        }
                    }
                }
            }
        } else {
            no_overlap = 1;
        }
    
    }


    if (no_overlap) {
        pt0_grp0_min_idx = 0;
        pt0_grp0_max_idx = pt0_q_cnt - 1;
        
        /* No group 1 therefore */
        pt0_grp1_min_idx = 0;
        pt0_grp1_max_idx = 0;
        
        pt1_grp0_min_idx = 0;
        pt1_grp0_max_idx = pt1_q_cnt - 1;
        
        /* No group 1 therefore */
        pt1_grp1_min_idx = 0;
        pt1_grp1_max_idx = 0;
    }

    pt0_grp0_base = pt0_queues[pt0_grp0_min_idx];
    pt0_grp0_max = pt0_queues[pt0_grp0_max_idx];

    pt0_grp1_base = pt0_queues[pt0_grp1_min_idx];
    pt0_grp1_max = pt0_queues[pt0_grp1_max_idx];

    pt1_grp0_base = pt1_queues[pt1_grp0_min_idx];
    pt1_grp0_max = pt1_queues[pt1_grp0_max_idx];

    pt1_grp1_base = pt1_queues[pt1_grp1_min_idx];
    pt1_grp1_max = pt1_queues[pt1_grp1_max_idx];

    /* There is the possibility of a NULL TDM being used */
    if (PT_INVALID_Q_VAL == pt0_grp0_base) {
        pt0_grp0_min_idx = 0;
        pt0_grp0_base = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE;
        pt0_grp0_max = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + 
            SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION - 1;
        pt0_grp1_base = pt0_grp1_max = 0;

        /* No group 1 therefore */
        pt0_grp1_min_idx = 0;
    }

    if (PT_INVALID_Q_VAL == pt1_grp0_base) {
        pt1_grp0_min_idx = 0;
        pt1_grp0_base = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE;
        pt1_grp0_max = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE + 
            SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION - 1;
        pt1_grp1_base = pt1_grp1_max = 0;

        /* No group 1 therefore */
        pt1_grp1_min_idx = 0;
    }

    for (pt_instance = 0; pt_instance < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE; pt_instance++) {
        regval = 0;

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_QM_CONFIGr,
                               PT_INSTANCE(pt_instance), 0, &regval));

        pt_ipte_grp[pt_instance].grp0_base = 0;
        pt_ipte_grp[pt_instance].grp0_range = 0;
        pt_ipte_grp[pt_instance].grp1_base = 0;
        pt_ipte_grp[pt_instance].grp1_range = 0;

        if (pt_instance == 0) {

            pt_ipte_grp[pt_instance].grp0_base = pt0_grp0_base;
            pt_ipte_grp[pt_instance].grp0_range = pt0_grp0_max - pt0_grp0_base + 1;

            if (pt0_grp1_min_idx != pt0_grp0_min_idx) {
                pt_ipte_grp[pt_instance].grp1_base = pt0_grp1_base;
                pt_ipte_grp[pt_instance].grp1_range = pt0_grp1_max - pt0_grp1_base + 1;
            } 

        } else if (pt_instance == 1) {

            pt_ipte_grp[pt_instance].grp0_base = pt1_grp0_base;
            pt_ipte_grp[pt_instance].grp0_range = pt1_grp0_max - pt1_grp0_base + 1;

            if (pt1_grp1_min_idx != pt1_grp0_min_idx) {
                pt_ipte_grp[pt_instance].grp1_base = pt1_grp1_base;
                pt_ipte_grp[pt_instance].grp1_range = pt1_grp1_max - pt1_grp1_base + 1;
            } 

        } else {
            assert(0);
        }

        soc_reg_field_set(unit, PT_IPTE_QM_CONFIGr, &regval,
                          PT_IPTE_GRP0_BASE_QUEUEf, 
                          pt_ipte_grp[pt_instance].grp0_base);

        soc_reg_field_set(unit, PT_IPTE_QM_CONFIGr, &regval, 
                          PT_IPTE_GRP0_QUEUE_RANGEf,
                          pt_ipte_grp[pt_instance].grp0_range); 

        soc_reg_field_set(unit, PT_IPTE_QM_CONFIGr, &regval,
                          PT_IPTE_GRP1_BASE_QUEUEf,
                          pt_ipte_grp[pt_instance].grp1_base);

        soc_reg_field_set(unit, PT_IPTE_QM_CONFIGr, &regval,
                          PT_IPTE_GRP1_QUEUE_RANGEf,
                          pt_ipte_grp[pt_instance].grp1_range);

        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_QM_CONFIGr,
                               PT_INSTANCE(pt_instance), 0, regval));
        pt_ipte_init = 1;
    }

    return SOC_E_NONE;
}


/**
 * Function:
 *     soc_sbx_caladan3_sws_hpte_init
 * Purpose:
 *     Initialize HPTE
 */
int 
soc_sbx_caladan3_sws_hpte_init(int unit)
{
    soc_sbx_caladan3_port_map_t *port_map __attribute__((unused));
    uint32 regval=0;
    int q, n, m;
    int status, instance;
    int ing_rqid0 = 0, ing_rqid1 = 0;
    int egr_rqid0 = 0, egr_rqid1 = 0;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    regval = 0;
    soc_reg_field_set(unit, PT_HPTE_CONFIGr, &regval,
                      PT_HPTE_INGRESS_BASE_QUEUEf, 
                      SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE);
    soc_reg_field_set(unit, PT_HPTE_CONFIGr, &regval, 
                      PT_HPTE_INGRESS_QUEUE_RANGEf,
                      SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION); 
    soc_reg_field_set(unit, PT_HPTE_CONFIGr, &regval,
                      PT_HPTE_EGRESS_BASE_QUEUEf,
                      SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE);
    soc_reg_field_set(unit, PT_HPTE_CONFIGr, &regval,
                      PT_HPTE_EGRESS_QUEUE_RANGEf,
                      SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_CONFIGr,
                           PT0_INSTANCE, 0, regval));

    regval = 0;
    soc_reg_field_set(unit, PT_HPTE_CONFIGr, &regval,
                      PT_HPTE_INGRESS_BASE_QUEUEf, 
                      SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE);
    soc_reg_field_set(unit, PT_HPTE_CONFIGr, &regval, 
                      PT_HPTE_INGRESS_QUEUE_RANGEf,
                      SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION); 
    soc_reg_field_set(unit, PT_HPTE_CONFIGr, &regval,
                      PT_HPTE_EGRESS_BASE_QUEUEf,
                      SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE);
    soc_reg_field_set(unit, PT_HPTE_CONFIGr, &regval,
                      PT_HPTE_EGRESS_QUEUE_RANGEf,
                      SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_CONFIGr,
                           PT1_INSTANCE, 0, regval));

    status = soc_sbx_caladan3_sws_redirect_queues_get(unit, 
            &ing_rqid0, &ing_rqid1, &egr_rqid0, &egr_rqid1);
    if (SOC_SUCCESS(status)) {
        for (instance=0; 
             instance < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE; 
                 instance++) {
            soc_sbx_caladan3_sws_hpte_redir_setup(unit, PT_INSTANCE(instance),
                0, ing_rqid0, ing_rqid1);
            soc_sbx_caladan3_sws_hpte_redir_setup(unit, PT_INSTANCE(instance),
                1, egr_rqid0, egr_rqid1);
        }
    }

    status = soc_sbx_caladan3_sws_app_queues_get(unit, &n, &m);
    if (SOC_SUCCESS(status) && (m > 0)) {
        for (q=n; q < m+n; q++) {
            soc_sbx_caladan3_sws_hpte_wdrr_setup(unit, q, 1);
            /* Application will determine the dqueue, set 0 */
            soc_sbx_caladan3_sws_hpte_map_setup(unit, q, 0);
        }
    }

    /* enable scheduler */
    for (instance=0; 
             instance < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE; 
                 instance++) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_HPTE_SCHEDULER_CFGr,
                               PT_INSTANCE(instance), 0, &regval));
        soc_reg_field_set(unit, PT_HPTE_SCHEDULER_CFGr, 
                          &regval, PT_HPTE_EG_PORT_ENf, 1);
        soc_reg_field_set(unit, PT_HPTE_SCHEDULER_CFGr, 
                          &regval, PT_HPTE_IG_PORT_ENf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_HPTE_SCHEDULER_CFGr,
                               PT_INSTANCE(instance), 0, regval));
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *   soc_sbx_caladan3_sws_cmic_hpp_loopback_map_set 
 * Purpose
 *   Setup CMIC loopback on SWS, Packets coming in CMIC is returned back to CMIC port
 */
int 
soc_sbx_caladan3_sws_cmic_hpp_loopback_map_set(int unit, int cmic_port, int enable)
{
    int idx, sq0, dq0, dq1;
    soc_sbx_caladan3_port_map_t *port_map;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    if (!port_map) {
        return SOC_E_NONE;
    }
    sq0 = dq0 = 0;
    dq1 = 0;
    for (idx = 0; idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {
        if (!(port_map->line_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        if (port_map->line_port_info[idx].port == cmic_port) {
            sq0 = port_map->line_port_info[idx].port_queues.squeue_base;
            dq0 = port_map->fabric_port_info[idx].port_queues.dqueue_base;
            dq1 = port_map->line_port_info[idx].port_queues.dqueue_base;
            break;
        }
    }
    if (idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
        if (enable) {
            soc_sbx_caladan3_sws_hpte_map_setup(unit, sq0, dq1);
        } else {
            soc_sbx_caladan3_sws_hpte_map_setup(unit, sq0, dq0);
        }
        
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *   soc_sbx_caladan3_sws_cmic_mac_loopback_map_set 
 * Purpose
 *   Setup CMIC to Port loopback on SWS,
 *   Packets coming in CMIC is switch to a given port
 *   Packets coming in given port is switch to a Cmic port
 */
int 
soc_sbx_caladan3_sws_cmic_mac_loopback_set(int unit, int cmic_port, 
                                           int port, int enable)
{
    int idx, sq0, dq0, sq1, dq1;
    soc_sbx_caladan3_port_map_t *port_map;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    if (!port_map) {
        return SOC_E_NONE;
    }
    sq0 = dq0 = 0;
    sq1 = dq1 = 0;

    /* detect cmic port source/destination queue */
    for (idx = 0; idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {
        if (!(port_map->line_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        if (port_map->line_port_info[idx].port == cmic_port) {
            /* Use L2/L3 Forwarding instead of changing queue map
             * in non-lrp-bypass mode. This is here only for the sake of it
             */
            if (soc_property_get(unit, spn_LRP_BYPASS, 0)) {
                sq0 = port_map->line_port_info[idx].port_queues.squeue_base;
                dq0 = port_map->line_port_info[idx].port_queues.dqueue_base;
            } else {
                sq0 = port_map->fabric_port_info[idx].port_queues.squeue_base;
                dq0 = port_map->fabric_port_info[idx].port_queues.dqueue_base;
            }
        }
    }

    /* didn't find a valid cpu port destination queue */
    if (!dq0) {
        return SOC_E_PARAM;
    }

    /* it's possible a port has multiple source/destination queues
     * (fabric side). To be safe, We need to point the cpu destination queue to one of the
     * port's queue and point all of the port's destnation queue to the cpu port
     */
    for (idx = 0; idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {
        if (!(port_map->line_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        if (port_map->line_port_info[idx].port == port) {
            sq1 = port_map->line_port_info[idx].port_queues.squeue_base;
            dq1 = port_map->line_port_info[idx].port_queues.dqueue_base;
            if (enable) {
                soc_sbx_caladan3_sws_hpte_map_setup(unit, sq0, dq1);
                soc_sbx_caladan3_sws_hpte_map_setup(unit, sq1, dq0);
            } else {
                soc_sbx_caladan3_sws_hpte_map_setup(unit, sq0, dq0);
                soc_sbx_caladan3_sws_hpte_map_setup(unit, sq1, dq1);
            }
        } else if (port_map->fabric_port_info[idx].port == port) {
            sq1 = port_map->fabric_port_info[idx].port_queues.squeue_base;
            dq1 = port_map->fabric_port_info[idx].port_queues.dqueue_base;
            if (enable) {
                soc_sbx_caladan3_sws_hpte_map_setup(unit, sq0, dq1);
                soc_sbx_caladan3_sws_hpte_map_setup(unit, sq1, dq0);
            } else {
                soc_sbx_caladan3_sws_hpte_map_setup(unit, sq0, dq0);
                soc_sbx_caladan3_sws_hpte_map_setup(unit, sq1, dq1);
            }
        }
    }
    return SOC_E_NONE;
}

static int soc_sbx_caladan3_pt_shaper_rate2float(uint32 kbits_sec, int *exponent, int *mantissa)
{
  uint32 bits_interval;
  int e, m;
  int i;


  bits_interval = kbits_sec / (1000000/512);
  
  e = 0;
  for (i=0; i<16; i++) {
    if ((1<<i) > bits_interval)
      break;
    e = i;
  }

  m = ((float)bits_interval / (1<<e) - 1) * 128;
  if (m < 0)
    m = 0;

  LOG_CLI((BSL_META(" kbits %d interval bits %d e=%d m=%d\n"), kbits_sec, bits_interval, e, m));
  
  *exponent = e;
  *mantissa = m;

  return SOC_E_NONE;
}

int soc_sbx_caladan3_sws_pt_shaper_set(int unit, int queue, uint32 kbits_sec, uint32 kbits_burst)
{
  int e, m;
  ipte_qmax_rate_entry_t qmax_rate;
  ipte_qmax_burst_entry_t qmax_burst;
  int pt, pt_blk, idx;

  if (queue >= 192) {
    pt = 0;
    idx = queue - 192;
  } else if (queue >= 128) {
    pt = 1;
    idx = queue - 128;
  } else {
    return SOC_E_PARAM;
  }
  pt_blk = soc_sbx_block_find(unit, SOC_BLK_PT, pt);
    
  soc_sbx_caladan3_pt_shaper_rate2float(kbits_sec, &e, &m);
  sal_memset(&qmax_rate, 0, sizeof(ipte_qmax_rate_entry_t));
  soc_mem_field_set(unit, IPTE_QMAX_RATEm, &qmax_rate.entry_data[0],
		    RATE_EXPONENTf, (uint32*)&e);
  soc_mem_field_set(unit, IPTE_QMAX_RATEm, &qmax_rate.entry_data[0],
		    RATE_MANTISSAf, (uint32*)&m);
  SOC_IF_ERROR_RETURN(soc_mem_write(unit, IPTE_QMAX_RATEm, pt_blk, idx, &qmax_rate));


  soc_sbx_caladan3_pt_shaper_rate2float(kbits_burst, &e, &m);
  sal_memset(&qmax_burst, 0, sizeof(ipte_qmax_burst_entry_t));
  soc_mem_field_set(unit, IPTE_QMAX_BURSTm, &qmax_burst.entry_data[0],
		    BURST_EXPONENTf, (uint32*)&e);
  soc_mem_field_set(unit, IPTE_QMAX_BURSTm, &qmax_burst.entry_data[0],
		    BURST_MANTISSAf, (uint32*)&m);
  SOC_IF_ERROR_RETURN(soc_mem_write(unit, IPTE_QMAX_BURSTm, pt_blk, idx, &qmax_burst));

  return SOC_E_NONE;
}

int soc_sbx_caladan3_sws_pt_shaper_get(int unit, int queue, uint32 *kbits_sec, uint32 *kbits_burst)
{
  int e, m;
  ipte_qmax_rate_entry_t qmax_rate;
  ipte_qmax_burst_entry_t qmax_burst;
  int pt, pt_blk, idx;

  if (queue >= 192) {
    pt = 0;
    idx = queue - 192;
  } else if (queue >= 128) {
    pt = 1;
    idx = queue - 128;
  } else {
    return SOC_E_PARAM;
  }
  pt_blk = soc_sbx_block_find(unit, SOC_BLK_PT, pt);
    

  SOC_IF_ERROR_RETURN(soc_mem_read(unit, IPTE_QMAX_RATEm, pt_blk, idx, &qmax_rate));
  soc_mem_field_get(unit, IPTE_QMAX_RATEm, &qmax_rate.entry_data[0],
		    RATE_EXPONENTf, (uint32*)&e);
  soc_mem_field_get(unit, IPTE_QMAX_RATEm, &qmax_rate.entry_data[0],
		    RATE_MANTISSAf, (uint32*)&m);
  *kbits_sec = (1 + (float)m/128) * (1<<e) * (1000000/512);

  SOC_IF_ERROR_RETURN(soc_mem_read(unit, IPTE_QMAX_BURSTm, pt_blk, idx, &qmax_burst));
  soc_mem_field_get(unit, IPTE_QMAX_BURSTm, &qmax_burst.entry_data[0],
		    BURST_EXPONENTf, (uint32*)&e);
  soc_mem_field_get(unit, IPTE_QMAX_BURSTm, &qmax_burst.entry_data[0],
		    BURST_MANTISSAf, (uint32*)&m);
  /* coverity [integer_division] */  
  *kbits_burst = (1 + (float)m/128) * (1<<e) * (1000000/512);
  
  return SOC_E_NONE;
}

/**
 * Function:
 *     soc_sbx_caladan3_sws_pt_uninit
 * Purpose:
 *     Clear up software state
 */
int
soc_sbx_caladan3_sws_pt_uninit(int unit)
{
    if (_pt_control[(unit)].port_entries[0]) {
        soc_cm_sfree(unit, _pt_control[(unit)].port_entries[0]);
    }
    _pt_control[(unit)].port_entries[0] = NULL;
    if (SOC_IS_CALADAN3_REVB(unit)) {
        _pt_control[(unit)].port_entries[1] = NULL;
    }
    if (_pt_control[(unit)].client_entries[0]) {
        soc_cm_sfree(unit, _pt_control[(unit)].client_entries[0]);
    }
    _pt_control[(unit)].client_entries[0] = NULL;
    if (SOC_IS_CALADAN3_REVB(unit)) {
        _pt_control[(unit)].client_entries[1] = NULL;
    }
    PT_CB_INUSE(unit) = 0;
    return SOC_E_NONE;
}

/**
 * Function:
 *     soc_sbx_caladan3_sws_pt_init
 * Purpose:
 *     Initialize Packet Transmitter
 */
int soc_sbx_caladan3_sws_pt_init(int unit)
{
    uint32 regval, instance;
    int timeout = 0;
    soc_sbx_caladan3_sws_pt_cfg_t *pt_cfg = NULL;
    sws_pt_port_fifo_t  *pt_fifo_cfg = NULL;
    ipte_port_cal_entry_t    *port_entries;
    ipte_client_cal_entry_t  *client_entries;
    uint32 alloc_size;
    int done = 0xffff;

    if (!SOC_RECONFIG_TDM) {
        soc_sbx_caladan3_sws_pt_uninit(unit);
    }

    if (!PT_CB_INUSE(unit)) {
        sal_memset(&_pt_control[unit], 0 , sizeof(pt_cb_t));
        PT_CB_MAX_CAL(unit) = SOC_IS_CALADAN3_REVB(unit) ? 2 : 1;
        if (SOC_IS_CALADAN3_REVB(unit)) {
            PT_REMAP_ENABLED(unit) = 0;
        } else {
            PT_REMAP_ENABLED(unit) = 1;
        }
        for (instance=0;
                 instance < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE;
                     instance++) {
            if (SOC_IS_CALADAN3_REVB(unit)) {
                SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG4r,
                                    PT_INSTANCE(instance), 0, &regval));
                ACTIVE_PORT_CAL_ID(unit, instance) =
                     soc_reg_field_get(unit, PT_IPTE_CONFIG4r, regval,
                                   PT_IPTE_ACTIVE_PORT_CALf);
                ACTIVE_CLIENT_CAL_ID(unit, instance) =
                     soc_reg_field_get(unit, PT_IPTE_CONFIG4r, regval,
                                   PT_IPTE_ACTIVE_CLIENT_CALf);
                if (ACTIVE_PORT_CAL_ID(unit, instance)) {
                    ACTIVE_PORT_CAL(unit, instance) = IPTE_PORT_CAL1m;
                    ACTIVE_CLIENT_CAL(unit, instance) = IPTE_CLIENT_CAL1m;
                    INACTIVE_PORT_CAL(unit, instance) = IPTE_PORT_CALm;
                    INACTIVE_CLIENT_CAL(unit, instance) = IPTE_CLIENT_CALm;
                } else {
                    ACTIVE_PORT_CAL(unit, instance) = IPTE_PORT_CALm;
                    ACTIVE_CLIENT_CAL(unit, instance) = IPTE_CLIENT_CALm;
                    INACTIVE_PORT_CAL(unit, instance) = IPTE_PORT_CAL1m;
                    INACTIVE_CLIENT_CAL(unit, instance) = IPTE_CLIENT_CAL1m;
                }
            } else {
                ACTIVE_PORT_CAL_ID(unit, instance) = 0;
                ACTIVE_CLIENT_CAL_ID(unit, instance) = 0;
                INACTIVE_PORT_CAL(unit, instance) = IPTE_PORT_CALm;
                INACTIVE_CLIENT_CAL(unit, instance) = IPTE_CLIENT_CALm;
                ACTIVE_PORT_CAL(unit, instance) = IPTE_PORT_CALm;
                ACTIVE_CLIENT_CAL(unit, instance) = IPTE_CLIENT_CALm;
            }
        }

        alloc_size = soc_mem_index_count(unit,IPTE_PORT_CALm) *
            sizeof(ipte_port_cal_entry_t);
        port_entries = soc_cm_salloc( unit, alloc_size, "IPTE_PORT_CALm");
        if (port_entries == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: Failed allocating port entries\n"), unit));
            return SOC_E_MEMORY;
        }
        _pt_control[(unit)].port_entries[0] = port_entries;
        if (SOC_IS_CALADAN3_REVB(unit)) {
            _pt_control[(unit)].port_entries[1] = port_entries;
        }

        alloc_size = soc_mem_index_count(unit,IPTE_CLIENT_CALm) *
            sizeof(ipte_client_cal_entry_t);
        client_entries = soc_cm_salloc( unit, alloc_size, "IPTE_CLIENT_CALm");
        if (client_entries == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: Failed allocating client entries\n"), unit));
            if (port_entries) {
                soc_cm_sfree(unit, port_entries);
            }
            return SOC_E_MEMORY;
        }
        _pt_control[(unit)].client_entries[0] = client_entries;
        if (SOC_IS_CALADAN3_REVB(unit)) {
            _pt_control[(unit)].client_entries[1] = client_entries;
        }
        PT_CB_INUSE(unit) = 1;
    }

    /* initalize memories */
    if (!SOC_RECONFIG_TDM || SOC_CONTROL(unit)->cl0_reset) {
    for (instance=0; 
             instance < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE;
                 instance++) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_GEN_CONFIGr, 
                                          PT_INSTANCE(instance), 0, &regval));
        soc_reg_field_set(unit, PT_GEN_CONFIGr, &regval, PT_SOFT_RESET_Nf, 0);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_GEN_CONFIGr,
                                          PT_INSTANCE(instance), 0, regval));
        soc_reg_field_set(unit, PT_GEN_CONFIGr, &regval, PT_SOFT_RESET_Nf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_GEN_CONFIGr,
                                          PT_INSTANCE(instance), 0, regval));
        sal_udelay(100);

        if (SOC_IS_CALADAN3_REVB(unit)) {
            done = 0x3ffff;
        }
        regval = 0;
        soc_reg_field_set(unit, PT_MEM_INITr, &regval, PT_DO_INITf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_MEM_INITr,
                                          PT_INSTANCE(instance), 0, regval));

#ifdef BCM_WARM_BOOT_SUPPORT
	if(!SOC_WARM_BOOT(unit)) {
#endif /* BCM_WARM_BOOT_SUPPORT */

        /* wait for init done - global config response */
        do {
            sal_udelay(1000);
            timeout++;
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_MEM_INIT_DONEr,
                                          PT_INSTANCE(instance), 0, &regval));
        } while ((regval != done) && (timeout < 100));

        if ((timeout >= 100) && !SAL_BOOT_PLISIM) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Timeout waiting for PT_MEM_INIT_DONE\n")));
            return SOC_E_TIMEOUT;
        }

        /* clear init done */
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_MEM_INIT_DONEr,
                                          PT_INSTANCE(instance), 0, &regval));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_MEM_INIT_DONEr,
                                          PT_INSTANCE(instance), 0, regval));

#ifdef BCM_WARM_BOOT_SUPPORT
        }
#endif /* BCM_WARM_BOOT_SUPPORT */ 

        /* Setup remap */
        if (SOC_IS_CALADAN3_REVB(unit)) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_CONFIG0r,
                                PT_INSTANCE(instance), 0, &regval));
            soc_reg_field_set(unit, PT_IPTE_CONFIG0r, &regval,
                              PT_IPTE_PORT_REMAP_ENf, 0);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_CONFIG0r,
                                PT_INSTANCE(instance), 0, regval));
        }


        if (SOC_RECONFIG_TDM) {
            break;
        }
    }
    }

    /* Initialize the port buffer allocations */
    for (instance=0;
             instance < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE; 
                instance++) {
        pt_cfg = soc_sbx_caladan3_sws_pt_cfg_get(unit, instance);
        if (pt_cfg == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d pt_cfg_get failed\n"), unit));
            return SOC_E_PARAM;
        }
        pt_fifo_cfg = pt_cfg->pt_fifo;
        if (pt_fifo_cfg->num_elements <= 0 &&
            soc_sbx_caladan3_sws_tdm_id_current(unit) != TDMNULL) {
            soc_sbx_caladan3_pt_port_fifo_alloc(unit, instance, pt_fifo_cfg);
        }
        soc_sbx_caladan3_pt_port_fifo_set(unit, instance, pt_fifo_cfg);
    }


    /* hpte init */
    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_sws_hpte_init(unit));

    soc_sbx_caladan3_sws_pt_os_scheduler_config(unit);

    /* Initialize scheduler calendars */
    for (instance = 0;
            instance < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE ; 
               instance++) {

        pt_cfg = soc_sbx_caladan3_sws_pt_cfg_get(unit, instance);
        if (pt_cfg == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d pt_cfg_get failed\n"), unit));
            return SOC_E_PARAM;
        }
        soc_sbx_caladan3_sws_pt_port_calendar_config(unit, instance,
                                                     pt_cfg->pt_port_cal);
        soc_sbx_caladan3_sws_pt_client_calendar_config(unit, instance,
                                                       pt_cfg->pt_client_cal);
    }

    /* ipte init */
    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_sws_ipte_init(unit));

    /* Enable all ports */

    if (!SOC_IS_CALADAN3_REVB(unit)) {
        /* Enable all ports */
        soc_sbx_caladan3_sws_pt_port_enable_all(unit);
    } else {
        soc_sbx_caladan3_sws_pt_port_enable_all_rev_b(unit);
    }

    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_sws_pt_fc_init(unit));
   
    /* enable pt block */
    for (instance=0; 
             instance < SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE ; 
                 instance++) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_GEN_CONFIGr,
                                          PT_INSTANCE(instance), 0, &regval));
        soc_reg_field_set(unit, PT_GEN_CONFIGr, &regval, PT_ENABLEf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_GEN_CONFIGr,
                                          PT_INSTANCE(instance), 0, regval));
        if (SOC_RECONFIG_TDM) {
            break;
        }
    }

    /* enable egress shaping on PT0 */
    if (!soc_property_get(unit, spn_LRP_BYPASS, 0)) {
        /* enable egress shaping on PT0 */
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_QMAX_CONFIGr, PT0_INSTANCE, 0, &regval));
        soc_reg_field_set(unit, PT_IPTE_QMAX_CONFIGr, &regval, PT_IPTE_QMAX_REFRESH_CYCLE_SELf, 0);
        soc_reg_field_set(unit, PT_IPTE_QMAX_CONFIGr, &regval, PT_IPTE_QMAX_ENf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_QMAX_CONFIGr, PT0_INSTANCE, 0, regval));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *   soc_sbx_caladan3_sws_pt_fifo_auto_set
 * Purpose
 *   Setup port fifos when TDM is unknown
 */
int
soc_sbx_caladan3_sws_pt_fifo_auto_set(int unit, sws_config_t *sws_cfg)
{
    int rv = SOC_E_NONE;
    rv = soc_sbx_caladan3_pt_port_fifo_alloc(unit, 0, &sws_cfg->line_pt_cfg.pt_fifo);
    if (SOC_SUCCESS(rv)) {
        rv = soc_sbx_caladan3_pt_port_fifo_alloc(unit, 1, &sws_cfg->fabric_pt_cfg.pt_fifo);
    }
    return rv;
}

/*
 * Function:
 *   soc_sbx_caladan3_sws_pt_port_calendar_auto_set
 * Purpose
 *   Setup port calendar when TDM is unknown
 */
int
soc_sbx_caladan3_sws_pt_port_calendar_auto_set(int unit, sws_config_t *sws_cfg)
{
    /*
     * This autogenerates port calendar for basic functioning
     * To get line rate traffic, fine tunning might be required
     */
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_port_map_info_t *lpinfo, *fpinfo;
    int i;
    sws_pt_port_cal_t *lcal, *fcal;

    lcal = &sws_cfg->line_pt_cfg.pt_port_cal;
    fcal = &sws_cfg->fabric_pt_cfg.pt_port_cal;
    sal_memset(&lcal->port_id[0], -1, 
         SOC_SBX_CALADAN3_SWS_PT_MAX_PORT_CALENDAR * sizeof(lcal->port_id[0]));
    sal_memset(&fcal->port_id[0], -1, 
         SOC_SBX_CALADAN3_SWS_PT_MAX_PORT_CALENDAR * sizeof(fcal->port_id[0]));
    lcal->num_elements = fcal->num_elements = 0;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    for (i = 0; i < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; i++) {
        lpinfo = &port_map->line_port_info[i];
        fpinfo = &port_map->fabric_port_info[i];
        if (!(lpinfo->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        lcal->port_id[i] = lpinfo->base_port;
        fcal->port_id[i] = fpinfo->base_port;
    }
    lcal->num_elements = fcal->num_elements = i;
   
#ifdef PT_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "Slot  Port |  Slot Port\n")));
    for (i=0; i < SOC_SBX_CALADAN3_SWS_PT_MAX_PORT_CALENDAR; i++) {
        LOG_CLI((BSL_META_U(unit,
                            "%-3d  %2d |"), i, lcal->port_id[i]));
        LOG_CLI((BSL_META_U(unit,
                            "%-3d  %2d \n"), i, fcal->port_id[i]));
    }
    LOG_CLI((BSL_META_U(unit,
                        "Max  %d | Max %d\n"), lcal->num_elements, fcal->num_elements));
#endif

    return SOC_E_NONE;
}

/*
 * Function:
 *   soc_sbx_caladan3_sws_pt_client_calendar_auto_set
 * Purpose
 *   Setup client calendar when TDM is unknown
 */
int
soc_sbx_caladan3_sws_pt_client_calendar_auto_set(int unit, sws_config_t *sws_cfg)
{
    /*
     * This autogenerates client calendar for basic functioning
     * To get line rate traffic, fine tunning might be required
     */
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_port_map_info_t *lpinfo, *fpinfo __attribute__((unused));
    int i, port, max, speed, offset;
    sws_pt_client_cal_t *lcal, *fcal;
    int client = CLIENT0;
    int last = 0;

    lcal = &sws_cfg->line_pt_cfg.pt_client_cal;
    fcal = &sws_cfg->fabric_pt_cfg.pt_client_cal;
    lcal->num_elements = fcal->num_elements = 0;
    for (i=0; i < SOC_SBX_CALADAN3_SWS_PT_MAX_CLIENT_CALENDAR; i++) {
        lcal->client_id[i] = CLIENTX;
        fcal->client_id[i] = CLIENTX;
    }

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    max = port_map->max_line_bw + SOC_SBX_CALADAN3_NUM_RESV_PORTS;
    /* Fix control slots */
    offset = max / 3;
    lcal->client_id[offset] = CLIENT5;
    lcal->client_id[offset+8] = CLIENT5;
    lcal->client_id[2*offset] = CLIENT4;
    lcal->client_id[8+2*offset] = CLIENT4;
    lcal->client_id[3*offset] = CLIENT4;
    lcal->client_id[8+3*offset] = CLIENT4;
    LOG_CLI((BSL_META_U(unit,
                        "Max %d, Offset %d\n"), max, offset));

    for (port = 0; port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {
        lpinfo = &port_map->line_port_info[port];
        fpinfo = &port_map->fabric_port_info[port];
        if (!(lpinfo->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        speed = port_map->intf_attr[lpinfo->intftype].speed;
        offset = max / speed;
        LOG_CLI((BSL_META_U(unit,
                            "Intf type %d, Speed %d, Offset %d\n"), 
                 lpinfo->intftype, speed, offset));
        switch (lpinfo->intftype) {
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
                client = CLIENT0;
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
                client = CLIENT0;
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
                client = CLIENT0;
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
            case SOC_SBX_CALADAN3_PORT_INTF_IL100:
                client = CLIENT0;
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
                client = CLIENT0;
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
                client = CLIENT0;
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_XTPORT:
                if (lpinfo->instance == 0) {
                    client = CLIENT1;
                } else if (lpinfo->instance == 1) {
                    client = CLIENT2;
                } else if (lpinfo->instance == 2) {
                    client = CLIENT3;
                }
                break;
            default:
                continue ;
                ;
        }
        for (i=0; i < SOC_SBX_CALADAN3_SWS_PT_MAX_CLIENT_CALENDAR; i+=offset) {
            while (lcal->client_id[i] != CLIENTX) {
                i++;  /* Skip to free slot */
                if (i >= SOC_SBX_CALADAN3_SWS_PT_MAX_CLIENT_CALENDAR) {
                    break;
                }
            }
            if (i < SOC_SBX_CALADAN3_SWS_PT_MAX_CLIENT_CALENDAR) {
                lcal->client_id[i] = client;
                if (last < i) {
                    last = i;
                }
            }
        }

    }
    lcal->num_elements = last >> 1;
    client = CLIENT0;
    for (i=0; i < max; i++) {
        fcal->client_id[i] = client;
    }
    fcal->num_elements = i >> 1;
   
#ifdef PT_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "Slot  Client |  Slot Client\n")));
    for (i=0; i < SOC_SBX_CALADAN3_SWS_PT_MAX_CLIENT_CALENDAR; i++) {
        LOG_CLI((BSL_META_U(unit,
                            "%-3d  %2d |"), i, lcal->client_id[i]));
        LOG_CLI((BSL_META_U(unit,
                            "%-3d  %2d \n"), i, fcal->client_id[i]));
    }
    LOG_CLI((BSL_META_U(unit,
                        "Max  %d | Max %d\n"), lcal->num_elements, fcal->num_elements));
#endif

    return SOC_E_NONE;
}

#endif


