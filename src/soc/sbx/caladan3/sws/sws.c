/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: sws.c,v 1.7.16.11 Broadcom SDK $
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
#include <soc/sbx/caladan3/sws_params.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/sbx/caladan3/ped.h>


#define MAX_QUEUE_BLOCKS   ((SOC_SBX_CALADAN3_SWS_QM_MAX_QUEUES + 31)/ 32)

/* Max per PR instance*/
#define SOC_SBX_CALADAN3_PR_ICC_MAX_TABLE_ENTRIES 256
#define SOC_SBX_CALADAN3_PR_ICC_MAX_WORDS SOC_SBX_CALADAN3_PR_ICC_MAX_TABLE_ENTRIES/sizeof(uint32)


typedef struct soc_sbx_caladan3_sws_dbase_s {
    int cfg_valid;   
    char *current_tdm_name;
    tdmid_t  current_tdm_id;
    tdmid_t  lastused_tdm_id;
    soc_sbx_caladan3_sws_queue_info_t *queue_info;
    soc_sbx_caladan3_sws_pt_cfg_t  line_pt_cfg;
    soc_sbx_caladan3_sws_pt_cfg_t  fabric_pt_cfg;
    soc_sbx_caladan3_sws_pr_cfg_t  line_pr_cfg;
    soc_sbx_caladan3_sws_pr_cfg_t  fabric_pr_cfg;
    sws_qm_config_t *qm_cfg;
    sws_intf_config_t *intf_cfg;
    sws_config_t *current_sws_cfg;
    int icc_bypass[SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE];
    uint32 pr_icc_allocation_map[SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE][SOC_SBX_CALADAN3_PR_ICC_MAX_WORDS];
    soc_sbx_caladan3_sws_pr_policer_cfg_t pr_policer_cfg[SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE];
    uint32 queues_used[MAX_QUEUE_BLOCKS];
    /* Inbuild SWS Config, used when TDM header is missing or when CFG overrides header parameters */
    sws_config_t local_sws_cfg;
    /* Application specific queues */
    uint8 app_queues_num;
    uint8 app_queues_start;
    uint8 ing_redirect_qid0;
    uint8 ing_redirect_qid1;
    uint8 egr_redirect_qid0;
    uint8 egr_redirect_qid1;
    uint8 ing_bubble_qid;
    uint8 egr_bubble_qid;
    /* Explicitly specify the hpte queue map */
    soc_sbx_caladan3_hpte_map_entry_t ingress_queue_map[SOC_SBX_CALADAN3_SWS_MAX_HPTE_MAP_ENTRIES];
    soc_sbx_caladan3_hpte_map_entry_t egress_queue_map[SOC_SBX_CALADAN3_SWS_MAX_HPTE_MAP_ENTRIES];
} soc_sbx_caladan3_sws_dbase_t;


/****************************/
/*** Static Global States ***/
/****************************/
static soc_sbx_caladan3_sws_dbase_t *sws_dbase[SOC_MAX_NUM_DEVICES];
static tdmid_t lastused_id[SOC_MAX_NUM_DEVICES];

/* 
 * Interface configs
 *  These configs enable the TDM to work, but would not support Line rate traffic flow
 *  This facility allows user to load any TDM and not be blocked.
 *  Once the paramters are fine tuned, these can be added into a TDM header
 *  This feature is not meant to be used for line rate config.
 */
typedef struct sws_intf_data_s {
    sws_qm_source_queue_cfg_t  line_queue;
    sws_qm_source_queue_cfg_t  fabric_queue;
} sws_intf_data_t;
   
sws_intf_data_t soc_sbx_caladan3_sws_intf_data[] =
{
        /*  1G    */
        { 
           {64, 48, 32, 0, 17, 17},       /* Line Queue cfg */
           {271, 204, 136, 121, 17, 17},   /* Fab Queue cfg */
        },
        /*  10GE  */
        { 
           {204, 153, 102, 0, 52, 52},  /* Line Queue cfg */
           {1007, 756, 504, 227, 68, 68},  /* Fabric Queue cfg */
        },
        /*  40GE  */
        { 
           {457, 343, 229, 0, 34, 34},  /* Line Queue cfg */
           {638, 479, 319, 229, 49, 49},  /* Fabric Queue cfg */
        },
        /*  100GE */
        { 
           {1152, 864, 576, 0, 102, 102},  /* Line Queue cfg */
           {1152, 864, 576, 0, 102, 102},  /* Fabric Queue cfg */
        },
        /*  HG126 */
        { 
           {1395, 1047, 698, 1257, 102, 102},  /* Line Queue cfg */
           {1395, 1047, 698, 1257, 102, 102},  /* Fabric Queue cfg */
        },
        /*  CMIC         */
        { 
           {258, 194, 129, 0, 64, 64},  /* Line Queue cfg */
           {258, 194, 129, 0, 64, 64},  /* Fabric Queue cfg */
        },
        /* XLPORT        */
        { 
           {256, 192, 128, 0, 64, 64},  /* Line Queue cfg */
           {256, 192, 128, 0, 64, 64},  /* Fabric Queue cfg */
        },
        /* Special queues */
        {
           {128, 96, 64, 0, 0, 64},   
           {128, 96, 64, 0, 0, 64},
        }
};

/*
 * SWS Helpers
 */
int
soc_sbx_caladan3_sws_auto_set(int unit, sws_config_t *sws_cfg);

tdmid_t
soc_sbx_caladan3_sws_tdm_id_last(int unit)
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_queue_info_get: Invalid unit\n")));
        return -1;
    }
    return ((sws_dbase[unit]) ? (sws_dbase[unit]->lastused_tdm_id) : 0);
}


tdmid_t
soc_sbx_caladan3_sws_tdm_id_current(int unit)
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_tdm_id_current: Invalid unit\n")));
        return -1;
    }
    return ((sws_dbase[unit]) ? (sws_dbase[unit]->current_tdm_id) : 0);
}

soc_sbx_caladan3_sws_queue_info_t *
soc_sbx_caladan3_sws_queue_info_get(int unit) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_queue_info_get: Invalid unit\n")));
        return NULL;
    }
    return sws_dbase[unit]->queue_info;
}

sws_qm_config_t *
soc_sbx_caladan3_sws_qm_cfg_get(int unit) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_qm_cfg_get: Invalid unit\n")));
        return NULL;
    }
    if (sws_dbase[unit] != NULL) 
        return sws_dbase[unit]->qm_cfg;
    else
        return NULL;
}


int sws_pr_icc_table_entry_alloc_status(
    int unit, int pr_instance, int base_entry_id, int spread) 
{
    int entry_not_found = 0;
    int num_entries_found = 0;
    int sprd;
    pr_instance &= 1;

    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES) || 
        pr_instance >= SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE) {

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_pr_icc_table_entry_alloc_status: Invalid unit or pr_instance %d\n"),
                              pr_instance));
        return SOC_E_PARAM;
    }

    for (sprd = base_entry_id; sprd < (base_entry_id + spread); sprd++) {
        if (!SOC_SBX_C3_BMP_MEMBER(sws_dbase[unit]->pr_icc_allocation_map[pr_instance], sprd)) {
            entry_not_found++;
        } else {
            num_entries_found++;
        }
    }
 
    if (sprd == (base_entry_id + spread) && entry_not_found == 0) {
        return SOC_E_EMPTY;                  /* Empty "slot"; i.e. ALL entries available */
    } else if (num_entries_found == spread) {
        return SOC_E_FULL;                   /* No entries available in this slot*/
    } else if (entry_not_found > 0) {
        return SOC_E_EXISTS;                 /* Partially empty/full slot*/
    }

    return SOC_E_FAIL;
}


/* Allocates base pr_icc table entry with "spread" number of 
 * consecutive entries that are currently free, for use by caller.
 * If successful return then entry_start_idx is filled in and valid.
 * Returns: SOC_E_NONE upon successful return
 */
int sws_pr_icc_table_entry_alloc(
    int unit, int pr_instance, int spread, int *entry_start_idx) 
{
    int index;
    int sprd;
    int entries_avail = 0;

    pr_instance &= 1;
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES) || 
        pr_instance >= SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE ||
        !entry_start_idx) {

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_pr_icc_table_entry_alloc: Invalid PARAM unit %d or pr_instance %d\n"), 
                              unit, pr_instance));
        return SOC_E_PARAM;
    }

    for (index = 0; index < SOC_SBX_CALADAN3_PR_ICC_MAX_TABLE_ENTRIES; index += spread) {
        /* Check for spread consecutive entries available */
        for (sprd = index; sprd < (index + spread); sprd++) {
            if (SOC_SBX_C3_BMP_MEMBER(sws_dbase[unit]->pr_icc_allocation_map[pr_instance], sprd)) {
                break;
            }
        }
        if (sprd == (index + spread)) {
            entries_avail = 1;
            break;
        }
    }

    if (entries_avail) {
        *entry_start_idx = index;
        for (sprd = index; sprd < (index + spread); sprd++) {
            SOC_SBX_C3_BMP_ADD(sws_dbase[unit]->pr_icc_allocation_map[pr_instance],
                               sprd);
        }
        return SOC_E_NONE;
    }

    return SOC_E_UNAVAIL;
}


int sws_pr_icc_table_entry_free(
    int unit, int pr_instance, int base_entry_id, int spread) 
{
    int sprd;
    int entries_not_found = 0;

    pr_instance &= 1;
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES) ||
        pr_instance >= SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE) {

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_pr_icc_table_entry_free: Invalid unit %d or pr_instance %d\n"),
                              unit, pr_instance));
        return SOC_E_PARAM;
    }

    for (sprd = base_entry_id; sprd < (base_entry_id + spread); sprd++) {
        if (SOC_SBX_C3_BMP_MEMBER(sws_dbase[unit]->pr_icc_allocation_map[pr_instance], sprd)) {
            SOC_SBX_C3_BMP_REMOVE(sws_dbase[unit]->pr_icc_allocation_map[pr_instance], sprd);
        } else {
            entries_not_found = 1;
        }
    }
 
    if (sprd == (base_entry_id + spread)) {
        return SOC_E_NONE;
    }

    if (entries_not_found) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_pr_icc_table_entry_free: FREE attempt of entry not allocated. unit %d or pr_instance %d  base:%d  spread%d\n"),
                              unit, pr_instance, base_entry_id, spread));
    }

    return SOC_E_NOT_FOUND;
}


int
soc_sbx_caladan3_sws_pr_port_buffer_cfg_is_valid(int unit, int instance)
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_pr_port_buffer_cfg_is_valid: Invalid unit\n")));
        return 0;
    }
    if (instance) {
      return (sws_dbase[unit]->fabric_pr_cfg.pr_buf_cfg_valid);
    } else {
      return (sws_dbase[unit]->line_pr_cfg.pr_buf_cfg_valid);
    }
}

sws_pr_idp_thresholds_config_t *
soc_sbx_caladan3_sws_pr_idp_cfg_get(int unit, int instance) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_pr_idp_cfg_get: Invalid unit\n")));
        return NULL;
    }
    if (instance) {
        return sws_dbase[unit]->fabric_pr_cfg.pr_idp_cfg;
    } else {
        return sws_dbase[unit]->line_pr_cfg.pr_idp_cfg;
    }
}

sws_pr_port_buffer_t*
soc_sbx_caladan3_sws_pr_port_buffer_cfg_get(int unit, int instance) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_pr_port_buffer_cfg_get: Invalid unit\n")));
        return NULL;
    }
    if (instance) {
        return sws_dbase[unit]->fabric_pr_cfg.pr_buf_cfg;
    } else {
        return sws_dbase[unit]->line_pr_cfg.pr_buf_cfg;
    }
}

soc_sbx_caladan3_sws_pt_cfg_t *
soc_sbx_caladan3_sws_pt_cfg_get(int unit, int instance) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_pt_cfg_get: Invalid unit\n")));
        return NULL;
    }
    if (instance) {
        return &sws_dbase[unit]->fabric_pt_cfg;
    } else {
        return &sws_dbase[unit]->line_pt_cfg;
    }
}

soc_sbx_caladan3_sws_pr_policer_cfg_t *
soc_sbx_caladan3_sws_pr_policer_cfg_get(int unit, int instance) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_pr_policer_cfg_get: Invalid unit\n")));
        return NULL;
    }
    return &sws_dbase[unit]->pr_policer_cfg[instance];
}

int 
soc_sbx_caladan3_sws_bubble_queues_get(int unit, 
                                       int *ibq, int *ebq)
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_queue_info_get: Invalid unit\n")));
        return SOC_E_INIT;
    }
    if (sws_dbase[unit]) {
        if (ibq) { *ibq = sws_dbase[unit]->ing_bubble_qid; }
        if (ebq) { *ebq = sws_dbase[unit]->egr_bubble_qid; }
        return SOC_E_NONE;
    } 
    return SOC_E_INIT;
}

int 
soc_sbx_caladan3_sws_redirect_queues_get(int unit, 
                                         int *irq0, int *irq1, 
                                         int *erq0, int *erq1) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_queue_info_get: Invalid unit\n")));
        return SOC_E_INIT;
    }
    if (sws_dbase[unit]) {
        if (irq0) { *irq0 = sws_dbase[unit]->ing_redirect_qid0; }
        if (irq1) { *irq1 = sws_dbase[unit]->ing_redirect_qid1; }
        if (erq0) { *erq0 = sws_dbase[unit]->egr_redirect_qid0; }
        if (erq1) { *erq1 = sws_dbase[unit]->egr_redirect_qid1; }
        return SOC_E_NONE;
    } 
    return SOC_E_INIT;
}


/* 
 * config read helpers
 */
int
soc_sbx_caladan3_sws_config_param_data_get(int unit, char *str, int port, int defl)
{
    int value;
    char propstr[128] = {0};
    int tdmid;

    /*
     * Allows specifying per tdm property using the tdm number
     * The global value, if present is applied if per TDM data is not found
     */
    tdmid = soc_sbx_caladan3_sws_tdm_id_current(unit);
    if (port < 0) {
        sal_sprintf(propstr, "%s_tdm%d", str, tdmid);
    } else {
        sal_sprintf(propstr, "%s_tdm%d_%d", str, tdmid, port);
    }
    value = soc_property_get(unit, propstr, defl);
    if (value == defl) {
        if (port < 0) {
            sal_sprintf(propstr, "%s", str);
            value = soc_property_get(unit, str, defl);
        } else {
            sal_sprintf(propstr, "%s_%d", str, port);
            value = soc_property_get(unit, propstr, defl);
        }
    } else {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d Config param %s loaded as %d\n"),
                     unit, propstr, value));
    }
    return value;
}


char *
soc_sbx_caladan3_sws_config_param_str_get(int unit, char *str, int port)
{
    char *value;
    char propstr[128] = {0};
    int tdmid;

    /*
     * Allows specifying per tdm property using the tdm number
     * The global value, if present is applied if per TDM data is not found
     */
    tdmid = soc_sbx_caladan3_sws_tdm_id_current(unit);
    if (port < 0) {
        sal_sprintf(propstr, "%s_tdm%d", str, tdmid);
    } else {
        sal_sprintf(propstr, "%s_tdm%d_%d", str, tdmid, port);
    }
    value = soc_property_get_str(unit, propstr);
    if (!value) {
        if (port < 0) {
            sal_sprintf(propstr, "%s", str);
            value = soc_property_get_str(unit, str);
        } else {
            sal_sprintf(propstr, "%s_%d", str, port);
            value = soc_property_get_str(unit, str);
        }
    } else {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d Config param %s loaded as %s\n"),
                     unit, propstr, value));
    }
    return value;
}



/* 
 * Queue Allocation
 *   The entire range of 256 queues are managed directly, there is no Line/Fab/Sq/Dq difference here
 */
int
soc_sbx_caladan3_sws_queue_alloc(int unit, int start_queue, int num_queues)
{
     int index = 0;
     int start = 0;
     uint32 m = 0, mask = 0, mask1 = 0;

     LOG_VERBOSE(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d caladan3_sws_queue_alloc: Allocating Start %d(%2x) Num Queues %d \n"),
                  unit, start_queue, start_queue, num_queues));
     if ((num_queues <= 0) || (num_queues > 64)) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "Unit %d caladan3_sws_queue invalid num_queues param\n"),
                    unit));
         return SOC_E_PARAM;
     }
     if ((start_queue < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) ||
          (start_queue > SOC_SBX_CALADAN3_SWS_QM_MAX_QUEUES)) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "Unit %d caladan3_sws_queue_alloc invalid num_queues param\n"),
                    unit));
         return SOC_E_PARAM;
     }
     start = (start_queue % 32); 
     index = (start_queue >> 5); 
     if (index >= MAX_QUEUE_BLOCKS) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "Unit %d caladan3_sws_queue_alloc queues"
                               " in range %d-%d invalid \n"),
                    unit, start_queue, start_queue + num_queues - 1));
         return SOC_E_PARAM;
     }
     if ((num_queues > 32) && (index & 1)) {
         LOG_VERBOSE(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Unit %d caladan3_sws_queue_alloc queues"
                                 " in range %d-%d overflows region\n"),
                      unit, start_queue, start_queue + num_queues - 1));
         return SOC_E_PARAM;
     }
     /* Check allocation */
     if (num_queues >= 32) {
         m = 0xffffffff;
         mask = m << start;
         mask1 = m >> (32 - start);
     } else {
         m = (1 << num_queues) - 1;
         mask = m << start;
         if ((start + num_queues) > 32) {
             mask1 = m >> (32 - start);
         } else {
             mask1 = 0;
         }
     }

     if (sws_dbase[unit]->queues_used[index] & mask) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "Unit %d caladan3_sws_queue_alloc queues"
                               " in range %d-%d already used\n"),
                    unit, start_queue, start_queue + num_queues - 1));
         return SOC_E_RESOURCE;
     } else {
         sws_dbase[unit]->queues_used[index] |= mask;
     }

     if ((index <= 6) && (mask1 > 0)) {
         if (sws_dbase[unit]->queues_used[index+1] & mask1) {
             LOG_ERROR(BSL_LS_SOC_COMMON,
                       (BSL_META_U(unit,
                                   "Unit %d caladan3_sws_queue_alloc queues"
                                   " in range %d-%d already used\n"),
                        unit, start_queue, start_queue + num_queues - 1));
             return SOC_E_RESOURCE;
         } else {
             sws_dbase[unit]->queues_used[index+1] |= mask;
         }
     }

     return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_queue_free(int unit, int start_queue, int num_queues)
{
    int index = 0;
    int start = 0;
    uint32 m = 0, mask = 0, mask1 = 0;

    if ((num_queues <= 0) || (num_queues > 64)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d caladan3_sws_queue invalid num_queues param\n"),
                   unit));
        return SOC_E_PARAM;
    }
    if ((start_queue < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) ||
         (start_queue > SOC_SBX_CALADAN3_SWS_QM_MAX_QUEUES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d caladan3_sws_queue invalid num_queues param\n"),
                   unit));
        return SOC_E_PARAM;
    }
    start = (start_queue % 32); 
    index = (start_queue >> 5); 
    if (index >= MAX_QUEUE_BLOCKS) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d caladan3_sws_queue queues"
                              " in range %d-%d invalid \n"),
                   unit, start_queue, start_queue + num_queues - 1));
        return SOC_E_PARAM;
    }
    if ((num_queues > 32) && (index & 1)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d caladan3_sws_queue queues"
                              " in range %d-%d overflows region\n"),
                   unit, start_queue, start_queue + num_queues - 1));
        return SOC_E_PARAM;
    }
    /* Check allocation */
    if (num_queues >= 32) {
         m = 0xffffffff;
         mask = m << start;
         mask1 = m >> (32 - start);
    } else {
         m = (1 << num_queues) - 1;
         mask = m << start;
         if ((start + num_queues) > 32) {
             mask1 = m >> (32 - start);
         } else {
             mask1 = 0;
         }
    }

    if ((sws_dbase[unit]->queues_used[index] & mask) == 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d caladan3_sws_queue_free queues"
                              " in range %d-%d already free\n"),
                   unit, start_queue, start_queue + num_queues - 1));
        return SOC_E_RESOURCE;
    } else {
        sws_dbase[unit]->queues_used[index] &= ~mask;
    }
    if ((index <= 6) && (mask1 > 0)) {
        if ((sws_dbase[unit]->queues_used[index+1] & mask1) == 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d caladan3_sws_queue_free queues"
                                  " in range %d-%d already free\n"),
                       unit, start_queue, start_queue + num_queues - 1));
            return SOC_E_RESOURCE;
        } else {
            sws_dbase[unit]->queues_used[index+1] &= ~mask1;
        }
    }
    return SOC_E_NONE;
}

/* 
 * Find a range of consecutive unused queues on either ingress/egress sq/dq
 */
int
soc_sbx_caladan3_sws_find_queue_range(int unit, int range, int type, int *start_queue) 
{
    uint32 mask;
    int index = 0, start = 0;
    int max, wrapidx, i;
    int found = 0;
    uint32 m1, m2;

    if ((range < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) ||
          (range > 31)) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "Unit %d caladan3_sws_find_queue_range invalid range param\n"),
                    unit));
         return SOC_E_PARAM;
    }
    if (!start_queue) {
         return SOC_E_PARAM;
    }

    switch (type) {
    case INGRESS_SQUEUE:
        start = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE;
        break;
    case  EGRESS_SQUEUE:
        start = SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE;
        break;
    case  EGRESS_DQUEUE:
        start = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE;
        break;
    case INGRESS_DQUEUE:
        start = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE;
        break;
    default:
        return SOC_E_PARAM;
    }

    index = start >> 5;
    max = SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION - range;    
    wrapidx = range;
    mask = ((1 << range) -1);
    found = 0;
    for (i=0; i < max; i++, wrapidx++) {
        LOG_CLI((BSL_META_U(unit,
                            "\n i %d wrapidx %d mask %x "), i, wrapidx, mask));
        if (wrapidx <= 32) {
            /* coverity [large_shift] */
            m1 = (mask << i);
            LOG_CLI((BSL_META_U(unit,
                                "\n m1 %x "), m1));
            if ((sws_dbase[unit]->queues_used[index] & m1) == 0) {
                found = 1;
                break;
            }
        } else if (i < 32) {
            m1 = ((mask & ((1 << (32 - i)) -1)) << i); /* Avoid overflow */
            /* coverity [large_shift] */
            m2 = (mask >> (32-i));
            LOG_CLI((BSL_META_U(unit,
                                "\n m1 %x m2 %x"), m1, m2));
            if (((sws_dbase[unit]->queues_used[index] & m1) == 0) &&
                    (sws_dbase[unit]->queues_used[index+1] & m2) == 0) {
                found = 1;
                break;
            }
        } else {
            m1 = mask << (i - 32);
            LOG_CLI((BSL_META_U(unit,
                                "\n m1 %x "), m1));
            if ((sws_dbase[unit]->queues_used[index+1] & m1) == 0) {
                found = 1;
                break;
            }
        }
    }
    if (found) {
        *start_queue = start + i;
        return SOC_E_NONE;
    } else {
        *start_queue = -1;
        return SOC_E_NOT_FOUND;
    }
}

int
soc_sbx_caladan3_sws_uport_to_line_port(int unit, int uport, int *index)
{
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    int idx, found = 0;

    if (index) {
        *index = -1;
    }
    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    for (idx = 0; idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {
        if (!(port_map->line_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        if (port_map->line_port_info[idx].uport == uport) {
            found = 1;
            break;
        }
    }
    if (!found) {
        return SOC_E_NOT_FOUND;
    }
    if (index) {
        *index = idx;
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_uport_to_fab_port(int unit, int uport, int *index)
{
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    int idx, found = 0;

    if (index) {
        *index = -1;
    }
    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    for (idx = 0; idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {
        if (!(port_map->fabric_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        if (port_map->fabric_port_info[idx].uport == uport) {
            found = 1;
            break;
        }
    }
    if (!found) {
        return SOC_E_NOT_FOUND;
    }
    if (index) {
        *index = idx;
    }
    return SOC_E_NONE;
}


int
soc_sbx_caladan3_sws_pbmp_to_line_port(int unit, int port, int *index)
{
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    int idx;

    if (index) {
        *index = -1;
    }
    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    idx = port_map->pbmp2idx[port].portidx;

    if (!port_map->pbmp2idx[port].line) {
        return SOC_E_NOT_FOUND;
    }
    if (index) {
        *index = idx;
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_pbmp_to_fab_port(int unit, int port, int *index)
{
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    int idx;

    if (index) {
        *index = -1;
    }
    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    idx = port_map->pbmp2idx[port].portidx;
    if (port_map->pbmp2idx[port].line) {
        return SOC_E_NOT_FOUND;
    }

    if (index) {
        *index = idx;
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_get_port_from_squeue(int unit, int queue, int *port)
{
    if (!port) return SOC_E_PARAM;
    if ((queue > SOC_SBX_CALADAN3_SWS_MAX_QUEUE_ID) ||
        (queue < 0)) {
        return SOC_E_PARAM;
    }
    if (sws_dbase[unit] && sws_dbase[unit]->queue_info) {
        *port = sws_dbase[unit]->queue_info[queue].port;
        return SOC_E_NONE;
    }
    return SOC_E_NOT_FOUND;
}

int 
soc_sbx_caladan3_get_dqueue_from_port(int unit, int uport, int direction, 
                                      int cos, int *dqueue)
{
    soc_sbx_caladan3_port_map_info_t *port_info;
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_queues_t *qi;
    int qid, index = 0;
    int status = 0;
    
    if (!dqueue) return SOC_E_PARAM;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    if (soc_sbx_caladan3_is_line_port(unit, uport)) {
#if 0
        status = soc_sbx_caladan3_sws_uport_to_line_port(unit, uport, &index);
#else
        status = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, uport, &index);
#endif
        if (SOC_FAILURE(status)) {
            return status;
        }
    } else {
        /* Failsafe point to index 0 on fab side */
        index = 0;
    }

    if (direction) {
        port_info = &port_map->fabric_port_info[index];
    } else {
       port_info = &port_map->line_port_info[index];
    }
    qi = &port_info->port_queues;
    status = SOC_E_NOT_FOUND;
    SOC_SBX_C3_BMP_ITER(qi->dqueue_bmp, qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {
        if ((qi->num_dqueue > 0) && (cos > 0)) {
            /* Note:
             *   Traditionally queues were bunched, so qid  = base+n was the qid for nth cos 
             *   With non-contigous queues this is not true, so we make an assumption here
             *   that if qid for nth cos is requested, and there are n (non-contig) queues
             *   qid = nth queue, else we return the first queue in the set
             */
            cos--;
            continue;
        }
        if (direction) {
            *dqueue = qid + SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE;
        } else {
            *dqueue = qid + SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE;
        }
        status = SOC_E_NONE;
        break;
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_get_squeue_from_port(int unit, int uport, int direction, 
                                      int cos, int *squeue)
{
    soc_sbx_caladan3_port_map_info_t *port_info;
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_queues_t *qi;
    int qid, index = 0;
    int status = 0;

    
    if (!squeue) return SOC_E_PARAM;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    if (soc_sbx_caladan3_is_line_port(unit, uport)) {
        status = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, uport, &index);
        if (SOC_FAILURE(status)) {
            return status;
        }
    } else {
        /* Failsafe point to index 0 on fab side */
        index = 0;

    }

    if (direction) {
        port_info = &port_map->fabric_port_info[index];
    } else {
       port_info = &port_map->line_port_info[index];
    }
    qi = &port_info->port_queues;
    status = SOC_E_NOT_FOUND;
    SOC_SBX_C3_BMP_ITER(qi->squeue_bmp, qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
        if ((qi->num_squeue > 0) && (cos > 0)) {
            /* Note:
             *   Traditionally queues were bunched, so qid  = base+n was the qid for nth cos 
             *   With non-contigous queues this is not true, so we make an assumption here
             *   that if qid for nth cos is requested, and there are n (non-contig) queues
             *   qid = nth queue, else we return the first queue in the set
             */
            cos--;
            continue;
        }
        if (direction) {
            *squeue = qid + SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE;
        } else {
            *squeue = qid + SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE;
        }
        status = SOC_E_NONE;
        break;
    }
    return status;
}

int
soc_sbx_caladan3_get_queues_from_port(int unit, int port, int *sq, int*dq, int *num_cos)
{
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_queues_t *qi = NULL;
    int status = SOC_E_NONE;
    int index = -1;

    if (!dq || !sq) return SOC_E_PARAM;
    
    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    
    if (soc_sbx_caladan3_is_line_port(unit, port)) {
        status = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, port, &index);
        if (SOC_FAILURE(status)) {
            return status;
        }
        qi = &port_map->line_port_info[index].port_queues;
    } else {
        status = soc_sbx_caladan3_sws_pbmp_to_fab_port(unit, port, &index);
        if (SOC_FAILURE(status)) {
            return status;
        }
        qi = &port_map->fabric_port_info[index].port_queues;
    }
    *sq = qi->squeue_base;
    *dq = qi->dqueue_base;
    *num_cos = qi->num_squeue;
    return SOC_E_NONE;
}


/* this doesn't take a pbmp port.  It takes a fabric port index */
int
soc_sbx_caladan3_get_queues_from_fabric_port_info(int unit, int fpidx, int *sq, int*dq, int *num_cos, soc_port_t *port)
{
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_queues_t *qi = NULL;

    if (!dq || !sq || !num_cos || !port) return SOC_E_PARAM;
    
    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    
    qi = &port_map->fabric_port_info[fpidx].port_queues;
    *sq = qi->squeue_base;
    *dq = qi->dqueue_base;
    *num_cos = qi->num_squeue;
    *port = port_map->fabric_port_info[fpidx].port;

    if ((*sq ==0) && (*dq == 0)) {
        return SOC_E_RESOURCE;
    }

    return SOC_E_NONE;
}

/*
 * Function
 *     soc_sbx_caladan3_sws_is_tdm_reusable
 * Purpose
 *     Determine if the last used TDM allows us to reuse paramteers
 *     Returns TRUE/FALSE
 */
int
soc_sbx_caladan3_sws_is_tdm_reusable(int unit)
{

    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_tdm_select: Invalid unit\n")));
        return FALSE;
    }

    if (sws_dbase[unit]->cfg_valid) {
        /* This comparision is only w.r.to sws data, Individual block reset
           needs to be considered to get the holistic idea of hotswappablity */
        if (sws_dbase[unit]->current_tdm_id == sws_dbase[unit]->lastused_tdm_id) {
            return TRUE;
        }
        return (soc_sbx_caladan3_sws_tdm_is_swapable(unit));
    }

    /* Config loaded outside of database, cannot determine compatiblity */
    return FALSE;

}


int
soc_sbx_caladan3_sws_tdm_select(int unit, int tdmid) 
{

    int rv = SOC_E_NONE;
    soc_sbx_caladan3_sws_pt_cfg_t *ptcfg = NULL;
    soc_sbx_caladan3_sws_pr_cfg_t *prcfg = NULL;
    tdm_identifier_t *id;

    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_tdm_select: Invalid unit\n")));
        return SOC_E_INIT;
    }

    if (sws_dbase[unit]->current_sws_cfg) {
        sws_dbase[unit]->cfg_valid = TRUE;
    } else {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d Missing Parameters for TDM (id %d)\n"),
                  unit, tdmid ));
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d Applying Dummy Parameters for TDM (id %d)\n"),
                  unit, tdmid ));
        rv = soc_sbx_caladan3_sws_auto_set(unit, &sws_dbase[unit]->local_sws_cfg);
        if (SOC_FAILURE(rv)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Unit %d sws_tdm_select: Autoload for TDM (id %d) failed(%d)\n"),
                      unit, tdmid, rv));
            return SOC_E_PARAM;
        }
        sws_dbase[unit]->cfg_valid = FALSE;
        sws_dbase[unit]->current_sws_cfg = &sws_dbase[unit]->local_sws_cfg;
    }
    sws_dbase[unit]->qm_cfg = &sws_dbase[unit]->current_sws_cfg->qm_cfg;
    sws_dbase[unit]->intf_cfg = &sws_dbase[unit]->current_sws_cfg->intf_cfg;
    ptcfg = &sws_dbase[unit]->line_pt_cfg;
    ptcfg->pt_fifo = 
             &sws_dbase[unit]->current_sws_cfg->line_pt_cfg.pt_fifo;
    ptcfg->pt_port_cal = 
             &sws_dbase[unit]->current_sws_cfg->line_pt_cfg.pt_port_cal;
    ptcfg->pt_client_cal = 
             &sws_dbase[unit]->current_sws_cfg->line_pt_cfg.pt_client_cal;

    ptcfg = &sws_dbase[unit]->fabric_pt_cfg;
    ptcfg->pt_fifo = 
             &sws_dbase[unit]->current_sws_cfg->fabric_pt_cfg.pt_fifo;
    ptcfg->pt_port_cal = 
             &sws_dbase[unit]->current_sws_cfg->fabric_pt_cfg.pt_port_cal;
    ptcfg->pt_client_cal = 
             &sws_dbase[unit]->current_sws_cfg->fabric_pt_cfg.pt_client_cal;

    prcfg = &sws_dbase[unit]->line_pr_cfg;
    prcfg->pr_buf_cfg = 
             &sws_dbase[unit]->current_sws_cfg->line_pr_cfg.rx_buffer_cfg;
    prcfg->pr_buf_cfg_valid =
             !(prcfg->pr_buf_cfg->num_elements == 0);
    prcfg->pr_idp_cfg =
             &sws_dbase[unit]->current_sws_cfg->line_pr_cfg.pr_idp_cfg;

    prcfg = &sws_dbase[unit]->fabric_pr_cfg;
    prcfg->pr_buf_cfg = 
             &sws_dbase[unit]->current_sws_cfg->fabric_pr_cfg.rx_buffer_cfg;
    prcfg->pr_buf_cfg_valid =
             !(prcfg->pr_buf_cfg->num_elements == 0);
    prcfg->pr_idp_cfg =
             &sws_dbase[unit]->current_sws_cfg->fabric_pr_cfg.pr_idp_cfg;

    sws_dbase[unit]->current_tdm_id = (tdmid_t)tdmid;
    sws_dbase[unit]->lastused_tdm_id = lastused_id[unit];
    lastused_id[unit] = sws_dbase[unit]->current_tdm_id;
    id = soc_sbx_caladan3_tdm_identifier_get(unit, tdmid);
    if (id)  {
        sws_dbase[unit]->current_tdm_name = id->name;
        LOG_CLI((BSL_META_U(unit,
                            "Unit %d Loading TDM %s\n"), unit, sws_dbase[unit]->current_tdm_name));
    } else {
        LOG_CLI((BSL_META_U(unit,
                            "Unit %d Loading TDM %d\n"), unit, tdmid));
    }

    return SOC_E_NONE;
}


int
soc_sbx_caladan3_sws_check_icc_state(int unit, int pr_instance) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_check_icc_state: Invalid unit\n")));
        return -1;
    }
    if ((pr_instance != PR0_INSTANCE) && (pr_instance != PR1_INSTANCE)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_check_icc_state: Invalid instance:%d\n"), pr_instance));
        return -1;
    }
    if (pr_instance == PR1_INSTANCE) {
        pr_instance = 1;
    } else {
        pr_instance = 0;
    }
    return sws_dbase[unit]->icc_bypass[pr_instance & 0xF];
}

void
soc_sbx_caladan3_sws_set_icc_state(int unit, int pr_instance, int state) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_set_icc_state: Invalid unit\n")));
        return;
    }
    if ((pr_instance != PR0_INSTANCE) && (pr_instance != PR1_INSTANCE)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_set_icc_state: Invalid instance:%d\n"), pr_instance));
        return;
    }
    if ((state != SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE) && (state != SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_set_icc_state: Invalid state:%d\n"), state));
        return;
    }
    if (pr_instance == PR1_INSTANCE) {
        pr_instance = 1;
    } else {
        pr_instance = 0;
    }
    sws_dbase[unit]->icc_bypass[pr_instance & 0xF] = state;
}

/*
 * Function: soc_sbx_caladan3_sws_queue_property_get
 * 
 * Purpose: Get the queue parameters from the cfg file 
 *        Format: 
 *            config_queue{N}_{tdm}={max_pages},{de1},{de2},{fc_treshold},{min_data_pages}{min_hdr_pages}
 *            config_queue{N}={max_pages},{de1},{de2},{fc_treshold},{min_data_pages}{min_hdr_pages}
 *        Example:
 *            config_queue64_tdm35=1007,756,504,227,68,68
 *            config_queue64=1007,756,504,227,68,68
 */
int
soc_sbx_caladan3_sws_queue_property_get(int unit, int qid, int *queue_cfg)
{

    int cnt = 0;
    char buffer[128] = {0};
    int tdmid = 0;
    int max = sizeof(sws_qm_source_queue_cfg_t)/sizeof(int);

    tdmid = soc_sbx_caladan3_sws_tdm_id_current(unit);
    sal_snprintf(buffer, 128, "%s%d_tdm%d", spn_CONFIG_QUEUE, qid, tdmid);
    cnt = soc_property_get_csv(unit, buffer, max, queue_cfg);
    if (cnt == 0) {
        sal_snprintf(buffer, 128, "%s%d", spn_CONFIG_QUEUE, qid);
        cnt = soc_property_get_csv(unit, buffer, max, queue_cfg);
    }
    if (cnt) {
        if (cnt == max) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Unit %d Property %s for Qid %d loaded \n"),
                         unit, buffer, qid));
            return SOC_E_NONE;
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Warning: Property %s for Qid %d incomplete, ingored(%d) values\n"),
                       unit, buffer, qid, cnt));
        }
    }
    return SOC_E_NOT_FOUND;
}

/*
 * Function:
 *    soc_sbx_caladan3_set_squeue_intf_info
 * Purpose
 *    Copy perqueue info from tdm database inteface info
 */
int
soc_sbx_caladan3_set_squeue_intf_info(int unit, 
                                      sws_qm_source_queue_cfg_t **queue_cfg,
                                      int line,
                                      soc_sbx_caladan3_port_map_info_t *portinfo,
                                      int squeue)
{
    /* See if the TDM params have interface level data, if so use it */
    sws_qm_source_queue_cfg_t *qcfg;
    if (sws_dbase[unit]->intf_cfg) {
        if (line) {
            qcfg = &sws_dbase[unit]->intf_cfg->intf[portinfo->intftype].lqcfg;
        } else {
            qcfg = &sws_dbase[unit]->intf_cfg->intf[portinfo->intftype].fqcfg;
        }
        if (qcfg->max_pages) {
            *queue_cfg = qcfg;
            return SOC_E_NONE;
        }
    }
    return SOC_E_NOT_FOUND;
}


/*
 * Load dummy parameters for unknown TDM
 * - Use this to enable the TDM, fine tune and add to registered.
 */
int
soc_sbx_caladan3_queue_info_auto_get_resvd(int unit, 
                                          sws_qm_source_queue_cfg_t **qmcfg)
{
    *qmcfg = &(soc_sbx_caladan3_sws_intf_data[7].line_queue);
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_queue_info_auto_set_intf(int unit,
                                          soc_sbx_caladan3_port_map_t *port_map,
                                          sws_config_t *sws_cfg,
                                          sws_intf_data_t *intf_data)
{
    int idx, num_cos __attribute__((unused));
    soc_sbx_caladan3_port_map_info_t *line_info, *fabric_info;
    soc_sbx_caladan3_queues_t *line_qi, *fabric_qi;
    sws_qm_source_queue_cfg_t *line_qcfg, *fabric_qcfg, *qmcfg;
    sws_qm_buffer_cfg_t *buf_cfg;
    int squeue;
    int skip = 0;

    if (!sws_cfg || !intf_data || !port_map) {
        return SOC_E_PARAM;
    }

    buf_cfg = &sws_cfg->qm_cfg.buffer;
    for (idx = 0; idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {
        qmcfg = NULL;
        /* Setup Line side */
        line_info = &port_map->line_port_info[idx];
        if (!(line_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        line_qi = &line_info->port_queues;
        num_cos = (line_qi->num_squeue>0) ? line_qi->num_squeue : 1;
        SOC_SBX_C3_BMP_ITER(line_qi->squeue_bmp, squeue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
            skip = 0;
            line_qcfg = &sws_cfg->qm_cfg.line_queues[squeue];
            switch (line_info->intftype) {
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
            case SOC_SBX_CALADAN3_PORT_INTF_XTPORT:
                qmcfg = &(intf_data[0].line_queue);
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE:
                qmcfg = &(intf_data[1].line_queue);
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
                qmcfg = &(intf_data[2].line_queue);
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
                qmcfg = &(intf_data[3].line_queue);
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
                if (line_qi->num_squeue > 1) {
                    qmcfg = &(intf_data[0].line_queue);
                } else {
                    qmcfg = &(intf_data[1].line_queue);
                }
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                if (line_qi->num_squeue > 1) {
                    qmcfg = &(intf_data[0].line_queue);
                } else {
                    qmcfg = &(intf_data[2].line_queue);
                }
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
                if (line_qi->num_squeue > 1) {
                    qmcfg = &(intf_data[0].line_queue);
                } else {
                    qmcfg = &(intf_data[4].line_queue);
                }
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
            case SOC_SBX_CALADAN3_PORT_INTF_IL100:
                qmcfg = &(intf_data[3].line_queue);
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CMIC:
                qmcfg = &(intf_data[5].line_queue);
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_XLPORT:
                qmcfg = &(intf_data[6].line_queue);
                if ((line_info->bindex & 1) && 
                        (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2)) {
                    skip = 1;
                }

                break;
            default:
                continue;
            }
            if (qmcfg) {
                sal_memcpy(line_qcfg, qmcfg, sizeof(sws_qm_source_queue_cfg_t));
                if (!skip) {
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "Unit %d, Port %2d Queue %2d Max pages = %6d "),
                                 unit, line_info->port, squeue, qmcfg->max_pages));
                    buf_cfg->ingress_max_pages += qmcfg->max_pages;
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "Ingress Max = %6d\n"), buf_cfg->ingress_max_pages));
                }
            }
        } 

        /* Setup Fabric side */
        qmcfg = NULL;
        fabric_info = &port_map->fabric_port_info[idx];
        fabric_qi = &fabric_info->port_queues;
        num_cos = (fabric_qi->num_squeue>0) ? fabric_qi->num_squeue : 1;
        /* Some ports are not expected to be configured on the fabric, 
         * but support them anyway, using line side config
         */
        SOC_SBX_C3_BMP_ITER(fabric_qi->squeue_bmp, squeue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
            skip = 0;
            fabric_qcfg = &sws_cfg->qm_cfg.fabric_queues[squeue];
            switch (fabric_info->intftype) {
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
            case SOC_SBX_CALADAN3_PORT_INTF_XTPORT:
                qmcfg = &(intf_data[0].line_queue);
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE:
                qmcfg = &(intf_data[1].line_queue);
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
                qmcfg = &(intf_data[2].line_queue);
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
                qmcfg = &(intf_data[3].line_queue);
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
                if (fabric_qi->num_squeue > 1) {
                    qmcfg = &(intf_data[0].fabric_queue);
                } else {
                    qmcfg = &(intf_data[1].fabric_queue);
                }
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                if (line_qi->num_squeue > 1) {
                    qmcfg = &(intf_data[0].fabric_queue);
                } else {
                    qmcfg = &(intf_data[2].fabric_queue);
                }
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
            case SOC_SBX_CALADAN3_PORT_INTF_IL100:
                /* Typical Fab Ports */
                switch (line_info->intftype) {
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
                case SOC_SBX_CALADAN3_PORT_INTF_XTPORT:
                    qmcfg = &(intf_data[0].fabric_queue);
                    break;
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE:
                    qmcfg = &(intf_data[1].fabric_queue);
                    break;
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
                    qmcfg = &(intf_data[2].fabric_queue);
                    break;
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
                    if (SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126 == fabric_info->intftype) {
                        qmcfg = &(intf_data[4].fabric_queue);
                    } else {
                        qmcfg = &(intf_data[3].fabric_queue);
                    }
                    break;
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
                    if (fabric_qi->num_squeue > 1) {
                        qmcfg = &(intf_data[0].fabric_queue);
                    } else {
                        qmcfg = &(intf_data[1].fabric_queue);
                    }
                    break;
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                    if (fabric_qi->num_squeue > 1) {
                        qmcfg = &(intf_data[0].fabric_queue);
                    } else {
                        qmcfg = &(intf_data[2].fabric_queue);
                    }
                    break;
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
                case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
                case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
                case SOC_SBX_CALADAN3_PORT_INTF_IL100:
                    if (fabric_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126) {
                        qmcfg = &(intf_data[4].fabric_queue);
                    } else {
                        qmcfg = &(intf_data[3].fabric_queue);
                    }
                    break;
                case SOC_SBX_CALADAN3_PORT_INTF_CMIC:
                    qmcfg = &(intf_data[5].fabric_queue);
                    break;
                case SOC_SBX_CALADAN3_PORT_INTF_XLPORT:
                    qmcfg = &(intf_data[6].fabric_queue);
                    if ((line_info->bindex & 1) && 
                            (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2)) {
                        skip = 1;
                    }
                    break;
                default:
                    continue;
                }
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CMIC:
                qmcfg = &(intf_data[5].line_queue);
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_XLPORT:
                qmcfg = &(intf_data[6].line_queue);
                break;
            default:
                continue;
            }
            if (qmcfg) {
                sal_memcpy(fabric_qcfg, qmcfg, sizeof(sws_qm_source_queue_cfg_t));
                if (!skip) {
                    buf_cfg->egress_max_pages += qmcfg->max_pages;
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "Unit %d, Port %2d Queue %2d Max pages = %6d "),
                                 unit, line_info->port, squeue, qmcfg->max_pages));
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            " Egress Max = %6d\n"),
                                 buf_cfg->egress_max_pages));
               }
            }
        }
    }

    buf_cfg->num_pages_reserved = SOC_SBX_CALADAN3_SWS_RESERVED_PAGES;
    buf_cfg->total_buff_max_pages = buf_cfg->ingress_max_pages + buf_cfg->egress_max_pages;

    if (buf_cfg->total_buff_max_pages > SOC_SBX_CALADAN3_SWS_MAX_PAGES) {
        buf_cfg->total_buff_max_pages = SOC_SBX_CALADAN3_SWS_MAX_PAGES - buf_cfg->num_pages_reserved;
    }
    buf_cfg->total_buff_drop_thres_de1 = buf_cfg->total_buff_max_pages *
                                             SOC_SBX_CALADAN3_SWS_DE1_TRESHOLD_PERCENT / 100;
    buf_cfg->total_buff_drop_thres_de2 = buf_cfg->total_buff_max_pages *
                                             SOC_SBX_CALADAN3_SWS_DE2_TRESHOLD_PERCENT / 100;
    buf_cfg->fc_total_buffer_xoff_thresh = buf_cfg->total_buff_max_pages *
                                             SOC_SBX_CALADAN3_SWS_FC_TRESHOLD_PERCENT / 100;
    buf_cfg->total_buff_hysteresis_delta = buf_cfg->total_buff_drop_thres_de1 *
                                              SOC_SBX_CALADAN3_SWS_GLOBAL_HYST_DELTA_PERCENT / 100;
    buf_cfg->ingress_drop_thres_de1 = buf_cfg->ingress_max_pages *
                                             SOC_SBX_CALADAN3_SWS_DE1_TRESHOLD_PERCENT / 100;
    buf_cfg->ingress_drop_thres_de2 = buf_cfg->ingress_max_pages *
                                             SOC_SBX_CALADAN3_SWS_DE2_TRESHOLD_PERCENT / 100;
    buf_cfg->fc_ingress_xoff_thresh = buf_cfg->ingress_max_pages *
                                             SOC_SBX_CALADAN3_SWS_FC_TRESHOLD_PERCENT / 100;
    buf_cfg->ingress_hysteresis_delta = buf_cfg->ingress_drop_thres_de1 *
                                             SOC_SBX_CALADAN3_SWS_INGRESS_HYST_DELTA_PERCENT / 100;
    buf_cfg->egress_drop_thres_de1 = buf_cfg->egress_max_pages *
                                             SOC_SBX_CALADAN3_SWS_DE1_TRESHOLD_PERCENT / 100;
    buf_cfg->egress_drop_thres_de2 = buf_cfg->egress_max_pages *
                                             SOC_SBX_CALADAN3_SWS_DE2_TRESHOLD_PERCENT / 100;
    buf_cfg->fc_egress_xoff_thresh = buf_cfg->egress_max_pages *
                                             SOC_SBX_CALADAN3_SWS_FC_TRESHOLD_PERCENT / 100;
    buf_cfg->egress_hysteresis_delta = buf_cfg->egress_drop_thres_de1 *
                                              SOC_SBX_CALADAN3_SWS_EGRESS_HYST_DELTA_PERCENT / 100;
    buf_cfg->per_queue_drop_hysteresis_delta = SOC_SBX_CALADAN3_SWS_QUEUE_HYSTERIS_DELTA;

    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_auto_set(int unit, sws_config_t *sws_cfg) 
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_port_map_t *port_map;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    rv = soc_sbx_caladan3_queue_info_auto_set_intf(unit, port_map, sws_cfg, 
                                              &soc_sbx_caladan3_sws_intf_data[0]);
    if (SOC_SUCCESS(rv)) {
        rv = soc_sbx_caladan3_sws_pt_fifo_auto_set(unit, sws_cfg);
        if (SOC_SUCCESS(rv)) {
            rv = soc_sbx_caladan3_sws_pt_port_calendar_auto_set(unit, sws_cfg);
        }
        if (SOC_SUCCESS(rv)) {
            rv = soc_sbx_caladan3_sws_pt_client_calendar_auto_set(unit, sws_cfg);
        }
    }
    return rv;
}



/*
 * Function:
 *    soc_sbx_caladan3_get_squeue_info
 * Purpose
 *    Copy perqueue info from tdm database
 */
int
soc_sbx_caladan3_get_squeue_info(int unit, 
                                 soc_sbx_caladan3_port_map_t *port_map,
                                 int port,
                                 int squeue,
                                 sws_qm_source_queue_cfg_t **queue_cfg)
{
    int offset;
    sws_qm_source_queue_cfg_t *db_queue_cfg;
    soc_sbx_caladan3_port_map_info_t *line_info;

    line_info = &port_map->line_port_info[port];

    if (squeue < SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE) {
        if (line_info->blk == soc_sbx_block_find(unit, SOC_BLK_CMIC, 0)) {
            db_queue_cfg = &sws_dbase[unit]->qm_cfg->cmic_queue;
            if (db_queue_cfg->max_pages <= 0) {
                if (SOC_FAILURE(
                        soc_sbx_caladan3_set_squeue_intf_info(unit, queue_cfg, TRUE,
                                                              line_info, squeue))) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d XL Squeue %d info not present in database\n"),
                               unit, squeue));
                    return SOC_E_PARAM;
                }
                return SOC_E_NONE;
            }
        } else if (line_info->blk == 
                       soc_sbx_block_find(unit, SOC_BLK_XLPORT, 0)) {
            offset = line_info->intf_instance;
            db_queue_cfg = &sws_dbase[unit]->qm_cfg->xlport_queue[offset];
            if (db_queue_cfg->max_pages <= 0) {
                if (SOC_FAILURE(
                        soc_sbx_caladan3_set_squeue_intf_info(unit, queue_cfg, TRUE,
                                                              line_info, squeue))) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d XL Squeue %d info not present in database\n"),
                               unit, squeue));
                    return SOC_E_PARAM;
                }
                return SOC_E_NONE;
            }
        } else {
            offset = squeue-SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE;
            db_queue_cfg = &sws_dbase[unit]->qm_cfg->line_queues[offset];
            if (db_queue_cfg->max_pages <= 0) {
                if (SOC_FAILURE(
                        soc_sbx_caladan3_set_squeue_intf_info(unit, queue_cfg, TRUE,
                                                              line_info, squeue))) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d Line Squeue %d info not present in database\n"), 
                               unit, squeue));
                    return SOC_E_PARAM;
                }
                return SOC_E_NONE;
            }
        }
    } else {
        offset = squeue - SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE;
        if (offset == SOC_SBX_CALADAN3_SWS_CMIC_QUEUE_BASE) {
            db_queue_cfg = &sws_dbase[unit]->qm_cfg->cmic_queue;
            if (db_queue_cfg->max_pages <= 0) {
                if (SOC_FAILURE(
                        soc_sbx_caladan3_set_squeue_intf_info(unit, queue_cfg, FALSE,
                                                              line_info, squeue))) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d CMIC Fabric Squeue %d info not present in database\n"),
                               unit, squeue));
                    return SOC_E_PARAM;
                }
                return SOC_E_NONE;
            }
        } else if ((offset >= SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE) &&
                    (offset < SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE +
                                   SOC_SBX_CALADAN3_MAX_XLPORT_PORT)) { 
            offset -= SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE;
            offset = offset >> 1;
            db_queue_cfg = &sws_dbase[unit]->qm_cfg->xlport_queue[offset];
            if (db_queue_cfg->max_pages <= 0) {
                if (SOC_FAILURE(
                        soc_sbx_caladan3_set_squeue_intf_info(unit, queue_cfg, FALSE,
                                                              line_info, squeue))) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d XL Fabric Squeue %d info not present in database\n"),
                               unit, squeue));
                    return SOC_E_PARAM;
                }
                return SOC_E_NONE;
            }
        } else {
            offset = squeue-SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE;
            db_queue_cfg = &sws_dbase[unit]->qm_cfg->fabric_queues[offset];
            if (db_queue_cfg->max_pages <= 0) {
                if (SOC_FAILURE(
                        soc_sbx_caladan3_set_squeue_intf_info(unit, queue_cfg, FALSE,
                                                              line_info, squeue))) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d Fabric Squeue %d info not present in database\n"),
                               unit, squeue));
                    return SOC_E_PARAM;
                }
                return SOC_E_NONE;
            }
        }
    }

    if (queue_cfg) {
        *queue_cfg =  db_queue_cfg;
    }


    return SOC_E_NONE;
}




int
soc_sbx_caladan3_sws_intf_queue_info_set(int unit, int qid, int line,
                                         sws_qm_source_queue_cfg_t *db_queue_cfg,
                                         soc_sbx_caladan3_sws_queue_info_t *queue_info)
{
    sws_qm_source_queue_cfg_t *qcfg;
    int rv = SOC_E_NONE;

    if (line) {
        qcfg = &sws_dbase[unit]->local_sws_cfg.qm_cfg.line_queues[qid];
    } else {
        qcfg = &sws_dbase[unit]->local_sws_cfg.qm_cfg.fabric_queues[qid-SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE];
    }
    rv = soc_sbx_caladan3_sws_queue_property_get(unit, qid, (int*)qcfg);
    if (SOC_SUCCESS(rv)) {
        queue_info[qid].squeue_config = qcfg;
    } else {
        if (!sws_dbase[unit]->cfg_valid) {
            /* No TDM params found */
            queue_info[qid].squeue_config = qcfg;
        } else {
            queue_info[qid].squeue_config = db_queue_cfg;
        }
    }
    queue_info[qid].enabled = TRUE;
    return SOC_E_NONE;
}


int
soc_sbx_caladan3_sws_rsvd_queue_info_set(int unit, int qid, 
                                         sws_qm_source_queue_cfg_t *db_queue_cfg,
                                         soc_sbx_caladan3_sws_queue_info_t *queue_info)
{
    sws_qm_source_queue_cfg_t *qcfg;
    int rv = SOC_E_NONE;

    if (qid < SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE) {
        qcfg = &sws_dbase[unit]->local_sws_cfg.qm_cfg.line_queues[qid];
    } else {
        qcfg = &sws_dbase[unit]->local_sws_cfg.qm_cfg.fabric_queues[qid-SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE];
    }
    rv = soc_sbx_caladan3_sws_queue_property_get(unit, qid, (int*)qcfg);
    if (SOC_SUCCESS(rv)) {
        queue_info[qid].squeue_config = qcfg;
    } else {
        if ((db_queue_cfg && db_queue_cfg->max_pages == 0) || 
              (!sws_dbase[unit]->cfg_valid)) {
            /* No TDM params found */
            soc_sbx_caladan3_queue_info_auto_get_resvd(unit, &qcfg);
            queue_info[qid].squeue_config = qcfg;
        } else {
            queue_info[qid].squeue_config = db_queue_cfg;
        }
    }
    queue_info[qid].enabled = TRUE;
    return SOC_E_NONE;
}


/**
 * Function:
 *     soc_sbx_caladan3_sws_queue_info_init
 * Purpose:
 *     Initialize QM queue database init
 */
int 
soc_sbx_caladan3_sws_queue_info_init(int unit)
{
    soc_sbx_caladan3_port_map_t *port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    soc_sbx_caladan3_sws_queue_info_t *queue_info;
    soc_sbx_caladan3_port_map_info_t *line_info, *fabric_info;
    soc_sbx_caladan3_queues_t *line_qi, *fabric_qi;
    sws_qm_source_queue_cfg_t *db_queue_cfg;
    int port, cos, rv = SOC_E_NONE;
    int qid, squeue, dqueue;

    if ((sws_dbase[unit] == NULL) || (sws_dbase[unit]->queue_info == NULL)) {
        return SOC_E_INIT;
    }
    queue_info = sws_dbase[unit]->queue_info;

    for (port=0; port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {
        if (!(port_map->line_port_info[port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        line_info = &port_map->line_port_info[port];
        line_qi = &line_info->port_queues;
        fabric_info = &port_map->fabric_port_info[port];
        fabric_qi = &fabric_info->port_queues;

        cos = 0;
        SOC_SBX_C3_BMP_ITER_RANGE(line_qi->squeue_bmp, SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE,
                                  squeue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
            assert(squeue >= SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE && squeue < SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE);
            db_queue_cfg = NULL;
            queue_info[squeue].port = line_info->port;
            queue_info[squeue].cos = cos++;
            queue_info[squeue].enabled =  TRUE;
            /* See if not tdm parameters are found, dummy data can be applied */
            if (sws_dbase[unit]->cfg_valid) {
                soc_sbx_caladan3_get_squeue_info(unit, port_map, port, squeue, &db_queue_cfg);
            }
            soc_sbx_caladan3_sws_intf_queue_info_set(unit, squeue, TRUE, db_queue_cfg, queue_info);
        }
        cos = 0;
        SOC_SBX_C3_BMP_ITER_RANGE(line_qi->dqueue_bmp, SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE,
                                  dqueue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {
            assert(dqueue >= SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE && dqueue < SOC_SBX_CALADAN3_SWS_QM_MAX_QUEUES);
            queue_info[dqueue].port = line_info->port;
            queue_info[dqueue].cos = cos++;
            queue_info[dqueue].enabled =  TRUE;
        }
        cos = 0;
        SOC_SBX_C3_BMP_ITER_RANGE(fabric_qi->squeue_bmp, SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE,
                                  squeue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
            assert(squeue >= SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE && squeue < SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE);
            db_queue_cfg = NULL;
            queue_info[squeue].port = fabric_info->port;
            queue_info[squeue].cos = cos++;
            queue_info[squeue].enabled =  TRUE;
            /* See if not tdm parameters are found, dummy data can be applied */
            if (sws_dbase[unit]->cfg_valid) {
                soc_sbx_caladan3_get_squeue_info(unit, port_map, port, squeue, &db_queue_cfg);
            }
            soc_sbx_caladan3_sws_intf_queue_info_set(unit, squeue, FALSE, db_queue_cfg, queue_info);
        }
        cos = 0;
        SOC_SBX_C3_BMP_ITER_RANGE(fabric_qi->dqueue_bmp, SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE,
                                  dqueue, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {
            assert(dqueue >= SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE && dqueue < SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE);
            queue_info[dqueue].port = fabric_info->port;
            queue_info[dqueue].cos = cos++;
            queue_info[dqueue].enabled =  TRUE;
        }
    }
 
    if (SOC_HOTSWAP_TDM) {
        return rv;
    }

    /* Set Special queue info */
    qid = sws_dbase[unit]->ing_redirect_qid0;
    if (qid >= 0) {
        db_queue_cfg = (sws_dbase[unit]->qm_cfg) ? &sws_dbase[unit]->qm_cfg->ing_rt_queue[0] : NULL;
        soc_sbx_caladan3_sws_rsvd_queue_info_set(unit, qid, db_queue_cfg, queue_info);
    }

    qid = sws_dbase[unit]->ing_redirect_qid1;
    if (qid >= 0) {
        db_queue_cfg = (sws_dbase[unit]->qm_cfg) ? &sws_dbase[unit]->qm_cfg->ing_rt_queue[1] : NULL;
        soc_sbx_caladan3_sws_rsvd_queue_info_set(unit, qid, db_queue_cfg, queue_info);
    }

    qid = sws_dbase[unit]->egr_redirect_qid0;
    if (qid >= 0) {
        db_queue_cfg = (sws_dbase[unit]->qm_cfg) ? &sws_dbase[unit]->qm_cfg->egr_rt_queue[0] : NULL;
        soc_sbx_caladan3_sws_rsvd_queue_info_set(unit, qid, db_queue_cfg, queue_info);
    }

    qid = sws_dbase[unit]->egr_redirect_qid1;
    if (qid >= 0) {
        db_queue_cfg = (sws_dbase[unit]->qm_cfg) ? &sws_dbase[unit]->qm_cfg->egr_rt_queue[1] : NULL;
        soc_sbx_caladan3_sws_rsvd_queue_info_set(unit, qid, db_queue_cfg, queue_info);
    }

    qid = sws_dbase[unit]->ing_bubble_qid;
    if (qid >= 0) {
        db_queue_cfg = (sws_dbase[unit]->qm_cfg) ? &sws_dbase[unit]->qm_cfg->ing_bubble_queue : NULL;
        soc_sbx_caladan3_sws_rsvd_queue_info_set(unit, qid, db_queue_cfg, queue_info);
    }

    qid = sws_dbase[unit]->egr_bubble_qid;
    if (qid >= 0) {
        db_queue_cfg = (sws_dbase[unit]->qm_cfg) ? &sws_dbase[unit]->qm_cfg->egr_bubble_queue : NULL;
        soc_sbx_caladan3_sws_rsvd_queue_info_set(unit, qid, db_queue_cfg, queue_info);
    }
    
    if( sws_dbase[unit]->app_queues_num > 0) {
        for (qid=sws_dbase[unit]->app_queues_start;
             qid < (sws_dbase[unit]->app_queues_start + sws_dbase[unit]->app_queues_num);
                 qid++) {
            soc_sbx_caladan3_sws_intf_queue_info_set(unit, qid, FALSE,
                                                     NULL, queue_info);
        }
    }

    if (!sws_dbase[unit]->cfg_valid) {
        if (!sws_dbase[unit]->qm_cfg) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Buffer Config data not found\n"), unit));
            return SOC_E_PARAM;
        } else {
            rv = soc_sbx_caladan3_buffer_param_config_get(unit, &sws_dbase[unit]->qm_cfg->buffer);
        }
    }

    return rv;
}

int
soc_sbx_caladan3_sws_app_queues_set(int unit, int start, int numq) 
{
    int q, rv = SOC_E_NONE;
    soc_sbx_caladan3_sws_queue_info_t *queue_info;

    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_app_queues_set: Invalid unit\n")));
        return SOC_E_PARAM;
    }
    if (numq > 64) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** Error: Num App queues (%d) illegal, ignored\n"), unit, numq));
        return SOC_E_PARAM;
    }
    if ((start < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) && 
            (start > (SOC_SBX_CALADAN3_SWS_QM_MAX_QUEUES + 
                          SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE))) {
        /* coverity[dead_error_begin] */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** Error: App queue start(%d) invalid \n"), unit, start));
        return SOC_E_PARAM;
    }
    if (sws_dbase[unit]->app_queues_num > 0) {
        rv  = soc_sbx_caladan3_sws_queue_free(unit, start, numq);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Error: Failed free app queue range(%d:%d) status(%d)\n"),
                       unit, start, numq, rv));
            return rv;
        }
    }
    rv  = soc_sbx_caladan3_sws_queue_alloc(unit, start, numq);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** Error: Failed allocating app queues (%d)\n"),
                   unit, rv));
        return rv;
    }
        
    sws_dbase[unit]->app_queues_start = start;
    sws_dbase[unit]->app_queues_num = numq;
    queue_info = sws_dbase[unit]->queue_info;
    for (q=start; q < (start+numq); q++) {
        /* Dynamically updating queue config could affect system thresholds */ 
        soc_sbx_caladan3_sws_rsvd_queue_info_set(unit, q, NULL, queue_info);
        soc_sbx_caladan3_sws_qm_source_queue_config(unit, q, queue_info[q].squeue_config, 1);
    }

    return SOC_E_NONE;
}


int
soc_sbx_caladan3_sws_app_queues_get(int unit, int *start, int *numq) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_app_queues_get: Invalid unit (%d)\n"), unit));
        return SOC_E_PARAM;
    }
    *start = sws_dbase[unit]->app_queues_start;
    *numq = sws_dbase[unit]->app_queues_num;
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_allocate_reserved_queues(int unit)
{
    int numq = 0;
    int start = 0;
    int rv = SOC_E_NONE;

    /* Ingress Redirect */
    sws_dbase[unit]->ing_redirect_qid0 = 
        soc_sbx_caladan3_sws_config_param_data_get(unit, 
                spn_INGRESS_TO_EGRESS_REDIRECT_QID0, -1, 
                SOC_SBX_CALADAN3_SWS_LINE_TO_FAB_REDIRECT_QID0);
    if (sws_dbase[unit]->ing_redirect_qid0 > 0 ) {
        rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sws_dbase[unit]->ing_redirect_qid0, 1);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Error: Failed allocating ing redir qid0 (%d)\n"),
                       unit, rv));
            return rv;
        }
    }
    sws_dbase[unit]->ing_redirect_qid1 = 
        soc_sbx_caladan3_sws_config_param_data_get(unit, 
                spn_INGRESS_TO_EGRESS_REDIRECT_QID1, -1,  
                SOC_SBX_CALADAN3_SWS_LINE_TO_FAB_REDIRECT_QID1);
    if (sws_dbase[unit]->ing_redirect_qid1 > 0 ) {
        rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sws_dbase[unit]->ing_redirect_qid1, 1);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Error: Failed allocating ing redir qid1 (%d)\n"),
                       unit, rv));
            return rv;
        }
    }

    /* Egress redirect */
    sws_dbase[unit]->egr_redirect_qid0 = 
        soc_sbx_caladan3_sws_config_param_data_get(unit, 
                spn_EGRESS_TO_INGRESS_REDIRECT_QID0, -1,  
                SOC_SBX_CALADAN3_SWS_FAB_TO_LINE_REDIRECT_QID0);
    if (sws_dbase[unit]->egr_redirect_qid0 > 0 ) {
        rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sws_dbase[unit]->egr_redirect_qid0, 1);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Error: Failed allocating egr redir qid0 (%d)\n"),
                       unit, rv));
            return rv;
        }
    }
    sws_dbase[unit]->egr_redirect_qid1 = 
        soc_sbx_caladan3_sws_config_param_data_get(unit, 
                spn_EGRESS_TO_INGRESS_REDIRECT_QID1, -1,  
                SOC_SBX_CALADAN3_SWS_FAB_TO_LINE_REDIRECT_QID1);
    if (sws_dbase[unit]->egr_redirect_qid1 > 0 ) {
        rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sws_dbase[unit]->egr_redirect_qid1, 1);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Error: Failed allocating egr redir qid1 (%d)\n"),
                       unit, rv));
            return rv;
        }
    }

    /* bubbles */
    sws_dbase[unit]->ing_bubble_qid = 
          soc_sbx_caladan3_sws_config_param_data_get(unit, 
                spn_INGRESS_BUBBLE_QID, -1,  
                SOC_SBX_CALADAN3_SWS_INGRESS_BUBBLE_SQUEUE);
    if (sws_dbase[unit]->ing_bubble_qid > 0 ) {
        rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sws_dbase[unit]->ing_bubble_qid, 1);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Error: Failed allocating ing bubble qid (%d)\n"),
                       unit, rv));
            return rv;
        }
    }

    sws_dbase[unit]->egr_bubble_qid = 
        soc_sbx_caladan3_sws_config_param_data_get(unit, 
                spn_EGRESS_BUBBLE_QID, -1,  
                SOC_SBX_CALADAN3_SWS_EGRESS_BUBBLE_SQUEUE);
    if (sws_dbase[unit]->egr_bubble_qid > 0 ) {
        rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sws_dbase[unit]->egr_bubble_qid, 1);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Error: Failed allocating ing bubble qid (%d)\n"),
                       unit, rv));
            return rv;
        }
    }

    /* App specific */
    numq = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_APP_QUEUES_NUM, -1, -1);
    if (numq < 0) {
        /* Not enabled */
        return SOC_E_NONE;
    } else if (numq > 64) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** Error: Num App queues (%d) illegal\n"), unit, numq));
        return SOC_E_PARAM;
    }
    start = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_APP_QUEUES_START, -1, -1);
    if ((start < 0) && (numq > 0)) {
        rv = soc_sbx_caladan3_sws_find_queue_range(unit, numq, EGRESS_SQUEUE, &start);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Error: Failed to find %d free queues for app\n"),
                       unit, numq));
            return rv;
        }
    } else {
        if ((start < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) && 
                (start > (SOC_SBX_CALADAN3_SWS_QM_MAX_QUEUES + 
                              SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE))) {
            /* coverity[dead_error_begin] */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Error: App queue start(%d) invalid \n"), unit, start));
            return rv;
        }
    }

    rv  = soc_sbx_caladan3_sws_queue_alloc(unit, start, numq);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** Error: Failed allocating app queues (%d)\n"),
                   unit, rv));
        return rv;
    }
    sws_dbase[unit]->app_queues_num = numq;
    sws_dbase[unit]->app_queues_start = start;
    return SOC_E_NONE;
}


/*
 * Function: soc_sbx_caladan3_sws_allocate_port_queues
 * Purpose:
 *   Allocate Port queues
 *   This routine allows user override for each of queue allocated
 */
int
soc_sbx_caladan3_sws_allocate_port_queues(int unit, int reserved_ports_only)
{ 
    int base_port, port;
    soc_sbx_caladan3_port_map_info_t *lp_info;
    soc_sbx_caladan3_port_map_info_t *fp_info;
    int num_dq=0, num_cos = 0, sq = 0, dq = 0;
    int rv = 0;
    int line_sq=0, line_dq=0, fab_sq = 0, fab_dq=0;
    int xlsq, xldq, xfsq, xfdq;
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_queues_t *lqinfo, *fqinfo;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    if (!port_map) {
        return SOC_E_PARAM;
    }
    xlsq = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_XL_INGRESS_SQUEUE, -1, -1);
    xldq = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_XL_INGRESS_DQUEUE, -1, -1);
    xfsq = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_XL_EGRESS_SQUEUE, -1, -1);
    xfdq = soc_sbx_caladan3_sws_config_param_data_get(unit, spn_XL_EGRESS_DQUEUE, -1, -1);

    for (port = reserved_ports_only ? port_map->first_reserved_port : 0;
            port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {

        lp_info = &port_map->line_port_info[port];
        if (!(lp_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        lqinfo = &lp_info->port_queues;
        lqinfo->spri0_qid = 
             soc_sbx_caladan3_sws_config_param_data_get(unit, spn_SPRI0_QID, lp_info->port, 0);
        lqinfo->spri1_qid = 
             soc_sbx_caladan3_sws_config_param_data_get(unit, spn_SPRI1_QID, lp_info->port, 0);
        lqinfo->spri_set = ((lqinfo->spri1_qid + lqinfo->spri0_qid) > 0);

        fp_info = &port_map->fabric_port_info[port];
        fqinfo = &fp_info->port_queues;
        fqinfo->spri0_qid = 
             soc_sbx_caladan3_sws_config_param_data_get(unit, spn_SPRI0_QID, fp_info->port, 0);
        fqinfo->spri1_qid = 
             soc_sbx_caladan3_sws_config_param_data_get(unit, spn_SPRI1_QID, fp_info->port, 0);
        fqinfo->spri_set = ((fqinfo->spri1_qid + fqinfo->spri0_qid) > 0);

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d caladan3_sws_allocate_port_queues: Allocating Queues for Port %d(%s)\n"),
                     unit, lp_info->port, SOC_PORT_NAME(unit, lp_info->port)));

        if (lp_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CMIC) {
            /* Line Squeue */
            lqinfo->squeue_base = 
               soc_sbx_caladan3_sws_config_param_data_get(unit, spn_CMIC_INGRESS_SQUEUE, 
                                                          -1, lqinfo->squeue_base);

            if (lqinfo->squeue_base < 0) {
                if (line_sq > SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE) {
                    sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE + line_sq++;
                } else {
                    sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE + 
                             SOC_SBX_CALADAN3_SWS_CMIC_QUEUE_BASE;
                }
            } else {
                sq = lqinfo->squeue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating line squeue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            port_map->default_hpte_hiprio_ingress_squeues[2] = sq;
            lqinfo->squeue_base = sq;
    
            /* Line Dqueue */
            lqinfo->dqueue_base = 
                soc_sbx_caladan3_sws_config_param_data_get(unit, spn_CMIC_INGRESS_DQUEUE, 
                                                           -1, lqinfo->dqueue_base);
            if (lqinfo->dqueue_base < 0) {
                if (lqinfo->dqueue_base > SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE) {
                    dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + line_dq++;
                } else {
                    dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + 
                             SOC_SBX_CALADAN3_SWS_CMIC_QUEUE_BASE;
                }
            } else {
                dq = lqinfo->dqueue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating line dqueue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            lqinfo->dqueue_base = dq;

            /* Fabric Squeue */
            fqinfo->squeue_base = 
                soc_sbx_caladan3_sws_config_param_data_get(unit, spn_CMIC_EGRESS_SQUEUE, 
                                                           -1, fqinfo->squeue_base);
            if (fqinfo->squeue_base < 0) {
                sq = SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE+
                         SOC_SBX_CALADAN3_SWS_CMIC_QUEUE_BASE;
            } else {
                sq = fqinfo->squeue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating fab squeue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            port_map->default_hpte_hiprio_egress_squeues[2] = sq;
            fqinfo->squeue_base = sq;
    
            /* Fabric Dqueue */
            fqinfo->dqueue_base = 
                 soc_sbx_caladan3_sws_config_param_data_get(unit, spn_CMIC_EGRESS_DQUEUE, 
                                                            -1, fqinfo->dqueue_base);
            if (fqinfo->dqueue_base < 0) {
                dq = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE +
                         SOC_SBX_CALADAN3_SWS_CMIC_QUEUE_BASE;
            } else {
                dq = fqinfo->dqueue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating fab dqueue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->dqueue_base = dq;

        } else if (lp_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_XLPORT) {

            if (xlsq < 0) {
                if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
                    if (line_sq > SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE) {
                        sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE + line_sq;
                        if (lp_info->bindex & 1) line_sq++;
                    } else {
                        sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE + 
                             SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE + lp_info->bindex/2;
                    }
                } else {
                    if (line_sq > SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE) {
                        sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE + line_sq++;
                    } else {
                        sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE + 
                             SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE + lp_info->bindex;
                    }
                }
            } else {
                if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
                    sq = xlsq + lp_info->bindex/2;
                } else {
                    sq = xlsq + lp_info->bindex;
                }
            }
            /* Line Squeue */
            if ((lp_info->bindex & 1) || (SOC_SBX_CALADAN3_MAX_XLPORT_PORT <= 2)) {
                rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, 1);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d *** Error: Failed allocating line squeue for port(%d)\n"),
                               unit, lp_info->port));
                    return rv;
                }
                if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT <= 2) {
                    port_map->default_hpte_hiprio_ingress_squeues[lp_info->bindex] = sq;
                } else {
                    port_map->default_hpte_hiprio_ingress_squeues[lp_info->bindex/2] = sq;
                }
            }
            lqinfo->squeue_base = sq;
   
            /* Line Dqueue */
            if (xldq < 0) {
                if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
                    if (line_dq > SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE) {
                        dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + line_dq;
                        if (lp_info->bindex & 1) line_dq ++;
                    } else {
                        dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + 
                             SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE + lp_info->bindex/2;
                    }
                } else {
                    if (line_dq > SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE) {
                        dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE+ line_dq++;
                    } else {
                        dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + 
                             SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE + lp_info->bindex;
                    }
                }
            } else  {
                dq = lqinfo->dqueue_base;
                if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
                    dq = xldq + lp_info->bindex/2;
                } else {
                    dq = xldq + lp_info->bindex;
                }
            }
            if ((lp_info->bindex & 1) || (SOC_SBX_CALADAN3_MAX_XLPORT_PORT <= 2)) {
                rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, 1);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d *** Error: Failed allocating line dqueue for port(%d)\n"),
                               unit, lp_info->port));
                    return rv;
                }
            }
            lqinfo->dqueue_base = dq;

            /* Fabric Squeue */
            if (xfsq < 0) {
                if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
                    sq = SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE+
                            SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE + lp_info->bindex/2;
                } else {
                    sq = SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE+
                            SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE + lp_info->bindex;
                }
            } else {
                sq = fqinfo->squeue_base;
                if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
                    sq = xfsq + lp_info->bindex/2;
                } else {
                    sq = xfsq + lp_info->bindex;
                }
            }
            if ((lp_info->bindex & 1) || (SOC_SBX_CALADAN3_MAX_XLPORT_PORT <= 2)) {
                rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, 1);
                if ((SOC_SBX_CALADAN3_MAX_XLPORT_PORT <= 2) && SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d *** Error: Failed allocating fab squeue for port(%d)\n"),
                               unit, fp_info->port));
                    return rv;
                }
                if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT <= 2) {
                    port_map->default_hpte_hiprio_egress_squeues[lp_info->bindex] = sq;
                } else {
                    port_map->default_hpte_hiprio_egress_squeues[lp_info->bindex/2] = sq;
                }
            }
            fqinfo->squeue_base = sq;
   
            /* Fabric Dqueue */
            if (xfdq < 0) {
                if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
                    dq = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE+
                            SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE + lp_info->bindex/2;
                } else  {
                    dq = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE+
                            SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE + lp_info->bindex;
                }
            } else {
                if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
                    dq = xfdq + lp_info->bindex/2;
                } else {
                    dq = xfdq + lp_info->bindex;
                }
            }
            if ((lp_info->bindex & 1) || (SOC_SBX_CALADAN3_MAX_XLPORT_PORT <= 2)) {
                rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, 1);
                if ((SOC_SBX_CALADAN3_MAX_XLPORT_PORT <= 2) && SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d *** Error: Failed allocating fab dqueue for port(%d)\n"),
                               unit, fp_info->port));
                    return rv;
                }
            }
            fqinfo->dqueue_base = dq;

        } else if (lp_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_XTPORT) {

            /* Line Squeue */
            if (lqinfo->squeue_base < 0) {
                sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE +
                        12 + lp_info->bindex + lp_info->instance * 12;
            } else {
                sq = lqinfo->squeue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating line squeue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            lqinfo->squeue_base = sq;
    
            /* Line Dqueue */
            if (lqinfo->dqueue_base < 0) {
                dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE +
                        12 + lp_info->bindex + lp_info->instance * 12;
            } else {
                dq = lqinfo->dqueue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating line dqueue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            lqinfo->dqueue_base = dq;

            /* Fabric Squeue */
            if (fqinfo->squeue_base < 0) {
                sq = SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + fab_sq;
            } else {
                sq = fqinfo->squeue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating fab dqueue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->squeue_base = sq;
    
            /* Fabric Dqueue */
            if (fqinfo->dqueue_base < 0) {
                dq = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE + fab_dq;
            } else {
                dq = fqinfo->dqueue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating fab dqueue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->dqueue_base = dq;
            fab_sq++;
            fab_dq++;
        } else if (((lp_info->intftype == 
                        SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE) ||
                   (lp_info->intftype == 
                        SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE) ||
                    (lp_info->intftype == 
                     SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE)) && 
                         (lqinfo->num_squeue <= 1) && lqinfo->num_dqueue <=1 ) {
            /* Line Squeue */
            if (lqinfo->squeue_base < 0) {
                sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE + 
                         lp_info->bindex + 4 * lp_info->intf_instance;
            } else {
                sq = lqinfo->squeue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating Line squeue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            lqinfo->squeue_base = sq;
   
            /* Line Dqueue */
            if (lqinfo->dqueue_base < 0) {
                dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + 
                         lp_info->bindex + 4 * lp_info->intf_instance;
            } else {
                dq = lqinfo->dqueue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating Line dqueue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            lqinfo->dqueue_base = dq;
    
            /* Fabric Squeue */
            if (fqinfo->squeue_base < 0) {
                sq = SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + fab_sq;
            } else {
                sq = fqinfo->squeue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating Fabric squeue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->squeue_base = sq;
    
            /* Fabric Dqueue */
            if (fqinfo->dqueue_base < 0) {
                dq = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE + fab_dq;
            } else {
                dq = fqinfo->dqueue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating Fabric dqueue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->dqueue_base = dq;
            fab_dq++; 
            fab_sq++;
        } else if ((lp_info->intftype == 
                       SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10) &&
                         (lqinfo->num_squeue != 0)) {
            int num_cos;
            int i, cnt = 0, c = 0;

            if ((lqinfo->squeue_base < 0) ||
                 (lqinfo->dqueue_base < 0)) {

                for (i = 0; i < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; i++) {
                    if (!(port_map->line_port_info[i].flags & 
                              SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID) ||
                       (port_map->line_port_info[i].intf_instance ==
                              lp_info->intf_instance)) {
                        continue;
                    }
                    if ((port_map->line_port_info[i].intftype ==
                              SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE) ||
                         (port_map->line_port_info[i].intftype ==
                              SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE)) {
                        c++;
                    }
                }
                if (c) { cnt = 12; }
            }
            if (lqinfo->squeue_base < 0) {
                line_sq = (line_sq > 0) ? line_sq : cnt;
                sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE + line_sq;
            } else {
                sq = lqinfo->squeue_base;
                line_sq = sq - SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE;
            }
            if (lqinfo->dqueue_base < 0) {
                line_dq = (line_dq > 0) ? line_dq : cnt;
                dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + line_dq;
            } else {
                dq = lqinfo->dqueue_base;
                line_dq = dq - SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE;
            }

            /* Line Squeue */
            num_cos = (lqinfo->num_squeue > 0) ? lqinfo->num_squeue : 1;
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, num_cos);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating line squeue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            lqinfo->squeue_base = sq;
    
            /* Line Dqueue */
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, num_cos);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating line dqueue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            } 
            lqinfo->dqueue_base = dq;
            line_sq += num_cos;
            line_dq += num_cos;

            num_cos = (fqinfo->num_squeue > 0) ? fqinfo->num_squeue : 1;
            if (fqinfo->squeue_base < 0) {
                /* Fabric Squeue */
                sq = SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + fab_sq;
            } else {
                sq = fqinfo->squeue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, num_cos);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating fab squeue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->squeue_base = sq;
    
            /* Fabric Dqueue */
            if (fqinfo->dqueue_base < 0) {
                dq = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE + fab_dq;
            } else {
                dq = fqinfo->dqueue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, num_cos);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating fab dqueue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->dqueue_base = dq;
            fab_sq += num_cos;
            fab_dq += num_cos;
        } else if (((lp_info->intftype ==
                       SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE) ||
                   (lp_info->intftype ==
                       SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42) ||
                   (lp_info->intftype ==
                       SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25)) &&
                          (lqinfo->num_squeue <= 1)) {
            /* Line Squeue */
            if (lqinfo->squeue_base < 0) {
                sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE + line_sq;
            } else {
                sq = lqinfo->squeue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating line squeue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            lqinfo->squeue_base = sq;
    
            /* Line Dqueue */
            if (lqinfo->dqueue_base < 0) {
                dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + line_dq;
            } else {
                dq = lqinfo->dqueue_base;
            }
            /* Default allocates 4 queues to satisfy mac to pr/pt map */
            num_dq = (lqinfo->num_dqueue > 1) ? (lqinfo->num_dqueue) : (1);
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, num_dq);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating line squeue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            lqinfo->dqueue_base = dq;
            line_sq += 4;
            line_dq += (lqinfo->num_dqueue > 4) ? (lqinfo->num_dqueue) : (4);
    
            /* Fabric Squeue */
            if (fqinfo->squeue_base < 0) {
                sq = SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + fab_sq;
            } else {
                sq = fqinfo->squeue_base;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, 1);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating fab squeue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->squeue_base = sq;
    
            /* Fabric Dqueue */
            if (fqinfo->dqueue_base < 0) {
                dq = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE + fab_dq;
            } else {
                dq = fqinfo->dqueue_base;
            }
            /* Default allocates 4 queues to satisfy mac to pr/pt map */
            num_dq = (fqinfo->num_dqueue > 1) ? (fqinfo->num_dqueue) : (1);
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, num_dq);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating fab dqueue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->dqueue_base = dq;
            fab_sq += 4;
            fab_dq += (fqinfo->num_dqueue > 4) ? (fqinfo->num_dqueue) : (4);
        } else {
            /* Line Squeue */
            if (lqinfo->squeue_base < 0) {
                sq = SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE + line_sq;
            } else {
                sq = lqinfo->squeue_base;
            }
            num_cos = (lqinfo->num_squeue > 0) ? lqinfo->num_squeue : 1;
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, num_cos);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating line squeue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            lqinfo->squeue_base = sq;
    
            /* Line Dqueue */
            if (lqinfo->dqueue_base < 0) {
                dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + line_dq;
            } else {
                dq = lqinfo->dqueue_base;
            }
            num_dq = (lqinfo->num_dqueue > 0) ? (lqinfo->num_dqueue) : (num_cos);
            base_port = lp_info->base_port;
            if ((lp_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE) ||
                   (lp_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42) ||
                   (lp_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25)) {
                if (soc_sbx_caladan3_sws_pt_remap_enabled(unit)) {
                    base_port >>= 2;
                }
            }
            if (!SOC_IS_CALADAN3_REVB(unit) && (base_port > 2) && (num_dq > 1)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      " *** Warning: Unit %d Cannot channelize Port %d, ignoring number of dqueues\n"),
                           unit, lp_info->port));
                num_dq = 1;
                lqinfo->num_dqueue=1;
                dq = SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE + line_dq;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, num_dq);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating line dqueue for port(%d)\n"),
                           unit, lp_info->port));
                return rv;
            }
            lqinfo->dqueue_base = dq;

            line_sq += num_cos;
            line_dq += num_dq;

            /* Fabric Squeue */
            if (fqinfo->squeue_base < 0) {
                sq = SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + fab_sq;
            } else {
                sq = fqinfo->squeue_base;
            }
            num_cos = (fqinfo->num_squeue > 0) ? fqinfo->num_squeue : 1;
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, sq, num_cos);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating fab squeue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->squeue_base = sq;
    
            /* Fabric Dqueue */
            if (fqinfo->dqueue_base < 0) {
                dq = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE + fab_dq;
            } else {
                dq = fqinfo->dqueue_base;
            }
            num_dq = (fqinfo->num_dqueue > 0) ? (fqinfo->num_dqueue) : (num_cos);
            base_port = fp_info->base_port;
            if ((fp_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE) ||
                   (fp_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42) ||
                   (fp_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25)) {
                if (soc_sbx_caladan3_sws_pt_remap_enabled(unit)) {
                    base_port >>= 2;
                }
            }
            if (!SOC_IS_CALADAN3_REVB(unit) && (base_port > 2) && (num_dq > 1)) {
                num_dq = 1;
                fqinfo->num_dqueue=1;
                dq = SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE + fab_dq;
            }
            rv  = soc_sbx_caladan3_sws_queue_alloc(unit, dq, num_dq);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: Failed allocating fab dqueue for port(%d)\n"),
                           unit, fp_info->port));
                return rv;
            }
            fqinfo->dqueue_base = dq;
            fab_sq += num_cos;
            fab_dq += num_dq;
        }

        /* 2 queues are reserved for redirection & bubble. Never allocate them */
        assert(line_sq < SOC_SBX_CALADAN3_SWS_AVAILABLE_INGRESS_QUEUES);
        assert(fab_sq < SOC_SBX_CALADAN3_SWS_AVAILABLE_EGRESS_QUEUES);

        /* assert if queue number is bogus */
        assert(line_sq < SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);
        assert(line_dq < SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);
        assert(fab_sq < SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);
        assert(fab_dq < SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION);

        /* Update port queues */
        rv = soc_sbx_caladan3_port_queues_update_bmp(unit, lqinfo);
        if (rv == SOC_E_NONE) {
            rv = soc_sbx_caladan3_port_queues_update_bmp(unit, fqinfo);
        }
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Failed updating queue bmp"), unit));
        }
    }
    
    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_sws_driver_param_init
 * Purpose:
 *     Initialize SWS driver parameters
 */
int soc_sbx_caladan3_sws_driver_param_init(int unit) 
{
    int status = SOC_E_NONE;
    soc_sbx_caladan3_sws_queue_info_t *queue_info;

    /* Clean up the driver before reinitializing */
    if (sws_dbase[unit] != NULL)
    {
        status = soc_sbx_caladan3_sws_driver_param_uninit(unit);
          if (status != SOC_E_NONE) {
              return status;
          }
    }

    sws_dbase[unit] = sal_alloc(sizeof(soc_sbx_caladan3_sws_dbase_t), 
                                "sws-dbase");
    if (sws_dbase[unit] == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(sws_dbase[unit], 0, sizeof(soc_sbx_caladan3_sws_dbase_t));

    queue_info = sal_alloc(sizeof(soc_sbx_caladan3_sws_queue_info_t) *\
                           SOC_SBX_CALADAN3_SWS_QM_MAX_QUEUES, "sws-queue_dbase");
    if (queue_info == NULL) {
        return SOC_E_MEMORY;
    }

    sal_memset(queue_info, 0, 
               sizeof(soc_sbx_caladan3_sws_queue_info_t) * 
               SOC_SBX_CALADAN3_SWS_QM_MAX_QUEUES);

    sws_dbase[unit]->queue_info = queue_info;
    /* Default to ICC Bypass */
    sws_dbase[unit]->icc_bypass[0] = SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS;
    sws_dbase[unit]->icc_bypass[1] = SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS;

   
    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_sws_driver_param_uninit
 * Purpose:
 *     Clean up SWS driver parameters
 */

int soc_sbx_caladan3_sws_driver_param_uninit(int unit)
{

    /* Check if we have anything to clean up */
    if(sws_dbase[unit] == NULL)
    {
        return SOC_E_NONE;
    }

    /* Free up queue memory if present */
    if (sws_dbase[unit]->queue_info) {
        sal_free(sws_dbase[unit]->queue_info);
    }

    /* Finally free the sws_dbase for the unit */
    sal_free(sws_dbase[unit]);
    sws_dbase[unit] = NULL;

    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_sws_driver_init
 * Purpose:
 *     Bring up SWS drivers
 */
int soc_sbx_caladan3_sws_driver_init(int unit) 
{
    int tdmid;
    int status;

    status = soc_sbx_caladan3_sws_driver_param_init(unit);
    if (SOC_FAILURE(status)) {
        return status;
    }

    status = soc_sbx_caladan3_sws_tdm_config_init(unit);
    if (SOC_FAILURE(status)) {
        return status;
    }
    status = soc_sbx_caladan3_sws_tdm_lookup(unit, &tdmid, &(sws_dbase[unit]->current_sws_cfg));

    if (SOC_SUCCESS(status)) {
        status = soc_sbx_caladan3_sws_tdm_select(unit, tdmid);
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Cannot identify configured TDM or Unsupported TDM configured\n"),
                   unit));

        return status;
    }
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d TDM init failed %d \n"),
                   unit, status));

        soc_sbx_caladan3_sws_driver_param_uninit(unit);
        return status;
    }

    status = soc_sbx_caladan3_sws_allocate_port_queues(unit, FALSE);
    if (SOC_FAILURE(status)) {
        return status;
    }
    
    status = soc_sbx_caladan3_sws_allocate_reserved_queues(unit);
    if (SOC_FAILURE(status)) {
        return status;
    }


    status = soc_sbx_caladan3_sws_queue_info_init(unit);
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Queue init failed %d \n"),
                   unit, status));
        soc_sbx_caladan3_sws_driver_param_uninit(unit);
        return status;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_sws_qm_init(unit));

    if (!SOC_RECONFIG_TDM) {
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_sws_pb_init(unit));
    }

    if (!SAL_BOOT_PLISIM) {
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_sws_pt_init(unit));
    }

    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_sws_pr_init(unit));

    return SOC_E_NONE;
}


extern int soc_sbx_g3p1_higig_loop_enable_get(int unit, uint32 *loop_back_enable);

/*
 *
 * Function:
 *     soc_sbx_caladan3_sws_hotswap
 * Purpose:
 *
 */
int soc_sbx_caladan3_sws_hotswap(int unit) 
{
    int                                             status = SOC_E_NONE;
    int                                             tdmid;
    uint32                                          instance;
    soc_sbx_caladan3_sws_pt_cfg_t                   *pt_cfg = NULL;
    sws_pt_port_fifo_t                              *pt_fifo_cfg = NULL;
    soc_sbx_caladan3_port_map_t                     *port_map = NULL;
    int                                             port;
    soc_sbx_caladan3_port_map_info_t                *port_info = NULL;
    int                                             base_port, blk;
    pr_icc_lookup_core_port_defaults_table_entry_t  entry;
    uint32                                          field;
    sws_pr_port_buffer_t                            pr_f_rx_buf_cfg;

    uint32 loopback_is_enabled = 0;
    uint32 fabric_programmed = 0;

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    status = soc_sbx_caladan3_sws_tdm_lookup(unit, &tdmid, &(sws_dbase[unit]->current_sws_cfg));
    if (SOC_SUCCESS(status)) {
        status = soc_sbx_caladan3_sws_tdm_select(unit, tdmid);
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Cannot identify configured TDM or Unsupported TDM configured\n"),
                   unit));
        return SOC_E_NONE;
    }
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d TDM init failed %d \n"),
                   unit, status));
        soc_sbx_caladan3_sws_driver_param_uninit(unit);
        return SOC_E_NONE;
    }
    
    if (!port_map->reserved_ports_configured) {
        status = soc_sbx_caladan3_sws_allocate_port_queues(unit, TRUE);
        if (SOC_FAILURE(status)) {
            return status;
        }

    }
    
    /* Update the IPTE port FIFO */
    instance=0;
    pt_cfg = soc_sbx_caladan3_sws_pt_cfg_get(unit, instance);
    if (pt_cfg == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d pt_cfg_get failed\n"), unit));
        return SOC_E_PARAM;
    }
    pt_fifo_cfg = pt_cfg->pt_fifo;
    if (pt_fifo_cfg->num_elements <= 0) {
        soc_sbx_caladan3_pt_port_fifo_alloc(unit, instance, pt_fifo_cfg);
    }
    soc_sbx_caladan3_pt_port_fifo_set_hotswap(unit, instance, pt_fifo_cfg);

    /* Update port and client calendars for the line side */
    instance = 0;
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

    if (!port_map->reserved_ports_configured) {

        instance=1;
        /* Set the fabric port fifo table */
        pt_cfg = soc_sbx_caladan3_sws_pt_cfg_get(unit, instance);
        if (pt_cfg == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d pt_cfg_get failed\n"), unit));
            return SOC_E_PARAM;
        }
        pt_fifo_cfg = pt_cfg->pt_fifo;
        if (pt_fifo_cfg->num_elements <= 0) {
            soc_sbx_caladan3_pt_port_fifo_alloc(unit, instance, pt_fifo_cfg);
        }
        soc_sbx_caladan3_pt_port_fifo_set_hotswap(unit, instance, pt_fifo_cfg);

        /* Set the fabric port and client calendars */
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

    soc_sbx_caladan3_sws_pt_port_enable_all_rev_b(unit);

    /* Configure Default ICC mapping */
    for (instance=0; 
         instance < SOC_SBX_CALADAN3_MAX_SYSTEM_INTF; 
         instance++) {

        for (port = 0; port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {
            port_info = (instance == 0) ? &port_map->line_port_info[port]:
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
        
    }

    /* Update source queues */
    status = soc_sbx_caladan3_sws_queue_info_init(unit);
    status = soc_sbx_caladan3_sws_source_queue_update(unit);

    if (!port_map->reserved_ports_configured) {

        status = soc_sbx_caladan3_sws_ipte_init(unit);

        instance = 1;
        soc_sbx_caladan3_pr_port_buffer_alloc(unit, instance, &pr_f_rx_buf_cfg);
        soc_sbx_caladan3_pr_port_buffer_set(unit, instance, &pr_f_rx_buf_cfg);

        SOC_PBMP_ITER(PBMP_ALL(unit), port) {
            if (PBMP_MEMBER(SOC_CONTROL(unit)->all_skip_pbm, port)) {
                continue;
            }
            if (IS_CPU_PORT(unit, port)) {
                continue;
            }
            soc_sbx_caladan3_sws_pr_port_enable(unit, port, TRUE);

        }

    }

    soc_sbx_caladan3_sws_pr_icc_program_port_match_entries(unit, 0, 0, 1);

    /* Update flow control */
    soc_sbx_caladan3_sws_pr_port_enable_all(unit);

    if (port_map->reserved_ports_configured) {

        /* If the fabric port is HG reprogram the fabric TCAM table */
        for (port = 0; port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {
            port_info = &port_map->fabric_port_info[port];
            if (!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                continue;
            }
            if (port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10 ||
                port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25 ||
                port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42 ||
                port_info->intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126) {

                /* Reserved ports & queues are not removed during hotswap. 
                 * Signal to callee here to NOT re-add them!
                 */
                soc_sbx_caladan3_sws_pr_icc_program_sirius_header(unit, 0, 0, 0, 1);
                fabric_programmed = 1;
                break;
            }

        }

        if (!fabric_programmed) {
            /*Required for dev environment support*/
            soc_sbx_g3p1_higig_loop_enable_get(unit, &loopback_is_enabled);
            if (loopback_is_enabled) {
                int debug_force = 1;
                soc_sbx_caladan3_sws_pr_icc_program_sirius_header(unit, debug_force, 0, 0, 1);
            }
        }
    }

    /* Update oversubscription configuration */
    soc_sbx_caladan3_sws_pt_os_scheduler_config(unit);

    port_map->reserved_ports_configured = TRUE;

    return status;
}

int
soc_sbx_caladan3_sws_cmic_to_port_mac_loopback(int unit, int port, int reset)
{
    if (reset <= 0) {
        soc_sbx_caladan3_sws_cmic_mac_loopback_set(unit, CMIC_PORT(unit), 
                                                   port, 1);
    } else {
        soc_sbx_caladan3_sws_cmic_mac_loopback_set(unit, CMIC_PORT(unit), 
                                                   port, 0);
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_cmic_port_hpp_loopback(int unit, int ingress, 
                                           int egress, int reset)
{
    /* Operates only on the PR0 block, sets loopback from cmic */
    if (reset <= 0) {
        soc_sbx_caladan3_sws_cmic_hpp_loopback_map_set(unit, CMIC_PORT(unit), 1);
        if (ingress < 0) {
           /* Egress configured */
           soc_sbx_caladan3_sws_cmic_hpp_loopback_dir_set(unit, 0, 1);
        } else {
           /* Ingress configured */
           soc_sbx_caladan3_sws_cmic_hpp_loopback_dir_set(unit, 1, 1);
        }
    } else {
        soc_sbx_caladan3_sws_cmic_hpp_loopback_map_set(unit, CMIC_PORT(unit), 0);
        soc_sbx_caladan3_sws_cmic_hpp_loopback_dir_set(unit, 1, 0);
    }
    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_sws_get_queue_status(int unit, int qid, int *enabled)
{
    if (enabled == NULL) {
        return SOC_E_PARAM;
    }

    if ((qid < SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE) ||
         (qid > SOC_SBX_CALADAN3_SWS_MAX_QUEUE_ID)) {
        return SOC_E_PARAM;
    } 
    if (!sws_dbase[unit]->queue_info) {
        return SOC_E_INIT;
    }

    *enabled = sws_dbase[unit]->queue_info[qid].enabled;
    return SOC_E_NONE;
}

void
soc_sbx_caladan3_sws_config_dump(int unit) 
{
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_config_status_dump: Invalid unit\n")));
        return;
    }
    if (!sws_dbase[unit]) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_config_status_dump: Not Initialized \n")));
        return;
    }
    LOG_CLI((BSL_META_U(unit,
                        "\nSWS Config Status\n")));
    LOG_CLI((BSL_META_U(unit,
                        "----------------------\n")));
    LOG_CLI((BSL_META_U(unit,
                        "  Current TDM Id      : %d (%s)\n"), sws_dbase[unit]->current_tdm_id,
             (sws_dbase[unit]->current_tdm_name != NULL) ?
             (sws_dbase[unit]->current_tdm_name) : ("unknown")));
    LOG_CLI((BSL_META_U(unit,
                        "  TDM Database status : %s \n"), (sws_dbase[unit]->cfg_valid) ? "Loaded" : "User defined"));
    LOG_CLI((BSL_META_U(unit,
                        "  PR0 ICC:\n")));
    LOG_CLI((BSL_META_U(unit,
                        "      State           : %s \n"), (sws_dbase[unit]->icc_bypass[0]) ? "Bypassed" : "Enabled"));

    LOG_CLI((BSL_META_U(unit,
                        "  PR1 ICC: \n")));
    LOG_CLI((BSL_META_U(unit,
                        "      State           : %s \n"), (sws_dbase[unit]->icc_bypass[1]) ? "Bypassed" : "Enabled"));

    LOG_CLI((BSL_META_U(unit,
                        "  Reserved Queues:\n")));
    LOG_CLI((BSL_META_U(unit,
                        "      Redirect Queues:\n")));
    LOG_CLI((BSL_META_U(unit,
                        "         Ingress Redirect Qid0 : %d \n"), sws_dbase[unit]->ing_redirect_qid0));
    LOG_CLI((BSL_META_U(unit,
                        "         Ingress Redirect Qid1 : %d \n"), sws_dbase[unit]->ing_redirect_qid1));
    LOG_CLI((BSL_META_U(unit,
                        "         Egress Redirect Qid0  : %d \n"), sws_dbase[unit]->egr_redirect_qid0));
    LOG_CLI((BSL_META_U(unit,
                        "         Egress Redirect Qid1  : %d \n"), sws_dbase[unit]->egr_redirect_qid1));
    LOG_CLI((BSL_META_U(unit,
                        "      Bubble Queues:\n")));
    LOG_CLI((BSL_META_U(unit,
                        "         Ingress Bubble Qid    : %d \n"), sws_dbase[unit]->ing_bubble_qid));
    LOG_CLI((BSL_META_U(unit,
                        "         Egress  Bubble Qid    : %d \n"), sws_dbase[unit]->egr_bubble_qid));
    LOG_CLI((BSL_META_U(unit,
                        "      Application Specific:\n")));
    LOG_CLI((BSL_META_U(unit,
                        "          Reserved Queues      : %d\n"), sws_dbase[unit]->app_queues_num));
    LOG_CLI((BSL_META_U(unit,
                        "          Reserved Queue Start : %d\n"), sws_dbase[unit]->app_queues_start));
    LOG_CLI((BSL_META_U(unit,
                        "  SWS Queues Used: \n")));
    LOG_CLI((BSL_META_U(unit,
                        "            Type                   63....3231.....0\n")));
    LOG_CLI((BSL_META_U(unit,
                        "      ---------------------------------------------\n")));
    LOG_CLI((BSL_META_U(unit,
                        "      Ingress Source Queues Used : %08X%08X\n"), sws_dbase[unit]->queues_used[1],sws_dbase[unit]->queues_used[0]));
    LOG_CLI((BSL_META_U(unit,
                        "      Egress  Source Queues Used : %08X%08X\n"), sws_dbase[unit]->queues_used[3],sws_dbase[unit]->queues_used[2]));
    LOG_CLI((BSL_META_U(unit,
                        "      Egress  Dest Queues Used   : %08X%08X\n"), sws_dbase[unit]->queues_used[5],sws_dbase[unit]->queues_used[4]));
    LOG_CLI((BSL_META_U(unit,
                        "      Ingress Dest Queues Used   : %08X%08X\n"), sws_dbase[unit]->queues_used[7],sws_dbase[unit]->queues_used[6]));

}


#endif


