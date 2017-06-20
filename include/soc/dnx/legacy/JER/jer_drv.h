/*
 * $Id: dfe_drv.h Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains structure and routine declarations for the
 * Switch-on-a-Chip Driver.
 *
 * This file also includes the more common include files so the
 * individual driver files don't have to include as much.
 */
#ifndef _SOC_DNX_JER2_JER_DRV_H
#define _SOC_DNX_JER2_JER_DRV_H

/* 
 * General defines
 */
#include <shared/bitop.h>
#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_traffic_mgmt.h>

#define SOC_JER2_JER_CORE_FREQ_KHZ_DEFAULT 720000

/* Stracture access*/
#define SOC_DNX_JER2_JER_CONFIG(unit)        (SOC_DNX_CONFIG(unit)->jer2_jer)

typedef struct soc_jer2_jer_pll_config_s {
    soc_dnxc_init_serdes_ref_clock_t ref_clk_pmh_in;
    soc_dnxc_init_serdes_ref_clock_t ref_clk_pmh_out;
    soc_dnxc_init_serdes_ref_clock_t ref_clk_pml_in[2];
    soc_dnxc_init_serdes_ref_clock_t ref_clk_pml_out[2];
    soc_dnxc_init_serdes_ref_clock_t ref_clk_fabric_in[2];
    soc_dnxc_init_serdes_ref_clock_t ref_clk_fabric_out[2];
    soc_dnxc_init_serdes_ref_clock_t ref_clk_synce_out[2];
    soc_dnxc_init_serdes_ref_clock_t ref_clk_pmx_in;
    soc_dnxc_init_serdes_ref_clock_t ref_clk_pmx_out;
} soc_jer2_jer_pll_config_t;

typedef struct soc_jer2_jer_excluded_mem_bmap_s {
    SHR_BITDCL    excluded_mems_bmap[_SHR_BITDCLSIZE(NUM_SOC_MEM)];
} soc_jer2_jer_excluded_mem_bmap_t;


/* Jericho specific PP configuration */
typedef struct soc_dnx_config_jer2_jer_pp_s {
  /* Protection */
  uint8   protection_ingress_coupled_mode;          /* Coupled/Decoupled mode for Ingress Protection */
  uint8   protection_egress_coupled_mode;           /* Coupled/Decoupled mode for Egress Protection */
  uint8   protection_fec_accelerated_reroute_mode;  /* FEC Protection accelerated reroute mode */
  uint8   roo_host_arp_msbs;                        /* MSBs of arp pointer for ROO applications */
  uint32  kaps_public_ip_frwrd_table_size;          /* size of public forwarding table in number of entries */
  uint32  kaps_private_ip_frwrd_table_size;         /* size of private forwarding table in number of entries */
  uint32  kaps_large_db_size;                       /* size of direct access DB in number of entries */
  uint8   truncate_delta_in_pp_counter_0;           /* enable/disable packet size compensation for IRPP source in counter command 0 */
  uint8   truncate_delta_in_pp_counter_1;           /* enable/disable packet size compensation for IRPP source in counter command 1 */
} soc_dnx_config_jer2_jer_pp_t;

/* Jericho NIF configuration */

#define SOC_JER2_JERICHO_PM_4x25          6
#define SOC_JER2_JERICHO_PM_4x10          12
#define SOC_JER2_JERICHO_PM_QSGMII        4
#define SOC_JER2_JERICHO_PM_FABRIC        9
#define SOC_QMX_PM_FABRIC            4
#define SOC_JER2_QAX_PM_FABRIC            4
#define SOC_JER2_JERICHO_PLUS_PM_FABRIC   12
#define SOC_JER2_JERICHO_PLUS_PM_4x25     12
#define SOC_JER2_JERICHO_PLUS_PM_4x10     6
#define SOC_JER2_JERICHO_PLUS_PM_QSGMII   2
#define SOC_JER2_JERICHO_CPU_PHY_CORE_0 0
#define SOC_JER2_JERICHO_CPU_PHY_CORE_1 500
#define SOC_JER2_JERICHO_MAX_ILKN_PORTS_OVER_FABRIC 2
#define SOC_JER2_JERICHO_NOF_QMLFS (18)

/* each ILKN requires both pysical resources (serdes) and memory resources */
typedef struct soc_jer2_jer_ilkn_qmlf_resources_s{
    SHR_BITDCLNAME (serdes_qmlfs, SOC_JER2_JERICHO_NOF_QMLFS);
    SHR_BITDCLNAME (memory_qmlfs, SOC_JER2_JERICHO_NOF_QMLFS);
}soc_jer2_jer_ilkn_qmlf_resources_t;

typedef struct soc_jer2_jer_config_nif_s {
    int                                 fw_verify[SOC_DNX_DEFS_MAX(NOF_PMS)];
    int                                 ilkn_over_fabric[SOC_JER2_JERICHO_MAX_ILKN_PORTS_OVER_FABRIC];
    soc_jer2_jer_ilkn_qmlf_resources_t       ilkn_qmlf_resources[SOC_DNX_DEFS_MAX(NOF_INTERLAKEN_PORTS)];
} soc_jer2_jer_config_nif_t;

typedef struct soc_jer2_jer_tm_config_s {
    uint16 nof_remote_faps_with_remote_credit_value;
    DNX_TMC_ITM_CGM_MGMT_GUARANTEE_MODE cgm_mgmt_guarantee_mode;
    int is_ilkn_big_cal;
    int truncate_delta_in_pp_counter[NUM_OF_COUNTERS_CMDS]; /* enable/disable truncate delta (part of packet size compensation feature) in pp for crps use */
    int dedicated_tdm_context;
} soc_jer2_jer_tm_config_t;



/* Main Jericho configuration structure */
typedef struct soc_dnx_config_jer2_jer_s {
    soc_jer2_jer_pll_config_t        pll;
    soc_jer2_jer_excluded_mem_bmap_t excluded_mems;
    soc_jer2_jer_tm_config_t         tm;
} soc_dnx_config_jer2_jer_t;

/* 
 * Init functions 
 */
void soc_restore_bcm88x7x(int unit, int core); 
int soc_dnx_jer2_jericho_init(int unit, int reset_action);
int soc_jer2_jer_device_reset(int unit, int mode, int action);
int soc_jer2_jer_specific_info_config_direct(int unit);
int soc_jer2_jer_specific_info_config_derived(int unit);
int soc_jer2_jer_init_reset(int unit, int reset_action);
int soc_jer2_jer_ports_config(int unit);
int soc_jer2_jer_info_config_custom_reg_access(int unit);

#endif /* !_SOC_DNX_JER2_JER_DRV_H  */

