/*
 * $Id:  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains structure and routine declarations for the
 * Switch-on-a-Chip Driver.
 *
 * This file also includes the more common include files so the
 * individual driver files don't have to include as much.
 */
#ifndef SOC_DNX_JER2_ARAD_DRV_H
#define SOC_DNX_JER2_ARAD_DRV_H

#include <shared/cyclic_buffer.h>
#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/TMC/tmc_api_end2end_scheduler.h>

typedef cyclic_buffer_t dnx_captured_buffer_t;

typedef struct soc_dnx_config_jer2_arad_s {
    JER2_ARAD_MGMT_INIT  init;
    int8 voq_mapping_mode; /* supported values are: VOQ_MAPPING_DIRECT, VOQ_MAPPING_INDIRECT */
    int8 port_egress_recycling_scheduler;
    int8 action_type_source_mode; /* supported values are: ACTION_TYPE_FROM_FORWARDING_ACTION, ACTION_TYPE_FROM_QUEUE_SIGNATURE */
    dnx_captured_buffer_t captured_cells_buffer;/*Used for cell dump*/
    dnx_captured_buffer_t sr_cells_buffer;
    uint16 tdm_source_fap_id_offset; /* In case sending VCS256 to a Petra device, the TDM source FAP ID will be the FAP ID + This value */
    int8 xgs_compatability_tm_system_port_encoding; /* supported value 0-1 */
    int8 enable_lpm_custom_lookup; /* lookup {InLIF, TC} in LPM to get a general property of InLIF */
    JER2_ARAD_MGMT_RESERVED_PORT_INFO reserved_ports[SOC_MAX_NUM_PORTS]; /* for dynamic nif */
    soc_pbmp_t reserved_isq_base_q_pair[SOC_DNX_DEFS_MAX(NOF_CORES)]; /* for dynamic nif */
    soc_pbmp_t reserved_fmq_base_q_pair[SOC_DNX_DEFS_MAX(NOF_CORES)]; /* for dynamic nif */
    uint32 region_nof_remote_cores[SOC_DNX_DEFS_MAX(NOF_CORES)][DNX_TMC_COSQ_TOTAL_FLOW_REGIONS];
    int caui_fast_recycle;
    int8 hqos_mapping_enable;
} soc_dnx_config_jer2_arad_t;

#define JER2_ARAD_LPM_CUSTOM_LOOKUP_ENABLED(unit) (SOC_DNX_CONFIG(unit)->jer2_arad->enable_lpm_custom_lookup)
#define SOC_DNX_NOF_IN_RIF_PROFILE_BITS       6
#define SOC_JER2_ARAD_CPU_PHY_CORE                 0 /* The only valid core id in Arad. */

int soc_jer2_arad_isq_hr_get(int unit, bcm_core_t core, int *hr_isq);
int soc_jer2_arad_fmq_base_hr_get(int unit, bcm_core_t core, int** base_hr_fmq);
int soc_jer2_arad_info_config_device_ports(int unit);
int soc_jer2_arad_is_olp(int unit, soc_port_t port, uint32* is_olp);
int soc_jer2_arad_is_oamp(int unit, soc_port_t port, uint32* is_oamp);
int soc_jer2_arad_fc_oob_mode_validate(int unit, int port);
int soc_jer2_arad_default_ofp_rates_set(int unit);
int soc_jer2_arad_ps_reserved_mapping_init(int unit);
int soc_jer2_arad_default_config_get(int unit, soc_dnx_config_jer2_arad_t *cfg);
int soc_jer2_arad_core_frequency_config_get(int unit, int dflt_freq_khz, uint32 *freq_khz);
int soc_jer2_arad_schan_timeout_config_get(int unit, int *schan_timeout);
int soc_jer2_arad_attach(int unit);
int soc_jer2_arad_device_reset(int unit, int mode, int action);
int soc_jer2_arad_reinit(int unit, int reset);
int soc_jer2_arad_info_config(int unit);
int soc_jer2_arad_dma_mutex_init(int unit);
void soc_jer2_arad_dma_mutex_destroy(int unit);
void soc_jer2_arad_free_cache_memory(int unit);

#ifdef BCM_WARM_BOOT_SUPPORT
void soc_jer2_arad_init_empty_scache(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */

int jer2_arad_info_config_custom_reg_access(int unit);
int soc_jer2_arad_validate_hr_is_free(int unit, int core, uint32 base_q_pair, uint8 *is_free);

int soc_jer2_arad_free_tm_port_and_recycle_channel(int unit, int port);
#endif  /* SOC_DNX_JER2_ARAD_DRV_H */
