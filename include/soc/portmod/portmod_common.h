/*
 *         
 * $Id:$
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *         
 *     
 *
 */

#ifndef _PORTMOD_COMMON_H_
#define _PORTMOD_COMMON_H_

#include <soc/portmod/portmod_internal.h>

int portmod_port_chain_core_access_get(int unit, int port, pm_info_t pm_info, phymod_core_access_t* core_access_arr, int max_buf, int* nof_cores);
int portmod_port_chain_phy_access_get(int unit, int port, pm_info_t pm_info, phymod_phy_access_t* core_access_arr, int max_buf, int* nof_cores);

int portmod_common_phy_prbs_config_set(int unit, int port, pm_info_t pm_info, int flags, const phymod_prbs_t* config);
int portmod_common_phy_prbs_config_get(int unit, int port, pm_info_t pm_info, int flags, phymod_prbs_t* config);
int portmod_common_phy_prbs_enable_set(int unit, int port, pm_info_t pm_info, int flags, int enable);
int portmod_common_phy_prbs_enable_get(int unit, int port, pm_info_t pm_info, int flags, int* enable);
int portmod_common_phy_prbs_status_get(int unit, int port, pm_info_t pm_info, int flags, phymod_prbs_status_t* status);
int portmod_common_phy_loopback_set(int unit, int port, pm_info_t pm_info, portmod_loopback_mode_t loopback_type, int enable);
int portmod_common_phy_loopback_get(int unit, int port, pm_info_t pm_info, portmod_loopback_mode_t loopback_type, int *enable);
int portmod_common_phy_firmware_mode_set(int unit, int port, phymod_firmware_mode_t fw_mode);
int portmod_common_phy_firmware_mode_get(int unit, int port, phymod_firmware_mode_t *fw_mode);

int portmod_common_phy_sbus_reg_write(soc_mem_t reg_access_mem, void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t val);
int portmod_common_phy_sbus_reg_read(soc_mem_t reg_access_mem, void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t *val);
int portmod_common_mutex_take(void* user_acc);
int portmod_common_mutex_give(void* user_acc);

int portmod_commmon_portmod_to_phymod_loopback_type(int unit, portmod_loopback_mode_t loopback_type, phymod_loopback_mode_t *phymod_lb_type);

void portmod_common_phy_to_port_ability (phymod_autoneg_ability_t *phyAbility,
                                         portmod_port_ability_t *portAbility);
void portmod_common_port_to_phy_ability (portmod_port_ability_t *portAbility,
                                         phymod_autoneg_ability_t *phyAbility,
                                         int port_num_lanes,
                                         int line_interface,
                                         int cx4_10g,
                                         int an_cl72,
                                         int an_fec,
                                         int hg_mode);

/* Redirect Functions */
int portmod_port_redirect_loopback_set(int unit, soc_port_t port,
                                   int phyn, int phy_lane,
                                   int sys_side, uint32 enable);
int portmod_port_redirect_loopback_get(int unit, soc_port_t port,
                                   int phyn, int phy_lane,
                                   int sys_side, uint32 *enable);

int portmod_port_redirect_autoneg_set(int unit, soc_port_t port,
                                   int phyn, int phy_lane,
                                   int sys_side, phymod_autoneg_control_t* an);
int portmod_port_redirect_autoneg_get(int unit, soc_port_t port,
                                   int phyn, int phy_lane,
                                   int sys_side, phymod_autoneg_control_t* an);

/* FIRMWARE LOAD */

typedef enum portmod_ucode_buf_order_e {
    portmod_ucode_buf_order_straight = 0,
    portmod_ucode_buf_order_half,
    portmod_ucode_buf_order_reversed
} portmod_ucode_buf_order_t;

typedef struct portmod_ucode_buf_s {
    void        *ucode_dma_buf;  /* DMA buffer for firmware load */
    uint32      ucode_alloc_size; /* DMA buffer firmware size */
} portmod_ucode_buf_t;

int
portmod_firmware_set(int unit,
                     int blk_id,
                     const uint8 *array,
                     uint32 datalen,
                     portmod_ucode_buf_order_t data_swap,
                     portmod_ucode_buf_t* buf,
                     portmod_ucode_buf_t* buf_2nd,
                     soc_mem_t ucmem_data,
                     soc_reg_t ucmem_ctrl);

int portmod_intf_from_phymod_intf (int unit,
                                   phymod_interface_t phymod_interface,
                                   soc_port_if_t *interface);
int portmod_intf_to_phymod_intf (int unit, int speed, soc_port_if_t interface,
                                       phymod_interface_t *phymod_interface);

int portmod_line_intf_from_config_get(const portmod_port_interface_config_t *config, soc_port_if_t *interface);

/* Simulator */
extern phymod_bus_t sim_bus;

/* These 2 functions below are a temporary fix - FIX IT */
int portmod_port_status_notify(int unit, int  port, int link);
int portmod_port_flags_test(int unit, soc_port_t port, int flag);

#define PORTMOD_CORE_TO_PHY_ACCESS(_phy_access, _core_access) \
    do{\
        PHYMOD_MEMCPY(&(_phy_access)->access, &(_core_access)->access, sizeof((_phy_access)->access));\
        (_phy_access)->type = (_core_access)->type; \
    }while(0)

int portmod_ext_to_int_cmd_set(int unit, int port, portmod_ext_to_int_phy_ctrlcode_t cmd, void *pdata);
int portmod_ext_to_int_cmd_get(int unit, int port, portmod_ext_to_int_phy_ctrlcode_t cmd, void *pdata);

int portmod_port_is_legacy_ext_phy_present(int unit, int port, int *is_legacy_present);


int portmod_port_ext_phy_control_set(int unit, int port, int phyn, int phy_lane, int sys_side,
                                     soc_phy_control_t control, uint32 value);
int portmod_port_ext_phy_control_get(int unit, int port, int phyn, int phy_lane, int sys_side,
                                     soc_phy_control_t control, uint32* value);

int portmod_port_ext_phy_mdix_set(int unit, int port, soc_port_mdix_t mode);
int portmod_port_ext_phy_mdix_get(int unit, int port, soc_port_mdix_t *mode);
int portmod_port_ext_phy_mdix_status_get(int unit, soc_port_t port,
                                         soc_port_mdix_status_t *status);

int phy_portmod_dispatch_ability_get(int unit, soc_port_t port, soc_port_ability_t *ability);
int phy_portmod_dispatch_interface_set(int unit, soc_port_t port, soc_port_if_t pif);
int phy_portmod_dispatch_nocxn_interface_get(int unit, soc_port_t port, soc_port_if_t *pif);

int portmod_pm_phy_control_set(phymod_phy_access_t*, int, soc_phy_control_t, 
                               phymod_tx_t*, uint32, uint32 v);
int portmod_pm_phy_control_get(phymod_phy_access_t*, int, soc_phy_control_t, 
                               phymod_tx_t*, uint32, uint32 *v);

int portmod_pm_phy_link_mon_enable_set (phymod_phy_access_t*, int, uint32);
int portmod_pm_phy_link_mon_status_get (phymod_phy_access_t*, int);

int portmod_common_timesync_config_set(const phymod_phy_access_t* phy_access,
                                       const portmod_phy_timesync_config_t* conf);
int portmod_common_timesync_config_get(const phymod_phy_access_t* phy_access,
                                       portmod_phy_timesync_config_t* conf);
int portmod_common_control_phy_timesync_set(const phymod_phy_access_t* phy_access,
                                            const portmod_port_control_phy_timesync_t type,
                                            const uint64 value);
int portmod_common_control_phy_timesync_get(const phymod_phy_access_t* phy_access,
                                            const portmod_port_control_phy_timesync_t type,
                                            uint64* value);
int portmod_common_ext_phy_fw_bcst(int unit, pbmp_t pbmp);
int portmod_common_ext_phy_core_reset_for_fw_load(int unit);
int portmod_common_ext_phy_precondition(int unit);

int portmod_media_type_from_port_intf(int unit,
                                    soc_port_if_t interface,
                                    phymod_phy_inf_config_t *phy_interface_config);
int portmod_intf_mode_from_phymod_intf(int unit,
                                    phymod_interface_t phymod_interface,
                                    phymod_phy_inf_config_t *phy_interface_config);

/*!
 * portmod_port_phy_drv_name_get
 *
 * @brief Get the phy driver name attached to the port
 *
 * @param [in]  unit     - unit id
 * @param [in]  port     - port num
 * @param [out] name     - phy driver name.
 * @param [in]  len      - max length of the name buffer.
 */
int portmod_port_phy_drv_name_get(int unit, int port, char *name, int len);

int portmod_common_default_interface_get(portmod_port_interface_config_t* interface_config);
#endif /*_PORTMOD_COMMON_H_*/
