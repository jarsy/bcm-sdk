/*
 * $Id:$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DNXC FABRIC PORTH
 */

#ifndef _SOC_DNXC_PORT_H_
#define _SOC_DNXC_PORT_H_


#include <shared/port.h>
#include <soc/phyctrl.h>

#include <soc/dnxc/legacy/dnxc_defs.h>

#ifdef PORTMOD_SUPPORT
#include <soc/portmod/portmod.h>
#endif

#define SOC_DNXC_PORT_NO_LANE_SWAP      (0x3210)

/* 
 *  Driver current config
 *  RAMON preemphasis soc property -
 *  
 *  pre : bits 00:03
 *  main: bits 04:10
 *  post: bits 11:16
 *  
 *  NOTE for RAMON main contains one extra bit compared with FE1600
 *  
 */
#define SOC_DNXC_PORT_SERDES_PRE_CURRENT_MASK   0xf
#define SOC_DNXC_PORT_SERDES_MAIN_CURRENT_MASK  0x7f0
#define SOC_DNXC_PORT_SERDES_POST_CURRENT_MASK  0x1f800
#define SOC_DNXC_PORT_SERDES_PRE_CURRENT_FIRST  0
#define SOC_DNXC_PORT_SERDES_MAIN_CURRENT_FIRST 4
#define SOC_DNXC_PORT_SERDES_POST_CURRENT_FIRST 11

#define SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT  6


typedef struct dnxc_port_fabric_init_config_s {
    int pcs;
    int speed;
    int cl72;
} dnxc_port_fabric_init_config_t;

typedef enum dnxc_port_init_stage_e {
    dnxc_port_init_full = 0,
    dnxc_port_init_until_fw_load,
    dnxc_port_init_resume_after_fw_load
} dnxc_port_init_stage_t;

typedef enum soc_dnxc_port_pcs_e
{
    soc_dnxc_port_pcs_8_9_legacy_fec        = (int)_SHR_PORT_PCS_8B9BLFEC,
    soc_dnxc_port_pcs_8_10                  = (int)_SHR_PORT_PCS_8B10B,
    soc_dnxc_port_pcs_64_66_fec             = (int)_SHR_PORT_PCS_64B66BFEC,
    soc_dnxc_port_pcs_64_66_bec             = (int)_SHR_PORT_PCS_64B66BBEC,
    soc_dnxc_port_pcs_64_66                 = (int)_SHR_PORT_PCS_64B66B,
    soc_dnxc_port_pcs_64_66_rs_fec          = (int)_SHR_PORT_PCS_64B66B_RS_FEC,
    soc_dnxc_port_pcs_64_66_ll_rs_fec       = (int)_SHR_PORT_PCS_64B66B_LOW_LATENCY_RS_FEC
} soc_dnxc_port_pcs_t;


typedef enum soc_dnxc_port_power_e
{
    soc_dnxc_port_power_off       = 0,
    soc_dnxc_port_power_on        = 1
} soc_dnxc_port_power_t;

typedef enum soc_dnxc_port_prbs_mode_e {
    soc_dnxc_port_prbs_mode_phy = 0,
    soc_dnxc_port_prbs_mode_mac = 1,

    soc_dnxc_port_prbs_mode_count = 2
} soc_dnxc_port_prbs_mode_t;

#define SOC_DNXC_PORT_CONTROL_FLAGS_RX_SERDES_IGNORE         (0x1)

#ifdef PORTMOD_SUPPORT
typedef int (*dnxc_core_address_get_f)(int unit, int core_index, uint16 *address);
soc_error_t soc_dnxc_external_phy_chain_info_get(int unit, int phy, int addresses_array_size, int *addresses, int *phys_in_chain, int *is_clause45);
soc_error_t soc_dnxc_fabric_pms_add(int unit, int cores_num , int first_port, int use_mutex, int quads_in_fsrd, dnxc_core_address_get_f address_get_func, void **user_data);
soc_error_t soc_dnxc_external_phy_core_access_get(int unit, uint32 addr, phymod_access_t *access);
soc_error_t soc_dnxc_fabric_port_probe(int unit, int port, dnxc_port_init_stage_t init_stage, int fw_verify, dnxc_port_fabric_init_config_t* default_port_config);
soc_error_t soc_dnxc_port_cl72_set(int unit, soc_port_t port, int enable);
soc_error_t soc_dnxc_port_cl72_get(int unit, soc_port_t port, int *enable);
soc_error_t soc_dnxc_port_phy_control_set(int unit, soc_port_t port, int phyn, int lane, int is_sys_side, soc_phy_control_t type, uint32 value);
soc_error_t soc_dnxc_port_phy_control_get(int unit, soc_port_t port, int phyn, int lane, int is_sys_side, soc_phy_control_t type, uint32 *value);
soc_error_t soc_dnxc_fabric_pcs_parse(int unit, char *pcs_str, int pcs_def, int *pcs);
soc_error_t soc_dnxc_port_config_get(int unit, soc_port_t port, dnxc_port_fabric_init_config_t* port_config);
int soc_dnxc_fabric_broadcast_firmware_loader(int unit, uint32 length, const uint8* data);

soc_error_t soc_dnxc_port_loopback_set(int unit, soc_port_t port, soc_dnxc_loopback_mode_t loopback);
soc_error_t soc_dnxc_port_loopback_get(int unit, soc_port_t port, soc_dnxc_loopback_mode_t* loopback);

soc_error_t  soc_dnxc_port_enable_set(int unit, soc_port_t port, int enable);
soc_error_t  soc_dnxc_port_enable_get(int unit, soc_port_t port, int *enable);

soc_error_t soc_dnxc_port_phy_reg_get(int unit, soc_port_t port, uint32 flags, uint32 phy_reg_addr, uint32 *phy_data);
soc_error_t soc_dnxc_port_phy_reg_set(int unit, soc_port_t port, uint32 flags, uint32 phy_reg_addr, uint32 phy_data);
soc_error_t soc_dnxc_port_phy_reg_modify(int unit, soc_port_t port, uint32 flags, uint32 phy_reg_addr, uint32 phy_data, uint32 phy_mask);
soc_error_t dnxc_soc_to_phymod_ref_clk(int unit, int  ref_clk, phymod_ref_clk_t *phymod_ref_clk);

/* port controls */
soc_error_t soc_dnxc_port_control_pcs_set(int unit, soc_port_t port, soc_dnxc_port_pcs_t pcs);
soc_error_t soc_dnxc_port_control_pcs_get(int unit, soc_port_t port, soc_dnxc_port_pcs_t *pcs);
soc_error_t soc_dnxc_port_control_power_set(int unit, soc_port_t port, uint32 flags, soc_dnxc_port_power_t power);
soc_error_t soc_dnxc_port_control_power_get(int unit, soc_port_t port, soc_dnxc_port_power_t* power);
soc_error_t soc_dnxc_port_control_rx_enable_set(int unit, soc_port_t port, uint32 flags, int enable);
soc_error_t soc_dnxc_port_control_tx_enable_set(int unit, soc_port_t port, int enable);
soc_error_t soc_dnxc_port_control_rx_enable_get(int unit, soc_port_t port, int* enable);
soc_error_t soc_dnxc_port_control_tx_enable_get(int unit, soc_port_t port, int* enable);
soc_error_t soc_dnxc_port_control_low_latency_llfc_set(int unit, soc_port_t port, int value);
soc_error_t soc_dnxc_port_control_low_latency_llfc_get(int unit, soc_port_t port, int *value);
soc_error_t soc_dnxc_port_control_fec_error_detect_set(int unit, soc_port_t port, int value);
soc_error_t soc_dnxc_port_control_fec_error_detect_get(int unit, soc_port_t port, int *value);
soc_error_t soc_dnxc_port_extract_cig_from_llfc_enable_set(int unit, soc_port_t port, int value);
soc_error_t soc_dnxc_port_extract_cig_from_llfc_enable_get(int unit, soc_port_t port, int *value);
soc_error_t soc_dnxc_port_rx_locked_get(int unit, soc_port_t port, uint32 *rx_locked);

/* PRBS */
soc_error_t soc_dnxc_port_prbs_tx_enable_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value);
soc_error_t soc_dnxc_port_prbs_rx_enable_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value);
soc_error_t soc_dnxc_port_prbs_tx_enable_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode,  int *value);
soc_error_t soc_dnxc_port_prbs_rx_enable_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int *value);
soc_error_t soc_dnxc_port_prbs_rx_status_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int* value);
soc_error_t soc_dnxc_port_prbs_tx_invert_data_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int invert);
soc_error_t soc_dnxc_port_prbs_tx_invert_data_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int *invert);
soc_error_t soc_dnxc_port_prbs_polynomial_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value);
soc_error_t soc_dnxc_port_prbs_polynomial_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int *value);
#endif /*PORTMOD_SUPPORT*/

#endif /*_SOC_DNXC_PORT_H_*/


